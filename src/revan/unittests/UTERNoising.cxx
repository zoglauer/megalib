/*
 * UTERNoising.cxx
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
#include <map>
#include <new>
#include <set>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGuardRing.h"
#include "MDVolumeSequence.h"
#include "MERNoising.h"
#include "MREAMGuardRingHit.h"
#include "MRECluster.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MREStripHit.h"
#include "MTime.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the noising and triggering stage MERNoising of revan
class UTERNoising : public UTRevanBase
{
public:
  //! Default constructor
  UTERNoising() : UTRevanBase("UTERNoising") {}
  //! Default destructor
  virtual ~UTERNoising() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the state after the construction and after the pre-analysis
  bool TestConstruction();
  //! Test the analysis without a geometry
  bool TestWithoutGeometry();
  //! Test single hits in all detectors with the detector effects engine 1
  bool TestEngine1SingleHits();
  //! Test the thresholds with the detector effects engine 1
  bool TestEngine1Thresholds();
  //! Test the veto and the trigger statistics with the detector effects engine 1
  bool TestEngine1Veto();
  //! Test the guard ring hits with the detector effects engine 1
  bool TestEngine1GuardRing();
  //! Test an event with a cluster with the detector effects engine 1
  bool TestEngine1NoHit();
  //! Test the noised energy with the detector effects engine 1
  bool TestEngine1EnergyResolution();
  //! Test the hits in the strip, calorimeter, and scintillator detectors with the detector effects engine 2
  bool TestEngine2Hits();
  //! Test the strip hits of the depth detector with the detector effects engine 2
  bool TestEngine2StripHits();
  //! Test the thresholds, the merging of hits, and the event time with the detector effects engine 2
  bool TestEngine2Thresholds();
  //! Test the veto and the trigger statistics with the detector effects engine 2
  bool TestEngine2Veto();
  //! Test the guard ring hits with the detector effects engine 2
  bool TestEngine2GuardRing();
  //! Test adding the statistics of two noising stages
  bool TestAddStatistics();
  //! Test the summary text
  bool TestToString();

  //! Return the telescope with ideal energy resolutions for the given engine, optionally without depth resolution and with a guard ring veto of 40 keV on the tracker
  MString GetNoisingGeometry(unsigned int Engine, bool WithDepthResolution = false, bool WithGuardRing = false) const;
  //! Add a hit with the given origin ID to the event, return false if it cannot be created
  bool AddHit(MRERawEvent& Event, MGeometryRevan& Geometry, const MVector& Position, double Energy, int DetectorType, unsigned int OriginID);
  //! Add a guard ring hit at the given position to the event as the reader of the evta file does, return false if the position has no guard ring
  bool AddGuardRingHit(MRERawEvent& Event, MGeometryRevan& Geometry, const MVector& Position, double Energy);
  //! Return true if the analysis without a geometry returns false
  bool IsAnalysisWithoutGeometryRejected();

  //! A position in strip (5, 6) of tracker layer 0, strip center (1, 2, 0)
  const MVector m_TrackerPosition = MVector(1.2, 2.1, 0.01);
  //! A position in strip (1, 6) of tracker layer 3, strip center (-3, 2, 3)
  const MVector m_TrackerPosition3 = MVector(-3.1, 1.8, 3.0);
  //! A position in the guard ring of tracker layer 0
  const MVector m_GuardRingPosition = MVector(4.8, 0.1, 0.0);
  //! A position in the calorimeter, 1 cm above its center (0, 0, -10)
  const MVector m_CalorimeterPosition = MVector(1.0, 1.0, -9.0);
  //! A position in strip (5, 5) of the depth detector, strip center (30.5, 0.5), 0.5 cm above the center of the wafer
  const MVector m_DepthPosition = MVector(30.2, 0.3, 0.5);
  //! A position in the shield
  const MVector m_ShieldPosition = MVector(2.0, 3.0, 10.0);
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERNoising::Run()
{
  bool Passed = true;

  // The noise of the energy and depth resolutions is random
  gRandom->SetSeed(1);

  Passed = TestConstruction() && Passed;
  Passed = TestWithoutGeometry() && Passed;
  Passed = TestEngine1SingleHits() && Passed;
  Passed = TestEngine1Thresholds() && Passed;
  Passed = TestEngine1Veto() && Passed;
  Passed = TestEngine1GuardRing() && Passed;
  Passed = TestEngine1NoHit() && Passed;
  Passed = TestEngine1EnergyResolution() && Passed;
  Passed = TestEngine2Hits() && Passed;
  Passed = TestEngine2StripHits() && Passed;
  Passed = TestEngine2Thresholds() && Passed;
  Passed = TestEngine2Veto() && Passed;
  Passed = TestEngine2GuardRing() && Passed;
  Passed = TestAddStatistics() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the telescope with ideal energy resolutions for the given engine, optionally without depth resolution and with a guard ring veto of 40 keV on the tracker
MString UTERNoising::GetNoisingGeometry(unsigned int Engine, bool WithDepthResolution, bool WithGuardRing) const
{
  // Ideal energy resolutions make the energies exact:
  MString Setup = GetTelescopeGeometry("", true);
  if (WithDepthResolution == false) {
    Setup.ReplaceAllInPlace("Depth.DepthResolution 10 0.1 0\nDepth.DepthResolution 10000 0.1 0\n", "");
  }

  Setup += MString("DetectorEffectsEngine ") + Engine + "\n";

  if (WithGuardRing == true) {
    Setup += GetGuardRingLines(0.001);
    Setup += "Trigger GuardRingVeto\n";
    Setup += "GuardRingVeto.Veto true\n";
    Setup += "GuardRingVeto.TriggerByChannel true\n";
    Setup += "GuardRingVeto.GuardRingDetector Tracker 1\n";
  }

  return Setup;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a hit with the given origin ID to the event, return false if it cannot be created
bool UTERNoising::AddHit(MRERawEvent& Event, MGeometryRevan& Geometry, const MVector& Position, double Energy, int DetectorType, unsigned int OriginID)
{
  MREHit* Hit = CreateHit(Position, Energy, DetectorType, &Geometry);
  if (Hit == nullptr) {
    return false;
  }
  Hit->AddOriginID(OriginID);
  Event.AddRESE(Hit);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a guard ring hit at the given position to the event as the reader of the evta file does, return false if the position has no guard ring
bool UTERNoising::AddGuardRingHit(MRERawEvent& Event, MGeometryRevan& Geometry, const MVector& Position, double Energy)
{
  DisableDefaultStreams();
  MDVolumeSequence* Sequence = Geometry.GetVolumeSequencePointer(Position, true, true);
  EnableDefaultStreams();
  if (Sequence->GetDetector() == nullptr || Sequence->GetDetector()->HasGuardRing() == false) {
    delete Sequence;
    return false;
  }
  Sequence->SetDetector(Sequence->GetDetector()->GetGuardRing());

  // The guard ring hit owns the volume sequence, the event owns the guard ring hit
  MREAMGuardRingHit* GuardRingHit = new MREAMGuardRingHit();
  GuardRingHit->SetVolumeSequence(Sequence);
  GuardRingHit->SetEnergy(Energy);
  GuardRingHit->SetEnergyResolution(Sequence->GetDetector()->GetEnergyResolution(Energy));
  Event.AddREAM(GuardRingHit);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the analysis without a geometry returns false
bool UTERNoising::IsAnalysisWithoutGeometryRejected()
{
  MERNoising Noising;
  Noising.PreAnalysis();
  MRERawEvent Event;
  DisableDefaultStreams();
  bool Analyzed = Noising.Analyze(&Event);
  EnableDefaultStreams();
  if (Analyzed == true) return false;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the construction and after the pre-analysis
bool UTERNoising::TestConstruction()
{
  bool Passed = true;

  // Construct into a block filled with a pattern, which is no valid counter
  alignas(MERNoising) unsigned char Block[sizeof(MERNoising)];
  FillMemory(Block, sizeof(MERNoising), 0xA5);
  MERNoising* Constructed = new (Block) MERNoising();
  // Defect REV-E-1: the constructor does not initialize the three event counters, only the pre-analysis does
  Passed = ExpectedToFail(Evaluate("MERNoising()", "construction", "The number of triggered events is zero", Constructed->GetNTriggeredEvents(), 0L)) && Passed;
  Passed = ExpectedToFail(Evaluate("MERNoising()", "construction", "The number of vetoed events is zero", Constructed->GetNVetoedEvents(), 0L)) && Passed;
  Passed = ExpectedToFail(Evaluate("MERNoising()", "construction", "The number of not triggered events is zero", Constructed->GetNNotTriggeredOrVetoedEvents(), 0L)) && Passed;
  Passed = EvaluateSize("MERNoising()", "construction", "The trigger map is empty", Constructed->GetTriggerMapTriggerNames().size(), 0) && Passed;
  Passed = EvaluateSize("MERNoising()", "construction", "The veto map of the triggers is empty", Constructed->GetVetoMapTriggerNames().size(), 0) && Passed;
  Passed = EvaluateSize("MERNoising()", "construction", "The veto map of the vetoes is empty", Constructed->GetVetoMapVetoNames().size(), 0) && Passed;

  Passed = EvaluateTrue("PreAnalysis()", "after construction", "The pre-analysis succeeds", Constructed->PreAnalysis()) && Passed;
  Passed = Evaluate("PreAnalysis()", "after construction", "The number of triggered events is zero", Constructed->GetNTriggeredEvents(), 0L) && Passed;
  Passed = Evaluate("PreAnalysis()", "after construction", "The number of vetoed events is zero", Constructed->GetNVetoedEvents(), 0L) && Passed;
  Passed = Evaluate("PreAnalysis()", "after construction", "The number of not triggered events is zero", Constructed->GetNNotTriggeredOrVetoedEvents(), 0L) && Passed;
  Passed = Evaluate("ToString()", "no events", "The summary of no events is empty", Constructed->ToString(), MString("")) && Passed;
  Constructed->~MERNoising();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the analysis without a geometry
bool UTERNoising::TestWithoutGeometry()
{
  bool Passed = true;

  // Defect REV-E-2: Analyze uses the geometry without a check
  Passed = ExpectedToCrash(EvaluateTrue("Analyze()", "no geometry", "The analysis without a geometry returns false", IsAnalysisWithoutGeometryRejected())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test single hits in all detectors with the detector effects engine 1
bool UTERNoising::TestEngine1SingleHits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 1", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(1))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // The tracker: 200 keV is above the trigger threshold of 30 keV, the energy resolution is ideal, the simulation has already centered the position
  {
    MRERawEvent Event(&Geometry);
    Passed = EvaluateTrue("CreateHit()", "tracker", "The hit is created", AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 7)) && Passed;
    Passed = EvaluateTrue("Analyze()", "engine 1: tracker hit of 200 keV", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 1: tracker hit of 200 keV", "The hit is kept", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      Passed = Evaluate("Analyze()", "engine 1: tracker hit of 200 keV", "It is still a hit", Hit->GetType(), MRESE::c_Hit) && Passed;
      Passed = EvaluateNear("Analyze()", "engine 1: tracker hit of 200 keV", "The energy is not changed by an ideal resolution", Hit->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "engine 1: tracker hit of 200 keV", "The position is not changed", Hit->GetPosition(), m_TrackerPosition, 1e-12) && Passed;
      Passed = EvaluateSize("Analyze()", "engine 1: tracker hit of 200 keV", "The origin is kept", Hit->GetOriginIDs().count(7), 1) && Passed;
      Passed = Evaluate("Analyze()", "engine 1: tracker hit of 200 keV", "There are no noise flags", Hit->GetNoiseFlags(), MString("")) && Passed;
    }
    Passed = EvaluateTrue("Analyze()", "engine 1: tracker hit of 200 keV", "The event is valid", Event.IsValid()) && Passed;
  }
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: tracker hit of 200 keV", "One event has triggered", Noising.GetNTriggeredEvents(), 1L) && Passed;
  Passed = Evaluate("GetTriggerMapTriggerNames()", "engine 1: tracker hit of 200 keV", "The tracker trigger was raised once", Noising.GetTriggerMapTriggerNames()["TrackerTrigger"], 1L) && Passed;
  Passed = EvaluateSize("GetTriggerMapTriggerNames()", "engine 1: tracker hit of 200 keV", "No other trigger was raised", Noising.GetTriggerMapTriggerNames().size(), 1) && Passed;

  // The calorimeter has no depth resolution: the hit is moved to the center of the block in z
  {
    MRERawEvent Event(&Geometry);
    Passed = EvaluateTrue("CreateHit()", "calorimeter", "The hit is created", AddHit(Event, Geometry, m_CalorimeterPosition, 200.0, MDDetector::c_Calorimeter, 8)) && Passed;
    Passed = EvaluateTrue("Analyze()", "engine 1: calorimeter hit of 200 keV", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 1: calorimeter hit of 200 keV", "The hit is kept", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = EvaluateNear("Analyze()", "engine 1: calorimeter hit of 200 keV", "The energy is not changed by an ideal resolution", Event.GetRESEAt(0)->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "engine 1: calorimeter hit of 200 keV", "The hit is at the z center of the block", Event.GetRESEAt(0)->GetPosition(), MVector(1.0, 1.0, -10.0), 1e-12) && Passed;
    }
  }
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: two events", "Two events have triggered", Noising.GetNTriggeredEvents(), 2L) && Passed;
  Passed = Evaluate("GetTriggerMapTriggerNames()", "engine 1: calorimeter hit of 200 keV", "The calorimeter trigger was raised once", Noising.GetTriggerMapTriggerNames()["CalorimeterTrigger"], 1L) && Passed;

  // The depth detector without depth resolution: the hit is moved to the center of the wafer in z
  {
    MRERawEvent Event(&Geometry);
    Passed = EvaluateTrue("CreateHit()", "depth detector", "The hit is created", AddHit(Event, Geometry, m_DepthPosition, 200.0, MDDetector::c_Strip3D, 9)) && Passed;
    Passed = EvaluateTrue("Analyze()", "engine 1: depth detector hit of 200 keV", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 1: depth detector hit of 200 keV", "The hit is kept", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = Evaluate("Analyze()", "engine 1: depth detector hit of 200 keV", "It is still a hit", Event.GetRESEAt(0)->GetType(), MRESE::c_Hit) && Passed;
      Passed = EvaluateNear("Analyze()", "engine 1: depth detector hit of 200 keV", "The energy is not changed by an ideal resolution", Event.GetRESEAt(0)->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "engine 1: depth detector hit of 200 keV", "The hit is at the z center of the wafer", Event.GetRESEAt(0)->GetPosition(), MVector(30.2, 0.3, 0.0), 1e-12) && Passed;
    }
  }
  Passed = Evaluate("GetTriggerMapTriggerNames()", "engine 1: depth detector hit of 200 keV", "The depth trigger was raised once", Noising.GetTriggerMapTriggerNames()["DepthTrigger"], 1L) && Passed;
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: three events", "Three events have triggered", Noising.GetNTriggeredEvents(), 3L) && Passed;
  Passed = Evaluate("GetNVetoedEvents()", "engine 1: three events", "No event was vetoed", Noising.GetNVetoedEvents(), 0L) && Passed;
  Passed = Evaluate("GetNNotTriggeredOrVetoedEvents()", "engine 1: three events", "All events have triggered", Noising.GetNNotTriggeredOrVetoedEvents(), 0L) && Passed;

  // An empty event does not trigger
  {
    MRERawEvent Event(&Geometry);
    Passed = EvaluateTrue("Analyze()", "engine 1: empty event", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 1: empty event", "The event stays empty", Event.GetNRESEs(), 0) && Passed;
  }
  Passed = Evaluate("GetNNotTriggeredOrVetoedEvents()", "engine 1: empty event", "The empty event has not triggered", Noising.GetNNotTriggeredOrVetoedEvents(), 1L) && Passed;

  // The pre-analysis resets the statistics
  Noising.PreAnalysis();
  Passed = Evaluate("PreAnalysis()", "after four events", "The number of triggered events is zero", Noising.GetNTriggeredEvents(), 0L) && Passed;
  Passed = Evaluate("PreAnalysis()", "after four events", "The number of not triggered events is zero", Noising.GetNNotTriggeredOrVetoedEvents(), 0L) && Passed;
  Passed = EvaluateSize("PreAnalysis()", "after four events", "The trigger map is empty", Noising.GetTriggerMapTriggerNames().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the thresholds with the detector effects engine 1
bool UTERNoising::TestEngine1Thresholds()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 1", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(1))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // The tracker has a noise threshold of 10 keV and a trigger threshold of 30 keV

  // 31 keV is just above the trigger threshold
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 31.0, MDDetector::c_Strip2D, 1);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: tracker hit of 31 keV", "The hit is kept", Event.GetNRESEs(), 1) && Passed;
  }
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: tracker hit of 31 keV", "The event has triggered", Noising.GetNTriggeredEvents(), 1L) && Passed;

  // 29 keV is just below the trigger threshold, but above the noise threshold: the event does not trigger and is emptied
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 29.0, MDDetector::c_Strip2D, 1);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: tracker hit of 29 keV", "The hit is removed", Event.GetNRESEs(), 0) && Passed;
  }
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: tracker hit of 29 keV", "The event has not triggered", Noising.GetNTriggeredEvents(), 1L) && Passed;
  Passed = Evaluate("GetNNotTriggeredOrVetoedEvents()", "engine 1: tracker hit of 29 keV", "One event has not triggered", Noising.GetNNotTriggeredOrVetoedEvents(), 1L) && Passed;

  // A triggering hit keeps a second hit between the noise and the trigger threshold (11 keV), but not one below the noise threshold (9 keV)
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_TrackerPosition3, 11.0, MDDetector::c_Strip2D, 2);
    AddHit(Event, Geometry, MVector(3.0, -2.0, GetTrackerLayerZ(1)), 9.0, MDDetector::c_Strip2D, 3);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: 200 keV, 11 keV, and 9 keV in the tracker", "Two hits are kept", Event.GetNRESEs(), 2) && Passed;
    if (Event.GetNRESEs() == 2) {
      Passed = EvaluateNear("Analyze()", "engine 1: 200 keV, 11 keV, and 9 keV in the tracker", "The first hit has 200 keV", Event.GetRESEAt(0)->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "engine 1: 200 keV, 11 keV, and 9 keV in the tracker", "The second hit has 11 keV", Event.GetRESEAt(1)->GetEnergy(), 11.0, 1e-12) && Passed;
    }
    Passed = EvaluateNear("GetEnergy()", "engine 1: 200 keV, 11 keV, and 9 keV in the tracker", "The event has 211 keV", Event.GetEnergy(), 211.0, 1e-12) && Passed;
  }
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: 200 keV, 11 keV, and 9 keV in the tracker", "The event has triggered", Noising.GetNTriggeredEvents(), 2L) && Passed;

  // Two hits below the trigger threshold in different channels do not trigger
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 29.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_TrackerPosition3, 29.0, MDDetector::c_Strip2D, 2);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: two tracker hits of 29 keV", "The hits are removed", Event.GetNRESEs(), 0) && Passed;
  }
  Passed = Evaluate("GetNNotTriggeredOrVetoedEvents()", "engine 1: two tracker hits of 29 keV", "Two events have not triggered", Noising.GetNNotTriggeredOrVetoedEvents(), 2L) && Passed;

  // The calorimeter has a trigger threshold of 50 keV: 49 keV does not trigger, 51 keV does
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_CalorimeterPosition, 49.0, MDDetector::c_Calorimeter, 1);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: calorimeter hit of 49 keV", "The hit is removed", Event.GetNRESEs(), 0) && Passed;
  }
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_CalorimeterPosition, 51.0, MDDetector::c_Calorimeter, 1);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: calorimeter hit of 51 keV", "The hit is kept", Event.GetNRESEs(), 1) && Passed;
  }
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: calorimeter hits of 49 keV and 51 keV", "Three events have triggered", Noising.GetNTriggeredEvents(), 3L) && Passed;
  Passed = Evaluate("GetNNotTriggeredOrVetoedEvents()", "engine 1: calorimeter hits of 49 keV and 51 keV", "Three events have not triggered", Noising.GetNNotTriggeredOrVetoedEvents(), 3L) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the veto and the trigger statistics with the detector effects engine 1
bool UTERNoising::TestEngine1Veto()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 1", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(1))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // A shield hit above its threshold of 50 keV vetoes the triggers of the tracker and the calorimeter
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_CalorimeterPosition, 300.0, MDDetector::c_Calorimeter, 2);
    AddHit(Event, Geometry, m_ShieldPosition, 51.0, MDDetector::c_Scintillator, 3);
    Passed = EvaluateTrue("Analyze()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "All hits are removed", Event.GetNRESEs(), 0) && Passed;
  }
  Passed = Evaluate("GetNVetoedEvents()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "One event was vetoed", Noising.GetNVetoedEvents(), 1L) && Passed;
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "No event has triggered", Noising.GetNTriggeredEvents(), 0L) && Passed;
  Passed = Evaluate("GetNNotTriggeredOrVetoedEvents()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "A vetoed event is not counted as not triggered", Noising.GetNNotTriggeredOrVetoedEvents(), 0L) && Passed;
  Passed = Evaluate("GetVetoMapVetoNames()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "The shield veto was raised once", Noising.GetVetoMapVetoNames()["ShieldVeto"], 1L) && Passed;
  Passed = EvaluateSize("GetVetoMapVetoNames()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "No other veto was raised", Noising.GetVetoMapVetoNames().size(), 1) && Passed;
  Passed = Evaluate("GetVetoMapTriggerNames()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "The tracker trigger was canceled once", Noising.GetVetoMapTriggerNames()["TrackerTrigger"], 1L) && Passed;
  Passed = Evaluate("GetVetoMapTriggerNames()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "The calorimeter trigger was canceled once", Noising.GetVetoMapTriggerNames()["CalorimeterTrigger"], 1L) && Passed;
  Passed = EvaluateSize("GetVetoMapTriggerNames()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "No other trigger was canceled", Noising.GetVetoMapTriggerNames().size(), 2) && Passed;
  Passed = EvaluateSize("GetTriggerMapTriggerNames()", "engine 1: tracker, calorimeter, and shield hit of 51 keV", "The trigger map of the triggered events is empty", Noising.GetTriggerMapTriggerNames().size(), 0) && Passed;

  // A shield hit below its threshold (the noise threshold equals the trigger threshold) is removed and does not veto
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_ShieldPosition, 49.0, MDDetector::c_Scintillator, 3);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: tracker hit and shield hit of 49 keV", "Only the tracker hit is kept", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = Evaluate("Analyze()", "engine 1: tracker hit and shield hit of 49 keV", "The kept hit is in the tracker", Event.GetRESEAt(0)->GetDetector(), static_cast<int>(MDDetector::c_Strip2D)) && Passed;
    }
  }
  Passed = Evaluate("GetNVetoedEvents()", "engine 1: tracker hit and shield hit of 49 keV", "Still one event was vetoed", Noising.GetNVetoedEvents(), 1L) && Passed;
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: tracker hit and shield hit of 49 keV", "One event has triggered", Noising.GetNTriggeredEvents(), 1L) && Passed;

  // A shield hit alone is a veto without a trigger
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_ShieldPosition, 200.0, MDDetector::c_Scintillator, 3);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: shield hit only", "The hit is removed", Event.GetNRESEs(), 0) && Passed;
  }
  Passed = Evaluate("GetNVetoedEvents()", "engine 1: shield hit only", "Two events were vetoed", Noising.GetNVetoedEvents(), 2L) && Passed;
  Passed = Evaluate("GetVetoMapVetoNames()", "engine 1: shield hit only", "The shield veto was raised twice", Noising.GetVetoMapVetoNames()["ShieldVeto"], 2L) && Passed;
  Passed = EvaluateSize("GetVetoMapTriggerNames()", "engine 1: shield hit only", "No further trigger was canceled", Noising.GetVetoMapTriggerNames().size(), 2) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the guard ring hits with the detector effects engine 1
bool UTERNoising::TestEngine1GuardRing()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 1 with guard ring", "The telescope with a guard ring veto can be read", ScanGeometry(Geometry, GetNoisingGeometry(1, false, true))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // The guard ring veto has a threshold of 40 keV, the resolution of the guard ring is 0.001 keV: 41 keV is 1000 sigma above it
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    Passed = EvaluateTrue("AddREAM()", "guard ring hit", "The position has a guard ring", AddGuardRingHit(Event, Geometry, m_GuardRingPosition, 41.0)) && Passed;
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: tracker hit and guard ring hit of 41 keV", "The hits are removed", Event.GetNRESEs(), 0) && Passed;
  }
  Passed = Evaluate("GetNVetoedEvents()", "engine 1: tracker hit and guard ring hit of 41 keV", "The event was vetoed", Noising.GetNVetoedEvents(), 1L) && Passed;
  Passed = Evaluate("GetVetoMapVetoNames()", "engine 1: tracker hit and guard ring hit of 41 keV", "The guard ring veto was raised once", Noising.GetVetoMapVetoNames()["GuardRingVeto"], 1L) && Passed;

  // 39 keV is 1000 sigma below the threshold: no veto
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddGuardRingHit(Event, Geometry, m_GuardRingPosition, 39.0);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 1: tracker hit and guard ring hit of 39 keV", "The tracker hit is kept", Event.GetNRESEs(), 1) && Passed;
  }
  Passed = Evaluate("GetNVetoedEvents()", "engine 1: tracker hit and guard ring hit of 39 keV", "Still one event was vetoed", Noising.GetNVetoedEvents(), 1L) && Passed;
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: tracker hit and guard ring hit of 39 keV", "The event has triggered", Noising.GetNTriggeredEvents(), 1L) && Passed;

  // A guard ring hit above the threshold is noised: energy within 5 sigma, resolution of the guard ring
  {
    MRERawEvent Event(&Geometry);
    AddGuardRingHit(Event, Geometry, m_GuardRingPosition, 100.0);
    Noising.Analyze(&Event);
    Passed = EvaluateSize("Analyze()", "engine 1: guard ring hit of 100 keV", "The guard ring hit is kept", Event.GetNREAMs(), 1) && Passed;
    if (Event.GetNREAMs() == 1) {
      MREAMGuardRingHit* GuardRingHit = dynamic_cast<MREAMGuardRingHit*>(Event.GetREAMAt(0));
      Passed = EvaluateNear("Analyze()", "engine 1: guard ring hit of 100 keV", "The energy is noised with a sigma of 0.001 keV", GuardRingHit->GetEnergy(), 100.0, 0.005) && Passed;
      Passed = EvaluateNear("Analyze()", "engine 1: guard ring hit of 100 keV", "The energy resolution is the one of the guard ring", GuardRingHit->GetEnergyResolution(), 0.001, 1e-12) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an event with a cluster with the detector effects engine 1
bool UTERNoising::TestEngine1NoHit()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 1", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(1))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // A cluster cannot be noised: it does not trigger and is removed with the not triggered event
  MRERawEvent Event(&Geometry);
  MRECluster* Cluster = new MRECluster();
  Cluster->AddRESE(CreateHit(m_TrackerPosition, 200.0, MDDetector::c_Strip2D, &Geometry));
  Event.AddRESE(Cluster);
  DisableDefaultStreams();
  bool Analyzed = Noising.Analyze(&Event);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Analyze()", "engine 1: event with a cluster", "The analysis succeeds", Analyzed) && Passed;
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: event with a cluster", "The event has not triggered", Noising.GetNTriggeredEvents(), 0L) && Passed;
  Passed = Evaluate("GetNNotTriggeredOrVetoedEvents()", "engine 1: event with a cluster", "The event is counted as not triggered", Noising.GetNNotTriggeredOrVetoedEvents(), 1L) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noised energy with the detector effects engine 1
bool UTERNoising::TestEngine1EnergyResolution()
{
  bool Passed = true;

  // The telescope with its Gaussian resolutions: 2 keV in the tracker
  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 1, Gaussian resolutions", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 1\n")) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // The mean and the sigma of 2000 noised hits of 200 keV: the error of the mean is 2/sqrt(2000) = 0.045 keV, the error of the sigma 2/sqrt(4000) = 0.032 keV - 5 sigma each
  unsigned int NEvents = 2000;
  double Sum = 0;
  double Sum2 = 0;
  unsigned int NKept = 0;
  for (unsigned int e = 0; e < NEvents; ++e) {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    Noising.Analyze(&Event);
    if (Event.GetNRESEs() == 1) {
      ++NKept;
      Sum += Event.GetRESEAt(0)->GetEnergy();
      Sum2 += Event.GetRESEAt(0)->GetEnergy()*Event.GetRESEAt(0)->GetEnergy();
    }
  }
  Passed = Evaluate("Analyze()", "engine 1: 2000 tracker hits of 200 keV", "All hits are kept", NKept, NEvents) && Passed;
  double Mean = Sum/NEvents;
  double Sigma = sqrt(Sum2/NEvents - Mean*Mean);
  Passed = EvaluateNear("Analyze()", "engine 1: 2000 tracker hits of 200 keV", "The mean energy is 200 keV", Mean, 200.0, 0.23) && Passed;
  Passed = EvaluateNear("Analyze()", "engine 1: 2000 tracker hits of 200 keV", "The sigma of the energy is 2 keV", Sigma, 2.0, 0.16) && Passed;
  Passed = Evaluate("GetNTriggeredEvents()", "engine 1: 2000 tracker hits of 200 keV", "All events have triggered", Noising.GetNTriggeredEvents(), 2000L) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the hits in the strip, calorimeter, and scintillator detectors with the detector effects engine 2
bool UTERNoising::TestEngine2Hits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 2", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(2))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // The tracker: the engine moves the hit to the center of its strip crossing (1, 2, 0)
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 7);
    Passed = EvaluateTrue("Analyze()", "engine 2: tracker hit of 200 keV", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 2: tracker hit of 200 keV", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      Passed = Evaluate("Analyze()", "engine 2: tracker hit of 200 keV", "It is a hit", Hit->GetType(), MRESE::c_Hit) && Passed;
      Passed = Evaluate("Analyze()", "engine 2: tracker hit of 200 keV", "The detector type is Strip2D", Hit->GetDetector(), static_cast<int>(MDDetector::c_Strip2D)) && Passed;
      Passed = EvaluateNear("Analyze()", "engine 2: tracker hit of 200 keV", "The energy is not changed by an ideal resolution", Hit->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "engine 2: tracker hit of 200 keV", "The hit is at the center of its strip crossing", Hit->GetPosition(), MVector(1.0, 2.0, 0.0), 1e-12) && Passed;
      // Position resolution: strip pitch 1 cm / sqrt(12), wafer thickness 0.05 cm / sqrt(12)
      Passed = EvaluateVectorNear("Analyze()", "engine 2: tracker hit of 200 keV", "The position resolution is the one of the strip crossing", Hit->GetPositionResolution(), MVector(1.0/sqrt(12.0), 1.0/sqrt(12.0), 0.05/sqrt(12.0)), 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "engine 2: tracker hit of 200 keV", "The energy resolution is ideal", Hit->GetEnergyResolution(), 0.0, 1e-12) && Passed;
      Passed = EvaluateSize("Analyze()", "engine 2: tracker hit of 200 keV", "The hit has one origin", Hit->GetOriginIDs().size(), 1) && Passed;
      Passed = EvaluateSize("Analyze()", "engine 2: tracker hit of 200 keV", "The origin is kept", Hit->GetOriginIDs().count(7), 1) && Passed;
      Passed = Evaluate("Analyze()", "engine 2: tracker hit of 200 keV", "There are no noise flags", Hit->GetNoiseFlags(), MString("")) && Passed;
      Passed = Evaluate("Analyze()", "engine 2: tracker hit of 200 keV", "The volume sequence points to the tracker", Hit->GetVolumeSequence()->GetDetector()->GetName(), MString("Tracker")) && Passed;
      Passed = Evaluate("Analyze()", "engine 2: tracker hit of 200 keV", "The grid point is the x strip 5", Hit->GetGridPoint().GetXGrid(), 5U) && Passed;
      Passed = Evaluate("Analyze()", "engine 2: tracker hit of 200 keV", "The grid point is the y strip 6", Hit->GetGridPoint().GetYGrid(), 6U) && Passed;
    }
    Passed = EvaluateTrue("Analyze()", "engine 2: tracker hit of 200 keV", "The event is valid", Event.IsValid()) && Passed;
  }

  // The calorimeter: one block, the hit is at its center
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_CalorimeterPosition, 200.0, MDDetector::c_Calorimeter, 8);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 2: calorimeter hit of 200 keV", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      Passed = Evaluate("Analyze()", "engine 2: calorimeter hit of 200 keV", "It is a hit", Hit->GetType(), MRESE::c_Hit) && Passed;
      Passed = Evaluate("Analyze()", "engine 2: calorimeter hit of 200 keV", "The detector type is Calorimeter", Hit->GetDetector(), static_cast<int>(MDDetector::c_Calorimeter)) && Passed;
      Passed = EvaluateNear("Analyze()", "engine 2: calorimeter hit of 200 keV", "The energy is not changed by an ideal resolution", Hit->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "engine 2: calorimeter hit of 200 keV", "The hit is at the center of the block", Hit->GetPosition(), MVector(0.0, 0.0, -10.0), 1e-12) && Passed;
      Passed = EvaluateSize("Analyze()", "engine 2: calorimeter hit of 200 keV", "The origin is kept", Hit->GetOriginIDs().count(8), 1) && Passed;
    }
  }

  // Hits in three tracker layers and the calorimeter: all are kept, each in its own layer
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 100.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, MVector(1.2, 2.1, GetTrackerLayerZ(1)), 110.0, MDDetector::c_Strip2D, 2);
    AddHit(Event, Geometry, m_TrackerPosition3, 120.0, MDDetector::c_Strip2D, 3);
    AddHit(Event, Geometry, m_CalorimeterPosition, 130.0, MDDetector::c_Calorimeter, 4);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 2: three tracker hits and a calorimeter hit", "There are four hits", Event.GetNRESEs(), 4) && Passed;
    Passed = EvaluateNear("GetEnergy()", "engine 2: three tracker hits and a calorimeter hit", "The event has 460 keV", Event.GetEnergy(), 460.0, 1e-12) && Passed;
    bool FoundLayer0 = false;
    bool FoundLayer1 = false;
    bool FoundLayer3 = false;
    bool FoundCalorimeter = false;
    for (int r = 0; r < Event.GetNRESEs(); ++r) {
      MRESE* Hit = Event.GetRESEAt(r);
      if (fabs(Hit->GetEnergy() - 100.0) < 1e-12 && (Hit->GetPosition() - MVector(1.0, 2.0, 0.0)).Mag() < 1e-12) FoundLayer0 = true;
      if (fabs(Hit->GetEnergy() - 110.0) < 1e-12 && (Hit->GetPosition() - MVector(1.0, 2.0, 1.0)).Mag() < 1e-12) FoundLayer1 = true;
      if (fabs(Hit->GetEnergy() - 120.0) < 1e-12 && (Hit->GetPosition() - MVector(-3.0, 2.0, 3.0)).Mag() < 1e-12) FoundLayer3 = true;
      if (fabs(Hit->GetEnergy() - 130.0) < 1e-12 && (Hit->GetPosition() - MVector(0.0, 0.0, -10.0)).Mag() < 1e-12) FoundCalorimeter = true;
    }
    Passed = EvaluateTrue("Analyze()", "engine 2: three tracker hits and a calorimeter hit", "The hit of 100 keV is at (1, 2, 0)", FoundLayer0) && Passed;
    Passed = EvaluateTrue("Analyze()", "engine 2: three tracker hits and a calorimeter hit", "The hit of 110 keV is at (1, 2, 1)", FoundLayer1) && Passed;
    Passed = EvaluateTrue("Analyze()", "engine 2: three tracker hits and a calorimeter hit", "The hit of 120 keV is at (-3, 2, 3)", FoundLayer3) && Passed;
    Passed = EvaluateTrue("Analyze()", "engine 2: three tracker hits and a calorimeter hit", "The hit of 130 keV is at the center of the calorimeter", FoundCalorimeter) && Passed;
  }

  // An empty event stays empty
  {
    MRERawEvent Event(&Geometry);
    Passed = EvaluateTrue("Analyze()", "engine 2: empty event", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 2: empty event", "The event stays empty", Event.GetNRESEs(), 0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the strip hits of the depth detector with the detector effects engine 2
bool UTERNoising::TestEngine2StripHits()
{
  bool Passed = true;

  // Without depth resolution the depth is not measured
  {
    MGeometryRevan Geometry;
    if (EvaluateTrue("ScanSetupFile()", "engine 2 without depth resolution", "The telescope can be read", ScanGeometry(Geometry, GetNoisingGeometry(2))) == false) {
      return false;
    }
    MERNoising Noising;
    Noising.SetGeometry(&Geometry);
    Noising.PreAnalysis();

    // The hit is in x strip 5 and y strip 5 of 10 strips with pitch 1 cm: the strip centers are at 0.5 cm in the wafer
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_DepthPosition, 200.0, MDDetector::c_Strip3D, 9);
    Passed = EvaluateTrue("Analyze()", "engine 2: depth detector hit of 200 keV", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 2: depth detector hit of 200 keV", "There are two strip hits", Event.GetNRESEs(), 2) && Passed;
    if (Event.GetNRESEs() == 2) {
      MREStripHit* XStrip = nullptr;
      MREStripHit* YStrip = nullptr;
      for (int r = 0; r < 2; ++r) {
        Passed = Evaluate("Analyze()", "engine 2: depth detector hit of 200 keV", "The element is a strip hit", Event.GetRESEAt(r)->GetType(), MRESE::c_StripHit) && Passed;
        MREStripHit* StripHit = dynamic_cast<MREStripHit*>(Event.GetRESEAt(r));
        if (StripHit != nullptr && StripHit->IsXStrip() == true) XStrip = StripHit;
        if (StripHit != nullptr && StripHit->IsXStrip() == false) YStrip = StripHit;
      }
      Passed = EvaluateTrue("Analyze()", "engine 2: depth detector hit of 200 keV", "There is an x strip hit", XStrip != nullptr) && Passed;
      Passed = EvaluateTrue("Analyze()", "engine 2: depth detector hit of 200 keV", "There is a y strip hit", YStrip != nullptr) && Passed;
      if (XStrip != nullptr && YStrip != nullptr) {
        MREStripHit* Strips[2] = { XStrip, YStrip };
        MString Names[2] = { "engine 2: x strip of the depth detector hit", "engine 2: y strip of the depth detector hit" };
        for (unsigned int s = 0; s < 2; ++s) {
          // Each side measures the full energy
          Passed = EvaluateNear("Analyze()", Names[s], "The strip has the full energy", Strips[s]->GetEnergy(), 200.0, 1e-12) && Passed;
          Passed = Evaluate("Analyze()", Names[s], "The strip ID is 5", Strips[s]->GetStripID(), 5) && Passed;
          Passed = EvaluateNear("Analyze()", Names[s], "The strip is centered at 0.5 cm in the wafer", Strips[s]->GetNonStripPosition(), 0.5, 1e-12) && Passed;
          Passed = EvaluateNear("Analyze()", Names[s], "The depth is the center of the wafer", Strips[s]->GetDepthPosition(), 0.0, 1e-12) && Passed;
          Passed = Evaluate("Analyze()", Names[s], "The missing depth is flagged", Strips[s]->GetNoiseFlags(), MString(" NODEPTH")) && Passed;
          Passed = Evaluate("Analyze()", Names[s], "The detector type is Strip3D", Strips[s]->GetDetector(), static_cast<int>(MDDetector::c_Strip3D)) && Passed;
          Passed = Evaluate("Analyze()", Names[s], "The volume sequence points to the depth detector", Strips[s]->GetVolumeSequence()->GetDetector()->GetName(), MString("Depth")) && Passed;
          Passed = EvaluateNear("Analyze()", Names[s], "The energy resolution is ideal", Strips[s]->GetEnergyResolution(), 0.0, 1e-12) && Passed;
          Passed = EvaluateSize("Analyze()", Names[s], "The strip has one origin", Strips[s]->GetOriginIDs().size(), 1) && Passed;
          Passed = EvaluateSize("Analyze()", Names[s], "The origin is kept", Strips[s]->GetOriginIDs().count(9), 1) && Passed;
          Passed = EvaluateTrue("Analyze()", Names[s], "The strip hit is valid", Strips[s]->IsValid()) && Passed;
        }
        Passed = Evaluate("Analyze()", "engine 2: depth detector hit of 200 keV", "Both strips have the same detector ID", XStrip->GetDetectorID(), YStrip->GetDetectorID()) && Passed;
        // The world position of a strip is only defined along its measured axis: wafer center 30 cm + 0.5 cm in x, 0 cm + 0.5 cm in y
        Passed = EvaluateNear("Analyze()", "engine 2: x strip of the depth detector hit", "The world x position is 30.5 cm", XStrip->GetPositionX(), 30.5, 1e-12) && Passed;
        Passed = EvaluateNear("Analyze()", "engine 2: y strip of the depth detector hit", "The world y position is 0.5 cm", YStrip->GetPositionY(), 0.5, 1e-12) && Passed;
      }
    }
    // Both sides measure the same energy deposit: the event has 200 keV, not 400 keV
    Passed = EvaluateNear("GetEnergy()", "engine 2: depth detector hit of 200 keV", "The event has 200 keV", Event.GetEnergy(), 200.0, 1e-12) && Passed;
  }

  // With a depth resolution of 0.1 cm each strip gets its own noised depth around 0.5 cm
  {
    MGeometryRevan Geometry;
    if (EvaluateTrue("ScanSetupFile()", "engine 2 with depth resolution", "The telescope can be read", ScanGeometry(Geometry, GetNoisingGeometry(2, true))) == false) {
      return false;
    }
    MERNoising Noising;
    Noising.SetGeometry(&Geometry);
    Noising.PreAnalysis();

    // The mean and the sigma of the depth of 1000 hits, x and y strips together: the error of the mean is 0.1/sqrt(2000) = 0.0022 cm, of the sigma 0.1/sqrt(4000) = 0.0016 cm - 5 sigma each
    // The depth is limited to the wafer (+-1 cm), which is 5 sigma away from 0.5 cm
    unsigned int NEvents = 1000;
    unsigned int NStrips = 0;
    unsigned int NFlagged = 0;
    double Sum = 0;
    double Sum2 = 0;
    for (unsigned int e = 0; e < NEvents; ++e) {
      MRERawEvent Event(&Geometry);
      AddHit(Event, Geometry, m_DepthPosition, 200.0, MDDetector::c_Strip3D, 9);
      Noising.Analyze(&Event);
      for (int r = 0; r < Event.GetNRESEs(); ++r) {
        MREStripHit* StripHit = dynamic_cast<MREStripHit*>(Event.GetRESEAt(r));
        if (StripHit != nullptr) {
          ++NStrips;
          Sum += StripHit->GetDepthPosition();
          Sum2 += StripHit->GetDepthPosition()*StripHit->GetDepthPosition();
          if (StripHit->GetNoiseFlags() != "") ++NFlagged;
        }
      }
    }
    Passed = Evaluate("Analyze()", "engine 2: 1000 depth detector hits", "There are 2000 strip hits", NStrips, 2000U) && Passed;
    Passed = Evaluate("Analyze()", "engine 2: 1000 depth detector hits", "No strip hit has a noise flag", NFlagged, 0U) && Passed;
    if (NStrips > 0) {
      double Mean = Sum/NStrips;
      double Sigma = sqrt(Sum2/NStrips - Mean*Mean);
      Passed = EvaluateNear("Analyze()", "engine 2: 1000 depth detector hits", "The mean depth is 0.5 cm", Mean, 0.5, 0.011) && Passed;
      Passed = EvaluateNear("Analyze()", "engine 2: 1000 depth detector hits", "The sigma of the depth is 0.1 cm", Sigma, 0.1, 0.008) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the thresholds, the merging of hits, and the event time with the detector effects engine 2
bool UTERNoising::TestEngine2Thresholds()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 2", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(2))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // 31 keV is above the trigger threshold of the tracker of 30 keV, 29 keV below
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 31.0, MDDetector::c_Strip2D, 1);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 2: tracker hit of 31 keV", "The hit is kept", Event.GetNRESEs(), 1) && Passed;
  }
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 29.0, MDDetector::c_Strip2D, 1);
    Passed = EvaluateTrue("Analyze()", "engine 2: tracker hit of 29 keV", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 2: tracker hit of 29 keV", "The event is empty", Event.GetNRESEs(), 0) && Passed;
  }

  // A triggering hit keeps a hit between the noise threshold of 10 keV and the trigger threshold, but not one below the noise threshold
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_TrackerPosition3, 11.0, MDDetector::c_Strip2D, 2);
    AddHit(Event, Geometry, MVector(3.0, -2.0, GetTrackerLayerZ(1)), 9.0, MDDetector::c_Strip2D, 3);
    BeginCaptureOutput();
    Noising.Analyze(&Event);
    MString Output = EndCaptureOutput();
    Passed = Evaluate("Analyze()", "engine 2: 200 keV, 11 keV, and 9 keV in the tracker", "Two hits are kept", Event.GetNRESEs(), 2) && Passed;
    // Defect REV-E-4 (geomega): for each hit which is not read out the engine asks for the sigmas above the noise level, which is an error message for a not Gaussian energy resolution
    Passed = ExpectedToFail(Evaluate("Analyze()", "engine 2: hit below the noise threshold, ideal energy resolution", "No error message is printed", Output, MString(""))) && Passed;
    Passed = EvaluateNear("GetEnergy()", "engine 2: 200 keV, 11 keV, and 9 keV in the tracker", "The event has 211 keV", Event.GetEnergy(), 211.0, 1e-12) && Passed;
  }

  // Two hits in the same strip crossing are one hit with the sum of the energies and both origins: 20 keV + 15 keV trigger together
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 20.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_TrackerPosition + MVector(0.1, -0.2, 0.0), 15.0, MDDetector::c_Strip2D, 2);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 2: 20 keV and 15 keV in one strip crossing", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = EvaluateNear("Analyze()", "engine 2: 20 keV and 15 keV in one strip crossing", "The hit has 35 keV", Event.GetRESEAt(0)->GetEnergy(), 35.0, 1e-12) && Passed;
      Passed = EvaluateSize("Analyze()", "engine 2: 20 keV and 15 keV in one strip crossing", "The hit has both origins", Event.GetRESEAt(0)->GetOriginIDs().size(), 2) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "engine 2: 20 keV and 15 keV in one strip crossing", "The hit is at the center of the strip crossing", Event.GetRESEAt(0)->GetPosition(), MVector(1.0, 2.0, 0.0), 1e-12) && Passed;
    }
  }

  // The event time is not changed without a time resolution of the system
  {
    MRERawEvent Event(&Geometry);
    Event.SetEventTime(MTime(1234.5));
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    Noising.Analyze(&Event);
    Passed = EvaluateNear("Analyze()", "engine 2: event time 1234.5 s", "The event time is kept", Event.GetEventTime().GetAsSeconds(), 1234.5, 1e-9) && Passed;
  }

  // A cluster cannot be noised: it is removed
  {
    MRERawEvent Event(&Geometry);
    MRECluster* Cluster = new MRECluster();
    Cluster->AddRESE(CreateHit(m_TrackerPosition, 200.0, MDDetector::c_Strip2D, &Geometry));
    Event.AddRESE(Cluster);
    DisableDefaultStreams();
    bool Analyzed = Noising.Analyze(&Event);
    EnableDefaultStreams();
    Passed = EvaluateTrue("Analyze()", "engine 2: event with a cluster", "The analysis succeeds", Analyzed) && Passed;
    Passed = Evaluate("Analyze()", "engine 2: event with a cluster", "The event is empty", Event.GetNRESEs(), 0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the veto and the trigger statistics with the detector effects engine 2
bool UTERNoising::TestEngine2Veto()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 2", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(2))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // A triggered event
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 2: tracker hit of 200 keV", "The hit is kept", Event.GetNRESEs(), 1) && Passed;
  }
  // Defect REV-E-3: the detector effects engine 2 path never fills the trigger statistics
  Passed = ExpectedToFail(Evaluate("GetNTriggeredEvents()", "engine 2: tracker hit of 200 keV", "One event has triggered", Noising.GetNTriggeredEvents(), 1L)) && Passed;
  Passed = ExpectedToFail(Evaluate("GetTriggerMapTriggerNames()", "engine 2: tracker hit of 200 keV", "The tracker trigger was raised once", Noising.GetTriggerMapTriggerNames()["TrackerTrigger"], 1L)) && Passed;

  // A shield hit above its threshold of 50 keV vetoes the event
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_ShieldPosition, 51.0, MDDetector::c_Scintillator, 3);
    Passed = EvaluateTrue("Analyze()", "engine 2: tracker hit and shield hit of 51 keV", "The analysis succeeds", Noising.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "engine 2: tracker hit and shield hit of 51 keV", "All hits are removed", Event.GetNRESEs(), 0) && Passed;
  }
  // Defect REV-E-3: see above
  Passed = ExpectedToFail(Evaluate("GetNVetoedEvents()", "engine 2: tracker hit and shield hit of 51 keV", "One event was vetoed", Noising.GetNVetoedEvents(), 1L)) && Passed;
  Passed = ExpectedToFail(Evaluate("GetVetoMapVetoNames()", "engine 2: tracker hit and shield hit of 51 keV", "The shield veto was raised once", Noising.GetVetoMapVetoNames()["ShieldVeto"], 1L)) && Passed;

  // A shield hit below its threshold does not veto and is not read out
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_ShieldPosition, 49.0, MDDetector::c_Scintillator, 3);
    // The error message of defect REV-E-4 is hidden
    DisableDefaultStreams();
    Noising.Analyze(&Event);
    EnableDefaultStreams();
    Passed = Evaluate("Analyze()", "engine 2: tracker hit and shield hit of 49 keV", "Only the tracker hit is kept", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = Evaluate("Analyze()", "engine 2: tracker hit and shield hit of 49 keV", "The kept hit is in the tracker", Event.GetRESEAt(0)->GetDetector(), static_cast<int>(MDDetector::c_Strip2D)) && Passed;
    }
  }

  // An event which does not trigger
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 29.0, MDDetector::c_Strip2D, 1);
    Noising.Analyze(&Event);
  }
  // Defect REV-E-3: see above
  Passed = ExpectedToFail(Evaluate("GetNNotTriggeredOrVetoedEvents()", "engine 2: tracker hit of 29 keV", "One event has not triggered", Noising.GetNNotTriggeredOrVetoedEvents(), 1L)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the guard ring hits with the detector effects engine 2
bool UTERNoising::TestEngine2GuardRing()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 2 with guard ring", "The telescope with a guard ring veto can be read", ScanGeometry(Geometry, GetNoisingGeometry(2, false, true))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();

  // The guard ring veto has a threshold of 40 keV, the resolution of the guard ring is 0.001 keV: 41 keV is 1000 sigma above it
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    Passed = EvaluateTrue("AddREAM()", "guard ring hit", "The position has a guard ring", AddGuardRingHit(Event, Geometry, m_GuardRingPosition, 41.0)) && Passed;
    Noising.Analyze(&Event);
    Passed = Evaluate("Analyze()", "engine 2: tracker hit and guard ring hit of 41 keV", "The hits are removed", Event.GetNRESEs(), 0) && Passed;
    Passed = EvaluateSize("Analyze()", "engine 2: tracker hit and guard ring hit of 41 keV", "The guard ring hit was handed to the engine", Event.GetNREAMs(), 0) && Passed;
  }

  // 39 keV is 1000 sigma below the threshold: no veto
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddGuardRingHit(Event, Geometry, m_GuardRingPosition, 39.0);
    Noising.Analyze(&Event);
    // Defect GEO-E-35 (geomega): the guard ring stays at engine version 1, where each guard ring hit vetoes
    Passed = ExpectedToFail(Evaluate("Analyze()", "engine 2: tracker hit and guard ring hit of 39 keV", "The tracker hit is kept", Event.GetNRESEs(), 1)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding the statistics of two noising stages
bool UTERNoising::TestAddStatistics()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 1", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(1))) == false) {
    return false;
  }

  // The first: two triggered tracker events, one not triggered
  MERNoising First;
  First.SetGeometry(&Geometry);
  First.PreAnalysis();
  for (unsigned int e = 0; e < 3; ++e) {
    MRERawEvent Event(&Geometry);
    double Energy = 200.0;
    if (e == 2) Energy = 29.0;
    AddHit(Event, Geometry, m_TrackerPosition, Energy, MDDetector::c_Strip2D, 1);
    First.Analyze(&Event);
  }

  // The second: one triggered tracker event, one triggered calorimeter event, one vetoed tracker event
  MERNoising Second;
  Second.SetGeometry(&Geometry);
  Second.PreAnalysis();
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    Second.Analyze(&Event);
  }
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_CalorimeterPosition, 200.0, MDDetector::c_Calorimeter, 1);
    Second.Analyze(&Event);
  }
  {
    MRERawEvent Event(&Geometry);
    AddHit(Event, Geometry, m_TrackerPosition, 200.0, MDDetector::c_Strip2D, 1);
    AddHit(Event, Geometry, m_ShieldPosition, 200.0, MDDetector::c_Scintillator, 2);
    Second.Analyze(&Event);
  }

  First.AddStatistics(&Second);
  Passed = Evaluate("AddStatistics()", "2 + 2 triggered events", "Four events have triggered", First.GetNTriggeredEvents(), 4L) && Passed;
  Passed = Evaluate("AddStatistics()", "0 + 1 vetoed events", "One event was vetoed", First.GetNVetoedEvents(), 1L) && Passed;
  Passed = Evaluate("AddStatistics()", "1 + 0 not triggered events", "One event has not triggered", First.GetNNotTriggeredOrVetoedEvents(), 1L) && Passed;
  Passed = Evaluate("AddStatistics()", "2 + 1 tracker triggers", "The tracker trigger was raised three times", First.GetTriggerMapTriggerNames()["TrackerTrigger"], 3L) && Passed;
  Passed = Evaluate("AddStatistics()", "0 + 1 calorimeter triggers", "The calorimeter trigger was raised once", First.GetTriggerMapTriggerNames()["CalorimeterTrigger"], 1L) && Passed;
  Passed = Evaluate("AddStatistics()", "0 + 1 shield vetoes", "The shield veto was raised once", First.GetVetoMapVetoNames()["ShieldVeto"], 1L) && Passed;
  Passed = Evaluate("AddStatistics()", "0 + 1 canceled tracker triggers", "The tracker trigger was canceled once", First.GetVetoMapTriggerNames()["TrackerTrigger"], 1L) && Passed;

  // The added one is not changed
  Passed = Evaluate("AddStatistics()", "the added statistics", "The added statistics still has two triggered events", Second.GetNTriggeredEvents(), 2L) && Passed;
  Passed = Evaluate("AddStatistics()", "the added statistics", "The added statistics still has one vetoed event", Second.GetNVetoedEvents(), 1L) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the summary text
bool UTERNoising::TestToString()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "engine 1", "The telescope with ideal resolutions can be read", ScanGeometry(Geometry, GetNoisingGeometry(1))) == false) {
    return false;
  }
  MERNoising Noising;
  Noising.SetGeometry(&Geometry);
  Noising.PreAnalysis();
  Passed = Evaluate("ToString()", "no events", "The summary of no events is empty", Noising.ToString(), MString("")) && Passed;

  // Two triggered events, one vetoed event, three not triggered events
  for (unsigned int e = 0; e < 6; ++e) {
    MRERawEvent Event(&Geometry);
    double Energy = 200.0;
    if (e >= 3) Energy = 29.0;
    AddHit(Event, Geometry, m_TrackerPosition, Energy, MDDetector::c_Strip2D, 1);
    if (e == 2) AddHit(Event, Geometry, m_ShieldPosition, 200.0, MDDetector::c_Scintillator, 2);
    Noising.Analyze(&Event);
  }

  MString Text = Noising.ToString();
  Passed = EvaluateTrue("ToString()", "2 triggered, 1 vetoed, 3 not triggered", "The text has a title", Text.Contains("Trigger statistics:")) && Passed;
  // The counts are right-aligned in six characters
  Passed = EvaluateTrue("ToString()", "2 triggered, 1 vetoed, 3 not triggered", "The text has the number of not triggered events", Text.Contains("Not triggered events: ..................................      3")) && Passed;
  Passed = EvaluateTrue("ToString()", "2 triggered, 1 vetoed, 3 not triggered", "The text has the number of vetoed events", Text.Contains("Number of vetoed events: ...............................      1")) && Passed;
  Passed = EvaluateTrue("ToString()", "2 triggered, 1 vetoed, 3 not triggered", "The text has the number of triggered events", Text.Contains("Number of triggered events: ............................      2")) && Passed;
  // The names are filled with dots to 50 characters: ShieldVeto has 10, TrackerTrigger 14
  Passed = EvaluateTrue("ToString()", "2 triggered, 1 vetoed, 3 not triggered", "The text has the raised veto", Text.Contains("      ShieldVeto: ........................................      1")) && Passed;
  Passed = EvaluateTrue("ToString()", "2 triggered, 1 vetoed, 3 not triggered", "The text has the raised trigger", Text.Contains("      TrackerTrigger: ....................................      2")) && Passed;
  Passed = EvaluateTrue("ToString()", "2 triggered, 1 vetoed, 3 not triggered", "The text has the canceled trigger", Text.Contains("      TrackerTrigger: ....................................      1")) && Passed;
  Passed = Evaluate("ToString()", "core only", "The core text is the same", Noising.ToString(true), Text) && Passed;

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

  UTERNoising Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERNoising.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
