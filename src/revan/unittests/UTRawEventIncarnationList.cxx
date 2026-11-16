/*
 * UTRawEventIncarnationList.cxx
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
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TError.h"

// MEGAlib libs:
#include "MExceptions.h"
#include "MGeometryRevan.h"
#include "MGlobal.h"
#include "MMultiEvent.h"
#include "MPETEvent.h"
#include "MPhotoEvent.h"
#include "MPhysicalEvent.h"
#include "MRawEventIncarnationList.h"
#include "MRawEventIncarnations.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MString.h"
#include "MTime.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the list of raw event incarnations MRawEventIncarnationList
class UTRawEventIncarnationList : public MUnitTest
{
public:
  //! Default constructor
  UTRawEventIncarnationList() : MUnitTest("UTRawEventIncarnationList") {}
  //! Default destructor
  virtual ~UTRawEventIncarnationList() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the state after the construction
  bool TestDefaults();
  //! Test adding, reading, removing, and deleting incarnations
  bool TestAddGetRemoveDelete();
  //! Test the optimum and the best try events
  bool TestOptimumAndBestTryEvents();
  //! Test the search for a valid event
  bool TestIsAnyEventValid();
  //! Test the physical event of one incarnation
  bool TestSinglePhysicalEvent();
  //! Test the multi event of several incarnations
  bool TestMultiEvent();
  //! Test the PET event of two incarnations with 511 keV
  bool TestPETEvent();
  //! Test the limits of the energy window of the PET event
  bool TestPETEnergyWindow();
  //! Test the multi event which contains a PET event
  bool TestMultiEventWithPET();
  //! Test the text output
  bool TestToString();

  //! Return new incarnations with one good photo event with one hit - they own the raw event, the caller owns them
  //! The raw event is the optimum event, or (if not) the best try event
  MRawEventIncarnations* CreateIncarnations(double Energy, double EnergyResolution, const MVector& Position, bool IsOptimum = true, double Time = 0.0);
  //! Return the physical event of the list: the optimum one, or (if not) the best try one
  MPhysicalEvent* GetPhysicalEvent(MRawEventIncarnationList& List, bool IsOptimum);
  //! Return true if removing incarnations which are not in the list changes nothing
  bool IsRemoveOfUnknownIncarnationsIgnored();

  //! The ID of all test events
  static const unsigned long c_EventID = 42;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRawEventIncarnationList::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestAddGetRemoveDelete() && Passed;
  Passed = TestOptimumAndBestTryEvents() && Passed;
  Passed = TestIsAnyEventValid() && Passed;
  Passed = TestSinglePhysicalEvent() && Passed;
  Passed = TestMultiEvent() && Passed;
  Passed = TestPETEvent() && Passed;
  Passed = TestPETEnergyWindow() && Passed;
  Passed = TestMultiEventWithPET() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the construction
bool UTRawEventIncarnationList::TestDefaults()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  vector<MString> Names = { "default constructor", "constructor with geometry" };
  for (unsigned int c = 0; c < Names.size(); ++c) {
    MRawEventIncarnationList* List = nullptr;
    if (c == 0) {
      List = new MRawEventIncarnationList();
    } else {
      List = new MRawEventIncarnationList(&Geometry);
    }
    Passed = Evaluate("Size()", Names[c], "The list is empty", List->Size(), 0U) && Passed;
    Passed = EvaluateFalse("HasOnlyOptimumEvents()", Names[c], "An empty list has no optimum events", List->HasOnlyOptimumEvents()) && Passed;
    Passed = EvaluateFalse("HasBestTryEvents()", Names[c], "An empty list has no best try events", List->HasBestTryEvents()) && Passed;
    Passed = EvaluateSize("GetOptimumEvents()", Names[c], "There are no optimum events", List->GetOptimumEvents().size(), 0) && Passed;
    Passed = EvaluateSize("GetBestTryEvents()", Names[c], "There are no best try events", List->GetBestTryEvents().size(), 0) && Passed;
    Passed = EvaluateTrue("GetOptimumPhysicalEvent()", Names[c], "There is no optimum physical event", List->GetOptimumPhysicalEvent() == nullptr) && Passed;
    Passed = EvaluateTrue("GetBestTryPhysicalEvent()", Names[c], "There is no best try physical event", List->GetBestTryPhysicalEvent() == nullptr) && Passed;
    Passed = EvaluateFalse("IsAnyEventValid()", Names[c], "No event is valid", List->IsAnyEventValid()) && Passed;
    Passed = Evaluate("ToString()", Names[c], "The text has only the head line", List->ToString(), MString("Raw event collection with the following incarnations:\n")) && Passed;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("Get()", Names[c], "Reading from an empty list throws an index exception", [List]() { List->Get(0); }) && Passed;

    // An empty list can be emptied:
    List->RemoveAll();
    List->DeleteAll();
    Passed = Evaluate("DeleteAll()", Names[c], "An empty list stays empty", List->Size(), 0U) && Passed;

    // The geometry can be set later:
    List->SetGeometry(&Geometry);
    Passed = Evaluate("SetGeometry()", Names[c], "Setting the geometry does not change the list", List->Size(), 0U) && Passed;

    delete List;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding, reading, removing, and deleting incarnations
bool UTRawEventIncarnationList::TestAddGetRemoveDelete()
{
  bool Passed = true;

  MRawEventIncarnationList List;
  MRawEventIncarnations* A = CreateIncarnations(100.0, 1.0, MVector(1.0, 0.0, 0.0));
  MRawEventIncarnations* B = CreateIncarnations(200.0, 1.0, MVector(2.0, 0.0, 0.0));
  MRawEventIncarnations* C = CreateIncarnations(300.0, 1.0, MVector(3.0, 0.0, 0.0));

  List.Add(A);
  Passed = Evaluate("Add()", "one", "The size is one", List.Size(), 1U) && Passed;
  Passed = EvaluateTrue("Get()", "one, index 0", "The incarnations are returned", List.Get(0) == A) && Passed;
  List.Add(B);
  List.Add(C);
  Passed = Evaluate("Add()", "three", "The size is three", List.Size(), 3U) && Passed;
  Passed = EvaluateTrue("Get()", "three, index 0", "The first incarnations are returned", List.Get(0) == A) && Passed;
  Passed = EvaluateTrue("Get()", "three, index 1", "The second incarnations are returned", List.Get(1) == B) && Passed;
  Passed = EvaluateTrue("Get()", "three, index 2", "The third incarnations are returned", List.Get(2) == C) && Passed;
  MRawEventIncarnationList* Pointer = &List;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("Get()", "three, index 3", "The index behind the end throws an index exception", [Pointer]() { Pointer->Get(3); }) && Passed;

  // Remove from the middle - the incarnations and their raw events are not deleted:
  List.Remove(B);
  Passed = Evaluate("Remove()", "middle", "The size is two", List.Size(), 2U) && Passed;
  Passed = EvaluateTrue("Remove()", "middle", "The first incarnations stay", List.Get(0) == A) && Passed;
  Passed = EvaluateTrue("Remove()", "middle", "The third incarnations move up", List.Get(1) == C) && Passed;
  Passed = Evaluate("Remove()", "middle", "The removed incarnations still have their raw event", B->GetNRawEvents(), 1) && Passed;
  // Defect REV-C-6: Remove and Delete erase the end of the list if the incarnations are not in the list
  Passed = ExpectedToCrash(EvaluateTrue("Remove()", "incarnations which are not in the list", "Removing unknown incarnations changes nothing", IsRemoveOfUnknownIncarnationsIgnored())) && Passed;

  // Delete - the incarnations and their raw events are deleted:
  List.Delete(A);
  Passed = Evaluate("Delete()", "first", "The size is one", List.Size(), 1U) && Passed;
  Passed = EvaluateTrue("Delete()", "first", "The third incarnations are the only ones", List.Get(0) == C) && Passed;

  // Remove all:
  List.Add(B);
  List.RemoveAll();
  Passed = Evaluate("RemoveAll()", "two", "The list is empty", List.Size(), 0U) && Passed;
  Passed = Evaluate("RemoveAll()", "two", "The removed incarnations still have their raw event", C->GetNRawEvents(), 1) && Passed;

  // Delete all:
  List.Add(B);
  List.Add(C);
  List.DeleteAll();
  Passed = Evaluate("DeleteAll()", "two", "The list is empty", List.Size(), 0U) && Passed;

  // The list can be used again:
  MRawEventIncarnations* D = CreateIncarnations(400.0, 1.0, MVector(4.0, 0.0, 0.0));
  List.Add(D);
  Passed = Evaluate("Add()", "after DeleteAll", "The list can be filled again", List.Size(), 1U) && Passed;
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the optimum and the best try events
bool UTRawEventIncarnationList::TestOptimumAndBestTryEvents()
{
  bool Passed = true;

  MRawEventIncarnationList List;
  MRawEventIncarnations* A = CreateIncarnations(100.0, 1.0, MVector(1.0, 0.0, 0.0), true);
  MRawEventIncarnations* B = CreateIncarnations(200.0, 1.0, MVector(2.0, 0.0, 0.0), true);
  List.Add(A);
  List.Add(B);

  // All incarnations have an optimum event:
  Passed = EvaluateTrue("HasOnlyOptimumEvents()", "two optimum", "All incarnations have an optimum event", List.HasOnlyOptimumEvents()) && Passed;
  Passed = EvaluateTrue("HasBestTryEvents()", "two optimum", "An optimum event counts as best try", List.HasBestTryEvents()) && Passed;
  vector<MRERawEvent*> Optimum = List.GetOptimumEvents();
  Passed = EvaluateSize("GetOptimumEvents()", "two optimum", "There are two optimum events", Optimum.size(), 2) && Passed;
  if (Optimum.size() == 2) {
    Passed = EvaluateTrue("GetOptimumEvents()", "two optimum", "The first one is the optimum event of the first incarnations", Optimum[0] == A->GetOptimumEvent()) && Passed;
    Passed = EvaluateTrue("GetOptimumEvents()", "two optimum", "The second one is the optimum event of the second incarnations", Optimum[1] == B->GetOptimumEvent()) && Passed;
  }
  vector<MRERawEvent*> BestTry = List.GetBestTryEvents();
  Passed = EvaluateSize("GetBestTryEvents()", "two optimum", "There are two best try events", BestTry.size(), 2) && Passed;
  if (BestTry.size() == 2) {
    Passed = EvaluateTrue("GetBestTryEvents()", "two optimum", "The optimum event is the best try of the first incarnations", BestTry[0] == A->GetOptimumEvent()) && Passed;
    Passed = EvaluateTrue("GetBestTryEvents()", "two optimum", "The optimum event is the best try of the second incarnations", BestTry[1] == B->GetOptimumEvent()) && Passed;
  }

  // One incarnation has only a best try event:
  MRawEventIncarnations* C = CreateIncarnations(300.0, 1.0, MVector(3.0, 0.0, 0.0), false);
  List.Add(C);
  Passed = EvaluateFalse("HasOnlyOptimumEvents()", "two optimum, one best try", "Not all incarnations have an optimum event", List.HasOnlyOptimumEvents()) && Passed;
  Passed = EvaluateTrue("HasBestTryEvents()", "two optimum, one best try", "All incarnations have a best try event", List.HasBestTryEvents()) && Passed;
  Optimum = List.GetOptimumEvents();
  Passed = EvaluateSize("GetOptimumEvents()", "two optimum, one best try", "There are three entries", Optimum.size(), 3) && Passed;
  if (Optimum.size() == 3) {
    Passed = EvaluateTrue("GetOptimumEvents()", "two optimum, one best try", "The entry of the incarnations without optimum event is a nullptr", Optimum[2] == nullptr) && Passed;
  }
  BestTry = List.GetBestTryEvents();
  Passed = EvaluateSize("GetBestTryEvents()", "two optimum, one best try", "There are three entries", BestTry.size(), 3) && Passed;
  if (BestTry.size() == 3) {
    Passed = EvaluateTrue("GetBestTryEvents()", "two optimum, one best try", "The third entry is the best try event", BestTry[2] == C->GetBestTryEvent() && BestTry[2] != nullptr) && Passed;
  }
  Passed = EvaluateTrue("GetOptimumPhysicalEvent()", "two optimum, one best try", "There is no optimum physical event", List.GetOptimumPhysicalEvent() == nullptr) && Passed;

  // Incarnations without any raw event:
  MRawEventIncarnations* Empty = new MRawEventIncarnations();
  List.Add(Empty);
  Passed = EvaluateFalse("HasBestTryEvents()", "with empty incarnations", "Not all incarnations have a best try event", List.HasBestTryEvents()) && Passed;
  Passed = EvaluateFalse("HasOnlyOptimumEvents()", "with empty incarnations", "Not all incarnations have an optimum event", List.HasOnlyOptimumEvents()) && Passed;
  BestTry = List.GetBestTryEvents();
  Passed = EvaluateSize("GetBestTryEvents()", "with empty incarnations", "There are four entries", BestTry.size(), 4) && Passed;
  if (BestTry.size() == 4) {
    Passed = EvaluateTrue("GetBestTryEvents()", "with empty incarnations", "The entry of the empty incarnations is a nullptr", BestTry[3] == nullptr) && Passed;
  }
  Passed = EvaluateTrue("GetBestTryPhysicalEvent()", "with empty incarnations", "There is no best try physical event", List.GetBestTryPhysicalEvent() == nullptr) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for a valid event
bool UTRawEventIncarnationList::TestIsAnyEventValid()
{
  bool Passed = true;

  MRawEventIncarnationList List;
  MRawEventIncarnations* A = CreateIncarnations(100.0, 1.0, MVector(1.0, 0.0, 0.0));
  MRawEventIncarnations* B = CreateIncarnations(200.0, 1.0, MVector(2.0, 0.0, 0.0));
  List.Add(A);
  List.Add(B);
  Passed = EvaluateTrue("IsAnyEventValid()", "two valid", "A valid event is found", List.IsAnyEventValid()) && Passed;
  A->GetRawEventAt(0)->SetValid(false);
  Passed = EvaluateTrue("IsAnyEventValid()", "second valid", "The valid event in the second incarnations is found", List.IsAnyEventValid()) && Passed;
  B->GetRawEventAt(0)->SetValid(false);
  Passed = EvaluateFalse("IsAnyEventValid()", "none valid", "No valid event is found", List.IsAnyEventValid()) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the physical event of one incarnation
bool UTRawEventIncarnationList::TestSinglePhysicalEvent()
{
  bool Passed = true;

  vector<bool> Modes = { true, false };
  for (bool IsOptimum: Modes) {
    MString Function = "GetBestTryPhysicalEvent()";
    if (IsOptimum == true) {
      Function = "GetOptimumPhysicalEvent()";
    }

    MRawEventIncarnationList List;
    MRawEventIncarnations* A = CreateIncarnations(661.7, 2.0, MVector(1.0, 2.0, 3.0), IsOptimum, 12.5);
    List.Add(A);

    // The physical event is the one of the raw event, which owns it
    MPhysicalEvent* Event = GetPhysicalEvent(List, IsOptimum);
    Passed = EvaluateTrue(Function, "one photo event", "A physical event is returned", Event != nullptr) && Passed;
    if (Event != nullptr) {
      Passed = Evaluate(Function, "one photo event", "It is a photo event", Event->GetType(), MPhysicalEvent::c_Photo) && Passed;
      Passed = EvaluateNear(Function, "one photo event", "The energy is the one of the hit", Event->GetEnergy(), 661.7, 1e-12) && Passed;
      Passed = EvaluateVectorNear(Function, "one photo event", "The position is the one of the hit", Event->GetPosition(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
      Passed = Evaluate(Function, "one photo event", "The ID is the one of the raw event", Event->GetId(), static_cast<long>(c_EventID)) && Passed;
      Passed = EvaluateNear(Function, "one photo event", "The time is the one of the raw event", Event->GetTime().GetAsSeconds(), 12.5, 1e-9) && Passed;
      Passed = EvaluateTrue(Function, "one photo event", "It is the physical event of the raw event", Event == A->GetRawEventAt(0)->GetPhysicalEvent()) && Passed;
    }

    // Asked again on the same list:
    MPhysicalEvent* Again = GetPhysicalEvent(List, IsOptimum);
    Passed = EvaluateTrue(Function, "one photo event, second call", "The same physical event is returned", Again == Event) && Passed;

    if (IsOptimum == true) {
      Passed = EvaluateTrue("GetBestTryPhysicalEvent()", "one optimum photo event", "The optimum event is the best try event", List.GetBestTryPhysicalEvent() == Event) && Passed;
    } else {
      Passed = EvaluateTrue("GetOptimumPhysicalEvent()", "one best try photo event", "There is no optimum physical event", List.GetOptimumPhysicalEvent() == nullptr) && Passed;
    }

    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the multi event of several incarnations
bool UTRawEventIncarnationList::TestMultiEvent()
{
  bool Passed = true;

  vector<bool> Modes = { true, false };
  for (bool IsOptimum: Modes) {
    MString Function = "GetBestTryPhysicalEvent()";
    if (IsOptimum == true) {
      Function = "GetOptimumPhysicalEvent()";
    }

    // Two events far away from 511 keV:
    MRawEventIncarnationList List;
    List.Add(CreateIncarnations(661.7, 2.0, MVector(1.0, 2.0, 3.0), IsOptimum, 12.5));
    List.Add(CreateIncarnations(1173.2, 3.0, MVector(4.0, 5.0, 6.0), IsOptimum, 12.5));

    MPhysicalEvent* Event = GetPhysicalEvent(List, IsOptimum);
    Passed = EvaluateTrue(Function, "two photo events", "A physical event is returned", Event != nullptr) && Passed;
    if (Event != nullptr) {
      Passed = Evaluate(Function, "two photo events", "It is a multi event", Event->GetType(), MPhysicalEvent::c_Multi) && Passed;
      MMultiEvent* Multi = dynamic_cast<MMultiEvent*>(Event);
      Passed = EvaluateTrue(Function, "two photo events", "It is a MMultiEvent", Multi != nullptr) && Passed;
      if (Multi != nullptr) {
        Passed = Evaluate(Function, "two photo events", "It has two events", Multi->GetNumberOfEvents(), 2U) && Passed;
        // 661.7 keV + 1173.2 keV = 1834.9 keV
        Passed = EvaluateNear(Function, "two photo events", "The energy is the sum of both events", Multi->GetEnergy(), 1834.9, 1e-9) && Passed;
        if (Multi->GetNumberOfEvents() == 2) {
          Passed = Evaluate(Function, "two photo events", "The first event is a photo event", Multi->GetEvent(0)->GetType(), MPhysicalEvent::c_Photo) && Passed;
          Passed = EvaluateNear(Function, "two photo events", "The first event has the energy of the first incarnations", Multi->GetEvent(0)->GetEnergy(), 661.7, 1e-12) && Passed;
          Passed = EvaluateNear(Function, "two photo events", "The second event has the energy of the second incarnations", Multi->GetEvent(1)->GetEnergy(), 1173.2, 1e-12) && Passed;
          Passed = EvaluateVectorNear(Function, "two photo events", "The second event has the position of the second incarnations", Multi->GetEvent(1)->GetPosition(), MVector(4.0, 5.0, 6.0), 1e-15) && Passed;
        }
        Passed = Evaluate(Function, "two photo events", "The ID is the one of the raw events", Multi->GetId(), static_cast<long>(c_EventID)) && Passed;
        Passed = EvaluateNear(Function, "two photo events", "The time is the one of the raw events", Multi->GetTime().GetAsSeconds(), 12.5, 1e-9) && Passed;
      }
      // The multi event is a new object for the caller, its events stay with the raw events
      delete Event;
    }

    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the PET event of two incarnations with 511 keV
bool UTRawEventIncarnationList::TestPETEvent()
{
  bool Passed = true;

  vector<bool> Modes = { true, false };
  for (bool IsOptimum: Modes) {
    MString Function = "GetBestTryPhysicalEvent()";
    if (IsOptimum == true) {
      Function = "GetOptimumPhysicalEvent()";
    }

    // Two events at 511 keV within their energy resolution of 2 keV and 3 keV:
    MRawEventIncarnationList List;
    MRawEventIncarnations* A = CreateIncarnations(510.0, 2.0, MVector(1.0, 2.0, 3.0), IsOptimum, 12.5);
    MRawEventIncarnations* B = CreateIncarnations(512.5, 3.0, MVector(-4.0, -5.0, -6.0), IsOptimum, 12.5);
    A->GetRawEventAt(0)->GetRESEAt(0)->SetTime(1.0e-9);
    A->GetRawEventAt(0)->GetRESEAt(0)->SetTimeResolution(0.25e-9);
    B->GetRawEventAt(0)->GetRESEAt(0)->SetTime(3.0e-9);
    B->GetRawEventAt(0)->GetRESEAt(0)->SetTimeResolution(0.5e-9);
    List.Add(A);
    List.Add(B);

    MPhysicalEvent* Event = GetPhysicalEvent(List, IsOptimum);
    Passed = EvaluateTrue(Function, "two 511 keV events", "A physical event is returned", Event != nullptr) && Passed;
    if (Event != nullptr) {
      Passed = Evaluate(Function, "two 511 keV events", "It is a PET event", Event->GetType(), MPhysicalEvent::c_PET) && Passed;
      MPETEvent* PET = dynamic_cast<MPETEvent*>(Event);
      Passed = EvaluateTrue(Function, "two 511 keV events", "It is a MPETEvent", PET != nullptr) && Passed;
      if (PET != nullptr) {
        Passed = EvaluateNear(Function, "two 511 keV events", "The first energy is the one of the first incarnations", PET->GetEnergy1(), 510.0, 1e-12) && Passed;
        Passed = EvaluateNear(Function, "two 511 keV events", "The second energy is the one of the second incarnations", PET->GetEnergy2(), 512.5, 1e-12) && Passed;
        Passed = EvaluateVectorNear(Function, "two 511 keV events", "The first position is the one of the first incarnations", PET->GetPosition1(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
        Passed = EvaluateVectorNear(Function, "two 511 keV events", "The second position is the one of the second incarnations", PET->GetPosition2(), MVector(-4.0, -5.0, -6.0), 1e-15) && Passed;
        Passed = EvaluateNear(Function, "two 511 keV events", "The first timing is the time of the first hit", PET->GetTiming1(), 1.0e-9, 1e-21) && Passed;
        Passed = EvaluateNear(Function, "two 511 keV events", "The second timing is the time of the second hit", PET->GetTiming2(), 3.0e-9, 1e-21) && Passed;
        Passed = EvaluateNear(Function, "two 511 keV events", "The first timing resolution is the one of the first hit", PET->GetTimingResolution1(), 0.25e-9, 1e-21) && Passed;
        Passed = EvaluateNear(Function, "two 511 keV events", "The second timing resolution is the one of the second hit", PET->GetTimingResolution2(), 0.5e-9, 1e-21) && Passed;
        Passed = Evaluate(Function, "two 511 keV events", "The ID is the one of the raw events", PET->GetId(), static_cast<long>(c_EventID)) && Passed;
        Passed = EvaluateNear(Function, "two 511 keV events", "The time is the one of the raw events", PET->GetTime().GetAsSeconds(), 12.5, 1e-9) && Passed;
        // Defect REV-C-9: The energy and position resolutions of the hits are not handed over to the PET event
        Passed = ExpectedToFail(EvaluateNear(Function, "two 511 keV events", "The first energy resolution is the one of the first incarnations", PET->GetEnergyResolution1(), 2.0, 1e-12)) && Passed;
        Passed = ExpectedToFail(EvaluateNear(Function, "two 511 keV events", "The second energy resolution is the one of the second incarnations", PET->GetEnergyResolution2(), 3.0, 1e-12)) && Passed;
        Passed = ExpectedToFail(EvaluateVectorNear(Function, "two 511 keV events", "The first position resolution is the one of the first hit", PET->GetPositionResolution1(), MVector(0.1, 0.2, 0.3), 1e-15)) && Passed;
        Passed = ExpectedToFail(EvaluateVectorNear(Function, "two 511 keV events", "The second position resolution is the one of the second hit", PET->GetPositionResolution2(), MVector(0.1, 0.2, 0.3), 1e-15)) && Passed;
      }
      // The PET event is a new object for the caller
      delete Event;
    }

    // With a start point the position is the one of the start point - here the hit of the other incarnations to see the difference
    A->GetRawEventAt(0)->SetStartPoint(B->GetRawEventAt(0)->GetRESEAt(0));
    Event = GetPhysicalEvent(List, IsOptimum);
    MPETEvent* WithStart = dynamic_cast<MPETEvent*>(Event);
    Passed = EvaluateTrue(Function, "two 511 keV events, start point", "A PET event is returned", WithStart != nullptr) && Passed;
    if (WithStart != nullptr) {
      Passed = EvaluateVectorNear(Function, "two 511 keV events, start point", "The first position is the one of the start point", WithStart->GetPosition1(), MVector(-4.0, -5.0, -6.0), 1e-15) && Passed;
    }
    delete Event;
    A->GetRawEventAt(0)->SetStartPoint(nullptr);

    // Without a start point and with two hits there is no position: an error message and no event
    MREHit* Second = new MREHit();
    Second->SetPosition(MVector(7.0, 8.0, 9.0));
    Second->SetEnergy(0.5);
    A->GetRawEventAt(0)->AddRESE(Second);
    DisableDefaultStreams();
    Event = GetPhysicalEvent(List, IsOptimum);
    EnableDefaultStreams();
    Passed = EvaluateTrue(Function, "two 511 keV events, first with two hits and no start point", "No event is returned", Event == nullptr) && Passed;
    delete Event;

    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the energy window of the PET event
bool UTRawEventIncarnationList::TestPETEnergyWindow()
{
  bool Passed = true;

  // An event counts as 511 keV if its energy is less than two energy resolutions away from c_E0
  // With a resolution of 2 keV: 4 keV. 3.9 keV below and above are inside, 4.1 keV are outside
  vector<double> Differences = { -4.1, -3.9, 3.9, 4.1 };
  vector<int> ExpectedTypes = { MPhysicalEvent::c_Multi, MPhysicalEvent::c_PET, MPhysicalEvent::c_PET, MPhysicalEvent::c_Multi };

  vector<bool> Modes = { true, false };
  for (bool IsOptimum: Modes) {
    MString Function = "GetBestTryPhysicalEvent()";
    if (IsOptimum == true) {
      Function = "GetOptimumPhysicalEvent()";
    }

    for (unsigned int d = 0; d < Differences.size(); ++d) {
      MString Input = MString("511 keV and 511 keV ") + Differences[d] + " keV, resolution 2 keV";
      MRawEventIncarnationList List;
      List.Add(CreateIncarnations(c_E0, 2.0, MVector(1.0, 2.0, 3.0), IsOptimum));
      List.Add(CreateIncarnations(c_E0 + Differences[d], 2.0, MVector(-4.0, -5.0, -6.0), IsOptimum));

      MPhysicalEvent* Event = GetPhysicalEvent(List, IsOptimum);
      Passed = EvaluateTrue(Function, Input, "A physical event is returned", Event != nullptr) && Passed;
      if (Event != nullptr) {
        Passed = Evaluate(Function, Input, "The type follows the energy window of two energy resolutions", Event->GetType(), ExpectedTypes[d]) && Passed;
      }
      delete Event;
      List.DeleteAll();
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the multi event which contains a PET event
bool UTRawEventIncarnationList::TestMultiEventWithPET()
{
  bool Passed = true;

  vector<bool> Modes = { true, false };
  for (bool IsOptimum: Modes) {
    MString Function = "GetBestTryPhysicalEvent()";
    if (IsOptimum == true) {
      Function = "GetOptimumPhysicalEvent()";
    }

    // Two events at 511 keV with another event between them:
    {
      MRawEventIncarnationList List;
      List.Add(CreateIncarnations(510.0, 2.0, MVector(1.0, 2.0, 3.0), IsOptimum, 12.5));
      List.Add(CreateIncarnations(1274.5, 3.0, MVector(0.0, 0.0, 1.0), IsOptimum, 12.5));
      List.Add(CreateIncarnations(512.5, 3.0, MVector(-4.0, -5.0, -6.0), IsOptimum, 12.5));

      MPhysicalEvent* Event = GetPhysicalEvent(List, IsOptimum);
      MMultiEvent* Multi = dynamic_cast<MMultiEvent*>(Event);
      Passed = EvaluateTrue(Function, "511 keV, 1274.5 keV, 511 keV", "A multi event is returned", Multi != nullptr) && Passed;
      if (Multi != nullptr) {
        Passed = Evaluate(Function, "511 keV, 1274.5 keV, 511 keV", "It has two events: the photo event and the PET event", Multi->GetNumberOfEvents(), 2U) && Passed;
        if (Multi->GetNumberOfEvents() == 2) {
          Passed = Evaluate(Function, "511 keV, 1274.5 keV, 511 keV", "The first event is the photo event", Multi->GetEvent(0)->GetType(), MPhysicalEvent::c_Photo) && Passed;
          Passed = EvaluateNear(Function, "511 keV, 1274.5 keV, 511 keV", "The photo event has 1274.5 keV", Multi->GetEvent(0)->GetEnergy(), 1274.5, 1e-12) && Passed;
          Passed = Evaluate(Function, "511 keV, 1274.5 keV, 511 keV", "The second event is the PET event", Multi->GetEvent(1)->GetType(), MPhysicalEvent::c_PET) && Passed;
          // 510 keV + 512.5 keV = 1022.5 keV
          Passed = EvaluateNear(Function, "511 keV, 1274.5 keV, 511 keV", "The PET event has the energy of both 511 keV events", Multi->GetEvent(1)->GetEnergy(), 1022.5, 1e-9) && Passed;
        }
        // 1274.5 keV + 1022.5 keV = 2297 keV
        Passed = EvaluateNear(Function, "511 keV, 1274.5 keV, 511 keV", "The energy is the sum of all three events", Multi->GetEnergy(), 2297.0, 1e-9) && Passed;
        Passed = Evaluate(Function, "511 keV, 1274.5 keV, 511 keV", "The ID is the one of the raw events", Multi->GetId(), static_cast<long>(c_EventID)) && Passed;
        // The PET event belongs to nobody else, the photo event to its raw event
        if (Multi->GetNumberOfEvents() == 2) {
          delete Multi->GetEvent(1);
        }
      }
      delete Event;
      List.DeleteAll();
    }

    // Three events at 511 keV are no PET event:
    {
      MRawEventIncarnationList List;
      List.Add(CreateIncarnations(510.0, 2.0, MVector(1.0, 2.0, 3.0), IsOptimum));
      List.Add(CreateIncarnations(511.0, 2.0, MVector(0.0, 0.0, 1.0), IsOptimum));
      List.Add(CreateIncarnations(512.0, 2.0, MVector(-4.0, -5.0, -6.0), IsOptimum));

      MPhysicalEvent* Event = GetPhysicalEvent(List, IsOptimum);
      MMultiEvent* Multi = dynamic_cast<MMultiEvent*>(Event);
      Passed = EvaluateTrue(Function, "three 511 keV events", "A multi event is returned", Multi != nullptr) && Passed;
      if (Multi != nullptr) {
        Passed = Evaluate(Function, "three 511 keV events", "It has three events", Multi->GetNumberOfEvents(), 3U) && Passed;
        for (unsigned int e = 0; e < Multi->GetNumberOfEvents(); ++e) {
          Passed = Evaluate(Function, "three 511 keV events", "Each event is a photo event", Multi->GetEvent(e)->GetType(), MPhysicalEvent::c_Photo) && Passed;
        }
      }
      delete Event;
      List.DeleteAll();
    }

    // One event at 511 keV and another one are no PET event:
    {
      MRawEventIncarnationList List;
      List.Add(CreateIncarnations(510.0, 2.0, MVector(1.0, 2.0, 3.0), IsOptimum));
      List.Add(CreateIncarnations(1274.5, 3.0, MVector(0.0, 0.0, 1.0), IsOptimum));

      MPhysicalEvent* Event = GetPhysicalEvent(List, IsOptimum);
      MMultiEvent* Multi = dynamic_cast<MMultiEvent*>(Event);
      Passed = EvaluateTrue(Function, "511 keV and 1274.5 keV", "A multi event is returned", Multi != nullptr) && Passed;
      if (Multi != nullptr) {
        Passed = Evaluate(Function, "511 keV and 1274.5 keV", "It has two events", Multi->GetNumberOfEvents(), 2U) && Passed;
      }
      delete Event;
      List.DeleteAll();
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTRawEventIncarnationList::TestToString()
{
  bool Passed = true;

  MRawEventIncarnationList List;
  MRawEventIncarnations* A = CreateIncarnations(100.0, 1.0, MVector(1.0, 0.0, 0.0));
  MRawEventIncarnations* B = CreateIncarnations(200.0, 1.0, MVector(2.0, 0.0, 0.0));
  List.Add(A);
  List.Add(B);

  // The text is the head line and the text of all incarnations, one level deeper
  MString Expected = "Raw event collection with the following incarnations:\n";
  Expected += A->ToString(true, 1);
  Expected += B->ToString(true, 1);
  Passed = Evaluate("ToString()", "two incarnations", "The text lists both incarnations", List.ToString(), Expected) && Passed;

  MString Indented = "   Raw event collection with the following incarnations:\n";
  Indented += A->ToString(false, 2);
  Indented += B->ToString(false, 2);
  Passed = Evaluate("ToString()", "two incarnations, without links, level 1", "The head line is indented by 3 blanks and the incarnations are one level deeper", List.ToString(false, 1), Indented) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return new incarnations with one good photo event with one hit - they own the raw event, the caller owns them
//! The raw event is the optimum event, or (if not) the best try event
MRawEventIncarnations* UTRawEventIncarnationList::CreateIncarnations(double Energy, double EnergyResolution, const MVector& Position, bool IsOptimum, double Time)
{
  MREHit* Hit = new MREHit();
  Hit->SetPosition(Position);
  Hit->SetPositionResolution(MVector(0.1, 0.2, 0.3));
  Hit->SetEnergy(Energy);
  Hit->SetEnergyResolution(EnergyResolution);

  MRERawEvent* Event = new MRERawEvent();
  Event->AddRESE(Hit);
  Event->SetEventID(c_EventID);
  Event->SetEventTime(MTime(Time));
  Event->SetEventType(MRERawEvent::c_PhotoEvent);
  Event->SetGoodEvent(true);

  MRawEventIncarnations* Incarnations = new MRawEventIncarnations();
  Incarnations->SetInitialRawEvent(Event);
  if (IsOptimum == true) {
    Incarnations->SetOptimumEvent(Event);
  } else {
    Incarnations->SetBestTryEvent(Event);
  }

  return Incarnations;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the physical event of the list: the optimum one, or (if not) the best try one
MPhysicalEvent* UTRawEventIncarnationList::GetPhysicalEvent(MRawEventIncarnationList& List, bool IsOptimum)
{
  if (IsOptimum == true) {
    return List.GetOptimumPhysicalEvent();
  }

  return List.GetBestTryPhysicalEvent();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if removing incarnations which are not in the list changes nothing
bool UTRawEventIncarnationList::IsRemoveOfUnknownIncarnationsIgnored()
{
  MRawEventIncarnationList List;
  MRawEventIncarnations* A = CreateIncarnations(100.0, 1.0, MVector(1.0, 0.0, 0.0));
  MRawEventIncarnations* Unknown = CreateIncarnations(200.0, 1.0, MVector(2.0, 0.0, 0.0));
  List.Add(A);
  List.Remove(Unknown);
  bool IsUnchanged = false;
  if (List.Size() == 1 && List.Get(0) == A) {
    IsUnchanged = true;
  }
  Unknown->DeleteAll();
  delete Unknown;
  List.DeleteAll();

  return IsUnchanged;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Hide the info messages of the geometry manager:
  gErrorIgnoreLevel = kWarning;

  UTRawEventIncarnationList Test;
  return Test.Run() == true ? 0 : 1;
}


// UTRawEventIncarnationList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
