/*
 * UTCalorBarSD.cc
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
#include "MCCalorBarSD.hh"
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
class MCCalorBarHitAccess : public MCCalorBarHit
{
public:
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Sensitive detector exposing the protected members of MCSD to the unit test
//! The detector has 8 x 8 bars of 1 x 1 cm (full size, as passed by the detector construction) and 4 cm depth without pitch or offset
class MCCalorBarSDAccess : public MCCalorBarSD
{
public:
  MCCalorBarSDAccess(G4String Name) : MCCalorBarSD(Name)
  {
    SetDetectorVolumeName("PanelLog");
    SetDetectorStructuralSize(G4ThreeVector(1.0*cm, 1.0*cm, 4.0*cm));
    SetDetectorStructuralPitch(G4ThreeVector(0.0, 0.0, 0.0));
    SetDetectorStructuralOffset(G4ThreeVector(0.0, 0.0, 0.0));
    SetDetectorStructuralDimension(G4ThreeVector(4.0*cm, 4.0*cm, 2.0*cm));
  }
  using MCSD::m_DiscretizeHits;
  using MCSD::m_Type;
};


//! Unit test class for MCCalorBarSD
class UTCalorBarSD : public MUnitTest
{
public:
  //! Default constructor
  UTCalorBarSD() : MUnitTest("UTCalorBarSD") {}
  //! Default destructor
  virtual ~UTCalorBarSD() {}

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
  //! Test the ADC counts
  bool TestADCCounts();
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
  vector<MCCalorBarHitAccess*> GetHits(G4VHitsCollection* Collection);
  //! Return the text printed by the hit
  MString GetPrint(MCCalorBarHit* Hit);
};


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarSD::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestEventCycle() && Passed;
  Passed = TestSimpleHit() && Passed;
  Passed = TestZeroEnergy() && Passed;
  Passed = TestStepStatusPosition() && Passed;
  Passed = TestDiscretization() && Passed;
  Passed = TestADCCounts() && Passed;
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


vector<MCCalorBarHitAccess*> UTCalorBarSD::GetHits(G4VHitsCollection* Collection)
{
  vector<MCCalorBarHitAccess*> Hits;
  MCCalorBarHitsCollection* HC = static_cast<MCCalorBarHitsCollection*>(Collection);
  if (HC == nullptr) return Hits;
  for (size_t h = 0; h < HC->entries(); ++h) {
    Hits.push_back(static_cast<MCCalorBarHitAccess*>((*HC)[h]));
  }
  return Hits;
}


////////////////////////////////////////////////////////////////////////////////


MString UTCalorBarSD::GetPrint(MCCalorBarHit* Hit)
{
  ostringstream Out;
  streambuf* Old = cout.rdbuf(Out.rdbuf());
  Hit->Print();
  cout.rdbuf(Old);
  return MString(Out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarSD::TestConstruction()
{
  bool Passed = true;

  MCCalorBarSDAccess SD("Cal");
  Passed = Evaluate("MCCalorBarSD()", "name", "The name of the detector is the one given", MString(SD.GetName().c_str()), MString("Cal")) && Passed;
  Passed = Evaluate("MCCalorBarSD()", "type", "The type of the detector is calorimeter", SD.GetType(), MCSD::c_Calorimeter) && Passed;
  Passed = Evaluate("MCCalorBarSD()", "collections", "The detector has one hit collection", SD.GetNumberOfCollections(), 1) && Passed;
  Passed = Evaluate("MCCalorBarSD()", "collection name", "The name of the collection is the name of the detector", MString(SD.GetCollectionName(0).c_str()), MString("Cal")) && Passed;
  Passed = EvaluateTrue("MCCalorBarSD()", "default", "Hits are discretized by default", SD.m_DiscretizeHits) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarSD::TestEventCycle()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
  MCCalorBarSDAccess SD("CycleSD");
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


bool UTCalorBarSD::TestSimpleHit()
{
  bool Passed = true;

  UTSDStepFixture F;
  F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector(20*cm, 0, 0));
  int Bar = F.AddVolume("Bar", G4ThreeVector(0.5*cm, 0.5*cm, 2*cm), G4ThreeVector(-3.5*cm, 2.5*cm, 0));

  MCCalorBarSDAccess SD("SimpleSD");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4Step* Step = F.CreateStep(662*keV, G4ThreeVector(16.5*cm, 2.5*cm, 0.5*cm), G4ThreeVector(16.6*cm, 2.6*cm, 0.4*cm), fPostStepDoItProc, 17, 0.0, Bar);
  Passed = EvaluateTrue("PostProcessHits()", "step with energy", "A step with energy is processed", SD.PostProcessHits(Step)) && Passed;
  vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
  Passed = EvaluateSize("PostProcessHits()", "step with energy", "One hit is created", Hits.size(), 1) && Passed;
  if (Hits.size() == 1) {
    MCCalorBarHitAccess* Hit = Hits[0];
    Passed = EvaluateNear("PostProcessHits()", "662 keV", "The hit has the deposited energy", Hit->GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = Evaluate("PostProcessHits()", "detector", "The hit knows the detector volume", MString(Hit->GetDetectorName().c_str()), MString("Panel")) && Passed;
    Passed = EvaluateTrue("PostProcessHits()", "track 17", "The origin of the hit is the ID of the track", Hit->m_Origins.count(17) == 1) && Passed;
    Passed = Evaluate("PostProcessHits()", "type", "The hit is a calorimeter hit", Hit->GetDetectorType(), (int) MDDetector::c_Calorimeter) && Passed;
    // Bar center at x = -3.5 cm -> bar 0, y = 2.5 cm -> bar 6:
    Passed = EvaluateTrue("PostProcessHits()", "x=-3.5 cm, y=2.5 cm", "The bars are (0, 6)", GetPrint(Hit).Contains("(0, 6)")) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "center", "The discretized x position is the center of the bar", Hit->GetPosition().x()/cm, 16.5, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "center", "The discretized y position is the center of the bar", Hit->GetPosition().y()/cm, 2.5, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "center", "The discretized z position is the center of the bar without 3D", Hit->GetPosition().z()/cm, 0.0, 1e-6) && Passed;
  }
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarSD::TestZeroEnergy()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
  MCCalorBarSDAccess SD("ZeroSD");
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  Passed = EvaluateFalse("PostProcessHits()", "zero energy", "A step without energy deposit is not processed", SD.PostProcessHits(F.CreateStep(0.0, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume))) && Passed;
  Passed = EvaluateSize("PostProcessHits()", "zero energy", "A step without energy deposit creates no hit", GetHits(F.EndEvent(SD, HCE)).size(), 0) && Passed;
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarSD::TestStepStatusPosition()
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
    MCCalorBarSDAccess SD(string("StatusSD_") + C.Name);
    SD.m_DiscretizeHits = false;
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Pre, Post, C.Status, 1, 0.0, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
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


bool UTCalorBarSD::TestDiscretization()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector(20*cm, 0, 0));
  G4ThreeVector Step(22.3*cm, -1.2*cm, 1.1*cm);

  // Discretized in 2D: the x and y positions are the ones of the detector center, z too:
  {
    MCCalorBarSDAccess SD("Disc2DSD");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Step, Step, fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", "2D", "The x position is discretized to the frame of the deepest volume", Hits[0]->GetPosition().x()/cm, 20.0, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "2D", "The y position is discretized", Hits[0]->GetPosition().y()/cm, 0.0, 1e-6) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "2D", "The z position is discretized", Hits[0]->GetPosition().z()/cm, 0.0, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", "2D", "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  // With 3D the depth is kept:
  {
    MCCalorBarSDAccess SD("Disc3DSD");
    SD.SetIs3D(true);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Step, Step, fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetIs3D()", "true", "The x position is discretized", Hits[0]->GetPosition().x()/cm, 20.0, 1e-6) && Passed;
      Passed = EvaluateNear("SetIs3D()", "true", "The z position is kept", Hits[0]->GetPosition().z()/cm, 1.1, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("SetIs3D()", "true", "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  // Without discretization everything is kept:
  {
    MCCalorBarSDAccess SD("NoDiscSD");
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Step, Step, fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
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


bool UTCalorBarSD::TestADCCounts()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());

  // Single sided: all energy on the top, no bottom:
  {
    MCCalorBarSDAccess SD("ADC2DSD");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(300*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", "2D", "The ADC counts are the energy in keV", Hits[0]->GetADCCountsTop(), 300.0, 1e-9) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "2D", "There is no bottom value", Hits[0]->GetADCCountsBottom(), -1.0, 1e-9) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", "2D", "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  // Double sided: top and bottom together are the energy, at every depth:
  for (double Z: { -1.9, -0.5, 0.0, 1.0, 1.9 }) {
    MCCalorBarSDAccess SD(string("ADC3DSD") + to_string(Z));
    SD.SetIs3D(true);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(300*keV, G4ThreeVector(0, 0, Z*cm), G4ThreeVector(0, 0, Z*cm), fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", string("3D, z = ") + to_string(Z), "Top and bottom ADC counts add up to the energy", Hits[0]->GetADCCountsTop() + Hits[0]->GetADCCountsBottom(), 300.0, 1e-6) && Passed;
      Passed = EvaluateTrue("PostProcessHits()", string("3D, z = ") + to_string(Z), "Both ADC values are not negative", Hits[0]->GetADCCountsTop() >= 0.0 && Hits[0]->GetADCCountsBottom() >= 0.0) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", string("3D, z = ") + to_string(Z), "One hit is created", Hits.size(), 1) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarSD::TestMerging()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());

  // Same bar: merged, other bar in x or y: not merged:
  {
    MCCalorBarSDAccess SD("MergeSD");
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(0.2*cm, 0.2*cm, 0), G4ThreeVector(0.2*cm, 0.2*cm, 0), fPostStepDoItProc, 1, 0.0, Volume));
    SD.PostProcessHits(F.CreateStep(250*keV, G4ThreeVector(0.4*cm, 0.6*cm, 1*cm), G4ThreeVector(0.4*cm, 0.6*cm, 1*cm), fPostStepDoItProc, 2, 0.0, Volume));
    SD.PostProcessHits(F.CreateStep(50*keV, G4ThreeVector(1.4*cm, 0.2*cm, 0), G4ThreeVector(1.4*cm, 0.2*cm, 0), fPostStepDoItProc, 3, 0.0, Volume));
    SD.PostProcessHits(F.CreateStep(70*keV, G4ThreeVector(0.2*cm, -1.4*cm, 0), G4ThreeVector(0.2*cm, -1.4*cm, 0), fPostStepDoItProc, 4, 0.0, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", "3 bars", "Three different bars give three hits", Hits.size(), 3) && Passed;
    if (Hits.size() == 3) {
      Passed = EvaluateNear("PostProcessHits()", "same bar", "The energies in the same bar are added", Hits[0]->GetEnergy()/keV, 350.0, 1e-9) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "same bar", "The ADC counts in the same bar are added", Hits[0]->GetADCCountsTop(), 350.0, 1e-9) && Passed;
      Passed = EvaluateSize("PostProcessHits()", "same bar", "The origins are kept", Hits[0]->m_Origins.size(), 2) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "other bar", "The hit in the next x bar is separate", Hits[1]->GetEnergy()/keV, 50.0, 1e-9) && Passed;
    }
    delete HCE;
  }

  // Without discretization nothing is merged:
  {
    MCCalorBarSDAccess SD("NoMergeSD");
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(0.2*cm, 0.2*cm, 0), G4ThreeVector(0.2*cm, 0.2*cm, 0), fPostStepDoItProc, 1, 0.0, Volume));
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(0.2*cm, 0.2*cm, 0), G4ThreeVector(0.2*cm, 0.2*cm, 0), fPostStepDoItProc, 2, 0.0, Volume));
    Passed = EvaluateSize("PostProcessHits()", "no discretization", "Without discretization hits are not merged", GetHits(F.EndEvent(SD, HCE)).size(), 2) && Passed;
    delete HCE;
  }

  // Same bar in different detector volumes is not merged (full volume history is compared):
  {
    UTSDStepFixture G;
    int Panel1 = G.AddVolume("Panel1", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector(20*cm, 0, 0));
    int Panel2 = G.AddPlacement("Panel2", Panel1, G4ThreeVector(-20*cm, 0, 0), nullptr, 0);
    MCCalorBarSDAccess SD("TwoPanelSD");
    G4HCofThisEvent* HCE = G.BeginEvent(SD);
    SD.PostProcessHits(G.CreateStep(100*keV, G4ThreeVector(20.2*cm, 0.2*cm, 0), G4ThreeVector(20.2*cm, 0.2*cm, 0), fPostStepDoItProc, 1, 0.0, Panel1));
    SD.PostProcessHits(G.CreateStep(100*keV, G4ThreeVector(-19.8*cm, 0.2*cm, 0), G4ThreeVector(-19.8*cm, 0.2*cm, 0), fPostStepDoItProc, 2, 0.0, Panel2));
    Passed = EvaluateSize("PostProcessHits()", "two detector volumes", "The same bar in two detector volumes gives two hits", GetHits(G.EndEvent(SD, HCE)).size(), 2) && Passed;
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarSD::TestEnergyLossMap()
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
    MCCalorBarSDAccess SD("LossSD");
    SD.SetEnergyLossMap(Map);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(200*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetEnergyLossMap()", "0.5", "The energy is reduced", Hits[0]->GetEnergy()/keV, 100.0, 1e-6) && Passed;
      Passed = EvaluateNear("SetEnergyLossMap()", "0.5", "The ADC counts follow the reduced energy", Hits[0]->GetADCCountsTop(), 100.0, 1e-6) && Passed;
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
    MCCalorBarSDAccess SD("ZeroLossSD");
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


bool UTCalorBarSD::TestTimeResolution()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
  for (bool Resolution: { false, true }) {
    MCCalorBarSDAccess SD(string("TimeSD") + to_string(Resolution));
    SD.SetHasTimeResolution(Resolution);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 12.5*ns, Volume));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
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


bool UTCalorBarSD::TestTrackInformation()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
  MCCalorBarSDAccess SD("DigitizedSD");
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


bool UTCalorBarSD::TestRotatedVolumes()
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
    MCCalorBarSDAccess SD(string("RotatedSD") + to_string(p));
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Positions[p], Positions[p], fPostStepDoItProc, 1, 0.0, Bar));
    vector<MCCalorBarHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
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


bool UTCalorBarSD::TestNeverTriggering()
{
  bool Passed = true;

  bool Works = UTSDHitShared::RunsCleanlyInChild([]() {
    UTSDStepFixture F;
    F.SetupEventAction();
    int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 2*cm), G4ThreeVector());
    MCCalorBarSDAccess SD("NeverSD");
    SD.SetIsNeverTriggering(true);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    bool Result = SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    MCCalorBarHitsCollection* HC = static_cast<MCCalorBarHitsCollection*>(F.EndEvent(SD, HCE));
    return Result == true && HC != nullptr && HC->entries() == 1;
  });
  Passed = EvaluateTrue("SetIsNeverTriggering()", "with event action", "A never triggering detector still creates its hit", Works) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCalorBarSD Test;
  return Test.Run() == true ? 0 : 1;
}
