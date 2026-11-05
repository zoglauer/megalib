/*
 * UTScintillatorHit.cc
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
#include "MCScintillatorHit.hh"
#include "UTSDHitShared.h"

// MEGAlib:
#include "MDDetector.h"
#include "MSimHT.h"
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <iostream>
#include <set>
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
  using MCVHit::m_DetectorType;
};


//! Unit test class for MCScintillatorHit
class UTScintillatorHit : public MUnitTest
{
public:
  //! Default constructor
  UTScintillatorHit() : MUnitTest("UTScintillatorHit") {}
  //! Default destructor
  virtual ~UTScintillatorHit() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default constructor
  bool TestDefaultConstruction();
  //! Test the setters and getters
  bool TestGettersSetters();
  //! Test the copy constructor
  bool TestCopyConstructor();
  //! Test the assignment operator
  bool TestAssignment();
  //! Test the equal operator
  bool TestEqualOperator();
  //! Test the addition operator
  bool TestAddOperator();
  //! Test the Geant4 allocator and the hit collection
  bool TestAllocationAndCollection();
  //! Test the conversion into a calibrated hit
  bool TestGetCalibrated();
  //! Test the output
  bool TestPrint();

  //! Create a hit with a representative content and the given volume history
  void Fill(MCScintillatorHitAccess& Hit, double Energy, double ADC, const G4ThreeVector& Position, double Time, unsigned int Origin, const vector<string>& History);
};


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestCopyConstructor() && Passed;
  Passed = TestAssignment() && Passed;
  Passed = TestEqualOperator() && Passed;
  Passed = TestAddOperator() && Passed;
  Passed = TestAllocationAndCollection() && Passed;
  Passed = TestGetCalibrated() && Passed;
  Passed = TestPrint() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTScintillatorHit::Fill(MCScintillatorHitAccess& Hit, double Energy, double ADC, const G4ThreeVector& Position, double Time, unsigned int Origin, const vector<string>& History)
{
  Hit.SetEnergy(Energy);
  Hit.SetADCCounts(ADC);
  Hit.SetPosition(Position);
  Hit.SetTime(Time);
  Hit.SetDetectorName("ACS_Panel");
  Hit.AddOrigin(Origin);
  for (const string& Name: History) {
    Hit.AddVolumeHistory(Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestDefaultConstruction()
{
  bool Passed = true;

  MCScintillatorHitAccess Hit;
  Passed = Evaluate("MCScintillatorHit()", "default", "The detector type is scintillator", Hit.GetDetectorType(), (int) MDDetector::c_Scintillator) && Passed;
  Passed = EvaluateNear("MCScintillatorHit()", "default", "The default ADC counts are zero", Hit.GetADCCounts(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCScintillatorHit()", "default", "The default energy is zero", Hit.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCScintillatorHit()", "default", "The default time is zero", Hit.GetTime(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCScintillatorHit()", "default", "The default name is Unknown", MString(Hit.GetDetectorName().c_str()), MString("Unknown")) && Passed;
  Passed = EvaluateSize("MCScintillatorHit()", "default", "A new hit has no origins", Hit.m_Origins.size(), 0) && Passed;
  Passed = EvaluateSize("MCScintillatorHit()", "default", "A new hit has no volume history", Hit.m_VolumeHistory.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestGettersSetters()
{
  bool Passed = true;

  MCScintillatorHitAccess Hit;
  Hit.SetADCCounts(1234.5);
  Passed = EvaluateNear("SetADCCounts/GetADCCounts", "1234.5", "GetADCCounts returns the counts set before", Hit.GetADCCounts(), 1234.5, 1e-9) && Passed;
  Hit.SetADCCounts(0.0);
  Passed = EvaluateNear("SetADCCounts/GetADCCounts", "zero", "Zero counts are stored", Hit.GetADCCounts(), 0.0, 1e-12) && Passed;
  Hit.SetADCCounts(-5.0);
  Passed = EvaluateNear("SetADCCounts/GetADCCounts", "negative", "Negative counts are stored as they are", Hit.GetADCCounts(), -5.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestCopyConstructor()
{
  bool Passed = true;

  // Without a volume history:
  {
    MCScintillatorHitAccess Original;
    Fill(Original, 511.0*keV, 511.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 4.0*s, 12, {});
    MCScintillatorHitAccess Copy(Original);
    Passed = EvaluateNear("MCScintillatorHit(const MCScintillatorHit&)", "no history", "The copy keeps the energy", Copy.GetEnergy()/keV, 511.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCScintillatorHit(const MCScintillatorHit&)", "no history", "The copy keeps the ADC counts", Copy.GetADCCounts(), 511.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCScintillatorHit(const MCScintillatorHit&)", "no history", "The copy keeps the position", Copy.GetPosition().getY()/cm, 2.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCScintillatorHit(const MCScintillatorHit&)", "no history", "The copy keeps the time", Copy.GetTime()/s, 4.0, 1e-9) && Passed;
    Passed = Evaluate("MCScintillatorHit(const MCScintillatorHit&)", "no history", "The copy keeps the name", MString(Copy.GetDetectorName().c_str()), MString("ACS_Panel")) && Passed;
    Passed = Evaluate("MCScintillatorHit(const MCScintillatorHit&)", "no history", "The copy keeps the detector type", Copy.GetDetectorType(), (int) MDDetector::c_Scintillator) && Passed;
    Passed = EvaluateTrue("MCScintillatorHit(const MCScintillatorHit&)", "no history", "The copy keeps the origin", Copy.m_Origins.count(12) == 1) && Passed;
    Copy.SetEnergy(1.0*keV);
    Passed = EvaluateNear("MCScintillatorHit(const MCScintillatorHit&)", "independence", "Changing the copy does not change the original", Original.GetEnergy()/keV, 511.0, 1e-9) && Passed;
  }

  // With a volume history - the copy must keep it (executed in a child process, since it crashes if the history is not copied correctly):
  bool KeepsHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCScintillatorHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("ACS");
    Original.AddVolumeHistory("Panel");
    MCScintillatorHitAccess Copy(Original);
    return Copy.m_VolumeHistory.size() == 3 && Copy.m_VolumeHistory[0] == "World" && Copy.m_VolumeHistory[2] == "Panel";
  });
  // Known failure: the copy constructor assigns into the empty volume history of the new hit instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("MCScintillatorHit(const MCScintillatorHit&)", "with history", "The copy of a hit with a volume history has the same volume history", KeepsHistory)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestAssignment()
{
  bool Passed = true;

  // Without a volume history:
  {
    MCScintillatorHitAccess Original;
    Fill(Original, 662.0*keV, 662.0, G4ThreeVector(-1.0*cm, 5.0*cm, 7.0*cm), 0.5*s, 3, {});
    MCScintillatorHitAccess Target;
    Target.AddOrigin(99);
    Target = Original;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the energy", Target.GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the ADC counts", Target.GetADCCounts(), 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the position", Target.GetPosition().getZ()/cm, 7.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the time", Target.GetTime()/s, 0.5, 1e-9) && Passed;
    Passed = EvaluateSize("operator=", "no history", "The origins of the assigned hit are replaced, not merged", Target.m_Origins.size(), 1) && Passed;
    Passed = EvaluateTrue("operator=", "no history", "The origin of the assigned hit is the assigned one", Target.m_Origins.count(3) == 1) && Passed;
  }

  // With a volume history into a fresh hit - executed in a child process, since it crashes if the history is not copied correctly:
  bool IntoFresh = UTSDHitShared::RunsCleanlyInChild([]() {
    MCScintillatorHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Panel");
    MCScintillatorHitAccess Target;
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[1] == "Panel";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into fresh hit", "Assigning a hit with a volume history gives the same volume history", IntoFresh)) && Passed;

  // Replacing an existing history:
  bool ReplacesHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCScintillatorHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Panel");
    MCScintillatorHitAccess Target;
    Target.AddVolumeHistory("Old1");
    Target.AddVolumeHistory("Old2");
    Target.AddVolumeHistory("Old3");
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[0] == "World" && Target.m_VolumeHistory[1] == "Panel";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into used hit", "Assigning replaces the old volume history", ReplacesHistory)) && Passed;

  // Self assignment must not lose data:
  bool SelfAssignment = UTSDHitShared::RunsCleanlyInChild([]() {
    MCScintillatorHitAccess Hit;
    Hit.SetEnergy(10.0*keV);
    Hit.AddVolumeHistory("World");
    Hit.AddVolumeHistory("Panel");
    Hit = Hit;
    return Hit.m_VolumeHistory.size() == 2 && Hit.GetEnergy() == 10.0*keV;
  });
  // Known failure: operator= has no self-assignment check and resizes its own volume history to zero before copying it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "self assignment", "Assigning a hit to itself keeps its data", SelfAssignment)) && Passed;

  // The assignment returns the hit for chaining:
  MCScintillatorHitAccess A;
  MCScintillatorHitAccess B;
  MCScintillatorHitAccess C;
  A.SetEnergy(5.0*keV);
  const MCScintillatorHit& Result = (B = A);
  Passed = EvaluateTrue("operator=", "chaining", "The assignment returns the assigned hit", &Result == &B) && Passed;
  C = B = A;
  Passed = EvaluateNear("operator=", "chaining", "A chained assignment reaches the last hit", C.GetEnergy()/keV, 5.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestEqualOperator()
{
  bool Passed = true;

  MCScintillatorHitAccess A;
  MCScintillatorHitAccess B;
  Passed = EvaluateTrue("operator==", "no history", "Two hits without volume history are equal", (A == B) != 0) && Passed;

  A.AddVolumeHistory("World");
  A.AddVolumeHistory("Panel");
  B.AddVolumeHistory("World");
  B.AddVolumeHistory("Panel");
  Passed = EvaluateTrue("operator==", "same history", "Hits in the same volume are equal", (A == B) != 0) && Passed;

  // The energy and other values are not compared:
  A.SetEnergy(1.0*keV);
  B.SetEnergy(900.0*keV);
  Passed = EvaluateTrue("operator==", "different energy", "Only the volumes are compared, not the energy", (A == B) != 0) && Passed;

  MCScintillatorHitAccess C;
  C.AddVolumeHistory("World");
  C.AddVolumeHistory("OtherPanel");
  Passed = EvaluateFalse("operator==", "different last volume", "Hits in different volumes are not equal", (A == C) != 0) && Passed;

  MCScintillatorHitAccess D;
  D.AddVolumeHistory("OtherWorld");
  D.AddVolumeHistory("Panel");
  Passed = EvaluateFalse("operator==", "different first volume", "Hits with a different first volume are not equal", (A == D) != 0) && Passed;

  // Different lengths of the volume history - executed in child processes, since the comparison reads beyond the shorter history:
  bool LongerOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCScintillatorHitAccess Short;
    Short.AddVolumeHistory("World");
    MCScintillatorHitAccess Long;
    Long.AddVolumeHistory("World");
    Long.AddVolumeHistory("Panel");
    return (Short == Long) == 0;
  });
  // Known failure: operator== compares only as many volumes as the own history has and ignores the longer history of the other hit
  Passed = ExpectedToFail(EvaluateTrue("operator==", "other has a longer history", "A hit is not equal to a hit with a longer volume history", LongerOther)) && Passed;

  bool ShorterOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCScintillatorHitAccess Short;
    Short.AddVolumeHistory("World");
    MCScintillatorHitAccess Long;
    Long.AddVolumeHistory("World");
    Long.AddVolumeHistory("Panel");
    return (Long == Short) == 0;
  });
  Passed = EvaluateTrue("operator==", "other has a shorter history", "A hit is not equal to a hit with a shorter volume history", ShorterOther) && Passed;

  bool EmptyOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCScintillatorHitAccess Empty;
    MCScintillatorHitAccess Long;
    Long.AddVolumeHistory("World");
    return (Long == Empty) == 0 && (Empty == Long) == 0;
  });
  // Known failure: operator== does not compare the history sizes and reads the other hit's empty history out of bounds
  Passed = ExpectedToFail(EvaluateTrue("operator==", "one empty history", "A hit with and a hit without volume history are not equal", EmptyOther)) && Passed;

  // Symmetry:
  Passed = EvaluateTrue("operator==", "symmetry", "Equality does not depend on the order", ((A == B) != 0) == ((B == A) != 0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestAddOperator()
{
  bool Passed = true;

  // Same volume:
  {
    MCScintillatorHitAccess A;
    Fill(A, 100.0*keV, 100.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 5.0*s, 1, { "World", "Panel" });
    MCScintillatorHitAccess B;
    Fill(B, 250.0*keV, 250.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, { "World", "Panel" });
    const MCScintillatorHit& Result = (A += B);
    Passed = EvaluateTrue("operator+=", "same volume", "The operator returns the hit it was called on", &Result == &A) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The energies are added", A.GetEnergy()/keV, 350.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The ADC counts are added", A.GetADCCounts(), 350.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The position is the one of the added hit", A.GetPosition().getX()/cm, 2.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The position is the one of the added hit (z)", A.GetPosition().getZ()/cm, 4.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The earlier time is kept", A.GetTime()/s, 3.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator+=", "same volume", "The origins are merged", A.m_Origins.size(), 2) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The added hit is not changed", B.GetEnergy()/keV, 250.0, 1e-9) && Passed;
  }

  // The earlier time of this hit is kept:
  {
    MCScintillatorHitAccess A;
    Fill(A, 100.0*keV, 100.0, G4ThreeVector(), 2.0*s, 1, { "World", "Panel" });
    MCScintillatorHitAccess B;
    Fill(B, 100.0*keV, 100.0, G4ThreeVector(), 9.0*s, 2, { "World", "Panel" });
    A += B;
    Passed = EvaluateNear("operator+=", "later time added", "A later time of the added hit does not change the time", A.GetTime()/s, 2.0, 1e-9) && Passed;
  }

  // Different volumes are not added:
  {
    MCScintillatorHitAccess A;
    Fill(A, 100.0*keV, 100.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 5.0*s, 1, { "World", "Panel1" });
    MCScintillatorHitAccess B;
    Fill(B, 250.0*keV, 250.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, { "World", "Panel2" });
    A += B;
    Passed = EvaluateNear("operator+=", "different volumes", "Hits in different volumes keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "different volumes", "Hits in different volumes keep their position", A.GetPosition().getX()/cm, 1.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator+=", "different volumes", "Hits in different volumes keep their origins", A.m_Origins.size(), 1) && Passed;
  }

  // Different length of the history is not added:
  {
    MCScintillatorHitAccess A;
    Fill(A, 100.0*keV, 100.0, G4ThreeVector(), 5.0*s, 1, { "World" });
    MCScintillatorHitAccess B;
    Fill(B, 250.0*keV, 250.0, G4ThreeVector(), 3.0*s, 2, { "World", "Panel" });
    A += B;
    Passed = EvaluateNear("operator+=", "different history length", "Hits with a different history length are not added", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  }

  // Empty history on both hits is the same volume:
  {
    MCScintillatorHitAccess A;
    A.SetEnergy(10.0*keV);
    MCScintillatorHitAccess B;
    B.SetEnergy(20.0*keV);
    A += B;
    Passed = EvaluateNear("operator+=", "no history", "Hits without volume history are added", A.GetEnergy()/keV, 30.0, 1e-9) && Passed;
  }

  // Adding a hit to itself doubles the energy and does not break the origins:
  {
    MCScintillatorHitAccess A;
    Fill(A, 100.0*keV, 100.0, G4ThreeVector(), 1.0*s, 1, { "World", "Panel" });
    A += A;
    Passed = EvaluateNear("operator+=", "self", "Adding a hit to itself doubles the energy", A.GetEnergy()/keV, 200.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator+=", "self", "Adding a hit to itself keeps the origins", A.m_Origins.size(), 1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestAllocationAndCollection()
{
  bool Passed = true;

  // Allocation through the Geant4 allocator:
  vector<MCScintillatorHit*> Hits;
  for (unsigned int i = 0; i < 1000; ++i) {
    MCScintillatorHit* Hit = new MCScintillatorHit();
    Hit->SetEnergy((i + 1)*keV);
    Hits.push_back(Hit);
  }
  set<MCScintillatorHit*> Addresses(Hits.begin(), Hits.end());
  Passed = EvaluateSize("operator new", "1000 hits", "All allocated hits have different addresses", Addresses.size(), 1000) && Passed;
  bool AllKept = true;
  for (unsigned int i = 0; i < Hits.size(); ++i) {
    if (Hits[i]->GetEnergy() != (i + 1)*keV) AllKept = false;
  }
  Passed = EvaluateTrue("operator new", "1000 hits", "Every allocated hit keeps its own energy", AllKept) && Passed;
  for (MCScintillatorHit* Hit: Hits) delete Hit;

  // Freed memory can be reused:
  MCScintillatorHit* Reused = new MCScintillatorHit();
  Passed = EvaluateNear("operator new", "reuse", "A new hit after freeing others is initialized", Reused->GetEnergy(), 0.0, 1e-12) && Passed;
  delete Reused;

  // Storage in a hit collection, which deletes its hits:
  MCScintillatorHitsCollection* Collection = new MCScintillatorHitsCollection("Detector", "Collection");
  MCScintillatorHit* First = new MCScintillatorHit();
  First->SetEnergy(511.0*keV);
  MCScintillatorHit* Second = new MCScintillatorHit();
  Second->SetEnergy(1275.0*keV);
  Collection->insert(First);
  Collection->insert(Second);
  Passed = EvaluateSize("G4THitsCollection::insert", "two hits", "The collection holds both hits", Collection->entries(), 2) && Passed;
  Passed = EvaluateNear("G4THitsCollection::operator[]", "first hit", "The first hit of the collection is the first inserted", (*Collection)[0]->GetEnergy()/keV, 511.0, 1e-9) && Passed;
  Passed = EvaluateNear("G4THitsCollection::operator[]", "second hit", "The second hit of the collection is the second inserted", (*Collection)[1]->GetEnergy()/keV, 1275.0, 1e-9) && Passed;
  delete Collection;
  Passed = EvaluateTrue("~G4THitsCollection", "two hits", "The collection deletes its hits with the allocator without problems", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestGetCalibrated()
{
  bool Passed = true;

  MCScintillatorHitAccess Hit;
  Fill(Hit, 1275.0*keV, 1275.0, G4ThreeVector(10.0*mm, 20.0*mm, -30.0*mm), 0.125*s, 6, {});
  MSimHT* HT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "scintillator hit", "A calibrated hit is returned", HT != nullptr) && Passed;
  if (HT != nullptr) {
    Passed = Evaluate("GetCalibrated()", "scintillator hit", "The detector type is scintillator", HT->GetDetectorType(), (int) MDDetector::c_Scintillator) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "scintillator hit", "The energy is given in keV", HT->GetEnergy(), 1275.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "scintillator hit", "The x position is given in cm", HT->GetPosition().X(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "scintillator hit", "The z position is given in cm", HT->GetPosition().Z(), -3.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "scintillator hit", "The time is given in seconds", HT->GetTime(), 0.125, 1e-9) && Passed;
    Passed = Evaluate("GetCalibrated()", "scintillator hit", "The origin is passed on", HT->GetNOrigins(), 1U) && Passed;
    delete HT;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTScintillatorHit::TestPrint()
{
  bool Passed = true;

  MCScintillatorHitAccess Hit;
  Fill(Hit, 662.0*keV, 662.0, G4ThreeVector(10.0*mm, -25.0*mm, 30.0*mm), 0.0, 1, {});

  ostringstream Out;
  streambuf* Old = cout.rdbuf(Out.rdbuf());
  Hit.Print();
  cout.rdbuf(Old);
  MString Text(Out.str().c_str());
  Passed = EvaluateTrue("Print()", "representative hit", "The output contains the detector name", Text.Contains("ACS_Panel")) && Passed;
  Passed = EvaluateTrue("Print()", "representative hit", "The output contains the energy in keV", Text.Contains("662 keV")) && Passed;
  Passed = EvaluateTrue("Print()", "representative hit", "The output contains the x position in cm", Text.Contains("(1,")) && Passed;
  Passed = EvaluateTrue("Print()", "representative hit", "The output contains the y position in cm", Text.Contains("-2.5")) && Passed;
  Passed = EvaluateTrue("Print()", "representative hit", "The output contains the z position in cm", Text.Contains("3)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTScintillatorHit Test;
  return Test.Run() == true ? 0 : 1;
}
