/*
 * UTDVoxel3D.cxx
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


//! Unit test of the voxel detector MDVoxel3D
//! Attention: the declared function SetWidth has no definition and cannot be linked, thus it is not tested
class UTDVoxel3D : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDVoxel3D() : UTDDetectorBase("UTDVoxel3D") {}
  //! Default destructor
  virtual ~UTDVoxel3D() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the fixture with one block: half size 3 x 2 x 1, offset 0.5 x 0.25 x 0.1, 10 x 7 x 4 voxels - size 0.5 x 0.5 x 0.45
  MString GetBody() const;
  //! Return the fixture with 2 x 1 x 2 blocks of half size 1, gap 0.2 x 0 x 0.2, 2 x 2 x 2 voxels
  MString GetBlocks(const MString& Offset = "0.0 0.0 0.0") const;

  //! Test the construction, setters, and getters
  bool TestConstruction();
  //! Test the keyword parsing and validation
  bool TestKeywords();
  //! Test the grid of one block
  bool TestGridOneBlock();
  //! Test the grid of many blocks
  bool TestGridManyBlocks();
  //! Test the guard ring
  bool TestGuardRing();
  //! Test the blocked channels
  bool TestBlockedChannels();
  //! Test the neighbor search and the position resolution
  bool TestNearAndResolution();
  //! Test the energy loss map
  bool TestEnergyLossMap();
  //! Test copy, named detectors, and text output
  bool TestCopyNamedText();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDVoxel3D::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestKeywords() && Passed;
  Passed = TestCommonNoiseChain(GetBody(), MVector(0.25, 0.25, 0.5)) && Passed;
  Passed = TestGridOneBlock() && Passed;
  Passed = TestGridManyBlocks() && Passed;
  Passed = TestGuardRing() && Passed;
  Passed = TestBlockedChannels() && Passed;
  Passed = TestNearAndResolution() && Passed;
  Passed = TestEnergyLossMap() && Passed;
  Passed = TestCopyNamedText() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture with one block: half size 3 x 2 x 1, offset 0.5 x 0.25 x 0.1, 10 x 7 x 4 voxels - size 0.5 x 0.5 x 0.45
MString UTDVoxel3D::GetBody() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "Voxel3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25 0.1\nD.Voxels 10 7 4\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture with 2 x 1 x 2 blocks of half size 1, gap 0.2 x 0 x 0.2, 2 x 2 x 2 voxels
MString UTDVoxel3D::GetBlocks(const MString& Offset) const
{
  MString Body = GetBoxVolume("Container", 2.1, 1.0, 2.1);
  // The block is the template of the four copies, and is not placed itself
  Body += "Volume Block\nBlock.Material Germanium\nBlock.Shape BRIK 1.0 1.0 1.0\n";
  Body += "Block.Copy B1\nB1.Position -1.1 0.0 -1.1\nB1.Mother Container\n";
  Body += "Block.Copy B2\nB2.Position 1.1 0.0 -1.1\nB2.Mother Container\n";
  Body += "Block.Copy B3\nB3.Position -1.1 0.0 1.1\nB3.Mother Container\n";
  Body += "Block.Copy B4\nB4.Position 1.1 0.0 1.1\nB4.Mother Container\n";
  Body += "Voxel3D D\nD.SensitiveVolume Block\nD.DetectorVolume Container\nD.StructuralPitch 0.2 0.0 0.2\nD.StructuralOffset 0.0 0.0 0.0\nD.Voxels 2 2 2\n";
  Body += MString("D.Offset ") + Offset + "\n";
  return Body;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction, setters, and getters
bool UTDVoxel3D::TestConstruction()
{
  bool Passed = true;

  MDVoxel3D Voxel("Voxel");
  Passed = EvaluateTrue("MDVoxel3D()", "name", "The name is stored", Voxel.GetName() == "Voxel") && Passed;
  Passed = Evaluate("MDVoxel3D()", "type", "The type is Voxel3D", Voxel.GetType(), MDDetector::c_Voxel3D) && Passed;
  Passed = EvaluateTrue("MDVoxel3D()", "type name", "The type name is Voxel3D", Voxel.GetTypeName() == MDDetector::c_Voxel3DName) && Passed;
  Passed = EvaluateFalse("MDVoxel3D()", "guard ring", "There is no guard ring", Voxel.HasGuardRing()) && Passed;
  Passed = Evaluate("MDVoxel3D()", "voxels", "The number of voxels is not defined", Voxel.GetNVoxelsZ(), g_IntNotDefined) && Passed;
  Passed = EvaluateNear("MDVoxel3D()", "offset", "The offset is not defined", Voxel.GetOffsetZ(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateNear("MDVoxel3D()", "width", "The width is not defined", Voxel.GetWidthZ(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;

  Voxel.SetOffset(1.5, 2.5, 3.5);
  Passed = EvaluateNear("SetOffset()", "x", "The value is stored", Voxel.GetOffsetX(), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetOffset()", "y", "The value is stored", Voxel.GetOffsetY(), 2.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetOffset()", "z", "The value is stored", Voxel.GetOffsetZ(), 3.5, 1e-12) && Passed;
  Voxel.SetNVoxels(4, 5, 6);
  Passed = Evaluate("SetNVoxels()", "x", "The value is stored", Voxel.GetNVoxelsX(), 4) && Passed;
  Passed = Evaluate("SetNVoxels()", "y", "The value is stored", Voxel.GetNVoxelsY(), 5) && Passed;
  Passed = Evaluate("SetNVoxels()", "z", "The value is stored", Voxel.GetNVoxelsZ(), 6) && Passed;

  Voxel.HasGuardRing(true);
  Passed = EvaluateTrue("HasGuardRing(bool)", "true", "The guard ring exists", Voxel.HasGuardRing()) && Passed;
  Passed = EvaluateTrue("HasGuardRing(bool)", "true", "The guard ring has the name of the detector", Voxel.GetGuardRing()->GetName() == "Voxel_GuardRing") && Passed;
  MDGuardRing* First = Voxel.GetGuardRing();
  Voxel.HasGuardRing(true);
  Passed = EvaluateTrue("HasGuardRing(bool)", "second true", "The existing guard ring stays", Voxel.GetGuardRing() == First) && Passed;
  Voxel.HasGuardRing(false);
  Passed = EvaluateFalse("HasGuardRing(bool)", "false", "The guard ring is gone", Voxel.HasGuardRing()) && Passed;
  Passed = EvaluateTrue("HasGuardRing(bool)", "false", "The guard ring pointer is empty", Voxel.GetGuardRing() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword parsing and validation
bool UTDVoxel3D::TestKeywords()
{
  bool Passed = true;

  const MString Volume = GetBoxVolume("Box", 3.0, 2.0, 1.0);
  const MString Head = Volume + "Voxel3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\n";

  Passed = EvaluateTrue("ScanSetupFile()", "complete", "A complete detector is accepted", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.Voxels 2 2 2\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "alias", "The class name is an alias of the type", IsScanOK(Volume, "MDVoxel3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.1 0.1 0.1\nD.Voxels 2 2 2\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "VoxelNumber", "The alternative keyword is accepted", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.VoxelNumber 2 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "no offset", "A missing offset is rejected", IsScanOK(Head, "D.Voxels 2 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "no voxels", "A missing number of voxels is rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "negative x-offset", "A negative offset is rejected", IsScanOK(Head, "D.Offset -0.1 0.1 0.1\nD.Voxels 2 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "negative y-offset", "A negative offset is rejected", IsScanOK(Head, "D.Offset 0.1 -0.1 0.1\nD.Voxels 2 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "zero voxels in x", "Zero voxels are rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.Voxels 0 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "zero voxels in y", "Zero voxels are rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.Voxels 2 0 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Offset with two values", "The wrong number of tokens is rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.Voxels 2 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Voxels with two values", "The wrong number of tokens is rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.Voxels 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "StripNumber", "A keyword of a strip detector is rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.Voxels 2 2 2\nD.StripNumber 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "DepthResolution", "A keyword of a 3D strip detector is rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.Voxels 2 2 2\nD.DepthResolution 100 1\n")) && Passed;

  // Defect GEO-D-059: The validation checks the offset and the number of voxels only in x and y and not in z, which leads to an infinite voxel size for zero voxels
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "zero voxels in z", "Zero voxels are rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.Voxels 2 2 0\n"))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "negative voxels in z", "Negative numbers of voxels are rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.Voxels 2 2 -1\n"))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "negative z-offset", "A negative offset is rejected", IsScanOK(Head, "D.Offset 0.1 0.1 -0.1\nD.Voxels 2 2 2\n"))) && Passed;
  // Defect GEO-D-060: An offset larger than half of the block results in a negative voxel size and is accepted
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "offset larger than the half width", "The offset must leave an active area", IsScanOK(Head, "D.Offset 3.5 0.1 0.1\nD.Voxels 2 2 2\n"))) && Passed;

  // Shapes and volumes
  Passed = EvaluateFalse("ScanSetupFile()", "sphere", "A sphere as detector volume is rejected", IsScanOK("Volume Ball\nBall.Material Silicon\nBall.Shape SPHE 0 2 0 180 0 360\nBall.Position 0 0 0\nBall.Mother World\n", "Voxel3D D\nD.SensitiveVolume Ball\nD.DetectorVolume Ball\nD.Offset 0.1 0.1 0.1\nD.Voxels 2 2 2\n")) && Passed;
  MString Two = GetBoxVolume("Container", 5.0, 5.0, 5.0) + GetBoxVolume("Part1", 1.0, 1.0, 1.0, 2.0, "Container") + GetBoxVolume("Part2", 1.0, 1.0, 1.0, -2.0, "Container");
  Passed = EvaluateFalse("ScanSetupFile()", "two sensitive volumes", "Two sensitive volumes are rejected", IsScanOK(Two, "Voxel3D D\nD.SensitiveVolume Part1\nD.SensitiveVolume Part2\nD.DetectorVolume Container\nD.StructuralPitch 0 0 0\nD.StructuralOffset 0 0 0\nD.Offset 0.1 0.1 0.1\nD.Voxels 2 2 2\n")) && Passed;

  MDVoxel3D Direct("Direct");
  DisableDefaultStreams();
  bool Result = Direct.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no volumes", "A detector without volumes is invalid", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the grid of one block
bool UTDVoxel3D::TestGridOneBlock()
{
  bool Passed = true;
  MString Flags;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one block", "The fixture is accepted", Scan(Geometry, GetBody())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDVoxel3D* D = dynamic_cast<MDVoxel3D*>(Geometry.GetDetector("D"));
  MDVolume* V = Geometry.GetVolume("Box");

  Passed = EvaluateNear("Validate()", "width x", "The width is twice the half size", D->GetWidthX(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "width y", "The width is twice the half size", D->GetWidthY(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "width z", "The width is twice the half size", D->GetWidthZ(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetVoxelSizeX()", "one block", "The size is 5/10", D->GetVoxelSizeX(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetVoxelSizeY()", "one block", "The size is 3.5/7", D->GetVoxelSizeY(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetVoxelSizeZ()", "one block", "The size is 1.8/4", D->GetVoxelSizeZ(), 0.45, 1e-12) && Passed;

  // All voxel centers by hand
  for (unsigned int i = 0; i < 10; ++i) {
    for (unsigned int j = 0; j < 7; ++j) {
      for (unsigned int k = 0; k < 4; ++k) {
        MVector Center(-2.5 + (i + 0.5)*0.5, -1.75 + (j + 0.5)*0.5, -0.9 + (k + 0.5)*0.45);
        Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "voxel center", "The center of the voxel is returned", D->GetPositionInDetectorVolume(i, j, k, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, V), Center, 1e-9) && Passed;
        MDGridPoint Point = D->GetGridPoint(Center);
        Passed = Evaluate("GetGridPoint()", "voxel center", "The voxel in x is found", Point.GetXGrid(), i) && Passed;
        Passed = Evaluate("GetGridPoint()", "voxel center", "The voxel in y is found", Point.GetYGrid(), j) && Passed;
        Passed = Evaluate("GetGridPoint()", "voxel center", "The voxel in z is found", Point.GetZGrid(), k) && Passed;
        Passed = Evaluate("GetGridPoint()", "voxel center", "The type is voxel", Point.GetType(), MDGridPoint::c_Voxel) && Passed;
      }
    }
  }

  // Just inside and outside
  Passed = Evaluate("GetGridPoint()", "just below a boundary in z", "The hit is in the lower voxel", D->GetGridPoint(MVector(0.0, 0.0, -0.0001)).GetZGrid(), 1u) && Passed;
  Passed = Evaluate("GetGridPoint()", "just above a boundary in z", "The hit is in the upper voxel", D->GetGridPoint(MVector(0.0, 0.0, 0.0001)).GetZGrid(), 2u) && Passed;
  Passed = Evaluate("GetGridPoint()", "inside the highest z-voxel", "The hit is a voxel", D->GetGridPoint(MVector(0.0, 0.0, 0.8999)).GetType(), MDGridPoint::c_Voxel) && Passed;
  Passed = Evaluate("GetGridPoint()", "outside the highest z-voxel", "The hit is in the guard ring", D->GetGridPoint(MVector(0.0, 0.0, 0.9001)).GetType(), MDGridPoint::c_GuardRing) && Passed;
  Passed = Evaluate("GetGridPoint()", "inside the lowest z-voxel", "The hit is a voxel", D->GetGridPoint(MVector(0.0, 0.0, -0.8999)).GetType(), MDGridPoint::c_Voxel) && Passed;
  Passed = Evaluate("GetGridPoint()", "outside the lowest z-voxel", "The hit is in the guard ring", D->GetGridPoint(MVector(0.0, 0.0, -0.9001)).GetType(), MDGridPoint::c_GuardRing) && Passed;

  // Exactly on the upper edge of the active area
  DisableDefaultStreams();
  unsigned int TypeOnEdge = D->GetGridPoint(MVector(2.5, 0.0, 0.0)).GetType();
  unsigned int TypeBelow = D->GetGridPoint(MVector(-3.5, 0.0, 0.0)).GetType();
  unsigned int TypeBelowZ = D->GetGridPoint(MVector(0.0, 0.0, -1.5)).GetType();
  unsigned int TypeAbove = D->GetGridPoint(MVector(3.6, 0.0, 0.0)).GetType();
  unsigned int TypeAboveZ = D->GetGridPoint(MVector(0.0, 0.0, 1.05)).GetType();
  unsigned int TypeFarAbove = D->GetGridPoint(MVector(30.0, 0.0, 0.0)).GetType();
  EnableDefaultStreams();
  // Defect GEO-D-061: A hit exactly on the upper boundary of the active area gets the voxel number N, which is not valid, and the hit is lost
  Passed = ExpectedToFail(Evaluate("GetGridPoint()", "exactly on the upper edge of the active area", "The hit belongs to the last voxel", TypeOnEdge, MDGridPoint::c_Voxel)) && Passed;
  Passed = Evaluate("GetGridPoint()", "below the block in x", "A hit outside is unknown", TypeBelow, MDGridPoint::c_Unknown) && Passed;
  Passed = Evaluate("GetGridPoint()", "below the block in z", "A hit outside is unknown", TypeBelowZ, MDGridPoint::c_Unknown) && Passed;
  Passed = Evaluate("GetGridPoint()", "far above the block", "A hit outside is unknown", TypeFarAbove, MDGridPoint::c_Unknown) && Passed;
  // Defect GEO-D-062: The number of blocks is one too high if the blocks fill the detector volume, and a hit outside of the block is mapped to voxels of a block which does not exist
  Passed = ExpectedToFail(Evaluate("GetGridPoint()", "above the block in x", "A hit outside is unknown", TypeAbove, MDGridPoint::c_Unknown)) && Passed;
  Passed = ExpectedToFail(Evaluate("GetGridPoint()", "above the block in z", "A hit outside is unknown", TypeAboveZ, MDGridPoint::c_Unknown)) && Passed;

  // Discretize
  vector<MDGridPoint> Points = D->Grid(MVector(0.25, 0.25, 0.5), 123.0, 45.0, V);
  Passed = EvaluateSize("Grid()", "active area", "There is one grid point", Points.size(), 1) && Passed;
  if (Points.size() == 1) {
    Passed = Evaluate("Grid()", "x", "The x-voxel is correct", Points[0].GetXGrid(), 5u) && Passed;
    Passed = Evaluate("Grid()", "y", "The y-voxel is correct", Points[0].GetYGrid(), 4u) && Passed;
    // (0.5 + 0.9)/0.45 = 3.1
    Passed = Evaluate("Grid()", "z", "The z-voxel is correct", Points[0].GetZGrid(), 3u) && Passed;
    Passed = EvaluateNear("Grid()", "energy", "The energy is stored", Points[0].GetEnergy(), 123.0, 1e-12) && Passed;
    Passed = EvaluateNear("Grid()", "time", "The time is stored", Points[0].GetTime(), 45.0, 1e-12) && Passed;
  }
  DisableDefaultStreams();
  Points = D->Grid(MVector(-3.5, 0.0, 0.0), 123.0, 45.0, V);
  EnableDefaultStreams();
  Passed = EvaluateSize("Grid()", "outside", "There is no grid point outside", Points.size(), 0) && Passed;

  // The noising does not change the position: the discretization is done in the simulation
  MVector Position(0.25, 0.25, 0.5);
  double Energy = 100.0;
  double Time = 0.0;
  D->Noise(Position, Energy, Time, Flags, V);
  Passed = EvaluateVectorNear("Noise()", "position", "The position stays", Position, MVector(0.25, 0.25, 0.5), 1e-12) && Passed;

  // One voxel per axis
  MDGeometry One;
  Passed = EvaluateTrue("ScanSetupFile()", "one voxel", "The fixture is accepted", Scan(One, GetBoxVolume("Box", 2.0, 2.0, 1.0), "Voxel3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0 0.0\nD.Voxels 1 1 1\n")) && Passed;
  if (CheckDetector(Passed, "One", One, "D", MDDetector::c_Voxel3D)) {
    MDGridPoint Point = One.GetDetector("D")->GetGridPoint(MVector(1.9, -1.9, 0.9));
    Passed = Evaluate("GetGridPoint()", "one voxel", "Everything is in the voxel", Point.GetZGrid(), 0u) && Passed;
    Passed = Evaluate("GetGridPoint()", "one voxel", "The type is voxel", Point.GetType(), MDGridPoint::c_Voxel) && Passed;
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "one voxel", "The position is the center", One.GetDetector("D")->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, One.GetVolume("Box")), MVector(0.0, 0.0, 0.0), 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the grid of many blocks
bool UTDVoxel3D::TestGridManyBlocks()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "many blocks", "The fixture is accepted", Scan(Geometry, GetBlocks())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Container");

  // Grid 3, 1, 2 by hand: block center x = -2.1 + 1 + 2.2 = 1.1, voxel center +0.5; y: voxel center +0.5; block z = 1.1, voxel center -0.5
  Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "3, 1, 2", "The position by hand is returned", D->GetPositionInDetectorVolume(3, 1, 2, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, V), MVector(1.6, 0.5, 0.6), 1e-9) && Passed;

  for (unsigned int i = 0; i < 4; ++i) {
    for (unsigned int j = 0; j < 2; ++j) {
      for (unsigned int k = 0; k < 4; ++k) {
        MDGridPoint Point = D->GetGridPoint(D->GetPositionInDetectorVolume(i, j, k, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, V));
        Passed = Evaluate("GetGridPoint()", "round trip x", "The round trip restores the x-voxel", Point.GetXGrid(), i) && Passed;
        Passed = Evaluate("GetGridPoint()", "round trip y", "The round trip restores the y-voxel", Point.GetYGrid(), j) && Passed;
        Passed = Evaluate("GetGridPoint()", "round trip z", "The round trip restores the z-voxel", Point.GetZGrid(), k) && Passed;
      }
    }
  }

  // In the gap between the blocks
  DisableDefaultStreams();
  unsigned int Gap = D->GetGridPoint(MVector(0.0, 0.0, -1.1)).GetType();
  EnableDefaultStreams();
  Passed = Evaluate("GetGridPoint()", "gap between the blocks", "A hit in the gap is unknown", Gap, MDGridPoint::c_Unknown) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the guard ring
bool UTDVoxel3D::TestGuardRing()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one block", "The fixture is accepted", Scan(Geometry, GetBody())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDVoxel3D* D = dynamic_cast<MDVoxel3D*>(Geometry.GetDetector("D"));
  MDVolume* V = Geometry.GetVolume("Box");

  Passed = EvaluateTrue("HasGuardRing()", "with offset", "An offset creates a guard ring", D->HasGuardRing()) && Passed;
  Passed = EvaluateFalse("IsActive()", "no guard ring keywords", "The automatically created guard ring is passive", D->GetGuardRing()->IsActive()) && Passed;
  // Middle of the band: -3 + 0.25, -2 + 0.125, -1 + 0.05
  Passed = EvaluateVectorNear("GetUniquePosition()", "with offset", "The position is in the middle of the band", D->GetGuardRing()->GetUniquePosition(), MVector(-2.75, -1.875, -0.95), 1e-9) && Passed;
  Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "guard ring", "The unique position is returned for guard ring hits", D->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_GuardRing, V), MVector(-2.75, -1.875, -0.95), 1e-9) && Passed;

  // Guard ring in the second block in x
  MDGeometry Blocks;
  Passed = EvaluateTrue("ScanSetupFile()", "many blocks with an offset", "The fixture is accepted", Scan(Blocks, GetBlocks("0.2 0.2 0.2"))) && Passed;
  if (CheckDetector(Passed, "Blocks", Blocks, "D", MDDetector::c_Voxel3D)) {
    MDDetector* B = Blocks.GetDetector("D");
    // Upper edge of the second block in x: 2.1, band from 1.9, and the second block in z
    MDGridPoint Point = B->GetGridPoint(MVector(2.0, 0.0, 1.1));
    Passed = Evaluate("GetGridPoint()", "guard ring of a block", "The type is guard ring", Point.GetType(), MDGridPoint::c_GuardRing) && Passed;
    Passed = Evaluate("GetGridPoint()", "guard ring of a block", "The first voxel of the block in x is stored", Point.GetXGrid(), 2u) && Passed;
    Passed = Evaluate("GetGridPoint()", "guard ring of a block", "The first voxel of the block in y is stored", Point.GetYGrid(), 0u) && Passed;
    Passed = Evaluate("GetGridPoint()", "guard ring of a block", "The first voxel of the block in z is stored", Point.GetZGrid(), 2u) && Passed;
    // Unique position relative to the center of the block at (1.1, 0, 1.1): -1 + 0.1 in all coordinates
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "guard ring of a block", "The unique position in the block is returned", B->GetPositionInDetectorVolume(2, 0, 2, MVector(0.0, 0.0, 0.0), MDGridPoint::c_GuardRing, Blocks.GetVolume("Container")), MVector(0.2, -0.9, 0.2), 1e-9) && Passed;
  }

  // Offset only along z
  const MString ZOnly = GetBoxVolume("Box", 2.0, 2.0, 1.0) + "Voxel3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0 0.1\nD.Voxels 4 4 4\n";
  MDGeometry OneAxis;
  Passed = EvaluateTrue("ScanSetupFile()", "offset only in z", "The fixture is accepted", Scan(OneAxis, ZOnly)) && Passed;
  if (CheckDetector(Passed, "OneAxis", OneAxis, "D", MDDetector::c_Voxel3D)) {
    Passed = Evaluate("GetGridPoint()", "offset only in z", "A hit in the band is a guard ring hit", OneAxis.GetDetector("D")->GetGridPoint(MVector(0.0, 0.0, 0.95)).GetType(), MDGridPoint::c_GuardRing) && Passed;
    // Defect GEO-D-063: A guard ring is only created if the x and y offsets are nonzero, but the grid points of a band along one axis are guard ring hits
    Passed = ExpectedToFail(EvaluateTrue("HasGuardRing()", "offset only in z", "There is a guard ring because hits in the band are guard ring hits", OneAxis.GetDetector("D")->HasGuardRing())) && Passed;
  }

  // Guard ring without an offset
  Passed = EvaluateFalse("ScanSetupFile()", "guard ring keywords without offset", "Guard ring keywords without an offset are rejected", IsScanOK(GetBoxVolume("Box", 2.0, 2.0, 1.0), "Voxel3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0 0.0\nD.Voxels 4 4 4\nD.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\n")) && Passed;

  // Without a guard ring
  MDGeometry NoRing;
  Passed = EvaluateTrue("ScanSetupFile()", "no offset", "The fixture is accepted", Scan(NoRing, GetBoxVolume("Box", 2.0, 2.0, 1.0), "Voxel3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0 0.0\nD.Voxels 4 4 4\n")) && Passed;
  if (CheckDetector(Passed, "NoRing", NoRing, "D", MDDetector::c_Voxel3D)) {
    Passed = EvaluateFalse("HasGuardRing()", "no offset", "There is no guard ring", NoRing.GetDetector("D")->HasGuardRing()) && Passed;
    DisableDefaultStreams();
    MVector Position = NoRing.GetDetector("D")->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_GuardRing, NoRing.GetVolume("Box"));
    EnableDefaultStreams();
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "guard ring type without a guard ring", "The center of the volume is returned", Position, MVector(0.0, 0.0, 0.0), 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the blocked channels
bool UTDVoxel3D::TestBlockedChannels()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "BlockTrigger", "The fixture is accepted", Scan(Geometry, GetBody(), "D.BlockTrigger 2 3\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "blocked channel in the first layer", "The blocked channel does not trigger", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(2, 3, 0))) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "neighbor in x", "The neighbor triggers", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(3, 3, 0))) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "neighbor in y", "The neighbor triggers", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(2, 2, 0))) && Passed;
  Passed = EvaluateTrue("BlockTriggerChannel()", "last channel in the first layer", "The channel can be blocked", D->BlockTriggerChannel(9, 6)) && Passed;

  // Attention: Whether the other layers in z are also blocked is a question to the user
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the neighbor search and the position resolution
bool UTDVoxel3D::TestNearAndResolution()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one block", "The fixture is accepted", Scan(Geometry, GetBody())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MVector Null(0.0, 0.0, 0.0);

  // Squared distance in voxels below level + 1e-5
  DisableDefaultStreams();
  bool Next = D->AreNear(Null, Null, MVector(0.5, 0.0, 0.0), Null, -1.0, 1);
  bool Outside = D->AreNear(Null, Null, MVector(0.5001, 0.0, 0.0), Null, -1.0, 1);
  bool NextZ = D->AreNear(Null, Null, MVector(0.0, 0.0, 0.45), Null, -1.0, 1);
  bool OutsideZ = D->AreNear(Null, Null, MVector(0.0, 0.0, 0.4501), Null, -1.0, 1);
  bool Diagonal1 = D->AreNear(Null, Null, MVector(0.5, 0.5, 0.0), Null, -1.0, 1);
  bool Diagonal2 = D->AreNear(Null, Null, MVector(0.5, 0.5, 0.0), Null, -1.0, 2);
  bool Space3 = D->AreNear(Null, Null, MVector(0.5, 0.5, 0.45), Null, -1.0, 3);
  EnableDefaultStreams();
  Passed = EvaluateTrue("AreNear()", "next in x", "The next voxel is near", Next) && Passed;
  Passed = EvaluateFalse("AreNear()", "just outside in x", "A slightly larger distance is not near", Outside) && Passed;
  Passed = EvaluateTrue("AreNear()", "next in z", "The next voxel is near", NextZ) && Passed;
  Passed = EvaluateFalse("AreNear()", "just outside in z", "A slightly larger distance is not near", OutsideZ) && Passed;
  Passed = EvaluateFalse("AreNear()", "diagonal, level 1", "The diagonal voxel is not near at level 1", Diagonal1) && Passed;
  Passed = EvaluateTrue("AreNear()", "diagonal, level 2", "The diagonal voxel is near at level 2", Diagonal2) && Passed;
  Passed = EvaluateTrue("AreNear()", "space diagonal, level 3", "The space diagonal voxel is near at level 3", Space3) && Passed;

  // Standard deviation of a uniform distribution
  MVector Resolution = D->GetPositionResolution(Null, 100.0);
  Passed = EvaluateNear("GetPositionResolution()", "x", "The resolution is the voxel size/sqrt(12)", Resolution.X(), 0.5/sqrt(12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPositionResolution()", "y", "The resolution is the voxel size/sqrt(12)", Resolution.Y(), 0.5/sqrt(12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPositionResolution()", "z", "The resolution is the voxel size/sqrt(12)", Resolution.Z(), 0.45/sqrt(12.0), 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the energy loss map
bool UTDVoxel3D::TestEnergyLossMap()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("Other.dat");
  ostringstream Map;
  Map<<"XA -5 5"<<endl<<"YA -5 5"<<endl<<"ZA -5 5"<<endl;
  for (int x = -5; x <= 5; x += 10) {
    for (int y = -5; y <= 5; y += 10) {
      for (int z = -5; z <= 5; z += 10) {
        Map<<"DP "<<x<<" "<<y<<" "<<z<<" 0.75"<<endl;
      }
    }
  }
  Map<<"EN"<<endl;
  Passed = EvaluateTrue("WriteTextFile()", "map", "The map can be created", WriteTextFile(FileName, Map.str().c_str())) && Passed;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "other map", "The fixture is accepted", Scan(Geometry, GetBody(), MString("D.EnergyLossMap ") + FileName + "\n")) && Passed;
  if (CheckDetector(Passed, "Geometry", Geometry, "D", MDDetector::c_Voxel3D)) {
    MFunction3D Rescaled = Geometry.GetDetector("D")->GetEnergyLossMap();
    Passed = Evaluate("GetEnergyLossType()", "other map", "The type is map", Geometry.GetDetector("D")->GetEnergyLossType(), MDDetector::c_EnergyLossTypeMap) && Passed;
    Passed = EvaluateNear("Validate()", "other map, x min", "The map is rescaled to the active area", Rescaled.GetXMin(), -2.5, 1e-9) && Passed;
    Passed = EvaluateNear("Validate()", "other map, x max", "The map is rescaled to the active area", Rescaled.GetXMax(), 2.5, 1e-9) && Passed;
    Passed = EvaluateNear("Validate()", "other map, y min", "The map is rescaled to the active area", Rescaled.GetYMin(), -1.75, 1e-9) && Passed;
    Passed = EvaluateNear("Validate()", "other map, y max", "The map is rescaled to the active area", Rescaled.GetYMax(), 1.75, 1e-9) && Passed;
    // Defect GEO-D-064: The energy loss map is compared with the active area in z but rescaled to the full thickness
    Passed = ExpectedToFail(EvaluateNear("Validate()", "other map, z min", "The map is rescaled to the active area", Rescaled.GetZMin(), -0.9, 1e-9)) && Passed;
    Passed = ExpectedToFail(EvaluateNear("Validate()", "other map, z max", "The map is rescaled to the active area", Rescaled.GetZMax(), 0.9, 1e-9)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy, named detectors, and text output
bool UTDVoxel3D::TestCopyNamedText()
{
  bool Passed = true;

  Passed = CheckCopyDataToNamedDetectors<MDVoxel3D>("Voxel3D", [](MDVoxel3D& D) { D.SetNVoxels(3, 4, 5); }, [](MDVoxel3D& D) { return D.GetNVoxelsX() == 3 && D.GetNVoxelsY() == 4 && D.GetNVoxelsZ() == 5; }) && Passed;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one block", "The fixture is accepted", Scan(Geometry, GetBody(), "D.NoiseThreshold 10 1\nD.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 2\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDVoxel3D* D = dynamic_cast<MDVoxel3D*>(Geometry.GetDetector("D"));

  Passed = CheckCloneIndependence("Voxel3D", D) && Passed;
  unique_ptr<MDDetector> Cloned(D->Clone());
  MDVoxel3D* Copy = dynamic_cast<MDVoxel3D*>(Cloned.get());
  if (EvaluateTrue("Clone()", "voxel detector", "The clone is a voxel detector", Copy != nullptr) == false) {
    return false;
  }
  Passed = EvaluateNear("MDVoxel3D(const MDVoxel3D&)", "width", "The value is copied", Copy->GetWidthZ(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDVoxel3D(const MDVoxel3D&)", "offset", "The value is copied", Copy->GetOffsetZ(), 0.1, 1e-12) && Passed;
  Passed = Evaluate("MDVoxel3D(const MDVoxel3D&)", "voxels", "The value is copied", Copy->GetNVoxelsZ(), 4) && Passed;
  Passed = EvaluateNear("MDVoxel3D(const MDVoxel3D&)", "voxel size", "The value is copied", Copy->GetVoxelSizeZ(), 0.45, 1e-12) && Passed;
  Passed = Evaluate("MDVoxel3D(const MDVoxel3D&)", "grid", "The grid works in the copy", Copy->GetGridPoint(MVector(0.25, 0.25, 0.5)).GetZGrid(), 3u) && Passed;
  Passed = EvaluateTrue("MDVoxel3D(const MDVoxel3D&)", "guard ring", "The copy has its own guard ring", Copy->GetGuardRing() != nullptr && Copy->GetGuardRing() != D->GetGuardRing()) && Passed;
  delete Copy->GetGuardRing();

  // Text
  MString Text = D->GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "type", "The type is written", Text.Contains("Voxel3D D")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "offset", "The offset is written", Text.Contains("D.Offset 0.5 0.25 0.1")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "voxels", "The number of voxels is written", Text.Contains("D.Voxels 10 7 4")) && Passed;
  MDGeometry Second;
  Passed = EvaluateTrue("ScanSetupFile()", "round trip", "The text can be scanned again", Scan(Second, GetBoxVolume("Box", 3.0, 2.0, 1.0), Text)) && Passed;
  if (CheckDetector(Passed, "Second", Second, "D", MDDetector::c_Voxel3D)) {
    Passed = Evaluate("GetGeomega()", "round trip, all fields", "The text written after the round trip is identical, so that no written field is lost", dynamic_cast<MDVoxel3D*>(Second.GetDetector("D"))->GetGeomega(), Text) && Passed;
    MDVoxel3D* R = dynamic_cast<MDVoxel3D*>(Second.GetDetector("D"));
    Passed = EvaluateNear("GetGeomega()", "round trip", "The voxel size is identical", R->GetVoxelSizeZ(), 0.45, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The noise threshold is identical", R->GetNoiseThreshold(), 10.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The sigma of the noise threshold is identical", R->GetNoiseThresholdSigma(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The energy resolution is identical", R->GetEnergyResolutionWidth1(150.0), 1.5, 1e-9) && Passed;
  }

  MString Dump = D->ToString();
  Passed = EvaluateTrue("ToString()", "type", "The type is written", Dump.Contains("Voxel3D")) && Passed;
  Passed = EvaluateTrue("ToString()", "voxels", "The number of voxels is written", Dump.Contains("voxels: 10, 7, 4")) && Passed;

  // Named detectors
  MDGeometry Named;
  Passed = EvaluateTrue("ScanSetupFile()", "named detector", "The fixture is accepted", Scan(Named, GetBody(), "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\nD.Named N\nN.Assign World.Box\nN.NoiseThreshold 50\n")) && Passed;
  if (CheckDetector(Passed, "Named", Named, "N", MDDetector::c_Voxel3D)) {
    MDVoxel3D* N = dynamic_cast<MDVoxel3D*>(Named.GetDetector("N"));
    Passed = Evaluate("CopyDataToNamedDetectors()", "voxels", "The number of voxels is inherited", N->GetNVoxelsZ(), 4) && Passed;
    Passed = EvaluateNear("CopyDataToNamedDetectors()", "offset", "The offset is inherited", N->GetOffsetZ(), 0.1, 1e-12) && Passed;
    Passed = EvaluateNear("Validate()", "voxel size", "The voxel size is derived", N->GetVoxelSizeZ(), 0.45, 1e-12) && Passed;
    Passed = EvaluateNear("GetNoiseThreshold()", "own value", "The own value is used", N->GetNoiseThreshold(), 50.0, 1e-12) && Passed;
    Passed = EvaluateTrue("GetGuardRing()", "inherited guard ring", "The named detector has an active guard ring", N->GetGuardRing() != nullptr && N->GetGuardRing()->IsActive()) && Passed;
    Passed = EvaluateNear("Noise()", "named detector, below the own threshold", "The hit is removed", NoiseOnce(N, Named.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 40.0), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "base detector, same energy", "The hit stays", NoiseOnce(Named.GetDetector("D"), Named.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 40.0), 40.0, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDVoxel3D Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDVoxel3D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
