/*
 * ETCosimaToMimrecLightCurve.cxx
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

#include "ETCosimaToMimrecLightCurve.h"

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
bool ETCosimaToMimrecLightCurve::Run()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  // Create the light curve: steps with 1 ms ramps
  const MString Curve = "DP 0 1\nDP 29.999 1\nDP 30.001 4\nDP 49.999 4\nDP 50.001 1\nDP 100 1\nEN\n";
  const double Duration = 100.0;
  // Return the integral of the light curve up to t:
  auto Integral = [](double Time) {
    if (Time < 30.0) {
      return Time;
    }
    if (Time < 50.0) {
      return 30.0 + 4.0*(Time - 30.0);
    }
    return 30.0 + 80.0 + (Time - 50.0);
  };
  const double Total = Integral(Duration);

  ETSimScenario LightCurve;
  LightCurve.m_Name = "LightCurve";
  LightCurve.m_Time = Duration;
  LightCurve.m_Files = { { "Curve.dat", Curve } };
  LightCurve.SetFarFieldPointSource(20, 0, "Mono 662", 3.0);
  LightCurve.m_SourceLines.push_back("LightCurve File false Curve.dat");

  ETSimScenario Short, Long;
  Short.m_Name = "Triggers4000";
  Short.m_Triggers = 4000;
  Long.m_Name = "Triggers8000";
  Long.m_Triggers = 8000;
  for (ETSimScenario* Scenario: { &Short, &Long }) {
    Scenario->SetFarFieldPointSource(20, 0, "Mono 662", 2.0);
  }

  vector<ETSimScenario> Scenarios = { LightCurve, Short, Long };
  if (PrepareSimulations(Scenarios) == false) {
    Summarize();
    return false;
  }
  const MString Common = Scenarios[0].m_Directory;
  Passed = EvaluateTrue("CreateRevanConfiguration()", "revan", "The default revan configuration can be created", CreateRevanConfiguration(Common)) && Passed;
  Passed = EvaluateTrue("CreateMimrecConfiguration()", "mimrec", "The default mimrec configuration can be created", CreateMimrecConfiguration(Common)) && Passed;
  // Select an energy window around the line
  const vector<MString> Window = { "EventSelections.FirstEnergyWindow.Min=637", "EventSelections.FirstEnergyWindow.Max=687" };

  vector<future<bool>> Runs;
  for (const ETSimScenario& Scenario: Scenarios) {
    bool LightCurveScenario = false;
    if (Scenario.m_Name == "LightCurve") {
      LightCurveScenario = true;
    }
    Runs.push_back(async(launch::async, [this, Scenario, Common, Window, LightCurveScenario]() {
      if (SimulateAndReconstruct(Scenario, Common + "/revan.cfg") == false) {
        return false;
      }
      if (LightCurveScenario == false) {
        return true;
      }
      if (Mimrec(Scenario, Common + "/mimrec.cfg", "-l", "lightcurve.root", Window) == false) {
        return false;
      }
      return Mimrec(Scenario, Common + "/mimrec.cfg", "-x", "selected.tra", Window);
    }));
  }
  vector<ETSimFileCoreData> Sims;
  vector<ETTraFileCoreData> Tras;
  bool AllRead = true;
  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    Passed = EvaluateTrue("Simulate()", Scenarios[s].m_Name, "Cosima, revan, and mimrec run through", Runs[s].get()) && Passed;
    Sims.push_back(ReadSim(Scenarios[s]));
    Tras.push_back(ReadTra(Scenarios[s]));
    Passed = EvaluateTrue("ReadSim()", Scenarios[s].m_Name, "The sim and tra files can be read", Sims.back().m_FileSuccessfullyRead == true && Tras.back().m_FileSuccessfullyRead == true) && Passed;
    if (Sims.back().m_FileSuccessfullyRead == false || Tras.back().m_FileSuccessfullyRead == false) {
      AllRead = false;
    }
  }
  // Stop only now to report the failures of every scenario
  if (AllRead == false) {
    Summarize();
    return false;
  }

  // The light curve:
  {
    const ETSimFileCoreData& Sim = Sims[0];
    const double Expected = 3.0*Sim.m_StartArea*Duration;
    Passed = EvaluateNear("LightCurve", "generated", "The light curve is normalized so that its mean is the flux: generated particles = flux * area * time (5 sigma Poisson)", static_cast<double>(Sim.m_SimulatedParticles), Expected, 5*sqrt(Expected)) && Passed;

    vector<double> Times;
    vector<unsigned int> Count(3, 0);
    for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
      const double Time = Event->GetTime().GetAsSeconds();
      Times.push_back(Time);
      ++Count[GetTimeBin(Time)];
    }
    const vector<double> Fractions = { 30.0/Total, 80.0/Total, 50.0/Total };
    for (unsigned int i = 0; i < 3; ++i) {
      const double Triggers = static_cast<double>(Times.size());
      Passed = EvaluateNear("LightCurve", MString("interval ") + i, "The triggered events are distributed over the intervals as the integral of the light curve (5 sigma binomial)", static_cast<double>(Count[i]), Triggers*Fractions[i], 5*sqrt(Triggers*Fractions[i]*(1.0 - Fractions[i])) + 1) && Passed;
    }
    sort(Times.begin(), Times.end());
    double Distance = 0.0;
    for (unsigned int i = 0; i < Times.size(); ++i) {
      const double Fraction = Integral(Times[i])/Total;
      Distance = max(Distance, max(fabs(Fraction - static_cast<double>(i)/Times.size()), fabs(Fraction - static_cast<double>(i + 1)/Times.size())));
    }
    Passed = EvaluateNear("LightCurve", "arrival times", "The arrival times follow the light curve (KS)", Distance, 0.0, 2.7/sqrt(static_cast<double>(Times.size()))) && Passed;
  }

  // Check the mimrec light curve against the extracted events and the source
  {
    unique_ptr<TH1D> Curve = FirstHistogram<TH1D>(Scenarios[0].m_Directory + "/lightcurve.root");
    const ETTraFileCoreData Extracted = ReadTraFile(Scenarios[0].m_Directory + "/selected.tra");
    Passed = EvaluateTrue("Mimrec", "light curve", "The light curve and the extracted events of mimrec can be read", Curve != nullptr && Extracted.m_FileSuccessfullyRead == true) && Passed;
    if (Curve != nullptr && Extracted.m_FileSuccessfullyRead == true) {
      // Multiply the rates with the bin widths:
      vector<double> Count(3, 0.0);
      vector<double> OwnCount(3, 0.0);
      for (int b = 1; b <= Curve->GetNbinsX(); ++b) {
        const double Time = Curve->GetXaxis()->GetBinCenter(b);
        Count[GetTimeBin(Time)] += Curve->GetBinContent(b)*Curve->GetXaxis()->GetBinWidth(b);
      }
      for (const shared_ptr<MComptonEvent>& Event: Extracted.Compton()) {
        OwnCount[GetTimeBin(Event->GetTime().GetAsSeconds())] += 1.0;
      }
      const double MimrecTotal = Count[0] + Count[1] + Count[2];
      const double OwnTotal = OwnCount[0] + OwnCount[1] + OwnCount[2];
      mout<<"Mimrec light curve: counts per interval "<<Count[0]<<", "<<Count[1]<<", "<<Count[2]<<" (source: 30, 80, 50 relative; extracted events "<<OwnCount[0]<<", "<<OwnCount[1]<<", "<<OwnCount[2]<<")"<<endl;
      Passed = EvaluateNear("Mimrec", "light curve, total", "The light curve of mimrec contains the events which mimrec extracted (1%)", MimrecTotal, OwnTotal, 0.01*OwnTotal) && Passed;
      const vector<double> Fractions = { 30.0/160.0, 80.0/160.0, 50.0/160.0 };
      for (unsigned int i = 0; i < 3; ++i) {
        Passed = EvaluateNear("Mimrec", MString("light curve, interval ") + i, "The counts of the light curve of mimrec in the interval follow the light curve of the source (5 sigma binomial)", Count[i], MimrecTotal*Fractions[i], 5*sqrt(MimrecTotal*Fractions[i]*(1.0 - Fractions[i])) + 1) && Passed;
      }
    }
  }

  // Runs which stop at a number of triggers:
  for (unsigned int s = 1; s <= 2; ++s) {
    Passed = Evaluate("Triggers", Scenarios[s].m_Name, "The run stops after exactly the requested number of triggers", Sims[s].m_Events.size(), static_cast<size_t>(Scenarios[s].m_Triggers)) && Passed;
  }
  // Expect a relative standard deviation of 1/sqrt(N) for N triggers
  {
    // Expect the relative uncertainty sqrt(1/4000 + 1/8000) of the ratio
    const double Ratio = Sims[2].m_SimulationTime/Sims[1].m_SimulationTime;
    const double Sigma = Ratio*sqrt(1.0/4000.0 + 1.0/8000.0);
    Passed = EvaluateNear("Triggers", "run time", "Twice the triggers take twice the time (5 sigma)", Ratio, 2.0, 5.0*Sigma) && Passed;
  }

  // Ordering and completeness:
  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const ETSimFileCoreData& Sim = Sims[s];
    bool Ordered = true;
    for (unsigned int e = 1; e < Sim.m_Events.size(); ++e) {
      if (Sim.m_Events[e]->GetID() <= Sim.m_Events[e-1]->GetID() || Sim.m_Events[e]->GetTime() < Sim.m_Events[e-1]->GetTime()) {
        Ordered = false;
      }
    }
    Passed = EvaluateTrue("Order", Scenarios[s].m_Name, "The event IDs increase and the event times do not decrease in the sim file", Ordered) && Passed;

    set<int> SimIDs;
    map<int, double> SimTime;
    for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
      SimIDs.insert(Event->GetID());
      SimTime[Event->GetID()] = Event->GetTime().GetAsSeconds();
    }
    unsigned int Unknown = 0;
    double MaxTimeDifference = 0.0;
    for (const shared_ptr<MComptonEvent>& Event: Tras[s].Compton()) {
      if (SimIDs.count(Event->GetId()) == 0) {
        ++Unknown;
      }
      else {
        MaxTimeDifference = GetMaximum(MaxTimeDifference, fabs(Event->GetTime().GetAsSeconds() - SimTime[Event->GetId()]));
      }
    }
    Passed = Evaluate("Order", Scenarios[s].m_Name + ", tra IDs", "Every reconstructed event comes from a triggered event of the sim file", Unknown, 0U) && Passed;
    Passed = EvaluateNear("Order", Scenarios[s].m_Name + ", tra times", "The reconstructed events keep the time of their simulated event", MaxTimeDifference, 0.0, 1e-6) && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the index of the interval of the light curve which contains the time in seconds: 0: before 30 s, 1: before 50 s, 2: later
unsigned int ETCosimaToMimrecLightCurve::GetTimeBin(double Time) const
{
  if (Time < 30.0) {
    return 0;
  }
  if (Time < 50.0) {
    return 1;
  }
  return 2;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecLightCurve Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecLightCurve.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
