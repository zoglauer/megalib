/*
 * UTVoxel3DHit.cc
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
#include "MCVoxel3DHit.hh"
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
#include <cstring>
#include <iostream>
#include <new>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;


//! Class exposing the protected members of the hit to the unit test
class MCVoxel3DHitAccess : public MCVoxel3DHit
{
public:
  MCVoxel3DHitAccess() : MCVoxel3DHit() {}
  using MCVoxel3DHit::m_ZVoxel;
  using MCVHit::m_Time;
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Unit test class for MCVoxel3DHit
class UTVoxel3DHit : public MUnitTest
{
public:
  //! Default constructor
  UTVoxel3DHit() : MUnitTest("UTVoxel3DHit") {}
  //! Default destructor
  virtual ~UTVoxel3DHit() {}

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
  void Fill(MCVoxel3DHitAccess& Hit, double Energy, double ADC, const G4ThreeVector& Position, double Time, unsigned int Origin, int XVoxel, int YVoxel, const vector<string>& History);
  //! Return the text printed by the hit
  MString GetPrint(MCVoxel3DHit& Hit);
};


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::Run()
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


void UTVoxel3DHit::Fill(MCVoxel3DHitAccess& Hit, double Energy, double ADC, const G4ThreeVector& Position, double Time, unsigned int Origin, int XVoxel, int YVoxel, const vector<string>& History)
{
  Hit.SetEnergy(Energy);
  Hit.SetADCCounts(ADC);
  Hit.SetPosition(Position);
  Hit.SetTime(Time);
  Hit.SetDetectorName("Voxels_1");
  Hit.SetXVoxel(XVoxel);
  Hit.SetYVoxel(YVoxel);
  Hit.SetZVoxel(5);
  Hit.AddOrigin(Origin);
  for (const string& Name: History) {
    Hit.AddVolumeHistory(Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


MString UTVoxel3DHit::GetPrint(MCVoxel3DHit& Hit)
{
  ostringstream Out;
  streambuf* Old = cout.rdbuf(Out.rdbuf());
  Hit.Print();
  cout.rdbuf(Old);
  return MString(Out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::TestDefaultConstruction()
{
  bool Passed = true;

  MCVoxel3DHitAccess Hit;
  Passed = Evaluate("MCVoxel3DHit()", "default", "The detector type is voxel", Hit.GetDetectorType(), (int) MDDetector::c_Voxel3D) && Passed;
  Passed = EvaluateNear("MCVoxel3DHit()", "default", "The default ADC counts are zero", Hit.GetADCCounts(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCVoxel3DHit()", "default", "The default x voxel is -1", Hit.GetXVoxel(), -1) && Passed;
  Passed = Evaluate("MCVoxel3DHit()", "default", "The default y voxel is -1", Hit.GetYVoxel(), -1) && Passed;
  Passed = EvaluateFalse("MCVoxel3DHit()", "default", "A new hit is not a guard ring hit", Hit.GetIsGuardringHit()) && Passed;
  Passed = EvaluateNear("MCVoxel3DHit()", "default", "The default energy is zero", Hit.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCVoxel3DHit()", "default", "The default name is Unknown", MString(Hit.GetDetectorName().c_str()), MString("Unknown")) && Passed;

  // The z voxel must be initialized like the x and the y voxel - construct the hit over memory filled with different patterns
  // If a member is not initialized, it keeps the pattern (default initialization - value initialization would zero the members first):
  alignas(MCVoxel3DHitAccess) unsigned char Zeros[sizeof(MCVoxel3DHitAccess)];
  alignas(MCVoxel3DHitAccess) unsigned char Ones[sizeof(MCVoxel3DHitAccess)];
  memset(Zeros, 0x00, sizeof(Zeros));
  memset(Ones, 0xFF, sizeof(Ones));
  MCVoxel3DHitAccess* FromZeros = ::new (Zeros) MCVoxel3DHitAccess;
  MCVoxel3DHitAccess* FromOnes = ::new (Ones) MCVoxel3DHitAccess;
  // Known failure: the constructor sets m_YVoxel twice and never initializes m_ZVoxel
  Passed = ExpectedToFail(Evaluate("MCVoxel3DHit()", "memory patterns", "The default z voxel is -1 like the x and y voxel", FromZeros->GetZVoxel(), -1)) && Passed;
  // Known failure: the constructor sets m_YVoxel twice and never initializes m_ZVoxel
  Passed = ExpectedToFail(Evaluate("MCVoxel3DHit()", "memory patterns", "The z voxel does not depend on the memory the hit is created in", FromZeros->GetZVoxel(), FromOnes->GetZVoxel())) && Passed;
  FromZeros->~MCVoxel3DHitAccess();
  FromOnes->~MCVoxel3DHitAccess();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::TestGettersSetters()
{
  bool Passed = true;

  MCVoxel3DHitAccess Hit;
  Hit.SetADCCounts(1234.5);
  Passed = EvaluateNear("SetADCCounts/GetADCCounts", "1234.5", "GetADCCounts returns the counts set before", Hit.GetADCCounts(), 1234.5, 1e-9) && Passed;
  Hit.SetXVoxel(17);
  Hit.SetYVoxel(-3);
  Passed = Evaluate("SetXVoxel/GetXVoxel", "17", "GetXVoxel returns the voxel set before", Hit.GetXVoxel(), 17) && Passed;
  Passed = Evaluate("SetYVoxel/GetYVoxel", "-3", "GetYVoxel returns the voxel set before", Hit.GetYVoxel(), -3) && Passed;
  Hit.SetZVoxel(29);
  Passed = Evaluate("SetZVoxel/GetZVoxel", "29", "GetZVoxel returns the voxel set before", Hit.GetZVoxel(), 29) && Passed;
  Hit.SetIsGuardringHit(true);
  Passed = EvaluateTrue("SetIsGuardringHit/GetIsGuardringHit", "true", "The guard ring flag is set", Hit.GetIsGuardringHit()) && Passed;
  Hit.SetIsGuardringHit(false);
  Passed = EvaluateFalse("SetIsGuardringHit/GetIsGuardringHit", "false", "The guard ring flag can be cleared", Hit.GetIsGuardringHit()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::TestCopyConstructor()
{
  bool Passed = true;

  {
    MCVoxel3DHitAccess Original;
    Fill(Original, 511.0*keV, 100.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 4.0*s, 12, 3, 4, {});
    Original.SetIsGuardringHit(true);
    MCVoxel3DHitAccess Copy(Original);
    Passed = EvaluateNear("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the energy", Copy.GetEnergy()/keV, 511.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the ADC counts", Copy.GetADCCounts(), 100.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the position", Copy.GetPosition().getY()/cm, 2.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the time", Copy.GetTime()/s, 4.0, 1e-9) && Passed;
    Passed = Evaluate("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the name", MString(Copy.GetDetectorName().c_str()), MString("Voxels_1")) && Passed;
    Passed = Evaluate("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the z voxel", Copy.GetZVoxel(), 5) && Passed;
    Passed = Evaluate("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the x voxel", Copy.GetXVoxel(), 3) && Passed;
    Passed = Evaluate("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the y voxel", Copy.GetYVoxel(), 4) && Passed;
    Passed = EvaluateTrue("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the guard ring flag", Copy.GetIsGuardringHit()) && Passed;
    Passed = EvaluateTrue("MCVoxel3DHit(const MCVoxel3DHit&)", "no history", "The copy keeps the origin", Copy.m_Origins.count(12) == 1) && Passed;
    Copy.SetEnergy(1.0*keV);
    Passed = EvaluateNear("MCVoxel3DHit(const MCVoxel3DHit&)", "independence", "Changing the copy does not change the original", Original.GetEnergy()/keV, 511.0, 1e-9) && Passed;
  }

  bool KeepsHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCVoxel3DHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Detector");
    Original.AddVolumeHistory("Layer");
    MCVoxel3DHitAccess Copy(Original);
    return Copy.m_VolumeHistory.size() == 3 && Copy.m_VolumeHistory[0] == "World" && Copy.m_VolumeHistory[2] == "Layer";
  });
  // Known failure: the copy constructor assigns into the empty volume history of the new hit instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("MCVoxel3DHit(const MCVoxel3DHit&)", "with history", "The copy of a hit with a volume history has the same volume history", KeepsHistory)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::TestAssignment()
{
  bool Passed = true;

  {
    MCVoxel3DHitAccess Original;
    Fill(Original, 662.0*keV, 10.0, G4ThreeVector(-1.0*cm, 5.0*cm, 7.0*cm), 0.5*s, 3, 6, 7, {});
    MCVoxel3DHitAccess Target;
    Target.AddOrigin(99);
    Target = Original;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the energy", Target.GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the ADC counts", Target.GetADCCounts(), 10.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the position", Target.GetPosition().getZ()/cm, 7.0, 1e-9) && Passed;
    Passed = Evaluate("operator=", "no history", "The assigned hit has the voxels", Target.GetXVoxel()*100 + Target.GetYVoxel(), 607) && Passed;
    Passed = Evaluate("operator=", "no history", "The assigned hit has the z voxel", Target.GetZVoxel(), 5) && Passed;
    Passed = EvaluateSize("operator=", "no history", "The origins of the assigned hit are replaced, not merged", Target.m_Origins.size(), 1) && Passed;
  }

  bool IntoFresh = UTSDHitShared::RunsCleanlyInChild([]() {
    MCVoxel3DHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Layer");
    MCVoxel3DHitAccess Target;
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[1] == "Layer";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into fresh hit", "Assigning a hit with a volume history gives the same volume history", IntoFresh)) && Passed;

  bool ReplacesHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCVoxel3DHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Layer");
    MCVoxel3DHitAccess Target;
    Target.AddVolumeHistory("Old1");
    Target.AddVolumeHistory("Old2");
    Target.AddVolumeHistory("Old3");
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[0] == "World" && Target.m_VolumeHistory[1] == "Layer";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into used hit", "Assigning replaces the old volume history", ReplacesHistory)) && Passed;

  bool SelfAssignment = UTSDHitShared::RunsCleanlyInChild([]() {
    MCVoxel3DHitAccess Hit;
    Hit.SetEnergy(10.0*keV);
    Hit.AddVolumeHistory("World");
    Hit.AddVolumeHistory("Layer");
    Hit = Hit;
    return Hit.m_VolumeHistory.size() == 2 && Hit.GetEnergy() == 10.0*keV;
  });
  // Known failure: operator= has no self-assignment check and resizes its own volume history to zero before copying it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "self assignment", "Assigning a hit to itself keeps its data", SelfAssignment)) && Passed;

  MCVoxel3DHitAccess A;
  MCVoxel3DHitAccess B;
  MCVoxel3DHitAccess C;
  A.SetEnergy(5.0*keV);
  const MCVoxel3DHit& Result = (B = A);
  Passed = EvaluateTrue("operator=", "chaining", "The assignment returns the assigned hit", &Result == &B) && Passed;
  C = B = A;
  Passed = EvaluateNear("operator=", "chaining", "A chained assignment reaches the last hit", C.GetEnergy()/keV, 5.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::TestEqualOperator()
{
  bool Passed = true;

  MCVoxel3DHitAccess A;
  MCVoxel3DHitAccess B;
  Fill(A, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "Detector" });
  Fill(B, 900.0*keV, 5.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 2.0*s, 2, 3, 4, { "World", "Detector" });
  Passed = EvaluateTrue("operator==", "same voxels", "Hits in the same voxels of the same detector are equal, energy and position are not compared", (A == B) != 0) && Passed;

  MCVoxel3DHitAccess C;
  Fill(C, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 5, 4, { "World", "Detector" });
  Passed = EvaluateFalse("operator==", "different x voxel", "Hits in a different x voxel are not equal", (A == C) != 0) && Passed;
  MCVoxel3DHitAccess D;
  Fill(D, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 9, { "World", "Detector" });
  Passed = EvaluateFalse("operator==", "different y voxel", "Hits in a different y voxel are not equal", (A == D) != 0) && Passed;
  MCVoxel3DHitAccess Z;
  Fill(Z, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "Detector" });
  Z.SetZVoxel(6);
  Passed = EvaluateFalse("operator==", "different z voxel", "Hits in a different z voxel are not equal", (A == Z) != 0) && Passed;
  MCVoxel3DHitAccess G1;
  Fill(G1, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 1, 1, { "World", "Detector" });
  G1.SetIsGuardringHit(true);
  MCVoxel3DHitAccess G2;
  Fill(G2, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 8, 9, { "World", "Detector" });
  G2.SetIsGuardringHit(true);
  Passed = EvaluateTrue("operator==", "guard ring hits", "Guard ring hits in the same detector are equal independent of their voxels", (G1 == G2) != 0) && Passed;
  MCVoxel3DHitAccess E;
  Fill(E, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "OtherDetector" });
  Passed = EvaluateFalse("operator==", "different detector", "Hits in the same voxels of another detector are not equal", (A == E) != 0) && Passed;

  bool LongerOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCVoxel3DHitAccess Short;
    Short.AddVolumeHistory("World");
    MCVoxel3DHitAccess Long;
    Long.AddVolumeHistory("World");
    Long.AddVolumeHistory("Detector");
    return (Short == Long) == 0;
  });
  Passed = EvaluateTrue("operator==", "other has a longer history", "A hit is not equal to a hit with a longer volume history", LongerOther) && Passed;

  bool EmptyOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCVoxel3DHitAccess Empty;
    MCVoxel3DHitAccess Long;
    Long.AddVolumeHistory("World");
    return (Long == Empty) == 0 && (Empty == Long) == 0;
  });
  // Known failure: operator== does not compare the history sizes and reads the other hit's empty history out of bounds
  Passed = ExpectedToFail(EvaluateTrue("operator==", "one empty history", "A hit with and a hit without volume history are not equal", EmptyOther)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::TestAddOperator()
{
  bool Passed = true;

  // Same voxels - the position is the energy weighted average:
  {
    MCVoxel3DHitAccess A;
    Fill(A, 100.0*keV, 10.0, G4ThreeVector(0.0, 0.0, 0.0), 5.0*s, 1, 3, 4, { "World", "Detector" });
    MCVoxel3DHitAccess B;
    Fill(B, 300.0*keV, 30.0, G4ThreeVector(4.0*cm, 8.0*cm, 12.0*cm), 3.0*s, 2, 3, 4, { "World", "Detector" });
    const MCVoxel3DHit& Result = (A += B);
    Passed = EvaluateTrue("operator+=", "same voxels", "The operator returns the hit it was called on", &Result == &A) && Passed;
    Passed = EvaluateNear("operator+=", "same voxels", "The energies are added", A.GetEnergy()/keV, 400.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same voxels", "The ADC counts are added", A.GetADCCounts(), 40.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same voxels", "The x position is the energy weighted average", A.GetPosition().x()/cm, 3.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same voxels", "The y position is the energy weighted average", A.GetPosition().y()/cm, 6.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same voxels", "The z position is the energy weighted average", A.GetPosition().z()/cm, 9.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same voxels", "The earlier time is kept", A.GetTime()/s, 3.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator+=", "same voxels", "The origins are merged", A.m_Origins.size(), 2) && Passed;
  }

  // Different voxels or detectors are not added:
  {
    MCVoxel3DHitAccess A;
    Fill(A, 100.0*keV, 10.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 5.0*s, 1, 3, 4, { "World", "Detector" });
    MCVoxel3DHitAccess OtherStrip;
    Fill(OtherStrip, 250.0*keV, 30.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 5, { "World", "Detector" });
    A += OtherStrip;
    Passed = EvaluateNear("operator+=", "different voxel", "Hits in different voxels keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    MCVoxel3DHitAccess OtherZ;
    Fill(OtherZ, 250.0*keV, 30.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 4, { "World", "Detector" });
    OtherZ.SetZVoxel(6);
    A += OtherZ;
    Passed = EvaluateNear("operator+=", "different z voxel", "Hits in different z voxels keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    MCVoxel3DHitAccess OtherDetector;
    Fill(OtherDetector, 250.0*keV, 30.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 4, { "World", "Detector2" });
    A += OtherDetector;
    Passed = EvaluateNear("operator+=", "different detector", "Hits in different detectors keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    MCVoxel3DHitAccess OtherLength;
    Fill(OtherLength, 250.0*keV, 30.0, G4ThreeVector(), 3.0*s, 2, 3, 4, { "World" });
    A += OtherLength;
    Passed = EvaluateNear("operator+=", "different history length", "Hits with a different history length keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  }

  // Guard ring hits are added without moving the position:
  {
    MCVoxel3DHitAccess A;
    Fill(A, 100.0*keV, 10.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 5.0*s, 1, -1, -1, { "World", "Detector" });
    A.SetIsGuardringHit(true);
    MCVoxel3DHitAccess B;
    Fill(B, 300.0*keV, 30.0, G4ThreeVector(9.0*cm, 9.0*cm, 9.0*cm), 3.0*s, 2, -1, -1, { "World", "Detector" });
    B.SetIsGuardringHit(true);
    A += B;
    Passed = EvaluateNear("operator+=", "guard rings", "The energies of guard ring hits are added", A.GetEnergy()/keV, 400.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "guard rings", "The ADC counts of guard ring hits are added", A.GetADCCounts(), 40.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "guard rings", "The position of guard ring hits stays the same", A.GetPosition().getX()/cm, 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "guard rings", "The earlier time of guard ring hits is kept", A.GetTime()/s, 3.0, 1e-9) && Passed;
  }

  // Hits without energy have a defined position:
  {
    MCVoxel3DHitAccess Zero1;
    Fill(Zero1, 0.0, 0.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 1.0*s, 1, 1, 1, {});
    MCVoxel3DHitAccess Zero2;
    Fill(Zero2, 0.0, 0.0, G4ThreeVector(3.0*cm, 4.0*cm, 5.0*cm), 1.0*s, 2, 1, 1, {});
    Zero1 += Zero2;
    // Known failure: operator+= divides the energy weighted position by the energy sum, which is zero here
    Passed = ExpectedToFail(EvaluateTrue("operator+=", "zero energies", "Adding two hits without energy does not give a position which is not a number", std::isfinite(Zero1.GetPosition().x()) && std::isfinite(Zero1.GetPosition().y()) && std::isfinite(Zero1.GetPosition().z()))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::TestAllocationAndCollection()
{
  bool Passed = true;

  vector<MCVoxel3DHit*> Hits;
  for (unsigned int i = 0; i < 1000; ++i) {
    MCVoxel3DHit* Hit = new MCVoxel3DHit();
    Hit->SetEnergy((i + 1)*keV);
    Hits.push_back(Hit);
  }
  set<MCVoxel3DHit*> Addresses(Hits.begin(), Hits.end());
  Passed = EvaluateSize("operator new", "1000 hits", "All allocated hits have different addresses", Addresses.size(), 1000) && Passed;
  bool AllKept = true;
  for (unsigned int i = 0; i < Hits.size(); ++i) {
    if (Hits[i]->GetEnergy() != (i + 1)*keV) AllKept = false;
  }
  Passed = EvaluateTrue("operator new", "1000 hits", "Every allocated hit keeps its own energy", AllKept) && Passed;
  for (MCVoxel3DHit* Hit: Hits) delete Hit;

  MCVoxel3DHit* Reused = new MCVoxel3DHit();
  Passed = EvaluateNear("operator new", "reuse", "A new hit after freeing others is initialized", Reused->GetEnergy(), 0.0, 1e-12) && Passed;
  delete Reused;

  MCVoxel3DHitsCollection* Collection = new MCVoxel3DHitsCollection("Detector", "Collection");
  MCVoxel3DHit* First = new MCVoxel3DHit();
  First->SetEnergy(511.0*keV);
  MCVoxel3DHit* Second = new MCVoxel3DHit();
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


bool UTVoxel3DHit::TestGetCalibrated()
{
  bool Passed = true;

  MCVoxel3DHitAccess Hit;
  Fill(Hit, 1275.0*keV, 10.0, G4ThreeVector(10.0*mm, 20.0*mm, -30.0*mm), 0.125*s, 6, 1, 2, {});
  MSimHT* HT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "voxel hit", "A calibrated hit is returned", HT != nullptr) && Passed;
  if (HT != nullptr) {
    Passed = Evaluate("GetCalibrated()", "voxel hit", "The detector type is voxel", HT->GetDetectorType(), (int) MDDetector::c_Voxel3D) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "voxel hit", "The energy is given in keV", HT->GetEnergy(), 1275.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "voxel hit", "The x position is given in cm", HT->GetPosition().X(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "voxel hit", "The z position is given in cm", HT->GetPosition().Z(), -3.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "voxel hit", "The time is given in seconds", HT->GetTime(), 0.125, 1e-9) && Passed;
    delete HT;
  }
  MSimGR* NoGuardring = Hit.GetGuardringCalibrated();
  Passed = EvaluateTrue("GetGuardringCalibrated()", "voxel hit", "A hit which is not in the guard ring has no guard ring hit", NoGuardring == nullptr) && Passed;
  delete NoGuardring;

  // Guard ring hits:
  Hit.SetIsGuardringHit(true);
  MSimHT* NoHT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "guard ring hit", "A guard ring hit has no calibrated hit", NoHT == nullptr) && Passed;
  delete NoHT;
  MSimGR* GR = Hit.GetGuardringCalibrated();
  Passed = EvaluateTrue("GetGuardringCalibrated()", "guard ring hit", "A guard ring hit has a calibrated guard ring hit", GR != nullptr) && Passed;
  if (GR != nullptr) {
    Passed = Evaluate("GetGuardringCalibrated()", "guard ring hit", "The detector type is passed on", GR->GetDetectorType(), (int) MDDetector::c_Voxel3D) && Passed;
    Passed = EvaluateNear("GetGuardringCalibrated()", "guard ring hit", "The energy is given in keV", GR->GetEnergy(), 1275.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetGuardringCalibrated()", "guard ring hit", "The y position is given in cm", GR->GetPosition().Y(), 2.0, 1e-9) && Passed;
    delete GR;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVoxel3DHit::TestPrint()
{
  bool Passed = true;

  MCVoxel3DHitAccess Hit;
  Fill(Hit, 662.0*keV, 100.4, G4ThreeVector(10.0*mm, -25.0*mm, 30.0*mm), 0.0, 1, 3, 4, {});
  MString Text = GetPrint(Hit);
  Passed = EvaluateTrue("Print()", "voxel hit", "The output contains the detector name", Text.Contains("Voxels_1")) && Passed;
  Passed = EvaluateTrue("Print()", "voxel hit", "The output contains the position in cm", Text.Contains("(1/-2.5/3)")) && Passed;
  Passed = EvaluateTrue("Print()", "voxel hit", "The output contains the x and y voxel", Text.Contains("(3, 4)")) && Passed;
  Passed = EvaluateTrue("Print()", "voxel hit", "The output contains the energy in keV", Text.Contains("662 keV")) && Passed;
  Passed = EvaluateTrue("Print()", "voxel hit", "The output contains the rounded ADC counts", Text.Contains("(100)")) && Passed;
  Passed = EvaluateFalse("Print()", "voxel hit", "A hit outside the guard ring is not marked as guard ring hit", Text.Contains("guardring")) && Passed;

  Hit.SetIsGuardringHit(true);
  Passed = EvaluateTrue("Print()", "guard ring hit", "A guard ring hit is marked", GetPrint(Hit).Contains("(guardring)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTVoxel3DHit Test;
  return Test.Run() == true ? 0 : 1;
}
