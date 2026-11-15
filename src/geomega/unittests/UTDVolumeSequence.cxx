/*
 * UTDVolumeSequence.cxx
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

// POSIX libs:
// Process control has no equivalent in the C++ standard library
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGeometry.h"
#include "MDGridPoint.h"
#include "MDMaterial.h"
#include "MDShapeBRIK.h"
#include "MDStrip2D.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "MRotation.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the volume sequence class MDVolumeSequence
class UTDVolumeSequence : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDVolumeSequence() : UTGeomegaBase("UTDVolumeSequence") {}
  //! Default destructor
  virtual ~UTDVolumeSequence() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Create a box volume - the volumes live until the end of the program
  MDVolume* MakeBox(const MString& Name, double HalfLength, MDVolume* Mother, const MVector& Position);
  //! Return the text of the fixture geometry
  MString GetFixtureGeometry() const;

  //! Test the state of a new and of a reset sequence
  bool TestDefaultsAndReset();
  //! Test adding volumes and positions
  bool TestAddVolumesAndPositions();
  //! Test the setters and getters of the detector data
  bool TestDetectorData();
  //! Test the search for volumes in the sequence
  bool TestHasVolume();
  //! Test copy constructor and assignment
  bool TestCopies();
  //! Test the transformations of positions
  bool TestPositionTransformations();
  //! Test the total rotations
  bool TestRotations();
  //! Test the comparison of detectors
  bool TestHasSameDetector();
  //! Test the named detectors
  bool TestNamedDetectors();
  //! Test the unique position of a detector
  bool TestUniqueWorldPosition();
  //! Test the grid point
  bool TestGetGridPoint();
  //! Test joining two sequences
  bool TestJoin();
  //! Test the text output
  bool TestStrings();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDVolumeSequence::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndReset() && Passed;
  Passed = TestAddVolumesAndPositions() && Passed;
  Passed = TestDetectorData() && Passed;
  Passed = TestHasVolume() && Passed;
  Passed = TestCopies() && Passed;
  Passed = TestPositionTransformations() && Passed;
  Passed = TestRotations() && Passed;
  Passed = TestHasSameDetector() && Passed;
  Passed = TestNamedDetectors() && Passed;
  Passed = TestUniqueWorldPosition() && Passed;
  Passed = TestGetGridPoint() && Passed;
  Passed = TestJoin() && Passed;
  Passed = TestStrings() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Create a box volume - the volumes live until the end of the program
MDVolume* UTDVolumeSequence::MakeBox(const MString& Name, double HalfLength, MDVolume* Mother, const MVector& Position)
{
  MDVolume* Volume = new MDVolume(Name);
  MDShapeBRIK* Shape = new MDShapeBRIK(Name + "Shape");
  Shape->Set(HalfLength, HalfLength, HalfLength);
  Shape->Validate();
  Volume->SetShape(Shape);
  Volume->SetMaterial(MDMaterial::GetDummyMaterial());
  Volume->SetPosition(Position);
  if (Mother != nullptr) {
    Volume->SetMother(Mother);
  }
  return Volume;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the text of the fixture geometry
MString UTDVolumeSequence::GetFixtureGeometry() const
{
  // Two rotated placements of a cell with a rotated wafer, plus a second detector type:
  // A local point (1, 2, 0.5) of the wafer is at (-2, 1, 2.5) in the cell, since the wafer is rotated by 90 deg around z
  // In CellA, rotated by 90 deg around x, this is (-2, -2.5, 1) plus (20, 0, 0)
  // In CellB, rotated by 90 deg around z, this is (-1, -2, 2.5) plus (-20, 0, 0)
  MString Text = GetGeometryHead();
  Text += "Volume Cell\n";
  Text += "Cell.Material Aluminium\n";
  Text += "Cell.Shape BRIK 8.0 8.0 8.0\n";
  Text += "Volume Wafer\n";
  Text += "Wafer.Material Germanium\n";
  Text += "Wafer.Shape BRIK 5.0 5.0 1.0\n";
  Text += "Wafer.Position 0.0 0.0 2.0\n";
  Text += "Wafer.Rotation 0.0 0.0 90.0\n";
  Text += "Wafer.Mother Cell\n";
  Text += "Cell.Copy CellA\n";
  Text += "CellA.Position 20.0 0.0 0.0\n";
  Text += "CellA.Rotation 90.0 0.0 0.0\n";
  Text += "CellA.Mother World\n";
  Text += "Cell.Copy CellB\n";
  Text += "CellB.Position -20.0 0.0 0.0\n";
  Text += "CellB.Rotation 0.0 0.0 90.0\n";
  Text += "CellB.Mother World\n";
  Text += "Volume Other\n";
  Text += "Other.Material Silicon\n";
  Text += "Other.Shape BRIK 2.0 2.0 2.0\n";
  Text += "Other.Position 0.0 50.0 0.0\n";
  Text += "Other.Mother World\n";
  Text += "MDStrip2D Det\n";
  Text += "Det.DetectorVolume Wafer\n";
  Text += "Det.SensitiveVolume Wafer\n";
  Text += "Det.StructuralPitch 0.0 0.0 0.0\n";
  Text += "Det.StructuralOffset 0.0 0.0 0.0\n";
  Text += "Det.Offset 0.0 0.0\n";
  Text += "Det.StripNumber 10 10\n";
  Text += "Det.NoiseThreshold 1.0\n";
  Text += "Det.TriggerThreshold 1.0\n";
  Text += "Det.EnergyResolution Ideal\n";
  Text += "MDStrip2D OtherDet\n";
  Text += "OtherDet.DetectorVolume Other\n";
  Text += "OtherDet.SensitiveVolume Other\n";
  Text += "OtherDet.StructuralPitch 0.0 0.0 0.0\n";
  Text += "OtherDet.StructuralOffset 0.0 0.0 0.0\n";
  Text += "OtherDet.Offset 0.0 0.0\n";
  Text += "OtherDet.StripNumber 4 4\n";
  Text += "OtherDet.NoiseThreshold 1.0\n";
  Text += "OtherDet.TriggerThreshold 1.0\n";
  Text += "OtherDet.EnergyResolution Ideal\n";
  Text += "Trigger T\n";
  Text += "T.Veto false\n";
  Text += "T.TriggerByChannel true\n";
  Text += "T.Detector Det 1\n";
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state of a new and of a reset sequence
bool UTDVolumeSequence::TestDefaultsAndReset()
{
  bool Passed = true;

  MDVolumeSequence VS;
  Passed = EvaluateTrue("IsEmpty()", "new", "A new sequence is empty", VS.IsEmpty()) && Passed;
  Passed = Evaluate("GetNVolumes()", "new", "A new sequence has no volumes", VS.GetNVolumes(), 0U) && Passed;
  Passed = EvaluateTrue("GetDeepestVolume()", "new", "A new sequence has no deepest volume", VS.GetDeepestVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("GetDetector()", "new", "A new sequence has no detector", VS.GetDetector() == nullptr) && Passed;
  Passed = EvaluateTrue("GetDetectorVolume()", "new", "A new sequence has no detector volume", VS.GetDetectorVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("GetSensitiveVolume()", "new", "A new sequence has no sensitive volume", VS.GetSensitiveVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("GetPositionInDetector()", "new", "The position in the detector is not defined", VS.GetPositionInDetector() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("GetPositionInSensitiveVolume()", "new", "The position in the sensitive volume is not defined", VS.GetPositionInSensitiveVolume() == g_VectorNotDefined) && Passed;
  Passed = EvaluateRotationNear("GetRotation()", "new", "The rotation of an empty sequence is the identity", VS.GetRotation(), MRotation(), 1e-15) && Passed;

  MDStrip2D Detector("Detector");
  MDVolume* A = MakeBox("A", 1, nullptr, MVector(0, 0, 0));
  VS.AddVolume(A);
  VS.AddPosition(MVector(1, 2, 3));
  VS.SetDetector(&Detector);
  VS.SetDetectorVolume(A);
  VS.SetSensitiveVolume(A);
  VS.SetPositionInDetector(MVector(4, 5, 6));
  VS.SetPositionInSensitiveVolume(MVector(7, 8, 9));
  VS.Reset();
  Passed = EvaluateTrue("Reset()", "filled", "The volumes are removed", VS.IsEmpty()) && Passed;
  Passed = EvaluateTrue("Reset()", "filled", "The detector is removed", VS.GetDetector() == nullptr) && Passed;
  Passed = EvaluateTrue("Reset()", "filled", "The detector volume is removed", VS.GetDetectorVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("Reset()", "filled", "The sensitive volume is removed", VS.GetSensitiveVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("Reset()", "filled", "The position in the detector is reset", VS.GetPositionInDetector() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("Reset()", "filled", "The position in the sensitive volume is reset", VS.GetPositionInSensitiveVolume() == g_VectorNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding volumes and positions
bool UTDVolumeSequence::TestAddVolumesAndPositions()
{
  bool Passed = true;

  MDVolume* A = MakeBox("A", 1, nullptr, MVector(0, 0, 0));
  MDVolume* B = MakeBox("B", 1, nullptr, MVector(0, 0, 0));
  MDVolume* C = MakeBox("C", 1, nullptr, MVector(0, 0, 0));

  MDVolumeSequence VS;
  VS.AddVolume(B);
  VS.AddPosition(MVector(2, 2, 2));
  VS.AddVolume(C);
  VS.AddPosition(MVector(3, 3, 3));
  VS.AddVolumeFront(A);
  VS.AddPositionFront(MVector(1, 1, 1));

  Passed = EvaluateFalse("IsEmpty()", "filled", "The sequence is not empty", VS.IsEmpty()) && Passed;
  Passed = Evaluate("GetNVolumes()", "three volumes", "Three volumes are stored", VS.GetNVolumes(), 3U) && Passed;
  Passed = EvaluateTrue("AddVolumeFront()", "front", "The volume added to the front is first", VS.GetVolumeAt(0) == A) && Passed;
  Passed = EvaluateTrue("AddVolume()", "first added", "The volume added to the end before is second", VS.GetVolumeAt(1) == B) && Passed;
  Passed = EvaluateTrue("GetVolumeAt()", "index n - 1", "The last volume is returned", VS.GetVolumeAt(2) == C) && Passed;
  Passed = EvaluateTrue("GetDeepestVolume()", "three volumes", "The deepest volume is the last one", VS.GetDeepestVolume() == C) && Passed;
  Passed = EvaluateVectorNear("AddPositionFront()", "front", "The position added to the front is first", VS.GetPositionAt(0), MVector(1, 1, 1), 1e-15) && Passed;
  Passed = EvaluateVectorNear("AddPosition()", "first added", "The position added to the end before is second", VS.GetPositionAt(1), MVector(2, 2, 2), 1e-15) && Passed;
  Passed = EvaluateVectorNear("GetPositionAt()", "index n - 1", "The last position is returned", VS.GetPositionAt(2), MVector(3, 3, 3), 1e-15) && Passed;

  // Out of bounds:
  DisableDefaultStreams();
  MDVolume* OutOfBoundsVolume = VS.GetVolumeAt(3);
  MVector OutOfBoundsPosition = VS.GetPositionAt(3);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetVolumeAt()", "index n", "The first index after the end returns zero", OutOfBoundsVolume == nullptr) && Passed;
  Passed = EvaluateTrue("GetPositionAt()", "index n", "The first index after the end returns g_VectorNotDefined", OutOfBoundsPosition == g_VectorNotDefined) && Passed;

  // A volume without a position: Reset keeps the old memory, thus the result is deterministic
  MDVolumeSequence Mismatch;
  Mismatch.AddVolume(A);
  Mismatch.AddPosition(MVector(1, 2, 3));
  Mismatch.Reset();
  Mismatch.AddVolume(A);
  DisableDefaultStreams();
  MVector Missing = Mismatch.GetPositionAt(0);
  EnableDefaultStreams();
  // Defect GEO-B-16: GetPositionAt checks the index against the number of volumes and reads behind the end of the positions
  Passed = ExpectedToFail(EvaluateTrue("GetPositionAt()", "volume without position", "A missing position returns g_VectorNotDefined", Missing == g_VectorNotDefined)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters of the detector data
bool UTDVolumeSequence::TestDetectorData()
{
  bool Passed = true;

  MDStrip2D Detector("Detector");
  MDVolume* A = MakeBox("A", 1, nullptr, MVector(0, 0, 0));

  MDVolumeSequence VS;
  VS.SetDetector(&Detector);
  Passed = EvaluateTrue("SetDetector()", "detector", "The detector is stored", VS.GetDetector() == &Detector) && Passed;

  DisableDefaultStreams();
  VS.SetDetector(nullptr);
  EnableDefaultStreams();
  Passed = EvaluateTrue("SetDetector()", "zero", "Zero removes the detector", VS.GetDetector() == nullptr) && Passed;

  VS.SetDetectorVolume(A);
  Passed = EvaluateTrue("SetDetectorVolume()", "A", "The detector volume is stored", VS.GetDetectorVolume() == A) && Passed;
  VS.SetSensitiveVolume(A);
  Passed = EvaluateTrue("SetSensitiveVolume()", "A", "The sensitive volume is stored", VS.GetSensitiveVolume() == A) && Passed;

  // The const versions return the same objects
  VS.SetDetector(&Detector);
  const MDVolumeSequence& Const = VS;
  Passed = EvaluateTrue("GetDetector() const", "detector", "The const version returns the detector", Const.GetDetector() == &Detector) && Passed;
  Passed = EvaluateTrue("GetDetectorVolume() const", "A", "The const version returns the detector volume", Const.GetDetectorVolume() == A) && Passed;
  Passed = EvaluateTrue("GetSensitiveVolume() const", "A", "The const version returns the sensitive volume", Const.GetSensitiveVolume() == A) && Passed;
  const MDVolumeSequence Empty;
  Passed = EvaluateTrue("GetDetector() const", "new", "The const version returns no detector for a new sequence", Empty.GetDetector() == nullptr) && Passed;
  Passed = EvaluateTrue("GetDetectorVolume() const", "new", "The const version returns no detector volume for a new sequence", Empty.GetDetectorVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("GetSensitiveVolume() const", "new", "The const version returns no sensitive volume for a new sequence", Empty.GetSensitiveVolume() == nullptr) && Passed;
  VS.SetPositionInDetector(MVector(1.5, 2.5, 3.5));
  Passed = EvaluateVectorNear("SetPositionInDetector()", "value", "The position is stored", VS.GetPositionInDetector(), MVector(1.5, 2.5, 3.5), 1e-15) && Passed;
  VS.SetPositionInSensitiveVolume(MVector(-1.5, -2.5, -3.5));
  Passed = EvaluateVectorNear("SetPositionInSensitiveVolume()", "value", "The position is stored", VS.GetPositionInSensitiveVolume(), MVector(-1.5, -2.5, -3.5), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for volumes in the sequence
bool UTDVolumeSequence::TestHasVolume()
{
  bool Passed = true;

  MDVolume* A = MakeBox("A", 1, nullptr, MVector(0, 0, 0));
  MDVolume* B = MakeBox("B", 1, nullptr, MVector(0, 0, 0));
  MDVolume* OtherA = MakeBox("A", 1, nullptr, MVector(0, 0, 0));

  MDVolumeSequence VS;
  Passed = EvaluateFalse("HasVolume()", "empty, name", "An empty sequence has no volume with a name", VS.HasVolume(MString("A"))) && Passed;
  Passed = EvaluateFalse("HasVolume()", "empty, pointer", "An empty sequence has no volume", VS.HasVolume(A)) && Passed;

  VS.AddVolume(A);
  VS.AddPosition(MVector(0, 0, 0));
  Passed = EvaluateTrue("HasVolume()", "name", "The name is found", VS.HasVolume(MString("A"))) && Passed;
  Passed = EvaluateFalse("HasVolume()", "other name", "Another name is not found", VS.HasVolume(MString("B"))) && Passed;
  Passed = EvaluateTrue("HasVolume()", "pointer", "The pointer is found", VS.HasVolume(A)) && Passed;
  Passed = EvaluateFalse("HasVolume()", "other pointer", "Another pointer is not found", VS.HasVolume(B)) && Passed;
  Passed = EvaluateFalse("HasVolume()", "other pointer with same name", "The pointer search does not use the name", VS.HasVolume(OtherA)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and assignment
bool UTDVolumeSequence::TestCopies()
{
  bool Passed = true;

  MDStrip2D Detector("Detector");
  MDVolume* A = MakeBox("A", 1, nullptr, MVector(0, 0, 0));
  MDVolume* B = MakeBox("B", 1, nullptr, MVector(0, 0, 0));
  A->SetRotation(0.0, 0.0, 90.0);

  MDVolumeSequence Original;
  Original.AddVolume(A);
  Original.AddPosition(MVector(1, 2, 3));
  Original.SetDetector(&Detector);
  Original.SetDetectorVolume(A);
  Original.SetSensitiveVolume(B);
  Original.SetPositionInDetector(MVector(4, 5, 6));
  Original.SetPositionInSensitiveVolume(MVector(7, 8, 9));
  MRotation OriginalRotation = Original.GetRotation();

  MDVolumeSequence Copy(Original);
  MDVolumeSequence Assigned;
  Assigned.AddVolume(B);
  Assigned.AddPosition(MVector(0, 0, 0));
  Assigned = Original;

  for (MDVolumeSequence* VS : { &Copy, &Assigned }) {
    Passed = Evaluate("MDVolumeSequence(), operator=", "filled", "The volumes are copied", VS->GetNVolumes(), 1U) && Passed;
    Passed = EvaluateTrue("MDVolumeSequence(), operator=", "filled", "The volume pointer is kept", VS->GetVolumeAt(0) == A) && Passed;
    Passed = EvaluateVectorNear("MDVolumeSequence(), operator=", "filled", "The positions are copied", VS->GetPositionAt(0), MVector(1, 2, 3), 1e-15) && Passed;
    Passed = EvaluateTrue("MDVolumeSequence(), operator=", "filled", "The detector is copied", VS->GetDetector() == &Detector) && Passed;
    Passed = EvaluateTrue("MDVolumeSequence(), operator=", "filled", "The detector volume is copied", VS->GetDetectorVolume() == A) && Passed;
    Passed = EvaluateTrue("MDVolumeSequence(), operator=", "filled", "The sensitive volume is copied", VS->GetSensitiveVolume() == B) && Passed;
    Passed = EvaluateVectorNear("MDVolumeSequence(), operator=", "filled", "The position in the detector is copied", VS->GetPositionInDetector(), MVector(4, 5, 6), 1e-15) && Passed;
    Passed = EvaluateVectorNear("MDVolumeSequence(), operator=", "filled", "The position in the sensitive volume is copied", VS->GetPositionInSensitiveVolume(), MVector(7, 8, 9), 1e-15) && Passed;
    Passed = EvaluateRotationNear("MDVolumeSequence(), operator=", "filled", "The rotation is copied", VS->GetRotation(), OriginalRotation, 1e-15) && Passed;
  }

  // Independence of the copy:
  Copy.Reset();
  Passed = Evaluate("MDVolumeSequence()", "reset of the copy", "The original is not changed", Original.GetNVolumes(), 1U) && Passed;
  Passed = Evaluate("operator=", "reset of the copy", "The assigned one is not changed", Assigned.GetNVolumes(), 1U) && Passed;

  // Self assignment:
  MDVolumeSequence& Reference = Assigned;
  Assigned = Reference;
  Passed = Evaluate("operator=", "self assignment", "A self assignment keeps the content", Assigned.GetNVolumes(), 1U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the transformations of positions
bool UTDVolumeSequence::TestPositionTransformations()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "fixture", "The geometry can be read", ScanGeometry(Geometry, GetFixtureGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  MDVolume* CellA = Geometry.GetVolume("CellA");
  MDVolume* Wafer = Geometry.GetVolume("Wafer");
  MDVolume* Other = Geometry.GetVolume("Other");

  // See the derivation at the fixture:
  const MVector Local(1, 2, 0.5);
  const MVector InCell(-2, 1, 2.5);
  const MVector InWorld(18, -2.5, 1);

  MDVolumeSequence VS;
  World->GetVolumeSequence(InWorld, &VS);
  Passed = Evaluate("GetVolumeSequence()", "wafer in cell A", "Three volumes are found", VS.GetNVolumes(), 3U) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "wafer in cell A", "The second is cell A", VS.GetVolumeAt(1) == CellA) && Passed;
  Passed = EvaluateVectorNear("GetPositionAt()", "world", "The first position is the world position", VS.GetPositionAt(0), InWorld, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionAt()", "cell", "The second position is the one in the cell", VS.GetPositionAt(1), InCell, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionAt()", "wafer", "The third position is the one in the wafer", VS.GetPositionAt(2), Local, 1e-13) && Passed;

  Passed = EvaluateVectorNear("GetPositionInVolume()", "world", "From the world into the world", VS.GetPositionInVolume(InWorld, World), InWorld, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionInVolume()", "cell", "From the world into the cell", VS.GetPositionInVolume(InWorld, CellA), InCell, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionInVolume()", "wafer", "From the world into the wafer", VS.GetPositionInVolume(InWorld, Wafer), Local, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionInFirstVolume()", "world", "From the world into the world", VS.GetPositionInFirstVolume(InWorld, World), InWorld, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionInFirstVolume()", "cell", "From the cell into the world", VS.GetPositionInFirstVolume(InCell, CellA), InWorld, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionInFirstVolume()", "wafer", "From the wafer into the world", VS.GetPositionInFirstVolume(Local, Wafer), InWorld, 1e-13) && Passed;

  // The inner point does not need to be inside:
  const MVector Far(1000, -2000, 3000);
  Passed = EvaluateVectorNear("GetPositionInVolume()", "far outside", "The round trip does not need a position inside", VS.GetPositionInFirstVolume(VS.GetPositionInVolume(Far, Wafer), Wafer), Far, 1e-10) && Passed;

  // Errors:
  Passed = EvaluateTrue("GetPositionInVolume()", "zero", "A zero volume returns g_VectorNotDefined", VS.GetPositionInVolume(InWorld, nullptr) == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("GetPositionInVolume()", "not in the sequence", "A volume not in the sequence returns g_VectorNotDefined", VS.GetPositionInVolume(InWorld, Other) == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("GetPositionInFirstVolume()", "zero", "A zero volume returns g_VectorNotDefined", VS.GetPositionInFirstVolume(Local, nullptr) == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("GetPositionInFirstVolume()", "not in the sequence", "A volume not in the sequence returns g_VectorNotDefined", VS.GetPositionInFirstVolume(Local, Other) == g_VectorNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the total rotations
bool UTDVolumeSequence::TestRotations()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "fixture", "The geometry can be read", ScanGeometry(Geometry, GetFixtureGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  MDVolume* CellA = Geometry.GetVolume("CellA");
  MDVolume* Wafer = Geometry.GetVolume("Wafer");

  const MVector InWorld(18, -2.5, 1);

  MDVolumeSequence VS;
  World->GetVolumeSequence(InWorld, &VS);

  // First the rotation of a volume in the sequence with one rotation only:
  // The x axis of the world is the x axis in cell A, since the rotation is around x, then it is -y in the wafer rotated by +90 deg around z
  // The difference of two transformed positions gives the same as a cross check
  const MVector Direction(1, 0, 0);
  const MVector ExpectedDirection(0, -1, 0);
  Passed = EvaluateVectorNear("GetPositionInVolume()", "direction", "The difference of two transformed positions is the direction derived by hand", VS.GetPositionInVolume(Direction, Wafer) - VS.GetPositionInVolume(MVector(0, 0, 0), Wafer), ExpectedDirection, 1e-13) && Passed;
  // Defect GEO-B-17: GetRotation multiplies the rotations in the wrong order for nested rotated volumes
  Passed = ExpectedToFail(EvaluateVectorNear("GetRotation()", "nested, not commuting rotations", "The total rotation world to deepest volume transforms the direction", VS.GetRotation()*Direction, ExpectedDirection, 1e-13)) && Passed;
  Passed = EvaluateNear("GetRotation()", "nested", "The determinant is +1 (right handed)", VS.GetRotation().GetDeterminant(), 1.0, 1e-13) && Passed;

  // Asking twice gives the same result
  MRotation First = VS.GetRotation();
  Passed = EvaluateRotationNear("GetRotation()", "second call", "The second call gives the same rotation", VS.GetRotation(), First, 1e-15) && Passed;

  // A sequence with one rotated volume only has no ordering problem
  MDVolume* Rotated = MakeBox("Rotated", 1, nullptr, MVector(0, 0, 0));
  Rotated->SetRotation(0.0, 0.0, 90.0);
  MDVolumeSequence Single;
  Single.AddVolume(Rotated);
  Single.AddPosition(MVector(0, 0, 0));
  Passed = EvaluateRotationNear("GetRotation()", "single volume", "The total rotation is the rotation of the volume", Single.GetRotation(), Rotated->GetRotationMatrix(), 1e-15) && Passed;

  // Adding a volume invalidates the rotation
  MDVolume* Plain = MakeBox("Plain", 1, nullptr, MVector(0, 0, 0));
  Single.AddVolume(Plain);
  Single.AddPosition(MVector(0, 0, 0));
  // Defect GEO-B-18: After the invalidation by adding a volume GetRotation multiplies onto the old result and counts rotations twice
  Passed = ExpectedToFail(EvaluateRotationNear("GetRotation()", "after adding an unrotated volume", "The total rotation is still the one of the rotated volume", Single.GetRotation(), Rotated->GetRotationMatrix(), 1e-15)) && Passed;

  // Rotation of a volume in the first volume:
  Passed = EvaluateRotationNear("GetRotationInFirstVolume()", "world", "The rotation of the world in the world is the identity", VS.GetRotationInFirstVolume(World), MRotation(), 1e-15) && Passed;
  Passed = EvaluateRotationNear("GetRotationInFirstVolume()", "cell", "The first level is the inverse rotation of this level", VS.GetRotationInFirstVolume(CellA), CellA->GetInvRotationMatrix(), 1e-15) && Passed;
  // The x axis of the wafer is y in the cell (rotation around z by 90 deg) and z in the world (rotation around x by 90 deg)
  // Defect GEO-B-19: GetRotationInFirstVolume multiplies the rotations in the wrong order for nested rotated volumes
  Passed = ExpectedToFail(EvaluateVectorNear("GetRotationInFirstVolume()", "nested, not commuting rotations", "The x axis of the wafer points to z in the world", VS.GetRotationInFirstVolume(Wafer)*MVector(1, 0, 0), MVector(0, 0, 1), 1e-13)) && Passed;

  Passed = EvaluateTrue("GetRotationInFirstVolume()", "zero", "A zero volume returns g_RotationNotDefined", VS.GetRotationInFirstVolume(nullptr) == g_RotationNotDefined) && Passed;
  Passed = EvaluateTrue("GetRotationInFirstVolume()", "not in the sequence", "A volume which is not in the sequence returns g_RotationNotDefined", VS.GetRotationInFirstVolume(Plain) == g_RotationNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the comparison of detectors
bool UTDVolumeSequence::TestHasSameDetector()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "fixture", "The geometry can be read", ScanGeometry(Geometry, GetFixtureGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();

  MDVolumeSequence InA1;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &InA1);
  MDVolumeSequence InA2;
  World->GetVolumeSequence(MVector(18.5, -2.5, 2), &InA2);
  MDVolumeSequence InB;
  World->GetVolumeSequence(MVector(-21, -2, 2.5), &InB);
  MDVolumeSequence InOther;
  World->GetVolumeSequence(MVector(0, 50, 0), &InOther);
  MDVolumeSequence InWorld;
  World->GetVolumeSequence(MVector(0, 0, 0), &InWorld);
  MDVolumeSequence InCell;
  World->GetVolumeSequence(MVector(20, 7, 0), &InCell);
  MDVolumeSequence Empty;

  Passed = EvaluateTrue("GetDetector()", "in cell A", "The hit in the wafer in cell A has a detector", InA1.GetDetector() == Geometry.GetDetector("Det")) && Passed;
  Passed = EvaluateTrue("GetDetector()", "second point", "The second hit in the wafer in cell A has a detector", InA2.GetDetector() == Geometry.GetDetector("Det")) && Passed;
  Passed = EvaluateTrue("GetDetector()", "in cell B", "The hit in the wafer in cell B has a detector", InB.GetDetector() == Geometry.GetDetector("Det")) && Passed;
  Passed = EvaluateTrue("GetDetector()", "in the other detector", "The hit in the other detector has the other detector", InOther.GetDetector() == Geometry.GetDetector("OtherDet")) && Passed;
  Passed = EvaluateTrue("GetDetector()", "cell only", "A hit in the cell beside the wafer has no detector", InCell.GetDetector() == nullptr) && Passed;

  // Reference overload:
  Passed = EvaluateTrue("HasSameDetector(&)", "same clone", "Two hits in the same placement have the same detector", InA1.HasSameDetector(InA2)) && Passed;
  Passed = EvaluateTrue("HasSameDetector(&)", "itself", "A hit has the same detector as itself", InA1.HasSameDetector(InA1)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(&)", "other clone", "Hits in two placements do not have the same detector", InA1.HasSameDetector(InB)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(&)", "other clone, reverse", "The comparison is symmetric", InB.HasSameDetector(InA1)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(&)", "other detector", "Hits in two detector types do not have the same detector", InA1.HasSameDetector(InOther)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(&)", "no detector", "A hit without a detector is not the same", InA1.HasSameDetector(InWorld)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(&)", "this has no detector", "A hit without a detector is not the same", InWorld.HasSameDetector(InA1)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(&)", "both have no detector", "Two hits without a detector are not the same", InWorld.HasSameDetector(InCell)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(&)", "empty", "An empty sequence is not the same", InA1.HasSameDetector(Empty)) && Passed;

  // Pointer overload:
  Passed = EvaluateTrue("HasSameDetector(*)", "same clone", "Two hits in the same placement have the same detector", InA1.HasSameDetector(&InA2)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(*)", "other clone", "Hits in two placements do not have the same detector", InA1.HasSameDetector(&InB)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(*)", "other detector", "Hits in two detector types do not have the same detector", InA1.HasSameDetector(&InOther)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(*)", "no detector", "A hit without a detector is not the same", InA1.HasSameDetector(&InWorld)) && Passed;
  Passed = EvaluateFalse("HasSameDetector(*)", "this has no detector", "A hit without a detector is not the same", InWorld.HasSameDetector(&InA1)) && Passed;

  // No detector volume set: Not the same detector
  MDVolumeSequence NoVolumeA(InA1);
  NoVolumeA.SetDetectorVolume(nullptr);
  Passed = EvaluateFalse("HasSameDetector(&)", "no detector volume", "Without a detector volume the detector cannot be identified", NoVolumeA.HasSameDetector(InA2)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the named detectors
bool UTDVolumeSequence::TestNamedDetectors()
{
  bool Passed = true;

  MString Text = GetFixtureGeometry();
  Text += "Det.Named NamedA\n";
  Text += "NamedA.Assign 18.0 -2.5 1.0\n";
  Text += "Det.Named NamedB\n";
  Text += "NamedB.Assign -21.0 -2.0 2.5\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "named detectors", "The geometry can be read", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();

  MDVolumeSequence InA;
  World->GetVolumeSequence(MVector(18.5, -2.5, 2), &InA);
  MDVolumeSequence InB;
  World->GetVolumeSequence(MVector(-21, -2, 2.5), &InB);

  Passed = EvaluateTrue("GetDetector()", "in the first placement", "The named detector of the first placement is returned", InA.GetDetector() != nullptr && InA.GetDetector()->GetName() == "NamedA") && Passed;
  Passed = EvaluateTrue("GetDetector()", "in the second placement", "The named detector of the second placement is returned", InB.GetDetector() != nullptr && InB.GetDetector()->GetName() == "NamedB") && Passed;
  Passed = EvaluateFalse("HasSameDetector(&)", "two named detectors", "Two named detectors are different", InA.HasSameDetector(InB)) && Passed;
  const MDVolumeSequence& ConstInA = InA;
  Passed = EvaluateTrue("GetDetector() const", "in the first placement", "The const version returns the named detector too", ConstInA.GetDetector() == InA.GetDetector()) && Passed;

  // Setting a named detector directly:
  MDVolumeSequence Direct;
  Direct.SetDetector(Geometry.GetDetector("NamedB"));
  Passed = EvaluateTrue("SetDetector()", "named", "A named detector is kept", Direct.GetDetector() == Geometry.GetDetector("NamedB")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position of a detector
bool UTDVolumeSequence::TestUniqueWorldPosition()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "fixture", "The geometry can be read", ScanGeometry(Geometry, GetFixtureGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();

  MDVolumeSequence InA;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &InA);
  MDVolumeSequence InB;
  World->GetVolumeSequence(MVector(-21, -2, 2.5), &InB);
  MDVolumeSequence InWorld;
  World->GetVolumeSequence(MVector(0, 0, 0), &InWorld);

  // The unique position of a box is its center. The wafer is at (0, 0, 2) in the cell
  // In A this is (20, 0, 0) + 2 * rotation of z around x by 90 deg, thus (20, -2, 0)
  // In B the rotation is around z, thus (-20, 0, 2)
  Passed = EvaluateVectorNear("GetUniqueWorldPositionOfDetector()", "first placement", "The center of the wafer in the first placement", InA.GetUniqueWorldPositionOfDetector(), MVector(20, -2, 0), 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetUniqueWorldPositionOfDetector()", "second placement", "The center of the wafer in the second placement", InB.GetUniqueWorldPositionOfDetector(), MVector(-20, 0, 2), 1e-13) && Passed;
  Passed = EvaluateTrue("GetUniqueWorldPositionOfDetector()", "no detector", "A sequence without a detector returns g_VectorNotDefined", InWorld.GetUniqueWorldPositionOfDetector() == g_VectorNotDefined) && Passed;

  MDVolumeSequence Empty;
  Passed = EvaluateTrue("GetUniqueWorldPositionOfDetector()", "empty", "An empty sequence returns g_VectorNotDefined", Empty.GetUniqueWorldPositionOfDetector() == g_VectorNotDefined) && Passed;

  // A detector volume which is not in the sequence
  MDVolumeSequence NotInside(InA);
  NotInside.SetDetectorVolume(Geometry.GetVolume("Other"));
  Passed = EvaluateTrue("GetUniqueWorldPositionOfDetector()", "detector volume not in the sequence", "The result is g_VectorNotDefined", NotInside.GetUniqueWorldPositionOfDetector() == g_VectorNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the grid point
bool UTDVolumeSequence::TestGetGridPoint()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "fixture", "The geometry can be read", ScanGeometry(Geometry, GetFixtureGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolumeSequence VS;
  Geometry.GetWorldVolume()->GetVolumeSequence(MVector(18, -2.5, 1), &VS);

  // The local position (1, 2, 0.5) in a wafer of half length 5 with 10 strips of width 1 has the strips 6 and 7
  MDGridPoint Point = VS.GetGridPoint();
  Passed = Evaluate("GetGridPoint()", "local (1, 2, 0.5)", "The x strip is number 6", Point.GetXGrid(), 6U) && Passed;
  Passed = Evaluate("GetGridPoint()", "local (1, 2, 0.5)", "The y strip is number 7", Point.GetYGrid(), 7U) && Passed;
  Passed = EvaluateTrue("GetGridPoint()", "local (1, 2, 0.5)", "The type is known", Point.GetType() != MDGridPoint::c_Unknown) && Passed;

  MDVolumeSequence Empty;
  MDGridPoint NoPoint = Empty.GetGridPoint();
  Passed = Evaluate("GetGridPoint()", "no detector", "Without a detector the type is unknown", NoPoint.GetType(), MDGridPoint::c_Unknown) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test joining two sequences
bool UTDVolumeSequence::TestJoin()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "fixture", "The geometry can be read", ScanGeometry(Geometry, GetFixtureGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  MDVolume* Wafer = Geometry.GetVolume("Wafer");

  // Two hits in the same wafer: Everything is averaged
  // The positions of a wafer point (1, 2, 0.5) and (1, 2, -0.5) differ by the rotated z axis of cell A:
  MDVolumeSequence First;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &First);
  MDVolumeSequence Second;
  World->GetVolumeSequence(MVector(18, -1.5, 1), &Second);
  First.Join(Second);
  Passed = Evaluate("Join()", "same volumes", "All volumes are kept", First.GetNVolumes(), 3U) && Passed;
  Passed = EvaluateVectorNear("Join()", "same volumes", "The world positions are averaged", First.GetPositionAt(0), MVector(18, -2.0, 1), 1e-13) && Passed;
  Passed = EvaluateVectorNear("Join()", "same volumes", "The positions in the wafer are averaged", First.GetPositionAt(2), MVector(1, 2, 0.0), 1e-13) && Passed;
  Passed = EvaluateVectorNear("Join()", "same volumes", "The positions in the detector are averaged", First.GetPositionInDetector(), MVector(1, 2, 0.0), 1e-13) && Passed;
  Passed = EvaluateVectorNear("Join()", "same volumes", "The positions in the sensitive volume are averaged", First.GetPositionInSensitiveVolume(), MVector(1, 2, 0.0), 1e-13) && Passed;
  Passed = EvaluateTrue("Join()", "same volumes", "The detector is kept", First.GetDetector() == Geometry.GetDetector("Det")) && Passed;
  Passed = EvaluateTrue("Join()", "same volumes", "The sensitive volume is kept", First.GetSensitiveVolume() == Wafer) && Passed;

  // Hits in the first and second placement: Only the world is left
  MDVolumeSequence InA;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &InA);
  MDVolumeSequence InB;
  World->GetVolumeSequence(MVector(-21, -2, 2.5), &InB);
  InA.Join(InB);
  Passed = Evaluate("Join()", "different placements", "The sequence is cut at the first difference", InA.GetNVolumes(), 1U) && Passed;
  Passed = EvaluateTrue("Join()", "different placements", "The world is kept", InA.GetVolumeAt(0) == World) && Passed;
  Passed = EvaluateVectorNear("Join()", "different placements", "The world position is averaged", InA.GetPositionAt(0), MVector(-1.5, -2.25, 1.75), 1e-13) && Passed;

  // Different detector types: The detector data is removed
  MDVolumeSequence InA2;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &InA2);
  MDVolumeSequence InOther;
  World->GetVolumeSequence(MVector(0, 50, 0), &InOther);
  InA2.Join(InOther);
  Passed = EvaluateTrue("Join()", "different detectors", "No detector is left", InA2.GetDetector() == nullptr) && Passed;
  Passed = EvaluateTrue("Join()", "different detectors", "No detector volume is left", InA2.GetDetectorVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("Join()", "different detectors", "No sensitive volume is left", InA2.GetSensitiveVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("Join()", "different detectors", "The position in the detector is not defined", InA2.GetPositionInDetector() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("Join()", "different detectors", "The position in the sensitive volume is not defined", InA2.GetPositionInSensitiveVolume() == g_VectorNotDefined) && Passed;

  // A sequence with no detector:
  MDVolumeSequence InA3;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &InA3);
  MDVolumeSequence InWorld;
  World->GetVolumeSequence(MVector(0, 0, 0), &InWorld);
  InA3.Join(InWorld);
  // Defect GEO-B-20: Join does not cut the sequence to the common part if the other sequence is shorter
  Passed = ExpectedToFail(Evaluate("Join()", "with world only", "Only the world is common", InA3.GetNVolumes(), 1U)) && Passed;
  Passed = EvaluateTrue("Join()", "with world only", "No detector is left", InA3.GetDetector() == nullptr) && Passed;

  // Joining with a shorter sequence whose volumes are all in common:
  MDVolumeSequence Deep;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &Deep);
  MDVolumeSequence ShorterCopy;
  World->GetVolumeSequence(MVector(20, 7, 0), &ShorterCopy);
  Passed = Evaluate("GetVolumeSequence()", "in the cell", "The shorter sequence has two volumes", ShorterCopy.GetNVolumes(), 2U) && Passed;
  Deep.Join(ShorterCopy);
  // Defect GEO-B-20: Join does not cut the sequence to the common part if the other sequence is shorter
  Passed = ExpectedToFail(Evaluate("Join()", "deep with shorter", "Only the common volumes stay", Deep.GetNVolumes(), 2U)) && Passed;

  // The reverse direction always has the common part only:
  MDVolumeSequence Shorter;
  World->GetVolumeSequence(MVector(20, 7, 0), &Shorter);
  MDVolumeSequence DeepCopy;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &DeepCopy);
  Shorter.Join(DeepCopy);
  Passed = Evaluate("Join()", "shorter with deep", "Only the common volumes stay", Shorter.GetNVolumes(), 2U) && Passed;

  // An empty sequence stays empty:
  MDVolumeSequence EmptyOne;
  MDVolumeSequence Other;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &Other);
  EmptyOne.Join(Other);
  Passed = EvaluateTrue("Join()", "empty with filled", "An empty sequence stays empty", EmptyOne.IsEmpty()) && Passed;

  // Joining invalidates the rotation: Only the world with no rotation is left
  MDVolumeSequence Rotated;
  World->GetVolumeSequence(MVector(18, -2.5, 1), &Rotated);
  Rotated.GetRotation();
  MDVolumeSequence RotatedOther;
  World->GetVolumeSequence(MVector(-21, -2, 2.5), &RotatedOther);
  Rotated.Join(RotatedOther);
  Passed = EvaluateRotationNear("Join()", "rotation", "The rotation is calculated from the remaining volumes", Rotated.GetRotation(), MRotation(), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTDVolumeSequence::TestStrings()
{
  bool Passed = true;

  MDVolumeSequence Empty;
  Passed = EvaluateTrue("ToStringVolumes()", "empty", "An empty sequence is reported", Empty.ToStringVolumes() == "No volumes defined") && Passed;
  MString EmptyText = Empty.ToString();
  Passed = EvaluateTrue("ToString()", "empty", "Missing volumes are reported", EmptyText.Contains("Volumes: none defined!")) && Passed;
  Passed = EvaluateTrue("ToString()", "empty", "A missing detector is reported", EmptyText.Contains("Detector: none defined!")) && Passed;
  Passed = EvaluateTrue("ToString()", "empty", "A missing sensitive volume is reported", EmptyText.Contains("Sensitive volume: none defined!")) && Passed;
  Passed = EvaluateTrue("ToString()", "empty", "A missing detector volume is reported", EmptyText.Contains("Detector volume: none defined!")) && Passed;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "fixture", "The geometry can be read", ScanGeometry(Geometry, GetFixtureGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolumeSequence VS;
  Geometry.GetWorldVolume()->GetVolumeSequence(MVector(18, -2.5, 1), &VS);

  Passed = EvaluateTrue("ToStringVolumes()", "three volumes", "The volumes are listed with arrows", VS.ToStringVolumes() == "World --> CellA --> Wafer") && Passed;
  MString Text = VS.ToString();
  Passed = EvaluateTrue("ToString()", "filled", "The volumes are listed", Text.Contains("World --> CellA --> Wafer")) && Passed;
  Passed = EvaluateTrue("ToString()", "filled", "The material is listed", Text.Contains("Last volumes material: Germanium")) && Passed;
  Passed = EvaluateTrue("ToString()", "filled", "The detector is listed", Text.Contains("Detector: Det")) && Passed;
  Passed = EvaluateTrue("ToString()", "filled", "The sensitive volume is listed", Text.Contains("Sensitive volume: Wafer")) && Passed;
  Passed = EvaluateTrue("ToString()", "filled", "The detector volume is listed", Text.Contains("Detector volume: Wafer")) && Passed;

  // A volume which is not complete:
  MDVolume Plain("Plain");
  MDVolumeSequence Incomplete;
  Incomplete.AddVolume(&Plain);
  Incomplete.AddPosition(MVector(0, 0, 0));
  // Defect GEO-B-21: ToString dereferences the material and the shape of the last volume without a check
  Passed = ExpectedToCrash(EvaluateFalse("ToString()", "volume without material", "The text is created for a volume without a material", Incomplete.ToString().IsEmpty())) && Passed;

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

  UTDVolumeSequence Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDVolumeSequence.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
