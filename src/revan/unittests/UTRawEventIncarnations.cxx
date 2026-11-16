/*
 * UTRawEventIncarnations.cxx
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
#include "MGeometryRevan.h"
#include "MRawEventIncarnations.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the list of the incarnations of one raw event MRawEventIncarnations
class UTRawEventIncarnations : public MUnitTest
{
public:
  //! Default constructor
  UTRawEventIncarnations() : MUnitTest("UTRawEventIncarnations") {}
  //! Default destructor
  virtual ~UTRawEventIncarnations() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the state after the construction
  bool TestDefaults();
  //! Test adding, reading, and replacing raw events
  bool TestAddAndAccess();
  //! Test removing and deleting raw events
  bool TestRemoveAndDelete();
  //! Test the initial raw event
  bool TestInitialRawEvent();
  //! Test the optimum and the best try event
  bool TestOptimumAndBestTry();
  //! Test the search for a valid event
  bool TestIsAnyEventValid();
  //! Test the sorting by the track quality factor
  bool TestSortByTrackQualityFactor();
  //! Test the text output
  bool TestToString();
  //! Test that the destructor keeps the raw events
  bool TestDestructor();

  //! Return a new raw event with one hit of the given energy (keV) and the given track quality factor - the caller owns it
  MRERawEvent* CreateRawEvent(double Energy, double TrackQualityFactor = MRERawEvent::c_NoQualityFactor);
  //! Return true if a negative index returns a nullptr
  bool IsNegativeIndexRejected();
  //! Return true if setting a raw event behind the end of the list changes nothing
  bool IsSetBehindEndRejected();
  //! Return true if removing a raw event which is not in the list changes nothing
  bool IsRemoveOfUnknownRawEventIgnored();
  //! Return true if deleting a raw event which is not in the list changes nothing
  bool IsDeleteOfUnknownRawEventIgnored();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRawEventIncarnations::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestAddAndAccess() && Passed;
  Passed = TestRemoveAndDelete() && Passed;
  Passed = TestInitialRawEvent() && Passed;
  Passed = TestOptimumAndBestTry() && Passed;
  Passed = TestIsAnyEventValid() && Passed;
  Passed = TestSortByTrackQualityFactor() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestDestructor() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the construction
bool UTRawEventIncarnations::TestDefaults()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  vector<MString> Names = { "default constructor", "constructor with geometry" };
  for (unsigned int c = 0; c < Names.size(); ++c) {
    MRawEventIncarnations* Incarnations = nullptr;
    if (c == 0) {
      Incarnations = new MRawEventIncarnations();
    } else {
      Incarnations = new MRawEventIncarnations(&Geometry);
    }
    Passed = Evaluate("GetNRawEvents()", Names[c], "There are no raw events", Incarnations->GetNRawEvents(), 0) && Passed;
    Passed = EvaluateTrue("GetInitialRawEvent()", Names[c], "There is no initial raw event", Incarnations->GetInitialRawEvent() == nullptr) && Passed;
    Passed = EvaluateFalse("HasOptimumEvent()", Names[c], "There is no optimum event", Incarnations->HasOptimumEvent()) && Passed;
    Passed = EvaluateTrue("GetOptimumEvent()", Names[c], "The optimum event is a nullptr", Incarnations->GetOptimumEvent() == nullptr) && Passed;
    Passed = EvaluateFalse("HasBestTryEvent()", Names[c], "There is no best try event", Incarnations->HasBestTryEvent()) && Passed;
    Passed = EvaluateTrue("GetBestTryEvent()", Names[c], "The best try event is a nullptr", Incarnations->GetBestTryEvent() == nullptr) && Passed;
    Passed = EvaluateFalse("IsAnyEventValid()", Names[c], "No event is valid", Incarnations->IsAnyEventValid()) && Passed;
    Passed = Evaluate("ToString()", Names[c], "The text has only the head line", Incarnations->ToString(), MString("Raw event with the following possible combinations:\n")) && Passed;

    // An empty list can be emptied and sorted:
    Incarnations->DeleteAll();
    Incarnations->SortByTrackQualityFactor(true);
    Incarnations->SortByTrackQualityFactor(false);
    Passed = Evaluate("DeleteAll()", Names[c], "An empty list stays empty", Incarnations->GetNRawEvents(), 0) && Passed;

    // The geometry can be set later:
    Incarnations->SetGeometry(&Geometry);
    Incarnations->SetGeometry(nullptr);
    Passed = Evaluate("SetGeometry()", Names[c], "Setting the geometry does not change the list", Incarnations->GetNRawEvents(), 0) && Passed;

    delete Incarnations;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding, reading, and replacing raw events
bool UTRawEventIncarnations::TestAddAndAccess()
{
  bool Passed = true;

  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  MRERawEvent* B = CreateRawEvent(200.0);
  MRERawEvent* C = CreateRawEvent(300.0);

  Incarnations.AddRawEvent(A);
  Passed = Evaluate("AddRawEvent()", "one event", "There is one raw event", Incarnations.GetNRawEvents(), 1) && Passed;
  Passed = EvaluateTrue("GetRawEventAt()", "one event, index 0", "The event is returned", Incarnations.GetRawEventAt(0) == A) && Passed;
  Incarnations.AddRawEvent(B);
  Incarnations.AddRawEvent(C);
  Passed = Evaluate("AddRawEvent()", "three events", "There are three raw events", Incarnations.GetNRawEvents(), 3) && Passed;
  Passed = EvaluateTrue("GetRawEventAt()", "three events, index 0", "The first event is returned", Incarnations.GetRawEventAt(0) == A) && Passed;
  Passed = EvaluateTrue("GetRawEventAt()", "three events, index 1", "The second event is returned", Incarnations.GetRawEventAt(1) == B) && Passed;
  Passed = EvaluateTrue("GetRawEventAt()", "three events, index 2", "The third event is returned", Incarnations.GetRawEventAt(2) == C) && Passed;

  // Adding neither sets the initial nor the optimum event:
  Passed = EvaluateTrue("AddRawEvent()", "three events", "Adding does not set the initial raw event", Incarnations.GetInitialRawEvent() == nullptr) && Passed;
  Passed = EvaluateFalse("AddRawEvent()", "three events", "Adding does not set the optimum event", Incarnations.HasOptimumEvent()) && Passed;

  // Behind the end:
  DisableDefaultStreams();
  MRERawEvent* BehindEnd = Incarnations.GetRawEventAt(3);
  MRERawEvent* FarBehindEnd = Incarnations.GetRawEventAt(1000);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetRawEventAt()", "three events, index 3", "The index behind the last event returns a nullptr", BehindEnd == nullptr) && Passed;
  Passed = EvaluateTrue("GetRawEventAt()", "three events, index 1000", "An index far behind the last event returns a nullptr", FarBehindEnd == nullptr) && Passed;
  // Defect REV-C-5: GetRawEventAt does not check for a negative index and reads in front of the list
  Passed = ExpectedToCrash(EvaluateTrue("GetRawEventAt()", "index -1", "A negative index returns a nullptr", IsNegativeIndexRejected())) && Passed;

  // Replace - the list does not delete the replaced event:
  MRERawEvent* D = CreateRawEvent(400.0);
  Incarnations.SetRawEventAt(D, 1);
  Passed = Evaluate("SetRawEventAt()", "index 1", "The number of raw events is unchanged", Incarnations.GetNRawEvents(), 3) && Passed;
  Passed = EvaluateTrue("SetRawEventAt()", "index 1", "The new event is at the index", Incarnations.GetRawEventAt(1) == D) && Passed;
  Passed = EvaluateTrue("SetRawEventAt()", "index 1", "The event in front is unchanged", Incarnations.GetRawEventAt(0) == A) && Passed;
  Passed = EvaluateTrue("SetRawEventAt()", "index 1", "The event behind is unchanged", Incarnations.GetRawEventAt(2) == C) && Passed;
  Passed = EvaluateNear("SetRawEventAt()", "index 1", "The replaced event is still alive", B->GetEnergy(), 200.0, 1e-12) && Passed;
  delete B;
  // Defect REV-C-5: SetRawEventAt ends the program with an assert if the index is behind the end
  Passed = ExpectedToCrash(EvaluateTrue("SetRawEventAt()", "index behind the end", "An index behind the end changes nothing", IsSetBehindEndRejected())) && Passed;

  Incarnations.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test removing and deleting raw events
bool UTRawEventIncarnations::TestRemoveAndDelete()
{
  bool Passed = true;

  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  MRERawEvent* B = CreateRawEvent(200.0);
  MRERawEvent* C = CreateRawEvent(300.0);
  MRERawEvent* D = CreateRawEvent(400.0);
  Incarnations.SetInitialRawEvent(A);
  Incarnations.AddRawEvent(B);
  Incarnations.AddRawEvent(C);
  Incarnations.AddRawEvent(D);
  B->SetGoodEvent(true);
  Incarnations.SetOptimumEvent(B);
  Incarnations.SetBestTryEvent(C);

  // Remove an event which has no role:
  Incarnations.RemoveRawEvent(D);
  Passed = Evaluate("RemoveRawEvent()", "last event", "One event less", Incarnations.GetNRawEvents(), 3) && Passed;
  Passed = EvaluateNear("RemoveRawEvent()", "last event", "The removed event is not deleted", D->GetEnergy(), 400.0, 1e-12) && Passed;
  Passed = EvaluateTrue("RemoveRawEvent()", "last event", "The initial raw event is kept", Incarnations.GetInitialRawEvent() == A) && Passed;
  Passed = EvaluateTrue("RemoveRawEvent()", "last event", "The optimum event is kept", Incarnations.GetOptimumEvent() == B) && Passed;
  Passed = EvaluateTrue("RemoveRawEvent()", "last event", "The best try event is kept", Incarnations.GetBestTryEvent() == C) && Passed;
  delete D;

  // Remove the optimum event from the middle:
  Incarnations.RemoveRawEvent(B);
  Passed = Evaluate("RemoveRawEvent()", "optimum event", "One event less", Incarnations.GetNRawEvents(), 2) && Passed;
  Passed = EvaluateTrue("RemoveRawEvent()", "optimum event", "The first event stays", Incarnations.GetRawEventAt(0) == A) && Passed;
  Passed = EvaluateTrue("RemoveRawEvent()", "optimum event", "The third event moves up", Incarnations.GetRawEventAt(1) == C) && Passed;
  Passed = EvaluateFalse("RemoveRawEvent()", "optimum event", "There is no optimum event any more", Incarnations.HasOptimumEvent()) && Passed;
  Passed = EvaluateTrue("RemoveRawEvent()", "optimum event", "The optimum event is a nullptr", Incarnations.GetOptimumEvent() == nullptr) && Passed;
  Passed = EvaluateNear("RemoveRawEvent()", "optimum event", "The removed event is not deleted", B->GetEnergy(), 200.0, 1e-12) && Passed;
  delete B;

  // Delete the best try event:
  Incarnations.DeleteRawEvent(C);
  Passed = Evaluate("DeleteRawEvent()", "best try event", "One event less", Incarnations.GetNRawEvents(), 1) && Passed;
  Passed = EvaluateTrue("DeleteRawEvent()", "best try event", "The best try event is now the first raw event", Incarnations.GetBestTryEvent() == A) && Passed;

  // Delete the initial event:
  Incarnations.DeleteRawEvent(A);
  Passed = Evaluate("DeleteRawEvent()", "initial raw event", "The list is empty", Incarnations.GetNRawEvents(), 0) && Passed;
  Passed = EvaluateTrue("DeleteRawEvent()", "initial raw event", "There is no initial raw event any more", Incarnations.GetInitialRawEvent() == nullptr) && Passed;
  Passed = EvaluateFalse("DeleteRawEvent()", "initial raw event", "There is no best try event any more", Incarnations.HasBestTryEvent()) && Passed;

  // Defect REV-C-6: RemoveRawEvent and DeleteRawEvent erase the end of the list if the raw event is not in the list
  Passed = ExpectedToCrash(EvaluateTrue("RemoveRawEvent()", "event which is not in the list", "Removing an unknown raw event changes nothing", IsRemoveOfUnknownRawEventIgnored())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("DeleteRawEvent()", "event which is not in the list", "Deleting an unknown raw event changes nothing", IsDeleteOfUnknownRawEventIgnored())) && Passed;

  // Delete all, with all roles set:
  MRERawEvent* E = CreateRawEvent(500.0);
  MRERawEvent* F = CreateRawEvent(600.0);
  Incarnations.SetInitialRawEvent(E);
  Incarnations.AddRawEvent(F);
  F->SetGoodEvent(true);
  Incarnations.SetOptimumEvent(F);
  Incarnations.SetBestTryEvent(E);
  Incarnations.DeleteAll();
  Passed = Evaluate("DeleteAll()", "two events", "The list is empty", Incarnations.GetNRawEvents(), 0) && Passed;
  Passed = EvaluateTrue("DeleteAll()", "two events", "There is no initial raw event", Incarnations.GetInitialRawEvent() == nullptr) && Passed;
  Passed = EvaluateTrue("DeleteAll()", "two events", "There is no optimum event", Incarnations.GetOptimumEvent() == nullptr) && Passed;
  Passed = EvaluateTrue("DeleteAll()", "two events", "There is no best try event", Incarnations.GetBestTryEvent() == nullptr) && Passed;

  // The list can be used again:
  MRERawEvent* G = CreateRawEvent(700.0);
  Incarnations.AddRawEvent(G);
  Passed = Evaluate("AddRawEvent()", "after DeleteAll", "The list can be filled again", Incarnations.GetNRawEvents(), 1) && Passed;
  Incarnations.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the initial raw event
bool UTRawEventIncarnations::TestInitialRawEvent()
{
  bool Passed = true;

  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  Incarnations.SetInitialRawEvent(A);
  Passed = EvaluateTrue("SetInitialRawEvent()", "empty list", "The initial raw event is returned", Incarnations.GetInitialRawEvent() == A) && Passed;
  Passed = Evaluate("SetInitialRawEvent()", "empty list", "The initial raw event is the only raw event", Incarnations.GetNRawEvents(), 1) && Passed;
  Passed = EvaluateTrue("SetInitialRawEvent()", "empty list", "The initial raw event is the first raw event", Incarnations.GetRawEventAt(0) == A) && Passed;
  Passed = EvaluateFalse("SetInitialRawEvent()", "empty list", "The initial raw event is no optimum event", Incarnations.HasOptimumEvent()) && Passed;
  Passed = EvaluateTrue("SetInitialRawEvent()", "empty list", "The initial raw event is the best try", Incarnations.GetBestTryEvent() == A) && Passed;

  // A new initial event deletes all old events and their roles:
  MRERawEvent* B = CreateRawEvent(200.0);
  MRERawEvent* C = CreateRawEvent(300.0);
  Incarnations.AddRawEvent(B);
  B->SetGoodEvent(true);
  Incarnations.SetOptimumEvent(B);
  Incarnations.SetBestTryEvent(B);
  Incarnations.SetInitialRawEvent(C);
  Passed = EvaluateTrue("SetInitialRawEvent()", "list with two events", "The new initial raw event is returned", Incarnations.GetInitialRawEvent() == C) && Passed;
  Passed = Evaluate("SetInitialRawEvent()", "list with two events", "The new initial raw event is the only raw event", Incarnations.GetNRawEvents(), 1) && Passed;
  Passed = EvaluateFalse("SetInitialRawEvent()", "list with two events", "The optimum event is removed", Incarnations.HasOptimumEvent()) && Passed;
  Passed = EvaluateTrue("SetInitialRawEvent()", "list with two events", "The best try is the new initial raw event", Incarnations.GetBestTryEvent() == C) && Passed;

  Incarnations.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the optimum and the best try event
bool UTRawEventIncarnations::TestOptimumAndBestTry()
{
  bool Passed = true;

  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  MRERawEvent* B = CreateRawEvent(200.0);
  MRERawEvent* C = CreateRawEvent(300.0);
  Incarnations.AddRawEvent(A);
  Incarnations.AddRawEvent(B);
  Incarnations.AddRawEvent(C);

  // Without anything set the first raw event is the best try:
  Passed = EvaluateTrue("HasBestTryEvent()", "three events, nothing set", "There is a best try event", Incarnations.HasBestTryEvent()) && Passed;
  Passed = EvaluateTrue("GetBestTryEvent()", "three events, nothing set", "The first raw event is the best try", Incarnations.GetBestTryEvent() == A) && Passed;
  Passed = EvaluateFalse("HasOptimumEvent()", "three events, nothing set", "There is no optimum event", Incarnations.HasOptimumEvent()) && Passed;

  // A best try event:
  Incarnations.SetBestTryEvent(B);
  Passed = EvaluateTrue("SetBestTryEvent()", "second event", "The best try event is returned", Incarnations.GetBestTryEvent() == B) && Passed;
  Passed = EvaluateFalse("SetBestTryEvent()", "second event", "A best try event is no optimum event", Incarnations.HasOptimumEvent()) && Passed;

  // An optimum event - it has to be a good event:
  C->SetGoodEvent(true);
  Incarnations.SetOptimumEvent(C);
  Passed = EvaluateTrue("SetOptimumEvent()", "third event", "There is an optimum event", Incarnations.HasOptimumEvent()) && Passed;
  Passed = EvaluateTrue("SetOptimumEvent()", "third event", "The optimum event is returned", Incarnations.GetOptimumEvent() == C) && Passed;
  Passed = EvaluateTrue("HasBestTryEvent()", "best try and optimum", "There is a best try event", Incarnations.HasBestTryEvent()) && Passed;
  // Defect REV-C-8: GetBestTryEvent returns the best try event although there is an optimum event, against its description
  Passed = ExpectedToFail(EvaluateTrue("GetBestTryEvent()", "best try and optimum", "The optimum event is returned instead of the best try", Incarnations.GetBestTryEvent() == C)) && Passed;

  // Only an optimum event:
  Incarnations.SetBestTryEvent(nullptr);
  Passed = EvaluateTrue("HasBestTryEvent()", "optimum only", "The optimum event counts as best try", Incarnations.HasBestTryEvent()) && Passed;
  Passed = EvaluateTrue("GetBestTryEvent()", "optimum only", "The optimum event is returned as best try", Incarnations.GetBestTryEvent() == C) && Passed;

  // Remove the optimum event:
  Incarnations.SetOptimumEvent(nullptr);
  Passed = EvaluateFalse("SetOptimumEvent()", "nullptr", "The optimum event can be removed", Incarnations.HasOptimumEvent()) && Passed;
  Passed = EvaluateTrue("SetOptimumEvent()", "nullptr", "The optimum event is a nullptr", Incarnations.GetOptimumEvent() == nullptr) && Passed;
  Passed = EvaluateTrue("GetBestTryEvent()", "nothing set again", "The first raw event is the best try again", Incarnations.GetBestTryEvent() == A) && Passed;
  Passed = Evaluate("SetOptimumEvent()", "nullptr", "The raw events are unchanged", Incarnations.GetNRawEvents(), 3) && Passed;

  Incarnations.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for a valid event
bool UTRawEventIncarnations::TestIsAnyEventValid()
{
  bool Passed = true;

  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  MRERawEvent* B = CreateRawEvent(200.0);
  Incarnations.AddRawEvent(A);
  Incarnations.AddRawEvent(B);
  Passed = EvaluateTrue("IsAnyEventValid()", "two valid events", "A valid event is found", Incarnations.IsAnyEventValid()) && Passed;

  // A rejection makes an event invalid:
  A->SetRejectionReason(MRERawEvent::c_RejectionTooManyHits);
  Passed = EvaluateTrue("IsAnyEventValid()", "first event rejected", "The second event is still valid", Incarnations.IsAnyEventValid()) && Passed;
  B->SetValid(false);
  Passed = EvaluateFalse("IsAnyEventValid()", "both events not valid", "No valid event is found", Incarnations.IsAnyEventValid()) && Passed;

  // An event with a hit which is not valid is not valid:
  MRERawEvent* C = CreateRawEvent(300.0);
  C->GetRESEAt(0)->SetValid(false);
  Incarnations.AddRawEvent(C);
  Passed = EvaluateFalse("IsAnyEventValid()", "third event with a hit which is not valid", "No valid event is found", Incarnations.IsAnyEventValid()) && Passed;
  C->GetRESEAt(0)->SetValid(true);
  Passed = EvaluateTrue("IsAnyEventValid()", "third event with a valid hit", "The third event is valid", Incarnations.IsAnyEventValid()) && Passed;

  Incarnations.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the sorting by the track quality factor
bool UTRawEventIncarnations::TestSortByTrackQualityFactor()
{
  bool Passed = true;

  // Good are high: the highest quality factor comes first
  {
    MRawEventIncarnations Incarnations;
    vector<double> QualityFactors = { 5.0, 3.0, 7.0, 4.0 };
    for (double QualityFactor: QualityFactors) {
      Incarnations.AddRawEvent(CreateRawEvent(100.0, QualityFactor));
    }
    Incarnations.SortByTrackQualityFactor(true);
    vector<double> Expected = { 7.0, 5.0, 4.0, 3.0 };
    for (unsigned int i = 0; i < Expected.size(); ++i) {
      Passed = EvaluateNear("SortByTrackQualityFactor()", MString("good are high, position ") + i, "The quality factors are in descending order", Incarnations.GetRawEventAt(i)->GetTrackQualityFactor(), Expected[i], 1e-15) && Passed;
    }

    // Sort again in the other direction:
    Incarnations.SortByTrackQualityFactor(false);
    for (unsigned int i = 0; i < Expected.size(); ++i) {
      Passed = EvaluateNear("SortByTrackQualityFactor()", MString("good are low, position ") + i, "The quality factors are in ascending order", Incarnations.GetRawEventAt(i)->GetTrackQualityFactor(), Expected[Expected.size() - 1 - i], 1e-15) && Passed;
    }
    Passed = Evaluate("SortByTrackQualityFactor()", "four events", "Sorting keeps the number of raw events", Incarnations.GetNRawEvents(), 4) && Passed;
    Incarnations.DeleteAll();
  }

  // Equal quality factors: the event whose first RESE has the lower energy comes first, in both directions
  {
    vector<bool> Directions = { true, false };
    for (bool GoodAreHigh: Directions) {
      MString Input = "equal quality factors, good are low";
      if (GoodAreHigh == true) {
        Input = "equal quality factors, good are high";
      }
      MRawEventIncarnations Incarnations;
      Incarnations.AddRawEvent(CreateRawEvent(300.0, 5.0));
      Incarnations.AddRawEvent(CreateRawEvent(100.0, 5.0));
      Incarnations.AddRawEvent(CreateRawEvent(200.0, 5.0));
      Incarnations.SortByTrackQualityFactor(GoodAreHigh);
      Passed = EvaluateNear("SortByTrackQualityFactor()", Input, "The event with 100 keV is first", Incarnations.GetRawEventAt(0)->GetEnergy(), 100.0, 1e-12) && Passed;
      Passed = EvaluateNear("SortByTrackQualityFactor()", Input, "The event with 200 keV is second", Incarnations.GetRawEventAt(1)->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateNear("SortByTrackQualityFactor()", Input, "The event with 300 keV is third", Incarnations.GetRawEventAt(2)->GetEnergy(), 300.0, 1e-12) && Passed;
      Incarnations.DeleteAll();
    }
  }

  // Events without a quality factor are at the end of the list, in both directions
  {
    MRawEventIncarnations Incarnations;
    Incarnations.AddRawEvent(CreateRawEvent(100.0));
    Incarnations.AddRawEvent(CreateRawEvent(100.0, 5.0));
    Incarnations.AddRawEvent(CreateRawEvent(100.0, 3.0));
    Incarnations.AddRawEvent(CreateRawEvent(100.0, 7.0));
    Incarnations.SortByTrackQualityFactor(false);
    Passed = EvaluateNear("SortByTrackQualityFactor()", "no quality factor at the front, good are low", "The lowest quality factor is first", Incarnations.GetRawEventAt(0)->GetTrackQualityFactor(), 3.0, 1e-15) && Passed;
    Passed = Evaluate("SortByTrackQualityFactor()", "no quality factor at the front, good are low", "The event without quality factor is last", Incarnations.GetRawEventAt(3)->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor) && Passed;
    Incarnations.DeleteAll();
  }
  {
    MRawEventIncarnations Incarnations;
    Incarnations.AddRawEvent(CreateRawEvent(100.0));
    Incarnations.AddRawEvent(CreateRawEvent(100.0, 5.0));
    Incarnations.AddRawEvent(CreateRawEvent(100.0, 3.0));
    Incarnations.AddRawEvent(CreateRawEvent(100.0, 7.0));
    Incarnations.SortByTrackQualityFactor(true);
    // Defect REV-C-7: The comparison for "good are high" is no strict weak ordering, so the event without quality factor stays where it is - here at the front
    Passed = ExpectedToFail(EvaluateNear("SortByTrackQualityFactor()", "no quality factor at the front, good are high", "The highest quality factor is first", Incarnations.GetRawEventAt(0)->GetTrackQualityFactor(), 7.0, 1e-15)) && Passed;
    Passed = ExpectedToFail(Evaluate("SortByTrackQualityFactor()", "no quality factor at the front, good are high", "The event without quality factor is last", Incarnations.GetRawEventAt(3)->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor)) && Passed;
    Incarnations.DeleteAll();
  }

  // A single event:
  {
    MRawEventIncarnations Incarnations;
    MRERawEvent* Single = CreateRawEvent(100.0, 5.0);
    Incarnations.AddRawEvent(Single);
    Incarnations.SortByTrackQualityFactor(true);
    Passed = EvaluateTrue("SortByTrackQualityFactor()", "one event", "A single event stays", Incarnations.GetRawEventAt(0) == Single) && Passed;
    Incarnations.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTRawEventIncarnations::TestToString()
{
  bool Passed = true;

  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  MRERawEvent* B = CreateRawEvent(200.0);
  Incarnations.AddRawEvent(A);
  Incarnations.AddRawEvent(B);

  // The text is the head line and for each raw event its number and its text, one level deeper
  MString Expected = "Raw event with the following possible combinations:\n";
  Expected += "Combination 1:\n";
  Expected += A->ToString(true, 1);
  Expected += "Combination 2:\n";
  Expected += B->ToString(true, 1);
  Passed = Evaluate("ToString()", "two events", "The text lists both raw events", Incarnations.ToString(), Expected) && Passed;

  MString Indented = "      Raw event with the following possible combinations:\n";
  Indented += "      Combination 1:\n";
  Indented += A->ToString(false, 3);
  Indented += "      Combination 2:\n";
  Indented += B->ToString(false, 3);
  Passed = Evaluate("ToString()", "two events, without links, level 2", "The lines are indented by 6 blanks and the raw events are one level deeper", Incarnations.ToString(false, 2), Indented) && Passed;

  Incarnations.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the destructor keeps the raw events
bool UTRawEventIncarnations::TestDestructor()
{
  bool Passed = true;

  MRERawEvent* A = CreateRawEvent(100.0);
  {
    MRawEventIncarnations Incarnations;
    Incarnations.SetInitialRawEvent(A);
  }
  // The destructor does not delete the raw events, as its description says
  Passed = EvaluateNear("~MRawEventIncarnations()", "one event", "The raw event is still alive after the list is gone", A->GetEnergy(), 100.0, 1e-12) && Passed;
  Passed = Evaluate("~MRawEventIncarnations()", "one event", "The raw event still has its hit", A->GetNRESEs(), 1) && Passed;
  delete A;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new raw event with one hit of the given energy (keV) and the given track quality factor - the caller owns it
MRERawEvent* UTRawEventIncarnations::CreateRawEvent(double Energy, double TrackQualityFactor)
{
  MREHit* Hit = new MREHit();
  Hit->SetPosition(MVector(1.0, 2.0, 3.0));
  Hit->SetEnergy(Energy);

  MRERawEvent* Event = new MRERawEvent();
  Event->AddRESE(Hit);
  Event->SetTrackQualityFactor(TrackQualityFactor);

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a negative index returns a nullptr
bool UTRawEventIncarnations::IsNegativeIndexRejected()
{
  MRawEventIncarnations Incarnations;
  Incarnations.AddRawEvent(CreateRawEvent(100.0));
  DisableDefaultStreams();
  MRERawEvent* Event = Incarnations.GetRawEventAt(-1);
  EnableDefaultStreams();
  Incarnations.DeleteAll();
  if (Event != nullptr) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if setting a raw event behind the end of the list changes nothing
bool UTRawEventIncarnations::IsSetBehindEndRejected()
{
  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  MRERawEvent* B = CreateRawEvent(200.0);
  Incarnations.AddRawEvent(A);
  DisableDefaultStreams();
  Incarnations.SetRawEventAt(B, 1);
  EnableDefaultStreams();
  bool IsUnchanged = false;
  if (Incarnations.GetNRawEvents() == 1 && Incarnations.GetRawEventAt(0) == A) {
    IsUnchanged = true;
  }
  delete B;
  Incarnations.DeleteAll();

  return IsUnchanged;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if removing a raw event which is not in the list changes nothing
bool UTRawEventIncarnations::IsRemoveOfUnknownRawEventIgnored()
{
  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  MRERawEvent* Unknown = CreateRawEvent(200.0);
  Incarnations.AddRawEvent(A);
  Incarnations.RemoveRawEvent(Unknown);
  bool IsUnchanged = false;
  if (Incarnations.GetNRawEvents() == 1 && Incarnations.GetRawEventAt(0) == A) {
    IsUnchanged = true;
  }
  delete Unknown;
  Incarnations.DeleteAll();

  return IsUnchanged;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if deleting a raw event which is not in the list changes nothing
bool UTRawEventIncarnations::IsDeleteOfUnknownRawEventIgnored()
{
  MRawEventIncarnations Incarnations;
  MRERawEvent* A = CreateRawEvent(100.0);
  MRERawEvent* Unknown = CreateRawEvent(200.0);
  Incarnations.AddRawEvent(A);
  Incarnations.DeleteRawEvent(Unknown);
  bool IsUnchanged = false;
  if (Incarnations.GetNRawEvents() == 1 && Incarnations.GetRawEventAt(0) == A) {
    IsUnchanged = true;
  }
  Incarnations.DeleteAll();

  return IsUnchanged;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Hide the info messages of the geometry manager:
  gErrorIgnoreLevel = kWarning;

  UTRawEventIncarnations Test;
  return Test.Run() == true ? 0 : 1;
}


// UTRawEventIncarnations.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
