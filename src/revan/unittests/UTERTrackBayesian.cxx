/*
 * UTERTrackBayesian.cxx
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
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MERTrackBayesian.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO5.h"
#include "UTERTrackShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the electron tracker MERTrackBayesian: the directions of the tracks are rated with
//! response matrices of good and bad track elements (start, central, stop, and tracks of two hits)
class UTERTrackBayesian : public UTERTrackBase
{
public:
  //! Default constructor
  UTERTrackBayesian() : UTERTrackBase("UTERTrackBayesian") {}
  //! Default destructor
  virtual ~UTERTrackBayesian() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the response file name: missing files and wrong types
  bool TestSetSpecialParameters();
  //! Test the option text
  bool TestToString();
  //! Test the tracking without response files
  bool TestWithoutResponse();
  //! Test the tracking with a small response
  bool TestWithResponse();
  //! Test a pair event
  bool TestPair();

  //! Write the nine response files with the given prefix into the temporary directory. Return the name of the ".t.goodbad.rsp" file, or an empty string on failure.
  //! Each matrix has one bin on each axis, except the deposited energy with the two bins below and above 100 keV:
  //! * start (total energy, incidence angle, deposited energy): good 24 below and 12 above 100 keV, bad 12 and 24
  //! * central (total energy, incidence angle, deposited energy, azimuthal and polar scatter angle): good 30 and 10, bad 10 and 30
  //! * stop (deposited energy, incidence angle): good 10 and 30, bad 30 and 10
  //! * dual (total energy, incidence angle, deposited energy): as start
  //! * goodbad: 20 good and 10 bad
  MString WriteResponse(const MString& Prefix);
  //! Return a new event with a bent track: (0, 0, 3) cm 50 keV, (1, 0, 2) cm 60 keV, (2, 1, 1) cm 80 keV, (3, 3, 0) cm 200 keV. The caller owns it, until it is handed to an incarnation list
  MRERawEvent* CreateBentTrackEvent(MGeometryRevan& Geometry);
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERTrackBayesian::Run()
{
  bool Passed = true;

  Passed = TestSetSpecialParameters() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestWithoutResponse() && Passed;
  Passed = TestWithResponse() && Passed;
  Passed = TestPair() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the response file name: missing files and wrong types
bool UTERTrackBayesian::TestSetSpecialParameters()
{
  bool Passed = true;

  // A file which does not exist:
  {
    MERTrackBayesian Tracker;
    DisableDefaultStreams();
    bool Result = Tracker.SetSpecialParameters(GetTemporaryFileName("Missing.t.goodbad.rsp"));
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetSpecialParameters()", "missing file", "A file which does not exist is rejected", Result) && Passed;
  }

  // An empty file name:
  {
    MERTrackBayesian Tracker;
    DisableDefaultStreams();
    bool Result = Tracker.SetSpecialParameters("");
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetSpecialParameters()", "empty file name", "An empty file name is rejected", Result) && Passed;
  }

  // A file of the wrong type:
  {
    MString FileName = GetTemporaryFileName("Wrong.t.start.good.rsp");
    Passed = EvaluateTrue("WriteTextFile()", "wrong type", "The file can be written", WriteTextFile(FileName, "# No response\n")) && Passed;
    MERTrackBayesian Tracker;
    DisableDefaultStreams();
    bool Result = Tracker.SetSpecialParameters(FileName);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetSpecialParameters()", "wrong type", "A file which does not end with .t.goodbad.rsp is rejected", Result) && Passed;
  }

  // All nine files:
  {
    MString FileName = WriteResponse("Complete");
    Passed = EvaluateTrue("WriteResponse()", "complete response", "The response files can be written", FileName != "") && Passed;
    MERTrackBayesian Tracker;
    DisableDefaultStreams();
    bool Result = Tracker.SetSpecialParameters(FileName);
    EnableDefaultStreams();
    Passed = EvaluateTrue("SetSpecialParameters()", "complete response", "A complete response is accepted", Result) && Passed;

    // The same object can read it again:
    DisableDefaultStreams();
    Result = Tracker.SetSpecialParameters(FileName);
    EnableDefaultStreams();
    Passed = EvaluateTrue("SetSpecialParameters()", "complete response, again", "A complete response is accepted again", Result) && Passed;
  }

  // Only the .t.goodbad.rsp file of the nine files exists:
  {
    MString FileName = WriteResponse("Partial");
    Passed = EvaluateTrue("WriteResponse()", "only the goodbad file", "The response files can be written", FileName != "") && Passed;
    MString Prefix = GetTemporaryFileName("Partial");
    MString Suffixes[8] = { ".t.start.good.rsp", ".t.start.bad.rsp", ".t.central.good.rsp", ".t.central.bad.rsp", ".t.stop.good.rsp", ".t.stop.bad.rsp", ".t.dual.good.rsp", ".t.dual.bad.rsp" };
    bool Removed = true;
    for (unsigned int s = 0; s < 8; ++s) {
      if (RemoveTemporaryFile(Prefix + Suffixes[s]) == false) {
        Removed = false;
      }
    }
    Passed = EvaluateTrue("RemoveTemporaryFile()", "only the goodbad file", "The eight other files can be removed", Removed) && Passed;

    MERTrackBayesian Tracker;
    DisableDefaultStreams();
    bool Result = Tracker.SetSpecialParameters(FileName);
    EnableDefaultStreams();
    // Defect REV-F-7: The check for the other response files tests the .t.goodbad.rsp file a second time, and the results of the Read() calls are ignored, so missing files are accepted
    Passed = ExpectedToFail(EvaluateFalse("SetSpecialParameters()", "only the goodbad file", "A response with missing files is rejected", Result)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the option text
bool UTERTrackBayesian::TestToString()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MString Core;
  Core += "# Search MIPs:                  0\n";
  Core += "# Search Pairs:                 1\n";
  Core += "# Search Comptons:              1\n";
  Core += "# Max. number of ambiguities:   5\n";
  Core += "# Max. layer jump Compton:      3\n";
  Core += "# Number of sequences to keep:  7\n";
  Core += "# Reject pure ambiguities:      1\n";

  // Without a response file:
  {
    MERTrackBayesian Tracker;
    SetupTracker(Tracker, Geometry, false, true, true, 3, 7, true);
    Passed = Evaluate("ToString()", "no response, core only", "The text lists the empty file name and the options", Tracker.ToString(true), MString("# File name:                  \n") + Core) && Passed;
    Passed = Evaluate("ToString()", "no response, full", "The text has a title and separators around the options", Tracker.ToString(false), MString("# Tracking - Bayesian options:\n# \n# File name:                  \n") + Core + "# \n") && Passed;
    Passed = Evaluate("ToString()", "no response, default argument", "The default is the full text", Tracker.ToString(), Tracker.ToString(false)) && Passed;
  }

  // With a response file:
  {
    MString FileName = WriteResponse("Options");
    Passed = EvaluateTrue("WriteResponse()", "response", "The response files can be written", FileName != "") && Passed;
    MERTrackBayesian Tracker;
    SetupTracker(Tracker, Geometry, false, true, true, 3, 7, true);
    DisableDefaultStreams();
    bool Result = Tracker.SetSpecialParameters(FileName);
    EnableDefaultStreams();
    Passed = EvaluateTrue("SetSpecialParameters()", "response", "The response is accepted", Result) && Passed;
    // Defect REV-F-8: SetSpecialParameters() cuts the suffix off its file name argument with MString::Remove(), which changes the string itself, and stores the result: the options show the name without .t.goodbad.rsp
    Passed = ExpectedToFail(Evaluate("ToString()", "response, core only", "The text lists the name of the response file", Tracker.ToString(true), MString("# File name:                  ") + FileName + "\n" + Core)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the tracking without response files
bool UTERTrackBayesian::TestWithoutResponse()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackBayesian Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 10);

  // Without statistics in the response a track has the worst rating, 1, in both directions
  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "bent track, no response", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateBentTrackEvent(Geometry))) && Passed;
  Passed = Evaluate("Analyze()", "bent track, no response", "Both directions are kept", List.GetNRawEvents(), 2) && Passed;
  for (int e = 0; e < List.GetNRawEvents(); ++e) {
    MRERawEvent* Event = List.GetRawEventAt(e);
    MString Input = MString("bent track, no response, sequence ") + e;
    Passed = Evaluate("Analyze()", Input, "The event has one track", GetNTracks(Event), 1U) && Passed;
    MRETrack* Track = GetTrack(Event, 0);
    if (Track != nullptr) {
      Passed = Evaluate("Analyze()", Input, "The track has the four hits", Track->GetNRESEs(), 4) && Passed;
      Passed = EvaluateNear("Analyze()", Input, "The quality factor of the track is 1", Track->GetQualityFactor(), 1.0, 0.0) && Passed;
    }
    Passed = EvaluateNear("Analyze()", Input, "The track quality factor of the event is 1", Event->GetTrackQualityFactor(), 1.0, 0.0) && Passed;
    Passed = EvaluateNear("Analyze()", Input, "The pair quality factor is not set", Event->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
  }
  List.DeleteAll();

  // Two hits in the same layer: no track
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddTrackerHit(Event, Geometry, 2.0, 0.0, 3, 60.0);
    Passed = EvaluateTrue("Analyze()", "two hits in the same layer", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "two hits in the same layer", "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      Passed = Evaluate("Analyze()", "two hits in the same layer", "The event has no track", GetNTracks(List.GetRawEventAt(0)), 0U) && Passed;
      // The product over no tracks is 1, and the track quality factor is 1 - product
      Passed = EvaluateNear("Analyze()", "two hits in the same layer", "The track quality factor is 0", List.GetRawEventAt(0)->GetTrackQualityFactor(), 0.0, 0.0) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the tracking with a small response
bool UTERTrackBayesian::TestWithResponse()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MString FileName = WriteResponse("Tracking");
  Passed = EvaluateTrue("WriteResponse()", "response", "The response files can be written", FileName != "") && Passed;

  MERTrackBayesian Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 10);
  DisableDefaultStreams();
  bool Result = Tracker.SetSpecialParameters(FileName);
  EnableDefaultStreams();
  Passed = EvaluateTrue("SetSpecialParameters()", "response", "The response is accepted", Result) && Passed;

  // Each element of a track gives the ratio (good entries / all good entries) / (bad entries / all bad entries) of its bin; the sums are equal in each pair of matrices.
  // From layer 3 (50 keV) to layer 0 (200 keV): start with 50 keV: 24/12 = 2, central with 60 and 80 keV: 30/10 = 3 each, stop with 200 keV: 30/10 = 3 -> 54
  // From layer 0 to layer 3: start with 200 keV: 12/24 = 0.5, central: 3 each, stop with 50 keV: 10/30 -> 1.5
  // With the ratio of all good to all bad tracks, 20/10 = 2, the odds are R = 108 and R = 3, the probability of a good track is R/(1 + R),
  // and the quality factors are 1 - R/(1 + R) = 1/109 and 1/4
  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "bent track", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateBentTrackEvent(Geometry))) && Passed;
  Passed = Evaluate("Analyze()", "bent track", "Both directions are kept", List.GetNRawEvents(), 2) && Passed;
  if (List.GetNRawEvents() == 2) {
    MRETrack* Best = GetTrack(List.GetRawEventAt(0), 0);
    MRETrack* Second = GetTrack(List.GetRawEventAt(1), 0);
    Passed = EvaluateTrue("Analyze()", "bent track", "Both events have a track", Best != nullptr && Second != nullptr) && Passed;
    if (Best != nullptr && Second != nullptr) {
      // The response matrices hold floats: the tolerance is the precision of a float
      Passed = Evaluate("Analyze()", "bent track", "The best track starts at the small energy deposit in layer 3", GetLayerSequence(Best), MString("3 2 1 0")) && Passed;
      Passed = EvaluateNear("Analyze()", "bent track", "The quality factor of the best track is 1/109", Best->GetQualityFactor(), 1.0/109.0, 1e-6) && Passed;
      Passed = Evaluate("Analyze()", "bent track", "The second track starts in layer 0", GetLayerSequence(Second), MString("0 1 2 3")) && Passed;
      Passed = EvaluateNear("Analyze()", "bent track", "The quality factor of the second track is 1/4", Second->GetQualityFactor(), 0.25, 1e-6) && Passed;
    }
    Passed = EvaluateNear("Analyze()", "bent track", "The track quality factor of the best event is 1/109", List.GetRawEventAt(0)->GetTrackQualityFactor(), 1.0/109.0, 1e-6) && Passed;
    Passed = EvaluateNear("Analyze()", "bent track", "The track quality factor of the second event is 1/4", List.GetRawEventAt(1)->GetTrackQualityFactor(), 0.25, 1e-6) && Passed;
    Passed = EvaluateTrue("Analyze()", "bent track", "The best event is the best try", List.GetBestTryEvent() == List.GetRawEventAt(0)) && Passed;
    Passed = EvaluateFalse("Analyze()", "bent track", "There is no optimum event", List.HasOptimumEvent()) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a pair event
bool UTERTrackBayesian::TestPair()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackBayesian Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  // Pairs are evaluated by the base class, without the response:
  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "pair", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreatePairEvent(Geometry, 9))) && Passed;
  Passed = Evaluate("Analyze()", "pair", "There is one event", List.GetNRawEvents(), 1) && Passed;
  if (List.GetNRawEvents() == 1) {
    MRERawEvent* Event = List.GetRawEventAt(0);
    Passed = Evaluate("Analyze()", "pair", "The event is a pair event", Event->GetEventType(), MRERawEvent::c_PairEvent) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event is good", Event->IsGoodEvent()) && Passed;
    Passed = Evaluate("Analyze()", "pair", "The event has two tracks", GetNTracks(Event), 2U) && Passed;

    // The sum of half the energy correlations of the two straight legs, see UTERTrack
    vector<double> ElectronEnergies;
    vector<double> PositronEnergies;
    ElectronEnergies.push_back(100.0);
    PositronEnergies.push_back(0.0);
    for (int l = 8; l >= 0; --l) {
      ElectronEnergies.push_back(100.0 + l);
      PositronEnergies.push_back(200.0 + l);
    }
    Passed = EvaluateNear("Analyze()", "pair", "The pair quality factor is the sum of the correlation scores of the two tracks", Event->GetPairQualityFactor(), 0.5*GetIndexCorrelation(ElectronEnergies) + 0.5*GetIndexCorrelation(PositronEnergies), 1e-12) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the nine response files with the given prefix into the temporary directory. Return the name of the ".t.goodbad.rsp" file, or an empty string on failure.
//! Each matrix has one bin on each axis, except the deposited energy with the two bins below and above 100 keV:
//! * start (total energy, incidence angle, deposited energy): good 24 below and 12 above 100 keV, bad 12 and 24
//! * central (total energy, incidence angle, deposited energy, azimuthal and polar scatter angle): good 30 and 10, bad 10 and 30
//! * stop (deposited energy, incidence angle): good 10 and 30, bad 30 and 10
//! * dual (total energy, incidence angle, deposited energy): as start
//! * goodbad: 20 good and 10 bad
MString UTERTrackBayesian::WriteResponse(const MString& Prefix)
{
  MString Path = GetTemporaryFileName(Prefix);
  if (Path == "") {
    return "";
  }

  vector<float> TotalEnergy = { 0.0f, 100000.0f };
  vector<float> DepositedEnergy = { 0.0f, 100.0f, 100000.0f };
  vector<float> Angle = { 0.0f, 181.0f };
  vector<float> GoodOrBad = { 0.0f, 1.0f, 2.0f };

  bool Written = true;
  DisableDefaultStreams();

  MResponseMatrixO1 GoodBad;
  GoodBad.SetAxis(GoodOrBad);
  GoodBad.SetBinContent(0, 20.0f);
  GoodBad.SetBinContent(1, 10.0f);
  if (GoodBad.Write(Path + ".t.goodbad.rsp", true) == false) {
    Written = false;
  }

  // Start and dual; index 0 is good, index 1 is bad
  float StartContent[2][2] = { { 24.0f, 12.0f }, { 12.0f, 24.0f } };
  MString StartNames[2][2] = { { ".t.start.good.rsp", ".t.start.bad.rsp" }, { ".t.dual.good.rsp", ".t.dual.bad.rsp" } };
  for (unsigned int Type = 0; Type < 2; ++Type) {
    for (unsigned int Quality = 0; Quality < 2; ++Quality) {
      MResponseMatrixO3 Matrix;
      Matrix.SetAxis(TotalEnergy, Angle, DepositedEnergy);
      Matrix.SetBinContent(0, 0, 0, StartContent[Quality][0]);
      Matrix.SetBinContent(0, 0, 1, StartContent[Quality][1]);
      if (Matrix.Write(Path + StartNames[Type][Quality], true) == false) {
        Written = false;
      }
    }
  }

  float CentralContent[2][2] = { { 30.0f, 10.0f }, { 10.0f, 30.0f } };
  MString CentralNames[2] = { ".t.central.good.rsp", ".t.central.bad.rsp" };
  for (unsigned int Quality = 0; Quality < 2; ++Quality) {
    MResponseMatrixO5 Matrix;
    Matrix.SetAxis(TotalEnergy, Angle, DepositedEnergy, Angle, Angle);
    Matrix.SetBinContent(0, 0, 0, 0, 0, CentralContent[Quality][0]);
    Matrix.SetBinContent(0, 0, 1, 0, 0, CentralContent[Quality][1]);
    if (Matrix.Write(Path + CentralNames[Quality], true) == false) {
      Written = false;
    }
  }

  float StopContent[2][2] = { { 10.0f, 30.0f }, { 30.0f, 10.0f } };
  MString StopNames[2] = { ".t.stop.good.rsp", ".t.stop.bad.rsp" };
  for (unsigned int Quality = 0; Quality < 2; ++Quality) {
    MResponseMatrixO2 Matrix;
    Matrix.SetAxis(DepositedEnergy, Angle);
    Matrix.SetBinContent(0, 0, StopContent[Quality][0]);
    Matrix.SetBinContent(1, 0, StopContent[Quality][1]);
    if (Matrix.Write(Path + StopNames[Quality], true) == false) {
      Written = false;
    }
  }

  EnableDefaultStreams();

  if (Written == false) {
    return "";
  }

  return Path + ".t.goodbad.rsp";
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new event with a bent track: (0, 0, 3) cm 50 keV, (1, 0, 2) cm 60 keV, (2, 1, 1) cm 80 keV, (3, 3, 0) cm 200 keV. The caller owns it, until it is handed to an incarnation list
MRERawEvent* UTERTrackBayesian::CreateBentTrackEvent(MGeometryRevan& Geometry)
{
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
  AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
  AddTrackerHit(Event, Geometry, 2.0, 1.0, 1, 80.0);
  AddTrackerHit(Event, Geometry, 3.0, 3.0, 0, 200.0);

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTERTrackBayesian Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERTrackBayesian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
