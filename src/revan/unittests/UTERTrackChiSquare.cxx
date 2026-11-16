/*
 * UTERTrackChiSquare.cxx
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
#include "MERTrackChiSquare.h"
#include "UTERTrackShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the electron tracker MERTrackChiSquare. Its own evaluation is not implemented ("nyi" in the code):
//! it uses the evaluation of its base class MERTrack
class UTERTrackChiSquare : public UTERTrackBase
{
public:
  //! Default constructor
  UTERTrackChiSquare() : UTERTrackBase("UTERTrackChiSquare") {}
  //! Default destructor
  virtual ~UTERTrackChiSquare() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the special parameters and the option text
  bool TestOptions();
  //! Test a straight Compton electron track
  bool TestStraightTrack();
  //! Test events without a track
  bool TestNoTrack();
  //! Test a pair event
  bool TestPair();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERTrackChiSquare::Run()
{
  bool Passed = true;

  Passed = TestOptions() && Passed;
  Passed = TestStraightTrack() && Passed;
  Passed = TestNoTrack() && Passed;
  Passed = TestPair() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the special parameters and the option text
bool UTERTrackChiSquare::TestOptions()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackChiSquare Tracker;
  Passed = EvaluateTrue("SetSpecialParameters()", "no parameters", "The call succeeds", Tracker.SetSpecialParameters()) && Passed;

  SetupTracker(Tracker, Geometry, false, true, true, 3, 7, true);
  MString Core;
  Core += "# Search MIPs:                  0\n";
  Core += "# Search Pairs:                 1\n";
  Core += "# Search Comptons:              1\n";
  Core += "# Max. number of ambiguities:   5\n";
  Core += "# Max. layer jump Compton:      3\n";
  Core += "# Number of sequences to keep:  7\n";
  Core += "# Reject pure ambiguities:      1\n";
  Passed = Evaluate("ToString()", "core only", "The text lists the options", Tracker.ToString(true), Core) && Passed;
  Passed = Evaluate("ToString()", "full", "The text has a title and separators around the options", Tracker.ToString(false), MString("# Tracking - ChiSquare options:\n# \n") + Core + "# \n") && Passed;
  Passed = Evaluate("ToString()", "default argument", "The default is the full text", Tracker.ToString(), Tracker.ToString(false)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a straight Compton electron track
bool UTERTrackChiSquare::TestStraightTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackChiSquare Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 2);

  // The evaluation is the one of the base class: 2/(tracks + remaining hits in the tracker) + the correlation score, high values are good
  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "straight track", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateStraightTrackEvent(Geometry))) && Passed;
  Passed = Evaluate("Analyze()", "straight track", "Both directions are kept", List.GetNRawEvents(), 2) && Passed;
  if (List.GetNRawEvents() == 2) {
    MRETrack* Best = GetTrack(List.GetRawEventAt(0), 0);
    MRETrack* Second = GetTrack(List.GetRawEventAt(1), 0);
    Passed = EvaluateTrue("Analyze()", "straight track", "Both events have a track", Best != nullptr && Second != nullptr) && Passed;
    if (Best != nullptr && Second != nullptr) {
      Passed = Evaluate("Analyze()", "straight track", "The best track starts at the small energy deposits in layer 3", GetLayerSequence(Best), MString("3 2 1 0")) && Passed;
      Passed = EvaluateNear("Analyze()", "straight track", "The quality factor of the best track is the correlation r", Best->GetQualityFactor(), GetStraightTrackScore(), 1e-12) && Passed;
      Passed = Evaluate("Analyze()", "straight track", "The second track starts in layer 0", GetLayerSequence(Second), MString("0 1 2 3")) && Passed;
    }
    Passed = EvaluateNear("Analyze()", "straight track", "The best event has the track quality factor 2 + r", List.GetRawEventAt(0)->GetTrackQualityFactor(), 2.0 + GetStraightTrackScore(), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "straight track", "The second event has the track quality factor 2 - r", List.GetRawEventAt(1)->GetTrackQualityFactor(), 2.0 - GetStraightTrackScore(), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "straight track", "The pair quality factor is not set", List.GetRawEventAt(0)->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
    Passed = EvaluateTrue("Analyze()", "straight track", "The best event is the best try", List.GetBestTryEvent() == List.GetRawEventAt(0)) && Passed;
    Passed = EvaluateFalse("Analyze()", "straight track", "There is no optimum event", List.HasOptimumEvent()) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events without a track
bool UTERTrackChiSquare::TestNoTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackChiSquare Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 2);

  // Two hits in the same layer:
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddTrackerHit(Event, Geometry, 2.0, 0.0, 3, 60.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "two hits in the same layer", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "two hits in the same layer", "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      Passed = Evaluate("Analyze()", "two hits in the same layer", "The event has no track", GetNTracks(List.GetRawEventAt(0)), 0U) && Passed;
      // 2/(0 tracks + 2 remaining hits in the tracker)
      Passed = EvaluateNear("Analyze()", "two hits in the same layer", "The track quality factor is 2/2 = 1", List.GetRawEventAt(0)->GetTrackQualityFactor(), 1.0, 1e-12) && Passed;
    }
    List.DeleteAll();
  }

  // One hit in the tracker: the tracker does not touch the event
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "one tracker hit", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "one tracker hit", "There is one event", List.GetNRawEvents(), 1) && Passed;
    Passed = EvaluateTrue("Analyze()", "one tracker hit", "The event is the initial one", List.GetRawEventAt(0) == Event) && Passed;
    Passed = Evaluate("Analyze()", "one tracker hit", "The event has no track", GetNTracks(Event), 0U) && Passed;
    Passed = EvaluateNear("Analyze()", "one tracker hit", "The track quality factor is not set", Event->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a pair event
bool UTERTrackChiSquare::TestPair()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackChiSquare Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "pair", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreatePairEvent(Geometry, 9))) && Passed;
  Passed = Evaluate("Analyze()", "pair", "There is one event", List.GetNRawEvents(), 1) && Passed;
  if (List.GetNRawEvents() == 1) {
    MRERawEvent* Event = List.GetRawEventAt(0);
    Passed = Evaluate("Analyze()", "pair", "The event is a pair event", Event->GetEventType(), MRERawEvent::c_PairEvent) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event is good", Event->IsGoodEvent()) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;
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


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTERTrackChiSquare Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERTrackChiSquare.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
