/*
 * UTERTrackDirectional.cxx
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
#include "MDVolumeSequence.h"
#include "MERTrackDirectional.h"
#include "MREAMDirectional.h"
#include "UTERTrackShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the electron tracker MERTrackDirectional: a hit with a measured direction of the electron
//! (as from a Strip3DDirectional detector) becomes a track with this direction
class UTERTrackDirectional : public UTERTrackBase
{
public:
  //! Default constructor
  UTERTrackDirectional() : UTERTrackBase("UTERTrackDirectional") {}
  //! Default destructor
  virtual ~UTERTrackDirectional() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the special parameters and the option text
  bool TestOptions();
  //! Test events with two hits in the tracker, with and without measured directions
  bool TestTwoHits();
  //! Test an event with one hit in the tracker which has a measured direction
  bool TestOneHit();
  //! Test a pair event
  bool TestPair();

  //! Add the measured direction of the electron of the hit to the event, as the noising does it
  void AddDirection(MRERawEvent* Event, MRESE* Hit, const MVector& Direction);
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERTrackDirectional::Run()
{
  bool Passed = true;

  Passed = TestOptions() && Passed;
  Passed = TestTwoHits() && Passed;
  Passed = TestOneHit() && Passed;
  Passed = TestPair() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the special parameters and the option text
bool UTERTrackDirectional::TestOptions()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackDirectional Tracker;
  Passed = EvaluateTrue("SetSpecialParameters()", "no parameters", "The call succeeds", Tracker.SetSpecialParameters()) && Passed;

  SetupTracker(Tracker, Geometry, true, true, false, 2, 1, true);
  MString Core;
  Core += "# Search MIPs:                  1\n";
  Core += "# Search Pairs:                 1\n";
  Core += "# Search Comptons:              0\n";
  Core += "# Max. number of ambiguities:   5\n";
  Core += "# Max. layer jump Compton:      2\n";
  Core += "# Number of sequences to keep:  1\n";
  Core += "# Reject pure ambiguities:      1\n";
  Passed = Evaluate("ToString()", "core only", "The text lists the options", Tracker.ToString(true), Core) && Passed;
  Passed = Evaluate("ToString()", "full", "The text has a title and separators around the options", Tracker.ToString(false), MString("# Tracking - Directional options:\n# \n") + Core + "# \n") && Passed;
  Passed = Evaluate("ToString()", "default argument", "The default is the full text", Tracker.ToString(), Tracker.ToString(false)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events with two hits in the tracker, with and without measured directions
bool UTERTrackDirectional::TestTwoHits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackDirectional Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 10);

  // Hits in layer 3 and 1 and in the calorimeter; 0, 1, or 2 of the tracker hits have a direction
  for (unsigned int NDirections = 0; NDirections <= 2; ++NDirections) {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    MRESE* First = AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    MRESE* Second = AddTrackerHit(Event, Geometry, 2.0, 0.0, 1, 60.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
    MString Input = MString("two tracker hits, directions: ") + NDirections;
    Passed = EvaluateTrue("AddTrackerHit()", Input, "The hits are created", First != nullptr && Second != nullptr) && Passed;
    if (First == nullptr || Second == nullptr) {
      delete Event;
      continue;
    }
    if (NDirections >= 1) {
      AddDirection(Event, First, MVector(0.0, 0.6, -0.8));
    }
    if (NDirections >= 2) {
      AddDirection(Event, Second, MVector(1.0, 0.0, 0.0));
    }

    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    // The direction is known, so there is only one sequence
    Passed = Evaluate("Analyze()", Input, "There is one event", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      MRERawEvent* RE = List.GetRawEventAt(0);
      Passed = Evaluate("Analyze()", Input, "Each hit with a direction is a track", GetNTracks(RE), NDirections) && Passed;
      Passed = Evaluate("Analyze()", Input, "The other hits remain", GetNNonTracks(RE), 3U - NDirections) && Passed;
      Passed = EvaluateNear("Analyze()", Input, "The energy of the event is 610 keV", RE->GetEnergy(), 610.0, 1e-10) && Passed;
      // The measured directions are taken as perfect:
      Passed = EvaluateNear("Analyze()", Input, "The track quality factor is 0", RE->GetTrackQualityFactor(), 0.0, 0.0) && Passed;
      Passed = EvaluateNear("Analyze()", Input, "The pair quality factor is not set", RE->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor, 0.0) && Passed;
      Passed = EvaluateTrue("Analyze()", Input, "The event is the best try", List.GetBestTryEvent() == RE) && Passed;
      Passed = EvaluateFalse("Analyze()", Input, "There is no optimum event", List.HasOptimumEvent()) && Passed;

      for (unsigned int t = 0; t < GetNTracks(RE); ++t) {
        MRETrack* Track = GetTrack(RE, t);
        MVector Direction(0.0, 0.6, -0.8);
        MVector Position(0.0, 0.0, 3.0);
        double Energy = 50.0;
        if (Track->GetEnergy() > 55.0) {
          Direction = MVector(1.0, 0.0, 0.0);
          Position = MVector(2.0, 0.0, 1.0);
          Energy = 60.0;
        }
        MString TrackInput = Input + ", track " + t;
        Passed = Evaluate("Analyze()", TrackInput, "The track has one hit", Track->GetNRESEs(), 1) && Passed;
        Passed = EvaluateNear("Analyze()", TrackInput, "The track has the energy of its hit", Track->GetEnergy(), Energy, 1e-12) && Passed;
        Passed = EvaluateTrue("Analyze()", TrackInput, "The direction of the track is fixed", Track->IsDirectionFixed()) && Passed;
        Passed = EvaluateVectorNear("Analyze()", TrackInput, "The direction of the track is the measured one", Track->GetDirection(), Direction, 1e-12) && Passed;
        Passed = EvaluateTrue("Analyze()", TrackInput, "The track has a start point", Track->GetStartPoint() != nullptr) && Passed;
        if (Track->GetStartPoint() != nullptr) {
          Passed = EvaluateVectorNear("Analyze()", TrackInput, "The track starts at its hit", Track->GetStartPoint()->GetPosition(), Position, 1e-12) && Passed;
        }
      }
    }
    List.DeleteAll();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an event with one hit in the tracker which has a measured direction
bool UTERTrackDirectional::TestOneHit()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackDirectional Tracker;
  SetupTracker(Tracker, Geometry, false, true, true, 2, 10);

  // The typical Compton event of a telescope with a directional tracker: one hit with a direction in the tracker, one hit in the calorimeter
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  MRESE* Hit = AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
  AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);
  Passed = EvaluateTrue("AddTrackerHit()", "one tracker hit", "The hit is created", Hit != nullptr) && Passed;
  if (Hit == nullptr) {
    delete Event;
    return Passed;
  }
  AddDirection(Event, Hit, MVector(0.0, 0.6, -0.8));

  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "one tracker hit with direction", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
  Passed = Evaluate("Analyze()", "one tracker hit with direction", "There is one event", List.GetNRawEvents(), 1) && Passed;
  if (List.GetNRawEvents() == 1) {
    // Defect REV-F-6: MERTrack::Analyze() only starts the tracking for at least two hits in the tracker, so the measured direction of a single hit is never used
    Passed = ExpectedToFail(Evaluate("Analyze()", "one tracker hit with direction", "The hit with the direction is a track", GetNTracks(List.GetRawEventAt(0)), 1U)) && Passed;
    Passed = EvaluateNear("Analyze()", "one tracker hit with direction", "The energy of the event is 550 keV", List.GetRawEventAt(0)->GetEnergy(), 550.0, 1e-10) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a pair event
bool UTERTrackDirectional::TestPair()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackDirectional Tracker;
  SetupTracker(Tracker, Geometry, false, true, true);

  // The pair search is the one of the base class:
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


//! Add the measured direction of the electron of the hit to the event, as the noising does it
void UTERTrackDirectional::AddDirection(MRERawEvent* Event, MRESE* Hit, const MVector& Direction)
{
  // The event owns the additional measurement, and the additional measurement its volume sequence
  MREAMDirectional* Measurement = new MREAMDirectional();
  Measurement->SetVolumeSequence(new MDVolumeSequence(*(Hit->GetVolumeSequence())));
  Measurement->SetRESE(Hit);
  Measurement->SetDirection(Direction);
  Measurement->SetEnergy(Hit->GetEnergy());
  Event->AddREAM(Measurement);
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTERTrackDirectional Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERTrackDirectional.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
