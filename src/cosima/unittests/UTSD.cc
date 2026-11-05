/*
 * UTSD.cc
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
#include "MCSD.hh"

// MEGAlib:
#include "MDDetector.h"
#include "MFunction3D.h"
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4TouchableHistory.hh"

// Standard lib:
#include <memory>
#include <set>
#include <vector>
using namespace std;


//! Concrete sensitive detector exposing the protected members of MCSD to the unit test
class MCSDAccess : public MCSD
{
public:
  MCSDAccess(G4String Name) : MCSD(Name) {}
  virtual ~MCSDAccess() {}

  virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*) { return true; }
  virtual G4bool PostProcessHits(const G4Step*) { return true; }

  using MCSD::m_Type;
  using MCSD::m_DetectorVolumeName;
  using MCSD::m_StructuralSize;
  using MCSD::m_StructuralPitch;
  using MCSD::m_StructuralOffset;
  using MCSD::m_StructuralDimension;
  using MCSD::m_DiscretizeHits;
  using MCSD::m_EnergyLoss;
  using MCSD::m_UseEnergyLoss;
  using MCSD::m_HasTimeResolution;
  using MCSD::m_IsNeverTriggering;
  using MCSD::m_Epsilon;
};


//! Unit test class for MCSD
class UTSD : public MUnitTest
{
public:
  //! Default constructor
  UTSD() : MUnitTest("UTSD") {}
  //! Default destructor
  virtual ~UTSD() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructor and its defaults
  bool TestConstruction();
  //! Test the detector type IDs
  bool TestTypeIDs();
  //! Test the structural setters
  bool TestStructureSetters();
  //! Test the flags
  bool TestFlags();
  //! Test the energy loss map
  bool TestEnergyLossMap();
  //! Test the destruction through the base class
  bool TestDestruction();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSD::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestTypeIDs() && Passed;
  Passed = TestStructureSetters() && Passed;
  Passed = TestFlags() && Passed;
  Passed = TestEnergyLossMap() && Passed;
  Passed = TestDestruction() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSD::TestConstruction()
{
  bool Passed = true;

  MCSDAccess SD("Crystals");
  Passed = Evaluate("MCSD()", "name", "The name of the detector is the one given", MString(SD.GetName().c_str()), MString("Crystals")) && Passed;
  Passed = Evaluate("GetType()", "default", "The default type is unknown", SD.GetType(), MCSD::c_Unknown) && Passed;
  Passed = Evaluate("MCSD()", "default", "The default type member is unknown", SD.m_Type, MCSD::c_Unknown) && Passed;
  Passed = EvaluateTrue("MCSD()", "default", "Hits are discretized by default", SD.m_DiscretizeHits) && Passed;
  Passed = EvaluateFalse("MCSD()", "default", "The energy loss map is not used by default", SD.m_UseEnergyLoss) && Passed;
  Passed = EvaluateFalse("MCSD()", "default", "The detector has no time resolution by default", SD.m_HasTimeResolution) && Passed;
  Passed = EvaluateFalse("MCSD()", "default", "The detector is triggering by default", SD.m_IsNeverTriggering) && Passed;
  Passed = EvaluateNear("MCSD()", "default", "The default epsilon is 1e-10", SD.m_Epsilon, 1e-10, 1e-20) && Passed;
  Passed = Evaluate("MCSD()", "default", "The default detector volume name is empty", MString(SD.m_DetectorVolumeName.c_str()), MString("")) && Passed;
  Passed = EvaluateNear("MCSD()", "default", "The default structural size is zero", SD.m_StructuralSize.mag(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCSD()", "default", "The default structural pitch is zero", SD.m_StructuralPitch.mag(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCSD()", "default", "The default structural offset is zero", SD.m_StructuralOffset.mag(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCSD()", "default", "The default structural dimension is zero", SD.m_StructuralDimension.mag(), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("MCSD()", "default", "The detector is active by default", SD.isActive()) && Passed;
  Passed = Evaluate("MCSD()", "default", "A new detector has no hit collections", SD.GetNumberOfCollections(), 0) && Passed;

  // Two detectors do not share any state:
  MCSDAccess Other("Shield");
  SD.m_Type = MCSD::c_Scintillator;
  Passed = Evaluate("GetType()", "two detectors", "The type of one detector does not change the other", Other.GetType(), MCSD::c_Unknown) && Passed;
  Passed = Evaluate("GetType()", "changed type", "GetType returns the type of the detector", SD.GetType(), MCSD::c_Scintillator) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSD::TestTypeIDs()
{
  bool Passed = true;

  Passed = Evaluate("c_Unknown", "type ID", "The unknown type is the undefined detector type of Geomega", MCSD::c_Unknown, (int) MDDetector::c_NoDetectorType) && Passed;
  Passed = Evaluate("c_2DStrip", "type ID", "The 2D strip type is the one of Geomega", MCSD::c_2DStrip, (int) MDDetector::c_Strip2D) && Passed;
  Passed = Evaluate("c_Calorimeter", "type ID", "The calorimeter type is the one of Geomega", MCSD::c_Calorimeter, (int) MDDetector::c_Calorimeter) && Passed;
  Passed = Evaluate("c_3DStrip", "type ID", "The 3D strip type is the one of Geomega", MCSD::c_3DStrip, (int) MDDetector::c_Strip3D) && Passed;
  Passed = Evaluate("c_Scintillator", "type ID", "The scintillator type is the one of Geomega", MCSD::c_Scintillator, (int) MDDetector::c_Scintillator) && Passed;
  Passed = Evaluate("c_DriftChamber", "type ID", "The drift chamber type is the one of Geomega", MCSD::c_DriftChamber, (int) MDDetector::c_DriftChamber) && Passed;
  Passed = Evaluate("c_Voxel3D", "type ID", "The voxel type is the one of Geomega", MCSD::c_Voxel3D, (int) MDDetector::c_Voxel3D) && Passed;
  Passed = Evaluate("c_AngerCamera", "type ID", "The Anger camera type is the one of Geomega", MCSD::c_AngerCamera, (int) MDDetector::c_AngerCamera) && Passed;

  // All types must be distinguishable:
  set<int> IDs = { MCSD::c_Unknown, MCSD::c_2DStrip, MCSD::c_Calorimeter, MCSD::c_3DStrip, MCSD::c_Scintillator, MCSD::c_DriftChamber, MCSD::c_Voxel3D, MCSD::c_AngerCamera };
  Passed = EvaluateSize("c_Unknown...c_AngerCamera", "all type IDs", "All eight type IDs are different", IDs.size(), 8) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSD::TestStructureSetters()
{
  bool Passed = true;

  MCSDAccess SD("Crystals");
  SD.SetDetectorVolumeName("Crystal_Log");
  Passed = Evaluate("SetDetectorVolumeName()", "Crystal_Log", "The detector volume name is stored", MString(SD.m_DetectorVolumeName.c_str()), MString("Crystal_Log")) && Passed;
  SD.SetDetectorVolumeName("");
  Passed = Evaluate("SetDetectorVolumeName()", "empty", "An empty detector volume name is stored", MString(SD.m_DetectorVolumeName.c_str()), MString("")) && Passed;

  SD.SetDetectorStructuralSize(G4ThreeVector(1.0*cm, 2.0*cm, 3.0*cm));
  Passed = EvaluateNear("SetDetectorStructuralSize()", "1, 2, 3 cm", "The x size is stored", SD.m_StructuralSize.x()/cm, 1.0, 1e-9) && Passed;
  Passed = EvaluateNear("SetDetectorStructuralSize()", "1, 2, 3 cm", "The y size is stored", SD.m_StructuralSize.y()/cm, 2.0, 1e-9) && Passed;
  Passed = EvaluateNear("SetDetectorStructuralSize()", "1, 2, 3 cm", "The z size is stored", SD.m_StructuralSize.z()/cm, 3.0, 1e-9) && Passed;

  SD.SetDetectorStructuralPitch(G4ThreeVector(0.5*mm, 0.25*mm, 0.0));
  Passed = EvaluateNear("SetDetectorStructuralPitch()", "0.5, 0.25, 0 mm", "The x pitch is stored", SD.m_StructuralPitch.x()/mm, 0.5, 1e-9) && Passed;
  Passed = EvaluateNear("SetDetectorStructuralPitch()", "0.5, 0.25, 0 mm", "The y pitch is stored", SD.m_StructuralPitch.y()/mm, 0.25, 1e-9) && Passed;
  Passed = EvaluateNear("SetDetectorStructuralPitch()", "0.5, 0.25, 0 mm", "The z pitch is stored", SD.m_StructuralPitch.z()/mm, 0.0, 1e-9) && Passed;

  SD.SetDetectorStructuralOffset(G4ThreeVector(-1.0*mm, 4.0*mm, 7.5*mm));
  Passed = EvaluateNear("SetDetectorStructuralOffset()", "-1, 4, 7.5 mm", "The x offset is stored", SD.m_StructuralOffset.x()/mm, -1.0, 1e-9) && Passed;
  Passed = EvaluateNear("SetDetectorStructuralOffset()", "-1, 4, 7.5 mm", "The z offset is stored", SD.m_StructuralOffset.z()/mm, 7.5, 1e-9) && Passed;

  SD.SetDetectorStructuralDimension(G4ThreeVector(64.0, 32.0, 1.0));
  Passed = EvaluateNear("SetDetectorStructuralDimension()", "64, 32, 1", "The x dimension is stored", SD.m_StructuralDimension.x(), 64.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetDetectorStructuralDimension()", "64, 32, 1", "The y dimension is stored", SD.m_StructuralDimension.y(), 32.0, 1e-12) && Passed;

  // The setters are independent of each other:
  Passed = EvaluateNear("SetDetectorStructuralSize()", "independence", "Setting the pitch and offset does not change the size", SD.m_StructuralSize.z()/cm, 3.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSD::TestFlags()
{
  bool Passed = true;

  MCSDAccess SD("Crystals");
  SD.SetDiscretizeHits(false);
  Passed = EvaluateFalse("SetDiscretizeHits()", "false", "The hits are not discretized after switching it off", SD.m_DiscretizeHits) && Passed;
  SD.SetDiscretizeHits(true);
  Passed = EvaluateTrue("SetDiscretizeHits()", "true", "The hits are discretized after switching it on again", SD.m_DiscretizeHits) && Passed;

  SD.SetHasTimeResolution(true);
  Passed = EvaluateTrue("SetHasTimeResolution()", "true", "The time resolution flag is set", SD.m_HasTimeResolution) && Passed;
  SD.SetHasTimeResolution(false);
  Passed = EvaluateFalse("SetHasTimeResolution()", "false", "The time resolution flag can be cleared", SD.m_HasTimeResolution) && Passed;

  SD.SetIsNeverTriggering(true);
  Passed = EvaluateTrue("SetIsNeverTriggering()", "true", "The never triggering flag is set", SD.m_IsNeverTriggering) && Passed;
  SD.SetIsNeverTriggering(false);
  Passed = EvaluateFalse("SetIsNeverTriggering()", "false", "The never triggering flag can be cleared", SD.m_IsNeverTriggering) && Passed;

  // The flags do not influence each other:
  SD.SetHasTimeResolution(true);
  Passed = EvaluateFalse("SetHasTimeResolution()", "independence", "Setting the time resolution does not change the never triggering flag", SD.m_IsNeverTriggering) && Passed;
  Passed = EvaluateTrue("SetHasTimeResolution()", "independence", "Setting the time resolution does not change the discretization", SD.m_DiscretizeHits) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSD::TestEnergyLossMap()
{
  bool Passed = true;

  // A 2x2x2 map with V = 1 + x + 2 y + 4 z, interpolated linearly:
  vector<double> X = { 0.0, 10.0 };
  vector<double> Y = { 0.0, 10.0 };
  vector<double> Z = { 0.0, 10.0 };
  // The values are stored with x running fastest:
  vector<double> Values;
  for (unsigned int z = 0; z < 2; ++z) {
    for (unsigned int y = 0; y < 2; ++y) {
      for (unsigned int x = 0; x < 2; ++x) {
        Values.push_back(1.0 + X[x] + 2.0*Y[y] + 4.0*Z[z]);
      }
    }
  }
  MFunction3D Map;
  Passed = EvaluateTrue("MFunction3D::Set()", "2x2x2 map", "The fixture energy loss map can be set", Map.Set(X, Y, Z, Values)) && Passed;

  MCSDAccess SD("Crystals");
  Passed = EvaluateFalse("SetEnergyLossMap()", "before", "The map is not used before it is set", SD.m_UseEnergyLoss) && Passed;
  SD.SetEnergyLossMap(Map);
  Passed = EvaluateTrue("SetEnergyLossMap()", "2x2x2 map", "Setting a map switches its use on", SD.m_UseEnergyLoss) && Passed;
  Passed = EvaluateNear("SetEnergyLossMap()", "origin", "The stored map has the value at the origin", SD.m_EnergyLoss.Evaluate(0.0, 0.0, 0.0), 1.0, 1e-9) && Passed;
  Passed = EvaluateNear("SetEnergyLossMap()", "corner", "The stored map has the value at the far corner", SD.m_EnergyLoss.Evaluate(10.0, 10.0, 10.0), 1.0 + 10.0 + 20.0 + 40.0, 1e-9) && Passed;
  Passed = EvaluateNear("SetEnergyLossMap()", "interior", "The stored map interpolates in the interior", SD.m_EnergyLoss.Evaluate(5.0, 2.5, 7.5), 1.0 + 5.0 + 5.0 + 30.0, 1e-9) && Passed;

  // The detector keeps its own copy:
  vector<double> OtherValues(8, 100.0);
  Map.Set(X, Y, Z, OtherValues);
  Passed = EvaluateNear("SetEnergyLossMap()", "copy", "Changing the original map does not change the stored map", SD.m_EnergyLoss.Evaluate(0.0, 0.0, 0.0), 1.0, 1e-9) && Passed;

  // A new map replaces the old one:
  SD.SetEnergyLossMap(Map);
  Passed = EvaluateNear("SetEnergyLossMap()", "replace", "A second map replaces the first one", SD.m_EnergyLoss.Evaluate(5.0, 5.0, 5.0), 100.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSD::TestDestruction()
{
  bool Passed = true;

  // Sensitive detectors are deleted through the base class by Geant4:
  MCSD* SD = new MCSDAccess("Deleted");
  Passed = Evaluate("GetType()", "through the base class", "The type can be read through a pointer to MCSD", SD->GetType(), MCSD::c_Unknown) && Passed;
  delete SD;
  Passed = EvaluateTrue("~MCSD()", "delete", "A detector can be deleted through a pointer to MCSD", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSD Test;
  return Test.Run() == true ? 0 : 1;
}
