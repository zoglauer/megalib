/*
 * UTDGridPointCollection.cxx
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
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TROOT.h>

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGeometry.h"
#include "MDGridPoint.h"
#include "MDGridPointCollection.h"
#include "MDVolumeSequence.h"
#include "MString.h"
#include "MVector.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the collection of grid points MDGridPointCollection
class UTDGridPointCollection : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDGridPointCollection();
  //! Default destructor
  virtual ~UTDGridPointCollection() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the geometry with one detector of each type used in the test
  MString GetGeometry() const;
  //! Return the volume sequence at the world position
  MDVolumeSequence GetSequence(const MVector& Position);
  //! Add a hit at a world position to a collection
  void AddHit(MDGridPointCollection& Collection, const MVector& Position, double Energy, double Time, const vector<int>& Origins);

  //! Test the constructor and the initial content
  bool TestConstruction();
  //! Test the identification of the detector
  bool TestHasSameDetector();
  //! Test hits in strip detectors
  bool TestStrip2D();
  //! Test hits in the guard ring of a strip detector
  bool TestGuardRing();
  //! Test hits in 3D strip detectors
  bool TestStrip3D();
  //! Test hits in voxel detectors
  bool TestVoxel3D();
  //! Test hits in calorimeters
  bool TestCalorimeter();
  //! Test hits in scintillators
  bool TestScintillator();
  //! Test hits in Anger cameras
  bool TestAngerCamera();
  //! Test hits outside of the detector
  bool TestOutsideDetector();
  //! Test the sums and weights over the grid points
  bool TestSumsAndWeights();
  //! Test adding hits without discretization
  bool TestAddUndiscretized();
  //! Test merging collections
  bool TestMerge();
  //! Test copying collections
  bool TestCopy();
  //! Test the function which is called after all hits have been added
  bool TestFinalizeGridding();

  //! The geometry
  MDGeometry m_Geometry;
  //! Flag indicating if the geometry has been read
  bool m_GeometryIsRead;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
UTDGridPointCollection::UTDGridPointCollection() : UTGeomegaBase("UTDGridPointCollection"), m_GeometryIsRead(false)
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDGridPointCollection::Run()
{
  bool Passed = true;

  m_GeometryIsRead = ScanGeometry(m_Geometry, GetGeometry());
  Passed = EvaluateTrue("ScanSetupFile()", "all detector types", "The geometry of the test can be read", m_GeometryIsRead) && Passed;

  if (m_GeometryIsRead == true) {
    Passed = TestConstruction() && Passed;
    Passed = TestHasSameDetector() && Passed;
    Passed = TestStrip2D() && Passed;
    Passed = TestGuardRing() && Passed;
    Passed = TestStrip3D() && Passed;
    Passed = TestVoxel3D() && Passed;
    Passed = TestCalorimeter() && Passed;
    Passed = TestScintillator() && Passed;
    Passed = TestAngerCamera() && Passed;
    Passed = TestOutsideDetector() && Passed;
    Passed = TestSumsAndWeights() && Passed;
    Passed = TestAddUndiscretized() && Passed;
    Passed = TestMerge() && Passed;
    Passed = TestCopy() && Passed;
    Passed = TestFinalizeGridding() && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the geometry with one detector of each type used in the test
MString UTDGridPointCollection::GetGeometry() const
{
  MString Setup = GetGeometryHead();

  // All detector volumes are centered at x = 0 and y = 0, the detectors differ in z

  // Two wafers of 10 x 10 cm in holders at z = -20 and z = -10:
  Setup += "Volume Holder\nHolder.Material Vacuum\nHolder.Shape BRIK 6.0 6.0 1.0\n";
  Setup += "Volume Wafer\nWafer.Material Germanium\nWafer.Shape BRIK 5.0 5.0 0.5\nWafer.Position 0.0 0.0 0.0\nWafer.Mother Holder\n";
  Setup += "Holder.Copy HolderA\nHolderA.Position 0.0 0.0 -20.0\nHolderA.Mother World\n";
  Setup += "Holder.Copy HolderB\nHolderB.Position 0.0 0.0 -10.0\nHolderB.Mother World\n";

  const vector<MString> Names = { "CalorimeterV", "Strip3DV", "ScintiV", "AngerXYV", "AngerXYZV", "AngerIndependentV" };
  const vector<MString> Positions = { "-40.0", "0.0", "40.0", "50.0", "60.0", "70.0" };
  for (unsigned int v = 0; v < Names.size(); ++v) {
    Setup += MString("Volume ") + Names[v] + "\n";
    Setup += Names[v] + ".Material Germanium\n";
    Setup += Names[v] + ".Shape BRIK 5.0 5.0 0.5\n";
    Setup += Names[v] + ".Position 0.0 0.0 " + Positions[v] + "\n";
    Setup += Names[v] + ".Mother World\n";
  }

  // A voxel detector of 10 x 10 x 10 cm at z = 20:
  Setup += "Volume VoxelV\nVoxelV.Material Germanium\nVoxelV.Shape BRIK 5.0 5.0 5.0\nVoxelV.Position 0.0 0.0 20.0\nVoxelV.Mother World\n";

  // 9 strips of pitch 1 cm and a guard ring of 0.5 cm:
  Setup += "Strip2D Strips\nStrips.DetectorVolume Wafer\nStrips.SensitiveVolume Wafer\n";
  Setup += "Strips.StructuralPitch 0.0 0.0 0.0\nStrips.StructuralOffset 0.0 0.0 0.0\nStrips.Offset 0.5 0.5\nStrips.StripNumber 9 9\n";
  Setup += "Strips.NoiseThreshold 10\nStrips.TriggerThreshold 30\nStrips.EnergyResolution Ideal\nStrips.FailureRate 0.0\n";
  Setup += "Strips.GuardRingTriggerThreshold 40\nStrips.GuardRingEnergyResolution 100 1\nStrips.GuardRingEnergyResolution 1000 1\n";

  // 10 strips of pitch 1 cm:
  Setup += "Strip3D Strips3D\nStrips3D.DetectorVolume Strip3DV\nStrips3D.SensitiveVolume Strip3DV\n";
  Setup += "Strips3D.StructuralPitch 0.0 0.0 0.0\nStrips3D.StructuralOffset 0.0 0.0 0.0\nStrips3D.Offset 0.0 0.0\nStrips3D.StripNumber 10 10\n";
  Setup += "Strips3D.DepthResolution 100 0.02\nStrips3D.DepthResolution 1000 0.02\n";
  Setup += "Strips3D.NoiseThreshold 10\nStrips3D.TriggerThreshold 30\nStrips3D.EnergyResolution Ideal\nStrips3D.FailureRate 0.0\n";

  // 5 x 5 x 5 voxels of 2 cm:
  Setup += "Voxel3D Voxels\nVoxels.DetectorVolume VoxelV\nVoxels.SensitiveVolume VoxelV\n";
  Setup += "Voxels.Offset 0.0 0.0 0.0\nVoxels.VoxelNumber 5 5 5\n";
  Setup += "Voxels.NoiseThreshold 10\nVoxels.TriggerThreshold 30\nVoxels.EnergyResolution Ideal\nVoxels.FailureRate 0.0\n";

  // One crystal:
  Setup += "Calorimeter Calo\nCalo.DetectorVolume CalorimeterV\nCalo.SensitiveVolume CalorimeterV\n";
  Setup += "Calo.StructuralPitch 0.0 0.0 0.0\nCalo.StructuralOffset 0.0 0.0 0.0\n";
  Setup += "Calo.NoiseThreshold 10\nCalo.TriggerThreshold 30\nCalo.EnergyResolution Ideal\nCalo.FailureRate 0.0\n";

  Setup += "Scintillator Scinti\nScinti.DetectorVolume ScintiV\nScinti.SensitiveVolume ScintiV\n";
  Setup += "Scinti.NoiseThreshold 10\nScinti.TriggerThreshold 30\nScinti.EnergyResolution Ideal\nScinti.FailureRate 0.0\n";

  Setup += "AngerCamera AngerXY\nAngerXY.DetectorVolume AngerXYV\nAngerXY.SensitiveVolume AngerXYV\n";
  Setup += "AngerXY.NoiseThreshold 10\nAngerXY.TriggerThreshold 30\nAngerXY.EnergyResolution Ideal\nAngerXY.FailureRate 0.0\n";
  Setup += "AngerXY.Positioning XY\nAngerXY.PositionResolution 10 0.5\nAngerXY.PositionResolution 1000 0.5\n";

  Setup += "AngerCamera AngerXYZ\nAngerXYZ.DetectorVolume AngerXYZV\nAngerXYZ.SensitiveVolume AngerXYZV\n";
  Setup += "AngerXYZ.NoiseThreshold 10\nAngerXYZ.TriggerThreshold 30\nAngerXYZ.EnergyResolution Ideal\nAngerXYZ.FailureRate 0.0\n";
  Setup += "AngerXYZ.Positioning XYZ\nAngerXYZ.PositionResolution 10 0.5\nAngerXYZ.PositionResolution 1000 0.5\n";

  Setup += "AngerCamera AngerIndependent\nAngerIndependent.DetectorVolume AngerIndependentV\nAngerIndependent.SensitiveVolume AngerIndependentV\n";
  Setup += "AngerIndependent.NoiseThreshold 10\nAngerIndependent.TriggerThreshold 30\nAngerIndependent.EnergyResolution Ideal\nAngerIndependent.FailureRate 0.0\n";
  Setup += "AngerIndependent.Positioning XYZIndependent\nAngerIndependent.PositionResolution 10 0.5 0.5 0.5\nAngerIndependent.PositionResolution 1000 0.5 0.5 0.5\n";

  return Setup;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the volume sequence at the world position
MDVolumeSequence UTDGridPointCollection::GetSequence(const MVector& Position)
{
  return m_Geometry.GetVolumeSequence(Position, true, true);
}


////////////////////////////////////////////////////////////////////////////////


//! Add a hit at a world position to a collection
void UTDGridPointCollection::AddHit(MDGridPointCollection& Collection, const MVector& Position, double Energy, double Time, const vector<int>& Origins)
{
  Collection.Add(GetSequence(Position).GetPositionInDetector(), Energy, Time, Origins);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructor and the initial content
bool UTDGridPointCollection::TestConstruction()
{
  bool Passed = true;

  MDVolumeSequence Sequence = GetSequence(MVector(0.1, 0.1, -20.0));
  MDGridPointCollection Collection(Sequence);

  Passed = EvaluateTrue("GetDetector()", "strips", "The collection knows the detector of the volume sequence", Collection.GetDetector() == m_Geometry.GetDetector("Strips")) && Passed;
  Passed = Evaluate("GetNGridPoints()", "new", "There are no grid points", Collection.GetNGridPoints(), 0U) && Passed;
  Passed = EvaluateNear("GetEnergy()", "new", "The energy is 0", Collection.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = Evaluate("GetHits()", "new", "There are no hits", Collection.GetHits(), 0U) && Passed;
  Passed = EvaluateNear("GetWeightedHits()", "new", "There are no weighted hits", Collection.GetWeightedHits(), 0.0, 0.0) && Passed;

  // The volume tree is the one of the first copy:
  MString Tree = Collection.GetVolumeTree();
  Passed = EvaluateTrue("GetVolumeTree()", "strips", "The tree has the world volume", Tree.Contains("World")) && Passed;
  Passed = EvaluateTrue("GetVolumeTree()", "strips", "The tree has the copy of the holder", Tree.Contains("HolderA")) && Passed;
  Passed = EvaluateTrue("GetVolumeTree()", "strips", "The tree has the wafer", Tree.Contains("Wafer")) && Passed;

  // The center of the detector volume in the world - the volumes are centered at x = 0 and y = 0:
  Passed = EvaluateVectorNear("GetWorldPosition()", "wafer at z = -20", "The position is the center of the wafer", Collection.GetWorldPosition(), MVector(0.0, 0.0, -20.0), 1e-9) && Passed;
  MDGridPointCollection Second(GetSequence(MVector(0.1, 0.1, -10.0)));
  Passed = EvaluateVectorNear("GetWorldPosition()", "wafer at z = -10", "The position is the center of the wafer", Second.GetWorldPosition(), MVector(0.0, 0.0, -10.0), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the identification of the detector
bool UTDGridPointCollection::TestHasSameDetector()
{
  bool Passed = true;

  MDVolumeSequence WaferA1 = GetSequence(MVector(0.1, 0.1, -20.0));
  MDVolumeSequence WaferA2 = GetSequence(MVector(-3.1, 2.1, -20.0));
  MDVolumeSequence WaferB = GetSequence(MVector(0.1, 0.1, -10.0));
  MDVolumeSequence Other = GetSequence(MVector(0.1, 0.1, 40.0));

  MDGridPointCollection A(WaferA1);
  MDGridPointCollection SameA(WaferA2);
  MDGridPointCollection B(WaferB);
  MDGridPointCollection C(Other);

  Passed = EvaluateTrue("HasSameDetector(MDVolumeSequence*)", "same wafer", "Another position in the wafer has the same detector", A.HasSameDetector(&WaferA2)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(MDVolumeSequence*)", "other copy", "The other copy of the wafer is another detector", A.HasSameDetector(&WaferB)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(MDVolumeSequence*)", "other type", "Another detector type is another detector", A.HasSameDetector(&Other)) && Passed;

  Passed = EvaluateTrue("HasSameDetector(MDGridPointCollection&)", "same wafer", "A collection of the same wafer has the same detector", A.HasSameDetector(SameA)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(MDGridPointCollection&)", "other copy", "A collection of the other copy has another detector", A.HasSameDetector(B)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(MDGridPointCollection&)", "other type", "A collection of another type has another detector", A.HasSameDetector(C)) && Passed;
  Passed = EvaluateTrue("HasSameDetector(MDGridPointCollection&)", "itself", "A collection has the same detector as itself", A.HasSameDetector(A)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in strip detectors
bool UTDGridPointCollection::TestStrip2D()
{
  bool Passed = true;

  MDGridPointCollection Collection(GetSequence(MVector(0.1, 0.1, -20.0)));

  // First hit: Strip index = int(x + 4.5) with the active area from -4.5 to 4.5 cm and a pitch of 1 cm: x = 0.1 -> 4
  AddHit(Collection, MVector(0.1, 0.1, -20.0), 100.0, 5.0, { 1, 2 });
  Passed = Evaluate("Add()", "first hit", "There is one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  const MDGridPoint& First = Collection.GetGridPointAt(0);
  Passed = Evaluate("GetGridPointAt()", "first hit", "The x strip is 4", First.GetXGrid(), 4U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "first hit", "The y strip is 4", First.GetYGrid(), 4U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "first hit", "The z grid is 0", First.GetZGrid(), 0U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "first hit", "The type is voxel", First.GetType(), MDGridPoint::c_Voxel) && Passed;
  Passed = EvaluateNear("GetGridPointAt()", "first hit", "The energy is stored", First.GetEnergy(), 100.0, 0.0) && Passed;
  Passed = EvaluateNear("GetGridPointAt()", "first hit", "The time is stored", First.GetTime(), 5.0, 0.0) && Passed;
  Passed = EvaluateTrue("GetGridPointAt()", "first hit", "The origins are stored", First.GetOrigins() == vector<int>({ 1, 2 })) && Passed;
  Passed = Evaluate("GetGridPointAt()", "first hit", "The number of hits is 1", First.GetHits(), 1U) && Passed;

  // The second hit is in the same strip pair at another position, with an earlier time:
  AddHit(Collection, MVector(-0.4, 0.4, -20.0), 50.0, 3.0, { 2, 3 });
  Passed = Evaluate("Add()", "same strips", "Hits in the same strips are one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  const MDGridPoint& Merged = Collection.GetGridPointAt(0);
  Passed = EvaluateNear("Add()", "same strips", "The energies add up", Merged.GetEnergy(), 150.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add()", "same strips", "The earlier time is taken", Merged.GetTime(), 3.0, 0.0) && Passed;
  Passed = EvaluateTrue("Add()", "same strips", "The origins are merged", Merged.GetOrigins() == vector<int>({ 1, 2, 3 })) && Passed;
  Passed = Evaluate("Add()", "same strips", "The hits add up", Merged.GetHits(), 2U) && Passed;

  // A hit in another strip: x = 2.1 -> 6, y = -1.6 -> 2
  AddHit(Collection, MVector(2.1, -1.6, -20.0), 25.0, 7.0, { 4 });
  Passed = Evaluate("Add()", "other strips", "Hits in other strips are another grid point", Collection.GetNGridPoints(), 2U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "other strips", "The x strip is 6", Collection.GetGridPointAt(1).GetXGrid(), 6U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "other strips", "The y strip is 2", Collection.GetGridPointAt(1).GetYGrid(), 2U) && Passed;
  Passed = EvaluateNear("GetGridPointAt()", "other strips", "The first grid point has not changed", Collection.GetGridPointAt(0).GetEnergy(), 150.0, 1e-12) && Passed;

  // Only one coordinate is different:
  AddHit(Collection, MVector(2.1, -0.6, -20.0), 1.0, 1.0, {});
  AddHit(Collection, MVector(3.1, -1.6, -20.0), 1.0, 1.0, {});
  Passed = Evaluate("Add()", "one other strip", "A difference in only one of the strips is another grid point", Collection.GetNGridPoints(), 4U) && Passed;

  // The world position is the center of the strips: x = -4.5 + 6.5 = 2, y = -4.5 + 2.5 = -2
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "strips 4, 4", "The position is the center of the strips", Collection.GetWorldPositionGridPointAt(0), MVector(0.0, 0.0, -20.0), 1e-9) && Passed;
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "strips 6, 2", "The position is the center of the strips", Collection.GetWorldPositionGridPointAt(1), MVector(2.0, -2.0, -20.0), 1e-9) && Passed;

  // The same in the other wafer:
  MDGridPointCollection Other(GetSequence(MVector(2.1, -1.6, -10.0)));
  AddHit(Other, MVector(2.1, -1.6, -10.0), 25.0, 7.0, {});
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "other wafer", "The position is in the other wafer", Other.GetWorldPositionGridPointAt(0), MVector(2.0, -2.0, -10.0), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in the guard ring of a strip detector
bool UTDGridPointCollection::TestGuardRing()
{
  bool Passed = true;

  // |x| > 4.5 cm is outside of the active area of the strips and inside of the wafer:
  MDGridPointCollection Collection(GetSequence(MVector(4.8, 0.1, -20.0)));
  AddHit(Collection, MVector(4.8, 0.1, -20.0), 100.0, 5.0, { 1 });
  Passed = Evaluate("Add()", "guard ring", "There is one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "guard ring", "The type is guard ring", Collection.GetGridPointAt(0).GetType(), MDGridPoint::c_GuardRing) && Passed;
  Passed = EvaluateNear("GetGridPointAt()", "guard ring", "The energy is stored", Collection.GetGridPointAt(0).GetEnergy(), 100.0, 0.0) && Passed;

  // The other side of the ring:
  AddHit(Collection, MVector(-4.8, -4.8, -20.0), 50.0, 5.0, { 2 });
  Passed = Evaluate("Add()", "second guard ring hit", "All guard ring hits are one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = EvaluateNear("Add()", "second guard ring hit", "The energies add up", Collection.GetGridPointAt(0).GetEnergy(), 150.0, 1e-12) && Passed;
  Passed = Evaluate("Add()", "second guard ring hit", "The hits add up", Collection.GetGridPointAt(0).GetHits(), 2U) && Passed;

  // The unique position of the guard ring is 0.5*(-width + offset) in both dimensions: 0.5*(-10 + 0.5) = -4.75
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "guard ring", "The position is the unique guard ring position", Collection.GetWorldPositionGridPointAt(0), MVector(-4.75, -4.75, -20.0), 1e-9) && Passed;

  // The boundary between strips and guard ring, just inside and just outside:
  MDGridPointCollection Inside(GetSequence(MVector(4.499, 0.1, -20.0)));
  AddHit(Inside, MVector(4.499, 0.1, -20.0), 100.0, 5.0, {});
  Passed = Evaluate("Add()", "x = 4.499 cm", "Just inside of the active area is a strip", Inside.GetGridPointAt(0).GetType(), MDGridPoint::c_Voxel) && Passed;
  Passed = Evaluate("Add()", "x = 4.499 cm", "Just inside of the active area is the last strip", Inside.GetGridPointAt(0).GetXGrid(), 8U) && Passed;
  MDGridPointCollection Outside(GetSequence(MVector(4.501, 0.1, -20.0)));
  AddHit(Outside, MVector(4.501, 0.1, -20.0), 100.0, 5.0, {});
  Passed = Evaluate("Add()", "x = 4.501 cm", "Just outside of the active area is the guard ring", Outside.GetGridPointAt(0).GetType(), MDGridPoint::c_GuardRing) && Passed;

  // Strips and guard ring in one collection are separated:
  AddHit(Inside, MVector(4.8, 0.1, -20.0), 50.0, 5.0, {});
  Passed = Evaluate("Add()", "strip and guard ring", "Strip and guard ring hits are different grid points", Inside.GetNGridPoints(), 2U) && Passed;
  Passed = EvaluateNear("GetEnergy()", "strip and guard ring", "The total energy has both", Inside.GetEnergy(), 150.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in 3D strip detectors
bool UTDGridPointCollection::TestStrip3D()
{
  bool Passed = true;

  MDGridPointCollection Collection(GetSequence(MVector(0.1, 0.1, 0.2)));
  AddHit(Collection, MVector(0.1, 0.1, 0.1), 10.0, 1.0, { 1 });
  AddHit(Collection, MVector(0.2, 0.2, 0.3), 30.0, 1.0, { 1 });
  Passed = Evaluate("Add()", "same strips, other depth", "Hits in the same strips are one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = EvaluateNear("Add()", "same strips, other depth", "The energies add up", Collection.GetGridPointAt(0).GetEnergy(), 40.0, 1e-12) && Passed;

  // The strip center is at 0.5 cm for strip 5 of 0..9, and the depth is energy weighted: (10*0.1 + 30*0.3)/40 = 0.25
  Passed = EvaluateNear("Add()", "same strips, other depth", "The depth is energy weighted", Collection.GetGridPointAt(0).GetPosition().Z(), 0.25, 1e-9) && Passed;
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "same strips, other depth", "The position has the strip center and the weighted depth", Collection.GetWorldPositionGridPointAt(0), MVector(0.5, 0.5, 0.25), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in voxel detectors
bool UTDGridPointCollection::TestVoxel3D()
{
  bool Passed = true;

  MDGridPointCollection Collection(GetSequence(MVector(0.1, 0.1, 20.1)));

  // Voxel index = int((x + 5)/2): 0.1 -> 2
  AddHit(Collection, MVector(0.1, 0.1, 20.1), 10.0, 1.0, {});
  Passed = Evaluate("GetGridPointAt()", "central voxel", "The x voxel is 2", Collection.GetGridPointAt(0).GetXGrid(), 2U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "central voxel", "The y voxel is 2", Collection.GetGridPointAt(0).GetYGrid(), 2U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "central voxel", "The z voxel is 2", Collection.GetGridPointAt(0).GetZGrid(), 2U) && Passed;

  // In the same voxel:
  AddHit(Collection, MVector(-0.9, 0.9, 20.9), 20.0, 1.0, {});
  Passed = Evaluate("Add()", "same voxel", "Hits in the same voxel are one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = EvaluateNear("Add()", "same voxel", "The energies add up", Collection.GetGridPointAt(0).GetEnergy(), 30.0, 1e-12) && Passed;

  // Only z is different: 21.1 - 20 = 1.1 -> int(6.1/2) = 3
  AddHit(Collection, MVector(0.1, 0.1, 21.1), 5.0, 1.0, {});
  Passed = Evaluate("Add()", "other z voxel", "Hits in another z voxel are another grid point", Collection.GetNGridPoints(), 2U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "other z voxel", "The z voxel is 3", Collection.GetGridPointAt(1).GetZGrid(), 3U) && Passed;

  // A corner: 3.1 -> int(8.1/2) = 4, -3.1 -> int(1.9/2) = 0, 4.9 -> int(9.9/2) = 4
  AddHit(Collection, MVector(3.1, -3.1, 24.9), 5.0, 1.0, {});
  Passed = Evaluate("Add()", "corner voxel", "A third voxel is a third grid point", Collection.GetNGridPoints(), 3U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "corner voxel", "The x voxel is 4", Collection.GetGridPointAt(2).GetXGrid(), 4U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "corner voxel", "The y voxel is 0", Collection.GetGridPointAt(2).GetYGrid(), 0U) && Passed;
  Passed = Evaluate("GetGridPointAt()", "corner voxel", "The z voxel is 4", Collection.GetGridPointAt(2).GetZGrid(), 4U) && Passed;

  // Voxel center = -5 + (index + 0.5)*2:
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "central voxel", "The position is the voxel center", Collection.GetWorldPositionGridPointAt(0), MVector(0.0, 0.0, 20.0), 1e-9) && Passed;
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "other z voxel", "The position is the voxel center", Collection.GetWorldPositionGridPointAt(1), MVector(0.0, 0.0, 22.0), 1e-9) && Passed;
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "corner voxel", "The position is the voxel center", Collection.GetWorldPositionGridPointAt(2), MVector(4.0, -4.0, 24.0), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in calorimeters
bool UTDGridPointCollection::TestCalorimeter()
{
  bool Passed = true;

  MDGridPointCollection Collection(GetSequence(MVector(0.1, 0.1, -40.0)));
  AddHit(Collection, MVector(0.1, 0.1, -40.0), 10.0, 2.0, { 1 });
  AddHit(Collection, MVector(-3.1, 2.1, -39.8), 30.0, 1.0, { 2 });
  Passed = Evaluate("Add()", "one crystal", "All hits in one crystal are one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = EvaluateNear("Add()", "one crystal", "The energies add up", Collection.GetGridPointAt(0).GetEnergy(), 40.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add()", "one crystal", "The earlier time is taken", Collection.GetGridPointAt(0).GetTime(), 1.0, 0.0) && Passed;
  Passed = EvaluateTrue("Add()", "one crystal", "The origins are merged", Collection.GetGridPointAt(0).GetOrigins() == vector<int>({ 1, 2 })) && Passed;

  // Without depth resolution the position is the center of the crystal:
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "one crystal", "The position is the center of the crystal", Collection.GetWorldPositionGridPointAt(0), MVector(0.0, 0.0, -40.0), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in scintillators
bool UTDGridPointCollection::TestScintillator()
{
  bool Passed = true;

  MDGridPointCollection Collection(GetSequence(MVector(0.1, 0.1, 40.0)));
  AddHit(Collection, MVector(0.1, 0.1, 40.0), 10.0, 2.0, { 1 });
  AddHit(Collection, MVector(-3.1, 2.1, 40.2), 30.0, 1.0, { 2 });
  Passed = Evaluate("Add()", "scintillator", "All hits in a scintillator are one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = EvaluateNear("Add()", "scintillator", "The energies add up", Collection.GetGridPointAt(0).GetEnergy(), 40.0, 1e-12) && Passed;
  Passed = Evaluate("Add()", "scintillator", "The hits add up", Collection.GetGridPointAt(0).GetHits(), 2U) && Passed;
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "scintillator", "The position is the unique position in the center of the volume", Collection.GetWorldPositionGridPointAt(0), MVector(0.0, 0.0, 40.0), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in Anger cameras
bool UTDGridPointCollection::TestAngerCamera()
{
  bool Passed = true;

  MDGridPointCollection XY(GetSequence(MVector(1.0, 2.0, 50.1)));
  AddHit(XY, MVector(1.0, 2.0, 50.1), 10.0, 2.0, { 1 });
  AddHit(XY, MVector(5.0 - 1.0, -2.0, 50.3), 30.0, 1.0, { 2 });
  Passed = Evaluate("Add()", "XY Anger", "All hits are one grid point", XY.GetNGridPoints(), 1U) && Passed;
  Passed = Evaluate("Add()", "XY Anger", "The type is XY Anger", XY.GetGridPointAt(0).GetType(), MDGridPoint::c_XYAnger) && Passed;
  Passed = EvaluateNear("Add()", "XY Anger", "The energies add up", XY.GetGridPointAt(0).GetEnergy(), 40.0, 1e-12) && Passed;
  // Energy weighted x: (10*1 + 30*4)/40 = 3.25, y: (10*2 - 30*2)/40 = -1, no depth information
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "XY Anger", "The position is energy weighted without depth", XY.GetWorldPositionGridPointAt(0), MVector(3.25, -1.0, 50.0), 1e-9) && Passed;

  MDGridPointCollection XYZ(GetSequence(MVector(1.0, 2.0, 60.1)));
  AddHit(XYZ, MVector(1.0, 2.0, 60.1), 10.0, 2.0, { 1 });
  AddHit(XYZ, MVector(4.0, -2.0, 60.3), 30.0, 1.0, { 2 });
  Passed = Evaluate("Add()", "XYZ Anger", "All hits are one grid point", XYZ.GetNGridPoints(), 1U) && Passed;
  Passed = Evaluate("Add()", "XYZ Anger", "The type is XYZ Anger", XYZ.GetGridPointAt(0).GetType(), MDGridPoint::c_XYZAnger) && Passed;
  // Energy weighted z: (10*0.1 + 30*0.3)/40 = 0.25
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "XYZ Anger", "The position is energy weighted in all dimensions", XYZ.GetWorldPositionGridPointAt(0), MVector(3.25, -1.0, 60.25), 1e-9) && Passed;

  // The positioning XYZIndependent is a keyword of the setup file, which is used in the examples:
  MDGridPointCollection Independent(GetSequence(MVector(1.0, 2.0, 70.1)));
  Passed = EvaluateTrue("GetDetector()", "XYZ independent Anger", "The fixture has the detector", Independent.GetDetector() == m_Geometry.GetDetector("AngerIndependent")) && Passed;
  ostringstream Captured;
  streambuf* Original = cerr.rdbuf(Captured.rdbuf());
  DisableDefaultStreams();
  AddHit(Independent, MVector(1.0, 2.0, 70.1), 10.0, 2.0, { 1 });
  AddHit(Independent, MVector(4.0, -2.0, 70.3), 30.0, 1.0, { 2 });
  double IndependentEnergy = Independent.GetEnergy();
  EnableDefaultStreams();
  cerr.rdbuf(Original);
  // Defect GEO-E-27: The type c_XYZIndependentAnger has no branch in the comparison and addition: two hits are "equal", and the energy of the second one is lost
  Passed = ExpectedToFail(EvaluateNear("GetEnergy()", "XYZ independent Anger, 10 keV and 30 keV", "The energy of all hits is kept", IndependentEnergy, 40.0, 1e-12)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits outside of the detector
bool UTDGridPointCollection::TestOutsideDetector()
{
  bool Passed = true;

  MDVolumeSequence Sequence = GetSequence(MVector(0.1, 0.1, -20.0));

  // A position in the detector which is outside of the wafer in negative y:
  const MVector Below(0.0, -7.0, 0.0);
  MDGridPointCollection BelowCollection(Sequence);
  DisableDefaultStreams();
  BelowCollection.Add(Below, 100.0, 1.0, {});
  EnableDefaultStreams();
  Passed = Evaluate("Add()", "y = -7", "A hit outside of the wafer results in no grid point", BelowCollection.GetNGridPoints(), 0U) && Passed;
  Passed = EvaluateNear("GetEnergy()", "y = -7", "A hit outside of the wafer has no energy in the grid", BelowCollection.GetEnergy(), 0.0, 0.0) && Passed;

  MDGridPointCollection BelowUndiscretized(Sequence);
  DisableDefaultStreams();
  BelowUndiscretized.AddUndiscretized(Below);
  EnableDefaultStreams();
  // Defect GEO-E-28: Without discretization a position outside of the detector is stored as grid point of unknown type
  Passed = ExpectedToFail(Evaluate("AddUndiscretized()", "y = -7", "A hit outside of the wafer results in no grid point", BelowUndiscretized.GetNGridPoints(), 0U)) && Passed;

  // A position in the detector which is outside of the wafer in positive x - one wafer fits exactly into this detector volume:
  const MVector Beyond(7.0, 0.0, 0.0);
  MDGridPointCollection BeyondCollection(Sequence);
  DisableDefaultStreams();
  BeyondCollection.Add(Beyond, 100.0, 1.0, {});
  EnableDefaultStreams();
  // Defect GEO-E-29: A wafer which exactly fills the detector volume is counted as 2 wafers, a position outside is taken for the phantom wafer
  Passed = ExpectedToFail(Evaluate("Add()", "x = 7", "A hit outside of the wafer results in no grid point", BeyondCollection.GetNGridPoints(), 0U)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the sums and weights over the grid points
bool UTDGridPointCollection::TestSumsAndWeights()
{
  bool Passed = true;

  MDGridPointCollection Collection(GetSequence(MVector(0.1, 0.1, -20.0)));
  AddHit(Collection, MVector(0.1, 0.1, -20.0), 100.0, 1.0, {});
  AddHit(Collection, MVector(0.2, 0.2, -20.0), 50.0, 1.0, {});
  AddHit(Collection, MVector(2.1, 0.1, -20.0), 25.0, 1.0, {});

  Passed = Evaluate("GetNGridPoints()", "2 strips", "There are two grid points", Collection.GetNGridPoints(), 2U) && Passed;
  Passed = EvaluateNear("GetEnergy()", "2 strips", "The energy is the sum over all points", Collection.GetEnergy(), 175.0, 1e-12) && Passed;
  Passed = Evaluate("GetHits()", "2 strips", "The hits are the sum over all points", Collection.GetHits(), 3U) && Passed;
  Passed = EvaluateNear("GetWeightedHits()", "default weight", "With a weight of 1 the weighted hits are the hits", Collection.GetWeightedHits(), 3.0, 1e-12) && Passed;

  Collection.SetWeight(2.5);
  Passed = EvaluateNear("GetGridPointAt()", "weight 2.5", "All points have the new weight", Collection.GetGridPointAt(1).GetWeight(), 2.5, 0.0) && Passed;
  Passed = EvaluateNear("SetWeight/GetWeightedHits", "weight 2.5", "The weighted hits are 3 * 2.5", Collection.GetWeightedHits(), 7.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergy()", "weight 2.5", "The weight does not scale the energy", Collection.GetEnergy(), 175.0, 1e-12) && Passed;

  // A new hit in an existing point with the default weight 1: the mean weight is (2*2.5 + 1*1)/3 = 2, the other point keeps 2.5
  AddHit(Collection, MVector(0.3, 0.3, -20.0), 1.0, 1.0, {});
  Passed = EvaluateNear("GetGridPointAt()", "new hit after weight", "The weight of the point is the hit weighted mean", Collection.GetGridPointAt(0).GetWeight(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetWeightedHits()", "new hit after weight", "The weighted hits are 3 * 2 + 1 * 2.5", Collection.GetWeightedHits(), 8.5, 1e-12) && Passed;

  Collection.SetWeight(0.0);
  Passed = EvaluateNear("GetWeightedHits()", "weight 0", "With a weight of 0 there are no weighted hits", Collection.GetWeightedHits(), 0.0, 0.0) && Passed;

  // No points:
  MDGridPointCollection Empty(GetSequence(MVector(0.1, 0.1, -20.0)));
  Empty.SetWeight(3.0);
  Passed = EvaluateNear("SetWeight()", "no points", "A weight without points does not create weighted hits", Empty.GetWeightedHits(), 0.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding hits without discretization
bool UTDGridPointCollection::TestAddUndiscretized()
{
  bool Passed = true;

  MDVolumeSequence Sequence = GetSequence(MVector(0.1, 0.1, -20.0));
  MDGridPointCollection Collection(Sequence);

  // x = 0.1 -> 4, y = 0.1 -> 4, and only counted:
  Collection.AddUndiscretized(Sequence.GetPositionInDetector());
  Passed = Evaluate("AddUndiscretized()", "first hit", "There is one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = Evaluate("AddUndiscretized()", "first hit", "The x strip is 4", Collection.GetGridPointAt(0).GetXGrid(), 4U) && Passed;
  Passed = Evaluate("AddUndiscretized()", "first hit", "The y strip is 4", Collection.GetGridPointAt(0).GetYGrid(), 4U) && Passed;
  Passed = EvaluateNear("AddUndiscretized()", "first hit", "The energy is 0", Collection.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = Evaluate("GetHits()", "first hit", "There is one hit", Collection.GetHits(), 1U) && Passed;

  // The same strip:
  Collection.AddUndiscretized(MVector(-0.4, 0.4, 0.2));
  Passed = Evaluate("AddUndiscretized()", "same strips", "Hits in the same strips are one grid point", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = Evaluate("GetHits()", "same strips", "The hits are counted", Collection.GetHits(), 2U) && Passed;

  // Another strip:
  Collection.AddUndiscretized(MVector(2.1, -1.6, 0.0));
  Passed = Evaluate("AddUndiscretized()", "other strips", "Hits in other strips are another grid point", Collection.GetNGridPoints(), 2U) && Passed;
  Passed = Evaluate("AddUndiscretized()", "other strips", "The x strip is 6", Collection.GetGridPointAt(1).GetXGrid(), 6U) && Passed;
  Passed = Evaluate("GetHits()", "other strips", "The hits are counted", Collection.GetHits(), 3U) && Passed;
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "other strips", "The position is the center of the strips", Collection.GetWorldPositionGridPointAt(1), MVector(2.0, -2.0, -20.0), 1e-9) && Passed;

  // In a detector with depth the combined position needs the energy, which does not exist here:
  MDVolumeSequence Sequence3D = GetSequence(MVector(0.1, 0.1, 0.2));
  MDGridPointCollection Collection3D(Sequence3D);
  Collection3D.AddUndiscretized(Sequence3D.GetPositionInDetector());
  Collection3D.AddUndiscretized(Sequence3D.GetPositionInDetector());
  // Defect GEO-E-9: Two points without energy result in a division by 0 and a NaN position
  Passed = ExpectedToFail(EvaluateVectorNear("GetWorldPositionGridPointAt()", "two hits at depth 0.2 without energy", "The position has the common depth of the hits", Collection3D.GetWorldPositionGridPointAt(0), MVector(0.5, 0.5, 0.2), 1e-9)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test merging collections
bool UTDGridPointCollection::TestMerge()
{
  bool Passed = true;

  MDVolumeSequence Sequence = GetSequence(MVector(0.1, 0.1, -20.0));

  MDGridPointCollection A(Sequence);
  AddHit(A, MVector(0.1, 0.1, -20.0), 100.0, 5.0, { 1, 2 });
  AddHit(A, MVector(2.1, 0.1, -20.0), 10.0, 5.0, { 7 });

  MDGridPointCollection B(Sequence);
  AddHit(B, MVector(0.2, 0.2, -20.0), 50.0, 3.0, { 2, 3 });
  AddHit(B, MVector(-2.1, 0.1, -20.0), 20.0, 4.0, { 9 });

  A.Add(B);
  // Common strip 4, 4, and strips 6, 4 and 2, 4:
  Passed = Evaluate("Add(MDGridPointCollection&)", "1 common and 2 different points", "The result has 3 grid points", A.GetNGridPoints(), 3U) && Passed;
  Passed = EvaluateNear("Add(MDGridPointCollection&)", "1 common and 2 different points", "The total energy is the sum", A.GetEnergy(), 180.0, 1e-12) && Passed;
  Passed = Evaluate("Add(MDGridPointCollection&)", "1 common and 2 different points", "The hits are the sum", A.GetHits(), 4U) && Passed;
  Passed = EvaluateNear("Add(MDGridPointCollection&)", "common point", "The energies are combined", A.GetGridPointAt(0).GetEnergy(), 150.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add(MDGridPointCollection&)", "common point", "The earlier time is taken", A.GetGridPointAt(0).GetTime(), 3.0, 0.0) && Passed;
  Passed = EvaluateTrue("Add(MDGridPointCollection&)", "common point", "The origins are merged", A.GetGridPointAt(0).GetOrigins() == vector<int>({ 1, 2, 3 })) && Passed;
  Passed = Evaluate("Add(MDGridPointCollection&)", "new point", "The new point is at the end", A.GetGridPointAt(2).GetXGrid(), 2U) && Passed;
  Passed = EvaluateTrue("Add(MDGridPointCollection&)", "new point", "The new point keeps its origins", A.GetGridPointAt(2).GetOrigins() == vector<int>({ 9 })) && Passed;

  // The source stays:
  Passed = Evaluate("Add(MDGridPointCollection&)", "source", "The added collection keeps its points", B.GetNGridPoints(), 2U) && Passed;
  Passed = EvaluateNear("Add(MDGridPointCollection&)", "source", "The added collection keeps its energy", B.GetEnergy(), 70.0, 1e-12) && Passed;

  // An empty collection on either side:
  MDGridPointCollection Empty(Sequence);
  A.Add(Empty);
  Passed = Evaluate("Add(MDGridPointCollection&)", "empty source", "Nothing changes", A.GetNGridPoints(), 3U) && Passed;
  Empty.Add(B);
  Passed = Evaluate("Add(MDGridPointCollection&)", "empty target", "The target has all points", Empty.GetNGridPoints(), 2U) && Passed;
  Passed = EvaluateNear("Add(MDGridPointCollection&)", "empty target", "The target has all energy", Empty.GetEnergy(), 70.0, 1e-12) && Passed;

  // Other detectors are not added - neither another copy nor another type:
  MDGridPointCollection OtherCopy(GetSequence(MVector(0.1, 0.1, -10.0)));
  AddHit(OtherCopy, MVector(0.1, 0.1, -10.0), 1000.0, 1.0, {});
  MDGridPointCollection OtherType(GetSequence(MVector(0.1, 0.1, 40.0)));
  AddHit(OtherType, MVector(0.1, 0.1, 40.0), 1000.0, 1.0, {});
  B.Add(OtherCopy);
  Passed = Evaluate("Add(MDGridPointCollection&)", "other copy", "A collection of another copy is not added", B.GetNGridPoints(), 2U) && Passed;
  Passed = EvaluateNear("Add(MDGridPointCollection&)", "other copy", "A collection of another copy is not added", B.GetEnergy(), 70.0, 1e-12) && Passed;
  B.Add(OtherType);
  Passed = EvaluateNear("Add(MDGridPointCollection&)", "other type", "A collection of another type is not added", B.GetEnergy(), 70.0, 1e-12) && Passed;

  // Adding a collection to itself doubles it:
  B.Add(B);
  Passed = Evaluate("Add(MDGridPointCollection&)", "itself", "The number of points stays", B.GetNGridPoints(), 2U) && Passed;
  Passed = EvaluateNear("Add(MDGridPointCollection&)", "itself", "The energy doubles", B.GetEnergy(), 140.0, 1e-12) && Passed;
  Passed = Evaluate("Add(MDGridPointCollection&)", "itself", "The hits double", B.GetHits(), 4U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copying collections
bool UTDGridPointCollection::TestCopy()
{
  bool Passed = true;

  MDVolumeSequence Sequence = GetSequence(MVector(0.1, 0.1, -20.0));
  MDGridPointCollection Original(Sequence);
  AddHit(Original, MVector(0.1, 0.1, -20.0), 100.0, 5.0, { 1 });

  MDGridPointCollection Copy(Original);
  Passed = Evaluate("MDGridPointCollection(const MDGridPointCollection&)", "one point", "The copy has the point", Copy.GetNGridPoints(), 1U) && Passed;
  Passed = EvaluateNear("MDGridPointCollection(const MDGridPointCollection&)", "one point", "The copy has the energy", Copy.GetEnergy(), 100.0, 0.0) && Passed;
  Passed = EvaluateTrue("MDGridPointCollection(const MDGridPointCollection&)", "one point", "The copy has the detector", Copy.GetDetector() == Original.GetDetector()) && Passed;
  Passed = EvaluateTrue("MDGridPointCollection(const MDGridPointCollection&)", "one point", "The copy has the same detector volume", Copy.HasSameDetector(Original)) && Passed;
  Passed = EvaluateVectorNear("MDGridPointCollection(const MDGridPointCollection&)", "one point", "The copy knows the world position", Copy.GetWorldPositionGridPointAt(0), MVector(0.0, 0.0, -20.0), 1e-9) && Passed;

  // The copy is independent:
  AddHit(Copy, MVector(2.1, 0.1, -20.0), 50.0, 5.0, {});
  Passed = Evaluate("MDGridPointCollection(const MDGridPointCollection&)", "independence", "A new point in the copy does not change the original", Original.GetNGridPoints(), 1U) && Passed;
  AddHit(Original, MVector(0.2, 0.2, -20.0), 1.0, 5.0, {});
  Passed = EvaluateNear("MDGridPointCollection(const MDGridPointCollection&)", "independence", "A change of the original does not change the copy", Copy.GetGridPointAt(0).GetEnergy(), 100.0, 0.0) && Passed;

  // The assignment, as used during sorting in BackgroundMixer:
  MDGridPointCollection Assigned(GetSequence(MVector(0.1, 0.1, 40.0)));
  Assigned = Original;
  Passed = EvaluateTrue("operator=", "other detector before", "The detector is replaced", Assigned.GetDetector() == Original.GetDetector()) && Passed;
  Passed = EvaluateNear("operator=", "other detector before", "The points are replaced", Assigned.GetEnergy(), Original.GetEnergy(), 0.0) && Passed;
  Passed = EvaluateVectorNear("operator=", "other detector before", "The world position is the new one", Assigned.GetWorldPosition(), MVector(0.0, 0.0, -20.0), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the function which is called after all hits have been added
bool UTDGridPointCollection::TestFinalizeGridding()
{
  bool Passed = true;

  MDGridPointCollection Collection(GetSequence(MVector(0.1, 0.1, -20.0)));
  AddHit(Collection, MVector(0.1, 0.1, -20.0), 100.0, 5.0, { 1 });

  // The call prints a note on the output:
  ostringstream Captured;
  streambuf* Original = cout.rdbuf(Captured.rdbuf());
  DisableDefaultStreams();
  Collection.FinalizeGridding();
  Collection.FinalizeGridding();
  EnableDefaultStreams();
  cout.rdbuf(Original);

  Passed = Evaluate("FinalizeGridding()", "voxel points, twice", "The number of points stays", Collection.GetNGridPoints(), 1U) && Passed;
  Passed = EvaluateNear("FinalizeGridding()", "voxel points, twice", "The energy stays", Collection.GetEnergy(), 100.0, 0.0) && Passed;
  Passed = Evaluate("FinalizeGridding()", "voxel points, twice", "The hits stay", Collection.GetHits(), 1U) && Passed;

  // A new hit is still possible:
  AddHit(Collection, MVector(0.2, 0.2, -20.0), 1.0, 5.0, {});
  Passed = EvaluateNear("FinalizeGridding()", "new hit", "Hits can be added after it", Collection.GetEnergy(), 101.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);

  // Hide the info messages of the geometry manager:
  gErrorIgnoreLevel = kWarning;

  UTDGridPointCollection Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDGridPointCollection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
