/*
 * UTDStrip3D.cxx
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
using namespace std;

// MEGAlib libs:
#include "UTDDetectorShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the three dimensional strip detector MDStrip3D
class UTDStrip3D : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDStrip3D() : UTDDetectorBase("UTDStrip3D") {}
  //! Default destructor
  virtual ~UTDStrip3D() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the fixture with one wafer: half size 3 x 2 x 1, offset 0.5 x 0.25, 10 x 7 strips - pitch 0.5 x 0.5
  MString GetBody() const;
  //! Return true if the fixture with the keywords is rejected, or if a hit of 300 keV at its center can be noised
  bool IsRejectedOrNoised(const MString& Keywords);

  //! Test the construction, setters, and getters
  bool TestConstruction();
  //! Test the keyword parsing and validation
  bool TestKeywords();
  //! Test the depth resolution of the noising
  bool TestDepthNoise();
  //! Test the depth corrections of the thresholds
  bool TestThresholdDepthCorrection();
  //! Test the depth correction of the energy resolution
  bool TestEnergyResolutionDepthCorrection();
  //! Test the grid
  bool TestGrid();
  //! Test the position resolution
  bool TestPositionResolution();
  //! Test the noising in the volume tree
  bool TestGeometryNoise();
  //! Test the copy, named detectors, and the text output
  bool TestCopyNamedText();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDStrip3D::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestKeywords() && Passed;
  Passed = TestCommonNoiseChain(GetBody(), MVector(0.25, 0.25, 0.5)) && Passed;
  Passed = TestDepthNoise() && Passed;
  Passed = TestThresholdDepthCorrection() && Passed;
  Passed = TestEnergyResolutionDepthCorrection() && Passed;
  Passed = TestGrid() && Passed;
  Passed = TestPositionResolution() && Passed;
  Passed = TestGeometryNoise() && Passed;
  Passed = TestCopyNamedText() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture with one wafer: half size 3 x 2 x 1, offset 0.5 x 0.25, 10 x 7 strips - pitch 0.5 x 0.5
MString UTDStrip3D::GetBody() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "Strip3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the fixture with the keywords is rejected, or if a hit of 300 keV at its center can be noised
bool UTDStrip3D::IsRejectedOrNoised(const MString& Keywords)
{
  MDGeometry Local;
  if (Scan(Local, GetBody(), Keywords) == false) {
    return true;
  }
  MVector Position(0.0, 0.0, 0.0);
  double Energy = 300.0;
  double Time = 0.0;
  Local.GetDetector("D")->Noise(Position, Energy, Time, Local.GetVolume("Box"));
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction, setters, and getters
bool UTDStrip3D::TestConstruction()
{
  bool Passed = true;

  MDStrip3D Strip("Strip");
  Passed = EvaluateTrue("MDStrip3D()", "name", "The name is stored", Strip.GetName() == "Strip") && Passed;
  Passed = Evaluate("MDStrip3D()", "type", "The type is Strip3D", Strip.GetType(), MDDetector::c_Strip3D) && Passed;
  Passed = EvaluateTrue("MDStrip3D()", "type name", "The type name is Strip3D", Strip.GetTypeName() == MDDetector::c_Strip3DName) && Passed;
  Passed = EvaluateFalse("MDStrip3D()", "depth correction", "There is no depth correction of the energy resolution", Strip.HasEnergyResolutionDepthCorrection()) && Passed;
  Passed = EvaluateFalse("MDStrip3D()", "depth correction", "There is no depth correction of the trigger threshold", Strip.HasTriggerThresholdDepthCorrection()) && Passed;
  Passed = EvaluateFalse("MDStrip3D()", "depth correction", "There is no depth correction of the noise threshold", Strip.HasNoiseThresholdDepthCorrection()) && Passed;

  DisableDefaultStreams();
  bool NegativeEnergy = Strip.SetDepthResolutionAt(-1.0, 1.0, 0.1);
  bool ZeroResolution = Strip.SetDepthResolutionAt(1.0, 0.0, 0.1);
  bool NegativeResolution = Strip.SetDepthResolutionAt(1.0, -1.0, 0.1);
  bool NegativeSigma = Strip.SetDepthResolutionAt(1.0, 1.0, -0.1);
  bool ZeroTrigger = Strip.SetTriggerThresholdDepthCorrection(0.0, 0.0);
  bool NegativeTrigger = Strip.SetTriggerThresholdDepthCorrection(0.0, -1.0);
  bool ZeroNoise = Strip.SetNoiseThresholdDepthCorrection(0.0, 0.0);
  bool NegativeNoise = Strip.SetNoiseThresholdDepthCorrection(0.0, -1.0);
  bool PeakZero = Strip.SetEnergyResolutionDepthCorrection(0.0, 0.0, 1.0);
  bool WidthNegative = Strip.SetEnergyResolutionDepthCorrection(0.0, 1.0, -1.0);
  bool Peak2Zero = Strip.SetEnergyResolutionDepthCorrection(0.0, 1.0, 1.0, 0.0);
  bool Width2Negative = Strip.SetEnergyResolutionDepthCorrection(0.0, 1.0, 1.0, 1.0, -1.0);
  bool RatioLow = Strip.SetEnergyResolutionDepthCorrection(0.0, 1.0, 1.0, 1.0, 1.0, -0.1);
  bool RatioHigh = Strip.SetEnergyResolutionDepthCorrection(0.0, 1.0, 1.0, 1.0, 1.0, 1.1);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetDepthResolutionAt()", "negative energy", "The input is rejected", NegativeEnergy) && Passed;
  Passed = EvaluateFalse("SetDepthResolutionAt()", "zero resolution", "The input is rejected", ZeroResolution) && Passed;
  Passed = EvaluateFalse("SetDepthResolutionAt()", "negative resolution", "The input is rejected", NegativeResolution) && Passed;
  Passed = EvaluateFalse("SetDepthResolutionAt()", "negative sigma", "The input is rejected", NegativeSigma) && Passed;
  Passed = EvaluateFalse("SetTriggerThresholdDepthCorrection()", "zero factor", "The input is rejected", ZeroTrigger) && Passed;
  Passed = EvaluateFalse("SetTriggerThresholdDepthCorrection()", "negative factor", "The input is rejected", NegativeTrigger) && Passed;
  Passed = EvaluateFalse("SetNoiseThresholdDepthCorrection()", "zero factor", "The input is rejected", ZeroNoise) && Passed;
  Passed = EvaluateFalse("SetNoiseThresholdDepthCorrection()", "negative factor", "The input is rejected", NegativeNoise) && Passed;
  Passed = EvaluateFalse("SetEnergyResolutionDepthCorrection()", "zero peak", "The input is rejected", PeakZero) && Passed;
  Passed = EvaluateFalse("SetEnergyResolutionDepthCorrection()", "negative width", "The input is rejected", WidthNegative) && Passed;
  Passed = EvaluateFalse("SetEnergyResolutionDepthCorrection()", "zero peak 2", "The input is rejected", Peak2Zero) && Passed;
  Passed = EvaluateFalse("SetEnergyResolutionDepthCorrection()", "negative width 2", "The input is rejected", Width2Negative) && Passed;
  Passed = EvaluateFalse("SetEnergyResolutionDepthCorrection()", "ratio below 0", "The input is rejected", RatioLow) && Passed;
  Passed = EvaluateFalse("SetEnergyResolutionDepthCorrection()", "ratio above 1", "The input is rejected", RatioHigh) && Passed;
  Passed = EvaluateFalse("HasEnergyResolutionDepthCorrection()", "after rejections", "There is still no depth correction", Strip.HasEnergyResolutionDepthCorrection()) && Passed;
  Passed = EvaluateFalse("HasTriggerThresholdDepthCorrection()", "after rejections", "There is still no depth correction", Strip.HasTriggerThresholdDepthCorrection()) && Passed;
  Passed = EvaluateFalse("HasNoiseThresholdDepthCorrection()", "after rejections", "There is still no depth correction", Strip.HasNoiseThresholdDepthCorrection()) && Passed;

  // Valid entries: linear factor from 1 at -1 to 2 at +1
  Passed = EvaluateTrue("SetTriggerThresholdDepthCorrection()", "first point", "The input is accepted", Strip.SetTriggerThresholdDepthCorrection(-1.0, 1.0)) && Passed;
  Passed = EvaluateTrue("SetTriggerThresholdDepthCorrection()", "second point", "The input is accepted", Strip.SetTriggerThresholdDepthCorrection(1.0, 2.0)) && Passed;
  Passed = EvaluateTrue("HasTriggerThresholdDepthCorrection()", "two points", "There is a depth correction", Strip.HasTriggerThresholdDepthCorrection()) && Passed;
  Passed = EvaluateNear("GetTriggerThresholdDepthCorrection()", "center", "The factor is interpolated", Strip.GetTriggerThresholdDepthCorrection(0.0), 1.5, 1e-12) && Passed;
  Passed = EvaluateTrue("SetNoiseThresholdDepthCorrection()", "first point", "The input is accepted", Strip.SetNoiseThresholdDepthCorrection(-1.0, 3.0)) && Passed;
  Passed = EvaluateTrue("SetNoiseThresholdDepthCorrection()", "second point", "The input is accepted", Strip.SetNoiseThresholdDepthCorrection(1.0, 5.0)) && Passed;
  Passed = EvaluateTrue("HasNoiseThresholdDepthCorrection()", "two points", "There is a depth correction", Strip.HasNoiseThresholdDepthCorrection()) && Passed;
  Passed = EvaluateNear("GetNoiseThresholdDepthCorrection()", "center", "The factor is interpolated", Strip.GetNoiseThresholdDepthCorrection(0.0), 4.0, 1e-12) && Passed;
  Passed = EvaluateTrue("SetEnergyResolutionDepthCorrection()", "valid", "The input is accepted", Strip.SetEnergyResolutionDepthCorrection(0.0, 1.0, 1.0)) && Passed;
  Passed = EvaluateTrue("HasEnergyResolutionDepthCorrection()", "one point", "There is a depth correction", Strip.HasEnergyResolutionDepthCorrection()) && Passed;
  Passed = EvaluateTrue("SetDepthResolutionThreshold()", "any value", "The call is successful", Strip.SetDepthResolutionThreshold(5.0)) && Passed;

  // Thresholds
  Strip.SetTriggerThreshold(10.0);
  Strip.SetTriggerThresholdSigma(1.0);
  Strip.SetNoiseThreshold(2.0);
  Passed = EvaluateNear("GetTriggerThreshold()", "center", "The threshold is multiplied by the factor", Strip.GetTriggerThreshold(MVector(0.0, 0.0, 0.0)), 15.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetNoiseThreshold()", "center", "The threshold is multiplied by the factor", Strip.GetNoiseThreshold(MVector(0.0, 0.0, 0.0)), 8.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetSecureUpperLimitTriggerThreshold()", "with correction", "No limit can be given with a correction", Strip.GetSecureUpperLimitTriggerThreshold(), c_LargestEnergy, 1e-3*c_LargestEnergy) && Passed;

  MDStrip3D NoCorrection("NoCorrection");
  NoCorrection.SetTriggerThreshold(10.0);
  NoCorrection.SetTriggerThresholdSigma(1.0);
  NoCorrection.SetNoiseThreshold(2.0);
  Passed = EvaluateNear("GetTriggerThreshold()", "no correction", "The threshold does not depend on the depth", NoCorrection.GetTriggerThreshold(MVector(0.0, 0.0, 0.7)), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetNoiseThreshold()", "no correction", "The threshold does not depend on the depth", NoCorrection.GetNoiseThreshold(MVector(0.0, 0.0, 0.7)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetSecureUpperLimitTriggerThreshold()", "no correction", "The limit is 1.25 T + 7 S", NoCorrection.GetSecureUpperLimitTriggerThreshold(), 19.5, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword parsing and validation
bool UTDStrip3D::TestKeywords()
{
  bool Passed = true;

  const MString Body = GetBody();
  Passed = EvaluateTrue("ScanSetupFile()", "complete", "A complete detector is accepted", IsScanOK(Body)) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "alias", "The class name is an alias of the type", IsScanOK(GetBoxVolume("Box", 3.0, 2.0, 1.0), "MDStrip3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n")) && Passed;

  vector<MString> Wrong = {
    "D.DepthResolution -100 0.1\n",
    "D.DepthResolution 100 -0.1\n",
    "D.DepthResolution 100 0.0\n",
    "D.DepthResolution 100 0.1 -0.1\n",
    "D.DepthResolutionThreshold\n",
    "D.DepthResolutionThreshold 1 2\n",
    "D.EnergyResolutionDepthCorrection\n",
    "D.EnergyResolutionDepthCorrection 0\n",
    "D.EnergyResolutionDepthCorrection 0 1 2\n",
    "D.EnergyResolutionDepthCorrection 0 -1\n",
    "D.EnergyResolutionDepthCorrection Gauss 0 1\n",
    "D.EnergyResolutionDepthCorrection Lorentz 0 1\n",
    "D.EnergyResolutionDepthCorrection GaussLandau 0 1 1\n",
    "D.EnergyResolutionDepthCorrection Unknown 0 1 1\n",
    "D.TriggerThresholdDepthCorrection\n",
    "D.TriggerThresholdDepthCorrection 0\n",
    "D.TriggerThresholdDepthCorrection 0 1 2\n",
    "D.TriggerThresholdDepthCorrection 0 -1\n",
    "D.TriggerThresholdDepthCorrection 0 0\n",
    "D.NoiseThresholdDepthCorrection\n",
    "D.NoiseThresholdDepthCorrection 0\n",
    "D.NoiseThresholdDepthCorrection 0 1 2\n",
    "D.NoiseThresholdDepthCorrection 0 -1\n",
    "D.NoiseThresholdDepthCorrection 0 0\n",
    "D.Voxels 2 2 2\n",
    "D.DirectionalResolution 100 1\n"
  };
  for (const MString& Keyword: Wrong) {
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "The wrong keyword is rejected", IsScanOK(Body, Keyword)) && Passed;
  }

  // Defect GEO-D-036: The result of setting the energy resolution depth correction in its Gauss, Lorentz, and Gauss-Landau forms is ignored by the parser
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "depth correction with a negative width in the Gauss form", "The invalid correction is rejected like in the short form", IsScanOK(Body, "D.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 1\nD.EnergyResolutionDepthCorrection Gauss 0 1 -1\n"))) && Passed;

  // Defect GEO-D-044: The parser of the depth resolution has no error branch for a wrong number of tokens and silently ignores such a line
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "DepthResolution with one value", "The wrong number of tokens is rejected", IsScanOK(Body, "D.DepthResolution 100\n"))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "DepthResolution with four values", "The wrong number of tokens is rejected", IsScanOK(Body, "D.DepthResolution 100 0.1 0.1 0.1\n"))) && Passed;

  // Mixed types
  Passed = EvaluateFalse("ScanSetupFile()", "Lorentz correction with a Gauss table", "A correction of another type is rejected", IsScanOK(Body, "D.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 1\nD.EnergyResolutionDepthCorrection Lorentz 0 1 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "GaussLandau correction with a Gauss table", "A correction of another type is rejected", IsScanOK(Body, "D.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 1\nD.EnergyResolutionDepthCorrection GaussLandau 0 1 1 1 1 0.5\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Gauss correction with a Lorentz table", "A correction of another type is rejected", IsScanOK(Body, "D.EnergyResolution Lorentz 100 100 1\nD.EnergyResolution Lorentz 200 200 1\nD.EnergyResolutionDepthCorrection Gauss 0 1 1\n")) && Passed;

  // Validation
  MDGeometry NoDepth;
  Passed = EvaluateTrue("ScanSetupFile()", "threshold without a depth resolution", "The fixture is accepted", Scan(NoDepth, Body, "D.DepthResolutionThreshold 50\n")) && Passed;
  MDStrip3D Direct("Direct");
  DisableDefaultStreams();
  bool Result = Direct.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no volumes", "A detector without volumes is invalid", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the depth resolution of the noising
bool UTDStrip3D::TestDepthNoise()
{
  bool Passed = true;
  gRandom->SetSeed(5001);

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution", "The fixture is accepted", Scan(Geometry, GetBody(), "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");

  // Widths by interpolation: 0.1 + (E - 100)*0.0005. The cut-off is at more than 5 sigma in all of these three cases
  Passed = CheckGauss("Noise()", "depth at the first table point", SamplePosition(D, V, MVector(0.0, 0.0, 0.0), 100.0, 2), 0.0, 0.1) && Passed;
  Passed = CheckGauss("Noise()", "depth at the table center", SamplePosition(D, V, MVector(0.0, 0.0, 0.0), 300.0, 2), 0.0, 0.2) && Passed;
  Passed = CheckGauss("Noise()", "depth below the table", SamplePosition(D, V, MVector(0.0, 0.0, 0.1), 50.0, 2), 0.1, 0.075) && Passed;

  // x and y are not noised
  Passed = EvaluateNear("Noise()", "x", "The x position stays", MStatistics::GetStandardDeviation(SamplePosition(D, V, MVector(0.25, 0.25, 0.0), 100.0, 0, 1000)), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Noise()", "y", "The y position stays", MStatistics::GetStandardDeviation(SamplePosition(D, V, MVector(0.25, 0.25, 0.0), 100.0, 1, 1000)), 0.0, 1e-12) && Passed;

  // Close to the boundary of the volume: rejection sampling yields a truncated Gaussian
  double Mean = 0.0;
  double RMS = 0.0;
  GetTruncatedGaussian(0.95, 0.3, -1.0, 1.0, Mean, RMS);
  vector<double> Values = SamplePosition(D, V, MVector(0.0, 0.0, 0.95), 500.0, 2);
  Passed = EvaluateNear("Noise()", "depth close to the upper boundary", "The mean is the one of the truncated Gaussian", MStatistics::GetMean(Values), Mean, GetMeanTolerance(RMS)) && Passed;
  Passed = EvaluateNear("Noise()", "depth close to the upper boundary", "The width is the one of the truncated Gaussian", MStatistics::GetStandardDeviation(Values), RMS, GetRMSTolerance(RMS)) && Passed;
  Passed = EvaluateTrue("Noise()", "depth close to the upper boundary", "All values are inside of the volume", *max_element(Values.begin(), Values.end()) <= 1.0 && *min_element(Values.begin(), Values.end()) >= -1.0) && Passed;
  GetTruncatedGaussian(-0.95, 0.3, -1.0, 1.0, Mean, RMS);
  Values = SamplePosition(D, V, MVector(0.0, 0.0, -0.95), 500.0, 2);
  Passed = EvaluateNear("Noise()", "depth close to the lower boundary", "The mean is the one of the truncated Gaussian", MStatistics::GetMean(Values), Mean, GetMeanTolerance(RMS)) && Passed;
  Passed = EvaluateTrue("Noise()", "depth close to the lower boundary", "All values are inside of the volume", *max_element(Values.begin(), Values.end()) <= 1.0 && *min_element(Values.begin(), Values.end()) >= -1.0) && Passed;

  // A hit far outside of the volume never gets a position in the volume
  // Defect GEO-D-037: A position outside the volume by much more than the depth resolution results in an infinite loop of the rejection sampling
  MVector FarPosition(0.0, 0.0, 1000.0);
  double FarEnergy = 100.0;
  double FarTime = 0.0;
  Passed = ExpectedToCrash(EvaluateTrue("Noise()", "position far outside of the volume", "The noising terminates", (D->Noise(FarPosition, FarEnergy, FarTime, V), true))) && Passed;

  // The sigma of the resolution adds in quadrature for the z-distribution: sqrt(0.2^2 + 0.05^2)
  MDGeometry Varying;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution with sigma", "The fixture is accepted", Scan(Varying, GetBody(), "D.DepthResolution 100 0.2 0.05\nD.DepthResolution 500 0.2 0.05\n")) && Passed;
  if (CheckDetector(Passed, "Varying", Varying, "D", MDDetector::c_Strip3D)) {
    double Expected = sqrt(0.2*0.2 + 0.05*0.05);
    Passed = CheckGauss("Noise()", "depth resolution with sigma", SamplePosition(Varying.GetDetector("D"), Varying.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 300.0, 2), 0.0, Expected) && Passed;
  }

  // One point
  MDGeometry OnePoint;
  Passed = EvaluateTrue("ScanSetupFile()", "one depth resolution point", "The fixture is accepted", Scan(OnePoint, GetBody(), "D.DepthResolution 100 0.15\n")) && Passed;
  if (CheckDetector(Passed, "OnePoint", OnePoint, "D", MDDetector::c_Strip3D)) {
    Passed = CheckGauss("Noise()", "one point", SamplePosition(OnePoint.GetDetector("D"), OnePoint.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 1000.0, 2), 0.0, 0.15) && Passed;
  }

  // No depth resolution: the center of the wafer
  MDGeometry NoDepth;
  Passed = EvaluateTrue("ScanSetupFile()", "no depth resolution", "The fixture is accepted", Scan(NoDepth, GetBody(), "D.DepthResolutionThreshold 50\n")) && Passed;
  if (CheckDetector(Passed, "NoDepth", NoDepth, "D", MDDetector::c_Strip3D)) {
    Passed = EvaluateNear("Noise()", "no depth resolution", "The depth is the center", MStatistics::GetMean(SamplePosition(NoDepth.GetDetector("D"), NoDepth.GetVolume("Box"), MVector(0.0, 0.0, 0.7), 100.0, 2, 10)), 0.0, 1e-12) && Passed;
  }

  // Threshold: below no depth resolution
  MDGeometry WithThreshold;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution threshold", "The fixture is accepted", Scan(WithThreshold, GetBody(), "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\nD.DepthResolutionThreshold 50\nD.NoiseThreshold 10\nD.Overflow 400\n")) && Passed;
  if (CheckDetector(Passed, "WithThreshold", WithThreshold, "D", MDDetector::c_Strip3D)) {
    D = WithThreshold.GetDetector("D");
    V = WithThreshold.GetVolume("Box");
    Passed = EvaluateNear("Noise()", "just below the depth threshold", "The depth is the center", MStatistics::GetStandardDeviation(SamplePosition(D, V, MVector(0.0, 0.0, 0.7), 49.999, 2, 100)) + fabs(MStatistics::GetMean(SamplePosition(D, V, MVector(0.0, 0.0, 0.7), 49.999, 2, 100))), 0.0, 1e-12) && Passed;
    // sigma at 50.001 keV is 0.075
    Passed = EvaluateNear("Noise()", "just above the depth threshold", "The depth is noised around the true depth", MStatistics::GetMean(SamplePosition(D, V, MVector(0.0, 0.0, 0.5), 50.001, 2)), 0.5, GetMeanTolerance(0.075)) && Passed;
    Passed = EvaluateNear("Noise()", "just above the depth threshold", "The depth is noised with the width of the table", MStatistics::GetStandardDeviation(SamplePosition(D, V, MVector(0.0, 0.0, 0.5), 50.001, 2)), 0.075, GetRMSTolerance(0.075)) && Passed;
    // Overflow: no depth resolution
    Passed = EvaluateNear("Noise()", "overflow", "The depth of an overflow is the center", MStatistics::GetStandardDeviation(SamplePosition(D, V, MVector(0.0, 0.0, 0.7), 500.0, 2, 100)) + fabs(MStatistics::GetMean(SamplePosition(D, V, MVector(0.0, 0.0, 0.7), 500.0, 2, 100))), 0.0, 1e-12) && Passed;
    // Removed by the noise threshold: the center
    Passed = EvaluateNear("Noise()", "below the noise threshold", "The depth of a removed hit is the center", MStatistics::GetMean(SamplePosition(D, V, MVector(0.0, 0.0, 0.7), 5.0, 2, 100)), 0.0, 1e-12) && Passed;
  }

  // The depth resolution is evaluated with the noised energy, peak 200 instead of 100 keV
  MDGeometry NoisedEnergy;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution at the noised energy", "The fixture is accepted", Scan(NoisedEnergy, GetBody(), "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\nD.EnergyResolution Gauss 100 300 0\nD.EnergyResolution Gauss 500 700 0\n")) && Passed;
  if (CheckDetector(Passed, "NoisedEnergy", NoisedEnergy, "D", MDDetector::c_Strip3D)) {
    Passed = CheckGauss("Noise()", "depth resolution at the noised energy", SamplePosition(NoisedEnergy.GetDetector("D"), NoisedEnergy.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 100.0, 2), 0.0, 0.2) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the depth corrections of the thresholds
bool UTDStrip3D::TestThresholdDepthCorrection()
{
  bool Passed = true;
  gRandom->SetSeed(5002);

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "depth correction of the thresholds", "The fixture is accepted", Scan(Geometry, GetBody(), "D.NoiseThreshold 20\nD.TriggerThreshold 30 0\nD.NoiseThresholdDepthCorrection -1 1\nD.NoiseThresholdDepthCorrection 1 3\nD.TriggerThresholdDepthCorrection -1 1\nD.TriggerThresholdDepthCorrection 1 2\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip3D* D = dynamic_cast<MDStrip3D*>(Geometry.GetDetector("D"));
  MDVolume* V = Geometry.GetVolume("Box");

  // Noise threshold: 20 * (2 + z)
  Passed = EvaluateNear("Noise()", "at the center, just below the threshold of 40 keV", "The hit is removed", NoiseOnce(D, V, MVector(0.0, 0.0, 0.0), 39.999), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Noise()", "at the center, just above the threshold of 40 keV", "The hit stays", NoiseOnce(D, V, MVector(0.0, 0.0, 0.0), 40.001), 40.001, 1e-9) && Passed;
  Passed = EvaluateNear("Noise()", "at the bottom, just below the threshold of 20 keV", "The hit is removed", NoiseOnce(D, V, MVector(0.0, 0.0, -1.0), 19.999), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Noise()", "at the bottom, just above the threshold of 20 keV", "The hit stays", NoiseOnce(D, V, MVector(0.0, 0.0, -1.0), 20.001), 20.001, 1e-9) && Passed;

  // Trigger threshold: 30 * (1.5 + 0.5 z)
  MDGridPoint Center = D->GetGridPoint(MVector(0.0, 0.0, 0.0));
  MDGridPoint Bottom = D->GetGridPoint(MVector(0.0, 0.0, -1.0));
  MDGridPoint Top = D->GetGridPoint(MVector(0.0, 0.0, 1.0));
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "center, just below 45 keV", "There is no trigger", D->IsAboveTriggerThreshold(44.999, Center)) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "center, just above 45 keV", "There is a trigger", D->IsAboveTriggerThreshold(45.001, Center)) && Passed;
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "bottom, just below 30 keV", "There is no trigger", D->IsAboveTriggerThreshold(29.999, Bottom)) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "bottom, just above 30 keV", "There is a trigger", D->IsAboveTriggerThreshold(30.001, Bottom)) && Passed;
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "top, just below 60 keV", "There is no trigger", D->IsAboveTriggerThreshold(59.999, Top)) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "top, just above 60 keV", "There is a trigger", D->IsAboveTriggerThreshold(60.001, Top)) && Passed;

  // With one threshold the depth correction of the trigger threshold is used
  MDGeometry Equal;
  Passed = EvaluateTrue("ScanSetupFile()", "equal thresholds with depth correction", "The fixture is accepted", Scan(Equal, GetBody(), "D.NoiseThresholdEqualsTriggerThreshold true\nD.TriggerThreshold 30 0\nD.TriggerThresholdDepthCorrection -1 1\nD.TriggerThresholdDepthCorrection 1 2\n")) && Passed;
  if (CheckDetector(Passed, "Equal", Equal, "D", MDDetector::c_Strip3D)) {
    Passed = EvaluateNear("Noise()", "equal thresholds, just below 45 keV", "The hit is removed", NoiseOnce(Equal.GetDetector("D"), Equal.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 44.999), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "equal thresholds, just above 45 keV", "The hit stays", NoiseOnce(Equal.GetDetector("D"), Equal.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 45.001), 45.001, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the depth correction of the energy resolution
bool UTDStrip3D::TestEnergyResolutionDepthCorrection()
{
  bool Passed = true;
  gRandom->SetSeed(5003);

  const MString Table = "D.EnergyResolution Gauss 100 100 2\nD.EnergyResolution Gauss 500 500 6\n";

  // New form, at z = 0: peak factor 1.1, width factor 3
  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "depth correction", "The fixture is accepted", Scan(Geometry, GetBody(), Table + "D.EnergyResolutionDepthCorrection Gauss -1 1.0 2\nD.EnergyResolutionDepthCorrection Gauss 1 1.2 4\n")) && Passed;
  if (CheckDetector(Passed, "Geometry", Geometry, "D", MDDetector::c_Strip3D)) {
    MDStrip3D* D = dynamic_cast<MDStrip3D*>(Geometry.GetDetector("D"));
    MDVolume* V = Geometry.GetVolume("Box");
    MVector Center(0.0, 0.0, 0.0);
    MVector Bottom(0.0, 0.0, -1.0);
    Passed = EvaluateTrue("HasEnergyResolutionDepthCorrection()", "keyword", "There is a depth correction", D->HasEnergyResolutionDepthCorrection()) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionPeak1()", "center", "The peak is the table value times the factor", D->GetEnergyResolutionPeak1(300.0, Center), 330.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionWidth1()", "center", "The width is the table value times the factor", D->GetEnergyResolutionWidth1(300.0, Center), 12.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolution()", "center", "The width is the table value times the factor", D->GetEnergyResolution(300.0, Center), 12.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolution()", "bottom", "The width is the table value times the factor", D->GetEnergyResolution(300.0, Bottom), 8.0, 1e-9) && Passed;
    Passed = CheckGauss("Noise()", "depth correction at the center", SampleEnergy(D, V, Center, 300.0), 330.0, 12.0) && Passed;
    Passed = CheckGauss("Noise()", "depth correction at the bottom", SampleEnergy(D, V, Bottom, 300.0), 300.0, 8.0) && Passed;
  }

  // The short form: Only the width is corrected
  MDGeometry Short;
  Passed = EvaluateTrue("ScanSetupFile()", "short depth correction", "The fixture is accepted", Scan(Short, GetBody(), Table + "D.EnergyResolutionDepthCorrection -1 2\nD.EnergyResolutionDepthCorrection 1 4\n")) && Passed;
  if (CheckDetector(Passed, "Short", Short, "D", MDDetector::c_Strip3D)) {
    Passed = CheckGauss("Noise()", "short depth correction at the center", SampleEnergy(Short.GetDetector("D"), Short.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 300.0), 300.0, 12.0) && Passed;
  }

  // Lorentz
  MDGeometry Lorentz;
  Passed = EvaluateTrue("ScanSetupFile()", "Lorentz depth correction", "The fixture is accepted", Scan(Lorentz, GetBody(), "D.EnergyResolution Lorentz 100 100 2\nD.EnergyResolution Lorentz 500 500 6\nD.EnergyResolutionDepthCorrection Lorentz -1 1.0 2\nD.EnergyResolutionDepthCorrection Lorentz 1 1.2 4\n")) && Passed;
  if (CheckDetector(Passed, "Lorentz", Lorentz, "D", MDDetector::c_Strip3D)) {
    Passed = EvaluateNear("GetEnergyResolution()", "Lorentz at the center", "The width is the table value times the factor", Lorentz.GetDetector("D")->GetEnergyResolution(300.0, MVector(0.0, 0.0, 0.0)), 12.0, 1e-9) && Passed;
    Passed = EvaluateNear("Noise()", "Lorentz at the center", "The peak is shifted with the factor", GetMedianDistance(SampleEnergy(Lorentz.GetDetector("D"), Lorentz.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 300.0), 330.0), 2.35/2.0*12.0*tan(0.5*atan(sqrt(0.9975/0.0025))), 1.5*GetMeanTolerance(2.35/2.0*12.0)) && Passed;
  }

  // Gauss-Landau
  MDGeometry Landau;
  MString LandauKeywords = "D.EnergyResolution GaussLandau 100 100 2 100 3 0.4\nD.EnergyResolution GaussLandau 500 500 2 500 3 0.4\nD.EnergyResolutionDepthCorrection GaussLandau -1 1 1 1 1 1.0\nD.EnergyResolutionDepthCorrection GaussLandau 1 1 3 1 5 0.5\n";
  Passed = EvaluateTrue("ScanSetupFile()", "Gauss-Landau depth correction", "The fixture is accepted", Scan(Landau, GetBody(), LandauKeywords)) && Passed;
  if (CheckDetector(Passed, "Landau", Landau, "D", MDDetector::c_Strip3D)) {
    MDDetector* D = Landau.GetDetector("D");
    MVector Center(0.0, 0.0, 0.0);
    Passed = EvaluateNear("GetEnergyResolutionPeak1()", "Gauss-Landau at the center", "The peak is the table value times the factor", D->GetEnergyResolutionPeak1(300.0, Center), 300.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionWidth1()", "Gauss-Landau at the center", "The width is the table value times the factor", D->GetEnergyResolutionWidth1(300.0, Center), 4.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionPeak2()", "Gauss-Landau at the center", "The peak is the table value times the factor", D->GetEnergyResolutionPeak2(300.0, Center), 300.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionWidth2()", "Gauss-Landau at the center", "The width is the table value times the factor", D->GetEnergyResolutionWidth2(300.0, Center), 9.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionRatio()", "Gauss-Landau at the center", "The ratio is the table value times the factor", D->GetEnergyResolutionRatio(300.0, Center), 0.3, 1e-9) && Passed;
    // Sum in quadrature by hand: sqrt((0.4*4)^2 + (0.6*9)^2), with the factors at z = 0: ratio 0.4 of the table only
    double Expected = sqrt(0.4*4.0*0.4*4.0 + 0.6*9.0*0.6*9.0);
    Passed = EvaluateNear("GetEnergyResolution()", "Gauss-Landau at the center", "The result is the weighted sum in quadrature of the widths", D->GetEnergyResolution(300.0, Center), Expected, 1e-9) && Passed;
  }

  // Gauss-Landau table with the short form of the correction
  // Defect GEO-D-038: A Gauss-Landau table with an incomplete depth correction table is accepted and the noising then fails with an uncaught exception
  Passed = ExpectedToCrash(EvaluateTrue("Noise()", "Gauss-Landau table with the short form of the depth correction", "The configuration is either rejected or works", IsRejectedOrNoised("D.EnergyResolution GaussLandau 100 100 2 100 3 0.4\nD.EnergyResolution GaussLandau 500 500 2 500 3 0.4\nD.EnergyResolutionDepthCorrection -1 2\nD.EnergyResolutionDepthCorrection 1 4\n"))) && Passed;

  // Clamping of the base class with a depth correction, width at 400 keV: 5 - 0.04*300 = -7
  MDGeometry Clamp;
  Passed = EvaluateTrue("ScanSetupFile()", "falling width with a depth correction", "The fixture is accepted", Scan(Clamp, GetBody(), "D.EnergyResolution Gauss 100 100 5\nD.EnergyResolution Gauss 200 200 1\nD.EnergyResolutionDepthCorrection Gauss -1 1 1\nD.EnergyResolutionDepthCorrection Gauss 1 1 1\n")) && Passed;
  if (CheckDetector(Passed, "Clamp", Clamp, "D", MDDetector::c_Strip3D)) {
    // Defect GEO-D-039: With a depth correction a width extrapolated below zero is not clamped as in the base class and results in a negative width
    Passed = ExpectedToFail(EvaluateNear("GetEnergyResolutionWidth1()", "extrapolated below zero with a depth correction", "A negative width is clamped to zero", Clamp.GetDetector("D")->GetEnergyResolutionWidth1(400.0, MVector(0.0, 0.0, 0.0)), 0.0, 1e-12)) && Passed;
  }
  MDGeometry ClampRatio;
  Passed = EvaluateTrue("ScanSetupFile()", "ratio with a depth correction", "The fixture is accepted", Scan(ClampRatio, GetBody(), "D.EnergyResolution GaussLandau 100 100 2 100 3 0.9\nD.EnergyResolution GaussLandau 500 500 2 500 3 0.9\nD.EnergyResolutionDepthCorrection GaussLandau -1 1 1 1 1 1.0\nD.EnergyResolutionDepthCorrection GaussLandau 1 1 1 1 1 1.0\nD.EnergyResolutionDepthCorrection GaussLandau 2 1 1 1 1 0.0\n")) && Passed;
  if (CheckDetector(Passed, "ClampRatio", ClampRatio, "D", MDDetector::c_Strip3D)) {
    // Defect GEO-D-039: The ratio is not limited to the range 0..1 as in the base class if a depth correction is used, here the factor is extrapolated to -1 at z = 3
    Passed = ExpectedToFail(EvaluateNear("GetEnergyResolutionRatio()", "ratio correction extrapolated below zero", "The ratio is clamped to zero", ClampRatio.GetDetector("D")->GetEnergyResolutionRatio(300.0, MVector(0.0, 0.0, 3.0)), 0.0, 1e-12)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the grid
bool UTDStrip3D::TestGrid()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "standard", "The fixture is accepted", Scan(Geometry, GetBody())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");

  for (unsigned int i = 0; i < 10; ++i) {
    for (unsigned int j = 0; j < 7; ++j) {
      double Depth = -0.9 + 0.2*i;
      MVector Center(-2.5 + (i + 0.5)*0.5, -1.75 + (j + 0.5)*0.5, Depth);
      Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "strip center", "The center of the strip with the depth is returned", D->GetPositionInDetectorVolume(i, j, 0, MVector(0.0, 0.0, Depth), MDGridPoint::c_Voxel, V), Center, 1e-9) && Passed;
      MDGridPoint Point = D->GetGridPoint(Center);
      Passed = Evaluate("GetGridPoint()", "strip center", "The strip in x is found", Point.GetXGrid(), i) && Passed;
      Passed = Evaluate("GetGridPoint()", "strip center", "The strip in y is found", Point.GetYGrid(), j) && Passed;
      Passed = EvaluateNear("GetGridPoint()", "strip center", "The depth is stored", Point.GetPosition().Z(), Depth, 1e-9) && Passed;
    }
  }

  vector<MDGridPoint> Points = D->Discretize(MVector(0.25, 0.25, 0.5), 123.0, 45.0, V);
  Passed = EvaluateSize("Discretize()", "active area", "There is one grid point", Points.size(), 1) && Passed;
  if (Points.size() == 1) {
    Passed = Evaluate("Discretize()", "x-strip", "The x-strip is correct", Points[0].GetXGrid(), 5u) && Passed;
    Passed = Evaluate("Discretize()", "y-strip", "The y-strip is correct", Points[0].GetYGrid(), 4u) && Passed;
    Passed = EvaluateNear("Discretize()", "depth", "The depth is stored", Points[0].GetPosition().Z(), 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("Discretize()", "energy", "The energy is stored", Points[0].GetEnergy(), 123.0, 1e-12) && Passed;
    Passed = EvaluateNear("Discretize()", "time", "The time is stored", Points[0].GetTime(), 45.0, 1e-12) && Passed;
  }
  DisableDefaultStreams();
  Points = D->Discretize(MVector(-3.5, 0.0, 0.5), 123.0, 45.0, V);
  EnableDefaultStreams();
  Passed = EvaluateSize("Discretize()", "outside", "There is no grid point outside", Points.size(), 0) && Passed;

  // Guard ring
  Passed = EvaluateTrue("HasGuardRing()", "with offset", "There is a guard ring", D->HasGuardRing()) && Passed;
  Passed = Evaluate("GetGridPoint()", "in the band", "The hit is in the guard ring", D->GetGridPoint(MVector(2.75, 0.0, 0.3)).GetType(), MDGridPoint::c_GuardRing) && Passed;
  // Defect GEO-D-040: MDStrip3D overrides GetPositionInDetectorVolume without the handling of the guard ring type of MDStrip2D
  Passed = ExpectedToFail(EvaluateVectorNear("GetPositionInDetectorVolume()", "guard ring", "The unique position of the guard ring is used", D->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_GuardRing, V), MVector(-2.75, -1.875, 0.0), 1e-9)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position resolution
bool UTDStrip3D::TestPositionResolution()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution", "The fixture is accepted", Scan(Geometry, GetBody(), "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MVector Resolution = Geometry.GetDetector("D")->GetPositionResolution(MVector(0.0, 0.0, 0.0), 300.0);
  Passed = EvaluateNear("GetPositionResolution()", "x", "The x resolution is the pitch/sqrt(12)", Resolution.X(), 0.5/sqrt(12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPositionResolution()", "y", "The y resolution is the pitch/sqrt(12)", Resolution.Y(), 0.5/sqrt(12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPositionResolution()", "z", "The depth resolution is interpolated", Resolution.Z(), 0.2, 1e-12) && Passed;

  MDGeometry NoDepth;
  Passed = EvaluateTrue("ScanSetupFile()", "no depth resolution", "The fixture is accepted", Scan(NoDepth, GetBody())) && Passed;
  if (CheckDetector(Passed, "NoDepth", NoDepth, "D", MDDetector::c_Strip3D)) {
    Resolution = NoDepth.GetDetector("D")->GetPositionResolution(MVector(0.0, 0.0, 0.0), 300.0);
    Passed = EvaluateNear("GetPositionResolution()", "no depth resolution, x", "The x resolution is the pitch/sqrt(12)", Resolution.X(), 0.5/sqrt(12.0), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising in the volume tree
bool UTDStrip3D::TestGeometryNoise()
{
  bool Passed = true;
  gRandom->SetSeed(5004);

  // Strip detector in a mother volume, at a local z = 2 + the depth resolution
  MString Translated;
  Translated += GetBoxVolume("Mother", 5.0, 5.0, 5.0, 10.0);
  Translated += GetBoxVolume("Box", 3.0, 2.0, 1.0, 2.0, "Mother");
  Translated += "Strip3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\nD.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\nD.EnergyResolution Gauss 100 100 2\nD.EnergyResolution Gauss 500 500 2\nD.EnergyResolutionDepthCorrection Gauss -1 1 1\nD.EnergyResolutionDepthCorrection Gauss 1 1 3\n";

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "translated detector", "The fixture is accepted", Scan(Geometry, Translated)) && Passed;
  if (Passed == false) {
    return false;
  }
  MDVolumeSequence Sequence = Geometry.GetVolumeSequence(MVector(0.25, 0.25, 12.0), true, true);
  vector<double> Depths;
  vector<double> Energies;
  for (unsigned int i = 0; i < c_Samples; ++i) {
    MVector Position(0.25, 0.25, 12.0);
    double Energy = 100.0;
    double Time = 0.0;
    Geometry.Noise(Position, Energy, Time, Sequence);
    Depths.push_back(Position.Z());
    Energies.push_back(Energy);
  }
  Passed = CheckGauss("MDGeometryQuest::Noise()", "translated detector depth", Depths, 12.0, 0.1) && Passed;
  // Energy resolution factor at the local z = 0 is 2: the width is 4 keV
  Passed = CheckGauss("MDGeometryQuest::Noise()", "translated detector energy", Energies, 100.0, 4.0) && Passed;

  // The resolutions for the user use the world position as depth for the correction
  MVector PositionResolution;
  double EnergyResolution = 0.0;
  double TimeResolution = 0.0;
  Geometry.GetResolutions(MVector(0.25, 0.25, 12.0), 100.0, 0.0, Sequence, PositionResolution, EnergyResolution, TimeResolution);
  // Defect GEO-D-041: MDGeometryQuest::GetResolutions evaluates the depth corrections at the world position and not at the position in the detector
  Passed = ExpectedToFail(EvaluateNear("MDGeometryQuest::GetResolutions()", "translated detector with a depth correction", "The energy resolution is the one at the local depth 0", EnergyResolution, 4.0, 1e-9)) && Passed;

  // Rotated by 90 deg around the x-axis: the depth axis z of the detector is along the y-axis of the world
  MString Rotated;
  Rotated += GetBoxVolume("Box", 3.0, 2.0, 1.0, 5.0);
  Rotated += "Box.Rotation 90 0 0\n";
  Rotated += "Strip3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\nD.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\n";
  MDGeometryQuest GeometryRotated;
  Passed = EvaluateTrue("ScanSetupFile()", "rotated detector", "The fixture is accepted", Scan(GeometryRotated, Rotated)) && Passed;
  if (CheckDetector(Passed, "GeometryRotated", GeometryRotated, "D", MDDetector::c_Strip3D)) {
    MDVolumeSequence Rot = GeometryRotated.GetVolumeSequence(MVector(0.25, 0.25, 5.25), true, true);
    vector<double> X;
    vector<double> Y;
    vector<double> Z;
    for (unsigned int i = 0; i < c_Samples; ++i) {
      MVector Position(0.25, 0.25, 5.25);
      double Energy = 100.0;
      double Time = 0.0;
      GeometryRotated.Noise(Position, Energy, Time, Rot);
      X.push_back(Position.X());
      Y.push_back(Position.Y());
      Z.push_back(Position.Z());
    }
    Passed = EvaluateNear("MDGeometryQuest::Noise()", "rotated detector, x", "The x axis stays", MStatistics::GetStandardDeviation(X), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("MDGeometryQuest::Noise()", "rotated detector, x", "The x axis stays", MStatistics::GetMean(X), 0.25, 1e-9) && Passed;
    Passed = CheckGauss("MDGeometryQuest::Noise()", "rotated detector, y", Y, 0.25, 0.1) && Passed;
    Passed = EvaluateNear("MDGeometryQuest::Noise()", "rotated detector, z", "The world z axis is a strip axis of the detector and stays", MStatistics::GetStandardDeviation(Z), 0.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy, named detectors, and the text output
bool UTDStrip3D::TestCopyNamedText()
{
  bool Passed = true;

  Passed = CheckCopyDataToNamedDetectors<MDStrip3D>("Strip3D", [](MDStrip3D& D) { D.SetNStrips(3, 4); }, [](MDStrip3D& D) { return D.GetNStripsX() == 3 && D.GetNStripsY() == 4; }) && Passed;
  gRandom->SetSeed(5005);

  MString Keywords;
  Keywords += "D.DepthResolution 100 0.1 0.01\nD.DepthResolution 500 0.3 0.03\nD.DepthResolutionThreshold 50\n";
  Keywords += "D.EnergyResolution Gauss 100 100 2\nD.EnergyResolution Gauss 500 500 6\n";
  Keywords += "D.EnergyResolutionDepthCorrection Gauss -1 1 1\nD.EnergyResolutionDepthCorrection Gauss 1 1 3\n";
  Keywords += "D.NoiseThreshold 20\nD.NoiseThresholdDepthCorrection -1 1\nD.NoiseThresholdDepthCorrection 1 3\n";
  Keywords += "D.TriggerThreshold 30\nD.TriggerThresholdDepthCorrection -1 1\nD.TriggerThresholdDepthCorrection 1 2\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "all keywords", "The fixture is accepted", Scan(Geometry, GetBody(), Keywords)) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip3D* D = dynamic_cast<MDStrip3D*>(Geometry.GetDetector("D"));
  MVector Center(0.0, 0.0, 0.0);

  // Clone
  Passed = CheckCloneIndependence("Strip3D", D) && Passed;
  unique_ptr<MDDetector> Cloned(D->Clone());
  MDStrip3D* Copy = dynamic_cast<MDStrip3D*>(Cloned.get());
  if (EvaluateTrue("Clone()", "strip detector", "The clone is a 3D strip detector", Copy != nullptr) == false) {
    return false;
  }
  Passed = EvaluateNear("MDStrip3D(const MDStrip3D&)", "depth resolution", "The depth resolution is copied", Copy->GetPositionResolution(Center, 300.0).Z(), 0.2, 1e-9) && Passed;
  Passed = EvaluateNear("MDStrip3D(const MDStrip3D&)", "energy resolution correction", "The correction is copied", Copy->GetEnergyResolutionWidth1(300.0, Center), 8.0, 1e-9) && Passed;
  Passed = EvaluateNear("MDStrip3D(const MDStrip3D&)", "trigger threshold correction", "The correction is copied", Copy->GetTriggerThresholdDepthCorrection(0.0), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("MDStrip3D(const MDStrip3D&)", "noise threshold correction", "The correction is copied", Copy->GetNoiseThresholdDepthCorrection(0.0), 2.0, 1e-12) && Passed;
  delete Copy->GetGuardRing();

  // Text round trip
  MString Text = D->GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "type", "The type is written", Text.Contains("Strip3D D")) && Passed;
  MDGeometry Second;
  Passed = EvaluateTrue("ScanSetupFile()", "round trip", "The text can be scanned again", Scan(Second, GetBoxVolume("Box", 3.0, 2.0, 1.0), Text)) && Passed;
  if (CheckDetector(Passed, "Second", Second, "D", MDDetector::c_Strip3D)) {
    Passed = Evaluate("GetGeomega()", "round trip, all fields", "The text written after the round trip is identical, so that no written field is lost", dynamic_cast<MDStrip3D*>(Second.GetDetector("D"))->GetGeomega(), Text) && Passed;
    MDStrip3D* R = dynamic_cast<MDStrip3D*>(Second.GetDetector("D"));
    Passed = EvaluateTrue("GetGeomega()", "round trip", "The type is identical", R != nullptr) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The depth resolution is identical", R->GetPositionResolution(Center, 300.0).Z(), 0.2, 1e-9) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The energy resolution correction is identical", R->GetEnergyResolutionWidth1(300.0, Center), 8.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The trigger threshold correction is identical", R->GetTriggerThresholdDepthCorrection(0.0), 1.5, 1e-9) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The noise threshold correction is identical", R->GetNoiseThresholdDepthCorrection(0.0), 2.0, 1e-9) && Passed;
  }

  // A volume name which contains the word
  MString Word = GetBoxVolume("Strip2DWafer", 3.0, 2.0, 1.0) + "Strip3D D\nD.SensitiveVolume Strip2DWafer\nD.DetectorVolume Strip2DWafer\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n";
  MDGeometry GeometryWord;
  Passed = EvaluateTrue("ScanSetupFile()", "volume name with Strip2D", "The fixture is accepted", Scan(GeometryWord, Word)) && Passed;
  if (CheckDetector(Passed, "GeometryWord", GeometryWord, "D", MDDetector::c_Strip3D)) {
    // Defect GEO-D-042: GetGeomega replaces the type name in the whole text, so volume names containing it are changed
    Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "volume name with Strip2D", "The volume name stays", dynamic_cast<MDStrip3D*>(GeometryWord.GetDetector("D"))->GetGeomega().Contains("D.SensitiveVolume Strip2DWafer"))) && Passed;
  }

  Passed = EvaluateTrue("ToString()", "standard", "The type is written", D->ToString().Contains("Strip3D")) && Passed;

  // Named detectors
  MString Named = GetBody() + Keywords + "D.Named N\nN.Assign World.Box\nN.DepthResolution 100 0.4 0\nN.DepthResolution 500 0.4 0\nD.Named M\nM.Assign 1.0 0.0 0.0\n";
  // Two names for the same volume are used here only to look at the inherited values
  MDGeometry GeometryNamed;
  Passed = EvaluateTrue("ScanSetupFile()", "named detector", "The fixture is accepted", Scan(GeometryNamed, Named)) && Passed;
  if (CheckDetector(Passed, "GeometryNamed", GeometryNamed, "N", MDDetector::c_Strip3D)) {
    MDStrip3D* N = dynamic_cast<MDStrip3D*>(GeometryNamed.GetDetector("N"));
    MDStrip3D* M = dynamic_cast<MDStrip3D*>(GeometryNamed.GetDetector("M"));
    Passed = Evaluate("CopyDataToNamedDetectors()", "strips", "The strips are inherited", M->GetNStripsX(), 10) && Passed;
    Passed = EvaluateNear("GetPositionResolution()", "own depth resolution", "The own depth resolution is used", N->GetPositionResolution(Center, 300.0).Z(), 0.4, 1e-9) && Passed;
    Passed = EvaluateNear("GetPositionResolution()", "inherited depth resolution", "The depth resolution is inherited", M->GetPositionResolution(Center, 300.0).Z(), 0.2, 1e-9) && Passed;
    Passed = EvaluateTrue("HasEnergyResolutionDepthCorrection()", "inherited", "The correction is inherited", M->HasEnergyResolutionDepthCorrection()) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionWidth1()", "inherited", "The correction is inherited", M->GetEnergyResolutionWidth1(300.0, Center), 8.0, 1e-9) && Passed;
    Passed = EvaluateTrue("HasTriggerThresholdDepthCorrection()", "inherited", "The correction is inherited", M->HasTriggerThresholdDepthCorrection()) && Passed;
    // Depth threshold: below 50 keV no depth, so the hit stays at the center
    Passed = EvaluateNear("Noise()", "inherited depth threshold", "Below the inherited threshold the depth is the center", MStatistics::GetMean(SamplePosition(M, GeometryNamed.GetVolume("Box"), MVector(0.0, 0.0, 0.5), 30.0, 2, 100)), 0.0, 1e-12) && Passed;
    // Defect GEO-D-043: The copy to named detectors checks the trigger correction flag twice, so the noise threshold depth correction is never inherited
    Passed = ExpectedToFail(EvaluateTrue("HasNoiseThresholdDepthCorrection()", "inherited", "The correction is inherited", M->HasNoiseThresholdDepthCorrection())) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDStrip3D Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDStrip3D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
