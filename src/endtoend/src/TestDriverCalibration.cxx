/*
 * TestDriverCalibration.cxx
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

#include "TestDriverCalibration.h"

// Standard libs:
#include <algorithm>
#include <chrono>

// MEGAlib libs:
#include "MSettingsTesting.h"


////////////////////////////////////////////////////////////////////////////////


//! The time in seconds which the fixed chain needs on the reference machine
const double c_ReferenceSeconds = 7.6;

//! The random seed of the fixed chain: every run does the same work
const unsigned int c_CalibrationSeed = 20261008;


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool TestDriverCalibration::Run()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  // The faster of two runs counts: a disturbance of a run makes it slower, never faster
  double Fastest = -1.0;
  for (const MString& Name: vector<MString>({ "CalibrationFirst", "CalibrationSecond" })) {
    const double Time = RunChain(Name);
    Passed = EvaluateTrue("RunChain()", Name, "Cosima, revan, and mimrec run through in the fixed chain", Time > 0.0) && Passed;
    if (Time > 0.0 && (Fastest < 0.0 || Time < Fastest)) {
      Fastest = Time;
    }
  }
  if (Fastest < 0.0) {
    Summarize();
    return false;
  }

  const double Slowdown = Fastest/c_ReferenceSeconds;
  mout<<"Calibration: the fixed chain took "<<Fastest<<" s, the reference machine needs "<<c_ReferenceSeconds<<" s: slowdown "<<Slowdown<<endl;
  MSettingsTesting Settings;
  Settings.Read();
  Settings.SetMachineSlowdown(Slowdown);
  Passed = EvaluateTrue("Write()", "settings", "The slowdown can be stored in the testing settings", Settings.Write()) && Passed;

  MSettingsTesting Stored;
  Stored.Read();
  Passed = EvaluateNear("GetMachineSlowdown()", "stored", "The stored slowdown is the measured one", Stored.GetMachineSlowdown(), Slowdown, 1e-3*Slowdown) && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Run the fixed chain cosima -> revan -> mimrec in a scenario of the given name, return the time in seconds, or a negative value if one of the programs failed
double TestDriverCalibration::RunChain(const MString& Name)
{
  ETSimScenario Scenario;
  Scenario.m_Name = Name;
  Scenario.m_Time = 100;
  Scenario.m_Seed = c_CalibrationSeed;
  Scenario.SetFarFieldPointSource(30, 0, "Mono 662", 5.0);
  if (PrepareSimulation(Scenario) == false) {
    return -1.0;
  }

  const chrono::steady_clock::time_point Start = chrono::steady_clock::now();
  if (CreateRevanConfiguration(Scenario.m_Directory) == false || CreateMimrecConfiguration(Scenario.m_Directory) == false) {
    return -1.0;
  }
  if (SimulateAndReconstruct(Scenario, Scenario.m_Directory + "/revan.cfg") == false) {
    return -1.0;
  }
  if (Mimrec(Scenario, Scenario.m_Directory + "/mimrec.cfg", "-x", "selected.tra", vector<MString>()) == false) {
    return -1.0;
  }
  return chrono::duration<double>(chrono::steady_clock::now() - Start).count();
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  TestDriverCalibration Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// TestDriverCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
