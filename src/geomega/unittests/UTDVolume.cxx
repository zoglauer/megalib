/*
 * UTDVolume.cxx
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
#include <limits>
#include <map>
#include <new>
#include <vector>

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
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TROOT.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGeometry.h"
#include "MDMaterial.h"
#include "MDShapeBRIK.h"
#include "MDStrip2D.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "MRotation.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected clone function of the volume
class UTDVolumeAccess : public MDVolume
{
public:
  //! Standard constructor with the name of the volume
  UTDVolumeAccess(const MString& Name) : MDVolume(Name) {}
  using MDVolume::Clone;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the volume class MDVolume
class UTDVolume : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDVolume() : UTGeomegaBase("UTDVolume") {}
  //! Default destructor
  virtual ~UTDVolume() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Create a validated box volume - the volumes live until the end of the program
  MDVolume* MakeBox(const MString& Name, double X, double Y, double Z, MDVolume* Mother, const MVector& Position);
  //! Fill a box volume with shape, material, position, and mother
  void SetUpBox(MDVolume* Volume, double X, double Y, double Z, MDVolume* Mother, const MVector& Position);
  //! Return the text of a geometry with a rotated holder, a rotated detector, and a passive plug in the detector
  MString GetNestedGeometry() const;
  //! Return the text of a geometry with two placements of a template with a daughter
  MString GetCloneGeometry() const;

  //! Test the default values of a new volume and the plain setters and getters
  bool TestDefaultsAndAccessors();
  //! Test the volume IDs
  bool TestIDs();
  //! Test mother and daughter handling
  bool TestMotherDaughter();
  //! Test the three kinds of rotations and their conventions
  bool TestRotations();
  //! Test the transformations from a volume to its mother and the world
  bool TestTransformations();
  //! Test the position inside checks at the boundaries
  bool TestInsideChecks();
  //! Test the search of a volume by position
  bool TestGetVolume();
  //! Test the search of a volume by name
  bool TestContainsVolume();
  //! Test the detector related flags
  bool TestDetectorFlags();
  //! Test the clone administration
  bool TestCloneAdministration();
  //! Test the clone function
  bool TestClone();
  //! Test copying the data of a template to its clones
  bool TestCopyDataToClones();
  //! Test the validation
  bool TestValidate();
  //! Test the check that all clones have the same mother
  bool TestValidateClonesHaveSameMotherVolume();
  //! Test removing virtual volumes
  bool TestRemoveVirtualVolumes();
  //! Test removing virtual volumes which are clones in a read geometry
  bool TestRemoveVirtualClonesInGeometry();
  //! Test virtualizing volumes without a detector
  bool TestVirtualizeNonDetectorVolumes();
  //! Test sorting the volume tree
  bool TestOptimizeVolumeTree();
  //! Test scaling
  bool TestScale();
  //! Test the volume sequence creation
  bool TestGetVolumeSequence();
  //! Test the noising of positions
  bool TestNoise();
  //! Test the pulse shape interface
  bool TestApplyPulseShape();
  //! Test the path lengths
  bool TestGetAbsorptionLengths();
  //! Test the masses
  bool TestGetMasses();
  //! Test the visible and sensitive volume counts
  bool TestCounts();
  //! Test the overlap search at one position
  bool TestFindOverlaps();
  //! Test the placements and the random positions
  bool TestPlacementsAndRandomPositions();
  //! Test the center locations
  bool TestGetCenterLocations();
  //! Test the ROOT geometry against the geomega geometry
  bool TestCreateRootGeometry();
  //! Test the text output
  bool TestStrings();
  //! Test that the text output can be read back
  bool TestGetGeomegaRoundTrip();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDVolume::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndAccessors() && Passed;
  Passed = TestIDs() && Passed;
  Passed = TestMotherDaughter() && Passed;
  Passed = TestRotations() && Passed;
  Passed = TestTransformations() && Passed;
  Passed = TestInsideChecks() && Passed;
  Passed = TestGetVolume() && Passed;
  Passed = TestContainsVolume() && Passed;
  Passed = TestDetectorFlags() && Passed;
  Passed = TestCloneAdministration() && Passed;
  Passed = TestClone() && Passed;
  Passed = TestCopyDataToClones() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestValidateClonesHaveSameMotherVolume() && Passed;
  Passed = TestRemoveVirtualVolumes() && Passed;
  Passed = TestRemoveVirtualClonesInGeometry() && Passed;
  Passed = TestVirtualizeNonDetectorVolumes() && Passed;
  Passed = TestOptimizeVolumeTree() && Passed;
  Passed = TestScale() && Passed;
  Passed = TestGetVolumeSequence() && Passed;
  Passed = TestNoise() && Passed;
  Passed = TestApplyPulseShape() && Passed;
  Passed = TestGetAbsorptionLengths() && Passed;
  Passed = TestGetMasses() && Passed;
  Passed = TestCounts() && Passed;
  Passed = TestFindOverlaps() && Passed;
  Passed = TestPlacementsAndRandomPositions() && Passed;
  Passed = TestGetCenterLocations() && Passed;
  Passed = TestCreateRootGeometry() && Passed;
  Passed = TestStrings() && Passed;
  Passed = TestGetGeomegaRoundTrip() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill a box volume with shape, material, position, and mother
void UTDVolume::SetUpBox(MDVolume* Volume, double X, double Y, double Z, MDVolume* Mother, const MVector& Position)
{
  MDShapeBRIK* Shape = new MDShapeBRIK(Volume->GetName() + "Shape");
  Shape->Set(X, Y, Z);
  Shape->Validate();
  Volume->SetShape(Shape);
  Volume->SetMaterial(MDMaterial::GetDummyMaterial());
  Volume->SetPosition(Position);
  if (Mother != nullptr) {
    Volume->SetMother(Mother);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Create a validated box volume - the volumes live until the end of the program
MDVolume* UTDVolume::MakeBox(const MString& Name, double X, double Y, double Z, MDVolume* Mother, const MVector& Position)
{
  MDVolume* Volume = new MDVolume(Name);
  SetUpBox(Volume, X, Y, Z, Mother, Position);
  return Volume;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the text of a geometry with a rotated holder, a rotated detector, and a passive plug in the detector
MString UTDVolume::GetNestedGeometry() const
{
  MString Text = GetGeometryHead();
  Text += "Volume Holder\n";
  Text += "Holder.Material Aluminium\n";
  Text += "Holder.Shape BRIK 20.0 20.0 20.0\n";
  Text += "Holder.Position 10.0 -5.0 20.0\n";
  Text += "Holder.Rotation 30.0 40.0 50.0\n";
  Text += "Holder.Mother World\n";
  Text += "Volume Wafer\n";
  Text += "Wafer.Material Germanium\n";
  Text += "Wafer.Shape BRIK 5.0 5.0 1.0\n";
  Text += "Wafer.Position 1.0 2.0 3.0\n";
  Text += "Wafer.Rotation 0.0 0.0 90.0\n";
  Text += "Wafer.Mother Holder\n";
  Text += "Volume Plug\n";
  Text += "Plug.Material Aluminium\n";
  Text += "Plug.Shape BRIK 1.0 1.0 0.5\n";
  Text += "Plug.Position 3.0 3.0 0.0\n";
  Text += "Plug.Mother Wafer\n";
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
  Text += "Trigger T\n";
  Text += "T.Veto false\n";
  Text += "T.TriggerByChannel true\n";
  Text += "T.Detector Det 1\n";
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the text of a geometry with two placements of a template with a daughter
MString UTDVolume::GetCloneGeometry() const
{
  MString Text = GetGeometryHead();
  Text += "Volume Cell\n";
  Text += "Cell.Material Aluminium\n";
  Text += "Cell.Shape BRIK 4.0 4.0 4.0\n";
  Text += "Volume Core\n";
  Text += "Core.Material Germanium\n";
  Text += "Core.Shape BRIK 1.0 1.0 1.0\n";
  Text += "Core.Position 2.0 0.0 0.0\n";
  Text += "Core.Mother Cell\n";
  Text += "Cell.Copy CellA\n";
  Text += "CellA.Position 20.0 0.0 0.0\n";
  Text += "CellA.Mother World\n";
  Text += "Cell.Copy CellB\n";
  Text += "CellB.Position -20.0 0.0 0.0\n";
  Text += "CellB.Rotation 0.0 0.0 90.0\n";
  Text += "CellB.Mother World\n";
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default values of a new volume and the plain setters and getters
bool UTDVolume::TestDefaultsAndAccessors()
{
  bool Passed = true;

  MDVolume V("Name");
  Passed = EvaluateTrue("GetName()", "constructor argument", "The name of the constructor is stored", V.GetName() == "Name") && Passed;
  Passed = EvaluateFalse("IsWorldVolume()", "new", "A new volume is not the world volume", V.IsWorldVolume()) && Passed;
  Passed = EvaluateFalse("IsVirtual()", "new", "A new volume is not virtual", V.IsVirtual()) && Passed;
  Passed = EvaluateFalse("IsMany()", "new", "A new volume has no many flag", V.IsMany()) && Passed;
  Passed = EvaluateTrue("DoAbsorptions()", "new", "A new volume is used for absorptions", V.DoAbsorptions()) && Passed;
  Passed = EvaluateTrue("GetMother()", "new", "A new volume has no mother", V.GetMother() == nullptr) && Passed;
  Passed = EvaluateTrue("GetShape()", "new", "A new volume has no shape", V.GetShape() == nullptr) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "new", "A new volume has no material", V.GetMaterial() == nullptr) && Passed;
  Passed = EvaluateTrue("GetPosition()", "new", "The position of a new volume is not defined", V.GetPosition() == g_VectorNotDefined) && Passed;
  Passed = EvaluateFalse("IsRotated()", "new", "A new volume is not rotated", V.IsRotated()) && Passed;
  Passed = EvaluateRotationNear("GetRotationMatrix()", "new", "The rotation of a new volume is the identity", V.GetRotationMatrix(), MRotation(), 1e-15) && Passed;
  Passed = EvaluateRotationNear("GetInvRotationMatrix()", "new", "The inverse rotation of a new volume is the identity", V.GetInvRotationMatrix(), MRotation(), 1e-15) && Passed;
  Passed = Evaluate("GetRotationID()", "new", "A new volume has the rotation ID 0", V.GetRotationID(), 0) && Passed;
  Passed = Evaluate("GetVisibility()", "new", "The visibility of a new volume is not defined", V.GetVisibility(), g_IntNotDefined) && Passed;
  Passed = Evaluate("GetColor()", "new", "The color of a new volume is not defined", V.GetColor(), g_IntNotDefined) && Passed;
  Passed = Evaluate("GetLineWidth()", "new", "The line width of a new volume is not defined", V.GetLineWidth(), g_IntNotDefined) && Passed;
  Passed = Evaluate("GetLineStyle()", "new", "The line style of a new volume is not defined", V.GetLineStyle(), g_IntNotDefined) && Passed;
  Passed = EvaluateTrue("GetDetector()", "new", "A new volume has no detector", V.GetDetector() == nullptr) && Passed;
  Passed = EvaluateTrue("GetDetectorVolume()", "new", "A new volume has no detector volume", V.GetDetectorVolume() == nullptr) && Passed;
  Passed = EvaluateFalse("IsDetectorVolume()", "new", "A new volume is no detector volume", V.IsDetectorVolume()) && Passed;
  Passed = EvaluateFalse("IsSensitive()", "new", "A new volume is not sensitive", V.IsSensitive()) && Passed;
  Passed = Evaluate("GetSensitiveVolumeID()", "new", "A new volume has the sensitive volume ID 0", V.GetSensitiveVolumeID(), 0) && Passed;
  Passed = Evaluate("GetNDaughters()", "new", "A new volume has no daughters", V.GetNDaughters(), 0U) && Passed;
  Passed = Evaluate("GetNClones()", "new", "A new volume has no clones", V.GetNClones(), 0U) && Passed;
  Passed = EvaluateFalse("IsClone()", "new", "A new volume is no clone", V.IsClone()) && Passed;
  Passed = EvaluateFalse("IsCloneTemplate()", "new", "A new volume is no clone template", V.IsCloneTemplate()) && Passed;
  Passed = EvaluateTrue("GetCloneTemplate()", "new", "A new volume has no clone template", V.GetCloneTemplate() == nullptr) && Passed;
  Passed = EvaluateFalse("IsCloneTemplateVolumeWritten()", "new", "The volume written flag starts as false", V.IsCloneTemplateVolumeWritten()) && Passed;
  Passed = EvaluateFalse("AreCloneTemplateDaughtersWritten()", "new", "The daughters written flag starts as false", V.AreCloneTemplateDaughtersWritten()) && Passed;

  MDVolume NoName;
  Passed = EvaluateTrue("MDVolume()", "no name", "The default name is empty", NoName.GetName() == "") && Passed;

  V.SetWorldVolume();
  Passed = EvaluateTrue("SetWorldVolume()", "set", "The volume is the world volume", V.IsWorldVolume()) && Passed;
  V.SetVirtual(true);
  Passed = EvaluateTrue("SetVirtual()", "true", "The volume is virtual", V.IsVirtual()) && Passed;
  V.SetVirtual(false);
  Passed = EvaluateFalse("SetVirtual()", "false", "The volume is not virtual again", V.IsVirtual()) && Passed;
  V.SetMany(true);
  Passed = EvaluateTrue("SetMany()", "true", "The many flag is set", V.IsMany()) && Passed;
  V.SetAbsorptions(false);
  Passed = EvaluateFalse("SetAbsorptions()", "false", "The absorption flag is reset", V.DoAbsorptions()) && Passed;

  V.SetVisibility(2);
  Passed = Evaluate("SetVisibility()", "2", "The visibility is stored", V.GetVisibility(), 2) && Passed;
  V.SetColor(7);
  Passed = Evaluate("SetColor()", "7", "The color is stored", V.GetColor(), 7) && Passed;
  V.SetLineWidth(3);
  Passed = Evaluate("SetLineWidth()", "3", "The line width is stored", V.GetLineWidth(), 3) && Passed;
  V.SetLineStyle(4);
  Passed = Evaluate("SetLineStyle()", "4", "The line style is stored", V.GetLineStyle(), 4) && Passed;
  V.SetPosition(MVector(1.5, -2.5, 3.5));
  Passed = EvaluateVectorNear("SetPosition()", "interior value", "The position is stored", V.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-15) && Passed;

  // The color is handed to the shape in both orders:
  MDShapeBRIK* Shape = new MDShapeBRIK("Shape");
  Shape->Set(1.0, 2.0, 3.0);
  Shape->Validate();
  V.SetShape(Shape);
  Passed = EvaluateTrue("SetShape()", "box", "The shape is stored", V.GetShape() == Shape) && Passed;
  Passed = EvaluateVectorNear("GetSize()", "box", "The size is the half length of the box", V.GetSize(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
  V.SetMaterial(MDMaterial::GetDummyMaterial());
  Passed = EvaluateTrue("SetMaterial()", "dummy", "The material is stored", V.GetMaterial() == MDMaterial::GetDummyMaterial()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume IDs
bool UTDVolume::TestIDs()
{
  bool Passed = true;

  const int OldCounter = MDVolume::m_IDCounter;

  MDVolume::ResetIDs();
  Passed = Evaluate("ResetIDs()", "reset", "The ID counter restarts at 1", MDVolume::m_IDCounter, 1) && Passed;
  MDVolume First("First");
  MDVolume Second("Second");
  Passed = Evaluate("GetID()", "first after reset", "The first volume after a reset has ID 1", First.GetID(), 1) && Passed;
  Passed = Evaluate("GetID()", "second after reset", "The second volume after a reset has ID 2", Second.GetID(), 2) && Passed;

  // Fill the memory with a pattern to see if the constructor sets the ID
  MDVolume::m_IDCounter = g_IntNotDefined;
  alignas(MDVolume) unsigned char Block[sizeof(MDVolume)];
  for (unsigned int b = 0; b < sizeof(MDVolume); ++b) {
    Block[b] = 0x5A;
  }
  MDVolume* Placed = new (Block) MDVolume("Placed");
  const int PatternID = 0x5A5A5A5A;
  // Defect GEO-B-1: The constructor leaves the ID uninitialized if the ID counter is g_IntNotDefined
  Passed = ExpectedToFail(EvaluateTrue("MDVolume()", "counter is g_IntNotDefined", "The constructor initializes the ID also when the counter wraps", Placed->GetID() != PatternID)) && Passed;
  Passed = Evaluate("MDVolume()", "counter is g_IntNotDefined", "The counter restarts at 0", MDVolume::m_IDCounter, 0) && Passed;
  Placed->~MDVolume();

  MDVolume::m_IDCounter = OldCounter;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test mother and daughter handling
bool UTDVolume::TestMotherDaughter()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 50, 50, 50, nullptr, MVector(0, 0, 0));
  MDVolume* A = MakeBox("A", 10, 10, 10, World, MVector(0, 0, 0));
  MDVolume* B = MakeBox("B", 5, 5, 5, A, MVector(0, 0, 0));
  MDVolume* C = MakeBox("C", 1, 1, 1, B, MVector(0, 0, 0));

  Passed = EvaluateTrue("SetMother()", "A in World", "The mother is stored", A->GetMother() == World) && Passed;
  Passed = Evaluate("SetMother()", "A in World", "The volume is a daughter of the mother", World->GetNDaughters(), 1U) && Passed;
  Passed = EvaluateTrue("GetDaughterAt()", "index 0", "The daughter is returned", World->GetDaughterAt(0) == A) && Passed;

  Passed = EvaluateTrue("HasMother()", "direct", "The direct mother is found", C->HasMother(B)) && Passed;
  Passed = EvaluateTrue("HasMother()", "two up", "The mother of the mother is found", C->HasMother(A)) && Passed;
  Passed = EvaluateTrue("HasMother()", "top", "The top volume is found", C->HasMother(World)) && Passed;
  Passed = EvaluateFalse("HasMother()", "itself", "A volume is not its own mother", C->HasMother(C)) && Passed;
  Passed = EvaluateFalse("HasMother()", "daughter", "A daughter is not a mother", B->HasMother(C)) && Passed;
  Passed = EvaluateFalse("HasMother()", "top volume", "The top volume has no mothers", World->HasMother(A)) && Passed;

  // Moving a volume updates both mothers:
  MDVolume* D = MakeBox("D", 1, 1, 1, nullptr, MVector(0, 0, 0));
  Passed = EvaluateTrue("SetMother()", "first mother", "The first mother is accepted", D->SetMother(A)) && Passed;
  Passed = Evaluate("SetMother()", "first mother", "A has two daughters", A->GetNDaughters(), 2U) && Passed;
  Passed = EvaluateTrue("SetMother()", "second mother", "The second mother is accepted", D->SetMother(B)) && Passed;
  Passed = Evaluate("SetMother()", "second mother", "D is removed from the old mother", A->GetNDaughters(), 1U) && Passed;
  Passed = Evaluate("SetMother()", "second mother", "D is added to the new mother", B->GetNDaughters(), 2U) && Passed;
  Passed = EvaluateTrue("SetMother()", "zero", "A zero mother is accepted", D->SetMother(nullptr)) && Passed;
  Passed = EvaluateTrue("SetMother()", "zero", "The volume has no mother anymore", D->GetMother() == nullptr) && Passed;
  Passed = Evaluate("SetMother()", "zero", "D is removed from its mother", B->GetNDaughters(), 1U) && Passed;

  // A loop is an error, and the header demands that the mother is not changed:
  DisableDefaultStreams();
  bool LoopResult = A->SetMother(C);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetMother()", "loop", "A daughter cannot become the mother", LoopResult) && Passed;
  Passed = EvaluateTrue("SetMother()", "loop", "The mother pointer stays", A->GetMother() == World) && Passed;
  // Defect GEO-B-2: A refused SetMother removes the volume from the daughters of its old mother
  Passed = ExpectedToFail(Evaluate("SetMother()", "loop", "The old mother still has its daughter after the refusal", World->GetNDaughters(), 1U)) && Passed;

  // A volume as its own mother would result in endless recursions in HasMother and the transformations:
  MDVolume* Self = MakeBox("Self", 1, 1, 1, nullptr, MVector(0, 0, 0));
  DisableDefaultStreams();
  bool SelfResult = Self->SetMother(Self);
  EnableDefaultStreams();
  // Defect GEO-B-3: SetMother accepts the volume itself as mother
  Passed = ExpectedToFail(EvaluateFalse("SetMother()", "itself", "A volume cannot be its own mother", SelfResult)) && Passed;
  Self->RemoveAllDaughters();

  // Duplicates are ignored
  MDVolume* M = MakeBox("M", 10, 10, 10, nullptr, MVector(0, 0, 0));
  MDVolume* E = MakeBox("E", 1, 1, 1, nullptr, MVector(0, 0, 0));
  M->AddDaughter(E);
  M->AddDaughter(E);
  Passed = Evaluate("AddDaughter()", "duplicate", "A daughter is only added once", M->GetNDaughters(), 1U) && Passed;

  // Removal:
  MDVolume* Outsider = MakeBox("Outsider", 1, 1, 1, nullptr, MVector(0, 0, 0));
  Passed = EvaluateTrue("RemoveDaughter()", "not a daughter", "The pointer is returned", M->RemoveDaughter(Outsider) == Outsider) && Passed;
  Passed = Evaluate("RemoveDaughter()", "not a daughter", "The daughters are not changed", M->GetNDaughters(), 1U) && Passed;
  Passed = EvaluateTrue("RemoveDaughter()", "daughter", "The pointer is returned", M->RemoveDaughter(E) == E) && Passed;
  Passed = Evaluate("RemoveDaughter()", "daughter", "The daughter is removed", M->GetNDaughters(), 0U) && Passed;

  MDVolume* F = MakeBox("F", 1, 1, 1, M, MVector(0, 0, 0));
  M->RemoveDaughter(F);
  // Defect GEO-B-4: RemoveDaughter does not reset the mother of the daughter as its description says
  Passed = ExpectedToFail(EvaluateTrue("RemoveDaughter()", "daughter with mother", "The removed daughter has no mother anymore", F->GetMother() == nullptr)) && Passed;

  MDVolume* G = MakeBox("G", 1, 1, 1, M, MVector(0, 0, 0));
  MDVolume* H = MakeBox("H", 1, 1, 1, M, MVector(0, 0, 0));
  Passed = EvaluateTrue("GetDaughterAt()", "index n - 1", "The last daughter is returned", M->GetDaughterAt(M->GetNDaughters() - 1) == H) && Passed;
  Passed = EvaluateTrue("GetDaughterAt()", "index 0", "The first daughter is returned", M->GetDaughterAt(0) == G) && Passed;
  // Defect GEO-B-42: An index behind the end stops the program with a ROOT Fatal
  Passed = ExpectedToCrash(EvaluateTrue("GetDaughterAt()", "index n", "The first index after the end returns no volume, and the program goes on", M->GetDaughterAt(2) == nullptr)) && Passed;
  M->RemoveAllDaughters();
  Passed = Evaluate("RemoveAllDaughters()", "two daughters", "No daughters are left", M->GetNDaughters(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the three kinds of rotations and their conventions
bool UTDVolume::TestRotations()
{
  bool Passed = true;

  const MVector X(1, 0, 0);
  const MVector Y(0, 1, 0);
  const MVector Z(0, 0, 1);

  // 90 deg counterclockwise around z: The x axis of the volume is the y axis of the mother
  MDVolume Rz("Rz");
  Rz.SetRotation(0.0, 0.0, 90.0);
  Passed = EvaluateTrue("SetRotation()", "x, y, z", "The volume is rotated", Rz.IsRotated()) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around z", "The x axis of the volume is the y axis of the mother", Rz.GetInvRotationMatrix()*X, Y, 1e-14) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around z", "The y axis of the volume is the negative x axis of the mother", Rz.GetInvRotationMatrix()*Y, -1.0*X, 1e-14) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around z", "The inverse maps the y axis of the mother onto the x axis of the volume", Rz.GetRotationMatrix()*Y, X, 1e-14) && Passed;

  MDVolume Rx("Rx");
  Rx.SetRotation(90.0, 0.0, 0.0);
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around x", "The y axis of the volume is the z axis of the mother", Rx.GetInvRotationMatrix()*Y, Z, 1e-14) && Passed;
  MDVolume Ry("Ry");
  Ry.SetRotation(0.0, 90.0, 0.0);
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around y", "The z axis of the volume is the x axis of the mother", Ry.GetInvRotationMatrix()*Z, X, 1e-14) && Passed;

  // Interior angles: The local to mother rotation is Rz*Ry*Rx
  MDVolume Mixed("Mixed");
  Mixed.SetRotation(30.0, 40.0, 50.0);
  const MVector Test(0.3, -1.7, 2.9);
  Passed = EvaluateVectorNear("SetRotation()", "30, 40, 50 deg", "The rotation is Rz*Ry*Rx, derived by hand", Mixed.GetInvRotationMatrix()*Test, RotateActive(Test, 30.0, 40.0, 50.0), 1e-13) && Passed;

  // Invariants: Inverse, length, and handedness
  MRotation Product = Mixed.GetRotationMatrix()*Mixed.GetInvRotationMatrix();
  Passed = EvaluateRotationNear("GetInvRotationMatrix()", "30, 40, 50 deg", "The product of the rotation and its inverse is the identity", Product, MRotation(), 1e-14) && Passed;
  Passed = EvaluateNear("GetRotationMatrix()", "30, 40, 50 deg", "The determinant is +1 (right handed)", Mixed.GetRotationMatrix().GetDeterminant(), 1.0, 1e-14) && Passed;
  Passed = EvaluateNear("GetRotationMatrix()", "30, 40, 50 deg", "The length is preserved", (Mixed.GetRotationMatrix()*Test).Mag(), Test.Mag(), 1e-14) && Passed;
  Passed = EvaluateRotationNear("GetInverseRotationMatrix()", "30, 40, 50 deg", "The second name of the inverse returns the same", Mixed.GetInverseRotationMatrix(), Mixed.GetInvRotationMatrix(), 0.0) && Passed;

  // Rotation IDs: Each call gets a new one
  MDVolume RotA("RotA");
  MDVolume RotB("RotB");
  RotA.SetRotation(1.0, 2.0, 3.0);
  int FirstID = RotA.GetRotationID();
  RotA.SetRotation(1.0, 2.0, 3.0);
  Passed = Evaluate("GetRotationID()", "second rotation", "Each rotation gets a new ID", RotA.GetRotationID(), FirstID + 1) && Passed;
  RotB.SetRotation(MRotation());
  Passed = Evaluate("GetRotationID()", "other volume", "The IDs are counted over all volumes", RotB.GetRotationID(), FirstID + 2) && Passed;

  // The rotation with ID keeps the ID and the same matrix convention as the rotation without ID:
  MDVolume WithoutID("WithoutID");
  MDVolume WithID("WithID");
  WithoutID.SetRotation(Mixed.GetRotationMatrix());
  WithID.SetRotation(Mixed.GetRotationMatrix(), 4711);
  Passed = Evaluate("SetRotation()", "with ID", "The ID is stored", WithID.GetRotationID(), 4711) && Passed;
  Passed = EvaluateTrue("SetRotation()", "with ID", "The volume is rotated", WithID.IsRotated()) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "with ID", "The rotation is stored", WithID.GetRotationMatrix(), Mixed.GetRotationMatrix(), 1e-14) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "with ID", "The inverse rotation is stored", WithID.GetInvRotationMatrix(), Mixed.GetInvRotationMatrix(), 1e-14) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "rotation matrix", "The rotation is stored", WithoutID.GetRotationMatrix(), Mixed.GetRotationMatrix(), 1e-14) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "rotation matrix", "The inverse rotation is stored", WithoutID.GetInvRotationMatrix(), Mixed.GetInvRotationMatrix(), 1e-14) && Passed;

  // Geant3 angles: The direction of the new axes in the mother coordinate system
  MDVolume Euler("Euler");
  Passed = EvaluateTrue("SetRotation()", "new x = +y, new y = -x, new z = z", "The angles of a rotation are accepted", Euler.SetRotation(90.0, 90.0, 90.0, 180.0, 0.0, 0.0)) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "Geant3 angles", "The x axis of the volume is the y axis of the mother", Euler.GetInvRotationMatrix()*X, Y, 1e-14) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "Geant3 angles", "The y axis of the volume is the negative x axis of the mother", Euler.GetInvRotationMatrix()*Y, -1.0*X, 1e-14) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "Geant3 angles", "The result is the same as 90 deg around z", Euler.GetRotationMatrix(), Rz.GetRotationMatrix(), 1e-14) && Passed;

  // Other orderings of the smallest scalar product use the other branches of the handedness check
  MDVolume Reflected("Reflected");
  DisableDefaultStreams();
  bool ReflectedResult = Reflected.SetRotation(90.0, 0.0, 90.0, 90.0, 180.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetRotation()", "reflection", "A reflection (new z = -z) is refused", ReflectedResult) && Passed;
  Passed = EvaluateFalse("SetRotation()", "reflection", "A refused rotation does not change the volume", Reflected.IsRotated()) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "reflection", "A refused rotation does not change the matrix", Reflected.GetRotationMatrix(), MRotation(), 1e-15) && Passed;

  MDVolume NotOrthogonal("NotOrthogonal");
  DisableDefaultStreams();
  bool NotOrthogonalResult = NotOrthogonal.SetRotation(90.0, 0.0, 90.0, 80.0, 0.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetRotation()", "10 deg from orthogonal", "Axes which are not orthogonal are refused", NotOrthogonalResult) && Passed;
  Passed = EvaluateFalse("SetRotation()", "10 deg from orthogonal", "A refused rotation does not change the volume", NotOrthogonal.IsRotated()) && Passed;

  // The threshold of the check: cos < 1e-6 corresponds to 5.73e-5 deg from orthogonal
  MDVolume Inside("Inside");
  DisableDefaultStreams();
  bool InsideResult = Inside.SetRotation(90.0, 0.0, 90.0, 90.0 + 5.0E-5, 0.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("SetRotation()", "5.0e-5 deg from orthogonal", "Just inside the threshold of cos = 1e-6 the rotation is accepted", InsideResult) && Passed;
  Passed = EvaluateNear("SetRotation()", "5.0e-5 deg from orthogonal", "The result is close to a rotation", Inside.GetRotationMatrix().GetDeterminant(), 1.0, 2.0e-6) && Passed;
  MDVolume Outside("Outside");
  DisableDefaultStreams();
  bool OutsideResult = Outside.SetRotation(90.0, 0.0, 90.0, 90.0 + 7.0E-5, 0.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetRotation()", "7.0e-5 deg from orthogonal", "Just outside the threshold of cos = 1e-6 the rotation is refused", OutsideResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the transformations from a volume to its mother and the world
bool UTDVolume::TestTransformations()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  World->SetWorldVolume();
  MDVolume* Outer = MakeBox("Outer", 40, 40, 40, World, MVector(10, -5, 20));
  Outer->SetRotation(30.0, 40.0, 50.0);
  MDVolume* Inner = MakeBox("Inner", 10, 10, 10, Outer, MVector(1, 2, 3));
  Inner->SetRotation(-20.0, 15.0, 70.0);

  const MVector Local(0.25, -0.75, 1.5);

  // Independent derivation with the hand-made rotations:
  const MVector InOuter = MVector(1, 2, 3) + RotateActive(Local, -20.0, 15.0, 70.0);
  const MVector InWorld = MVector(10, -5, 20) + RotateActive(InOuter, 30.0, 40.0, 50.0);

  Passed = EvaluateVectorNear("GetPositionInMotherVolume()", "rotated and offset", "The position is rotated and then translated", Inner->GetPositionInMotherVolume(Local), InOuter, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionInWorldVolume()", "two levels", "The position runs through both levels", Inner->GetPositionInWorldVolume(Local), InWorld, 1e-13) && Passed;
  Passed = EvaluateVectorNear("GetPositionInWorldVolume()", "world volume", "The position in the world volume is unchanged", World->GetPositionInWorldVolume(Local), Local, 1e-15) && Passed;

  // Not rotated volume:
  MDVolume* Plain = MakeBox("Plain", 1, 1, 1, World, MVector(7, 8, 9));
  Passed = EvaluateVectorNear("GetPositionInMotherVolume()", "not rotated", "The position is only translated", Plain->GetPositionInMotherVolume(Local), Local + MVector(7, 8, 9), 1e-14) && Passed;

  // Round trip with the forward direction which the search uses:
  Passed = EvaluateTrue("GetVolume()", "round trip", "The world position leads back to the inner volume", World->GetVolume(InWorld) == Inner) && Passed;

  MDVolumeSequence VS;
  World->GetVolumeSequence(InWorld, &VS);
  Passed = EvaluateVectorNear("GetVolumeSequence()", "round trip", "The position in the deepest volume is the original local position", VS.GetPositionAt(2), Local, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetVolumeSequence()", "round trip", "The position in the outer volume is the one derived by hand", VS.GetPositionAt(1), InOuter, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position inside checks at the boundaries
bool UTDVolume::TestInsideChecks()
{
  bool Passed = true;

  MDVolume* Mother = MakeBox("Mother", 10, 10, 10, nullptr, MVector(0, 0, 0));
  MDVolume* V = MakeBox("V", 5, 3, 2, Mother, MVector(10, 0, 0));
  V->SetRotation(0.0, 0.0, 90.0);
  MDVolume* D = MakeBox("D", 1, 1, 1, V, MVector(3, 0, 0));

  // The long axis of the rotated box runs along y in the mother volume:
  Passed = EvaluateTrue("IsMotherPositionInside()", "center", "The center is inside", V->IsMotherPositionInside(MVector(10, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsMotherPositionInside()", "along the rotated long axis", "The point 4.5 along y is inside", V->IsMotherPositionInside(MVector(10, 4.5, 0))) && Passed;
  Passed = EvaluateFalse("IsMotherPositionInside()", "along the rotated short axis", "The point 4.5 along x is outside", V->IsMotherPositionInside(MVector(14.5, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsMotherPositionInside()", "just inside", "A point 1e-9 inside the surface is inside", V->IsMotherPositionInside(MVector(10, 5.0 - 1e-9, 0))) && Passed;
  Passed = EvaluateFalse("IsMotherPositionInside()", "just outside", "A point 1e-9 outside the surface is outside", V->IsMotherPositionInside(MVector(10, 5.0 + 1e-9, 0))) && Passed;
  Passed = EvaluateTrue("IsMotherPositionInside()", "within tolerance", "A point 0.5e-3 outside with tolerance 1e-3 counts as inside", V->IsMotherPositionInside(MVector(10, 5.0 + 0.5e-3, 0), 1e-3)) && Passed;
  Passed = EvaluateFalse("IsMotherPositionInside()", "outside tolerance", "A point 2e-3 outside with tolerance 1e-3 is outside", V->IsMotherPositionInside(MVector(10, 5.0 + 2e-3, 0), 1e-3)) && Passed;

  // The daughter is at (3, 0, 0) in the volume, thus at (0, 3, 0) + (10, 0, 0) in the mother:
  Passed = EvaluateFalse("IsMotherPositionExclusivelyInside()", "in the daughter", "A point in the daughter is not exclusive", V->IsMotherPositionExclusivelyInside(MVector(10, 3, 0))) && Passed;
  Passed = EvaluateTrue("IsMotherPositionExclusivelyInside()", "beside the daughter", "A point at the mirror position is exclusive", V->IsMotherPositionExclusivelyInside(MVector(10, -3, 0))) && Passed;
  Passed = EvaluateFalse("IsMotherPositionExclusivelyInside()", "outside", "A point outside is not exclusive", V->IsMotherPositionExclusivelyInside(MVector(20, 0, 0))) && Passed;

  Passed = EvaluateTrue("IsInternalPositionInside()", "local point", "The local long axis is x", V->IsInternalPositionInside(MVector(4.5, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInternalPositionInside()", "local point", "The local short axis is y", V->IsInternalPositionInside(MVector(0, 4.5, 0))) && Passed;
  Passed = EvaluateTrue("IsInternalPositionInside()", "tolerance", "A local point within the tolerance is inside", V->IsInternalPositionInside(MVector(5.0005, 0, 0), 1e-3)) && Passed;

  Passed = EvaluateFalse("IsInternalPositionExclusivelyInside()", "in the daughter", "A local point in the daughter is not exclusive", V->IsInternalPositionExclusivelyInside(MVector(3, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInternalPositionExclusivelyInside()", "beside the daughter", "A local point beside the daughter is exclusive", V->IsInternalPositionExclusivelyInside(MVector(-3, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInternalPositionExclusivelyInside()", "outside", "A local point outside is not exclusive", V->IsInternalPositionExclusivelyInside(MVector(6, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInternalPositionExclusivelyInside()", "daughter", "The daughter itself has a local point", D->IsInternalPositionExclusivelyInside(MVector(0, 0, 0))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search of a volume by position
bool UTDVolume::TestGetVolume()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  MDVolume* A = MakeBox("A", 10, 10, 10, World, MVector(50, 0, 0));
  MDVolume* B = MakeBox("B", 2, 4, 6, A, MVector(0, 0, 3));
  B->SetRotation(0.0, 90.0, 0.0);

  Passed = EvaluateTrue("GetVolume()", "only in the world", "The world volume is returned", World->GetVolume(MVector(0, 0, 0)) == World) && Passed;
  Passed = EvaluateTrue("GetVolume()", "in the daughter", "The daughter is returned", World->GetVolume(MVector(45, 0, -5)) == A) && Passed;
  Passed = EvaluateTrue("GetVolume()", "outside of everything", "Zero is returned", World->GetVolume(MVector(101, 0, 0)) == nullptr) && Passed;

  // B has half lengths (2, 4, 6) and is rotated by 90 deg around y: In A it spans x = +-6, y = +-4, z = 3 +- 2
  Passed = EvaluateTrue("GetVolume()", "deepest", "The deepest volume is returned", World->GetVolume(MVector(55, 0, 4)) == B) && Passed;
  Passed = EvaluateTrue("GetVolume()", "rotated long axis", "The rotated long axis runs along x", World->GetVolume(MVector(55.9, 3.9, 4.9)) == B) && Passed;
  Passed = EvaluateTrue("GetVolume()", "beside the rotated axis", "Along z the volume ends after 2", World->GetVolume(MVector(50, 0, 5.1)) == A) && Passed;
  Passed = EvaluateTrue("GetVolume()", "just inside A", "1e-9 inside A is in A", World->GetVolume(MVector(60.0 - 1e-9, 9, 0)) == A) && Passed;
  Passed = EvaluateTrue("GetVolume()", "just outside A", "1e-9 outside A is in the world", World->GetVolume(MVector(60.0 + 1e-9, 9, 0)) == World) && Passed;

  // Relative position: Already in the coordinates of this volume
  Passed = EvaluateTrue("GetVolume()", "relative", "A relative position does not get the offset of the volume", A->GetVolume(MVector(0, 0, 4), true) == B) && Passed;
  Passed = EvaluateTrue("GetVolume()", "not relative", "A non relative position gets the offset of the volume", A->GetVolume(MVector(50, 0, 4), false) == B) && Passed;
  Passed = EvaluateTrue("GetVolume()", "not relative", "The same point is outside as relative position", A->GetVolume(MVector(50, 0, 4), true) == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search of a volume by name
bool UTDVolume::TestContainsVolume()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  MDVolume* Template = MakeBox("Template", 1, 1, 1, nullptr, MVector(0, 0, 0));
  MDVolume* Clone = new MDVolume("Clone");
  Template->AddClone(Clone);
  Clone->SetMother(World);

  Passed = EvaluateTrue("ContainsVolume()", "itself", "A volume contains itself", World->ContainsVolume("World")) && Passed;
  Passed = EvaluateTrue("ContainsVolume()", "daughter", "A volume contains its daughters", World->ContainsVolume("Clone")) && Passed;
  Passed = EvaluateFalse("ContainsVolume()", "other", "An unknown name is not contained", World->ContainsVolume("Unknown")) && Passed;
  Passed = EvaluateFalse("ContainsVolume()", "template, no templates", "The template name is not used by default", World->ContainsVolume("Template")) && Passed;
  Passed = EvaluateTrue("ContainsVolume()", "template with templates", "The template name is used on demand", World->ContainsVolume("Template", true)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the detector related flags
bool UTDVolume::TestDetectorFlags()
{
  bool Passed = true;

  MDStrip2D DetectorA("DetectorA");
  MDStrip2D DetectorB("DetectorB");

  MDVolume* A = MakeBox("A", 10, 10, 10, nullptr, MVector(0, 0, 0));
  MDVolume* B = MakeBox("B", 5, 5, 5, A, MVector(0, 0, 0));
  MDVolume* C = MakeBox("C", 1, 1, 1, B, MVector(0, 0, 0));
  MDVolume* Nested = MakeBox("Nested", 1, 1, 1, A, MVector(5, 5, 5));

  Passed = EvaluateFalse("ContainsDetectorVolume()", "nothing", "Without a detector there is no detector volume", A->ContainsDetectorVolume()) && Passed;

  Nested->SetDetector(&DetectorB);
  Passed = EvaluateTrue("SetDetector()", "pointer", "The detector is stored", Nested->GetDetector() == &DetectorB) && Passed;
  Passed = EvaluateTrue("ContainsDetectorVolume()", "in a daughter", "A detector in a daughter is found", A->ContainsDetectorVolume()) && Passed;
  Passed = EvaluateFalse("ContainsDetectorVolume()", "other branch", "A detector in another branch is not found", B->ContainsDetectorVolume()) && Passed;

  A->SetIsDetectorVolume(&DetectorA);
  Passed = EvaluateTrue("SetIsDetectorVolume()", "A", "The flag is set", A->IsDetectorVolume()) && Passed;
  Passed = EvaluateTrue("SetIsDetectorVolume()", "A", "The detector is set", A->GetDetector() == &DetectorA) && Passed;
  Passed = EvaluateTrue("SetIsDetectorVolume()", "A", "The volume is its own detector volume", A->GetDetectorVolume() == A) && Passed;
  Passed = EvaluateTrue("SetIsDetectorVolume()", "daughter", "The daughter gets the detector", B->GetDetector() == &DetectorA) && Passed;
  Passed = EvaluateTrue("SetIsDetectorVolume()", "daughter", "The daughter gets the detector volume", B->GetDetectorVolume() == A) && Passed;
  Passed = EvaluateFalse("SetIsDetectorVolume()", "daughter", "The daughter does not get the flag", B->IsDetectorVolume()) && Passed;
  Passed = EvaluateTrue("SetIsDetectorVolume()", "daughter of the daughter", "The detector runs through all levels", C->GetDetector() == &DetectorA) && Passed;
  Passed = EvaluateTrue("SetIsDetectorVolume()", "nested detector", "A nested detector is kept", Nested->GetDetector() == &DetectorB) && Passed;

  // A new daughter of a detector volume inherits the detector
  MDVolume* Late = MakeBox("Late", 1, 1, 1, nullptr, MVector(0, 0, 0));
  A->AddDaughter(Late);
  Passed = EvaluateTrue("AddDaughter()", "to a detector volume", "The new daughter inherits the detector", Late->GetDetector() == &DetectorA) && Passed;
  MDVolume* LateOther = MakeBox("LateOther", 1, 1, 1, nullptr, MVector(0, 0, 0));
  LateOther->SetDetector(&DetectorB);
  A->AddDaughter(LateOther);
  Passed = EvaluateTrue("AddDaughter()", "with own detector", "The new daughter keeps its own detector", LateOther->GetDetector() == &DetectorB) && Passed;

  // Clones become detector volumes too
  MDVolume* Template = MakeBox("Template", 1, 1, 1, nullptr, MVector(0, 0, 0));
  MDVolume* Clone = new MDVolume("Clone");
  Template->AddClone(Clone);
  Template->SetIsDetectorVolume(&DetectorA);
  Passed = EvaluateTrue("SetIsDetectorVolume()", "clone", "The clone gets the flag", Clone->IsDetectorVolume()) && Passed;
  Passed = EvaluateTrue("SetIsDetectorVolume()", "clone", "The clone is its own detector volume", Clone->GetDetectorVolume() == Clone) && Passed;

  // Direct setter:
  MDVolume* Direct = MakeBox("Direct", 1, 1, 1, nullptr, MVector(0, 0, 0));
  Direct->SetDetectorVolume(A, &DetectorA);
  Passed = EvaluateTrue("SetDetectorVolume()", "A", "The detector volume is stored", Direct->GetDetectorVolume() == A) && Passed;
  Passed = EvaluateTrue("SetDetectorVolume()", "A", "The detector is stored", Direct->GetDetector() == &DetectorA) && Passed;
  Passed = EvaluateFalse("SetDetectorVolume()", "A", "The volume does not become a detector volume", Direct->IsDetectorVolume()) && Passed;

  // Sensitive volumes:
  MDVolume* Sensitive = MakeBox("Sensitive", 1, 1, 1, nullptr, MVector(0, 0, 0));
  MDVolume* SensitiveClone = new MDVolume("SensitiveClone");
  Sensitive->AddClone(SensitiveClone);
  Sensitive->SetSensitiveVolumeID(17);
  Passed = Evaluate("SetSensitiveVolumeID()", "17", "The ID is stored", Sensitive->GetSensitiveVolumeID(), 17) && Passed;
  Passed = EvaluateTrue("SetSensitiveVolumeID()", "17", "The volume is sensitive", Sensitive->IsSensitive()) && Passed;
  Passed = EvaluateTrue("SetSensitiveVolumeID()", "clone", "The clone is sensitive", SensitiveClone->IsSensitive()) && Passed;
  MDVolume* Flagged = MakeBox("Flagged", 1, 1, 1, nullptr, MVector(0, 0, 0));
  Flagged->SetIsSensitive();
  Passed = EvaluateTrue("SetIsSensitive()", "set", "The volume is sensitive", Flagged->IsSensitive()) && Passed;
  Passed = Evaluate("SetIsSensitive()", "set", "The ID is not changed", Flagged->GetSensitiveVolumeID(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the clone administration
bool UTDVolume::TestCloneAdministration()
{
  bool Passed = true;

  MDVolume* Template = MakeBox("Template", 1, 1, 1, nullptr, MVector(0, 0, 0));
  MDVolume* First = new MDVolume("First");
  MDVolume* Second = new MDVolume("Second");
  MDVolume* Stranger = new MDVolume("Stranger");
  Template->AddClone(First);
  Template->AddClone(Second);

  Passed = Evaluate("GetNClones()", "two clones", "Two clones are stored", Template->GetNClones(), 2U) && Passed;
  Passed = EvaluateTrue("GetCloneAt()", "index 0", "The first clone is returned", Template->GetCloneAt(0) == First) && Passed;
  Passed = EvaluateTrue("GetCloneAt()", "index n - 1", "The last clone is returned", Template->GetCloneAt(1) == Second) && Passed;
  // Defect GEO-B-42: An index behind the end stops the program with a ROOT Fatal
  Passed = ExpectedToCrash(EvaluateTrue("GetCloneAt()", "index n", "The first index after the end returns no volume, and the program goes on", Template->GetCloneAt(2) == nullptr)) && Passed;
  Passed = EvaluateTrue("IsCloneTemplate()", "two clones", "The volume is a clone template", Template->IsCloneTemplate()) && Passed;
  Passed = EvaluateFalse("IsClone()", "template", "The template is no clone", Template->IsClone()) && Passed;
  Passed = EvaluateTrue("IsClone()", "clone", "The clone knows it", First->IsClone()) && Passed;
  Passed = EvaluateTrue("GetCloneTemplate()", "clone", "AddClone sets the template of the clone", First->GetCloneTemplate() == Template) && Passed;

  MDVolume* Manual = new MDVolume("Manual");
  Manual->SetCloneTemplate(Template);
  Passed = EvaluateTrue("SetCloneTemplate()", "template", "The template is stored", Manual->GetCloneTemplate() == Template) && Passed;
  Passed = Evaluate("SetCloneTemplate()", "template", "The template does not get to know the clone", Template->GetNClones(), 2U) && Passed;

  // Clone IDs:
  Passed = Evaluate("GetCloneId()", "no clone", "An unknown volume gets the maximum number", Template->GetCloneId(Stranger), numeric_limits<unsigned int>::max()) && Passed;
  unsigned int FirstCall = Template->GetCloneId(First);
  unsigned int SecondCall = Template->GetCloneId(First);
  // Defect GEO-B-5: GetCloneId returns a running number instead of a fixed ID of the clone
  Passed = ExpectedToFail(Evaluate("GetCloneId()", "same clone twice", "Asking twice for the ID of the same clone gives the same ID", SecondCall, FirstCall)) && Passed;

  // The written flags:
  Template->SetCloneTemplateVolumeWritten(true);
  Passed = EvaluateTrue("SetCloneTemplateVolumeWritten()", "true", "The flag is set", Template->IsCloneTemplateVolumeWritten()) && Passed;
  Template->SetCloneTemplateDaughtersWritten(true);
  Passed = EvaluateTrue("SetCloneTemplateDaughtersWritten()", "true", "The flag is set", Template->AreCloneTemplateDaughtersWritten()) && Passed;
  First->SetCloneTemplateVolumeWritten(true);

  MDVolume* Daughter = MakeBox("Daughter", 1, 1, 1, Template, MVector(0, 0, 0));
  Daughter->SetCloneTemplateVolumeWritten(true);
  Daughter->SetCloneTemplateDaughtersWritten(true);

  Template->ResetCloneTemplateFlags();
  Passed = EvaluateFalse("ResetCloneTemplateFlags()", "template", "The volume flag is reset", Template->IsCloneTemplateVolumeWritten()) && Passed;
  Passed = EvaluateFalse("ResetCloneTemplateFlags()", "template", "The daughters flag is reset", Template->AreCloneTemplateDaughtersWritten()) && Passed;
  Passed = EvaluateFalse("ResetCloneTemplateFlags()", "clone", "The flag of the clone is reset", First->IsCloneTemplateVolumeWritten()) && Passed;
  Passed = EvaluateFalse("ResetCloneTemplateFlags()", "daughter", "The volume flag of the daughter is reset", Daughter->IsCloneTemplateVolumeWritten()) && Passed;
  Passed = EvaluateFalse("ResetCloneTemplateFlags()", "daughter", "The daughters flag of the daughter is reset", Daughter->AreCloneTemplateDaughtersWritten()) && Passed;

  // A reset from the clone reaches its template:
  Template->SetCloneTemplateVolumeWritten(true);
  Template->SetCloneTemplateDaughtersWritten(true);
  Second->ResetCloneTemplateFlags();
  Passed = EvaluateFalse("ResetCloneTemplateFlags()", "from the clone", "The volume flag of the template is reset", Template->IsCloneTemplateVolumeWritten()) && Passed;
  Passed = EvaluateFalse("ResetCloneTemplateFlags()", "from the clone", "The daughters flag of the template is reset", Template->AreCloneTemplateDaughtersWritten()) && Passed;
  Passed = Evaluate("ResetCloneTemplateFlags()", "from the clone", "The running clone number of the template restarts", Template->GetCloneId(First), 1U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the clone function
bool UTDVolume::TestClone()
{
  bool Passed = true;

  MDStrip2D Detector("Detector");

  MDVolume* Mother = MakeBox("Mother", 50, 50, 50, nullptr, MVector(0, 0, 0));
  UTDVolumeAccess* Original = new UTDVolumeAccess("Original");
  SetUpBox(Original, 5, 5, 5, Mother, MVector(1, 2, 3));
  Original->SetRotation(10.0, 20.0, 30.0);
  Original->SetColor(4);
  Original->SetLineWidth(5);
  Original->SetLineStyle(6);
  Original->SetVisibility(2);
  Original->SetVirtual(true);
  Original->SetMany(true);
  MDVolume* Daughter = MakeBox("Daughter", 1, 1, 1, Original, MVector(0, 0, 0));

  MDVolume* New = Original->Clone("Prefix");

  Passed = EvaluateTrue("Clone()", "plain volume", "The name consists of prefix and name", New->GetName() == "Prefix_Original") && Passed;
  Passed = EvaluateTrue("Clone()", "plain volume", "The new volume is a clone of the original", New->GetCloneTemplate() == Original) && Passed;
  Passed = Evaluate("Clone()", "plain volume", "The original knows the clone", Original->GetNClones(), 1U) && Passed;
  Passed = EvaluateTrue("Clone()", "plain volume", "The shape is shared", New->GetShape() == Original->GetShape()) && Passed;
  Passed = EvaluateTrue("Clone()", "plain volume", "The material is shared", New->GetMaterial() == Original->GetMaterial()) && Passed;
  Passed = EvaluateTrue("Clone()", "plain volume", "The virtual flag is copied", New->IsVirtual()) && Passed;
  Passed = EvaluateTrue("Clone()", "plain volume", "The many flag is copied", New->IsMany()) && Passed;
  Passed = EvaluateTrue("Clone()", "plain volume", "The mother is copied", New->GetMother() == Mother) && Passed;
  Passed = Evaluate("Clone()", "plain volume", "The daughters are copied", New->GetNDaughters(), 1U) && Passed;
  Passed = EvaluateTrue("Clone()", "plain volume", "The daughters are shared", New->GetDaughterAt(0) == Daughter) && Passed;
  Passed = Evaluate("Clone()", "plain volume", "The color is copied", New->GetColor(), 4) && Passed;
  Passed = Evaluate("Clone()", "plain volume", "The line width is copied", New->GetLineWidth(), 5) && Passed;
  Passed = Evaluate("Clone()", "plain volume", "The line style is copied", New->GetLineStyle(), 6) && Passed;
  Passed = Evaluate("Clone()", "plain volume", "The visibility is copied", New->GetVisibility(), 2) && Passed;
  Passed = EvaluateVectorNear("Clone()", "plain volume", "The position is copied", New->GetPosition(), MVector(1, 2, 3), 1e-15) && Passed;
  Passed = EvaluateTrue("Clone()", "plain volume", "The volume is rotated", New->IsRotated()) && Passed;
  Passed = EvaluateRotationNear("Clone()", "plain volume", "The rotation is copied", New->GetRotationMatrix(), Original->GetRotationMatrix(), 1e-14) && Passed;
  Passed = Evaluate("Clone()", "plain volume", "The rotation ID is copied", New->GetRotationID(), Original->GetRotationID()) && Passed;

  // Clone of a clone: The data comes from the template, the placement from the clone
  UTDVolumeAccess* Template = new UTDVolumeAccess("Tmpl");
  SetUpBox(Template, 5, 5, 5, Mother, MVector(0, 0, 0));
  UTDVolumeAccess* Placed = new UTDVolumeAccess("Placed");
  Template->AddClone(Placed);
  Placed->SetPosition(MVector(4, 5, 6));
  Placed->SetRotation(0.0, 0.0, 90.0);
  MDVolume* Second = Placed->Clone("Prefix");
  Passed = EvaluateTrue("Clone()", "clone", "The name contains prefix, template, and clone", Second->GetName() == "Prefix_Tmpl[Placed]") && Passed;
  Passed = EvaluateTrue("Clone()", "clone", "The template of the new clone is the template", Second->GetCloneTemplate() == Template) && Passed;
  Passed = Evaluate("Clone()", "clone", "The template knows two clones", Template->GetNClones(), 2U) && Passed;
  Passed = EvaluateTrue("Clone()", "clone", "The shape comes from the template", Second->GetShape() == Template->GetShape()) && Passed;
  Passed = EvaluateTrue("Clone()", "clone", "The mother comes from the template", Second->GetMother() == Mother) && Passed;
  Passed = EvaluateVectorNear("Clone()", "clone", "The position comes from the clone", Second->GetPosition(), MVector(4, 5, 6), 1e-15) && Passed;
  Passed = EvaluateRotationNear("Clone()", "clone", "The rotation comes from the clone", Second->GetRotationMatrix(), Placed->GetRotationMatrix(), 1e-14) && Passed;

  // A sensitive volume which is not the detector volume
  UTDVolumeAccess* Sensitive = new UTDVolumeAccess("Sensitive");
  SetUpBox(Sensitive, 5, 5, 5, Mother, MVector(0, 0, 0));
  Sensitive->SetSensitiveVolumeID(5);
  Sensitive->SetDetector(&Detector);
  MDVolume* SensitiveClone = Sensitive->Clone("Prefix");
  Passed = EvaluateTrue("Clone()", "sensitive volume", "The clone is sensitive", SensitiveClone->IsSensitive()) && Passed;
  // Defect GEO-B-6: Clone turns the clone of each sensitive volume into a detector volume, also if the original is none
  Passed = ExpectedToFail(EvaluateFalse("Clone()", "sensitive volume which is no detector volume", "The clone is no detector volume since the original is none", SensitiveClone->IsDetectorVolume())) && Passed;
  // Defect GEO-B-7: Clone does not copy the sensitive volume ID
  Passed = ExpectedToFail(Evaluate("Clone()", "sensitive volume", "The clone has the sensitive volume ID of the original", SensitiveClone->GetSensitiveVolumeID(), 5)) && Passed;

  // The absorption flag:
  UTDVolumeAccess* NoAbsorptions = new UTDVolumeAccess("NoAbsorptions");
  SetUpBox(NoAbsorptions, 5, 5, 5, Mother, MVector(0, 0, 0));
  NoAbsorptions->SetAbsorptions(false);
  MDVolume* NoAbsorptionsClone = NoAbsorptions->Clone("Prefix");
  // Defect GEO-B-8: Clone does not copy the flag for the calculation of absorptions
  Passed = ExpectedToFail(EvaluateFalse("Clone()", "no absorptions", "The clone has the absorption flag of the original", NoAbsorptionsClone->DoAbsorptions())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copying the data of a template to its clones
bool UTDVolume::TestCopyDataToClones()
{
  bool Passed = true;

  MDStrip2D Detector("Detector");

  MDVolume* Mother = MakeBox("Mother", 50, 50, 50, nullptr, MVector(0, 0, 0));
  MDVolume* Template = MakeBox("Template", 5, 5, 5, Mother, MVector(1, 2, 3));
  Template->SetRotation(10.0, 20.0, 30.0);
  Template->SetColor(4);
  Template->SetLineWidth(5);
  Template->SetLineStyle(6);
  Template->SetVisibility(2);
  Template->SetVirtual(true);
  Template->SetMany(true);
  Template->SetDetector(&Detector);
  MDVolume* Daughter = MakeBox("Daughter", 1, 1, 1, Template, MVector(0, 0, 0));

  MDVolume* Clone = new MDVolume("Clone");
  Template->AddClone(Clone);

  MDVolume* Own = new MDVolume("Own");
  Template->AddClone(Own);
  MDVolume* OtherMother = MakeBox("OtherMother", 50, 50, 50, nullptr, MVector(0, 0, 0));
  MDVolume* OwnShape = MakeBox("OwnShape", 1, 1, 1, nullptr, MVector(0, 0, 0));
  Own->SetShape(OwnShape->GetShape());
  Own->SetMaterial(OwnShape->GetMaterial());
  Own->SetColor(9);
  Own->SetLineWidth(8);
  Own->SetLineStyle(7);
  Own->SetVisibility(0);
  Own->SetRotation(0.0, 0.0, 90.0);
  Own->SetMother(OtherMother);
  Own->SetPosition(MVector(7, 7, 7));

  Passed = EvaluateTrue("CopyDataToClones()", "template with shape", "The call succeeds", Template->CopyDataToClones()) && Passed;

  Passed = EvaluateTrue("CopyDataToClones()", "empty clone", "The shape is copied", Clone->GetShape() == Template->GetShape()) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "empty clone", "The material is copied", Clone->GetMaterial() == Template->GetMaterial()) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "empty clone", "The virtual flag is copied", Clone->IsVirtual()) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "empty clone", "The many flag is copied", Clone->IsMany()) && Passed;
  Passed = Evaluate("CopyDataToClones()", "empty clone", "The color is copied", Clone->GetColor(), 4) && Passed;
  Passed = Evaluate("CopyDataToClones()", "empty clone", "The line width is copied", Clone->GetLineWidth(), 5) && Passed;
  Passed = Evaluate("CopyDataToClones()", "empty clone", "The line style is copied", Clone->GetLineStyle(), 6) && Passed;
  Passed = Evaluate("CopyDataToClones()", "empty clone", "The visibility is copied", Clone->GetVisibility(), 2) && Passed;
  Passed = EvaluateRotationNear("CopyDataToClones()", "empty clone", "The rotation is copied", Clone->GetRotationMatrix(), Template->GetRotationMatrix(), 1e-14) && Passed;
  Passed = Evaluate("CopyDataToClones()", "empty clone", "The rotation ID is copied", Clone->GetRotationID(), Template->GetRotationID()) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "empty clone", "The mother is copied", Clone->GetMother() == Mother) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "empty clone", "The detector is copied", Clone->GetDetector() == &Detector) && Passed;
  Passed = EvaluateVectorNear("CopyDataToClones()", "empty clone", "The position is copied", Clone->GetPosition(), MVector(1, 2, 3), 1e-15) && Passed;
  Passed = Evaluate("CopyDataToClones()", "empty clone", "The daughters are copied", Clone->GetNDaughters(), 1U) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "empty clone", "The daughters are shared", Clone->GetDaughterAt(0) == Daughter) && Passed;

  Passed = EvaluateTrue("CopyDataToClones()", "clone with data", "The own shape stays", Own->GetShape() == OwnShape->GetShape()) && Passed;
  Passed = Evaluate("CopyDataToClones()", "clone with data", "The own color stays", Own->GetColor(), 9) && Passed;
  Passed = Evaluate("CopyDataToClones()", "clone with data", "The own line width stays", Own->GetLineWidth(), 8) && Passed;
  Passed = Evaluate("CopyDataToClones()", "clone with data", "The own line style stays", Own->GetLineStyle(), 7) && Passed;
  Passed = Evaluate("CopyDataToClones()", "clone with data", "The own visibility stays", Own->GetVisibility(), 0) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "clone with data", "The own mother stays", Own->GetMother() == OtherMother) && Passed;
  Passed = EvaluateVectorNear("CopyDataToClones()", "clone with data", "The own position stays", Own->GetPosition(), MVector(7, 7, 7), 1e-15) && Passed;
  Passed = EvaluateVectorNear("CopyDataToClones()", "clone with data", "The own rotation stays", Own->GetInvRotationMatrix()*MVector(1, 0, 0), MVector(0, 1, 0), 1e-14) && Passed;

  // A second call does not add the daughters again:
  Passed = EvaluateTrue("CopyDataToClones()", "second call", "The call succeeds", Template->CopyDataToClones()) && Passed;
  Passed = Evaluate("CopyDataToClones()", "second call", "The daughters are not added twice", Clone->GetNDaughters(), 1U) && Passed;

  // The absorption flag:
  MDVolume* NoAbsorptions = MakeBox("NoAbsorptions", 1, 1, 1, nullptr, MVector(0, 0, 0));
  NoAbsorptions->SetAbsorptions(false);
  MDVolume* NoAbsorptionsClone = new MDVolume("NoAbsorptionsClone");
  NoAbsorptions->AddClone(NoAbsorptionsClone);
  NoAbsorptions->CopyDataToClones();
  // Defect GEO-B-8: CopyDataToClones does not copy the flag for the calculation of absorptions
  Passed = ExpectedToFail(EvaluateFalse("CopyDataToClones()", "no absorptions", "The clone has the absorption flag of the template", NoAbsorptionsClone->DoAbsorptions())) && Passed;

  // The error path writes to cerr, which only a redirection silences
  MDVolume NoShape("NoShape");
  streambuf* OldBuffer = cerr.rdbuf(nullptr);
  bool NoShapeResult = NoShape.CopyDataToClones();
  cerr.clear();
  cerr.rdbuf(OldBuffer);
  Passed = EvaluateFalse("CopyDataToClones()", "no shape", "A template without a shape is refused", NoShapeResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation
bool UTDVolume::TestValidate()
{
  bool Passed = true;

  // Missing shape:
  MDVolume NoShape("NoShape");
  DisableDefaultStreams();
  bool Result = NoShape.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no shape", "A volume without a shape is refused", Result) && Passed;

  // Missing material:
  MDVolume* NoMaterial = MakeBox("NoMaterial", 1, 1, 1, nullptr, MVector(0, 0, 0));
  NoMaterial->SetWorldVolume();
  NoMaterial->SetMaterial(nullptr);
  DisableDefaultStreams();
  Result = NoMaterial->Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no material", "A volume without a material is refused", Result) && Passed;

  // Missing mother:
  MDVolume* NoMother = MakeBox("NoMother", 1, 1, 1, nullptr, MVector(0, 0, 0));
  DisableDefaultStreams();
  Result = NoMother->Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no mother", "A volume without mother which is not the world volume is refused", Result) && Passed;

  // Good world volume with defaults:
  MDVolume* World = new MDVolume("World");
  SetUpBox(World, 10, 10, 10, nullptr, g_VectorNotDefined);
  World->SetWorldVolume();
  World->SetVirtual(true);
  World->SetVisibility(1);
  MDVolume* Daughter = MakeBox("Daughter", 1, 1, 1, World, g_VectorNotDefined);
  DisableDefaultStreams();
  Result = World->Validate();
  EnableDefaultStreams();
  Passed = EvaluateTrue("Validate()", "defaults", "A complete volume is accepted", Result) && Passed;
  Passed = EvaluateVectorNear("Validate()", "no position", "A missing position becomes the origin", World->GetPosition(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateFalse("Validate()", "virtual world", "The world volume cannot be virtual", World->IsVirtual()) && Passed;
  Passed = Evaluate("Validate()", "world", "The world volume is not visible", World->GetVisibility(), 0) && Passed;
  Passed = Evaluate("Validate()", "defaults", "The default color is 1", Daughter->GetColor(), 1) && Passed;
  Passed = Evaluate("Validate()", "defaults", "The default line width is 1", Daughter->GetLineWidth(), 1) && Passed;
  Passed = Evaluate("Validate()", "defaults", "The default line style is 1", Daughter->GetLineStyle(), 1) && Passed;
  Passed = Evaluate("Validate()", "defaults", "The default visibility is 1", Daughter->GetVisibility(), 1) && Passed;
  Passed = EvaluateVectorNear("Validate()", "daughter", "The validation reaches the daughters", Daughter->GetPosition(), MVector(0, 0, 0), 1e-15) && Passed;

  // The error in a daughter is returned:
  MDVolume* BadMother = MakeBox("BadMother", 1, 1, 1, nullptr, MVector(0, 0, 0));
  BadMother->SetWorldVolume();
  MDVolume* BadDaughter = MakeBox("BadDaughter", 1, 1, 1, BadMother, MVector(0, 0, 0));
  BadDaughter->SetMaterial(nullptr);
  DisableDefaultStreams();
  Result = BadMother->Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "error in daughter", "An error in a daughter is returned", Result) && Passed;

  // Visibility inheritance:
  for (int Visibility : { -1, 3 }) {
    MDVolume* Top = MakeBox("Top", 1, 1, 1, nullptr, MVector(0, 0, 0));
    Top->SetWorldVolume();
    MDVolume* Middle = MakeBox("Middle", 1, 1, 1, Top, MVector(0, 0, 0));
    MDVolume* Bottom = MakeBox("Bottom", 1, 1, 1, Middle, MVector(0, 0, 0));
    Bottom->SetVisibility(1);
    Middle->SetVisibility(Visibility);
    DisableDefaultStreams();
    Top->Validate();
    EnableDefaultStreams();
    Passed = Evaluate("Validate()", "visibility -1 or 3", "The daughters get the visibility", Bottom->GetVisibility(), Visibility) && Passed;
  }

  // A clone needs a mother
  MDVolume* Template = MakeBox("Template", 1, 1, 1, nullptr, MVector(0, 0, 0));
  Template->SetWorldVolume();
  MDVolume* Clone = new MDVolume("Clone");
  Template->AddClone(Clone);
  Clone->SetShape(Template->GetShape());
  Clone->SetMaterial(Template->GetMaterial());
  Clone->SetWorldVolume();
  DisableDefaultStreams();
  Result = Clone->Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "clone without mother", "A clone without a mother is refused", Result) && Passed;

  // The template gets the color of the clone:
  MDVolume* ColorMother = MakeBox("ColorMother", 1, 1, 1, nullptr, MVector(0, 0, 0));
  ColorMother->SetWorldVolume();
  MDVolume* ColorTemplate = MakeBox("ColorTemplate", 1, 1, 1, nullptr, MVector(0, 0, 0));
  MDVolume* ColorClone = new MDVolume("ColorClone");
  ColorTemplate->AddClone(ColorClone);
  ColorClone->SetShape(ColorTemplate->GetShape());
  ColorClone->SetMaterial(ColorTemplate->GetMaterial());
  ColorClone->SetMother(ColorMother);
  ColorClone->SetColor(5);
  DisableDefaultStreams();
  Result = ColorMother->Validate();
  EnableDefaultStreams();
  Passed = EvaluateTrue("Validate()", "clone with color", "The clone is accepted", Result) && Passed;
  Passed = Evaluate("Validate()", "clone with color", "A template without a color gets the color of the clone", ColorTemplate->GetColor(), 5) && Passed;

  // A sensitive volume needs a detector
  MDVolume* Sensitive = MakeBox("Sensitive", 1, 1, 1, nullptr, MVector(0, 0, 0));
  Sensitive->SetWorldVolume();
  Sensitive->SetIsSensitive();
  DisableDefaultStreams();
  Result = Sensitive->Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "sensitive without detector", "A sensitive volume without detector is refused", Result) && Passed;

  // A positioned template is allowed but deprecated:
  MDVolume* PlacedMother = MakeBox("PlacedMother", 1, 1, 1, nullptr, MVector(0, 0, 0));
  PlacedMother->SetWorldVolume();
  MDVolume* PlacedTemplate = MakeBox("PlacedTemplate", 1, 1, 1, PlacedMother, MVector(0, 0, 0));
  MDVolume* PlacedClone = new MDVolume("PlacedClone");
  PlacedTemplate->AddClone(PlacedClone);
  DisableDefaultStreams();
  Result = PlacedMother->Validate();
  EnableDefaultStreams();
  Passed = EvaluateTrue("Validate()", "positioned template", "A positioned template only gets a warning", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the check that all clones have the same mother
bool UTDVolume::TestValidateClonesHaveSameMotherVolume()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 50, 50, 50, nullptr, MVector(0, 0, 0));
  MDVolume* Template = MakeBox("Template", 1, 1, 1, nullptr, MVector(0, 0, 0));
  for (MString Name : { "CloneA", "CloneB" }) {
    MDVolume* Clone = new MDVolume(Name);
    Template->AddClone(Clone);
    Clone->SetShape(Template->GetShape());
    Clone->SetMother(World);
  }

  DisableDefaultStreams();
  bool Result = World->ValidateClonesHaveSameMotherVolume();
  EnableDefaultStreams();
  Passed = EvaluateTrue("ValidateClonesHaveSameMotherVolume()", "same mother", "Clones in the same mother are accepted", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test removing virtual volumes
bool UTDVolume::TestRemoveVirtualVolumes()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  MDVolume* Virtual = MakeBox("Virtual", 40, 40, 40, World, MVector(10, -5, 20));
  Virtual->SetRotation(30.0, 40.0, 50.0);
  Virtual->SetVirtual(true);
  MDVolume* Daughter = MakeBox("Daughter", 3, 2, 1, Virtual, MVector(1, 2, 3));
  Daughter->SetRotation(-20.0, 15.0, 70.0);
  MDVolume* Grand = MakeBox("Grand", 0.5, 0.5, 0.5, Daughter, MVector(1, 0, 0));

  // The world position of a local point before the removal, derived by hand
  const MVector Local(0.1, 0.2, 0.3);
  const MVector InDaughter = MVector(1, 0, 0) + Local;
  const MVector InVirtual = MVector(1, 2, 3) + RotateActive(InDaughter, -20.0, 15.0, 70.0);
  const MVector InWorld = MVector(10, -5, 20) + RotateActive(InVirtual, 30.0, 40.0, 50.0);
  Passed = EvaluateVectorNear("GetPositionInWorldVolume()", "before the removal", "The transformation is the one derived by hand", Grand->GetPositionInWorldVolume(Local), InWorld, 1e-12) && Passed;

  vector<MDVolume*> NewVolumes;
  Passed = EvaluateFalse("RemoveVirtualVolumes()", "world", "The world volume itself is not removed", World->RemoveVirtualVolumes(NewVolumes)) && Passed;
  Passed = EvaluateSize("RemoveVirtualVolumes()", "no clones", "No new volumes are created", NewVolumes.size(), 0) && Passed;
  Passed = Evaluate("RemoveVirtualVolumes()", "one virtual volume", "The world contains the daughter only", World->GetNDaughters(), 1U) && Passed;
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "one virtual volume", "The daughter is now in the world", World->GetDaughterAt(0) == Daughter) && Passed;
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "one virtual volume", "The mother of the daughter is the world", Daughter->GetMother() == World) && Passed;
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "one virtual volume", "The virtual volume has no mother anymore", Virtual->GetMother() == nullptr) && Passed;

  Passed = EvaluateVectorNear("RemoveVirtualVolumes()", "new position", "The daughter is at the former world position", Daughter->GetPosition(), MVector(10, -5, 20) + RotateActive(MVector(1, 2, 3), 30.0, 40.0, 50.0), 1e-12) && Passed;
  Passed = EvaluateVectorNear("RemoveVirtualVolumes()", "transformation", "The world position of a local point of the grand daughter is unchanged", Grand->GetPositionInWorldVolume(Local), InWorld, 1e-12) && Passed;
  Passed = EvaluateNear("RemoveVirtualVolumes()", "rotation", "The new rotation of the daughter is right handed", Daughter->GetRotationMatrix().GetDeterminant(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetVolume()", "after the removal", "The search finds the grand daughter", World->GetVolume(InWorld) == Grand) && Passed;

  // A virtual volume without mother is an error
  MDVolume* Orphan = MakeBox("Orphan", 1, 1, 1, nullptr, MVector(0, 0, 0));
  Orphan->SetVirtual(true);
  DisableDefaultStreams();
  bool Result = Orphan->RemoveVirtualVolumes(NewVolumes);
  EnableDefaultStreams();
  Passed = EvaluateFalse("RemoveVirtualVolumes()", "virtual without mother", "The error is returned", Result) && Passed;

  // Two levels of virtual volumes:
  MDVolume* WorldTwo = MakeBox("WorldTwo", 100, 100, 100, nullptr, MVector(0, 0, 0));
  MDVolume* First = MakeBox("First", 40, 40, 40, WorldTwo, MVector(10, 0, 0));
  First->SetVirtual(true);
  First->SetRotation(0.0, 0.0, 90.0);
  MDVolume* Second = MakeBox("Second", 40, 40, 40, First, MVector(10, 0, 0));
  Second->SetVirtual(true);
  Second->SetRotation(90.0, 0.0, 0.0);
  MDVolume* Real = MakeBox("Real", 1, 1, 1, Second, MVector(0, 10, 0));
  WorldTwo->RemoveVirtualVolumes(NewVolumes);
  Passed = Evaluate("RemoveVirtualVolumes()", "two levels", "Only the real volume is left", WorldTwo->GetNDaughters(), 1U) && Passed;
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "two levels", "The real volume is a direct daughter", Real->GetMother() == WorldTwo) && Passed;
  // (0, 10, 0) around x by 90 deg is (0, 0, 10), plus (10, 0, 0) is (10, 0, 10), around z by 90 deg is (0, 10, 10), plus (10, 0, 0)
  Passed = EvaluateVectorNear("RemoveVirtualVolumes()", "two levels", "The position runs through both virtual volumes", Real->GetPosition(), MVector(10, 10, 10), 1e-12) && Passed;

  // The daughters of a clone template which is a mother get the new volume in all its clones
  MDVolume* WorldThree = MakeBox("WorldThree", 100, 100, 100, nullptr, MVector(0, 0, 0));
  MDVolume* Cell = MakeBox("Cell", 20, 20, 20, nullptr, MVector(0, 0, 0));
  MDVolume* CellClone = new MDVolume("CellClone");
  Cell->AddClone(CellClone);
  CellClone->SetShape(Cell->GetShape());
  CellClone->SetMaterial(Cell->GetMaterial());
  CellClone->SetMother(WorldThree);
  CellClone->SetPosition(MVector(50, 0, 0));
  MDVolume* VirtualInCell = MakeBox("VirtualInCell", 10, 10, 10, Cell, MVector(1, 0, 0));
  VirtualInCell->SetVirtual(true);
  CellClone->AddDaughter(VirtualInCell);
  MDVolume* RealInCell = MakeBox("RealInCell", 1, 1, 1, VirtualInCell, MVector(2, 0, 0));
  Cell->RemoveVirtualVolumes(NewVolumes);
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "in a template", "The volume is a daughter of the template", Cell->GetDaughterAt(0) == RealInCell) && Passed;
  Passed = Evaluate("RemoveVirtualVolumes()", "in a template", "The clone has one daughter", CellClone->GetNDaughters(), 1U) && Passed;
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "in a template", "The volume is a daughter of the clone", CellClone->GetDaughterAt(0) == RealInCell) && Passed;
  Passed = EvaluateVectorNear("RemoveVirtualVolumes()", "in a template", "The position is moved", RealInCell->GetPosition(), MVector(3, 0, 0), 1e-13) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test removing virtual volumes which are clones in a read geometry
bool UTDVolume::TestRemoveVirtualClonesInGeometry()
{
  bool Passed = true;

  MString Text = GetGeometryHead();
  Text += "Volume Group\n";
  Text += "Group.Material Vacuum\n";
  Text += "Group.Shape BRIK 10.0 10.0 10.0\n";
  Text += "Group.Virtual true\n";
  Text += "Volume Member\n";
  Text += "Member.Material Germanium\n";
  Text += "Member.Shape BRIK 1.0 1.0 1.0\n";
  Text += "Member.Position 3.0 0.0 0.0\n";
  Text += "Member.Mother Group\n";
  Text += "Group.Copy GroupA\n";
  Text += "GroupA.Position 30.0 0.0 0.0\n";
  Text += "GroupA.Mother World\n";
  Text += "Group.Copy GroupB\n";
  Text += "GroupB.Position -30.0 0.0 0.0\n";
  Text += "GroupB.Rotation 0.0 0.0 90.0\n";
  Text += "GroupB.Mother World\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "virtual clones", "The geometry can be read", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // In GroupA: (33, 0, 0); in GroupB: the member is at (3, 0, 0) rotated by 90 deg, thus at (-30, 3, 0)
  MDVolume* World = Geometry.GetWorldVolume();
  Passed = Evaluate("RemoveVirtualVolumes()", "two virtual clones", "Two members are in the world", World->GetNDaughters(), 2U) && Passed;
  MDVolume* InA = World->GetVolume(MVector(33.5, 0.5, 0.5));
  MDVolume* InB = World->GetVolume(MVector(-30.5, 3.5, 0.5));
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "first virtual clone", "A volume is found at the position of the first member", InA != nullptr && InA != World) && Passed;
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "second virtual clone", "A volume is found at the position of the second member", InB != nullptr && InB != World) && Passed;
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "virtual clones", "Both members are different volumes", InA != InB) && Passed;
  if (InA != nullptr && InA != World) {
    Passed = EvaluateTrue("Clone()", "first member", "The name has the prefix of the clone", InA->GetName() == "GroupA_Member") && Passed;
    Passed = EvaluateTrue("Clone()", "first member", "The material is the one of the member", InA->GetMaterial() == Geometry.GetMaterial("Germanium")) && Passed;
  }
  Passed = EvaluateTrue("RemoveVirtualVolumes()", "virtual clones", "The place of the former virtual volume is in the world", World->GetVolume(MVector(30, 0, 0)) == World) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test virtualizing volumes without a detector
bool UTDVolume::TestVirtualizeNonDetectorVolumes()
{
  bool Passed = true;

  MDStrip2D Detector("Detector");

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  World->SetWorldVolume();
  MDVolume* WithDetector = MakeBox("WithDetector", 10, 10, 10, World, MVector(0, 0, 0));
  MDVolume* Sensor = MakeBox("Sensor", 1, 1, 1, WithDetector, MVector(0, 0, 0));
  Sensor->SetDetector(&Detector);
  MDVolume* Passive = MakeBox("Passive", 1, 1, 1, WithDetector, MVector(5, 5, 5));
  MDVolume* Empty = MakeBox("Empty", 10, 10, 10, World, MVector(50, 0, 0));
  MDVolume* EmptyDaughter = MakeBox("EmptyDaughter", 1, 1, 1, Empty, MVector(0, 0, 0));
  MDVolume* EmptyClone = new MDVolume("EmptyClone");
  Empty->AddClone(EmptyClone);

  World->VirtualizeNonDetectorVolumes();

  Passed = EvaluateFalse("VirtualizeNonDetectorVolumes()", "world", "The world with a detector stays", World->IsVirtual()) && Passed;
  Passed = EvaluateFalse("VirtualizeNonDetectorVolumes()", "with detector", "A volume containing a detector stays", WithDetector->IsVirtual()) && Passed;
  Passed = EvaluateFalse("VirtualizeNonDetectorVolumes()", "sensor", "A volume with a detector stays", Sensor->IsVirtual()) && Passed;
  Passed = EvaluateTrue("VirtualizeNonDetectorVolumes()", "passive", "A passive sister of the sensor is virtual", Passive->IsVirtual()) && Passed;
  Passed = EvaluateTrue("VirtualizeNonDetectorVolumes()", "empty", "A volume without detector is virtual", Empty->IsVirtual()) && Passed;
  Passed = EvaluateTrue("VirtualizeNonDetectorVolumes()", "empty daughter", "The daughters are virtual too", EmptyDaughter->IsVirtual()) && Passed;
  Passed = EvaluateTrue("VirtualizeNonDetectorVolumes()", "empty clone", "The clones are virtual too", EmptyClone->IsVirtual()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test sorting the volume tree
bool UTDVolume::TestOptimizeVolumeTree()
{
  bool Passed = true;

  MDStrip2D Detector("Detector");

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  MDVolume* A = MakeBox("A", 1, 1, 1, World, MVector(0, 0, 0));
  MDVolume* B = MakeBox("B", 1, 1, 1, World, MVector(0, 0, 0));
  MDVolume* C = MakeBox("C", 1, 1, 1, World, MVector(0, 0, 0));
  MDVolume* D = MakeBox("D", 1, 1, 1, World, MVector(0, 0, 0));
  MDVolume* BPassive = MakeBox("BPassive", 1, 1, 1, B, MVector(0, 0, 0));
  MDVolume* BActive = MakeBox("BActive", 1, 1, 1, B, MVector(0, 0, 0));
  BActive->SetDetector(&Detector);
  D->SetDetector(&Detector);

  World->OptimizeVolumeTree();

  // Swapping: (A, B, C, D) -> (B, A, C, D) -> (B, D, C, A)
  Passed = Evaluate("OptimizeVolumeTree()", "four daughters", "All daughters are kept", World->GetNDaughters(), 4U) && Passed;
  Passed = EvaluateTrue("OptimizeVolumeTree()", "first position", "The first daughter with detector is first", World->GetDaughterAt(0) == B) && Passed;
  Passed = EvaluateTrue("OptimizeVolumeTree()", "second position", "The second daughter with detector is second", World->GetDaughterAt(1) == D) && Passed;
  Passed = EvaluateTrue("OptimizeVolumeTree()", "third position", "The passive daughter C is third", World->GetDaughterAt(2) == C) && Passed;
  Passed = EvaluateTrue("OptimizeVolumeTree()", "fourth position", "The passive daughter A is last", World->GetDaughterAt(3) == A) && Passed;
  Passed = EvaluateTrue("OptimizeVolumeTree()", "recursive", "The next level is sorted too", B->GetDaughterAt(0) == BActive) && Passed;
  Passed = EvaluateTrue("OptimizeVolumeTree()", "recursive", "The passive daughter of the next level is second", B->GetDaughterAt(1) == BPassive) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test scaling
bool UTDVolume::TestScale()
{
  bool Passed = true;

  // Isotropic scaling:
  MDVolume* Mother = MakeBox("Mother", 5, 6, 7, nullptr, MVector(11, 12, 13));
  MDVolume* Daughter = MakeBox("Daughter", 1, 1, 1, Mother, MVector(2, 3, 4));
  Passed = EvaluateTrue("Scale()", "2, XYZ", "The scaling is accepted", Mother->Scale(2.0, "XYZ")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "2, XYZ", "The shape of the volume is scaled", Mother->GetSize(), MVector(10, 12, 14), 1e-14) && Passed;
  Passed = EvaluateVectorNear("Scale()", "2, XYZ", "The position of the volume is unchanged", Mother->GetPosition(), MVector(11, 12, 13), 1e-14) && Passed;
  Passed = EvaluateVectorNear("Scale()", "2, XYZ", "The shape of the daughter is scaled", Daughter->GetSize(), MVector(2, 2, 2), 1e-14) && Passed;
  Passed = EvaluateVectorNear("Scale()", "2, XYZ", "The position of the daughter is scaled", Daughter->GetPosition(), MVector(4, 6, 8), 1e-14) && Passed;

  // No scaling:
  MDVolume* One = MakeBox("One", 5, 6, 7, nullptr, MVector(0, 0, 0));
  Passed = EvaluateTrue("Scale()", "factor 1", "A factor 1 is accepted", One->Scale(1.0, "XYZ")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "factor 1", "A factor of 1 does not change the shape", One->GetSize(), MVector(5, 6, 7), 1e-14) && Passed;
  Passed = EvaluateTrue("Scale()", "no axes", "No axes are accepted", One->Scale(2.0, "")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "no axes", "No axes do not change the shape", One->GetSize(), MVector(5, 6, 7), 1e-14) && Passed;

  // Only one axis:
  MDVolume* XMother = MakeBox("XMother", 5, 6, 7, nullptr, MVector(0, 0, 0));
  MDVolume* XDaughter = MakeBox("XDaughter", 1, 1, 1, XMother, MVector(2, 3, 4));
  Passed = EvaluateTrue("Scale()", "2, X", "The scaling is accepted", XMother->Scale(2.0, "X")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "2, X", "Only x of the shape is scaled", XMother->GetSize(), MVector(10, 6, 7), 1e-14) && Passed;
  Passed = EvaluateVectorNear("Scale()", "2, X", "Only x of the daughter shape is scaled", XDaughter->GetSize(), MVector(2, 1, 1), 1e-14) && Passed;
  // Defect GEO-B-9: Scale multiplies all three coordinates of the daughter position independent of the axes
  Passed = ExpectedToFail(EvaluateVectorNear("Scale()", "2, X", "Only x of the daughter position is scaled", XDaughter->GetPosition(), MVector(4, 3, 4), 1e-14)) && Passed;

  // The failed scaling of a shape is returned
  MDVolume* Bad = MakeBox("Bad", 5, 6, 7, nullptr, MVector(0, 0, 0));
  DisableDefaultStreams();
  bool BadResult = Bad->Scale(2.0, "Q");
  EnableDefaultStreams();
  Passed = EvaluateFalse("Scale()", "unknown axis", "An unknown axis is refused", BadResult) && Passed;

  // Clones share one shape and one set of daughters
  MDVolume* World = MakeBox("World", 50, 50, 50, nullptr, MVector(0, 0, 0));
  MDVolume* Template = MakeBox("Template", 5, 5, 5, nullptr, MVector(0, 0, 0));
  MDVolume* Shared = MakeBox("Shared", 1, 1, 1, Template, MVector(1, 0, 0));
  for (MString Name : { "CloneA", "CloneB" }) {
    MDVolume* Clone = new MDVolume(Name);
    Template->AddClone(Clone);
    Clone->SetMother(World);
    Clone->SetPosition(MVector(10, 0, 0));
  }
  DisableDefaultStreams();
  Template->CopyDataToClones();
  EnableDefaultStreams();
  Passed = EvaluateTrue("Scale()", "two clones", "The scaling is accepted", World->Scale(2.0, "XYZ")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "two clones", "The shared shape is scaled once", Template->GetSize(), MVector(10, 10, 10), 1e-14) && Passed;
  Passed = EvaluateVectorNear("Scale()", "two clones", "The positions of the clones are scaled", World->GetDaughterAt(1)->GetPosition(), MVector(20, 0, 0), 1e-14) && Passed;
  Passed = EvaluateVectorNear("Scale()", "two clones", "The shape of the shared daughter is scaled once", Shared->GetSize(), MVector(2, 2, 2), 1e-14) && Passed;
  // Defect GEO-B-10: Scale multiplies the position of a shared daughter once per clone of its mother
  Passed = ExpectedToFail(EvaluateVectorNear("Scale()", "two clones", "The position of the shared daughter is scaled once", Shared->GetPosition(), MVector(2, 0, 0), 1e-14)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume sequence creation
bool UTDVolume::TestGetVolumeSequence()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "nested geometry", "The geometry can be read", ScanGeometry(Geometry, GetNestedGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  MDVolume* Holder = Geometry.GetVolume("Holder");
  MDVolume* Wafer = Geometry.GetVolume("Wafer");
  MDVolume* Plug = Geometry.GetVolume("Plug");

  // A point in the wafer, not in the plug:
  const MVector Local(-2.0, -3.0, 0.5);
  const MVector InHolder = MVector(1, 2, 3) + RotateActive(Local, 0.0, 0.0, 90.0);
  const MVector InWorld = MVector(10, -5, 20) + RotateActive(InHolder, 30.0, 40.0, 50.0);

  MDVolumeSequence VS;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The position is inside", World->GetVolumeSequence(InWorld, &VS)) && Passed;
  Passed = Evaluate("GetVolumeSequence()", "in the wafer", "Three volumes are found", VS.GetNVolumes(), 3U) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The first is the world", VS.GetVolumeAt(0) == World) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The second is the holder", VS.GetVolumeAt(1) == Holder) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The third is the wafer", VS.GetVolumeAt(2) == Wafer) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The sensitive volume is the wafer", VS.GetSensitiveVolume() == Wafer) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The detector volume is the wafer", VS.GetDetectorVolume() == Wafer) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The detector is found", VS.GetDetector() == Geometry.GetDetector("Det")) && Passed;
  Passed = EvaluateVectorNear("GetVolumeSequence()", "in the wafer", "The position in the sensitive volume is the local position", VS.GetPositionInSensitiveVolume(), Local, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetVolumeSequence()", "in the wafer", "The position in the detector is the local position", VS.GetPositionInDetector(), Local, 1e-12) && Passed;

  // In the plug: Not sensitive, but inside of the detector volume
  const MVector PlugLocal(0.5, 0.5, 0.25);
  const MVector PlugInHolder = MVector(1, 2, 3) + RotateActive(MVector(3, 3, 0) + PlugLocal, 0.0, 0.0, 90.0);
  const MVector PlugInWorld = MVector(10, -5, 20) + RotateActive(PlugInHolder, 30.0, 40.0, 50.0);
  MDVolumeSequence PlugVS;
  World->GetVolumeSequence(PlugInWorld, &PlugVS);
  Passed = Evaluate("GetVolumeSequence()", "in the plug", "Four volumes are found", PlugVS.GetNVolumes(), 4U) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the plug", "The deepest volume is the plug", PlugVS.GetDeepestVolume() == Plug) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the plug", "There is no sensitive volume", PlugVS.GetSensitiveVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the plug", "The detector volume is the wafer", PlugVS.GetDetectorVolume() == Wafer) && Passed;
  Passed = EvaluateVectorNear("GetVolumeSequence()", "in the plug", "The position in the detector volume is the position in the wafer", PlugVS.GetPositionInDetector(), MVector(3, 3, 0) + PlugLocal, 1e-12) && Passed;

  // Outside:
  MDVolumeSequence OutsideVS;
  Passed = EvaluateFalse("GetVolumeSequence()", "outside of the world", "The position is not inside", World->GetVolumeSequence(MVector(1000, 0, 0), &OutsideVS)) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "outside of the world", "The sequence stays empty", OutsideVS.IsEmpty()) && Passed;

  // Inverse: The position starts in the volume
  MDVolumeSequence InverseVS;
  Passed = EvaluateTrue("GetVolumeSequenceInverse()", "in the wafer", "The position is accepted", Wafer->GetVolumeSequenceInverse(Local, &InverseVS)) && Passed;
  Passed = EvaluateTrue("GetVolumeSequenceInverse()", "in the wafer", "The sensitive volume is the wafer", InverseVS.GetSensitiveVolume() == Wafer) && Passed;
  Passed = EvaluateVectorNear("GetVolumeSequenceInverse()", "in the wafer", "The round trip restores the local position", InverseVS.GetPositionInSensitiveVolume(), Local, 1e-12) && Passed;

  // The sequence is overwritten:
  InverseVS.SetSensitiveVolume(Holder);
  Passed = EvaluateTrue("GetVolumeSequenceInverse()", "reused sequence", "The position is accepted", Wafer->GetVolumeSequenceInverse(Local, &InverseVS)) && Passed;
  Passed = Evaluate("GetVolumeSequenceInverse()", "reused sequence", "The sequence is reset first", InverseVS.GetNVolumes(), 3U) && Passed;

  MDVolumeSequence OutsideInverse;
  DisableDefaultStreams();
  bool Result = Wafer->GetVolumeSequenceInverse(MVector(50, 0, 0), &OutsideInverse);
  EnableDefaultStreams();
  Passed = EvaluateFalse("GetVolumeSequenceInverse()", "outside", "A position outside of the volume is refused", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising of positions
bool UTDVolume::TestNoise()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "nested geometry", "The geometry can be read", ScanGeometry(Geometry, GetNestedGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();

  // The strip detector does not move a position, thus the position has to survive the trip down and up
  // A point in the plug which has the detector of the wafer:
  const MVector PlugInHolder = MVector(1, 2, 3) + RotateActive(MVector(3.5, 3.5, 0.25), 0.0, 0.0, 90.0);
  const MVector PlugInWorld = MVector(10, -5, 20) + RotateActive(PlugInHolder, 30.0, 40.0, 50.0);
  MVector Position = PlugInWorld;
  double Energy = 100.0;
  double Time = 1.0;
  Passed = EvaluateTrue("Noise()", "in the deepest volume", "The hit is noised", World->Noise(Position, Energy, Time)) && Passed;
  Passed = EvaluateVectorNear("Noise()", "in the deepest volume", "The position is restored", Position, PlugInWorld, 1e-12) && Passed;

  // A point in the wafer, which has a daughter it is not in:
  const MVector WaferInHolder = MVector(1, 2, 3) + RotateActive(MVector(-2, -3, 0.5), 0.0, 0.0, 90.0);
  const MVector WaferInWorld = MVector(10, -5, 20) + RotateActive(WaferInHolder, 30.0, 40.0, 50.0);
  Position = WaferInWorld;
  Passed = EvaluateTrue("Noise()", "volume with a daughter", "The hit is noised", World->Noise(Position, Energy, Time)) && Passed;
  // Defect GEO-B-11: After a failed search in the daughters Noise goes on with the position in the coordinates of the last daughter
  Passed = ExpectedToFail(EvaluateVectorNear("Noise()", "volume with a daughter", "The position is restored", Position, WaferInWorld, 1e-12)) && Passed;

  // Outside of a detector - and the world has a daughter:
  const MVector InWorld(-90.0, -90.0, -90.0);
  Position = InWorld;
  Passed = EvaluateFalse("Noise()", "passive", "A hit outside of a detector is not noised", World->Noise(Position, Energy, Time)) && Passed;
  // Defect GEO-B-11: After a failed search in the daughters Noise goes on with the position in the coordinates of the last daughter
  Passed = ExpectedToFail(EvaluateVectorNear("Noise()", "passive", "The position is restored", Position, InWorld, 1e-12)) && Passed;

  // Outside of the world
  Position = MVector(1000, 0, 0);
  Passed = EvaluateFalse("Noise()", "outside", "A hit outside is not noised", World->Noise(Position, Energy, Time)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the pulse shape interface
bool UTDVolume::TestApplyPulseShape()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "nested geometry", "The geometry can be read", ScanGeometry(Geometry, GetNestedGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();

  const MVector PlugInHolder = MVector(1, 2, 3) + RotateActive(MVector(3.5, 3.5, 0.25), 0.0, 0.0, 90.0);
  const MVector PlugInWorld = MVector(10, -5, 20) + RotateActive(PlugInHolder, 30.0, 40.0, 50.0);

  MVector Position = PlugInWorld;
  double Energy = 100.0;
  Passed = EvaluateTrue("ApplyPulseShape()", "in the deepest volume", "The hit is handled", World->ApplyPulseShape(0.0, Position, Energy)) && Passed;
  Passed = EvaluateVectorNear("ApplyPulseShape()", "in the deepest volume", "The position is restored", Position, PlugInWorld, 1e-12) && Passed;

  // A point in the wafer, which has a daughter it is not in:
  const MVector WaferInHolder = MVector(1, 2, 3) + RotateActive(MVector(-2, -3, 0.5), 0.0, 0.0, 90.0);
  const MVector WaferInWorld = MVector(10, -5, 20) + RotateActive(WaferInHolder, 30.0, 40.0, 50.0);
  Position = WaferInWorld;
  Passed = EvaluateTrue("ApplyPulseShape()", "volume with a daughter", "The hit is handled", World->ApplyPulseShape(0.0, Position, Energy)) && Passed;
  // Defect GEO-B-12: After a failed search in the daughters ApplyPulseShape goes on with the position in the coordinates of the last daughter
  Passed = ExpectedToFail(EvaluateVectorNear("ApplyPulseShape()", "volume with a daughter", "The position is restored", Position, WaferInWorld, 1e-12)) && Passed;

  // Outside of the world:
  Position = MVector(1000, 0, 0);
  Passed = EvaluateFalse("ApplyPulseShape()", "outside", "A hit outside is not handled", World->ApplyPulseShape(0.0, Position, Energy)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the path lengths
bool UTDVolume::TestGetAbsorptionLengths()
{
  bool Passed = true;

  // Each crossing into a volume starts 1e-7 cm behind the surface, thus the lengths are exact up to a few 1e-7 cm
  const double Tolerance = 5e-7;

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  World->SetMaterial(MDMaterial::GetDummyMaterial());
  MDMaterial* Material = new MDMaterial("UTDVolumeA");
  MDVolume* Box = MakeBox("Box", 5, 5, 5, World, MVector(10, 0, 0));
  Box->SetMaterial(Material);
  MDVolume* NoAbsorptions = MakeBox("NoAbsorptions", 5, 5, 5, World, MVector(-20, 0, 0));
  NoAbsorptions->SetMaterial(Material);
  NoAbsorptions->SetAbsorptions(false);

  // Through the box:
  map<MDMaterial*, double> Lengths;
  double Returned = World->GetAbsorptionLengths(Lengths, MVector(0, 0, 0), MVector(30, 0, 0));
  Passed = EvaluateNear("GetAbsorptionLengths()", "through the box", "The return value is the length in this volume", Returned, 30.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionLengths()", "through the box", "The length in the box is its width", Lengths[Material], 10.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionLengths()", "through the box", "The length in the world is the rest", Lengths[MDMaterial::GetDummyMaterial()], 20.0, Tolerance) && Passed;

  // Into the box:
  Lengths.clear();
  World->GetAbsorptionLengths(Lengths, MVector(0, 0, 0), MVector(12, 0, 0));
  Passed = EvaluateNear("GetAbsorptionLengths()", "stop in the box", "The length in the box ends at the stop", Lengths[Material], 7.0, Tolerance) && Passed;

  // Start in the box:
  Lengths.clear();
  World->GetAbsorptionLengths(Lengths, MVector(12, 0, 0), MVector(30, 0, 0));
  Passed = EvaluateNear("GetAbsorptionLengths()", "start in the box", "The length in the box starts at the start", Lengths[Material], 3.0, Tolerance) && Passed;

  // In the box only:
  Lengths.clear();
  World->GetAbsorptionLengths(Lengths, MVector(8, 0, 0), MVector(12, 0, 0));
  Passed = EvaluateNear("GetAbsorptionLengths()", "only in the box", "The length is the full distance", Lengths[Material], 4.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionLengths()", "only in the box", "There is nothing in the world", Lengths[MDMaterial::GetDummyMaterial()], 0.0, Tolerance) && Passed;

  // Missing the box:
  Lengths.clear();
  World->GetAbsorptionLengths(Lengths, MVector(0, 10, 0), MVector(30, 10, 0));
  Passed = EvaluateTrue("GetAbsorptionLengths()", "beside the box", "The box does not get an entry", Lengths.find(Material) == Lengths.end()) && Passed;

  // Stopping before the box:
  Lengths.clear();
  World->GetAbsorptionLengths(Lengths, MVector(0, 0, 0), MVector(4.5, 0, 0));
  Passed = EvaluateTrue("GetAbsorptionLengths()", "stop before the box", "The box does not get an entry", Lengths.find(Material) == Lengths.end()) && Passed;

  // Start equals stop:
  Lengths.clear();
  Returned = World->GetAbsorptionLengths(Lengths, MVector(10, 0, 0), MVector(10, 0, 0));
  Passed = EvaluateNear("GetAbsorptionLengths()", "start equals stop", "The length is zero", Returned, 0.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetAbsorptionLengths()", "start equals stop", "No material gets an entry", Lengths.size(), 0) && Passed;

  // Reversed direction:
  Lengths.clear();
  World->GetAbsorptionLengths(Lengths, MVector(30, 0, 0), MVector(0, 0, 0));
  Passed = EvaluateNear("GetAbsorptionLengths()", "reversed", "The length does not depend on the direction", Lengths[Material], 10.0, Tolerance) && Passed;

  // No absorptions in a volume: This one does not get an entry
  Lengths.clear();
  double NoAbsorptionReturned = NoAbsorptions->GetAbsorptionLengths(Lengths, MVector(-30, 0, 0), MVector(-10, 0, 0));
  Passed = EvaluateNear("GetAbsorptionLengths()", "no absorptions", "The return value is zero", NoAbsorptionReturned, 0.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetAbsorptionLengths()", "no absorptions", "No material gets an entry", Lengths.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the masses
bool UTDVolume::TestGetMasses()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "nested geometry", "The geometry can be read", ScanGeometry(Geometry, GetNestedGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  map<MDMaterial*, double> Masses;
  double Volume = Geometry.GetWorldVolume()->GetMasses(Masses);
  MDMaterial* Aluminium = Geometry.GetMaterial("Aluminium");
  MDMaterial* Germanium = Geometry.GetMaterial("Germanium");

  // Volumes from the full lengths of the boxes: holder 40^3, wafer 10 * 10 * 2, plug 2 * 2 * 1
  Passed = EvaluateNear("GetMasses()", "nested boxes", "The return value is the volume of the top volume", Volume, 8.0e6, 1e-6) && Passed;
  Passed = EvaluateNear("GetMasses()", "nested boxes", "The holder without the wafer and the plug add to the aluminium mass", Masses[Aluminium], Aluminium->GetDensity()*(64000.0 - 200.0 + 4.0), 1e-6) && Passed;
  Passed = EvaluateNear("GetMasses()", "nested boxes", "The wafer without the plug is the germanium mass", Masses[Germanium], Germanium->GetDensity()*(200.0 - 4.0), 1e-9) && Passed;

  // The masses add up in a reused map:
  double AluminiumMass = Masses[Aluminium];
  Geometry.GetVolume("Plug")->GetMasses(Masses);
  Passed = EvaluateNear("GetMasses()", "reused map", "The masses are added to the map", Masses[Aluminium], AluminiumMass + Aluminium->GetDensity()*4.0, 1e-6) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the visible and sensitive volume counts
bool UTDVolume::TestCounts()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  World->SetVisibility(0);
  MDVolume* A = MakeBox("A", 1, 1, 1, World, MVector(0, 0, 0));
  A->SetVisibility(1);
  MDVolume* B = MakeBox("B", 1, 1, 1, A, MVector(0, 0, 0));
  B->SetVisibility(2);
  MDVolume* C = MakeBox("C", 1, 1, 1, World, MVector(0, 0, 0));
  C->SetVisibility(0);
  C->SetIsSensitive();
  B->SetIsSensitive();

  Passed = Evaluate("GetNVisibleVolumes()", "tree", "The volumes with visibility above zero are counted", World->GetNVisibleVolumes(), 2U) && Passed;
  Passed = Evaluate("GetNSensitiveVolumes()", "tree", "The sensitive volumes are counted", World->GetNSensitiveVolumes(), 2U) && Passed;
  Passed = Evaluate("GetNSensitiveVolumes()", "subtree", "Only the tree below the volume is counted", A->GetNSensitiveVolumes(), 1U) && Passed;
  Passed = Evaluate("GetNVisibleVolumes()", "single invisible volume", "An invisible volume is not counted", C->GetNVisibleVolumes(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the overlap search at one position
bool UTDVolume::TestFindOverlaps()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  MDVolume* A = MakeBox("A", 5, 5, 5, World, MVector(0, 0, 0));
  MDVolume* B = MakeBox("B", 5, 5, 5, World, MVector(8, 0, 0));
  MDVolume* AA = MakeBox("AA", 1, 1, 1, A, MVector(4, 0, 0));
  MDVolume* AB = MakeBox("AB", 1, 1, 1, A, MVector(4.5, 0, 0));

  vector<MDVolume*> Overlaps;
  Passed = EvaluateTrue("FindOverlaps()", "world only", "The position is inside", World->FindOverlaps(MVector(50, 0, 0), Overlaps)) && Passed;
  Passed = EvaluateSize("FindOverlaps()", "world only", "One volume is found", Overlaps.size(), 1) && Passed;
  Passed = EvaluateTrue("FindOverlaps()", "world only", "The volume is the world", Overlaps[0] == World) && Passed;

  // The mother does not count as overlap with its daughters:
  Overlaps.clear();
  World->FindOverlaps(MVector(-4, 0, 0), Overlaps);
  Passed = EvaluateSize("FindOverlaps()", "in A only", "One volume is found", Overlaps.size(), 1) && Passed;
  Passed = EvaluateTrue("FindOverlaps()", "in A only", "The deepest volume is A", Overlaps[0] == A) && Passed;

  // Two sisters overlap:
  Overlaps.clear();
  World->FindOverlaps(MVector(4, 2, 0), Overlaps);
  Passed = EvaluateSize("FindOverlaps()", "in A and B", "Two volumes are found", Overlaps.size(), 2) && Passed;
  Passed = EvaluateTrue("FindOverlaps()", "in A and B", "The first is A", Overlaps[0] == A) && Passed;
  Passed = EvaluateTrue("FindOverlaps()", "in A and B", "The second is B", Overlaps[1] == B) && Passed;

  // Deepest volumes at a position in A and B and both daughters of A: (4.25, 0, 0) is in AA, AB, and B
  Overlaps.clear();
  World->FindOverlaps(MVector(4.25, 0, 0), Overlaps);
  Passed = EvaluateSize("FindOverlaps()", "deep", "Three volumes are found", Overlaps.size(), 3) && Passed;
  Passed = EvaluateTrue("FindOverlaps()", "deep", "The first is AA", Overlaps[0] == AA) && Passed;
  Passed = EvaluateTrue("FindOverlaps()", "deep", "The second is AB", Overlaps[1] == AB) && Passed;
  Passed = EvaluateTrue("FindOverlaps()", "deep", "The third is B", Overlaps[2] == B) && Passed;

  // The tolerance of 1e-6 cm: On the surface a point is outside
  Overlaps.clear();
  Passed = EvaluateFalse("FindOverlaps()", "outside", "A position outside of the volume is not inside", World->FindOverlaps(MVector(1000, 0, 0), Overlaps)) && Passed;
  Passed = EvaluateSize("FindOverlaps()", "outside", "No volume is found", Overlaps.size(), 0) && Passed;
  Overlaps.clear();
  World->FindOverlaps(MVector(-5.0 + 0.5e-6, 0, 0), Overlaps);
  Passed = EvaluateTrue("FindOverlaps()", "0.5e-6 inside of A", "A point closer to the surface than 1e-6 is not in A", Overlaps.size() == 1 && Overlaps[0] == World) && Passed;
  Overlaps.clear();
  World->FindOverlaps(MVector(-5.0 + 2e-6, 0, 0), Overlaps);
  Passed = EvaluateTrue("FindOverlaps()", "2e-6 inside of A", "A point farther from the surface than 1e-6 is in A", Overlaps.size() == 1 && Overlaps[0] == A) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the placements and the random positions
bool UTDVolume::TestPlacementsAndRandomPositions()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "clone geometry", "The geometry can be read", ScanGeometry(Geometry, GetCloneGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  MDVolume* Core = Geometry.GetVolume("Core");

  // World: two placements of the cell, each cell: one core
  vector<int> Placements;
  int TreeDepth = -1;
  Passed = EvaluateTrue("GetNPlacements()", "core", "The volume is found", World->GetNPlacements(Core, Placements, TreeDepth)) && Passed;
  Passed = Evaluate("GetNPlacements()", "core", "The tree depth is restored", TreeDepth, -1) && Passed;
  Passed = EvaluateSize("GetNPlacements()", "core", "The three levels are found", Placements.size(), 3) && Passed;
  if (Placements.size() == 3) {
    Passed = Evaluate("GetNPlacements()", "core", "The world has two placements", Placements[0], 2) && Passed;
    Passed = Evaluate("GetNPlacements()", "core", "The cell has one placement", Placements[1], 1) && Passed;
    Passed = Evaluate("GetNPlacements()", "core", "The core has one placement", Placements[2], 1) && Passed;
  }

  vector<int> PlacementsOfTheWorld;
  TreeDepth = -1;
  Passed = EvaluateTrue("GetNPlacements()", "world", "The search for the top volume succeeds", World->GetNPlacements(World, PlacementsOfTheWorld, TreeDepth)) && Passed;
  Passed = EvaluateSize("GetNPlacements()", "world", "There is one level", PlacementsOfTheWorld.size(), 1) && Passed;

  MDVolume Unknown("Unknown");
  vector<int> PlacementsUnknown;
  TreeDepth = -1;
  Passed = EvaluateFalse("GetNPlacements()", "unknown volume", "An unknown volume is not found", World->GetNPlacements(&Unknown, PlacementsUnknown, TreeDepth)) && Passed;

  // Random position in the first and in the second placement:
  // First: center (22, 0, 0), half length 1; second: center (-20, 2, 0), half length 1
  TRandom* OldRandom = gRandom;
  gRandom->SetSeed(4711);
  bool AllInFirst = true;
  bool AllInSecond = true;
  for (unsigned int i = 0; i < 100; ++i) {
    vector<int> First = { 0, 0, 0 };
    TreeDepth = -1;
    MVector P1 = World->GetRandomPositionInVolume(Core, First, TreeDepth);
    if (fabs(P1.X() - 22.0) > 1.0 || fabs(P1.Y()) > 1.0 || fabs(P1.Z()) > 1.0) {
      AllInFirst = false;
    }
    vector<int> Second = { 1, 0, 0 };
    TreeDepth = -1;
    MVector P2 = World->GetRandomPositionInVolume(Core, Second, TreeDepth);
    if (fabs(P2.X() + 20.0) > 1.0 || fabs(P2.Y() - 2.0) > 1.0 || fabs(P2.Z()) > 1.0) {
      AllInSecond = false;
    }
  }
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "first placement", "All positions are in the core of the first cell", AllInFirst) && Passed;
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "second placement", "All positions are in the core of the second cell, which is rotated by 90 deg", AllInSecond) && Passed;

  // Errors: A placement which does not exist
  vector<int> TooMany = { 5, 0, 0 };
  TreeDepth = -1;
  DisableDefaultStreams();
  MVector NotFound = World->GetRandomPositionInVolume(Core, TooMany, TreeDepth);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "placement does not exist", "The position is not defined", NotFound == g_VectorNotDefined) && Passed;
  Passed = Evaluate("GetRandomPositionInVolume()", "placement does not exist", "The tree depth is restored", TreeDepth, -1) && Passed;

  // Exclusively inside: Half of the box is filled with a daughter
  MDVolume* Box = MakeBox("Box", 2, 2, 2, nullptr, MVector(0, 0, 0));
  MakeBox("HalfDaughter", 1, 2, 2, Box, MVector(1, 0, 0));
  bool AllOutsideOfDaughter = true;
  for (unsigned int i = 0; i < 100; ++i) {
    MVector P = Box->GetRandomPositionExclusivelyInside();
    if (P == g_VectorNotDefined || P.X() > 0.0 || fabs(P.X()) > 2.0 || fabs(P.Y()) > 2.0 || fabs(P.Z()) > 2.0) {
      AllOutsideOfDaughter = false;
    }
  }
  Passed = EvaluateTrue("GetRandomPositionExclusivelyInside()", "half filled box", "All positions are in the free half", AllOutsideOfDaughter) && Passed;

  // A completely filled volume has no position
  MDVolume* Full = MakeBox("Full", 2, 2, 2, nullptr, MVector(0, 0, 0));
  MakeBox("FullDaughter", 2, 2, 2, Full, MVector(0, 0, 0));
  Passed = EvaluateTrue("GetRandomPositionExclusivelyInside()", "filled box", "A filled box has no free position", Full->GetRandomPositionExclusivelyInside() == g_VectorNotDefined) && Passed;
  gRandom = OldRandom;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the center locations
bool UTDVolume::TestGetCenterLocations()
{
  bool Passed = true;

  MDVolume* World = MakeBox("World", 100, 100, 100, nullptr, MVector(0, 0, 0));
  MDVolume* A = MakeBox("A", 20, 20, 20, World, MVector(10, 0, 0));
  A->SetRotation(0.0, 0.0, 90.0);
  MakeBox("B", 1, 1, 1, A, MVector(1, 0, 0));
  MakeBox("C", 1, 1, 1, World, MVector(0, 5, 0));

  vector<MVector> Locations = World->GetCenterLocations("C");
  Passed = EvaluateSize("GetCenterLocations()", "direct daughter", "One location is found", Locations.size(), 1) && Passed;
  if (Locations.size() == 1) {
    Passed = EvaluateVectorNear("GetCenterLocations()", "direct daughter", "The location is the position of the daughter", Locations[0], MVector(0, 5, 0), 1e-14) && Passed;
  }

  Locations = World->GetCenterLocations("Unknown");
  Passed = EvaluateSize("GetCenterLocations()", "unknown name", "No location is found", Locations.size(), 0) && Passed;

  // The daughter B is at (1, 0, 0) in A, which is rotated by 90 deg around z, and at (10, 0, 0) in the world
  Locations = World->GetCenterLocations("B");
  Passed = EvaluateSize("GetCenterLocations()", "daughter of a daughter", "One location is found", Locations.size(), 1) && Passed;
  if (Locations.size() == 1) {
    // Defect GEO-B-13: GetCenterLocations transforms with the rotation and position of the wrong level of the tree
    Passed = ExpectedToFail(EvaluateVectorNear("GetCenterLocations()", "daughter of a daughter", "The location is in the world coordinates", Locations[0], MVector(10, 1, 0), 1e-13)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the ROOT geometry against the geomega geometry
bool UTDVolume::TestCreateRootGeometry()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "nested geometry", "The geometry can be read", ScanGeometry(Geometry, GetNestedGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // ROOT reports its work to stderr
  const int OldLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  // Attention: The manager belongs to the geometry and a new one would delete it
  TGeoManager* Manager = gGeoManager;
  Geometry.GetWorldVolume()->CreateRootGeometry(Manager, nullptr);
  Manager->CloseGeometry();

  // The ROOT search has to find the same deepest volumes as the geomega search
  const MVector PlugInHolder = MVector(1, 2, 3) + RotateActive(MVector(3.5, 3.5, 0.25), 0.0, 0.0, 90.0);
  const MVector PlugInWorld = MVector(10, -5, 20) + RotateActive(PlugInHolder, 30.0, 40.0, 50.0);
  const MVector WaferInHolder = MVector(1, 2, 3) + RotateActive(MVector(-2, -3, 0.5), 0.0, 0.0, 90.0);
  const MVector WaferInWorld = MVector(10, -5, 20) + RotateActive(WaferInHolder, 30.0, 40.0, 50.0);
  const MVector HolderInWorld = MVector(10, -5, 20) + RotateActive(MVector(15, 15, 15), 30.0, 40.0, 50.0);

  vector<MVector> Points = { PlugInWorld, WaferInWorld, HolderInWorld, MVector(-90, -90, -90) };
  vector<MString> Names = { "Plug", "Wafer", "Holder", "World" };
  for (unsigned int p = 0; p < Points.size(); ++p) {
    TGeoNode* Node = Manager->FindNode(Points[p].X(), Points[p].Y(), Points[p].Z());
    MString Found = (Node != nullptr) ? Node->GetVolume()->GetName() : "";
    Passed = EvaluateTrue("CreateRootGeometry()", Names[p], "ROOT finds the volume which the rotations derived by hand expect", Found == Names[p]) && Passed;
  }

  gErrorIgnoreLevel = OldLevel;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTDVolume::TestStrings()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "nested geometry", "The geometry can be read", ScanGeometry(Geometry, GetNestedGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  MDVolume* Wafer = Geometry.GetVolume("Wafer");

  MString Text = Wafer->ToString();
  Passed = EvaluateTrue("ToString()", "wafer", "The name is in the text", Text.Contains("Volume Wafer")) && Passed;
  Passed = EvaluateTrue("ToString()", "wafer", "The material is in the text", Text.Contains("Material: Germanium")) && Passed;
  Passed = EvaluateTrue("ToString()", "wafer", "The position is in the text", Text.Contains("Position: 1, 2, 3")) && Passed;
  Passed = EvaluateTrue("ToString()", "wafer", "The mother is in the text", Text.Contains("Mother volume: Holder")) && Passed;
  Passed = EvaluateTrue("ToString()", "wafer", "The detector is in the text", Text.Contains("Detector: Det")) && Passed;
  Passed = EvaluateTrue("ToString()", "wafer", "The sensitive flag is in the text", Text.Contains("Sensitive volume: yes")) && Passed;
  Passed = EvaluateFalse("ToString()", "not recursive", "The daughter is not in the text", Text.Contains("Volume Plug")) && Passed;

  MString Recursive = Wafer->ToString(true);
  Passed = EvaluateTrue("ToString()", "recursive", "The daughter is in the text", Recursive.Contains("Volume Plug")) && Passed;

  MString WorldText = World->ToString();
  Passed = EvaluateTrue("ToString()", "world", "The world is marked", WorldText.Contains("World volume!")) && Passed;

  MDVolume Empty("Empty");
  MString EmptyText = Empty.ToString();
  Passed = EvaluateTrue("ToString()", "empty", "A missing shape is reported", EmptyText.Contains("Shape not defined")) && Passed;
  Passed = EvaluateTrue("ToString()", "empty", "A missing material is reported", EmptyText.Contains("Material not defined")) && Passed;
  Passed = EvaluateTrue("ToString()", "empty", "A missing mother is reported", EmptyText.Contains("No mother volume defined")) && Passed;

  MDVolume VirtualVolume("VirtualVolume");
  VirtualVolume.SetVirtual(true);
  Passed = EvaluateTrue("ToString()", "virtual", "A virtual volume is marked", VirtualVolume.ToString().Contains("virtual volume")) && Passed;

  MString Tree = World->ToStringVolumeTree();
  Passed = EvaluateTrue("ToStringVolumeTree()", "level 0", "The world starts in the first column", Tree.BeginsWith("World")) && Passed;
  Passed = EvaluateTrue("ToStringVolumeTree()", "level 1", "The holder has two spaces", Tree.Contains("\n  Holder - Vis:")) && Passed;
  Passed = EvaluateTrue("ToStringVolumeTree()", "level 2", "The wafer has four spaces and its detector", Tree.Contains("\n    Wafer (Det) - Vis:")) && Passed;
  Passed = EvaluateTrue("ToStringVolumeTree()", "level 3", "The plug has six spaces", Tree.Contains("\n      Plug")) && Passed;
  Passed = EvaluateTrue("ToStringVolumeTree()", "start level", "The start level moves the text", Wafer->ToStringVolumeTree(2).BeginsWith("    Wafer")) && Passed;

  // Keywords of the geomega file:
  DisableDefaultStreams();
  MString Geomega = Wafer->GetGeomega();
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetGeomega()", "wafer", "The volume keyword is in the text", Geomega.Contains("Volume Wafer")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "wafer", "The material is in the text", Geomega.Contains("Wafer.Material Germanium")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "wafer", "The position is in the text", Geomega.Contains("Wafer.Position 1 2 3")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "wafer", "The mother is in the text", Geomega.Contains("Wafer.Mother Holder")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "wafer", "The rotation is in the text", Geomega.Contains("Wafer.Rotation")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "wafer", "The daughter is in the text", Geomega.Contains("Volume Plug")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the text output can be read back
bool UTDVolume::TestGetGeomegaRoundTrip()
{
  bool Passed = true;

  MString Head;
  Head += "Name UnitTest\n";
  Head += "Version 1.0\n";
  Head += "SurroundingSphere 50.0 0.0 0.0 0.0 50.0\n";
  Head += "Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo\n";

  MString Simple = GetGeometryHead();
  Simple += "Volume Box\n";
  Simple += "Box.Material Aluminium\n";
  Simple += "Box.Shape BRIK 2.0 3.0 4.0\n";
  Simple += "Box.Position 1.0 2.0 3.0\n";
  Simple += "Box.Rotation 0.0 0.0 90.0\n";
  Simple += "Box.Mother World\n";
  Simple += "Volume Tilted\n";
  Simple += "Tilted.Material Aluminium\n";
  Simple += "Tilted.Shape BRIK 2.0 3.0 4.0\n";
  Simple += "Tilted.Position -10.0 2.0 3.0\n";
  Simple += "Tilted.Rotation 30.0 40.0 50.0\n";
  Simple += "Tilted.Mother World\n";
  MDGeometry SimpleGeometry;
  Passed = EvaluateTrue("ScanSetupFile()", "two boxes", "The geometry can be read", ScanGeometry(SimpleGeometry, Simple, "Simple.geo.setup")) && Passed;
  if (SimpleGeometry.IsScanned() == false) {
    return Passed;
  }

  DisableDefaultStreams();
  MString SimpleText = SimpleGeometry.GetWorldVolume()->GetGeomega();
  EnableDefaultStreams();

  // The local x axis points to +y, the local y axis to -x, and the local z axis to +z:
  Passed = EvaluateTrue("GetGeomega()", "90 deg around z", "The Geant3 angles of the axes are written", SimpleText.Contains("Box.Rotation 90 90 90 180 0 0")) && Passed;

  MDGeometry RoundTrip;
  bool Read = ScanGeometry(RoundTrip, Head + SimpleText, "RoundTrip.geo.setup");
  // Defect GEO-B-14: The shape line of GetGeomega contains the definition of a named shape, which the parser refuses
  Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "round trip", "The text of a read geometry can be read again", Read)) && Passed;

  // The rotation line has to survive being read back on its own:
  vector<MString> Lines = SimpleText.Tokenize("\n");
  vector<double> Angles;
  for (const MString& Line : Lines) {
    if (Line.BeginsWith("Tilted.Rotation") == true) {
      vector<MString> Tokens = Line.Tokenize(" ");
      for (unsigned int t = 1; t < Tokens.size(); ++t) {
        Angles.push_back(Tokens[t].ToDouble());
      }
    }
  }
  Passed = EvaluateSize("GetGeomega()", "30, 40, 50 deg", "Six angles are written", Angles.size(), 6) && Passed;
  if (Angles.size() == 6) {
    MDVolume Parsed("Parsed");
    DisableDefaultStreams();
    bool Accepted = Parsed.SetRotation(Angles[0], Angles[1], Angles[2], Angles[3], Angles[4], Angles[5]);
    EnableDefaultStreams();
    // Defect GEO-B-15: Six significant digits of the written angles can violate the orthogonality check of 1e-6
    Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "30, 40, 50 deg", "The written angles are accepted as a rotation", Accepted)) && Passed;
  }

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

  UTDVolume Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDVolume.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
