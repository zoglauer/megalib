/*
 * ETCosimaToMimrecCounts.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "ETCosimaToMimrecCounts.h"

// Standard libs:
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <set>
#include <sstream>


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecCounts::Run()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  // Create the reference scenario, the variations in time and flux, and the rotated directions
  vector<ETSimScenario> Scenarios = {
    Point("Reference", 30, 0, 1.0, 100), Point("DoubleTime", 30, 0, 1.0, 200), Point("DoubleFlux", 30, 0, 2.0, 100),
    Point("Azimuth90", 30, 90, 1.0, 100), Point("Azimuth180", 30, 180, 1.0, 100), Point("Azimuth270", 30, 270, 1.0, 100)
  };
  if (PrepareSimulations(Scenarios) == false) {
    Summarize();
    return false;
  }
  const MString Common = Scenarios[0].m_Directory;
  Passed = EvaluateTrue("CreateRevanConfiguration()", "revan", "The default revan configuration can be created", CreateRevanConfiguration(Common)) && Passed;
  Passed = EvaluateTrue("CreateMimrecConfiguration()", "mimrec", "The default mimrec configuration can be created", CreateMimrecConfiguration(Common)) && Passed;
  // Select an energy window around the line
  const vector<MString> Window = { "EventSelections.FirstEnergyWindow.Min=637", "EventSelections.FirstEnergyWindow.Max=687" };

  // The external programs run in parallel:
  vector<future<bool>> Runs;
  for (const ETSimScenario& Scenario: Scenarios) {
    Runs.push_back(async(launch::async, [this, Scenario, Common, Window]() {
      if (SimulateAndReconstruct(Scenario, Common + "/revan.cfg") == false) {
        return false;
      }
      return Mimrec(Scenario, Common + "/mimrec.cfg", "-x", "selected.tra", Window);
    }));
  }
  vector<ETSimFileCoreData> Sims;
  vector<ETTraFileCoreData> Tras;
  vector<ETTraFileCoreData> Selected;
  bool AllRead = true;
  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    Passed = EvaluateTrue("Simulate()", Scenarios[s].m_Name, "Cosima, revan, and mimrec run through", Runs[s].get()) && Passed;
    Sims.push_back(ReadSim(Scenarios[s]));
    Tras.push_back(ReadTra(Scenarios[s]));
    Selected.push_back(ReadTraFile(Scenarios[s].m_Directory + "/selected.tra"));
    Passed = EvaluateTrue("ReadSim()", Scenarios[s].m_Name, "The sim and tra files can be read", Sims.back().m_FileSuccessfullyRead == true && Tras.back().m_FileSuccessfullyRead == true && Selected.back().m_FileSuccessfullyRead == true) && Passed;
    if (Sims.back().m_FileSuccessfullyRead == false || Tras.back().m_FileSuccessfullyRead == false || Selected.back().m_FileSuccessfullyRead == false) {
      AllRead = false;
    }
  }
  // Stop only now to report the failures of every scenario
  if (AllRead == false) {
    Summarize();
    return false;
  }

  // Absolute counts, simulated time, uniform arrival times:
  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const ETSimScenario& Sc = Scenarios[s];
    const ETSimFileCoreData& Sim = Sims[s];
    double Flux = 1.0;
    for (const MString& Line: Sc.m_SourceLines) {
      if (Line.BeginsWith("Flux ")) {
        Flux = atof(Line.Data() + 5);
      }
    }
    const double Expected = Flux*Sim.m_StartArea*Sc.m_Time;
    Passed = EvaluateNear("ReadSim()", Sc.m_Name + ", start area", "The start area of the far field source is the area of the surrounding sphere of the geometry (10 cm radius)", Sim.m_StartArea, c_Pi*10.0*10.0, 1e-3) && Passed;
    Passed = EvaluateNear("Simulate()", Sc.m_Name + ", time", "The simulated time is the requested time", Sim.m_SimulationTime, Sc.m_Time, 1e-6) && Passed;
    Passed = EvaluateNear("Simulate()", Sc.m_Name + ", generated", "The number of generated particles is flux * start area * time (5 sigma Poisson)", static_cast<double>(Sim.m_SimulatedParticles), Expected, 5*sqrt(Expected)) && Passed;

    vector<double> Times;
    for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
      Times.push_back(Event->GetTime().GetAsSeconds());
    }
    const double Critical = 2.7/sqrt(static_cast<double>(Times.size()));
    Passed = EvaluateNear("Simulate()", Sc.m_Name + ", arrival times", "The arrival times of the triggered events are uniform in the simulated time (KS)", UniformDistance(Times, Sc.m_Time), 0.0, Critical) && Passed;
    Passed = EvaluateTrue("Simulate()", Sc.m_Name + ", enough events", "The scenario has enough triggered events for the statistical checks (> 500)", Sim.m_Events.size() > 500) && Passed;
  }

  // Scaling with time and flux (the generated particles; independent of the detector):
  Passed = EvaluateNear("Scaling", "time", "Twice the time gives twice the generated particles", RatioSigma(static_cast<double>(Sims[1].m_SimulatedParticles), static_cast<double>(Sims[0].m_SimulatedParticles), 2.0), 0.0, 5.0) && Passed;
  Passed = EvaluateNear("Scaling", "flux", "Twice the flux gives twice the generated particles", RatioSigma(static_cast<double>(Sims[2].m_SimulatedParticles), static_cast<double>(Sims[0].m_SimulatedParticles), 2.0), 0.0, 5.0) && Passed;

  // Compare the efficiency and the Compton fraction of the scenarios:
  auto Efficiency = [&](unsigned int s, double& Successes, double& Trials) { Successes = static_cast<double>(Sims[s].m_Events.size()); Trials = static_cast<double>(Sims[s].m_SimulatedParticles); };
  auto ComptonFraction = [&](unsigned int s, double& Successes, double& Trials) { Successes = static_cast<double>(Tras[s].Count(MPhysicalEvent::c_Compton)); Trials = static_cast<double>(Sims[s].m_Events.size()); };
  double K0, N0, K1, N1;
  for (unsigned int s = 1; s < Scenarios.size(); ++s) {
    Efficiency(0, K0, N0);
    Efficiency(s, K1, N1);
    MString What = "The detection efficiency does not depend on the time and the flux";
    if (s >= 3) {
      What = "The detection efficiency is the same for a source direction rotated by 90, 180, and 270 degrees around the detector axis";
    }
    Passed = EvaluateNear("Efficiency", Scenarios[s].m_Name, What + " (two-proportion test)", ProportionSigma(K0, N0, K1, N1), 0.0, 5.0) && Passed;
    ComptonFraction(0, K0, N0);
    ComptonFraction(s, K1, N1);
    Passed = EvaluateNear("ComptonFraction", Scenarios[s].m_Name, "The fraction of reconstructed Compton events is the same as for the reference", ProportionSigma(K0, N0, K1, N1), 0.0, 5.0) && Passed;
  }

  // Check the events which mimrec selects:
  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    set<int> ExpectedIDs;
    for (const shared_ptr<MComptonEvent>& Event: Tras[s].Compton()) {
      const double Total = Event->Eg() + Event->Ee();
      if (Total >= 637.0 && Total <= 687.0 && Tras[s].HasPhysicalScatterAngle(*Event) == true) {
        ExpectedIDs.insert(Event->GetId());
      }
    }
    set<int> SelectedIDs;
    unsigned int NotExpected = 0;
    for (const shared_ptr<MComptonEvent>& Event: Selected[s].Compton()) {
      SelectedIDs.insert(Event->GetId());
      if (ExpectedIDs.count(Event->GetId()) == 0) {
        ++NotExpected;
      }
    }
    unsigned int Missing = 0;
    for (int ID: ExpectedIDs) {
      if (SelectedIDs.count(ID) == 0) {
        ++Missing;
      }
    }
    const unsigned int NotCompton = Selected[s].m_Events.size() - Selected[s].Count(MPhysicalEvent::c_Compton);
    const unsigned int Duplicates = Selected[s].Compton().size() - SelectedIDs.size();
    Passed = EvaluateTrue("Mimrec", Scenarios[s].m_Name + ", expected events", "The energy window contains more than 100 Compton events with a physical scatter angle", ExpectedIDs.size() > 100) && Passed;
    Passed = Evaluate("Mimrec", Scenarios[s].m_Name + ", event type", "Every event which mimrec extracts is a Compton event", NotCompton, 0U) && Passed;
    Passed = Evaluate("Mimrec", Scenarios[s].m_Name + ", subset", "Every event which mimrec extracts is a Compton event in the energy window of the own analysis", NotExpected, 0U) && Passed;
    Passed = Evaluate("Mimrec", Scenarios[s].m_Name + ", duplicates", "Mimrec extracts every event only once", Duplicates, 0U) && Passed;
    Passed = Evaluate("Mimrec", Scenarios[s].m_Name + ", completeness", "Mimrec extracts all Compton events of the energy window with a physical scatter angle", Missing, 0U) && Passed;
    if (s > 0) {
      Passed = EvaluateNear("MimrecEfficiency", Scenarios[s].m_Name, "The number of events selected by mimrec per generated particle is the same as for the reference (two-proportion test)", ProportionSigma(static_cast<double>(Selected[0].Count(MPhysicalEvent::c_Compton)), static_cast<double>(Sims[0].m_SimulatedParticles), static_cast<double>(Selected[s].Count(MPhysicalEvent::c_Compton)), static_cast<double>(Sims[s].m_SimulatedParticles)), 0.0, 5.0) && Passed;
    }
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the scenario of a point source (polar angle and azimuth in degree) with a flux in ph/cm2/s
ETSimScenario ETCosimaToMimrecCounts::Point(const MString& Name, double Theta, double Phi, double Flux, double Time) const
{
  ETSimScenario Scenario;
  Scenario.m_Name = Name;
  Scenario.m_Time = Time;
  Scenario.SetFarFieldPointSource(Theta, Phi, "Mono 662", Flux);
  return Scenario;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the KS distance of the times from the uniform distribution in [0, T]
double ETCosimaToMimrecCounts::UniformDistance(vector<double> Times, double Duration)
{
  sort(Times.begin(), Times.end());
  double Distance = 0.0;
  for (unsigned int i = 0; i < Times.size(); ++i) {
    const double Fraction = Times[i]/Duration;
    Distance = max(Distance, max(fabs(Fraction - static_cast<double>(i)/Times.size()), fabs(Fraction - static_cast<double>(i + 1)/Times.size())));
  }
  return Distance;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecCounts Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecCounts.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
