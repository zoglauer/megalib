/*
 * UTCalorBarHit.cc
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
#include "MCCalorBarHit.hh"
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
class MCCalorBarHitAccess : public MCCalorBarHit
{
public:
  using MCVHit::m_Time;
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Unit test class for MCCalorBarHit
class UTCalorBarHit : public MUnitTest
{
public:
  //! Default constructor
  UTCalorBarHit() : MUnitTest("UTCalorBarHit") {}
  //! Default destructor
  virtual ~UTCalorBarHit() {}

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
  void Fill(MCCalorBarHitAccess& Hit, double Energy, double ADCTop, double ADCBottom, const G4ThreeVector& Position, double Time, unsigned int Origin, int XBar, int YBar, const vector<string>& History);
  //! Return the text printed by the hit
  MString GetPrint(MCCalorBarHit& Hit);
};


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::Run()
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


void UTCalorBarHit::Fill(MCCalorBarHitAccess& Hit, double Energy, double ADCTop, double ADCBottom, const G4ThreeVector& Position, double Time, unsigned int Origin, int XBar, int YBar, const vector<string>& History)
{
  Hit.SetEnergy(Energy);
  Hit.SetADCCounts(ADCTop, ADCBottom);
  Hit.SetPosition(Position);
  Hit.SetTime(Time);
  Hit.SetDetectorName("Calorimeter_1");
  Hit.SetXBar(XBar);
  Hit.SetYBar(YBar);
  Hit.AddOrigin(Origin);
  for (const string& Name: History) {
    Hit.AddVolumeHistory(Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


MString UTCalorBarHit::GetPrint(MCCalorBarHit& Hit)
{
  ostringstream Out;
  streambuf* Old = cout.rdbuf(Out.rdbuf());
  Hit.Print();
  cout.rdbuf(Old);
  return MString(Out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::TestDefaultConstruction()
{
  bool Passed = true;

  MCCalorBarHitAccess Hit;
  Passed = Evaluate("MCCalorBarHit()", "default", "The detector type is calorimeter", Hit.GetDetectorType(), (int) MDDetector::c_Calorimeter) && Passed;
  Passed = EvaluateNear("MCCalorBarHit()", "default", "The default ADC counts at the top are zero", Hit.GetADCCountsTop(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCCalorBarHit()", "default", "The default ADC counts at the bottom are zero", Hit.GetADCCountsBottom(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCCalorBarHit()", "default", "GetADCCounts returns the counts at the top", Hit.GetADCCounts(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCCalorBarHit()", "default", "The default energy is zero", Hit.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCCalorBarHit()", "default", "The default name is Unknown", MString(Hit.GetDetectorName().c_str()), MString("Unknown")) && Passed;
  Passed = EvaluateSize("MCCalorBarHit()", "default", "A new hit has no volume history", Hit.m_VolumeHistory.size(), 0) && Passed;
  Passed = EvaluateTrue("MCCalorBarHit()", "default", "The default bars are -1, -1", GetPrint(Hit).Contains("(-1, -1)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::TestGettersSetters()
{
  bool Passed = true;

  MCCalorBarHitAccess Hit;
  Hit.SetADCCounts(120.5, 340.25);
  Passed = EvaluateNear("SetADCCounts()", "120.5, 340.25", "The ADC counts at the top are stored", Hit.GetADCCountsTop(), 120.5, 1e-9) && Passed;
  Passed = EvaluateNear("SetADCCounts()", "120.5, 340.25", "The ADC counts at the bottom are stored", Hit.GetADCCountsBottom(), 340.25, 1e-9) && Passed;
  Passed = EvaluateNear("SetADCCounts()", "120.5, 340.25", "GetADCCounts returns the counts at the top", Hit.GetADCCounts(), 120.5, 1e-9) && Passed;

  Hit.SetADCCounts(77.0);
  Passed = EvaluateNear("SetADCCounts()", "77 only", "The counts at the top are stored", Hit.GetADCCountsTop(), 77.0, 1e-9) && Passed;
  Passed = EvaluateNear("SetADCCounts()", "77 only", "Without a value for the bottom the bottom is marked as not available with -1", Hit.GetADCCountsBottom(), -1.0, 1e-12) && Passed;

  Hit.SetXBar(3);
  Hit.SetYBar(11);
  Passed = EvaluateTrue("SetXBar/SetYBar", "3, 11", "The bars are stored", GetPrint(Hit).Contains("(3, 11)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::TestCopyConstructor()
{
  bool Passed = true;

  {
    MCCalorBarHitAccess Original;
    Fill(Original, 511.0*keV, 100.0, 200.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 4.0*s, 12, 3, 4, {});
    MCCalorBarHitAccess Copy(Original);
    Passed = EvaluateNear("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the energy", Copy.GetEnergy()/keV, 511.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the ADC counts at the top", Copy.GetADCCountsTop(), 100.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the ADC counts at the bottom", Copy.GetADCCountsBottom(), 200.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the position", Copy.GetPosition().getY()/cm, 2.0, 1e-9) && Passed;
    Passed = EvaluateNear("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the time", Copy.GetTime()/s, 4.0, 1e-9) && Passed;
    Passed = Evaluate("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the name", MString(Copy.GetDetectorName().c_str()), MString("Calorimeter_1")) && Passed;
    Passed = Evaluate("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the detector type", Copy.GetDetectorType(), (int) MDDetector::c_Calorimeter) && Passed;
    Passed = EvaluateTrue("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the bars", GetPrint(Copy).Contains("(3, 4)")) && Passed;
    Passed = EvaluateTrue("MCCalorBarHit(const MCCalorBarHit&)", "no history", "The copy keeps the origin", Copy.m_Origins.count(12) == 1) && Passed;
    Copy.SetEnergy(1.0*keV);
    Passed = EvaluateNear("MCCalorBarHit(const MCCalorBarHit&)", "independence", "Changing the copy does not change the original", Original.GetEnergy()/keV, 511.0, 1e-9) && Passed;
  }

  bool KeepsHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCCalorBarHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Calorimeter");
    Original.AddVolumeHistory("Bar");
    MCCalorBarHitAccess Copy(Original);
    return Copy.m_VolumeHistory.size() == 3 && Copy.m_VolumeHistory[0] == "World" && Copy.m_VolumeHistory[2] == "Bar";
  });
  // Known failure: the copy constructor assigns into the empty volume history of the new hit instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("MCCalorBarHit(const MCCalorBarHit&)", "with history", "The copy of a hit with a volume history has the same volume history", KeepsHistory)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::TestAssignment()
{
  bool Passed = true;

  {
    MCCalorBarHitAccess Original;
    Fill(Original, 662.0*keV, 10.0, 20.0, G4ThreeVector(-1.0*cm, 5.0*cm, 7.0*cm), 0.5*s, 3, 6, 7, {});
    MCCalorBarHitAccess Target;
    Target.AddOrigin(99);
    Target = Original;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the energy", Target.GetEnergy()/keV, 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the ADC counts at the top", Target.GetADCCountsTop(), 10.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the ADC counts at the bottom", Target.GetADCCountsBottom(), 20.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the position", Target.GetPosition().getZ()/cm, 7.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator=", "no history", "The assigned hit has the time", Target.GetTime()/s, 0.5, 1e-9) && Passed;
    Passed = EvaluateTrue("operator=", "no history", "The assigned hit has the bars", GetPrint(Target).Contains("(6, 7)")) && Passed;
    Passed = EvaluateSize("operator=", "no history", "The origins of the assigned hit are replaced, not merged", Target.m_Origins.size(), 1) && Passed;
  }

  bool IntoFresh = UTSDHitShared::RunsCleanlyInChild([]() {
    MCCalorBarHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Bar");
    MCCalorBarHitAccess Target;
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[1] == "Bar";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into fresh hit", "Assigning a hit with a volume history gives the same volume history", IntoFresh)) && Passed;

  bool ReplacesHistory = UTSDHitShared::RunsCleanlyInChild([]() {
    MCCalorBarHitAccess Original;
    Original.AddVolumeHistory("World");
    Original.AddVolumeHistory("Bar");
    MCCalorBarHitAccess Target;
    Target.AddVolumeHistory("Old1");
    Target.AddVolumeHistory("Old2");
    Target.AddVolumeHistory("Old3");
    Target = Original;
    return Target.m_VolumeHistory.size() == 2 && Target.m_VolumeHistory[0] == "World" && Target.m_VolumeHistory[1] == "Bar";
  });
  // Known failure: operator= resizes the volume history to zero and then assigns into it instead of appending to it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "history into used hit", "Assigning replaces the old volume history", ReplacesHistory)) && Passed;

  bool SelfAssignment = UTSDHitShared::RunsCleanlyInChild([]() {
    MCCalorBarHitAccess Hit;
    Hit.SetEnergy(10.0*keV);
    Hit.AddVolumeHistory("World");
    Hit.AddVolumeHistory("Bar");
    Hit = Hit;
    return Hit.m_VolumeHistory.size() == 2 && Hit.GetEnergy() == 10.0*keV;
  });
  // Known failure: operator= has no self-assignment check and resizes its own volume history to zero before copying it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "self assignment", "Assigning a hit to itself keeps its data", SelfAssignment)) && Passed;

  MCCalorBarHitAccess A;
  MCCalorBarHitAccess B;
  MCCalorBarHitAccess C;
  A.SetEnergy(5.0*keV);
  const MCCalorBarHit& Result = (B = A);
  Passed = EvaluateTrue("operator=", "chaining", "The assignment returns the assigned hit", &Result == &B) && Passed;
  C = B = A;
  Passed = EvaluateNear("operator=", "chaining", "A chained assignment reaches the last hit", C.GetEnergy()/keV, 5.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::TestEqualOperator()
{
  bool Passed = true;

  MCCalorBarHitAccess A;
  MCCalorBarHitAccess B;
  Fill(A, 1.0*keV, 1.0, 2.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "Calorimeter" });
  Fill(B, 900.0*keV, 5.0, 6.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 2.0*s, 2, 3, 4, { "World", "Calorimeter" });
  Passed = EvaluateTrue("operator==", "same bar", "Hits in the same bar of the same module are equal, energy and position are not compared", (A == B) != 0) && Passed;

  MCCalorBarHitAccess C;
  Fill(C, 1.0*keV, 1.0, 2.0, G4ThreeVector(), 0.0, 1, 5, 4, { "World", "Calorimeter" });
  Passed = EvaluateFalse("operator==", "different x bar", "Hits in a different x bar are not equal", (A == C) != 0) && Passed;
  MCCalorBarHitAccess D;
  Fill(D, 1.0*keV, 1.0, 2.0, G4ThreeVector(), 0.0, 1, 3, 9, { "World", "Calorimeter" });
  Passed = EvaluateFalse("operator==", "different y bar", "Hits in a different y bar are not equal", (A == D) != 0) && Passed;
  MCCalorBarHitAccess E;
  Fill(E, 1.0*keV, 1.0, 2.0, G4ThreeVector(), 0.0, 1, 3, 4, { "World", "OtherCalorimeter" });
  Passed = EvaluateFalse("operator==", "different module", "Hits in the same bar of another module are not equal", (A == E) != 0) && Passed;

  bool LongerOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCCalorBarHitAccess Short;
    Short.AddVolumeHistory("World");
    MCCalorBarHitAccess Long;
    Long.AddVolumeHistory("World");
    Long.AddVolumeHistory("Bar");
    return (Short == Long) == 0;
  });
  // Known failure: operator== compares only as many volumes as the own history has and ignores the longer history of the other hit
  Passed = ExpectedToFail(EvaluateTrue("operator==", "other has a longer history", "A hit is not equal to a hit with a longer volume history", LongerOther)) && Passed;

  bool EmptyOther = UTSDHitShared::RunsCleanlyInChild([]() {
    MCCalorBarHitAccess Empty;
    MCCalorBarHitAccess Long;
    Long.AddVolumeHistory("World");
    return (Long == Empty) == 0 && (Empty == Long) == 0;
  });
  // Known failure: operator== does not compare the history sizes and reads the other hit's empty history out of bounds
  Passed = ExpectedToFail(EvaluateTrue("operator==", "one empty history", "A hit with and a hit without volume history are not equal", EmptyOther)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::TestAddOperator()
{
  bool Passed = true;

  // Same bar - the position is the energy weighted average:
  {
    MCCalorBarHitAccess A;
    Fill(A, 100.0*keV, 10.0, 20.0, G4ThreeVector(0.0, 0.0, 0.0), 5.0*s, 1, 3, 4, { "World", "Calorimeter" });
    MCCalorBarHitAccess B;
    Fill(B, 300.0*keV, 30.0, 40.0, G4ThreeVector(4.0*cm, 8.0*cm, 12.0*cm), 3.0*s, 2, 3, 4, { "World", "Calorimeter" });
    const MCCalorBarHit& Result = (A += B);
    Passed = EvaluateTrue("operator+=", "same bar", "The operator returns the hit it was called on", &Result == &A) && Passed;
    Passed = EvaluateNear("operator+=", "same bar", "The energies are added", A.GetEnergy()/keV, 400.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same bar", "The ADC counts at the top are added", A.GetADCCountsTop(), 40.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same bar", "The ADC counts at the bottom are added", A.GetADCCountsBottom(), 60.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same bar", "The x position is the energy weighted average", A.GetPosition().x()/cm, 3.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same bar", "The y position is the energy weighted average", A.GetPosition().y()/cm, 6.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same bar", "The z position is the energy weighted average", A.GetPosition().z()/cm, 9.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "same bar", "The earlier time is kept", A.GetTime()/s, 3.0, 1e-9) && Passed;
    Passed = EvaluateSize("operator+=", "same bar", "The origins are merged", A.m_Origins.size(), 2) && Passed;
    Passed = EvaluateNear("operator+=", "same bar", "The added hit is not changed", B.GetEnergy()/keV, 300.0, 1e-9) && Passed;
  }

  // Different bar or module is not added:
  {
    MCCalorBarHitAccess A;
    Fill(A, 100.0*keV, 10.0, 20.0, G4ThreeVector(1.0*cm, 1.0*cm, 1.0*cm), 5.0*s, 1, 3, 4, { "World", "Calorimeter" });
    MCCalorBarHitAccess OtherBar;
    Fill(OtherBar, 250.0*keV, 30.0, 40.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 5, { "World", "Calorimeter" });
    A += OtherBar;
    Passed = EvaluateNear("operator+=", "different bar", "Hits in different bars keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "different bar", "Hits in different bars keep their position", A.GetPosition().getX()/cm, 1.0, 1e-9) && Passed;
    MCCalorBarHitAccess OtherModule;
    Fill(OtherModule, 250.0*keV, 30.0, 40.0, G4ThreeVector(2.0*cm, 3.0*cm, 4.0*cm), 3.0*s, 2, 3, 4, { "World", "Calorimeter2" });
    A += OtherModule;
    Passed = EvaluateNear("operator+=", "different module", "Hits in different modules keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
    MCCalorBarHitAccess OtherLength;
    Fill(OtherLength, 250.0*keV, 30.0, 40.0, G4ThreeVector(), 3.0*s, 2, 3, 4, { "World" });
    A += OtherLength;
    Passed = EvaluateNear("operator+=", "different history length", "Hits with a different history length keep their energy", A.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  }

  // The not available marker of the bottom counts is not a count:
  {
    MCCalorBarHitAccess NoBottom1;
    Fill(NoBottom1, 10.0*keV, 10.0, -1.0, G4ThreeVector(), 1.0*s, 1, 1, 1, {});
    MCCalorBarHitAccess NoBottom2;
    Fill(NoBottom2, 10.0*keV, 20.0, -1.0, G4ThreeVector(), 1.0*s, 2, 1, 1, {});
    NoBottom1 += NoBottom2;
    Passed = EvaluateNear("operator+=", "both without bottom", "Two hits without counts at the bottom stay without counts at the bottom", NoBottom1.GetADCCountsBottom(), -1.0, 1e-12) && Passed;
    Passed = EvaluateNear("operator+=", "both without bottom", "The counts at the top are added", NoBottom1.GetADCCountsTop(), 30.0, 1e-9) && Passed;

    MCCalorBarHitAccess WithBottom;
    Fill(WithBottom, 10.0*keV, 10.0, 5.0, G4ThreeVector(), 1.0*s, 3, 1, 1, {});
    MCCalorBarHitAccess WithoutBottom;
    Fill(WithoutBottom, 10.0*keV, 10.0, -1.0, G4ThreeVector(), 1.0*s, 4, 1, 1, {});
    WithBottom += WithoutBottom;
    // Known failure: operator+= adds the not available marker -1 of the other hit's bottom counts to the real bottom counts (5 - 1 = 4)
    Passed = ExpectedToFail(EvaluateNear("operator+=", "with and without bottom", "The not available marker -1 is not added to real counts at the bottom", WithBottom.GetADCCountsBottom(), 5.0, 1e-9)) && Passed;
  }

  // Hits without energy have a defined position:
  {
    MCCalorBarHitAccess Zero1;
    Fill(Zero1, 0.0, 0.0, 0.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 1.0*s, 1, 1, 1, {});
    MCCalorBarHitAccess Zero2;
    Fill(Zero2, 0.0, 0.0, 0.0, G4ThreeVector(3.0*cm, 4.0*cm, 5.0*cm), 1.0*s, 2, 1, 1, {});
    Zero1 += Zero2;
    // Known failure: operator+= divides the energy weighted position by the energy sum, which is zero here
    Passed = ExpectedToFail(EvaluateTrue("operator+=", "zero energies", "Adding two hits without energy does not give a position which is not a number", std::isfinite(Zero1.GetPosition().x()) && std::isfinite(Zero1.GetPosition().y()) && std::isfinite(Zero1.GetPosition().z()))) && Passed;
  }

  // Adding a hit to itself:
  {
    MCCalorBarHitAccess A;
    Fill(A, 100.0*keV, 10.0, 20.0, G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm), 1.0*s, 1, 3, 4, { "World", "Calorimeter" });
    A += A;
    Passed = EvaluateNear("operator+=", "self", "Adding a hit to itself doubles the energy", A.GetEnergy()/keV, 200.0, 1e-9) && Passed;
    Passed = EvaluateNear("operator+=", "self", "Adding a hit to itself keeps the position", A.GetPosition().getY()/cm, 2.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::TestAllocationAndCollection()
{
  bool Passed = true;

  vector<MCCalorBarHit*> Hits;
  for (unsigned int i = 0; i < 1000; ++i) {
    MCCalorBarHit* Hit = new MCCalorBarHit();
    Hit->SetEnergy((i + 1)*keV);
    Hits.push_back(Hit);
  }
  set<MCCalorBarHit*> Addresses(Hits.begin(), Hits.end());
  Passed = EvaluateSize("operator new", "1000 hits", "All allocated hits have different addresses", Addresses.size(), 1000) && Passed;
  bool AllKept = true;
  for (unsigned int i = 0; i < Hits.size(); ++i) {
    if (Hits[i]->GetEnergy() != (i + 1)*keV) AllKept = false;
  }
  Passed = EvaluateTrue("operator new", "1000 hits", "Every allocated hit keeps its own energy", AllKept) && Passed;
  for (MCCalorBarHit* Hit: Hits) delete Hit;

  MCCalorBarHit* Reused = new MCCalorBarHit();
  Passed = EvaluateNear("operator new", "reuse", "A new hit after freeing others is initialized", Reused->GetEnergy(), 0.0, 1e-12) && Passed;
  delete Reused;

  MCCalorBarHitsCollection* Collection = new MCCalorBarHitsCollection("Detector", "Collection");
  MCCalorBarHit* First = new MCCalorBarHit();
  First->SetEnergy(511.0*keV);
  MCCalorBarHit* Second = new MCCalorBarHit();
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


bool UTCalorBarHit::TestGetCalibrated()
{
  bool Passed = true;

  MCCalorBarHitAccess Hit;
  Fill(Hit, 1275.0*keV, 10.0, 20.0, G4ThreeVector(10.0*mm, 20.0*mm, -30.0*mm), 0.125*s, 6, 1, 2, {});
  MSimHT* HT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "calorimeter hit", "A calibrated hit is returned", HT != nullptr) && Passed;
  if (HT != nullptr) {
    Passed = Evaluate("GetCalibrated()", "calorimeter hit", "The detector type is calorimeter", HT->GetDetectorType(), (int) MDDetector::c_Calorimeter) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "calorimeter hit", "The energy is given in keV", HT->GetEnergy(), 1275.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "calorimeter hit", "The x position is given in cm", HT->GetPosition().X(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "calorimeter hit", "The z position is given in cm", HT->GetPosition().Z(), -3.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "calorimeter hit", "The time is given in seconds", HT->GetTime(), 0.125, 1e-9) && Passed;
    Passed = Evaluate("GetCalibrated()", "calorimeter hit", "The origin is passed on", HT->GetNOrigins(), 1U) && Passed;
    delete HT;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCalorBarHit::TestPrint()
{
  bool Passed = true;

  MCCalorBarHitAccess Both;
  Fill(Both, 662.0*keV, 100.4, 200.6, G4ThreeVector(10.0*mm, -25.0*mm, 30.0*mm), 0.0, 1, 3, 4, {});
  MString Text = GetPrint(Both);
  Passed = EvaluateTrue("Print()", "both channels", "The output contains the detector name", Text.Contains("Calorimeter_1")) && Passed;
  Passed = EvaluateTrue("Print()", "both channels", "The output contains the energy in keV", Text.Contains("662 keV")) && Passed;
  Passed = EvaluateTrue("Print()", "both channels", "The output contains the position in cm", Text.Contains("(1/-2.5/3)")) && Passed;
  Passed = EvaluateTrue("Print()", "both channels", "The output contains the bars", Text.Contains("(3, 4)")) && Passed;
  Passed = EvaluateTrue("Print()", "both channels", "The output contains the rounded counts at both ends", Text.Contains("(100, 201)")) && Passed;

  MCCalorBarHitAccess TopOnly;
  Fill(TopOnly, 662.0*keV, 100.4, -1.0, G4ThreeVector(), 0.0, 1, 3, 4, {});
  MString TopText = GetPrint(TopOnly);
  Passed = EvaluateTrue("Print()", "top only", "The output contains only the counts at the top if the bottom is not available", TopText.Contains("(100)")) && Passed;
  Passed = EvaluateFalse("Print()", "top only", "The output does not contain a second count if the bottom is not available", TopText.Contains(", 0)") || TopText.Contains(", -1)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCalorBarHit Test;
  return Test.Run() == true ? 0 : 1;
}
