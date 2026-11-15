/*
 * UTDAngerCamera.cxx
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


//! Unit test of the Anger camera MDAngerCamera
class UTDAngerCamera : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDAngerCamera() : UTDDetectorBase("UTDAngerCamera") {}
  //! Default destructor
  virtual ~UTDAngerCamera() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the fixture without a position resolution: half size 3 x 2 x 1
  MString GetBody() const;
  //! Return true if the fixture with the keywords is rejected, or if a hit at its center can be noised
  bool IsRejectedOrNoised(const MString& Body, const MString& Keywords);

  //! Test the construction, setters, and getters
  bool TestConstruction();
  //! Test the keyword parsing and validation
  bool TestKeywords();
  //! Test the position noising
  bool TestPositionNoise();
  //! Test the boundary handling of the position noising
  bool TestBoundaries();
  //! Test the discretization
  bool TestDiscretize();
  //! Test the position resolution
  bool TestPositionResolution();
  //! Test copy, named detectors, and text output
  bool TestCopyNamedText();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDAngerCamera::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestKeywords() && Passed;
  Passed = TestCommonNoiseChain(GetBody() + "D.Positioning XYZ\nD.PositionResolution 100 0.01\nD.PositionResolution 500 0.01\n", MVector(0.25, 0.25, 0.5)) && Passed;
  Passed = TestPositionNoise() && Passed;
  Passed = TestBoundaries() && Passed;
  Passed = TestDiscretize() && Passed;
  Passed = TestPositionResolution() && Passed;
  Passed = TestCopyNamedText() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture without a position resolution: half size 3 x 2 x 1
MString UTDAngerCamera::GetBody() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "AngerCamera D\nD.SensitiveVolume Box\nD.DetectorVolume Box\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the fixture with the keywords is rejected, or if a hit at its center can be noised
bool UTDAngerCamera::IsRejectedOrNoised(const MString& Body, const MString& Keywords)
{
  MString Flags;
  MDGeometry Local;
  if (Scan(Local, Body, Keywords) == false) {
    return true;
  }
  MVector Position(0.0, 0.0, 0.0);
  double Energy = 100.0;
  double Time = 0.0;
  Local.GetDetector("D")->Noise(Position, Energy, Time, Flags, Local.GetVolume("Box"));
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction, setters, and getters
bool UTDAngerCamera::TestConstruction()
{
  bool Passed = true;

  MDAngerCamera Camera("Camera");
  Passed = EvaluateTrue("MDAngerCamera()", "name", "The name is stored", Camera.GetName() == "Camera") && Passed;
  Passed = Evaluate("MDAngerCamera()", "type", "The type is Anger camera", Camera.GetType(), MDDetector::c_AngerCamera) && Passed;
  Passed = EvaluateTrue("MDAngerCamera()", "type name", "The type name is Anger camera", Camera.GetTypeName() == MDDetector::c_AngerCameraName) && Passed;
  Passed = Evaluate("MDAngerCamera()", "positioning", "The positioning is unknown", Camera.GetPositioning(), MDAngerCamera::c_PositionResolutionUnknown) && Passed;

  Passed = Evaluate("c_PositionResolutionUnknown", "constant", "The value is the one of the grid point", MDAngerCamera::c_PositionResolutionUnknown, static_cast<int>(MDGridPoint::c_Unknown)) && Passed;
  Passed = Evaluate("c_PositionResolutionXY", "constant", "The value is the one of the grid point", MDAngerCamera::c_PositionResolutionXY, static_cast<int>(MDGridPoint::c_XYAnger)) && Passed;
  Passed = Evaluate("c_PositionResolutionXYZ", "constant", "The value is the one of the grid point", MDAngerCamera::c_PositionResolutionXYZ, static_cast<int>(MDGridPoint::c_XYZAnger)) && Passed;
  Passed = Evaluate("c_PositionResolutionXYZIndependent", "constant", "The value is the one of the grid point", MDAngerCamera::c_PositionResolutionXYZIndependent, static_cast<int>(MDGridPoint::c_XYZIndependentAnger)) && Passed;

  Camera.SetPositioning(MDAngerCamera::c_PositionResolutionXY);
  Passed = Evaluate("SetPositioning()", "XY", "The value is stored", Camera.GetPositioning(), MDAngerCamera::c_PositionResolutionXY) && Passed;

  // Invalid values are not stored: the only valid point determines the constant result
  MDAngerCamera Input("Input");
  Input.SetPositioning(MDAngerCamera::c_PositionResolutionXY);
  DisableDefaultStreams();
  Input.SetPositionResolution(-1.0, 7.0);
  Input.SetPositionResolution(1.0, 0.0);
  Input.SetPositionResolution(1.0, -1.0);
  EnableDefaultStreams();
  Input.SetPositionResolution(10.0, 2.0);
  Passed = EvaluateNear("SetPositionResolution()", "after invalid inputs", "Only the valid point is used", Input.GetPositionResolution(MVector(0.0, 0.0, 0.0), 100.0).X(), 2.0/sqrt(2.0), 1e-12) && Passed;

  MDAngerCamera InputXYZ("InputXYZ");
  InputXYZ.SetPositioning(MDAngerCamera::c_PositionResolutionXYZIndependent);
  DisableDefaultStreams();
  InputXYZ.SetPositionResolutionXYZ(-1.0, 7.0, 7.0, 7.0);
  InputXYZ.SetPositionResolutionXYZ(1.0, 0.0, 7.0, 7.0);
  InputXYZ.SetPositionResolutionXYZ(1.0, 7.0, 0.0, 7.0);
  InputXYZ.SetPositionResolutionXYZ(1.0, 7.0, 7.0, 0.0);
  EnableDefaultStreams();
  InputXYZ.SetPositionResolutionXYZ(10.0, 2.0, 3.0, 4.0);
  Passed = EvaluateVectorNear("SetPositionResolutionXYZ()", "after invalid inputs", "Only the valid point is used", InputXYZ.GetPositionResolution(MVector(0.0, 0.0, 0.0), 100.0), MVector(2.0, 3.0, 4.0), 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword parsing and validation
bool UTDAngerCamera::TestKeywords()
{
  bool Passed = true;

  const MString Body = GetBody();
  Passed = EvaluateTrue("ScanSetupFile()", "XY", "The positioning XY is accepted", IsScanOK(Body, "D.Positioning XY\nD.PositionResolution 100 1\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZ", "The positioning XYZ is accepted", IsScanOK(Body, "D.Positioning XYZ\nD.PositionResolution 100 1\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZIndependent", "The positioning XYZIndependent is accepted", IsScanOK(Body, "D.Positioning XYZIndependent\nD.PositionResolution 100 1 2 3\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "alias", "The class name is an alias of the type", IsScanOK(GetBoxVolume("Box", 3.0, 2.0, 1.0), "MDAngerCamera D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Positioning XY\nD.PositionResolution 100 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "no positioning", "A missing positioning is rejected", IsScanOK(Body)) && Passed;

  vector<MString> Wrong = {
    "D.Positioning\n",
    "D.Positioning XY Z\n",
    "D.Positioning Unknown\n",
    "D.PositionResolution\n",
    "D.PositionResolution 100\n",
    "D.PositionResolution 100 1 2\n",
    "D.PositionResolution 100 1 2 3 4\n",
    "D.Offset 0.1 0.1\n",
    "D.StripNumber 2 2\n",
    "D.Voxels 2 2 2\n",
    "D.DepthResolution 100 1\n"
  };
  for (const MString& Keyword: Wrong) {
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "The wrong keyword is rejected", IsScanOK(Body, MString("D.Positioning XY\n") + Keyword)) && Passed;
  }

  // Direct validation
  MDAngerCamera Direct("Direct");
  DisableDefaultStreams();
  bool Result = Direct.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no volumes", "A detector without volumes is invalid", Result) && Passed;

  // Missing tables
  vector<MString> Missing = {
    "D.Positioning XY\n",
    "D.Positioning XYZ\n",
    "D.Positioning XYZIndependent\n",
    "D.Positioning XYZIndependent\nD.PositionResolution 100 1\n",
    "D.Positioning XY\nD.PositionResolution 100 1 2 3\n"
  };
  for (const MString& Keyword: Missing) {
    // Defect GEO-D-065: The validation does not check if the table for the selected positioning exists, and the first hit stops the program with an exception
    Passed = ExpectedToCrash(EvaluateTrue("Noise()", Keyword, "A missing table of the position resolution is rejected or works", IsRejectedOrNoised(Body, Keyword))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position noising
bool UTDAngerCamera::TestPositionNoise()
{
  bool Passed = true;
  gRandom->SetSeed(9001);

  const MString Table = "D.PositionResolution 100 0.1\nD.PositionResolution 500 0.3\n";

  // XY: a Gaussian radius with the resolution at a uniform angle - by hand: sigma/sqrt(2) per axis, 0.2 at 300 keV
  MDGeometry XY;
  Passed = EvaluateTrue("ScanSetupFile()", "XY", "The fixture is accepted", Scan(XY, GetBody(), MString("D.Positioning XY\n") + Table)) && Passed;
  if (CheckDetector(Passed, "XY", XY, "D", MDDetector::c_AngerCamera)) {
    MDDetector* D = XY.GetDetector("D");
    MDVolume* V = XY.GetVolume("Box");
    MVector Start(0.25, -0.25, 0.5);
    double Sigma = 0.2/sqrt(2.0);
    Passed = EvaluateNear("Noise()", "XY, x mean", "The mean is the true position", MStatistics::GetMean(SamplePosition(D, V, Start, 300.0, 0)), 0.25, GetMeanTolerance(Sigma)) && Passed;
    // Tolerance: the kurtosis of the product of a Gaussian and a cosine is 4.5, thus the error is sqrt(3.5/2) larger than for a Gaussian
    Passed = EvaluateNear("Noise()", "XY, x width", "The width is the resolution/sqrt(2)", MStatistics::GetStandardDeviation(SamplePosition(D, V, Start, 300.0, 0)), Sigma, 1.4*GetRMSTolerance(Sigma)) && Passed;
    Passed = EvaluateNear("Noise()", "XY, y mean", "The mean is the true position", MStatistics::GetMean(SamplePosition(D, V, Start, 300.0, 1)), -0.25, GetMeanTolerance(Sigma)) && Passed;
    Passed = EvaluateNear("Noise()", "XY, y width", "The width is the resolution/sqrt(2)", MStatistics::GetStandardDeviation(SamplePosition(D, V, Start, 300.0, 1)), Sigma, 1.4*GetRMSTolerance(Sigma)) && Passed;
    vector<double> Z = SamplePosition(D, V, Start, 300.0, 2, 1000);
    Passed = EvaluateNear("Noise()", "XY, z", "The depth stays", MStatistics::GetMean(Z), 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "XY, z", "The depth stays", MStatistics::GetStandardDeviation(Z), 0.0, 1e-9) && Passed;
    // The energy dependence: 0.1 at 100 keV, 0.3 at 500 keV
    Passed = EvaluateNear("Noise()", "XY, x width at 100 keV", "The width is the resolution/sqrt(2)", MStatistics::GetStandardDeviation(SamplePosition(D, V, Start, 100.0, 0)), 0.1/sqrt(2.0), 1.4*GetRMSTolerance(0.1/sqrt(2.0))) && Passed;
    Passed = EvaluateNear("Noise()", "XY, x width at 500 keV", "The width is the resolution/sqrt(2)", MStatistics::GetStandardDeviation(SamplePosition(D, V, Start, 500.0, 0)), 0.3/sqrt(2.0), 1.4*GetRMSTolerance(0.3/sqrt(2.0))) && Passed;
  }

  // XYZ: the per axis width has to be the one given by GetPositionResolution: 0.2/sqrt(3)
  MDGeometry XYZ;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZ", "The fixture is accepted", Scan(XYZ, GetBody(), MString("D.Positioning XYZ\n") + Table)) && Passed;
  if (CheckDetector(Passed, "XYZ", XYZ, "D", MDDetector::c_AngerCamera)) {
    MDDetector* D = XYZ.GetDetector("D");
    MDVolume* V = XYZ.GetVolume("Box");
    MVector Start(0.0, 0.0, 0.0);
    double Sigma = 0.2/sqrt(3.0);
    Passed = EvaluateNear("Noise()", "XYZ, x mean", "The mean is the true position", MStatistics::GetMean(SamplePosition(D, V, Start, 300.0, 0)), 0.0, GetMeanTolerance(0.2)) && Passed;
    Passed = EvaluateNear("Noise()", "XYZ, y mean", "The mean is the true position", MStatistics::GetMean(SamplePosition(D, V, Start, 300.0, 1)), 0.0, GetMeanTolerance(0.2)) && Passed;
    Passed = EvaluateNear("Noise()", "XYZ, z mean", "The mean is the true position", MStatistics::GetMean(SamplePosition(D, V, Start, 300.0, 2)), 0.0, GetMeanTolerance(0.2)) && Passed;
    // Defect GEO-D-066: The XYZ positioning samples the polar angle uniformly and not the cosine of it, thus the noise is not isotropic: sigma/2 in x and y and sigma/sqrt(2) in z instead of sigma/sqrt(3)
    Passed = ExpectedToFail(EvaluateNear("Noise()", "XYZ, x width", "The noise is isotropic with the reported resolution", MStatistics::GetStandardDeviation(SamplePosition(D, V, Start, 300.0, 0)), Sigma, 1.4*GetRMSTolerance(Sigma))) && Passed;
    Passed = ExpectedToFail(EvaluateNear("Noise()", "XYZ, z width", "The noise is isotropic with the reported resolution", MStatistics::GetStandardDeviation(SamplePosition(D, V, Start, 300.0, 2)), Sigma, 1.4*GetRMSTolerance(Sigma))) && Passed;
  }

  // XYZ independent: Gaussian axes with interpolated widths
  MDGeometry Independent;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZIndependent", "The fixture is accepted", Scan(Independent, GetBody(), "D.Positioning XYZIndependent\nD.PositionResolution 100 0.1 0.05 0.02\nD.PositionResolution 500 0.3 0.15 0.06\n")) && Passed;
  if (CheckDetector(Passed, "Independent", Independent, "D", MDDetector::c_AngerCamera)) {
    MDDetector* D = Independent.GetDetector("D");
    MDVolume* V = Independent.GetVolume("Box");
    MVector Start(0.25, -0.25, 0.5);
    Passed = CheckGauss("Noise()", "XYZIndependent, x at 300 keV", SamplePosition(D, V, Start, 300.0, 0), 0.25, 0.2) && Passed;
    Passed = CheckGauss("Noise()", "XYZIndependent, y at 300 keV", SamplePosition(D, V, Start, 300.0, 1), -0.25, 0.1) && Passed;
    Passed = CheckGauss("Noise()", "XYZIndependent, z at 300 keV", SamplePosition(D, V, Start, 300.0, 2), 0.5, 0.04) && Passed;
  }

  // Position noise with the resolution at the noised energy: peak 300 keV at 100 keV input
  MDGeometry NoisedEnergy;
  Passed = EvaluateTrue("ScanSetupFile()", "noised energy", "The fixture is accepted", Scan(NoisedEnergy, GetBody(), "D.Positioning XYZIndependent\nD.PositionResolution 100 0.1 0.1 0.1\nD.PositionResolution 500 0.3 0.3 0.3\nD.EnergyResolution Gauss 100 300 0\nD.EnergyResolution Gauss 500 700 0\n")) && Passed;
  if (CheckDetector(Passed, "NoisedEnergy", NoisedEnergy, "D", MDDetector::c_AngerCamera)) {
    Passed = CheckGauss("Noise()", "resolution at the noised energy", SamplePosition(NoisedEnergy.GetDetector("D"), NoisedEnergy.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 100.0, 0), 0.0, 0.2) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the boundary handling of the position noising
bool UTDAngerCamera::TestBoundaries()
{
  bool Passed = true;
  MString Flags;
  gRandom->SetSeed(9002);

  // Close to the boundary in z: rejection of the complete position, so a truncated Gaussian as the other axes are independent and wide
  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "boundary", "The fixture is accepted", Scan(Geometry, GetBody(), "D.Positioning XYZIndependent\nD.PositionResolution 100 0.1 0.1 0.3\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");
  double Mean = 0.0;
  double RMS = 0.0;
  GetTruncatedGaussian(0.95, 0.3, -1.0, 1.0, Mean, RMS);
  vector<double> Values = SamplePosition(D, V, MVector(0.0, 0.0, 0.95), 100.0, 2);
  Passed = EvaluateNear("Noise()", "close to the upper boundary", "The mean is the one of the truncated Gaussian", MStatistics::GetMean(Values), Mean, GetMeanTolerance(RMS)) && Passed;
  Passed = EvaluateNear("Noise()", "close to the upper boundary", "The width is the one of the truncated Gaussian", MStatistics::GetStandardDeviation(Values), RMS, GetRMSTolerance(RMS)) && Passed;
  Passed = EvaluateTrue("Noise()", "close to the upper boundary", "All positions are in the volume", *max_element(Values.begin(), Values.end()) <= 1.0 + 1E-5) && Passed;

  // Positions outside of the volume: tolerance 1e-5
  Passed = EvaluateNear("Noise()", "position outside of the volume", "The energy is zero", NoiseOnce(D, V, MVector(0.0, 0.0, 1.1), 100.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Noise()", "just outside the tolerance", "The energy is zero", NoiseOnce(D, V, MVector(3.00002, 0.0, 0.0), 100.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Noise()", "just inside the tolerance", "The energy stays", NoiseOnce(D, V, MVector(2.99999, 0.0, 0.0), 100.0), 100.0, 1e-12) && Passed;

  // A resolution much larger than the volume
  MDGeometry Wide;
  Passed = EvaluateTrue("ScanSetupFile()", "very wide resolution", "The fixture is accepted", Scan(Wide, GetBody(), "D.Positioning XYZIndependent\nD.PositionResolution 100 1000 1000 1000\n")) && Passed;
  if (CheckDetector(Passed, "Wide", Wide, "D", MDDetector::c_AngerCamera)) {
    MVector Position(0.25, 0.25, 0.25);
    double Energy = 100.0;
    double Time = 0.0;
    DisableDefaultStreams();
    Wide.GetDetector("D")->Noise(Position, Energy, Time, Flags, Wide.GetVolume("Box"));
    EnableDefaultStreams();
    // Defect GEO-D-067: After all trials failed the message says that the position is not noised, but the last position outside of the volume is used
    Passed = ExpectedToFail(EvaluateVectorNear("Noise()", "all trials failed", "The position is not noised", Position, MVector(0.25, 0.25, 0.25), 1e-12)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the discretization
bool UTDAngerCamera::TestDiscretize()
{
  bool Passed = true;

  MVector Position(0.25, -0.25, 0.5);

  MDGeometry XY;
  Passed = EvaluateTrue("ScanSetupFile()", "XY", "The fixture is accepted", Scan(XY, GetBody(), "D.Positioning XY\nD.PositionResolution 100 0.1\n")) && Passed;
  if (CheckDetector(Passed, "XY", XY, "D", MDDetector::c_AngerCamera)) {
    MDDetector* D = XY.GetDetector("D");
    vector<MDGridPoint> Points = D->Grid(Position, 123.0, 45.0, XY.GetVolume("Box"));
    Passed = EvaluateSize("Grid()", "XY", "There is one grid point", Points.size(), 1) && Passed;
    Passed = Evaluate("Grid()", "XY", "The type is XY", Points[0].GetType(), MDGridPoint::c_XYAnger) && Passed;
    Passed = EvaluateVectorNear("Grid()", "XY", "The depth is removed", Points[0].GetPosition(), MVector(0.25, -0.25, 0.0), 1e-12) && Passed;
    Passed = EvaluateNear("Grid()", "XY", "The energy is stored", Points[0].GetEnergy(), 123.0, 1e-12) && Passed;
    Passed = EvaluateNear("Grid()", "XY", "The time is stored", Points[0].GetTime(), 45.0, 1e-12) && Passed;
    Passed = Evaluate("GetGridPoint()", "XY", "The type is XY", D->GetGridPoint(Position).GetType(), MDGridPoint::c_XYAnger) && Passed;
  }

  MDGeometry XYZ;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZ", "The fixture is accepted", Scan(XYZ, GetBody(), "D.Positioning XYZ\nD.PositionResolution 100 0.1\n")) && Passed;
  if (CheckDetector(Passed, "XYZ", XYZ, "D", MDDetector::c_AngerCamera)) {
    MDDetector* D = XYZ.GetDetector("D");
    vector<MDGridPoint> Points = D->Grid(Position, 123.0, 45.0, XYZ.GetVolume("Box"));
    Passed = Evaluate("Grid()", "XYZ", "The type is XYZ", Points[0].GetType(), MDGridPoint::c_XYZAnger) && Passed;
    Passed = EvaluateVectorNear("Grid()", "XYZ", "The position is stored", Points[0].GetPosition(), Position, 1e-12) && Passed;
    Passed = Evaluate("GetGridPoint()", "XYZ", "The type is XYZ", D->GetGridPoint(Position).GetType(), MDGridPoint::c_XYZAnger) && Passed;
  }

  MDGeometry Independent;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZIndependent", "The fixture is accepted", Scan(Independent, GetBody(), "D.Positioning XYZIndependent\nD.PositionResolution 100 0.1 0.1 0.1\n")) && Passed;
  if (CheckDetector(Passed, "Independent", Independent, "D", MDDetector::c_AngerCamera)) {
    MDDetector* D = Independent.GetDetector("D");
    vector<MDGridPoint> Points = D->Grid(Position, 123.0, 45.0, Independent.GetVolume("Box"));
    Passed = Evaluate("Grid()", "XYZIndependent", "The type is XYZIndependent", Points[0].GetType(), MDGridPoint::c_XYZIndependentAnger) && Passed;
    Passed = EvaluateVectorNear("Grid()", "XYZIndependent", "The position is stored", Points[0].GetPosition(), Position, 1e-12) && Passed;
    Passed = Evaluate("GetGridPoint()", "XYZIndependent", "The type is XYZIndependent", D->GetGridPoint(Position).GetType(), MDGridPoint::c_XYZIndependentAnger) && Passed;
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "XYZIndependent", "The position in the grid is the position in the detector", D->GetPositionInDetectorVolume(0, 0, 0, Position, MDGridPoint::c_XYZIndependentAnger, Independent.GetVolume("Box")), Position, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position resolution
bool UTDAngerCamera::TestPositionResolution()
{
  bool Passed = true;

  MVector Null(0.0, 0.0, 0.0);
  const MString Table = "D.PositionResolution 100 0.1\nD.PositionResolution 500 0.3\n";

  MDGeometry XY;
  Passed = EvaluateTrue("ScanSetupFile()", "XY", "The fixture is accepted", Scan(XY, GetBody(), MString("D.Positioning XY\n") + Table)) && Passed;
  if (CheckDetector(Passed, "XY", XY, "D", MDDetector::c_AngerCamera)) {
    MVector Resolution = XY.GetDetector("D")->GetPositionResolution(Null, 300.0);
    Passed = EvaluateNear("GetPositionResolution()", "XY, x", "The resolution is the interpolated value/sqrt(2)", Resolution.X(), 0.2/sqrt(2.0), 1e-12) && Passed;
    Passed = EvaluateNear("GetPositionResolution()", "XY, y", "The resolution is the interpolated value/sqrt(2)", Resolution.Y(), 0.2/sqrt(2.0), 1e-12) && Passed;
    // Defect GEO-D-068: The structural size is never set for an Anger camera, thus the depth resolution of the type XY is the value for not defined/sqrt(12)
    Passed = ExpectedToFail(EvaluateNear("GetPositionResolution()", "XY, z", "The resolution is the thickness/sqrt(12)", Resolution.Z(), 2.0/sqrt(12.0), 1e-9)) && Passed;
  }

  MDGeometry XYZ;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZ", "The fixture is accepted", Scan(XYZ, GetBody(), MString("D.Positioning XYZ\n") + Table)) && Passed;
  if (CheckDetector(Passed, "XYZ", XYZ, "D", MDDetector::c_AngerCamera)) {
    double Expected = 0.2/sqrt(3.0);
    Passed = EvaluateVectorNear("GetPositionResolution()", "XYZ", "The resolution is the interpolated value/sqrt(3)", XYZ.GetDetector("D")->GetPositionResolution(Null, 300.0), MVector(Expected, Expected, Expected), 1e-12) && Passed;
  }

  MDGeometry Independent;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZIndependent", "The fixture is accepted", Scan(Independent, GetBody(), "D.Positioning XYZIndependent\nD.PositionResolution 100 0.1 0.05 0.02\nD.PositionResolution 500 0.3 0.15 0.06\n")) && Passed;
  if (CheckDetector(Passed, "Independent", Independent, "D", MDDetector::c_AngerCamera)) {
    Passed = EvaluateVectorNear("GetPositionResolution()", "XYZIndependent", "The resolution is the interpolated value per axis", Independent.GetDetector("D")->GetPositionResolution(Null, 300.0), MVector(0.2, 0.1, 0.04), 1e-12) && Passed;
    Passed = EvaluateVectorNear("GetPositionResolution()", "XYZIndependent above the table", "The resolution is the extrapolated value per axis", Independent.GetDetector("D")->GetPositionResolution(Null, 900.0), MVector(0.5, 0.25, 0.1), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy, named detectors, and text output
bool UTDAngerCamera::TestCopyNamedText()
{
  bool Passed = true;

  Passed = CheckCopyDataToNamedDetectors<MDAngerCamera>("AngerCamera") && Passed;

  MVector Null(0.0, 0.0, 0.0);
  const MString Keywords = "D.Positioning XYZ\nD.PositionResolution 100 0.1\nD.PositionResolution 500 0.3\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZ", "The fixture is accepted", Scan(Geometry, GetBody(), Keywords)) && Passed;
  if (Passed == false) {
    return false;
  }
  MDAngerCamera* D = dynamic_cast<MDAngerCamera*>(Geometry.GetDetector("D"));
  double Expected = 0.2/sqrt(3.0);

  // Clone
  Passed = CheckCloneIndependence("AngerCamera", D) && Passed;
  unique_ptr<MDDetector> Cloned(D->Clone());
  MDAngerCamera* Copy = dynamic_cast<MDAngerCamera*>(Cloned.get());
  if (EvaluateTrue("Clone()", "Anger camera", "The clone is an Anger camera", Copy != nullptr) == false) {
    return false;
  }
  Passed = Evaluate("MDAngerCamera(const MDAngerCamera&)", "positioning", "The positioning is copied", Copy->GetPositioning(), MDAngerCamera::c_PositionResolutionXYZ) && Passed;
  Passed = EvaluateNear("MDAngerCamera(const MDAngerCamera&)", "table", "The table is copied", Copy->GetPositionResolution(Null, 300.0).X(), Expected, 1e-12) && Passed;

  // Text
  MString Text = D->GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "type", "The type is written", Text.Contains("AngerCamera D")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "positioning", "The positioning is written", Text.Contains("D.Positioning XYZ")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "table", "The table is written", Text.Contains("D.PositionResolution 500 0.3")) && Passed;
  MDGeometry Second;
  Passed = EvaluateTrue("ScanSetupFile()", "round trip", "The text can be scanned again", Scan(Second, GetBoxVolume("Box", 3.0, 2.0, 1.0), Text)) && Passed;
  if (CheckDetector(Passed, "Second", Second, "D", MDDetector::c_AngerCamera)) {
    Passed = Evaluate("GetGeomega()", "round trip, all fields", "The text written after the round trip is identical, so that no written field is lost", dynamic_cast<MDAngerCamera*>(Second.GetDetector("D"))->GetGeomega(), Text) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The resolution is identical", Second.GetDetector("D")->GetPositionResolution(Null, 300.0).X(), Expected, 1e-12) && Passed;
  }
  MDGeometry XY;
  Passed = EvaluateTrue("ScanSetupFile()", "XY", "The fixture is accepted", Scan(XY, GetBody(), "D.Positioning XY\nD.PositionResolution 100 0.1\n")) && Passed;
  if (CheckDetector(Passed, "XY", XY, "D", MDDetector::c_AngerCamera)) {
    Passed = EvaluateTrue("GetGeomega()", "XY", "The positioning is written", dynamic_cast<MDAngerCamera*>(XY.GetDetector("D"))->GetGeomega().Contains("D.Positioning XY\n")) && Passed;
  }

  MDGeometry Independent;
  Passed = EvaluateTrue("ScanSetupFile()", "XYZIndependent", "The fixture is accepted", Scan(Independent, GetBody(), "D.Positioning XYZIndependent\nD.PositionResolution 100 0.1 0.05 0.02\n")) && Passed;
  if (CheckDetector(Passed, "Independent", Independent, "D", MDDetector::c_AngerCamera)) {
    MString IndependentText = dynamic_cast<MDAngerCamera*>(Independent.GetDetector("D"))->GetGeomega();
    // Defect GEO-D-069: GetGeomega writes the positioning XYZ and no table for the positioning XYZIndependent
    Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "XYZIndependent", "The positioning XYZIndependent is written", IndependentText.Contains("D.Positioning XYZIndependent"))) && Passed;
    Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "XYZIndependent", "The table of the three axes is written", IndependentText.Contains("D.PositionResolution 100 0.1 0.05 0.02"))) && Passed;
  }

  Passed = EvaluateTrue("ToString()", "standard", "The type is written", D->ToString().Contains("AngerCamera")) && Passed;

  // Named detectors
  MDGeometry Named;
  Passed = EvaluateTrue("ScanSetupFile()", "named detector inherits", "The fixture is accepted", Scan(Named, GetBody(), Keywords + "D.Named N\nN.Assign World.Box\n")) && Passed;
  if (CheckDetector(Passed, "Named", Named, "N", MDDetector::c_AngerCamera)) {
    MDAngerCamera* N = dynamic_cast<MDAngerCamera*>(Named.GetDetector("N"));
    Passed = Evaluate("CopyDataToNamedDetectors()", "positioning", "The positioning is inherited", N->GetPositioning(), MDAngerCamera::c_PositionResolutionXYZ) && Passed;
    Passed = EvaluateNear("CopyDataToNamedDetectors()", "table", "The table is inherited", N->GetPositionResolution(Null, 300.0).X(), Expected, 1e-12) && Passed;
  }
  MDGeometry Own;
  Passed = EvaluateTrue("ScanSetupFile()", "named detector with its own positioning", "The fixture is accepted", Scan(Own, GetBody(), Keywords + "D.Named N\nN.Assign World.Box\nN.Positioning XY\nN.PositionResolution 100 0.4\n")) && Passed;
  if (CheckDetector(Passed, "Own", Own, "N", MDDetector::c_AngerCamera)) {
    MDAngerCamera* N = dynamic_cast<MDAngerCamera*>(Own.GetDetector("N"));
    Passed = Evaluate("CopyDataToNamedDetectors()", "own positioning", "The own positioning stays", N->GetPositioning(), MDAngerCamera::c_PositionResolutionXY) && Passed;
    Passed = EvaluateNear("CopyDataToNamedDetectors()", "own table", "The own table stays", N->GetPositionResolution(Null, 300.0).X(), 0.4/sqrt(2.0), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDAngerCamera Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDAngerCamera.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
