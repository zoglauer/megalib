/*
 * UTDGeometryExamples.cxx
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


// Standard libs:
#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include <system_error>
#include <vector>

// POSIX libs: process control has no equivalent in the C++ standard library
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TROOT.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MDGeometry.h"
#include "MDMaterial.h"
#include "MDShape.h"
#include "MDVolume.h"
#include "MFile.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Expected content of one example geometry
//! All numbers come from an independent python preprocessor, which is not part of MEGAlib, and not from MDGeometry:
//! it expands comments, includes, constants, loops, and conditions, and it counts the keywords and the copies of virtual volumes
struct UTDGeometryExamplesEntry {
  //! Path below the geomega example directory
  MString Path;
  //! Number of volumes: declared, copied, and the copies of the daughters of virtual copies
  unsigned int Volumes;
  //! Number of materials including copies
  unsigned int Materials;
  //! Number of detectors including named detectors and guard rings
  unsigned int Detectors;
  //! Number of triggers
  unsigned int Triggers;
  //! Number of vectors
  unsigned int Vectors;
  //! Number of read files
  int Files;
  //! Radius of the surrounding sphere as text (cm)
  MString Radius;
  //! Half length of the world box in x (cm)
  double WorldX;
  //! Half length of the world box in y (cm)
  double WorldY;
  //! Half length of the world box in z (cm)
  double WorldZ;
  //! Name of the world volume
  MString World;
  //! Material of the world volume
  MString WorldMaterial;
  //! Name of the first volume with a material in the file
  MString SampleVolume;
  //! Material of the first volume with a material in the file
  MString SampleMaterial;
  //! Name and material of the last volume with a material in the file, separated by a colon
  MString LastSample;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test which loads all example geometries with MDGeometry
//! Attention: special/CodedMaskGeACT.geo.setup does not load within the run time limit of a unit test, thus only its load time is checked
class UTDGeometryExamples : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDGeometryExamples() : UTGeomegaBase("UTDGeometryExamples") {}
  //! Default destructor
  virtual ~UTDGeometryExamples() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Read a geometry without output and without cross section creation
  bool Load(MDGeometry& Geometry, const MString& FileName);

  //! Test that the list of the example geometries is the expected one
  bool TestList();
  //! Test the content of all geometries which can be loaded
  bool TestContent();
  //! Test the geometries which cannot be loaded
  bool TestKnownFailures();
  //! Test that the slowest geometry loads within a limit
  bool TestLoadTime();

  //! Return the path of the example directory
  MString GetExampleDirectory() const { return "$(MEGALIB)/resource/examples/geomega"; }
  //! Return the expected content of all geometries which can be loaded
  vector<UTDGeometryExamplesEntry> GetEntries() const;
  //! Return the geometries which do not load
  vector<MString> GetFailures() const;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDGeometryExamples::Run()
{
  bool Passed = true;

  // A missing precondition is a failed check: otherwise a run without any check would count as passed
  if (EvaluateTrue("Run()", "MEGALIB", "MEGAlib is set up: the environment variable MEGALIB is defined (source bin/source-megalib.sh)", getenv("MEGALIB") != nullptr) == false) {
    Summarize();
    return false;
  }

  Passed = TestList() && Passed;
  Passed = TestContent() && Passed;
  Passed = TestKnownFailures() && Passed;
  Passed = TestLoadTime() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Read a geometry without output and without cross section creation
bool UTDGeometryExamples::Load(MDGeometry& Geometry, const MString& FileName)
{
  ostringstream Silent;
  streambuf* OriginalOut = cout.rdbuf(Silent.rdbuf());
  streambuf* OriginalErr = cerr.rdbuf(Silent.rdbuf());
  DisableDefaultStreams();
  bool Result = Geometry.ScanSetupFile(FileName, false, false, false);
  EnableDefaultStreams();
  cout.rdbuf(OriginalOut);
  cerr.rdbuf(OriginalErr);
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the list of the example geometries is the expected one
bool UTDGeometryExamples::TestList()
{
  bool Passed = true;

  // The expected list:
  vector<MString> Expected = GetFailures();
  for (const UTDGeometryExamplesEntry& Entry: GetEntries()) {
    Expected.push_back(Entry.Path);
  }
  Passed = EvaluateSize("GetEntries()", "expected list", "The expected list has all 42 geometries", Expected.size(), 42) && Passed;

  // The list on disk, found at run time:
  MString Directory = GetExampleDirectory();
  MFile::ExpandFileName(Directory);
  vector<MString> Found;
  std::error_code Error;
  for (const auto& File: std::filesystem::recursive_directory_iterator(Directory.Data(), Error)) {
    MString Name = File.path().string();
    if (Name.EndsWith(".geo.setup") == true) {
      Found.push_back(Name.GetSubString(Directory.Length() + 1, Name.Length()));
    }
  }
  Passed = Evaluate("recursive_directory_iterator", "example directory", "The directory can be read", Error.value(), 0) && Passed;

  for (const MString& Name: Expected) {
    Passed = EvaluateTrue("File list", Name, "The expected geometry exists", find(Found.begin(), Found.end(), Name) != Found.end()) && Passed;
  }
  for (const MString& Name: Found) {
    Passed = EvaluateTrue("File list", Name, "The geometry is in the expected list", find(Expected.begin(), Expected.end(), Name) != Expected.end()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the content of all geometries which can be loaded
bool UTDGeometryExamples::TestContent()
{
  bool Passed = true;

  for (const UTDGeometryExamplesEntry& Entry: GetEntries()) {
    const MString& P = Entry.Path;

    MDGeometry Geometry;
    if (EvaluateTrue("ScanSetupFile()", P, "The example geometry loads", Load(Geometry, GetExampleDirectory() + "/" + P)) == false) {
      Passed = false;
      continue;
    }

    Passed = EvaluateTrue("IsScanned()", P, "The geometry is scanned", Geometry.IsScanned()) && Passed;
    Passed = Evaluate("GetNVolumes()", P, "The number of volumes is the expected one", Geometry.GetNVolumes(), Entry.Volumes) && Passed;
    Passed = Evaluate("GetNMaterials()", P, "The number of materials is the expected one", Geometry.GetNMaterials(), Entry.Materials) && Passed;
    Passed = Evaluate("GetNDetectors()", P, "The number of detectors is the expected one", Geometry.GetNDetectors(), Entry.Detectors) && Passed;
    Passed = Evaluate("GetNTriggers()", P, "The number of triggers is the expected one", Geometry.GetNTriggers(), Entry.Triggers) && Passed;
    Passed = Evaluate("GetNVectors()", P, "The number of vectors is the expected one", Geometry.GetNVectors(), Entry.Vectors) && Passed;
    Passed = Evaluate("GetNIncludes()", P, "The number of read files is the expected one", Geometry.GetNIncludes(), Entry.Files) && Passed;
    Passed = EvaluateNear("GetStartSphereRadius()", P, "The radius of the surrounding sphere is the expected one", Geometry.GetStartSphereRadius(), Entry.Radius.ToDouble(), 1e-9) && Passed;

    // The world is the bounding box:
    MDVolume* World = Geometry.GetWorldVolume();
    if (EvaluateTrue("GetWorldVolume()", P, "The world volume exists", World != nullptr) == false) {
      Passed = false;
      continue;
    }
    Passed = EvaluateTrue("GetWorldVolume()", P, "The world volume has the expected name", World->GetName() == Entry.World) && Passed;
    Passed = EvaluateTrue("GetVolumeAt()", P, "The world volume is the first in the list", Geometry.GetVolumeAt(0) == World) && Passed;
    Passed = EvaluateTrue("MDVolume::GetMaterial()", P, "The world volume has the expected material", World->GetMaterial()->GetName() == Entry.WorldMaterial) && Passed;
    Passed = EvaluateVectorNear("MDShape::GetSize()", P, "The bounding box is the expected one", World->GetShape()->GetSize(), MVector(Entry.WorldX, Entry.WorldY, Entry.WorldZ), 1e-9) && Passed;

    // Samples of the material assignments:
    vector<MString> Last = Entry.LastSample.Tokenize(":");
    MDVolume* First = Geometry.GetVolume(Entry.SampleVolume);
    if (EvaluateTrue("GetVolume()", P, "The first volume with a material exists", First != nullptr) == true) {
      Passed = EvaluateTrue("MDVolume::GetMaterial()", P, "The first volume has the expected material", First->GetMaterial()->GetName() == Entry.SampleMaterial) && Passed;
    } else {
      Passed = false;
    }
    MDVolume* Second = Geometry.GetVolume(Last[0]);
    if (EvaluateTrue("GetVolume()", P, "The last volume with a material exists", Second != nullptr) == true) {
      Passed = EvaluateTrue("MDVolume::GetMaterial()", P, "The last volume has the expected material", Second->GetMaterial()->GetName() == Last[1]) && Passed;
    } else {
      Passed = false;
    }

    // Every volume is complete:
    unsigned int Incomplete = 0;
    for (MDVolume* V: Geometry.GetVolumeList()) {
      if (V->GetMaterial() == nullptr || V->GetShape() == nullptr) {
        ++Incomplete;
      }
    }
    Passed = Evaluate("GetVolumeList()", P, "All volumes have a material and a shape", Incomplete, 0U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometries which cannot be loaded
bool UTDGeometryExamples::TestKnownFailures()
{
  bool Passed = true;

  for (const MString Name: {"cosiballoon/COSIBalloon.WithGondola.geo.setup", "cosiballoon/COSIBalloon.WithGondolaAndAntenna.geo.setup"}) {
    MDGeometry Geometry;
    // Defect GEO-A-53: The example includes COSI_TriggerCriteria.det, which is not part of the repository
    Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", Name, "The example geometry loads", Load(Geometry, GetExampleDirectory() + "/" + Name))) && Passed;
  }

  for (const MString Name: {"detailedprototype/DetectorHead.geo.setup", "detailedprototype/MegaInBalloon.geo.setup", "detailedprototype/MegaOnXYTable.geo.setup", "detailedprototype/SensitiveMaterialOnly.geo.setup"}) {
    MDGeometry Geometry;
    // Defect GEO-A-54: The included files of the example each define the surrounding sphere, which is no longer allowed
    Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", Name, "The example geometry loads", Load(Geometry, GetExampleDirectory() + "/" + Name))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the slowest geometry loads within a limit
bool UTDGeometryExamples::TestLoadTime()
{
  bool Passed = true;

  // The independent preprocessor counts 60151 volumes
  const MString Name = "special/CodedMaskGeACT.geo.setup";
  // Defect GEO-A-52: The time for a scan grows at least quadratically with the number of volumes - this one did not end within 1500 s
  MDGeometry Geometry;
  Passed = ExpectedToCrash(EvaluateTrue("ScanSetupFile()", Name, "The example geometry loads within 10 s", Geometry.ScanSetupFile(GetExampleDirectory() + "/" + Name, false, false, false))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the expected content of all geometries which can be loaded
vector<UTDGeometryExamplesEntry> UTDGeometryExamples::GetEntries() const
{
  // Independent derivation, see UTDGeometryExamplesEntry
  return {
    {"cosiballoon/COSIBalloon.10Detector.geo.setup", 1340, 73, 27, 21, 0, 47, "60.0", 1000.0, 1000.0, 1000.0, "WorldVolume", "Vacuum", "GeD_Mother_0", "mvacuum", "TinLayer_2:mtin"},
    {"cosiballoon/COSIBalloon.12Detector.geo.setup", 1340, 73, 31, 25, 0, 49, "60.0", 1000.0, 1000.0, 1000.0, "WorldVolume", "Vacuum", "GeD_Mother_0", "mvacuum", "TinLayer_2:mtin"},
    {"cosiballoon/COSIBalloon.9Detector.geo.setup", 1340, 73, 25, 19, 0, 46, "60.0", 1000.0, 1000.0, 1000.0, "WorldVolume", "Vacuum", "GeD_Mother_1", "mvacuum", "TinLayer_2:mtin"},
    {"detailedprototype/BalloonOnly.geo.setup", 149, 47, 0, 0, 0, 2, "200.0", 1000.0, 1000.0, 1000.0, "Flight_Volume", "Air", "FV_HalfSphere_1", "Aluminium", "FV_SteelCordP_171:Steel"},
    // The world is a tube with an inner radius of 0, an outer radius of 300, and a half height of 400 - by hand its bounding box has the half lengths 300, 300, 400
    {"GRI/GRI_Flexible.geo.setup", 325, 39, 6, 6, 0, 10, "110.0", 300.0, 300.0, 400.0, "VAC0", "vacuum", "DetectorHead", "vacuum", "DPUH:isot_cu"},
    {"GRIPS/GRIPS_extended.geo.setup", 1286, 47, 8, 1, 0, 2, "155.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "BottomCalorimeterCrystal", "LaBr3", "SupportTube:CarbonCyanateEster"},
    {"GRIPS/GRIPS.geo.setup", 1286, 47, 8, 3, 0, 2, "155.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "BottomCalorimeterCrystal", "LaBr3", "SupportTube:CarbonCyanateEster"},
    {"mpesatellitebaseline/Satellite.geo.setup", 154, 47, 3, 1, 0, 8, "50.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "SiStripDetector", "PCB", "CsI4Layer:Vacuum"},
    {"mpesatellitebaseline/SatelliteWithACS.geo.setup", 158, 47, 6, 4, 0, 9, "80.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "SiStripDetector", "PCB", "Sat:Aluminium"},
    {"simplifiedprototype/PrototypeDuke.geo.setup", 136, 47, 4, 1, 0, 10, "25.0", 500.0, 500.0, 500.0, "WorldVolume", "Air", "SiStripDetector", "PCB", "Calorimeter2:Aluminium"},
    {"simplifiedprototype/Prototype.geo.setup", 136, 47, 4, 1, 0, 10, "25.0", 500.0, 500.0, 500.0, "WorldVolume", "Vacuum", "SiStripDetector", "PCB", "Calorimeter2:Aluminium"},
    {"simplifiedprototype/PrototypeWithACS.geo.setup", 141, 47, 6, 1, 0, 11, "50.0", 500.0, 500.0, 500.0, "WorldVolume", "Vacuum", "SiStripDetector", "PCB", "LowerACS3:NE110"},
    {"simplifiedprototype/TomographySimple.geo.setup", 141, 47, 4, 1, 0, 10, "100.0", 500.0, 500.0, 500.0, "WorldVolume", "Vacuum", "SiStripDetector", "PCB", "MegaVolume:Vacuum"},
    {"special/ActivationSphere.geo.setup", 2, 47, 1, 1, 0, 2, "5.1", 100.0, 100.0, 100.0, "WorldVolume", "Vacuum", "BigSphere", "Germanium", "BigSphere:Germanium"},
    {"special/BooleanVolumes.geo.setup", 8, 47, 0, 0, 0, 2, "10.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "Subtraction", "CsI", "Rod:CsI"},
    {"special/Constant.geo.setup", 6, 47, 1, 1, 0, 2, "10.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "GeStripDetector", "Aluminium", "GeWafer:Germanium"},
    {"special/DeadVoxels.geo.setup", 39, 47, 1, 1, 0, 2, "10.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "GoodPixel", "CZT", "BadPixel:CZT"},
    {"special/DriftChamber.geo.setup", 4, 47, 1, 1, 0, 2, "35.0", 100.0, 100.0, 100.0, "WorldVolume", "Vacuum", "XeDriftChamber", "XenonLXeGRIT", "XeDriftChamber:XenonLXeGRIT"},
    {"special/Dummy.geo.setup", 2, 47, 1, 1, 0, 2, "7000.0", 10000.0, 10000.0, 10000.0, "WorldVolume", "Vacuum", "DummyVolume", "Germanium", "DummyVolume:Germanium"},
    {"special/EnergyResolutionTester.geo.setup", 6, 47, 5, 5, 0, 2, "10.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "GeWafer_Gauss", "Germanium", "GeWafer_GaussLoss:Germanium"},
    {"special/GeACT.geo.setup", 23, 47, 1, 1, 0, 2, "40.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "GeStripDetector", "Aluminium", "GeLayer:Vacuum"},
    {"special/HighResolutionPixelDetector.geo.setup", 2, 47, 1, 1, 0, 2, "150.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "CZTWafer", "CZT", "CZTWafer:CZT"},
    {"special/If.geo.setup", 2, 1, 2, 2, 0, 1, "10.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "GeWafer", "Vacuum", "GeWafer:Vacuum"},
    {"special/Max.geo.setup", 10, 47, 6, 2, 0, 2, "10.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "GeStripDetector", "Aluminium", "GeWaferGuardRing:Germanium"},
    {"special/Micro.geo.setup", 2, 47, 1, 1, 0, 2, "1.0", 50.0, 50.0, 50.0, "WorldVolume", "Vacuum", "GeWafer", "Germanium", "GeWafer:Germanium"},
    {"special/Mini.geo.setup", 2, 47, 2, 2, 0, 2, "10.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "GeWafer", "Germanium", "GeWafer:Germanium"},
    {"special/Moliere.geo.setup", 2, 2, 1, 1, 0, 1, "175.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "SiSD", "Silicon", "SiSD:Silicon"},
    {"special/RadioactiveDecay.geo.setup", 3, 48, 1, 1, 0, 2, "10.0", 100.0, 100.0, 100.0, "WorldVolume", "Vacuum", "SourceNa22", "Na", "GeDetector:Germanium"},
    {"special/Simple_Ge_NCT_try1_040721.geo.setup", 48, 47, 1, 1, 0, 2, "40.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "InstrumentVolume", "Vacuum", "ScrewDummy:Steel_18_8"},
    {"special/Spheric.geo.setup", 2, 47, 1, 1, 0, 2, "75.0", 100.0, 100.0, 100.0, "WorldVolume", "Vacuum", "BigSphere", "Densimet180", "BigSphere:Densimet180"},
    {"special/SPILike.geo.setup", 448, 47, 1, 1, 0, 2, "110.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "ACSRing", "BGO", "Mask:Vacuum"},
    {"special/SuperSi.geo.setup", 33, 47, 1, 1, 0, 2, "175.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "SiSD", "Silicon", "SiSDB:Vacuum"},
    {"special/Trapezoid.geo.setup", 3, 47, 0, 0, 0, 2, "10.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "Trap", "Germanium", "Trap2:Germanium"},
    {"special/UnionOfTubes.geo.setup", 2, 47, 0, 0, 0, 2, "30.0", 5000.0, 5000.0, 5000.0, "World", "Vacuum", "Volume123", "Vacuum", "Volume123:Vacuum"},
    {"special/Vectors.geo.setup", 9, 47, 0, 0, 1, 2, "100.0", 5000.0, 5000.0, 5000.0, "WorldVolume", "Vacuum", "Detector_1_1_1", "Germanium", "Detector_8_8_8:Germanium"},
  };
}


////////////////////////////////////////////////////////////////////////////////


//! Return the geometries which do not load
vector<MString> UTDGeometryExamples::GetFailures() const
{
  return {
    "cosiballoon/COSIBalloon.WithGondola.geo.setup",
    "cosiballoon/COSIBalloon.WithGondolaAndAntenna.geo.setup",
    "detailedprototype/DetectorHead.geo.setup",
    "detailedprototype/MegaInBalloon.geo.setup",
    "detailedprototype/MegaOnXYTable.geo.setup",
    "detailedprototype/SensitiveMaterialOnly.geo.setup",
    "special/CodedMaskGeACT.geo.setup"
  };
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kError;

  UTDGeometryExamples Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDGeometryExamples.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
