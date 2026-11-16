/*
 * UTERTrack.cxx
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
#include <new>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MERTrack.h"
#include "MMuonEvent.h"
#include "MPhysicalEvent.h"
#include "UTERTrackShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected members of MERTrack
class UTERTrackAccess : public MERTrack
{
public:
  //! Default constructor - user provided, so that the placement new of the initialization test does not zero the memory first
  UTERTrackAccess() : MERTrack() {}
  //! Default destructor
  virtual ~UTERTrackAccess() {}

  using MERTrack::EvaluateTracks;
  using MERTrack::EvaluateTrack;
  using MERTrack::EvaluatePairs;
  using MERTrack::SortByTrackQualityFactor;
  using MERTrack::IsInTracker;

  using MERTrack::m_ComptonMaxLayerJump;
  using MERTrack::m_NSequencesToKeep;
  using MERTrack::m_MaxNAmbiguities;
  using MERTrack::m_NLayersForVertexSearch;
  using MERTrack::m_DetectorList;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the base class of the electron trackers, MERTrack, which is the tracker "modified Pearson" itself
class UTERTrack : public UTERTrackBase
{
public:
  //! Default constructor
  UTERTrack() : UTERTrackBase("UTERTrack") {}
  //! Default destructor
  virtual ~UTERTrack() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default values after the construction
  bool TestConstruction();
  //! Test the parameters: storage, limits, and the detector list
  bool TestSetParameters();
  //! Test the option text for the tra file
  bool TestToString();
  //! Test which elements of an event are in the tracker
  bool TestIsInTracker();
  //! Test events with fewer than two hits in the tracker: nothing is tracked
  bool TestTooFewHits();
  //! Test a straight Compton electron track: the track, its direction, and the quality factors
  bool TestStraightTrack();
  //! Test the number of kept sequences
  bool TestSequencesToKeep();
  //! Test a track of two hits and two hits in the same layer
  bool TestTwoHits();
  //! Test the maximum layer jump with a track with a missing layer
  bool TestLayerJump();
  //! Test an event with two hits in one layer: the links are ambiguous
  bool TestAmbiguousTrack();
  //! Test the rejection of purely ambiguous events
  bool TestRejectPureAmbiguities();
  //! Test the tracking of events with more ambiguities than the limit
  bool TestHighEnergyTracking();
  //! Test a pair event: vertex, the two tracks, and the quality factors
  bool TestPair();
  //! Test the limits of the vertex search
  bool TestPairLimits();
  //! Test the search for minimum ionizing particles and showers
  bool TestMips();
  //! Test the limits of the search for minimum ionizing particles
  bool TestMipLimits();
  //! Test the search switches: pairs, minimum ionizing particles, Compton tracks
  bool TestSearchSwitches();
  //! Test the evaluation functions directly
  bool TestEvaluation();
  //! Test that a tracker can be used for several events, and the post analysis
  bool TestReuse();

  //! Return true if SetParameters() with a detector list works before the geometry is set
  bool SetParametersWithoutGeometry();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERTrack::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestSetParameters() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestIsInTracker() && Passed;
  Passed = TestTooFewHits() && Passed;
  Passed = TestStraightTrack() && Passed;
  Passed = TestSequencesToKeep() && Passed;
  Passed = TestTwoHits() && Passed;
  Passed = TestLayerJump() && Passed;
  Passed = TestAmbiguousTrack() && Passed;
  Passed = TestRejectPureAmbiguities() && Passed;
  Passed = TestHighEnergyTracking() && Passed;
  Passed = TestPair() && Passed;
  Passed = TestPairLimits() && Passed;
  Passed = TestMips() && Passed;
  Passed = TestMipLimits() && Passed;
  Passed = TestSearchSwitches() && Passed;
  Passed = TestEvaluation() && Passed;
  Passed = TestReuse() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default values after the construction
bool UTERTrack::TestConstruction()
{
  bool Passed = true;

  // Construct the object in a block filled with a pattern, to find members which are not initialized
  const unsigned char Pattern = 0xA5;
  const unsigned int PatternWord = 0xA5A5A5A5;
  alignas(UTERTrackAccess) unsigned char Block[sizeof(UTERTrackAccess)];
  FillMemory(Block, sizeof(UTERTrackAccess), Pattern);
  UTERTrackAccess* Tracker = new (Block) UTERTrackAccess();

  Passed = Evaluate("MERTrack()", "new object", "The maximum layer jump is 2", Tracker->m_ComptonMaxLayerJump, 2U) && Passed;
  Passed = Evaluate("MERTrack()", "new object", "The maximum number of ambiguities is 5", Tracker->m_MaxNAmbiguities, 5U) && Passed;
  Passed = Evaluate("MERTrack()", "new object", "The number of layers for the vertex search is 5", Tracker->m_NLayersForVertexSearch, 5U) && Passed;
  Passed = EvaluateSize("MERTrack()", "new object", "The detector list is empty", Tracker->m_DetectorList.size(), 0) && Passed;
  // Defect REV-F-1: The constructor does not initialize m_NSequencesToKeep, so Analyze() without SetParameters() keeps a random number of sequences
  Passed = ExpectedToFail(EvaluateTrue("MERTrack()", "new object", "The number of sequences to keep is initialized", Tracker->m_NSequencesToKeep != PatternWord)) && Passed;

  MString Options = Tracker->ToString(true);
  Passed = EvaluateTrue("MERTrack()", "new object", "The search for minimum ionizing particles is on", Options.Contains("# Search MIPs:                  1")) && Passed;
  Passed = EvaluateTrue("MERTrack()", "new object", "The search for pairs is on", Options.Contains("# Search Pairs:                 1")) && Passed;
  Passed = EvaluateTrue("MERTrack()", "new object", "The search for Compton tracks is on", Options.Contains("# Search Comptons:              1")) && Passed;
  Passed = EvaluateTrue("MERTrack()", "new object", "Pure ambiguities are not rejected", Options.Contains("# Reject pure ambiguities:      0")) && Passed;

  Tracker->~UTERTrackAccess();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the parameters: storage, limits, and the detector list
bool UTERTrack::TestSetParameters()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // Representative values:
  {
    UTERTrackAccess Tracker;
    Tracker.SetGeometry(&Geometry);
    DisableDefaultStreams();
    bool Result = Tracker.SetParameters(false, true, false, 3, 7, true, 6, GetTrackingDetectors());
    EnableDefaultStreams();
    Passed = EvaluateTrue("SetParameters()", "representative values", "The parameters are accepted", Result) && Passed;
    Passed = Evaluate("SetParameters()", "representative values", "The maximum layer jump is 3", Tracker.m_ComptonMaxLayerJump, 3U) && Passed;
    Passed = Evaluate("SetParameters()", "representative values", "The number of sequences to keep is 7", Tracker.m_NSequencesToKeep, 7U) && Passed;
    Passed = Evaluate("SetParameters()", "representative values", "The number of layers for the vertex search is 6", Tracker.m_NLayersForVertexSearch, 6U) && Passed;
    Passed = EvaluateSize("SetParameters()", "representative values", "There is one tracking detector", Tracker.m_DetectorList.size(), 1) && Passed;
    if (Tracker.m_DetectorList.size() == 1) {
      Passed = Evaluate("SetParameters()", "representative values", "The tracking detector is the tracker", Tracker.m_DetectorList[0]->GetName(), MString("Tracker")) && Passed;
    }
    MString Options = Tracker.ToString(true);
    Passed = EvaluateTrue("SetParameters()", "representative values", "The search for minimum ionizing particles is off", Options.Contains("# Search MIPs:                  0")) && Passed;
    Passed = EvaluateTrue("SetParameters()", "representative values", "The search for pairs is on", Options.Contains("# Search Pairs:                 1")) && Passed;
    Passed = EvaluateTrue("SetParameters()", "representative values", "The search for Compton tracks is off", Options.Contains("# Search Comptons:              0")) && Passed;
    Passed = EvaluateTrue("SetParameters()", "representative values", "Pure ambiguities are rejected", Options.Contains("# Reject pure ambiguities:      1")) && Passed;

    // A second call replaces the first one:
    DisableDefaultStreams();
    Tracker.SetParameters(true, false, true, 1, 2, false, 4, GetTrackingDetectors());
    EnableDefaultStreams();
    Passed = Evaluate("SetParameters()", "second call", "The maximum layer jump is 1", Tracker.m_ComptonMaxLayerJump, 1U) && Passed;
    Passed = Evaluate("SetParameters()", "second call", "The number of sequences to keep is 2", Tracker.m_NSequencesToKeep, 2U) && Passed;
    // Defect REV-F-2: SetParameters() never clears the detector list, so each call appends the detectors again
    Passed = ExpectedToFail(EvaluateSize("SetParameters()", "second call", "There is still one tracking detector", Tracker.m_DetectorList.size(), 1)) && Passed;
  }

  // The limits: just at the limit the values are kept, below they are set to the limit
  {
    UTERTrackAccess Tracker;
    Tracker.SetGeometry(&Geometry);
    DisableDefaultStreams();
    bool Result = Tracker.SetParameters(true, true, true, 1, 1, false, 4, GetTrackingDetectors());
    EnableDefaultStreams();
    Passed = EvaluateTrue("SetParameters()", "at the limits", "The parameters are accepted", Result) && Passed;
    Passed = Evaluate("SetParameters()", "at the limits", "The maximum layer jump stays 1", Tracker.m_ComptonMaxLayerJump, 1U) && Passed;
    Passed = Evaluate("SetParameters()", "at the limits", "The number of sequences to keep stays 1", Tracker.m_NSequencesToKeep, 1U) && Passed;
    Passed = Evaluate("SetParameters()", "at the limits", "The number of layers for the vertex search stays 4", Tracker.m_NLayersForVertexSearch, 4U) && Passed;

    DisableDefaultStreams();
    Result = Tracker.SetParameters(true, true, true, 0, 0, false, 3, GetTrackingDetectors());
    EnableDefaultStreams();
    Passed = EvaluateTrue("SetParameters()", "below the limits", "The parameters are accepted", Result) && Passed;
    Passed = Evaluate("SetParameters()", "below the limits", "The maximum layer jump is set to 1", Tracker.m_ComptonMaxLayerJump, 1U) && Passed;
    Passed = Evaluate("SetParameters()", "below the limits", "The number of sequences to keep is set to 1", Tracker.m_NSequencesToKeep, 1U) && Passed;
    Passed = Evaluate("SetParameters()", "below the limits", "The number of layers for the vertex search is set to 4", Tracker.m_NLayersForVertexSearch, 4U) && Passed;
  }

  // The detector list:
  {
    UTERTrackAccess Tracker;
    Tracker.SetGeometry(&Geometry);
    vector<MString> None;
    DisableDefaultStreams();
    bool Result = Tracker.SetParameters(true, true, true, 2, 1, false, 4, None);
    EnableDefaultStreams();
    Passed = EvaluateTrue("SetParameters()", "no detectors", "The parameters are accepted", Result) && Passed;
    Passed = EvaluateSize("SetParameters()", "no detectors", "There is no tracking detector", Tracker.m_DetectorList.size(), 0) && Passed;
  }
  {
    UTERTrackAccess Tracker;
    Tracker.SetGeometry(&Geometry);
    vector<MString> Names;
    Names.push_back("NoSuchDetector");
    Names.push_back("Depth");
    Names.push_back("Tracker");
    DisableDefaultStreams();
    bool Result = Tracker.SetParameters(true, true, true, 2, 1, false, 4, Names);
    EnableDefaultStreams();
    Passed = EvaluateTrue("SetParameters()", "unknown detector", "The parameters are accepted", Result) && Passed;
    Passed = EvaluateSize("SetParameters()", "unknown detector", "The two known detectors are the tracking detectors", Tracker.m_DetectorList.size(), 2) && Passed;
    if (Tracker.m_DetectorList.size() == 2) {
      Passed = Evaluate("SetParameters()", "unknown detector", "The first tracking detector is the depth detector", Tracker.m_DetectorList[0]->GetName(), MString("Depth")) && Passed;
      Passed = Evaluate("SetParameters()", "unknown detector", "The second tracking detector is the tracker", Tracker.m_DetectorList[1]->GetName(), MString("Tracker")) && Passed;
    }
  }

  // Defect REV-F-3: SetParameters() with a detector list dereferences the geometry without a check, so it crashes if SetGeometry() was not called before
  Passed = ExpectedToCrash(EvaluateTrue("SetParameters()", "no geometry", "The call does not crash without a geometry", SetParametersWithoutGeometry())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the option text for the tra file
bool UTERTrack::TestToString()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
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
  Passed = Evaluate("ToString()", "full", "The text has a title and separators around the options", Tracker.ToString(false), MString("# Tracking - FoM options:\n# \n") + Core + "# \n") && Passed;
  Passed = Evaluate("ToString()", "default argument", "The default is the full text", Tracker.ToString(), Tracker.ToString(false)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test which elements of an event are in the tracker
bool UTERTrack::TestIsInTracker()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  UTERTrackAccess Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  MRERawEvent* Event = new MRERawEvent(&Geometry);
  MRESE* TrackerHit = AddTrackerHit(Event, Geometry, 1.0, 2.0, 3, 100.0);
  MRESE* CalorimeterHit = AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
  MREHit* DepthHit = CreateHit(MVector(30.0, 0.0, 0.0), 100.0, MDDetector::c_Strip3D, &Geometry);
  MREHit* ShieldHit = CreateHit(MVector(0.0, 0.0, 10.0), 100.0, MDDetector::c_Scintillator, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "four detectors", "All hits are created", TrackerHit != nullptr && CalorimeterHit != nullptr && DepthHit != nullptr && ShieldHit != nullptr) && Passed;
  if (TrackerHit != nullptr && CalorimeterHit != nullptr && DepthHit != nullptr && ShieldHit != nullptr) {
    Event->AddRESE(DepthHit);
    Event->AddRESE(ShieldHit);

    Passed = EvaluateTrue("IsInTracker()", "hit in the tracker", "A hit in a tracking detector is in the tracker", Tracker.IsInTracker(TrackerHit)) && Passed;
    Passed = EvaluateFalse("IsInTracker()", "hit in the calorimeter", "A hit in the calorimeter is not in the tracker", Tracker.IsInTracker(CalorimeterHit)) && Passed;
    Passed = EvaluateFalse("IsInTracker()", "hit in the depth detector", "A hit in a strip detector which is not in the list is not in the tracker", Tracker.IsInTracker(DepthHit)) && Passed;
    Passed = EvaluateFalse("IsInTracker()", "hit in the shield", "A hit in the shield is not in the tracker", Tracker.IsInTracker(ShieldHit)) && Passed;

    // A track is always in the tracker, also an empty one
    MRETrack Track;
    Passed = EvaluateTrue("IsInTracker()", "track", "A track is in the tracker", Tracker.IsInTracker(&Track)) && Passed;

    // With the depth detector in the list:
    UTERTrackAccess Both;
    Both.SetGeometry(&Geometry);
    vector<MString> Names = GetTrackingDetectors();
    Names.push_back("Depth");
    DisableDefaultStreams();
    Both.SetParameters(false, true, true, 2, 1, false, 4, Names);
    EnableDefaultStreams();
    Passed = EvaluateTrue("IsInTracker()", "two tracking detectors, tracker", "A hit in the first tracking detector is in the tracker", Both.IsInTracker(TrackerHit)) && Passed;
    Passed = EvaluateTrue("IsInTracker()", "two tracking detectors, depth detector", "A hit in the second tracking detector is in the tracker", Both.IsInTracker(DepthHit)) && Passed;
    Passed = EvaluateFalse("IsInTracker()", "two tracking detectors, calorimeter", "A hit in the calorimeter is not in the tracker", Both.IsInTracker(CalorimeterHit)) && Passed;
  }

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events with fewer than two hits in the tracker: nothing is tracked
bool UTERTrack::TestTooFewHits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, true, true, true);

  // One hit in the tracker, one in the calorimeter:
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "one tracker hit", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "one tracker hit", "There is one event", List.GetNRawEvents(), 1) && Passed;
    Passed = EvaluateTrue("Analyze()", "one tracker hit", "The event is the initial one", List.GetRawEventAt(0) == Event) && Passed;
    Passed = Evaluate("Analyze()", "one tracker hit", "The event has no track", GetNTracks(Event), 0U) && Passed;
    Passed = Evaluate("Analyze()", "one tracker hit", "The event still has its two hits", Event->GetNRESEs(), 2) && Passed;
    Passed = EvaluateNear("Analyze()", "one tracker hit", "The track quality factor is not set", Event->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
    Passed = EvaluateFalse("Analyze()", "one tracker hit", "There is no optimum event", List.HasOptimumEvent()) && Passed;
    // Without a best try event the list returns its first event
    Passed = EvaluateTrue("Analyze()", "one tracker hit", "The best try is the initial event", List.GetBestTryEvent() == Event) && Passed;
    Passed = Evaluate("Analyze()", "one tracker hit", "The event type is unknown", Event->GetEventType(), MRERawEvent::c_UnknownEvent) && Passed;
    List.DeleteAll();
  }

  // Two hits in the calorimeter and none in the tracker:
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
    AddCalorimeterHit(Event, Geometry, MVector(2.0, 0.0, -9.0), 300.0);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "no tracker hit", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "no tracker hit", "There is one event", List.GetNRawEvents(), 1) && Passed;
    Passed = Evaluate("Analyze()", "no tracker hit", "The event has no track", GetNTracks(Event), 0U) && Passed;
    Passed = Evaluate("Analyze()", "no tracker hit", "The event still has its two hits", Event->GetNRESEs(), 2) && Passed;
    List.DeleteAll();
  }

  // No hits at all:
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "empty event", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "empty event", "There is one event", List.GetNRawEvents(), 1) && Passed;
    Passed = Evaluate("Analyze()", "empty event", "The event is empty", Event->GetNRESEs(), 0) && Passed;
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a straight Compton electron track: the track, its direction, and the quality factors
bool UTERTrack::TestStraightTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "straight track", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateStraightTrackEvent(Geometry))) && Passed;
  Passed = Evaluate("Analyze()", "straight track", "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
  if (List.GetNRawEvents() == 1) {
    MRERawEvent* Event = List.GetRawEventAt(0);
    Passed = Evaluate("Analyze()", "straight track", "The event has one track", GetNTracks(Event), 1U) && Passed;
    Passed = Evaluate("Analyze()", "straight track", "The event has one more element: the calorimeter hit", GetNNonTracks(Event), 1U) && Passed;
    Passed = EvaluateTrue("Analyze()", "straight track", "The event is the best try", List.GetBestTryEvent() == Event) && Passed;
    Passed = EvaluateFalse("Analyze()", "straight track", "There is no optimum event: the event type is not decided by the tracker", List.HasOptimumEvent()) && Passed;
    Passed = Evaluate("Analyze()", "straight track", "The event type is unknown", Event->GetEventType(), MRERawEvent::c_UnknownEvent) && Passed;
    Passed = EvaluateNear("Analyze()", "straight track", "The energy of the event is the sum of all hits, 890 keV", Event->GetEnergy(), 890.0, 1e-10) && Passed;
    Passed = EvaluateTrue("Analyze()", "straight track", "The event has no vertex", Event->GetVertex() == nullptr) && Passed;

    MRETrack* Track = GetTrack(Event, 0);
    if (Track != nullptr) {
      Passed = Evaluate("Analyze()", "straight track", "The track has all four tracker hits", Track->GetNRESEs(), 4) && Passed;
      // An electron deposits more energy at the end of its track, thus the start is the end with the small deposits
      Passed = Evaluate("Analyze()", "straight track", "The track starts in layer 3 and follows the layers down to layer 0", GetLayerSequence(Track), MString("3 2 1 0")) && Passed;
      Passed = EvaluateNear("Analyze()", "straight track", "The energy of the track is 50 + 60 + 80 + 200 = 390 keV", Track->GetEnergy(), 390.0, 1e-10) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "straight track", "The direction of the track is the one from the first to the second hit, (1, 0, -1)/sqrt(2)", Track->GetDirection(), MVector(1.0, 0.0, -1.0).Unit(), 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "straight track", "The quality factor of the track is the correlation of its energies with the position along the track", Track->GetQualityFactor(), GetStraightTrackScore(), 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "straight track", "The reference implementation gives the same correlation", GetCorrelationScore(Track), GetStraightTrackScore(), 1e-12) && Passed;
      Passed = EvaluateFalse("Analyze()", "straight track", "The direction of the track is not fixed", Track->IsDirectionFixed()) && Passed;
    }

    // The quality factor is 2/(tracks + remaining hits in the tracker) + mean track quality = 2/1 + r
    Passed = EvaluateNear("Analyze()", "straight track", "The track quality factor of the event is 2 + the one of the track", Event->GetTrackQualityFactor(), 2.0 + GetStraightTrackScore(), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "straight track", "The pair quality factor is not set", Event->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
  }
  List.DeleteAll();

  // The same track with the large energy deposits at the top: now it starts at the bottom
  MRERawEvent* Upward = new MRERawEvent(&Geometry);
  AddTrackerHit(Upward, Geometry, 0.0, 0.0, 3, 200.0);
  AddTrackerHit(Upward, Geometry, 1.0, 0.0, 2, 80.0);
  AddTrackerHit(Upward, Geometry, 2.0, 0.0, 1, 60.0);
  AddTrackerHit(Upward, Geometry, 3.0, 0.0, 0, 50.0);
  Passed = EvaluateTrue("Analyze()", "upward track", "The analysis succeeds", AnalyzeEvent(Tracker, List, Upward)) && Passed;
  Passed = Evaluate("Analyze()", "upward track", "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
  if (List.GetNRawEvents() == 1) {
    MRETrack* Track = GetTrack(List.GetRawEventAt(0), 0);
    Passed = EvaluateTrue("Analyze()", "upward track", "The event has a track", Track != nullptr) && Passed;
    if (Track != nullptr) {
      Passed = Evaluate("Analyze()", "upward track", "The track starts in layer 0 and ends in layer 3", GetLayerSequence(Track), MString("0 1 2 3")) && Passed;
      Passed = EvaluateNear("Analyze()", "upward track", "The quality factor of the track is the same as for the downward track", Track->GetQualityFactor(), GetStraightTrackScore(), 1e-12) && Passed;
    }
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the number of kept sequences
bool UTERTrack::TestSequencesToKeep()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // The straight track has two sequences: the two directions. Keep both:
  for (unsigned int Keep = 2; Keep <= 3; ++Keep) {
    MERTrack Tracker;
    SetupTracker(Tracker, Geometry, false, true, true, 2, Keep);

    MString Input = MString("straight track, keep ") + Keep;
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateStraightTrackEvent(Geometry))) && Passed;
    Passed = Evaluate("Analyze()", Input, "Both directions of the track are kept", List.GetNRawEvents(), 2) && Passed;
    if (List.GetNRawEvents() == 2) {
      MRETrack* Best = GetTrack(List.GetRawEventAt(0), 0);
      MRETrack* Second = GetTrack(List.GetRawEventAt(1), 0);
      Passed = EvaluateTrue("Analyze()", Input, "Both events have a track", Best != nullptr && Second != nullptr) && Passed;
      if (Best != nullptr && Second != nullptr) {
        Passed = Evaluate("Analyze()", Input, "The first event has the track from layer 3 to 0", GetLayerSequence(Best), MString("3 2 1 0")) && Passed;
        Passed = Evaluate("Analyze()", Input, "The second event has the track from layer 0 to 3", GetLayerSequence(Second), MString("0 1 2 3")) && Passed;
        // In the wrong direction the energies decrease along the track: the correlation changes its sign
        Passed = EvaluateNear("Analyze()", Input, "The quality factor of the second track is the negative correlation", Second->GetQualityFactor(), -GetStraightTrackScore(), 1e-12) && Passed;
      }
      Passed = EvaluateNear("Analyze()", Input, "The first event has the track quality factor 2 + r", List.GetRawEventAt(0)->GetTrackQualityFactor(), 2.0 + GetStraightTrackScore(), 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", Input, "The second event has the track quality factor 2 - r", List.GetRawEventAt(1)->GetTrackQualityFactor(), 2.0 - GetStraightTrackScore(), 1e-12) && Passed;
      Passed = EvaluateTrue("Analyze()", Input, "The first event is the best try", List.GetBestTryEvent() == List.GetRawEventAt(0)) && Passed;
      Passed = Evaluate("Analyze()", Input, "The calorimeter hit is in the first event", GetNNonTracks(List.GetRawEventAt(0)), 1U) && Passed;
      Passed = Evaluate("Analyze()", Input, "The calorimeter hit is in the second event", GetNNonTracks(List.GetRawEventAt(1)), 1U) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a track of two hits and two hits in the same layer
bool UTERTrack::TestTwoHits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 2);

  // Two hits in neighboring layers are a track:
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "two hits in neighboring layers", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "two hits in neighboring layers", "Both directions are kept", List.GetNRawEvents(), 2) && Passed;
    if (List.GetNRawEvents() == 2) {
      MRETrack* Best = GetTrack(List.GetRawEventAt(0), 0);
      MRETrack* Second = GetTrack(List.GetRawEventAt(1), 0);
      Passed = EvaluateTrue("Analyze()", "two hits in neighboring layers", "Both events have a track", Best != nullptr && Second != nullptr) && Passed;
      if (Best != nullptr && Second != nullptr) {
        Passed = Evaluate("Analyze()", "two hits in neighboring layers", "The best track starts at the hit with the smaller energy", GetLayerSequence(Best), MString("3 2")) && Passed;
        // Two values which increase have the correlation +1, two which decrease -1
        Passed = EvaluateNear("Analyze()", "two hits in neighboring layers", "The quality factor of the best track is 1", Best->GetQualityFactor(), 1.0, 1e-12) && Passed;
        Passed = Evaluate("Analyze()", "two hits in neighboring layers", "The second track has the other direction", GetLayerSequence(Second), MString("2 3")) && Passed;
        Passed = EvaluateNear("Analyze()", "two hits in neighboring layers", "The quality factor of the second track is -1", Second->GetQualityFactor(), -1.0, 1e-12) && Passed;
      }
      Passed = EvaluateNear("Analyze()", "two hits in neighboring layers", "The best event has the track quality factor 2/1 + 1 = 3", List.GetRawEventAt(0)->GetTrackQualityFactor(), 3.0, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "two hits in neighboring layers", "The second event has the track quality factor 2/1 - 1 = 1", List.GetRawEventAt(1)->GetTrackQualityFactor(), 1.0, 1e-12) && Passed;
    }
    List.DeleteAll();
  }

  // Two hits in the same layer are no track:
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
      Passed = Evaluate("Analyze()", "two hits in the same layer", "The event has its three hits", List.GetRawEventAt(0)->GetNRESEs(), 3) && Passed;
      // 2/(0 tracks + 2 remaining hits in the tracker) + 0
      Passed = EvaluateNear("Analyze()", "two hits in the same layer", "The track quality factor is 2/2 = 1", List.GetRawEventAt(0)->GetTrackQualityFactor(), 1.0, 1e-12) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the maximum layer jump with a track with a missing layer
bool UTERTrack::TestLayerJump()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // Hits in layers 3 and 2, and then in layer 2 - Distance: the hit is linked if the distance is at most the maximum layer jump
  for (unsigned int Jump = 1; Jump <= 3; ++Jump) {
    for (int Distance = 1; Distance <= 4; ++Distance) {
      MERTrack Tracker;
      SetupTracker(Tracker, Geometry, false, true, true, Jump, 1);

      MRERawEvent* Event = new MRERawEvent(&Geometry);
      AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
      AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
      AddTrackerHit(Event, Geometry, 3.0, 0.0, 2 - Distance, 200.0);

      MString Input = MString("maximum jump ") + Jump + ", layer distance " + Distance;
      MRawEventIncarnations List(&Geometry);
      Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
      Passed = Evaluate("Analyze()", Input, "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
      if (List.GetNRawEvents() == 1) {
        MRETrack* Track = GetTrack(List.GetRawEventAt(0), 0);
        Passed = EvaluateTrue("Analyze()", Input, "The event has a track", Track != nullptr) && Passed;
        if (Track != nullptr) {
          if (static_cast<unsigned int>(Distance) <= Jump) {
            Passed = Evaluate("Analyze()", Input, "The track has all three hits", Track->GetNRESEs(), 3) && Passed;
            Passed = Evaluate("Analyze()", Input, "No hit remains", GetNNonTracks(List.GetRawEventAt(0)), 0U) && Passed;
            // Correlation of (50, 60, 200) with (1, 2, 3): deviations (-53.33, -43.33, 96.67) and (-1, 0, 1), thus r = 150/sqrt(2*14066.67) = 0.894295...
            Passed = EvaluateNear("Analyze()", Input, "The track quality factor is 2/1 + r(50, 60, 200)", List.GetRawEventAt(0)->GetTrackQualityFactor(), 2.0 + 150.0/sqrt(2.0*42200.0/3.0), 1e-12) && Passed;
          } else {
            Passed = Evaluate("Analyze()", Input, "The track has only the two hits in neighboring layers", Track->GetNRESEs(), 2) && Passed;
            Passed = Evaluate("Analyze()", Input, "The third hit remains", GetNNonTracks(List.GetRawEventAt(0)), 1U) && Passed;
            // 2/(1 track + 1 remaining hit) + 1
            Passed = EvaluateNear("Analyze()", Input, "The track quality factor is 2/2 + 1 = 2", List.GetRawEventAt(0)->GetTrackQualityFactor(), 2.0, 1e-12) && Passed;
          }
        }
      }
      List.DeleteAll();
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an event with two hits in one layer: the links are ambiguous
bool UTERTrack::TestAmbiguousTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // One hit in layer 3, two in layer 2, one in layer 1: several sequences are possible. Keep up to 100:
  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 100);

  MRERawEvent* Event = new MRERawEvent(&Geometry);
  AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
  AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
  AddTrackerHit(Event, Geometry, -1.0, 0.0, 2, 70.0);
  AddTrackerHit(Event, Geometry, 2.0, 0.0, 1, 80.0);

  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "two hits in one of three layers", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
  Passed = EvaluateTrue("Analyze()", "two hits in one of three layers", "There are several sequences", List.GetNRawEvents() > 2) && Passed;

  bool Sorted = true;
  bool AllHitsKept = true;
  bool AllEnergiesKept = true;
  bool QualityFactorsMatch = true;
  for (int e = 0; e < List.GetNRawEvents(); ++e) {
    MRERawEvent* RE = List.GetRawEventAt(e);
    if (e > 0 && RE->GetTrackQualityFactor() > List.GetRawEventAt(e-1)->GetTrackQualityFactor()) {
      Sorted = false;
    }

    // Each sequence contains each of the four hits once, in a track or not, and thus all the energy:
    int NHits = GetNNonTracks(RE);
    double TrackScores = 0.0;
    for (unsigned int t = 0; t < GetNTracks(RE); ++t) {
      NHits += GetTrack(RE, t)->GetNRESEs();
      TrackScores += GetCorrelationScore(GetTrack(RE, t));
    }
    if (NHits != 4) {
      AllHitsKept = false;
    }
    if (fabs(RE->GetEnergy() - 260.0) > 1e-10) {
      AllEnergiesKept = false;
    }

    // 2/(tracks + remaining hits) + the mean of the track scores
    double Reference = 2.0/RE->GetNRESEs();
    if (GetNTracks(RE) > 0) {
      Reference += TrackScores/GetNTracks(RE);
    }
    if (fabs(RE->GetTrackQualityFactor() - Reference) > 1e-12) {
      QualityFactorsMatch = false;
    }
  }
  Passed = EvaluateTrue("Analyze()", "two hits in one of three layers", "The sequences are sorted by decreasing track quality factor", Sorted) && Passed;
  Passed = EvaluateTrue("Analyze()", "two hits in one of three layers", "Each sequence contains the four hits", AllHitsKept) && Passed;
  Passed = EvaluateTrue("Analyze()", "two hits in one of three layers", "Each sequence has the total energy of 260 keV", AllEnergiesKept) && Passed;
  Passed = EvaluateTrue("Analyze()", "two hits in one of three layers", "Each track quality factor is 2/(tracks + remaining hits) + the mean correlation score of the tracks", QualityFactorsMatch) && Passed;

  if (List.GetNRawEvents() > 0) {
    // The best sequence has all hits in one track. The best correlation of the four energies in a sequence which the links allow
    // (layer 3 and layer 1 each link to layer 2 only) is 0.8, e.g. for (60, 50, 70, 80): deviations (-5, -15, 5, 15) and (-1.5, -0.5, 0.5, 1.5),
    // thus r = 40/sqrt(5*500) = 0.8; (50, 60, 70, 80) would need a link between the two hits of layer 2
    Passed = Evaluate("Analyze()", "two hits in one of three layers", "The best sequence has one track", GetNTracks(List.GetRawEventAt(0)), 1U) && Passed;
    Passed = Evaluate("Analyze()", "two hits in one of three layers", "The best sequence has no remaining hit", GetNNonTracks(List.GetRawEventAt(0)), 0U) && Passed;
    Passed = EvaluateNear("Analyze()", "two hits in one of three layers", "The best track quality factor is 2/1 + 0.8", List.GetRawEventAt(0)->GetTrackQualityFactor(), 2.8, 1e-12) && Passed;
    Passed = EvaluateTrue("Analyze()", "two hits in one of three layers", "The best sequence is the best try", List.GetBestTryEvent() == List.GetRawEventAt(0)) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rejection of purely ambiguous events
bool UTERTrack::TestRejectPureAmbiguities()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // Two hits in each of two layers: there is no link which is not ambiguous
  for (unsigned int Reject = 0; Reject <= 1; ++Reject) {
    MERTrack Tracker;
    if (Reject == 1) {
      SetupTracker(Tracker, Geometry, false, true, true, 2, 1, true);
    } else {
      SetupTracker(Tracker, Geometry, false, true, true, 2, 1, false);
    }

    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddTrackerHit(Event, Geometry, 2.0, 0.0, 3, 60.0);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 2, 50.0);
    AddTrackerHit(Event, Geometry, 2.0, 0.0, 2, 60.0);

    MRawEventIncarnations List(&Geometry);
    if (Reject == 1) {
      Passed = EvaluateTrue("Analyze()", "purely ambiguous, rejection on", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
      Passed = Evaluate("Analyze()", "purely ambiguous, rejection on", "There is one event", List.GetNRawEvents(), 1) && Passed;
      if (List.GetNRawEvents() == 1) {
        Passed = Evaluate("Analyze()", "purely ambiguous, rejection on", "The event has no track", GetNTracks(List.GetRawEventAt(0)), 0U) && Passed;
        Passed = Evaluate("Analyze()", "purely ambiguous, rejection on", "The event has its four hits", List.GetRawEventAt(0)->GetNRESEs(), 4) && Passed;
        // 2/(0 tracks + 4 remaining hits)
        Passed = EvaluateNear("Analyze()", "purely ambiguous, rejection on", "The track quality factor is 2/4 = 0.5", List.GetRawEventAt(0)->GetTrackQualityFactor(), 0.5, 1e-12) && Passed;
      }
    } else {
      Passed = EvaluateTrue("Analyze()", "purely ambiguous, rejection off", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
      Passed = Evaluate("Analyze()", "purely ambiguous, rejection off", "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
      if (List.GetNRawEvents() == 1) {
        Passed = Evaluate("Analyze()", "purely ambiguous, rejection off", "The best sequence has one track", GetNTracks(List.GetRawEventAt(0)), 1U) && Passed;
        Passed = Evaluate("Analyze()", "purely ambiguous, rejection off", "The best sequence has no remaining hit", GetNNonTracks(List.GetRawEventAt(0)), 0U) && Passed;
        // The best sequence is (50, 50, 60, 60): deviations (-5, -5, 5, 5) and (-1.5, -0.5, 0.5, 1.5), thus r = 20/sqrt(5*100) = 2/sqrt(5)
        Passed = EvaluateNear("Analyze()", "purely ambiguous, rejection off", "The track quality factor is 2/1 + 2/sqrt(5)", List.GetRawEventAt(0)->GetTrackQualityFactor(), 2.0 + 2.0/sqrt(5.0), 1e-12) && Passed;
      }
    }
    List.DeleteAll();

    // The straight track has links which are not ambiguous: it is tracked in both cases
    MString Input = "straight track, rejection off";
    if (Reject == 1) {
      Input = "straight track, rejection on";
    }
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateStraightTrackEvent(Geometry))) && Passed;
    Passed = Evaluate("Analyze()", Input, "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      Passed = Evaluate("Analyze()", Input, "The event has one track", GetNTracks(List.GetRawEventAt(0)), 1U) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the tracking of events with more ambiguities than the limit
bool UTERTrack::TestHighEnergyTracking()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 2);

  // One hit in layers 9 and 8 and two hits in layers 7, 6, and 5:
  // The number of ambiguities is the number of hits minus the number of links which are not ambiguous: 8 - 1 = 7, above the limit of 5.
  // Then no combinations are tested: The one track segment (9 - 8) is continued step by step with the hit which
  // is within the maximum layer jump of one of its ends and deviates least from the direction of the track at this end
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  AddTrackerHit(Event, Geometry, 0.0, 0.0, 9, 50.0);
  AddTrackerHit(Event, Geometry, 0.0, 0.0, 8, 50.0);
  for (int l = 7; l >= 5; --l) {
    AddTrackerHit(Event, Geometry, 0.0, 0.0, l, 100.0 + l);
    AddTrackerHit(Event, Geometry, 3.0, 0.0, l, 200.0 + l);
  }

  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "seven ambiguities", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
  // One track in its two directions:
  Passed = Evaluate("Analyze()", "seven ambiguities", "There are two sequences", List.GetNRawEvents(), 2) && Passed;
  if (List.GetNRawEvents() == 2) {
    MRERawEvent* Best = List.GetRawEventAt(0);
    Passed = Evaluate("Analyze()", "seven ambiguities", "The best sequence has one track", GetNTracks(Best), 1U) && Passed;
    Passed = Evaluate("Analyze()", "seven ambiguities", "The best sequence has no remaining hit", GetNNonTracks(Best), 0U) && Passed;
    Passed = EvaluateNear("Analyze()", "seven ambiguities", "The event has the total energy of 1036 keV", Best->GetEnergy(), 1036.0, 1e-10) && Passed;
    MRETrack* Track = GetTrack(Best, 0);
    if (Track != nullptr) {
      // First the three hits on the axis below the segment continue it without deviation (layers 7, 6, 5).
      // At the end in layer 5 the hits at x = 3 cm in layer 6 and 7 are in reach (the one in layer 5 has the layer distance 0):
      // (3, 0, 1) deviates by 108.4 degrees from the direction (0, 0, -1) of the track, (3, 0, 2) by 123.7 degrees.
      // At the other end in layer 9 only the hit at x = 3 cm in layer 7 is in reach, with 123.7 degrees. Thus layer 6 at x = 3 cm follows,
      // then layer 7 (71.6 degrees instead of 108.4 degrees for layer 5), and finally layer 5.
      vector<MRESE*> Sequence = GetSequence(Track);
      Passed = Evaluate("Analyze()", "seven ambiguities", "The track follows the layers 9 8 7 6 5 down, and then 6 7 5", GetLayerSequence(Track), MString("9 8 7 6 5 6 7 5")) && Passed;
      if (Sequence.size() == 8) {
        Passed = EvaluateNear("Analyze()", "seven ambiguities", "The fifth hit is on the axis", Sequence[4]->GetPosition().X(), 0.0, 1e-12) && Passed;
        Passed = EvaluateNear("Analyze()", "seven ambiguities", "The sixth hit is at x = 3 cm", Sequence[5]->GetPosition().X(), 3.0, 1e-12) && Passed;
        Passed = EvaluateNear("Analyze()", "seven ambiguities", "The last hit is at x = 3 cm", Sequence[7]->GetPosition().X(), 3.0, 1e-12) && Passed;
      }
      Passed = EvaluateNear("Analyze()", "seven ambiguities", "The quality factor of the track is the mean of the correlations of its energies and its direction changes", Track->GetQualityFactor(), GetCorrelationScore(Track), 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "seven ambiguities", "The track quality factor is 2/1 + the one of the track", Best->GetTrackQualityFactor(), 2.0 + GetCorrelationScore(Track), 1e-12) && Passed;
    }
    MRETrack* Reverse = GetTrack(List.GetRawEventAt(1), 0);
    Passed = EvaluateTrue("Analyze()", "seven ambiguities", "The second sequence has a track", Reverse != nullptr) && Passed;
    if (Reverse != nullptr) {
      Passed = Evaluate("Analyze()", "seven ambiguities", "The second sequence is the track in the other direction", GetLayerSequence(Reverse), MString("5 7 6 5 6 7 8 9")) && Passed;
    }
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a pair event: vertex, the two tracks, and the quality factors
bool UTERTrack::TestPair()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, true, true, true);

  // A vertex in layer 9 and two legs through all layers down to layer 0:
  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "pair", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreatePairEvent(Geometry, 9))) && Passed;
  Passed = Evaluate("Analyze()", "pair", "There is one event", List.GetNRawEvents(), 1) && Passed;
  if (List.GetNRawEvents() == 1) {
    MRERawEvent* Event = List.GetRawEventAt(0);
    Passed = Evaluate("Analyze()", "pair", "The event is a pair event", Event->GetEventType(), MRERawEvent::c_PairEvent) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event is good", Event->IsGoodEvent()) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event is the best try", List.GetBestTryEvent() == Event) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event has a vertex", Event->GetVertex() != nullptr) && Passed;
    if (Event->GetVertex() != nullptr) {
      Passed = EvaluateVectorNear("Analyze()", "pair", "The vertex is the hit in layer 9", Event->GetVertex()->GetPosition(), MVector(0.0, 0.0, 9.0), 1e-12) && Passed;
    }
    Passed = Evaluate("Analyze()", "pair", "The vertex direction is downward", Event->GetVertexDirection(), -1) && Passed;
    Passed = Evaluate("Analyze()", "pair", "The event has two tracks", GetNTracks(Event), 2U) && Passed;
    Passed = Evaluate("Analyze()", "pair", "The event has no remaining hit", GetNNonTracks(Event), 0U) && Passed;
    // 100 keV at the vertex, and sum(100 + l) + sum(200 + l) for l = 0 to 8: 100 + 936 + 1836
    Passed = EvaluateNear("Analyze()", "pair", "The event has the total energy of 2872 keV", Event->GetEnergy(), 2872.0, 1e-10) && Passed;

    MRETrack* Electron = dynamic_cast<MRETrack*>(Event->GetElectronTrack());
    MRETrack* Positron = dynamic_cast<MRETrack*>(Event->GetPositronTrack());
    Passed = EvaluateTrue("Analyze()", "pair", "The electron and the positron track are set", Electron != nullptr && Positron != nullptr && Electron != Positron) && Passed;
    if (Electron != nullptr && Positron != nullptr) {
      // The first leg (x > 0) is taken as the electron: its hit is the first one below the vertex
      Passed = Evaluate("Analyze()", "pair", "The electron track follows all layers from the vertex down", GetLayerSequence(Electron), MString("9 8 7 6 5 4 3 2 1 0")) && Passed;
      Passed = Evaluate("Analyze()", "pair", "The positron track follows all layers from the vertex down", GetLayerSequence(Positron), MString("9 8 7 6 5 4 3 2 1 0")) && Passed;
      Passed = EvaluateTrue("Analyze()", "pair", "The electron track has a stop point", Electron->GetStopPoint() != nullptr) && Passed;
      Passed = EvaluateTrue("Analyze()", "pair", "The positron track has a stop point", Positron->GetStopPoint() != nullptr) && Passed;
      if (Electron->GetStopPoint() != nullptr && Positron->GetStopPoint() != nullptr) {
        Passed = EvaluateVectorNear("Analyze()", "pair", "The electron track stops at the end of the first leg", Electron->GetStopPoint()->GetPosition(), MVector(2.25, 0.0, 0.0), 1e-12) && Passed;
        Passed = EvaluateVectorNear("Analyze()", "pair", "The positron track stops at the end of the second leg", Positron->GetStopPoint()->GetPosition(), MVector(-2.25, 0.0, 0.0), 1e-12) && Passed;
      }
      bool LegsSeparated = true;
      for (MRESE* RESE: GetSequence(Electron)) {
        if (RESE->GetPosition().X() < 0.0) {
          LegsSeparated = false;
        }
      }
      for (MRESE* RESE: GetSequence(Positron)) {
        if (RESE->GetPosition().X() > 0.0) {
          LegsSeparated = false;
        }
      }
      Passed = EvaluateTrue("Analyze()", "pair", "No hit of one leg is in the track of the other leg", LegsSeparated) && Passed;
      // The energy of the vertex counts once: the electron track has it, the vertex of the positron track is a copy without energy
      Passed = EvaluateNear("Analyze()", "pair", "The electron track has the vertex energy and the first leg: 100 + 936 keV", Electron->GetEnergy(), 1036.0, 1e-10) && Passed;
      Passed = EvaluateNear("Analyze()", "pair", "The positron track has the second leg: 1836 keV", Positron->GetEnergy(), 1836.0, 1e-10) && Passed;

      // The legs are straight, so the direction changes do not correlate with the position (0), and the score is half the energy correlation
      vector<double> ElectronEnergies;
      vector<double> PositronEnergies;
      ElectronEnergies.push_back(100.0);
      PositronEnergies.push_back(0.0);
      for (int l = 8; l >= 0; --l) {
        ElectronEnergies.push_back(100.0 + l);
        PositronEnergies.push_back(200.0 + l);
      }
      double ElectronScore = 0.5*GetIndexCorrelation(ElectronEnergies);
      double PositronScore = 0.5*GetIndexCorrelation(PositronEnergies);
      Passed = EvaluateNear("Analyze()", "pair", "The quality factor of the electron track is half the correlation of its energies with the position", Electron->GetQualityFactor(), ElectronScore, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "pair", "The quality factor of the positron track is half the correlation of its energies with the position", Positron->GetQualityFactor(), PositronScore, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "pair", "The pair quality factor is the sum of the quality factors of the two tracks", Event->GetPairQualityFactor(), ElectronScore + PositronScore, 1e-12) && Passed;
    }
    Passed = EvaluateNear("Analyze()", "pair", "The track quality factor is not set", Event->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the vertex search
bool UTERTrack::TestPairLimits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  // A vertex needs more than four layers with at least two hits below it:
  for (int NLayers = 4; NLayers <= 5; ++NLayers) {
    MString Input = MString("legs with ") + NLayers + " layers";
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, CreatePairEvent(Geometry, NLayers))) && Passed;
    Passed = EvaluateTrue("Analyze()", Input, "There is an event", List.GetNRawEvents() >= 1) && Passed;
    if (List.GetNRawEvents() >= 1) {
      MRERawEvent* Event = List.GetRawEventAt(0);
      if (NLayers == 4) {
        Passed = EvaluateTrue("Analyze()", Input, "Four layers are not enough: the event has no vertex", Event->GetVertex() == nullptr) && Passed;
        Passed = Evaluate("Analyze()", Input, "The event type is unknown", Event->GetEventType(), MRERawEvent::c_UnknownEvent) && Passed;
        Passed = EvaluateFalse("Analyze()", Input, "There is no optimum event", List.HasOptimumEvent()) && Passed;
      } else {
        Passed = EvaluateTrue("Analyze()", Input, "Five layers are enough: the event has a vertex", Event->GetVertex() != nullptr) && Passed;
        Passed = Evaluate("Analyze()", Input, "The event is a pair event", Event->GetEventType(), MRERawEvent::c_PairEvent) && Passed;
        Passed = Evaluate("Analyze()", Input, "The event has two tracks", GetNTracks(Event), 2U) && Passed;
        if (GetNTracks(Event) == 2) {
          Passed = Evaluate("Analyze()", Input, "The first track follows the layers from the vertex down", GetLayerSequence(GetTrack(Event, 0)), MString("9 8 7 6 5 4")) && Passed;
          Passed = Evaluate("Analyze()", Input, "The second track follows the layers from the vertex down", GetLayerSequence(GetTrack(Event, 1)), MString("9 8 7 6 5 4")) && Passed;
        }
      }
    }
    List.DeleteAll();
  }

  // The layers with at least two hits must be more than half of the layers between the first two and the last two neighboring layers with two hits:
  // The first leg has a hit in each layer below the vertex in layer 9, the second leg only at the given layer distances to the vertex
  for (unsigned int Case = 0; Case <= 1; ++Case) {
    // CheckForPair() counts the layers with at least two hits, and divides by (stop - start): start is the first of two neighboring layers with two hits (distance 1),
    // stop the layer distance of the last hit, moved up to the layer below the last two neighboring layers with two hits.
    // Case 0: two hits at the layer distances 1, 2, 7, 8, 9 and one hit at 3 to 6 and 10: stop is 10, thus 5/(10 - 1) > 0.5 -> a vertex
    // Case 1: two hits at the layer distances 1, 2, 8, 9, 10 and one hit at 3 to 7 and 11: stop is 11, thus 5/(11 - 1) = 0.5 -> no vertex
    int NLayers = 10;
    int FirstLowerDouble = 7;
    MString Input = "second leg at layer distances 1, 2, 7, 8, 9 of 10";
    if (Case == 1) {
      NLayers = 11;
      FirstLowerDouble = 8;
      Input = "second leg at layer distances 1, 2, 8, 9, 10 of 11";
    }
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 9, 100.0);
    for (int d = 1; d <= NLayers; ++d) {
      AddTrackerHit(Event, Geometry, 0.25*d, 0.0, 9 - d, 100.0 + d);
      if (d == 1 || d == 2 || (d >= FirstLowerDouble && d <= FirstLowerDouble + 2)) {
        AddTrackerHit(Event, Geometry, -0.25*d, 0.0, 9 - d, 200.0 + d);
      }
    }
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = EvaluateTrue("Analyze()", Input, "There is an event", List.GetNRawEvents() >= 1) && Passed;
    if (List.GetNRawEvents() >= 1) {
      if (Case == 0) {
        // 5 layers with two hits, stop - start = 10 - 1 = 9: 5/9 > 0.5
        Passed = EvaluateTrue("Analyze()", Input, "5 of 9 layers have two hits: the event has a vertex", List.GetRawEventAt(0)->GetVertex() != nullptr) && Passed;
      } else {
        // 5 layers with two hits, stop - start = 11 - 1 = 10: 5/10 is not above 0.5
        Passed = EvaluateTrue("Analyze()", Input, "5 of 10 layers have two hits: the event has no vertex", List.GetRawEventAt(0)->GetVertex() == nullptr) && Passed;
      }
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for minimum ionizing particles and showers
bool UTERTrack::TestMips()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, true, true, true);

  // A straight line through ten layers, 100 keV in each:
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    for (int l = 9; l >= 0; --l) {
      AddTrackerHit(Event, Geometry, 0.25*l, 0.0, l, 100.0);
    }
    AddCalorimeterHit(Event, Geometry, MVector(-2.5, 0.0, -10.0), 5000.0);

    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "muon", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "muon", "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      MRERawEvent* RE = List.GetRawEventAt(0);
      Passed = Evaluate("Analyze()", "muon", "The event is a muon event", RE->GetEventType(), MRERawEvent::c_MipEvent) && Passed;
      Passed = EvaluateTrue("Analyze()", "muon", "The event is good", RE->IsGoodEvent()) && Passed;
      Passed = EvaluateTrue("Analyze()", "muon", "The event is the optimum event", List.GetOptimumEvent() == RE) && Passed;
      Passed = EvaluateTrue("Analyze()", "muon", "The event is the best try", List.GetBestTryEvent() == RE) && Passed;
      Passed = Evaluate("Analyze()", "muon", "The event has one track", GetNTracks(RE), 1U) && Passed;
      Passed = Evaluate("Analyze()", "muon", "The calorimeter hit remains", GetNNonTracks(RE), 1U) && Passed;
      MRETrack* Track = GetTrack(RE, 0);
      if (Track != nullptr) {
        Passed = Evaluate("Analyze()", "muon", "The track follows all layers from the top to the bottom", GetLayerSequence(Track), MString("9 8 7 6 5 4 3 2 1 0")) && Passed;
        Passed = EvaluateNear("Analyze()", "muon", "The track has the energy of the ten hits", Track->GetEnergy(), 1000.0, 1e-10) && Passed;
      }
      MPhysicalEvent* Physical = RE->GetPhysicalEvent();
      Passed = EvaluateTrue("Analyze()", "muon", "The event has a physical event", Physical != nullptr) && Passed;
      if (Physical != nullptr) {
        Passed = Evaluate("Analyze()", "muon", "The physical event is a muon", Physical->GetType(), MPhysicalEvent::c_Muon) && Passed;
        MMuonEvent* Muon = dynamic_cast<MMuonEvent*>(Physical);
        if (Muon != nullptr) {
          Passed = EvaluateNear("Analyze()", "muon", "The energy of the muon is the energy of all hits", Muon->GetEnergy(), 6000.0, 1e-10) && Passed;
          // All hits have the same energy, so the center is the middle of the line from (2.25, 0, 9) to (0, 0, 0)
          Passed = EvaluateVectorNear("Analyze()", "muon", "The center of gravity is the center of the track", Muon->GetCenterOfGravity(), MVector(1.125, 0.0, 4.5), 1e-12) && Passed;
        }
      }
    }
    List.DeleteAll();
  }

  // Two straight lines which are not on a common line: a shower
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    for (int l = 9; l >= 5; --l) {
      AddTrackerHit(Event, Geometry, 0.0, 0.0, l, 100.0);
    }
    for (int l = 4; l >= 0; --l) {
      AddTrackerHit(Event, Geometry, 3.0, 0.0, l, 100.0);
    }

    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "shower", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "shower", "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      MRERawEvent* RE = List.GetRawEventAt(0);
      Passed = Evaluate("Analyze()", "shower", "The event is a shower event", RE->GetEventType(), MRERawEvent::c_ShowerEvent) && Passed;
      Passed = EvaluateTrue("Analyze()", "shower", "The event is good", RE->IsGoodEvent()) && Passed;
      Passed = EvaluateTrue("Analyze()", "shower", "The event is the optimum event", List.GetOptimumEvent() == RE) && Passed;
      Passed = Evaluate("Analyze()", "shower", "The event has two tracks", GetNTracks(RE), 2U) && Passed;
      Passed = Evaluate("Analyze()", "shower", "No hit remains", GetNNonTracks(RE), 0U) && Passed;
      if (GetNTracks(RE) == 2) {
        MString First = GetLayerSequence(GetTrack(RE, 0));
        MString Second = GetLayerSequence(GetTrack(RE, 1));
        Passed = EvaluateTrue("Analyze()", "shower", "One track has the layers 9 to 5 and the other one the layers 4 to 0", (First == "9 8 7 6 5" && Second == "4 3 2 1 0") || (First == "4 3 2 1 0" && Second == "9 8 7 6 5")) && Passed;
      }
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the search for minimum ionizing particles
bool UTERTrack::TestMipLimits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, true, true, true);

  // The straightness: three hits are on a line if the directions of the two segments differ by less than 0.05 rad.
  // Hits on the z axis in layers 9, 8, 6, 5, 4, 3, and the one in layer 7 next to the axis, so that the segment 8 -> 7 has the given angle to the axis
  for (unsigned int Case = 0; Case <= 1; ++Case) {
    double Angle = 0.049;
    MString Input = "0.049 rad off the line";
    if (Case == 1) {
      Angle = 0.051;
      Input = "0.051 rad off the line";
    }
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 9, 100.0);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 8, 100.0);
    AddTrackerHit(Event, Geometry, tan(Angle), 0.0, 7, 100.0);
    for (int l = 6; l >= 3; --l) {
      AddTrackerHit(Event, Geometry, 0.0, 0.0, l, 100.0);
    }

    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", Input, "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      MRERawEvent* RE = List.GetRawEventAt(0);
      Passed = Evaluate("Analyze()", Input, "The event is a muon event", RE->GetEventType(), MRERawEvent::c_MipEvent) && Passed;
      Passed = Evaluate("Analyze()", Input, "The event has one track", GetNTracks(RE), 1U) && Passed;
      MRETrack* Track = GetTrack(RE, 0);
      if (Track != nullptr) {
        if (Case == 0) {
          Passed = Evaluate("Analyze()", Input, "The hit in layer 7 is on the line: the track has all seven hits", Track->GetNRESEs(), 7) && Passed;
          Passed = Evaluate("Analyze()", Input, "No hit remains", GetNNonTracks(RE), 0U) && Passed;
        } else {
          Passed = Evaluate("Analyze()", Input, "The hit in layer 7 is not on the line: the track has six hits", Track->GetNRESEs(), 6) && Passed;
          Passed = Evaluate("Analyze()", Input, "The hit in layer 7 remains", GetNNonTracks(RE), 1U) && Passed;
        }
      }
    }
    List.DeleteAll();
  }

  // The length: a track of a minimum ionizing particle needs at least four hits (MinElements in CheckForMips()).
  // Three hits on a line are a short Compton electron track
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 2, 50.0);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 1, 60.0);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 0, 200.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);

    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "three hits on a line", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "three hits on a line", "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      // Defect REV-F-4: CheckForMips() determines the tracks with more than MinElements hits, but then takes any single track as a muon, also one with three hits
      Passed = ExpectedToFail(Evaluate("Analyze()", "three hits on a line", "Three hits are not a muon: the event type is unknown", List.GetRawEventAt(0)->GetEventType(), MRERawEvent::c_UnknownEvent)) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search switches: pairs, minimum ionizing particles, Compton tracks
bool UTERTrack::TestSearchSwitches()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // Without the pair search the pair is no pair event:
  {
    MERTrack Tracker;
    SetupTracker(Tracker, Geometry, false, false, true);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "pair, no pair search", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreatePairEvent(Geometry, 9))) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair, no pair search", "There is an event", List.GetNRawEvents() >= 1) && Passed;
    if (List.GetNRawEvents() >= 1) {
      Passed = EvaluateTrue("Analyze()", "pair, no pair search", "The event has no vertex", List.GetRawEventAt(0)->GetVertex() == nullptr) && Passed;
      Passed = Evaluate("Analyze()", "pair, no pair search", "The event type is unknown", List.GetRawEventAt(0)->GetEventType(), MRERawEvent::c_UnknownEvent) && Passed;
      Passed = EvaluateFalse("Analyze()", "pair, no pair search", "There is no optimum event", List.HasOptimumEvent()) && Passed;
    }
    List.DeleteAll();
  }

  // Without the search for minimum ionizing particles the straight line is a Compton track:
  {
    MERTrack Tracker;
    SetupTracker(Tracker, Geometry, false, true, true);
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    for (int l = 9; l >= 0; --l) {
      AddTrackerHit(Event, Geometry, 0.25*l, 0.0, l, 100.0);
    }
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "muon, no muon search", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "muon, no muon search", "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      MRERawEvent* RE = List.GetRawEventAt(0);
      Passed = Evaluate("Analyze()", "muon, no muon search", "The event type is unknown", RE->GetEventType(), MRERawEvent::c_UnknownEvent) && Passed;
      Passed = Evaluate("Analyze()", "muon, no muon search", "The event has one track", GetNTracks(RE), 1U) && Passed;
      // All energies are the same and the line is straight: both correlations are 0
      Passed = EvaluateNear("Analyze()", "muon, no muon search", "The track quality factor is 2/1 + 0", RE->GetTrackQualityFactor(), 2.0, 1e-12) && Passed;
    }
    List.DeleteAll();
  }

  // Without any search nothing is tracked:
  {
    MERTrack Tracker;
    SetupTracker(Tracker, Geometry, false, false, false);
    MRERawEvent* Event = CreateStraightTrackEvent(Geometry);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "straight track, no search", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "straight track, no search", "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      Passed = EvaluateTrue("Analyze()", "straight track, no search", "The event is the initial one", List.GetRawEventAt(0) == Event) && Passed;
      Passed = Evaluate("Analyze()", "straight track, no search", "The event has no track", GetNTracks(List.GetRawEventAt(0)), 0U) && Passed;
      Passed = Evaluate("Analyze()", "straight track, no search", "The event has its five hits", List.GetRawEventAt(0)->GetNRESEs(), 5) && Passed;
      Passed = EvaluateNear("Analyze()", "straight track, no search", "The track quality factor is not set", List.GetRawEventAt(0)->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
    }
    List.DeleteAll();
  }

  // Only the pair search: the straight track is not tracked
  {
    MERTrack Tracker;
    SetupTracker(Tracker, Geometry, false, true, false);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "straight track, only pair search", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateStraightTrackEvent(Geometry))) && Passed;
    Passed = Evaluate("Analyze()", "straight track, only pair search", "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      Passed = Evaluate("Analyze()", "straight track, only pair search", "The event has no track", GetNTracks(List.GetRawEventAt(0)), 0U) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the evaluation functions directly
bool UTERTrack::TestEvaluation()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  UTERTrackAccess Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  // A track of three hits with a start point: EvaluateTrack() sets the correlation score
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  MRESE* Hit3 = AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
  MRESE* Hit2 = AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
  MRESE* Hit1 = AddTrackerHit(Event, Geometry, 2.0, 0.0, 1, 200.0);
  MRESE* Loose = AddTrackerHit(Event, Geometry, -3.0, 0.0, 7, 70.0);
  AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
  Passed = EvaluateTrue("AddTrackerHit()", "four hits", "The hits are created", Hit3 != nullptr && Hit2 != nullptr && Hit1 != nullptr && Loose != nullptr) && Passed;
  if (Hit3 != nullptr && Hit2 != nullptr && Hit1 != nullptr && Loose != nullptr) {
    Event->RemoveRESE(Hit3);
    Event->RemoveRESE(Hit2);
    Event->RemoveRESE(Hit1);
    Event->CompressRESEs();
    MRETrack* Track = new MRETrack();
    Track->AddRESE(Hit3);
    Track->AddRESE(Hit2);
    Track->AddRESE(Hit1);
    Track->LinkSequential();
    Event->AddRESE(Track);

    // (50, 60, 200) with (1, 2, 3): r = 150/sqrt(2*42200/3) = 0.894295..., see TestLayerJump()
    double Score = 150.0/sqrt(2.0*42200.0/3.0);
    Track->SetQualityFactor(-5.0);
    Passed = EvaluateTrue("EvaluateTrack()", "track of three hits", "The evaluation succeeds", Tracker.EvaluateTrack(Track)) && Passed;
    Passed = EvaluateNear("EvaluateTrack()", "track of three hits", "The quality factor is the correlation of the energies with the position", Track->GetQualityFactor(), Score, 1e-12) && Passed;

    // The event: one track and one more hit in the tracker; the calorimeter hit does not count
    Track->SetQualityFactor(-5.0);
    Event->SetPairQualityFactor(1.0);
    Passed = EvaluateTrue("EvaluateTracks()", "track and a hit", "The evaluation succeeds", Tracker.EvaluateTracks(Event)) && Passed;
    Passed = EvaluateNear("EvaluateTracks()", "track and a hit", "The quality factor of the track is set", Track->GetQualityFactor(), Score, 1e-12) && Passed;
    Passed = EvaluateNear("EvaluateTracks()", "track and a hit", "The track quality factor is 2/(1 + 1) + r", Event->GetTrackQualityFactor(), 1.0 + Score, 1e-12) && Passed;
    Passed = EvaluateNear("EvaluateTracks()", "track and a hit", "The pair quality factor is reset", Event->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;

    // With a vertex the pair evaluation is used: without electron and positron track there is no quality
    Event->SetVertex(Loose);
    Event->SetTrackQualityFactor(7.0);
    Passed = EvaluateTrue("EvaluateTracks()", "event with vertex", "The evaluation succeeds", Tracker.EvaluateTracks(Event)) && Passed;
    Passed = EvaluateNear("EvaluateTracks()", "event with vertex", "The track quality factor is not changed", Event->GetTrackQualityFactor(), 7.0, 0.0) && Passed;
    Passed = EvaluateNear("EvaluatePairs()", "vertex without tracks", "The pair quality factor is not defined", Event->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
    // Defect REV-F-5: EvaluatePairs() only rejects a pair quality below -3.3, so an event with a vertex but without electron and positron track is a good pair event
    Passed = ExpectedToFail(EvaluateFalse("EvaluatePairs()", "vertex without tracks", "An event without electron and positron track is not a good pair event", Event->IsGoodEvent())) && Passed;
  }
  delete Event;

  // SortByTrackQualityFactor(): high values first
  MRawEventIncarnations List(&Geometry);
  double Factors[4] = { 1.5, 3.0, -2.0, 2.25 };
  for (unsigned int i = 0; i < 4; ++i) {
    MRERawEvent* RE = new MRERawEvent(&Geometry);
    RE->SetTrackQualityFactor(Factors[i]);
    List.AddRawEvent(RE);
  }
  Tracker.SortByTrackQualityFactor(&List);
  double Sorted[4] = { 3.0, 2.25, 1.5, -2.0 };
  for (int i = 0; i < 4; ++i) {
    Passed = EvaluateNear("SortByTrackQualityFactor()", MString("position ") + i, "The events are sorted by decreasing track quality factor", List.GetRawEventAt(i)->GetTrackQualityFactor(), Sorted[i], 0.0) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that a tracker can be used for several events, and the post analysis
bool UTERTrack::TestReuse()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrack Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  // Compton track and pair, twice - with the same tracker and the same list:
  MRawEventIncarnations List(&Geometry);
  for (unsigned int Round = 0; Round < 2; ++Round) {
    MString Input = MString("straight track, round ") + Round;
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateStraightTrackEvent(Geometry))) && Passed;
    Passed = Evaluate("Analyze()", Input, "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      Passed = EvaluateNear("Analyze()", Input, "The track quality factor is 2 + r", List.GetRawEventAt(0)->GetTrackQualityFactor(), 2.0 + GetStraightTrackScore(), 1e-12) && Passed;
      Passed = EvaluateFalse("Analyze()", Input, "There is no optimum event", List.HasOptimumEvent()) && Passed;
    }

    Input = MString("pair, round ") + Round;
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, CreatePairEvent(Geometry, 9))) && Passed;
    Passed = Evaluate("Analyze()", Input, "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      Passed = Evaluate("Analyze()", Input, "The event is a pair event", List.GetRawEventAt(0)->GetEventType(), MRERawEvent::c_PairEvent) && Passed;
    }
  }
  List.DeleteAll();

  Passed = EvaluateTrue("PostAnalysis()", "after four events", "The post analysis succeeds", Tracker.PostAnalysis()) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "after four events", "The pre analysis succeeds", Tracker.PreAnalysis()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if SetParameters() with a detector list works before the geometry is set
bool UTERTrack::SetParametersWithoutGeometry()
{
  MERTrack Tracker;
  DisableDefaultStreams();
  bool Result = Tracker.SetParameters(true, true, true, 2, 1, false, 4, GetTrackingDetectors());
  EnableDefaultStreams();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTERTrack Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERTrack.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
