/*
 * UTC2DStripHit.cc
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
#include "MC2DStripHit.hh"
#include "UTSDHitShared.h"

// MEGAlib:
#include "MDDetector.h"
#include "MSimGR.h"
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
class MC2DStripHitAccess : public MC2DStripHit
{
public:
  MC2DStripHitAccess(bool Is3D = false) : MC2DStripHit(Is3D) {}
  using MCVHit::m_Time;
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Unit test class for MC2DStripHit
class UTC2DStripHit : public MUnitTest
{
public:
  //! Default constructor
  UTC2DStripHit() : MUnitTest("UTC2DStripHit") {}
  //! Default destructor
  virtual ~UTC2DStripHit() {}

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
  //! Test the conversion into calibrated hits and guard ring hits
  bool TestGetCalibrated();
  //! Test the output
  bool TestPrint();

  //! Fill a hit with a representative content
  void Fill(MC2DStripHitAccess& Hit, double Energy, double ADC, const G4ThreeVector& Position, double Time, unsigned int Origin, int XStrip, int YStrip, const vector<string>& History);
  //! Return the text printed by the hit
  MString GetPrint(MC2DStripHit& Hit);
};


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::Run()
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


void UTC2DStripHit::Fill(MC2DStripHitAccess& Hit, double Energy, double ADC, const G4ThreeVector& Position, double Time, unsigned int Origin, int XStrip, int YStrip, const vector<string>& History)
{
  Hit.SetEnergy(Energy);
  Hit.SetADCCounts(ADC);
  Hit.SetPosition(Position);
  Hit.SetTime(Time);
  Hit.SetDetectorName("Strips_1");
  Hit.SetXStrip(XStrip);
  Hit.SetYStrip(YStrip);
  Hit.AddOrigin(Origin);
  for (const string& Name: History) {
    Hit.AddVolumeHistory(Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


MString UTC2DStripHit::GetPrint(MC2DStripHit& Hit)
{
  ostringstream Out;
  streambuf* Old = cout.rdbuf(Out.rdbuf());
  Hit.Print();
  cout.rdbuf(Old);
  return MString(Out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::TestDefaultConstruction()
{
  bool Passed = true;

  MC2DStripHitAccess Hit2D(false);
  Passed = Evaluate("MC2DStripHit(false)", "2D", "A 2D strip hit has the type 2D strip", Hit2D.GetDetectorType(), (int) MDDetector::c_Strip2D) && Passed;
  Passed = EvaluateNear("MC2DStripHit()", "default", "The default ADC counts are zero", Hit2D.GetADCCounts(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MC2DStripHit()", "default", "The default x strip is -1", Hit2D.GetXStrip(), -1) && Passed;
  Passed = Evaluate("MC2DStripHit()", "default", "The default y strip is -1", Hit2D.GetYStrip(), -1) && Passed;
  Passed = EvaluateFalse("MC2DStripHit()", "default", "A new hit is not a guard ring hit", Hit2D.GetIsGuardringHit()) && Passed;
  Passed = EvaluateNear("MC2DStripHit()", "default", "The default energy is zero", Hit2D.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MC2DStripHit()", "default", "The default name is Unknown", MString(Hit2D.GetDetectorName().c_str()), MString("Unknown")) && Passed;

  MC2DStripHitAccess Hit3D(true);
  Passed = Evaluate("MC2DStripHit(true)", "3D", "A 3D strip hit has the type 3D strip", Hit3D.GetDetectorType(), (int) MDDetector::c_Strip3D) && Passed;

  MC2DStripHitAccess Default;
  Passed = Evaluate("MC2DStripHit()", "no argument", "Without argument the hit is a 2D strip hit", Default.GetDetectorType(), (int) MDDetector::c_Strip2D) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::TestGettersSetters()
{
  bool Passed = true;

  MC2DStripHitAccess Hit;
  Hit.SetADCCounts(1234.5);
  Passed = EvaluateNear("SetADCCounts/GetADCCounts", "1234.5", "GetADCCounts returns the counts set before", Hit.GetADCCounts(), 1234.5, 1e-9) && Passed;
  Hit.SetXStrip(17);
  Hit.SetYStrip(-3);
  Passed = Evaluate("SetXStrip/GetXStrip", "17", "GetXStrip returns the strip set before", Hit.GetXStrip(), 17) && Passed;
  Passed = Evaluate("SetYStrip/GetYStrip", "-3", "GetYStrip returns the strip set before", Hit.GetYStrip(), -3) && Passed;
  Hit.SetIsGuardringHit(true);
  Passed = EvaluateTrue("SetIsGuardringHit/GetIsGuardringHit", "true", "The guard ring flag is set", Hit.GetIsGuardringHit()) && Passed;
  Hit.SetIsGuardringHit(false);
  Passed = EvaluateFalse("SetIsGuardringHit/GetIsGuardringHit", "false", "The guard ring flag can be cleared", Hit.GetIsGuardringHit()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::TestCopyConstructor()
{
  bool Passed = true;

  {
    MC2DStripHitAccess Original(true);
    Fill(Original, 511.0*keV, 100.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 4.0*s, 12, 3, 4, {});
    Original.SetIsGuardringHit(true);
    MC2DStripHitAccess Copy(Original);
    Passed = EvaluateNear("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the energy", Copy.GetEnergy()/keV, 511.0, 1e-9) && Passed;
    Passed = EvaluateNear("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the ADC counts", Copy.GetADCCounts(), 100.0, 1e-9) && Passed;
    Passed = EvaluateNear("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the position", Copy.GetPosition().getY()/cm, 2.0, 1e-9) && Passed;
    Passed = EvaluateNear("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the time", Copy.GetTime()/s, 4.0, 1e-9) && Passed;
    Passed = Evaluate("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the name", MString(Copy.GetDetectorName().c_str()), MString("Strips_1")) && Passed;
    Passed = Evaluate("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy of a 3D hit is a 3D hit", Copy.GetDetectorType(), (int) MDDetector::c_Strip3D) && Passed;
    Passed = Evaluate("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the x strip", Copy.GetXStrip(), 3) && Passed;
    Passed = Evaluate("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the y strip", Copy.GetYStrip(), 4) && Passed;
    Passed = EvaluateTrue("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the guard ring flag", Copy.GetIsGuardringHit()) && Passed;
    Passed = EvaluateTrue("MC2DStripHit(const MC2DStripHit&)", "no history", "The copy keeps the origin", Copy.m_Origins.count(12) == 1) && Passed;
    Copy.SetEnergy(1.0*keV);
    Passed = EvaluateNear("MC2DStripHit(const MC2DStripHit&)", "independence", "Changing the copy does not change the original", Original.GetEnergy()/keV, 511.0, 1e-9) && Passed;
  }

  bool KeepsHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MC2DStripHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Wafer");
    Original.AddVolumeHistory("Layer");
    MC2DStripHitAccess Copy(Original);
    return Copy.m_VolumeHistory.size() == 3 && Copy.m_VolumeHistory[0] == "World" && Copy.m_VolumeHistory[2] == "Layer";
  });
  // Known failure: the copy constructor assigns into the empty volume history of the new hit instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("MC2DStripHit(const MC2DStripHit&)", "with history", "The copy of a hit with a volume history has the same volume history", KeepsHistory)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::TestAssignment()
{
  bool Passed = true;

  {
    MC2DStripHitAccess Original(true);
    Fill(Original, 662.0*keV, 10.0, G4ThreeVector(-1.0*cm, 5.0*cm, 7.0*cm), 0.5*s, 3, 6, 7, {});
    MC2DStripHitAccess Target;
    Target.AddOrigin(99);
    Target = Original;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the energy", Target.GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the ADC counts", Target.GetADCCounts(), 10.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the position", Target.GetPosition().getZ()/cm, 7.0, 1e-9) && Passed;
    Passed = Evaluate("operator=", "no history", "The assigned hit has the strips", Target.GetXStrip()*100 + Target.GetYStrip(), 607) && Passed;
    Passed = Evaluate("operator=", "no history", "Assigning a 3D hit gives a 3D hit", Target.GetDetectorType(), (int) MDDetector::c_Strip3D) && Passed;
    Passed = EvaluateSize("operator=", "no history", "The origins of the assigned hit are replaced, not merged", Target.m_Origins.size(), 1) && Passed;
  }

  bool IntoFresh = UTSDHitShared::RunsCleanlyInChild([]() {
    MC2DStripHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Layer");
    MC2DStripHitAccess Target;
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[1] == "Layer";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into fresh hit", "Assigning a hit with a volume history gives the same volume history", IntoFresh)) && Passed;

  bool ReplacesHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MC2DStripHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Layer");
    MC2DStripHitAccess Target;
    Target.AddVolumeHistory("Old1");
    Target.AddVolumeHistory("Old2");
    Target.AddVolumeHistory("Old3");
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[0] == "World" && Target.m_VolumeHistory[1] == "Layer";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into used hit", "Assigning replaces the old volume history", ReplacesHistory)) && Passed;

  bool SelfAssignment = UTSDHitShared::RunsCleanlyInChild([]() {
    MC2DStripHitAccess Hit;
    Hit.SetEnergy(10.0*keV);
    Hit.AddVolumeHistory("World");
    Hit.AddVolumeHistory("Layer");
    Hit = Hit;
    return Hit.m_VolumeHistory.size() == 2 && Hit.GetEnergy() == 10.0*keV;
  });
  // Known failure: operator= has no self-assignment check and resizes its own volume history to zero before copying it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "self assignment", "Assigning a hit to itself keeps its data", SelfAssignment)) && Passed;

  MC2DStripHitAccess A;
  MC2DStripHitAccess B;
  MC2DStripHitAccess C;
  A.SetEnergy(5.0*keV);
  const MC2DStripHit& Result = (B = A);
  Passed = EvaluateTrue("operator=", "chaining", "The assignment returns the assigned hit", &Result == &B) && Passed;
  C = B = A;
  Passed = EvaluateNear("operator=", "chaining", "A chained assignment reaches the last hit", C.GetEnergy()/keV, 5.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::TestEqualOperator()
{
  bool Passed = true;

  MC2DStripHitAccess A;
  MC2DStripHitAccess B;
  Fill(A, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "Wafer" });
  Fill(B, 900.0*keV, 5.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 2.0*s, 2, 3, 4, { "World", "Wafer" });
  Passed = EvaluateTrue("operator==", "same strips", "Hits in the same strips of the same wafer are equal, energy and position are not compared", (A == B) != 0) && Passed;

  MC2DStripHitAccess C;
  Fill(C, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 5, 4, { "World", "Wafer" });
  Passed = EvaluateFalse("operator==", "different x strip", "Hits in a different x strip are not equal", (A == C) != 0) && Passed;
  MC2DStripHitAccess D;
  Fill(D, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 9, { "World", "Wafer" });
  Passed = EvaluateFalse("operator==", "different y strip", "Hits in a different y strip are not equal", (A == D) != 0) && Passed;
  MC2DStripHitAccess E;
  Fill(E, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "OtherWafer" });
  Passed = EvaluateFalse("operator==", "different wafer", "Hits in the same strips of another wafer are not equal", (A == E) != 0) && Passed;

  bool LongerOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MC2DStripHitAccess Short;
    Short.AddVolumeHistory("World");
    MC2DStripHitAccess Long;
    Long.AddVolumeHistory("World");
    Long.AddVolumeHistory("Wafer");
    return (Short == Long) == 0;
  });
  // Known failure: operator== compares only as many volumes as the own history has and ignores the longer history of the other hit
  Passed = ExpectedToFail(EvaluateTrue("operator==", "other has a longer history", "A hit is not equal to a hit with a longer volume history", LongerOther)) && Passed;

  bool EmptyOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MC2DStripHitAccess Empty;
    MC2DStripHitAccess Long;
    Long.AddVolumeHistory("World");
    return (Long == Empty) == 0 && (Empty == Long) == 0;
  });
  // Known failure: operator== does not compare the history sizes and reads the other hit's empty history out of bounds
  Passed = ExpectedToFail(EvaluateTrue("operator==", "one empty history", "A hit with and a hit without volume history are not equal", EmptyOther)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::TestAddOperator()
{
  bool Passed = true;

  // Same strips - the position is the energy weighted average:
  {
    MC2DStripHitAccess A;
    Fill(A, 100.0*keV, 10.0, G4ThreeVector(0.0, 0.0, 0.0), 5.0*s, 1, 3, 4, { "World", "Wafer" });
    MC2DStripHitAccess B;
    Fill(B, 300.0*keV, 30.0, G4ThreeVector(4.0*cm, 8.0*cm, 12.0*cm), 3.0*s, 2, 3, 4, { "World", "Wafer" });
    const MC2DStripHit& Result = (A += B);
    Passed = EvaluateTrue("operator+=", "same strips", "The operator returns the hit it was called on", &Result == &A) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The energies are added", A.GetEnergy()/keV, 400.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The ADC counts are added", A.GetADCCounts(), 40.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The x position is the energy weighted average", A.GetPosition().x()/cm, 3.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The y position is the energy weighted average", A.GetPosition().y()/cm, 6.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The z position is the energy weighted average", A.GetPosition().z()/cm, 9.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The earlier time is kept", A.GetTime()/s, 3.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator+=", "same strips", "The origins are merged", A.m_Origins.size(), 2) && Passed;
  }

  // Different strips or wafers are not added:
  {
    MC2DStripHitAccess A;
    Fill(A, 100.0*keV, 10.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 5.0*s, 1, 3, 4, { "World", "Wafer" });
    MC2DStripHitAccess OtherStrip;
    Fill(OtherStrip, 250.0*keV, 30.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 5, { "World", "Wafer" });
    A += OtherStrip;
    Passed = EvaluateNear("operator+=", "different strip", "Hits in different strips keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    MC2DStripHitAccess OtherWafer;
    Fill(OtherWafer, 250.0*keV, 30.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 4, { "World", "Wafer2" });
    A += OtherWafer;
    Passed = EvaluateNear("operator+=", "different wafer", "Hits in different wafers keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    MC2DStripHitAccess OtherLength;
    Fill(OtherLength, 250.0*keV, 30.0, G4ThreeVector(), 3.0*s, 2, 3, 4, { "World" });
    A += OtherLength;
    Passed = EvaluateNear("operator+=", "different history length", "Hits with a different history length keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  }

  // Guard ring hits are added without moving the position:
  {
    MC2DStripHitAccess A;
    Fill(A, 100.0*keV, 10.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 5.0*s, 1, -1, -1, { "World", "Wafer" });
    A.SetIsGuardringHit(true);
    MC2DStripHitAccess B;
    Fill(B, 300.0*keV, 30.0, G4ThreeVector(9.0*cm, 9.0*cm, 9.0*cm), 3.0*s, 2, -1, -1, { "World", "Wafer" });
    B.SetIsGuardringHit(true);
    A += B;
    Passed = EvaluateNear("operator+=", "guard rings", "The energies of guard ring hits are added", A.GetEnergy()/keV, 400.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "guard rings", "The ADC counts of guard ring hits are added", A.GetADCCounts(), 40.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "guard rings", "The position of guard ring hits stays the same", A.GetPosition().getX()/cm, 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "guard rings", "The earlier time of guard ring hits is kept", A.GetTime()/s, 3.0, 1e-9) && Passed;
  }

  // Hits without energy have a defined position:
  {
    MC2DStripHitAccess Zero1;
    Fill(Zero1, 0.0, 0.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 1.0*s, 1, 1, 1, {});
    MC2DStripHitAccess Zero2;
    Fill(Zero2, 0.0, 0.0, G4ThreeVector(3.0*cm, 4.0*cm, 5.0*cm), 1.0*s, 2, 1, 1, {});
    Zero1 += Zero2;
    // Known failure: operator+= divides the energy weighted position by the energy sum, which is zero here
    Passed = ExpectedToFail(EvaluateTrue("operator+=", "zero energies", "Adding two hits without energy does not give a position which is not a number", std::isfinite(Zero1.GetPosition().x()) && std::isfinite(Zero1.GetPosition().y()) && std::isfinite(Zero1.GetPosition().z()))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::TestAllocationAndCollection()
{
  bool Passed = true;

  vector<MC2DStripHit*> Hits;
  for (unsigned int i = 0; i < 1000; ++i) {
    MC2DStripHit* Hit = new MC2DStripHit();
    Hit->SetEnergy((i + 1)*keV);
    Hits.push_back(Hit);
  }
  set<MC2DStripHit*> Addresses(Hits.begin(), Hits.end());
  Passed = EvaluateSize("operator new", "1000 hits", "All allocated hits have different addresses", Addresses.size(), 1000) && Passed;
  bool AllKept = true;
  for (unsigned int i = 0; i < Hits.size(); ++i) {
    if (Hits[i]->GetEnergy() != (i + 1)*keV) AllKept = false;
  }
  Passed = EvaluateTrue("operator new", "1000 hits", "Every allocated hit keeps its own energy", AllKept) && Passed;
  for (MC2DStripHit* Hit: Hits) delete Hit;

  MC2DStripHit* Reused = new MC2DStripHit();
  Passed = EvaluateNear("operator new", "reuse", "A new hit after freeing others is initialized", Reused->GetEnergy(), 0.0, 1e-12) && Passed;
  delete Reused;

  MC2DStripHitsCollection* Collection = new MC2DStripHitsCollection("Detector", "Collection");
  MC2DStripHit* First = new MC2DStripHit();
  First->SetEnergy(511.0*keV);
  MC2DStripHit* Second = new MC2DStripHit();
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


bool UTC2DStripHit::TestGetCalibrated()
{
  bool Passed = true;

  MC2DStripHitAccess Hit;
  Fill(Hit, 1275.0*keV, 10.0, G4ThreeVector(10.0*mm, 20.0*mm, -30.0*mm), 0.125*s, 6, 1, 2, {});
  MSimHT* HT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "strip hit", "A calibrated hit is returned", HT != nullptr) && Passed;
  if (HT != nullptr) {
    Passed = Evaluate("GetCalibrated()", "strip hit", "The detector type is 2D strip", HT->GetDetectorType(), (int) MDDetector::c_Strip2D) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "strip hit", "The energy is given in keV", HT->GetEnergy(), 1275.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "strip hit", "The x position is given in cm", HT->GetPosition().X(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "strip hit", "The z position is given in cm", HT->GetPosition().Z(), -3.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "strip hit", "The time is given in seconds", HT->GetTime(), 0.125, 1e-9) && Passed;
    delete HT;
  }
  MSimGR* NoGuardring = Hit.GetGuardringCalibrated();
  Passed = EvaluateTrue("GetGuardringCalibrated()", "strip hit", "A hit which is not in the guard ring has no guard ring hit", NoGuardring == nullptr) && Passed;
  delete NoGuardring;

  // Guard ring hits:
  Hit.SetIsGuardringHit(true);
  MSimHT* NoHT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "guard ring hit", "A guard ring hit has no calibrated hit", NoHT == nullptr) && Passed;
  delete NoHT;
  MSimGR* GR = Hit.GetGuardringCalibrated();
  Passed = EvaluateTrue("GetGuardringCalibrated()", "guard ring hit", "A guard ring hit has a calibrated guard ring hit", GR != nullptr) && Passed;
  if (GR != nullptr) {
    Passed = Evaluate("GetGuardringCalibrated()", "guard ring hit", "The detector type is passed on", GR->GetDetectorType(), (int) MDDetector::c_Strip2D) && Passed;
    Passed = EvaluateNear("GetGuardringCalibrated()", "guard ring hit", "The energy is given in keV", GR->GetEnergy(), 1275.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetGuardringCalibrated()", "guard ring hit", "The y position is given in cm", GR->GetPosition().Y(), 2.0, 1e-9) && Passed;
    delete GR;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTC2DStripHit::TestPrint()
{
  bool Passed = true;

  MC2DStripHitAccess Hit;
  Fill(Hit, 662.0*keV, 100.4, G4ThreeVector(10.0*mm, -25.0*mm, 30.0*mm), 0.0, 1, 3, 4, {});
  MString Text = GetPrint(Hit);
  Passed = EvaluateTrue("Print()", "strip hit", "The output contains the detector name", Text.Contains("Strips_1")) && Passed;
  Passed = EvaluateTrue("Print()", "strip hit", "The output contains the position in cm", Text.Contains("(1/-2.5/3)")) && Passed;
  Passed = EvaluateTrue("Print()", "strip hit", "The output contains the strips", Text.Contains("(3, 4)")) && Passed;
  Passed = EvaluateTrue("Print()", "strip hit", "The output contains the energy in keV", Text.Contains("662 keV")) && Passed;
  Passed = EvaluateTrue("Print()", "strip hit", "The output contains the rounded ADC counts", Text.Contains("(100)")) && Passed;
  Passed = EvaluateFalse("Print()", "strip hit", "A hit outside the guard ring is not marked as guard ring hit", Text.Contains("guardring")) && Passed;

  Hit.SetIsGuardringHit(true);
  Passed = EvaluateTrue("Print()", "guard ring hit", "A guard ring hit is marked", GetPrint(Hit).Contains("(guardring)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTC2DStripHit Test;
  return Test.Run() == true ? 0 : 1;
}
