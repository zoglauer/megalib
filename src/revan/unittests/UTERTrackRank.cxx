/*
 * UTERTrackRank.cxx
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
#include "MERTrackRank.h"
#include "UTERTrackShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected members of MERTrackRank
class UTERTrackRankAccess : public MERTrackRank
{
public:
  //! Default constructor
  UTERTrackRankAccess() : MERTrackRank() {}
  //! Default destructor
  virtual ~UTERTrackRankAccess() {}

  using MERTrackRank::EvaluateTracks;
  using MERTrackRank::EvaluateTrack;
  using MERTrackRank::SortByTrackQualityFactor;

  using MERTrackRank::m_AllowOnlyMinNumberOfRESEsD1;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the electron tracker MERTrackRank: the sequences and directions of the tracks are rated with
//! the Spearman rank correlation scaled to 0 (good) to 1 (bad)
class UTERTrackRank : public UTERTrackBase
{
public:
  //! Default constructor
  UTERTrackRank() : UTERTrackBase("UTERTrackRank") {}
  //! Default destructor
  virtual ~UTERTrackRank() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the construction, the special parameters, and the option text
  bool TestConstructionAndOptions();
  //! Test a straight Compton electron track
  bool TestStraightTrack();
  //! Test tracks with and without the correlation of the direction changes
  bool TestEvaluateTrack();
  //! Test an event with two tracks, and events without a track
  bool TestTwoTracksAndNoTrack();
  //! Test a pair event
  bool TestPair();
  //! Test the sorting
  bool TestSorting();

  //! Return the Spearman rank correlation of the values with their index 1, 2, ..., N for values without ties: 1 - 6 sum(d^2)/(N (N^2 - 1)),
  //! where d is the difference between the rank of a value (1 for the smallest) and its index
  double GetRankCorrelation(const vector<double>& Values) const;
  //! Return a new track with the given x positions and energies of its hits, which are in the layers 9, 8, ... . The first hit is the start. Return nullptr if a hit cannot be created
  MRETrack* CreateTrack(MGeometryRevan& Geometry, const vector<double>& X, const vector<double>& Energies);
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERTrackRank::Run()
{
  bool Passed = true;

  Passed = TestConstructionAndOptions() && Passed;
  Passed = TestStraightTrack() && Passed;
  Passed = TestEvaluateTrack() && Passed;
  Passed = TestTwoTracksAndNoTrack() && Passed;
  Passed = TestPair() && Passed;
  Passed = TestSorting() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction, the special parameters, and the option text
bool UTERTrackRank::TestConstructionAndOptions()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  UTERTrackRankAccess Tracker;
  Passed = EvaluateTrue("MERTrackRank()", "new object", "Only the sequences with the smallest number of elements in the tracker are allowed", Tracker.m_AllowOnlyMinNumberOfRESEsD1) && Passed;
  Passed = EvaluateTrue("SetSpecialParameters()", "no parameters", "The call succeeds", Tracker.SetSpecialParameters()) && Passed;

  SetupTracker(Tracker, Geometry, true, false, true, 1, 3, false);
  MString Core;
  Core += "# Search MIPs:                  1\n";
  Core += "# Search Pairs:                 0\n";
  Core += "# Search Comptons:              1\n";
  Core += "# Max. number of ambiguities:   5\n";
  Core += "# Max. layer jump Compton:      1\n";
  Core += "# Number of sequences to keep:  3\n";
  Core += "# Reject pure ambiguities:      0\n";
  Passed = Evaluate("ToString()", "core only", "The text lists the options", Tracker.ToString(true), Core) && Passed;
  Passed = Evaluate("ToString()", "full", "The text has a title and separators around the options", Tracker.ToString(false), MString("# Tracking - Rank options:\n# \n") + Core + "# \n") && Passed;
  Passed = Evaluate("ToString()", "default argument", "The default is the full text", Tracker.ToString(), Tracker.ToString(false)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a straight Compton electron track
bool UTERTrackRank::TestStraightTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // The energies (50, 60, 200, 80) from layer 3 to 0 have the ranks (1, 2, 4, 3): the sum of the squared rank differences is 2,
  // thus rho = 1 - 6*2/(4*15) = 0.8 and the quality factor (1 - rho)/2 = 0.1.
  // From layer 0 to 3: (80, 200, 60, 50) has the ranks (3, 4, 2, 1), the sum is 4 + 4 + 1 + 9 = 18, rho = -0.8, and the quality factor 0.9
  for (unsigned int Keep = 1; Keep <= 5; Keep += 4) {
    MERTrackRank Tracker;
    SetupTracker(Tracker, Geometry, false, true, true, 2, Keep);

    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
    AddTrackerHit(Event, Geometry, 2.0, 0.0, 1, 200.0);
    AddTrackerHit(Event, Geometry, 3.0, 0.0, 0, 80.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);

    MString Input = MString("straight track, keep ") + Keep;
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    if (Keep == 1) {
      Passed = Evaluate("Analyze()", Input, "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
    } else {
      Passed = Evaluate("Analyze()", Input, "Both directions are kept", List.GetNRawEvents(), 2) && Passed;
    }
    if (List.GetNRawEvents() >= 1) {
      MRERawEvent* Best = List.GetRawEventAt(0);
      Passed = Evaluate("Analyze()", Input, "The best event has one track", GetNTracks(Best), 1U) && Passed;
      Passed = Evaluate("Analyze()", Input, "The calorimeter hit remains", GetNNonTracks(Best), 1U) && Passed;
      Passed = EvaluateTrue("Analyze()", Input, "The best event is the best try", List.GetBestTryEvent() == Best) && Passed;
      Passed = EvaluateFalse("Analyze()", Input, "There is no optimum event", List.HasOptimumEvent()) && Passed;
      MRETrack* Track = GetTrack(Best, 0);
      if (Track != nullptr) {
        Passed = Evaluate("Analyze()", Input, "The best track starts in layer 3", GetLayerSequence(Track), MString("3 2 1 0")) && Passed;
        Passed = EvaluateNear("Analyze()", Input, "The quality factor of the best track is 0.1", Track->GetQualityFactor(), 0.1, 1e-12) && Passed;
      }
      Passed = EvaluateNear("Analyze()", Input, "The track quality factor of the best event is 0.1", Best->GetTrackQualityFactor(), 0.1, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", Input, "The pair quality factor is not set", Best->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
    }
    if (List.GetNRawEvents() == 2) {
      MRETrack* Track = GetTrack(List.GetRawEventAt(1), 0);
      Passed = EvaluateTrue("Analyze()", Input, "The second event has a track", Track != nullptr) && Passed;
      if (Track != nullptr) {
        Passed = Evaluate("Analyze()", Input, "The second track starts in layer 0", GetLayerSequence(Track), MString("0 1 2 3")) && Passed;
      }
      Passed = EvaluateNear("Analyze()", Input, "The track quality factor of the second event is 0.9", List.GetRawEventAt(1)->GetTrackQualityFactor(), 0.9, 1e-12) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test tracks with and without the correlation of the direction changes
bool UTERTrackRank::TestEvaluateTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  UTERTrackRankAccess Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  // A track through the layers 9 to 4 which bends at each layer. The slopes dx/dz of its five segments are 0, 0.25, 0.75, 1.25, 1.75,
  // i.e. the angles to the z axis are 0, 14.036, 36.870, 51.340, 60.255 degrees, and the direction changes at the four inner hits are
  // 14.036, 22.834, 14.470, 8.915 degrees with the ranks (2, 4, 3, 1): the sum of the squared rank differences is 1 + 4 + 0 + 9 = 14, rho = 1 - 6*14/(4*15) = -0.4
  vector<double> X = { 0.0, 0.0, 0.25, 1.0, 2.25, 4.0 };
  vector<double> Increasing = { 50.0, 60.0, 70.0, 80.0, 90.0, 200.0 };

  // Six hits, increasing energies (rho = 1): (2 - 1 - (-0.4))/4 = 0.35
  {
    MRETrack* Track = CreateTrack(Geometry, X, Increasing);
    Passed = EvaluateTrue("CreateTrack()", "six hits", "The track is created", Track != nullptr) && Passed;
    if (Track != nullptr) {
      Passed = EvaluateTrue("EvaluateTrack()", "six hits", "The evaluation succeeds", Tracker.EvaluateTrack(Track)) && Passed;
      Passed = EvaluateNear("EvaluateTrack()", "six hits", "The quality factor is (2 - rhoE - rhoA)/4 = 0.35", Track->GetQualityFactor(), 0.35, 1e-12) && Passed;
      delete Track;
    }
  }

  // The same track in the other direction: the energies decrease (rho = -1), and the direction changes
  // (8.915, 14.470, 22.834, 14.036) have the ranks (1, 3, 4, 2) with the sum 0 + 1 + 1 + 4 = 6, rho = 0.4: (2 + 1 - 0.4)/4 = 0.65
  {
    vector<double> ReverseX = { 4.0, 2.25, 1.0, 0.25, 0.0, 0.0 };
    vector<double> Decreasing = { 200.0, 90.0, 80.0, 70.0, 60.0, 50.0 };
    MRETrack* Track = CreateTrack(Geometry, ReverseX, Decreasing);
    Passed = EvaluateTrue("CreateTrack()", "six hits, other direction", "The track is created", Track != nullptr) && Passed;
    if (Track != nullptr) {
      Passed = EvaluateTrue("EvaluateTrack()", "six hits, other direction", "The evaluation succeeds", Tracker.EvaluateTrack(Track)) && Passed;
      Passed = EvaluateNear("EvaluateTrack()", "six hits, other direction", "The quality factor is (2 - rhoE - rhoA)/4 = 0.65", Track->GetQualityFactor(), 0.65, 1e-12) && Passed;
      delete Track;
    }
  }

  // Five hits: the direction changes are only used from six hits on, so only the energies count
  {
    vector<double> FiveX = { 0.0, 0.0, 0.25, 1.0, 2.25 };
    vector<double> Mixed = { 60.0, 50.0, 80.0, 70.0, 200.0 };
    MRETrack* Track = CreateTrack(Geometry, FiveX, Mixed);
    Passed = EvaluateTrue("CreateTrack()", "five hits", "The track is created", Track != nullptr) && Passed;
    if (Track != nullptr) {
      // Ranks (2, 1, 4, 3, 5): the sum is 1 + 1 + 1 + 1 + 0 = 4, rho = 1 - 24/120 = 0.8, thus (1 - 0.8)/2 = 0.1
      Passed = EvaluateNear("GetRankCorrelation()", "five hits", "The reference of the rank correlation is 0.8", GetRankCorrelation(Mixed), 0.8, 1e-12) && Passed;
      Passed = EvaluateTrue("EvaluateTrack()", "five hits", "The evaluation succeeds", Tracker.EvaluateTrack(Track)) && Passed;
      Passed = EvaluateNear("EvaluateTrack()", "five hits", "The quality factor is (1 - rhoE)/2 = 0.1", Track->GetQualityFactor(), 0.1, 1e-12) && Passed;
      delete Track;
    }
  }

  // Two hits: increasing energies are perfect, decreasing ones the worst
  {
    vector<double> TwoX = { 0.0, 1.0 };
    vector<double> Up = { 50.0, 60.0 };
    vector<double> Down = { 60.0, 50.0 };
    MRETrack* TrackUp = CreateTrack(Geometry, TwoX, Up);
    MRETrack* TrackDown = CreateTrack(Geometry, TwoX, Down);
    Passed = EvaluateTrue("CreateTrack()", "two hits", "The tracks are created", TrackUp != nullptr && TrackDown != nullptr) && Passed;
    if (TrackUp != nullptr && TrackDown != nullptr) {
      Tracker.EvaluateTrack(TrackUp);
      Tracker.EvaluateTrack(TrackDown);
      Passed = EvaluateNear("EvaluateTrack()", "two hits, increasing energies", "The quality factor is 0", TrackUp->GetQualityFactor(), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("EvaluateTrack()", "two hits, decreasing energies", "The quality factor is 1", TrackDown->GetQualityFactor(), 1.0, 1e-12) && Passed;
    }
    delete TrackUp;
    delete TrackDown;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an event with two tracks, and events without a track
bool UTERTrackRank::TestTwoTracksAndNoTrack()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackRank Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 10);

  // Track A in layers 3 and 2 (50 and 60 keV), track B in layers 9, 8, 7 (80, 60, 70 keV). They are too far apart to be linked.
  // A from layer 3: rho = +1, q = 0; from layer 2: rho = -1, q = 1
  // B from layer 7: (70, 60, 80) has the ranks (2, 1, 3), the sum is 2, rho = 1 - 12/24 = 0.5, q = 0.25; from layer 9: ranks (3, 1, 2), the sum is 6, rho = -0.5, q = 0.75
  // The event has 1 - (1 - qA)(1 - qB): 0.25, 0.75, 1, 1
  {
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
    }
    List.DeleteAll();
  }

  // Two hits in the same layer are no track:
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddTrackerHit(Event, Geometry, 2.0, 0.0, 3, 60.0);
    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", "two hits in the same layer", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", "two hits in the same layer", "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      Passed = Evaluate("Analyze()", "two hits in the same layer", "The event has no track", GetNTracks(List.GetRawEventAt(0)), 0U) && Passed;
      Passed = EvaluateNear("Analyze()", "two hits in the same layer", "An event without a track has no track quality factor", List.GetRawEventAt(0)->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
      Passed = EvaluateNear("Analyze()", "two hits in the same layer", "An event without a track has no pair quality factor", List.GetRawEventAt(0)->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a pair event
bool UTERTrackRank::TestPair()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackRank Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "pair", "The analysis succeeds", AnalyzeEvent(Tracker, List, CreatePairEvent(Geometry, 9))) && Passed;
  Passed = Evaluate("Analyze()", "pair", "There is one event", List.GetNRawEvents(), 1) && Passed;
  if (List.GetNRawEvents() == 1) {
    MRERawEvent* Event = List.GetRawEventAt(0);
    Passed = Evaluate("Analyze()", "pair", "The event is a pair event", Event->GetEventType(), MRERawEvent::c_PairEvent) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event is good", Event->IsGoodEvent()) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;
    Passed = EvaluateTrue("Analyze()", "pair", "The event has a vertex", Event->GetVertex() != nullptr) && Passed;
    Passed = Evaluate("Analyze()", "pair", "The event has two tracks", GetNTracks(Event), 2U) && Passed;
    if (GetNTracks(Event) == 2) {
      MRETrack* First = GetTrack(Event, 0);
      MRETrack* Second = GetTrack(Event, 1);
      Passed = Evaluate("Analyze()", "pair", "The first track follows all layers from the vertex down", GetLayerSequence(First), MString("9 8 7 6 5 4 3 2 1 0")) && Passed;
      Passed = Evaluate("Analyze()", "pair", "The second track follows all layers from the vertex down", GetLayerSequence(Second), MString("9 8 7 6 5 4 3 2 1 0")) && Passed;
      Passed = EvaluateTrue("Analyze()", "pair", "The quality factor of the first track is between 0 and 1", First->GetQualityFactor() >= 0.0 && First->GetQualityFactor() <= 1.0) && Passed;
      Passed = EvaluateTrue("Analyze()", "pair", "The quality factor of the second track is between 0 and 1", Second->GetQualityFactor() >= 0.0 && Second->GetQualityFactor() <= 1.0) && Passed;
      // The pair quality factor is 1 - (1 - (1 - q1)(1 - q2)): 1 for two perfect tracks.
      // The values of the tracks are not checked: all direction changes of the straight legs are equal, and ties have no defined rank here
      Passed = EvaluateNear("Analyze()", "pair", "The pair quality factor is (1 - q1)(1 - q2)", Event->GetPairQualityFactor(), (1.0 - First->GetQualityFactor())*(1.0 - Second->GetQualityFactor()), 1e-12) && Passed;
    }
    Passed = EvaluateNear("Analyze()", "pair", "The track quality factor is not set", Event->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the sorting
bool UTERTrackRank::TestSorting()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  UTERTrackRankAccess Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  // Low values first:
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


//! Return the Spearman rank correlation of the values with their index 1, 2, ..., N for values without ties: 1 - 6 sum(d^2)/(N (N^2 - 1)),
//! where d is the difference between the rank of a value (1 for the smallest) and its index
double UTERTrackRank::GetRankCorrelation(const vector<double>& Values) const
{
  double N = Values.size();
  double Sum = 0.0;
  for (unsigned int i = 0; i < Values.size(); ++i) {
    // The rank is one plus the number of smaller values
    double Rank = 1.0;
    for (unsigned int j = 0; j < Values.size(); ++j) {
      if (Values[j] < Values[i]) {
        Rank += 1.0;
      }
    }
    Sum += (Rank - (i + 1))*(Rank - (i + 1));
  }

  return 1.0 - 6.0*Sum/(N*(N*N - 1.0));
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new track with the given x positions and energies of its hits, which are in the layers 9, 8, ... . The first hit is the start. Return nullptr if a hit cannot be created
MRETrack* UTERTrackRank::CreateTrack(MGeometryRevan& Geometry, const vector<double>& X, const vector<double>& Energies)
{
  MRETrack* Track = new MRETrack();
  for (unsigned int i = 0; i < X.size(); ++i) {
    MREHit* Hit = CreateHit(MVector(X[i], 0.0, 9.0 - i), Energies[i], MDDetector::c_Strip2D, &Geometry);
    if (Hit == nullptr) {
      delete Track;
      return nullptr;
    }
    Track->AddRESE(Hit);
  }
  Track->LinkSequential();

  return Track;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTERTrackRank Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERTrackRank.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
