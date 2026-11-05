/*
 * UTVHit.cc
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
#include "MCVHit.hh"

// MEGAlib:
#include "MDDetector.h"
#include "MGlobal.h"
#include "MSimHT.h"
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <limits>
#include <set>
#include <string>
#include <vector>
using namespace std;


//! Class exposing the protected members of MCVHit to the unit test
class MCVHitAccess : public MCVHit
{
public:
  using MCVHit::m_Energy;
  using MCVHit::m_Position;
  using MCVHit::m_Time;
  using MCVHit::m_Name;
  using MCVHit::m_DetectorType;
  using MCVHit::m_Origins;
  using MCVHit::m_VolumeHistory;
};


//! Unit test class for MCVHit
class UTVHit : public MUnitTest
{
public:
  //! Default constructor
  UTVHit() : MUnitTest("UTVHit") {}
  //! Default destructor
  virtual ~UTVHit() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default constructor
  bool TestDefaultConstruction();
  //! Test the setters and getters
  bool TestGettersSetters();
  //! Test adding origins
  bool TestOrigins();
  //! Test the volume history
  bool TestVolumeHistory();
  //! Test the conversion into a calibrated hit
  bool TestGetCalibrated();
  //! Test copying
  bool TestCopy();
  //! Test the use as base class
  bool TestPolymorphism();
};


////////////////////////////////////////////////////////////////////////////////


bool UTVHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestOrigins() && Passed;
  Passed = TestVolumeHistory() && Passed;
  Passed = TestGetCalibrated() && Passed;
  Passed = TestCopy() && Passed;
  Passed = TestPolymorphism() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVHit::TestDefaultConstruction()
{
  bool Passed = true;

  MCVHitAccess Hit;
  Passed = EvaluateNear("MCVHit()", "default", "The default energy is zero", Hit.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCVHit()", "default", "The default x position is zero", Hit.GetPosition().getX(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCVHit()", "default", "The default y position is zero", Hit.GetPosition().getY(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCVHit()", "default", "The default z position is zero", Hit.GetPosition().getZ(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCVHit()", "default", "The default time is zero", Hit.GetTime(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCVHit()", "default", "The default detector name is Unknown", MString(Hit.GetDetectorName().c_str()), MString("Unknown")) && Passed;
  Passed = Evaluate("MCVHit()", "default", "The default detector type is the undefined detector type", Hit.GetDetectorType(), (int) MDDetector::c_NoDetectorType) && Passed;
  Passed = EvaluateSize("MCVHit()", "default", "A new hit has no origins", Hit.m_Origins.size(), 0) && Passed;
  Passed = EvaluateSize("MCVHit()", "default", "A new hit has no volume history", Hit.m_VolumeHistory.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVHit::TestGettersSetters()
{
  bool Passed = true;

  MCVHitAccess Hit;
  Hit.SetEnergy(511.0*keV);
  Passed = EvaluateNear("SetEnergy/GetEnergy", "511 keV", "GetEnergy returns the energy set before", Hit.GetEnergy()/keV, 511.0, 1e-9) && Passed;
  Hit.SetEnergy(-3.25*keV);
  Passed = EvaluateNear("SetEnergy/GetEnergy", "negative", "A negative energy is stored as it is", Hit.GetEnergy()/keV, -3.25, 1e-9) && Passed;
  Hit.SetEnergy(0.0);
  Passed = EvaluateNear("SetEnergy/GetEnergy", "zero", "A zero energy is stored", Hit.GetEnergy(), 0.0, 1e-12) && Passed;

  Hit.SetPosition(G4ThreeVector(1.5*cm, -2.25*cm, 33.0*cm));
  Passed = EvaluateNear("SetPosition/GetPosition", "representative", "The x position is stored", Hit.GetPosition().getX()/cm, 1.5, 1e-9) && Passed;
  Passed = EvaluateNear("SetPosition/GetPosition", "representative", "The y position is stored", Hit.GetPosition().getY()/cm, -2.25, 1e-9) && Passed;
  Passed = EvaluateNear("SetPosition/GetPosition", "representative", "The z position is stored", Hit.GetPosition().getZ()/cm, 33.0, 1e-9) && Passed;

  Hit.SetTime(2.5*s);
  Passed = EvaluateNear("SetTime/GetTime", "2.5 s", "GetTime returns the time set before", Hit.GetTime()/s, 2.5, 1e-9) && Passed;

  Hit.SetDetectorName("Crystal_1");
  Passed = Evaluate("SetDetectorName/GetDetectorName", "Crystal_1", "GetDetectorName returns the name set before", MString(Hit.GetDetectorName().c_str()), MString("Crystal_1")) && Passed;
  Hit.SetDetectorName("");
  Passed = Evaluate("SetDetectorName/GetDetectorName", "empty", "An empty name is stored", MString(Hit.GetDetectorName().c_str()), MString("")) && Passed;

  // The detector type can only be set by derived classes:
  Hit.m_DetectorType = MDDetector::c_Scintillator;
  Passed = Evaluate("GetDetectorType()", "scintillator", "GetDetectorType returns the type of the detector", Hit.GetDetectorType(), (int) MDDetector::c_Scintillator) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVHit::TestOrigins()
{
  bool Passed = true;

  MCVHitAccess Hit;
  Hit.AddOrigin(7);
  Passed = EvaluateSize("AddOrigin()", "one origin", "One origin is stored", Hit.m_Origins.size(), 1) && Passed;
  Passed = EvaluateTrue("AddOrigin()", "one origin", "The origin is the one added", Hit.m_Origins.count(7) == 1) && Passed;

  Hit.AddOrigin(7);
  Passed = EvaluateSize("AddOrigin()", "same origin twice", "The same origin is not added twice", Hit.m_Origins.size(), 1) && Passed;

  Hit.AddOrigin(0);
  Hit.AddOrigin(numeric_limits<unsigned int>::max());
  Passed = EvaluateSize("AddOrigin()", "boundary values", "The smallest and the largest track ID are added", Hit.m_Origins.size(), 3) && Passed;

  // Several origins - duplicates are ignored:
  MCVHitAccess Other;
  Other.AddOrigin(1);
  Other.AddOrigin(7);
  Other.AddOrigin(9);
  Hit.AddOrigins(Other.m_Origins);
  Passed = EvaluateSize("AddOrigins()", "overlapping origins", "Adding overlapping origins keeps every origin once", Hit.m_Origins.size(), 5) && Passed;

  set<unsigned int> Empty;
  Hit.AddOrigins(Empty);
  Passed = EvaluateSize("AddOrigins()", "empty set", "Adding an empty set changes nothing", Hit.m_Origins.size(), 5) && Passed;

  // The source is not modified:
  Passed = EvaluateSize("AddOrigins()", "source", "The set of origins which is added stays unchanged", Other.m_Origins.size(), 3) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVHit::TestVolumeHistory()
{
  bool Passed = true;

  MCVHitAccess Hit;
  Hit.AddVolumeHistory("World");
  Hit.AddVolumeHistory("Tower");
  Hit.AddVolumeHistory("Crystal");
  Hit.AddVolumeHistory("Tower");
  Passed = EvaluateSize("AddVolumeHistory()", "four entries", "Every entry is stored, also duplicates", Hit.m_VolumeHistory.size(), 4) && Passed;
  Passed = Evaluate("AddVolumeHistory()", "order", "The first entry is the first added", MString(Hit.m_VolumeHistory[0].c_str()), MString("World")) && Passed;
  Passed = Evaluate("AddVolumeHistory()", "order", "The order of the entries is kept", MString(Hit.m_VolumeHistory[2].c_str()), MString("Crystal")) && Passed;
  Passed = Evaluate("AddVolumeHistory()", "duplicate", "A duplicate entry is stored at the end", MString(Hit.m_VolumeHistory[3].c_str()), MString("Tower")) && Passed;

  Hit.AddVolumeHistory("");
  Passed = EvaluateSize("AddVolumeHistory()", "empty name", "An empty name is stored", Hit.m_VolumeHistory.size(), 5) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVHit::TestGetCalibrated()
{
  bool Passed = true;

  MCVHitAccess Hit;
  Hit.m_DetectorType = MDDetector::c_Calorimeter;
  Hit.SetEnergy(662.0*keV);
  Hit.SetPosition(G4ThreeVector(10.0*mm, -20.0*mm, 35.0*mm));
  Hit.SetTime(0.25*s);
  Hit.AddOrigin(9);
  Hit.AddOrigin(2);
  Hit.AddOrigin(5);

  MSimHT* HT = Hit.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "representative hit", "A calibrated hit is returned", HT != nullptr) && Passed;
  if (HT != nullptr) {
    Passed = Evaluate("GetCalibrated()", "calorimeter", "The detector type is passed on", HT->GetDetectorType(), (int) MDDetector::c_Calorimeter) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "10 mm", "The x position is given in cm", HT->GetPosition().X(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "-20 mm", "The y position is given in cm", HT->GetPosition().Y(), -2.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "35 mm", "The z position is given in cm", HT->GetPosition().Z(), 3.5, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "662 keV", "The energy is given in keV", HT->GetEnergy(), 662.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "0.25 s", "The time is given in seconds", HT->GetTime(), 0.25, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "original energy", "The original energy is the energy", HT->GetOriginalEnergy(), 662.0, 1e-9) && Passed;
    Passed = Evaluate("GetCalibrated()", "three origins", "All origins are passed on", HT->GetNOrigins(), 3U) && Passed;
    vector<unsigned int> Origins = HT->GetOrigins();
    if (Origins.size() == 3) {
      Passed = Evaluate("GetCalibrated()", "origin order", "The origins are sorted ascending", Origins[0], 2U) && Passed;
      Passed = Evaluate("GetCalibrated()", "origin order", "The last origin is the largest", Origins[2], 9U) && Passed;
    }
    delete HT;
  }

  // Default hit - and repeated calls give independent objects:
  MCVHitAccess Empty;
  MSimHT* First = Empty.GetCalibrated();
  MSimHT* Second = Empty.GetCalibrated();
  Passed = EvaluateTrue("GetCalibrated()", "default hit", "A default hit can be converted", First != nullptr && Second != nullptr) && Passed;
  Passed = EvaluateTrue("GetCalibrated()", "repeated call", "Every call returns a new object", First != Second) && Passed;
  if (First != nullptr) {
    Passed = EvaluateNear("GetCalibrated()", "default hit", "A default hit has zero energy", First->GetEnergy(), 0.0, 1e-12) && Passed;
    Passed = Evaluate("GetCalibrated()", "default hit", "A default hit has no origins", First->GetNOrigins(), 0U) && Passed;
  }
  delete First;
  delete Second;

  // Negative energies and coordinates are passed on:
  MCVHitAccess Negative;
  Negative.SetEnergy(-10.0*keV);
  Negative.SetPosition(G4ThreeVector(-1.0*cm, -2.0*cm, -3.0*cm));
  MSimHT* NegativeHT = Negative.GetCalibrated();
  if (NegativeHT != nullptr) {
    Passed = EvaluateNear("GetCalibrated()", "negative", "A negative energy is passed on", NegativeHT->GetEnergy(), -10.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetCalibrated()", "negative", "A negative z position is passed on", NegativeHT->GetPosition().Z(), -3.0, 1e-9) && Passed;
    delete NegativeHT;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVHit::TestCopy()
{
  bool Passed = true;

  MCVHitAccess Original;
  Original.SetEnergy(100.0*keV);
  Original.SetPosition(G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm));
  Original.SetTime(1.5*s);
  Original.SetDetectorName("Crystal");
  Original.m_DetectorType = MDDetector::c_Strip3D;
  Original.AddOrigin(4);
  Original.AddVolumeHistory("World");
  Original.AddVolumeHistory("Crystal");

  MCVHitAccess Copy(Original);
  Passed = EvaluateNear("MCVHit(const MCVHit&)", "copy", "The copy keeps the energy", Copy.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  Passed = EvaluateNear("MCVHit(const MCVHit&)", "copy", "The copy keeps the position", Copy.GetPosition().getZ()/cm, 3.0, 1e-9) && Passed;
  Passed = EvaluateNear("MCVHit(const MCVHit&)", "copy", "The copy keeps the time", Copy.GetTime()/s, 1.5, 1e-9) && Passed;
  Passed = Evaluate("MCVHit(const MCVHit&)", "copy", "The copy keeps the name", MString(Copy.GetDetectorName().c_str()), MString("Crystal")) && Passed;
  Passed = Evaluate("MCVHit(const MCVHit&)", "copy", "The copy keeps the detector type", Copy.GetDetectorType(), (int) MDDetector::c_Strip3D) && Passed;
  Passed = EvaluateSize("MCVHit(const MCVHit&)", "copy", "The copy keeps the origins", Copy.m_Origins.size(), 1) && Passed;
  Passed = EvaluateSize("MCVHit(const MCVHit&)", "copy", "The copy keeps the volume history", Copy.m_VolumeHistory.size(), 2) && Passed;

  Copy.SetEnergy(1.0*keV);
  Copy.AddOrigin(8);
  Copy.AddVolumeHistory("Extra");
  Passed = EvaluateNear("MCVHit(const MCVHit&)", "independence", "Changing the copy does not change the energy of the original", Original.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  Passed = EvaluateSize("MCVHit(const MCVHit&)", "independence", "Changing the copy does not change the origins of the original", Original.m_Origins.size(), 1) && Passed;
  Passed = EvaluateSize("MCVHit(const MCVHit&)", "independence", "Changing the copy does not change the history of the original", Original.m_VolumeHistory.size(), 2) && Passed;

  MCVHitAccess Assigned;
  Assigned = Original;
  Passed = EvaluateNear("operator=", "assignment", "The assigned hit has the energy", Assigned.GetEnergy()/keV, 100.0, 1e-9) && Passed;
  Passed = EvaluateSize("operator=", "assignment", "The assigned hit has the volume history", Assigned.m_VolumeHistory.size(), 2) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVHit::TestPolymorphism()
{
  bool Passed = true;

  // The hit is used through the base class in the hit collections and must be destructible through it:
  G4VHit* Base = new MCVHit();
  MCVHit* Hit = dynamic_cast<MCVHit*>(Base);
  Passed = EvaluateTrue("dynamic_cast", "G4VHit -> MCVHit", "A hit stored as G4VHit can be cast back", Hit != nullptr) && Passed;
  if (Hit != nullptr) {
    MSimHT* HT = Hit->GetCalibrated();
    Passed = EvaluateTrue("GetCalibrated()", "through the base class", "The calibrated hit is created through a pointer to MCVHit", HT != nullptr) && Passed;
    delete HT;
  }
  delete Base;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTVHit Test;
  return Test.Run() == true ? 0 : 1;
}
