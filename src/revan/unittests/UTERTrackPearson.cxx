/*
 * UTERTrackPearson.cxx
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
#include "MERTrackPearson.h"
#include "UTERTrackShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected members of MERTrackPearson
class UTERTrackPearsonAccess : public MERTrackPearson
{
public:
  //! Default constructor
  UTERTrackPearsonAccess() : MERTrackPearson() {}
  //! Default destructor
  virtual ~UTERTrackPearsonAccess() {}

  using MERTrackPearson::EvaluateTracks;
  using MERTrackPearson::EvaluateTrack;
  using MERTrackPearson::SortByTrackQualityFactor;

  using MERTrackPearson::m_AllowOnlyMinNumberOfRESEsD1;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the electron tracker MERTrackPearson: the sequences and directions of the tracks are rated with
//! the Pearson correlation scaled to 0 (good) to 1 (bad)
class UTERTrackPearson : public UTERTrackBase
{
public:
  //! Default constructor
  UTERTrackPearson() : UTERTrackBase("UTERTrackPearson") {}
  //! Default destructor
  virtual ~UTERTrackPearson() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the construction, the special parameters, and the option text
  bool TestConstructionAndOptions();
  //! Test a straight Compton electron track
  bool TestStraightTrack();
  //! Test an event with two tracks
  bool TestTwoTracks();
  //! Test events without a track
  bool TestNoTrack();
  //! Test that only the sequences with the smallest number of elements in the tracker are kept
  bool TestMinimumNumberOfElements();
  //! Test a pair event
  bool TestPair();
  //! Test the evaluation functions directly
  bool TestEvaluation();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERTrackPearson::Run()
{
  bool Passed = true;

  Passed = TestConstructionAndOptions() && Passed;
  Passed = TestStraightTrack() && Passed;
  Passed = TestTwoTracks() && Passed;
  Passed = TestNoTrack() && Passed;
  Passed = TestMinimumNumberOfElements() && Passed;
  Passed = TestPair() && Passed;
  Passed = TestEvaluation() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction, the special parameters, and the option text
bool UTERTrackPearson::TestConstructionAndOptions()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  UTERTrackPearsonAccess Tracker;
  Passed = EvaluateTrue("MERTrackPearson()", "new object", "Only the sequences with the smallest number of elements in the tracker are allowed", Tracker.m_AllowOnlyMinNumberOfRESEsD1) && Passed;
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
  Passed = Evaluate("ToString()", "full", "The text has a title and separators around the options", Tracker.ToString(false), MString("# Tracking - Pearson options:\n# \n") + Core + "# \n") && Passed;
  Passed = Evaluate("ToString()", "default argument", "The default is the full text", Tracker.ToString(), Tracker.ToString(false)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a straight Compton electron track
bool UTERTrackPearson::TestStraightTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // The quality factor of a track is (1 - r)/2, with the correlation r of the energies with the position along the track:
  // 0 is a perfect electron track, 1 a track in the wrong direction
  double Good = 0.5*(1.0 - GetStraightTrackScore());
  double Bad = 0.5*(1.0 + GetStraightTrackScore());

  // Keep one sequence:
  {
    MERTrackPearson Tracker;
    SetupTracker(Tracker, Geometry, false, true, true, 2, 1);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "straight track", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateStraightTrackEvent(Geometry))) && Passed;
    Passed = Evaluate("Analyze()", "straight track", "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      MRERawEvent* Event = List.GetRawEventAt(0);
      Passed = Evaluate("Analyze()", "straight track", "The event has one track", GetNTracks(Event), 1U) && Passed;
      Passed = Evaluate("Analyze()", "straight track", "The calorimeter hit remains", GetNNonTracks(Event), 1U) && Passed;
      Passed = EvaluateTrue("Analyze()", "straight track", "The event is the best try", List.GetBestTryEvent() == Event) && Passed;
      Passed = EvaluateFalse("Analyze()", "straight track", "There is no optimum event", List.HasOptimumEvent()) && Passed;
      MRETrack* Track = GetTrack(Event, 0);
      if (Track != nullptr) {
        Passed = Evaluate("Analyze()", "straight track", "The track starts at the small energy deposits in layer 3", GetLayerSequence(Track), MString("3 2 1 0")) && Passed;
        Passed = EvaluateNear("Analyze()", "straight track", "The quality factor of the track is (1 - r)/2", Track->GetQualityFactor(), Good, 1e-12) && Passed;
      }
      // One track: 1 - (1 - q) = q
      Passed = EvaluateNear("Analyze()", "straight track", "The track quality factor of the event is the one of its track", Event->GetTrackQualityFactor(), Good, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "straight track", "The pair quality factor is not set", Event->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
    }
    List.DeleteAll();
  }

  // Keep both directions: the good one is the first
  {
    MERTrackPearson Tracker;
    SetupTracker(Tracker, Geometry, false, true, true, 2, 5);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "straight track, keep 5", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreateStraightTrackEvent(Geometry))) && Passed;
    Passed = Evaluate("Analyze()", "straight track, keep 5", "Both directions are kept", List.GetNRawEvents(), 2) && Passed;
    if (List.GetNRawEvents() == 2) {
      Passed = EvaluateNear("Analyze()", "straight track, keep 5", "The first event has the track quality factor (1 - r)/2", List.GetRawEventAt(0)->GetTrackQualityFactor(), Good, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "straight track, keep 5", "The second event has the track quality factor (1 + r)/2", List.GetRawEventAt(1)->GetTrackQualityFactor(), Bad, 1e-12) && Passed;
      MRETrack* Second = GetTrack(List.GetRawEventAt(1), 0);
      Passed = EvaluateTrue("Analyze()", "straight track, keep 5", "The second event has a track", Second != nullptr) && Passed;
      if (Second != nullptr) {
        Passed = Evaluate("Analyze()", "straight track, keep 5", "The track of the second event starts in layer 0", GetLayerSequence(Second), MString("0 1 2 3")) && Passed;
      }
      Passed = EvaluateTrue("Analyze()", "straight track, keep 5", "The first event is the best try", List.GetBestTryEvent() == List.GetRawEventAt(0)) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an event with two tracks
bool UTERTrackPearson::TestTwoTracks()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackPearson Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 10);

  // Track A in layers 3 and 2 (50 and 60 keV), track B in layers 9, 8, 7 (80, 60, 70 keV). They are too far apart to be linked.
  // A from layer 3: r = +1, q = 0; from layer 2: r = -1, q = 1
  // B from layer 7: (70, 60, 80) with (1, 2, 3): deviations (0, -10, 10) and (-1, 0, 1), r = 10/sqrt(2*200) = 0.5, q = 0.25; from layer 9: r = -0.5, q = 0.75
  // The event has 1 - (1 - qA)(1 - qB): 0.25, 0.75, 1, 1
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
  AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
  AddTrackerHit(Event, Geometry, 0.0, 0.0, 9, 80.0);
  AddTrackerHit(Event, Geometry, 1.0, 0.0, 8, 60.0);
  AddTrackerHit(Event, Geometry, 1.0, 0.0, 7, 70.0);

  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "two tracks", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
  Passed = Evaluate("Analyze()", "two tracks", "The four combinations of the directions are kept", List.GetNRawEvents(), 4) && Passed;
  if (List.GetNRawEvents() == 4) {
    double Factors[4] = { 0.25, 0.75, 1.0, 1.0 };
    for (int e = 0; e < 4; ++e) {
      Passed = Evaluate("Analyze()", MString("two tracks, sequence ") + e, "The event has two tracks", GetNTracks(List.GetRawEventAt(e)), 2U) && Passed;
      Passed = EvaluateNear("Analyze()", MString("two tracks, sequence ") + e, "The track quality factors increase: 0.25, 0.75, 1, 1", List.GetRawEventAt(e)->GetTrackQualityFactor(), Factors[e], 1e-12) && Passed;
    }
    MRERawEvent* Best = List.GetRawEventAt(0);
    if (GetNTracks(Best) == 2) {
      MRETrack* A = GetTrack(Best, 0);
      MRETrack* B = GetTrack(Best, 1);
      if (A->GetNRESEs() == 3) {
        A = GetTrack(Best, 1);
        B = GetTrack(Best, 0);
      }
      Passed = Evaluate("Analyze()", "two tracks, best sequence", "The track of two hits starts in layer 3", GetLayerSequence(A), MString("3 2")) && Passed;
      Passed = Evaluate("Analyze()", "two tracks, best sequence", "The track of three hits starts in layer 7", GetLayerSequence(B), MString("7 8 9")) && Passed;
      Passed = EvaluateNear("Analyze()", "two tracks, best sequence", "The quality factor of the track of two hits is 0", A->GetQualityFactor(), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "two tracks, best sequence", "The quality factor of the track of three hits is 0.25", B->GetQualityFactor(), 0.25, 1e-12) && Passed;
    }
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events without a track
bool UTERTrackPearson::TestNoTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackPearson Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 10);

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
      Passed = Evaluate("Analyze()", "two hits in the same layer", "The event has its three hits", List.GetRawEventAt(0)->GetNRESEs(), 3) && Passed;
      Passed = EvaluateNear("Analyze()", "two hits in the same layer", "An event without a track has no track quality factor", List.GetRawEventAt(0)->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
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
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that only the sequences with the smallest number of elements in the tracker are kept
bool UTERTrackPearson::TestMinimumNumberOfElements()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // One hit in layer 3, two in layer 2, one in layer 1: the possible sequences have one track of four hits,
  // or tracks and remaining hits. The base class keeps all of them, this class only those with the fewest elements in the tracker: one track
  for (unsigned int Class = 0; Class <= 1; ++Class) {
    MERTrack* Tracker = nullptr;
    MString Input;
    if (Class == 0) {
      Tracker = new MERTrack();
      Input = "MERTrack";
    } else {
      Tracker = new MERTrackPearson();
      Input = "MERTrackPearson";
    }
    SetupTracker(*Tracker, Geometry, false, true, true, 2, 100);

    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
    AddTrackerHit(Event, Geometry, -1.0, 0.0, 2, 70.0);
    AddTrackerHit(Event, Geometry, 2.0, 0.0, 1, 80.0);

    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(*Tracker, List, Event)) && Passed;
    Passed = EvaluateTrue("Analyze()", Input, "There are sequences", List.GetNRawEvents() >= 1) && Passed;

    int MaximumElements = 0;
    bool Sorted = true;
    for (int e = 0; e < List.GetNRawEvents(); ++e) {
      if (List.GetRawEventAt(e)->GetNRESEs() > MaximumElements) {
        MaximumElements = List.GetRawEventAt(e)->GetNRESEs();
      }
      if (e > 0 && List.GetRawEventAt(e)->GetTrackQualityFactor() < List.GetRawEventAt(e-1)->GetTrackQualityFactor()) {
        Sorted = false;
      }
    }
    if (Class == 0) {
      Passed = EvaluateTrue("Analyze()", Input, "The base class also keeps sequences with several elements in the tracker", MaximumElements > 1) && Passed;
    } else {
      Passed = Evaluate("Analyze()", Input, "All kept sequences have one element in the tracker: one track with all hits", MaximumElements, 1) && Passed;
      Passed = EvaluateTrue("Analyze()", Input, "The sequences are sorted by increasing track quality factor", Sorted) && Passed;
      if (List.GetNRawEvents() >= 1) {
        // The best sequence which the links allow has the correlation 0.8, e.g. (60, 50, 70, 80), see UTERTrack: (1 - 0.8)/2 = 0.1
        Passed = EvaluateNear("Analyze()", Input, "The best track quality factor is 0.1", List.GetRawEventAt(0)->GetTrackQualityFactor(), 0.1, 1e-12) && Passed;
      }
    }
    List.DeleteAll();
    delete Tracker;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a pair event
bool UTERTrackPearson::TestPair()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackPearson Tracker;
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

    // Pairs are evaluated by the base class: the sum of half the energy correlations of the two straight legs, see UTERTrack
    vector<double> ElectronEnergies;
    vector<double> PositronEnergies;
    ElectronEnergies.push_back(100.0);
    PositronEnergies.push_back(0.0);
    for (int l = 8; l >= 0; --l) {
      ElectronEnergies.push_back(100.0 + l);
      PositronEnergies.push_back(200.0 + l);
    }
    Passed = EvaluateNear("Analyze()", "pair", "The pair quality factor is the sum of the correlation scores of the two tracks", Event->GetPairQualityFactor(), 0.5*GetIndexCorrelation(ElectronEnergies) + 0.5*GetIndexCorrelation(PositronEnergies), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "pair", "The track quality factor is not set", Event->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the evaluation functions directly
bool UTERTrackPearson::TestEvaluation()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  UTERTrackPearsonAccess Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  // A track of six hits which bends more and more, with increasing energies: both correlations count
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  MRETrack* Track = new MRETrack();
  double X[6] = { 0.0, 0.0, 0.25, 1.0, 2.25, 4.0 };
  double Energies[6] = { 50.0, 60.0, 70.0, 80.0, 90.0, 200.0 };
  vector<MVector> Positions;
  vector<double> EnergyList;
  bool AllCreated = true;
  for (int i = 0; i < 6; ++i) {
    MREHit* Hit = CreateHit(MVector(X[i], 0.0, 9.0 - i), Energies[i], MDDetector::c_Strip2D, &Geometry);
    if (Hit == nullptr) {
      AllCreated = false;
    } else {
      Track->AddRESE(Hit);
      Positions.push_back(Hit->GetPosition());
      EnergyList.push_back(Energies[i]);
    }
  }
  Passed = EvaluateTrue("CreateHit()", "six hits", "The hits are created", AllCreated) && Passed;
  Track->LinkSequential();
  Event->AddRESE(Track);

  if (AllCreated == true) {
    // (2 - rE - rA)/4 with the correlations of the energies and of the direction changes with the position along the track
    double Reference = 0.5*(1.0 - GetCorrelationScore(Positions, EnergyList));
    Passed = EvaluateTrue("EvaluateTrack()", "track of six hits", "The evaluation succeeds", Tracker.EvaluateTrack(Track)) && Passed;
    Passed = EvaluateNear("EvaluateTrack()", "track of six hits", "The quality factor is (2 - rE - rA)/4", Track->GetQualityFactor(), Reference, 1e-12) && Passed;
    Passed = EvaluateTrue("EvaluateTrack()", "track of six hits", "The quality factor is between 0 and 0.5 for a track with increasing energies", Track->GetQualityFactor() > 0.0 && Track->GetQualityFactor() < 0.5) && Passed;

    Track->SetQualityFactor(-5.0);
    Event->SetPairQualityFactor(1.0);
    Passed = EvaluateTrue("EvaluateTracks()", "one track", "The evaluation succeeds", Tracker.EvaluateTracks(Event)) && Passed;
    Passed = EvaluateNear("EvaluateTracks()", "one track", "The quality factor of the track is set", Track->GetQualityFactor(), Reference, 1e-12) && Passed;
    Passed = EvaluateNear("EvaluateTracks()", "one track", "The track quality factor of the event is the one of the track", Event->GetTrackQualityFactor(), Reference, 1e-12) && Passed;
    Passed = EvaluateNear("EvaluateTracks()", "one track", "The pair quality factor is reset", Event->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
  }
  delete Event;

  // An event without a track:
  MRERawEvent* Empty = new MRERawEvent(&Geometry);
  Empty->SetTrackQualityFactor(0.5);
  Passed = EvaluateTrue("EvaluateTracks()", "no track", "The evaluation succeeds", Tracker.EvaluateTracks(Empty)) && Passed;
  Passed = EvaluateNear("EvaluateTracks()", "no track", "An event without a track has no track quality factor", Empty->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
  delete Empty;

  // SortByTrackQualityFactor(): low values first
  MRawEventIncarnations List(&Geometry);
  double Factors[4] = { 0.5, 1.0, 0.0, 0.75 };
  for (unsigned int i = 0; i < 4; ++i) {
    MRERawEvent* RE = new MRERawEvent(&Geometry);
    RE->SetTrackQualityFactor(Factors[i]);
    List.AddRawEvent(RE);
  }
  Tracker.SortByTrackQualityFactor(&List);
  double Sorted[4] = { 0.0, 0.5, 0.75, 1.0 };
  for (int i = 0; i < 4; ++i) {
    Passed = EvaluateNear("SortByTrackQualityFactor()", MString("position ") + i, "The events are sorted by increasing track quality factor", List.GetRawEventAt(i)->GetTrackQualityFactor(), Sorted[i], 0.0) && Passed;
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

  UTERTrackPearson Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERTrackPearson.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
