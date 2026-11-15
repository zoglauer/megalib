/*
 * UTDSystem.cxx
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


// Standard libs:
#include <cmath>
#include <sstream>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MDGeometry.h"
#include "MDSystem.h"
#include "MTime.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the system class MDSystem
//! Attention: The declared function Reset has no definition and cannot be linked, thus it is not tested
class UTDSystem : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDSystem() : UTGeomegaBase("UTDSystem") {}
  //! Default destructor
  virtual ~UTDSystem() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the name and the default values
  bool TestDefaults();
  //! Test the time resolution type
  bool TestTimeResolutionType();
  //! Test the time resolution sigma
  bool TestTimeResolutionGaussSigma();
  //! Test the noising of the time
  bool TestNoiseTime();
  //! Test the text output
  bool TestStrings();
  //! Test reading the system from a geometry file
  bool TestGeometryFile();
  //! Test that the text of the system can be read again
  bool TestGeomegaRoundTrip();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDSystem::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestTimeResolutionType() && Passed;
  Passed = TestTimeResolutionGaussSigma() && Passed;
  Passed = TestNoiseTime() && Passed;
  Passed = TestStrings() && Passed;
  Passed = TestGeometryFile() && Passed;
  Passed = TestGeomegaRoundTrip() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the name and the default values
bool UTDSystem::TestDefaults()
{
  bool Passed = true;

  MDSystem System("Telescope");
  Passed = EvaluateTrue("GetName()", "constructor argument", "The name of the constructor is stored", System.GetName() == "Telescope") && Passed;
  Passed = EvaluateNear("GetTimeResolutionType()", "new", "The default type is ideal", System.GetTimeResolutionType(), static_cast<double>(MDSystem::c_TimeResolutionTypeIdeal), 1e-15) && Passed;
  Passed = EvaluateNear("GetTimeResolutionGaussSigma()", "new", "The default sigma is 1e-10", System.GetTimeResolutionGaussSigma(), 1e-10, 1e-25) && Passed;
  Passed = EvaluateTrue("Validate()", "new", "A new system is valid", System.Validate()) && Passed;

  // The three types are different:
  Passed = EvaluateTrue("c_TimeResolutionType", "values", "Unknown differs from ideal", MDSystem::c_TimeResolutionTypeUnknown != MDSystem::c_TimeResolutionTypeIdeal) && Passed;
  Passed = EvaluateTrue("c_TimeResolutionType", "values", "Ideal differs from Gauss", MDSystem::c_TimeResolutionTypeIdeal != MDSystem::c_TimeResolutionTypeGauss) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the time resolution type
bool UTDSystem::TestTimeResolutionType()
{
  bool Passed = true;

  MDSystem System("Telescope");

  Passed = EvaluateTrue("SetTimeResolutionType()", "same type", "Setting the current type is accepted", System.SetTimeResolutionType(MDSystem::c_TimeResolutionTypeIdeal)) && Passed;
  Passed = EvaluateTrue("SetTimeResolutionType()", "Gauss", "The type Gauss is accepted", System.SetTimeResolutionType(MDSystem::c_TimeResolutionTypeGauss)) && Passed;
  Passed = EvaluateNear("SetTimeResolutionType()", "Gauss", "The type Gauss is stored", System.GetTimeResolutionType(), static_cast<double>(MDSystem::c_TimeResolutionTypeGauss), 1e-15) && Passed;

  // Invalid types:
  DisableDefaultStreams();
  bool Unknown = System.SetTimeResolutionType(MDSystem::c_TimeResolutionTypeUnknown);
  bool Negative = System.SetTimeResolutionType(-1);
  bool TooLarge = System.SetTimeResolutionType(MDSystem::c_TimeResolutionTypeGauss + 1);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetTimeResolutionType()", "unknown", "The unknown type is refused", Unknown) && Passed;
  Passed = EvaluateFalse("SetTimeResolutionType()", "-1", "A negative type is refused", Negative) && Passed;
  Passed = EvaluateFalse("SetTimeResolutionType()", "n + 1", "The first type after the last one is refused", TooLarge) && Passed;
  Passed = EvaluateNear("SetTimeResolutionType()", "after refusals", "A refused type does not change the stored type", System.GetTimeResolutionType(), static_cast<double>(MDSystem::c_TimeResolutionTypeGauss), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the time resolution sigma
bool UTDSystem::TestTimeResolutionGaussSigma()
{
  bool Passed = true;

  MDSystem System("Telescope");
  System.SetTimeResolutionGaussSigma(2.5E-6);
  Passed = EvaluateNear("SetTimeResolutionGaussSigma()", "2.5e-6", "The value is stored", System.GetTimeResolutionGaussSigma(), 2.5E-6, 1e-21) && Passed;

  DisableDefaultStreams();
  System.SetTimeResolutionGaussSigma(0.0);
  EnableDefaultStreams();
  Passed = EvaluateNear("SetTimeResolutionGaussSigma()", "zero", "Zero is refused, the value stays", System.GetTimeResolutionGaussSigma(), 2.5E-6, 1e-21) && Passed;

  DisableDefaultStreams();
  System.SetTimeResolutionGaussSigma(-1.0);
  EnableDefaultStreams();
  Passed = EvaluateNear("SetTimeResolutionGaussSigma()", "negative", "A negative value is refused, the value stays", System.GetTimeResolutionGaussSigma(), 2.5E-6, 1e-21) && Passed;

  System.SetTimeResolutionGaussSigma(1E-300);
  Passed = EvaluateNear("SetTimeResolutionGaussSigma()", "smallest", "A tiny positive value is accepted", System.GetTimeResolutionGaussSigma() / 1E-300, 1.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising of the time
bool UTDSystem::TestNoiseTime()
{
  bool Passed = true;

  // Ideal: Nothing happens
  MDSystem Ideal("Ideal");
  MTime Time(1000, 500);
  Ideal.NoiseTime(Time);
  Passed = EvaluateTrue("NoiseTime()", "ideal", "An ideal system does not change the time", Time == MTime(1000, 500)) && Passed;

  // Gauss: Mean and width from N trials, with the tolerances from the expected standard errors
  const double Sigma = 1.0E-3;
  const unsigned int N = 10000;
  MDSystem Gauss("Gauss");
  Gauss.SetTimeResolutionType(MDSystem::c_TimeResolutionTypeGauss);
  Gauss.SetTimeResolutionGaussSigma(Sigma);

  gRandom->SetSeed(4711);
  const MTime Start(1000, 0);
  double Sum = 0.0;
  double Sum2 = 0.0;
  double SmallestDifference = 1E300;
  double LargestDifference = -1E300;
  for (unsigned int i = 0; i < N; ++i) {
    MTime Noised = Start;
    Gauss.NoiseTime(Noised);
    double Difference = (Noised - Start).GetAsDouble();
    Sum += Difference;
    Sum2 += Difference*Difference;
    if (Difference < SmallestDifference) {
      SmallestDifference = Difference;
    }
    if (Difference > LargestDifference) {
      LargestDifference = Difference;
    }
  }
  const double Mean = Sum / N;
  const double RMS = sqrt(Sum2 / N - Mean*Mean);

  // Standard error of the mean: sigma / sqrt(N), of the RMS: sigma / sqrt(2 N), 5 standard errors are allowed
  Passed = EvaluateNear("NoiseTime()", "Gauss, 10000 trials", "The mean difference is zero", Mean, 0.0, 5.0*Sigma/sqrt(static_cast<double>(N))) && Passed;
  Passed = EvaluateNear("NoiseTime()", "Gauss, 10000 trials", "The width of the difference is sigma", RMS, Sigma, 5.0*Sigma/sqrt(2.0*N)) && Passed;
  Passed = EvaluateTrue("NoiseTime()", "Gauss, 10000 trials", "Times are shifted to earlier times", SmallestDifference < 0.0) && Passed;
  Passed = EvaluateTrue("NoiseTime()", "Gauss, 10000 trials", "Times are shifted to later times", LargestDifference > 0.0) && Passed;

  // Reusing the time: Two calls are two steps
  MTime Reused(2000, 0);
  gRandom->SetSeed(4712);
  Gauss.NoiseTime(Reused);
  double FirstStep = (Reused - MTime(2000, 0)).GetAsDouble();
  Gauss.NoiseTime(Reused);
  double BothSteps = (Reused - MTime(2000, 0)).GetAsDouble();
  Passed = EvaluateTrue("NoiseTime()", "second call", "The second call adds its own noise", BothSteps != FirstStep) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTDSystem::TestStrings()
{
  bool Passed = true;

  MDSystem System("Telescope");

  ostringstream Out;
  Out<<System;
  Passed = EvaluateTrue("operator<<", "name", "The name is written", Out.str() == "System Telescope\n") && Passed;
  Passed = EvaluateTrue("ToString()", "name", "The text is the same as the one of the stream operator", System.ToString() == "System Telescope\n") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reading the system from a geometry file
bool UTDSystem::TestGeometryFile()
{
  bool Passed = true;

  MString Text = GetGeometryHead();
  Text += "System Telescope\n";
  Text += "Telescope.TimeResolution Gauss 2.0E-6\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "Gauss", "The geometry can be read", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == true) {
    Passed = EvaluateTrue("GetSystem()", "Gauss", "The system is found", Geometry.GetSystem() != nullptr && Geometry.GetSystem()->GetName() == "Telescope") && Passed;
    Passed = EvaluateNear("GetTimeResolutionType()", "Gauss", "The type is read", Geometry.GetSystem()->GetTimeResolutionType(), static_cast<double>(MDSystem::c_TimeResolutionTypeGauss), 1e-15) && Passed;
    Passed = EvaluateNear("GetTimeResolutionGaussSigma()", "Gauss", "The sigma is read", Geometry.GetSystem()->GetTimeResolutionGaussSigma(), 2.0E-6, 1e-20) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the text of the system can be read again
bool UTDSystem::TestGeomegaRoundTrip()
{
  bool Passed = true;

  MDSystem Gauss("Telescope");
  Gauss.SetTimeResolutionType(MDSystem::c_TimeResolutionTypeGauss);
  Gauss.SetTimeResolutionGaussSigma(2.0E-6);
  MString GaussText = Gauss.GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "Gauss", "The system line is written", GaussText.Contains("System Telescope")) && Passed;

  MDGeometry GaussGeometry;
  bool GaussRead = ScanGeometry(GaussGeometry, GetGeometryHead() + GaussText, "Gauss.geo.setup");
  // Defect GEO-B-22: GetGeomega writes the keywords TimeResolutionType and "Time Resolution", but the parser only knows TimeResolution
  Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "round trip Gauss", "The text can be read with the geometry parser", GaussRead)) && Passed;

  MDSystem Ideal("Telescope");
  MString IdealText = Ideal.GetGeomega();
  MDGeometry IdealGeometry;
  bool IdealRead = ScanGeometry(IdealGeometry, GetGeometryHead() + IdealText, "Ideal.geo.setup");
  // Defect GEO-B-22: GetGeomega writes the keywords TimeResolutionType and "Time Resolution", but the parser only knows TimeResolution
  Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "round trip ideal", "The text can be read with the geometry parser", IdealRead)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTDSystem Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDSystem.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
