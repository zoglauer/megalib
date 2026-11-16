/*
 * UTERDecay.cxx
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
#include <iomanip>
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MERDecay.h"
#include "UTERCSRShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the search for decay lines MERDecay
class UTERDecay : public UTERCSRBase
{
public:
  //! Default constructor
  UTERDecay() : UTERCSRBase("UTERDecay") {}
  //! Default destructor
  virtual ~UTERDecay() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Write the decay file of the tests - one line at 511 keV with a window of 3 sigma - and return its name
  MString WriteDecayFile();
  //! Return true if a new Compton event with hits of the energies (keV) is flagged as decay - the positions are in tracker layer 3, 1, 0 and in the calorimeter
  bool IsDecay(MERDecay& Decay, const vector<double>& Energies, int EventType = MRERawEvent::c_ComptonEvent);

  //! Test setting the parameters and the text of the options
  bool TestSetParametersAndToString();
  //! Test the search in single hits
  bool TestSingleHits();
  //! Test the search in sums of hits
  bool TestSums();
  //! Test events which are not searched
  bool TestOtherEvents();
  //! Test the summary of the search
  bool TestSummary();

  //! The test telescope
  MGeometryRevan m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERDecay::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The test telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  Passed = TestSetParametersAndToString() && Passed;
  if (m_Geometry.IsScanned() == true) {
    Passed = TestSingleHits() && Passed;
    Passed = TestSums() && Passed;
    Passed = TestOtherEvents() && Passed;
    Passed = TestSummary() && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the decay file of the tests - one line at 511 keV with a window of 3 sigma - and return its name
MString UTERDecay::WriteDecayFile()
{
  MString FileName = GetTemporaryFileName("Lines.decay");
  if (WriteTextFile(FileName, "# Decay lines of the test\nVersion 1\nType Decay\nDE 511 3\n") == false) {
    return "";
  }

  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a new Compton event with hits of the energies (keV) is flagged as decay - the positions are in tracker layer 3, 1, 0 and in the calorimeter
bool UTERDecay::IsDecay(MERDecay& Decay, const vector<double>& Energies, int EventType)
{
  vector<MVector> AllPositions = { MVector(0.0, 0.0, 3.0), MVector(2.0, 0.0, 1.0), MVector(-1.0, 2.0, 0.0), MVector(1.0, 1.0, -8.0) };
  vector<MVector> Positions(AllPositions.begin(), AllPositions.begin() + Energies.size());

  MRERawEvent* Event = CreateEvent(m_Geometry, Positions, Energies);
  if (Event == nullptr) {
    return false;
  }
  Event->SetEventType(EventType);

  MRawEventIncarnations List;
  List.SetInitialRawEvent(Event);
  DisableDefaultStreams();
  Decay.Analyze(&List);
  EnableDefaultStreams();
  bool Flag = Event->IsDecay();
  List.DeleteAll();

  return Flag;
}


////////////////////////////////////////////////////////////////////////////////


//! Test setting the parameters and the text of the options
bool UTERDecay::TestSetParametersAndToString()
{
  bool Passed = true;

  Passed = Evaluate("c_AllCombinations", "constant", "The value is 0", MERDecay::c_AllCombinations, 0) && Passed;
  Passed = Evaluate("c_AllButOneCombination", "constant", "The value is 1", MERDecay::c_AllButOneCombination, 1) && Passed;
  Passed = Evaluate("c_ComptonSequenceEnd", "constant", "The value is 2", MERDecay::c_ComptonSequenceEnd, 2) && Passed;

  MERDecay Decay;
  vector<double> NoEnergies;
  vector<double> NoErrors;

  // A missing file
  DisableDefaultStreams();
  bool Accepted = Decay.SetParameters(GetTemporaryFileName("Missing.decay"), NoEnergies, NoErrors);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "missing file", "The parameters are rejected", Accepted) && Passed;

  // A file without lines
  MString Empty = GetTemporaryFileName("Empty.decay");
  Passed = EvaluateTrue("WriteTextFile()", "file without lines", "The file can be written", WriteTextFile(Empty, "Version 1\nType Decay\n")) && Passed;
  DisableDefaultStreams();
  Accepted = Decay.SetParameters(Empty, NoEnergies, NoErrors);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "file without lines", "The parameters are rejected", Accepted) && Passed;

  // The file with one line
  MString FileName = WriteDecayFile();
  Passed = EvaluateFalse("WriteDecayFile()", "one line", "The file can be written", FileName.IsEmpty()) && Passed;
  Passed = EvaluateTrue("SetParameters()", "file with one line", "The parameters are accepted", Decay.SetParameters(FileName, NoEnergies, NoErrors)) && Passed;
  MString Text = Decay.ToString();
  Passed = EvaluateTrue("ToString()", "file with one line", "The title is listed", Text.Contains("# Decay detection options:\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "file with one line", "The line is listed with its window", Text.Contains("# Decay 511  3\n")) && Passed;

  // The file plus two further lines
  vector<double> Energies = { 1274.5, 2614.5 };
  vector<double> Errors = { 2.0, 1.5 };
  Passed = EvaluateTrue("SetParameters()", "file and two further lines", "The parameters are accepted", Decay.SetParameters(FileName, Energies, Errors)) && Passed;
  Text = Decay.ToString();
  Passed = EvaluateTrue("ToString()", "file and two further lines", "The line of the file is listed", Text.Contains("# Decay 511  3\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "file and two further lines", "The first further line is listed", Text.Contains("# Decay 1274.5  2\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "file and two further lines", "The second further line is listed", Text.Contains("# Decay 2614.5  1.5\n")) && Passed;

  // Setting the parameters again replaces the lines
  Passed = EvaluateTrue("SetParameters()", "file again", "The parameters are accepted", Decay.SetParameters(FileName, NoEnergies, NoErrors)) && Passed;
  Passed = EvaluateFalse("ToString()", "file again", "The further lines are gone", Decay.ToString().Contains("1274.5")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search in single hits
bool UTERDecay::TestSingleHits()
{
  bool Passed = true;

  MERDecay Decay;
  vector<double> NoEnergies;
  vector<double> NoErrors;
  Decay.SetParameters(WriteDecayFile(), NoEnergies, NoErrors);
  Decay.PreAnalysis();

  // The tracker has an energy resolution of 2 keV: the window of 3 sigma around 511 keV is 505 to 517 keV
  Passed = EvaluateTrue("Analyze()", "tracker hit with 511 keV", "The event is a decay", IsDecay(Decay, { 511.0, 100.0 })) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker hit with 516.9 keV", "The event is a decay", IsDecay(Decay, { 516.9, 100.0 })) && Passed;
  Passed = EvaluateFalse("Analyze()", "tracker hit with 517.1 keV", "The event is no decay", IsDecay(Decay, { 517.1, 100.0 })) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker hit with 505.1 keV", "The event is a decay", IsDecay(Decay, { 505.1, 100.0 })) && Passed;
  Passed = EvaluateFalse("Analyze()", "tracker hit with 504.9 keV", "The event is no decay", IsDecay(Decay, { 504.9, 100.0 })) && Passed;
  Passed = EvaluateTrue("Analyze()", "second tracker hit with 511 keV", "The event is a decay", IsDecay(Decay, { 100.0, 511.0 })) && Passed;

  // The calorimeter has an energy resolution of 10 keV: the window is 481 to 541 keV
  Passed = EvaluateTrue("Analyze()", "calorimeter hit with 540 keV", "The event is a decay", IsDecay(Decay, { 100.0, 100.0, 100.0, 540.0 })) && Passed;
  Passed = EvaluateFalse("Analyze()", "calorimeter hit with 542 keV", "The event is no decay", IsDecay(Decay, { 100.0, 100.0, 100.0, 542.0 })) && Passed;

  // A second line with a window of 2 sigma: 1274.5 keV +- 4 keV in the tracker
  vector<double> Energies = { 1274.5 };
  vector<double> Errors = { 2.0 };
  Decay.SetParameters(WriteDecayFile(), Energies, Errors);
  Passed = EvaluateTrue("Analyze()", "tracker hit with 1278.4 keV", "The event is a decay", IsDecay(Decay, { 1278.4, 100.0 })) && Passed;
  Passed = EvaluateFalse("Analyze()", "tracker hit with 1278.6 keV", "The event is no decay", IsDecay(Decay, { 1278.6, 100.0 })) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker hit with 511 keV, two lines", "The event is a decay", IsDecay(Decay, { 511.0, 100.0 })) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search in sums of hits
bool UTERDecay::TestSums()
{
  bool Passed = true;

  MERDecay Decay;
  vector<double> NoEnergies;
  vector<double> NoErrors;
  Decay.SetParameters(WriteDecayFile(), NoEnergies, NoErrors);
  Decay.PreAnalysis();

  // The sum of two tracker hits has a resolution of sqrt(2^2 + 2^2) keV = 2.83 keV: the window is 511 keV +- 8.49 keV
  Passed = EvaluateTrue("Analyze()", "200 + 311 keV of three hits", "The event is a decay", IsDecay(Decay, { 200.0, 311.0, 400.0 })) && Passed;
  Passed = EvaluateTrue("Analyze()", "200 + 319.4 keV of three hits", "The event is a decay", IsDecay(Decay, { 200.0, 319.4, 400.0 })) && Passed;
  Passed = EvaluateFalse("Analyze()", "200 + 319.6 keV of three hits", "The event is no decay", IsDecay(Decay, { 200.0, 319.6, 400.0 })) && Passed;
  // The first and the last hit, and the last two hits
  Passed = EvaluateTrue("Analyze()", "first and third of three hits", "The event is a decay", IsDecay(Decay, { 200.0, 400.0, 311.0 })) && Passed;
  Passed = EvaluateTrue("Analyze()", "second and third of three hits", "The event is a decay", IsDecay(Decay, { 400.0, 200.0, 311.0 })) && Passed;
  // Three of four hits
  Passed = EvaluateTrue("Analyze()", "first, second, and fourth of four hits", "The event is a decay", IsDecay(Decay, { 100.0, 111.0, 700.0, 300.0 })) && Passed;
  Passed = EvaluateTrue("Analyze()", "second, third, and fourth of four hits", "The event is a decay", IsDecay(Decay, { 700.0, 100.0, 111.0, 300.0 })) && Passed;

  // The sum of all hits is the photon itself and no decay inside the event
  Passed = EvaluateFalse("Analyze()", "200 + 311 keV of two hits", "The event is no decay", IsDecay(Decay, { 200.0, 311.0 })) && Passed;
  Passed = EvaluateFalse("Analyze()", "100 + 200 + 211 keV of three hits", "The event is no decay", IsDecay(Decay, { 100.0, 200.0, 211.0 })) && Passed;
  Passed = EvaluateFalse("Analyze()", "100 + 100 + 100 + 211 keV of four hits", "The event is no decay", IsDecay(Decay, { 100.0, 100.0, 100.0, 211.0 })) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events which are not searched
bool UTERDecay::TestOtherEvents()
{
  bool Passed = true;

  MERDecay Decay;
  vector<double> NoEnergies;
  vector<double> NoErrors;
  Decay.SetParameters(WriteDecayFile(), NoEnergies, NoErrors);
  Decay.PreAnalysis();

  // Only Compton events are searched
  Passed = EvaluateFalse("Analyze()", "event of unknown type with 511 keV", "The event is no decay", IsDecay(Decay, { 511.0, 100.0 }, MRERawEvent::c_UnknownEvent)) && Passed;
  Passed = EvaluateFalse("Analyze()", "photo event with 511 keV", "The event is no decay", IsDecay(Decay, { 511.0, 100.0 }, MRERawEvent::c_PhotoEvent)) && Passed;
  Passed = EvaluateFalse("Analyze()", "pair event with 511 keV", "The event is no decay", IsDecay(Decay, { 511.0, 100.0 }, MRERawEvent::c_PairEvent)) && Passed;

  // An empty list
  MRawEventIncarnations List;
  DisableDefaultStreams();
  bool Result = Decay.Analyze(&List);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Analyze()", "empty list", "The analysis returns true", Result) && Passed;

  // Several incarnations: each one is searched
  MRERawEvent* First = CreateEvent(m_Geometry, { MVector(0.0, 0.0, 3.0), MVector(1.0, 1.0, -8.0) }, { 300.0, 100.0 });
  MRERawEvent* Second = CreateEvent(m_Geometry, { MVector(0.0, 0.0, 3.0), MVector(1.0, 1.0, -8.0) }, { 511.0, 100.0 });
  First->SetEventType(MRERawEvent::c_ComptonEvent);
  Second->SetEventType(MRERawEvent::c_ComptonEvent);
  List.SetInitialRawEvent(First);
  List.AddRawEvent(Second);
  BeginCaptureOutput();
  Result = Decay.Analyze(&List);
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("Analyze()", "two incarnations", "The analysis returns true", Result) && Passed;
  Passed = EvaluateFalse("Analyze()", "two incarnations", "The incarnation without the line is no decay", First->IsDecay()) && Passed;
  Passed = EvaluateTrue("Analyze()", "two incarnations", "The incarnation with the line is a decay", Second->IsDecay()) && Passed;
  // Defect REV-G-11: Each call, i.e. each event, writes "Starting decay search..." to the standard output
  Passed = ExpectedToFail(EvaluateTrue("Analyze()", "two incarnations", "Nothing is written for each event", Output.IsEmpty())) && Passed;
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the summary of the search
bool UTERDecay::TestSummary()
{
  bool Passed = true;

  MERDecay Decay;
  vector<double> Energies = { 1274.5 };
  vector<double> Errors = { 2.0 };
  Decay.SetParameters(WriteDecayFile(), Energies, Errors);

  // No decays
  Passed = EvaluateTrue("PreAnalysis()", "two lines", "The preparation returns true", Decay.PreAnalysis()) && Passed;
  BeginCaptureOutput();
  bool Result = Decay.PostAnalysis();
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("PostAnalysis()", "no decays", "The summary returns true", Result) && Passed;
  Passed = EvaluateTrue("PostAnalysis()", "no decays", "The summary has a title", Output.Contains("Decay analysis: ")) && Passed;
  Passed = EvaluateTrue("PostAnalysis()", "no decays", "The summary tells that no decays were found", Output.Contains("No decays found!")) && Passed;

  // Three decays of the first line, one of the second
  IsDecay(Decay, { 511.0, 100.0 });
  IsDecay(Decay, { 100.0, 511.0 });
  IsDecay(Decay, { 300.0, 211.0, 100.0 });
  IsDecay(Decay, { 1274.5, 100.0 });
  IsDecay(Decay, { 700.0, 100.0 });
  BeginCaptureOutput();
  Decay.PostAnalysis();
  Output = EndCaptureOutput();
  mout<<setprecision(6);
  Passed = EvaluateTrue("PostAnalysis()", "three and one decays", "The first line is listed with 3 decays, i.e. 75%", Output.Contains("E:    511 keV:      3 (75%)")) && Passed;
  // Defect REV-G-12: The precision of 4 digits of the percentage stays set, so the energy of all later lines is rounded to 4 digits (1274 keV)
  Passed = ExpectedToFail(EvaluateTrue("PostAnalysis()", "three and one decays", "The second line is listed with 1 decay, i.e. 25%", Output.Contains("E: 1274.5 keV:      1 (25%)"))) && Passed;
  Passed = EvaluateFalse("PostAnalysis()", "three and one decays", "The summary does not tell that no decays were found", Output.Contains("No decays found!")) && Passed;

  // The summary must not change the format of the stream for the output which follows
  BeginCaptureOutput();
  Decay.PostAnalysis();
  mout<<1234.5678<<endl;
  Output = EndCaptureOutput();
  mout<<setprecision(6);
  // Defect REV-G-12: The summary leaves the precision of the standard output stream at 4 digits
  Passed = ExpectedToFail(EvaluateTrue("PostAnalysis()", "number written after the summary", "A number is written with the default precision of 6 digits", Output.Contains("1234.57"))) && Passed;

  // A new analysis starts the counting again
  Decay.PreAnalysis();
  BeginCaptureOutput();
  Decay.PostAnalysis();
  Output = EndCaptureOutput();
  Passed = EvaluateTrue("PreAnalysis()", "after decays", "The counters are reset", Output.Contains("No decays found!")) && Passed;

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

  UTERDecay Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERDecay.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
