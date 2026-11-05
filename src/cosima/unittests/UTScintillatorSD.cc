/*
 * UTScintillatorSD.cc
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


// Cosima:
#include "MCScintillatorSD.hh"
#include "UTSDHitShared.h"
#include "UTSDStepFixture.h"

// MEGAlib:
#include "MDDetector.h"
#include "MFunction3D.h"
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace std;


//! Class exposing the protected members of the hit to the unit test
class MCScintillatorHitAccess : public MCScintillatorHit
{
public:
  using MCVHit::m_Time;
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Sensitive detector exposing the protected members of MCSD to the unit test
class MCScintillatorSDAccess : public MCScintillatorSD
{
public:
  MCScintillatorSDAccess(G4String Name) : MCScintillatorSD(Name) {}
  using MCSD::m_DiscretizeHits;
  using MCSD::m_HasTimeResolution;
  using MCSD::m_IsNeverTriggering;
  using MCSD::m_UseEnergyLoss;
  using MCSD::m_Type;
};


//! Unit test class for MCScintillatorSD
class UTScintillatorSD : public MUnitTest
{
public:
  //! Default constructor
  UTScintillatorSD() : MUnitTest("UTScintillatorSD") {}
  //! Default destructor
  virtual ~UTScintillatorSD() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructor
  bool TestConstruction();
  //! Test the event cycle - Initialize and EndOfEvent
  bool TestEventCycle();
  //! Test ProcessHits, which does nothing
  bool TestProcessHits();
  //! Test a simple hit
  bool TestSimpleHit();
  //! Test steps without energy deposit
  bool TestZeroEnergy();
  //! Test the position of the hit for the different step statuses
  bool TestStepStatusPosition();
  //! Test the discretization and the unique positions
  bool TestDiscretization();
  //! Test the volume history and the common volume
  bool TestVolumeHistory();
  //! Test the merging of hits
  bool TestMerging();
  //! Test the energy loss map
  bool TestEnergyLossMap();
  //! Test the time resolution
  bool TestTimeResolution();
  //! Test the digitized flag of the track information
  bool TestTrackInformation();
  //! Test the coordinate transformations with rotated volumes
  bool TestRotatedVolumes();
  //! Test detectors which are never triggering
  bool TestNeverTriggering();

  //! Return the hits of the event as list
  vector<MCScintillatorHitAccess*> GetHits(G4VHitsCollection* Collection);
};


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestEventCycle() && Passed;
  Passed = TestProcessHits() && Passed;
  Passed = TestSimpleHit() && Passed;
  Passed = TestZeroEnergy() && Passed;
  Passed = TestStepStatusPosition() && Passed;
  Passed = TestDiscretization() && Passed;
  Passed = TestVolumeHistory() && Passed;
  Passed = TestMerging() && Passed;
  Passed = TestEnergyLossMap() && Passed;
  Passed = TestTimeResolution() && Passed;
  Passed = TestTrackInformation() && Passed;
  Passed = TestRotatedVolumes() && Passed;
  Passed = TestNeverTriggering() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


vector<MCScintillatorHitAccess*> UTScintillatorSD::GetHits(G4VHitsCollection* Collection)
{
  vector<MCScintillatorHitAccess*> Hits;
  MCScintillatorHitsCollection* HC = static_cast<MCScintillatorHitsCollection*>(Collection);
  if (HC == nullptr) return Hits;
  for (size_t h = 0; h < HC->entries(); ++h) {
    Hits.push_back(static_cast<MCScintillatorHitAccess*>((*HC)[h]));
  }
  return Hits;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestConstruction()
{
  bool Passed = true;

  MCScintillatorSDAccess SD("ACS");
  Passed = Evaluate("MCScintillatorSD()", "name", "The name of the detector is the one given", MString(SD.GetName().c_str()), MString("ACS")) && Passed;
  Passed = Evaluate("MCScintillatorSD()", "type", "The type of the detector is scintillator", SD.GetType(), MCSD::c_Scintillator) && Passed;
  Passed = Evaluate("MCScintillatorSD()", "collections", "The detector has one hit collection", SD.GetNumberOfCollections(), 1) && Passed;
  Passed = Evaluate("MCScintillatorSD()", "collection name", "The name of the collection is the name of the detector", MString(SD.GetCollectionName(0).c_str()), MString("ACS")) && Passed;
  Passed = EvaluateTrue("MCScintillatorSD()", "default", "Hits are discretized by default", SD.m_DiscretizeHits) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestEventCycle()
{
  bool Passed = true;

  UTSDStepFixture F;
  MCScintillatorSDAccess SD("CycleSD");

  // The collection is created by Initialize and handed to Geant4 in EndOfEvent:
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4VHitsCollection* Collection = F.EndEvent(SD, HCE);
  Passed = EvaluateTrue("EndOfEvent()", "empty event", "The hit collection is handed to the event", Collection != nullptr) && Passed;
  Passed = EvaluateSize("EndOfEvent()", "empty event", "An event without steps has no hits", GetHits(Collection).size(), 0) && Passed;
  Passed = EvaluateTrue("EndOfEvent()", "collection name", "The collection has the name of the detector", Collection != nullptr && Collection->GetName() == "CycleSD") && Passed;
  Passed = EvaluateTrue("EndOfEvent()", "detector name", "The collection knows the detector", Collection != nullptr && Collection->GetSDname() == "CycleSD") && Passed;

  // A second call does not add the collection again:
  bool SecondCall = UTSDHitShared::RunsCleanlyInChild([&]() {
    SD.EndOfEvent(HCE);
    return true;
  });
  Passed = EvaluateTrue("EndOfEvent()", "second call", "Ending an event twice does not crash", SecondCall) && Passed;
  delete HCE;

  // Every event gets a new empty collection:
  UTSDStepFixture G;
  int Volume = G.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector());
  SD.SetDetectorVolumeName("PanelLog");
  G4HCofThisEvent* First = G.BeginEvent(SD);
  SD.PostProcessHits(G.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
  G4VHitsCollection* FirstCollection = G.EndEvent(SD, First);
  G4HCofThisEvent* Second = G.BeginEvent(SD);
  G4VHitsCollection* SecondCollection = G.EndEvent(SD, Second);
  Passed = EvaluateSize("Initialize()", "first event", "The first event has one hit", GetHits(FirstCollection).size(), 1) && Passed;
  Passed = EvaluateSize("Initialize()", "second event", "The second event starts without hits", GetHits(SecondCollection).size(), 0) && Passed;
  Passed = EvaluateTrue("Initialize()", "second event", "The second event has its own collection", FirstCollection != SecondCollection) && Passed;
  delete First;
  delete Second;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestProcessHits()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector());
  MCScintillatorSDAccess SD("ProcessSD");
  SD.SetDetectorVolumeName("PanelLog");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4Step* Step = F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume);
  Passed = EvaluateTrue("ProcessHits()", "step with energy", "ProcessHits accepts the step", SD.ProcessHits(Step, nullptr)) && Passed;
  G4VHitsCollection* Collection = F.EndEvent(SD, HCE);
  Passed = EvaluateSize("ProcessHits()", "step with energy", "ProcessHits does not create hits - this is done in PostProcessHits", GetHits(Collection).size(), 0) && Passed;
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestSimpleHit()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(20*cm, 0, 0));
  int Crystal = F.AddVolume("Crystal", G4ThreeVector(2*cm, 2*cm, 2*cm), G4ThreeVector(0, 3*cm, 0));
  (void) Panel;

  MCScintillatorSDAccess SD("SimpleSD");
  SD.SetDetectorVolumeName("PanelLog");
  SD.SetCommonVolumeName("Panel");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4Step* Step = F.CreateStep(662*keV, G4ThreeVector(20.5*cm, 3.0*cm, 0.5*cm), G4ThreeVector(20.9*cm, 3.2*cm, 0.4*cm), fPostStepDoItProc, 17, 0.0, Crystal);
  Passed = EvaluateTrue("PostProcessHits()", "step with energy", "A step with energy is processed", SD.PostProcessHits(Step)) && Passed;
  G4VHitsCollection* Collection = F.EndEvent(SD, HCE);
  vector<MCScintillatorHitAccess*> Hits = GetHits(Collection);
  Passed = EvaluateSize("PostProcessHits()", "step with energy", "One hit is created", Hits.size(), 1) && Passed;
  if (Hits.size() == 1) {
    MCScintillatorHitAccess* Hit = Hits[0];
    Passed = EvaluateNear("PostProcessHits()", "662 keV", "The hit has the deposited energy", Hit->GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "662 keV", "The ADC counts are the energy in keV", Hit->GetADCCounts(), 662.0, 1e-9) && Passed;
    Passed = Evaluate("PostProcessHits()", "detector", "The hit knows the detector volume", MString(Hit->GetDetectorName().c_str()), MString("Panel")) && Passed;
    Passed = EvaluateTrue("PostProcessHits()", "track 17", "The origin of the hit is the ID of the track", Hit->m_Origins.count(17) == 1) && Passed;
    Passed = Evaluate("PostProcessHits()", "type", "The hit is a scintillator hit", Hit->GetDetectorType(), (int) MDDetector::c_Scintillator) && Passed;
    // Discretized without a unique position: the center of the deepest volume:
    Passed = EvaluateNear("PostProcessHits()", "center", "The discretized x position is the center of the crystal", Hit->GetPosition().x()/cm, 20.0, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "center", "The discretized y position is the center of the crystal", Hit->GetPosition().y()/cm, 3.0, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "center", "The discretized z position is the center of the crystal", Hit->GetPosition().z()/cm, 0.0, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "no time resolution", "Without time resolution the time stays zero", Hit->GetTime(), 0.0, 1e-12) && Passed;
  }
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestZeroEnergy()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector());
  MCScintillatorSDAccess SD("ZeroSD");
  SD.SetDetectorVolumeName("PanelLog");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4Step* Step = F.CreateStep(0.0, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume);
  Passed = EvaluateFalse("PostProcessHits()", "zero energy", "A step without energy deposit is not processed", SD.PostProcessHits(Step)) && Passed;
  G4VHitsCollection* Collection = F.EndEvent(SD, HCE);
  Passed = EvaluateSize("PostProcessHits()", "zero energy", "A step without energy deposit creates no hit", GetHits(Collection).size(), 0) && Passed;
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestStepStatusPosition()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(20*cm, 0, 0));
  (void) Panel;
  G4ThreeVector Pre(21.0*cm, 1.0*cm, 2.0*cm);
  G4ThreeVector Post(23.0*cm, 3.0*cm, 6.0*cm);

  struct Case { G4StepStatus Status; G4ThreeVector Expected; const char* Name; };
  vector<Case> Cases = {
    { fPostStepDoItProc, Post, "post step process" },
    { fGeomBoundary, Post, "geometry boundary" },
    { fAlongStepDoItProc, 0.5*(Pre + Post), "along step process" },
    { fUndefined, Pre, "undefined status" }
  };
  for (const Case& C: Cases) {
    MCScintillatorSDAccess SD(string("StatusSD_") + C.Name);
    SD.SetDetectorVolumeName("PanelLog");
    SD.m_DiscretizeHits = false;
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Pre, Post, C.Status));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", C.Name, "One hit is created", Hits.size(), 1) && Passed;
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", C.Name, "The x position depends on the step status as documented in Geant4", Hits[0]->GetPosition().x()/cm, C.Expected.x()/cm, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", C.Name, "The y position depends on the step status as documented in Geant4", Hits[0]->GetPosition().y()/cm, C.Expected.y()/cm, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", C.Name, "The z position depends on the step status as documented in Geant4", Hits[0]->GetPosition().z()/cm, C.Expected.z()/cm, 1e-6) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestDiscretization()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(20*cm, 0, 0));
  int Crystal = F.AddVolume("Crystal", G4ThreeVector(2*cm, 2*cm, 2*cm), G4ThreeVector(0, 3*cm, 0));
  (void) Panel;

  // The unique position is relative to the center of the deepest volume:
  {
    MCScintillatorSDAccess SD("UniqueSD");
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetUniquePosition("CrystalLog", G4ThreeVector(1.0*cm, 0.5*cm, -0.25*cm));
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(20.5*cm, 3.5*cm, 1.0*cm), G4ThreeVector(20.6*cm, 3.6*cm, 1.1*cm), fPostStepDoItProc, 1, 0.0, Crystal));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("SetUniquePosition()", "one hit", "One hit is created", Hits.size(), 1) && Passed;
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetUniquePosition()", "1, 0.5, -0.25 cm", "The discretized x position is center plus unique position", Hits[0]->GetPosition().x()/cm, 21.0, 1e-6) && Passed;
      Passed = EvaluateNear("SetUniquePosition()", "1, 0.5, -0.25 cm", "The discretized y position is center plus unique position", Hits[0]->GetPosition().y()/cm, 3.5, 1e-6) && Passed;
      Passed = EvaluateNear("SetUniquePosition()", "1, 0.5, -0.25 cm", "The discretized z position is center plus unique position", Hits[0]->GetPosition().z()/cm, -0.25, 1e-6) && Passed;
    }
    delete HCE;
  }

  // Without discretization the position of the step is kept:
  {
    MCScintillatorSDAccess SD("NoDiscretizeSD");
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetUniquePosition("CrystalLog", G4ThreeVector(1.0*cm, 0.5*cm, -0.25*cm));
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(20.5*cm, 3.5*cm, 1.0*cm), G4ThreeVector(20.6*cm, 3.6*cm, 1.1*cm), fPostStepDoItProc, 1, 0.0, Crystal));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetDiscretizeHits()", "false", "The position of the step is kept", Hits[0]->GetPosition().x()/cm, 20.6, 1e-6) && Passed;
      Passed = EvaluateNear("SetDiscretizeHits()", "false", "The y position of the step is kept", Hits[0]->GetPosition().y()/cm, 3.6, 1e-6) && Passed;
      Passed = EvaluateNear("SetDiscretizeHits()", "false", "The z position of the step is kept", Hits[0]->GetPosition().z()/cm, 1.1, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("SetDiscretizeHits()", "false", "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestVolumeHistory()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(20*cm, 0, 0));
  int Crystal = F.AddVolume("Crystal", G4ThreeVector(2*cm, 2*cm, 2*cm), G4ThreeVector(0, 3*cm, 0));
  (void) Panel;

  // The history is stored from the top down to the common volume:
  {
    MCScintillatorSDAccess SD("HistorySD");
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetCommonVolumeName("Panel");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(20.5*cm, 3.5*cm, 1.0*cm), G4ThreeVector(20.6*cm, 3.6*cm, 1.1*cm), fPostStepDoItProc, 1, 0.0, Crystal));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateSize("SetCommonVolumeName()", "Panel", "The history ends at the common volume", Hits[0]->m_VolumeHistory.size(), 1) && Passed;
      if (Hits[0]->m_VolumeHistory.size() == 1) {
        Passed = Evaluate("SetCommonVolumeName()", "Panel", "The history contains the common volume", MString(Hits[0]->m_VolumeHistory[0].c_str()), MString("Panel")) && Passed;
      }
    }
    delete HCE;
  }

  // Without a common volume the complete history is stored:
  {
    MCScintillatorSDAccess SD("FullHistorySD");
    SD.SetDetectorVolumeName("PanelLog");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(20.5*cm, 3.5*cm, 1.0*cm), G4ThreeVector(20.6*cm, 3.6*cm, 1.1*cm), fPostStepDoItProc, 1, 0.0, Crystal));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateSize("PostProcessHits()", "no common volume", "Without a common volume all volumes below the world are stored", Hits[0]->m_VolumeHistory.size(), 2) && Passed;
      if (Hits[0]->m_VolumeHistory.size() == 2) {
        Passed = Evaluate("PostProcessHits()", "no common volume", "The history starts with the volume below the world", MString(Hits[0]->m_VolumeHistory[0].c_str()), MString("Panel")) && Passed;
        Passed = Evaluate("PostProcessHits()", "no common volume", "The history ends with the deepest volume", MString(Hits[0]->m_VolumeHistory[1].c_str()), MString("Crystal")) && Passed;
      }
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestMerging()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(20*cm, 0, 0));
  int CrystalA = F.AddVolume("CrystalA", G4ThreeVector(2*cm, 2*cm, 2*cm), G4ThreeVector(0, 3*cm, 0), nullptr, Panel);
  int CrystalB = F.AddVolume("CrystalB", G4ThreeVector(2*cm, 2*cm, 2*cm), G4ThreeVector(0, -3*cm, 0), nullptr, Panel);
  int Panel2 = F.AddPlacement("Panel2", Panel, G4ThreeVector(-20*cm, 0, 0), nullptr, 0);
  int Crystal2 = F.AddPlacement("Crystal2", CrystalA, G4ThreeVector(0, 3*cm, 0), nullptr, Panel2);

  G4ThreeVector InA(20.0*cm, 3.0*cm, 0.0);
  G4ThreeVector InB(20.0*cm, -3.0*cm, 0.0);
  G4ThreeVector In2(-20.0*cm, 3.0*cm, 0.0);

  // Two steps in the same crystal are merged:
  {
    MCScintillatorSDAccess SD("MergeSameSD");
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetCommonVolumeName("Panel");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, InA, InA, fPostStepDoItProc, 1, 0.0, CrystalA));
    SD.PostProcessHits(F.CreateStep(250*keV, InA, InA, fPostStepDoItProc, 2, 0.0, CrystalA));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", "two steps in one crystal", "Two steps in the same volume give one hit", Hits.size(), 1) && Passed;
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", "two steps in one crystal", "The energies are added", Hits[0]->GetEnergy()/keV, 350.0, 1e-9) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "two steps in one crystal", "The ADC counts are added", Hits[0]->GetADCCounts(), 350.0, 1e-9) && Passed;
      Passed = EvaluateSize("PostProcessHits()", "two steps in one crystal", "The origins of both tracks are kept", Hits[0]->m_Origins.size(), 2) && Passed;
    }
    delete HCE;
  }

  // Steps in different crystals below the common volume are merged, too:
  {
    MCScintillatorSDAccess SD("MergeCrystalsSD");
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetCommonVolumeName("Panel");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, InA, InA, fPostStepDoItProc, 1, 0.0, CrystalA));
    SD.PostProcessHits(F.CreateStep(200*keV, InB, InB, fPostStepDoItProc, 2, 0.0, CrystalB));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", "two crystals, one panel", "Steps in the crystals of one panel give one hit", Hits.size(), 1) && Passed;
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", "two crystals, one panel", "The energies of both crystals are added", Hits[0]->GetEnergy()/keV, 300.0, 1e-9) && Passed;
    }
    delete HCE;
  }

  // Steps in different panels are not merged:
  {
    MCScintillatorSDAccess SD("MergePanelsSD");
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetCommonVolumeName("Panel");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, InA, InA, fPostStepDoItProc, 1, 0.0, CrystalA));
    SD.PostProcessHits(F.CreateStep(200*keV, In2, In2, fPostStepDoItProc, 2, 0.0, Crystal2));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", "two panels", "Steps in different panels give two hits", Hits.size(), 2) && Passed;
    if (Hits.size() == 2) {
      Passed = EvaluateNear("PostProcessHits()", "two panels", "The first panel keeps its energy", Hits[0]->GetEnergy()/keV, 100.0, 1e-9) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "two panels", "The second panel keeps its energy", Hits[1]->GetEnergy()/keV, 200.0, 1e-9) && Passed;
      Passed = Evaluate("PostProcessHits()", "two panels", "The second hit knows its panel", MString(Hits[1]->GetDetectorName().c_str()), MString("Panel2")) && Passed;
    }
    delete HCE;
  }

  // Without discretization every step is a hit:
  {
    MCScintillatorSDAccess SD("NoMergeSD");
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, InA, InA, fPostStepDoItProc, 1, 0.0, CrystalA));
    SD.PostProcessHits(F.CreateStep(250*keV, InA, InA, fPostStepDoItProc, 2, 0.0, CrystalA));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", "not discretized", "Without discretization every step is its own hit", Hits.size(), 2) && Passed;
    delete HCE;
  }

  // Without a common volume, crystals are separate hits:
  {
    MCScintillatorSDAccess SD("NoCommonSD");
    SD.SetDetectorVolumeName("PanelLog");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, InA, InA, fPostStepDoItProc, 1, 0.0, CrystalA));
    SD.PostProcessHits(F.CreateStep(200*keV, InB, InB, fPostStepDoItProc, 2, 0.0, CrystalB));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", "no common volume", "Without a common volume the crystals are separate hits", Hits.size(), 2) && Passed;
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestEnergyLossMap()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(20*cm, 0, 0));
  int Crystal = F.AddVolume("Crystal", G4ThreeVector(2*cm, 2*cm, 2*cm), G4ThreeVector(0, 3*cm, 0));
  (void) Panel;

  // The map depends on x in the frame of the crystal: 2 + 0.5 x[cm]
  vector<double> X = { -10.0, 10.0 };
  vector<double> Y = { -10.0, 10.0 };
  vector<double> Z = { -10.0, 10.0 };
  vector<double> Values;
  for (unsigned int z = 0; z < 2; ++z) {
    for (unsigned int y = 0; y < 2; ++y) {
      for (unsigned int x = 0; x < 2; ++x) {
        Values.push_back(2.0 + 0.5*X[x]);
      }
    }
  }
  MFunction3D Map;
  Map.Set(X, Y, Z, Values);

  MCScintillatorSDAccess SD("LossSD");
  SD.SetDetectorVolumeName("PanelLog");
  SD.SetEnergyLossMap(Map);
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  // The step is at x = 1 cm in the frame of the crystal:
  SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(21.0*cm, 3.0*cm, 0.0), G4ThreeVector(21.0*cm, 3.0*cm, 0.0), fPostStepDoItProc, 1, 0.0, Crystal));
  vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
  Passed = EvaluateSize("SetEnergyLossMap()", "map value 2.5", "One hit is created", Hits.size(), 1) && Passed;
  if (Hits.size() == 1) {
    Passed = EvaluateNear("SetEnergyLossMap()", "map value 2.5", "The energy is multiplied with the map value at the position", Hits[0]->GetEnergy()/keV, 250.0, 1e-6) && Passed;
    Passed = EvaluateNear("SetEnergyLossMap()", "map value 2.5", "The ADC counts follow the reduced energy", Hits[0]->GetADCCounts(), 250.0, 1e-6) && Passed;
  }
  delete HCE;

  // A map which is zero at the position removes the hit:
  vector<double> Zeros(8, 0.0);
  MFunction3D ZeroMap;
  ZeroMap.Set(X, Y, Z, Zeros);
  MCScintillatorSDAccess ZeroSD("ZeroLossSD");
  ZeroSD.SetDetectorVolumeName("PanelLog");
  ZeroSD.SetEnergyLossMap(ZeroMap);
  G4HCofThisEvent* ZeroHCE = F.BeginEvent(ZeroSD);
  G4Step* ZeroStep = F.CreateStep(100*keV, G4ThreeVector(21.0*cm, 3.0*cm, 0.0), G4ThreeVector(21.0*cm, 3.0*cm, 0.0), fPostStepDoItProc, 1, 0.0, Crystal);
  bool ZeroResult = true;
  {
    // The detector reports the removed hit on the MEGAlib streams:
    DisableDefaultStreams();
    ZeroResult = ZeroSD.PostProcessHits(ZeroStep);
    EnableDefaultStreams();
  }
  Passed = EvaluateFalse("SetEnergyLossMap()", "zero map", "A step whose energy is removed by the map is not processed", ZeroResult) && Passed;
  Passed = EvaluateSize("SetEnergyLossMap()", "zero map", "A zero energy after the map creates no hit", GetHits(F.EndEvent(ZeroSD, ZeroHCE)).size(), 0) && Passed;
  Passed = EvaluateFalse("SetEnergyLossMap()", "zero map", "A removed hit does not mark the track as digitized", F.GetTrackInformation(ZeroStep)->IsDigitized()) && Passed;
  delete ZeroHCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestTimeResolution()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector());

  MCScintillatorSDAccess WithTime("TimeSD");
  WithTime.SetDetectorVolumeName("PanelLog");
  WithTime.SetHasTimeResolution(true);
  G4HCofThisEvent* HCE = F.BeginEvent(WithTime);
  WithTime.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 12.5*ns, Volume));
  vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(WithTime, HCE));
  if (Hits.size() == 1) {
    Passed = EvaluateNear("SetHasTimeResolution()", "true", "The hit has the global time of the track", Hits[0]->GetTime()/ns, 12.5, 1e-9) && Passed;
  } else {
    Passed = EvaluateSize("SetHasTimeResolution()", "true", "One hit is created", Hits.size(), 1) && Passed;
  }
  delete HCE;

  // Merged hits keep the earliest time:
  MCScintillatorSDAccess Merged("TimeMergeSD");
  Merged.SetDetectorVolumeName("PanelLog");
  Merged.SetHasTimeResolution(true);
  G4HCofThisEvent* MergedHCE = F.BeginEvent(Merged);
  Merged.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 30.0*ns, Volume));
  Merged.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 2, 10.0*ns, Volume));
  vector<MCScintillatorHitAccess*> MergedHits = GetHits(F.EndEvent(Merged, MergedHCE));
  if (MergedHits.size() == 1) {
    Passed = EvaluateNear("SetHasTimeResolution()", "merged", "A hit merged from an earlier step has the earlier time", MergedHits[0]->GetTime()/ns, 10.0, 1e-9) && Passed;
  } else {
    Passed = EvaluateSize("SetHasTimeResolution()", "merged", "The steps are merged into one hit", MergedHits.size(), 1) && Passed;
  }
  delete MergedHCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestTrackInformation()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector());
  MCScintillatorSDAccess SD("DigitizedSD");
  SD.SetDetectorVolumeName("PanelLog");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);

  G4Step* Used = F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume);
  Passed = EvaluateFalse("PostProcessHits()", "before", "A new track is not digitized", F.GetTrackInformation(Used)->IsDigitized()) && Passed;
  SD.PostProcessHits(Used);
  Passed = EvaluateTrue("PostProcessHits()", "after a hit", "A track with a hit is marked as digitized", F.GetTrackInformation(Used)->IsDigitized()) && Passed;

  G4Step* Empty = F.CreateStep(0.0, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 2, 0.0, Volume);
  SD.PostProcessHits(Empty);
  Passed = EvaluateFalse("PostProcessHits()", "no energy", "A track without energy deposit is not marked as digitized", F.GetTrackInformation(Empty)->IsDigitized()) && Passed;

  // Also merged hits mark the track as digitized:
  G4Step* Merged = F.CreateStep(50*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 3, 0.0, Volume);
  SD.PostProcessHits(Merged);
  Passed = EvaluateTrue("PostProcessHits()", "merged hit", "A track whose hit was merged is marked as digitized", F.GetTrackInformation(Merged)->IsDigitized()) && Passed;
  F.EndEvent(SD, HCE);
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestRotatedVolumes()
{
  bool Passed = true;

  // Without discretization, the position must survive the transformation into the detector frame and back:
  UTSDStepFixture F;
  G4RotationMatrix* PanelRotation = new G4RotationMatrix();
  PanelRotation->rotateZ(30.0*deg);
  PanelRotation->rotateX(20.0*deg);
  int Panel = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(20*cm, 5*cm, -3*cm), PanelRotation);
  G4RotationMatrix* CrystalRotation = new G4RotationMatrix();
  CrystalRotation->rotateY(-40.0*deg);
  int Crystal = F.AddVolume("Crystal", G4ThreeVector(2*cm, 2*cm, 2*cm), G4ThreeVector(1*cm, 3*cm, 0.5*cm), CrystalRotation, Panel);

  vector<G4ThreeVector> Positions = {
    G4ThreeVector(21.3*cm, 6.1*cm, -2.7*cm),
    G4ThreeVector(19.0*cm, 2.0*cm, -3.4*cm),
    G4ThreeVector(23.7*cm, 9.9*cm, 1.2*cm)
  };
  for (unsigned int p = 0; p < Positions.size(); ++p) {
    MCScintillatorSDAccess SD(string("RotatedSD") + to_string(p));
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Positions[p], Positions[p], fPostStepDoItProc, 1, 0.0, Crystal));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      G4ThreeVector Result = Hits[0]->GetPosition();
      Passed = EvaluateNear("PostProcessHits()", string("rotated volumes, position ") + to_string(p), "The position is the same after the transformation into the detector frame and back", (Result - Positions[p]).mag()/mm, 0.0, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", string("rotated volumes, position ") + to_string(p), "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  // The discretized hit is at the center of the crystal, which is the origin of its frame:
  {
    MCScintillatorSDAccess SD("RotatedCenterSD");
    SD.SetDetectorVolumeName("PanelLog");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Positions[0], Positions[0], fPostStepDoItProc, 1, 0.0, Crystal));
    vector<MCScintillatorHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      // The crystal center in the frame of the panel is (1, 3, 0.5) cm, the panel is rotated by the inverse of the given frame rotation:
      G4ThreeVector Center = (PanelRotation->inverse())*G4ThreeVector(1*cm, 3*cm, 0.5*cm) + G4ThreeVector(20*cm, 5*cm, -3*cm);
      Passed = EvaluateNear("PostProcessHits()", "rotated volumes, discretized", "The discretized hit is at the center of the crystal", (Hits[0]->GetPosition() - Center).mag()/mm, 0.0, 1e-6) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorSD::TestNeverTriggering()
{
  bool Passed = true;

  // A detector which never triggers reports the energy loss to the event action but still creates the hit (executed in a child process, since it needs the event manager):
  bool Works = UTSDHitShared::RunsCleanlyInChild([]() {
    UTSDStepFixture F;
    F.SetupEventAction();
    int Volume = F.AddVolume("Panel", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector());
    MCScintillatorSDAccess SD("NeverSD");
    SD.SetDetectorVolumeName("PanelLog");
    SD.SetIsNeverTriggering(true);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    bool Result = SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    G4VHitsCollection* Collection = F.EndEvent(SD, HCE);
    MCScintillatorHitsCollection* HC = static_cast<MCScintillatorHitsCollection*>(Collection);
    return Result == true && HC != nullptr && HC->entries() == 1;
  });
  Passed = EvaluateTrue("SetIsNeverTriggering()", "with event action", "A never triggering detector still creates its hit", Works) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTScintillatorSD Test;
  return Test.Run() == true ? 0 : 1;
}
