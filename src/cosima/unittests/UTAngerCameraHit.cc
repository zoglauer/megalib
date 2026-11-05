/*
 * UTAngerCameraHit.cc
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
#include "MCAngerCameraHit.hh"
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
#include <cmath>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;


//! Class exposing the protected members of the hit to the unit test
class MCAngerCameraHitAccess : public MCAngerCameraHit
{
public:
  using MCVHit::m_Time;
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Unit test class for MCAngerCameraHit
class UTAngerCameraHit : public MUnitTest
{
public:
  //! Default constructor
  UTAngerCameraHit() : MUnitTest("UTAngerCameraHit") {}
  //! Default destructor
  virtual ~UTAngerCameraHit() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default constructor
  bool TestDefaultConstruction();
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

  //! Fill a hit with a representative content
  void Fill(MCAngerCameraHitAccess& Hit, double Energy, const G4ThreeVector& Position, double Time, unsigned int Origin, const vector<string>& History);
};


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
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


void UTAngerCameraHit::Fill(MCAngerCameraHitAccess& Hit, double Energy, const G4ThreeVector& Position, double Time, unsigned int Origin, const vector<string>& History)
{
  Hit.SetEnergy(Energy);
  Hit.SetPosition(Position);
  Hit.SetTime(Time);
  Hit.SetDetectorName("Anger_1");
  Hit.AddOrigin(Origin);
  for (const string& Name: History) {
    Hit.AddVolumeHistory(Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::TestDefaultConstruction()
{
  bool Passed = true;

  MCAngerCameraHitAccess Hit;
  Passed = Evaluate("MCAngerCameraHit()", "default", "The detector type is Anger camera", Hit.GetDetectorType(), (int) MDDetector::c_AngerCamera) && Passed;
  Passed = EvaluateNear("MCAngerCameraHit()", "default", "The default energy is zero", Hit.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCAngerCameraHit()", "default", "The default time is zero", Hit.GetTime(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCAngerCameraHit()", "default", "The default name is Unknown", MString(Hit.GetDetectorName().c_str()), MString("Unknown")) && Passed;
  Passed = EvaluateSize("MCAngerCameraHit()", "default", "A new hit has no origins", Hit.m_Origins.size(), 0) && Passed;
  Passed = EvaluateSize("MCAngerCameraHit()", "default", "A new hit has no volume history", Hit.m_VolumeHistory.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::TestCopyConstructor()
{
  bool Passed = true;

  {
    MCAngerCameraHitAccess Original;
    Fill(Original, 511.0*keV, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 4.0*s, 12, {});
    MCAngerCameraHitAccess Copy(Original);
    Passed = EvaluateNear("MCAngerCameraHit(const MCAngerCameraHit&)", "no history", "The copy keeps the energy", Copy.GetEnergy()/keV, 511.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCAngerCameraHit(const MCAngerCameraHit&)", "no history", "The copy keeps the position", Copy.GetPosition().getY()/cm, 2.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCAngerCameraHit(const MCAngerCameraHit&)", "no history", "The copy keeps the time", Copy.GetTime()/s, 4.0, 1e-9) && Passed;
    Passed = Evaluate("MCAngerCameraHit(const MCAngerCameraHit&)", "no history", "The copy keeps the name", MString(Copy.GetDetectorName().c_str()), MString("Anger_1")) && Passed;
    Passed = Evaluate("MCAngerCameraHit(const MCAngerCameraHit&)", "no history", "The copy keeps the detector type", Copy.GetDetectorType(), (int) MDDetector::c_AngerCamera) && Passed;
    Passed = EvaluateTrue("MCAngerCameraHit(const MCAngerCameraHit&)", "no history", "The copy keeps the origin", Copy.m_Origins.count(12) == 1) && Passed;
    Copy.SetEnergy(1.0*keV);
    Passed = EvaluateNear("MCAngerCameraHit(const MCAngerCameraHit&)", "independence", "Changing the copy does not change the original", Original.GetEnergy()/keV, 511.0, 1e-9) && Passed;
  }

  bool KeepsHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCAngerCameraHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Camera");
    Original.AddVolumeHistory("Crystal");
    MCAngerCameraHitAccess Copy(Original);
    return Copy.m_VolumeHistory.size() == 3 && Copy.m_VolumeHistory[0] == "World" && Copy.m_VolumeHistory[2] == "Crystal";
  });
  // Known failure: the copy constructor assigns into the empty volume history of the new hit instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("MCAngerCameraHit(const MCAngerCameraHit&)", "with history", "The copy of a hit with a volume history has the same volume history", KeepsHistory)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::TestAssignment()
{
  bool Passed = true;

  {
    MCAngerCameraHitAccess Original;
    Fill(Original, 662.0*keV, G4ThreeVector(-1.0*cm, 5.0*cm, 7.0*cm), 0.5*s, 3, {});
    MCAngerCameraHitAccess Target;
    Target.AddOrigin(99);
    Target = Original;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the energy", Target.GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the position", Target.GetPosition().getZ()/cm, 7.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the time", Target.GetTime()/s, 0.5, 1e-9) && Passed;
    Passed = EvaluateSize("operator=", "no history", "The origins of the assigned hit are replaced, not merged", Target.m_Origins.size(), 1) && Passed;
  }

  bool IntoFresh = UTSDHitShared::RunsCleanlyInChild([]() {
    MCAngerCameraHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Crystal");
    MCAngerCameraHitAccess Target;
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[1] == "Crystal";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into fresh hit", "Assigning a hit with a volume history gives the same volume history", IntoFresh)) && Passed;

  bool ReplacesHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCAngerCameraHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Crystal");
    MCAngerCameraHitAccess Target;
    Target.AddVolumeHistory("Old1");
    Target.AddVolumeHistory("Old2");
    Target.AddVolumeHistory("Old3");
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[0] == "World" && Target.m_VolumeHistory[1] == "Crystal";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into used hit", "Assigning replaces the old volume history", ReplacesHistory)) && Passed;

  bool SelfAssignment = UTSDHitShared::RunsCleanlyInChild([]() {
    MCAngerCameraHitAccess Hit;
    Hit.SetEnergy(10.0*keV);
    Hit.AddVolumeHistory("World");
    Hit.AddVolumeHistory("Crystal");
    Hit = Hit;
    return Hit.m_VolumeHistory.size() == 2 && Hit.GetEnergy() == 10.0*keV;
  });
  // Known failure: operator= has no self-assignment check and resizes its own volume history to zero before copying it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "self assignment", "Assigning a hit to itself keeps its data", SelfAssignment)) && Passed;

  MCAngerCameraHitAccess A;
  MCAngerCameraHitAccess B;
  MCAngerCameraHitAccess C;
  A.SetEnergy(5.0*keV);
  const MCAngerCameraHit& Result = (B = A);
  Passed = EvaluateTrue("operator=", "chaining", "The assignment returns the assigned hit", &Result == &B) && Passed;
  C = B = A;
  Passed = EvaluateNear("operator=", "chaining", "A chained assignment reaches the last hit", C.GetEnergy()/keV, 5.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::TestEqualOperator()
{
  bool Passed = true;

  MCAngerCameraHitAccess A;
  MCAngerCameraHitAccess B;
  Passed = EvaluateTrue("operator==", "no history", "Two hits without volume history are equal", (A == B) != 0) && Passed;

  Fill(A, 1.0*keV, G4ThreeVector(), 0.0, 1, { "World", "Camera" });
  Fill(B, 900.0*keV, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 2.0*s, 2, { "World", "Camera" });
  Passed = EvaluateTrue("operator==", "same volume", "Hits in the same volume are equal, energy and position are not compared", (A == B) != 0) && Passed;

  MCAngerCameraHitAccess C;
  Fill(C, 1.0*keV, G4ThreeVector(), 0.0, 1, { "World", "OtherCamera" });
  Passed = EvaluateFalse("operator==", "different volume", "Hits in different volumes are not equal", (A == C) != 0) && Passed;

  bool LongerOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCAngerCameraHitAccess Short;
    Short.AddVolumeHistory("World");
    MCAngerCameraHitAccess Long;
    Long.AddVolumeHistory("World");
    Long.AddVolumeHistory("Crystal");
    return (Short == Long) == 0;
  });
  // Known failure: operator== compares only as many volumes as the own history has and ignores the longer history of the other hit
  Passed = ExpectedToFail(EvaluateTrue("operator==", "other has a longer history", "A hit is not equal to a hit with a longer volume history", LongerOther)) && Passed;

  bool EmptyOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCAngerCameraHitAccess Empty;
    MCAngerCameraHitAccess Long;
    Long.AddVolumeHistory("World");
    return (Long == Empty) == 0 && (Empty == Long) == 0;
  });
  // Known failure: operator== does not compare the history sizes and reads the other hit's empty history out of bounds
  Passed = ExpectedToFail(EvaluateTrue("operator==", "one empty history", "A hit with and a hit without volume history are not equal", EmptyOther)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::TestAddOperator()
{
  bool Passed = true;

  {
    MCAngerCameraHitAccess A;
    Fill(A, 100.0*keV, G4ThreeVector(0.0, 0.0, 0.0), 5.0*s, 1, { "World", "Camera" });
    MCAngerCameraHitAccess B;
    Fill(B, 300.0*keV, G4ThreeVector(4.0*cm, 8.0*cm, 12.0*cm), 3.0*s, 2, { "World", "Camera" });
    const MCAngerCameraHit& Result = (A += B);
    Passed = EvaluateTrue("operator+=", "same volume", "The operator returns the hit it was called on", &Result == &A) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The energies are added", A.GetEnergy()/keV, 400.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The x position is the energy weighted average", A.GetPosition().x()/cm, 3.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The y position is the energy weighted average", A.GetPosition().y()/cm, 6.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The z position is the energy weighted average", A.GetPosition().z()/cm, 9.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The earlier time is kept", A.GetTime()/s, 3.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator+=", "same volume", "The origins are merged", A.m_Origins.size(), 2) && Passed;
    Passed = EvaluateNear("operator+=", "same volume", "The added hit is not changed", B.GetEnergy()/keV, 300.0, 1e-9) && Passed;
  }

  {
    MCAngerCameraHitAccess A;
    Fill(A, 100.0*keV, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 5.0*s, 1, { "World", "Camera" });
    MCAngerCameraHitAccess Other;
    Fill(Other, 250.0*keV, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, { "World", "Camera2" });
    A += Other;
    Passed = EvaluateNear("operator+=", "different volumes", "Hits in different volumes keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "different volumes", "Hits in different volumes keep their position", A.GetPosition().getX()/cm, 1.0, 1e-9) && Passed;
    MCAngerCameraHitAccess OtherLength;
    Fill(OtherLength, 250.0*keV, G4ThreeVector(), 3.0*s, 2, { "World" });
    A += OtherLength;
    Passed = EvaluateNear("operator+=", "different history length", "Hits with a different history length keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  }

  {
    MCAngerCameraHitAccess Zero1;
    Fill(Zero1, 0.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 1.0*s, 1, {});
    MCAngerCameraHitAccess Zero2;
    Fill(Zero2, 0.0, G4ThreeVector(3.0*cm, 4.0*cm, 5.0*cm), 1.0*s, 2, {});
    Zero1 += Zero2;
    // Known failure: operator+= divides the energy weighted position by the energy sum, which is zero here
    Passed = ExpectedToFail(EvaluateTrue("operator+=", "zero energies", "Adding two hits without energy does not give a position which is not a number", std::isfinite(Zero1.GetPosition().x()) && std::isfinite(Zero1.GetPosition().y()) && std::isfinite(Zero1.GetPosition().z()))) && Passed;
  }

  {
    MCAngerCameraHitAccess A;
    Fill(A, 100.0*keV, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 1.0*s, 1, { "World", "Camera" });
    A += A;
    Passed = EvaluateNear("operator+=", "self", "Adding a hit to itself doubles the energy", A.GetEnergy()/keV, 200.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "self", "Adding a hit to itself keeps the position", A.GetPosition().getY()/cm, 2.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::TestAllocationAndCollection()
{
  bool Passed = true;

  vector<MCAngerCameraHit*> Hits;
  for (unsigned int i = 0; i < 1000; ++i) {
    MCAngerCameraHit* Hit = new MCAngerCameraHit();
    Hit->SetEnergy((i + 1)*keV);
    Hits.push_back(Hit);
  }
  set<MCAngerCameraHit*> Addresses(Hits.begin(), Hits.end());
  Passed = EvaluateSize("operator new", "1000 hits", "All allocated hits have different addresses", Addresses.size(), 1000) && Passed;
  bool AllKept = true;
  for (unsigned int i = 0; i < Hits.size(); ++i) {
    if (Hits[i]->GetEnergy() != (i + 1)*keV) AllKept = false;
  }
  Passed = EvaluateTrue("operator new", "1000 hits", "Every allocated hit keeps its own energy", AllKept) && Passed;
  for (MCAngerCameraHit* Hit: Hits) delete Hit;

  MCAngerCameraHit* Reused = new MCAngerCameraHit();
  Passed = EvaluateNear("operator new", "reuse", "A new hit after freeing others is initialized", Reused->GetEnergy(), 0.0, 1e-12) && Passed;
  delete Reused;

  MCAngerCameraHitsCollection* Collection = new MCAngerCameraHitsCollection("Detector", "Collection");
  MCAngerCameraHit* First = new MCAngerCameraHit();
  First->SetEnergy(511.0*keV);
  MCAngerCameraHit* Second = new MCAngerCameraHit();
  Second->SetEnergy(1275.0*keV);
  Collection->insert(First);
  Collection->insert(Second);
  Passed = EvaluateSize("G4THitsCollection::insert", "two hits", "The collection holds both hits", Collection->entries(), 2) && Passed;
  Passed = EvaluateNear("G4THitsCollection::operator[]", "second hit", "The second hit of the collection is the second inserted", (*Collection)[1]->GetEnergy()/keV, 1275.0, 1e-9) && Passed;
  delete Collection;
  Passed = EvaluateTrue("~G4THitsCollection", "two hits", "The collection deletes its hits with the allocator without problems", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::TestGetCalibrated()
{
  bool Passed = true;

  MCAngerCameraHitAccess Hit;
  Fill(Hit, 1275.0*keV, G4ThreeVector(10.0*mm, 20.0*mm, -30.0*mm), 0.125*s, 6, {});
  MSimHT* HT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "Anger camera hit", "A calibrated hit is returned", HT != nullptr) && Passed;
  if (HT != nullptr) {
    Passed = Evaluate("GetCalibrated()", "Anger camera hit", "The detector type is Anger camera", HT->GetDetectorType(), (int) MDDetector::c_AngerCamera) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "Anger camera hit", "The energy is given in keV", HT->GetEnergy(), 1275.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "Anger camera hit", "The x position is given in cm", HT->GetPosition().X(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "Anger camera hit", "The z position is given in cm", HT->GetPosition().Z(), -3.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "Anger camera hit", "The time is given in seconds", HT->GetTime(), 0.125, 1e-9) && Passed;
    delete HT;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAngerCameraHit::TestPrint()
{
  bool Passed = true;

  MCAngerCameraHitAccess Hit;
  Fill(Hit, 662.0*keV, G4ThreeVector(10.0*mm, -25.0*mm, 30.0*mm), 0.0, 1, {});
  ostringstream Out;
  streambuf* Old = cout.rdbuf(Out.rdbuf());
  Hit.Print();
  cout.rdbuf(Old);
  MString Text(Out.str().c_str());
  Passed = EvaluateTrue("Print()", "representative hit", "The output contains the detector name", Text.Contains("Anger_1")) && Passed;
  Passed = EvaluateTrue("Print()", "representative hit", "The output contains the position in cm", Text.Contains("(1/-2.5/3)")) && Passed;
  Passed = EvaluateTrue("Print()", "representative hit", "The output contains the energy in keV", Text.Contains("662 keV")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTAngerCameraHit Test;
  return Test.Run() == true ? 0 : 1;
}
