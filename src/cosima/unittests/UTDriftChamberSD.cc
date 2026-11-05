/*
 * UTDriftChamberSD.cc
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
#include "MCDriftChamberSD.hh"
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
#include <cstring>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <vector>
using namespace std;


//! Class exposing the protected members of the hit to the unit test
class MCDriftChamberHitAccess : public MCDriftChamberHit
{
public:
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Sensitive detector exposing the protected members to the unit test
//! The default geometry: wafer half size 4 cm, offset 0.5 cm, pitch 1 cm, 7 strips in x and y, one wafer per detector volume
class MCDriftChamberSDAccess : public MCDriftChamberSD
{
public:
  //! Default constructor without any geometry
  MCDriftChamberSDAccess(G4String Name) : MCDriftChamberSD(Name) {}
  //! Set the standard geometry
  void SetStandardGeometry()
  {
    SetDetectorVolumeName("PanelLog");
    SetSize(4.0*cm, 4.0*cm);
    SetOffset(0.5*cm, 0.5*cm);
    SetPitch(1.0*cm, 1.0*cm);
    SetNStrips(7, 7);
    SetLightSpeed(10.0*cm/ns);
    SetLightDetectorPosition(0);
    SetDriftConstant(0.01);
    SetEnergyPerElectron(3.6*eV);
    SetDetectorStructuralSize(G4ThreeVector(8.0*cm, 8.0*cm, 1.0*cm));
    SetDetectorStructuralPitch(G4ThreeVector(0.0, 0.0, 0.0));
    SetDetectorStructuralOffset(G4ThreeVector(0.0, 0.0, 0.0));
    SetDetectorStructuralDimension(G4ThreeVector(4.0*cm, 4.0*cm, 0.5*cm));
  }
  using MCSD::m_DiscretizeHits;
  using MCSD::m_Type;
  using MC2DStripSD::m_XSize;
  using MC2DStripSD::m_XOffset;
  using MC2DStripSD::m_XPitch;
  using MC2DStripSD::m_XNStrips;
  using MC2DStripSD::m_YSize;
  using MC2DStripSD::m_YOffset;
  using MC2DStripSD::m_YPitch;
  using MC2DStripSD::m_YNStrips;
  using MC2DStripSD::m_Is3D;
};


//! Unit test class for MCDriftChamberSD
class UTDriftChamberSD : public MUnitTest
{
public:
  //! Default constructor
  UTDriftChamberSD() : MUnitTest("UTDriftChamberSD") {}
  //! Default destructor
  virtual ~UTDriftChamberSD() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructor
  bool TestConstruction();
  //! Test that the constructor initializes all geometry members
  bool TestUninitializedMembers();
  //! Test SetIs3D
  bool TestSetIs3D();
  //! Test the event cycle
  bool TestEventCycle();
  //! Test a simple hit and the strips
  bool TestSimpleHit();
  //! Test the strip numbers
  bool TestStrips();
  //! Test steps without energy deposit
  bool TestZeroEnergy();
  //! Test the position of the hit for the different step statuses
  bool TestStepStatusPosition();
  //! Test the discretization
  bool TestDiscretization();
  //! Test the guard ring
  bool TestGuardRing();
  //! Test the merging of hits
  bool TestMerging();
  //! Test the strips of multiple detector volumes
  bool TestDetectorStrips();
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
  vector<MCDriftChamberHitAccess*> GetHits(G4VHitsCollection* Collection);
  //! Create one hit at the given position (world coordinates = frame of the panel at the origin) and return it, the event is ended
  vector<MCDriftChamberHitAccess*> Shoot(UTSDStepFixture& F, MCDriftChamberSDAccess& SD, int Volume, const G4ThreeVector& Position, double Energy = 100*keV);
};


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestUninitializedMembers() && Passed;
  Passed = TestSetIs3D() && Passed;
  Passed = TestEventCycle() && Passed;
  Passed = TestSimpleHit() && Passed;
  Passed = TestStrips() && Passed;
  Passed = TestZeroEnergy() && Passed;
  Passed = TestStepStatusPosition() && Passed;
  Passed = TestDiscretization() && Passed;
  Passed = TestGuardRing() && Passed;
  Passed = TestMerging() && Passed;
  Passed = TestDetectorStrips() && Passed;
  Passed = TestEnergyLossMap() && Passed;
  Passed = TestTimeResolution() && Passed;
  Passed = TestTrackInformation() && Passed;
  Passed = TestRotatedVolumes() && Passed;
  Passed = TestNeverTriggering() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


vector<MCDriftChamberHitAccess*> UTDriftChamberSD::GetHits(G4VHitsCollection* Collection)
{
  vector<MCDriftChamberHitAccess*> Hits;
  MCDriftChamberHitsCollection* HC = static_cast<MCDriftChamberHitsCollection*>(Collection);
  if (HC == nullptr) return Hits;
  for (size_t h = 0; h < HC->entries(); ++h) {
    Hits.push_back(static_cast<MCDriftChamberHitAccess*>((*HC)[h]));
  }
  return Hits;
}


////////////////////////////////////////////////////////////////////////////////


vector<MCDriftChamberHitAccess*> UTDriftChamberSD::Shoot(UTSDStepFixture& F, MCDriftChamberSDAccess& SD, int Volume, const G4ThreeVector& Position, double Energy)
{
  // The hits stay valid since the event is deliberately not deleted - the memory is released at the end of the process
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  SD.PostProcessHits(F.CreateStep(Energy, Position, Position, fPostStepDoItProc, 1, 0.0, Volume));
  return GetHits(F.EndEvent(SD, HCE));
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestConstruction()
{
  bool Passed = true;

  MCDriftChamberSDAccess SD("Strip");
  Passed = Evaluate("MCDriftChamberSD()", "name", "The name of the detector is the one given", MString(SD.GetName().c_str()), MString("Strip")) && Passed;
  Passed = Evaluate("MCDriftChamberSD()", "type", "The type of the detector is drift chamber", SD.GetType(), MCSD::c_DriftChamber) && Passed;
  // Known failure: the MCDriftChamberSD constructor inserts the collection name again although the MC2DStripSD constructor already did
  Passed = ExpectedToFail(Evaluate("MCDriftChamberSD()", "collections", "The detector has one hit collection", SD.GetNumberOfCollections(), 1)) && Passed;
  Passed = Evaluate("MCDriftChamberSD()", "collection name", "The name of the collection is the name of the detector", MString(SD.GetCollectionName(0).c_str()), MString("Strip")) && Passed;
  Passed = EvaluateTrue("MCDriftChamberSD()", "default", "Hits are discretized by default", SD.m_DiscretizeHits) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestUninitializedMembers()
{
  bool Passed = true;

  // Construct the detector in memory filled with 0x00 and with 0xFF - the members which are not initialized differ
  alignas(MCDriftChamberSDAccess) static unsigned char Zero[sizeof(MCDriftChamberSDAccess)];
  alignas(MCDriftChamberSDAccess) static unsigned char Full[sizeof(MCDriftChamberSDAccess)];
  memset(Zero, 0x00, sizeof(Zero));
  memset(Full, 0xFF, sizeof(Full));
  MCDriftChamberSDAccess* A = ::new (Zero) MCDriftChamberSDAccess("UninitA");
  MCDriftChamberSDAccess* B = ::new (Full) MCDriftChamberSDAccess("UninitB");

  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_XSize
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The x size is initialized", memcmp(&A->m_XSize, &B->m_XSize, sizeof(double)) == 0)) && Passed;
  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_XOffset
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The x offset is initialized", memcmp(&A->m_XOffset, &B->m_XOffset, sizeof(double)) == 0)) && Passed;
  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_XPitch
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The x pitch is initialized", memcmp(&A->m_XPitch, &B->m_XPitch, sizeof(double)) == 0)) && Passed;
  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_XNStrips
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The number of x strips is initialized", A->m_XNStrips == B->m_XNStrips)) && Passed;
  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_YSize
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The y size is initialized", memcmp(&A->m_YSize, &B->m_YSize, sizeof(double)) == 0)) && Passed;
  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_YOffset
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The y offset is initialized", memcmp(&A->m_YOffset, &B->m_YOffset, sizeof(double)) == 0)) && Passed;
  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_YPitch
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The y pitch is initialized", memcmp(&A->m_YPitch, &B->m_YPitch, sizeof(double)) == 0)) && Passed;
  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_YNStrips
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The number of y strips is initialized", A->m_YNStrips == B->m_YNStrips)) && Passed;
  // Known failure: neither the MC2DStripSD nor the MCDriftChamberSD constructor initializes m_Is3D
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberSD()", "default", "The 3D flag is initialized", memcmp(&A->m_Is3D, &B->m_Is3D, sizeof(bool)) == 0)) && Passed;

  // Leak the objects on purpose: their destructors would only free Geant4 vectors which are not needed
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestSetIs3D()
{
  bool Passed = true;

  // MCDriftChamberSD::SetIs3D is declared but not defined (see findings), the inherited version is used through the base class
  // Only the effect on the z-resolution is checked here:
  MCDriftChamberSDAccess SD("ThreeD");
  static_cast<MC2DStripSD&>(SD).SetIs3D(true);
  Passed = EvaluateTrue("SetIs3D()", "true", "The z-resolution flag is set", SD.m_Is3D) && Passed;
  static_cast<MC2DStripSD&>(SD).SetIs3D(false);
  Passed = EvaluateFalse("SetIs3D()", "false", "The z-resolution flag is removed", SD.m_Is3D) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestEventCycle()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());
  MCDriftChamberSDAccess SD("CycleSD");
  SD.SetStandardGeometry();
  SD.m_Is3D = false;
  G4HCofThisEvent* First = F.BeginEvent(SD);
  SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
  G4VHitsCollection* FirstCollection = F.EndEvent(SD, First);
  G4HCofThisEvent* Second = F.BeginEvent(SD);
  G4VHitsCollection* SecondCollection = F.EndEvent(SD, Second);
  Passed = EvaluateTrue("EndOfEvent()", "collection", "The hit collection is handed to the event", FirstCollection != nullptr && SecondCollection != nullptr) && Passed;
  Passed = EvaluateSize("Initialize()", "first event", "The first event has one hit", GetHits(FirstCollection).size(), 1) && Passed;
  Passed = EvaluateSize("Initialize()", "second event", "The second event starts without hits", GetHits(SecondCollection).size(), 0) && Passed;

  G4HCofThisEvent* Third = F.BeginEvent(SD);
  Passed = EvaluateTrue("ProcessHits()", "any step", "ProcessHits accepts the step", SD.ProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume), nullptr)) && Passed;
  Passed = EvaluateSize("ProcessHits()", "any step", "ProcessHits does not create hits", GetHits(F.EndEvent(SD, Third)).size(), 0) && Passed;
  delete First;
  delete Second;
  delete Third;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestSimpleHit()
{
  bool Passed = true;

  UTSDStepFixture F;
  F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector(20*cm, 0, 0));
  int Wafer = F.AddVolume("Wafer", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());

  MCDriftChamberSDAccess SD("SimpleSD");
  SD.SetStandardGeometry();
  SD.m_Is3D = false;
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4Step* Step = F.CreateStep(662*keV, G4ThreeVector(21.4*cm, -2.2*cm, 0.3*cm), G4ThreeVector(21.6*cm, -2.1*cm, 0.2*cm), fPostStepDoItProc, 17, 0.0, Wafer);
  Passed = EvaluateTrue("PostProcessHits()", "step with energy", "A step with energy is processed", SD.PostProcessHits(Step)) && Passed;
  vector<MCDriftChamberHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
  Passed = EvaluateSize("PostProcessHits()", "step with energy", "One hit is created", Hits.size(), 1) && Passed;
  if (Hits.size() == 1) {
    MCDriftChamberHitAccess* Hit = Hits[0];
    Passed = EvaluateNear("PostProcessHits()", "662 keV", "The hit has the deposited energy", Hit->GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "662 keV", "The ADC counts are the energy in keV", Hit->GetADCCounts(), 662.0, 1e-9) && Passed;
    Passed = Evaluate("PostProcessHits()", "detector", "The hit knows the detector volume", MString(Hit->GetDetectorName().c_str()), MString("Panel")) && Passed;
    Passed = EvaluateTrue("PostProcessHits()", "track 17", "The origin of the hit is the ID of the track", Hit->m_Origins.count(17) == 1) && Passed;
    // Local x = 1.6 cm -> (1.6 + 4 - 0.5)/1 = 5.1 -> strip 5, local y = -2.1 cm -> (-2.1 + 3.5) = 1.4 -> strip 1
    Passed = Evaluate("PostProcessHits()", "x = 1.6 cm", "The x strip is 5", Hit->GetXStrip(), 5) && Passed;
    Passed = Evaluate("PostProcessHits()", "y = -2.1 cm", "The y strip is 1", Hit->GetYStrip(), 1) && Passed;
    Passed = EvaluateFalse("PostProcessHits()", "sensitive", "The hit is not in the guard ring", Hit->GetIsGuardringHit()) && Passed;
    // Strip centers: x = -4 + 0.5 + 5.5 = 2.0, y = -4 + 0.5 + 1.5 = -2.0
    Passed = EvaluateNear("PostProcessHits()", "strip 5", "The discretized x position is the strip center", Hit->GetPosition().x()/cm, 22.0, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "strip 1", "The discretized y position is the strip center", Hit->GetPosition().y()/cm, -2.0, 1e-6) && Passed;
    Passed = EvaluateNear("PostProcessHits()", "2D", "The z position is the center plane", Hit->GetPosition().z()/cm, 0.0, 1e-6) && Passed;
  }
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestStrips()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());

  struct Case { double X; int Strip; };
  vector<Case> Cases = { { -3.4, 0 }, { -2.6, 0 }, { -2.4, 1 }, { 0.0, 3 }, { 2.9, 6 }, { 3.4, 6 } };
  for (const Case& C: Cases) {
    MCDriftChamberSDAccess SD("StripSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    vector<MCDriftChamberHitAccess*> Hits = Shoot(F, SD, Panel, G4ThreeVector(C.X*cm, 0.0, 0.0));
    if (Hits.size() == 1) {
      Passed = Evaluate("PostProcessHits()", string("x = ") + to_string(C.X) + " cm", "The x strip is the one containing the position", Hits[0]->GetXStrip(), C.Strip) && Passed;
      Passed = Evaluate("PostProcessHits()", string("x = ") + to_string(C.X) + " cm", "The center of the strip is between the strip edges", (int) floor(Hits[0]->GetPosition().x()/cm + 3.5), C.Strip) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", string("x = ") + to_string(C.X) + " cm", "One hit is created", Hits.size(), 1) && Passed;
    }
  }

  // A single strip in a direction is always strip 0:
  {
    MCDriftChamberSDAccess SD("OneStripSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    SD.SetNStrips(1, 7);
    vector<MCDriftChamberHitAccess*> Hits = Shoot(F, SD, Panel, G4ThreeVector(2.0*cm, 0.0, 0.0));
    if (Hits.size() == 1) {
      Passed = Evaluate("PostProcessHits()", "one strip", "With one strip the strip number is 0", Hits[0]->GetXStrip(), 0) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", "one strip", "One hit is created", Hits.size(), 1) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestZeroEnergy()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());
  MCDriftChamberSDAccess SD("ZeroSD");
  SD.SetStandardGeometry();
  SD.m_Is3D = false;
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  Passed = EvaluateFalse("PostProcessHits()", "zero energy", "A step without energy deposit is not processed", SD.PostProcessHits(F.CreateStep(0.0, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume))) && Passed;
  Passed = EvaluateSize("PostProcessHits()", "zero energy", "A step without energy deposit creates no hit", GetHits(F.EndEvent(SD, HCE)).size(), 0) && Passed;
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestStepStatusPosition()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector(20*cm, 0, 0));
  G4ThreeVector Pre(20.5*cm, 1.0*cm, 0.2*cm);
  G4ThreeVector Post(21.5*cm, 3.0*cm, 0.4*cm);

  struct Case { G4StepStatus Status; G4ThreeVector Expected; const char* Name; };
  vector<Case> Cases = {
    { fPostStepDoItProc, Post, "post step process" },
    { fGeomBoundary, Post, "geometry boundary" },
    { fAlongStepDoItProc, 0.5*(Pre + Post), "along step process" },
    { fUndefined, Pre, "undefined status" }
  };
  for (const Case& C: Cases) {
    MCDriftChamberSDAccess SD(string("StatusSD_") + C.Name);
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    SD.m_DiscretizeHits = false;
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, Pre, Post, C.Status, 1, 0.0, Volume));
    vector<MCDriftChamberHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", C.Name, "One hit is created", Hits.size(), 1) && Passed;
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", C.Name, "The position depends on the step status", (Hits[0]->GetPosition() - C.Expected).mag()/mm, 0.0, 1e-6) && Passed;
    }
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestDiscretization()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector(20*cm, 0, 0));
  G4ThreeVector Step(21.4*cm, -2.2*cm, 0.3*cm);

  // 3D: the depth is kept:
  {
    MCDriftChamberSDAccess SD("Disc3DSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = true;
    vector<MCDriftChamberHitAccess*> Hits = Shoot(F, SD, Panel, Step);
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetIs3D()", "true", "The z position is kept", Hits[0]->GetPosition().z()/cm, 0.3, 1e-6) && Passed;
      Passed = EvaluateNear("SetIs3D()", "true", "The x position is discretized", Hits[0]->GetPosition().x()/cm, 21.0, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("SetIs3D()", "true", "One hit is created", Hits.size(), 1) && Passed;
    }
  }

  // Without discretization the position is kept:
  {
    MCDriftChamberSDAccess SD("NoDiscSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    SD.SetDiscretizeHits(false);
    vector<MCDriftChamberHitAccess*> Hits = Shoot(F, SD, Panel, Step);
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetDiscretizeHits()", "false", "The position is kept", (Hits[0]->GetPosition() - Step).mag()/mm, 0.0, 1e-6) && Passed;
      Passed = Evaluate("SetDiscretizeHits()", "false", "The x strip is still determined", Hits[0]->GetXStrip(), 4) && Passed;
    } else {
      Passed = EvaluateSize("SetDiscretizeHits()", "false", "One hit is created", Hits.size(), 1) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestGuardRing()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector(20*cm, 0, 0));

  // The drift chamber has no guard ring hits: everything between 3.5 and 4 cm and outside is ignored (the SD may report it, keep the output clean)
  vector<G4ThreeVector> Ignored = {
    G4ThreeVector(23.7*cm, 1.0*cm, 0.2*cm),
    G4ThreeVector(20.0*cm, -3.6*cm, 0.0),
    G4ThreeVector(24.5*cm, 0.0, 0.0)
  };
  for (unsigned int i = 0; i < Ignored.size(); ++i) {
    MCDriftChamberSDAccess SD(string("IgnoreSD") + to_string(i));
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    ostringstream Silent;
    streambuf* OldOut = cout.rdbuf(Silent.rdbuf());
    streambuf* OldErr = cerr.rdbuf(Silent.rdbuf());
    bool Result = SD.PostProcessHits(F.CreateStep(100*keV, Ignored[i], Ignored[i], fPostStepDoItProc, 1, 0.0, Panel));
    cout.rdbuf(OldOut);
    cerr.rdbuf(OldErr);
    Passed = EvaluateFalse("PostProcessHits()", string("outside of the sensitive area ") + to_string(i), "A step outside of the sensitive area is not processed", Result) && Passed;
    Passed = EvaluateSize("PostProcessHits()", string("outside of the sensitive area ") + to_string(i), "A step outside of the sensitive area creates no hit", GetHits(F.EndEvent(SD, HCE)).size(), 0) && Passed;
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestMerging()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());

  {
    MCDriftChamberSDAccess SD("MergeSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    // Same strips (5, 1):
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(1.6*cm, -2.2*cm, 0), G4ThreeVector(1.6*cm, -2.2*cm, 0), fPostStepDoItProc, 1, 0.0, Panel));
    SD.PostProcessHits(F.CreateStep(250*keV, G4ThreeVector(1.9*cm, -1.6*cm, 0.3*cm), G4ThreeVector(1.9*cm, -1.6*cm, 0.3*cm), fPostStepDoItProc, 2, 0.0, Panel));
    // Other x strip and other y strip:
    SD.PostProcessHits(F.CreateStep(50*keV, G4ThreeVector(0.6*cm, -2.2*cm, 0), G4ThreeVector(0.6*cm, -2.2*cm, 0), fPostStepDoItProc, 3, 0.0, Panel));
    SD.PostProcessHits(F.CreateStep(70*keV, G4ThreeVector(1.6*cm, 0.2*cm, 0), G4ThreeVector(1.6*cm, 0.2*cm, 0), fPostStepDoItProc, 4, 0.0, Panel));
    vector<MCDriftChamberHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
    Passed = EvaluateSize("PostProcessHits()", "3 strip pairs", "Three different strip pairs give three hits", Hits.size(), 3) && Passed;
    if (Hits.size() == 3) {
      Passed = EvaluateNear("PostProcessHits()", "same strips", "The energies in the same strips are added", Hits[0]->GetEnergy()/keV, 350.0, 1e-9) && Passed;
      Passed = EvaluateNear("PostProcessHits()", "same strips", "The ADC counts in the same strips are added", Hits[0]->GetADCCounts(), 350.0, 1e-9) && Passed;
      Passed = EvaluateSize("PostProcessHits()", "same strips", "The origins are kept", Hits[0]->m_Origins.size(), 2) && Passed;
    }
    delete HCE;
  }

  {
    MCDriftChamberSDAccess SD("NoMergeSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    SD.SetDiscretizeHits(false);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(1.6*cm, -2.2*cm, 0), G4ThreeVector(1.6*cm, -2.2*cm, 0), fPostStepDoItProc, 1, 0.0, Panel));
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(1.6*cm, -2.2*cm, 0), G4ThreeVector(1.6*cm, -2.2*cm, 0), fPostStepDoItProc, 2, 0.0, Panel));
    Passed = EvaluateSize("PostProcessHits()", "no discretization", "Without discretization hits are not merged", GetHits(F.EndEvent(SD, HCE)).size(), 2) && Passed;
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestDetectorStrips()
{
  bool Passed = true;

  // Two wafers side by side in one detector volume: local frame is the wafer
  // The detector consists of 3 x 2 wafers of 8 x 8 cm, the detector volume is 24 x 16 cm large with its center at the origin
  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(12*cm, 8*cm, 0.5*cm), G4ThreeVector());
  int Wafer00 = F.AddVolume("Wafer00", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector(-8*cm, -4*cm, 0), nullptr, Panel);
  int Wafer21 = F.AddVolume("Wafer21", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector(8*cm, 4*cm, 0), nullptr, Panel);

  MCDriftChamberSDAccess SD("WaferSD");
  SD.SetStandardGeometry();
  SD.m_Is3D = false;
  SD.SetDetectorStructuralSize(G4ThreeVector(8.0*cm, 8.0*cm, 1.0*cm));
  SD.SetDetectorStructuralDimension(G4ThreeVector(12.0*cm, 8.0*cm, 0.5*cm));
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(-8.0*cm, -4.0*cm, 0), G4ThreeVector(-8.0*cm, -4.0*cm, 0), fPostStepDoItProc, 1, 0.0, Wafer00));
  SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(8.0*cm, 4.0*cm, 0), G4ThreeVector(8.0*cm, 4.0*cm, 0), fPostStepDoItProc, 2, 0.0, Wafer21));
  vector<MCDriftChamberHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
  Passed = EvaluateSize("PostProcessHits()", "two wafers", "Two wafers give two hits", Hits.size(), 2) && Passed;
  if (Hits.size() == 2) {
    // Wafer (0, 0): strips 3, 3; wafer (2, 1): 2*7 + 3 = 17, 1*7 + 3 = 10
    Passed = Evaluate("PostProcessHits()", "wafer 0, 0", "The x strip of the first wafer is the local one", Hits[0]->GetXStrip(), 3) && Passed;
    Passed = Evaluate("PostProcessHits()", "wafer 0, 0", "The y strip of the first wafer is the local one", Hits[0]->GetYStrip(), 3) && Passed;
    Passed = Evaluate("PostProcessHits()", "wafer 2, 1", "The x strip continues after the strips of the wafers before", Hits[1]->GetXStrip(), 17) && Passed;
    Passed = Evaluate("PostProcessHits()", "wafer 2, 1", "The y strip continues after the strips of the wafers before", Hits[1]->GetYStrip(), 10) && Passed;
  }
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestEnergyLossMap()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());

  vector<double> X = { -10.0, 10.0 };
  {
    MFunction3D Map;
    Map.Set(X, X, X, vector<double>(8, 0.5));
    MCDriftChamberSDAccess SD("LossSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    SD.SetEnergyLossMap(Map);
    vector<MCDriftChamberHitAccess*> Hits = Shoot(F, SD, Panel, G4ThreeVector(), 200*keV);
    if (Hits.size() == 1) {
      Passed = EvaluateNear("SetEnergyLossMap()", "0.5", "The energy is reduced", Hits[0]->GetEnergy()/keV, 100.0, 1e-6) && Passed;
      Passed = EvaluateNear("SetEnergyLossMap()", "0.5", "The ADC counts follow the reduced energy", Hits[0]->GetADCCounts(), 100.0, 1e-6) && Passed;
    } else {
      Passed = EvaluateSize("SetEnergyLossMap()", "0.5", "One hit is created", Hits.size(), 1) && Passed;
    }
  }

  {
    MFunction3D ZeroMap;
    ZeroMap.Set(X, X, X, vector<double>(8, 0.0));
    MCDriftChamberSDAccess SD("ZeroLossSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    SD.SetEnergyLossMap(ZeroMap);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    ostringstream Silent;
    streambuf* Old = cout.rdbuf(Silent.rdbuf());
    bool Result = SD.PostProcessHits(F.CreateStep(200*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Panel));
    cout.rdbuf(Old);
    Passed = EvaluateFalse("SetEnergyLossMap()", "0.0", "A zero energy after the loss is not processed", Result) && Passed;
    Passed = EvaluateSize("SetEnergyLossMap()", "0.0", "No hit is created", GetHits(F.EndEvent(SD, HCE)).size(), 0) && Passed;
    delete HCE;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestTimeResolution()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());
  for (bool Resolution: { false, true }) {
    MCDriftChamberSDAccess SD(string("TimeSD") + to_string(Resolution));
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    SD.SetHasTimeResolution(Resolution);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 12.5*ns, Panel));
    vector<MCDriftChamberHitAccess*> Hits = GetHits(F.EndEvent(SD, HCE));
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


bool UTDriftChamberSD::TestTrackInformation()
{
  bool Passed = true;

  UTSDStepFixture F;
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());
  MCDriftChamberSDAccess SD("DigitizedSD");
  SD.SetStandardGeometry();
  SD.m_Is3D = false;
  G4HCofThisEvent* HCE = F.BeginEvent(SD);
  G4Step* With = F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Panel);
  SD.PostProcessHits(With);
  Passed = EvaluateTrue("PostProcessHits()", "energy", "A track with a hit is marked as digitized", F.GetTrackInformation(With)->IsDigitized()) && Passed;
  G4Step* Without = F.CreateStep(0.0, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 2, 0.0, Panel);
  SD.PostProcessHits(Without);
  Passed = EvaluateFalse("PostProcessHits()", "no energy", "A track without energy is not marked as digitized", F.GetTrackInformation(Without)->IsDigitized()) && Passed;
  G4Step* Merged = F.CreateStep(50*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 3, 0.0, Panel);
  SD.PostProcessHits(Merged);
  Passed = EvaluateTrue("PostProcessHits()", "merged", "A track with a merged hit is marked as digitized", F.GetTrackInformation(Merged)->IsDigitized()) && Passed;
  F.EndEvent(SD, HCE);
  delete HCE;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestRotatedVolumes()
{
  bool Passed = true;

  UTSDStepFixture F;
  G4RotationMatrix* PanelRotation = new G4RotationMatrix();
  PanelRotation->rotateZ(30.0*deg);
  PanelRotation->rotateX(20.0*deg);
  int Panel = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector(20*cm, 5*cm, -3*cm), PanelRotation);
  G4RotationMatrix* WaferRotation = new G4RotationMatrix();
  WaferRotation->rotateY(-40.0*deg);
  int Wafer = F.AddVolume("Wafer", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector(0.0, 0.0, 0.0), WaferRotation, Panel);

  // Positions inside the sensitive area of the wafer, expressed in the wafer system and transformed to world:
  vector<G4ThreeVector> Local = { G4ThreeVector(1.3*cm, 0.6*cm, 0.2*cm), G4ThreeVector(-2.0*cm, 1.4*cm, -0.3*cm) };
  for (unsigned int p = 0; p < Local.size(); ++p) {
    // Frame rotations are the passive ones: world = inverse(rot) * local + translation for one level:
    G4ThreeVector InPanel = WaferRotation->inverse() * Local[p];
    G4ThreeVector World = PanelRotation->inverse() * InPanel + G4ThreeVector(20*cm, 5*cm, -3*cm);
    MCDriftChamberSDAccess SD(string("RotatedSD") + to_string(p));
    SD.SetStandardGeometry();
    SD.m_Is3D = true;
    SD.SetDiscretizeHits(false);
    vector<MCDriftChamberHitAccess*> Hits = Shoot(F, SD, Wafer, World);
    if (Hits.size() == 1) {
      Passed = EvaluateNear("PostProcessHits()", string("rotated, position ") + to_string(p), "The position is the same after the transformation into the detector frame and back", (Hits[0]->GetPosition() - World).mag()/mm, 0.0, 1e-6) && Passed;
      // x strip from the local x, y strip from the local y in the wafer frame:
      Passed = Evaluate("PostProcessHits()", string("rotated, position ") + to_string(p), "The x strip is determined in the wafer frame", Hits[0]->GetXStrip(), (int) floor(Local[p].x()/cm + 3.5)) && Passed;
    } else {
      Passed = EvaluateSize("PostProcessHits()", string("rotated, position ") + to_string(p), "One hit is created", Hits.size(), 1) && Passed;
    }
  }
  (void) Panel;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberSD::TestNeverTriggering()
{
  bool Passed = true;

  bool Works = UTSDHitShared::RunsCleanlyInChild([]() {
    UTSDStepFixture F;
    F.SetupEventAction();
    int Volume = F.AddVolume("Panel", G4ThreeVector(4*cm, 4*cm, 0.5*cm), G4ThreeVector());
    MCDriftChamberSDAccess SD("NeverSD");
    SD.SetStandardGeometry();
    SD.m_Is3D = false;
    SD.SetIsNeverTriggering(true);
    G4HCofThisEvent* HCE = F.BeginEvent(SD);
    bool Result = SD.PostProcessHits(F.CreateStep(100*keV, G4ThreeVector(), G4ThreeVector(), fPostStepDoItProc, 1, 0.0, Volume));
    MCDriftChamberHitsCollection* HC = static_cast<MCDriftChamberHitsCollection*>(F.EndEvent(SD, HCE));
    return Result == true && HC != nullptr && HC->entries() == 1;
  });
  Passed = EvaluateTrue("SetIsNeverTriggering()", "with event action", "A never triggering detector still creates its hit", Works) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDriftChamberSD Test;
  return Test.Run() == true ? 0 : 1;
}
