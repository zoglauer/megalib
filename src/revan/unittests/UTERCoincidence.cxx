/*
 * UTERCoincidence.cxx
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
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MERCoincidence.h"
#include "MRawEventIncarnations.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MTime.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the coincidence search MERCoincidence of revan
class UTERCoincidence : public UTRevanBase
{
public:
  //! Default constructor
  UTERCoincidence() : UTRevanBase("UTERCoincidence") {}
  //! Default destructor
  virtual ~UTERCoincidence() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the search without a coincidence window
  bool TestNoCoincidenceSearch();
  //! Test the merging of the events in the coincidence window
  bool TestWindow();
  //! Test the limits of the coincidence window
  bool TestWindowLimits();
  //! Test the search if all events of the list are coincident
  bool TestAllCoincident();
  //! Test events without hits and a jump back in time
  bool TestSpecialEvents();
  //! Test repeated searches in one list
  bool TestRepeatedSearch();
  //! Test the options text
  bool TestToString();

  //! Add a new event with the given ID, time in seconds, and number of hits (each 100 keV) to the list
  void AddEvent(MRawEventIncarnations& List, unsigned long ID, double Time, unsigned int NHits);
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCoincidence::Run()
{
  bool Passed = true;

  Passed = TestNoCoincidenceSearch() && Passed;
  Passed = TestWindow() && Passed;
  Passed = TestWindowLimits() && Passed;
  Passed = TestAllCoincident() && Passed;
  Passed = TestSpecialEvents() && Passed;
  Passed = TestRepeatedSearch() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a new event with the given ID, time in seconds, and number of hits (each 100 keV) to the list
void UTERCoincidence::AddEvent(MRawEventIncarnations& List, unsigned long ID, double Time, unsigned int NHits)
{
  // The list does not own the event: the test deletes it
  MRERawEvent* Event = new MRERawEvent();
  Event->SetEventID(ID);
  Event->SetEventTime(MTime(Time));
  for (unsigned int h = 0; h < NHits; ++h) {
    Event->AddRESE(CreateHit(MVector(1.0*h, 1.0*ID, 0.0), 100.0, MDDetector::c_Strip2D));
  }
  List.AddRawEvent(Event);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search without a coincidence window
bool UTERCoincidence::TestNoCoincidenceSearch()
{
  bool Passed = true;

  MERCoincidence Coincidence;

  // An empty list
  MRawEventIncarnations List;
  Passed = EvaluateTrue("Search()", "no window, empty list", "Nothing is returned", Coincidence.Search(&List, false) == nullptr) && Passed;
  Passed = EvaluateTrue("Search()", "no window, empty list, clear", "Nothing is returned", Coincidence.Search(&List, true) == nullptr) && Passed;

  // Two events at the same time: without a window each event is returned on its own
  AddEvent(List, 1, 10.0, 2);
  AddEvent(List, 2, 10.0, 1);
  MRERawEvent* First = List.GetRawEventAt(0);
  MRERawEvent* Second = List.GetRawEventAt(1);
  int FirstHitID = First->GetRESEAt(0)->GetID();

  MRERawEvent* Found = Coincidence.Search(&List, false);
  Passed = EvaluateTrue("Search()", "no window, two events at the same time", "The first event is returned", Found == First) && Passed;
  Passed = Evaluate("Search()", "no window, two events at the same time", "The first event is removed from the list", List.GetNRawEvents(), 1) && Passed;
  Passed = Evaluate("Search()", "no window, two events at the same time", "The first event keeps its two hits", First->GetNRESEs(), 2) && Passed;
  Passed = Evaluate("Search()", "no window, two events at the same time", "The first event keeps its ID", First->GetEventID(), 1UL) && Passed;
  Passed = EvaluateNear("Search()", "no window, two events at the same time", "The coincidence window of the event is zero", First->GetCoincidenceWindow().GetAsSeconds(), 0.0, 1e-15) && Passed;
  // The hits get new IDs, which are unique
  Passed = EvaluateTrue("Search()", "no window, two events at the same time", "The first hit has a new ID", First->GetRESEAt(0)->GetID() != FirstHitID) && Passed;
  Passed = EvaluateTrue("Search()", "no window, two events at the same time", "The two hits have different IDs", First->GetRESEAt(0)->GetID() != First->GetRESEAt(1)->GetID()) && Passed;
  Passed = EvaluateTrue("Search()", "no window, two events at the same time", "The event and its hits have different IDs", First->GetID() != First->GetRESEAt(0)->GetID() && First->GetID() != First->GetRESEAt(1)->GetID()) && Passed;

  Found = Coincidence.Search(&List, false);
  Passed = EvaluateTrue("Search()", "no window, second search", "The second event is returned", Found == Second) && Passed;
  Passed = Evaluate("Search()", "no window, second search", "The list is empty", List.GetNRawEvents(), 0) && Passed;
  Passed = EvaluateTrue("Search()", "no window, third search", "Nothing is returned", Coincidence.Search(&List, false) == nullptr) && Passed;

  delete First;
  delete Second;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the merging of the events in the coincidence window
bool UTERCoincidence::TestWindow()
{
  bool Passed = true;

  MERCoincidence Coincidence;
  Passed = EvaluateTrue("SetCoincidenceWindow()", "1 ms", "The window is accepted", Coincidence.SetCoincidenceWindow(0.001)) && Passed;

  // An empty list
  MRawEventIncarnations List;
  Passed = EvaluateTrue("Search()", "window 1 ms, empty list", "Nothing is returned", Coincidence.Search(&List, true) == nullptr) && Passed;

  // Three events at 10 s, 10.0002 s, and 10.0005 s are in the window, the one at 10.002 s is not
  AddEvent(List, 11, 10.0, 2);
  AddEvent(List, 12, 10.0002, 1);
  AddEvent(List, 13, 10.0005, 3);
  AddEvent(List, 14, 10.002, 1);
  MRERawEvent* First = List.GetRawEventAt(0);
  MRERawEvent* Last = List.GetRawEventAt(3);

  MRERawEvent* Found = Coincidence.Search(&List, false);
  Passed = EvaluateTrue("Search()", "window 1 ms, three coincident events and a later one", "The first event is returned", Found == First) && Passed;
  Passed = Evaluate("Search()", "window 1 ms, three coincident events and a later one", "The event has the six hits of the three events", First->GetNRESEs(), 6) && Passed;
  Passed = EvaluateNear("GetEnergy()", "window 1 ms, three coincident events and a later one", "The event has 600 keV", First->GetEnergy(), 600.0, 1e-12) && Passed;
  // The ID is the one of the last merged event
  Passed = Evaluate("Search()", "window 1 ms, three coincident events and a later one", "The event has the ID of the last coincident event", First->GetEventID(), 13UL) && Passed;
  Passed = EvaluateNear("Search()", "window 1 ms, three coincident events and a later one", "The event keeps its time", First->GetEventTime().GetAsSeconds(), 10.0, 1e-9) && Passed;
  // The largest time difference: 10.0005 s - 10 s
  Passed = EvaluateNear("Search()", "window 1 ms, three coincident events and a later one", "The coincidence window of the event is 0.5 ms", First->GetCoincidenceWindow().GetAsSeconds(), 0.0005, 1e-9) && Passed;
  Passed = Evaluate("Search()", "window 1 ms, three coincident events and a later one", "Only the later event stays in the list", List.GetNRawEvents(), 1) && Passed;
  Passed = EvaluateTrue("Search()", "window 1 ms, three coincident events and a later one", "The later event is not changed", List.GetNRawEvents() == 1 && List.GetRawEventAt(0) == Last && Last->GetNRESEs() == 1 && Last->GetEventID() == 14UL) && Passed;
  // All hits have unique IDs
  bool Unique = true;
  for (int r = 0; r < First->GetNRESEs(); ++r) {
    for (int s = r + 1; s < First->GetNRESEs(); ++s) {
      if (First->GetRESEAt(r)->GetID() == First->GetRESEAt(s)->GetID()) Unique = false;
    }
  }
  Passed = EvaluateTrue("Search()", "window 1 ms, three coincident events and a later one", "The hits have unique IDs", Unique) && Passed;

  delete First;
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the coincidence window
bool UTERCoincidence::TestWindowLimits()
{
  bool Passed = true;

  MERCoincidence Coincidence;
  Coincidence.SetCoincidenceWindow(0.001);

  // A difference of exactly the window is coincident: 10 s and 10.001 s can be represented in ns
  {
    MRawEventIncarnations List;
    AddEvent(List, 1, 10.0, 1);
    AddEvent(List, 2, 10.001, 1);
    AddEvent(List, 3, 20.0, 1);
    MRERawEvent* Found = Coincidence.Search(&List, false);
    Passed = EvaluateTrue("Search()", "window 1 ms, difference 1 ms", "An event is returned", Found != nullptr) && Passed;
    if (Found != nullptr) {
      Passed = Evaluate("Search()", "window 1 ms, difference 1 ms", "The events are merged", Found->GetNRESEs(), 2) && Passed;
      delete Found;
    }
    List.DeleteAll();
  }

  // 1 us more is not coincident
  {
    MRawEventIncarnations List;
    AddEvent(List, 1, 10.0, 1);
    AddEvent(List, 2, 10.001001, 1);
    AddEvent(List, 3, 20.0, 1);
    MRERawEvent* Found = Coincidence.Search(&List, false);
    Passed = EvaluateTrue("Search()", "window 1 ms, difference 1.001 ms", "An event is returned", Found != nullptr) && Passed;
    if (Found != nullptr) {
      Passed = Evaluate("Search()", "window 1 ms, difference 1.001 ms", "The events are not merged", Found->GetNRESEs(), 1) && Passed;
      Passed = Evaluate("Search()", "window 1 ms, difference 1.001 ms", "The event keeps its ID", Found->GetEventID(), 1UL) && Passed;
      Passed = EvaluateNear("Search()", "window 1 ms, difference 1.001 ms", "The coincidence window of the event is zero", Found->GetCoincidenceWindow().GetAsSeconds(), 0.0, 1e-15) && Passed;
      delete Found;
    }
    Passed = Evaluate("Search()", "window 1 ms, difference 1.001 ms", "Two events stay in the list", List.GetNRawEvents(), 2) && Passed;
    List.DeleteAll();
  }

  // 1 us less is coincident
  {
    MRawEventIncarnations List;
    AddEvent(List, 1, 10.0, 1);
    AddEvent(List, 2, 10.000999, 1);
    AddEvent(List, 3, 20.0, 1);
    MRERawEvent* Found = Coincidence.Search(&List, false);
    Passed = EvaluateTrue("Search()", "window 1 ms, difference 0.999 ms", "An event is returned", Found != nullptr) && Passed;
    if (Found != nullptr) {
      Passed = Evaluate("Search()", "window 1 ms, difference 0.999 ms", "The events are merged", Found->GetNRESEs(), 2) && Passed;
      delete Found;
    }
    List.DeleteAll();
  }

  // A window of zero merges only events at the same time
  {
    MERCoincidence Zero;
    Zero.SetCoincidenceWindow(0.0);
    MRawEventIncarnations List;
    AddEvent(List, 1, 10.0, 1);
    AddEvent(List, 2, 10.0, 1);
    AddEvent(List, 3, 10.000001, 1);
    MRERawEvent* Found = Zero.Search(&List, false);
    Passed = EvaluateTrue("Search()", "window 0, two events at the same time and one 1 us later", "An event is returned", Found != nullptr) && Passed;
    if (Found != nullptr) {
      Passed = Evaluate("Search()", "window 0, two events at the same time and one 1 us later", "The two events at the same time are merged", Found->GetNRESEs(), 2) && Passed;
      delete Found;
    }
    Passed = Evaluate("Search()", "window 0, two events at the same time and one 1 us later", "The later event stays in the list", List.GetNRawEvents(), 1) && Passed;
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search if all events of the list are coincident
bool UTERCoincidence::TestAllCoincident()
{
  bool Passed = true;

  MERCoincidence Coincidence;
  Coincidence.SetCoincidenceWindow(0.001);

  // One event: a later event could still be coincident, so it is only returned if the list is cleared
  {
    MRawEventIncarnations List;
    AddEvent(List, 1, 10.0, 1);
    MRERawEvent* Event = List.GetRawEventAt(0);
    Passed = EvaluateTrue("Search()", "window 1 ms, one event, no clear", "Nothing is returned", Coincidence.Search(&List, false) == nullptr) && Passed;
    Passed = Evaluate("Search()", "window 1 ms, one event, no clear", "The event stays in the list", List.GetNRawEvents(), 1) && Passed;
    Passed = EvaluateTrue("Search()", "window 1 ms, one event, clear", "The event is returned", Coincidence.Search(&List, true) == Event) && Passed;
    Passed = Evaluate("Search()", "window 1 ms, one event, clear", "The list is empty", List.GetNRawEvents(), 0) && Passed;
    delete Event;
  }

  // Two coincident events and nothing later
  {
    MRawEventIncarnations List;
    AddEvent(List, 1, 10.0, 1);
    AddEvent(List, 2, 10.0005, 2);
    MRERawEvent* First = List.GetRawEventAt(0);
    Passed = EvaluateTrue("Search()", "window 1 ms, two coincident events, no clear", "Nothing is returned", Coincidence.Search(&List, false) == nullptr) && Passed;
    Passed = Evaluate("Search()", "window 1 ms, two coincident events, no clear", "Both events stay in the list", List.GetNRawEvents(), 2) && Passed;
    Passed = Evaluate("Search()", "window 1 ms, two coincident events, no clear", "The first event is not changed", First->GetNRESEs(), 1) && Passed;
    MRERawEvent* Found = Coincidence.Search(&List, true);
    Passed = EvaluateTrue("Search()", "window 1 ms, two coincident events, clear", "The first event is returned", Found == First) && Passed;
    Passed = Evaluate("Search()", "window 1 ms, two coincident events, clear", "The event has the three hits of both events", First->GetNRESEs(), 3) && Passed;
    Passed = Evaluate("Search()", "window 1 ms, two coincident events, clear", "The list is empty", List.GetNRawEvents(), 0) && Passed;
    delete First;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events without hits and a jump back in time
bool UTERCoincidence::TestSpecialEvents()
{
  bool Passed = true;

  MERCoincidence Coincidence;
  Coincidence.SetCoincidenceWindow(0.001);

  // A coincident event without hits adds nothing, but it is consumed
  {
    MRawEventIncarnations List;
    AddEvent(List, 1, 10.0, 1);
    AddEvent(List, 2, 10.0005, 0);
    AddEvent(List, 3, 20.0, 1);
    MRERawEvent* Found = Coincidence.Search(&List, false);
    Passed = EvaluateTrue("Search()", "window 1 ms, coincident event without hits", "An event is returned", Found != nullptr) && Passed;
    if (Found != nullptr) {
      Passed = Evaluate("Search()", "window 1 ms, coincident event without hits", "The event has its one hit", Found->GetNRESEs(), 1) && Passed;
      Passed = EvaluateNear("Search()", "window 1 ms, coincident event without hits", "An event without hits does not open the coincidence window", Found->GetCoincidenceWindow().GetAsSeconds(), 0.0, 1e-15) && Passed;
      delete Found;
    }
    Passed = Evaluate("Search()", "window 1 ms, coincident event without hits", "One event stays in the list", List.GetNRawEvents(), 1) && Passed;
    List.DeleteAll();
  }

  // The next event is more than 0.1 s earlier, as at the start of a new file: the search stops there
  {
    MRawEventIncarnations List;
    AddEvent(List, 1, 10.0, 1);
    AddEvent(List, 2, 0.5, 1);
    DisableDefaultStreams();
    MRERawEvent* Found = Coincidence.Search(&List, false);
    EnableDefaultStreams();
    Passed = EvaluateTrue("Search()", "window 1 ms, next event 9.5 s earlier", "An event is returned", Found != nullptr) && Passed;
    if (Found != nullptr) {
      Passed = Evaluate("Search()", "window 1 ms, next event 9.5 s earlier", "The events are not merged", Found->GetNRESEs(), 1) && Passed;
      Passed = Evaluate("Search()", "window 1 ms, next event 9.5 s earlier", "The first event is returned", Found->GetEventID(), 1UL) && Passed;
      delete Found;
    }
    Passed = Evaluate("Search()", "window 1 ms, next event 9.5 s earlier", "The earlier event stays in the list", List.GetNRawEvents(), 1) && Passed;
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test repeated searches in one list
bool UTERCoincidence::TestRepeatedSearch()
{
  bool Passed = true;

  MERCoincidence Coincidence;
  Coincidence.SetCoincidenceWindow(0.001);

  // Pairs of events at 1 s, 2 s, and 3 s, and a single event at 4 s
  MRawEventIncarnations List;
  AddEvent(List, 1, 1.0, 1);
  AddEvent(List, 2, 1.0001, 1);
  AddEvent(List, 3, 2.0, 1);
  AddEvent(List, 4, 2.0001, 1);
  AddEvent(List, 5, 3.0, 1);
  AddEvent(List, 6, 3.0001, 1);
  AddEvent(List, 7, 4.0, 1);

  unsigned long ExpectedIDs[3] = { 2UL, 4UL, 6UL };
  for (unsigned int i = 0; i < 3; ++i) {
    MRERawEvent* Found = Coincidence.Search(&List, false);
    MString Input = MString("window 1 ms, search ") + (i + 1);
    Passed = EvaluateTrue("Search()", Input, "An event is returned", Found != nullptr) && Passed;
    if (Found != nullptr) {
      Passed = Evaluate("Search()", Input, "The event has the hits of a pair", Found->GetNRESEs(), 2) && Passed;
      Passed = Evaluate("Search()", Input, "The event has the ID of the second event of the pair", Found->GetEventID(), ExpectedIDs[i]) && Passed;
      Passed = EvaluateNear("Search()", Input, "The event has the time of the first event of the pair", Found->GetEventTime().GetAsSeconds(), 1.0 + i, 1e-9) && Passed;
      delete Found;
    }
  }
  Passed = Evaluate("Search()", "window 1 ms, after three searches", "The single event stays in the list", List.GetNRawEvents(), 1) && Passed;
  Passed = EvaluateTrue("Search()", "window 1 ms, fourth search, no clear", "Nothing is returned", Coincidence.Search(&List, false) == nullptr) && Passed;
  MRERawEvent* Found = Coincidence.Search(&List, true);
  Passed = EvaluateTrue("Search()", "window 1 ms, fifth search, clear", "The single event is returned", Found != nullptr && Found->GetEventID() == 7UL) && Passed;
  delete Found;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the options text
bool UTERCoincidence::TestToString()
{
  bool Passed = true;

  // Without a window only the title is given
  MERCoincidence Coincidence;
  Passed = Evaluate("ToString()", "no window", "The text has only the title", Coincidence.ToString(false), MString("# Coincidence options:\n# \n# \n")) && Passed;

  Coincidence.SetCoincidenceWindow(0.001);
  Passed = Evaluate("ToString()", "window 1 ms", "The text has the window in seconds", Coincidence.ToString(false), MString("# Coincidence options:\n# \n# Coincidence window: 0.001\n# \n")) && Passed;
  Passed = Evaluate("ToString()", "core only", "The core text is the same", Coincidence.ToString(true), Coincidence.ToString(false)) && Passed;

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

  UTERCoincidence Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCoincidence.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
