/*
 * UTDTriggerUnit.cxx
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
#include <sstream>
#include <vector>
using namespace std;

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TRandom.h>
#include <TROOT.h>

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGeometry.h"
#include "MDTrigger.h"
#include "MDTriggerBasic.h"
#include "MDTriggerMap.h"
#include "MDTriggerUnit.h"
#include "MDVolumeSequence.h"
#include "MString.h"
#include "MVector.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the trigger unit MDTriggerUnit
class UTDTriggerUnit : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDTriggerUnit();
  //! Default destructor
  virtual ~UTDTriggerUnit() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a geometry: Strips (two wafers with guard rings, threshold 30, guard ring 40), Scinti (threshold 50) and Shield (noise equals trigger threshold 50)
  MString GetGeometry() const;
  //! Scan the geometry with the additional setup lines, and report if it fails
  bool Setup(MDGeometry& Geometry, const MString& Lines, const MString& Input);

  //! Test a geometry without any trigger
  bool TestNoTriggers();
  //! Test the return values and thresholds of adding a hit
  bool TestAddHitThresholds();
  //! Test the thresholds with a uncertainty
  bool TestNoisedThresholds();
  //! Test blocked channels
  bool TestBlockedChannels();
  //! Test ignoring the thresholds
  bool TestIgnoreThresholds();
  //! Test hits which have no detector
  bool TestHitsWithoutDetector();
  //! Test adding hits with volume sequences
  bool TestAddHitVolumeSequence();
  //! Test guard ring hits
  bool TestGuardRingHits();
  //! Test channel against detector count with real positions
  bool TestChannelAgainstDetector();
  //! Test that vetoes have the priority over triggers
  bool TestVetoPriority();
  //! Test that vetoes can be ignored
  bool TestIgnoreVetoes();
  //! Test trigger maps in the unit
  bool TestTriggerMaps();
  //! Test the flags which always assume a result
  bool TestAlwaysAssume();
  //! Test the lists of the raised triggers and vetoes
  bool TestNameLists();
  //! Test that no state leaks between events
  bool TestReset();
  //! Test the validation
  bool TestValidate();
  //! Test the search for detectors which never trigger
  bool TestIsNeverTriggering();
  //! Test named detectors
  bool TestNamedDetectors();
  //! Test the stream output
  bool TestStream();

  //! A position in the first wafer (cm)
  MVector m_WaferA;
  //! A position in the second wafer (cm)
  MVector m_WaferB;
  //! A position in the guard ring of the first wafer (cm)
  MVector m_GuardRingA;
  //! A position in the scintillator (cm)
  MVector m_Scinti;
  //! A position in the shield (cm)
  MVector m_Shield;
  //! A position in no detector (cm)
  MVector m_Nowhere;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
UTDTriggerUnit::UTDTriggerUnit() :
  UTGeomegaBase("UTDTriggerUnit"),
  m_WaferA(0.1, 0.1, -20.0), m_WaferB(0.1, 0.1, -10.0), m_GuardRingA(4.8, 0.1, -20.0),
  m_Scinti(0.0, 0.0, 10.0), m_Shield(0.0, 0.0, 20.0), m_Nowhere(50.0, 50.0, 50.0)
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDTriggerUnit::Run()
{
  bool Passed = true;

  Passed = TestNoTriggers() && Passed;
  Passed = TestAddHitThresholds() && Passed;
  Passed = TestNoisedThresholds() && Passed;
  Passed = TestBlockedChannels() && Passed;
  Passed = TestIgnoreThresholds() && Passed;
  Passed = TestHitsWithoutDetector() && Passed;
  Passed = TestAddHitVolumeSequence() && Passed;
  Passed = TestGuardRingHits() && Passed;
  Passed = TestChannelAgainstDetector() && Passed;
  Passed = TestVetoPriority() && Passed;
  Passed = TestIgnoreVetoes() && Passed;
  Passed = TestTriggerMaps() && Passed;
  Passed = TestAlwaysAssume() && Passed;
  Passed = TestNameLists() && Passed;
  Passed = TestReset() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestIsNeverTriggering() && Passed;
  Passed = TestNamedDetectors() && Passed;
  Passed = TestStream() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a geometry: Strips (two wafers with guard rings, threshold 30, guard ring 40), Scinti (threshold 50) and Shield (noise equals trigger threshold 50)
MString UTDTriggerUnit::GetGeometry() const
{
  MString Setup = GetGeometryHead();

  // The wafer is 10 x 10 cm in a holder, which is placed twice:
  Setup += "Volume Holder\n";
  Setup += "Holder.Material Vacuum\n";
  Setup += "Holder.Shape BRIK 6.0 6.0 1.0\n";
  Setup += "Volume Wafer\n";
  Setup += "Wafer.Material Germanium\n";
  Setup += "Wafer.Shape BRIK 5.0 5.0 0.5\n";
  Setup += "Wafer.Position 0.0 0.0 0.0\n";
  Setup += "Wafer.Mother Holder\n";
  Setup += "Holder.Copy HolderA\n";
  Setup += "HolderA.Position 0.0 0.0 -20.0\n";
  Setup += "HolderA.Mother World\n";
  Setup += "Holder.Copy HolderB\n";
  Setup += "HolderB.Position 0.0 0.0 -10.0\n";
  Setup += "HolderB.Mother World\n";

  Setup += "Volume ScintiVolume\n";
  Setup += "ScintiVolume.Material Germanium\n";
  Setup += "ScintiVolume.Shape BRIK 5.0 5.0 0.5\n";
  Setup += "ScintiVolume.Position 0.0 0.0 10.0\n";
  Setup += "ScintiVolume.Mother World\n";

  Setup += "Volume ShieldVolume\n";
  Setup += "ShieldVolume.Material Germanium\n";
  Setup += "ShieldVolume.Shape BRIK 5.0 5.0 0.5\n";
  Setup += "ShieldVolume.Position 0.0 0.0 20.0\n";
  Setup += "ShieldVolume.Mother World\n";

  // 9 strips of pitch 1 cm and a guard ring of 0.5 cm:
  Setup += "Strip2D Strips\n";
  Setup += "Strips.DetectorVolume Wafer\n";
  Setup += "Strips.SensitiveVolume Wafer\n";
  Setup += "Strips.StructuralPitch 0.0 0.0 0.0\n";
  Setup += "Strips.StructuralOffset 0.0 0.0 0.0\n";
  Setup += "Strips.Offset 0.5 0.5\n";
  Setup += "Strips.StripNumber 9 9\n";
  Setup += "Strips.NoiseThreshold 10\n";
  Setup += "Strips.TriggerThreshold 30\n";
  Setup += "Strips.EnergyResolution Ideal\n";
  Setup += "Strips.FailureRate 0.0\n";
  Setup += "Strips.GuardRingTriggerThreshold 40\n";
  Setup += "Strips.GuardRingEnergyResolution 100 1\n";
  Setup += "Strips.GuardRingEnergyResolution 1000 1\n";

  Setup += "Scintillator Scinti\n";
  Setup += "Scinti.DetectorVolume ScintiVolume\n";
  Setup += "Scinti.SensitiveVolume ScintiVolume\n";
  Setup += "Scinti.NoiseThreshold 10\n";
  Setup += "Scinti.TriggerThreshold 50\n";
  Setup += "Scinti.EnergyResolution Ideal\n";
  Setup += "Scinti.FailureRate 0.0\n";

  Setup += "Scintillator Shield\n";
  Setup += "Shield.DetectorVolume ShieldVolume\n";
  Setup += "Shield.SensitiveVolume ShieldVolume\n";
  Setup += "Shield.NoiseThresholdEqualsTriggerThreshold true\n";
  Setup += "Shield.TriggerThreshold 50\n";
  Setup += "Shield.EnergyResolution Ideal\n";
  Setup += "Shield.FailureRate 0.0\n";

  return Setup;
}


////////////////////////////////////////////////////////////////////////////////


//! Scan the geometry with the additional setup lines, and report if it fails
bool UTDTriggerUnit::Setup(MDGeometry& Geometry, const MString& Lines, const MString& Input)
{
  return EvaluateTrue("ScanSetupFile()", Input, "The geometry of the test can be read", ScanGeometry(Geometry, GetGeometry() + Lines));
}


////////////////////////////////////////////////////////////////////////////////


//! Test a geometry without any trigger
bool UTDTriggerUnit::TestNoTriggers()
{
  bool Passed = true;

  MDGeometry Geometry;
  if (Setup(Geometry, "", "no triggers") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  Passed = EvaluateTrue("GetTriggerUnit()", "no triggers", "The geometry has a trigger unit", Unit != nullptr) && Passed;
  Unit->Reset();
  Passed = EvaluateTrue("HasTriggered()", "no triggers, no hit", "Without triggers everything has triggered", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "no triggers, no hit", "Without triggers nothing is vetoed", Unit->HasVetoed()) && Passed;
  Passed = EvaluateFalse("AddHit()", "no triggers", "A hit cannot be added to any trigger", Unit->AddHit(m_Scinti, 1000.0)) && Passed;
  Passed = EvaluateSize("GetTriggerNameList()", "no triggers", "There are no names", Unit->GetTriggerNameList().size(), 0) && Passed;
  Passed = EvaluateSize("GetVetoNameList()", "no triggers", "There are no names", Unit->GetVetoNameList().size(), 0) && Passed;
  Passed = EvaluateTrue("Validate()", "no triggers", "A geometry without triggers is valid", Unit->Validate()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the return values and thresholds of adding a hit
bool UTDTriggerUnit::TestAddHitThresholds()
{
  bool Passed = true;

  MString Lines;
  Lines += "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";

  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "threshold") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  // The threshold of the scintillator is 50 keV without an uncertainty, just outside and inside:
  Unit->Reset();
  Passed = EvaluateFalse("AddHit()", "49.999 keV", "A hit just below the threshold is not added", Unit->AddHit(m_Scinti, 49.999)) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "49.999 keV", "A hit just below the threshold does not trigger", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("AddHit()", "50 keV", "A hit exactly at the threshold is not above the threshold", Unit->AddHit(m_Scinti, 50.0)) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "50 keV", "A hit exactly at the threshold does not trigger", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("AddHit()", "0 keV", "A hit without energy is not added", Unit->AddHit(m_Scinti, 0.0)) && Passed;
  Passed = EvaluateFalse("AddHit()", "-1 keV", "A hit with negative energy is not added", Unit->AddHit(m_Scinti, -1.0)) && Passed;
  Passed = EvaluateTrue("AddHit()", "50.001 keV", "A hit just above the threshold is added", Unit->AddHit(m_Scinti, 50.001)) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "50.001 keV", "A hit just above the threshold triggers", Unit->HasTriggered()) && Passed;

  // Thresholds apply per hit and do not add up:
  Unit->Reset();
  Unit->AddHit(m_Scinti, 30.0);
  Unit->AddHit(m_Scinti, 30.0);
  Passed = EvaluateFalse("HasTriggered()", "two hits of 30 keV", "The energies of two hits below the threshold do not add up", Unit->HasTriggered()) && Passed;

  // A hit above the threshold in a detector which is not part of the trigger:
  Unit->Reset();
  Passed = EvaluateFalse("AddHit()", "other detector", "A hit in a detector without trigger is not added", Unit->AddHit(m_WaferA, 1000.0)) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "other detector", "A hit in a detector without trigger does not trigger", Unit->HasTriggered()) && Passed;

  // A detector whose noise threshold is the trigger threshold, here all hits are above the threshold:
  MString NoiseLines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Shield 1\n";
  MDGeometry NoiseGeometry;
  if (Setup(NoiseGeometry, NoiseLines, "noise threshold equals trigger threshold") == true) {
    MDTriggerUnit* NoiseUnit = NoiseGeometry.GetTriggerUnit();
    NoiseUnit->Reset();
    // Attention: The flag means that the hit has passed this threshold during noising before
    Passed = EvaluateTrue("AddHit()", "1 keV, noise threshold equals trigger threshold", "A noised hit passed the threshold already", NoiseUnit->AddHit(m_Shield, 1.0)) && Passed;
    Passed = EvaluateTrue("HasTriggered()", "1 keV, noise threshold equals trigger threshold", "A noised hit passed the threshold already", NoiseUnit->HasTriggered()) && Passed;
  } else {
    Passed = false;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the thresholds with a uncertainty
bool UTDTriggerUnit::TestNoisedThresholds()
{
  bool Passed = true;

  // Two identical triggers on a detector with a threshold of 50 +- 10 keV:
  MString Setup = GetGeometry();
  Setup.ReplaceAllInPlace("Scinti.TriggerThreshold 50\n", "Scinti.TriggerThreshold 50 10\n");
  Setup += "Trigger T1\nT1.Veto false\nT1.TriggerByChannel true\nT1.Detector Scinti 1\n";
  Setup += "Trigger T2\nT2.Veto false\nT2.TriggerByChannel true\nT2.Detector Scinti 1\n";

  MDGeometry Geometry;
  bool Scanned = ScanGeometry(Geometry, Setup);
  Passed = EvaluateTrue("ScanSetupFile()", "noised threshold", "The geometry of the test can be read", Scanned) && Passed;
  if (Scanned == false) {
    return Passed;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();
  MDTrigger* T1 = Geometry.GetTrigger("T1");
  MDTrigger* T2 = Geometry.GetTrigger("T2");

  const UInt_t OldSeed = gRandom->GetSeed();
  gRandom->SetSeed(20261104);

  // At +1 sigma: Probability of a threshold below the energy = Phi(1) = 0.84134, 5 sigma of the binomial error for 10000 trials
  const unsigned int NTrials = 10000;
  const double Probability = 0.841345;
  const double Tolerance = 5.0*sqrt(Probability*(1.0 - Probability)/NTrials);
  unsigned int NRaised = 0;
  for (unsigned int t = 0; t < NTrials; ++t) {
    Unit->Reset();
    Unit->AddHit(m_Scinti, 60.0);
    if (T1->HasTriggered() == true) {
      ++NRaised;
    }
  }
  Passed = EvaluateNear("AddHit()", "60 keV, threshold 50 +- 10 keV", "The fraction of the raised triggers is Phi(1) of the Gaussian threshold", static_cast<double>(NRaised)/NTrials, Probability, Tolerance) && Passed;

  // One hit at one time is above or below the threshold of its channel, independent of the number of triggers:
  unsigned int NDifferent = 0;
  for (unsigned int t = 0; t < NTrials; ++t) {
    Unit->Reset();
    Unit->AddHit(m_Scinti, 50.0);
    if (T1->HasTriggered() != T2->HasTriggered()) {
      ++NDifferent;
    }
  }
  // Defect GEO-E-25: The noised threshold is drawn once per trigger, two triggers see a different threshold for the same hit
  Passed = ExpectedToFail(Evaluate("AddHit()", "50 keV, threshold 50 +- 10 keV", "Two identical triggers always have the same result for one hit", NDifferent, 0U)) && Passed;

  gRandom->SetSeed(OldSeed);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test blocked channels
bool UTDTriggerUnit::TestBlockedChannels()
{
  bool Passed = true;

  MString Lines = "Strips.BlockTrigger 4 4\nTrigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Strips 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "blocked channel") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  // Strip 4 of 0..8 with a pitch of 1 cm and a guard ring of 0.5 cm covers -0.5 to 0.5 cm in both dimensions:
  Unit->Reset();
  Passed = EvaluateFalse("AddHit()", "blocked channel", "A hit in a blocked channel does not trigger, independent of its energy", Unit->AddHit(MVector(0.1, 0.1, -20.0), 1000.0)) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "blocked channel", "A hit in a blocked channel does not trigger", Unit->HasTriggered()) && Passed;

  // The neighbors in both dimensions:
  Passed = EvaluateTrue("AddHit()", "neighbor in x", "The neighbor in x of the blocked channel is not blocked", Unit->AddHit(MVector(0.6, 0.1, -20.0), 1000.0)) && Passed;
  Unit->Reset();
  Passed = EvaluateTrue("AddHit()", "neighbor in y", "The neighbor in y of the blocked channel is not blocked", Unit->AddHit(MVector(0.1, 0.6, -20.0), 1000.0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test ignoring the thresholds
bool UTDTriggerUnit::TestIgnoreThresholds()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\nTrigger G\nG.Veto true\nG.TriggerByChannel true\nG.GuardRingDetector Strips 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "ignore thresholds") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  Unit->IgnoreThresholds(true);
  Unit->Reset();
  Passed = EvaluateTrue("IgnoreThresholds()", "true, 0.001 keV", "A hit far below the threshold is added", Unit->AddHit(m_Scinti, 0.001)) && Passed;
  Passed = EvaluateTrue("IgnoreThresholds()", "true, 0.001 keV", "A hit far below the threshold triggers", Unit->HasTriggered()) && Passed;
  Passed = EvaluateTrue("IgnoreThresholds()", "true, guard ring", "A guard ring hit far below the threshold is added", Unit->AddGuardRingHit(m_GuardRingA, 0.001)) && Passed;

  // Back to normal:
  Unit->IgnoreThresholds(false);
  Unit->Reset();
  Passed = EvaluateFalse("IgnoreThresholds()", "false, 49.999 keV", "The threshold is back", Unit->AddHit(m_Scinti, 49.999)) && Passed;

  // Ignoring the thresholds does not extend the trigger to other detectors:
  Unit->IgnoreThresholds(true);
  Unit->Reset();
  Passed = EvaluateFalse("IgnoreThresholds()", "true, other detector", "A hit in a detector without trigger is not added", Unit->AddHit(m_WaferA, 1000.0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits which have no detector
bool UTDTriggerUnit::TestHitsWithoutDetector()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "no detector") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  // In the world volume and outside of the world volume:
  const vector<MVector> Positions = { m_Nowhere, MVector(1000.0, 0.0, 0.0) };
  for (const MVector& Position: Positions) {
    Unit->Reset();

    // The geometry writes the warnings about the missing detector directly to the output:
    ostringstream Captured;
    streambuf* Original = cout.rdbuf(Captured.rdbuf());
    DisableDefaultStreams();
    bool Added = Unit->AddHit(Position, 1000.0);
    bool AddedGuardRing = Unit->AddGuardRingHit(Position, 1000.0);
    bool Triggered = Unit->HasTriggered();
    EnableDefaultStreams();
    cout.rdbuf(Original);

    MString Input = MString("Position x = ") + Position.X();
    Passed = EvaluateFalse("AddHit()", Input, "A hit without a detector is not added", Added) && Passed;
    Passed = EvaluateFalse("AddGuardRingHit()", Input, "A guard ring hit without a detector is not added", AddedGuardRing) && Passed;
    Passed = EvaluateFalse("HasTriggered()", Input, "A hit without a detector does not trigger", Triggered) && Passed;
  }

  // An empty volume sequence:
  MDVolumeSequence Empty;
  DisableDefaultStreams();
  bool AddedEmpty = Unit->AddHit(1000.0, Empty);
  bool AddedEmptyGuardRing = Unit->AddGuardRingHit(1000.0, Empty);
  EnableDefaultStreams();
  Passed = EvaluateFalse("AddHit()", "empty volume sequence", "An empty sequence is not added", AddedEmpty) && Passed;
  Passed = EvaluateFalse("AddGuardRingHit()", "empty volume sequence", "An empty sequence is not added", AddedEmptyGuardRing) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding hits with volume sequences
bool UTDTriggerUnit::TestAddHitVolumeSequence()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\nTrigger G\nG.Veto true\nG.TriggerByChannel true\nG.GuardRingDetector Strips 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "volume sequence") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  MDVolumeSequence Hit = Geometry.GetVolumeSequence(m_Scinti, false, true);
  Unit->Reset();
  Passed = EvaluateFalse("AddHit(Energy, VS)", "49.999 keV", "A hit just below the threshold is not added", Unit->AddHit(49.999, Hit)) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "49.999 keV", "A hit just below the threshold does not trigger", Unit->HasTriggered()) && Passed;
  Passed = EvaluateTrue("AddHit(Energy, VS)", "50.001 keV", "A hit just above the threshold is added", Unit->AddHit(50.001, Hit)) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "50.001 keV", "A hit just above the threshold triggers", Unit->HasTriggered()) && Passed;

  MDVolumeSequence GuardRing = Geometry.GetVolumeSequence(m_GuardRingA, false, true);
  Unit->Reset();
  Passed = EvaluateTrue("AddGuardRingHit(Energy, VS)", "100 keV", "The guard ring hit is added", Unit->AddGuardRingHit(100.0, GuardRing)) && Passed;
  Passed = EvaluateTrue("HasVetoed()", "100 keV", "The guard ring veto is raised", Unit->HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test guard ring hits
bool UTDTriggerUnit::TestGuardRingHits()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Strips 1\n";
  Lines += "Trigger G\nG.Veto true\nG.TriggerByChannel true\nG.GuardRingDetector Strips 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "guard ring") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  Unit->Reset();
  Unit->AddHit(m_WaferA, 100.0);
  Passed = EvaluateTrue("HasTriggered()", "strip hit", "The strip hit triggers", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "strip hit", "The strip hit does not raise the guard ring veto", Unit->HasVetoed()) && Passed;

  Passed = EvaluateTrue("AddGuardRingHit()", "guard ring hit", "The guard ring hit is added", Unit->AddGuardRingHit(m_GuardRingA, 100.0)) && Passed;
  Passed = EvaluateTrue("HasVetoed()", "strip and guard ring hit", "The guard ring hit raises the veto", Unit->HasVetoed()) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "strip and guard ring hit", "The veto has the priority", Unit->HasTriggered()) && Passed;

  // Attention: The keyword sets the flag NoiseThresholdEqualsTriggerThreshold - the threshold is applied during the noising
  Unit->Reset();
  Passed = EvaluateTrue("AddGuardRingHit()", "1 keV in the guard ring with threshold 40 keV", "A noised guard ring hit passed the threshold already", Unit->AddGuardRingHit(m_GuardRingA, 1.0)) && Passed;

  // A detector without a guard ring:
  Unit->Reset();
  DisableDefaultStreams();
  bool Added = Unit->AddGuardRingHit(m_Scinti, 1000.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("AddGuardRingHit()", "detector without guard ring", "The hit is not added", Added) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "detector without guard ring", "The hit does not raise the veto", Unit->HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test channel against detector count with real positions
bool UTDTriggerUnit::TestChannelAgainstDetector()
{
  bool Passed = true;

  MString ByChannelLines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Strips 2\n";
  MDGeometry ByChannel;
  if (Setup(ByChannel, ByChannelLines, "by channel") == false) {
    return false;
  }
  MString ByDetectorLines = "Trigger T\nT.Veto false\nT.TriggerByDetector true\nT.Detector Strips 2\n";
  MDGeometry ByDetector;
  if (Setup(ByDetector, ByDetectorLines, "by detector") == false) {
    return false;
  }

  MDTriggerUnit* UnitChannel = ByChannel.GetTriggerUnit();
  MDTriggerUnit* UnitDetector = ByDetector.GetTriggerUnit();

  // Two hits in the same wafer:
  UnitChannel->Reset();
  UnitChannel->AddHit(m_WaferA, 100.0);
  Passed = EvaluateFalse("HasTriggered()", "by channel, 1 hit", "One channel is not enough", UnitChannel->HasTriggered()) && Passed;
  UnitChannel->AddHit(MVector(1.5, 0.1, -20.0), 100.0);
  Passed = EvaluateTrue("HasTriggered()", "by channel, 2 hits in one wafer", "Two channels are enough", UnitChannel->HasTriggered()) && Passed;

  UnitDetector->Reset();
  UnitDetector->AddHit(m_WaferA, 100.0);
  UnitDetector->AddHit(MVector(1.5, 0.1, -20.0), 100.0);
  Passed = EvaluateFalse("HasTriggered()", "by detector, 2 hits in one wafer", "One wafer is not enough", UnitDetector->HasTriggered()) && Passed;

  // A hit in the second wafer - the two holders are one detector with two volumes:
  UnitDetector->AddHit(m_WaferB, 100.0);
  Passed = EvaluateTrue("HasTriggered()", "by detector, hits in two wafers", "Two wafers are enough", UnitDetector->HasTriggered()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that vetoes have the priority over triggers
bool UTDTriggerUnit::TestVetoPriority()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "veto") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  // All combinations of the hits, in both orders:
  const vector<bool> Options = { false, true };
  for (bool TriggerHit: Options) {
    for (bool VetoHit: Options) {
      for (bool VetoFirst: Options) {
        Unit->Reset();
        if (VetoFirst == true && VetoHit == true) {
          Unit->AddHit(m_Shield, 100.0);
        }
        if (TriggerHit == true) {
          Unit->AddHit(m_Scinti, 100.0);
        }
        if (VetoFirst == false && VetoHit == true) {
          Unit->AddHit(m_Shield, 100.0);
        }

        bool ExpectedTriggered = false;
        if (TriggerHit == true && VetoHit == false) {
          ExpectedTriggered = true;
        }
        MString Input = MString("Trigger hit ") + (TriggerHit == true ? "yes" : "no") + ", veto hit " + (VetoHit == true ? "yes" : "no") + ", veto first " + (VetoFirst == true ? "yes" : "no");
        Passed = Evaluate("HasTriggered()", Input, "A veto stops the trigger", Unit->HasTriggered(), ExpectedTriggered) && Passed;
        Passed = Evaluate("HasVetoed()", Input, "A veto hit raises the veto", Unit->HasVetoed(), VetoHit) && Passed;
      }
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that vetoes can be ignored
bool UTDTriggerUnit::TestIgnoreVetoes()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "ignore vetoes") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  // Pretriggering in the simulation:
  Unit->IgnoreVetoes(true);
  Unit->Reset();
  Unit->AddHit(m_Scinti, 100.0);
  Passed = EvaluateTrue("AddHit()", "ignored veto", "The hit of the veto is still accepted", Unit->AddHit(m_Shield, 100.0)) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "ignored veto", "The ignored veto does not stop the trigger", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "ignored veto", "The unit has not vetoed", Unit->HasVetoed()) && Passed;
  Passed = EvaluateSize("GetVetoNameList()", "ignored veto", "No veto is listed", Unit->GetVetoNameList().size(), 0) && Passed;
  // The flag is transferred to the individual triggers:
  Passed = EvaluateFalse("IgnoreVetoes()", "transfer to trigger", "The veto trigger itself has not vetoed", Geometry.GetTrigger("V")->HasVetoed()) && Passed;

  // A veto hit alone does not trigger:
  Unit->Reset();
  Unit->AddHit(m_Shield, 100.0);
  Passed = EvaluateFalse("HasTriggered()", "ignored veto, only veto hit", "An ignored veto does not trigger", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "ignored veto, only veto hit", "The unit has not vetoed", Unit->HasVetoed()) && Passed;

  // Back again, the hits are the same:
  Unit->IgnoreVetoes(false);
  Passed = EvaluateTrue("HasVetoed()", "not ignored anymore", "The veto is back", Unit->HasVetoed()) && Passed;
  Passed = EvaluateTrue("IgnoreVetoes()", "transfer to trigger", "The veto trigger itself has vetoed", Geometry.GetTrigger("V")->HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test trigger maps in the unit
bool UTDTriggerUnit::TestTriggerMaps()
{
  bool Passed = true;

  // TV: Scinti, TN: 2 in Scinti, VT: Shield
  const MString MapFile = GetTemporaryFileName("Unit.triggers");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(MapFile, "DT Scinti Shield Strips_GuardRing\nTV 1+ 0+ 0+\nTN 2+ 0+ 0+\nVT 0+ 1+ 0+\nVT 0+ 0+ 1+\n")) && Passed;

  MString Lines = "TriggerMap M\nM.Map " + MapFile + "\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "trigger map") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  Unit->Reset();
  Passed = EvaluateFalse("HasTriggered()", "map, no hits", "Without hits nothing is raised", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "map, no hits", "Without hits nothing is raised", Unit->HasVetoed()) && Passed;

  // The thresholds are applied:
  Passed = EvaluateFalse("AddHit()", "map, 49.999 keV", "A hit below the threshold is not added", Unit->AddHit(m_Scinti, 49.999)) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "map, 49.999 keV", "A hit below the threshold does not trigger", Unit->HasTriggered()) && Passed;

  // Vetoable trigger:
  Passed = EvaluateTrue("AddHit()", "map, 50.001 keV", "A hit above the threshold is added", Unit->AddHit(m_Scinti, 50.001)) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "map, one hit", "The vetoable trigger is raised", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "map, one hit", "There is no veto", Unit->HasVetoed()) && Passed;

  // Veto stops it:
  Unit->AddHit(m_Shield, 100.0);
  Passed = EvaluateFalse("HasTriggered()", "map, one hit and veto hit", "The veto stops the vetoable trigger", Unit->HasTriggered()) && Passed;
  Passed = EvaluateTrue("HasVetoed()", "map, one hit and veto hit", "The veto is raised", Unit->HasVetoed()) && Passed;

  // Veto as during pretriggering:
  Unit->IgnoreVetoes(true);
  Passed = EvaluateTrue("HasTriggered()", "map, ignored veto", "The ignored veto does not stop the trigger", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "map, ignored veto", "The ignored veto is not raised", Unit->HasVetoed()) && Passed;
  Unit->IgnoreVetoes(false);

  // A non-vetoable trigger is not stopped:
  Unit->AddHit(m_Scinti, 100.0);
  Passed = EvaluateTrue("HasTriggered()", "map, two hits and veto hit", "The non-vetoable trigger is not stopped", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "map, two hits and veto hit", "The non-vetoable trigger cancels the veto", Unit->HasVetoed()) && Passed;

  // The guard ring column:
  Unit->Reset();
  Unit->AddHit(m_Scinti, 100.0);
  Passed = EvaluateTrue("AddGuardRingHit()", "map, guard ring listed", "The hit is added", Unit->AddGuardRingHit(m_GuardRingA, 100.0)) && Passed;
  Passed = EvaluateTrue("HasVetoed()", "map, guard ring hit", "The guard ring veto is raised", Unit->HasVetoed()) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "map, guard ring hit", "The guard ring veto stops the trigger", Unit->HasTriggered()) && Passed;

  // Reset:
  Unit->Reset();
  Passed = EvaluateFalse("HasVetoed()", "map, after reset", "The veto is gone", Unit->HasVetoed()) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "map, after reset", "The trigger is gone", Unit->HasTriggered()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the flags which always assume a result
bool UTDTriggerUnit::TestAlwaysAssume()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "always assume") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();
  Unit->Reset();

  Unit->AlwaysAssumeTrigger(true);
  Passed = EvaluateTrue("AlwaysAssumeTrigger()", "true, no hits", "A trigger is assumed", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("AlwaysAssumeTrigger()", "true, no hits", "No veto is assumed", Unit->HasVetoed()) && Passed;
  Unit->AddHit(m_Shield, 100.0);
  Passed = EvaluateTrue("AlwaysAssumeTrigger()", "true, veto hit", "A trigger is assumed, even with a veto hit", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("AlwaysAssumeTrigger()", "true, veto hit", "No veto is assumed, even with a veto hit", Unit->HasVetoed()) && Passed;
  Unit->AlwaysAssumeTrigger(false);

  Unit->Reset();
  Unit->AlwaysAssumeVeto(true);
  Passed = EvaluateFalse("AlwaysAssumeVeto()", "true, no hits", "No trigger is assumed", Unit->HasTriggered()) && Passed;
  Passed = EvaluateTrue("AlwaysAssumeVeto()", "true, no hits", "A veto is assumed", Unit->HasVetoed()) && Passed;
  Unit->AddHit(m_Scinti, 100.0);
  Passed = EvaluateFalse("AlwaysAssumeVeto()", "true, trigger hit", "No trigger is assumed, even with a trigger hit", Unit->HasTriggered()) && Passed;
  Passed = EvaluateTrue("AlwaysAssumeVeto()", "true, trigger hit", "A veto is assumed, even with a trigger hit", Unit->HasVetoed()) && Passed;

  // The trigger flag has the priority if both are set:
  Unit->AlwaysAssumeTrigger(true);
  Passed = EvaluateTrue("AlwaysAssumeTrigger()", "both flags", "The trigger flag is evaluated first", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("AlwaysAssumeTrigger()", "both flags", "The trigger flag is evaluated first", Unit->HasVetoed()) && Passed;
  Unit->AlwaysAssumeTrigger(false);
  Unit->AlwaysAssumeVeto(false);

  // Normal behavior after clearing both flags - the trigger hit is still there:
  Passed = EvaluateTrue("AlwaysAssumeVeto()", "both false, trigger hit", "The real trigger is back", Unit->HasTriggered()) && Passed;
  Passed = EvaluateFalse("AlwaysAssumeVeto()", "both false, trigger hit", "The real veto is back", Unit->HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the lists of the raised triggers and vetoes
bool UTDTriggerUnit::TestNameLists()
{
  bool Passed = true;

  MString Lines = "Trigger T1\nT1.Veto false\nT1.TriggerByChannel true\nT1.Detector Scinti 1\n";
  Lines += "Trigger T2\nT2.Veto false\nT2.TriggerByChannel true\nT2.Detector Strips 1\n";
  Lines += "Trigger V1\nV1.Veto true\nV1.TriggerByChannel true\nV1.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "name lists") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  Unit->Reset();
  Passed = EvaluateSize("GetTriggerNameList()", "no hits", "No trigger is listed", Unit->GetTriggerNameList().size(), 0) && Passed;
  Passed = EvaluateSize("GetVetoNameList()", "no hits", "No veto is listed", Unit->GetVetoNameList().size(), 0) && Passed;

  Unit->AddHit(m_Scinti, 100.0);
  Passed = EvaluateTrue("GetTriggerNameList()", "one trigger", "The raised trigger is listed", Unit->GetTriggerNameList() == vector<MString>({ "T1" })) && Passed;

  Unit->AddHit(m_WaferA, 100.0);
  Passed = EvaluateTrue("GetTriggerNameList()", "two triggers", "Both raised triggers are listed", Unit->GetTriggerNameList() == vector<MString>({ "T1", "T2" })) && Passed;
  Passed = EvaluateSize("GetVetoNameList()", "two triggers", "No veto is listed", Unit->GetVetoNameList().size(), 0) && Passed;

  // The basic triggers stay in the list, also when vetoed, thus one can see what has been vetoed:
  Unit->AddHit(m_Shield, 100.0);
  Passed = EvaluateTrue("GetVetoNameList()", "veto", "The raised veto is listed", Unit->GetVetoNameList() == vector<MString>({ "V1" })) && Passed;
  Passed = EvaluateTrue("GetTriggerNameList()", "veto", "The raised triggers are listed", Unit->GetTriggerNameList() == vector<MString>({ "T1", "T2" })) && Passed;

  Unit->Reset();
  Passed = EvaluateSize("GetTriggerNameList()", "after reset", "No trigger is listed", Unit->GetTriggerNameList().size(), 0) && Passed;
  Passed = EvaluateSize("GetVetoNameList()", "after reset", "No veto is listed", Unit->GetVetoNameList().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that no state leaks between events
bool UTDTriggerUnit::TestReset()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 2\n";
  Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "reset") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  // Event 1 triggers:
  Unit->Reset();
  Unit->AddHit(m_Scinti, 100.0);
  Unit->AddHit(m_Scinti, 100.0);
  Passed = EvaluateTrue("HasTriggered()", "event 1", "Event 1 triggers", Unit->HasTriggered()) && Passed;

  // Event 2 with one hit too few:
  Unit->Reset();
  Passed = EvaluateFalse("Reset()", "after event 1", "The trigger is gone", Unit->HasTriggered()) && Passed;
  Unit->AddHit(m_Scinti, 100.0);
  Passed = EvaluateFalse("HasTriggered()", "event 2", "One hit is not enough after the reset", Unit->HasTriggered()) && Passed;

  // Event 3 with a veto:
  Unit->Reset();
  Unit->AddHit(m_Shield, 100.0);
  Passed = EvaluateTrue("HasVetoed()", "event 3", "Event 3 is vetoed", Unit->HasVetoed()) && Passed;

  // Event 4 does not see the veto of event 3:
  Unit->Reset();
  Passed = EvaluateFalse("Reset()", "after event 3", "The veto is gone", Unit->HasVetoed()) && Passed;
  Unit->AddHit(m_Scinti, 100.0);
  Unit->AddHit(m_Scinti, 100.0);
  Passed = EvaluateTrue("HasTriggered()", "event 4", "Event 4 triggers without the veto of event 3", Unit->HasTriggered()) && Passed;

  // The flags survive the reset:
  Unit->IgnoreVetoes(true);
  Unit->IgnoreThresholds(true);
  Unit->Reset();
  Unit->AddHit(m_Shield, 0.001);
  Unit->AddHit(m_Scinti, 0.001);
  Unit->AddHit(m_Scinti, 0.001);
  Passed = EvaluateTrue("Reset()", "flags", "The flags to ignore the vetoes and thresholds survive the reset", Unit->HasTriggered()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation
bool UTDTriggerUnit::TestValidate()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "validate") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();
  Passed = EvaluateTrue("Validate()", "one basic trigger", "One basic trigger is valid", Unit->Validate()) && Passed;

  // Mixing the trigger classes - the geometry owns the added triggers:
  MDGeometry Mixed;
  if (Setup(Mixed, Lines, "validate") == false) {
    return false;
  }
  MDTriggerMap* Added = new MDTriggerMap("Added");
  Added->SetDetectors({ "Scinti" });
  Added->SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1 }, { true });
  Mixed.AddTrigger(Added);
  DisableDefaultStreams();
  bool MixedResult = Mixed.GetTriggerUnit()->Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "basic trigger and map", "Trigger classes cannot be mixed", MixedResult) && Passed;

  // A map with an unknown detector:
  MDGeometry UnknownDetector;
  if (Setup(UnknownDetector, "", "validate") == false) {
    return false;
  }
  MDTriggerMap* Unknown = new MDTriggerMap("Unknown");
  Unknown->SetDetectors({ "Scinti", "Typo" });
  UnknownDetector.AddTrigger(Unknown);
  DisableDefaultStreams();
  bool UnknownResult = UnknownDetector.GetTriggerUnit()->Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "map with unknown detector", "A map with an unknown detector is invalid", UnknownResult) && Passed;

  // A valid map with a guard ring as detector:
  MDGeometry WithGuardRing;
  if (Setup(WithGuardRing, "", "validate") == false) {
    return false;
  }
  MDTriggerMap* GuardRing = new MDTriggerMap("GuardRing");
  GuardRing->SetDetectors({ "Scinti", "Strips_GuardRing" });
  WithGuardRing.AddTrigger(GuardRing);
  Passed = EvaluateTrue("Validate()", "map with guard ring", "A map with a guard ring is valid", WithGuardRing.GetTriggerUnit()->Validate()) && Passed;

  // A detector which only vetoes needs the flag for the thresholds - Scinti does not have it, Shield has it:
  MDGeometry VetoWithoutFlag;
  if (Setup(VetoWithoutFlag, "", "validate") == false) {
    return false;
  }
  MDTriggerBasic* VetoScinti = new MDTriggerBasic("V");
  VetoScinti->SetVeto(true);
  VetoScinti->SetDetector(VetoWithoutFlag.GetDetector("Scinti"), 1);
  VetoWithoutFlag.AddTrigger(VetoScinti);
  DisableDefaultStreams();
  bool VetoWithoutFlagResult = VetoWithoutFlag.GetTriggerUnit()->Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "only veto, flag missing", "A detector with only vetoes needs the flag", VetoWithoutFlagResult) && Passed;

  // With a trigger on the same detector it is valid:
  MDTriggerBasic* TriggerScinti = new MDTriggerBasic("T");
  TriggerScinti->SetDetector(VetoWithoutFlag.GetDetector("Scinti"), 1);
  VetoWithoutFlag.AddTrigger(TriggerScinti);
  Passed = EvaluateTrue("Validate()", "veto and trigger, flag missing", "A detector which also triggers does not need the flag", VetoWithoutFlag.GetTriggerUnit()->Validate()) && Passed;

  MDGeometry VetoWithFlag;
  if (Setup(VetoWithFlag, "", "validate") == false) {
    return false;
  }
  MDTriggerBasic* VetoShield = new MDTriggerBasic("V");
  VetoShield->SetVeto(true);
  VetoShield->SetDetector(VetoWithFlag.GetDetector("Shield"), 1);
  VetoWithFlag.AddTrigger(VetoShield);
  Passed = EvaluateTrue("Validate()", "only veto, flag set", "A detector with only vetoes and the flag is valid", VetoWithFlag.GetTriggerUnit()->Validate()) && Passed;

  // The same during reading of the setup file:
  MDGeometry FromFile;
  Passed = EvaluateFalse("ScanSetupFile()", "only veto, flag missing", "The validation is applied when reading a setup file", ScanGeometry(FromFile, GetGeometry() + "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Scinti 1\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for detectors which never trigger
bool UTDTriggerUnit::TestIsNeverTriggering()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "never triggering") == false) {
    return false;
  }
  MDTriggerUnit* Unit = Geometry.GetTriggerUnit();

  Passed = EvaluateFalse("IsNeverTriggering()", "detector with trigger", "A detector with a trigger triggers", Unit->IsNeverTriggering(Geometry.GetDetector("Scinti"))) && Passed;
  Passed = EvaluateTrue("IsNeverTriggering()", "detector with veto", "A detector with only a veto never triggers", Unit->IsNeverTriggering(Geometry.GetDetector("Shield"))) && Passed;
  Passed = EvaluateTrue("IsNeverTriggering()", "detector without trigger", "A detector without a trigger never triggers", Unit->IsNeverTriggering(Geometry.GetDetector("Strips"))) && Passed;

  // With a map:
  const MString MapFile = GetTemporaryFileName("Never.triggers");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(MapFile, "DT Scinti Shield Strips\nTV 1+ 0+ 0+\nVT 0+ 1+ 0+\n")) && Passed;
  MDGeometry MapGeometry;
  if (Setup(MapGeometry, "TriggerMap M\nM.Map " + MapFile + "\n", "never triggering, map") == false) {
    return false;
  }
  MDTriggerUnit* MapUnit = MapGeometry.GetTriggerUnit();
  Passed = EvaluateFalse("IsNeverTriggering()", "map, trigger column", "A detector with a required hit in a trigger triggers", MapUnit->IsNeverTriggering(MapGeometry.GetDetector("Scinti"))) && Passed;
  Passed = EvaluateTrue("IsNeverTriggering()", "map, veto column", "A detector with only required hits in a veto never triggers", MapUnit->IsNeverTriggering(MapGeometry.GetDetector("Shield"))) && Passed;
  Passed = EvaluateTrue("IsNeverTriggering()", "map, only 0+", "A detector without required hits never triggers", MapUnit->IsNeverTriggering(MapGeometry.GetDetector("Strips"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test named detectors
bool UTDTriggerUnit::TestNamedDetectors()
{
  bool Passed = true;

  MString NamedLines = "Strips.Named StripsA\nStripsA.Assign 0.0 0.0 -20.0\nStrips.Named StripsB\nStripsB.Assign 0.0 0.0 -10.0\n";

  // A hit in the first wafer belongs to the named detector:
  MDGeometry Basic;
  if (Setup(Basic, NamedLines + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Strips 1\n", "named, basic") == false) {
    return false;
  }
  MDVolumeSequence Hit = Basic.GetVolumeSequence(m_WaferA, false, true);
  Passed = EvaluateTrue("GetVolumeSequence()", "named detector", "The hit in the wafer belongs to the named detector", Hit.GetDetector() != nullptr && Hit.GetDetector()->GetName() == "StripsA") && Passed;

  // The threshold is inherited from the base detector:
  MDTriggerUnit* BasicUnit = Basic.GetTriggerUnit();
  BasicUnit->Reset();
  Passed = EvaluateFalse("AddHit()", "named, basic, 29.999 keV", "The inherited threshold is applied", BasicUnit->AddHit(m_WaferA, 29.999)) && Passed;
  Passed = EvaluateTrue("AddHit()", "named, basic, 30.001 keV", "A hit in the named detector counts for the condition on the base detector", BasicUnit->AddHit(m_WaferA, 30.001)) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "named, basic", "A hit in the named detector raises the trigger", BasicUnit->HasTriggered()) && Passed;

  // The simulation asks for the base detector:
  Passed = EvaluateFalse("IsNeverTriggering()", "named, basic, condition on base", "The base detector triggers", BasicUnit->IsNeverTriggering(Basic.GetDetector("Strips"))) && Passed;

  // A map with the names of the named detectors:
  const MString MapNamedFile = GetTemporaryFileName("Named.triggers");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(MapNamedFile, "DT StripsA StripsB\nTV 1+ 0+\n")) && Passed;
  MDGeometry MapNamed;
  if (Setup(MapNamed, NamedLines + "TriggerMap M\nM.Map " + MapNamedFile + "\n", "named, map with named detectors") == false) {
    return false;
  }
  MDTriggerUnit* MapNamedUnit = MapNamed.GetTriggerUnit();
  MapNamedUnit->Reset();
  Passed = EvaluateTrue("AddHit()", "named, map lists StripsA, hit in StripsA", "The hit is added", MapNamedUnit->AddHit(m_WaferA, 100.0)) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "named, map lists StripsA, hit in StripsA", "The hit raises the trigger", MapNamedUnit->HasTriggered()) && Passed;
  MapNamedUnit->Reset();
  Passed = EvaluateTrue("AddHit()", "named, map lists StripsB, hit in StripsB", "The hit is added", MapNamedUnit->AddHit(m_WaferB, 100.0)) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "named, map lists StripsB, hit in StripsB", "The hit in the second column does not raise a rule on the first column", MapNamedUnit->HasTriggered()) && Passed;

  // Line 583 of MCDetectorConstruction: The simulation asks with the base detector, although the map has a hit requirement on the named detector
  // Defect GEO-E-26: For a map on named detectors the base detector is reported as never triggering, and thus the simulation marks triggering detectors as never triggering
  Passed = ExpectedToFail(EvaluateFalse("IsNeverTriggering()", "named, map lists named detectors, base detector", "The base detector of a triggering named detector can trigger", MapNamedUnit->IsNeverTriggering(MapNamed.GetDetector("Strips")))) && Passed;
  Passed = EvaluateFalse("IsNeverTriggering()", "named, map lists named detectors, named detector", "The named detector triggers", MapNamedUnit->IsNeverTriggering(MapNamed.GetDetector("StripsA"))) && Passed;

  // A map with the name of the base detector, the validation accepts it:
  const MString MapBaseFile = GetTemporaryFileName("Base.triggers");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(MapBaseFile, "DT Strips\nTV 1+\n")) && Passed;
  MDGeometry MapBase;
  if (Setup(MapBase, NamedLines + "TriggerMap M\nM.Map " + MapBaseFile + "\n", "named, map with base detector") == false) {
    return false;
  }
  MDTriggerUnit* MapBaseUnit = MapBase.GetTriggerUnit();
  MapBaseUnit->Reset();
  // Defect GEO-E-15: The second search compares the old name again, a hit in a named detector does not count for the detector it is named after
  Passed = ExpectedToFail(EvaluateTrue("AddHit()", "named, map lists base detector", "A hit in the named detector counts for the base detector", MapBaseUnit->AddHit(m_WaferA, 100.0))) && Passed;
  // Defect GEO-E-15: The second search compares the old name again, a hit in a named detector does not count for the detector it is named after
  Passed = ExpectedToFail(EvaluateTrue("HasTriggered()", "named, map lists base detector", "A hit in the named detector raises the trigger", MapBaseUnit->HasTriggered())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the stream output
bool UTDTriggerUnit::TestStream()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "stream") == false) {
    return false;
  }

  ostringstream Out;
  Out<<*Geometry.GetTriggerUnit();
  MString Expected = "Trigger unit:\nTrigger (Basic): T\n  Veto: no\n  Detector names: Scinti 1\n";
  Passed = EvaluateTrue("operator<<", "one basic trigger", "The output has the headline and the text of each trigger", MString(Out.str()) == Expected) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);

  // Hide the info messages of the geometry manager:
  gErrorIgnoreLevel = kWarning;

  UTDTriggerUnit Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDTriggerUnit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
