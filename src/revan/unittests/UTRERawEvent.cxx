/*
 * UTRERawEvent.cxx
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
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TRandom.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MDDetector.h"
#include "MExceptions.h"
#include "MGeometryRevan.h"
#include "MGlobal.h"
#include "MPairEvent.h"
#include "MPhotoEvent.h"
#include "MPhysicalEvent.h"
#include "MREAM.h"
#include "MREAMDirectional.h"
#include "MREAMDriftChamberEnergy.h"
#include "MREAMGuardRingHit.h"
#include "MREAMStartInformation.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MRESE.h"
#include "MREStripHit.h"
#include "MRETrack.h"
#include "MString.h"
#include "MTime.h"
#include "MUnidentifiableEvent.h"
#include "MVector.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the raw event MRERawEvent
class UTRERawEvent : public UTRevanBase
{
public:
  //! Default constructor
  UTRERawEvent() : UTRevanBase("UTRERawEvent") {}
  //! Default destructor
  virtual ~UTRERawEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  // The state of the event:

  //! Test the constants
  bool TestConstants();
  //! Test the state after the construction
  bool TestDefaults();
  //! Test the simple setters and getters
  bool TestGettersSetters();
  //! Test the event types
  bool TestEventTypes();
  //! Test the rejection reasons
  bool TestRejectionReasons();
  //! Test the flags of a bad event
  bool TestBadFlags();
  //! Test the quality factors
  bool TestQualityFactors();
  //! Test the energy, the track length, and the sequence length
  bool TestEnergyAndLengths();
  //! Test the validity of the event
  bool TestIsValid();
  //! Test deleting and shuffling the RESEs
  bool TestDeleteAllAndShuffle();

  // Reading lines of an evta or sim file:

  //! Test parsing the lines with the data of the event: ID, times, quality, flags, comments
  bool TestParseLineEventData();
  //! Test parsing the lines with the orientations
  bool TestParseLineOrientations();
  //! Test parsing hits without a geometry
  bool TestParseLineHits();
  //! Test the limit of the number of hits
  bool TestParseLineHitLimit();
  //! Test parsing hits with a geometry, which gives the resolutions
  bool TestParseLineHitsWithGeometry();
  //! Test parsing the additional measurements: guard ring, drift chamber, direction, start information
  bool TestParseLineMeasurements();

  // The additional measurements:

  //! Test the container of the additional measurements
  bool TestMeasurements();

  // Tracks and sequences:

  //! Test the iteration along the links
  bool TestIterator();
  //! Test the creation of tracks from linked hits
  bool TestCreateTracks();
  //! Test the kinematics check of the electron direction
  bool TestTestElectronDirection();

  // The physical event:

  //! Test the physical event of a single hit
  bool TestPhysicalEventPhoto();
  //! Test the physical event of a Compton sequence of hits
  bool TestPhysicalEventCompton();
  //! Test the physical event of a Compton sequence which starts with a track
  bool TestPhysicalEventComptonWithTrack();
  //! Test the physical event of a pair
  bool TestPhysicalEventPair();
  //! Test the physical event of an event which was not identified
  bool TestPhysicalEventUnidentifiable();
  //! Test setting the physical event from outside
  bool TestSetPhysicalEvent();
  //! Test the physical events which cannot be made
  bool TestPhysicalEventWithoutInput();

  // Copies and text:

  //! Test the copy constructor and the duplication
  bool TestCopies();
  //! Test the text output
  bool TestToString();
  //! Test the compact text
  bool TestToCompactString();
  //! Test the text for the evta file
  bool TestToEvtaString();

  // Helpers:

  //! Return a new hit in a strip detector with the given position (cm), energy and its resolution (keV), and time (s)
  //! The position resolution is (0.1, 0.2, 0.3) cm and the time resolution 1 ns. No geometry is used. The caller owns the hit
  MREHit* CreatePlainHit(const MVector& Position, double Energy, double EnergyResolution, double Time = 0.0);
  //! Link two RESEs with each other
  void Link(MRESE* A, MRESE* B);
  //! Fill the event with a Compton sequence of two hits: 200 +- 2 keV at the origin at 0 s, then 300 +- 4 keV at (3, 0, -4) cm at 1 ns
  void FillComptonEvent(MRERawEvent& Event);
  //! Return the lines which add a guard ring to the tracker (trigger threshold 40 keV, energy resolution 5 keV), a drift chamber at (-30, 0, 0) cm
  //! with a light energy resolution of 3 keV, and a directional strip detector at (0, 30, 0) cm to the test telescope
  MString GetAdditionalDetectors(bool WithLightEnergyResolution = true) const;
  //! Parse a line with all output silenced: the MEGAlib streams and cout, which the search for the volume uses
  int ParseSilently(MRERawEvent& Event, const MString& Line, int Version);

  //! Return true if a guard ring line is rejected with 1 if the event has no geometry
  bool IsGuardRingLineWithoutGeometryRejected();
  //! Return true if the drift chamber energy is read if the chamber has an ideal light energy resolution
  bool IsDriftChamberEnergyWithIdealResolutionRead();
  //! Return true if the iterator is not started without a start point
  bool IsIteratorWithoutStartPointRejected();
  //! Return true if the iterator is not started at a RESE with two links
  bool IsIteratorAtMiddleRejected();
  //! Return true if a Compton event without start point gives no Compton event
  bool IsComptonWithoutStartPointHandled();
  //! Return true if a pair event without vertex gives no pair event
  bool IsPairWithoutVertexHandled();
  //! Return true if a photo event with two hits gives no photo event
  bool IsPhotoWithTwoHitsHandled();
  //! Return true if the compact text of a good event without start point is empty
  bool IsCompactStringWithoutStartPointEmpty();

  //! The version of a sim file, with hits "HTsim detector;x;y;z;energy;time;origins"
  static const int c_SimVersion = 25;
  //! The version of an evta file, with hits "HT detector;x;y;z;energy;dx;dy;dz;denergy"
  static const int c_EvtaVersion = 21;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRERawEvent::Run()
{
  bool Passed = true;

  // Shuffle uses the random number generator
  gRandom->SetSeed(1);

  Passed = TestConstants() && Passed;
  Passed = TestDefaults() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestEventTypes() && Passed;
  Passed = TestRejectionReasons() && Passed;
  Passed = TestBadFlags() && Passed;
  Passed = TestQualityFactors() && Passed;
  Passed = TestEnergyAndLengths() && Passed;
  Passed = TestIsValid() && Passed;
  Passed = TestDeleteAllAndShuffle() && Passed;

  Passed = TestParseLineEventData() && Passed;
  Passed = TestParseLineOrientations() && Passed;
  Passed = TestParseLineHits() && Passed;
  Passed = TestParseLineHitLimit() && Passed;
  Passed = TestParseLineHitsWithGeometry() && Passed;
  Passed = TestParseLineMeasurements() && Passed;

  Passed = TestMeasurements() && Passed;

  Passed = TestIterator() && Passed;
  Passed = TestCreateTracks() && Passed;
  Passed = TestTestElectronDirection() && Passed;

  Passed = TestPhysicalEventPhoto() && Passed;
  Passed = TestPhysicalEventCompton() && Passed;
  Passed = TestPhysicalEventComptonWithTrack() && Passed;
  Passed = TestPhysicalEventPair() && Passed;
  Passed = TestPhysicalEventUnidentifiable() && Passed;
  Passed = TestSetPhysicalEvent() && Passed;
  Passed = TestPhysicalEventWithoutInput() && Passed;

  Passed = TestCopies() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestToCompactString() && Passed;
  Passed = TestToEvtaString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constants
bool UTRERawEvent::TestConstants()
{
  bool Passed = true;

  // The event types are the ones of the physical events:
  Passed = Evaluate("c_UnknownEvent", "constant", "The unknown event is the unknown physical event", MRERawEvent::c_UnknownEvent, MPhysicalEvent::c_Unknown) && Passed;
  Passed = Evaluate("c_ComptonEvent", "constant", "The Compton event is the Compton physical event", MRERawEvent::c_ComptonEvent, MPhysicalEvent::c_Compton) && Passed;
  Passed = Evaluate("c_PairEvent", "constant", "The pair event is the pair physical event", MRERawEvent::c_PairEvent, MPhysicalEvent::c_Pair) && Passed;
  Passed = Evaluate("c_MipEvent", "constant", "The MIP event is the muon physical event", MRERawEvent::c_MipEvent, MPhysicalEvent::c_Muon) && Passed;
  Passed = Evaluate("c_ShowerEvent", "constant", "The shower event is the shower physical event", MRERawEvent::c_ShowerEvent, MPhysicalEvent::c_Shower) && Passed;
  Passed = Evaluate("c_PhotoEvent", "constant", "The photo event is the photo physical event", MRERawEvent::c_PhotoEvent, MPhysicalEvent::c_Photo) && Passed;

  // The rejection reasons are numbered without gaps from 0 to 38:
  Passed = Evaluate("c_RejectionNone", "constant", "No rejection is 0", MRERawEvent::c_RejectionNone, 0) && Passed;
  Passed = Evaluate("c_RejectionStripPairingNotResolvable", "constant", "The last rejection reason is 38", MRERawEvent::c_RejectionStripPairingNotResolvable, 38) && Passed;

  // The value for no quality factor is a third of the largest double:
  Passed = Evaluate("c_NoQualityFactor", "constant", "No quality factor is a third of the largest double", MRERawEvent::c_NoQualityFactor, numeric_limits<double>::max()/3) && Passed;
  Passed = Evaluate("c_NoScore", "constant", "No score is the same as no quality factor", MRERawEvent::c_NoScore, MRERawEvent::c_NoQualityFactor) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the construction
bool UTRERawEvent::TestDefaults()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  vector<MString> Names = { "default constructor", "constructor with geometry" };
  for (unsigned int c = 0; c < Names.size(); ++c) {
    MRERawEvent* Event = nullptr;
    if (c == 0) {
      Event = new MRERawEvent();
      Passed = EvaluateTrue("GetGeometry()", Names[c], "There is no geometry", Event->GetGeometry() == nullptr) && Passed;
    } else {
      Event = new MRERawEvent(&Geometry);
      Passed = EvaluateTrue("GetGeometry()", Names[c], "The geometry is the given one", Event->GetGeometry() == &Geometry) && Passed;
    }
    Passed = Evaluate("GetType()", Names[c], "The RESE is an event", Event->GetType(), MRESE::c_Event) && Passed;
    Passed = Evaluate("GetEventID()", Names[c], "The event ID is 0", Event->GetEventID(), 0UL) && Passed;
    Passed = Evaluate("GetEventType()", Names[c], "The event type is unknown", Event->GetEventType(), MRERawEvent::c_UnknownEvent) && Passed;
    Passed = EvaluateNear("GetEventTime()", Names[c], "The event time is zero", Event->GetEventTime().GetAsSeconds(), 0.0, 1e-15) && Passed;
    Passed = EvaluateNear("GetEventClock()", Names[c], "The event clock is zero", Event->GetEventClock().GetAsSeconds(), 0.0, 1e-15) && Passed;
    Passed = EvaluateNear("GetCoincidenceWindow()", Names[c], "The coincidence window is zero", Event->GetCoincidenceWindow().GetAsSeconds(), 0.0, 1e-15) && Passed;
    Passed = EvaluateFalse("IsGoodEvent()", Names[c], "The event is not good", Event->IsGoodEvent()) && Passed;
    Passed = EvaluateTrue("IsValid()", Names[c], "The event is valid", Event->IsValid()) && Passed;
    Passed = EvaluateFalse("IsDecay()", Names[c], "The event is no decay", Event->IsDecay()) && Passed;
    Passed = EvaluateFalse("IsBad()", Names[c], "The event is not bad", Event->IsBad()) && Passed;
    Passed = Evaluate("GetNBadFlags()", Names[c], "There are no bad flags", Event->GetNBadFlags(), 0U) && Passed;
    Passed = Evaluate("GetRejectionReason()", Names[c], "There is no rejection reason", Event->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
    Passed = Evaluate("GetClusteringQualityFactor()", Names[c], "There is no clustering quality factor", Event->GetClusteringQualityFactor(), MRERawEvent::c_NoQualityFactor) && Passed;
    Passed = Evaluate("GetComptonQualityFactor1()", Names[c], "There is no first Compton quality factor", Event->GetComptonQualityFactor1(), MRERawEvent::c_NoQualityFactor) && Passed;
    Passed = Evaluate("GetComptonQualityFactor2()", Names[c], "There is no second Compton quality factor", Event->GetComptonQualityFactor2(), MRERawEvent::c_NoQualityFactor) && Passed;
    Passed = Evaluate("GetTrackQualityFactor()", Names[c], "There is no track quality factor", Event->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor) && Passed;
    Passed = Evaluate("GetPairQualityFactor()", Names[c], "There is no pair quality factor", Event->GetPairQualityFactor(), MRERawEvent::c_NoQualityFactor) && Passed;
    Passed = EvaluateTrue("GetStartPoint()", Names[c], "There is no start point", Event->GetStartPoint() == nullptr) && Passed;
    Passed = EvaluateTrue("GetVertex()", Names[c], "There is no vertex", Event->GetVertex() == nullptr) && Passed;
    Passed = Evaluate("GetVertexDirection()", Names[c], "The vertex direction is not known: 0", Event->GetVertexDirection(), 0) && Passed;
    Passed = EvaluateTrue("GetElectronTrack()", Names[c], "There is no electron track", Event->GetElectronTrack() == nullptr) && Passed;
    Passed = EvaluateTrue("GetPositronTrack()", Names[c], "There is no positron track", Event->GetPositronTrack() == nullptr) && Passed;
    Passed = Evaluate("GetNREAMs()", Names[c], "There are no additional measurements", Event->GetNREAMs(), 0U) && Passed;
    Passed = EvaluateTrue("GetREAMBegin()", Names[c], "The iterators of the additional measurements are equal", Event->GetREAMBegin() == Event->GetREAMEnd()) && Passed;
    Passed = Evaluate("GetNRESEs()", Names[c], "There are no RESEs", Event->GetNRESEs(), 0) && Passed;
    Passed = EvaluateNear("GetEnergy()", Names[c], "The energy is zero", Event->GetEnergy(), 0.0, 1e-15) && Passed;
    Passed = Evaluate("GetTrackLength()", Names[c], "The track length is zero", Event->GetTrackLength(), 0) && Passed;
    Passed = Evaluate("GetSequenceLength()", Names[c], "The sequence length is zero", Event->GetSequenceLength(), 0) && Passed;
    Passed = EvaluateFalse("HasGalacticPointing()", Names[c], "There is no Galactic pointing", Event->HasGalacticPointing()) && Passed;
    Passed = EvaluateFalse("HasDetectorRotation()", Names[c], "There is no detector rotation", Event->HasDetectorRotation()) && Passed;
    Passed = EvaluateFalse("HasHorizonPointing()", Names[c], "There is no horizon pointing", Event->HasHorizonPointing()) && Passed;
    Passed = EvaluateFalse("ResetIterator()", Names[c], "The next RESE of an iterator which was not started is a nullptr", Event->GetNextRESE() != nullptr) && Passed;
    delete Event;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the simple setters and getters
bool UTRERawEvent::TestGettersSetters()
{
  bool Passed = true;

  MRERawEvent Event;

  // The ID of the event is not the ID of the RESE:
  int RESEID = Event.GetID();
  Event.SetEventID(123456789012UL);
  Passed = Evaluate("SetEventID()", "123456789012", "The event ID is returned, also above 32 bits", Event.GetEventID(), 123456789012UL) && Passed;
  Passed = Evaluate("SetEventID()", "123456789012", "The ID of the RESE is unchanged", Event.GetID(), RESEID) && Passed;
  Event.SetEventId(17);
  Passed = Evaluate("SetEventId()", "17", "The deprecated setter sets the event ID", Event.GetEventID(), 17UL) && Passed;
  Passed = Evaluate("GetEventId()", "17", "The deprecated getter returns the event ID", Event.GetEventId(), 17UL) && Passed;

  // Times:
  Event.SetEventTime(MTime(1234.5));
  Event.SetEventClock(MTime(0.25));
  Event.SetCoincidenceWindow(MTime(2.0e-6));
  Passed = EvaluateNear("SetEventTime()", "1234.5 s", "The event time is returned", Event.GetEventTime().GetAsSeconds(), 1234.5, 1e-9) && Passed;
  Passed = EvaluateNear("SetEventClock()", "0.25 s", "The event clock is returned", Event.GetEventClock().GetAsSeconds(), 0.25, 1e-9) && Passed;
  Passed = EvaluateNear("SetCoincidenceWindow()", "2 us", "The coincidence window is returned", Event.GetCoincidenceWindow().GetAsSeconds(), 2.0e-6, 1e-12) && Passed;
  Passed = EvaluateNear("SetEventTime()", "1234.5 s", "The time of the RESE is unchanged", Event.GetTime(), 0.0, 1e-15) && Passed;

  // Geometry - not owned:
  MGeometryRevan Geometry;
  Event.SetGeometry(&Geometry);
  Passed = EvaluateTrue("SetGeometry()", "geometry", "The geometry is returned", Event.GetGeometry() == &Geometry) && Passed;
  Event.SetGeometry(nullptr);
  Passed = EvaluateTrue("SetGeometry()", "nullptr", "The geometry can be removed", Event.GetGeometry() == nullptr) && Passed;

  // Flags:
  Event.SetGoodEvent();
  Passed = EvaluateTrue("SetGoodEvent()", "default argument", "The event is good", Event.IsGoodEvent()) && Passed;
  Event.SetGoodEvent(false);
  Passed = EvaluateFalse("SetGoodEvent()", "false", "The event is not good", Event.IsGoodEvent()) && Passed;
  Event.SetDecay();
  Passed = EvaluateTrue("SetDecay()", "default argument", "The event is a decay", Event.IsDecay()) && Passed;
  Event.SetDecay(false);
  Passed = EvaluateFalse("SetDecay()", "false", "The event is no decay", Event.IsDecay()) && Passed;

  // Start point, vertex, and pair tracks are links to RESEs of the event - they are not owned
  MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 1.0);
  MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 200.0, 1.0);
  Event.AddRESE(A);
  Event.AddRESE(B);
  Event.SetStartPoint(A);
  Passed = EvaluateTrue("SetStartPoint()", "first hit", "The start point is returned", Event.GetStartPoint() == A) && Passed;
  Event.SetVertex(B);
  Event.SetVertexDirection(-1);
  Passed = EvaluateTrue("SetVertex()", "second hit", "The vertex is returned", Event.GetVertex() == B) && Passed;
  Passed = Evaluate("SetVertexDirection()", "-1", "The vertex direction is returned", Event.GetVertexDirection(), -1) && Passed;
  Passed = EvaluateTrue("SetVertex()", "second hit", "The start point is unchanged", Event.GetStartPoint() == A) && Passed;
  Event.SetElectronTrack(A);
  Event.SetPositronTrack(B);
  Passed = EvaluateTrue("SetElectronTrack()", "first hit", "The electron track is returned", Event.GetElectronTrack() == A) && Passed;
  Passed = EvaluateTrue("SetPositronTrack()", "second hit", "The positron track is returned", Event.GetPositronTrack() == B) && Passed;
  Event.SetStartPoint(nullptr);
  Event.SetVertex(nullptr);
  Passed = EvaluateTrue("SetStartPoint()", "nullptr", "The start point can be removed", Event.GetStartPoint() == nullptr) && Passed;
  Passed = EvaluateTrue("SetVertex()", "nullptr", "The vertex can be removed", Event.GetVertex() == nullptr) && Passed;

  // The origin information is an additional measurement:
  Event.SetOriginInformation(MVector(1.0, 2.0, 3.0), MVector(0.0, 0.0, -1.0), MVector(1.0, 0.0, 0.0), 511.0, 1);
  Passed = Evaluate("SetOriginInformation()", "all values", "There is one additional measurement", Event.GetNREAMs(), 1U) && Passed;
  MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(Event.GetREAMAt(0));
  Passed = EvaluateTrue("SetOriginInformation()", "all values", "It is a start information", Start != nullptr) && Passed;
  if (Start != nullptr) {
    Passed = EvaluateVectorNear("SetOriginInformation()", "all values", "The position is stored", Start->GetPosition(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
    Passed = EvaluateVectorNear("SetOriginInformation()", "all values", "The direction is stored", Start->GetDirection(), MVector(0.0, 0.0, -1.0), 1e-15) && Passed;
    Passed = EvaluateVectorNear("SetOriginInformation()", "all values", "The polarization is stored", Start->GetPolarization(), MVector(1.0, 0.0, 0.0), 1e-15) && Passed;
    Passed = EvaluateNear("SetOriginInformation()", "all values", "The energy is stored", Start->GetEnergy(), 511.0, 1e-12) && Passed;
    Passed = Evaluate("SetOriginInformation()", "all values", "The particle ID is stored", Start->GetParticleID(), 1) && Passed;
  }
  Event.SetOriginInformation(MVector(1.0, 2.0, 3.0), MVector(0.0, 0.0, -1.0), MVector(1.0, 0.0, 0.0), 511.0);
  MREAMStartInformation* WithoutID = dynamic_cast<MREAMStartInformation*>(Event.GetREAMAt(1));
  Passed = EvaluateTrue("SetOriginInformation()", "without particle ID", "A second start information is added", WithoutID != nullptr) && Passed;
  if (WithoutID != nullptr) {
    Passed = Evaluate("SetOriginInformation()", "without particle ID", "The particle ID is not defined", WithoutID->GetParticleID(), g_IntNotDefined) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the event types
bool UTRERawEvent::TestEventTypes()
{
  bool Passed = true;

  MRERawEvent Event;
  vector<int> Types = { MRERawEvent::c_UnknownEvent, MRERawEvent::c_ComptonEvent, MRERawEvent::c_PairEvent, MRERawEvent::c_MipEvent, MRERawEvent::c_ShowerEvent, MRERawEvent::c_PhotoEvent };
  vector<MString> Names = { "Unidentifiable", "Compton", "Pair", "MIP", "Shower", "Photo" };
  for (unsigned int t = 0; t < Types.size(); ++t) {
    Event.SetEventType(Types[t]);
    Passed = Evaluate("SetEventType()", Names[t], "The event type is returned", Event.GetEventType(), Types[t]) && Passed;
    Passed = Evaluate("GetEventTypeAsString()", Names[t], "The name of the event type is returned", Event.GetEventTypeAsString(), Names[t]) && Passed;
  }

  // A type which does not exist:
  Event.SetEventType(999);
  Passed = Evaluate("SetEventType()", "999", "A type which does not exist is stored", Event.GetEventType(), 999) && Passed;
  Passed = Evaluate("GetEventTypeAsString()", "999", "A type which does not exist has the name for it", Event.GetEventTypeAsString(), MString("Unknown event")) && Passed;

  // The type does not change the quality of the event:
  Passed = EvaluateFalse("SetEventType()", "999", "Setting the type does not make the event good", Event.IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("SetEventType()", "999", "Setting the type keeps the event valid", Event.IsValid()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rejection reasons
bool UTRERawEvent::TestRejectionReasons()
{
  bool Passed = true;

  // All reasons have a long and a short text, the short one without blanks, and no two are the same
  set<MString> LongTexts;
  set<MString> ShortTexts;
  bool AllLongFilled = true;
  bool AllShortFilled = true;
  bool NoBlankInShort = true;
  for (int r = MRERawEvent::c_RejectionNone; r <= MRERawEvent::c_RejectionStripPairingNotResolvable; ++r) {
    MString Long = MRERawEvent::GetRejectionReasonAsString(r);
    MString Short = MRERawEvent::GetRejectionReasonAsString(r, true);
    if (Long.IsEmpty() == true || Long.BeginsWith("Unknown rejection type") == true) {
      AllLongFilled = false;
    }
    if (Short.IsEmpty() == true) {
      AllShortFilled = false;
    }
    if (Short.Contains(" ") == true) {
      NoBlankInShort = false;
    }
    LongTexts.insert(Long);
    ShortTexts.insert(Short);
  }
  Passed = EvaluateTrue("GetRejectionReasonAsString()", "0 to 38, long", "Each reason has a long text", AllLongFilled) && Passed;
  Passed = EvaluateTrue("GetRejectionReasonAsString()", "0 to 38, short", "Each reason has a short text", AllShortFilled) && Passed;
  Passed = EvaluateTrue("GetRejectionReasonAsString()", "0 to 38, short", "No short text has a blank: it is a flag in the tra file", NoBlankInShort) && Passed;
  Passed = EvaluateSize("GetRejectionReasonAsString()", "0 to 38, long", "All 39 long texts are different", LongTexts.size(), 39) && Passed;
  Passed = EvaluateSize("GetRejectionReasonAsString()", "0 to 38, short", "All 39 short texts are different", ShortTexts.size(), 39) && Passed;

  // Some texts:
  Passed = Evaluate("GetRejectionReasonAsString()", "c_RejectionNone, long", "No rejection is None", MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionNone), MString("None")) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "c_RejectionNone, short", "No rejection is None", MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionNone, true), MString("None")) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "c_RejectionTooManyHits, long", "The long text is a sentence", MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionTooManyHits), MString("Too many overall hits")) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "c_RejectionTooManyHits, short", "The short text is one word", MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionTooManyHits, true), MString("TooManyOverallHits")) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "c_RejectionD2Only, short", "The short text names the calorimeters", MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionD2Only, true), MString("EventRestrictedToCalorimeters")) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "c_RejectionExternalBadEventFlag, short", "The short text names the external flag", MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionExternalBadEventFlag, true), MString("ExternalBadEventFlagRaised")) && Passed;
  // Defect REV-C-12: The short text of this reason, which is a flag in the tra file, is misspelled as StripPairinTooManyStrips
  Passed = ExpectedToFail(Evaluate("GetRejectionReasonAsString()", "c_RejectionStripPairinTooManyStrips, short", "The short text is spelled as the other strip pairing reasons", MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionStripPairinTooManyStrips, true), MString("StripPairingTooManyStrips"))) && Passed;

  // Reasons which do not exist:
  Passed = Evaluate("GetRejectionReasonAsString()", "39, long", "A reason behind the last one has the text for an unknown reason", MRERawEvent::GetRejectionReasonAsString(39), MString("Unknown rejection type: 39")) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "39, short", "A reason behind the last one has no short text", MRERawEvent::GetRejectionReasonAsString(39, true), MString("")) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "-1, long", "A negative reason has the text for an unknown reason", MRERawEvent::GetRejectionReasonAsString(-1), MString("Unknown rejection type: -1")) && Passed;

  // The reason of an event:
  MRERawEvent Event;
  Passed = Evaluate("GetRejectionReasonAsString()", "new event", "A new event has the text None", Event.GetRejectionReasonAsString(), MString("None")) && Passed;
  Event.SetRejectionReason(MRERawEvent::c_RejectionTooManyHits);
  Passed = Evaluate("SetRejectionReason()", "c_RejectionTooManyHits", "The reason is returned", Event.GetRejectionReason(), MRERawEvent::c_RejectionTooManyHits) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "c_RejectionTooManyHits, long", "The long text of the reason of the event is returned", Event.GetRejectionReasonAsString(), MString("Too many overall hits")) && Passed;
  Passed = Evaluate("GetRejectionReasonAsString()", "c_RejectionTooManyHits, short", "The short text of the reason of the event is returned", Event.GetRejectionReasonAsString(true), MString("TooManyOverallHits")) && Passed;
  Passed = EvaluateFalse("SetRejectionReason()", "c_RejectionTooManyHits", "A rejected event is not valid", Event.IsValid()) && Passed;

  // No rejection keeps a valid event valid:
  MRERawEvent NotRejected;
  NotRejected.SetRejectionReason(MRERawEvent::c_RejectionNone);
  Passed = EvaluateTrue("SetRejectionReason()", "c_RejectionNone", "An event without rejection stays valid", NotRejected.IsValid()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the flags of a bad event
bool UTRERawEvent::TestBadFlags()
{
  bool Passed = true;

  MRERawEvent Event;
  Passed = EvaluateFalse("HasBadFlag()", "new event", "A new event has no flag", Event.HasBadFlag("Saturated")) && Passed;

  Event.AddBadFlag("Saturated");
  Passed = EvaluateTrue("AddBadFlag()", "Saturated", "The event is bad", Event.IsBad()) && Passed;
  Passed = Evaluate("AddBadFlag()", "Saturated", "There is one flag", Event.GetNBadFlags(), 1U) && Passed;
  Passed = Evaluate("GetBadFlag()", "index 0", "The flag is returned", Event.GetBadFlag(0), MString("Saturated")) && Passed;
  Passed = EvaluateTrue("HasBadFlag()", "Saturated", "The flag is found", Event.HasBadFlag("Saturated")) && Passed;
  Passed = EvaluateFalse("HasBadFlag()", "saturated", "The search is case sensitive", Event.HasBadFlag("saturated")) && Passed;
  Passed = EvaluateFalse("HasBadFlag()", "Satur", "A part of a flag is not found", Event.HasBadFlag("Satur")) && Passed;

  Event.AddBadFlag("Pile up");
  Passed = Evaluate("AddBadFlag()", "second flag", "There are two flags", Event.GetNBadFlags(), 2U) && Passed;
  Passed = Evaluate("GetBadFlag()", "index 1", "The second flag is returned", Event.GetBadFlag(1), MString("Pile up")) && Passed;
  Passed = Evaluate("GetBadFlag()", "index 0", "The first flag is unchanged", Event.GetBadFlag(0), MString("Saturated")) && Passed;

  // The flags accumulate, also if they are equal:
  Event.AddBadFlag("Saturated");
  Passed = Evaluate("AddBadFlag()", "same flag again", "The same flag is stored a second time", Event.GetNBadFlags(), 3U) && Passed;

  // Behind the end:
  MRERawEvent* Pointer = &Event;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBadFlag()", "index 3 of 3", "The index behind the last flag throws an index exception", [Pointer]() { Pointer->GetBadFlag(3); }) && Passed;
  MRERawEvent Empty;
  MRERawEvent* EmptyPointer = &Empty;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBadFlag()", "index 0 of 0", "Reading a flag of an event without flags throws an index exception", [EmptyPointer]() { EmptyPointer->GetBadFlag(0); }) && Passed;

  // The flags do not change the validity or the quality:
  Passed = EvaluateTrue("AddBadFlag()", "three flags", "A bad event stays valid", Event.IsValid()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factors
bool UTRERawEvent::TestQualityFactors()
{
  bool Passed = true;

  // Each value is different to detect swapped members
  MRERawEvent Event;
  Event.SetClusteringQualityFactor(0.125);
  Event.SetComptonQualityFactors(0.25, 0.5);
  Event.SetTrackQualityFactor(0.75);
  Event.SetPairQualityFactor(1.5);
  Passed = EvaluateNear("SetClusteringQualityFactor()", "0.125", "The clustering quality factor is returned", Event.GetClusteringQualityFactor(), 0.125, 1e-15) && Passed;
  Passed = EvaluateNear("SetComptonQualityFactors()", "0.25, 0.5", "The quality factor of the best sequence is returned", Event.GetComptonQualityFactor1(), 0.25, 1e-15) && Passed;
  Passed = EvaluateNear("SetComptonQualityFactors()", "0.25, 0.5", "The quality factor of the second best sequence is returned", Event.GetComptonQualityFactor2(), 0.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetComptonQualityFactor()", "0.25, 0.5", "The Compton quality factor is the one of the best sequence", Event.GetComptonQualityFactor(), 0.25, 1e-15) && Passed;
  Passed = EvaluateNear("SetTrackQualityFactor()", "0.75", "The track quality factor is returned", Event.GetTrackQualityFactor(), 0.75, 1e-15) && Passed;
  Passed = EvaluateNear("SetPairQualityFactor()", "1.5", "The pair quality factor is returned", Event.GetPairQualityFactor(), 1.5, 1e-15) && Passed;

  // Zero and negative values are stored:
  Event.SetTrackQualityFactor(0.0);
  Event.SetPairQualityFactor(-2.5);
  Passed = EvaluateNear("SetTrackQualityFactor()", "0", "A zero quality factor is stored", Event.GetTrackQualityFactor(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("SetPairQualityFactor()", "-2.5", "A negative quality factor is stored", Event.GetPairQualityFactor(), -2.5, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the energy, the track length, and the sequence length
bool UTRERawEvent::TestEnergyAndLengths()
{
  bool Passed = true;

  // Hits:
  MRERawEvent Event;
  Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.25, 3.0));
  Passed = EvaluateNear("GetEnergy()", "one hit", "The energy is the one of the hit", Event.GetEnergy(), 100.25, 1e-12) && Passed;
  Passed = Evaluate("GetSequenceLength()", "one hit", "The sequence length is one", Event.GetSequenceLength(), 1) && Passed;
  Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 1.0), 50.5, 4.0));
  // 100.25 keV + 50.5 keV = 150.75 keV, resolution sqrt(3^2 + 4^2) keV = 5 keV
  Passed = EvaluateNear("GetEnergy()", "two hits", "The energy is the sum of the hits", Event.GetEnergy(), 150.75, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergyResolution()", "two hits", "The energy resolution is the quadratic sum of the hits", Event.GetEnergyResolution(), 5.0, 1e-12) && Passed;
  Passed = Evaluate("GetSequenceLength()", "two hits", "The sequence length is two", Event.GetSequenceLength(), 2) && Passed;
  Passed = Evaluate("GetTrackLength()", "two hits", "Without a track the track length is zero", Event.GetTrackLength(), 0) && Passed;

  // The additional energy counts:
  Event.SetAdditionalEnergy(7.5);
  Passed = EvaluateNear("GetEnergy()", "two hits and 7.5 keV additional energy", "The additional energy is added", Event.GetEnergy(), 158.25, 1e-12) && Passed;
  Event.SetAdditionalEnergy(0.0);

  // A strip hit is one of the two sides of a deposit, thus it counts half:
  MREStripHit* Strip = new MREStripHit();
  Strip->SetEnergy(80.0);
  Event.AddRESE(Strip);
  // 150.75 keV + 0.5*80 keV = 190.75 keV
  Passed = EvaluateNear("GetEnergy()", "two hits and a strip hit of 80 keV", "A strip hit counts with half of its energy", Event.GetEnergy(), 190.75, 1e-12) && Passed;
  Passed = Evaluate("GetSequenceLength()", "two hits and a strip hit", "The sequence length is three", Event.GetSequenceLength(), 3) && Passed;

  // Tracks: the track length is the number of RESEs of the longest track
  MRETrack* Short = new MRETrack();
  Short->AddRESE(CreatePlainHit(MVector(1.0, 0.0, 0.0), 10.0, 1.0));
  Short->AddRESE(CreatePlainHit(MVector(1.0, 0.0, 1.0), 20.0, 1.0));
  Event.AddRESE(Short);
  Passed = Evaluate("GetTrackLength()", "one track with two hits", "The track length is two", Event.GetTrackLength(), 2) && Passed;
  MRETrack* Long = new MRETrack();
  Long->AddRESE(CreatePlainHit(MVector(2.0, 0.0, 0.0), 1.0, 1.0));
  Long->AddRESE(CreatePlainHit(MVector(2.0, 0.0, 1.0), 2.0, 1.0));
  Long->AddRESE(CreatePlainHit(MVector(2.0, 0.0, 2.0), 3.0, 1.0));
  Event.AddRESE(Long);
  Passed = Evaluate("GetTrackLength()", "tracks with two and three hits", "The track length is the one of the longest track", Event.GetTrackLength(), 3) && Passed;
  Passed = Evaluate("GetSequenceLength()", "two hits, a strip hit, two tracks", "A track counts as one element of the sequence", Event.GetSequenceLength(), 5) && Passed;
  // 190.75 keV + 30 keV + 6 keV = 226.75 keV
  Passed = EvaluateNear("GetEnergy()", "two hits, a strip hit, two tracks", "The energies of the tracks are added", Event.GetEnergy(), 226.75, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validity of the event
bool UTRERawEvent::TestIsValid()
{
  bool Passed = true;

  MRERawEvent Event;
  MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 1.0);
  MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 200.0, 1.0);
  Event.AddRESE(A);
  Event.AddRESE(B);
  Passed = EvaluateTrue("IsValid()", "two valid hits", "The event is valid", Event.IsValid()) && Passed;

  // One hit which is not valid makes the event not valid:
  B->SetValid(false);
  Passed = EvaluateFalse("IsValid()", "second hit not valid", "The event is not valid", Event.IsValid()) && Passed;
  B->SetValid(true);
  Passed = EvaluateTrue("IsValid()", "second hit valid again", "The event is valid again", Event.IsValid()) && Passed;

  // The flag of the event itself:
  Event.SetValid(false);
  Passed = EvaluateFalse("IsValid()", "event not valid, valid hits", "The event is not valid", Event.IsValid()) && Passed;
  Event.SetValid(true);
  Passed = EvaluateTrue("IsValid()", "event valid again", "The event is valid again", Event.IsValid()) && Passed;

  // A good event which is not valid:
  Event.SetGoodEvent(true);
  A->SetValid(false);
  Passed = EvaluateFalse("IsValid()", "good event, first hit not valid", "A good event with a hit which is not valid is not valid", Event.IsValid()) && Passed;
  Passed = EvaluateTrue("IsGoodEvent()", "good event, first hit not valid", "The event is still good", Event.IsGoodEvent()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test deleting and shuffling the RESEs
bool UTRERawEvent::TestDeleteAllAndShuffle()
{
  bool Passed = true;

  MRERawEvent Event;
  vector<MRESE*> Hits;
  for (unsigned int h = 0; h < 6; ++h) {
    Hits.push_back(CreatePlainHit(MVector(0.0, 0.0, 1.0*h), 100.0 + h, 1.0));
    Event.AddRESE(Hits.back());
  }

  // Shuffle keeps all hits:
  Event.Shuffle();
  Passed = Evaluate("Shuffle()", "six hits", "The number of hits is unchanged", Event.GetNRESEs(), 6) && Passed;
  bool AllThere = true;
  for (unsigned int h = 0; h < Hits.size(); ++h) {
    if (Event.ContainsRESE(Hits[h]) == false) {
      AllThere = false;
    }
  }
  Passed = EvaluateTrue("Shuffle()", "six hits", "All hits are still in the event", AllThere) && Passed;
  // 100 + 101 + ... + 105 keV = 615 keV
  Passed = EvaluateNear("Shuffle()", "six hits", "The energy is unchanged", Event.GetEnergy(), 615.0, 1e-12) && Passed;

  // Delete all: the hits are deleted, but their slots stay until the list is compressed, as everywhere in the RESE lists
  Event.DeleteAll();
  bool AllSlotsEmpty = true;
  for (int r = 0; r < Event.GetNRESEs(); ++r) {
    if (Event.GetRESEAt(r) != nullptr) {
      AllSlotsEmpty = false;
    }
  }
  Passed = EvaluateTrue("DeleteAll()", "six hits", "No hit is left", AllSlotsEmpty) && Passed;
  Event.CompressRESEs();
  Passed = Evaluate("DeleteAll()", "six hits, compressed", "There are no RESEs any more", Event.GetNRESEs(), 0) && Passed;
  Passed = EvaluateNear("DeleteAll()", "six hits, compressed", "The energy is zero", Event.GetEnergy(), 0.0, 1e-15) && Passed;

  // The event can be filled and emptied again:
  Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 50.0, 1.0));
  Passed = Evaluate("AddRESE()", "after DeleteAll", "The event can be filled again", Event.GetNRESEs(), 1) && Passed;
  Event.DeleteAll();
  Event.DeleteAll();
  Event.CompressRESEs();
  Passed = Evaluate("DeleteAll()", "twice", "A second call changes nothing", Event.GetNRESEs(), 0) && Passed;

  // An empty event can be shuffled:
  Event.Shuffle();
  Passed = Evaluate("Shuffle()", "no hits", "An empty event stays empty", Event.GetNRESEs(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the lines with the data of the event: ID, times, quality, flags, comments
bool UTRERawEvent::TestParseLineEventData()
{
  bool Passed = true;

  MRERawEvent Event;

  // Lines which are not for the event return 2:
  Passed = Evaluate("ParseLine()", "empty line", "An empty line is not parsed: 2", Event.ParseLine("", c_SimVersion), 2) && Passed;
  Passed = Evaluate("ParseLine()", "one character", "A line with one character is not parsed: 2", Event.ParseLine("T", c_SimVersion), 2) && Passed;
  Passed = Evaluate("ParseLine()", "SE", "The start of the event is not parsed: 2", Event.ParseLine("SE", c_SimVersion), 2) && Passed;
  Passed = Evaluate("ParseLine()", "ZZ 1", "An unknown keyword is not parsed: 2", Event.ParseLine("ZZ 1", c_SimVersion), 2) && Passed;
  Passed = Evaluate("ParseLine()", "ti 1.0", "The keywords are case sensitive: 2", Event.ParseLine("ti 1.0", c_SimVersion), 2) && Passed;

  // ID:
  Passed = Evaluate("ParseLine()", "ID 17 23", "The ID line of a sim file with two numbers is parsed", Event.ParseLine("ID 17 23", c_SimVersion), 0) && Passed;
  Passed = Evaluate("ParseLine()", "ID 17 23", "The first number is the event ID", Event.GetEventID(), 17UL) && Passed;
  Passed = Evaluate("ParseLine()", "ID 99", "The ID line with one number is parsed", Event.ParseLine("ID 99", c_SimVersion), 0) && Passed;
  Passed = Evaluate("ParseLine()", "ID 99", "The number is the event ID", Event.GetEventID(), 99UL) && Passed;
  Passed = Evaluate("ParseLine()", "ID 123456789012", "An ID above 32 bits is parsed", Event.ParseLine("ID 123456789012", c_SimVersion), 0) && Passed;
  Passed = Evaluate("ParseLine()", "ID 123456789012", "The ID above 32 bits is stored", Event.GetEventID(), 123456789012UL) && Passed;
  Passed = Evaluate("ParseLine()", "ID abc", "An ID which is no number is an error: 1", Event.ParseLine("ID abc", c_SimVersion), 1) && Passed;
  Passed = Evaluate("ParseLine()", "ID abc", "A failed ID line keeps the old ID", Event.GetEventID(), 123456789012UL) && Passed;
  Passed = Evaluate("ParseLine()", "ID", "An ID line without number is an error: 1", Event.ParseLine("ID", c_SimVersion), 1) && Passed;

  // Time and clock:
  Passed = Evaluate("ParseLine()", "TI 100.250000000", "The time line is parsed", Event.ParseLine("TI 100.250000000", c_SimVersion), 0) && Passed;
  Passed = EvaluateNear("ParseLine()", "TI 100.250000000", "The time is stored", Event.GetEventTime().GetAsSeconds(), 100.25, 1e-9) && Passed;
  Passed = Evaluate("ParseLine()", "CL 3.5", "The clock line is parsed", Event.ParseLine("CL 3.5", c_SimVersion), 0) && Passed;
  Passed = EvaluateNear("ParseLine()", "CL 3.5", "The clock is stored", Event.GetEventClock().GetAsSeconds(), 3.5, 1e-9) && Passed;
  Passed = EvaluateNear("ParseLine()", "CL 3.5", "The time is unchanged", Event.GetEventTime().GetAsSeconds(), 100.25, 1e-9) && Passed;

  // Clustering quality factor:
  Passed = Evaluate("ParseLine()", "PQ 0.25", "The clustering quality line is parsed", Event.ParseLine("PQ 0.25", c_SimVersion), 0) && Passed;
  Passed = EvaluateNear("ParseLine()", "PQ 0.25", "The clustering quality factor is stored", Event.GetClusteringQualityFactor(), 0.25, 1e-15) && Passed;
  Passed = Evaluate("ParseLine()", "PQ abc", "A quality factor which is no number is an error: 1", Event.ParseLine("PQ abc", c_SimVersion), 1) && Passed;
  Passed = EvaluateNear("ParseLine()", "PQ abc", "A failed line keeps the old quality factor", Event.GetClusteringQualityFactor(), 0.25, 1e-15) && Passed;

  // Time walk - it is only visible in the physical event:
  Passed = Evaluate("ParseLine()", "TW 5", "The time walk line is parsed", Event.ParseLine("TW 5", c_SimVersion), 0) && Passed;
  Passed = Evaluate("ParseLine()", "TW 5", "The time walk is handed to the physical event", Event.GetPhysicalEvent()->GetTimeWalk(), 5) && Passed;
  Passed = Evaluate("ParseLine()", "TW abc", "A time walk which is no number is an error: 1", Event.ParseLine("TW abc", c_SimVersion), 1) && Passed;

  // Bad flags - each line is one flag:
  Passed = Evaluate("ParseLine()", "BD Saturated strip", "The bad flag line is parsed", Event.ParseLine("BD Saturated strip\n", c_SimVersion), 0) && Passed;
  Passed = Evaluate("ParseLine()", "BD Saturated strip", "There is one flag", Event.GetNBadFlags(), 1U) && Passed;
  if (Event.GetNBadFlags() == 1) {
    Passed = Evaluate("ParseLine()", "BD Saturated strip", "The flag is the text behind the keyword without the line break", Event.GetBadFlag(0), MString("Saturated strip")) && Passed;
  }
  Passed = Evaluate("ParseLine()", "BD PileUp", "A second bad flag line is parsed", Event.ParseLine("BD PileUp", c_SimVersion), 0) && Passed;
  Passed = Evaluate("ParseLine()", "BD PileUp", "The flags accumulate", Event.GetNBadFlags(), 2U) && Passed;
  Passed = EvaluateTrue("ParseLine()", "BD PileUp", "The second flag is found", Event.HasBadFlag("PileUp")) && Passed;
  Passed = EvaluateTrue("ParseLine()", "BD lines", "A bad flag does not make the event not valid", Event.IsValid()) && Passed;

  // Comments - they are only visible in the physical event:
  Passed = Evaluate("ParseLine()", "CC First comment", "The comment line is parsed", Event.ParseLine("CC First comment\n", c_SimVersion), 0) && Passed;
  Passed = Evaluate("ParseLine()", "CC Second", "A second comment line is parsed", Event.ParseLine("CC Second", c_SimVersion), 0) && Passed;
  MPhysicalEvent* Physical = Event.GetPhysicalEvent();
  Passed = Evaluate("ParseLine()", "two CC lines", "The physical event has both comments", Physical->GetNComments(), 2U) && Passed;
  if (Physical->GetNComments() == 2) {
    Passed = Evaluate("ParseLine()", "CC First comment", "The comment is the text behind the keyword without the line break", Physical->GetComment(0), MString("First comment")) && Passed;
    Passed = Evaluate("ParseLine()", "CC Second", "The second comment is stored", Physical->GetComment(1), MString("Second")) && Passed;
  }

  // No line added a hit or a measurement:
  Passed = Evaluate("ParseLine()", "event data lines", "No hit was added", Event.GetNRESEs(), 0) && Passed;
  Passed = Evaluate("ParseLine()", "event data lines", "No additional measurement was added", Event.GetNREAMs(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the lines with the orientations
bool UTRERawEvent::TestParseLineOrientations()
{
  bool Passed = true;

  // Detector rotation:
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "RX 0 1 0", "The x axis of the detector rotation is parsed", Event.ParseLine("RX 0 1 0", c_SimVersion), 0) && Passed;
    Passed = EvaluateTrue("ParseLine()", "RX 0 1 0", "The event has a detector rotation", Event.HasDetectorRotation()) && Passed;
    Passed = Evaluate("ParseLine()", "RZ -1 0 0", "The z axis of the detector rotation is parsed", Event.ParseLine("RZ -1 0 0", c_SimVersion), 0) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "RX 0 1 0", "The x axis is stored", Event.GetDetectorRotationXAxis(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "RZ -1 0 0", "The z axis is stored", Event.GetDetectorRotationZAxis(), MVector(-1.0, 0.0, 0.0), 1e-15) && Passed;
    Passed = EvaluateFalse("ParseLine()", "RX and RZ", "The event has no Galactic pointing", Event.HasGalacticPointing()) && Passed;
    Passed = EvaluateFalse("ParseLine()", "RX and RZ", "The event has no horizon pointing", Event.HasHorizonPointing()) && Passed;
  }

  // Galactic pointing: longitude and latitude in degrees
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "GX 10 20", "The x axis of the Galactic pointing is parsed", Event.ParseLine("GX 10 20", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "GZ 100 -30", "The z axis of the Galactic pointing is parsed", Event.ParseLine("GZ 100 -30", c_SimVersion), 0) && Passed;
    Passed = EvaluateTrue("ParseLine()", "GX and GZ", "The event has a Galactic pointing", Event.HasGalacticPointing()) && Passed;
    Passed = EvaluateNear("ParseLine()", "GX 10 20", "The longitude of the x axis is 10 deg", Event.GetGalacticPointingXAxisLongitude()*c_Deg, 10.0, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "GX 10 20", "The latitude of the x axis is 20 deg", Event.GetGalacticPointingXAxisLatitude()*c_Deg, 20.0, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "GZ 100 -30", "The longitude of the z axis is 100 deg", Event.GetGalacticPointingZAxisLongitude()*c_Deg, 100.0, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "GZ 100 -30", "The latitude of the z axis is -30 deg", Event.GetGalacticPointingZAxisLatitude()*c_Deg, -30.0, 1e-12) && Passed;
    Passed = EvaluateFalse("ParseLine()", "GX and GZ", "The event has no detector rotation", Event.HasDetectorRotation()) && Passed;
  }

  // Horizon pointing: azimuth and altitude in degrees
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "HX 10 20", "The x axis of the horizon pointing is parsed", Event.ParseLine("HX 10 20", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "HZ 100 -30", "The z axis of the horizon pointing is parsed", Event.ParseLine("HZ 100 -30", c_SimVersion), 0) && Passed;
    Passed = EvaluateTrue("ParseLine()", "HX and HZ", "The event has a horizon pointing", Event.HasHorizonPointing()) && Passed;
    // The axis is the unit vector with the polar angle 90 deg - altitude and the azimuth
    MVector ExpectedX(sin(70.0*c_Rad)*cos(10.0*c_Rad), sin(70.0*c_Rad)*sin(10.0*c_Rad), cos(70.0*c_Rad));
    MVector ExpectedZ(sin(120.0*c_Rad)*cos(100.0*c_Rad), sin(120.0*c_Rad)*sin(100.0*c_Rad), cos(120.0*c_Rad));
    Passed = EvaluateVectorNear("ParseLine()", "HX 10 20", "The x axis has the azimuth 10 deg and the altitude 20 deg", Event.GetHorizonPointingXAxis(), ExpectedX, 1e-14) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "HZ 100 -30", "The z axis has the azimuth 100 deg and the altitude -30 deg", Event.GetHorizonPointingZAxis(), ExpectedZ, 1e-14) && Passed;
    Passed = EvaluateFalse("ParseLine()", "HX and HZ", "The event has no Galactic pointing", Event.HasGalacticPointing()) && Passed;
  }

  // Lines with too few numbers are errors and leave the event as it was
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "RX 0 1", "A detector rotation with two numbers is an error: 1", Event.ParseLine("RX 0 1", c_SimVersion), 1) && Passed;
    // Defect REV-C-13: A failed orientation line still sets the flag of the orientation, and the axis keeps the numbers which were read
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "RX 0 1", "A failed line does not give the event a detector rotation", Event.HasDetectorRotation())) && Passed;
    Passed = ExpectedToFail(EvaluateVectorNear("ParseLine()", "RX 0 1", "A failed line keeps the x axis of the detector", Event.GetDetectorRotationXAxis(), MVector(1.0, 0.0, 0.0), 1e-15)) && Passed;
  }
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "RZ", "A detector rotation without numbers is an error: 1", Event.ParseLine("RZ", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "GX 10", "A Galactic pointing with one number is an error: 1", Event.ParseLine("GX 10", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "GZ abc", "A Galactic pointing without number is an error: 1", Event.ParseLine("GZ abc", c_SimVersion), 1) && Passed;
    // Defect REV-C-13: A failed Galactic pointing line sets the axis from variables which were never set
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "GX 10", "A failed line does not give the event a Galactic pointing", Event.HasGalacticPointing())) && Passed;
  }
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "HX 10", "A horizon pointing with one number is an error: 1", Event.ParseLine("HX 10", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "HZ", "A horizon pointing without numbers is an error: 1", Event.ParseLine("HZ", c_SimVersion), 1) && Passed;
    // Defect REV-C-13: A failed horizon pointing line sets the axis from variables which were never set
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "HX 10", "A failed line does not give the event a horizon pointing", Event.HasHorizonPointing())) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing hits without a geometry
bool UTRERawEvent::TestParseLineHits()
{
  bool Passed = true;

  // A hit of a sim file: detector type, position, energy, time, origins
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The hit of a sim file is parsed", Event.ParseLine("HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The event has one RESE", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      Passed = Evaluate("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The RESE is a hit", Hit->GetType(), MRESE::c_Hit) && Passed;
      Passed = Evaluate("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The detector type is 1", Hit->GetDetector(), 1) && Passed;
      Passed = EvaluateVectorNear("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The position is stored", Hit->GetPosition(), MVector(1.5, -2.5, 0.25), 1e-15) && Passed;
      Passed = EvaluateNear("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The energy is stored", Hit->GetEnergy(), 150.5, 1e-12) && Passed;
      Passed = EvaluateNear("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The time is stored", Hit->GetTime(), 0.5, 1e-15) && Passed;
      set<unsigned int> Origins = { 3, 4 };
      Passed = EvaluateTrue("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The origins are 3 and 4", Hit->GetOriginIDs() == Origins) && Passed;
      // Without geometry the resolutions are ideal:
      Passed = EvaluateNear("ParseLine()", "HTsim without geometry", "The energy resolution is zero", Hit->GetEnergyResolution(), 0.0, 1e-15) && Passed;
      Passed = EvaluateVectorNear("ParseLine()", "HTsim without geometry", "The position resolution is zero", Hit->GetPositionResolution(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
    }
    Passed = EvaluateNear("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The energy of the event is the one of the hit", Event.GetEnergy(), 150.5, 1e-12) && Passed;
    Passed = EvaluateTrue("ParseLine()", "HTsim 1;1.5;-2.5;0.25;150.5;0.5;3;4", "The event is valid", Event.IsValid()) && Passed;

    // A second hit on the same event:
    Passed = Evaluate("ParseLine()", "second HTsim", "A second hit is parsed", Event.ParseLine("HTsim 2;0.0;0.0;-10.0;300.0;0.75", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "second HTsim", "The event has two RESEs", Event.GetNRESEs(), 2) && Passed;
    Passed = EvaluateNear("ParseLine()", "second HTsim", "The energy of the event is the sum", Event.GetEnergy(), 450.5, 1e-12) && Passed;
  }

  // A hit of an evta file: detector type, position, energy, position resolution, energy resolution
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", "The hit of an evta file is parsed", Event.ParseLine("HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", c_EvtaVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", "The event has one RESE", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      Passed = Evaluate("ParseLine()", "HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", "The detector type is 3", Hit->GetDetector(), 3) && Passed;
      Passed = EvaluateVectorNear("ParseLine()", "HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", "The position is stored", Hit->GetPosition(), MVector(1.0, 2.0, 1.0), 1e-15) && Passed;
      Passed = EvaluateNear("ParseLine()", "HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", "The energy is stored", Hit->GetEnergy(), 100.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("ParseLine()", "HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", "The position resolution is stored", Hit->GetPositionResolution(), MVector(0.1, 0.2, 0.3), 1e-15) && Passed;
      Passed = EvaluateNear("ParseLine()", "HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", "The energy resolution is stored", Hit->GetEnergyResolution(), 2.5, 1e-15) && Passed;
    }
    Passed = EvaluateNear("ParseLine()", "HT 3;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", "The energy resolution of the event is the one of the hit", Event.GetEnergyResolution(), 2.5, 1e-12) && Passed;
  }

  // A hit with a strip which was not read out is not used, and the event is not valid
  {
    vector<MString> Flags = { "XO", "YO" };
    for (const MString& Flag: Flags) {
      MRERawEvent Event;
      MString Line = MString("HT 1;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5;") + Flag;
      Passed = Evaluate("ParseLine()", Line, "A hit with only one strip is not parsed: 2", Event.ParseLine(Line, c_EvtaVersion), 2) && Passed;
      Passed = Evaluate("ParseLine()", Line, "The hit is not added", Event.GetNRESEs(), 0) && Passed;
      Passed = EvaluateFalse("ParseLine()", Line, "The event is not valid", Event.IsValid()) && Passed;
    }
  }

  // A hit with an overflow is used, but the event is not valid
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "HT with OF", "A hit with an overflow is parsed", Event.ParseLine("HT 1;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5;OF", c_EvtaVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "HT with OF", "The hit is added", Event.GetNRESEs(), 1) && Passed;
    Passed = EvaluateFalse("ParseLine()", "HT with OF", "The event is not valid", Event.IsValid()) && Passed;
  }

  // A hit without energy is not used, and the event is not valid
  {
    vector<MString> Energies = { "0.0", "-5.0" };
    for (const MString& Energy: Energies) {
      MRERawEvent Event;
      MString Line = MString("HTsim 1;1.0;2.0;0.0;") + Energy + ";0.5";
      DisableDefaultStreams();
      int Result = Event.ParseLine(Line, c_SimVersion);
      EnableDefaultStreams();
      Passed = Evaluate("ParseLine()", Line, "A hit without positive energy is no parse error", Result, 0) && Passed;
      Passed = Evaluate("ParseLine()", Line, "The hit is not added", Event.GetNRESEs(), 0) && Passed;
      Passed = EvaluateFalse("ParseLine()", Line, "The event is not valid", Event.IsValid()) && Passed;
    }
  }

  // A hit line with missing numbers
  {
    MRERawEvent Event;
    DisableDefaultStreams();
    int Result = Event.ParseLine("HTsim 1;1.0;2.0", c_SimVersion);
    EnableDefaultStreams();
    Passed = Evaluate("ParseLine()", "HTsim 1;1.0;2.0", "The hit is not added", Event.GetNRESEs(), 0) && Passed;
    Passed = EvaluateFalse("ParseLine()", "HTsim 1;1.0;2.0", "The event is not valid", Event.IsValid()) && Passed;
    // Defect REV-C-14: A hit line with missing numbers counts as correctly parsed, since MREHit::ParseLine reports success for it - the hit is only dropped later for its zero energy
    Passed = ExpectedToFail(EvaluateTrue("ParseLine()", "HTsim 1;1.0;2.0", "A hit line with missing numbers is not reported as correctly parsed", Result != 0)) && Passed;
  }

  // A file version which is not known
  {
    MRERawEvent Event;
    DisableDefaultStreams();
    int Result = Event.ParseLine("HTsim 1;1.5;-2.5;0.25;150.5;0.5", 7);
    EnableDefaultStreams();
    Passed = Evaluate("ParseLine()", "HTsim with file version 7", "A hit of an unknown file version is not parsed: 2", Result, 2) && Passed;
    Passed = Evaluate("ParseLine()", "HTsim with file version 7", "The hit is not added", Event.GetNRESEs(), 0) && Passed;
    Passed = EvaluateFalse("ParseLine()", "HTsim with file version 7", "The event is not valid", Event.IsValid()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limit of the number of hits
bool UTRERawEvent::TestParseLineHitLimit()
{
  bool Passed = true;

  // An event takes at most 10000 hits:
  MRERawEvent Event;
  int Accepted = 0;
  for (unsigned int h = 0; h < 10000; ++h) {
    if (Event.ParseLine("HTsim 1;1.0;2.0;0.0;100.0;0.5", c_SimVersion) == 0) {
      ++Accepted;
    }
  }
  Passed = Evaluate("ParseLine()", "10000 hits", "10000 hits are accepted", Accepted, 10000) && Passed;
  Passed = Evaluate("ParseLine()", "10000 hits", "The event has 10000 hits", Event.GetNRESEs(), 10000) && Passed;
  Passed = EvaluateTrue("ParseLine()", "10000 hits", "The event is valid", Event.IsValid()) && Passed;

  DisableDefaultStreams();
  int Result = Event.ParseLine("HTsim 1;1.0;2.0;0.0;100.0;0.5", c_SimVersion);
  EnableDefaultStreams();
  // Defect REV-C-15: The limit is checked with "more than 10000", so the event takes 10001 hits before it stops
  Passed = ExpectedToFail(Evaluate("ParseLine()", "hit 10001", "The hit above the limit of 10000 is not parsed: 2", Result, 2)) && Passed;
  Passed = ExpectedToFail(Evaluate("ParseLine()", "hit 10001", "The event stays at 10000 hits", Event.GetNRESEs(), 10000)) && Passed;

  DisableDefaultStreams();
  Result = Event.ParseLine("HTsim 1;1.0;2.0;0.0;100.0;0.5", c_SimVersion);
  EnableDefaultStreams();
  Passed = Evaluate("ParseLine()", "hit 10002", "A hit far above the limit is not parsed: 2", Result, 2) && Passed;
  Passed = EvaluateFalse("ParseLine()", "hit 10002", "An event which lost hits is not valid", Event.IsValid()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing hits with a geometry, which gives the resolutions
bool UTRERawEvent::TestParseLineHitsWithGeometry()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Passed == false) {
    return false;
  }

  // A hit of a sim file in tracker layer 0: the resolutions come from the geometry
  {
    MRERawEvent Event(&Geometry);
    Passed = Evaluate("ParseLine()", "HTsim in the tracker", "The hit is parsed", ParseSilently(Event, "HTsim 1;1.0;2.0;0.0;100.0;0.5", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "HTsim in the tracker", "The event has one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      // The tracker has an energy resolution of 2 keV, strips with a pitch of 1 cm, and a thickness of 0.05 cm
      Passed = EvaluateNear("ParseLine()", "HTsim in the tracker", "The energy resolution is the 2 keV of the tracker", Hit->GetEnergyResolution(), 2.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("ParseLine()", "HTsim in the tracker", "The position resolution is pitch and thickness / sqrt(12)", Hit->GetPositionResolution(), MVector(1.0/sqrt(12.0), 1.0/sqrt(12.0), 0.05/sqrt(12.0)), 1e-14) && Passed;
      Passed = EvaluateTrue("ParseLine()", "HTsim in the tracker", "The hit knows its detector", Hit->GetVolumeSequence()->GetDetector() != nullptr) && Passed;
      if (Hit->GetVolumeSequence()->GetDetector() != nullptr) {
        Passed = Evaluate("ParseLine()", "HTsim in the tracker", "The detector is the tracker", Hit->GetVolumeSequence()->GetDetector()->GetName(), MString("Tracker")) && Passed;
      }
      Passed = EvaluateVectorNear("ParseLine()", "HTsim in the tracker", "The position is not noised", Hit->GetPosition(), MVector(1.0, 2.0, 0.0), 1e-15) && Passed;
      Passed = EvaluateNear("ParseLine()", "HTsim in the tracker", "The energy is not noised", Hit->GetEnergy(), 100.0, 1e-12) && Passed;
    }

    // A second hit in the calorimeter, with 10 keV resolution: the event has sqrt(2^2 + 10^2) keV
    Passed = Evaluate("ParseLine()", "HTsim in the calorimeter", "The hit is parsed", ParseSilently(Event, "HTsim 2;0.5;0.5;-10.0;400.0;0.5", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "HTsim in the calorimeter", "The event has two hits", Event.GetNRESEs(), 2) && Passed;
    Passed = EvaluateNear("ParseLine()", "tracker and calorimeter", "The energy resolution of the event is the quadratic sum", Event.GetEnergyResolution(), sqrt(104.0), 1e-12) && Passed;
    Passed = EvaluateTrue("ParseLine()", "tracker and calorimeter", "The event is valid", Event.IsValid()) && Passed;
  }

  // A hit of an evta file keeps its own resolutions, but gets its volume
  {
    MRERawEvent Event(&Geometry);
    Passed = Evaluate("ParseLine()", "HT in the tracker", "The hit is parsed", ParseSilently(Event, "HT 1;1.0;2.0;1.0;100.0;0.1;0.2;0.3;2.5", c_EvtaVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "HT in the tracker", "The event has one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      Passed = EvaluateNear("ParseLine()", "HT in the tracker", "The energy resolution is the one of the line", Hit->GetEnergyResolution(), 2.5, 1e-15) && Passed;
      Passed = EvaluateVectorNear("ParseLine()", "HT in the tracker", "The position resolution is the one of the line", Hit->GetPositionResolution(), MVector(0.1, 0.2, 0.3), 1e-15) && Passed;
      Passed = EvaluateTrue("ParseLine()", "HT in the tracker", "The hit knows its detector", Hit->GetVolumeSequence()->GetDetector() != nullptr) && Passed;
    }
  }

  // Hits which are in no detector are not used, and the event is not valid
  {
    MRERawEvent Event(&Geometry);
    Passed = Evaluate("ParseLine()", "HTsim in no detector", "A hit of a sim file outside of all detectors is not parsed: 2", ParseSilently(Event, "HTsim 1;1.0;2.0;50.0;100.0;0.5", c_SimVersion), 2) && Passed;
    Passed = Evaluate("ParseLine()", "HTsim in no detector", "The hit is not added", Event.GetNRESEs(), 0) && Passed;
    Passed = EvaluateFalse("ParseLine()", "HTsim in no detector", "The event is not valid", Event.IsValid()) && Passed;
  }
  {
    MRERawEvent Event(&Geometry);
    Passed = Evaluate("ParseLine()", "HT in no detector", "A hit of an evta file outside of all detectors is not parsed: 2", ParseSilently(Event, "HT 1;1.0;2.0;50.0;100.0;0.1;0.2;0.3;2.5", c_EvtaVersion), 2) && Passed;
    Passed = Evaluate("ParseLine()", "HT in no detector", "The hit is not added", Event.GetNRESEs(), 0) && Passed;
    Passed = EvaluateFalse("ParseLine()", "HT in no detector", "The event is not valid", Event.IsValid()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the additional measurements: guard ring, drift chamber, direction, start information
bool UTRERawEvent::TestParseLineMeasurements()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope with guard ring, drift chamber, directional detector", "The telescope can be scanned", ScanTelescopeGeometry(Geometry, GetAdditionalDetectors())) && Passed;
  if (Passed == false) {
    return false;
  }

  // Guard ring hits: "GR x;y;z;energy", and from a sim file "GRsim detector;x;y;z;energy"
  {
    MRERawEvent Event(&Geometry);
    Passed = Evaluate("ParseLine()", "GR 4.8;0.2;1.0;45.5", "The guard ring hit in the tracker is parsed", ParseSilently(Event, "GR 4.8;0.2;1.0;45.5", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "GR 4.8;0.2;1.0;45.5", "There is one additional measurement", Event.GetNREAMs(), 1U) && Passed;
    if (Event.GetNREAMs() == 1) {
      Passed = Evaluate("ParseLine()", "GR 4.8;0.2;1.0;45.5", "It has the type of a guard ring hit", Event.GetREAMAt(0)->GetType(), MREAM::c_GuardRingHit) && Passed;
      MREAMGuardRingHit* GuardRing = dynamic_cast<MREAMGuardRingHit*>(Event.GetREAMAt(0));
      Passed = EvaluateTrue("ParseLine()", "GR 4.8;0.2;1.0;45.5", "It is a guard ring hit", GuardRing != nullptr) && Passed;
      if (GuardRing != nullptr) {
        Passed = EvaluateNear("ParseLine()", "GR 4.8;0.2;1.0;45.5", "The energy is the one of the line - it is not noised", GuardRing->GetEnergy(), 45.5, 1e-12) && Passed;
        Passed = EvaluateNear("ParseLine()", "GR 4.8;0.2;1.0;45.5", "The energy resolution is the 5 keV of the guard ring", GuardRing->GetEnergyResolution(), 5.0, 1e-12) && Passed;
        Passed = EvaluateTrue("ParseLine()", "GR 4.8;0.2;1.0;45.5", "The measurement has a volume sequence with a detector", GuardRing->GetVolumeSequence() != nullptr && GuardRing->GetVolumeSequence()->GetDetector() != nullptr) && Passed;
        if (GuardRing->GetVolumeSequence() != nullptr && GuardRing->GetVolumeSequence()->GetDetector() != nullptr) {
          Passed = Evaluate("ParseLine()", "GR 4.8;0.2;1.0;45.5", "The detector is the guard ring of the tracker", GuardRing->GetVolumeSequence()->GetDetector()->GetType(), MDDetector::c_GuardRing) && Passed;
          // The hit is in layer 1 at z = 1 cm
          Passed = EvaluateVectorNear("ParseLine()", "GR 4.8;0.2;1.0;45.5", "The position in the wafer is the world position minus the position of the layer", GuardRing->GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(4.8, 0.2, 0.0), 1e-12) && Passed;
        }
      }
    }
    Passed = Evaluate("ParseLine()", "GR 4.8;0.2;1.0;45.5", "A guard ring hit is no RESE", Event.GetNRESEs(), 0) && Passed;
    Passed = EvaluateNear("ParseLine()", "GR 4.8;0.2;1.0;45.5", "A guard ring hit does not count for the energy", Event.GetEnergy(), 0.0, 1e-15) && Passed;

    Passed = Evaluate("ParseLine()", "GRsim 1;4.8;0.2;2.0;60", "The guard ring hit of a sim file is parsed", ParseSilently(Event, "GRsim 1;4.8;0.2;2.0;60", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "GRsim 1;4.8;0.2;2.0;60", "There are two additional measurements", Event.GetNREAMs(), 2U) && Passed;
    if (Event.GetNREAMs() == 2) {
      MREAMGuardRingHit* GuardRing = dynamic_cast<MREAMGuardRingHit*>(Event.GetREAMAt(1));
      Passed = EvaluateTrue("ParseLine()", "GRsim 1;4.8;0.2;2.0;60", "It is a guard ring hit", GuardRing != nullptr) && Passed;
      if (GuardRing != nullptr) {
        Passed = EvaluateNear("ParseLine()", "GRsim 1;4.8;0.2;2.0;60", "The energy is the one of the line", GuardRing->GetEnergy(), 60.0, 1e-12) && Passed;
      }
    }

    // Rejected lines are errors and add nothing:
    Passed = Evaluate("ParseLine()", "GR in the calorimeter", "A guard ring hit in a detector without guard ring is an error: 1", ParseSilently(Event, "GR 0;0;-10;40", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "GR in no detector", "A guard ring hit outside of all detectors is an error: 1", ParseSilently(Event, "GR 0;0;50;40", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "GR 4.8;0.2", "A guard ring hit with too few numbers is an error: 1", ParseSilently(Event, "GR 4.8;0.2", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "rejected GR lines", "The rejected lines add no measurement", Event.GetNREAMs(), 2U) && Passed;
    Passed = EvaluateTrue("ParseLine()", "rejected GR lines", "The rejected lines keep the event valid", Event.IsValid()) && Passed;
  }
  // Defect REV-C-16: The lines of the measurements in a detector end the program with an assert if the event has no geometry, while a hit line is read without
  Passed = ExpectedToCrash(EvaluateTrue("ParseLine()", "GR without geometry", "A guard ring hit of an event without geometry is an error: 1", IsGuardRingLineWithoutGeometryRejected())) && Passed;

  // Energy in a drift chamber: "XE x;y;z;energy"
  {
    MRERawEvent Event(&Geometry);
    Passed = Evaluate("ParseLine()", "XE -30;0;0;55.5", "The energy in the drift chamber is parsed", ParseSilently(Event, "XE -30;0;0;55.5", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "XE -30;0;0;55.5", "There is one additional measurement", Event.GetNREAMs(), 1U) && Passed;
    if (Event.GetNREAMs() == 1) {
      Passed = Evaluate("ParseLine()", "XE -30;0;0;55.5", "It has the type of a drift chamber energy", Event.GetREAMAt(0)->GetType(), MREAM::c_DriftChamberEnergy) && Passed;
      MREAMDriftChamberEnergy* Chamber = dynamic_cast<MREAMDriftChamberEnergy*>(Event.GetREAMAt(0));
      Passed = EvaluateTrue("ParseLine()", "XE -30;0;0;55.5", "It is a drift chamber energy", Chamber != nullptr) && Passed;
      if (Chamber != nullptr) {
        Passed = EvaluateNear("ParseLine()", "XE -30;0;0;55.5", "The energy is the one of the line - it is not noised", Chamber->GetEnergy(), 55.5, 1e-12) && Passed;
        Passed = EvaluateNear("ParseLine()", "XE -30;0;0;55.5", "The energy resolution is the 3 keV light energy resolution of the chamber", Chamber->GetEnergyResolution(), 3.0, 1e-12) && Passed;
        Passed = EvaluateTrue("ParseLine()", "XE -30;0;0;55.5", "The measurement has a volume sequence with a detector", Chamber->GetVolumeSequence() != nullptr && Chamber->GetVolumeSequence()->GetDetector() != nullptr) && Passed;
      }
    }
    Passed = Evaluate("ParseLine()", "XE in the tracker", "An energy in a detector which is no drift chamber is an error: 1", ParseSilently(Event, "XE 0;0;0;55", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "XE in no detector", "An energy outside of all detectors is an error: 1", ParseSilently(Event, "XE 0;0;50;55", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "XE -30;0", "An energy with too few numbers is an error: 1", ParseSilently(Event, "XE -30;0", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "rejected XE lines", "The rejected lines add no measurement", Event.GetNREAMs(), 1U) && Passed;
  }
  // Defect REV-C-17: The light energy resolution is asked from a chamber with an ideal one, which throws an exception nobody catches
  Passed = ExpectedToCrash(EvaluateTrue("ParseLine()", "XE in a chamber with ideal light energy resolution", "The energy is read with a resolution of zero", IsDriftChamberEnergyWithIdealResolutionRead())) && Passed;

  // Direction of a charged particle: "DR x;y;z;dx;dy;dz;energy"
  {
    MRERawEvent Event(&Geometry);
    Passed = Evaluate("ParseLine()", "DR 0;30;0;0.6;0;-0.8;77.5", "The direction in the directional detector is parsed", ParseSilently(Event, "DR 0;30;0;0.6;0;-0.8;77.5", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "DR 0;30;0;0.6;0;-0.8;77.5", "There is one additional measurement", Event.GetNREAMs(), 1U) && Passed;
    if (Event.GetNREAMs() == 1) {
      Passed = Evaluate("ParseLine()", "DR 0;30;0;0.6;0;-0.8;77.5", "It has the type of a directional measurement", Event.GetREAMAt(0)->GetType(), MREAM::c_Directional) && Passed;
      MREAMDirectional* Directional = dynamic_cast<MREAMDirectional*>(Event.GetREAMAt(0));
      Passed = EvaluateTrue("ParseLine()", "DR 0;30;0;0.6;0;-0.8;77.5", "It is a directional measurement", Directional != nullptr) && Passed;
      if (Directional != nullptr) {
        Passed = EvaluateVectorNear("ParseLine()", "DR 0;30;0;0.6;0;-0.8;77.5", "The direction is the one of the line - it is not noised", Directional->GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
        Passed = EvaluateNear("ParseLine()", "DR 0;30;0;0.6;0;-0.8;77.5", "The energy is the one of the line", Directional->GetEnergy(), 77.5, 1e-12) && Passed;
        Passed = EvaluateTrue("ParseLine()", "DR 0;30;0;0.6;0;-0.8;77.5", "The measurement is not yet linked to a RESE", Directional->GetRESE() == nullptr) && Passed;
        Passed = EvaluateTrue("ParseLine()", "DR 0;30;0;0.6;0;-0.8;77.5", "The measurement has a volume sequence with a detector", Directional->GetVolumeSequence() != nullptr && Directional->GetVolumeSequence()->GetDetector() != nullptr) && Passed;
      }
    }
    Passed = Evaluate("ParseLine()", "DR in the tracker", "A direction in a detector which is not directional is an error: 1", ParseSilently(Event, "DR 0;0;0;0;0;1;77", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "DR in no detector", "A direction outside of all detectors is an error: 1", ParseSilently(Event, "DR 0;0;50;0;0;1;77", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "DR 0;30;0;0;0;1", "A direction with too few numbers is an error: 1", ParseSilently(Event, "DR 0;30;0;0;0;1", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "rejected DR lines", "The rejected lines add no measurement", Event.GetNREAMs(), 1U) && Passed;
  }

  // Start information of the original particle: position, direction, polarization, energy, and optional the particle ID - it needs no geometry
  {
    MRERawEvent Event;
    Passed = Evaluate("ParseLine()", "OI with particle ID", "The start information is parsed", Event.ParseLine("OI 1.5;-2.5;30;0.6;0;-0.8;0;1;0;1808.7;4", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "OI with particle ID", "There is one additional measurement", Event.GetNREAMs(), 1U) && Passed;
    if (Event.GetNREAMs() == 1) {
      Passed = Evaluate("ParseLine()", "OI with particle ID", "It has the type of a start information", Event.GetREAMAt(0)->GetType(), MREAM::c_StartInformation) && Passed;
      MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(Event.GetREAMAt(0));
      Passed = EvaluateTrue("ParseLine()", "OI with particle ID", "It is a start information", Start != nullptr) && Passed;
      if (Start != nullptr) {
        Passed = EvaluateVectorNear("ParseLine()", "OI with particle ID", "The position is stored", Start->GetPosition(), MVector(1.5, -2.5, 30.0), 1e-15) && Passed;
        Passed = EvaluateVectorNear("ParseLine()", "OI with particle ID", "The direction is stored", Start->GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
        Passed = EvaluateVectorNear("ParseLine()", "OI with particle ID", "The polarization is stored", Start->GetPolarization(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;
        Passed = EvaluateNear("ParseLine()", "OI with particle ID", "The energy is stored", Start->GetEnergy(), 1808.7, 1e-12) && Passed;
        Passed = Evaluate("ParseLine()", "OI with particle ID", "The particle ID is stored", Start->GetParticleID(), 4) && Passed;
      }
    }

    // Older files have no particle ID:
    Passed = Evaluate("ParseLine()", "OI without particle ID", "The start information of an older file is parsed", Event.ParseLine("OI 1.5;-2.5;30;0.6;0;-0.8;0;1;0;1808.7", c_SimVersion), 0) && Passed;
    Passed = Evaluate("ParseLine()", "OI without particle ID", "There are two additional measurements", Event.GetNREAMs(), 2U) && Passed;
    if (Event.GetNREAMs() == 2) {
      MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(Event.GetREAMAt(1));
      Passed = EvaluateTrue("ParseLine()", "OI without particle ID", "It is a start information", Start != nullptr) && Passed;
      if (Start != nullptr) {
        Passed = Evaluate("ParseLine()", "OI without particle ID", "The particle ID is not defined", Start->GetParticleID(), g_IntNotDefined) && Passed;
        Passed = EvaluateNear("ParseLine()", "OI without particle ID", "The energy is stored", Start->GetEnergy(), 1808.7, 1e-12) && Passed;
      }
    }

    Passed = Evaluate("ParseLine()", "OI 1.5;-2.5;30", "A start information with too few numbers is an error: 1", Event.ParseLine("OI 1.5;-2.5;30", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "OI with 9 numbers", "A start information without energy is an error: 1", Event.ParseLine("OI 1.5;-2.5;30;0.6;0;-0.8;0;1;0", c_SimVersion), 1) && Passed;
    Passed = Evaluate("ParseLine()", "rejected OI lines", "The rejected lines add no measurement", Event.GetNREAMs(), 2U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the container of the additional measurements
bool UTRERawEvent::TestMeasurements()
{
  bool Passed = true;

  // The event owns the measurements
  MRERawEvent Event;
  MREAMStartInformation* A = new MREAMStartInformation();
  A->SetEnergy(1.0);
  MREAMStartInformation* B = new MREAMStartInformation();
  B->SetEnergy(2.0);
  MREAMStartInformation* C = new MREAMStartInformation();
  C->SetEnergy(3.0);
  Event.AddREAM(A);
  Passed = Evaluate("AddREAM()", "one", "There is one measurement", Event.GetNREAMs(), 1U) && Passed;
  Passed = EvaluateTrue("GetREAMAt()", "one, index 0", "The measurement is returned", Event.GetREAMAt(0) == A) && Passed;
  Event.AddREAM(B);
  Event.AddREAM(C);
  Passed = Evaluate("AddREAM()", "three", "There are three measurements", Event.GetNREAMs(), 3U) && Passed;
  Passed = EvaluateTrue("GetREAMAt()", "three, index 0", "The first measurement is returned", Event.GetREAMAt(0) == A) && Passed;
  Passed = EvaluateTrue("GetREAMAt()", "three, index 1", "The second measurement is returned", Event.GetREAMAt(1) == B) && Passed;
  Passed = EvaluateTrue("GetREAMAt()", "three, index 2", "The third measurement is returned", Event.GetREAMAt(2) == C) && Passed;

  // Behind the end:
  DisableDefaultStreams();
  MREAM* BehindEnd = Event.GetREAMAt(3);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetREAMAt()", "three, index 3", "The index behind the last measurement returns a nullptr", BehindEnd == nullptr) && Passed;

  // The iterators cover all measurements in their order:
  unsigned int Counter = 0;
  bool InOrder = true;
  for (vector<MREAM*>::iterator Iter = Event.GetREAMBegin(); Iter != Event.GetREAMEnd(); ++Iter) {
    if ((*Iter) != Event.GetREAMAt(Counter)) {
      InOrder = false;
    }
    ++Counter;
  }
  Passed = Evaluate("GetREAMBegin()", "three", "The iterators cover three measurements", Counter, 3U) && Passed;
  Passed = EvaluateTrue("GetREAMBegin()", "three", "The iterators follow the order of the measurements", InOrder) && Passed;

  // Remove the second one: it is not deleted, and the iterator is at the next measurement, as the loops in MERNoising need it
  vector<MREAM*>::iterator Iter = Event.GetREAMBegin();
  ++Iter;
  MREAM* Removed = Event.RemoveREAM(Iter);
  Passed = EvaluateTrue("RemoveREAM()", "second of three", "The removed measurement is returned", Removed == B) && Passed;
  Passed = Evaluate("RemoveREAM()", "second of three", "There are two measurements", Event.GetNREAMs(), 2U) && Passed;
  Passed = EvaluateTrue("RemoveREAM()", "second of three", "The iterator is at the measurement behind the removed one", Iter != Event.GetREAMEnd() && (*Iter) == C) && Passed;
  Passed = EvaluateNear("RemoveREAM()", "second of three", "The removed measurement is not deleted", B->GetEnergy(), 2.0, 1e-15) && Passed;
  delete B;

  // Delete the last one: the iterator is at the end
  Event.DeleteREAM(Iter);
  Passed = Evaluate("DeleteREAM()", "last of two", "There is one measurement", Event.GetNREAMs(), 1U) && Passed;
  Passed = EvaluateTrue("DeleteREAM()", "last of two", "The iterator is at the end", Iter == Event.GetREAMEnd()) && Passed;
  Passed = EvaluateTrue("DeleteREAM()", "last of two", "The first measurement stays", Event.GetREAMAt(0) == A) && Passed;

  // Delete the only one:
  Iter = Event.GetREAMBegin();
  Event.DeleteREAM(Iter);
  Passed = Evaluate("DeleteREAM()", "the only one", "There is no measurement any more", Event.GetNREAMs(), 0U) && Passed;
  Passed = EvaluateTrue("DeleteREAM()", "the only one", "Begin and end are equal", Event.GetREAMBegin() == Event.GetREAMEnd()) && Passed;

  // The container can be filled again:
  Event.AddREAM(new MREAMStartInformation());
  Passed = Evaluate("AddREAM()", "after deleting all", "A measurement can be added again", Event.GetNREAMs(), 1U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the iteration along the links
bool UTRERawEvent::TestIterator()
{
  bool Passed = true;

  // Three hits, linked A - B - C, stored in another order
  MRERawEvent Event;
  MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 1.0);
  MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 200.0, 1.0);
  MREHit* C = CreatePlainHit(MVector(0.0, 0.0, 2.0), 300.0, 1.0);
  Event.AddRESE(C);
  Event.AddRESE(A);
  Event.AddRESE(B);
  Link(A, B);
  Link(B, C);

  // From the start point of the event:
  Event.SetStartPoint(A);
  Passed = EvaluateTrue("ResetIterator()", "start point A", "The iterator is started at the start point", Event.ResetIterator()) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "first call", "The first RESE is the start point", Event.GetNextRESE() == A) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "second call", "The second RESE is the link of the start point", Event.GetNextRESE() == B) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "third call", "The third RESE is the end of the chain", Event.GetNextRESE() == C) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "fourth call", "Behind the end a nullptr is returned", Event.GetNextRESE() == nullptr) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "fifth call", "A nullptr is returned again", Event.GetNextRESE() == nullptr) && Passed;

  // Started again, from the other end:
  Passed = EvaluateTrue("ResetIterator()", "RESE C", "The iterator is started at the given end", Event.ResetIterator(C)) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "first call from C", "The first RESE is C", Event.GetNextRESE() == C) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "second call from C", "The second RESE is B", Event.GetNextRESE() == B) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "third call from C", "The third RESE is A", Event.GetNextRESE() == A) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "fourth call from C", "Behind the end a nullptr is returned", Event.GetNextRESE() == nullptr) && Passed;

  // The iterator does not change the event:
  Passed = Evaluate("GetNextRESE()", "after two iterations", "The event still has three RESEs", Event.GetNRESEs(), 3) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "after two iterations", "The start point is unchanged", Event.GetStartPoint() == A) && Passed;

  // Defect REV-C-18: The iterator ends the program with a fatal error instead of returning false
  Passed = ExpectedToCrash(EvaluateTrue("ResetIterator()", "no start point", "Without a start point the iterator is not started", IsIteratorWithoutStartPointRejected())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("ResetIterator()", "RESE with two links", "The iterator is not started in the middle of a chain", IsIteratorAtMiddleRejected())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the creation of tracks from linked hits
bool UTRERawEvent::TestCreateTracks()
{
  bool Passed = true;

  // Three linked hits A - B - C and a single hit D: the linked hits become one track
  {
    MRERawEvent Event;
    MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 2.0);
    MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 110.0, 2.0);
    MREHit* C = CreatePlainHit(MVector(0.0, 0.0, 2.0), 120.0, 2.0);
    MREHit* D = CreatePlainHit(MVector(5.0, 5.0, -10.0), 400.0, 5.0);
    Event.AddRESE(A);
    Event.AddRESE(D);
    Event.AddRESE(B);
    Event.AddRESE(C);
    Link(A, B);
    Link(B, C);

    Passed = EvaluateTrue("CreateTracks()", "chain of three hits and a single hit", "The tracks are created", Event.CreateTracks()) && Passed;
    Passed = Evaluate("CreateTracks()", "chain of three hits and a single hit", "The event has two RESEs: the hit and the track", Event.GetNRESEs(), 2) && Passed;
    Passed = EvaluateTrue("CreateTracks()", "chain of three hits and a single hit", "The single hit is still in the event", Event.ContainsRESE(D)) && Passed;
    MRETrack* Track = nullptr;
    for (int r = 0; r < Event.GetNRESEs(); ++r) {
      if (Event.GetRESEAt(r)->GetType() == MRESE::c_Track) {
        Track = dynamic_cast<MRETrack*>(Event.GetRESEAt(r));
      }
    }
    Passed = EvaluateTrue("CreateTracks()", "chain of three hits and a single hit", "The event has a track", Track != nullptr) && Passed;
    if (Track != nullptr) {
      Passed = Evaluate("CreateTracks()", "chain of three hits and a single hit", "The track has the three linked hits", Track->GetNRESEs(), 3) && Passed;
      Passed = EvaluateTrue("CreateTracks()", "chain of three hits and a single hit", "The track has the hits A, B, and C", Track->ContainsRESE(A) && Track->ContainsRESE(B) && Track->ContainsRESE(C)) && Passed;
      // 100 keV + 110 keV + 120 keV = 330 keV
      Passed = EvaluateNear("CreateTracks()", "chain of three hits and a single hit", "The energy of the track is the sum of its hits", Track->GetEnergy(), 330.0, 1e-12) && Passed;
      Passed = Evaluate("CreateTracks()", "chain of three hits and a single hit", "The track has two end points", Track->GetNEndPoints(), 2) && Passed;
      if (Track->GetNEndPoints() == 2) {
        bool EndsAreAAndC = false;
        if ((Track->GetEndPointAt(0) == A && Track->GetEndPointAt(1) == C) || (Track->GetEndPointAt(0) == C && Track->GetEndPointAt(1) == A)) {
          EndsAreAAndC = true;
        }
        Passed = EvaluateTrue("CreateTracks()", "chain of three hits and a single hit", "The end points are the ends of the chain", EndsAreAAndC) && Passed;
      }
    }
    Passed = Evaluate("GetTrackLength()", "after CreateTracks", "The track length is three", Event.GetTrackLength(), 3) && Passed;
    // 330 keV + 400 keV = 730 keV
    Passed = EvaluateNear("CreateTracks()", "chain of three hits and a single hit", "The energy of the event is unchanged", Event.GetEnergy(), 730.0, 1e-12) && Passed;

    // A second call finds nothing new:
    Passed = EvaluateTrue("CreateTracks()", "second call", "A second call succeeds", Event.CreateTracks()) && Passed;
    Passed = Evaluate("CreateTracks()", "second call", "The event still has two RESEs", Event.GetNRESEs(), 2) && Passed;
  }

  // Two separate chains become two tracks
  {
    MRERawEvent Event;
    MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 2.0);
    MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 110.0, 2.0);
    MREHit* C = CreatePlainHit(MVector(4.0, 0.0, 0.0), 120.0, 2.0);
    MREHit* D = CreatePlainHit(MVector(4.0, 0.0, 1.0), 130.0, 2.0);
    Event.AddRESE(A);
    Event.AddRESE(C);
    Event.AddRESE(B);
    Event.AddRESE(D);
    Link(A, B);
    Link(C, D);
    Passed = EvaluateTrue("CreateTracks()", "two chains of two hits", "The tracks are created", Event.CreateTracks()) && Passed;
    Passed = Evaluate("CreateTracks()", "two chains of two hits", "The event has two RESEs", Event.GetNRESEs(), 2) && Passed;
    bool BothTracks = true;
    for (int r = 0; r < Event.GetNRESEs(); ++r) {
      if (Event.GetRESEAt(r)->GetType() != MRESE::c_Track || Event.GetRESEAt(r)->GetNRESEs() != 2) {
        BothTracks = false;
      }
    }
    Passed = EvaluateTrue("CreateTracks()", "two chains of two hits", "Both RESEs are tracks with two hits", BothTracks) && Passed;
    if (Event.GetNRESEs() == 2 && BothTracks == true) {
      bool Separated = false;
      if ((Event.GetRESEAt(0)->ContainsRESE(A) && Event.GetRESEAt(0)->ContainsRESE(B) && Event.GetRESEAt(1)->ContainsRESE(C) && Event.GetRESEAt(1)->ContainsRESE(D)) ||
          (Event.GetRESEAt(1)->ContainsRESE(A) && Event.GetRESEAt(1)->ContainsRESE(B) && Event.GetRESEAt(0)->ContainsRESE(C) && Event.GetRESEAt(0)->ContainsRESE(D))) {
        Separated = true;
      }
      Passed = EvaluateTrue("CreateTracks()", "two chains of two hits", "Each track has the hits of one chain", Separated) && Passed;
    }
  }

  // Hits without links stay as they are
  {
    MRERawEvent Event;
    Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 2.0));
    Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 1.0), 110.0, 2.0));
    Passed = EvaluateTrue("CreateTracks()", "two hits without links", "The call succeeds", Event.CreateTracks()) && Passed;
    Passed = Evaluate("CreateTracks()", "two hits without links", "The event still has two RESEs", Event.GetNRESEs(), 2) && Passed;
    Passed = Evaluate("CreateTracks()", "two hits without links", "There is no track", Event.GetTrackLength(), 0) && Passed;

    MRERawEvent Empty;
    Passed = EvaluateTrue("CreateTracks()", "no hits", "The call succeeds on an empty event", Empty.CreateTracks()) && Passed;
  }

  // A link which is not answered is removed: A - B - C and in addition A links to C
  {
    MRERawEvent Event;
    MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 2.0);
    MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 110.0, 2.0);
    MREHit* C = CreatePlainHit(MVector(0.0, 0.0, 2.0), 120.0, 2.0);
    Event.AddRESE(A);
    Event.AddRESE(B);
    Event.AddRESE(C);
    Link(A, B);
    Link(B, C);
    A->AddLink(C);
    Passed = EvaluateTrue("CreateTracks()", "chain with one link which is not answered", "The tracks are created", Event.CreateTracks()) && Passed;
    Passed = Evaluate("CreateTracks()", "chain with one link which is not answered", "The event has one track", Event.GetNRESEs(), 1) && Passed;
    Passed = Evaluate("FindEndpoints()", "chain with one link which is not answered", "The link which is not answered is removed", A->GetNLinks(), 1) && Passed;
    Passed = EvaluateTrue("FindEndpoints()", "chain with one link which is not answered", "The answered link stays", A->IsLink(B)) && Passed;
    Passed = Evaluate("FindEndpoints()", "chain with one link which is not answered", "The middle keeps its two links", B->GetNLinks(), 2) && Passed;
    Passed = Evaluate("FindEndpoints()", "chain with one link which is not answered", "The other end keeps its link", C->GetNLinks(), 1) && Passed;
  }

  // An only link which is not answered is answered: A links to B, but B not to A
  {
    MRERawEvent Event;
    MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 2.0);
    MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 110.0, 2.0);
    Event.AddRESE(A);
    Event.AddRESE(B);
    A->AddLink(B);
    Passed = EvaluateTrue("CreateTracks()", "two hits with a link in one direction", "The tracks are created", Event.CreateTracks()) && Passed;
    Passed = Evaluate("CreateTracks()", "two hits with a link in one direction", "The event has one track", Event.GetNRESEs(), 1) && Passed;
    Passed = EvaluateTrue("FindEndpoints()", "two hits with a link in one direction", "The link is answered", B->IsLink(A)) && Passed;
    if (Event.GetNRESEs() == 1 && Event.GetRESEAt(0)->GetType() == MRESE::c_Track) {
      Passed = Evaluate("FindEndpoints()", "two hits with a link in one direction", "The track has two end points", dynamic_cast<MRETrack*>(Event.GetRESEAt(0))->GetNEndPoints(), 2) && Passed;
    }
  }

  // A ring of three hits has no ends: no tracks
  {
    MRERawEvent Event;
    MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 2.0);
    MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 110.0, 2.0);
    MREHit* C = CreatePlainHit(MVector(0.0, 1.0, 1.0), 120.0, 2.0);
    Event.AddRESE(A);
    Event.AddRESE(B);
    Event.AddRESE(C);
    Link(A, B);
    Link(B, C);
    Link(C, A);
    Passed = EvaluateFalse("CreateTracks()", "ring of three hits", "A ring is rejected", Event.CreateTracks()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the kinematics check of the electron direction
bool UTRERawEvent::TestTestElectronDirection()
{
  bool Passed = true;

  // The function gets the energy of the electron and of the scattered photon, and accepts what Compton kinematics allows:
  // cos(phi) = 1 - E0 (1/Eg - 1/(Ee + Eg)) has to be between -1 and 1
  MRERawEvent Event;

  // 500 keV + 500 keV: cos(phi) = 1 - 511 (1/500 - 1/1000) = 0.489
  Passed = EvaluateTrue("TestElectronDirection()", "Ee = 500 keV, Eg = 500 keV", "Allowed kinematics is accepted", Event.TestElectronDirection(500.0, 500.0)) && Passed;

  // 900 keV + 100 keV: cos(phi) = 1 - 511 (1/100 - 1/1000) = -3.6
  Passed = EvaluateFalse("TestElectronDirection()", "Ee = 900 keV, Eg = 100 keV", "An electron energy above the Compton edge is rejected", Event.TestElectronDirection(900.0, 100.0)) && Passed;

  // The limit for a total of 1000 keV is the back scattering at Eg = 1000 / (1 + 2*1000/511) keV = 203.5 keV:
  // Eg = 204 keV: cos(phi) = 1 - 511.004 (1/204 - 1/1000) = -0.9939, Eg = 203 keV: cos(phi) = -1.0063
  Passed = EvaluateTrue("TestElectronDirection()", "Ee = 796 keV, Eg = 204 keV", "Just below the Compton edge is accepted", Event.TestElectronDirection(796.0, 204.0)) && Passed;
  Passed = EvaluateFalse("TestElectronDirection()", "Ee = 797 keV, Eg = 203 keV", "Just above the Compton edge is rejected", Event.TestElectronDirection(797.0, 203.0)) && Passed;

  // No photon energy:
  Passed = EvaluateFalse("TestElectronDirection()", "Ee = 100 keV, Eg = 0 keV", "A photon without energy is rejected", Event.TestElectronDirection(100.0, 0.0)) && Passed;

  // 150 keV + 150 keV: cos(phi) = 1 - 511 (1/150 - 1/300) = -0.703, which is allowed (the Compton edge of 300 keV is at 162 keV)
  // The electron angle: cos(theta) = Ee (Ei + E0) / (Ei sqrt(Ee^2 + 2 Ee E0)) = 150*811/(300*sqrt(22500 + 153300)) = 0.967
  // Defect REV-C-19: The electron angle is calculated with the total energy in place of the electron rest mass in the momentum, sqrt(Ee^2 + 2 Ee Ei),
  // which gives a cosine above 1 for allowed kinematics below 511 keV
  Passed = ExpectedToFail(EvaluateTrue("TestElectronDirection()", "Ee = 150 keV, Eg = 150 keV", "Allowed kinematics below 511 keV is accepted", Event.TestElectronDirection(150.0, 150.0))) && Passed;
  // 60 keV + 140 keV: cos(phi) = 1 - 511 (1/140 - 1/200) = -0.095, the Compton edge of 200 keV is at 87.8 keV
  Passed = ExpectedToFail(EvaluateTrue("TestElectronDirection()", "Ee = 60 keV, Eg = 140 keV", "Allowed kinematics at 200 keV is accepted", Event.TestElectronDirection(60.0, 140.0))) && Passed;

  // The function does not use the content of the event:
  Passed = Evaluate("TestElectronDirection()", "empty event", "The event is unchanged", Event.GetNRESEs(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the physical event of a single hit
bool UTRERawEvent::TestPhysicalEventPhoto()
{
  bool Passed = true;

  MRERawEvent Event;
  MREHit* Hit = CreatePlainHit(MVector(1.0, 2.0, 3.0), 661.7, 2.0);
  Event.AddRESE(Hit);
  Event.SetEventType(MRERawEvent::c_PhotoEvent);
  Event.SetGoodEvent(true);
  Event.SetEventID(42);
  Event.SetEventTime(MTime(12.5));
  Event.SetDecay(true);
  Event.AddBadFlag("Saturated");
  Event.SetOriginInformation(MVector(1.5, -2.5, 30.0), MVector(0.6, 0.0, -0.8), MVector(0.0, 1.0, 0.0), 1808.7, 4);
  Event.ParseLine("TW 5", c_SimVersion);
  Event.ParseLine("CC A comment", c_SimVersion);
  Event.ParseLine("RX 0 1 0", c_SimVersion);
  Event.ParseLine("RZ -1 0 0", c_SimVersion);

  MPhysicalEvent* Physical = Event.GetPhysicalEvent();
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event", "A physical event is returned", Physical != nullptr) && Passed;
  if (Physical == nullptr) {
    return false;
  }
  Passed = Evaluate("GetPhysicalEvent()", "photo event", "It is a photo event", Physical->GetType(), MPhysicalEvent::c_Photo) && Passed;
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event", "It is a MPhotoEvent", dynamic_cast<MPhotoEvent*>(Physical) != nullptr) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "photo event", "The energy is the one of the hit", Physical->GetEnergy(), 661.7, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "photo event", "The position is the one of the hit", Physical->GetPosition(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;

  // The data of the event:
  Passed = Evaluate("GetPhysicalEvent()", "photo event", "The ID is the event ID", Physical->GetId(), 42L) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "photo event", "The time is the event time", Physical->GetTime().GetAsSeconds(), 12.5, 1e-9) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "photo event", "The time walk is handed over", Physical->GetTimeWalk(), 5) && Passed;
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event", "The decay flag is handed over", Physical->IsDecay()) && Passed;
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event", "All hits are good", Physical->AllHitsGood()) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "photo event", "There is one bad flag", Physical->GetNBadFlags(), 1U) && Passed;
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event", "The bad flag of the raw event is handed over", Physical->HasBadFlag("Saturated")) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "photo event", "There is one comment", Physical->GetNComments(), 1U) && Passed;
  if (Physical->GetNComments() == 1) {
    Passed = Evaluate("GetPhysicalEvent()", "photo event", "The comment is handed over", Physical->GetComment(0), MString("A comment")) && Passed;
  }
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event", "The detector rotation is handed over", Physical->HasDetectorRotation()) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "photo event", "The x axis of the detector rotation is handed over", Physical->GetDetectorRotationXAxis(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "photo event", "The z axis of the detector rotation is handed over", Physical->GetDetectorRotationZAxis(), MVector(-1.0, 0.0, 0.0), 1e-15) && Passed;

  // The origin information:
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "photo event", "The origin position is handed over", Physical->GetOIPosition(), MVector(1.5, -2.5, 30.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "photo event", "The origin direction is handed over", Physical->GetOIDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "photo event", "The origin polarization is handed over", Physical->GetOIPolarization(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "photo event", "The origin energy is handed over", Physical->GetOIEnergy(), 1808.7, 1e-12) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "photo event", "The origin particle ID is handed over", Physical->GetOIParticleID(), 4) && Passed;

  // A second call gives the same event, and nothing is doubled:
  MPhysicalEvent* Again = Event.GetPhysicalEvent();
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event, second call", "The same physical event is returned", Again == Physical) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "photo event, second call", "There is still one bad flag", Again->GetNBadFlags(), 1U) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "photo event, second call", "There is still one comment", Again->GetNComments(), 1U) && Passed;

  // Changes of the raw event reach the physical event with the next call:
  Event.AddBadFlag("PileUp");
  Event.SetDecay(false);
  Event.SetEventID(43);
  Hit->SetValid(false);
  Again = Event.GetPhysicalEvent();
  Passed = Evaluate("GetPhysicalEvent()", "photo event, changed raw event", "The new bad flag is added", Again->GetNBadFlags(), 2U) && Passed;
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event, changed raw event", "The new bad flag is found", Again->HasBadFlag("PileUp")) && Passed;
  Passed = EvaluateFalse("GetPhysicalEvent()", "photo event, changed raw event", "The decay flag follows", Again->IsDecay()) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "photo event, changed raw event", "The ID follows", Again->GetId(), 43L) && Passed;
  Passed = EvaluateFalse("GetPhysicalEvent()", "photo event, hit not valid", "Not all hits are good any more", Again->AllHitsGood()) && Passed;

  // The photo event does not make the raw event good or bad:
  Passed = EvaluateTrue("GetPhysicalEvent()", "photo event", "The raw event is still good", Event.IsGoodEvent()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the physical event of a Compton sequence of hits
bool UTRERawEvent::TestPhysicalEventCompton()
{
  bool Passed = true;

  // Two hits: 200 +- 2 keV at the origin at 0 s, then 300 +- 4 keV at (3, 0, -4) cm at 1 ns
  {
    MRERawEvent Event;
    FillComptonEvent(Event);
    Event.SetEventID(42);
    Event.SetEventTime(MTime(12.5));
    Event.SetCoincidenceWindow(MTime(2.0e-6));
    Event.SetClusteringQualityFactor(0.125);
    Event.SetComptonQualityFactors(0.25, 0.5);

    MPhysicalEvent* Physical = Event.GetPhysicalEvent();
    Passed = EvaluateTrue("GetPhysicalEvent()", "Compton event with two hits", "A physical event is returned", Physical != nullptr) && Passed;
    MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Physical);
    Passed = EvaluateTrue("GetPhysicalEvent()", "Compton event with two hits", "It is a MComptonEvent", Compton != nullptr) && Passed;
    if (Compton == nullptr) {
      return false;
    }
    Passed = Evaluate("GetPhysicalEvent()", "Compton event with two hits", "The type is Compton", Compton->GetType(), MPhysicalEvent::c_Compton) && Passed;

    // Energies: the electron has the energy of the start point, the photon the rest
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The electron energy is the energy of the first hit", Compton->Ee(), 200.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The photon energy is the energy of the second hit", Compton->Eg(), 300.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The total energy is 500 keV", Compton->GetEnergy(), 500.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The electron energy resolution is the one of the first hit", Compton->dEe(), 2.0, 1e-12) && Passed;
    // The resolution of the event is sqrt(2^2 + 4^2) keV, without the first hit sqrt(20 - 4) keV = 4 keV
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The photon energy resolution is the one of the second hit", Compton->dEg(), 4.0, 1e-12) && Passed;

    // Positions:
    Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with two hits", "The first interaction is at the first hit", Compton->C1(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
    Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with two hits", "The second interaction is at the second hit", Compton->C2(), MVector(3.0, 0.0, -4.0), 1e-15) && Passed;
    Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with two hits", "The resolution of the first position is the one of the hit", Compton->dC1(), MVector(0.1, 0.2, 0.3), 1e-15) && Passed;
    Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with two hits", "The resolution of the second position is the one of the hit", Compton->dC2(), MVector(0.1, 0.2, 0.3), 1e-15) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The lever arm is the distance of the hits: 5 cm", Compton->LeverArm(), 5.0, 1e-12) && Passed;

    // Kinematics: cos(phi) = 1 - E0 (1/Eg - 1/(Ee + Eg))
    double ExpectedPhi = acos(1.0 - c_E0*(1.0/300.0 - 1.0/500.0));
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The Compton scatter angle follows from the energies", Compton->Phi(), ExpectedPhi, 1e-12) && Passed;

    // No track:
    Passed = EvaluateFalse("GetPhysicalEvent()", "Compton event with two hits", "The event has no track", Compton->HasTrack()) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "Compton event with two hits", "The track length is one", Compton->TrackLength(), 1U) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The initial deposit is the energy of the first hit", Compton->TrackInitialDeposit(), 200.0, 1e-12) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "Compton event with two hits", "The sequence length is two", Compton->SequenceLength(), 2U) && Passed;

    // Time of flight: 1 ns between the hits, each with 1 ns resolution
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The time of flight is the time difference of the hits", Compton->ToF(), 1.0e-9, 1e-21) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The resolution of the time of flight is the quadratic sum", Compton->dToF(), sqrt(2.0)*1.0e-9, 1e-21) && Passed;

    // Quality factors and the data of the event:
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The clustering quality factor is handed over", Compton->ClusteringQualityFactor(), 0.125, 1e-15) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The first Compton quality factor is handed over", Compton->ComptonQualityFactor1(), 0.25, 1e-15) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The second Compton quality factor is handed over", Compton->ComptonQualityFactor2(), 0.5, 1e-15) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The coincidence window is handed over", Compton->CoincidenceWindow().GetAsSeconds(), 2.0e-6, 1e-12) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "Compton event with two hits", "The ID is the event ID", Compton->GetId(), 42L) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The time is the event time", Compton->GetTime().GetAsSeconds(), 12.5, 1e-9) && Passed;

    // The hits:
    Passed = Evaluate("GetPhysicalEvent()", "Compton event with two hits", "The physical event has two hits", Compton->GetNHits(), 2U) && Passed;
    if (Compton->GetNHits() == 2) {
      Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with two hits", "The first hit has the energy of the start point", Compton->GetHit(0).GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with two hits", "The second hit has the position of the second hit", Compton->GetHit(1).GetPosition(), MVector(3.0, 0.0, -4.0), 1e-15) && Passed;
    }
    Passed = Evaluate("GetPhysicalEvent()", "Compton event with two hits", "A good Compton event has no bad flag", Compton->GetNBadFlags(), 0U) && Passed;
    Passed = EvaluateFalse("GetPhysicalEvent()", "Compton event with two hits", "The raw event got no bad flag", Event.IsBad()) && Passed;
  }

  // A hit without depth information marks the event
  {
    MRERawEvent Event;
    FillComptonEvent(Event);
    Event.GetRESEAt(1)->SetNoiseFlags(" NODEPTH");
    MPhysicalEvent* Physical = Event.GetPhysicalEvent();
    Passed = EvaluateTrue("GetPhysicalEvent()", "Compton event, second hit without depth", "The raw event gets the bad flag NODEPTH", Event.HasBadFlag("NODEPTH")) && Passed;
    Passed = EvaluateTrue("GetPhysicalEvent()", "Compton event, second hit without depth", "The physical event gets the bad flag NODEPTH", Physical->HasBadFlag("NODEPTH")) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "Compton event, second hit without depth", "It is the only bad flag", Physical->GetNBadFlags(), 1U) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "Compton event, second hit without depth", "It is still a Compton event", Physical->GetType(), MPhysicalEvent::c_Compton) && Passed;

    // A second call does not double the flag:
    Physical = Event.GetPhysicalEvent();
    Passed = Evaluate("GetPhysicalEvent()", "Compton event, second hit without depth, second call", "The raw event still has one bad flag", Event.GetNBadFlags(), 1U) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "Compton event, second hit without depth, second call", "The physical event still has one bad flag", Physical->GetNBadFlags(), 1U) && Passed;
  }

  // Three hits A - B - C, stored in another order: the photon energy is the sum of B and C
  {
    MRERawEvent Event;
    MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0);
    MREHit* B = CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0);
    MREHit* C = CreatePlainHit(MVector(3.0, 0.0, -6.0), 100.0, 3.0);
    Event.AddRESE(C);
    Event.AddRESE(A);
    Event.AddRESE(B);
    Link(A, B);
    Link(B, C);
    Event.SetStartPoint(A);
    Event.SetEventType(MRERawEvent::c_ComptonEvent);
    Event.SetGoodEvent(true);

    MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Event.GetPhysicalEvent());
    Passed = EvaluateTrue("GetPhysicalEvent()", "Compton event with three hits", "It is a MComptonEvent", Compton != nullptr) && Passed;
    if (Compton != nullptr) {
      Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with three hits", "The electron energy is the energy of the start point", Compton->Ee(), 200.0, 1e-12) && Passed;
      Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with three hits", "The photon energy is the sum of the other hits: 400 keV", Compton->Eg(), 400.0, 1e-12) && Passed;
      // sqrt(4^2 + 3^2) keV = 5 keV
      Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with three hits", "The photon energy resolution is the quadratic sum of the other hits: 5 keV", Compton->dEg(), 5.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with three hits", "The first interaction is at the start point", Compton->C1(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
      Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with three hits", "The second interaction is at the link of the start point", Compton->C2(), MVector(3.0, 0.0, -4.0), 1e-15) && Passed;
      // The distances are 5 cm and 2 cm
      Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with three hits", "The lever arm is the shortest distance in the sequence: 2 cm", Compton->LeverArm(), 2.0, 1e-12) && Passed;
      Passed = Evaluate("GetPhysicalEvent()", "Compton event with three hits", "The sequence length is three", Compton->SequenceLength(), 3U) && Passed;
      Passed = Evaluate("GetPhysicalEvent()", "Compton event with three hits", "The physical event has three hits", Compton->GetNHits(), 3U) && Passed;
      if (Compton->GetNHits() == 3) {
        Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with three hits", "The hits are in the order of the sequence: 200 keV", Compton->GetHit(0).GetEnergy(), 200.0, 1e-12) && Passed;
        Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with three hits", "The hits are in the order of the sequence: 300 keV", Compton->GetHit(1).GetEnergy(), 300.0, 1e-12) && Passed;
        Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with three hits", "The hits are in the order of the sequence: 100 keV", Compton->GetHit(2).GetEnergy(), 100.0, 1e-12) && Passed;
      }
      double ExpectedPhi = acos(1.0 - c_E0*(1.0/400.0 - 1.0/600.0));
      Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with three hits", "The Compton scatter angle follows from the energies", Compton->Phi(), ExpectedPhi, 1e-12) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the physical event of a Compton sequence which starts with a track
bool UTRERawEvent::TestPhysicalEventComptonWithTrack()
{
  bool Passed = true;

  // A track of two hits going down, then a hit of the scattered photon
  MRERawEvent Event;
  MREHit* T1 = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 1.5);
  MREHit* T2 = CreatePlainHit(MVector(0.0, 0.6, -0.8), 150.0, 2.0);
  Link(T1, T2);
  MRETrack* Track = new MRETrack();
  Track->AddRESE(T1);
  Track->AddRESE(T2);
  Track->SetStartPoint(T1);
  MREHit* B = CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0);
  Event.AddRESE(Track);
  Event.AddRESE(B);
  Link(Track, B);
  Event.SetStartPoint(Track);
  Event.SetEventType(MRERawEvent::c_ComptonEvent);
  Event.SetGoodEvent(true);
  Event.SetTrackQualityFactor(0.75);

  MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Event.GetPhysicalEvent());
  Passed = EvaluateTrue("GetPhysicalEvent()", "Compton event with track", "It is a MComptonEvent", Compton != nullptr) && Passed;
  if (Compton == nullptr) {
    return false;
  }
  Passed = EvaluateTrue("GetPhysicalEvent()", "Compton event with track", "The event has a track", Compton->HasTrack()) && Passed;
  // 100 keV + 150 keV = 250 keV, resolution sqrt(1.5^2 + 2^2) keV = 2.5 keV
  Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with track", "The electron energy is the energy of the track", Compton->Ee(), 250.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with track", "The electron energy resolution is the one of the track", Compton->dEe(), 2.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with track", "The photon energy is the energy of the hit", Compton->Eg(), 300.0, 1e-12) && Passed;
  // sqrt(2.5^2 + 4^2 - 2.5^2) keV = 4 keV
  Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with track", "The photon energy resolution is the one of the hit", Compton->dEg(), 4.0, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with track", "The first interaction is at the start of the track", Compton->C1(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with track", "The second interaction is at the hit", Compton->C2(), MVector(3.0, 0.0, -4.0), 1e-15) && Passed;
  // The direction is from the first to the second hit of the track, which have a distance of 1 cm
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "Compton event with track", "The electron direction is the direction of the track", Compton->De(), MVector(0.0, 0.6, -0.8), 1e-14) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "Compton event with track", "The track length is the number of hits of the track", Compton->TrackLength(), 2U) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with track", "The initial deposit is the energy of the first hit of the track", Compton->TrackInitialDeposit(), 100.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with track", "The track quality factor is handed over", Compton->TrackQualityFactor1(), 0.75, 1e-15) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "Compton event with track", "The sequence length is two: track and hit", Compton->SequenceLength(), 2U) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "Compton event with track", "The resolution of the first position is the one of the start of the track", (Compton->dC1() - MVector(0.1, 0.2, 0.3)).Mag(), 0.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the physical event of a pair
bool UTRERawEvent::TestPhysicalEventPair()
{
  bool Passed = true;

  // The vertex and two tracks which start below it
  MRERawEvent Event;
  MREHit* Vertex = CreatePlainHit(MVector(0.0, 0.0, 3.0), 50.0, 1.0);

  MREHit* E1 = CreatePlainHit(MVector(0.0, 0.0, 2.0), 110.0, 1.0);
  MREHit* E2 = CreatePlainHit(MVector(0.6, 0.0, 1.2), 120.0, 1.0);
  Link(E1, E2);
  MRETrack* Electron = new MRETrack();
  Electron->AddRESE(E1);
  Electron->AddRESE(E2);
  Electron->SetStartPoint(E1);
  Electron->SetQualityFactor(0.25);

  MREHit* P1 = CreatePlainHit(MVector(0.0, 0.0, 2.0), 210.0, 1.0);
  MREHit* P2 = CreatePlainHit(MVector(0.0, -0.8, 1.4), 220.0, 1.0);
  Link(P1, P2);
  MRETrack* Positron = new MRETrack();
  Positron->AddRESE(P1);
  Positron->AddRESE(P2);
  Positron->SetStartPoint(P1);
  Positron->SetQualityFactor(0.75);

  Event.AddRESE(Vertex);
  Event.AddRESE(Electron);
  Event.AddRESE(Positron);
  Event.SetVertex(Vertex);
  Event.SetVertexDirection(-1);
  Event.SetElectronTrack(Electron);
  Event.SetPositronTrack(Positron);
  Event.SetEventType(MRERawEvent::c_PairEvent);
  Event.SetGoodEvent(true);
  Event.SetEventID(42);

  MPhysicalEvent* Physical = Event.GetPhysicalEvent();
  Passed = EvaluateTrue("GetPhysicalEvent()", "pair event", "A physical event is returned", Physical != nullptr) && Passed;
  MPairEvent* Pair = dynamic_cast<MPairEvent*>(Physical);
  Passed = EvaluateTrue("GetPhysicalEvent()", "pair event", "It is a MPairEvent", Pair != nullptr) && Passed;
  if (Pair == nullptr) {
    return false;
  }
  Passed = Evaluate("GetPhysicalEvent()", "pair event", "The type is pair", Pair->GetType(), MPhysicalEvent::c_Pair) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "pair event", "The pair creation is at the start of the electron track", Pair->GetPairCreationIA(), MVector(0.0, 0.0, 2.0), 1e-15) && Passed;
  // The directions are from the first to the second hit of the tracks, which have a distance of 1 cm
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "pair event", "The electron direction is the direction of the electron track", Pair->GetElectronDirection(), MVector(0.6, 0.0, -0.8), 1e-14) && Passed;
  Passed = EvaluateVectorNear("GetPhysicalEvent()", "pair event", "The positron direction is the direction of the positron track", Pair->GetPositronDirection(), MVector(0.0, -0.8, -0.6), 1e-14) && Passed;
  // 110 keV + 120 keV = 230 keV and 210 keV + 220 keV = 430 keV
  Passed = EvaluateNear("GetPhysicalEvent()", "pair event", "The electron energy is the energy of the electron track", Pair->GetEnergyElectron(), 230.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "pair event", "The positron energy is the energy of the positron track", Pair->GetEnergyPositron(), 430.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhysicalEvent()", "pair event", "The initial deposit is the energy of the vertex", Pair->GetInitialEnergyDeposit(), 50.0, 1e-12) && Passed;
  // 0.5*0.25 + 0.5*0.75 = 0.5
  Passed = EvaluateNear("GetPhysicalEvent()", "pair event", "The track quality factor is the mean of both tracks", Pair->GetTrackQualityFactor(), 0.5, 1e-15) && Passed;
  Passed = Evaluate("GetPhysicalEvent()", "pair event", "The ID is the event ID", Pair->GetId(), 42L) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the physical event of an event which was not identified
bool UTRERawEvent::TestPhysicalEventUnidentifiable()
{
  bool Passed = true;

  // Two hits and a rejection reason:
  {
    MRERawEvent Event;
    Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0));
    Event.AddRESE(CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0));
    Event.SetAdditionalEnergy(11.0);
    Event.SetEventID(42);
    Event.SetRejectionReason(MRERawEvent::c_RejectionTooManyHits);
    Event.AddBadFlag("Saturated");

    MPhysicalEvent* Physical = Event.GetPhysicalEvent();
    Passed = EvaluateTrue("GetPhysicalEvent()", "rejected event with two hits", "A physical event is returned", Physical != nullptr) && Passed;
    if (Physical == nullptr) {
      return false;
    }
    Passed = Evaluate("GetPhysicalEvent()", "rejected event with two hits", "It is an unidentifiable event", Physical->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
    Passed = EvaluateTrue("GetPhysicalEvent()", "rejected event with two hits", "It is a MUnidentifiableEvent", dynamic_cast<MUnidentifiableEvent*>(Physical) != nullptr) && Passed;
    // 200 keV + 300 keV + 11 keV = 511 keV
    Passed = EvaluateNear("GetPhysicalEvent()", "rejected event with two hits", "The energy is the energy of the raw event", Physical->GetEnergy(), 511.0, 1e-12) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "rejected event with two hits", "The ID is the event ID", Physical->GetId(), 42L) && Passed;
    Passed = EvaluateTrue("GetPhysicalEvent()", "rejected event with two hits", "The short rejection reason is a bad flag", Physical->HasBadFlag("TooManyOverallHits")) && Passed;
    Passed = EvaluateTrue("GetPhysicalEvent()", "rejected event with two hits", "The bad flag of the raw event is handed over", Physical->HasBadFlag("Saturated")) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "rejected event with two hits", "There are two bad flags", Physical->GetNBadFlags(), 2U) && Passed;
    Passed = EvaluateFalse("GetPhysicalEvent()", "rejected event with two hits", "A rejected event has not only good hits", Physical->AllHitsGood()) && Passed;
    Passed = EvaluateFalse("GetPhysicalEvent()", "rejected event with two hits", "The raw event does not become good", Event.IsGoodEvent()) && Passed;
    Passed = EvaluateFalse("GetPhysicalEvent()", "rejected event with two hits", "The rejection reason is no bad flag of the raw event", Event.HasBadFlag("TooManyOverallHits")) && Passed;
  }

  // Rejected for an external flag: the flag itself is the reason
  {
    MRERawEvent Event;
    Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0));
    Event.SetRejectionReason(MRERawEvent::c_RejectionExternalBadEventFlag);
    Event.AddBadFlag("Saturated");
    MPhysicalEvent* Physical = Event.GetPhysicalEvent();
    Passed = Evaluate("GetPhysicalEvent()", "event rejected for an external flag", "It is an unidentifiable event", Physical->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
    Passed = Evaluate("GetPhysicalEvent()", "event rejected for an external flag", "The external flag is the only bad flag", Physical->GetNBadFlags(), 1U) && Passed;
    Passed = EvaluateTrue("GetPhysicalEvent()", "event rejected for an external flag", "The external flag is handed over", Physical->HasBadFlag("Saturated")) && Passed;
  }

  // Event types without a physical event of their own:
  {
    vector<int> Types = { MRERawEvent::c_MipEvent, MRERawEvent::c_ShowerEvent, 999 };
    vector<MString> Names = { "MIP event", "shower event", "event type 999" };
    for (unsigned int t = 0; t < Types.size(); ++t) {
      MRERawEvent Event;
      Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0));
      Event.SetEventType(Types[t]);
      MPhysicalEvent* Physical = Event.GetPhysicalEvent();
      Passed = Evaluate("GetPhysicalEvent()", Names[t], "It is an unidentifiable event", Physical->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
      Passed = EvaluateNear("GetPhysicalEvent()", Names[t], "The energy is the energy of the raw event", Physical->GetEnergy(), 200.0, 1e-12) && Passed;
      Physical = Event.GetPhysicalEvent();
      Passed = EvaluateNear("GetPhysicalEvent()", Names[t] + ", second call", "A second call gives the same energy", Physical->GetEnergy(), 200.0, 1e-12) && Passed;
    }
  }

  // An event without hits:
  {
    MRERawEvent Event;
    MPhysicalEvent* Physical = Event.GetPhysicalEvent();
    Passed = Evaluate("GetPhysicalEvent()", "new event", "A new event gives an unidentifiable event", Physical->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
    Passed = EvaluateNear("GetPhysicalEvent()", "new event", "The energy is zero", Physical->GetEnergy(), 0.0, 1e-15) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test setting the physical event from outside
bool UTRERawEvent::TestSetPhysicalEvent()
{
  bool Passed = true;

  // The raw event takes the ownership of the physical event
  {
    MRERawEvent Event;
    Event.AddRESE(CreatePlainHit(MVector(1.0, 2.0, 3.0), 661.7, 2.0));
    Event.SetEventType(MRERawEvent::c_PhotoEvent);
    MPhotoEvent* Photo = new MPhotoEvent();
    Photo->SetEnergy(123.0);
    Photo->AddBadFlag("FromOutside");
    Event.SetPhysicalEvent(Photo);
    Passed = EvaluateTrue("SetPhysicalEvent()", "photo event", "Setting a physical event makes the raw event good", Event.IsGoodEvent()) && Passed;
    MPhysicalEvent* Physical = Event.GetPhysicalEvent();
    Passed = EvaluateTrue("SetPhysicalEvent()", "photo event", "The physical event which was set is returned", Physical == Photo) && Passed;
    Passed = EvaluateNear("SetPhysicalEvent()", "photo event", "Its energy is not replaced by the energy of the hits", Physical->GetEnergy(), 123.0, 1e-12) && Passed;
    Passed = EvaluateTrue("SetPhysicalEvent()", "photo event", "Its own bad flag is kept", Physical->HasBadFlag("FromOutside")) && Passed;

    // The data of the raw event is added:
    Event.SetEventID(42);
    Event.AddBadFlag("Saturated");
    Physical = Event.GetPhysicalEvent();
    Passed = Evaluate("SetPhysicalEvent()", "photo event", "The ID of the raw event is set", Physical->GetId(), 42L) && Passed;
    Passed = Evaluate("SetPhysicalEvent()", "photo event", "The bad flag of the raw event is added to its own", Physical->GetNBadFlags(), 2U) && Passed;

    // A second physical event replaces the first one, which is deleted:
    MPhotoEvent* Second = new MPhotoEvent();
    Second->SetEnergy(456.0);
    Event.SetPhysicalEvent(Second);
    Physical = Event.GetPhysicalEvent();
    Passed = EvaluateTrue("SetPhysicalEvent()", "second photo event", "The second physical event is returned", Physical == Second) && Passed;
    Passed = EvaluateNear("SetPhysicalEvent()", "second photo event", "It has its own energy", Physical->GetEnergy(), 456.0, 1e-12) && Passed;
  }

  // A physical event of another type than the raw event is replaced by one of the type of the raw event
  {
    MRERawEvent Event;
    Event.AddRESE(CreatePlainHit(MVector(1.0, 2.0, 3.0), 661.7, 2.0));
    MUnidentifiableEvent* Unidentifiable = new MUnidentifiableEvent();
    Unidentifiable->SetEnergy(123.0);
    Event.SetPhysicalEvent(Unidentifiable);
    Event.SetEventType(MRERawEvent::c_PhotoEvent);
    MPhysicalEvent* Physical = Event.GetPhysicalEvent();
    Passed = Evaluate("SetPhysicalEvent()", "unidentifiable event, then the type photo", "The physical event is a photo event", Physical->GetType(), MPhysicalEvent::c_Photo) && Passed;
    Passed = EvaluateNear("SetPhysicalEvent()", "unidentifiable event, then the type photo", "It has the energy of the hit", Physical->GetEnergy(), 661.7, 1e-12) && Passed;
  }

  // An unidentifiable event for a raw event which is not identified
  {
    MRERawEvent Event;
    Event.AddRESE(CreatePlainHit(MVector(1.0, 2.0, 3.0), 661.7, 2.0));
    MUnidentifiableEvent* Unidentifiable = new MUnidentifiableEvent();
    Unidentifiable->SetEnergy(123.0);
    Unidentifiable->AddBadFlag("FromOutside");
    Event.SetPhysicalEvent(Unidentifiable);
    MPhysicalEvent* Physical = Event.GetPhysicalEvent();
    Passed = Evaluate("SetPhysicalEvent()", "unidentifiable event", "The physical event is unidentifiable", Physical->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
    // Defect REV-C-20: The type of an unidentifiable physical event (100) is never the unknown type of the raw event (-1),
    // so the physical event is deleted and made new with each call: an event set from outside is lost, and the pointer of the call before dangles
    Passed = ExpectedToFail(EvaluateTrue("SetPhysicalEvent()", "unidentifiable event", "The bad flag of the event which was set is kept", Physical->HasBadFlag("FromOutside"))) && Passed;
    Passed = ExpectedToFail(EvaluateNear("SetPhysicalEvent()", "unidentifiable event", "The energy of the event which was set is kept", Physical->GetEnergy(), 123.0, 1e-12)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the physical events which cannot be made
bool UTRERawEvent::TestPhysicalEventWithoutInput()
{
  bool Passed = true;

  // Defect REV-C-21: The physical event is built without a check of what it needs: the program crashes or ends with an assert
  Passed = ExpectedToCrash(EvaluateTrue("GetPhysicalEvent()", "Compton event without start point", "Without a start point no Compton event is made", IsComptonWithoutStartPointHandled())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("GetPhysicalEvent()", "pair event without vertex", "Without a vertex no pair event is made", IsPairWithoutVertexHandled())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("GetPhysicalEvent()", "photo event with two hits", "With two hits no photo event is made", IsPhotoWithTwoHitsHandled())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor and the duplication
bool UTRERawEvent::TestCopies()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  MRERawEvent* Original = new MRERawEvent(&Geometry);
  FillComptonEvent(*Original);
  MRESE* A = Original->GetRESEAt(0);
  MRESE* B = Original->GetRESEAt(1);
  Original->SetVertex(B);
  Original->SetVertexDirection(-1);
  Original->SetEventID(42);
  Original->SetEventTime(MTime(12.5));
  Original->SetEventClock(MTime(0.25));
  Original->SetCoincidenceWindow(MTime(2.0e-6));
  Original->SetDecay(true);
  Original->AddBadFlag("Saturated");
  Original->SetClusteringQualityFactor(0.125);
  Original->SetComptonQualityFactors(0.25, 0.5);
  Original->SetTrackQualityFactor(0.75);
  Original->SetPairQualityFactor(1.5);
  Original->SetOriginInformation(MVector(1.5, -2.5, 30.0), MVector(0.6, 0.0, -0.8), MVector(0.0, 1.0, 0.0), 1808.7, 4);
  Original->ParseLine("CC A comment", c_SimVersion);
  Original->ParseLine("TW 5", c_SimVersion);
  Original->ParseLine("RX 0 1 0", c_SimVersion);
  Original->ParseLine("RZ -1 0 0", c_SimVersion);
  Original->ParseLine("GX 10 20", c_SimVersion);
  Original->ParseLine("HX 30 40", c_SimVersion);
  MPhysicalEvent* OriginalPhysical = Original->GetPhysicalEvent();

  vector<MString> Names = { "MRERawEvent(MRERawEvent*)", "Duplicate()" };
  for (unsigned int c = 0; c < Names.size(); ++c) {
    MRERawEvent* Copy = nullptr;
    if (c == 0) {
      Copy = new MRERawEvent(Original);
    } else {
      Copy = Original->Duplicate();
    }
    Passed = EvaluateTrue(Names[c], "Compton event", "A copy is made", Copy != nullptr) && Passed;
    if (Copy == nullptr) {
      continue;
    }

    // The data of the event:
    Passed = Evaluate(Names[c], "Compton event", "The ID of the RESE is kept", Copy->GetID(), Original->GetID()) && Passed;
    Passed = Evaluate(Names[c], "Compton event", "The event ID is copied", Copy->GetEventID(), 42UL) && Passed;
    Passed = Evaluate(Names[c], "Compton event", "The event type is copied", Copy->GetEventType(), MRERawEvent::c_ComptonEvent) && Passed;
    Passed = EvaluateNear(Names[c], "Compton event", "The event time is copied", Copy->GetEventTime().GetAsSeconds(), 12.5, 1e-9) && Passed;
    Passed = EvaluateNear(Names[c], "Compton event", "The event clock is copied", Copy->GetEventClock().GetAsSeconds(), 0.25, 1e-9) && Passed;
    Passed = EvaluateNear(Names[c], "Compton event", "The coincidence window is copied", Copy->GetCoincidenceWindow().GetAsSeconds(), 2.0e-6, 1e-12) && Passed;
    Passed = EvaluateTrue(Names[c], "Compton event", "The geometry is the same", Copy->GetGeometry() == &Geometry) && Passed;
    Passed = EvaluateTrue(Names[c], "Compton event", "The copy is good", Copy->IsGoodEvent()) && Passed;
    Passed = EvaluateTrue(Names[c], "Compton event", "The copy is valid", Copy->IsValid()) && Passed;
    Passed = EvaluateTrue(Names[c], "Compton event", "The decay flag is copied", Copy->IsDecay()) && Passed;
    Passed = Evaluate(Names[c], "Compton event", "The rejection reason is copied", Copy->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
    Passed = Evaluate(Names[c], "Compton event", "The bad flags are copied", Copy->GetNBadFlags(), 1U) && Passed;
    Passed = EvaluateTrue(Names[c], "Compton event", "The bad flag is found", Copy->HasBadFlag("Saturated")) && Passed;
    Passed = EvaluateNear(Names[c], "Compton event", "The clustering quality factor is copied", Copy->GetClusteringQualityFactor(), 0.125, 1e-15) && Passed;
    Passed = EvaluateNear(Names[c], "Compton event", "The first Compton quality factor is copied", Copy->GetComptonQualityFactor1(), 0.25, 1e-15) && Passed;
    Passed = EvaluateNear(Names[c], "Compton event", "The second Compton quality factor is copied", Copy->GetComptonQualityFactor2(), 0.5, 1e-15) && Passed;
    Passed = EvaluateNear(Names[c], "Compton event", "The track quality factor is copied", Copy->GetTrackQualityFactor(), 0.75, 1e-15) && Passed;
    Passed = EvaluateNear(Names[c], "Compton event", "The pair quality factor is copied", Copy->GetPairQualityFactor(), 1.5, 1e-15) && Passed;

    // The orientations:
    Passed = EvaluateTrue(Names[c], "Compton event", "The detector rotation is copied", Copy->HasDetectorRotation()) && Passed;
    Passed = EvaluateVectorNear(Names[c], "Compton event", "The x axis of the detector rotation is copied", Copy->GetDetectorRotationXAxis(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;
    Passed = EvaluateVectorNear(Names[c], "Compton event", "The z axis of the detector rotation is copied", Copy->GetDetectorRotationZAxis(), MVector(-1.0, 0.0, 0.0), 1e-15) && Passed;
    Passed = EvaluateTrue(Names[c], "Compton event", "The Galactic pointing is copied", Copy->HasGalacticPointing()) && Passed;
    Passed = EvaluateVectorNear(Names[c], "Compton event", "The x axis of the Galactic pointing is copied", Copy->GetGalacticPointingXAxis(), Original->GetGalacticPointingXAxis(), 1e-15) && Passed;
    Passed = EvaluateTrue(Names[c], "Compton event", "The horizon pointing is copied", Copy->HasHorizonPointing()) && Passed;
    Passed = EvaluateVectorNear(Names[c], "Compton event", "The x axis of the horizon pointing is copied", Copy->GetHorizonPointingXAxis(), Original->GetHorizonPointingXAxis(), 1e-15) && Passed;

    // The RESEs are copies with the same IDs, and start point and vertex are the copies:
    Passed = Evaluate(Names[c], "Compton event", "The copy has two RESEs", Copy->GetNRESEs(), 2) && Passed;
    if (Copy->GetNRESEs() == 2) {
      Passed = EvaluateTrue(Names[c], "Compton event", "The RESEs are new objects", Copy->GetRESEAt(0) != A && Copy->GetRESEAt(1) != B) && Passed;
      Passed = Evaluate(Names[c], "Compton event", "The first RESE keeps its ID", Copy->GetRESEAt(0)->GetID(), A->GetID()) && Passed;
      Passed = EvaluateNear(Names[c], "Compton event", "The first RESE keeps its energy", Copy->GetRESEAt(0)->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear(Names[c], "Compton event", "The second RESE keeps its position", Copy->GetRESEAt(1)->GetPosition(), MVector(3.0, 0.0, -4.0), 1e-15) && Passed;
      Passed = EvaluateTrue(Names[c], "Compton event", "The start point is the copy of the start point", Copy->GetStartPoint() == Copy->GetRESEAt(0)) && Passed;
      Passed = EvaluateTrue(Names[c], "Compton event", "The vertex is the copy of the vertex", Copy->GetVertex() == Copy->GetRESEAt(1)) && Passed;
      Passed = Evaluate(Names[c], "Compton event", "The vertex direction is copied", Copy->GetVertexDirection(), -1) && Passed;
      Passed = Evaluate(Names[c], "Compton event", "The link of the start point is copied", Copy->GetRESEAt(0)->GetNLinks(), 1) && Passed;
      if (Copy->GetRESEAt(0)->GetNLinks() == 1) {
        Passed = EvaluateTrue(Names[c], "Compton event", "The link of the start point goes to the copy of the second RESE", Copy->GetRESEAt(0)->GetLinkAt(0) == Copy->GetRESEAt(1)) && Passed;
      }
    }
    Passed = EvaluateNear(Names[c], "Compton event", "The energy is the same", Copy->GetEnergy(), 500.0, 1e-12) && Passed;

    // The additional measurements are clones:
    Passed = Evaluate(Names[c], "Compton event", "The additional measurement is copied", Copy->GetNREAMs(), 1U) && Passed;
    if (Copy->GetNREAMs() == 1) {
      Passed = EvaluateTrue(Names[c], "Compton event", "The additional measurement is a new object", Copy->GetREAMAt(0) != Original->GetREAMAt(0)) && Passed;
      MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(Copy->GetREAMAt(0));
      Passed = EvaluateTrue(Names[c], "Compton event", "It is a start information", Start != nullptr) && Passed;
      if (Start != nullptr) {
        Passed = EvaluateNear(Names[c], "Compton event", "The energy of the start information is copied", Start->GetEnergy(), 1808.7, 1e-12) && Passed;
      }
    }

    // The physical event is a copy:
    MPhysicalEvent* Physical = Copy->GetPhysicalEvent();
    Passed = EvaluateTrue(Names[c], "Compton event", "The copy has its own physical event", Physical != nullptr && Physical != OriginalPhysical) && Passed;
    MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Physical);
    Passed = EvaluateTrue(Names[c], "Compton event", "It is a Compton event", Compton != nullptr) && Passed;
    if (Compton != nullptr) {
      Passed = EvaluateNear(Names[c], "Compton event", "The electron energy is the same", Compton->Ee(), 200.0, 1e-12) && Passed;
      Passed = EvaluateNear(Names[c], "Compton event", "The photon energy is the same", Compton->Eg(), 300.0, 1e-12) && Passed;
      Passed = Evaluate(Names[c], "Compton event", "The time walk is copied", Compton->GetTimeWalk(), 5) && Passed;
      Passed = Evaluate(Names[c], "Compton event", "The comment is copied", Compton->GetNComments(), 1U) && Passed;
      Passed = EvaluateVectorNear(Names[c], "Compton event", "The origin position is copied", Compton->GetOIPosition(), MVector(1.5, -2.5, 30.0), 1e-15) && Passed;
    }

    // The copy is independent:
    Copy->SetEventID(43);
    Copy->AddBadFlag("OnlyInCopy");
    Copy->GetRESEAt(0)->SetEnergy(1.0);
    Passed = Evaluate(Names[c], "copy changed", "The ID of the original is unchanged", Original->GetEventID(), 42UL) && Passed;
    Passed = Evaluate(Names[c], "copy changed", "The bad flags of the original are unchanged", Original->GetNBadFlags(), 1U) && Passed;
    Passed = EvaluateNear(Names[c], "copy changed", "The energy of the hit of the original is unchanged", A->GetEnergy(), 200.0, 1e-12) && Passed;

    delete Copy;
  }

  // The original is still complete after its copies are deleted:
  Passed = Evaluate("~MRERawEvent()", "copies deleted", "The original still has two RESEs", Original->GetNRESEs(), 2) && Passed;
  Passed = EvaluateNear("~MRERawEvent()", "copies deleted", "The original still has its energy", Original->GetEnergy(), 500.0, 1e-12) && Passed;
  Passed = Evaluate("~MRERawEvent()", "copies deleted", "The original still has its additional measurement", Original->GetNREAMs(), 1U) && Passed;
  delete Original;

  // A copy of an event without a physical event makes its own one:
  {
    MRERawEvent WithoutPhysical;
    FillComptonEvent(WithoutPhysical);
    MRERawEvent* Copy = WithoutPhysical.Duplicate();
    MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Copy->GetPhysicalEvent());
    Passed = EvaluateTrue("Duplicate()", "Compton event without physical event", "The copy can make its physical event", Compton != nullptr) && Passed;
    if (Compton != nullptr) {
      Passed = EvaluateNear("Duplicate()", "Compton event without physical event", "The electron energy is the one of the start point", Compton->Ee(), 200.0, 1e-12) && Passed;
    }
    delete Copy;
  }

  // A copy of a new event:
  {
    MRERawEvent New;
    MRERawEvent* Copy = New.Duplicate();
    Passed = Evaluate("Duplicate()", "new event", "The copy has no RESEs", Copy->GetNRESEs(), 0) && Passed;
    Passed = EvaluateTrue("Duplicate()", "new event", "The copy has no start point", Copy->GetStartPoint() == nullptr) && Passed;
    Passed = Evaluate("Duplicate()", "new event", "The copy has no additional measurements", Copy->GetNREAMs(), 0U) && Passed;
    Passed = EvaluateFalse("Duplicate()", "new event", "The copy has no detector rotation", Copy->HasDetectorRotation()) && Passed;
    Passed = Evaluate("Duplicate()", "new event", "The copy has no quality factor", Copy->GetTrackQualityFactor(), MRERawEvent::c_NoQualityFactor) && Passed;
    delete Copy;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTRERawEvent::TestToString()
{
  bool Passed = true;

  // A rejected event with two hits, without vertex and track: head line, vertex line, the hits one level deeper, score line, rejection
  {
    MRERawEvent Event;
    MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 3.0);
    MREHit* B = CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0);
    Event.AddRESE(A);
    Event.AddRESE(B);
    Event.SetEventID(42);
    Event.SetEventTime(MTime(12.5));
    Event.SetRejectionReason(MRERawEvent::c_RejectionTooManyHits);

    MString Expected = "Raw Event (42) at 12.500000 s with 2 hits and 500.00+-5.00 keV, starting with (-1):\n";
    Expected += "No vertex!\n";
    Expected += A->ToString(true, 1);
    Expected += B->ToString(true, 1);
    Expected += "RE total score: None\n";
    Expected += "BAD event:Too many overall hits\n";
    Passed = Evaluate("ToString()", "rejected event with two hits", "The text has head line, vertex, hits, score, and rejection", Event.ToString(), Expected) && Passed;

    MString Indented = "   Raw Event (42) at 12.500000 s with 2 hits and 500.00+-5.00 keV, starting with (-1):\n";
    Indented += "   No vertex!\n";
    Indented += A->ToString(false, 2);
    Indented += B->ToString(false, 2);
    Indented += "   RE total score: None\n";
    Indented += "   BAD event:Too many overall hits\n";
    Passed = Evaluate("ToString()", "rejected event with two hits, without links, level 1", "Each line is indented by 3 blanks and the hits are one level deeper", Event.ToString(false, 1), Indented) && Passed;
  }

  // A good Compton event with start point, quality factors, and decay flag
  {
    MRERawEvent Event;
    FillComptonEvent(Event);
    Event.SetComptonQualityFactors(0.25, 0.5);
    Event.SetTrackQualityFactor(0.75);
    Event.SetDecay(true);
    MString Text = Event.ToString();
    ostringstream Head;
    Head<<"Raw Event (0) at 0.000000 s with 2 hits and 500.00+-4.47 keV, starting with ("<<Event.GetStartPoint()->GetID()<<"):"<<endl;
    Passed = EvaluateTrue("ToString()", "good Compton event", "The head line names the ID of the start point", Text.BeginsWith(Head.str().c_str())) && Passed;
    Passed = EvaluateTrue("ToString()", "good Compton event", "The score line has the Compton and the track quality factor", Text.Contains("RE total score: Compton: 0.25000000 Track: 0.75000000 \n")) && Passed;
    Passed = EvaluateFalse("ToString()", "good Compton event", "A good event has no rejection line", Text.Contains("BAD event")) && Passed;
    Passed = EvaluateTrue("ToString()", "good Compton event", "The decay flag is the last line", Text.EndsWith("Decay flag set")) && Passed;
  }

  // An event with a vertex
  {
    MRERawEvent Event;
    MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 3.0);
    Event.AddRESE(A);
    Event.SetVertex(A);
    Event.SetPairQualityFactor(1.5);
    Event.SetGoodEvent(true);
    MString Text = Event.ToString();
    Passed = EvaluateTrue("ToString()", "event with vertex", "The vertex is written", Text.Contains(MString("Vertex: ") + A->ToString())) && Passed;
    Passed = EvaluateFalse("ToString()", "event with vertex", "There is no line for a missing vertex", Text.Contains("No vertex!")) && Passed;
    Passed = EvaluateTrue("ToString()", "event with vertex", "The score line has the pair quality factor", Text.EndsWith("RE total (pair) score 1.500 ")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the compact text
bool UTRERawEvent::TestToCompactString()
{
  bool Passed = true;

  // An event which is not good has no text:
  MRERawEvent NotGood;
  NotGood.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0));
  Passed = Evaluate("ToCompactString()", "event which is not good", "The text is empty", NotGood.ToCompactString(), MString("")) && Passed;

  // A Compton event of hits: the two positions, no electron direction, the energy of the start point and the rest
  MRERawEvent Event;
  FillComptonEvent(Event);
  Passed = Evaluate("ToCompactString()", "Compton event with two hits", "The text has positions, zero direction, and energies", Event.ToCompactString(), MString("C;0.000;0.000;0.000;3.000;0.000;-4.000;0.000000;0.000000;0.000000;200.0;300.0; 10.0;10.0\n")) && Passed;

  // A Compton event which starts with a track: with the direction of the track
  MRERawEvent WithTrack;
  MREHit* T1 = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 1.5);
  MREHit* T2 = CreatePlainHit(MVector(0.0, 0.6, -0.8), 150.0, 2.0);
  Link(T1, T2);
  MRETrack* Track = new MRETrack();
  Track->AddRESE(T1);
  Track->AddRESE(T2);
  Track->SetStartPoint(T1);
  MREHit* B = CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0);
  WithTrack.AddRESE(Track);
  WithTrack.AddRESE(B);
  Link(Track, B);
  WithTrack.SetStartPoint(Track);
  WithTrack.SetGoodEvent(true);
  Passed = Evaluate("ToCompactString()", "Compton event with track", "The text has positions, the direction of the track, and energies", WithTrack.ToCompactString(), MString("C;0.000;0.000;0.000;3.000;0.000;-4.000;0.000000;0.600000;-0.800000;250.0;300.0; 10.0;10.0\n")) && Passed;

  // Defect REV-C-21: The start point is used without a check
  Passed = ExpectedToCrash(EvaluateTrue("ToCompactString()", "good event without start point", "A good event without start point has no text", IsCompactStringWithoutStartPointEmpty())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text for the evta file
bool UTRERawEvent::TestToEvtaString()
{
  bool Passed = true;

  MRERawEvent Event;
  MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0);
  MREHit* B = CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0);
  Event.AddRESE(A);
  Event.AddRESE(B);
  Event.SetEventID(42);
  Event.SetEventTime(MTime(12.5));
  Event.ParseLine("RX 0 1 0", c_SimVersion);
  Event.ParseLine("RZ -1 0 0", c_SimVersion);
  Event.ParseLine("CC A comment", c_SimVersion);

  MString Text = Event.ToEvtaString(5, c_SimVersion);
  Passed = EvaluateTrue("ToEvtaString()", "event with two hits", "The text starts with the start of the event, the ID, and the time", Text.BeginsWith("SE\nID 42\nTI 12.500000000\n")) && Passed;
  Passed = EvaluateTrue("ToEvtaString()", "event with two hits", "The text has the line of the first hit", Text.Contains(A->ToEvtaString(5, c_SimVersion))) && Passed;
  Passed = EvaluateTrue("ToEvtaString()", "event with two hits", "The text has the line of the second hit", Text.Contains(B->ToEvtaString(5, c_SimVersion))) && Passed;
  Passed = EvaluateTrue("ToEvtaString()", "event with two hits", "The hits are in their order", Text.Index(A->ToEvtaString(5, c_SimVersion)) < Text.Index(B->ToEvtaString(5, c_SimVersion))) && Passed;
  Passed = EvaluateTrue("ToEvtaString()", "event with two hits", "The text ends with a line break", Text.EndsWith("\n")) && Passed;

  // Round trip of the lines of the event itself: each line is given to a new event
  MRERawEvent Reread;
  vector<MString> Lines = Text.Tokenize("\n");
  for (const MString& Line: Lines) {
    if (Line.BeginsWith("HT") == false) {
      Reread.ParseLine(Line, c_SimVersion);
    }
  }
  Passed = Evaluate("ToEvtaString()", "round trip", "The ID is read back", Reread.GetEventID(), 42UL) && Passed;
  Passed = EvaluateNear("ToEvtaString()", "round trip", "The time is read back", Reread.GetEventTime().GetAsSeconds(), 12.5, 1e-9) && Passed;
  Passed = EvaluateTrue("ToEvtaString()", "round trip", "The detector rotation is read back", Reread.HasDetectorRotation()) && Passed;
  Passed = EvaluateVectorNear("ToEvtaString()", "round trip", "The x axis of the detector rotation is read back", Reread.GetDetectorRotationXAxis(), MVector(0.0, 1.0, 0.0), 1e-6) && Passed;
  Passed = EvaluateVectorNear("ToEvtaString()", "round trip", "The z axis of the detector rotation is read back", Reread.GetDetectorRotationZAxis(), MVector(-1.0, 0.0, 0.0), 1e-6) && Passed;
  // Defect REV-C-22: The comment is written without its keyword CC, so it is no line of an evta file and is lost when the text is read
  Passed = ExpectedToFail(Evaluate("ToEvtaString()", "round trip", "The comment is read back", Reread.GetPhysicalEvent()->GetNComments(), 1U)) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("ToEvtaString()", "event with a comment", "The comment is written with its keyword", Text.Contains("CC A comment\n"))) && Passed;

  // An event without hits:
  MRERawEvent Empty;
  Passed = Evaluate("ToEvtaString()", "new event", "A new event has only the start of the event, the ID, and the time", Empty.ToEvtaString(5, c_SimVersion), MString("SE\nID 0\nTI 0.000000000\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new hit in a strip detector with the given position (cm), energy and its resolution (keV), and time (s)
//! The position resolution is (0.1, 0.2, 0.3) cm and the time resolution 1 ns. No geometry is used. The caller owns the hit
MREHit* UTRERawEvent::CreatePlainHit(const MVector& Position, double Energy, double EnergyResolution, double Time)
{
  MREHit* Hit = new MREHit();
  Hit->SetPosition(Position);
  Hit->SetPositionResolution(MVector(0.1, 0.2, 0.3));
  Hit->SetEnergy(Energy);
  Hit->SetEnergyResolution(EnergyResolution);
  Hit->SetTime(Time);
  Hit->SetTimeResolution(1.0e-9);
  Hit->SetDetector(MDDetector::c_Strip2D);

  return Hit;
}


////////////////////////////////////////////////////////////////////////////////


//! Link two RESEs with each other
void UTRERawEvent::Link(MRESE* A, MRESE* B)
{
  A->AddLink(B);
  B->AddLink(A);
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the event with a Compton sequence of two hits: 200 +- 2 keV at the origin at 0 s, then 300 +- 4 keV at (3, 0, -4) cm at 1 ns
void UTRERawEvent::FillComptonEvent(MRERawEvent& Event)
{
  MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0, 0.0);
  MREHit* B = CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0, 1.0e-9);
  Event.AddRESE(A);
  Event.AddRESE(B);
  Link(A, B);
  Event.SetStartPoint(A);
  Event.SetEventType(MRERawEvent::c_ComptonEvent);
  Event.SetGoodEvent(true);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the lines which add a guard ring to the tracker (trigger threshold 40 keV, energy resolution 5 keV), a drift chamber at (-30, 0, 0) cm
//! with a light energy resolution of 3 keV, and a directional strip detector at (0, 30, 0) cm to the test telescope
MString UTRERawEvent::GetAdditionalDetectors(bool WithLightEnergyResolution) const
{
  MString Lines;

  Lines += GetGuardRingLines();

  Lines += "Volume ChamberBox\n";
  Lines += "ChamberBox.Material Silicon\n";
  Lines += "ChamberBox.Shape BRIK 5.0 5.0 1.0\n";
  Lines += "ChamberBox.Position -30.0 0.0 0.0\n";
  Lines += "ChamberBox.Mother World\n";
  Lines += "DriftChamber Chamber\n";
  Lines += "Chamber.SensitiveVolume ChamberBox\n";
  Lines += "Chamber.DetectorVolume ChamberBox\n";
  Lines += "Chamber.StripNumber 10 10\n";
  Lines += "Chamber.Offset 0.0 0.0\n";
  Lines += "Chamber.NoiseThreshold 10\n";
  Lines += "Chamber.TriggerThreshold 30\n";
  Lines += "Chamber.EnergyResolution Gauss 10 10 1\n";
  Lines += "Chamber.EnergyResolution Gauss 10000 10000 1\n";
  Lines += "Chamber.FailureRate 0.0\n";
  if (WithLightEnergyResolution == true) {
    Lines += "Chamber.LightEnergyResolution 10 3\n";
    Lines += "Chamber.LightEnergyResolution 10000 3\n";
  }
  Lines += "Trigger ChamberTrigger\n";
  Lines += "ChamberTrigger.Veto false\n";
  Lines += "ChamberTrigger.TriggerByChannel true\n";
  Lines += "ChamberTrigger.Detector Chamber 1\n";

  Lines += "Volume DirectionalBox\n";
  Lines += "DirectionalBox.Material Silicon\n";
  Lines += "DirectionalBox.Shape BRIK 5.0 5.0 1.0\n";
  Lines += "DirectionalBox.Position 0.0 30.0 0.0\n";
  Lines += "DirectionalBox.Mother World\n";
  Lines += "Strip3DDirectional Directional\n";
  Lines += "Directional.SensitiveVolume DirectionalBox\n";
  Lines += "Directional.DetectorVolume DirectionalBox\n";
  Lines += "Directional.StripNumber 10 10\n";
  Lines += "Directional.Offset 0.0 0.0\n";
  Lines += "Directional.NoiseThreshold 10\n";
  Lines += "Directional.TriggerThreshold 30\n";
  Lines += "Directional.EnergyResolution Gauss 10 10 1\n";
  Lines += "Directional.EnergyResolution Gauss 10000 10000 1\n";
  Lines += "Directional.FailureRate 0.0\n";
  Lines += "Trigger DirectionalTrigger\n";
  Lines += "DirectionalTrigger.Veto false\n";
  Lines += "DirectionalTrigger.TriggerByChannel true\n";
  Lines += "DirectionalTrigger.Detector Directional 1\n";

  return Lines;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a line with all output silenced: the MEGAlib streams and cout, which the search for the volume uses
int UTRERawEvent::ParseSilently(MRERawEvent& Event, const MString& Line, int Version)
{
  DisableDefaultStreams();
  BeginCaptureOutput();
  int Result = Event.ParseLine(Line, Version);
  EndCaptureOutput();
  EnableDefaultStreams();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a guard ring line is rejected with 1 if the event has no geometry
bool UTRERawEvent::IsGuardRingLineWithoutGeometryRejected()
{
  MRERawEvent Event;
  if (ParseSilently(Event, "GR 4.8;0.2;1.0;45.5", c_SimVersion) != 1) {
    return false;
  }
  if (Event.GetNREAMs() != 0) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the drift chamber energy is read if the chamber has an ideal light energy resolution
bool UTRERawEvent::IsDriftChamberEnergyWithIdealResolutionRead()
{
  MGeometryRevan Geometry;
  if (ScanTelescopeGeometry(Geometry, GetAdditionalDetectors(false)) == false) {
    return false;
  }
  MRERawEvent Event(&Geometry);
  if (ParseSilently(Event, "XE -30;0;0;55.5", c_SimVersion) != 0) {
    return false;
  }
  if (Event.GetNREAMs() != 1) {
    return false;
  }
  MREAMDriftChamberEnergy* Chamber = dynamic_cast<MREAMDriftChamberEnergy*>(Event.GetREAMAt(0));
  if (Chamber == nullptr || Chamber->GetEnergyResolution() != 0.0) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the iterator is not started without a start point
bool UTRERawEvent::IsIteratorWithoutStartPointRejected()
{
  MRERawEvent Event;
  Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 1.0));
  if (Event.ResetIterator() == true) {
    return false;
  }
  if (Event.GetNextRESE() != nullptr) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the iterator is not started at a RESE with two links
bool UTRERawEvent::IsIteratorAtMiddleRejected()
{
  MRERawEvent Event;
  MREHit* A = CreatePlainHit(MVector(0.0, 0.0, 0.0), 100.0, 1.0);
  MREHit* B = CreatePlainHit(MVector(0.0, 0.0, 1.0), 200.0, 1.0);
  MREHit* C = CreatePlainHit(MVector(0.0, 0.0, 2.0), 300.0, 1.0);
  Event.AddRESE(A);
  Event.AddRESE(B);
  Event.AddRESE(C);
  Link(A, B);
  Link(B, C);
  if (Event.ResetIterator(B) == true) {
    return false;
  }
  if (Event.GetNextRESE() != nullptr) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a Compton event without start point gives no Compton event
bool UTRERawEvent::IsComptonWithoutStartPointHandled()
{
  MRERawEvent Event;
  Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0));
  Event.AddRESE(CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0));
  Event.SetEventType(MRERawEvent::c_ComptonEvent);
  MPhysicalEvent* Physical = Event.GetPhysicalEvent();
  if (Physical != nullptr && Physical->GetType() == MPhysicalEvent::c_Compton) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a pair event without vertex gives no pair event
bool UTRERawEvent::IsPairWithoutVertexHandled()
{
  MRERawEvent Event;
  MREHit* E1 = CreatePlainHit(MVector(0.0, 0.0, 2.0), 110.0, 1.0);
  MREHit* E2 = CreatePlainHit(MVector(0.6, 0.0, 1.2), 120.0, 1.0);
  Link(E1, E2);
  MRETrack* Electron = new MRETrack();
  Electron->AddRESE(E1);
  Electron->AddRESE(E2);
  Electron->SetStartPoint(E1);
  MREHit* P1 = CreatePlainHit(MVector(0.0, 0.0, 2.0), 210.0, 1.0);
  MREHit* P2 = CreatePlainHit(MVector(0.0, -0.8, 1.4), 220.0, 1.0);
  Link(P1, P2);
  MRETrack* Positron = new MRETrack();
  Positron->AddRESE(P1);
  Positron->AddRESE(P2);
  Positron->SetStartPoint(P1);
  Event.AddRESE(Electron);
  Event.AddRESE(Positron);
  Event.SetElectronTrack(Electron);
  Event.SetPositronTrack(Positron);
  Event.SetEventType(MRERawEvent::c_PairEvent);
  MPhysicalEvent* Physical = Event.GetPhysicalEvent();
  if (Physical != nullptr && Physical->GetType() == MPhysicalEvent::c_Pair) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a photo event with two hits gives no photo event
bool UTRERawEvent::IsPhotoWithTwoHitsHandled()
{
  MRERawEvent Event;
  Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0));
  Event.AddRESE(CreatePlainHit(MVector(3.0, 0.0, -4.0), 300.0, 4.0));
  Event.SetEventType(MRERawEvent::c_PhotoEvent);
  MPhysicalEvent* Physical = Event.GetPhysicalEvent();
  if (Physical != nullptr && Physical->GetType() == MPhysicalEvent::c_Photo) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the compact text of a good event without start point is empty
bool UTRERawEvent::IsCompactStringWithoutStartPointEmpty()
{
  MRERawEvent Event;
  Event.AddRESE(CreatePlainHit(MVector(0.0, 0.0, 0.0), 200.0, 2.0));
  Event.SetEventType(MRERawEvent::c_PhotoEvent);
  Event.SetGoodEvent(true);

  return Event.ToCompactString().IsEmpty();
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);

  // Hide the info messages of the geometry manager:
  gErrorIgnoreLevel = kWarning;

  UTRERawEvent Test;
  return Test.Run() == true ? 0 : 1;
}


// UTRERawEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
