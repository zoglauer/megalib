/*
 * UTAngerCameraSD.cc
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
#include "MCAngerCameraSD.hh"
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
#include <sstream>
#include <string>
#include <vector>
using namespace std;


//! Class exposing the protected members of the hit to the unit test
class MCAngerCameraHitAccess : public MCAngerCameraHit
{
public:
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Sensitive detector exposing the protected members of MCSD to the unit test
class MCAngerCameraSDAccess : public MCAngerCameraSD
{
public:
  MCAngerCameraSDAccess(G4String Name) : MCAngerCameraSD(Name)
  {
    SetDetectorVolumeName("PanelLog");
  }
  using MCSD::m_DiscretizeHits;
  using MCSD::m_Type;
};


//! Unit test class for MCAngerCameraSD
class UTAngerCameraSD : public MUnitTest
{
public:
  //! Default constructor
  UTAngerCameraSD() : MUnitTest("UTAngerCameraSD") {}
  //! Default destructor
  virtual ~UTAngerCameraSD() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructor
  bool TestConstruction();
  //! Test the event cycle
  bool TestEventCycle();
  //! Test a simple hit and the bars
  bool TestSimpleHit();
  //! Test steps without energy deposit
  bool TestZeroEnergy();
  //! Test the position of the hit for the different step statuses
  bool TestStepStatusPosition();
  //! Test the discretization
  bool TestDiscretization();
  //! Test the merging of hits
  bool TestMerging();
  //! Test the energy loss map
  bool TestEnergyLossMap();
  //! Test the time resolution
  bool TestTimeResolution();
  //! Test the digitized flag
  bool TestTrackInformation();
  //! Test the coordinate transformations
  bool TestRotatedVolumes();
  //! Test detectors which are never triggering
  bool TestNeverTriggering();

  //! Return the hits of the event
  vector<MCAngerCameraHitAccess*> GetHits(G4VHitsCollection* Collection);
  //! Return the text printed by the hit
  MString GetPrint(MCAngerCameraHit* Hit);
};


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestEventCycle() && Passed;
  Passed = TestSimpleHit() && Passed;
  Passed = TestZeroEnergy() && Passed;
  Passed = TestStepStatusPosition() && Passed;
  Passed = TestDiscretization() && Passed;
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


vector<MCAngerCameraHitAccess*> UTAngerCameraSD::GetHits(G4VHitsCollection* Collection)
{
  vector<MCAngerCameraHitAccess*> Hits;
  MCAngerCameraHitsCollection* HC = static_cast<MCAngerCameraHitsCollection*>(Collection);
  if (HC == nullptr) return Hits;
  for (size_t h = 0; h < HC->entries(); ++h) {
    Hits.push_back(static_cast<MCAngerCameraHitAccess*>((*HC)[h]));
  }
  return Hits;
}


////////////////////////////////////////////////////////////////////////////////


MString UTAngerCameraSD::GetPrint(MCAngerCameraHit* Hit)
{
  ostringstream Out;
  streambuf* Old = cout.rdbuf(Out.rdbuf());
  Hit->Print();
  cout.rdbuf(Old);
  return MString(Out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestConstruction()
{
  bool Passed = true;

  MCAngerCameraSDAccess SD("Cal");
  Passed = Evaluate("MCAngerCameraSD()", "name", "The name of the detector is the one given", MString(SD.GetName().c_str()), MString("Cal")) && Passed;
  Passed = Evaluate("MCAngerCameraSD()", "type", "The type of the detector is Anger camera", SD.GetType(), MCSD::c_AngerCamera) && Passed;
  Passed = Evaluate("MCAngerCameraSD()", "collections", "The detector has one hit collection", SD.GetNumberOfCollections(), 1) && Passed;
  Passed = Evaluate("MCAngerCameraSD()", "collection name", "The name of the collection is the name of the detector", MString(SD.GetCollectionName(0).c_str()), MString("Cal")) && Passed;
  Passed = EvaluateTrue("MCAngerCameraSD()", "default", "Hits are discretized by default", SD.m_DiscretizeHits) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestEventCycle()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
  MCAngerCameraSDAccess SD("CycleSD");
  G4HCofThisEvent* First = F.BeginEvent(SD);
  SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
  G4VHitsCollection* FirstCollection = F.EndEvent(SD, First);
  G4HCofThisEvent* Second = F.BeginEvent(SD);
  G4VHitsCollection* SecondCollection = F.EndEvent(SD, Second);
  Passed = EvaluateTrue("EndOfEvent()", "collection", "The hit collection is handed to the event", FirstCollection != nullptr && SecondCollection != nullptr) && Passed;
  Passed = EvaluateSize("Initialize()", "first event", "The first event has one hit", GetHits(FirstCollection).size(), 1) && Passed;
  Passed = EvaluateSize("Initialize()", "second event", "The second event starts without hits", GetHits(SecondCollection).size(), 0) && Passed;

  // ProcessHits does nothing:
  G4HCofThisEvent* Third = F.BeginEvent(SD);
  Passed = EvaluateTrue("ProcessHits()", "any step", "ProcessHits accepts the step", SD.ProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume), nullptr)) && Passed;
  Passed = EvaluateSize("ProcessHits()", "any step", "ProcessHits does not create hits", GetHits(F.EndEvent(SD, Third)).size(), 0) && Passed;
  delete First;
  delete Second;
  delete Third;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestSimpleHit()
{
  bool Passed = true;

  UTSDStepFixture F;
  F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector(20*cm, 0, 0));
  int Bar = F.AddVolume("Bar", G4ThreeVector(0.5*cm, 0.5*cm, 2*cm), G4ThreeVector(-3.5*cm, 2.5*cm, 0));

  MCAngerCameraSDAccess SD("SimpleSD");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4Step* Step = F.CreateStep(662*keV, G4ThreeVector(16.5*cm, 2.5*cm, 0.5*cm), G4ThreeVector(16.6*cm, 2.6*cm, 0.4*cm), fPostStepDoItProc, 17, 0.0, Bar);
  Passed = EvaluateTrue("PostProcessHits()", "step with energy", "A step with energy is processed", SD.PostProcessHits(Step)) && Passed;
  vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
  Passed = EvaluateSize("PostProcessHits()", "step with energy", "One hit is created", Hits.size(), 1) && Passed;
  if (Hits.size() == 1) {
    MCAngerCameraHitAccess* Hit = Hits[0];
    Passed = EvaluateNear("PostProcessHits()", "662 keV", "The hit has the deposited energy", Hit->GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = Evaluate("PostProcessHits()", "detector", "The hit knows the detector volume", MString(Hit->GetDetectorName().c_str()), MString("Panel")) && Passed;
    Passed = EvaluateTrue("PostProcessHits()", "track 17", "The origin of the hit is the ID of the track", Hit->m_Origins.count(17) == 1) && Passed;
    Passed = Evaluate("PostProcessHits()", "type", "The hit is a Anger camera hit", Hit->GetDetectorType(), (int) MDDetector::c_AngerCamera) && Passed;
    // x and y are kept, z is set to the center of the deepest volume (2D camera):
    Passed = EvaluateNear("PostProcessHits()", "center", "The x position of the step is kept", Hit->GetPosition().x()/cm, 16.6, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "center", "The y position of the step is kept", Hit->GetPosition().y()/cm, 2.6, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "center", "The discretized z position is the center plane without 3D", Hit->GetPosition().z()/cm, 0.0, 1e-6) && Passed;
  }
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestZeroEnergy()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
  MCAngerCameraSDAccess SD("ZeroSD");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  Passed = EvaluateFalse("PostProcessHits()", "zero energy", "A step without energy deposit is not processed", SD.PostProcessHits(F.CreateStep(0.0, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume))) && Passed;
  Passed = EvaluateSize("PostProcessHits()", "zero energy", "A step without energy deposit creates no hit", GetHits(F.EndEvent(SD, HCE)).size(), 0) && Passed;
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestStepStatusPosition()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector(20*cm, 0, 0));
  G4ThreeVector Pre(20.5*cm, 1.0*cm, 0.2*cm);
  G4ThreeVector Post(21.5*cm, 3.0*cm, 1.0*cm);

  struct Case { G4StepStatus Status; G4ThreeVector Expected; const char* Name; };
  vector<Case> Cases = {
    { fPostStepDoItProc, Post, "post step process" },
    { fGeomBoundary, Post, "geometry boundary" },
    { fAlongStepDoItProc, 0.5*(Pre + Post), "along step process" },
    { fUndefined, Pre, "undefined status" }
  };
  for (const Case& C: Cases) {
    MCAngerCameraSDAccess SD(string("StatusSD_") + C.Name);
    SD.m_DiscretizeHits = false;
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Pre, Post, C.Status, 1, 0.0, Volume));
    vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", C.Name, "One hit is created", Hits.size(), 1) && Passed;
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", C.Name, "The x position depends on the step status", Hits[0]->GetPosition().x()/cm, C.Expected.x()/cm, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", C.Name, "The y position depends on the step status", Hits[0]->GetPosition().y()/cm, C.Expected.y()/cm, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", C.Name, "The z position depends on the step status", Hits[0]->GetPosition().z()/cm, C.Expected.z()/cm, 1e-6) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestDiscretization()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector(20*cm, 0, 0));
  G4ThreeVector Step(22.3*cm, -1.2*cm, 1.1*cm);

  // Discretized in 2D: only z is set to zero in the frame of the deepest volume:
  {
    MCAngerCameraSDAccess SD("Disc2DSD");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Step, Step, fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", "2D", "The x position is kept", Hits[0]->GetPosition().x()/cm, 22.3, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "2D", "The y position is kept", Hits[0]->GetPosition().y()/cm, -1.2, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "2D", "The z position is discretized", Hits[0]->GetPosition().z()/cm, 0.0, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", "2D", "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  // With 3D the depth is kept:
  {
    MCAngerCameraSDAccess SD("Disc3DSD");
    SD.SetIs3D(true);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Step, Step, fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetIs3D()", "true", "The x position is kept", Hits[0]->GetPosition().x()/cm, 22.3, 1e-6) && Passed;
      Passed = EvaluateNear("SetIs3D()", "true", "The z position is kept", Hits[0]->GetPosition().z()/cm, 1.1, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("SetIs3D()", "true", "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  // Without discretization everything is kept:
  {
    MCAngerCameraSDAccess SD("NoDiscSD");
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Step, Step, fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetDiscretizeHits()", "false", "The position is kept", (Hits[0]->GetPosition() - Step).mag()/mm, 0.0, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("SetDiscretizeHits()", "false", "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestMerging()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());

  // Steps in the same volume are merged into one hit at the energy weighted position:
  {
    MCAngerCameraSDAccess SD("MergeSD");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(1.0*cm, 0.0, 0), G4ThreeVector(1.0*cm, 0.0, 0), fPostStepDoItProc, 1, 0.0, Volume));
    SD.PostProcessHits(F.CreateStep(300*keV, G4ThreeVector(-1.0*cm, 2.0*cm, 0), G4ThreeVector(-1.0*cm, 2.0*cm, 0), fPostStepDoItProc, 2, 0.0, Volume));
    vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", "two steps", "Two steps in the same volume give one hit", Hits.size(), 1) && Passed;
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", "two steps", "The energies are added", Hits[0]->GetEnergy()/keV, 400.0, 1e-9) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "two steps", "The x position is energy weighted", Hits[0]->GetPosition().x()/cm, -0.5, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "two steps", "The y position is energy weighted", Hits[0]->GetPosition().y()/cm, 1.5, 1e-6) && Passed;
      Passed = EvaluateSize("PostProcessHits()", "two steps", "The origins are kept", Hits[0]->m_Origins.size(), 2) && Passed;
    }
    delete HCE;
  }

  // Without discretization nothing is merged:
  {
    MCAngerCameraSDAccess SD("NoMergeSD");
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 2, 0.0, Volume));
    Passed = EvaluateSize("PostProcessHits()", "no discretization", "Without discretization hits are not merged", GetHits(F.EndEvent(SD, HCE)).size(), 2) && Passed;
    delete HCE;
  }

  // Same position in different detector volumes is not merged:
  {
    UTSDStepFixture G;
    int Panel1 = G.AddVolume("Panel1", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector(20*cm, 0, 0));
    int Panel2 = G.AddPlacement("Panel2", Panel1, G4ThreeVector(-20*cm, 0, 0), nullptr, 0);
    MCAngerCameraSDAccess SD("TwoPanelSD");
    G4HCofThisEvent* HCE = G.BeginEvent(SD);
    SD.PostProcessHits(G.CreateStep(100*keV, G4ThreeVector(20.2*cm, 0.2*cm, 0), G4ThreeVector(20.2*cm, 0.2*cm, 0), fPostStepDoItProc, 1, 0.0, Panel1));
    SD.PostProcessHits(G.CreateStep(100*keV, G4ThreeVector(-19.8*cm, 0.2*cm, 0), G4ThreeVector(-19.8*cm, 0.2*cm, 0), fPostStepDoItProc, 2, 0.0, Panel2));
    Passed = EvaluateSize("PostProcessHits()", "two detector volumes", "The same position in two detector volumes gives two hits", GetHits(G.EndEvent(SD, HCE)).size(), 2) && Passed;
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestEnergyLossMap()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());

  // A map with a constant value 0.5:
  MFunction3D Map;
  vector<double> X = { -10.0, 10.0 };
  vector<double> Values(8, 0.5);
  Map.Set(X, X, X, Values);

  {
    MCAngerCameraSDAccess SD("LossSD");
    SD.SetEnergyLossMap(Map);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(200*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetEnergyLossMap()", "0.5", "The energy is reduced", Hits[0]->GetEnergy()/keV, 100.0, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("SetEnergyLossMap()", "0.5", "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  // A map with zero: no hit
  {
    vector<double> Zeros(8, 0.0);
    MFunction3D ZeroMap;
    ZeroMap.Set(X, X, X, Zeros);
    MCAngerCameraSDAccess SD("ZeroLossSD");
    SD.SetEnergyLossMap(ZeroMap);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    // The detector reports the zero energy on the screen - keep the output clean:
    ostringstream Silent;
    streambuf* Old = cout.rdbuf(Silent.rdbuf());
    bool Result = SD.PostProcessHits(F.CreateStep(200*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    cout.rdbuf(Old);
    Passed = EvaluateFalse("SetEnergyLossMap()", "0.0", "A zero energy after the loss is not processed", Result) && Passed;
    Passed = EvaluateSize("SetEnergyLossMap()", "0.0", "No hit is created", GetHits(F.EndEvent(SD, HCE)).size(), 0) && Passed;
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestTimeResolution()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
  for (bool Resolution: { false, true }) {
    MCAngerCameraSDAccess SD(string("TimeSD") + to_string(Resolution));
    SD.SetHasTimeResolution(Resolution);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 12.5*ns, Volume));
    vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetHasTimeResolution()", to_string(Resolution), "The time is the global time only with time resolution", Hits[0]->GetTime()/ns, Resolution ? 12.5 : 0.0, 1e-9) && Passed;
    } else {
      Passed = EvaluateSize("SetHasTimeResolution()", to_string(Resolution), "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestTrackInformation()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
  MCAngerCameraSDAccess SD("DigitizedSD");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4Step* With = F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume);
  SD.PostProcessHits(With);
  Passed = EvaluateTrue("PostProcessHits()", "energy", "A track with a hit is marked as digitized", F.GetTrackInformation(With)->IsDigitized()) && Passed;
  G4Step* Without = F.CreateStep(0.0, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 2, 0.0, Volume);
  SD.PostProcessHits(Without);
  Passed = EvaluateFalse("PostProcessHits()", "no energy", "A track without energy is not marked as digitized", F.GetTrackInformation(Without)->IsDigitized()) && Passed;
  G4Step* Merged = F.CreateStep(50*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 3, 0.0, Volume);
  SD.PostProcessHits(Merged);
  Passed = EvaluateTrue("PostProcessHits()", "merged", "A track with a merged hit is marked as digitized", F.GetTrackInformation(Merged)->IsDigitized()) && Passed;
  F.EndEvent(SD, HCE);
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestRotatedVolumes()
{
  bool Passed = true;

  UTSDStepFixture F;
  G4RotationMatrix* PanelRotation = new G4RotationMatrix();
  PanelRotation->rotateZ(30.0*deg);
  PanelRotation->rotateX(20.0*deg);
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector(20*cm, 5*cm, -3*cm), PanelRotation);
  G4RotationMatrix* BarRotation = new G4RotationMatrix();
  BarRotation->rotateY(-40.0*deg);
  int Bar = F.AddVolume("Bar", G4ThreeVector(0.5*cm, 0.5*cm, 2*cm), G4ThreeVector(0.5*cm, 0.5*cm, 0.0), BarRotation, Panel);

  vector<G4ThreeVector> Positions = {
    G4ThreeVector(21.3*cm, 6.1*cm, -2.7*cm),
    G4ThreeVector(19.0*cm, 2.0*cm, -3.4*cm)
  };
  for (unsigned int p = 0; p < Positions.size(); ++p) {
    MCAngerCameraSDAccess SD(string("RotatedSD") + to_string(p));
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Positions[p], Positions[p], fPostStepDoItProc, 1, 0.0, Bar));
    vector<MCAngerCameraHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", string("rotated, position ") + to_string(p), "The position is the same after the transformation into the detector frame and back", (Hits[0]->GetPosition() - Positions[p]).mag()/mm, 0.0, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", string("rotated, position ") + to_string(p), "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraSD::TestNeverTriggering()
{
  bool Passed = true;

  bool Works = UTSDHitShared::RunsCleanlyInChild([]() {
    UTSDStepFixture F;
    F.SetupEventAction();
    int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
    MCAngerCameraSDAccess SD("NeverSD");
    SD.SetIsNeverTriggering(true);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    bool Result = SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    MCAngerCameraHitsCollection* HC = static_cast<MCAngerCameraHitsCollection*>(F.EndEvent(SD, HCE));
    return Result == true && HC != nullptr && HC->entries() == 1;
  });
  Passed = EvaluateTrue("SetIsNeverTriggering()", "with event action", "A never triggering detector still creates its hit", Works) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTAngerCameraSD Test;
  return Test.Run() == true ? 0 : 1;
}
