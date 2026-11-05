/*
 * UTDriftChamberHit.cc
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
#include "MCDriftChamberHit.hh"
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
class MCDriftChamberHitAccess : public MCDriftChamberHit
{
public:
  using MCVHit::m_Time;
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Unit test class for MCDriftChamberHit
class UTDriftChamberHit : public MUnitTest
{
public:
  //! Default constructor
  UTDriftChamberHit() : MUnitTest("UTDriftChamberHit") {}
  //! Default destructor
  virtual ~UTDriftChamberHit() {}

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

  //! Fill a hit with a representative content
  void Fill(MCDriftChamberHitAccess& Hit, double Energy, double ADC, const G4ThreeVector& Position, double Time, unsigned int Origin, int XStrip, int YStrip, const vector<string>& History);
  //! Return the text printed by the hit
  MString GetPrint(MCDriftChamberHit& Hit);
};


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::Run()
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


void UTDriftChamberHit::Fill(MCDriftChamberHitAccess& Hit, double Energy, double ADC, const G4ThreeVector& Position, double Time, unsigned int Origin, int XStrip, int YStrip, const vector<string>& History)
{
  Hit.SetEnergy(Energy);
  Hit.SetADCCounts(ADC);
  Hit.SetPosition(Position);
  Hit.SetTime(Time);
  Hit.SetDetectorName("Drift_1");
  Hit.SetXStrip(XStrip);
  Hit.SetYStrip(YStrip);
  Hit.AddOrigin(Origin);
  for (const string& Name: History) {
    Hit.AddVolumeHistory(Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


MString UTDriftChamberHit::GetPrint(MCDriftChamberHit& Hit)
{
  ostringstream Out;
  streambuf* Old = cout.rdbuf(Out.rdbuf());
  Hit.Print();
  cout.rdbuf(Old);
  return MString(Out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::TestDefaultConstruction()
{
  bool Passed = true;

  MCDriftChamberHitAccess Hit;
  Passed = EvaluateNear("MCDriftChamberHit()", "default", "The default energy is zero", Hit.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCDriftChamberHit()", "default", "The default name is Unknown", MString(Hit.GetDetectorName().c_str()), MString("Unknown")) && Passed;
  Passed = EvaluateFalse("MCDriftChamberHit()", "default", "A new hit is not a guard ring hit", Hit.GetIsGuardringHit()) && Passed;

  // The strips and the ADC counts of the hit must be initialized - construct the hit over memory filled with different patterns
  // If a member is not initialized, it keeps the pattern:
  alignas(MCDriftChamberHitAccess) unsigned char Zeros[sizeof(MCDriftChamberHitAccess)];
  alignas(MCDriftChamberHitAccess) unsigned char Ones[sizeof(MCDriftChamberHitAccess)];
  memset(Zeros, 0x00, sizeof(Zeros));
  memset(Ones, 0xFF, sizeof(Ones));
  // Default initialization - value initialization would zero the members first:
  MCDriftChamberHitAccess* FromZeros = ::new (Zeros) MCDriftChamberHitAccess;
  MCDriftChamberHitAccess* FromOnes = ::new (Ones) MCDriftChamberHitAccess;
  // Known failure: MCDriftChamberHit redeclares m_XStrip, which shadows the initialized member of MC2DStripHit and stays uninitialized
  Passed = ExpectedToFail(Evaluate("MCDriftChamberHit()", "memory patterns", "The x strip does not depend on the memory the hit is created in", FromZeros->GetXStrip(), FromOnes->GetXStrip())) && Passed;
  // Known failure: MCDriftChamberHit redeclares m_YStrip, which shadows the initialized member of MC2DStripHit and stays uninitialized
  Passed = ExpectedToFail(Evaluate("MCDriftChamberHit()", "memory patterns", "The y strip does not depend on the memory the hit is created in", FromZeros->GetYStrip(), FromOnes->GetYStrip())) && Passed;
  // Known failure: MCDriftChamberHit redeclares m_ADCCounts, which shadows the initialized member of MC2DStripHit and stays uninitialized
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberHit()", "memory patterns", "The ADC counts are a number after construction", std::isfinite(FromOnes->GetADCCounts()) == true && std::isfinite(FromZeros->GetADCCounts()) == true)) && Passed;
  // Known failure: MCDriftChamberHit redeclares m_ADCCounts, which shadows the initialized member of MC2DStripHit and stays uninitialized
  Passed = ExpectedToFail(EvaluateNear("MCDriftChamberHit()", "memory patterns", "The ADC counts do not depend on the memory the hit is created in", FromOnes->GetADCCounts(), FromZeros->GetADCCounts(), 1e-12)) && Passed;
  FromZeros->~MCDriftChamberHitAccess();
  FromOnes->~MCDriftChamberHitAccess();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::TestGettersSetters()
{
  bool Passed = true;

  MCDriftChamberHitAccess Hit;
  Hit.SetADCCounts(1234.5);
  Passed = EvaluateNear("SetADCCounts/GetADCCounts", "1234.5", "GetADCCounts returns the counts set before", Hit.GetADCCounts(), 1234.5, 1e-9) && Passed;
  Hit.SetXStrip(17);
  Hit.SetYStrip(-3);
  Passed = Evaluate("SetXStrip/GetXStrip", "17", "GetXStrip returns the strip set before", Hit.GetXStrip(), 17) && Passed;
  Passed = Evaluate("SetYStrip/GetYStrip", "-3", "GetYStrip returns the strip set before", Hit.GetYStrip(), -3) && Passed;
  Hit.SetIsGuardringHit(true);
  Passed = EvaluateTrue("SetIsGuardringHit/GetIsGuardringHit", "true", "The guard ring flag is set", Hit.GetIsGuardringHit()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::TestCopyConstructor()
{
  bool Passed = true;

  // The strips and the ADC counts of the drift chamber hit must be copied - construct the copy over memory filled with a pattern, so that not copied members are detected:
  {
    MCDriftChamberHitAccess Original;
    Fill(Original, 511.0*keV, 100.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 4.0*s, 12, 3, 4, {});
    alignas(MCDriftChamberHitAccess) unsigned char Buffer[sizeof(MCDriftChamberHitAccess)];
    memset(Buffer, 0xFF, sizeof(Buffer));
    MCDriftChamberHitAccess* Copy = ::new (Buffer) MCDriftChamberHitAccess(Original);
    Passed = EvaluateNear("MCDriftChamberHit(const MCDriftChamberHit&)", "no history", "The copy keeps the energy", Copy->GetEnergy()/keV, 511.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCDriftChamberHit(const MCDriftChamberHit&)", "no history", "The copy keeps the position", Copy->GetPosition().getY()/cm, 2.0, 1e-9) && Passed;
    Passed = Evaluate("MCDriftChamberHit(const MCDriftChamberHit&)", "no history", "The copy keeps the name", MString(Copy->GetDetectorName().c_str()), MString("Drift_1")) && Passed;
    // Known failure: the copy constructor copies only the members of MC2DStripHit, not the shadowing m_XStrip of MCDriftChamberHit
    Passed = ExpectedToFail(Evaluate("MCDriftChamberHit(const MCDriftChamberHit&)", "no history", "The copy keeps the x strip", Copy->GetXStrip(), 3)) && Passed;
    // Known failure: the copy constructor copies only the members of MC2DStripHit, not the shadowing m_YStrip of MCDriftChamberHit
    Passed = ExpectedToFail(Evaluate("MCDriftChamberHit(const MCDriftChamberHit&)", "no history", "The copy keeps the y strip", Copy->GetYStrip(), 4)) && Passed;
    // Known failure: the copy constructor copies only the members of MC2DStripHit, not the shadowing m_ADCCounts of MCDriftChamberHit
    Passed = ExpectedToFail(EvaluateNear("MCDriftChamberHit(const MCDriftChamberHit&)", "no history", "The copy keeps the ADC counts", Copy->GetADCCounts(), 100.0, 1e-9)) && Passed;
    Passed = EvaluateTrue("MCDriftChamberHit(const MCDriftChamberHit&)", "no history", "The copy keeps the origin", Copy->m_Origins.count(12) == 1) && Passed;
    Copy->~MCDriftChamberHitAccess();
  }

  bool KeepsHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCDriftChamberHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Drift");
    Original.AddVolumeHistory("Layer");
    MCDriftChamberHitAccess Copy(Original);
    return Copy.m_VolumeHistory.size() == 3 && Copy.m_VolumeHistory[0] == "World" && Copy.m_VolumeHistory[2] == "Layer";
  });
  // Known failure: the copy constructor of MC2DStripHit assigns into the empty volume history of the new hit instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("MCDriftChamberHit(const MCDriftChamberHit&)", "with history", "The copy of a hit with a volume history has the same volume history", KeepsHistory)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::TestAssignment()
{
  bool Passed = true;

  {
    MCDriftChamberHitAccess Original;
    Fill(Original, 662.0*keV, 10.0, G4ThreeVector(-1.0*cm, 5.0*cm, 7.0*cm), 0.5*s, 3, 6, 7, {});
    MCDriftChamberHitAccess Target;
    Fill(Target, 1.0*keV, 99.0, G4ThreeVector(), 0.0, 99, -5, -6, {});
    Target = Original;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the energy", Target.GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the position", Target.GetPosition().getZ()/cm, 7.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator=", "no history", "The origins of the assigned hit are replaced, not merged", Target.m_Origins.size(), 1) && Passed;
    // Known failure: operator= calls only MC2DStripHit::operator=, which leaves the shadowing m_XStrip of MCDriftChamberHit unchanged
    Passed = ExpectedToFail(Evaluate("operator=", "no history", "The assigned hit has the x strip", Target.GetXStrip(), 6)) && Passed;
    // Known failure: operator= calls only MC2DStripHit::operator=, which leaves the shadowing m_YStrip of MCDriftChamberHit unchanged
    Passed = ExpectedToFail(Evaluate("operator=", "no history", "The assigned hit has the y strip", Target.GetYStrip(), 7)) && Passed;
    // Known failure: operator= calls only MC2DStripHit::operator=, which leaves the shadowing m_ADCCounts of MCDriftChamberHit unchanged
    Passed = ExpectedToFail(EvaluateNear("operator=", "no history", "The assigned hit has the ADC counts", Target.GetADCCounts(), 10.0, 1e-9)) && Passed;
  }

  bool IntoFresh = UTSDHitShared::RunsCleanlyInChild([]() {
    MCDriftChamberHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Layer");
    MCDriftChamberHitAccess Target;
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[1] == "Layer";
  });
  // Known failure: MC2DStripHit::operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into fresh hit", "Assigning a hit with a volume history gives the same volume history", IntoFresh)) && Passed;

  bool ReplacesHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCDriftChamberHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Layer");
    MCDriftChamberHitAccess Target;
    Target.AddVolumeHistory("Old1");
    Target.AddVolumeHistory("Old2");
    Target.AddVolumeHistory("Old3");
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[0] == "World" && Target.m_VolumeHistory[1] == "Layer";
  });
  // Known failure: MC2DStripHit::operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into used hit", "Assigning replaces the old volume history", ReplacesHistory)) && Passed;

  bool SelfAssignment = UTSDHitShared::RunsCleanlyInChild([]() {
    MCDriftChamberHitAccess Hit;
    Hit.SetEnergy(10.0*keV);
    Hit.AddVolumeHistory("World");
    Hit.AddVolumeHistory("Layer");
    Hit = Hit;
    return Hit.m_VolumeHistory.size() == 2 && Hit.GetEnergy() == 10.0*keV;
  });
  // Known failure: MC2DStripHit::operator= has no self-assignment check and resizes its own volume history to zero before copying it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "self assignment", "Assigning a hit to itself keeps its data", SelfAssignment)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::TestEqualOperator()
{
  bool Passed = true;

  MCDriftChamberHitAccess A;
  MCDriftChamberHitAccess B;
  Fill(A, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "Drift" });
  Fill(B, 900.0*keV, 5.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 2.0*s, 2, 3, 4, { "World", "Drift" });
  Passed = EvaluateTrue("operator==", "same strips", "Hits in the same strips of the same volume are equal, energy and position are not compared", (A == B) != 0) && Passed;

  MCDriftChamberHitAccess C;
  Fill(C, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 5, 4, { "World", "Drift" });
  Passed = EvaluateFalse("operator==", "different x strip", "Hits in a different x strip are not equal", (A == C) != 0) && Passed;
  MCDriftChamberHitAccess D;
  Fill(D, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 9, { "World", "Drift" });
  Passed = EvaluateFalse("operator==", "different y strip", "Hits in a different y strip are not equal", (A == D) != 0) && Passed;
  MCDriftChamberHitAccess E;
  Fill(E, 1.0*keV, 1.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "OtherDrift" });
  Passed = EvaluateFalse("operator==", "different volume", "Hits in the same strips of another volume are not equal", (A == E) != 0) && Passed;

  bool LongerOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCDriftChamberHitAccess Short;
    Short.SetXStrip(3);
    Short.SetYStrip(4);
    Short.AddVolumeHistory("World");
    MCDriftChamberHitAccess Long;
    Long.SetXStrip(3);
    Long.SetYStrip(4);
    Long.AddVolumeHistory("World");
    Long.AddVolumeHistory("Drift");
    return (Short == Long) == 0;
  });
  // Known failure: operator== compares only as many volumes as the own history has and ignores the longer history of the other hit
  Passed = ExpectedToFail(EvaluateTrue("operator==", "other has a longer history", "A hit is not equal to a hit with a longer volume history", LongerOther)) && Passed;

  bool EmptyOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCDriftChamberHitAccess Empty;
    Empty.SetXStrip(3);
    Empty.SetYStrip(4);
    MCDriftChamberHitAccess Long;
    Long.SetXStrip(3);
    Long.SetYStrip(4);
    Long.AddVolumeHistory("World");
    return (Long == Empty) == 0 && (Empty == Long) == 0;
  });
  // Known failure: operator== does not compare the history sizes and reads the other hit's empty history out of bounds
  Passed = ExpectedToFail(EvaluateTrue("operator==", "one empty history", "A hit with and a hit without volume history are not equal", EmptyOther)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::TestAddOperator()
{
  bool Passed = true;

  {
    MCDriftChamberHitAccess A;
    Fill(A, 100.0*keV, 10.0, G4ThreeVector(0.0, 0.0, 0.0), 5.0*s, 1, 3, 4, { "World", "Drift" });
    MCDriftChamberHitAccess B;
    Fill(B, 300.0*keV, 30.0, G4ThreeVector(4.0*cm, 8.0*cm, 12.0*cm), 3.0*s, 2, 3, 4, { "World", "Drift" });
    const MCDriftChamberHit& Result = (A += B);
    Passed = EvaluateTrue("operator+=", "same strips", "The operator returns the hit it was called on", &Result == &A) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The energies are added", A.GetEnergy()/keV, 400.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The ADC counts are added", A.GetADCCounts(), 40.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The x position is the energy weighted average", A.GetPosition().x()/cm, 3.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The y position is the energy weighted average", A.GetPosition().y()/cm, 6.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The z position is the energy weighted average", A.GetPosition().z()/cm, 9.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same strips", "The earlier time is kept", A.GetTime()/s, 3.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator+=", "same strips", "The origins are merged", A.m_Origins.size(), 2) && Passed;
  }

  {
    MCDriftChamberHitAccess A;
    Fill(A, 100.0*keV, 10.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 5.0*s, 1, 3, 4, { "World", "Drift" });
    MCDriftChamberHitAccess OtherStrip;
    Fill(OtherStrip, 250.0*keV, 30.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 5, { "World", "Drift" });
    A += OtherStrip;
    Passed = EvaluateNear("operator+=", "different strip", "Hits in different strips keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    MCDriftChamberHitAccess OtherVolume;
    Fill(OtherVolume, 250.0*keV, 30.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 4, { "World", "Drift2" });
    A += OtherVolume;
    Passed = EvaluateNear("operator+=", "different volume", "Hits in different volumes keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  }

  {
    MCDriftChamberHitAccess Zero1;
    Fill(Zero1, 0.0, 0.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 1.0*s, 1, 1, 1, {});
    MCDriftChamberHitAccess Zero2;
    Fill(Zero2, 0.0, 0.0, G4ThreeVector(3.0*cm, 4.0*cm, 5.0*cm), 1.0*s, 2, 1, 1, {});
    Zero1 += Zero2;
    // Known failure: operator+= divides the energy weighted position by the energy sum, which is zero here
    Passed = ExpectedToFail(EvaluateTrue("operator+=", "zero energies", "Adding two hits without energy does not give a position which is not a number", std::isfinite(Zero1.GetPosition().x()) && std::isfinite(Zero1.GetPosition().y()) && std::isfinite(Zero1.GetPosition().z()))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::TestAllocationAndCollection()
{
  bool Passed = true;

  vector<MCDriftChamberHit*> Hits;
  for (unsigned int i = 0; i < 1000; ++i) {
    MCDriftChamberHit* Hit = new MCDriftChamberHit();
    Hit->SetEnergy((i + 1)*keV);
    Hits.push_back(Hit);
  }
  set<MCDriftChamberHit*> Addresses(Hits.begin(), Hits.end());
  Passed = EvaluateSize("operator new", "1000 hits", "All allocated hits have different addresses", Addresses.size(), 1000) && Passed;
  bool AllKept = true;
  for (unsigned int i = 0; i < Hits.size(); ++i) {
    if (Hits[i]->GetEnergy() != (i + 1)*keV) AllKept = false;
  }
  Passed = EvaluateTrue("operator new", "1000 hits", "Every allocated hit keeps its own energy", AllKept) && Passed;
  for (MCDriftChamberHit* Hit: Hits) delete Hit;

  MCDriftChamberHitsCollection* Collection = new MCDriftChamberHitsCollection("Detector", "Collection");
  MCDriftChamberHit* First = new MCDriftChamberHit();
  First->SetEnergy(511.0*keV);
  MCDriftChamberHit* Second = new MCDriftChamberHit();
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


bool UTDriftChamberHit::TestGetCalibrated()
{
  bool Passed = true;

  MCDriftChamberHitAccess Hit;
  Fill(Hit, 1275.0*keV, 10.0, G4ThreeVector(10.0*mm, 20.0*mm, -30.0*mm), 0.125*s, 6, 1, 2, {});
  MSimHT* HT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "drift chamber hit", "A calibrated hit is returned", HT != nullptr) && Passed;
  if (HT != nullptr) {
    Passed = EvaluateNear("GetCalibrated()", "drift chamber hit", "The energy is given in keV", HT->GetEnergy(), 1275.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "drift chamber hit", "The x position is given in cm", HT->GetPosition().X(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "drift chamber hit", "The z position is given in cm", HT->GetPosition().Z(), -3.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "drift chamber hit", "The time is given in seconds", HT->GetTime(), 0.125, 1e-9) && Passed;
    Passed = Evaluate("GetCalibrated()", "drift chamber hit", "The origin is passed on", HT->GetNOrigins(), 1U) && Passed;
    delete HT;
  }

  Hit.SetIsGuardringHit(true);
  MSimHT* NoHT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "guard ring hit", "A guard ring hit has no calibrated hit", NoHT == nullptr) && Passed;
  delete NoHT;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDriftChamberHit::TestPrint()
{
  bool Passed = true;

  MCDriftChamberHitAccess Hit;
  Fill(Hit, 662.0*keV, 100.4, G4ThreeVector(10.0*mm, -25.0*mm, 30.0*mm), 0.0, 1, 3, 4, {});
  MString Text = GetPrint(Hit);
  Passed = EvaluateTrue("Print()", "drift chamber hit", "The output contains the detector name", Text.Contains("Drift_1")) && Passed;
  Passed = EvaluateTrue("Print()", "drift chamber hit", "The output contains the position in cm", Text.Contains("(1/-2.5/3)")) && Passed;
  Passed = EvaluateTrue("Print()", "drift chamber hit", "The output contains the strips", Text.Contains("(3, 4)")) && Passed;
  Passed = EvaluateTrue("Print()", "drift chamber hit", "The output contains the energy in keV", Text.Contains("662 keV")) && Passed;
  Passed = EvaluateTrue("Print()", "drift chamber hit", "The output contains the rounded ADC counts", Text.Contains("(100)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDriftChamberHit Test;
  return Test.Run() == true ? 0 : 1;
}
