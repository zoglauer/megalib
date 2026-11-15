/*
 * UTDCalorimeter.cxx
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


//! Unit test of the calorimeter MDCalorimeter
class UTDCalorimeter : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDCalorimeter() : UTDDetectorBase("UTDCalorimeter") {}
  //! Default destructor
  virtual ~UTDCalorimeter() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the fixture with one crystal of half size 3 x 2 x 1
  MString GetBody() const;
  //! Return the fixture with 3 x 2 bars of half size 0.5 x 0.5 x 2, pitch 0.1, offset 0.05
  MString GetBars(double ContainerX = 1.65) const;

  //! Test the construction and depth resolution input
  bool TestConstruction();
  //! Test the keyword parsing and validation
  bool TestKeywords();
  //! Test the depth noising
  bool TestDepthNoise();
  //! Test the grid
  bool TestGrid();
  //! Test the blocked channels
  bool TestBlockedChannels();
  //! Test the neighbor search and the position resolution
  bool TestNearAndResolution();
  //! Test copy, named detectors, and text output
  bool TestCopyNamedText();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDCalorimeter::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestKeywords() && Passed;
  // GEO-D-075: The detector does not flag failures and overflows
  Passed = TestCommonNoiseChain(GetBody(), MVector(0.25, 0.25, 0.5), false) && Passed;
  Passed = TestDepthNoise() && Passed;
  Passed = TestGrid() && Passed;
  Passed = TestBlockedChannels() && Passed;
  Passed = TestNearAndResolution() && Passed;
  Passed = TestCopyNamedText() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture with one crystal of half size 3 x 2 x 1
MString UTDCalorimeter::GetBody() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "Calorimeter D\nD.SensitiveVolume Box\nD.DetectorVolume Box\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture with 3 x 2 bars of half size 0.5 x 0.5 x 2, pitch 0.1, offset 0.05
MString UTDCalorimeter::GetBars(double ContainerX) const
{
  MString Body = GetBoxVolume("Container", ContainerX, 1.1, 2.0);
  Body += "Volume Bar\nBar.Material CsI\nBar.Shape BRIK 0.5 0.5 2.0\nBar.Position -1.1 -0.55 0.0\nBar.Mother Container\n";
  Body += "Calorimeter D\nD.SensitiveVolume Bar\nD.DetectorVolume Container\nD.StructuralPitch 0.1 0.1 0.0\nD.StructuralOffset 0.05 0.05 0.0\n";
  return Body;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction and depth resolution input
bool UTDCalorimeter::TestConstruction()
{
  bool Passed = true;

  MDCalorimeter Calorimeter("Calorimeter");
  Passed = EvaluateTrue("MDCalorimeter()", "name", "The name is stored", Calorimeter.GetName() == "Calorimeter") && Passed;
  Passed = Evaluate("MDCalorimeter()", "type", "The type is calorimeter", Calorimeter.GetType(), MDDetector::c_Calorimeter) && Passed;
  Passed = EvaluateTrue("MDCalorimeter()", "type name", "The type name is calorimeter", Calorimeter.GetTypeName() == MDDetector::c_CalorimeterName) && Passed;
  Passed = EvaluateFalse("HasDepthResolution()", "new detector", "There is no depth resolution", Calorimeter.HasDepthResolution()) && Passed;

  DisableDefaultStreams();
  bool NegativeEnergy = Calorimeter.SetDepthResolutionAt(-1.0, 1.0, 0.1);
  bool ZeroResolution = Calorimeter.SetDepthResolutionAt(1.0, 0.0, 0.1);
  bool NegativeResolution = Calorimeter.SetDepthResolutionAt(1.0, -1.0, 0.1);
  bool NegativeSigma = Calorimeter.SetDepthResolutionAt(1.0, 1.0, -0.1);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetDepthResolutionAt()", "negative energy", "The input is rejected", NegativeEnergy) && Passed;
  Passed = EvaluateFalse("SetDepthResolutionAt()", "zero resolution", "The input is rejected", ZeroResolution) && Passed;
  Passed = EvaluateFalse("SetDepthResolutionAt()", "negative resolution", "The input is rejected", NegativeResolution) && Passed;
  Passed = EvaluateFalse("SetDepthResolutionAt()", "negative sigma", "The input is rejected", NegativeSigma) && Passed;
  Passed = EvaluateFalse("HasDepthResolution()", "after rejections", "There is still no depth resolution", Calorimeter.HasDepthResolution()) && Passed;
  Passed = EvaluateTrue("SetDepthResolutionAt()", "energy zero", "The energy zero is accepted", Calorimeter.SetDepthResolutionAt(0.0, 1.0, 0.0)) && Passed;
  Passed = EvaluateTrue("HasDepthResolution()", "one point", "There is a depth resolution", Calorimeter.HasDepthResolution()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword parsing and validation
bool UTDCalorimeter::TestKeywords()
{
  bool Passed = true;

  const MString Body = GetBody();
  Passed = EvaluateTrue("ScanSetupFile()", "minimal", "A minimal detector is accepted", IsScanOK(Body)) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "alias", "The class name is an alias of the type", IsScanOK(GetBoxVolume("Box", 3.0, 2.0, 1.0), "MDCalorimeter D\nD.SensitiveVolume Box\nD.DetectorVolume Box\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "bars", "The bars are accepted", IsScanOK(GetBars())) && Passed;

  vector<MString> Wrong = {
    "D.DepthResolution -100 0.1\n",
    "D.DepthResolution 100 -0.1\n",
    "D.DepthResolution 100 0.0\n",
    "D.DepthResolution 100 0.1 -0.1\n",
    "D.DepthResolutionThreshold 100\n",
    "D.Offset 0.1 0.1\n",
    "D.StripNumber 2 2\n",
    "D.Voxels 2 2 2\n",
    "D.GuardRingTriggerThreshold 10\n",
    "D.PositionResolution 100 1\n",
    "D.TriggerThresholdDepthCorrection 0 1\n",
    "D.NoiseThresholdDepthCorrection 0 1\n",
    "D.EnergyResolutionDepthCorrection 0 1\n"
  };
  for (const MString& Keyword: Wrong) {
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "The wrong keyword is rejected", IsScanOK(Body, Keyword)) && Passed;
  }

  // Shapes and volumes
  const MString Ball = "Volume Ball\nBall.Material CsI\nBall.Shape SPHE 0 2 0 180 0 360\nBall.Position 0 0 0\nBall.Mother World\n";
  Passed = EvaluateFalse("ScanSetupFile()", "sphere", "A sphere as detector volume is rejected", IsScanOK(Ball, "Calorimeter D\nD.SensitiveVolume Ball\nD.DetectorVolume Ball\n")) && Passed;
  MString SphereInBox = GetBoxVolume("Container", 3.0, 3.0, 3.0) + "Volume Ball\nBall.Material CsI\nBall.Shape SPHE 0 2 0 180 0 360\nBall.Position 0 0 0\nBall.Mother Container\n";
  Passed = EvaluateFalse("ScanSetupFile()", "sphere in a box", "A sphere as sensitive volume is rejected", IsScanOK(SphereInBox, "Calorimeter D\nD.SensitiveVolume Ball\nD.DetectorVolume Container\nD.StructuralPitch 0 0 0\nD.StructuralOffset 0 0 0\n")) && Passed;
  MString Two = GetBoxVolume("Container", 5.0, 5.0, 5.0) + GetBoxVolume("Part1", 1.0, 1.0, 1.0, 2.0, "Container") + GetBoxVolume("Part2", 1.0, 1.0, 1.0, -2.0, "Container");
  Passed = EvaluateFalse("ScanSetupFile()", "two sensitive volumes", "Two sensitive volumes are rejected", IsScanOK(Two, "Calorimeter D\nD.SensitiveVolume Part1\nD.SensitiveVolume Part2\nD.DetectorVolume Container\nD.StructuralPitch 0 0 0\nD.StructuralOffset 0 0 0\n")) && Passed;

  MDCalorimeter Direct("Direct");
  DisableDefaultStreams();
  bool Result = Direct.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no volumes", "A detector without volumes is invalid", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the depth noising
bool UTDCalorimeter::TestDepthNoise()
{
  bool Passed = true;
  MString Flags;
  gRandom->SetSeed(8001);

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution", "The fixture is accepted", Scan(Geometry, GetBody(), "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");

  // Width by interpolation: 0.1 + (E - 100)*0.0005 - the clipping is more than 5 sigma away for these energies
  Passed = CheckGauss("Noise()", "depth at the first table point", SamplePosition(D, V, MVector(0.0, 0.0, 0.0), 100.0, 2), 0.0, 0.1) && Passed;
  Passed = CheckGauss("Noise()", "depth at the table center", SamplePosition(D, V, MVector(0.0, 0.0, 0.0), 300.0, 2), 0.0, 0.2) && Passed;
  Passed = CheckGauss("Noise()", "depth below the table", SamplePosition(D, V, MVector(0.0, 0.0, 0.1), 50.0, 2), 0.1, 0.075) && Passed;
  Passed = EvaluateNear("Noise()", "x", "The x position stays", MStatistics::GetStandardDeviation(SamplePosition(D, V, MVector(0.25, 0.25, 0.0), 100.0, 0, 1000)), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Noise()", "y", "The y position stays", MStatistics::GetStandardDeviation(SamplePosition(D, V, MVector(0.25, 0.25, 0.0), 100.0, 1, 1000)), 0.0, 1e-12) && Passed;

  // Close to the boundary: Everything outside is set to 0.9999 of the half size, i.e. Phi(-0.05/0.3) of the hits at z = 0.95
  vector<double> Values = SamplePosition(D, V, MVector(0.0, 0.0, 0.95), 500.0, 2);
  double Clipped = 0.0;
  for (double Z: Values) {
    if (fabs(Z - 0.9999) < 1e-12) {
      Clipped += 1.0;
    }
  }
  double Expected = GetPhi(-0.05/0.3);
  Passed = EvaluateNear("Noise()", "depth close to the upper boundary", "The hits outside of the volume are clipped", Clipped/Values.size(), Expected, GetFractionTolerance(Expected)) && Passed;
  Passed = EvaluateTrue("Noise()", "depth close to the upper boundary", "All values are in the volume", *max_element(Values.begin(), Values.end()) <= 1.0 && *min_element(Values.begin(), Values.end()) >= -1.0) && Passed;
  Values = SamplePosition(D, V, MVector(0.0, 0.0, -0.95), 500.0, 2);
  Clipped = 0.0;
  for (double Z: Values) {
    if (fabs(Z + 0.9999) < 1e-12) {
      Clipped += 1.0;
    }
  }
  Passed = EvaluateNear("Noise()", "depth close to the lower boundary", "The hits outside of the volume are clipped", Clipped/Values.size(), Expected, GetFractionTolerance(Expected)) && Passed;

  // Sigma of the resolution: sqrt(0.2^2 + 0.05^2)
  MDGeometry Varying;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution with sigma", "The fixture is accepted", Scan(Varying, GetBody(), "D.DepthResolution 100 0.2 0.05\nD.DepthResolution 500 0.2 0.05\n")) && Passed;
  if (CheckDetector(Passed, "Varying", Varying, "D", MDDetector::c_Calorimeter)) {
    Passed = CheckGauss("Noise()", "depth resolution with sigma", SamplePosition(Varying.GetDetector("D"), Varying.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 300.0, 2), 0.0, sqrt(0.2*0.2 + 0.05*0.05)) && Passed;
  }

  // No depth resolution, overflow, removed hits: the center
  MDGeometry NoDepth;
  Passed = EvaluateTrue("ScanSetupFile()", "no depth resolution", "The fixture is accepted", Scan(NoDepth, GetBody())) && Passed;
  if (CheckDetector(Passed, "NoDepth", NoDepth, "D", MDDetector::c_Calorimeter)) {
    Passed = EvaluateNear("Noise()", "no depth resolution", "The depth is the center", MStatistics::GetMean(SamplePosition(NoDepth.GetDetector("D"), NoDepth.GetVolume("Box"), MVector(0.0, 0.0, 0.7), 100.0, 2, 10)), 0.0, 1e-12) && Passed;
  }
  MDGeometry Limits;
  Passed = EvaluateTrue("ScanSetupFile()", "limits", "The fixture is accepted", Scan(Limits, GetBody(), "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\nD.Overflow 400\nD.NoiseThreshold 10\n")) && Passed;
  if (CheckDetector(Passed, "Limits", Limits, "D", MDDetector::c_Calorimeter)) {
    D = Limits.GetDetector("D");
    V = Limits.GetVolume("Box");
    Passed = EvaluateNear("Noise()", "overflow", "The depth of an overflow is the center", MStatistics::GetMean(SamplePosition(D, V, MVector(0.0, 0.0, 0.7), 500.0, 2, 10)), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "overflow", "The energy is the overflow", NoiseOnce(D, V, MVector(0.0, 0.0, 0.7), 500.0), 400.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "below the noise threshold", "The depth of a removed hit is the center", MStatistics::GetMean(SamplePosition(D, V, MVector(0.0, 0.0, 0.7), 5.0, 2, 10)), 0.0, 1e-12) && Passed;
  }

  // In a volume tree: bar center at z = 10, x = -1.1, y = -0.55
  MString Tree = GetBars();
  Tree.ReplaceAll("Container.Position 0.0 0.0 0", "Container.Position 0.0 0.0 10");
  MDGeometryQuest Quest;
  Passed = EvaluateTrue("ScanSetupFile()", "volume tree", "The fixture is accepted", Scan(Quest, Tree, "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\n")) && Passed;
  if (CheckDetector(Passed, "Quest", Quest, "D", MDDetector::c_Calorimeter)) {
    MDVolumeSequence Sequence = Quest.GetVolumeSequence(MVector(-1.1, -0.55, 11.0), true, true);
    vector<double> Depths;
    for (unsigned int i = 0; i < c_Samples; ++i) {
      MVector Position(-1.1, -0.55, 11.0);
      double Energy = 100.0;
      double Time = 0.0;
      Quest.Noise(Position, Energy, Time, Flags, Sequence);
      Depths.push_back(Position.Z());
    }
    Passed = CheckGauss("MDGeometryQuest::Noise()", "bar in a translated volume", Depths, 11.0, 0.1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the grid
bool UTDCalorimeter::TestGrid()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "bars", "The fixture is accepted", Scan(Geometry, GetBars(), "D.DepthResolution 100 0.1\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Container");

  // Bar centers by hand: x = -1.65 + 0.05 + 0.5 + i*1.1, y = -1.1 + 0.05 + 0.5 + j*1.1
  for (unsigned int i = 0; i < 3; ++i) {
    for (unsigned int j = 0; j < 2; ++j) {
      MVector Center(-1.1 + i*1.1, -0.55 + j*1.1, 0.3);
      Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "bar center", "The center of the bar is returned", D->GetPositionInDetectorVolume(i, j, 0, MVector(0.0, 0.0, 0.3), MDGridPoint::c_Voxel, V), Center, 1e-9) && Passed;
      MDGridPoint Point = D->GetGridPoint(Center);
      Passed = Evaluate("GetGridPoint()", "bar center", "The bar in x is found", Point.GetXGrid(), i) && Passed;
      Passed = Evaluate("GetGridPoint()", "bar center", "The bar in y is found", Point.GetYGrid(), j) && Passed;
      Passed = Evaluate("GetGridPoint()", "bar center", "There is no z-grid", Point.GetZGrid(), 0u) && Passed;
      Passed = Evaluate("GetGridPoint()", "bar center", "The type is voxel", Point.GetType(), MDGridPoint::c_Voxel) && Passed;
      vector<MDGridPoint> Points = D->Grid(Center, 123.0, 45.0, V);
      Passed = EvaluateSize("Grid()", "bar center", "There is one grid point", Points.size(), 1) && Passed;
      Passed = EvaluateNear("Grid()", "bar center", "The depth relative to the center of the bar is stored", Points[0].GetPosition().Z(), 0.3, 1e-9) && Passed;
      Passed = EvaluateNear("Grid()", "bar center", "The energy is stored", Points[0].GetEnergy(), 123.0, 1e-12) && Passed;
      Passed = EvaluateNear("Grid()", "bar center", "The time is stored", Points[0].GetTime(), 45.0, 1e-12) && Passed;
    }
  }

  // Outside of all bars
  DisableDefaultStreams();
  unsigned int TypeBelow = D->GetGridPoint(MVector(-5.0, 0.0, 0.0)).GetType();
  unsigned int TypeAbove = D->GetGridPoint(MVector(5.0, 0.0, 0.0)).GetType();
  EnableDefaultStreams();
  // Defect GEO-D-057: GetGridPoint of the calorimeter has no range check and returns valid looking voxels for positions outside of all bars, a negative index wraps to a huge one
  Passed = ExpectedToFail(Evaluate("GetGridPoint()", "below all bars", "A position outside is unknown like in all other detectors", TypeBelow, MDGridPoint::c_Unknown)) && Passed;
  Passed = ExpectedToFail(Evaluate("GetGridPoint()", "above all bars", "A position outside is unknown like in all other detectors", TypeAbove, MDGridPoint::c_Unknown)) && Passed;

  // Without a depth resolution the grid position is the center of the bar
  MDGeometry NoDepth;
  Passed = EvaluateTrue("ScanSetupFile()", "no depth resolution", "The fixture is accepted", Scan(NoDepth, GetBars())) && Passed;
  if (CheckDetector(Passed, "NoDepth", NoDepth, "D", MDDetector::c_Calorimeter)) {
    vector<MDGridPoint> Points = NoDepth.GetDetector("D")->Grid(MVector(-1.1, -0.55, 1.5), 100.0, 0.0, NoDepth.GetVolume("Container"));
    Passed = EvaluateVectorNear("Grid()", "no depth resolution", "The position is at the center of the bar", Points[0].GetPosition(), MVector(0.0, 0.0, 0.0), 1e-12) && Passed;
  }

  // Bar not in the center of the container in z: half size 2.5 and z-offset 0.25, the center of the bar is at -0.25
  MString OffsetZ = GetBoxVolume("Container", 1.65, 1.1, 2.5);
  OffsetZ += "Volume Bar\nBar.Material CsI\nBar.Shape BRIK 0.5 0.5 2.0\nBar.Position -1.1 -0.55 -0.25\nBar.Mother Container\n";
  OffsetZ += "Calorimeter D\nD.SensitiveVolume Bar\nD.DetectorVolume Container\nD.StructuralPitch 0.1 0.1 0.0\nD.StructuralOffset 0.05 0.05 0.25\nD.DepthResolution 100 0.1\n";
  MDGeometry GeometryZ;
  Passed = EvaluateTrue("ScanSetupFile()", "offset in z", "The fixture is accepted", Scan(GeometryZ, OffsetZ)) && Passed;
  if (CheckDetector(Passed, "GeometryZ", GeometryZ, "D", MDDetector::c_Calorimeter)) {
    vector<MDGridPoint> Points = GeometryZ.GetDetector("D")->Grid(MVector(-1.1, -0.55, 0.05), 100.0, 0.0, GeometryZ.GetVolume("Container"));
    Passed = EvaluateNear("Grid()", "offset in z", "The depth is relative to the center of the bar", Points[0].GetPosition().Z(), 0.3, 1e-9) && Passed;
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "offset in z", "The round trip restores the position", GeometryZ.GetDetector("D")->GetPositionInDetectorVolume(0, 0, 0, Points[0].GetPosition(), MDGridPoint::c_Voxel, GeometryZ.GetVolume("Container")), MVector(-1.1, -0.55, 0.05), 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the blocked channels
bool UTDCalorimeter::TestBlockedChannels()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "BlockTrigger", "The fixture is accepted", Scan(Geometry, GetBars(), "D.BlockTrigger 1 1\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "blocked bar", "The blocked bar does not trigger", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(1, 1, 0))) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "neighbor", "The neighbor triggers", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(0, 1, 0))) && Passed;
  Passed = EvaluateTrue("BlockTriggerChannel()", "last bar", "The last bar can be blocked", D->BlockTriggerChannel(2, 1)) && Passed;
  DisableDefaultStreams();
  bool OutX = D->BlockTriggerChannel(3, 0);
  bool OutY = D->BlockTriggerChannel(0, 2);
  EnableDefaultStreams();
  Passed = EvaluateFalse("BlockTriggerChannel()", "behind the last bar in x", "A bar which does not exist is rejected", OutX) && Passed;
  Passed = EvaluateFalse("BlockTriggerChannel()", "behind the last bar in y", "A bar which does not exist is rejected", OutY) && Passed;

  // A detector volume which does not fit a whole number of bars
  MDGeometry NoFit;
  Passed = EvaluateTrue("ScanSetupFile()", "no whole number of bars", "The fixture is accepted", Scan(NoFit, GetBars(2.0))) && Passed;
  if (CheckDetector(Passed, "NoFit", NoFit, "D", MDDetector::c_Calorimeter)) {
    MDDetector* F = NoFit.GetDetector("D");
    DisableDefaultStreams();
    bool Accepted = F->BlockTriggerChannel(0, 0);
    EnableDefaultStreams();
    bool Triggers = F->IsAboveTriggerThreshold(1000.0, MDGridPoint(0, 0, 0));
    // Defect GEO-D-058: If the number of bars is not a whole number, blocking a channel reports success but no channel is ever blocked
    Passed = ExpectedToFail(EvaluateFalse("BlockTriggerChannel()", "no whole number of bars", "A reported success means the channel does not trigger", Accepted == true && Triggers == true)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the neighbor search and the position resolution
bool UTDCalorimeter::TestNearAndResolution()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "bars", "The fixture is accepted", Scan(Geometry, GetBars(), "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MVector Null(0.0, 0.0, 0.0);
  MVector DZ(0.0, 0.0, 0.1);

  // Distance of the bars: 1.1, limit in squared bar distances: level + 1e-5
  Passed = EvaluateTrue("AreNear()", "next bar, level 1", "The next bar is near", D->AreNear(Null, Null, MVector(1.1, 0.0, 0.0), Null, -1.0, 1)) && Passed;
  Passed = EvaluateFalse("AreNear()", "just outside", "A distance of 1.1001 is not near", D->AreNear(Null, Null, MVector(1.1001, 0.0, 0.0), Null, -1.0, 1)) && Passed;
  Passed = EvaluateFalse("AreNear()", "diagonal, level 1", "The diagonal bar is not near at level 1", D->AreNear(Null, Null, MVector(1.1, 1.1, 0.0), Null, -1.0, 1)) && Passed;
  Passed = EvaluateTrue("AreNear()", "diagonal, level 2", "The diagonal bar is near at level 2", D->AreNear(Null, Null, MVector(1.1, 1.1, 0.0), Null, -1.0, 2)) && Passed;
  Passed = EvaluateTrue("AreNear()", "depth just inside", "A depth difference of 0.0999 with 1 sigma of 0.1 is near", D->AreNear(Null, DZ, MVector(0.0, 0.0, 0.0999), DZ, 1.0, 1)) && Passed;
  Passed = EvaluateFalse("AreNear()", "depth just outside", "A depth difference of 0.1001 with 1 sigma of 0.1 is not near", D->AreNear(Null, DZ, MVector(0.0, 0.0, 0.1001), DZ, 1.0, 1)) && Passed;

  // Position resolution: Standard deviation of a uniform distribution over the full width
  MVector Resolution = D->GetPositionResolution(Null, 300.0);
  Passed = EvaluateNear("GetPositionResolution()", "x", "The resolution is the width/sqrt(12)", Resolution.X(), 1.0/sqrt(12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPositionResolution()", "y", "The resolution is the width/sqrt(12)", Resolution.Y(), 1.0/sqrt(12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPositionResolution()", "z", "The resolution is the interpolated depth resolution", Resolution.Z(), 0.2, 1e-12) && Passed;

  MDGeometry NoDepth;
  Passed = EvaluateTrue("ScanSetupFile()", "no depth resolution", "The fixture is accepted", Scan(NoDepth, GetBars())) && Passed;
  if (CheckDetector(Passed, "NoDepth", NoDepth, "D", MDDetector::c_Calorimeter)) {
    Passed = EvaluateNear("GetPositionResolution()", "no depth resolution", "The resolution is the height/sqrt(12)", NoDepth.GetDetector("D")->GetPositionResolution(Null, 300.0).Z(), 4.0/sqrt(12.0), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy, named detectors, and text output
bool UTDCalorimeter::TestCopyNamedText()
{
  bool Passed = true;
  MString Flags;

  Passed = CheckCopyDataToNamedDetectors<MDCalorimeter>("Calorimeter") && Passed;
  gRandom->SetSeed(8002);

  const MString Keywords = "D.DepthResolution 100 0.1 0\nD.DepthResolution 500 0.3 0\n";
  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution", "The fixture is accepted", Scan(Geometry, GetBody(), Keywords)) && Passed;
  if (Passed == false) {
    return false;
  }
  MDCalorimeter* D = dynamic_cast<MDCalorimeter*>(Geometry.GetDetector("D"));
  MVector Null(0.0, 0.0, 0.0);

  Passed = CheckCloneIndependence("Calorimeter", D) && Passed;
  unique_ptr<MDDetector> Cloned(D->Clone());
  MDCalorimeter* Copy = dynamic_cast<MDCalorimeter*>(Cloned.get());
  if (EvaluateTrue("Clone()", "calorimeter", "The clone is a calorimeter", Copy != nullptr) == false) {
    return false;
  }
  Passed = EvaluateTrue("MDCalorimeter(const MDCalorimeter&)", "depth resolution", "The flag is copied", Copy->HasDepthResolution()) && Passed;
  Passed = EvaluateNear("MDCalorimeter(const MDCalorimeter&)", "depth resolution", "The table is copied", Copy->GetPositionResolution(Null, 300.0).Z(), 0.2, 1e-12) && Passed;

  // Text
  MString Text = D->GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "type", "The type is written", Text.Contains("Calorimeter D")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "depth resolution", "The depth resolution is written", Text.Contains("D.DepthResolution 500 0.3 0")) && Passed;
  MDGeometry Second;
  Passed = EvaluateTrue("ScanSetupFile()", "round trip", "The text can be scanned again", Scan(Second, GetBoxVolume("Box", 3.0, 2.0, 1.0), Text)) && Passed;
  if (CheckDetector(Passed, "Second", Second, "D", MDDetector::c_Calorimeter)) {
    Passed = Evaluate("GetGeomega()", "round trip, all fields", "The text written after the round trip is identical, so that no written field is lost", dynamic_cast<MDCalorimeter*>(Second.GetDetector("D"))->GetGeomega(), Text) && Passed;
    Passed = Evaluate("GetGeomega()", "round trip", "The type is identical", Second.GetDetector("D")->GetType(), MDDetector::c_Calorimeter) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The depth resolution is identical", Second.GetDetector("D")->GetPositionResolution(Null, 300.0).Z(), 0.2, 1e-12) && Passed;
  }
  Passed = EvaluateTrue("ToString()", "standard", "The type is written", D->ToString().Contains("Calorimeter")) && Passed;

  // Named detectors
  MString Named = GetBody() + Keywords + "D.Named N\nN.Assign World.Box\nN.DepthResolution 100 0.4 0\nN.DepthResolution 500 0.4 0\n";
  MDGeometryQuest GeometryOwn;
  Passed = EvaluateTrue("ScanSetupFile()", "named detector with its own depth resolution", "The fixture is accepted", Scan(GeometryOwn, Named)) && Passed;
  if (CheckDetector(Passed, "GeometryOwn", GeometryOwn, "N", MDDetector::c_Calorimeter)) {
    Passed = EvaluateNear("GetPositionResolution()", "named detector with its own depth resolution", "The own table is used", GeometryOwn.GetDetector("N")->GetPositionResolution(Null, 300.0).Z(), 0.4, 1e-12) && Passed;
    Passed = EvaluateNear("GetPositionResolution()", "base detector", "The base keeps its table", GeometryOwn.GetDetector("D")->GetPositionResolution(Null, 300.0).Z(), 0.2, 1e-12) && Passed;
    MDVolumeSequence Sequence = GeometryOwn.GetVolumeSequence(Null, true, true);
    vector<double> Depths;
    for (unsigned int i = 0; i < c_Samples; ++i) {
      MVector Position = Null;
      double Energy = 300.0;
      double Time = 0.0;
      GeometryOwn.Noise(Position, Energy, Time, Flags, Sequence);
      Depths.push_back(Position.Z());
    }
    // The clipping at c = 0.9999 with t = c/sigma matters here. Variance of a clipped Gaussian: sigma^2 (1 - 2Q(t) - 2 t phi(t)) + 2 Q(t) c^2
    double Sigma = 0.4;
    double Cut = 0.9999;
    double T = Cut/Sigma;
    double Q = GetPhi(-T);
    double Density = exp(-0.5*T*T)/sqrt(2.0*c_Pi);
    double Expected = sqrt(Sigma*Sigma*(1.0 - 2.0*Q - 2.0*T*Density) + 2.0*Q*Cut*Cut);
    Passed = EvaluateNear("MDGeometryQuest::Noise()", "named detector with its own depth resolution", "The mean is zero", MStatistics::GetMean(Depths), 0.0, GetMeanTolerance(Sigma)) && Passed;
    Passed = EvaluateNear("MDGeometryQuest::Noise()", "named detector with its own depth resolution", "The width is the one of the clipped Gaussian with the own resolution", MStatistics::GetStandardDeviation(Depths), Expected, GetRMSTolerance(Sigma)) && Passed;
  }
  MDGeometry GeometryInherit;
  Passed = EvaluateTrue("ScanSetupFile()", "named detector inherits", "The fixture is accepted", Scan(GeometryInherit, GetBody() + Keywords, "D.Named N\nN.Assign World.Box\n")) && Passed;
  if (CheckDetector(Passed, "GeometryInherit", GeometryInherit, "N", MDDetector::c_Calorimeter)) {
    Passed = EvaluateNear("GetPositionResolution()", "named detector inherits", "The table is inherited", GeometryInherit.GetDetector("N")->GetPositionResolution(Null, 300.0).Z(), 0.2, 1e-12) && Passed;
    Passed = EvaluateTrue("HasDepthResolution()", "named detector inherits", "The flag is inherited", dynamic_cast<MDCalorimeter*>(GeometryInherit.GetDetector("N"))->HasDepthResolution()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDCalorimeter Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDCalorimeter.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
