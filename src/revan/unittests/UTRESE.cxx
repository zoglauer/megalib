/*
 * UTRESE.cxx
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
#include <algorithm>
#include <limits>
#include <new>
#include <set>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGridPoint.h"
#include "MDVolumeSequence.h"
#include "MPhysicalEventHit.h"
#include "MREHit.h"
#include "MRESE.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! A RESE which counts its living objects and gives access to the protected functions of MRESE
class UTRESECounted : public UTRevanCountedRESE
{
public:
  //! Default constructor
  UTRESECounted() : UTRevanCountedRESE() {}
  //! Default destructor
  virtual ~UTRESECounted() {}

  using MRESE::RecalculateResolutions;
  using MRESE::Reset;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the raw event sub element class MRESE
class UTRESE : public UTRevanBase
{
public:
  //! Default constructor
  UTRESE() : UTRevanBase("UTRESE") {}
  //! Default destructor
  virtual ~UTRESE() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a new RESE with the given energy resolution (keV), time (s), and time resolution (s) - the caller owns it
  MRESE* CreateRESE(double EnergyResolution, double Time, double TimeResolution) const;
  //! Return true if redoing the IDs survives an empty slot
  bool ReIDWithEmptySlot();
  //! Return true if the evta string can be created with an empty slot
  bool ToEvtaStringWithEmptySlot();
  //! Return true if deleting at an index which does not exist keeps the content
  bool DeleteAtIndexOutOfBounds();
  //! Return true if deleting an ID which does not exist keeps the content
  bool DeleteUnknownID();

  //! Test the type constants
  bool TestConstants();
  //! Test that the default constructor sets all members
  bool TestDefaultConstruction();
  //! Test the standard constructor
  bool TestStandardConstruction();
  //! Test the distribution of the IDs
  bool TestIDs();
  //! Test the setters and getters of the data
  bool TestSettersAndGetters();
  //! Test the grid point and the origin IDs
  bool TestGridPointAndOriginIDs();
  //! Test the comparison operator
  bool TestComparison();
  //! Test the links
  bool TestLinks();
  //! Test adding and finding RESEs
  bool TestContainer();
  //! Test the resolutions and origin IDs after adding and removing RESEs
  bool TestRecalculateResolutions();
  //! Test the volume sequence of a RESE and of a container
  bool TestVolumeSequence();
  //! Test removing RESEs
  bool TestRemove();
  //! Test deleting RESEs and the ownership
  bool TestDelete();
  //! Test resetting a RESE
  bool TestReset();
  //! Test the random shuffling
  bool TestShuffle();
  //! Test the functions which the derived classes have to implement
  bool TestBaseImplementations();
  //! Test the duplication
  bool TestDuplicate();
  //! Test the conversion to a physical event hit
  bool TestCreatePhysicalEventHit();
  //! Test the sorting by z
  bool TestCompareRESEByZ();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRESE::Run()
{
  bool Passed = true;

  Passed = TestConstants() && Passed;
  Passed = TestDefaultConstruction() && Passed;
  Passed = TestStandardConstruction() && Passed;
  Passed = TestIDs() && Passed;
  Passed = TestSettersAndGetters() && Passed;
  Passed = TestGridPointAndOriginIDs() && Passed;
  Passed = TestComparison() && Passed;
  Passed = TestLinks() && Passed;
  Passed = TestContainer() && Passed;
  Passed = TestRecalculateResolutions() && Passed;
  Passed = TestVolumeSequence() && Passed;
  Passed = TestRemove() && Passed;
  Passed = TestDelete() && Passed;
  Passed = TestReset() && Passed;
  Passed = TestShuffle() && Passed;
  Passed = TestBaseImplementations() && Passed;
  Passed = TestDuplicate() && Passed;
  Passed = TestCreatePhysicalEventHit() && Passed;
  Passed = TestCompareRESEByZ() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new RESE with the given energy resolution (keV), time (s), and time resolution (s) - the caller owns it
MRESE* UTRESE::CreateRESE(double EnergyResolution, double Time, double TimeResolution) const
{
  MRESE* RESE = new MRESE();
  RESE->SetEnergyResolution(EnergyResolution);
  RESE->SetTime(Time);
  RESE->SetTimeResolution(TimeResolution);

  return RESE;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if redoing the IDs survives an empty slot
bool UTRESE::ReIDWithEmptySlot()
{
  MRESE Parent;
  MRESE First;
  MRESE Second;
  Parent.AddRESE(&First);
  Parent.AddRESE(&Second);
  Parent.RemoveRESE(&First);
  Parent.ReID();

  if (Second.GetID() == Parent.GetID() + 1) {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the evta string can be created with an empty slot
bool UTRESE::ToEvtaStringWithEmptySlot()
{
  MRESE Parent;
  MREHit First;
  MREHit Second;
  Parent.AddRESE(&First);
  Parent.AddRESE(&Second);
  Parent.RemoveRESE(&First);

  if (Parent.ToEvtaString(5, 1) == Second.ToEvtaString(5, 1)) {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if deleting at an index which does not exist keeps the content
bool UTRESE::DeleteAtIndexOutOfBounds()
{
  MRESE Parent;
  MRESE* Only = new MRESE();
  Parent.AddRESE(Only);
  Parent.DeleteRESEAt(1);

  bool Kept = false;
  if (Parent.GetNRESEs() == 1 && Parent.GetRESEAt(0) == Only) {
    Kept = true;
  }
  Parent.DeleteAll();

  return Kept;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if deleting an ID which does not exist keeps the content
bool UTRESE::DeleteUnknownID()
{
  MRESE Parent;
  MRESE* Only = new MRESE();
  Parent.AddRESE(Only);
  Parent.DeleteRESE(Only->GetID() + 1000);

  bool Kept = false;
  if (Parent.GetNRESEs() == 1 && Parent.GetRESEAt(0) == Only) {
    Kept = true;
  }
  Parent.DeleteAll();

  return Kept;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the type constants
bool UTRESE::TestConstants()
{
  bool Passed = true;

  // The values are part of the interface: they are listed in the description of the type getter
  Passed = Evaluate("c_Unknown", "constant", "The unknown type is 0", MRESE::c_Unknown, 0) && Passed;
  Passed = Evaluate("c_Hit", "constant", "The hit type is 1", MRESE::c_Hit, 1) && Passed;
  Passed = Evaluate("c_Track", "constant", "The track type is 2", MRESE::c_Track, 2) && Passed;
  Passed = Evaluate("c_Cluster", "constant", "The cluster type is 3", MRESE::c_Cluster, 3) && Passed;
  Passed = Evaluate("c_Bremsstrahlung", "constant", "The bremsstrahlung type is 4", MRESE::c_Bremsstrahlung, 4) && Passed;
  Passed = Evaluate("c_Event", "constant", "The event type is 5", MRESE::c_Event, 5) && Passed;
  Passed = Evaluate("c_StripHit", "constant", "The strip hit type is 6", MRESE::c_StripHit, 6) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the default constructor sets all members
bool UTRESE::TestDefaultConstruction()
{
  bool Passed = true;

  // Construct on memory with a pattern: a member which is not set keeps the pattern
  alignas(MRESE) unsigned char Block[sizeof(MRESE)];
  FillMemory(Block, sizeof(Block), 0xAB);
  MRESE* RESE = new (Block) MRESE();

  Passed = Evaluate("MRESE()", "default", "The type is unknown", RESE->GetType(), MRESE::c_Unknown) && Passed;
  Passed = EvaluateNear("MRESE()", "default", "The energy is zero", RESE->GetEnergy(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("MRESE()", "default", "The energy resolution is zero", RESE->GetEnergyResolution(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("MRESE()", "default", "The time is zero", RESE->GetTime(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("MRESE()", "default", "The time resolution is zero", RESE->GetTimeResolution(), 0.0, 1e-15) && Passed;
  Passed = EvaluateVectorNear("MRESE()", "default", "The position is the origin", RESE->GetPosition(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("MRESE()", "default", "The position resolution is zero", RESE->GetPositionResolution(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = Evaluate("MRESE()", "default", "There is no detector type", RESE->GetDetector(), MDDetector::c_NoDetectorType) && Passed;
  Passed = EvaluateTrue("MRESE()", "default", "The RESE is valid", RESE->IsValid()) && Passed;
  Passed = EvaluateTrue("MRESE()", "default", "Overwriting is allowed", RESE->AllowOverwrite()) && Passed;
  Passed = Evaluate("MRESE()", "default", "There are no noise flags", RESE->GetNoiseFlags(), MString("")) && Passed;
  Passed = EvaluateSize("MRESE()", "default", "There are no origin IDs", RESE->GetOriginIDs().size(), 0) && Passed;
  Passed = Evaluate("MRESE()", "default", "There are no links", RESE->GetNLinks(), 0) && Passed;
  Passed = EvaluateFalse("MRESE()", "default", "HasLinks reports no links", RESE->HasLinks()) && Passed;
  Passed = Evaluate("MRESE()", "default", "There are no RESEs", RESE->GetNRESEs(), 0) && Passed;
  Passed = EvaluateTrue("MRESE()", "default", "There is a volume sequence", RESE->GetVolumeSequence() != nullptr) && Passed;
  if (RESE->GetVolumeSequence() != nullptr) {
    Passed = Evaluate("MRESE()", "default", "The volume sequence is empty", RESE->GetVolumeSequence()->GetNVolumes(), 0U) && Passed;
  }
  Passed = Evaluate("MRESE()", "default", "The grid point has the unknown type", RESE->GetGridPoint().GetType(), static_cast<unsigned int>(MDGridPoint::c_Unknown)) && Passed;

  RESE->~MRESE();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the standard constructor
bool UTRESE::TestStandardConstruction()
{
  bool Passed = true;

  MRESE RESE(MVector(1.5, -2.5, 3.5), 511.0, 0.25, MDDetector::c_Calorimeter, MVector(0.1, 0.2, 0.3), 12.5, 1e-6);

  Passed = Evaluate("MRESE(...)", "standard", "The type is unknown", RESE.GetType(), MRESE::c_Unknown) && Passed;
  Passed = EvaluateVectorNear("MRESE(...)", "standard", "The position is set", RESE.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-15) && Passed;
  Passed = EvaluateNear("MRESE(...)", "standard", "The energy is set", RESE.GetEnergy(), 511.0, 1e-15) && Passed;
  Passed = EvaluateNear("MRESE(...)", "standard", "The time is set", RESE.GetTime(), 0.25, 1e-15) && Passed;
  Passed = Evaluate("MRESE(...)", "standard", "The detector type is set", RESE.GetDetector(), MDDetector::c_Calorimeter) && Passed;
  Passed = EvaluateVectorNear("MRESE(...)", "standard", "The position resolution is set", RESE.GetPositionResolution(), MVector(0.1, 0.2, 0.3), 1e-15) && Passed;
  Passed = EvaluateNear("MRESE(...)", "standard", "The energy resolution is set", RESE.GetEnergyResolution(), 12.5, 1e-15) && Passed;
  Passed = EvaluateNear("MRESE(...)", "standard", "The time resolution is set", RESE.GetTimeResolution(), 1e-6, 1e-21) && Passed;
  Passed = EvaluateTrue("MRESE(...)", "standard", "The RESE is valid", RESE.IsValid()) && Passed;
  Passed = EvaluateTrue("MRESE(...)", "standard", "Overwriting is allowed", RESE.AllowOverwrite()) && Passed;
  Passed = Evaluate("MRESE(...)", "standard", "There are no links", RESE.GetNLinks(), 0) && Passed;
  Passed = Evaluate("MRESE(...)", "standard", "There are no RESEs", RESE.GetNRESEs(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distribution of the IDs
bool UTRESE::TestIDs()
{
  bool Passed = true;

  MRESE::ResetIDCounter();
  MRESE First;
  MRESE Second;
  MRESE Third(MVector(0.0, 0.0, 0.0), 1.0, 0.0, MDDetector::c_Strip2D, MVector(0.0, 0.0, 0.0), 0.0, 0.0);
  Passed = Evaluate("GetID()", "after resetting the counter", "The first RESE has the ID 1", First.GetID(), 1) && Passed;
  Passed = Evaluate("GetID()", "second RESE", "The second RESE has the ID 2", Second.GetID(), 2) && Passed;
  Passed = Evaluate("GetID()", "standard constructor", "The third RESE has the ID 3", Third.GetID(), 3) && Passed;

  // A copy keeps the ID and does not use up a new one:
  MRESE Copy(&First);
  MRESE Fourth;
  Passed = Evaluate("GetID()", "copy", "The copy has the ID of the original", Copy.GetID(), 1) && Passed;
  Passed = Evaluate("GetID()", "after a copy", "The next RESE has the ID 4", Fourth.GetID(), 4) && Passed;

  MRESE::ResetIDCounter();
  MRESE AfterReset;
  Passed = Evaluate("ResetIDCounter()", "second reset", "The IDs start again with 1", AfterReset.GetID(), 1) && Passed;

  // Redo the IDs: the RESE gets the next free one, then its RESEs in their order
  MRESE::ResetIDCounter();
  MRESE Parent;
  MRESE ChildA;
  MRESE ChildB;
  Parent.AddRESE(&ChildA);
  Parent.AddRESE(&ChildB);
  Parent.ReID();
  Passed = Evaluate("ReID()", "RESE with two RESEs", "The RESE gets the ID 4", Parent.GetID(), 4) && Passed;
  Passed = Evaluate("ReID()", "RESE with two RESEs", "Its first RESE gets the ID 5", ChildA.GetID(), 5) && Passed;
  Passed = Evaluate("ReID()", "RESE with two RESEs", "Its second RESE gets the ID 6", ChildB.GetID(), 6) && Passed;

  MRESE Next;
  Passed = Evaluate("ReID()", "RESE created afterwards", "The next RESE gets the ID 7", Next.GetID(), 7) && Passed;

  // Defect REV-A-1: ReID dereferences the empty slot which RemoveRESE leaves behind
  Passed = ExpectedToCrash(EvaluateTrue("ReID()", "empty slot", "The IDs are redone if there is an empty slot", ReIDWithEmptySlot())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters of the data
bool UTRESE::TestSettersAndGetters()
{
  bool Passed = true;

  MRESE RESE;

  RESE.SetPosition(1.25, -2.5, 3.75);
  Passed = EvaluateVectorNear("SetPosition(x, y, z)", "(1.25, -2.5, 3.75)", "The position is returned", RESE.GetPosition(), MVector(1.25, -2.5, 3.75), 1e-15) && Passed;
  Passed = EvaluateNear("GetPositionX()", "(1.25, -2.5, 3.75)", "The x position is returned", RESE.GetPositionX(), 1.25, 1e-15) && Passed;
  Passed = EvaluateNear("GetPositionY()", "(1.25, -2.5, 3.75)", "The y position is returned", RESE.GetPositionY(), -2.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetPositionZ()", "(1.25, -2.5, 3.75)", "The z position is returned", RESE.GetPositionZ(), 3.75, 1e-15) && Passed;

  RESE.SetPosition(MVector(-7.0, 8.0, -9.0));
  Passed = EvaluateVectorNear("SetPosition(MVector)", "(-7, 8, -9)", "The position is returned", RESE.GetPosition(), MVector(-7.0, 8.0, -9.0), 1e-15) && Passed;

  RESE.SetPositionResolution(MVector(0.5, 0.25, 0.125));
  Passed = EvaluateVectorNear("SetPositionResolution()", "(0.5, 0.25, 0.125)", "The position resolution is returned", RESE.GetPositionResolution(), MVector(0.5, 0.25, 0.125), 1e-15) && Passed;
  Passed = EvaluateNear("GetPositionResolutionX()", "(0.5, 0.25, 0.125)", "The x resolution is returned", RESE.GetPositionResolutionX(), 0.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetPositionResolutionY()", "(0.5, 0.25, 0.125)", "The y resolution is returned", RESE.GetPositionResolutionY(), 0.25, 1e-15) && Passed;
  Passed = EvaluateNear("GetPositionResolutionZ()", "(0.5, 0.25, 0.125)", "The z resolution is returned", RESE.GetPositionResolutionZ(), 0.125, 1e-15) && Passed;

  RESE.SetEnergy(661.7);
  Passed = EvaluateNear("SetEnergy()", "661.7 keV", "The energy is returned", RESE.GetEnergy(), 661.7, 1e-12) && Passed;
  RESE.SetEnergyResolution(2.5);
  Passed = EvaluateNear("SetEnergyResolution()", "2.5 keV", "The energy resolution is returned", RESE.GetEnergyResolution(), 2.5, 1e-15) && Passed;

  // The energy getter returns the total energy: the additional energy is added
  RESE.SetAdditionalEnergy(38.3);
  Passed = EvaluateNear("SetAdditionalEnergy()", "661.7 keV plus 38.3 keV", "The energy includes the additional energy", RESE.GetEnergy(), 700.0, 1e-12) && Passed;
  RESE.SetAdditionalEnergy(0.0);
  Passed = EvaluateNear("SetAdditionalEnergy()", "back to zero", "The energy is the energy without additional energy", RESE.GetEnergy(), 661.7, 1e-12) && Passed;

  RESE.SetEnergy(0.0);
  Passed = EvaluateNear("SetEnergy()", "zero", "A zero energy is returned", RESE.GetEnergy(), 0.0, 1e-15) && Passed;
  RESE.SetEnergy(-5.0);
  Passed = EvaluateNear("SetEnergy()", "negative", "A negative energy is stored unchanged", RESE.GetEnergy(), -5.0, 1e-15) && Passed;

  RESE.SetTime(1.5e-3);
  Passed = EvaluateNear("SetTime()", "1.5 ms", "The time is returned", RESE.GetTime(), 1.5e-3, 1e-18) && Passed;
  RESE.SetTimeResolution(2e-9);
  Passed = EvaluateNear("SetTimeResolution()", "2 ns", "The time resolution is returned", RESE.GetTimeResolution(), 2e-9, 1e-24) && Passed;

  RESE.SetDetector(MDDetector::c_Strip3D);
  Passed = Evaluate("SetDetector()", "Strip3D", "The detector type is returned", RESE.GetDetector(), MDDetector::c_Strip3D) && Passed;

  RESE.SetNoiseFlags(" OVERFLOW NODEPTH");
  Passed = Evaluate("SetNoiseFlags()", "two flags", "The noise flags are returned", RESE.GetNoiseFlags(), MString(" OVERFLOW NODEPTH")) && Passed;

  RESE.SetValid(false);
  Passed = EvaluateFalse("SetValid()", "false", "The RESE is not valid", RESE.IsValid()) && Passed;
  RESE.SetValid(true);
  Passed = EvaluateTrue("SetValid()", "true", "The RESE is valid again", RESE.IsValid()) && Passed;

  RESE.AllowOverwrite(false);
  Passed = EvaluateFalse("AllowOverwrite()", "false", "Overwriting is not allowed", RESE.AllowOverwrite()) && Passed;
  RESE.AllowOverwrite(true);
  Passed = EvaluateTrue("AllowOverwrite()", "true", "Overwriting is allowed again", RESE.AllowOverwrite()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the grid point and the origin IDs
bool UTRESE::TestGridPointAndOriginIDs()
{
  bool Passed = true;

  MRESE RESE;

  MDGridPoint Point(3, 4, 5, MDGridPoint::c_Voxel, MVector(0.5, 1.5, 2.5), 77.0, 0.125);
  RESE.SetGridPoint(Point);
  MDGridPoint Stored = RESE.GetGridPoint();
  Passed = Evaluate("SetGridPoint()", "voxel 3, 4, 5", "The x grid is returned", Stored.GetXGrid(), 3U) && Passed;
  Passed = Evaluate("SetGridPoint()", "voxel 3, 4, 5", "The y grid is returned", Stored.GetYGrid(), 4U) && Passed;
  Passed = Evaluate("SetGridPoint()", "voxel 3, 4, 5", "The z grid is returned", Stored.GetZGrid(), 5U) && Passed;
  Passed = Evaluate("SetGridPoint()", "voxel 3, 4, 5", "The type is returned", Stored.GetType(), static_cast<unsigned int>(MDGridPoint::c_Voxel)) && Passed;
  Passed = EvaluateVectorNear("SetGridPoint()", "voxel 3, 4, 5", "The position in the grid is returned", Stored.GetPosition(), MVector(0.5, 1.5, 2.5), 1e-15) && Passed;
  Passed = EvaluateNear("SetGridPoint()", "voxel 3, 4, 5", "The energy is returned", Stored.GetEnergy(), 77.0, 1e-15) && Passed;

  RESE.AddOriginID(7);
  RESE.AddOriginID(3);
  RESE.AddOriginID(7);
  set<unsigned int> IDs = RESE.GetOriginIDs();
  Passed = EvaluateSize("AddOriginID()", "7, 3, 7", "An ID is only stored once", IDs.size(), 2) && Passed;
  Passed = EvaluateSize("AddOriginID()", "7, 3, 7", "The ID 3 is stored", IDs.count(3), 1) && Passed;
  Passed = EvaluateSize("AddOriginID()", "7, 3, 7", "The ID 7 is stored", IDs.count(7), 1) && Passed;

  set<unsigned int> More = { 1, 3, 12 };
  RESE.AddOriginIDs(More);
  IDs = RESE.GetOriginIDs();
  Passed = EvaluateSize("AddOriginIDs()", "1, 3, 12 added to 3, 7", "There are four IDs", IDs.size(), 4) && Passed;
  Passed = EvaluateSize("AddOriginIDs()", "1, 3, 12 added to 3, 7", "The ID 1 is stored", IDs.count(1), 1) && Passed;
  Passed = EvaluateSize("AddOriginIDs()", "1, 3, 12 added to 3, 7", "The ID 12 is stored", IDs.count(12), 1) && Passed;
  Passed = EvaluateSize("AddOriginIDs()", "1, 3, 12 added to 3, 7", "The ID 7 is still stored", IDs.count(7), 1) && Passed;

  RESE.AddOriginIDs(set<unsigned int>());
  Passed = EvaluateSize("AddOriginIDs()", "empty set", "An empty set changes nothing", RESE.GetOriginIDs().size(), 4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the comparison operator
bool UTRESE::TestComparison()
{
  bool Passed = true;

  MRESE Reference(MVector(1.0, 2.0, 3.0), 100.0, 0.5, MDDetector::c_Strip2D, MVector(0.1, 0.1, 0.1), 2.0, 1e-6);

  MRESE Same(MVector(1.0, 2.0, 3.0), 100.0, 0.5, MDDetector::c_Strip2D, MVector(0.1, 0.1, 0.1), 2.0, 1e-6);
  Passed = EvaluateTrue("operator==", "same data, different ID", "Two RESEs with the same data are equal", Reference == Same) && Passed;
  Passed = EvaluateTrue("operator==", "itself", "A RESE is equal to itself", Reference == Reference) && Passed;

  MRESE OtherPosition(MVector(1.0, 2.0, 3.5), 100.0, 0.5, MDDetector::c_Strip2D, MVector(0.1, 0.1, 0.1), 2.0, 1e-6);
  Passed = EvaluateFalse("operator==", "different position", "A different position is not equal", Reference == OtherPosition) && Passed;

  MRESE OtherEnergy(MVector(1.0, 2.0, 3.0), 100.5, 0.5, MDDetector::c_Strip2D, MVector(0.1, 0.1, 0.1), 2.0, 1e-6);
  Passed = EvaluateFalse("operator==", "different energy", "A different energy is not equal", Reference == OtherEnergy) && Passed;

  MRESE OtherTime(MVector(1.0, 2.0, 3.0), 100.0, 0.75, MDDetector::c_Strip2D, MVector(0.1, 0.1, 0.1), 2.0, 1e-6);
  Passed = EvaluateFalse("operator==", "different time", "A different time is not equal", Reference == OtherTime) && Passed;

  MRESE OtherDetector(MVector(1.0, 2.0, 3.0), 100.0, 0.5, MDDetector::c_Calorimeter, MVector(0.1, 0.1, 0.1), 2.0, 1e-6);
  Passed = EvaluateFalse("operator==", "different detector type", "A different detector type is not equal", Reference == OtherDetector) && Passed;

  MRESE OtherAdditionalEnergy(MVector(1.0, 2.0, 3.0), 100.0, 0.5, MDDetector::c_Strip2D, MVector(0.1, 0.1, 0.1), 2.0, 1e-6);
  OtherAdditionalEnergy.SetAdditionalEnergy(1.0);
  Passed = EvaluateFalse("operator==", "different additional energy", "A different additional energy is not equal", Reference == OtherAdditionalEnergy) && Passed;

  // A hit has another type than a plain RESE:
  MREHit Hit;
  Hit.SetPosition(MVector(1.0, 2.0, 3.0));
  Hit.SetEnergy(100.0);
  Hit.SetTime(0.5);
  Hit.SetDetector(MDDetector::c_Strip2D);
  Passed = EvaluateFalse("operator==", "different type", "A hit is not equal to a plain RESE with the same data", Reference == Hit) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the links
bool UTRESE::TestLinks()
{
  bool Passed = true;

  MRESE Center;
  MRESE Left;
  MRESE Right;
  MRESE Other;

  Center.AddLink(&Left);
  Passed = Evaluate("AddLink()", "first link", "There is one link", Center.GetNLinks(), 1) && Passed;
  Passed = EvaluateTrue("HasLinks()", "one link", "The RESE has links", Center.HasLinks()) && Passed;
  Passed = EvaluateTrue("IsLink()", "the linked RESE", "The linked RESE is a link", Center.IsLink(&Left)) && Passed;
  Passed = EvaluateFalse("IsLink()", "another RESE", "Another RESE is no link", Center.IsLink(&Right)) && Passed;
  Passed = EvaluateTrue("GetLinkAt()", "index 0", "The first link is the linked RESE", Center.GetLinkAt(0) == &Left) && Passed;
  // A link has one direction: the track classes add both directions themselves
  Passed = Evaluate("AddLink()", "the linked RESE", "The linked RESE does not get a link back", Left.GetNLinks(), 0) && Passed;

  Passed = EvaluateTrue("GetOtherLink()", "one link, nullptr", "A nullptr returns the first link", Center.GetOtherLink(nullptr) == &Left) && Passed;
  Passed = EvaluateTrue("GetOtherLink()", "one link, the link", "There is no other link", Center.GetOtherLink(&Left) == nullptr) && Passed;

  Center.AddLink(&Right);
  Passed = Evaluate("AddLink()", "second link", "There are two links", Center.GetNLinks(), 2) && Passed;
  Passed = EvaluateTrue("GetLinkAt()", "index 1", "The second link is the second linked RESE", Center.GetLinkAt(1) == &Right) && Passed;
  Passed = EvaluateTrue("GetOtherLink()", "two links, first", "The other link of the first is the second", Center.GetOtherLink(&Left) == &Right) && Passed;
  Passed = EvaluateTrue("GetOtherLink()", "two links, second", "The other link of the second is the first", Center.GetOtherLink(&Right) == &Left) && Passed;
  Passed = EvaluateTrue("GetOtherLink()", "two links, nullptr", "A nullptr returns the first link", Center.GetOtherLink(nullptr) == &Left) && Passed;

  DisableDefaultStreams();
  MRESE* OutOfBounds = Center.GetLinkAt(2);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetLinkAt()", "index behind the last link", "A nullptr is returned", OutOfBounds == nullptr) && Passed;

  // A third link is announced as an error, but only a build with assertions stops there
  DisableDefaultStreams();
  Center.AddLink(&Other);
  EnableDefaultStreams();
  // Defect REV-A-2: AddLink adds a third link in the default build, because the limit of two links is only a compiled-out assertion
  Passed = ExpectedToFail(Evaluate("AddLink()", "third link", "A third link is not added", Center.GetNLinks(), 2)) && Passed;
  DisableDefaultStreams();
  Center.RemoveLink(&Other);
  EnableDefaultStreams();

  // A link which exists is announced as refused, with the RESEs printed to the standard output
  MRESE Twice;
  Twice.AddLink(&Left);
  DisableDefaultStreams();
  BeginCaptureOutput();
  Twice.AddLink(&Left);
  EndCaptureOutput();
  EnableDefaultStreams();
  // Defect REV-A-2: AddLink adds a link a second time in the default build, although its message says that it refuses to
  Passed = ExpectedToFail(Evaluate("AddLink()", "the same link again", "The link is not added a second time", Twice.GetNLinks(), 1)) && Passed;

  // Removing compresses the list:
  Center.RemoveLink(&Left);
  Passed = Evaluate("RemoveLink()", "first of two links", "One link is left", Center.GetNLinks(), 1) && Passed;
  Passed = EvaluateTrue("RemoveLink()", "first of two links", "The second link is now the first", Center.GetLinkAt(0) == &Right) && Passed;
  Passed = EvaluateFalse("RemoveLink()", "first of two links", "The removed RESE is no link any more", Center.IsLink(&Left)) && Passed;

  DisableDefaultStreams();
  Center.RemoveLink(&Other);
  EnableDefaultStreams();
  Passed = Evaluate("RemoveLink()", "a RESE which is no link", "The links are unchanged", Center.GetNLinks(), 1) && Passed;

  Center.AddLink(&Left);
  Center.RemoveAllLinks();
  Passed = Evaluate("RemoveAllLinks()", "two links", "No link is left", Center.GetNLinks(), 0) && Passed;
  Passed = EvaluateFalse("RemoveAllLinks()", "two links", "HasLinks reports no links", Center.HasLinks()) && Passed;
  Center.RemoveAllLinks();
  Passed = Evaluate("RemoveAllLinks()", "no links", "Removing from an empty list changes nothing", Center.GetNLinks(), 0) && Passed;

  // Without links there is no other link - a build with assertions stops here
  DisableDefaultStreams();
  MRESE* NoLink = Center.GetOtherLink(nullptr);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetOtherLink()", "no links", "A nullptr is returned", NoLink == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding and finding RESEs
bool UTRESE::TestContainer()
{
  bool Passed = true;

  MRESE Parent;
  MRESE First;
  MRESE Second;
  MRESE Third;
  MRESE NotAdded;

  Parent.AddRESE(&First);
  Passed = Evaluate("AddRESE()", "first RESE", "There is one RESE", Parent.GetNRESEs(), 1) && Passed;
  Parent.AddRESE(&Second);
  Passed = Evaluate("AddRESE()", "second RESE", "There are two RESEs", Parent.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "index 0", "The first added RESE is first", Parent.GetRESEAt(0) == &First) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "index 1", "The second added RESE is second", Parent.GetRESEAt(1) == &Second) && Passed;

  Parent.AddRESEFirst(&Third);
  Passed = Evaluate("AddRESEFirst()", "third RESE", "There are three RESEs", Parent.GetNRESEs(), 3) && Passed;
  Passed = EvaluateTrue("AddRESEFirst()", "third RESE", "The RESE is the first one", Parent.GetRESEAt(0) == &Third) && Passed;
  Passed = EvaluateTrue("AddRESEFirst()", "third RESE", "The former first RESE is the second one", Parent.GetRESEAt(1) == &First) && Passed;
  Passed = EvaluateTrue("AddRESEFirst()", "third RESE", "The former second RESE is the third one", Parent.GetRESEAt(2) == &Second) && Passed;

  Parent.AddRESE(nullptr);
  Passed = Evaluate("AddRESE()", "nullptr", "A nullptr is not added", Parent.GetNRESEs(), 3) && Passed;

  Passed = EvaluateTrue("GetRESE()", "ID of the second RESE", "The RESE with the ID is returned", Parent.GetRESE(Second.GetID()) == &Second) && Passed;
  Passed = EvaluateTrue("ContainsRESE(MRESE*)", "added RESE", "An added RESE is contained", Parent.ContainsRESE(&First)) && Passed;
  Passed = EvaluateFalse("ContainsRESE(MRESE*)", "other RESE", "Another RESE is not contained", Parent.ContainsRESE(&NotAdded)) && Passed;
  Passed = EvaluateTrue("ContainsRESE(int)", "ID of an added RESE", "The ID of an added RESE is contained", Parent.ContainsRESE(Third.GetID())) && Passed;
  Passed = EvaluateFalse("ContainsRESE(int)", "ID of another RESE", "The ID of another RESE is not contained", Parent.ContainsRESE(NotAdded.GetID())) && Passed;

  DisableDefaultStreams();
  MRESE* OutOfBounds = Parent.GetRESEAt(3);
  MRESE* UnknownID = Parent.GetRESE(NotAdded.GetID());
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetRESEAt()", "index behind the last RESE", "A nullptr is returned", OutOfBounds == nullptr) && Passed;
  Passed = EvaluateTrue("GetRESE()", "unknown ID", "A nullptr is returned", UnknownID == nullptr) && Passed;

  // Defect REV-A-3: GetRESEAt only checks the upper limit, a negative index reads in front of the list
  Passed = ExpectedToCrash(EvaluateTrue("GetRESEAt()", "index -1", "A nullptr is returned", Parent.GetRESEAt(-1) == nullptr)) && Passed;

  // Search through all levels: the RESE of the top level is returned
  MRESE Top;
  MRESE Middle;
  MRESE Leaf;
  MRESE Single;
  Middle.AddRESE(&Leaf);
  Top.AddRESE(&Single);
  Top.AddRESE(&Middle);
  Passed = EvaluateTrue("GetRESEContainingRESE()", "ID of a RESE of the top level", "The RESE itself is returned", Top.GetRESEContainingRESE(Single.GetID()) == &Single) && Passed;
  Passed = EvaluateTrue("GetRESEContainingRESE()", "ID of a RESE of the second level", "The RESE of the top level which contains it is returned", Top.GetRESEContainingRESE(Leaf.GetID()) == &Middle) && Passed;
  Passed = EvaluateTrue("GetRESEContainingRESE()", "unknown ID", "A nullptr is returned", Top.GetRESEContainingRESE(NotAdded.GetID()) == nullptr) && Passed;
  Passed = EvaluateFalse("ContainsRESE(int)", "ID of a RESE of the second level", "Only the top level is searched", Top.ContainsRESE(Leaf.GetID())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the resolutions and origin IDs after adding and removing RESEs
bool UTRESE::TestRecalculateResolutions()
{
  bool Passed = true;

  // Energy resolutions 3 and 4 keV, the second RESE is the earlier one
  MRESE* Late = CreateRESE(3.0, 2.0, 0.5);
  Late->AddOriginID(1);
  Late->AddOriginID(2);
  MRESE* Early = CreateRESE(4.0, 1.0, 0.25);
  Early->AddOriginID(2);
  Early->AddOriginID(3);

  MRESE Parent;
  Parent.AddRESE(Late);
  Passed = EvaluateNear("AddRESE()", "one RESE", "The energy resolution is the one of the RESE", Parent.GetEnergyResolution(), 3.0, 1e-15) && Passed;
  Passed = EvaluateNear("AddRESE()", "one RESE", "The time resolution is the one of the RESE", Parent.GetTimeResolution(), 0.5, 1e-15) && Passed;
  Passed = EvaluateSize("AddRESE()", "one RESE", "The origin IDs are the ones of the RESE", Parent.GetOriginIDs().size(), 2) && Passed;

  Parent.AddRESE(Early);
  // The energy resolutions add in quadrature: sqrt(3^2 + 4^2) = 5
  Passed = EvaluateNear("AddRESE()", "energy resolutions 3 and 4 keV", "The energy resolution is 5 keV", Parent.GetEnergyResolution(), 5.0, 1e-14) && Passed;
  // The time resolution is the one of the earliest RESE:
  Passed = EvaluateNear("AddRESE()", "times 2 s and 1 s", "The time resolution is the one of the earlier RESE", Parent.GetTimeResolution(), 0.25, 1e-15) && Passed;
  set<unsigned int> IDs = Parent.GetOriginIDs();
  Passed = EvaluateSize("AddRESE()", "origin IDs 1, 2 and 2, 3", "There are three origin IDs", IDs.size(), 3) && Passed;
  Passed = EvaluateSize("AddRESE()", "origin IDs 1, 2 and 2, 3", "The ID 1 is included", IDs.count(1), 1) && Passed;
  Passed = EvaluateSize("AddRESE()", "origin IDs 1, 2 and 2, 3", "The ID 3 is included", IDs.count(3), 1) && Passed;

  // Removing leaves an empty slot, which is skipped:
  Parent.RemoveRESE(Early);
  Passed = EvaluateNear("RemoveRESE()", "the RESE with 4 keV", "The energy resolution is 3 keV again", Parent.GetEnergyResolution(), 3.0, 1e-15) && Passed;
  Passed = EvaluateNear("RemoveRESE()", "the earlier RESE", "The time resolution is the one of the remaining RESE", Parent.GetTimeResolution(), 0.5, 1e-15) && Passed;
  IDs = Parent.GetOriginIDs();
  Passed = EvaluateSize("RemoveRESE()", "the RESE with the IDs 2, 3", "There are two origin IDs", IDs.size(), 2) && Passed;
  Passed = EvaluateSize("RemoveRESE()", "the RESE with the IDs 2, 3", "The ID 3 is gone", IDs.count(3), 0) && Passed;

  Parent.RemoveRESEAndCompress(Late);
  Passed = EvaluateNear("RemoveRESEAndCompress()", "the last RESE", "The energy resolution is zero", Parent.GetEnergyResolution(), 0.0, 1e-15) && Passed;
  Passed = EvaluateSize("RemoveRESEAndCompress()", "the last RESE", "There are no origin IDs", Parent.GetOriginIDs().size(), 0) && Passed;

  // Without the permission to overwrite only the origin IDs are updated:
  MRESE Fixed;
  Fixed.SetEnergyResolution(7.0);
  Fixed.SetTimeResolution(9.0);
  Fixed.SetPositionResolution(MVector(1.0, 2.0, 3.0));
  Fixed.AllowOverwrite(false);
  Fixed.AddRESE(Late);
  Fixed.AddRESE(Early);
  Passed = EvaluateNear("AllowOverwrite()", "false, then two RESEs", "The energy resolution is unchanged", Fixed.GetEnergyResolution(), 7.0, 1e-15) && Passed;
  Passed = EvaluateNear("AllowOverwrite()", "false, then two RESEs", "The time resolution is unchanged", Fixed.GetTimeResolution(), 9.0, 1e-15) && Passed;
  Passed = EvaluateVectorNear("AllowOverwrite()", "false, then two RESEs", "The position resolution is unchanged", Fixed.GetPositionResolution(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
  Passed = EvaluateSize("AllowOverwrite()", "false, then two RESEs", "The origin IDs are updated", Fixed.GetOriginIDs().size(), 3) && Passed;

  // Adding at the front updates in the same way:
  MRESE Front;
  Front.AddRESEFirst(Late);
  Front.AddRESEFirst(Early);
  Passed = EvaluateNear("AddRESEFirst()", "energy resolutions 3 and 4 keV", "The energy resolution is 5 keV", Front.GetEnergyResolution(), 5.0, 1e-14) && Passed;
  Passed = EvaluateSize("AddRESEFirst()", "origin IDs 1, 2 and 2, 3", "There are three origin IDs", Front.GetOriginIDs().size(), 3) && Passed;

  delete Late;
  delete Early;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume sequence of a RESE and of a container
bool UTRESE::TestVolumeSequence()
{
  bool Passed = true;

  // The RESE takes over the volume sequence:
  MRESE Owner;
  MDVolumeSequence* Sequence = new MDVolumeSequence();
  Owner.SetVolumeSequence(Sequence);
  Passed = EvaluateTrue("SetVolumeSequence()", "new sequence", "The sequence is returned", Owner.GetVolumeSequence() == Sequence) && Passed;

  MGeometryRevan Geometry;
  bool Scanned = ScanTelescopeGeometry(Geometry);
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope geometry can be scanned", Scanned) && Passed;
  if (Scanned == false) {
    return false;
  }

  // Two hits in the lowest tracker wafer and one in the calorimeter:
  MREHit* TrackerA = CreateHit(MVector(1.0, 2.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* TrackerB = CreateHit(MVector(-2.0, 1.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Calorimeter = CreateHit(MVector(0.0, 0.0, -10.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  bool Created = false;
  if (TrackerA != nullptr && TrackerB != nullptr && Calorimeter != nullptr) {
    Created = true;
  }
  Passed = EvaluateTrue("CreateHit()", "telescope", "The hits can be created", Created) && Passed;
  if (Created == false) {
    delete TrackerA;
    delete TrackerB;
    delete Calorimeter;
    return false;
  }

  MRESE Parent;
  Parent.AddRESE(TrackerA);
  Passed = EvaluateTrue("AddRESE()", "first hit", "The container has its own volume sequence", Parent.GetVolumeSequence() != TrackerA->GetVolumeSequence()) && Passed;
  Passed = EvaluateTrue("AddRESE()", "first hit", "The container has the detector of the hit", Parent.GetVolumeSequence()->GetDetector() == TrackerA->GetVolumeSequence()->GetDetector()) && Passed;
  // World, holder, wafer:
  Passed = Evaluate("AddRESE()", "first hit", "The container has the three volumes of the hit", Parent.GetVolumeSequence()->GetNVolumes(), 3U) && Passed;

  Parent.AddRESE(TrackerB);
  Passed = EvaluateTrue("AddRESE()", "second hit in the same wafer", "The container keeps the detector", Parent.GetVolumeSequence()->GetDetector() == TrackerA->GetVolumeSequence()->GetDetector()) && Passed;
  Passed = Evaluate("AddRESE()", "second hit in the same wafer", "The container keeps the three volumes", Parent.GetVolumeSequence()->GetNVolumes(), 3U) && Passed;

  Parent.AddRESE(Calorimeter);
  Passed = EvaluateTrue("AddRESE()", "third hit in another detector", "The container has no detector", Parent.GetVolumeSequence()->GetDetector() == nullptr) && Passed;
  Passed = Evaluate("AddRESE()", "third hit in another detector", "Only the world volume is common", Parent.GetVolumeSequence()->GetNVolumes(), 1U) && Passed;

  // The only RESE of a container after a removal:
  MRESE Reused;
  Reused.AddRESE(Calorimeter);
  Reused.RemoveRESE(Calorimeter);
  Reused.AddRESE(TrackerA);
  // Defect REV-A-4: AddRESE counts the empty slot of a removed RESE and joins with the volume sequence of the removed RESE
  Passed = ExpectedToFail(EvaluateTrue("AddRESE()", "only RESE after a removal", "The container has the detector of its only RESE", Reused.GetVolumeSequence()->GetDetector() == TrackerA->GetVolumeSequence()->GetDetector())) && Passed;

  delete TrackerA;
  delete TrackerB;
  delete Calorimeter;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test removing RESEs
bool UTRESE::TestRemove()
{
  bool Passed = true;

  MRESE Parent;
  MRESE A;
  MRESE B;
  MRESE C;
  MRESE D;
  MRESE NotAdded;
  Parent.AddRESE(&A);
  Parent.AddRESE(&B);
  Parent.AddRESE(&C);
  Parent.AddRESE(&D);

  // Removing leaves an empty slot:
  Passed = EvaluateTrue("RemoveRESE(MRESE*)", "second of four", "The removed RESE is returned", Parent.RemoveRESE(&B) == &B) && Passed;
  Passed = Evaluate("RemoveRESE(MRESE*)", "second of four", "The number of slots is unchanged", Parent.GetNRESEs(), 4) && Passed;
  Passed = EvaluateTrue("RemoveRESE(MRESE*)", "second of four", "The slot is empty", Parent.GetRESEAt(1) == nullptr) && Passed;
  Passed = EvaluateFalse("RemoveRESE(MRESE*)", "second of four", "The RESE is not contained any more", Parent.ContainsRESE(&B)) && Passed;

  Parent.CompressRESEs();
  Passed = Evaluate("CompressRESEs()", "one empty slot", "Three RESEs are left", Parent.GetNRESEs(), 3) && Passed;
  Passed = EvaluateTrue("CompressRESEs()", "one empty slot", "The first RESE stays", Parent.GetRESEAt(0) == &A) && Passed;
  Passed = EvaluateTrue("CompressRESEs()", "one empty slot", "The third RESE moves up", Parent.GetRESEAt(1) == &C) && Passed;
  Passed = EvaluateTrue("CompressRESEs()", "one empty slot", "The fourth RESE moves up", Parent.GetRESEAt(2) == &D) && Passed;

  Passed = EvaluateTrue("RemoveRESEAndCompress(MRESE*)", "second of three", "The removed RESE is returned", Parent.RemoveRESEAndCompress(&C) == &C) && Passed;
  Passed = Evaluate("RemoveRESEAndCompress(MRESE*)", "second of three", "Two RESEs are left", Parent.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("RemoveRESEAndCompress(MRESE*)", "second of three", "The last RESE moves up", Parent.GetRESEAt(1) == &D) && Passed;

  // Now: A, D, B, C
  Parent.AddRESE(&B);
  Parent.AddRESE(&C);

  Passed = EvaluateTrue("RemoveRESEAt()", "index 0", "The removed RESE is returned", Parent.RemoveRESEAt(0) == &A) && Passed;
  Passed = Evaluate("RemoveRESEAt()", "index 0", "The number of slots is unchanged", Parent.GetNRESEs(), 4) && Passed;
  Passed = EvaluateTrue("RemoveRESEAt()", "index 0", "The slot is empty", Parent.GetRESEAt(0) == nullptr) && Passed;

  // Compressing also removes the empty slot from before - left: B, C
  Passed = EvaluateTrue("RemoveRESEAtAndCompress()", "index 1", "The removed RESE is returned", Parent.RemoveRESEAtAndCompress(1) == &D) && Passed;
  Passed = Evaluate("RemoveRESEAtAndCompress()", "index 1", "Two RESEs are left", Parent.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("RemoveRESEAtAndCompress()", "index 1", "The first of the remaining RESEs is first", Parent.GetRESEAt(0) == &B) && Passed;

  Passed = EvaluateTrue("RemoveRESE(int)", "ID of the first RESE", "The removed RESE is returned", Parent.RemoveRESE(B.GetID()) == &B) && Passed;
  Passed = Evaluate("RemoveRESE(int)", "ID of the first RESE", "The number of slots is unchanged", Parent.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("RemoveRESE(int)", "ID of the first RESE", "The slot is empty", Parent.GetRESEAt(0) == nullptr) && Passed;

  Passed = EvaluateTrue("RemoveRESEAndCompress(int)", "ID of the last RESE", "The removed RESE is returned", Parent.RemoveRESEAndCompress(C.GetID()) == &C) && Passed;
  Passed = Evaluate("RemoveRESEAndCompress(int)", "ID of the last RESE", "No RESE is left", Parent.GetNRESEs(), 0) && Passed;

  // Remove all:
  Parent.AddRESE(&A);
  Parent.AddRESE(&B);
  Parent.RemoveAll();
  Passed = Evaluate("RemoveAll()", "two RESEs", "The number of slots is unchanged", Parent.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("RemoveAll()", "two RESEs", "The first slot is empty", Parent.GetRESEAt(0) == nullptr) && Passed;
  Passed = EvaluateTrue("RemoveAll()", "two RESEs", "The second slot is empty", Parent.GetRESEAt(1) == nullptr) && Passed;
  Parent.RemoveAllAndCompress();
  Passed = Evaluate("RemoveAllAndCompress()", "two empty slots", "No slot is left", Parent.GetNRESEs(), 0) && Passed;
  Parent.AddRESE(&A);
  Parent.AddRESE(&B);
  Parent.RemoveAllAndCompress();
  Passed = Evaluate("RemoveAllAndCompress()", "two RESEs", "No slot is left", Parent.GetNRESEs(), 0) && Passed;
  Parent.RemoveAllAndCompress();
  Passed = Evaluate("RemoveAllAndCompress()", "empty", "Removing from an empty container changes nothing", Parent.GetNRESEs(), 0) && Passed;

  // What is not contained cannot be removed:
  Parent.AddRESE(&A);
  DisableDefaultStreams();
  MRESE* NotContained = Parent.RemoveRESE(&NotAdded);
  MRESE* OutOfBounds = Parent.RemoveRESEAt(1);
  MRESE* UnknownID = Parent.RemoveRESE(NotAdded.GetID());
  MRESE* OutOfBoundsCompress = Parent.RemoveRESEAtAndCompress(1);
  MRESE* UnknownIDCompress = Parent.RemoveRESEAndCompress(NotAdded.GetID());
  EnableDefaultStreams();
  // Defect REV-A-5: RemoveRESE returns the RESE it was given, also if the RESE is not contained and nothing was removed
  Passed = ExpectedToFail(EvaluateTrue("RemoveRESE(MRESE*)", "RESE which is not contained", "A nullptr is returned", NotContained == nullptr)) && Passed;
  Passed = EvaluateTrue("RemoveRESEAt()", "index behind the last RESE", "A nullptr is returned", OutOfBounds == nullptr) && Passed;
  Passed = EvaluateTrue("RemoveRESE(int)", "unknown ID", "A nullptr is returned", UnknownID == nullptr) && Passed;
  Passed = EvaluateTrue("RemoveRESEAtAndCompress()", "index behind the last RESE", "A nullptr is returned", OutOfBoundsCompress == nullptr) && Passed;
  Passed = EvaluateTrue("RemoveRESEAndCompress(int)", "unknown ID", "A nullptr is returned", UnknownIDCompress == nullptr) && Passed;
  Passed = Evaluate("RemoveRESE()", "five failed removals", "The RESE is still contained", Parent.GetNRESEs(), 1) && Passed;
  Passed = EvaluateTrue("RemoveRESE()", "five failed removals", "The RESE is still the first one", Parent.GetRESEAt(0) == &A) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test deleting RESEs and the ownership
bool UTRESE::TestDelete()
{
  bool Passed = true;

  const int Before = UTRESECounted::m_NumberOfLivingObjects;

  MRESE Parent;
  UTRESECounted* C1 = new UTRESECounted();
  UTRESECounted* C2 = new UTRESECounted();
  UTRESECounted* C3 = new UTRESECounted();
  UTRESECounted* C4 = new UTRESECounted();
  UTRESECounted* C5 = new UTRESECounted();
  UTRESECounted* C6 = new UTRESECounted();
  const int ID5 = C5->GetID();
  const int ID6 = C6->GetID();
  Parent.AddRESE(C1);
  Parent.AddRESE(C2);
  Parent.AddRESE(C3);
  Parent.AddRESE(C4);
  Parent.AddRESE(C5);
  Parent.AddRESE(C6);

  Parent.DeleteRESE(C1);
  Passed = Evaluate("DeleteRESE(MRESE*)", "first of six", "The RESE is deleted", UTRESECounted::m_NumberOfLivingObjects - Before, 5) && Passed;
  Passed = Evaluate("DeleteRESE(MRESE*)", "first of six", "The number of slots is unchanged", Parent.GetNRESEs(), 6) && Passed;
  Passed = EvaluateTrue("DeleteRESE(MRESE*)", "first of six", "The slot is empty", Parent.GetRESEAt(0) == nullptr) && Passed;

  // Left: C3, C4, C5, C6
  Parent.DeleteRESEAndCompress(C2);
  Passed = Evaluate("DeleteRESEAndCompress(MRESE*)", "second of six", "The RESE is deleted", UTRESECounted::m_NumberOfLivingObjects - Before, 4) && Passed;
  Passed = Evaluate("DeleteRESEAndCompress(MRESE*)", "second of six", "Four RESEs are left", Parent.GetNRESEs(), 4) && Passed;
  Passed = EvaluateTrue("DeleteRESEAndCompress(MRESE*)", "second of six", "The third RESE is now the first", Parent.GetRESEAt(0) == C3) && Passed;

  Parent.DeleteRESEAt(0);
  Passed = Evaluate("DeleteRESEAt()", "index 0", "The RESE is deleted", UTRESECounted::m_NumberOfLivingObjects - Before, 3) && Passed;
  Passed = Evaluate("DeleteRESEAt()", "index 0", "The number of slots is unchanged", Parent.GetNRESEs(), 4) && Passed;
  Passed = EvaluateTrue("DeleteRESEAt()", "index 0", "The slot is empty", Parent.GetRESEAt(0) == nullptr) && Passed;

  // Left: C5, C6
  Parent.DeleteRESEAtAndCompress(1);
  Passed = Evaluate("DeleteRESEAtAndCompress()", "index 1", "The RESE is deleted", UTRESECounted::m_NumberOfLivingObjects - Before, 2) && Passed;
  Passed = Evaluate("DeleteRESEAtAndCompress()", "index 1", "Two RESEs are left", Parent.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("DeleteRESEAtAndCompress()", "index 1", "The fifth RESE is now the first", Parent.GetRESEAt(0) == C5) && Passed;

  Parent.DeleteRESE(ID5);
  Passed = Evaluate("DeleteRESE(int)", "ID of the first RESE", "The RESE is deleted", UTRESECounted::m_NumberOfLivingObjects - Before, 1) && Passed;
  Passed = Evaluate("DeleteRESE(int)", "ID of the first RESE", "The number of slots is unchanged", Parent.GetNRESEs(), 2) && Passed;

  Parent.DeleteRESEAndCompress(ID6);
  Passed = Evaluate("DeleteRESEAndCompress(int)", "ID of the last RESE", "The RESE is deleted", UTRESECounted::m_NumberOfLivingObjects - Before, 0) && Passed;
  Passed = Evaluate("DeleteRESEAndCompress(int)", "ID of the last RESE", "No RESE is left", Parent.GetNRESEs(), 0) && Passed;

  // Delete all, through all levels:
  UTRESECounted* Middle = new UTRESECounted();
  UTRESECounted* Leaf = new UTRESECounted();
  UTRESECounted* Single = new UTRESECounted();
  Middle->AddRESE(Leaf);
  Parent.AddRESE(Middle);
  Parent.AddRESE(Single);
  Parent.DeleteAll();
  Passed = Evaluate("DeleteAll()", "two RESEs, one with a RESE of its own", "All three RESEs are deleted", UTRESECounted::m_NumberOfLivingObjects - Before, 0) && Passed;
  // The header states that the array is not shrunk:
  Passed = Evaluate("DeleteAll()", "two RESEs, one with a RESE of its own", "The number of slots is unchanged", Parent.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("DeleteAll()", "two RESEs, one with a RESE of its own", "The first slot is empty", Parent.GetRESEAt(0) == nullptr) && Passed;
  Parent.DeleteAll();
  Passed = Evaluate("DeleteAll()", "only empty slots", "Deleting again changes nothing", Parent.GetNRESEs(), 2) && Passed;

  // The destructor does not delete the RESEs:
  UTRESECounted* Survivor = new UTRESECounted();
  MRESE* Temporary = new MRESE();
  Temporary->AddRESE(Survivor);
  delete Temporary;
  Passed = Evaluate("~MRESE()", "container with one RESE", "The RESE is not deleted with its container", UTRESECounted::m_NumberOfLivingObjects - Before, 1) && Passed;
  delete Survivor;

  // Defect REV-A-6: DeleteRESEAt and DeleteRESE by ID dereference the nullptr which they get for an index or ID which does not exist
  Passed = ExpectedToCrash(EvaluateTrue("DeleteRESEAt()", "index behind the last RESE", "Nothing is deleted", DeleteAtIndexOutOfBounds())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("DeleteRESE(int)", "unknown ID", "Nothing is deleted", DeleteUnknownID())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test resetting a RESE
bool UTRESE::TestReset()
{
  bool Passed = true;

  const int Before = UTRESECounted::m_NumberOfLivingObjects;

  UTRESECounted RESE;
  MRESE Link;
  RESE.SetPosition(1.0, 2.0, 3.0);
  RESE.SetPositionResolution(MVector(0.1, 0.2, 0.3));
  RESE.SetEnergy(100.0);
  RESE.SetAdditionalEnergy(5.0);
  RESE.SetEnergyResolution(2.0);
  RESE.SetTime(4.0);
  RESE.SetTimeResolution(0.5);
  RESE.SetDetector(MDDetector::c_Strip2D);
  RESE.SetNoiseFlags(" OVERFLOW");
  RESE.SetValid(false);
  RESE.AllowOverwrite(false);
  RESE.AddLink(&Link);
  RESE.AddRESE(new UTRESECounted());
  RESE.AddOriginID(5);

  RESE.Reset();

  Passed = Evaluate("Reset()", "filled RESE", "The contained RESE is deleted", UTRESECounted::m_NumberOfLivingObjects - Before, 1) && Passed;
  Passed = Evaluate("Reset()", "filled RESE", "There are no RESEs", RESE.GetNRESEs(), 0) && Passed;
  Passed = Evaluate("Reset()", "filled RESE", "There are no links", RESE.GetNLinks(), 0) && Passed;
  Passed = EvaluateVectorNear("Reset()", "filled RESE", "The position is the origin", RESE.GetPosition(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("Reset()", "filled RESE", "The position resolution is zero", RESE.GetPositionResolution(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateNear("Reset()", "filled RESE", "The energy and the additional energy are zero", RESE.GetEnergy(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("Reset()", "filled RESE", "The energy resolution is zero", RESE.GetEnergyResolution(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("Reset()", "filled RESE", "The time is zero", RESE.GetTime(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("Reset()", "filled RESE", "The time resolution is zero", RESE.GetTimeResolution(), 0.0, 1e-15) && Passed;
  Passed = Evaluate("Reset()", "filled RESE", "There is no detector type", RESE.GetDetector(), MDDetector::c_NoDetectorType) && Passed;
  Passed = Evaluate("Reset()", "filled RESE", "There are no noise flags", RESE.GetNoiseFlags(), MString("")) && Passed;
  Passed = EvaluateTrue("Reset()", "filled RESE", "The RESE is valid", RESE.IsValid()) && Passed;
  Passed = EvaluateTrue("Reset()", "filled RESE", "Overwriting is allowed", RESE.AllowOverwrite()) && Passed;
  Passed = EvaluateSize("Reset()", "filled RESE", "There are no origin IDs", RESE.GetOriginIDs().size(), 0) && Passed;
  Passed = EvaluateTrue("Reset()", "filled RESE", "There is a volume sequence", RESE.GetVolumeSequence() != nullptr) && Passed;
  if (RESE.GetVolumeSequence() != nullptr) {
    Passed = Evaluate("Reset()", "filled RESE", "The volume sequence is empty", RESE.GetVolumeSequence()->GetNVolumes(), 0U) && Passed;
  }

  // The resolutions without RESEs:
  RESE.SetEnergyResolution(3.0);
  RESE.SetPositionResolution(MVector(1.0, 1.0, 1.0));
  RESE.RecalculateResolutions();
  Passed = EvaluateNear("RecalculateResolutions()", "no RESEs", "The energy resolution is zero", RESE.GetEnergyResolution(), 0.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the random shuffling
bool UTRESE::TestShuffle()
{
  bool Passed = true;

  const unsigned int Size = 6;
  const unsigned int Seed = 4711;

  // The documented procedure: twice as many swaps of two random slots as there are RESEs
  // The reference does them with indices, drawing the source before the target
  vector<unsigned int> Expected;
  for (unsigned int i = 0; i < Size; ++i) {
    Expected.push_back(i);
  }
  gRandom->SetSeed(Seed);
  for (unsigned int i = 0; i < 2*Size; ++i) {
    unsigned int From = gRandom->Integer(Size);
    unsigned int To = gRandom->Integer(Size);
    swap(Expected[To], Expected[From]);
  }

  MRESE Parent;
  vector<MRESE*> RESEs;
  for (unsigned int i = 0; i < Size; ++i) {
    RESEs.push_back(new MRESE());
    Parent.AddRESE(RESEs[i]);
  }

  gRandom->SetSeed(Seed);
  Parent.Shuffle();

  Passed = Evaluate("Shuffle()", "six RESEs", "The number of RESEs is unchanged", Parent.GetNRESEs(), static_cast<int>(Size)) && Passed;
  bool IsPermutation = true;
  bool IsExpectedOrder = true;
  bool IsOriginalOrder = true;
  for (unsigned int i = 0; i < Size; ++i) {
    if (Parent.ContainsRESE(RESEs[i]) == false) {
      IsPermutation = false;
    }
    if (Parent.GetRESEAt(i) != RESEs[Expected[i]]) {
      IsExpectedOrder = false;
    }
    if (Parent.GetRESEAt(i) != RESEs[i]) {
      IsOriginalOrder = false;
    }
  }
  Passed = EvaluateTrue("Shuffle()", "six RESEs", "All RESEs are still contained", IsPermutation) && Passed;
  Passed = EvaluateTrue("Shuffle()", "six RESEs, seed 4711", "The order is the one of twelve random swaps", IsExpectedOrder) && Passed;
  Passed = EvaluateFalse("Shuffle()", "six RESEs, seed 4711", "The order has changed", IsOriginalOrder) && Passed;

  for (unsigned int i = 0; i < Size; ++i) {
    delete RESEs[i];
  }

  MRESE Empty;
  Empty.Shuffle();
  Passed = Evaluate("Shuffle()", "no RESEs", "An empty container stays empty", Empty.GetNRESEs(), 0) && Passed;

  MRESE One;
  MRESE Only;
  One.AddRESE(&Only);
  One.Shuffle();
  Passed = EvaluateTrue("Shuffle()", "one RESE", "The only RESE stays", One.GetRESEAt(0) == &Only) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the functions which the derived classes have to implement
bool UTRESE::TestBaseImplementations()
{
  bool Passed = true;

  MRESE A;
  MRESE B;

  // The base class has no distance: it reports an error and returns the largest value
  DisableDefaultStreams();
  double Distance = A.ComputeMinDistance(&B);
  MVector DistanceVector = A.ComputeMinDistanceVector(&B);
  bool Adjacent = A.AreAdjacent(&B);
  EnableDefaultStreams();
  const double Largest = numeric_limits<double>::max();
  Passed = EvaluateTrue("ComputeMinDistance()", "base class", "The largest double is returned", Distance == Largest) && Passed;
  Passed = EvaluateTrue("ComputeMinDistanceVector()", "base class", "A vector of the largest doubles is returned", DistanceVector.X() == Largest && DistanceVector.Y() == Largest && DistanceVector.Z() == Largest) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "base class", "Two plain RESEs are not adjacent", Adjacent) && Passed;

  // Defect REV-A-7: The text of ToString is misspelled as "Unkown RESE"
  Passed = ExpectedToFail(Evaluate("ToString()", "base class", "The text names an unknown RESE", A.ToString(), MString("Unknown RESE"))) && Passed;

  // The evta text of a plain RESE is the one of its RESEs:
  Passed = Evaluate("ToEvtaString()", "no RESEs", "The text is empty", A.ToEvtaString(5, 1), MString("")) && Passed;

  MREHit First;
  First.SetPosition(1.0, 2.0, 3.0);
  First.SetEnergy(100.0);
  First.SetDetector(MDDetector::c_Strip2D);
  MREHit Second;
  Second.SetPosition(-4.0, 5.0, -6.0);
  Second.SetEnergy(250.0);
  Second.SetDetector(MDDetector::c_Calorimeter);
  A.AddRESE(&First);
  A.AddRESE(&Second);
  Passed = Evaluate("ToEvtaString()", "two hits", "The text is the one of the first hit followed by the one of the second hit", A.ToEvtaString(5, 1), First.ToEvtaString(5, 1) + Second.ToEvtaString(5, 1)) && Passed;
  Passed = EvaluateFalse("ToEvtaString()", "two hits", "The text is not empty", A.ToEvtaString(5, 1).IsEmpty()) && Passed;

  // Defect REV-A-1: ToEvtaString dereferences the empty slot which RemoveRESE leaves behind
  Passed = ExpectedToCrash(EvaluateTrue("ToEvtaString()", "empty slot", "The text is the one of the remaining hit", ToEvtaStringWithEmptySlot())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the duplication
bool UTRESE::TestDuplicate()
{
  bool Passed = true;

  MRESE Source(MVector(1.5, -2.5, 3.5), 511.0, 0.25, MDDetector::c_Calorimeter, MVector(0.1, 0.2, 0.3), 12.5, 1e-6);
  Source.SetAdditionalEnergy(7.0);
  Source.SetNoiseFlags(" OVERFLOW");
  Source.SetValid(false);
  Source.SetGridPoint(MDGridPoint(3, 4, 5, MDGridPoint::c_Voxel, MVector(0.5, 1.5, 2.5), 77.0, 0.125));

  // Two RESEs which are linked with each other, and a link to the outside:
  MRESE* ChildA = new MRESE();
  ChildA->AddOriginID(4);
  MRESE* ChildB = new MRESE();
  ChildB->AddOriginID(9);
  ChildA->AddLink(ChildB);
  ChildB->AddLink(ChildA);
  Source.AddRESE(ChildA);
  Source.AddRESE(ChildB);
  MRESE Outside;
  Source.AddLink(&Outside);

  // Set the resolutions again, since adding has recalculated them
  Source.SetPositionResolution(MVector(0.1, 0.2, 0.3));
  Source.SetEnergyResolution(12.5);
  Source.SetTimeResolution(1e-6);
  Source.AllowOverwrite(false);

  MRESE* Copy = Source.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "filled RESE", "A new object is returned", Copy != nullptr && Copy != &Source) && Passed;
  if (Copy == nullptr) {
    delete ChildA;
    delete ChildB;
    return false;
  }

  Passed = Evaluate("Duplicate()", "filled RESE", "The ID is the same", Copy->GetID(), Source.GetID()) && Passed;
  Passed = Evaluate("Duplicate()", "filled RESE", "The type is the same", Copy->GetType(), Source.GetType()) && Passed;
  Passed = EvaluateVectorNear("Duplicate()", "filled RESE", "The position is copied", Copy->GetPosition(), MVector(1.5, -2.5, 3.5), 1e-15) && Passed;
  // 511 keV plus 7 keV additional energy:
  Passed = EvaluateNear("Duplicate()", "filled RESE", "The energy and the additional energy are copied", Copy->GetEnergy(), 518.0, 1e-12) && Passed;
  Passed = EvaluateNear("Duplicate()", "filled RESE", "The time is copied", Copy->GetTime(), 0.25, 1e-15) && Passed;
  Passed = Evaluate("Duplicate()", "filled RESE", "The detector type is copied", Copy->GetDetector(), MDDetector::c_Calorimeter) && Passed;
  Passed = EvaluateVectorNear("Duplicate()", "filled RESE", "The position resolution is copied", Copy->GetPositionResolution(), MVector(0.1, 0.2, 0.3), 1e-15) && Passed;
  Passed = EvaluateNear("Duplicate()", "filled RESE", "The energy resolution is copied", Copy->GetEnergyResolution(), 12.5, 1e-15) && Passed;
  Passed = EvaluateNear("Duplicate()", "filled RESE", "The time resolution is copied", Copy->GetTimeResolution(), 1e-6, 1e-21) && Passed;
  Passed = Evaluate("Duplicate()", "filled RESE", "The noise flags are copied", Copy->GetNoiseFlags(), MString(" OVERFLOW")) && Passed;
  Passed = EvaluateFalse("Duplicate()", "filled RESE", "The valid flag is copied", Copy->IsValid()) && Passed;
  Passed = EvaluateFalse("Duplicate()", "filled RESE", "The overwrite flag is copied", Copy->AllowOverwrite()) && Passed;
  Passed = EvaluateSize("Duplicate()", "filled RESE", "The origin IDs are copied", Copy->GetOriginIDs().size(), 2) && Passed;
  Passed = EvaluateTrue("Duplicate()", "filled RESE", "The copy has its own volume sequence", Copy->GetVolumeSequence() != nullptr && Copy->GetVolumeSequence() != Source.GetVolumeSequence()) && Passed;
  // Defect REV-A-8: The copy constructor does not copy the grid point
  Passed = ExpectedToFail(Evaluate("Duplicate()", "filled RESE", "The grid point is copied", Copy->GetGridPoint().GetXGrid(), 3U)) && Passed;

  // The links of the RESE itself cannot be resolved by the copy, as stated in the copy constructor:
  Passed = Evaluate("Duplicate()", "filled RESE", "The copy has no links", Copy->GetNLinks(), 0) && Passed;

  // The RESEs are duplicated, with the links between them:
  Passed = Evaluate("Duplicate()", "filled RESE", "The copy has two RESEs", Copy->GetNRESEs(), 2) && Passed;
  if (Copy->GetNRESEs() == 2) {
    MRESE* CopyA = Copy->GetRESEAt(0);
    MRESE* CopyB = Copy->GetRESEAt(1);
    Passed = EvaluateTrue("Duplicate()", "filled RESE", "The first RESE is a new object", CopyA != ChildA && CopyA != nullptr) && Passed;
    Passed = EvaluateTrue("Duplicate()", "filled RESE", "The second RESE is a new object", CopyB != ChildB && CopyB != nullptr) && Passed;
    if (CopyA != nullptr && CopyB != nullptr) {
      Passed = Evaluate("Duplicate()", "filled RESE", "The first RESE keeps its ID", CopyA->GetID(), ChildA->GetID()) && Passed;
      Passed = Evaluate("Duplicate()", "filled RESE", "The second RESE keeps its ID", CopyB->GetID(), ChildB->GetID()) && Passed;
      Passed = Evaluate("Duplicate()", "filled RESE", "The first RESE has one link", CopyA->GetNLinks(), 1) && Passed;
      Passed = EvaluateTrue("Duplicate()", "filled RESE", "The first RESE is linked to the copy of the second", CopyA->IsLink(CopyB)) && Passed;
      Passed = EvaluateTrue("Duplicate()", "filled RESE", "The second RESE is linked to the copy of the first", CopyB->IsLink(CopyA)) && Passed;
    }
  }

  // The copy is independent:
  Copy->SetEnergy(1.0);
  Copy->SetPosition(0.0, 0.0, 0.0);
  Passed = EvaluateNear("Duplicate()", "copy changed", "The energy of the original is unchanged", Source.GetEnergy(), 518.0, 1e-12) && Passed;
  Passed = EvaluateVectorNear("Duplicate()", "copy changed", "The position of the original is unchanged", Source.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-15) && Passed;
  Passed = Evaluate("Duplicate()", "copy changed", "The original keeps its link", Source.GetNLinks(), 1) && Passed;

  Copy->DeleteAll();
  delete Copy;

  // The copy constructor does the same:
  MRESE Direct(&Source);
  Passed = Evaluate("MRESE(MRESE*)", "filled RESE", "The ID is the same", Direct.GetID(), Source.GetID()) && Passed;
  Passed = EvaluateNear("MRESE(MRESE*)", "filled RESE", "The energy and the additional energy are copied", Direct.GetEnergy(), 518.0, 1e-12) && Passed;
  Passed = Evaluate("MRESE(MRESE*)", "filled RESE", "The copy has two RESEs", Direct.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("MRESE(MRESE*)", "filled RESE", "The RESEs are new objects", Direct.GetRESEAt(0) != ChildA && Direct.GetRESEAt(1) != ChildB) && Passed;
  Direct.DeleteAll();

  // An empty slot is kept - a build with assertions stops in the list
  ChildA->RemoveAllLinks();
  ChildB->RemoveAllLinks();
  Source.RemoveRESE(ChildA);
  MRESE* WithSlot = Source.Duplicate();
  Passed = Evaluate("Duplicate()", "empty slot", "The copy has two slots", WithSlot->GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("Duplicate()", "empty slot", "The first slot of the copy is empty", WithSlot->GetRESEAt(0) == nullptr) && Passed;
  Passed = EvaluateTrue("Duplicate()", "empty slot", "The second slot of the copy is a new object", WithSlot->GetRESEAt(1) != nullptr && WithSlot->GetRESEAt(1) != ChildB) && Passed;
  WithSlot->DeleteAll();
  delete WithSlot;

  delete ChildA;
  delete ChildB;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the conversion to a physical event hit
bool UTRESE::TestCreatePhysicalEventHit()
{
  bool Passed = true;

  MRESE RESE(MVector(1.5, -2.5, 3.5), 511.0, 0.25, MDDetector::c_Calorimeter, MVector(0.1, 0.2, 0.3), 12.5, 0.5);
  MPhysicalEventHit Hit = RESE.CreatePhysicalEventHit();

  Passed = EvaluateVectorNear("CreatePhysicalEventHit()", "filled RESE", "The position is handed over", Hit.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-15) && Passed;
  Passed = EvaluateVectorNear("CreatePhysicalEventHit()", "filled RESE", "The position resolution is handed over", Hit.GetPositionUncertainty(), MVector(0.1, 0.2, 0.3), 1e-15) && Passed;
  Passed = EvaluateNear("CreatePhysicalEventHit()", "filled RESE", "The energy is handed over", Hit.GetEnergy(), 511.0, 1e-15) && Passed;
  Passed = EvaluateNear("CreatePhysicalEventHit()", "filled RESE", "The energy resolution is handed over", Hit.GetEnergyUncertainty(), 12.5, 1e-15) && Passed;
  // The time is stored with nanoseconds:
  Passed = EvaluateNear("CreatePhysicalEventHit()", "filled RESE", "The time is handed over", Hit.GetTime().GetAsDouble(), 0.25, 1e-9) && Passed;
  Passed = EvaluateNear("CreatePhysicalEventHit()", "filled RESE", "The time resolution is handed over", Hit.GetTimeUncertainty().GetAsDouble(), 0.5, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the sorting by z
bool UTRESE::TestCompareRESEByZ()
{
  bool Passed = true;

  MRESE Low;
  Low.SetPosition(0.0, 0.0, -3.0);
  MRESE Middle;
  Middle.SetPosition(5.0, 5.0, 0.5);
  MRESE High;
  High.SetPosition(-1.0, 0.0, 7.0);
  MRESE AlsoHigh;
  AlsoHigh.SetPosition(9.0, 9.0, 7.0);

  CompareRESEByZ Compare;
  Passed = EvaluateTrue("CompareRESEByZ", "z = 7 and z = 0.5", "The RESE with the larger z comes first", Compare(&High, &Middle)) && Passed;
  Passed = EvaluateFalse("CompareRESEByZ", "z = 0.5 and z = 7", "The RESE with the smaller z does not come first", Compare(&Middle, &High)) && Passed;
  Passed = EvaluateFalse("CompareRESEByZ", "both z = 7", "Two RESEs with the same z are equivalent", Compare(&High, &AlsoHigh)) && Passed;
  Passed = EvaluateFalse("CompareRESEByZ", "both z = 7, swapped", "Two RESEs with the same z are equivalent in both orders", Compare(&AlsoHigh, &High)) && Passed;

  vector<MRESE*> List = { &Middle, &Low, &High };
  sort(List.begin(), List.end(), CompareRESEByZ());
  Passed = EvaluateTrue("CompareRESEByZ", "sorting z = 0.5, -3, 7", "The highest RESE is first", List[0] == &High) && Passed;
  Passed = EvaluateTrue("CompareRESEByZ", "sorting z = 0.5, -3, 7", "The middle RESE is second", List[1] == &Middle) && Passed;
  Passed = EvaluateTrue("CompareRESEByZ", "sorting z = 0.5, -3, 7", "The lowest RESE is last", List[2] == &Low) && Passed;

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

  UTRESE Test;
  return Test.Run() == true ? 0 : 1;
}


// UTRESE.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
