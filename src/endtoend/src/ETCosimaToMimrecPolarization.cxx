/*
 * ETCosimaToMimrecPolarization.cxx
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

#include "ETCosimaToMimrecPolarization.h"

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
bool ETCosimaToMimrecPolarization::Run()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  struct Case {
    MString m_Name;
    double m_Theta;
    MString m_Polarization;
  };
  const vector<Case> Cases = {
    { "Unpolarized", 0, "Polarization random" },
    { "Angle0", 0, "Polarization absolute 1.0 1 0 0" },
    { "Angle45", 0, "Polarization absolute 1.0 3 3 0" },
    { "Angle90", 0, "Polarization absolute 1.0 0 1 0" },
    // Off axis: the polarization vector is not perpendicular to the direction
    { "OffAxisUnpolarized", 30, "Polarization random" },
    { "OffAxisProjected", 30, "Polarization absolute 1.0 1 1 1" },
    // Add an independent unpolarized simulation as the background of mimrec
    { "UnpolarizedBackground", 0, "Polarization random" }
  };
  vector<ETSimScenario> Scenarios;
  for (const Case& Setup: Cases) {
    ETSimScenario Scenario;
    Scenario.m_Name = Setup.m_Name;
    Scenario.m_Time = 250;
    Scenario.SetFarFieldPointSource(Setup.m_Theta, 0, "Mono 662", 2.0);
    Scenario.m_SourceLines.push_back(Setup.m_Polarization);
    Scenarios.push_back(Scenario);
  }
  if (PrepareSimulations(Scenarios) == false) {
    Summarize();
    return false;
  }
  const MString Common = Scenarios[0].m_Directory;
  Passed = EvaluateTrue("CreateRevanConfiguration()", "revan", "The default revan configuration can be created", CreateRevanConfiguration(Common)) && Passed;
  Passed = EvaluateTrue("CreateMimrecConfiguration()", "mimrec", "The default mimrec configuration can be created", CreateMimrecConfiguration(Common)) && Passed;

  vector<future<bool>> Runs;
  for (const ETSimScenario& Scenario: Scenarios) {
    Runs.push_back(async(launch::async, [this, Scenario, Common]() { return SimulateAndReconstruct(Scenario, Common + "/revan.cfg"); }));
  }
  vector<ETTraFileCoreData> Tras;
  bool AllRead = true;
  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    Passed = EvaluateTrue("Simulate()", Scenarios[s].m_Name, "Cosima and revan run through", Runs[s].get()) && Passed;
    Tras.push_back(ReadTra(Scenarios[s]));
    Passed = EvaluateTrue("ReadTra()", Scenarios[s].m_Name, "The tra file can be read", Tras.back().m_FileSuccessfullyRead) && Passed;
    if (Tras.back().m_FileSuccessfullyRead == false) {
      AllRead = false;
    }
  }
  // Stop only now to report the failures of every scenario
  if (AllRead == false) {
    Summarize();
    return false;
  }

  // Check the mimrec polarization analysis on axis, with the second unpolarized simulation as background:
  const vector<MString> Window = { "EventSelections.FirstEnergyWindow.Min=637", "EventSelections.FirstEnergyWindow.Max=687", "TestPositions.Use=true",
                                   "TestPositions.CoordinateSystemSpherical.Theta=0", "TestPositions.CoordinateSystemSpherical.Phi=0" };
  const unsigned int Background = 6;
  struct MimrecResult { bool m_Valid = false; double m_Modulation = 0, m_ModulationError = 0, m_Angle = 0, m_AngleError = 0; };
  vector<MimrecResult> Mimrecs(4);
  {
    // Run the analyses in parallel:
    vector<future<bool>> Analyses;
    const MString BackgroundFile = Scenarios[Background].m_Directory + "/" + Scenarios[Background].m_Name + ".inc1.id1.tra.gz";
    for (unsigned int s = 0; s < 4; ++s) {
      const ETSimScenario Scenario = Scenarios[s];
      vector<MString> Changes = Window;
      Changes.push_back(MString("Polarization.BackgroundFile=") + BackgroundFile);
      Analyses.push_back(async(launch::async, [this, Scenario, Common, Changes]() { return Mimrec(Scenario, Common + "/mimrec.cfg", "-p", "pol.root", Changes); }));
    }
    for (unsigned int s = 0; s < 4; ++s) {
      Passed = EvaluateTrue("Mimrec", Scenarios[s].m_Name + ", polarization analysis", "Mimrec runs the polarization analysis", Analyses[s].get()) && Passed;
      // Read the modulation and the angle from the log of mimrec:
      for (const MString& Line: ReadLines(Scenarios[s].m_Directory + "/mimrec_pol.root.log")) {
        if (Line.BeginsWith("Modulation:") == true) {
          const MString Rest = Line.GetSubString(11);
          const size_t Plus = Rest.FindFirst("+-");
          if (Plus != string::npos) {
            Mimrecs[s].m_Modulation = Rest.ToDouble();
            Mimrecs[s].m_ModulationError = Rest.GetSubString(Plus + 2).ToDouble();
          }
        } else if (Line.BeginsWith("Polarization angle:") == true) {
          const size_t Open = Line.First('(');
          const size_t Plus = Line.FindFirst("+-");
          if (Open != string::npos && Plus != string::npos) {
            Mimrecs[s].m_Angle = Line.GetSubString(Open + 1).ToDouble();
            Mimrecs[s].m_AngleError = Line.GetSubString(Plus + 2).ToDouble();
            Mimrecs[s].m_Valid = true;
          }
        }
      }
      Passed = EvaluateTrue("Mimrec", Scenarios[s].m_Name + ", result", "The modulation and the polarization angle can be read from the output of mimrec", Mimrecs[s].m_Valid) && Passed;
    }
  }

  // On axis: the incident direction is -z, the axes of the plane are x and y
  const MVector AxisX(1.0, 0.0, 0.0);
  const MVector AxisY(0.0, 1.0, 0.0);
  const MVector DownZ(0.0, 0.0, -1.0);
  vector<Modulation> Modulations;
  for (unsigned int s = 0; s < 4; ++s) {
    Modulations.push_back(Measure(Tras[s], AxisX, AxisY, DownZ));
    mout<<Scenarios[s].m_Name<<": "<<Modulations.back().m_N<<" full energy Compton events, modulation vector ("<<Modulations.back().m_Cos<<", "<<Modulations.back().m_Sin<<") +- "<<Modulations.back().Sigma()<<endl;
    Passed = EvaluateTrue("Polarization", Scenarios[s].m_Name + ", events", "There are enough full energy Compton events (> 3000)", Modulations.back().m_N > 3000) && Passed;
  }
  const double Five = 5.0;
  // Unpolarized: zero modulation
  Passed = EvaluateNear("Polarization", "Unpolarized, cos", "Unpolarized photons have no modulation: <cos 2 eta> (5 sigma)", Modulations[0].m_Cos, 0.0, Five*Modulations[0].Sigma()) && Passed;
  Passed = EvaluateNear("Polarization", "Unpolarized, sin", "Unpolarized photons have no modulation: <sin 2 eta> (5 sigma)", Modulations[0].m_Sin, 0.0, Five*Modulations[0].Sigma()) && Passed;

  // Expect the modulation (-M cos 2 alpha, -M sin 2 alpha) - the acceptance changes the amplitude, not the phase:
  const vector<double> Angles = { 0.0, 45.0, 90.0 };
  for (unsigned int a = 0; a < 3; ++a) {
    const Modulation& Measured = Modulations[a + 1];
    const double Size = sqrt(Measured.m_Cos*Measured.m_Cos + Measured.m_Sin*Measured.m_Sin);
    const MString Name = MString("Angle") + static_cast<int>(Angles[a]);
    // Check the significance of the modulation:
    Passed = EvaluateNear("Polarization", Name + ", modulation", "Photons which are polarized scatter perpendicular to their polarization: the modulation is significant (> 5 sigma; shown: the shortfall) and below 100%", max(Five*Measured.Sigma() - Size, 0.0) + max(Size - 1.0, 0.0), 0.0, 0.0) && Passed;
    double Angle = 0.5*atan2(-Measured.m_Sin, -Measured.m_Cos)*c_Deg;
    double Difference = fabs(Angle - Angles[a]);
    if (Difference > 90.0) {
      Difference = 180.0 - Difference;
    }
    // Get the uncertainty of the angle: sigma/(2*size) in radians
    const double AngleSigma = 0.5*Measured.Sigma()/Size*c_Deg;
    mout<<Name<<": modulation "<<Size<<", measured polarization angle "<<Angle<<" +- "<<AngleSigma<<" deg"<<endl;
    Passed = EvaluateNear("Polarization", Name + ", phase", "The polarization angle which is measured from the modulation is the polarization angle of the source (5 sigma + 2 degrees)", Difference, 0.0, Five*AngleSigma + 2.0) && Passed;
  }

  // Off axis: project the polarization vector onto the plane perpendicular to the flight direction
  MVector ToSource;
  ToSource.SetMagThetaPhi(1.0, 30.0*c_Rad, 0.0);
  const MVector Flight = ToSource*(-1.0);
  const MVector Given(1.0, 1.0, 1.0);
  const MVector Projected = (Given - Flight*Given.Dot(Flight)).Unit();
  const MVector Other = Flight.Cross(Projected);
  const Modulation Control = Measure(Tras[4], Projected, Other, Flight);
  const Modulation Polarized = Measure(Tras[5], Projected, Other, Flight);
  mout<<"Off axis: control modulation ("<<Control.m_Cos<<", "<<Control.m_Sin<<") +- "<<Control.Sigma()<<", polarized relative to the projected vector ("<<Polarized.m_Cos<<", "<<Polarized.m_Sin<<") +- "<<Polarized.Sigma()<<endl;
  // Expect <cos 2 eta> below the unpolarized control:
  const double Difference = Control.m_Cos - Polarized.m_Cos;
  const double DifferenceSigma = sqrt(Control.Sigma()*Control.Sigma() + Polarized.Sigma()*Polarized.Sigma());
  Passed = EvaluateNear("Polarization", "OffAxis, modulation", "The photons which are polarized along the projected vector scatter perpendicular to it: <cos 2 eta> is lower than for the unpolarized control (> 5 sigma; shown: the shortfall)", max(Five*DifferenceSigma - Difference, 0.0), 0.0, 0.0) && Passed;
  // Expect the sin component of the control:
  Passed = EvaluateNear("Polarization", "OffAxis, phase", "The modulation is along the projected polarization vector: the sin component equals the one of the control (5 sigma)", Polarized.m_Sin, Control.m_Sin, Five*DifferenceSigma) && Passed;

  // Check the mimrec results: angle, modulation, and no modulation unpolarized
  for (unsigned int a = 0; a < 3; ++a) {
    const MimrecResult& Analysis = Mimrecs[a + 1];
    if (Analysis.m_Valid == false) {
      continue;
    }
    const MString Name = MString("Angle") + static_cast<int>(Angles[a]);
    double Difference = fabs(Analysis.m_Angle - Angles[a]);
    Difference = fmod(Difference, 180.0);
    if (Difference > 90.0) {
      Difference = 180.0 - Difference;
    }
    mout<<Name<<": mimrec modulation "<<Analysis.m_Modulation<<" +- "<<Analysis.m_ModulationError<<", polarization angle "<<Analysis.m_Angle<<" +- "<<Analysis.m_AngleError<<" deg"<<endl;
    Passed = EvaluateNear("Mimrec", Name + ", modulation", "The modulation which mimrec measures is significant (> 4 sigma; shown: the shortfall)", max(4.0*Analysis.m_ModulationError - Analysis.m_Modulation, 0.0), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("Mimrec", Name + ", angle", "The polarization angle which mimrec measures is the polarization angle of the source (5 sigma + 2 degrees)", Difference, 0.0, 5.0*Analysis.m_AngleError + 2.0) && Passed;
    // Compare with the own analysis - the mimrec modulation is twice <cos 2 eta>
    const Modulation& Own = Modulations[a + 1];
    const double OwnAmplitude = 2.0*sqrt(Own.m_Cos*Own.m_Cos + Own.m_Sin*Own.m_Sin);
    Passed = EvaluateNear("Mimrec", Name + ", modulation size", "The modulation of mimrec is twice the modulation of the own analysis (within 4 sigma of the two uncertainties and 30% of the amplitude)", Analysis.m_Modulation, OwnAmplitude, 4.0*sqrt(Analysis.m_ModulationError*Analysis.m_ModulationError + 4.0*Own.Sigma()*Own.Sigma()) + 0.3*OwnAmplitude) && Passed;
  }
  if (Mimrecs[0].m_Valid == true) {
    Passed = EvaluateNear("Mimrec", "Unpolarized, modulation", "An unpolarized source has no significant modulation in mimrec (5 sigma)", Mimrecs[0].m_Modulation, 0.0, 5.0*Mimrecs[0].m_ModulationError) && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Measure the modulation of the events of a tra file
ETCosimaToMimrecPolarization::Modulation ETCosimaToMimrecPolarization::Measure(const ETTraFileCoreData& Tra, const MVector& E1, const MVector& E2, const MVector& Direction)
{
  Modulation Result;
  double SumCos = 0.0, SumSin = 0.0;
  for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
    if (fabs(Event->Eg() + Event->Ee() - 662.0) > 25.0) {
      continue;
    }
    const MVector Scatter = (Event->C2() - Event->C1()).Unit();
    const MVector Perpendicular = Scatter - Direction*Scatter.Dot(Direction);
    if (Perpendicular.Mag() < 1e-6) {
      continue;
    }
    const double Eta = atan2(Perpendicular.Dot(E2), Perpendicular.Dot(E1));
    SumCos += cos(2.0*Eta);
    SumSin += sin(2.0*Eta);
    ++Result.m_N;
  }
  if (Result.m_N > 0) {
    Result.m_Cos = SumCos/Result.m_N;
    Result.m_Sin = SumSin/Result.m_N;
  }
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecPolarization Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecPolarization.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
