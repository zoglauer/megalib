/*
 * UTERTrackGas.cxx
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
#include "MERTrackGas.h"
#include "UTERTrackShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the electron tracker MERTrackGas: the hits of a track are found by their distance, not by layers
class UTERTrackGas : public UTERTrackBase
{
public:
  //! Default constructor
  UTERTrackGas() : UTERTrackBase("UTERTrackGas") {}
  //! Default destructor
  virtual ~UTERTrackGas() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the special parameters and the option text
  bool TestOptions();
  //! Test a chain of hits with a hit close to it and a hit far away
  bool TestChain();
  //! Test the search radius with two hits
  bool TestSearchRadius();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERTrackGas::Run()
{
  bool Passed = true;

  Passed = TestOptions() && Passed;
  Passed = TestChain() && Passed;
  Passed = TestSearchRadius() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the special parameters and the option text
bool UTERTrackGas::TestOptions()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackGas Tracker;
  Passed = EvaluateTrue("SetSpecialParameters()", "no parameters", "The call succeeds", Tracker.SetSpecialParameters()) && Passed;

  SetupTracker(Tracker, Geometry, false, false, true, 2, 4, false);
  MString Core;
  Core += "# Search MIPs:                  0\n";
  Core += "# Search Pairs:                 0\n";
  Core += "# Search Comptons:              1\n";
  Core += "# Max. number of ambiguities:   5\n";
  Core += "# Max. layer jump Compton:      2\n";
  Core += "# Number of sequences to keep:  4\n";
  Core += "# Reject pure ambiguities:      0\n";
  Passed = Evaluate("ToString()", "core only", "The text lists the options", Tracker.ToString(true), Core) && Passed;
  Passed = Evaluate("ToString()", "full", "The text has a title and separators around the options", Tracker.ToString(false), MString("# Tracking - Gas options:\n# \n") + Core + "# \n") && Passed;
  Passed = Evaluate("ToString()", "default argument", "The default is the full text", Tracker.ToString(), Tracker.ToString(false)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a chain of hits with a hit close to it and a hit far away
bool UTERTrackGas::TestChain()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  // Only the Compton tracking is special for this class
  MERTrackGas Tracker;
  SetupTracker(Tracker, Geometry, false, false, true, 2, 2);

  // All hits are in the wafer of layer 0:
  // * A chain of seven hits at x = 0, 0.25, ..., 1.5 cm with 100, 110, ..., 160 keV (910 keV): neighbors are 0.25 cm apart, less than the search radius of 0.5 cm
  // * A hit of 30 keV at (0.75, 0.52) cm: more than 0.5 cm from each hit of the chain, but within the chain
  // * A hit of 70 keV at (3, 0) cm: far from the chain
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  for (int i = 0; i < 7; ++i) {
    AddTrackerHit(Event, Geometry, 0.25*i, 0.0, 0, 100.0 + 10.0*i);
  }
  AddTrackerHit(Event, Geometry, 0.75, 0.52, 0, 30.0);
  AddTrackerHit(Event, Geometry, 3.0, 0.0, 0, 70.0);
  AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);

  MRawEventIncarnations List(&Geometry);
  Passed = EvaluateTrue("Analyze()", "chain", "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
  // One track in its two directions:
  Passed = Evaluate("Analyze()", "chain", "There are two sequences", List.GetNRawEvents(), 2) && Passed;
  if (List.GetNRawEvents() == 2) {
    MRERawEvent* Best = List.GetRawEventAt(0);
    Passed = Evaluate("Analyze()", "chain", "The best event has one track", GetNTracks(Best), 1U) && Passed;
    // The hit within the chain is removed from the event: the track is grown from the hit closest to the center of gravity of all hits
    // by adding the closest hit within 0.5 cm of one of its ends, and then all hits which are closer to the center of gravity of the track
    // than its most distant hit are thrown out (as stated in TrackComptons()). The center of the chain is at x = 0.25*3010/910 = 0.827 cm,
    // its most distant hit 0.827 cm away, and the hit at (0.75, 0.52) cm is 0.526 cm away.
    Passed = Evaluate("Analyze()", "chain", "The far hit and the calorimeter hit remain, the hit within the chain is removed", GetNNonTracks(Best), 2U) && Passed;
    Passed = EvaluateNear("Analyze()", "chain", "The energy of the event is 910 + 70 + 500 = 1480 keV", Best->GetEnergy(), 1480.0, 1e-10) && Passed;
    MRETrack* Track = GetTrack(Best, 0);
    if (Track != nullptr) {
      Passed = Evaluate("Analyze()", "chain", "The track has the seven hits of the chain", Track->GetNRESEs(), 7) && Passed;
      Passed = EvaluateNear("Analyze()", "chain", "The track has the energy of the chain, 910 keV", Track->GetEnergy(), 910.0, 1e-10) && Passed;
      vector<MRESE*> Sequence = GetSequence(Track);
      Passed = EvaluateSize("Analyze()", "chain", "The links of the track lead through all seven hits", Sequence.size(), 7) && Passed;
      bool Ordered = true;
      for (unsigned int i = 0; i < Sequence.size(); ++i) {
        if (fabs(Sequence[i]->GetPosition().X() - 0.25*i) > 1e-12) {
          Ordered = false;
        }
      }
      // The energies increase with x, so the best direction starts at x = 0
      Passed = EvaluateTrue("Analyze()", "chain", "The track starts at x = 0 and follows the chain", Ordered) && Passed;
      // The energies increase linearly (correlation 1) and the chain is straight (correlation of the direction changes 0): (1 + 0)/2
      Passed = EvaluateNear("Analyze()", "chain", "The quality factor of the track is 0.5", Track->GetQualityFactor(), 0.5, 1e-12) && Passed;
    }
    // 2/(1 track + 1 remaining hit in the tracker) + 0.5
    Passed = EvaluateNear("Analyze()", "chain", "The track quality factor of the best event is 1.5", Best->GetTrackQualityFactor(), 1.5, 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "chain", "The track quality factor of the other direction is 2/2 - 0.5 = 0.5", List.GetRawEventAt(1)->GetTrackQualityFactor(), 0.5, 1e-12) && Passed;
    Passed = EvaluateTrue("Analyze()", "chain", "The best event is the best try", List.GetBestTryEvent() == Best) && Passed;
    Passed = EvaluateFalse("Analyze()", "chain", "There is no optimum event", List.HasOptimumEvent()) && Passed;
  }
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search radius with two hits
bool UTERTrackGas::TestSearchRadius()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTrackingGeometry()", "tracking geometry", "The geometry can be scanned", ScanTrackingGeometry(Geometry)) && Passed;

  MERTrackGas Tracker;
  SetupTracker(Tracker, Geometry, false, false, true, 2, 1);

  // Two hits in the wafer of layer 0, just inside and just outside of the search radius of 0.5 cm
  for (unsigned int Case = 0; Case <= 1; ++Case) {
    double Distance = 0.49;
    MString Input = "two hits, 0.49 cm apart";
    if (Case == 1) {
      Distance = 0.51;
      Input = "two hits, 0.51 cm apart";
    }
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 0, 50.0);
    AddTrackerHit(Event, Geometry, Distance, 0.0, 0, 60.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);

    MRawEventIncarnations List(&Geometry);
    Passed = EvaluateTrue("Analyze()", Input, "The analysis succeeds", AnalyzeEvent(Tracker, List, Event)) && Passed;
    Passed = Evaluate("Analyze()", Input, "One sequence is kept", List.GetNRawEvents(), 1) && Passed;
    if (List.GetNRawEvents() == 1) {
      MRERawEvent* RE = List.GetRawEventAt(0);
      Passed = EvaluateNear("Analyze()", Input, "The energy of the event is 610 keV", RE->GetEnergy(), 610.0, 1e-10) && Passed;
      if (Case == 0) {
        Passed = Evaluate("Analyze()", Input, "The two hits are a track", GetNTracks(RE), 1U) && Passed;
        Passed = Evaluate("Analyze()", Input, "The calorimeter hit remains", GetNNonTracks(RE), 1U) && Passed;
        // 2/1 + the correlation +1 of the two increasing energies
        Passed = EvaluateNear("Analyze()", Input, "The track quality factor is 3", RE->GetTrackQualityFactor(), 3.0, 1e-12) && Passed;
        MRETrack* Track = GetTrack(RE, 0);
        if (Track != nullptr && Track->GetStartPoint() != nullptr) {
          Passed = EvaluateNear("Analyze()", Input, "The track starts at the hit with the smaller energy", Track->GetStartPoint()->GetEnergy(), 50.0, 1e-12) && Passed;
        }
      } else {
        Passed = Evaluate("Analyze()", Input, "The two hits are no track", GetNTracks(RE), 0U) && Passed;
        Passed = Evaluate("Analyze()", Input, "The three hits remain", GetNNonTracks(RE), 3U) && Passed;
        // 2/(0 tracks + 2 remaining hits in the tracker)
        Passed = EvaluateNear("Analyze()", Input, "The track quality factor is 1", RE->GetTrackQualityFactor(), 1.0, 1e-12) && Passed;
      }
    }
    List.DeleteAll();
  }

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

  UTERTrackGas Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERTrackGas.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
