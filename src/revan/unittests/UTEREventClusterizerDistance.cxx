/*
 * UTEREventClusterizerDistance.cxx
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
#include <limits>
#include <sstream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MEREventClusterizerDistance.h"
#include "MRawEventIncarnationList.h"
#include "MRawEventIncarnations.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MTime.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the event clusterizer by distance MEREventClusterizerDistance of revan
class UTEREventClusterizerDistance : public UTRevanBase
{
public:
  //! Default constructor
  UTEREventClusterizerDistance() : UTRevanBase("UTEREventClusterizerDistance") {}
  //! Default destructor
  virtual ~UTEREventClusterizerDistance() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the distance cut off and the options text
  bool TestDistanceCutOff();
  //! Test events which are not split
  bool TestNoSplit();
  //! Test the split of an event into two
  bool TestSplit();
  //! Test the limits of the distance cut off
  bool TestCutOffLimits();
  //! Test the merging of groups of hits
  bool TestGroupMerging();
  //! Test incarnations with several events
  bool TestSeveralIncarnations();
  //! Test a list with two events
  bool TestTwoEvents();
  //! Test an empty list
  bool TestEmptyList();

  //! Add new incarnations with one event (ID 42, time 12.5 s) with hits of 100 keV, 110 keV, ... at the given x positions in cm to the list, return the incarnations
  MRawEventIncarnations* AddEvent(MRawEventIncarnationList& List, const vector<double>& XPositions);
  //! Analyze the list and hide the output
  bool Clusterize(MEREventClusterizerDistance& Clusterizer, MRawEventIncarnationList& List);
  //! Delete the incarnations (not their events) if the list does not hold them any more
  void DeleteIfNotInList(MRawEventIncarnationList& List, MRawEventIncarnations* Incarnations);
  //! Return the first event of the list with the given number of hits, or a nullptr
  MRERawEvent* FindEvent(MRawEventIncarnationList& List, int NHits);
  //! Return the number of all hits in the list
  int GetNHits(MRawEventIncarnationList& List);
  //! Return true if the analysis of an empty list returns true
  bool IsEmptyListAnalyzed();

  //! The output to cout of the last analysis
  MString m_Output;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTEREventClusterizerDistance::Run()
{
  bool Passed = true;

  Passed = TestDistanceCutOff() && Passed;
  Passed = TestNoSplit() && Passed;
  Passed = TestSplit() && Passed;
  Passed = TestCutOffLimits() && Passed;
  Passed = TestGroupMerging() && Passed;
  Passed = TestSeveralIncarnations() && Passed;
  Passed = TestTwoEvents() && Passed;
  Passed = TestEmptyList() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Add new incarnations with one event (ID 42, time 12.5 s) with hits of 100 keV, 110 keV, ... at the given x positions in cm to the list, return the incarnations
MRawEventIncarnations* UTEREventClusterizerDistance::AddEvent(MRawEventIncarnationList& List, const vector<double>& XPositions)
{
  MRERawEvent* Event = new MRERawEvent();
  Event->SetEventID(42);
  Event->SetEventTime(MTime(12.5));
  for (unsigned int p = 0; p < XPositions.size(); ++p) {
    Event->AddRESE(CreateHit(MVector(XPositions[p], 2.0, -1.0), 100.0 + 10.0*p, MDDetector::c_Strip2D));
  }
  MRawEventIncarnations* Incarnations = new MRawEventIncarnations();
  Incarnations->AddRawEvent(Event);
  List.Add(Incarnations);

  return Incarnations;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze the list and hide the output
bool UTEREventClusterizerDistance::Clusterize(MEREventClusterizerDistance& Clusterizer, MRawEventIncarnationList& List)
{
  DisableDefaultStreams();
  BeginCaptureOutput();
  bool Analyzed = Clusterizer.Analyze(&List);
  m_Output = EndCaptureOutput();
  EnableDefaultStreams();

  return Analyzed;
}


////////////////////////////////////////////////////////////////////////////////


//! Delete the incarnations (not their events) if the list does not hold them any more
void UTEREventClusterizerDistance::DeleteIfNotInList(MRawEventIncarnationList& List, MRawEventIncarnations* Incarnations)
{
  for (unsigned int i = 0; i < List.Size(); ++i) {
    if (List.Get(i) == Incarnations) {
      return;
    }
  }
  delete Incarnations;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the first event of the list with the given number of hits, or a nullptr
MRERawEvent* UTEREventClusterizerDistance::FindEvent(MRawEventIncarnationList& List, int NHits)
{
  for (unsigned int i = 0; i < List.Size(); ++i) {
    if (List.Get(i)->GetNRawEvents() == 1 && List.Get(i)->GetRawEventAt(0)->GetNRESEs() == NHits) {
      return List.Get(i)->GetRawEventAt(0);
    }
  }

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of all hits in the list
int UTEREventClusterizerDistance::GetNHits(MRawEventIncarnationList& List)
{
  int NHits = 0;
  for (unsigned int i = 0; i < List.Size(); ++i) {
    for (int e = 0; e < List.Get(i)->GetNRawEvents(); ++e) {
      NHits += List.Get(i)->GetRawEventAt(e)->GetNRESEs();
    }
  }

  return NHits;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the analysis of an empty list returns true
bool UTEREventClusterizerDistance::IsEmptyListAnalyzed()
{
  MEREventClusterizerDistance Clusterizer;
  MRawEventIncarnationList List;
  return Clusterize(Clusterizer, List);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance cut off and the options text
bool UTEREventClusterizerDistance::TestDistanceCutOff()
{
  bool Passed = true;

  MEREventClusterizerDistance Clusterizer;

  // The default is no cut off: the largest double
  ostringstream NoCutOff;
  NoCutOff<<"# Event clustering options:"<<endl<<"# "<<endl<<"# Distance cut "<<numeric_limits<double>::max()<<endl<<"# "<<endl;
  Passed = Evaluate("ToString()", "default", "The text has no cut off", Clusterizer.ToString(), MString(NoCutOff.str())) && Passed;

  Passed = EvaluateTrue("SetDistanceCutOff()", "2.5 cm", "A positive cut off is accepted", Clusterizer.SetDistanceCutOff(2.5)) && Passed;
  Passed = Evaluate("ToString()", "2.5 cm", "The text has the cut off", Clusterizer.ToString(), MString("# Event clustering options:\n# \n# Distance cut 2.5\n# \n")) && Passed;
  Passed = Evaluate("ToString()", "core only", "The core text is the same", Clusterizer.ToString(true), Clusterizer.ToString(false)) && Passed;

  // Zero and negative values are rejected and switch the cut off off
  DisableDefaultStreams();
  bool Zero = Clusterizer.SetDistanceCutOff(0.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetDistanceCutOff()", "0 cm", "A cut off of zero is rejected", Zero) && Passed;
  Passed = Evaluate("ToString()", "after 0 cm", "The text has no cut off", Clusterizer.ToString(), MString(NoCutOff.str())) && Passed;

  Clusterizer.SetDistanceCutOff(2.5);
  DisableDefaultStreams();
  bool Negative = Clusterizer.SetDistanceCutOff(-1.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetDistanceCutOff()", "-1 cm", "A negative cut off is rejected", Negative) && Passed;
  Passed = Evaluate("ToString()", "after -1 cm", "The text has no cut off", Clusterizer.ToString(), MString(NoCutOff.str())) && Passed;

  // The analysis of incarnations is the one of the base class, which the derived class hides
  MRawEventIncarnations Incarnations;
  MEREventClusterizer* Base = &Clusterizer;
  Passed = EvaluateFalse("Analyze()", "incarnations", "The analysis of incarnations is not available", Base->Analyze(&Incarnations)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events which are not split
bool UTEREventClusterizerDistance::TestNoSplit()
{
  bool Passed = true;

  // Without a cut off all hits belong to one event
  {
    MEREventClusterizerDistance Clusterizer;
    MRawEventIncarnationList List;
    MRawEventIncarnations* Incarnations = AddEvent(List, { 0.0, 50.0, -70.0 });
    MRERawEvent* Event = Incarnations->GetRawEventAt(0);
    Passed = EvaluateTrue("Analyze()", "no cut off, three hits", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
    // Defect REV-E-14: the analysis prints the hits of each event with cout
    Passed = ExpectedToFail(Evaluate("Analyze()", "no cut off, three hits", "Nothing is printed", m_Output, MString(""))) && Passed;
    Passed = Evaluate("Analyze()", "no cut off, three hits", "The list has one entry", List.Size(), 1U) && Passed;
    Passed = EvaluateTrue("Analyze()", "no cut off, three hits", "The event is still in the list", FindEvent(List, 3) == Event) && Passed;
    Passed = EvaluateNear("GetEnergy()", "no cut off, three hits", "The event has 330 keV", Event->GetEnergy(), 330.0, 1e-12) && Passed;
    DeleteIfNotInList(List, Incarnations);
    List.DeleteAll();
  }

  // An event with one hit and an event without hits are not touched
  {
    MEREventClusterizerDistance Clusterizer;
    Clusterizer.SetDistanceCutOff(1.0);
    MRawEventIncarnationList List;
    MRawEventIncarnations* Single = AddEvent(List, { 3.0 });
    MRawEventIncarnations* Empty = AddEvent(List, { });
    Passed = EvaluateTrue("Analyze()", "cut off 1 cm, one hit and no hits", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
    Passed = Evaluate("Analyze()", "cut off 1 cm, one hit and no hits", "The list has two entries", List.Size(), 2U) && Passed;
    if (List.Size() == 2) {
      Passed = EvaluateTrue("Analyze()", "cut off 1 cm, one hit", "The first incarnations are not replaced", List.Get(0) == Single) && Passed;
      Passed = EvaluateTrue("Analyze()", "cut off 1 cm, no hits", "The second incarnations are not replaced", List.Get(1) == Empty) && Passed;
    }
    Passed = Evaluate("Analyze()", "cut off 1 cm, one hit and no hits", "There is still one hit", GetNHits(List), 1) && Passed;
    DeleteIfNotInList(List, Single);
    DeleteIfNotInList(List, Empty);
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the split of an event into two
bool UTEREventClusterizerDistance::TestSplit()
{
  bool Passed = true;

  // Hits at x = 0 and 1 cm (100 and 110 keV) and at x = 5 and 6 cm (120 and 130 keV) with a cut off of 2 cm
  MEREventClusterizerDistance Clusterizer;
  Clusterizer.SetDistanceCutOff(2.0);
  MRawEventIncarnationList List;
  MRawEventIncarnations* Incarnations = AddEvent(List, { 0.0, 1.0, 5.0, 6.0 });
  MRERawEvent* Original = Incarnations->GetRawEventAt(0);
  MRESE* FirstHit = Original->GetRESEAt(0);
  MRESE* ThirdHit = Original->GetRESEAt(2);
  Passed = EvaluateTrue("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
  Passed = Evaluate("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The list has two entries", List.Size(), 2U) && Passed;
  Passed = Evaluate("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "All four hits are in the list", GetNHits(List), 4) && Passed;
  if (List.Size() == 2 && List.Get(0)->GetNRawEvents() == 1 && List.Get(1)->GetNRawEvents() == 1) {
    MRERawEvent* First = List.Get(0)->GetRawEventAt(0);
    MRERawEvent* Second = List.Get(1)->GetRawEventAt(0);
    Passed = EvaluateTrue("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The original event is the first one", First == Original) && Passed;
    Passed = Evaluate("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The first event has two hits", First->GetNRESEs(), 2) && Passed;
    Passed = Evaluate("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The second event has two hits", Second->GetNRESEs(), 2) && Passed;
    Passed = EvaluateTrue("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The first event has the hit at 0 cm", First->ContainsRESE(FirstHit)) && Passed;
    Passed = EvaluateTrue("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The second event has the hit at 5 cm", Second->ContainsRESE(ThirdHit)) && Passed;
    Passed = EvaluateNear("GetEnergy()", "cut off 2 cm, two pairs 4 cm apart", "The first event has 210 keV", First->GetEnergy(), 210.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetEnergy()", "cut off 2 cm, two pairs 4 cm apart", "The second event has 250 keV", Second->GetEnergy(), 250.0, 1e-12) && Passed;
    // The new event is a copy of the original one without its hits
    Passed = Evaluate("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The second event has the ID of the original", Second->GetEventID(), 42UL) && Passed;
    Passed = EvaluateNear("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "The second event has the time of the original", Second->GetEventTime().GetAsSeconds(), 12.5, 1e-9) && Passed;
    Passed = EvaluateTrue("Analyze()", "cut off 2 cm, two pairs 4 cm apart", "Both events are valid", First->IsValid() == true && Second->IsValid() == true) && Passed;
  }
  DeleteIfNotInList(List, Incarnations);
  List.DeleteAll();

  // The order of the hits does not matter: hits at 5, 0, 6, 1, and 20 cm give three events
  MRawEventIncarnations* Mixed = AddEvent(List, { 5.0, 0.0, 6.0, 1.0, 20.0 });
  Clusterize(Clusterizer, List);
  Passed = Evaluate("Analyze()", "cut off 2 cm, mixed order, two pairs and a single hit", "The list has three entries", List.Size(), 3U) && Passed;
  Passed = Evaluate("Analyze()", "cut off 2 cm, mixed order, two pairs and a single hit", "All five hits are in the list", GetNHits(List), 5) && Passed;
  Passed = EvaluateTrue("Analyze()", "cut off 2 cm, mixed order, two pairs and a single hit", "One event has the single hit", FindEvent(List, 1) != nullptr) && Passed;
  if (FindEvent(List, 1) != nullptr) {
    // The fifth hit has 140 keV
    Passed = EvaluateNear("GetEnergy()", "cut off 2 cm, mixed order, two pairs and a single hit", "The single hit is the one at 20 cm", FindEvent(List, 1)->GetEnergy(), 140.0, 1e-12) && Passed;
  }
  DeleteIfNotInList(List, Mixed);
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the distance cut off
bool UTEREventClusterizerDistance::TestCutOffLimits()
{
  bool Passed = true;

  // Two hits 2 cm apart: a distance equal to the cut off is still one event
  {
    MEREventClusterizerDistance Clusterizer;
    Clusterizer.SetDistanceCutOff(2.0);
    MRawEventIncarnationList List;
    MRawEventIncarnations* Incarnations = AddEvent(List, { 1.0, 3.0 });
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "cut off 2 cm, hits 2 cm apart", "The list has one entry", List.Size(), 1U) && Passed;
    DeleteIfNotInList(List, Incarnations);
    List.DeleteAll();
  }
  {
    MEREventClusterizerDistance Clusterizer;
    Clusterizer.SetDistanceCutOff(1.999);
    MRawEventIncarnationList List;
    MRawEventIncarnations* Incarnations = AddEvent(List, { 1.0, 3.0 });
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "cut off 1.999 cm, hits 2 cm apart", "The list has two entries", List.Size(), 2U) && Passed;
    DeleteIfNotInList(List, Incarnations);
    List.DeleteAll();
  }
  {
    MEREventClusterizerDistance Clusterizer;
    Clusterizer.SetDistanceCutOff(2.001);
    MRawEventIncarnationList List;
    MRawEventIncarnations* Incarnations = AddEvent(List, { 1.0, 3.0 });
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "cut off 2.001 cm, hits 2 cm apart", "The list has one entry", List.Size(), 1U) && Passed;
    DeleteIfNotInList(List, Incarnations);
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the merging of groups of hits
bool UTEREventClusterizerDistance::TestGroupMerging()
{
  bool Passed = true;

  // Hits at 0, 1, 4, 5.1, and 2.4 cm. The distances in rising order: 1.0 (0 - 1), 1.1 (4 - 5.1), 1.4 (1 - 2.4), 1.6 (2.4 - 4)
  // The first two make two groups, the third adds a hit to the first group, the fourth connects the two groups

  // A cut off of 1.5 cm: the groups are not connected
  {
    MEREventClusterizerDistance Clusterizer;
    Clusterizer.SetDistanceCutOff(1.5);
    MRawEventIncarnationList List;
    MRawEventIncarnations* Incarnations = AddEvent(List, { 0.0, 1.0, 4.0, 5.1, 2.4 });
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "cut off 1.5 cm, chain with a gap of 1.6 cm", "The list has two entries", List.Size(), 2U) && Passed;
    Passed = EvaluateTrue("Analyze()", "cut off 1.5 cm, chain with a gap of 1.6 cm", "One event has three hits", FindEvent(List, 3) != nullptr) && Passed;
    Passed = EvaluateTrue("Analyze()", "cut off 1.5 cm, chain with a gap of 1.6 cm", "One event has two hits", FindEvent(List, 2) != nullptr) && Passed;
    if (FindEvent(List, 3) != nullptr && FindEvent(List, 2) != nullptr) {
      // The hits at 0, 1, and 2.4 cm have 100 + 110 + 140 keV, the ones at 4 and 5.1 cm 120 + 130 keV
      Passed = EvaluateNear("GetEnergy()", "cut off 1.5 cm, chain with a gap of 1.6 cm", "The event with three hits has 350 keV", FindEvent(List, 3)->GetEnergy(), 350.0, 1e-12) && Passed;
      Passed = EvaluateNear("GetEnergy()", "cut off 1.5 cm, chain with a gap of 1.6 cm", "The event with two hits has 250 keV", FindEvent(List, 2)->GetEnergy(), 250.0, 1e-12) && Passed;
    }
    DeleteIfNotInList(List, Incarnations);
    List.DeleteAll();
  }

  // A cut off of 1.7 cm: the two groups become one event
  {
    MEREventClusterizerDistance Clusterizer;
    Clusterizer.SetDistanceCutOff(1.7);
    MRawEventIncarnationList List;
    MRawEventIncarnations* Incarnations = AddEvent(List, { 0.0, 1.0, 4.0, 5.1, 2.4 });
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "cut off 1.7 cm, chain with a gap of 1.6 cm", "The list has one entry", List.Size(), 1U) && Passed;
    Passed = EvaluateTrue("Analyze()", "cut off 1.7 cm, chain with a gap of 1.6 cm", "The event has all five hits", FindEvent(List, 5) != nullptr) && Passed;
    DeleteIfNotInList(List, Incarnations);
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test incarnations with several events
bool UTEREventClusterizerDistance::TestSeveralIncarnations()
{
  bool Passed = true;

  // Incarnations with two events cannot be clusterized: they are skipped
  MEREventClusterizerDistance Clusterizer;
  Clusterizer.SetDistanceCutOff(2.0);
  MRawEventIncarnationList List;
  MRawEventIncarnations* Incarnations = AddEvent(List, { 0.0, 10.0 });
  MRERawEvent* Second = new MRERawEvent();
  Second->AddRESE(CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D));
  Second->AddRESE(CreateHit(MVector(10.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D));
  Incarnations->AddRawEvent(Second);
  Passed = EvaluateTrue("Analyze()", "incarnations with two events", "The analysis returns true", Clusterize(Clusterizer, List)) && Passed;
  Passed = Evaluate("Analyze()", "incarnations with two events", "The list has one entry", List.Size(), 1U) && Passed;
  Passed = Evaluate("Analyze()", "incarnations with two events", "The incarnations still have two events", Incarnations->GetNRawEvents(), 2) && Passed;
  Passed = Evaluate("Analyze()", "incarnations with two events", "The events still have their four hits", GetNHits(List), 4) && Passed;
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a list with two events
bool UTEREventClusterizerDistance::TestTwoEvents()
{
  bool Passed = true;

  // Two entries, each with one event of two hits 10 cm apart: each event is split into two
  MEREventClusterizerDistance Clusterizer;
  Clusterizer.SetDistanceCutOff(2.0);
  MRawEventIncarnationList List;
  MRawEventIncarnations* First = AddEvent(List, { 0.0, 10.0 });
  MRawEventIncarnations* Second = AddEvent(List, { 20.0, 30.0 });
  MRERawEvent* SecondEvent = Second->GetRawEventAt(0);
  Clusterize(Clusterizer, List);
  // Defect REV-E-15: after the split of the first event the list is replaced by the new events, and the other entries of the list are lost
  Passed = ExpectedToFail(Evaluate("Analyze()", "cut off 2 cm, two events with two hits 10 cm apart", "The list has four entries", List.Size(), 4U)) && Passed;
  Passed = ExpectedToFail(Evaluate("Analyze()", "cut off 2 cm, two events with two hits 10 cm apart", "All four hits are in the list", GetNHits(List), 4)) && Passed;

  // The lost event has to be deleted here
  bool SecondEventInList = false;
  for (unsigned int i = 0; i < List.Size(); ++i) {
    for (int e = 0; e < List.Get(i)->GetNRawEvents(); ++e) {
      if (List.Get(i)->GetRawEventAt(e) == SecondEvent) SecondEventInList = true;
    }
  }
  if (SecondEventInList == false) {
    delete SecondEvent;
  }
  DeleteIfNotInList(List, First);
  DeleteIfNotInList(List, Second);
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an empty list
bool UTEREventClusterizerDistance::TestEmptyList()
{
  bool Passed = true;

  // Defect REV-E-11: the debug output at the start reads the first event of the list without a check, which throws an exception for an empty list
  Passed = ExpectedToCrash(EvaluateTrue("Analyze()", "empty list", "The analysis of an empty list succeeds", IsEmptyListAnalyzed())) && Passed;

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

  UTEREventClusterizerDistance Test;
  return Test.Run() == true ? 0 : 1;
}


// UTEREventClusterizerDistance.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
