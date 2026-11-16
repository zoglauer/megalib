/*
 * UTEREventClusterizerTMVA.cxx
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
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MERCSRTMVAMethods.h"
#include "MEREventClusterizerTMVA.h"
#include "MRawEventIncarnationList.h"
#include "MRawEventIncarnations.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the event clusterizer with TMVA MEREventClusterizerTMVA of revan
//! Attention: there are no trained TMVA files, so the evaluation of the methods and the split of the events are not covered
class UTEREventClusterizerTMVA : public UTRevanBase
{
public:
  //! Default constructor
  UTEREventClusterizerTMVA() : UTRevanBase("UTEREventClusterizerTMVA") {}
  //! Default destructor
  virtual ~UTEREventClusterizerTMVA() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the options text
  bool TestToString();
  //! Test steering files which are rejected
  bool TestSteeringFileRejected();
  //! Test steering files with missing entries
  bool TestSteeringFileIncomplete();
  //! Test the analysis before the set up
  bool TestAnalyzeWithoutSetUp();
  //! Test the rejection of events outside the trained range
  bool TestEventLimits();
  //! Test an empty list
  bool TestEmptyList();

  //! Write the steering file with the given content and try to set it, return the result
  bool SetSteeringFile(MEREventClusterizerTMVA& Clusterizer, const MString& Content, const MString& UsedMethods);
  //! Add new incarnations with one event with the given number of hits and total energy in keV to the list, return the event
  MRERawEvent* AddEvent(MRawEventIncarnationList& List, unsigned int NHits, double Energy);
  //! Analyze the list and hide the output
  bool Clusterize(MEREventClusterizerTMVA& Clusterizer, MRawEventIncarnationList& List);
  //! Return true if a steering file without the maximum number of hits is rejected
  bool IsSteeringFileWithoutMaximumHitsRejected();
  //! Return true if an event with two hits is analyzed before the set up
  bool IsAnalyzedWithoutSetUp();
  //! Return true if an event with two hits is analyzed after a set up without methods
  bool IsAnalyzedWithoutMethods();
  //! Return true if the analysis of an empty list returns true
  bool IsEmptyListAnalyzed();

  //! The output to cout of the last set up
  MString m_Output;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTEREventClusterizerTMVA::Run()
{
  bool Passed = true;

  Passed = TestToString() && Passed;
  Passed = TestSteeringFileRejected() && Passed;
  Passed = TestSteeringFileIncomplete() && Passed;
  Passed = TestAnalyzeWithoutSetUp() && Passed;
  Passed = TestEventLimits() && Passed;
  Passed = TestEmptyList() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the steering file with the given content and try to set it, return the result
bool UTEREventClusterizerTMVA::SetSteeringFile(MEREventClusterizerTMVA& Clusterizer, const MString& Content, const MString& UsedMethods)
{
  const MString FileName = GetTemporaryFileName("Steering.eventclusterizer.tmva");
  if (WriteTextFile(FileName, Content) == false) {
    return false;
  }
  MERCSRTMVAMethods Methods;
  Methods.ResetUsedMethods();
  if (UsedMethods != "") {
    Methods.SetUsedMethods(UsedMethods);
  }

  DisableDefaultStreams();
  BeginCaptureOutput();
  bool IsSet = Clusterizer.SetTMVAFileNameAndMethod(FileName, Methods);
  m_Output = EndCaptureOutput();
  EnableDefaultStreams();

  return IsSet;
}


////////////////////////////////////////////////////////////////////////////////


//! Add new incarnations with one event with the given number of hits and total energy in keV to the list, return the event
MRERawEvent* UTEREventClusterizerTMVA::AddEvent(MRawEventIncarnationList& List, unsigned int NHits, double Energy)
{
  MRERawEvent* Event = new MRERawEvent();
  Event->SetEventID(42);
  for (unsigned int h = 0; h < NHits; ++h) {
    Event->AddRESE(CreateHit(MVector(1.0*h, 0.0, 0.0), Energy/NHits, MDDetector::c_Strip2D));
  }
  MRawEventIncarnations* Incarnations = new MRawEventIncarnations();
  Incarnations->AddRawEvent(Event);
  List.Add(Incarnations);

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze the list and hide the output
bool UTEREventClusterizerTMVA::Clusterize(MEREventClusterizerTMVA& Clusterizer, MRawEventIncarnationList& List)
{
  DisableDefaultStreams();
  bool Analyzed = Clusterizer.Analyze(&List);
  EnableDefaultStreams();

  return Analyzed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a steering file without the maximum number of hits is rejected
bool UTEREventClusterizerTMVA::IsSteeringFileWithoutMaximumHitsRejected()
{
  MEREventClusterizerTMVA Clusterizer;
  if (SetSteeringFile(Clusterizer, "MG 2\nEB 0 1000\nDN Test\n", "") == true) return false;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if an event with two hits is analyzed before the set up
bool UTEREventClusterizerTMVA::IsAnalyzedWithoutSetUp()
{
  MEREventClusterizerTMVA Clusterizer;
  MRawEventIncarnationList List;
  AddEvent(List, 2, 200.0);
  bool Analyzed = Clusterize(Clusterizer, List);
  List.DeleteAll();
  return Analyzed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if an event with two hits is analyzed after a set up without methods
bool UTEREventClusterizerTMVA::IsAnalyzedWithoutMethods()
{
  MEREventClusterizerTMVA Clusterizer;
  SetSteeringFile(Clusterizer, "MH 3\nMG 2\nEB 0 1000\nDN Test\n", "");
  MRawEventIncarnationList List;
  AddEvent(List, 2, 200.0);
  bool Analyzed = Clusterize(Clusterizer, List);
  List.DeleteAll();
  return Analyzed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the analysis of an empty list returns true
bool UTEREventClusterizerTMVA::IsEmptyListAnalyzed()
{
  MEREventClusterizerTMVA Clusterizer;
  MRawEventIncarnationList List;
  return Clusterize(Clusterizer, List);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the options text
bool UTEREventClusterizerTMVA::TestToString()
{
  bool Passed = true;

  MEREventClusterizerTMVA Clusterizer;
  MString Text = Clusterizer.ToString();
  Passed = EvaluateTrue("ToString()", "default", "The text starts with the title", Text.BeginsWith("# Event clustering options:")) && Passed;
  Passed = EvaluateTrue("ToString()", "default", "The text has an empty file name", Text.Contains("# TMVA file:    \n")) && Passed;
  Passed = Evaluate("ToString()", "core only", "The core text is the same", Clusterizer.ToString(true), Text) && Passed;

  // The file name and the methods are stored also if the set up fails
  MERCSRTMVAMethods Methods;
  Methods.ResetUsedMethods();
  Methods.AddUsedMethod(MERCSRTMVAMethod::c_BDTD);
  const MString FileName = GetTemporaryFileName("Missing.eventclusterizer.tmva");
  DisableDefaultStreams();
  bool IsSet = Clusterizer.SetTMVAFileNameAndMethod(FileName, Methods);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetTMVAFileNameAndMethod()", "missing steering file", "A missing steering file is rejected", IsSet) && Passed;
  Text = Clusterizer.ToString();
  Passed = EvaluateTrue("ToString()", "after a missing steering file", "The text has the file name", Text.Contains(MString("# TMVA file:    ") + FileName + "\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "after a missing steering file", "The text has the method", Text.Contains(MString("# TMVA methods: ") + Methods.GetUsedMethodsString() + "\n")) && Passed;

  // The analysis of incarnations is the one of the base class, which the derived class hides
  MRawEventIncarnations Incarnations;
  MEREventClusterizer* Base = &Clusterizer;
  Passed = EvaluateFalse("Analyze()", "incarnations", "The analysis of incarnations is not available", Base->Analyze(&Incarnations)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test steering files which are rejected
bool UTEREventClusterizerTMVA::TestSteeringFileRejected()
{
  bool Passed = true;

  // MH and MG need exactly one value
  {
    MEREventClusterizerTMVA Clusterizer;
    Passed = EvaluateFalse("SetTMVAFileNameAndMethod()", "MH with two values", "The steering file is rejected", SetSteeringFile(Clusterizer, "MH 3 4\nMG 2\nEB 0 1000\nDN Test\n", "")) && Passed;
  }
  {
    MEREventClusterizerTMVA Clusterizer;
    Passed = EvaluateFalse("SetTMVAFileNameAndMethod()", "MG with two values", "The steering file is rejected", SetSteeringFile(Clusterizer, "MH 3\nMG 2 3\nEB 0 1000\nDN Test\n", "")) && Passed;
  }

  // At least two energy bin edges are needed
  {
    MEREventClusterizerTMVA Clusterizer;
    Passed = EvaluateFalse("SetTMVAFileNameAndMethod()", "no energy bin edges", "The steering file is rejected", SetSteeringFile(Clusterizer, "MH 3\nMG 2\nDN Test\n", "")) && Passed;
  }
  {
    MEREventClusterizerTMVA Clusterizer;
    Passed = EvaluateFalse("SetTMVAFileNameAndMethod()", "one energy bin edge", "The steering file is rejected", SetSteeringFile(Clusterizer, "MH 3\nMG 2\nEB 1000\nDN Test\n", "")) && Passed;
  }
  {
    MEREventClusterizerTMVA Clusterizer;
    Passed = EvaluateFalse("SetTMVAFileNameAndMethod()", "empty steering file", "The steering file is rejected", SetSteeringFile(Clusterizer, "", "")) && Passed;
  }

  // A complete steering file, but the weights file of the used method does not exist
  {
    MEREventClusterizerTMVA Clusterizer;
    Passed = EvaluateFalse("SetTMVAFileNameAndMethod()", "complete steering file without weights files", "The steering file is rejected", SetSteeringFile(Clusterizer, "MH 3\nMG 2\nEB 0 1000\nDN Test\nTA BDTD\n", "BDTD")) && Passed;
    // Defect REV-E-16: the set up prints the methods and the names of the weights files with cout
    Passed = ExpectedToFail(Evaluate("SetTMVAFileNameAndMethod()", "complete steering file without weights files", "Nothing is printed", m_Output, MString(""))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test steering files with missing entries
bool UTEREventClusterizerTMVA::TestSteeringFileIncomplete()
{
  bool Passed = true;

  // Defect REV-E-17: the maximum numbers of hits and groups are not initialized, and a steering file without them is used with their random values
  Passed = ExpectedToCrash(EvaluateTrue("SetTMVAFileNameAndMethod()", "steering file without MH", "The steering file is rejected", IsSteeringFileWithoutMaximumHitsRejected())) && Passed;

  // Defect REV-E-18: a set up without a used method is accepted, and the analysis then reads the name of the first method of an empty list
  MEREventClusterizerTMVA Clusterizer;
  Passed = ExpectedToFail(EvaluateFalse("SetTMVAFileNameAndMethod()", "complete steering file, no used method", "A set up without a method is rejected", SetSteeringFile(Clusterizer, "MH 3\nMG 2\nEB 0 1000\nDN Test\n", ""))) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("Analyze()", "event with two hits after a set up without methods", "The analysis does not crash", IsAnalyzedWithoutMethods())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the analysis before the set up
bool UTEREventClusterizerTMVA::TestAnalyzeWithoutSetUp()
{
  bool Passed = true;

  // Events with no or one hit are never clusterized
  MEREventClusterizerTMVA Clusterizer;
  MRawEventIncarnationList List;
  MRERawEvent* Empty = AddEvent(List, 0, 0.0);
  MRERawEvent* Single = AddEvent(List, 1, 200.0);
  Passed = EvaluateTrue("Analyze()", "no set up, events with no and one hit", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
  Passed = Evaluate("Analyze()", "no set up, events with no and one hit", "The list has two entries", List.Size(), 2U) && Passed;
  Passed = Evaluate("Analyze()", "no set up, event without hits", "The event is not rejected", Empty->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  Passed = Evaluate("Analyze()", "no set up, event with one hit", "The event is not rejected", Single->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  Passed = Evaluate("Analyze()", "no set up, event with one hit", "The event keeps its hit", Single->GetNRESEs(), 1) && Passed;
  List.DeleteAll();

  // Incarnations with two events are skipped
  MRawEventIncarnations* Double = new MRawEventIncarnations();
  Double->AddRawEvent(new MRERawEvent());
  Double->AddRawEvent(new MRERawEvent());
  List.Add(Double);
  Passed = EvaluateTrue("Analyze()", "no set up, incarnations with two events", "The analysis returns true", Clusterize(Clusterizer, List)) && Passed;
  Passed = Evaluate("Analyze()", "no set up, incarnations with two events", "The incarnations still have two events", Double->GetNRawEvents(), 2) && Passed;
  List.DeleteAll();

  // Defect REV-E-17: see above - the analysis compares with the not initialized maximum number of hits and reads the first of no energy bin edges
  Passed = ExpectedToCrash(EvaluateTrue("Analyze()", "no set up, event with two hits", "The analysis does not crash", IsAnalyzedWithoutSetUp())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rejection of events outside the trained range
bool UTEREventClusterizerTMVA::TestEventLimits()
{
  bool Passed = true;

  // At most three hits, energies from 100 to 1000 keV. No method is used, so no weights files are needed
  MEREventClusterizerTMVA Clusterizer;
  SetSteeringFile(Clusterizer, "MH 3\nMG 2\nEB 100 500 1000\nDN Test\n", "");

  MRawEventIncarnationList List;
  MRERawEvent* TooManyHits = AddEvent(List, 4, 400.0);
  MRERawEvent* TooLow = AddEvent(List, 2, 99.0);
  MRERawEvent* TooHigh = AddEvent(List, 3, 1001.0);
  MRERawEvent* Single = AddEvent(List, 1, 5000.0);
  Passed = EvaluateTrue("Analyze()", "events outside the trained range", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
  Passed = Evaluate("Analyze()", "events outside the trained range", "The list has four entries", List.Size(), 4U) && Passed;
  Passed = Evaluate("Analyze()", "four hits, at most three trained", "The event is rejected for too many hits", TooManyHits->GetRejectionReason(), MRERawEvent::c_RejectionEventClusteringTooManyHits) && Passed;
  Passed = EvaluateFalse("Analyze()", "four hits, at most three trained", "The event is not valid", TooManyHits->IsValid()) && Passed;
  Passed = Evaluate("Analyze()", "four hits, at most three trained", "The event keeps its hits", TooManyHits->GetNRESEs(), 4) && Passed;
  Passed = Evaluate("Analyze()", "99 keV, trained from 100 keV", "The event is rejected for its energy", TooLow->GetRejectionReason(), MRERawEvent::c_RejectionEventClusteringEnergyOutOfBounds) && Passed;
  Passed = EvaluateFalse("Analyze()", "99 keV, trained from 100 keV", "The event is not valid", TooLow->IsValid()) && Passed;
  Passed = Evaluate("Analyze()", "1001 keV, trained up to 1000 keV", "The event is rejected for its energy", TooHigh->GetRejectionReason(), MRERawEvent::c_RejectionEventClusteringEnergyOutOfBounds) && Passed;
  Passed = EvaluateFalse("Analyze()", "1001 keV, trained up to 1000 keV", "The event is not valid", TooHigh->IsValid()) && Passed;
  // An event with one hit is not checked
  Passed = Evaluate("Analyze()", "one hit of 5000 keV", "The event is not rejected", Single->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an empty list
bool UTEREventClusterizerTMVA::TestEmptyList()
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

  UTEREventClusterizerTMVA Test;
  return Test.Run() == true ? 0 : 1;
}


// UTEREventClusterizerTMVA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
