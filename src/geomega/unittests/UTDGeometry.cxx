/*
 * UTDGeometry.cxx
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
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <new>
#include <sstream>
#include <vector>

// POSIX libs: process control has no equivalent in the C++ standard library
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TGeoManager.h>
#include <TRandom.h>
#include <TRandom3.h>
#include <TROOT.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MDDetector.h"
#include "MDDriftChamber.h"
#include "MDGeometry.h"
#include "MDGuardRing.h"
#include "MDMaterial.h"
#include "MDOrientation.h"
#include "MDShapeBRIK.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDSystem.h"
#include "MDTriggerBasic.h"
#include "MDTriggerUnit.h"
#include "MDVector.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "MDVoxel3D.h"
#include "MFile.h"
#include "MGlobal.h"
#include "MProcessManager.h"
#include "MSystem.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Expose the protected interface of MDGeometry to the test
class UTDGeometryAccess : public MDGeometry
{
public:
  using MDGeometry::AddFile;
  using MDGeometry::ContainsReplacableConstant;
  using MDGeometry::ImportGDML;
  using MDGeometry::ReplaceWholeWords;
  using MDGeometry::WFS;
  using MDGeometry::m_ComplexER;
  using MDGeometry::m_CrossSectionFileDirectory;
  using MDGeometry::m_DefaultColor;
  using MDGeometry::m_DetectorSearchTolerance;
  using MDGeometry::m_DoSanityChecks;
  using MDGeometry::m_ShowVolumes;
  using MDGeometry::m_Version;
  using MDGeometry::m_VirtualizeNonDetectorVolumes;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the geometry parser and container MDGeometry
//! Attention: the declared functions CreateNode and CloseGeoView3D have no definition and cannot be linked, thus they are not tested
class UTDGeometry : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDGeometry() : UTGeomegaBase("UTDGeometry") {}
  //! Default destructor
  virtual ~UTDGeometry() {}

  //! Run all tests
  virtual bool Run();

  //! Run one geometry with the cross section creation - the child process of the test of the temporary cross section files
  static int RunCrossSectionChild(const MString& FileName);

private:
  //! Return true if a new geometry object accepts the head of the geometry followed by the text
  bool Accepts(const MString& Text);
  //! Return true if a geometry with the world volume as start volume still finds the volume Other at (-20, 0, 0)
  bool IsWholeGeometryFoundFromWorld(const MString& Text);
  //! Return true if a geometry which imports Import.gdml contains the volume GDMLInner, or if the import is rejected
  bool IsGDMLVolumeImportedOrRejected();
  //! Return true if the import of the GDML file gives geomega lines
  bool HasGDMLImportLines(const MString& FileName);
  //! Return a placed box volume of half length 1
  MString Marker(const MString& Name, double X = 0.0) const;
  //! Return a placed silicon wafer of half lengths 5, 5, 0.5
  MString Wafer(const MString& Name = "Wafer", double Z = 0.0) const;
  //! Return the lines common to all detectors
  MString DetectorCommon(const MString& Name, const MString& VolumeName = "Wafer") const;
  //! Return a complete detector of the given keyword with a wafer volume
  MString FullDetector(const MString& Type, const MString& Name = "D", const MString& VolumeName = "Wafer") const;
  //! Check that a volume exists and has the position
  bool CheckPosition(MDGeometry& Geometry, const MString& Name, const MVector& Position, const MString& Input);
  //! Return the names of the given candidate volumes which exist in the geometry
  MString ExistingVolumes(MDGeometry& Geometry, const vector<MString>& Candidates);

  //! Test the state of a new geometry and all calls which need a scan
  bool TestConstruction();
  //! Test the file handling of the scan
  bool TestFiles();
  //! Test Include and the deprecated material path
  bool TestInclude();
  //! Test Exit, comments, line continuation, and very long lines
  bool TestLines();
  //! Test the keyword Constant
  bool TestConstants();
  //! Test the keywords Vector and ForVector
  bool TestVectors();
  //! Test the keyword For
  bool TestFor();
  //! Test the keyword RandomDouble
  bool TestRandom();
  //! Test the keywords If, Else, and EndIf
  bool TestIf();
  //! Test the keywords Print and Echo
  bool TestPrint();
  //! Test the first order keywords and their rejection
  bool TestFirstOrderKeywords();
  //! Test the names and their duplicates
  bool TestNames();
  //! Test the material keywords
  bool TestMaterials();
  //! Test the shape keywords
  bool TestShapes();
  //! Test the orientation keywords
  bool TestOrientations();
  //! Test the volume keywords
  bool TestVolumes();
  //! Test the keyword Scale
  bool TestScale();
  //! Test virtual volumes
  bool TestVirtual();
  //! Test the detector keywords
  bool TestDetectors();
  //! Test named detectors and Assign
  bool TestNamedDetectors();
  //! Test the trigger keywords
  bool TestTriggers();
  //! Test the system keywords
  bool TestSystem();
  //! Test the reuse of one object with several scans
  bool TestReuse();
  //! Test two geometry objects at the same time
  bool TestTwoObjects();
  //! Test GetHash and RequiresReload
  bool TestHashAndReload();
  //! Test the index, name, and list getters
  bool TestGetters();
  //! Test the add functions
  bool TestAdders();
  //! Test the includes interface
  bool TestIncludeInterface();
  //! Test ValidName, MakeValidName, and NameExists
  bool TestNameFunctions();
  //! Test the protected text helpers
  bool TestTextHelpers();
  //! Test ToString and DumpInformation
  bool TestDescriptions();
  //! Test CalculateMasses
  bool TestMasses();
  //! Test CheckOverlaps
  bool TestOverlaps();
  //! Test DrawGeometry and StoreViewParameters
  bool TestDrawing();
  //! Test GetListOfUnusedMaterials
  bool TestUnusedMaterials();
  //! Test the volume sequence and position lookups
  bool TestVolumeSequences();
  //! Test GetGlobalPosition
  bool TestGlobalPosition();
  //! Test GetRandomPositionInVolume
  bool TestRandomPositions();
  //! Test the start volume
  bool TestStartVolume();
  //! Test the preferred visible volumes
  bool TestPreferredVisibleVolumes();
  //! Test the keyword Import
  bool TestImport();
  //! Test the cross section file interface
  bool TestCrossSections();
  //! Test the error messages
  bool TestErrorMessages();
  //! Test that constants are evaluated independently of the order of their definition
  bool TestConstantsOrder();
  //! Test a file with more than 2000 lines, where the constants are replaced in parallel
  bool TestLargeFile();
  //! Test that each object is found by its name
  bool TestNameLookups();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDGeometry::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestFiles() && Passed;
  Passed = TestInclude() && Passed;
  Passed = TestLines() && Passed;
  Passed = TestConstants() && Passed;
  Passed = TestVectors() && Passed;
  Passed = TestFor() && Passed;
  Passed = TestRandom() && Passed;
  Passed = TestIf() && Passed;
  Passed = TestPrint() && Passed;
  Passed = TestFirstOrderKeywords() && Passed;
  Passed = TestNames() && Passed;
  Passed = TestMaterials() && Passed;
  Passed = TestShapes() && Passed;
  Passed = TestOrientations() && Passed;
  Passed = TestVolumes() && Passed;
  Passed = TestScale() && Passed;
  Passed = TestVirtual() && Passed;
  Passed = TestDetectors() && Passed;
  Passed = TestNamedDetectors() && Passed;
  Passed = TestTriggers() && Passed;
  Passed = TestSystem() && Passed;
  Passed = TestReuse() && Passed;
  Passed = TestTwoObjects() && Passed;
  Passed = TestHashAndReload() && Passed;
  Passed = TestGetters() && Passed;
  Passed = TestAdders() && Passed;
  Passed = TestIncludeInterface() && Passed;
  Passed = TestNameFunctions() && Passed;
  Passed = TestTextHelpers() && Passed;
  Passed = TestDescriptions() && Passed;
  Passed = TestMasses() && Passed;
  Passed = TestOverlaps() && Passed;
  Passed = TestDrawing() && Passed;
  Passed = TestUnusedMaterials() && Passed;
  Passed = TestVolumeSequences() && Passed;
  Passed = TestGlobalPosition() && Passed;
  Passed = TestRandomPositions() && Passed;
  Passed = TestStartVolume() && Passed;
  Passed = TestPreferredVisibleVolumes() && Passed;
  Passed = TestImport() && Passed;
  Passed = TestCrossSections() && Passed;
  Passed = TestErrorMessages() && Passed;
  Passed = TestConstantsOrder() && Passed;
  Passed = TestLargeFile() && Passed;
  Passed = TestNameLookups() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a new geometry object accepts the head of the geometry followed by the text
bool UTDGeometry::Accepts(const MString& Text)
{
  // A rejected file also writes directly to cout and cerr:
  MDGeometry Geometry;
  ostringstream Silent;
  streambuf* OriginalOut = cout.rdbuf(Silent.rdbuf());
  streambuf* OriginalErr = cerr.rdbuf(Silent.rdbuf());
  bool Result = ScanGeometry(Geometry, GetGeometryHead() + Text);
  cout.rdbuf(OriginalOut);
  cerr.rdbuf(OriginalErr);
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a geometry with the world volume as start volume still finds the volume Other at (-20, 0, 0)
bool UTDGeometry::IsWholeGeometryFoundFromWorld(const MString& Text)
{
  MDGeometry Geometry;
  Geometry.SetStartVolumeName("World");
  if (ScanGeometry(Geometry, Text) == false) {
    return false;
  }
  return Geometry.GetVolumeSequence(MVector(-20, 0, 0)).GetDeepestVolume() == Geometry.GetVolume("Other");
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a geometry which imports Import.gdml contains the volume GDMLInner, or if the import is rejected
bool UTDGeometry::IsGDMLVolumeImportedOrRejected()
{
  MDGeometry Geometry;
  if (ScanGeometry(Geometry, GetGeometryHead() + "Import GDML Import.gdml\n") == false) {
    return true;
  }
  return Geometry.GetVolume("GDMLInner") != nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the import of the GDML file gives geomega lines
bool UTDGeometry::HasGDMLImportLines(const MString& FileName)
{
  UTDGeometryAccess Geometry;
  list<MDDebugInfo> Lines;
  if (Geometry.ImportGDML(FileName, Lines) == false) {
    return false;
  }
  return Lines.empty() == false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a placed box volume of half length 1
MString UTDGeometry::Marker(const MString& Name, double X) const
{
  MString Text;
  Text += "Volume " + Name + "\n";
  Text += Name + ".Material Aluminium\n";
  Text += Name + ".Shape BRIK 1.0 1.0 1.0\n";
  Text += Name + ".Position " + MString(X) + " 0.0 0.0\n";
  Text += Name + ".Mother World\n";
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a placed silicon wafer of half lengths 5, 5, 0.5
MString UTDGeometry::Wafer(const MString& Name, double Z) const
{
  MString Text;
  Text += "Volume " + Name + "\n";
  Text += Name + ".Material Silicon\n";
  Text += Name + ".Shape BRIK 5.0 5.0 0.5\n";
  Text += Name + ".Position 0.0 0.0 " + MString(Z) + "\n";
  Text += Name + ".Mother World\n";
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the lines common to all detectors
MString UTDGeometry::DetectorCommon(const MString& Name, const MString& VolumeName) const
{
  MString Text;
  Text += Name + ".SensitiveVolume " + VolumeName + "\n";
  Text += Name + ".DetectorVolume " + VolumeName + "\n";
  Text += Name + ".TriggerThreshold 30\n";
  Text += Name + ".NoiseThreshold 15\n";
  Text += Name + ".EnergyResolution Gauss 10 10 1\n";
  Text += Name + ".EnergyResolution Gauss 1000 1000 1\n";
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a complete detector of the given keyword with a wafer volume
MString UTDGeometry::FullDetector(const MString& Type, const MString& Name, const MString& VolumeName) const
{
  MString Text = Type + " " + Name + "\n" + DetectorCommon(Name, VolumeName);

  if (Type.Contains("Strip") == true || Type == "DriftChamber") {
    Text += Name + ".StructuralPitch 0.0 0.0 0.0\n";
    Text += Name + ".StructuralOffset 0.0 0.0 0.0\n";
    Text += Name + ".Offset 0.5 0.5\n";
    Text += Name + ".StripNumber 9 9\n";
  }
  if (Type == "Voxel3D") {
    Text += Name + ".StructuralPitch 0.0 0.0 0.0\n";
    Text += Name + ".StructuralOffset 0.0 0.0 0.0\n";
    Text += Name + ".Offset 0.5 0.5 0.1\n";
    Text += Name + ".VoxelNumber 9 9 4\n";
  }
  if (Type == "Strip3D" || Type == "Strip3DDirectional" || Type == "DriftChamber" || Type == "Calorimeter") {
    Text += Name + ".DepthResolution 10 0.1\n";
    Text += Name + ".DepthResolution 1000 0.1\n";
  }
  if (Type == "Strip3DDirectional") {
    Text += Name + ".DirectionalResolution 10 5.0\n";
    Text += Name + ".DirectionalResolution 1000 5.0\n";
  }
  if (Type == "DriftChamber") {
    Text += Name + ".LightSpeed 18E+9\n";
    Text += Name + ".LightDetectorPosition 3\n";
    Text += Name + ".DriftConstant 1E-6\n";
    Text += Name + ".EnergyPerElectron 15.6\n";
    Text += Name + ".LightEnergyResolution 10 1.0\n";
    Text += Name + ".LightEnergyResolution 1000 1.0\n";
  }
  if (Type == "AngerCamera") {
    Text += Name + ".Positioning XY\n";
    Text += Name + ".PositionResolution 10 0.5\n";
    Text += Name + ".PositionResolution 1000 0.5\n";
  }
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Check that a volume exists and has the position
bool UTDGeometry::CheckPosition(MDGeometry& Geometry, const MString& Name, const MVector& Position, const MString& Input)
{
  MDVolume* Volume = Geometry.GetVolume(Name);
  if (EvaluateTrue("GetVolume()", Input, "The volume " + Name + " exists", Volume != nullptr) == false) {
    return false;
  }
  return EvaluateVectorNear("MDVolume::GetPosition()", Input, "The volume " + Name + " has the expected position", Volume->GetPosition(), Position, 1e-9);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the names of the given candidate volumes which exist in the geometry
MString UTDGeometry::ExistingVolumes(MDGeometry& Geometry, const vector<MString>& Candidates)
{
  MString Names;
  for (const MString& Name: Candidates) {
    if (Geometry.GetVolume(Name) != nullptr) {
      Names += Name + " ";
    }
  }
  return Names;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state of a new geometry and all calls which need a scan
bool UTDGeometry::TestConstruction()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateFalse("IsScanned()", "new object", "A new geometry is not scanned", Geometry.IsScanned()) && Passed;
  Passed = EvaluateTrue("GetFileName()", "new object", "A new geometry has no file name", Geometry.GetFileName() == "") && Passed;
  Passed = EvaluateTrue("GetWorldVolume()", "new object", "A new geometry has no world volume", Geometry.GetWorldVolume() == nullptr) && Passed;
  Passed = Evaluate("GetNVolumes()", "new object", "A new geometry has no volumes", Geometry.GetNVolumes(), 0U) && Passed;
  Passed = Evaluate("GetNMaterials()", "new object", "A new geometry has no materials", Geometry.GetNMaterials(), 0U) && Passed;
  Passed = Evaluate("GetNDetectors()", "new object", "A new geometry has no detectors", Geometry.GetNDetectors(), 0U) && Passed;
  Passed = Evaluate("GetNShapes()", "new object", "A new geometry has no shapes", Geometry.GetNShapes(), 0U) && Passed;
  Passed = Evaluate("GetNOrientations()", "new object", "A new geometry has no orientations", Geometry.GetNOrientations(), 0U) && Passed;
  Passed = Evaluate("GetNTriggers()", "new object", "A new geometry has no triggers", Geometry.GetNTriggers(), 0U) && Passed;
  Passed = Evaluate("GetNVectors()", "new object", "A new geometry has no vectors", Geometry.GetNVectors(), 0U) && Passed;
  Passed = Evaluate("GetNIncludes()", "new object", "A new geometry has no includes", Geometry.GetNIncludes(), 0) && Passed;
  Passed = EvaluateTrue("GetHash()", "new object", "The hash of a new geometry is 0", Geometry.GetHash() == 0) && Passed;
  Passed = EvaluateFalse("RequiresReload()", "new object", "A new geometry has no file which could have changed", Geometry.RequiresReload()) && Passed;
  Passed = EvaluateTrue("AreCrossSectionsPresent()", "new object", "No material misses a cross section", Geometry.AreCrossSectionsPresent()) && Passed;
  Passed = EvaluateNear("GetStartSphereRadius()", "new object", "The radius is not defined", Geometry.GetStartSphereRadius(), g_DoubleNotDefined, 1e-9) && Passed;
  Passed = EvaluateNear("GetStartSphereDistance()", "new object", "The distance is not defined", Geometry.GetStartSphereDistance(), g_DoubleNotDefined, 1e-9) && Passed;
  Passed = EvaluateTrue("GetStartSpherePosition()", "new object", "The position is not defined", Geometry.GetStartSpherePosition() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("HasComplexER()", "new object", "The complex event reconstruction is the default", Geometry.HasComplexER()) && Passed;
  Passed = EvaluateTrue("GetTriggerUnit()", "new object", "The trigger unit exists", Geometry.GetTriggerUnit() != nullptr) && Passed;
  Passed = EvaluateTrue("GetSystem()", "new object", "The default system exists", Geometry.GetSystem() != nullptr) && Passed;
  Passed = EvaluateTrue("GetSystem(Name)", "default name", "The default system has the name NoName", Geometry.GetSystem("NoName") == Geometry.GetSystem()) && Passed;
  Passed = EvaluateTrue("GetSystem(Name)", "unknown name", "An unknown system name returns nullptr", Geometry.GetSystem("Other") == nullptr) && Passed;
  Passed = EvaluateTrue("GetStartVolumeName()", "new object", "There is no start volume", Geometry.GetStartVolumeName() == "") && Passed;
  Passed = EvaluateTrue("ToString()", "new object", "The description contains the default version", Geometry.ToString().Contains("version: 0.0")) && Passed;

  // All these calls have to reject an unscanned geometry:
  Int_t ErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  DisableDefaultStreams();
  bool Overlaps = Geometry.CheckOverlaps();
  ostringstream Diagnostics;
  bool OverlapsDiagnostics = Geometry.CheckOverlaps(Diagnostics);
  bool Draw = Geometry.DrawGeometry();
  MVector Random = Geometry.GetRandomPositionInVolume("World");
  Geometry.DumpInformation();
  Geometry.CalculateMasses();
  Geometry.StoreViewParameters();
  EnableDefaultStreams();
  gErrorIgnoreLevel = ErrorLevel;
  Passed = EvaluateFalse("CheckOverlaps()", "not scanned", "An unscanned geometry cannot be checked", Overlaps) && Passed;
  Passed = EvaluateFalse("CheckOverlaps(Diagnostics)", "not scanned", "An unscanned geometry cannot be checked", OverlapsDiagnostics) && Passed;
  Passed = EvaluateFalse("DrawGeometry()", "not scanned", "An unscanned geometry cannot be drawn", Draw) && Passed;
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "not scanned", "An unscanned geometry returns g_VectorNotDefined", Random == g_VectorNotDefined) && Passed;

  // Defect GEO-A-01: The position lookups dereference the null world volume of an unscanned geometry
  MDGeometry Unscanned;
  Passed = ExpectedToCrash(EvaluateTrue("GetVolumeSequence()", "not scanned", "The call returns an empty sequence and does not crash on an unscanned geometry", Unscanned.GetVolumeSequence(MVector(0, 0, 0)).IsEmpty())) && Passed;
  // Defect GEO-A-01: The position lookups dereference the null world volume of an unscanned geometry
  Passed = ExpectedToCrash(EvaluateTrue("GetVolumeSequencePointer()", "not scanned", "The call returns no sequence and does not crash on an unscanned geometry", Unscanned.GetVolumeSequencePointer(MVector(0, 0, 0)) == nullptr)) && Passed;
  // Defect GEO-A-01: The position lookups dereference the null world volume of an unscanned geometry
  Passed = ExpectedToCrash(EvaluateTrue("GetDetector(Position)", "not scanned", "The call returns no detector and does not crash on an unscanned geometry", Unscanned.GetDetector(MVector(0, 0, 0)) == nullptr)) && Passed;
  // Defect GEO-A-01: The position lookups dereference the null world volume of an unscanned geometry
  Passed = ExpectedToCrash(EvaluateTrue("GetDetectorType()", "not scanned", "The call does not crash on an unscanned geometry", (Unscanned.GetDetectorType(MVector(0, 0, 0)), true))) && Passed;

  // Check that the constructor initializes the members with a recognizable pattern below:
  const unsigned char Pattern = 0xA7;
  vector<unsigned char> Block(sizeof(UTDGeometryAccess) + alignof(UTDGeometryAccess));
  fill(Block.begin(), Block.end(), Pattern);
  void* Aligned = reinterpret_cast<void*>((reinterpret_cast<size_t>(Block.data()) + alignof(UTDGeometryAccess) - 1) & ~(alignof(UTDGeometryAccess) - 1));
  UTDGeometryAccess* Placed = new (Aligned) UTDGeometryAccess;
  double PatternDouble = 0.0;
  fill_n(reinterpret_cast<unsigned char*>(&PatternDouble), sizeof(double), Pattern);
  bool ToleranceInitialized = Placed->m_DetectorSearchTolerance != PatternDouble;
  bool VirtualizeInitialized = *reinterpret_cast<unsigned char*>(&Placed->m_VirtualizeNonDetectorVolumes) != Pattern;
  bool ShowVolumesInitialized = *reinterpret_cast<unsigned char*>(&Placed->m_ShowVolumes) != Pattern;
  Placed->~UTDGeometryAccess();
  Passed = EvaluateTrue("MDGeometry()", "memory pattern", "The constructor initializes m_ShowVolumes", ShowVolumesInitialized) && Passed;
  // Defect GEO-A-02: The constructor leaves several members uninitialized until the first Reset
  Passed = ExpectedToFail(EvaluateTrue("MDGeometry()", "memory pattern", "The constructor initializes m_DetectorSearchTolerance", ToleranceInitialized)) && Passed;
  // Defect GEO-A-02: The constructor leaves several members uninitialized until the first Reset
  Passed = ExpectedToFail(EvaluateTrue("MDGeometry()", "memory pattern", "The constructor initializes m_VirtualizeNonDetectorVolumes", VirtualizeInitialized)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the file handling of the scan
bool UTDGeometry::TestFiles()
{
  bool Passed = true;

  MDGeometry Geometry;

  DisableDefaultStreams();
  bool Empty = Geometry.ScanSetupFile("", false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("ScanSetupFile()", "empty name", "An empty file name is rejected", Empty) && Passed;

  DisableDefaultStreams();
  bool Missing = Geometry.ScanSetupFile(GetTemporaryFileName("DoesNotExist.geo.setup"), false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("ScanSetupFile()", "missing file", "A missing file is rejected", Missing) && Passed;
  Passed = EvaluateFalse("IsScanned()", "missing file", "The geometry is not scanned after a missing file", Geometry.IsScanned()) && Passed;

  Passed = EvaluateFalse("ScanSetupFile()", "empty file", "An empty file is rejected", ScanGeometry(Geometry, "")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "only comments", "A file with only comments is rejected", ScanGeometry(Geometry, "// Nothing\n# Nothing\n")) && Passed;

  // A surrounding sphere is required:
  MString NoSphere = "Name N\nInclude $(MEGALIB)/resource/examples/geomega/materials/Materials.geo\nVolume World\nWorld.Material Vacuum\nWorld.Shape BRIK 1 1 1\nWorld.Mother 0\n";
  // Defect GEO-A-43: A missing surrounding sphere is not detected since the check compares with DBL_MAX instead of g_DoubleNotDefined
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "no surrounding sphere", "A geometry without a surrounding sphere is rejected", ScanGeometry(Geometry, NoSphere))) && Passed;

  // A world volume is required:
  MString NoWorld = "Name N\nSurroundingSphere 1 0 0 0 1\nInclude $(MEGALIB)/resource/examples/geomega/materials/Materials.geo\n";
  Passed = EvaluateFalse("ScanSetupFile()", "no world volume", "A geometry without a world volume is rejected", ScanGeometry(Geometry, NoWorld)) && Passed;

  // A directory is no geometry:
  PrepareTemporaryDirectory("ADirectory");
  Int_t ErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  bool Directory = true;
  BeginCaptureOutput();
  Directory = Geometry.ScanSetupFile(GetTemporaryDirectoryName("ADirectory"), false, false, false);
  EndCaptureOutput();
  gErrorIgnoreLevel = ErrorLevel;
  Passed = EvaluateFalse("ScanSetupFile()", "directory", "A directory is rejected", Directory) && Passed;

  // A minimal valid geometry:
  Passed = EvaluateTrue("ScanSetupFile()", "minimal geometry", "The minimal geometry is accepted", ScanGeometry(Geometry, GetGeometryHead())) && Passed;
  Passed = EvaluateTrue("IsScanned()", "minimal geometry", "The geometry is scanned", Geometry.IsScanned()) && Passed;
  Passed = EvaluateTrue("GetFileName()", "minimal geometry", "The file name is stored", Geometry.GetFileName() == GetTemporaryFileName("Fixture.geo.setup")) && Passed;
  Passed = EvaluateTrue("GetName()", "minimal geometry", "The name is stored", Geometry.GetName() == "UnitTest") && Passed;
  Passed = EvaluateTrue("GetWorldVolume()", "minimal geometry", "The world volume is stored", Geometry.GetWorldVolume() == Geometry.GetVolume("World")) && Passed;
  Passed = EvaluateNear("GetStartSphereRadius()", "minimal geometry", "The radius is the one of the head", Geometry.GetStartSphereRadius(), 50.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetStartSphereDistance()", "minimal geometry", "The distance is the one of the head", Geometry.GetStartSphereDistance(), 50.0, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetStartSpherePosition()", "minimal geometry", "The position is the one of the head", Geometry.GetStartSpherePosition(), MVector(0, 0, 0), 1e-12) && Passed;
  Passed = Evaluate("GetNVolumes()", "minimal geometry", "There is one volume", Geometry.GetNVolumes(), 1U) && Passed;

  // A relative file name is relative to the working directory:
  const MString Absolute = GetTemporaryFileName("Relative.geo.setup");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Absolute, GetGeometryHead())) && Passed;
  MString Working = gSystem->WorkingDirectory();
  gSystem->ChangeDirectory(GetTemporaryDirectoryName());
  DisableDefaultStreams();
  bool Relative = Geometry.ScanSetupFile("Relative.geo.setup", false, false, false);
  EnableDefaultStreams();
  gSystem->ChangeDirectory(Working);
  Passed = EvaluateTrue("ScanSetupFile()", "relative name", "A relative file name is found in the working directory", Relative) && Passed;
  Passed = EvaluateTrue("GetFileName()", "relative name", "The stored file name ends with the file name", Geometry.GetFileName().EndsWith("/Relative.geo.setup")) && Passed;
  Passed = EvaluateTrue("GetFileName()", "relative name", "The stored file name is absolute", gSystem->IsAbsoluteFileName(Geometry.GetFileName()) == kTRUE) && Passed;

  // An environment variable in the file name is expanded:
  gSystem->Setenv("UTDGEOMETRYDIR", GetTemporaryDirectoryName());
  DisableDefaultStreams();
  bool Expanded = Geometry.ScanSetupFile("$(UTDGEOMETRYDIR)/Relative.geo.setup", false, false, false);
  EnableDefaultStreams();
  gSystem->Unsetenv("UTDGEOMETRYDIR");
  Passed = EvaluateTrue("ScanSetupFile()", "environment variable", "An environment variable in the file name is expanded", Expanded) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test Include and the deprecated material path
bool UTDGeometry::TestInclude()
{
  bool Passed = true;

  PrepareTemporaryDirectory("Include");
  const MString Dir = GetTemporaryDirectoryName("Include");

  // One include, next to the main file:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/A.geo", Marker("InA"))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Main1.geo.setup", GetGeometryHead() + "Include A.geo\n")) && Passed;
  MDGeometry Geometry;
  DisableDefaultStreams();
  bool Result = Geometry.ScanSetupFile(Dir + "/Main1.geo.setup", false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "relative include", "An include next to the main file is found", Result) && Passed;
  Passed = EvaluateTrue("GetVolume()", "relative include", "The content of the include is read", Geometry.GetVolume("InA") != nullptr) && Passed;
  // Main file, Materials.geo, A.geo:
  Passed = Evaluate("GetNIncludes()", "relative include", "Three files are read", Geometry.GetNIncludes(), 3) && Passed;

  // The same file twice only counts once:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Main2.geo.setup", GetGeometryHead() + "Include A.geo\nInclude A.geo\n")) && Passed;
  DisableDefaultStreams();
  Result = Geometry.ScanSetupFile(Dir + "/Main2.geo.setup", false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "include twice", "A file included twice is only read once", Result) && Passed;
  Passed = Evaluate("GetNIncludes()", "include twice", "Three files are read", Geometry.GetNIncludes(), 3) && Passed;

  // An include of an include:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/B.geo", "Include C.geo\n" + Marker("InB"))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/C.geo", Marker("InC", 3.0))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Main3.geo.setup", GetGeometryHead() + "Include B.geo\n")) && Passed;
  DisableDefaultStreams();
  Result = Geometry.ScanSetupFile(Dir + "/Main3.geo.setup", false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "nested include", "An include of an include is read", Result) && Passed;
  Passed = EvaluateTrue("GetVolume()", "nested include", "The content of the nested include is read", Geometry.GetVolume("InC") != nullptr) && Passed;

  // Includes which include each other end:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Cycle1.geo", "Include Cycle2.geo\n" + Marker("InCycle1"))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Cycle2.geo", "Include Cycle1.geo\n" + Marker("InCycle2", 3.0))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Main4.geo.setup", GetGeometryHead() + "Include Cycle1.geo\n")) && Passed;
  DisableDefaultStreams();
  Result = Geometry.ScanSetupFile(Dir + "/Main4.geo.setup", false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "cyclic include", "Cyclic includes end and every file is read once", Result) && Passed;
  Passed = Evaluate("GetNVolumes()", "cyclic include", "World and the two volumes of the files exist", Geometry.GetNVolumes(), 3U) && Passed;

  // A file which includes itself:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Main5.geo.setup", GetGeometryHead() + "Include Main5.geo.setup\n")) && Passed;
  DisableDefaultStreams();
  Result = Geometry.ScanSetupFile(Dir + "/Main5.geo.setup", false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "self include", "A file which includes itself is only read once", Result) && Passed;

  // Missing and bad includes:
  Passed = EvaluateFalse("ScanSetupFile()", "missing include", "A missing include is rejected", Accepts("Include DoesNotExist.geo\n")) && Passed;
  Passed = EvaluateFalse("IsScanned()", "after missing include", "The geometry is not scanned", Geometry.IsScanned() == true && Accepts("Include DoesNotExist.geo\n") == true) && Passed;

  // Defect GEO-A-03: Two spellings of the path of one file are read twice, which is rejected because of the duplicate names
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Main6.geo.setup", GetGeometryHead() + "Include A.geo\nInclude ./A.geo\n")) && Passed;
  DisableDefaultStreams();
  Result = Geometry.ScanSetupFile(Dir + "/Main6.geo.setup", false, false, false);
  EnableDefaultStreams();
  Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", "same file, two spellings", "The same file reached with another spelling of its path is only read once", Result)) && Passed;

  // The deprecated material path still works, but is flagged:
  MFile::CreateDirectory(Dir + "/resource/geometries/materials");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/resource/geometries/materials/Materials.geo", "")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/Main7.geo.setup", GetGeometryHead() + "Include resource/geometries/materials/Materials.geo\n")) && Passed;
  BeginCaptureOutput();
  Result = Geometry.ScanSetupFile(Dir + "/Main7.geo.setup", false, false, false);
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("ScanSetupFile()", "old material path", "The old material path is accepted", Result) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "old material path", "The old material path is flagged", Output.Contains("old MEGAlib material path")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test Exit, comments, line continuation, and very long lines
bool UTDGeometry::TestLines()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Comments:
  Passed = EvaluateTrue("ScanSetupFile()", "comments", "All comment forms are accepted", ScanGeometry(Geometry, GetGeometryHead() + "// Volume A\n# Volume B\n! Volume C\n" + Marker("Box") + "Box.Color 4 // A comment\n")) && Passed;
  Passed = Evaluate("GetNVolumes()", "comments", "The commented volumes are not read", Geometry.GetNVolumes(), 2U) && Passed;
  Passed = Evaluate("MDVolume::GetColor()", "trailing comment", "The value before a trailing comment is read", Geometry.GetVolume("Box")->GetColor(), 4) && Passed;

  // Comment blocks, also nested:
  Passed = EvaluateTrue("ScanSetupFile()", "comment block", "A comment block is accepted", ScanGeometry(Geometry, GetGeometryHead() + "BeginComment\n" + Marker("A") + "BeginComment\n" + Marker("B") + "EndComment\n" + Marker("C") + "EndComment\n" + Marker("D"))) && Passed;
  Passed = EvaluateTrue("BeginComment", "nested", "Only the volume outside of the comment blocks is read", ExistingVolumes(Geometry, {"A", "B", "C", "D"}) == "D ") && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "EndComment alone", "An EndComment without BeginComment is rejected", Accepts("EndComment\n")) && Passed;

  // Exit stops reading this file:
  Passed = EvaluateTrue("ScanSetupFile()", "Exit", "A file with Exit is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("A") + "Exit\n" + Marker("B", 3.0))) && Passed;
  Passed = EvaluateTrue("Exit", "volume after Exit", "The content after Exit is not read", ExistingVolumes(Geometry, {"A", "B"}) == "A ") && Passed;

  // Defect GEO-A-04: Exit inside of a comment block is executed
  Passed = EvaluateTrue("ScanSetupFile()", "Exit in comment", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "BeginComment\nExit\nEndComment\n" + Marker("A"))) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("Exit", "inside of a comment block", "A commented Exit does not stop reading the file", Geometry.GetVolume("A") != nullptr)) && Passed;

  // Defect GEO-A-05: A comment block which is never closed silently swallows the rest of the file
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "BeginComment alone", "A BeginComment without EndComment is rejected", Accepts("BeginComment\n" + Marker("A")))) && Passed;

  // Line continuation:
  Passed = EvaluateTrue("ScanSetupFile()", "continuation", "A continued line is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("A") + "A.Position 1.0 \\\\\n 2.0 \\\\\n 3.0\n")) && Passed;
  Passed = CheckPosition(Geometry, "A", MVector(1.0, 2.0, 3.0), "continued position") && Passed;

  // Lines just below the length limit of 9999 characters are read:
  MString Below = "// ";
  while (Below.Length() < 9999) {
    Below += "x";
  }
  Passed = EvaluateTrue("ScanSetupFile()", "line of 9999 characters", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + Below + "\n" + Marker("A"))) && Passed;
  Passed = EvaluateTrue("AddFile()", "line of 9999 characters", "The content after the line is read", Geometry.GetVolume("A") != nullptr) && Passed;

  // Defect GEO-A-06: A line of 10000 characters or more silently ends reading the file
  MString Above = Below + "x";
  Passed = EvaluateTrue("ScanSetupFile()", "line of 10000 characters", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + Above + "\n" + Marker("A"))) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("AddFile()", "line of 10000 characters", "The content after the line is read", Geometry.GetVolume("A") != nullptr)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword Constant
bool UTDGeometry::TestConstants()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Simple values:
  Passed = EvaluateTrue("ScanSetupFile()", "constants", "A constant is accepted", ScanGeometry(Geometry, GetGeometryHead() + "Constant PosX 1.5\n" + Marker("A") + "A.Position PosX 0.0 0.0\n")) && Passed;
  Passed = CheckPosition(Geometry, "A", MVector(1.5, 0, 0), "simple constant") && Passed;

  // Maths, constants in constants, and the definition after the use: 2.5 * (1.5 + 0.5) = 5.0
  Passed = EvaluateTrue("ScanSetupFile()", "maths in constants", "Constants of constants are accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("A") + "A.Position Total 0.0 0.0\nConstant Total { 2.5 * Sum }\nConstant Sum { Base + 0.5 }\nConstant Base 1.5\n")) && Passed;
  Passed = CheckPosition(Geometry, "A", MVector(5.0, 0, 0), "constants of constants") && Passed;

  // A constant used in maths on a keyword line: 2 * 1.5 = 3
  Passed = EvaluateTrue("ScanSetupFile()", "constant in maths", "A constant in a maths expression is accepted", ScanGeometry(Geometry, GetGeometryHead() + "Constant Base 1.5\n" + Marker("A") + "A.Position { 2 * Base } 0.0 0.0\n")) && Passed;
  Passed = CheckPosition(Geometry, "A", MVector(3.0, 0, 0), "constant in maths") && Passed;

  // Only whole words are replaced:
  Passed = EvaluateTrue("ScanSetupFile()", "whole words", "The geometry is accepted", ScanGeometry(Geometry, GetGeometryHead() + "Constant A 1.5\n" + Marker("AB") + "AB.Position A 0.0 0.0\n")) && Passed;
  Passed = CheckPosition(Geometry, "AB", MVector(1.5, 0, 0), "whole word replacement") && Passed;

  // The same constant twice:
  Passed = EvaluateTrue("ScanSetupFile()", "identical constants", "Identical definitions are accepted", Accepts("Constant A 1.5\nConstant A 1.5\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "different constants", "Different definitions are rejected", Accepts("Constant A 1.5\nConstant A 2.5\n")) && Passed;

  // Rejections:
  Passed = EvaluateFalse("ScanSetupFile()", "two tokens", "A constant without a value is rejected", Accepts("Constant A\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "four tokens", "A constant with two values is rejected", Accepts("Constant A 1 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "identical name and value", "A constant which replaces itself is rejected", Accepts("Constant A A\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "bad maths", "Bad maths in a constant is rejected", Accepts("Constant A { 1 + foo }\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "recursive maths", "Recursively defined maths is rejected", Accepts("Constant A { B + 1 }\nConstant B { A + 1 }\n")) && Passed;

  // A constant has the same name space as the objects, also in another case:
  Passed = EvaluateFalse("ScanSetupFile()", "constant named like a volume", "A constant with the name of a volume is rejected", Accepts("Constant Clash 1.5\n" + Marker("Clash"))) && Passed;
  // Defect GEO-A-69: The check for existing names is case sensitive, while its comment says that names are case insensitive
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "constant named like a volume in another case", "A constant with the name of a volume in another case is rejected", Accepts("Constant clash 1.5\n" + Marker("Clash")))) && Passed;

  // Reserved names:
  for (const MString Name: {"Volume", "Material", "System", "Trigger", "TriggerBasic", "MDStrip2D", "Strip3D", "MDStrip3D", "Strip3DDirectional", "DriftChamber", "AngerCamera", "Simple", "Scintillator", "ACS", "Calorimeter", "Voxel3D", "If", "Endif", "For", "Done", "Vector", "H", "Si", "Lr"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Name, "The reserved name " + Name + " cannot be a constant", Accepts("Constant " + Name + " 1.5\n")) && Passed;
  }
  // Defect GEO-A-07: A missing comma merges the reserved constant names TriggerMap and Strip2D into one name
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "TriggerMap", "The reserved name TriggerMap cannot be a constant", Accepts("Constant TriggerMap 1.5\n"))) && Passed;
  // Defect GEO-A-07: A missing comma merges the reserved constant names TriggerMap and Strip2D into one name
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "Strip2D", "The reserved name Strip2D cannot be a constant", Accepts("Constant Strip2D 1.5\n"))) && Passed;

  // Defect GEO-A-08: Two constants which refer to each other without maths hang the scan forever
  Passed = ExpectedToCrash(EvaluateFalse("ScanSetupFile()", "cyclic constants", "Cyclic constants are rejected and the scan ends", Accepts("Constant CycleOne CycleTwo\nConstant CycleTwo CycleOne\n"))) && Passed;

  // Defect GEO-A-09: The search for a constant in a maths expression stops at the first partial match such as the n in sin
  Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", "constant named like a part of a function", "The constant n in the expression sin(n) is found", Accepts("Constant n 0.0\nConstant Val { sin(n) }\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keywords Vector and ForVector
bool UTDGeometry::TestVectors()
{
  bool Passed = true;
  MDGeometry Geometry;

  // A 2 x 2 x 2 matrix: the values run through x first, then y, then z
  MString Matrix = "Vector M\nM.Matrix 2 1.5  2 2.5  2 3.5  1 2 3 4 5 6 7 8\n";
  Passed = EvaluateTrue("ScanSetupFile()", "matrix", "The matrix is accepted", ScanGeometry(Geometry, GetGeometryHead() + Matrix)) && Passed;
  Passed = Evaluate("GetNVectors()", "matrix", "There is one vector", Geometry.GetNVectors(), 1U) && Passed;
  MDVector* V = Geometry.GetVector("M");
  if (EvaluateTrue("GetVector()", "matrix", "The vector can be found", V != nullptr) == true) {
    Passed = EvaluateSize("MDVector::GetSize()", "matrix", "The vector has 8 entries", V->GetSize(), 8) && Passed;
    Passed = EvaluateVectorNear("MDVector::GetPosition()", "index 0", "The first entry is at the origin", V->GetPosition(0), MVector(0, 0, 0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("MDVector::GetPosition()", "index 1", "The second entry is one step in x", V->GetPosition(1), MVector(1.5, 0, 0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("MDVector::GetPosition()", "index 2", "The third entry is one step in y", V->GetPosition(2), MVector(0, 2.5, 0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("MDVector::GetPosition()", "index 4", "The fifth entry is one step in z", V->GetPosition(4), MVector(0, 0, 3.5), 1e-12) && Passed;
    Passed = EvaluateVectorNear("MDVector::GetPosition()", "index 7", "The last entry is the far corner", V->GetPosition(7), MVector(1.5, 2.5, 3.5), 1e-12) && Passed;
    Passed = EvaluateNear("MDVector::GetValue()", "index 5", "The value of the entry at x = 1, y = 0, z = 1 is the sixth", V->GetValue(5), 6.0, 1e-12) && Passed;
  }

  // One by one by one is the smallest matrix:
  Passed = EvaluateTrue("ScanSetupFile()", "1 x 1 x 1", "The smallest matrix is accepted", ScanGeometry(Geometry, GetGeometryHead() + "Vector M\nM.Matrix 1 1.0 1 1.0 1 1.0 7\n")) && Passed;
  Passed = EvaluateSize("MDVector::GetSize()", "1 x 1 x 1", "The vector has one entry", Geometry.GetVector("M")->GetSize(), 1) && Passed;

  // A vector without data has no entries:
  Passed = EvaluateTrue("ScanSetupFile()", "no matrix", "A vector without data is accepted", ScanGeometry(Geometry, GetGeometryHead() + "Vector M\n")) && Passed;
  Passed = EvaluateSize("MDVector::GetSize()", "no matrix", "The vector has no entries", Geometry.GetVector("M")->GetSize(), 0) && Passed;

  // Rejections:
  Passed = EvaluateFalse("ScanSetupFile()", "no name", "A vector without a name is rejected", Accepts("Vector\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "two names", "A vector with two names is rejected", Accepts("Vector A B\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "invalid name", "A vector with an invalid name is rejected", Accepts("Vector A-B\n")) && Passed;
  // Defect GEO-A-69: The check for existing names is case sensitive, while its comment says that names are case insensitive
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "duplicate name", "A second vector of the same name is rejected", Accepts("Vector A\nVector a\n"))) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "unknown option", "An unknown option of a vector is rejected", Accepts("Vector M\nM.Unknown 1 2 3\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "too few tokens", "A matrix without enough tokens is rejected", Accepts("Vector M\nM.Matrix 1 1.0 1 1.0 1 1.0\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "too few values", "A matrix with too few values is rejected", Accepts("Vector M\nM.Matrix 2 1.0 1 1.0 1 1.0 7\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "too many values", "A matrix with too many values is rejected", Accepts("Vector M\nM.Matrix 1 1.0 1 1.0 1 1.0 7 8\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "negative size", "A matrix with a negative size is rejected", Accepts("Vector M\nM.Matrix -1 1.0 1 1.0 1 1.0 7\n")) && Passed;

  // ForVector: the numbers are made by hand from the matrix values
  MString Loop = "Vector M\nM.Matrix 2 3.5  1 0.0  1 0.0  10 20\nForVector M X Y Z W\n";
  Loop += "Volume Loop_%X\nLoop_%X.Material Aluminium\nLoop_%X.Shape BRIK 1 1 1\nLoop_%X.Position $X $Y $W\nLoop_%X.Mother World\nDoneVector\n";
  Passed = EvaluateTrue("ScanSetupFile()", "ForVector", "ForVector is accepted", ScanGeometry(Geometry, GetGeometryHead() + Loop)) && Passed;
  Passed = CheckPosition(Geometry, "Loop_1", MVector(0.0, 0.0, 10.0), "ForVector first") && Passed;
  Passed = CheckPosition(Geometry, "Loop_2", MVector(3.5, 0.0, 20.0), "ForVector second") && Passed;
  Passed = Evaluate("GetNVolumes()", "ForVector", "The loop body is expanded once per entry", Geometry.GetNVolumes(), 3U) && Passed;

  Passed = EvaluateFalse("ScanSetupFile()", "ForVector, missing vector", "A ForVector with an unknown vector is rejected", Accepts("ForVector M X Y Z W\nDoneVector\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "ForVector, too few tokens", "A ForVector without all indices is rejected", Accepts("Vector M\nForVector M X Y Z\nDoneVector\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "ForVector, empty vector", "A ForVector over an empty vector does nothing", Accepts("Vector M\nForVector M X Y Z W\nMissing.Position 1 2 3\nDoneVector\n")) && Passed;

  // Defect GEO-A-10: A ForVector without DoneVector is not detected
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "ForVector, no DoneVector", "A ForVector without DoneVector is rejected", Accepts("Vector M\nM.Matrix 1 1.0 1 1.0 1 1.0 7\nForVector M X Y Z W\n" + Marker("A")))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword For
bool UTDGeometry::TestFor()
{
  bool Passed = true;
  MDGeometry Geometry;

  MString Body = "Volume L_%I\nL_%I.Material Aluminium\nL_%I.Shape BRIK 1 1 1\nL_%I.Position $I 0.0 0.0\nL_%I.Mother World\n";

  // Three loops with the values 1.5, 3.5, 5.5:
  Passed = EvaluateTrue("ScanSetupFile()", "For", "A for loop is accepted", ScanGeometry(Geometry, GetGeometryHead() + "For I 3 1.5 2.0\n" + Body + "Done\n")) && Passed;
  Passed = Evaluate("GetNVolumes()", "For", "The body is expanded three times", Geometry.GetNVolumes(), 4U) && Passed;
  Passed = CheckPosition(Geometry, "L_1", MVector(1.5, 0, 0), "For first") && Passed;
  Passed = CheckPosition(Geometry, "L_3", MVector(5.5, 0, 0), "For last") && Passed;

  // The lower case form:
  Passed = EvaluateTrue("ScanSetupFile()", "for", "The lower case form is accepted", ScanGeometry(Geometry, GetGeometryHead() + "for I 2 0.0 3.0\n" + Body + "done\n")) && Passed;
  Passed = Evaluate("GetNVolumes()", "for", "The body is expanded twice", Geometry.GetNVolumes(), 3U) && Passed;

  // Boundaries of the loop number: 0, 1
  Passed = EvaluateTrue("ScanSetupFile()", "no loops", "A loop without iterations is accepted", ScanGeometry(Geometry, GetGeometryHead() + "For I 0 0.0 1.0\n" + Body + "Done\n")) && Passed;
  Passed = Evaluate("GetNVolumes()", "no loops", "The body is not expanded", Geometry.GetNVolumes(), 1U) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "one loop", "A loop with one iteration is accepted", ScanGeometry(Geometry, GetGeometryHead() + "For I 1 0.0 1.0\n" + Body + "Done\n")) && Passed;
  Passed = Evaluate("GetNVolumes()", "one loop", "The body is expanded once", Geometry.GetNVolumes(), 2U) && Passed;

  // Maths and constants in the head of the loop:
  Passed = EvaluateTrue("ScanSetupFile()", "maths in For", "Maths in the head is accepted", ScanGeometry(Geometry, GetGeometryHead() + "Constant Number 2\nFor I { Number + 1 } { 2 * 3 } 2\n" + Body + "Done\n")) && Passed;
  Passed = Evaluate("GetNVolumes()", "maths in For", "The body is expanded three times", Geometry.GetNVolumes(), 4U) && Passed;
  Passed = CheckPosition(Geometry, "L_3", MVector(10.0, 0, 0), "maths in For: 6 + 2 * 2") && Passed;

  // Nested loops: outer value * 10 + inner value
  MString Nested = "For A 2 1 1\nFor B 2 1 1\nVolume N_%A_%B\nN_%A_%B.Material Aluminium\nN_%A_%B.Shape BRIK 1 1 1\nN_%A_%B.Position { 10 * $A + $B } 0.0 0.0\nN_%A_%B.Mother World\nDone\nDone\n";
  Passed = EvaluateTrue("ScanSetupFile()", "nested For", "Nested loops are accepted", ScanGeometry(Geometry, GetGeometryHead() + Nested)) && Passed;
  Passed = Evaluate("GetNVolumes()", "nested For", "The body is expanded four times", Geometry.GetNVolumes(), 5U) && Passed;
  Passed = CheckPosition(Geometry, "N_2_1", MVector(21.0, 0, 0), "nested For") && Passed;

  // Two loops one after the other:
  MString Second = "For J 2 0.0 1.0\nVolume M_%J\nM_%J.Material Aluminium\nM_%J.Shape BRIK 1 1 1\nM_%J.Position $J 0.0 0.0\nM_%J.Mother World\nDone\n";
  Passed = EvaluateTrue("ScanSetupFile()", "two For", "Two loops are accepted", ScanGeometry(Geometry, GetGeometryHead() + "For I 2 0.0 1.0\n" + Body + "Done\n" + Second)) && Passed;
  Passed = Evaluate("GetNVolumes()", "two For", "Both bodies are expanded", Geometry.GetNVolumes(), 5U) && Passed;

  // The precision of the loop values: 0.1 + 2 * 0.2 = 0.5 and a large offset with 15 significant digits
  Passed = EvaluateTrue("ScanSetupFile()", "fractional steps", "Fractional steps are accepted", ScanGeometry(Geometry, GetGeometryHead() + "For I 3 12345.6789012 0.0000001\n" + Body + "Done\n")) && Passed;
  Passed = EvaluateNear("For", "large value, small step", "The third value keeps its digits", Geometry.GetVolume("L_3")->GetPosition().X(), 12345.6789014, 1e-9) && Passed;

  // Rejections:
  Passed = EvaluateFalse("ScanSetupFile()", "too few tokens", "A For with four tokens is rejected", Accepts("For I 3 1.0\nDone\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "too many tokens", "A For with six tokens is rejected", Accepts("For I 3 1.0 1.0 1.0\nDone\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "negative loop number", "A negative loop number is rejected", Accepts("For I -1 1.0 1.0\nDone\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "fractional loop number", "A fractional loop number is rejected", Accepts("For I 1.5 1.0 1.0\nDone\n")) && Passed;
  // Defect GEO-A-21: A non-numeric value of a numeric keyword is silently read as 0
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "text as loop number", "A text as loop number is rejected", Accepts("For I abc 1.0 1.0\nDone\n"))) && Passed;

  // Defect GEO-A-11: A For without Done is not detected
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "no Done", "A For without Done is rejected", Accepts("For I 2 1.0 1.0\n" + Body))) && Passed;

  // Defect GEO-A-12: The index of a loop is also replaced in a longer index which starts with it, which gives wrong values silently
  MString Prefix = "For A 2 1.0 1.0\nFor A1 1 5.0 1.0\nVolume P_%A\nP_%A.Material Aluminium\nP_%A.Shape BRIK 0.1 0.1 0.1\nP_%A.Position $A $A1 0.0\nP_%A.Mother World\nDone\nDone\n";
  Passed = EvaluateTrue("ScanSetupFile()", "index A and index A1", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + Prefix)) && Passed;
  // The outer loop has the values 1, 2 and the inner one only the value 5, thus the second volume is at (2, 5, 0), not at (2, 21, 0):
  Passed = ExpectedToFail(EvaluateVectorNear("For", "index A and index A1", "The value of the inner loop is used", Geometry.GetVolume("P_2")->GetPosition(), MVector(2, 5, 0), 1e-9)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword RandomDouble
bool UTDGeometry::TestRandom()
{
  bool Passed = true;
  MDGeometry Geometry;

  // The random numbers come from a TRandom3 with the fixed seed 11031879 in the order of the file
  TRandom3 Reference;
  Reference.SetSeed(11031879);
  double First = Reference.Rndm();
  double Second = Reference.Rndm();
  double Third = Reference.Rndm();

  MString Text = GetGeometryHead() + Marker("A") + "A.Position RandomDouble RandomDouble 0.0\n" + Marker("B") + "B.Position { 2 * RandomDouble } 0.0 0.0\n";
  Passed = EvaluateTrue("ScanSetupFile()", "RandomDouble", "RandomDouble is accepted", ScanGeometry(Geometry, Text)) && Passed;
  Passed = EvaluateNear("RandomDouble", "first in the line", "The first value is the first random number", Geometry.GetVolume("A")->GetPosition().X(), First, 1e-12) && Passed;
  Passed = EvaluateNear("RandomDouble", "second in the line", "The second value is the second random number", Geometry.GetVolume("A")->GetPosition().Y(), Second, 1e-12) && Passed;
  Passed = EvaluateNear("RandomDouble", "in maths", "A value in maths is the third random number", Geometry.GetVolume("B")->GetPosition().X(), 2*Third, 1e-12) && Passed;

  // A second scan gives the same values:
  Passed = EvaluateTrue("ScanSetupFile()", "RandomDouble, second scan", "The second scan is accepted", ScanGeometry(Geometry, Text)) && Passed;
  Passed = EvaluateNear("RandomDouble", "second scan", "The second scan gives the same value", Geometry.GetVolume("A")->GetPosition().X(), First, 1e-12) && Passed;

  // In a loop each iteration has its own number:
  Text = GetGeometryHead() + "For I 2 0 1\nVolume L_%I\nL_%I.Material Aluminium\nL_%I.Shape BRIK 1 1 1\nL_%I.Position RandomDouble 0.0 0.0\nL_%I.Mother World\nDone\n";
  Passed = EvaluateTrue("ScanSetupFile()", "RandomDouble in For", "RandomDouble in a loop is accepted", ScanGeometry(Geometry, Text)) && Passed;
  Passed = EvaluateNear("RandomDouble", "first iteration", "The first iteration has the first number", Geometry.GetVolume("L_1")->GetPosition().X(), First, 1e-12) && Passed;
  Passed = EvaluateNear("RandomDouble", "second iteration", "The second iteration has the second number", Geometry.GetVolume("L_2")->GetPosition().X(), Second, 1e-12) && Passed;

  // The random numbers in a geometry do not change the global generator:
  gRandom->SetSeed(1234);
  double Expected = gRandom->Rndm();
  gRandom->SetSeed(1234);
  ScanGeometry(Geometry, Text);
  Passed = EvaluateNear("RandomDouble", "gRandom", "The scan leaves the global random number generator alone", gRandom->Rndm(), Expected, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keywords If, Else, and EndIf
bool UTDGeometry::TestIf()
{
  bool Passed = true;
  MDGeometry Geometry;

  // The plain forms, all spellings of the end, and maths with constants:
  struct IfCase {
    MString Text;
    MString Expected;
    MString Label;
  };
  vector<IfCase> Cases = {
    { "If { 1 == 1 }\n" + Marker("A") + "EndIf\n" + Marker("Z", 3), "A Z ", "true without Else" },
    { "If { 1 == 2 }\n" + Marker("A") + "EndIf\n" + Marker("Z", 3), "Z ", "false without Else" },
    { "If { 1 == 1 }\n" + Marker("A") + "Else\n" + Marker("B") + "EndIf\n" + Marker("Z", 3), "A Z ", "true with Else" },
    { "If { 1 == 2 }\n" + Marker("A") + "Else\n" + Marker("B") + "EndIf\n" + Marker("Z", 3), "B Z ", "false with Else" },
    { "If { 1 == 1 }\n" + Marker("A") + "Endif\n" + Marker("Z", 3), "A Z ", "spelling Endif" },
    { "If { 1 == 1 }\n" + Marker("A") + "endif\n" + Marker("Z", 3), "A Z ", "spelling endif" },
    { "If { 1 == 2 }\n" + Marker("A") + "else\n" + Marker("B") + "endif\n" + Marker("Z", 3), "B Z ", "spelling else" },
    { "Constant Number 3\nIf { Number > 2 }\n" + Marker("A") + "EndIf\n", "A ", "constant in the condition" },
    { "If { 1 == 2 }\n" + Marker("A") + "EndIf\nIf { 2 == 2 }\n" + Marker("B") + "EndIf\n", "B ", "two in a row" },
    { "If { 1 == 2 }\nBad.Unknown 1\nEndIf\n", "", "false branch is not interpreted" },
    // Nesting without Else:
    { "If { 1 == 1 }\nIf { 1 == 1 }\n" + Marker("A") + "EndIf\n" + Marker("B", 3) + "EndIf\n", "A B ", "nested: true, true" },
    { "If { 1 == 1 }\nIf { 1 == 2 }\n" + Marker("A") + "EndIf\n" + Marker("B", 3) + "EndIf\n", "B ", "nested: true, false" },
    { "If { 1 == 2 }\nIf { 1 == 1 }\n" + Marker("A") + "EndIf\n" + Marker("B", 3) + "EndIf\n" + Marker("Z", 6), "Z ", "nested: false, true" },
    { "If { 1 == 2 }\nIf { 1 == 2 }\n" + Marker("A") + "EndIf\n" + Marker("B", 3) + "EndIf\n" + Marker("Z", 6), "Z ", "nested: false, false" },
  };
  for (const IfCase& Case: Cases) {
    bool Result = ScanGeometry(Geometry, GetGeometryHead() + Case.Text);
    Passed = EvaluateTrue("ScanSetupFile()", Case.Label, "The file is accepted", Result) && Passed;
    Passed = EvaluateTrue("If", Case.Label, "Exactly the volumes of the taken branches exist", ExistingVolumes(Geometry, {"A", "B", "Z"}) == Case.Expected) && Passed;
  }

  // If in For: value 1 and 2 go through the first branch, value 3 through the second
  MString Loop = "For I 3 1 1\nIf { $I < 3 }\n";
  Loop += "Volume Low_%I\nLow_%I.Material Aluminium\nLow_%I.Shape BRIK 1 1 1\nLow_%I.Position $I 0 0\nLow_%I.Mother World\n";
  Loop += "Else\n";
  Loop += "Volume High_%I\nHigh_%I.Material Aluminium\nHigh_%I.Shape BRIK 1 1 1\nHigh_%I.Position $I 0 0\nHigh_%I.Mother World\n";
  Loop += "EndIf\nDone\n";
  Passed = EvaluateTrue("ScanSetupFile()", "If in For", "If in For is accepted", ScanGeometry(Geometry, GetGeometryHead() + Loop)) && Passed;
  Passed = EvaluateTrue("If", "If in For", "The branches follow the loop value", ExistingVolumes(Geometry, {"Low_1", "Low_2", "Low_3", "High_1", "High_2", "High_3"}) == "Low_1 Low_2 High_3 ") && Passed;

  // The nested form with Else on two levels: truth table by hand
  // Every branch has its own volume, and After is outside of all
  vector<MString> Names = {"OuterTrue1", "OuterTrue2", "InnerTT", "InnerTF", "OuterElse1", "OuterElse2", "InnerFT", "InnerFF", "After"};
  for (int Outer = 0; Outer <= 1; ++Outer) {
    for (int Inner = 0; Inner <= 1; ++Inner) {
      MString Text = "Constant OuterCondition "; Text += Outer; Text += "\nConstant InnerCondition "; Text += Inner; Text += "\n";
      Text += "If { OuterCondition == 1 }\n" + Marker("OuterTrue1");
      Text += "If { InnerCondition == 1 }\n" + Marker("InnerTT") + "Else\n" + Marker("InnerTF") + "EndIf\n";
      Text += Marker("OuterTrue2");
      Text += "Else\n" + Marker("OuterElse1");
      Text += "If { InnerCondition == 1 }\n" + Marker("InnerFT") + "Else\n" + Marker("InnerFF") + "EndIf\n";
      Text += Marker("OuterElse2");
      Text += "EndIf\n" + Marker("After");

      MString Expected;
      if (Outer == 1) {
        Expected = "OuterTrue1 OuterTrue2 ";
        if (Inner == 1) {
          Expected += "InnerTT ";
        } else {
          Expected += "InnerTF ";
        }
      } else {
        Expected = "OuterElse1 OuterElse2 ";
        if (Inner == 1) {
          Expected += "InnerFT ";
        } else {
          Expected += "InnerFF ";
        }
      }
      Expected += "After ";

      MString Label = "If/Else in If/Else, outer "; Label += Outer; Label += ", inner "; Label += Inner;
      bool Result = ScanGeometry(Geometry, GetGeometryHead() + Text);
      Passed = EvaluateTrue("ScanSetupFile()", Label, "The file is accepted", Result) && Passed;
      // Defect GEO-A-13: An Else of a nested If is taken as the Else of the outer If, which silently selects the wrong volumes
      Passed = ExpectedToFail(Evaluate("If", Label, "Exactly the volumes of the taken branches exist", ExistingVolumes(Geometry, Names), Expected)) && Passed;
    }
  }

  // Rejections:
  Passed = EvaluateFalse("ScanSetupFile()", "no condition", "An If without a condition is rejected", Accepts("If\nEndIf\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "two conditions", "An If with two conditions is rejected", Accepts("If 1 1\nEndIf\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "bad maths", "An If with bad maths is rejected", Accepts("If { foo == 1 }\nEndIf\n")) && Passed;

  // Defect GEO-A-14: An If without EndIf is not detected and hides the rest of the file silently
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "no EndIf", "An If without EndIf is rejected", Accepts("If { 1 == 2 }\n" + Marker("A")))) && Passed;

  // Defect GEO-A-15: The lower case if is ignored on the top level, while else, endif, for, and done work in lower case
  Passed = EvaluateTrue("ScanSetupFile()", "lower case if", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "if { 1 == 2 }\n" + Marker("A") + "endif\n")) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("If", "lower case if", "The false branch of a lower case if is not read", Geometry.GetVolume("A") == nullptr)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keywords Print and Echo
bool UTDGeometry::TestPrint()
{
  bool Passed = true;
  MDGeometry Geometry;
  const MString FileName = GetTemporaryFileName("Print.geo.setup");

  // Print shows the text after the evaluation of constants and maths: 1 + 2 = 3
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(FileName, GetGeometryHead() + "Constant Val 3.5\nPrint Text Val\nEcho { 1 + 2 }\n")) && Passed;
  bool Result = false;
  BeginCaptureOutput();
  Result = Geometry.ScanSetupFile(FileName, false, false, false);
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("ScanSetupFile()", "Print and Echo", "The file is accepted", Result) && Passed;
  Passed = EvaluateTrue("Print", "text and constant", "Print shows the text", Output.Contains("User Info: Text  3.5")) && Passed;
  Passed = EvaluateTrue("Echo", "maths", "Echo shows the result of the maths", Output.Contains("User Info: 3.0000")) && Passed;

  // No output for a false branch:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(FileName, GetGeometryHead() + "If { 1 == 2 }\nPrint Hidden\nEndIf\n")) && Passed;
  BeginCaptureOutput();
  Result = Geometry.ScanSetupFile(FileName, false, false, false);
  Output = EndCaptureOutput();
  Passed = EvaluateFalse("Print", "false branch", "A print in a false branch does not show up", Output.Contains("Hidden")) && Passed;

  // Rejections:
  Passed = EvaluateFalse("ScanSetupFile()", "Print without text", "Print needs a text", Accepts("Print\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Echo without text", "Echo needs a text", Accepts("Echo\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the first order keywords and their rejection
bool UTDGeometry::TestFirstOrderKeywords()
{
  bool Passed = true;
  UTDGeometryAccess Geometry;

  // Wrong number of tokens for every keyword:
  for (const MString Line: {"Name", "Name A B", "Version", "Version 1 2", "ShowSurroundingSphere", "ShowSurroundingSphere true false", "ShowVolumes", "ShowVolumes a b",
    "ShowOnlySensitiveVolumes", "ShowOnlySensitiveVolumes a b", "DoSanityChecks", "DoSanityChecks a b", "VirtualizeNonDetectorVolumes", "VirtualizeNonDetectorVolumes a b",
    "ComplexER", "ComplexER a b", "DefaultColor", "DefaultColor 1 2", "DetectorSearchTolerance", "DetectorSearchTolerance 1 2", "AbsorptionFileDirectory", "AbsorptionFileDirectory a b",
    "CrossSectionFilesDirectory", "CrossSectionFilesDirectory a b", "CrossSectionPath", "CrossSectionPath a b", "Material", "Material A B", "Shape BRIK", "Shape BRIK A B", "Orientation", "Orientation A B",
    "Trigger", "Trigger A B", "TriggerBasic", "TriggerBasic A B", "TriggerMap", "TriggerMap A B", "System", "System A B", "Volume", "Volume A B",
    "Strip2D", "Strip2D A B", "MDStrip2D", "Strip3D", "Strip3D A B", "MDStrip3D", "Strip3DDirectional", "Strip3DDirectional A B", "MDStrip3DDirectional", "DriftChamber", "DriftChamber A B", "MDDriftChamber",
    "AngerCamera", "AngerCamera A B", "MDAngerCamera", "Scintillator", "Scintillator A B", "Simple", "ACS", "MDACS", "Calorimeter", "Calorimeter A B", "MDCalorimeter", "Voxel3D", "Voxel3D A B", "MDVoxel3D"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The line with the wrong number of tokens is rejected", Accepts(MString(Line) + "\n")) && Passed;
  }

  // The surrounding sphere:
  MString Rest = "Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo\nVolume World\nWorld.Material Vacuum\nWorld.Shape BRIK 100 100 100\nWorld.Mother 0\n";
  Passed = EvaluateFalse("ScanSetupFile()", "SurroundingSphere, 4 values", "A surrounding sphere with too few values is rejected", ScanGeometry(Geometry, "SurroundingSphere 1 0 0 0\n" + Rest)) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "SurroundingSphere, 6 values", "A surrounding sphere with too many values is rejected", ScanGeometry(Geometry, "SurroundingSphere 1 0 0 0 1 1\n" + Rest)) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "SurroundingSphere, radius differs from distance", "A distance different from the radius is rejected", ScanGeometry(Geometry, "SurroundingSphere 1 0 0 0 2\n" + Rest)) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "SurroundingSphere twice", "A second surrounding sphere is rejected", Accepts("SurroundingSphere 2 0 0 0 2\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "SurroundingSphere, off center", "An off center surrounding sphere is accepted", ScanGeometry(Geometry, "SurroundingSphere 2.5 1.5 -2.5 3.5 2.5\n" + Rest)) && Passed;
  Passed = EvaluateVectorNear("GetStartSpherePosition()", "off center", "The position is stored", Geometry.GetStartSpherePosition(), MVector(1.5, -2.5, 3.5), 1e-12) && Passed;

  // Defect GEO-A-16: A surrounding sphere with a negative or zero radius is accepted
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "SurroundingSphere, negative radius", "A negative radius is rejected", ScanGeometry(Geometry, "SurroundingSphere -2 0 0 0 -2\n" + Rest))) && Passed;
  // Defect GEO-A-16: A surrounding sphere with a negative or zero radius is accepted
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "SurroundingSphere, zero radius", "A zero radius is rejected", ScanGeometry(Geometry, "SurroundingSphere 0 0 0 0 0\n" + Rest))) && Passed;

  // Name and version:
  Passed = EvaluateTrue("ScanSetupFile()", "version", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead())) && Passed;
  Passed = EvaluateTrue("Version", "1.0", "The version is stored", Geometry.m_Version == "1.0") && Passed;

  // The flags:
  Passed = EvaluateTrue("ScanSetupFile()", "ComplexER false", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "ComplexER false\n")) && Passed;
  Passed = EvaluateFalse("HasComplexER()", "ComplexER false", "The keyword is stored", Geometry.HasComplexER()) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "no ComplexER", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead())) && Passed;
  Passed = EvaluateTrue("HasComplexER()", "no ComplexER", "The default is back after the second scan", Geometry.HasComplexER()) && Passed;

  Passed = EvaluateTrue("ScanSetupFile()", "DetectorSearchTolerance", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "DetectorSearchTolerance 0.5\n")) && Passed;
  Passed = EvaluateNear("DetectorSearchTolerance", "0.5", "The value is stored", Geometry.m_DetectorSearchTolerance, 0.5, 1e-12) && Passed;

  Passed = EvaluateTrue("ScanSetupFile()", "VirtualizeNonDetectorVolumes", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "VirtualizeNonDetectorVolumes true\n")) && Passed;
  Passed = EvaluateTrue("VirtualizeNonDetectorVolumes", "true", "The value is stored", Geometry.m_VirtualizeNonDetectorVolumes) && Passed;

  // The sanity checks can be switched off, which allows other names:
  Passed = EvaluateFalse("ScanSetupFile()", "sanity checks on", "A bad volume name is rejected", Accepts("Volume A-B\nA-B.Material Vacuum\nA-B.Shape BRIK 1 1 1\nA-B.Mother World\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "sanity checks off", "A bad volume name is accepted without sanity checks", Accepts("DoSanityChecks false\nVolume A-B\nA-B.Material Vacuum\nA-B.Shape BRIK 1 1 1\nA-B.Mother World\n")) && Passed;

  // Show the volumes:
  Passed = EvaluateTrue("ScanSetupFile()", "ShowVolumes false", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "ShowVolumes false\n" + Marker("A") + "A.Visibility 1\n" + Marker("B") + "B.Visibility 2\n")) && Passed;
  Passed = Evaluate("ShowVolumes", "visibility 1", "A visibility of 1 is reset to 0", Geometry.GetVolume("A")->GetVisibility(), 0) && Passed;
  Passed = Evaluate("ShowVolumes", "visibility 2", "A visibility of 2 is kept", Geometry.GetVolume("B")->GetVisibility(), 2) && Passed;

  // Default color:
  Passed = EvaluateTrue("ScanSetupFile()", "DefaultColor", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "DefaultColor 7\n" + Marker("A") + "A.Color 2\n")) && Passed;
  Passed = Evaluate("DefaultColor", "color 2 and default 7", "The default color is applied to all volumes", Geometry.GetVolume("A")->GetColor(), 7) && Passed;

  // Show only the sensitive volumes:
  Passed = EvaluateTrue("ScanSetupFile()", "ShowOnlySensitiveVolumes", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "ShowOnlySensitiveVolumes true\n" + Wafer() + Marker("A", 20.0) + FullDetector("Strip2D"))) && Passed;
  // Defect GEO-A-44: ShowOnlySensitiveVolumes is evaluated before the sensitive volumes are known, thus it hides all volumes
  Passed = ExpectedToFail(Evaluate("ShowOnlySensitiveVolumes", "sensitive", "The sensitive volume is visible", Geometry.GetVolume("Wafer")->GetVisibility(), 1)) && Passed;
  Passed = Evaluate("ShowOnlySensitiveVolumes", "not sensitive", "The other volume is not visible", Geometry.GetVolume("A")->GetVisibility(), 0) && Passed;

  // The surrounding sphere is only shown in geomega:
  Passed = EvaluateTrue("ScanSetupFile()", "ShowSurroundingSphere, library", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "ShowSurroundingSphere true\n")) && Passed;
  Passed = EvaluateTrue("ShowSurroundingSphere", "library", "No volume is added in library mode", Geometry.GetVolume("SurroundingSphereVolume") == nullptr) && Passed;
  MDGeometry Launched;
  Launched.LaunchedByGeomega();
  Passed = EvaluateTrue("ScanSetupFile()", "ShowSurroundingSphere, geomega", "The file is accepted", ScanGeometry(Launched, GetGeometryHead() + "ShowSurroundingSphere true\n")) && Passed;
  Passed = EvaluateTrue("LaunchedByGeomega()", "ShowSurroundingSphere", "The sphere is added as a volume in geomega", Launched.GetVolume("SurroundingSphereVolume") != nullptr) && Passed;
  Passed = EvaluateTrue("LaunchedByGeomega()", "ShowSurroundingSphere", "The sphere material is added in geomega", Launched.GetMaterial("SurroundingSphereVolumeMaterial") != nullptr) && Passed;

  // The cross section path: absolute, relative to the setup file, and its three names
  Passed = EvaluateTrue("ScanSetupFile()", "no CrossSectionPath", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead())) && Passed;
  Passed = EvaluateTrue("CrossSectionPath", "default", "The default is cross-sections next to the setup file", Geometry.m_CrossSectionFileDirectory == GetTemporaryDirectoryName() + "/cross-sections") && Passed;
  for (const MString Keyword: {"AbsorptionFileDirectory", "CrossSectionFilesDirectory", "CrossSectionPath"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Keyword, "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + Keyword + " /A/B\n")) && Passed;
    Passed = EvaluateTrue(Keyword, "absolute", "The absolute path is stored", Geometry.m_CrossSectionFileDirectory == "/A/B") && Passed;
  }
  Passed = EvaluateTrue("ScanSetupFile()", "relative CrossSectionPath", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "CrossSectionPath Other\n")) && Passed;
  Passed = EvaluateTrue("CrossSectionPath", "relative", "A relative path is relative to the setup file", Geometry.m_CrossSectionFileDirectory == GetTemporaryDirectoryName() + "/Other") && Passed;

  // The outdated keyword is ignored with a message:
  Passed = EvaluateTrue("ScanSetupFile()", "IgnoreShortNames", "The outdated keyword is ignored", Accepts("IgnoreShortNames false\n")) && Passed;

  // Defect GEO-A-17: A mistyped first order keyword or object name is silently ignored, which leaves the geometry different from the intent
  for (const MString Line: {"Volum Typo", "Materal Typo", "Nam Typo", "Bo.Position 1 2 3", "SurroundingSpheres 1 0 0 0 1", "Done", "Else", "EndIf", "DoneVector"}) {
    Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", Line, "The unknown line is rejected", Accepts(Marker("Box") + Line + "\n"))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the names and their duplicates
bool UTDGeometry::TestNames()
{
  bool Passed = true;

  // Invalid characters:
  for (const MString Keyword: {"Volume", "Material", "Orientation", "Trigger", "TriggerMap", "System", "Strip2D", "Strip3D", "Strip3DDirectional", "DriftChamber", "AngerCamera", "Scintillator", "Calorimeter", "Voxel3D", "Vector"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "A name with a minus sign is rejected", Accepts(Keyword + " Bad-Name\n")) && Passed;
  }
  Passed = EvaluateFalse("ScanSetupFile()", "Shape", "A shape name with a minus sign is rejected", Accepts("Shape BRIK Bad-Name\n")) && Passed;

  // Everything is one name space, and it is case insensitive:
  vector<MString> Definitions = {"Volume Same\nSame.Material Vacuum\nSame.Shape BRIK 1 1 1\nSame.Mother World\n", "Material Same\nSame.Density 1\nSame.Component H 1\n", FullDetector("Scintillator", "Same", "Wafer"), "Trigger Same\nSame.Veto false\nSame.TriggerByChannel true\n", "Vector Same\n"};
  for (unsigned int i = 0; i < Definitions.size(); ++i) {
    for (unsigned int j = 0; j < Definitions.size(); ++j) {
      // Lines in upper case for the second definition make the check case insensitive:
      MString Second = Definitions[j];
      Second.ReplaceAll("Same", "SAME");
      MString Input = "Definition "; Input += i; Input += " and definition "; Input += j;
      if (i == 3 || j == 3) {
        // A second trigger is rejected for another reason
        Passed = EvaluateFalse("ScanSetupFile()", Input, "The second object with the same name is rejected", Accepts(Wafer() + Definitions[i] + Second)) && Passed;
      } else {
        // Defect GEO-A-69: The check for existing names is case sensitive, while its comment says that names are case insensitive
        Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", Input, "The second object with the same name is rejected", Accepts(Wafer() + Definitions[i] + Second))) && Passed;
      }
    }
  }

  // Defect GEO-A-18: Shapes and orientations are not part of the duplicate name check
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "two shapes of the same name", "The second shape of the same name is rejected", Accepts("Shape BRIK S\nS.Parameters 1 1 1\nShape BRIK S\nS.Parameters 2 2 2\n"))) && Passed;
  // Defect GEO-A-18: Shapes and orientations are not part of the duplicate name check
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "two orientations of the same name", "The second orientation of the same name is rejected", Accepts("Orientation O\nO.Position 1 1 1\nOrientation O\nO.Position 2 2 2\n"))) && Passed;
  // The lines of a volume with the name of a shape go to the shape, which rejects them:
  Passed = EvaluateFalse("ScanSetupFile()", "shape named like a volume", "A shape with the name of a volume is rejected", Accepts(Marker("Same") + "Shape BRIK Same\nSame.Parameters 1 1 1\n")) && Passed;

  // Defect GEO-A-19: The name of a volume made by Copy is not checked for valid characters
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "Copy with a bad name", "A copy with a minus sign in the name is rejected", Accepts(Marker("A") + "A.Copy Bad-Name\nBad-Name.Position 3 0 0\nBad-Name.Mother World\n"))) && Passed;
  // The lines of a copy with the name of a material go to the material, which rejects them:
  Passed = EvaluateFalse("ScanSetupFile()", "volume Copy named like a material", "A copy with the name of a material is rejected", Accepts(Marker("A") + "A.Copy Aluminium\nAluminium.Position 3 0 0\nAluminium.Mother World\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the material keywords
bool UTDGeometry::TestMaterials()
{
  bool Passed = true;
  MDGeometry Geometry;

  // The material and its properties:
  MString M = "Material Mine\nMine.Density 2.5\nMine.RadiationLength 8.5\nMine.ComponentByAtoms Si 1\nMine.ComponentByAtoms O 2\n";
  Passed = EvaluateTrue("ScanSetupFile()", "material", "A material is accepted", ScanGeometry(Geometry, GetGeometryHead() + M)) && Passed;
  MDMaterial* Material = Geometry.GetMaterial("Mine");
  if (EvaluateTrue("GetMaterial()", "material", "The material exists", Material != nullptr) == true) {
    Passed = EvaluateNear("Density", "2.5", "The density is stored", Material->GetDensity(), 2.5, 1e-12) && Passed;
    Passed = EvaluateNear("RadiationLength", "8.5", "The radiation length is stored", Material->GetRadiationLength(), 8.5, 1e-12) && Passed;
    Passed = Evaluate("ComponentByAtoms", "Si and O", "Both components are stored", Material->GetNComponents(), 2U) && Passed;
    Passed = Evaluate("MDMaterialComponent::GetAtomicNumber()", "O", "The element of the second component is oxygen", Material->GetComponentAt(1)->GetAtomicNumber(), 8) && Passed;
  }

  // All the forms of a component: element name or atomic weight and number
  for (const MString Line: {"Mine.Component Si 1", "Mine.ComponentByAtoms Si 1", "Mine.ComponentByMass Si 1.0", "Mine.Component 28.0 14 1", "Mine.ComponentByAtoms 28.0 14 1", "Mine.ComponentByMass 28.0 14 1.0"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Line, "The form of the component is accepted", Accepts(MString("Material Mine\nMine.Density 2.5\n") + Line + "\n")) && Passed;
  }

  // Rejections:
  for (const MString Line: {"Mine.Density", "Mine.Density 1 2", "Mine.RadiationLength", "Mine.RadiationLength 1 2", "Mine.Component", "Mine.Component Si", "Mine.Component 1 2 3 4", "Mine.ComponentByAtoms Si", "Mine.ComponentByMass", "Mine.ComponentByMass Si", "Mine.ComponentByMass 1 2 3 4", "Mine.ComponentByAtoms Xx 1", "Mine.ComponentByMass Xx 1", "Mine.Unknown 1"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad material line is rejected", Accepts(MString("Material Mine\nMine.Density 2.5\nMine.Component Si 1\n") + Line + "\n")) && Passed;
  }

  // Without density or components the material is invalid:
  Passed = EvaluateFalse("ScanSetupFile()", "no density", "A material without density is rejected", Accepts("Material Mine\nMine.Component Si 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "no component", "A material without component is rejected", Accepts("Material Mine\nMine.Density 1.0\n")) && Passed;

  // The redundant keyword is accepted:
  Passed = EvaluateTrue("ScanSetupFile()", "Sensitivity", "The redundant keyword is accepted", Accepts("Material Mine\nMine.Density 2.5\nMine.Component Si 1\nMine.Sensitivity 1\n")) && Passed;

  // Copies of materials:
  Passed = EvaluateTrue("ScanSetupFile()", "material copy", "A material copy is accepted", ScanGeometry(Geometry, GetGeometryHead() + M + "Mine.Copy MineCopy\n")) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "material copy", "The copy exists", Geometry.GetMaterial("MineCopy") != nullptr) && Passed;
  Passed = EvaluateNear("Copy", "material", "The copy has the density of the original", Geometry.GetMaterial("MineCopy")->GetDensity(), 2.5, 1e-12) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "material copy of existing name", "A copy with the name of a material is rejected", Accepts(M + "Mine.Copy Aluminium\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the shape keywords
bool UTDGeometry::TestShapes()
{
  bool Passed = true;
  MDGeometry Geometry;

  // All inline shapes, with the volume of each by hand
  struct ShapeCase {
    MString Parameters;
    MString Type;
    double Volume;
  };
  const double Pi = c_Pi;
  vector<ShapeCase> Cases = {
    {"BRIK 1 2 3", "BRIK", 8*1.0*2*3},
    {"BOX 1 2 3", "BRIK", 8*1.0*2*3},
    {"brik 1 2 3", "BRIK", 8*1.0*2*3},
    {"SPHE 0 2 0 180 0 360", "SPHE", 4.0/3*Pi*8},
    {"SPHERE 0 2 0 180 0 360", "SPHE", 4.0/3*Pi*8},
    {"TUBS 0 2 3 0 360", "TUBS", Pi*4*6},
    {"TUBE 0 2 3 0 360", "TUBS", Pi*4*6},
    {"CONE 3 0 2 0 2", "CONE", Pi*4*6},
    {"CONS 3 0 2 0 2 0 360", "CONS", Pi*4*6},
    {"TRD1 2 2 1 3", "TRD1", 8*2.0*1*3},
    {"TRD2 2 2 1 1 3", "TRD2", 8*2.0*1*3},
    {"TRAP 3 0 0 1 2 2 0 1 2 2 0", "TRAP", 8*2.0*1*3},
    {"GTRA 3 0 0 0 1 2 2 0 1 2 2 0", "GTRA", 8*2.0*1*3},
    {"PCON 0 360 2 -3 0 2 3 0 2", "PCON", Pi*4*6},
  };
  for (const ShapeCase& Case: Cases) {
    MString Text = GetGeometryHead() + "Volume A\nA.Material Aluminium\nA.Shape " + Case.Parameters + "\nA.Mother World\n";
    bool Result = ScanGeometry(Geometry, Text);
    Passed = EvaluateTrue("ScanSetupFile()", Case.Parameters, "The inline shape is accepted", Result) && Passed;
    if (Result == true) {
      MString Type = Geometry.GetVolume("A")->GetShape()->GetType();
      Type.ToUpperInPlace();
      Passed = EvaluateTrue("MDShape::GetType()", Case.Parameters, "The type of the shape is " + Case.Type, Type == Case.Type) && Passed;
      Passed = EvaluateNear("MDShape::GetVolume()", Case.Parameters, "The volume of the shape is the one by hand", Geometry.GetVolume("A")->GetShape()->GetVolume(), Case.Volume, 1e-9) && Passed;
      Passed = EvaluateTrue("GetShape()", Case.Parameters, "The shape is in the list under the name of the volume", Geometry.GetShape("A_Shape") == Geometry.GetVolume("A")->GetShape()) && Passed;
    }
  }
  // PGON: the volume of a polygon of 4 sides with an inscribed radius of 2 is the one of a box of half lengths 2, 2, 3
  Passed = EvaluateTrue("ScanSetupFile()", "PGON", "The inline PGON is accepted", Accepts("Volume A\nA.Material Aluminium\nA.Shape PGON 45 360 4 2 -3 0 2 3 0 2\nA.Mother World\n")) && Passed;

  // Rejected inline shapes:
  for (const MString Line: {"A.Shape", "A.Shape UNKNOWN 1 2 3", "A.Shape BRIK 1 2", "A.Shape BRIK 1 2 3 4", "A.Shape BRIK -1 2 3", "A.Shape SPHE 2", "A.Shape TUBS 1", "A.Shape CONE 1", "A.Shape CONS 1", "A.Shape TRAP 1", "A.Shape GTRA 1", "A.Shape TRD1 1", "A.Shape TRD2 1", "A.Shape PCON 1", "A.Shape PGON 1",
    "A.Shape Union A B", "A.Shape Subtraction A B", "A.Shape Intersection A B", "A.Shape Union X Y O", "A.Shape Subtraction X Y O", "A.Shape Intersection X Y O"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad shape is rejected", Accepts(MString("Volume A\nA.Material Aluminium\nA.Mother World\n") + Line + "\n")) && Passed;
  }

  // A volume without a shape:
  Passed = EvaluateFalse("ScanSetupFile()", "no shape", "A volume without a shape is rejected", Accepts("Volume A\nA.Material Aluminium\nA.Mother World\n")) && Passed;

  // Named shapes:
  MString Named = "Shape BRIK S1\nS1.Parameters 1 2 3\nVolume A\nA.Material Aluminium\nA.Shape S1\nA.Mother World\n";
  Passed = EvaluateTrue("ScanSetupFile()", "named shape", "A named shape is accepted", ScanGeometry(Geometry, GetGeometryHead() + Named)) && Passed;
  Passed = Evaluate("GetNShapes()", "named shape", "The world shape and the named shape exist", Geometry.GetNShapes(), 2U) && Passed;
  Passed = EvaluateTrue("GetShape()", "named shape", "The volume uses the named shape", Geometry.GetVolume("A")->GetShape() == Geometry.GetShape("S1")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "unknown shape", "An unknown shape name is rejected", Accepts("Volume A\nA.Material Aluminium\nA.Shape S1\nA.Mother World\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "unknown type", "An unknown shape type is rejected", Accepts("Shape FOO S1\n")) && Passed;
  // Defect GEO-A-45: A named shape without parameters is accepted as a volume shape of size 0
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "named shape without parameters", "A named shape without parameters is rejected", Accepts("Shape BRIK S1\nVolume A\nA.Material Aluminium\nA.Shape S1\nA.Mother World\n"))) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "unknown option of a shape", "A shape with an unknown option is rejected", Accepts("Shape BRIK S1\nS1.Unknown 1 2 3\n")) && Passed;
  for (const MString Type: {"box", "brik", "sphe", "sphere", "cone", "cons", "pcon", "pgon", "tubs", "tube", "trap", "trd1", "trd2", "gtra", "subtraction", "union", "intersection"}) {
    MDGeometry Direct;
    Passed = EvaluateTrue("AddShape(Type, Name)", Type, "The shape type is known", Direct.AddShape(Type, "S")) && Passed;
    Passed = Evaluate("GetNShapes()", Type, "The shape has been added", Direct.GetNShapes(), 1U) && Passed;
  }
  MDGeometry Direct;
  DisableDefaultStreams();
  bool Unknown = Direct.AddShape("Foo", "S");
  EnableDefaultStreams();
  Passed = EvaluateFalse("AddShape(Type, Name)", "unknown", "An unknown type is rejected", Unknown) && Passed;

  // Boolean shapes in both forms:
  MString Parts = "Shape BRIK S1\nS1.Parameters 2 2 2\nShape BRIK S2\nS2.Parameters 1 1 1\nOrientation O\nO.Position 0 0 0\n";
  for (const MString Type: {"Union", "Subtraction", "Intersection"}) {
    MString Inline = Parts + "Volume A\nA.Material Aluminium\nA.Shape " + Type + " S1 S2 O\nA.Mother World\n";
    Passed = EvaluateTrue("ScanSetupFile()", Type, "The inline boolean shape is accepted", ScanGeometry(Geometry, GetGeometryHead() + Inline)) && Passed;
    Passed = EvaluateTrue("MDShape::GetType()", Type, "The boolean shape has the right type", Geometry.GetVolume("A")->GetShape()->GetType() == Type) && Passed;

    MString Separate = Parts + "Shape " + Type + " B\nB.Parameters S1 S2 O\nVolume A\nA.Material Aluminium\nA.Shape B\nA.Mother World\n";
    Passed = EvaluateTrue("ScanSetupFile()", Type, "The named boolean shape is accepted", ScanGeometry(Geometry, GetGeometryHead() + Separate)) && Passed;
    Passed = EvaluateTrue("MDShape::GetType()", Type, "The named boolean shape has the right type", Geometry.GetShape("B")->GetType() == Type) && Passed;

    // Without orientation one is created:
    MString NoOrientation = Parts + "Shape " + Type + " B\nB.Parameters S1 S2\nVolume A\nA.Material Aluminium\nA.Shape B\nA.Mother World\n";
    Passed = EvaluateTrue("ScanSetupFile()", Type, "A named boolean shape without orientation is accepted", ScanGeometry(Geometry, GetGeometryHead() + NoOrientation)) && Passed;
    Passed = EvaluateTrue("GetOrientation()", Type, "The missing orientation is created", Geometry.GetOrientation("B_Orientation") != nullptr) && Passed;

    // Bad parameters:
    Passed = EvaluateFalse("ScanSetupFile()", Type, "Too few parameters are rejected", Accepts(Parts + "Shape " + Type + " B\nB.Parameters S1\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Type, "Too many parameters are rejected", Accepts(Parts + "Shape " + Type + " B\nB.Parameters S1 S2 O O\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Type, "An unknown first shape is rejected", Accepts(Parts + "Shape " + Type + " B\nB.Parameters Sx S2 O\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Type, "An unknown second shape is rejected", Accepts(Parts + "Shape " + Type + " B\nB.Parameters S1 Sx O\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Type, "An unknown orientation is rejected", Accepts(Parts + "Shape " + Type + " B\nB.Parameters S1 S2 Ox\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Type, "An inline boolean with an unknown first shape is rejected", Accepts(Parts + "Volume A\nA.Material Aluminium\nA.Shape " + Type + " Sx S2 O\nA.Mother World\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Type, "An inline boolean with an unknown second shape is rejected", Accepts(Parts + "Volume A\nA.Material Aluminium\nA.Shape " + Type + " S1 Sx O\nA.Mother World\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Type, "An inline boolean with an unknown orientation is rejected", Accepts(Parts + "Volume A\nA.Material Aluminium\nA.Shape " + Type + " S1 S2 Ox\nA.Mother World\n")) && Passed;

    // Defect GEO-A-20: An option other than Parameters of a boolean shape is ignored without a message, while it is an error for the other shapes
    Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", Type, "A boolean shape with an unknown option is rejected", Accepts(Parts + "Shape " + Type + " B\nB.Parameters S1 S2 O\nB.Unknown 1\nVolume A\nA.Material Aluminium\nA.Shape B\nA.Mother World\n"))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the orientation keywords
bool UTDGeometry::TestOrientations()
{
  bool Passed = true;
  MDGeometry Geometry;

  MString Text = "Orientation O\nO.Position 1.5 2.5 3.5\nO.Rotation 0 0 90\n" + Marker("A") + "A.Orientation O\n";
  Passed = EvaluateTrue("ScanSetupFile()", "orientation", "An orientation is accepted", ScanGeometry(Geometry, GetGeometryHead() + Text)) && Passed;
  Passed = Evaluate("GetNOrientations()", "orientation", "One orientation is stored", Geometry.GetNOrientations(), 1U) && Passed;
  Passed = CheckPosition(Geometry, "A", MVector(1.5, 2.5, 3.5), "orientation") && Passed;
  // A rotation of +90 deg around z takes the x axis to the y axis:
  Passed = EvaluateVectorNear("Orientation", "rotation", "The rotation of the orientation is given to the volume", Geometry.GetVolume("A")->GetRotationMatrix()*MVector(1, 0, 0), Geometry.GetOrientation("O")->GetRotationMatrix()*MVector(1, 0, 0), 1e-12) && Passed;

  Passed = EvaluateFalse("ScanSetupFile()", "unknown orientation", "An unknown orientation is rejected", Accepts(Marker("A") + "A.Orientation O\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "orientation token count", "An orientation line without a name is rejected", Accepts(Marker("A") + "A.Orientation\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "unknown orientation option", "An unknown option of an orientation is rejected", Accepts("Orientation O\nO.Unknown 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "orientation position, too few", "An orientation position with too few values is rejected", Accepts("Orientation O\nO.Position 1 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "orientation rotation, too few", "An orientation rotation with too few values is rejected", Accepts("Orientation O\nO.Rotation 1 2\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume keywords
bool UTDGeometry::TestVolumes()
{
  bool Passed = true;
  MDGeometry Geometry;

  // All display and tree properties:
  MString Text = Marker("A") + "A.Color 5\nA.LineStyle 2\nA.LineWidth 3\nA.Visibility 2\nA.Many true\nA.Position 1.5 2.5 3.5\nA.Rotation 0 0 90\nA.Absorptions File\n";
  Passed = EvaluateTrue("ScanSetupFile()", "volume options", "All volume options are accepted", ScanGeometry(Geometry, GetGeometryHead() + Text)) && Passed;
  MDVolume* V = Geometry.GetVolume("A");
  if (EvaluateTrue("GetVolume()", "volume options", "The volume exists", V != nullptr) == true) {
    Passed = Evaluate("Color", "5", "The color is stored", V->GetColor(), 5) && Passed;
    Passed = Evaluate("LineStyle", "2", "The line style is stored", V->GetLineStyle(), 2) && Passed;
    Passed = Evaluate("LineWidth", "3", "The line width is stored", V->GetLineWidth(), 3) && Passed;
    Passed = Evaluate("Visibility", "2", "The visibility is stored", V->GetVisibility(), 2) && Passed;
    Passed = EvaluateTrue("Many", "true", "The many flag is stored", V->IsMany()) && Passed;
    Passed = EvaluateVectorNear("Position", "1.5 2.5 3.5", "The position is stored", V->GetPosition(), MVector(1.5, 2.5, 3.5), 1e-12) && Passed;
    Passed = EvaluateTrue("Rotation", "0 0 90", "The rotation is stored", V->IsRotated()) && Passed;
    Passed = EvaluateTrue("Material", "Aluminium", "The material is stored", V->GetMaterial() == Geometry.GetMaterial("Aluminium")) && Passed;
    Passed = EvaluateTrue("Mother", "World", "The mother is stored", V->GetMother() == Geometry.GetVolume("World")) && Passed;
  }
  Passed = EvaluateTrue("World", "Mother 0", "The world is the first volume in the list", Geometry.GetVolumeAt(0) == Geometry.GetWorldVolume()) && Passed;
  Passed = EvaluateTrue("IsWorldVolume()", "Mother 0", "The world knows that it is the world", Geometry.GetWorldVolume()->IsWorldVolume()) && Passed;

  // The rotation with three angles: 90 deg around z gives the matrix x -> -y in the coordinate system of the volume, by hand
  Passed = EvaluateTrue("ScanSetupFile()", "rotation", "The rotation is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("A") + "A.Rotate 0 0 90\n")) && Passed;
  Passed = EvaluateVectorNear("Rotate", "90 deg around z", "A point on the x axis in the volume is on the y axis in the mother", Geometry.GetVolume("A")->GetInvRotationMatrix()*MVector(1, 0, 0), MVector(0, 1, 0), 1e-12) && Passed;

  // The rotation with six angles, the same rotation in the Geant3 notation: x axis to (90, 90), y axis to (90, 180), z axis to (0, 0)
  Passed = EvaluateTrue("ScanSetupFile()", "rotation with six angles", "The rotation is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("A") + "A.Rotation 90 90 90 180 0 0\n")) && Passed;
  Passed = EvaluateVectorNear("Rotation", "six angles", "A point on the x axis in the volume is on the y axis in the mother", Geometry.GetVolume("A")->GetInvRotationMatrix()*MVector(1, 0, 0), MVector(0, 1, 0), 1e-12) && Passed;

  // Virtual:
  Passed = EvaluateTrue("ScanSetupFile()", "Virtual", "A virtual volume is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("A") + "A.Virtual true\n")) && Passed;
  Passed = EvaluateTrue("Virtual", "true", "The virtual flag is stored", Geometry.GetVolume("A")->IsVirtual()) && Passed;

  // Boundaries of the visibility:
  for (int Visibility = -2; Visibility <= 4; ++Visibility) {
    MString Line = Marker("A") + "A.Visibility "; Line += Visibility; Line += "\n";
    MString Input = "Visibility "; Input += Visibility;
    if (Visibility < -1 || Visibility > 3) {
      Passed = EvaluateFalse("ScanSetupFile()", Input, "The visibility out of range is rejected", Accepts(Line)) && Passed;
    } else {
      Passed = EvaluateTrue("ScanSetupFile()", Input, "The visibility in range is accepted", Accepts(Line)) && Passed;
    }
  }
  // Boundaries of color, line style, and line width:
  for (const MString Keyword: {"Color", "LineStyle", "LineWidth"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Keyword, "The value 0 is accepted", Accepts(Marker("A") + "A." + Keyword + " 0\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "The value -1 is rejected", Accepts(Marker("A") + "A." + Keyword + " -1\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "No value is rejected", Accepts(Marker("A") + "A." + Keyword + "\n")) && Passed;
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "Two values are rejected", Accepts(Marker("A") + "A." + Keyword + " 1 2\n")) && Passed;
  }
  // Boundaries of the Geant3 angles:
  for (const MString Line: {"A.Rotation -1 0 90 0 90 90", "A.Rotation 181 0 90 0 90 90", "A.Rotation 90 0 -1 0 90 90", "A.Rotation 90 0 181 0 90 90", "A.Rotation 90 0 90 0 -1 90", "A.Rotation 90 0 90 0 181 90"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The theta angle out of range is rejected", Accepts(Marker("A") + Line + "\n")) && Passed;
  }

  // Further rejections:
  for (const MString Line: {"A.Material Unknown", "A.Material", "A.Mother", "A.Mother X Y", "A.Mother Unknown", "A.Mother A", "A.Virtual", "A.Virtual 1 2", "A.Many", "A.Many 1 2", "A.Position 1 2", "A.Position 1 2 3 4", "A.Rotation 1 2", "A.Rotation 1 2 3 4", "A.Unknown 1", "A.Mother 0"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad volume line is rejected", Accepts(Marker("A") + Line + "\n")) && Passed;
  }

  // Tree errors:
  Passed = EvaluateFalse("ScanSetupFile()", "no material", "A volume without a material is rejected", Accepts("Volume A\nA.Shape BRIK 1 1 1\nA.Mother World\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "cyclic mothers", "Cyclic mothers are rejected", Accepts(Marker("A") + Marker("B") + "A.Mother B\nB.Mother A\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "copy as mother", "A copy cannot be a mother", Accepts(Marker("A") + "A.Copy C\nC.Mother World\n" + Marker("B") + "B.Mother C\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "copy of a copy", "A copy of a copy is rejected", Accepts(Marker("A") + "A.Copy C\nC.Mother World\nC.Copy D\nD.Mother World\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "copy name exists", "A copy with an existing volume name is rejected", Accepts(Marker("A") + Marker("B", 3) + "A.Copy B\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "copy of the world", "A copy of the world volume is rejected", Accepts("World.Copy W2\nW2.Mother World\n")) && Passed;

  // Copies:
  Text = Marker("A") + "A.Copy B\nB.Position 3 0 0\nB.Mother World\nA.Copy C\nC.Position 6 0 0\nC.Mother World\nC.Color 5\n";
  Passed = EvaluateTrue("ScanSetupFile()", "copies", "Copies are accepted", ScanGeometry(Geometry, GetGeometryHead() + Text)) && Passed;
  Passed = Evaluate("GetNVolumes()", "copies", "The copies are volumes", Geometry.GetNVolumes(), 4U) && Passed;
  Passed = Evaluate("MDVolume::GetNClones()", "copies", "The template has two clones", Geometry.GetVolume("A")->GetNClones(), 2U) && Passed;
  Passed = EvaluateTrue("MDVolume::IsClone()", "copies", "The copy is a clone", Geometry.GetVolume("B")->IsClone()) && Passed;
  Passed = EvaluateTrue("MDVolume::GetMaterial()", "copies", "The copy has the material of the template", Geometry.GetVolume("B")->GetMaterial() == Geometry.GetMaterial("Aluminium")) && Passed;
  Passed = EvaluateTrue("MDVolume::GetShape()", "copies", "The copy has the shape of the template", Geometry.GetVolume("B")->GetShape() == Geometry.GetVolume("A")->GetShape()) && Passed;
  Passed = Evaluate("MDVolume::GetColor()", "copies", "The copy keeps its own color", Geometry.GetVolume("C")->GetColor(), 5) && Passed;

  // Defect GEO-A-21: A non-numeric value of a numeric keyword is silently read as 0
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "A.Position 1 2 abc", "A position which is not a number is rejected", Accepts(Marker("A") + "A.Position 1 2 abc\n"))) && Passed;
  // Defect GEO-A-21: A non-numeric value of a numeric keyword is silently read as 0
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "A.Color red", "A color which is not a number is rejected", Accepts(Marker("A") + "A.Color red\n"))) && Passed;

  // Defect GEO-A-22: Superfluous values after the name of the material of a volume are not rejected
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "A.Material Aluminium Vacuum", "A material line with two names is rejected", Accepts(Marker("A") + "A.Material Aluminium Vacuum\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword Scale
bool UTDGeometry::TestScale()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Scale a box with a daughter by 2: half lengths and the position of the daughter double
  MString Text = "Volume M\nM.Material Vacuum\nM.Shape BRIK 2 3 4\nM.Mother World\n";
  Text += "Volume D\nD.Material Vacuum\nD.Shape BRIK 0.5 0.5 0.5\nD.Position 1 1 1\nD.Mother M\n";
  Passed = EvaluateTrue("ScanSetupFile()", "Scale 2", "Scaling is accepted", ScanGeometry(Geometry, GetGeometryHead() + Text + "M.Scale 2\n")) && Passed;
  Passed = EvaluateVectorNear("Scale", "2, mother", "All half lengths of the mother double", Geometry.GetVolume("M")->GetSize(), MVector(4, 6, 8), 1e-12) && Passed;
  Passed = EvaluateVectorNear("Scale", "2, daughter", "All half lengths of the daughter double", Geometry.GetVolume("D")->GetSize(), MVector(1, 1, 1), 1e-12) && Passed;
  Passed = EvaluateVectorNear("Scale", "2, daughter position", "The daughter position doubles", Geometry.GetVolume("D")->GetPosition(), MVector(2, 2, 2), 1e-12) && Passed;
  Passed = EvaluateVectorNear("Scale", "2, own position", "The position of the scaled volume stays", Geometry.GetVolume("M")->GetPosition(), MVector(0, 0, 0), 1e-12) && Passed;

  // Scale along one axis:
  Passed = EvaluateTrue("ScanSetupFile()", "Scale 2 X", "Scaling along an axis is accepted", ScanGeometry(Geometry, GetGeometryHead() + Text + "M.Scale 2 X\n")) && Passed;
  Passed = EvaluateVectorNear("Scale", "2 X, mother", "Only the half length in x doubles", Geometry.GetVolume("M")->GetSize(), MVector(4, 3, 4), 1e-12) && Passed;
  Passed = EvaluateVectorNear("Scale", "2 X, daughter", "Only the half length in x of the daughter doubles", Geometry.GetVolume("D")->GetSize(), MVector(1, 0.5, 0.5), 1e-12) && Passed;
  // Defect GEO-A-23: The position of a daughter is scaled on all axes also if only some axes are scaled
  Passed = ExpectedToFail(EvaluateVectorNear("Scale", "2 X, daughter position", "Only the x position of the daughter doubles", Geometry.GetVolume("D")->GetPosition(), MVector(2, 1, 1), 1e-12)) && Passed;

  // Boundaries and errors:
  Passed = EvaluateFalse("ScanSetupFile()", "Scale 0", "A zero scale is rejected", Accepts(Text + "M.Scale 0\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Scale -1", "A negative scale is rejected", Accepts(Text + "M.Scale -1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Scale", "A scale without a value is rejected", Accepts(Text + "M.Scale\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Scale 1 X Y", "A scale with too many values is rejected", Accepts(Text + "M.Scale 1 X Y\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "Scale 1", "A scale of one is accepted", ScanGeometry(Geometry, GetGeometryHead() + Text + "M.Scale 1\n")) && Passed;
  Passed = EvaluateVectorNear("Scale", "1", "A scale of one changes nothing", Geometry.GetVolume("M")->GetSize(), MVector(2, 3, 4), 1e-12) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Scale of a copy", "Scaling a copy is rejected", Accepts(Text + "D.Copy E\nE.Mother M\nE.Position 1 -1 1\nE.Scale 2\n")) && Passed;

  // Scale a mother with only copies of one template inside: the shape of the template is scaled once
  MString Copies = "Volume M\nM.Material Vacuum\nM.Shape BRIK 4 4 4\nM.Mother World\nVolume T\nT.Material Vacuum\nT.Shape BRIK 0.5 0.5 0.5\nT.Copy C1\nC1.Position 1 0 0\nC1.Mother M\nT.Copy C2\nC2.Position -1 0 0\nC2.Mother M\n";
  Passed = EvaluateTrue("ScanSetupFile()", "Scale with copies", "Scaling copies is accepted", ScanGeometry(Geometry, GetGeometryHead() + Copies + "M.Scale 2\n")) && Passed;
  Passed = EvaluateVectorNear("Scale", "copies", "The shape shared by the copies is scaled once", Geometry.GetVolume("C1")->GetSize(), MVector(1, 1, 1), 1e-12) && Passed;
  Passed = EvaluateVectorNear("Scale", "copies, position", "The position of the copy is scaled", Geometry.GetVolume("C2")->GetPosition(), MVector(-2, 0, 0), 1e-12) && Passed;

  // The shape of a template placed together with its copy is scaled once:
  MString Both = "Volume M\nM.Material Vacuum\nM.Shape BRIK 4 4 4\nM.Mother World\nVolume T\nT.Material Vacuum\nT.Shape BRIK 0.5 0.5 0.5\nT.Position 1 0 0\nT.Mother M\nT.Copy C1\nC1.Position -1 0 0\nC1.Mother M\n";
  Passed = EvaluateTrue("ScanSetupFile()", "Scale with template and copy", "Scaling is accepted", ScanGeometry(Geometry, GetGeometryHead() + Both + "M.Scale 2\n")) && Passed;
  Passed = EvaluateVectorNear("Scale", "template and copy placed", "The half length of 0.5 of the shared shape doubles to 1", Geometry.GetVolume("T")->GetSize(), MVector(1, 1, 1), 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test virtual volumes
bool UTDGeometry::TestVirtual()
{
  bool Passed = true;
  MDGeometry Geometry;

  // A virtual mother at (0, 0, 10) with a daughter at (1, 0, 0) inside: the daughter moves into the world at (1, 0, 10)
  MString Text = "Volume V\nV.Material Vacuum\nV.Shape BRIK 2 2 2\nV.Position 0 0 10\nV.Virtual true\nV.Mother World\n";
  Text += "Volume D\nD.Material Aluminium\nD.Shape BRIK 0.5 0.5 0.5\nD.Position 1 0 0\nD.Mother V\n";
  Passed = EvaluateTrue("ScanSetupFile()", "virtual mother", "The geometry is accepted", ScanGeometry(Geometry, GetGeometryHead() + Text)) && Passed;
  Passed = CheckPosition(Geometry, "D", MVector(1, 0, 10), "virtual mother") && Passed;
  Passed = EvaluateTrue("MDVolume::GetMother()", "virtual mother", "The daughter is now in the world", Geometry.GetVolume("D")->GetMother() == Geometry.GetWorldVolume()) && Passed;
  MDVolumeSequence Sequence = Geometry.GetVolumeSequence(MVector(1, 0, 10));
  Passed = EvaluateTrue("GetVolumeSequence()", "virtual mother", "The daughter is found at the new position", Sequence.GetDeepestVolume() == Geometry.GetVolume("D")) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(0, 0, 10));
  Passed = EvaluateTrue("GetVolumeSequence()", "virtual mother", "The virtual volume is not in the tree", Sequence.GetDeepestVolume() == Geometry.GetWorldVolume()) && Passed;

  // A rotated virtual mother: rotated by 90 deg around z, the daughter at (1, 0, 0) moves to (0, 1, 0)
  MString Rotated = "Volume V\nV.Material Vacuum\nV.Shape BRIK 2 2 2\nV.Rotation 0 0 90\nV.Virtual true\nV.Mother World\n";
  Rotated += "Volume D\nD.Material Aluminium\nD.Shape BRIK 0.5 0.5 0.5\nD.Position 1 0 0\nD.Mother V\n";
  Passed = EvaluateTrue("ScanSetupFile()", "rotated virtual mother", "The geometry is accepted", ScanGeometry(Geometry, GetGeometryHead() + Rotated)) && Passed;
  Passed = CheckPosition(Geometry, "D", MVector(0, 1, 0), "rotated virtual mother") && Passed;

  // A virtual template which is not placed, with two copies: each copy gets its own version of the daughter
  MString Copies = "Volume V\nV.Material Vacuum\nV.Shape BRIK 2 2 2\nV.Virtual true\n";
  Copies += "Volume D\nD.Material Aluminium\nD.Shape BRIK 0.5 0.5 0.5\nD.Position 1 0 0\nD.Mother V\n";
  Copies += "V.Copy C1\nC1.Position 0 0 10\nC1.Mother World\nV.Copy C2\nC2.Position 0 0 -10\nC2.Mother World\n";
  Passed = EvaluateTrue("ScanSetupFile()", "virtual template, two copies", "The geometry is accepted", ScanGeometry(Geometry, GetGeometryHead() + Copies)) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(1, 0, 10));
  Passed = EvaluateTrue("GetVolumeSequence()", "virtual template, first copy", "The daughter is at the position of the first copy", Sequence.GetDeepestVolume() != nullptr && Sequence.GetDeepestVolume()->GetMaterial() == Geometry.GetMaterial("Aluminium")) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(1, 0, -10));
  Passed = EvaluateTrue("GetVolumeSequence()", "virtual template, second copy", "The daughter is at the position of the second copy", Sequence.GetDeepestVolume() != nullptr && Sequence.GetDeepestVolume()->GetMaterial() == Geometry.GetMaterial("Aluminium")) && Passed;
  // Two new volumes for the two copies, in addition to World, V, D, C1, C2:
  Passed = Evaluate("GetNVolumes()", "virtual template, two copies", "The copies of the daughter are in the list", Geometry.GetNVolumes(), 7U) && Passed;

  // Defect GEO-A-25: The daughters of a virtual copy get a wrong position when its virtual template is placed too, since the position of the template is included twice
  MString Placed = "Volume V\nV.Material Vacuum\nV.Shape BRIK 2 2 2\nV.Position 0 0 10\nV.Virtual true\nV.Mother World\n";
  Placed += "Volume D\nD.Material Aluminium\nD.Shape BRIK 0.5 0.5 0.5\nD.Position 1 0 0\nD.Mother V\n";
  Placed += "V.Copy C\nC.Position 0 0 -10\nC.Mother World\n";
  Passed = EvaluateTrue("ScanSetupFile()", "placed virtual template", "The geometry is accepted", ScanGeometry(Geometry, GetGeometryHead() + Placed)) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(1, 0, 10));
  Passed = EvaluateTrue("GetVolumeSequence()", "placed virtual template", "The daughter of the template is at the position of the template", Sequence.GetDeepestVolume() != nullptr && Sequence.GetDeepestVolume()->GetMaterial() == Geometry.GetMaterial("Aluminium")) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(1, 0, -10));
  Passed = ExpectedToFail(EvaluateTrue("GetVolumeSequence()", "placed virtual template", "The daughter of the copy is at the position of the copy", Sequence.GetDeepestVolume() != nullptr && Sequence.GetDeepestVolume()->GetMaterial() == Geometry.GetMaterial("Aluminium"))) && Passed;

  // The virtual volume can be switched on in the scan for all non detector volumes:
  MString Tree = Marker("A") + "Volume D\nD.Material Silicon\nD.Shape BRIK 0.5 0.5 0.5\nD.Mother A\n" + FullDetector("Scintillator", "Det", "D") + Marker("Passive", 20.0);
  const MString FileName = GetTemporaryFileName("Virtualized.geo.setup");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(FileName, GetGeometryHead() + Tree)) && Passed;
  DisableDefaultStreams();
  bool Result = Geometry.ScanSetupFile(FileName, false, true, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "VirtualizeNonDetectorVolumes", "The virtualization is accepted", Result) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(0, 0, 0));
  Passed = EvaluateTrue("ScanSetupFile()", "VirtualizeNonDetectorVolumes", "The detector volume is found", Sequence.GetDeepestVolume() == Geometry.GetVolume("D")) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(0.9, 0, 0));
  Passed = EvaluateTrue("ScanSetupFile()", "VirtualizeNonDetectorVolumes", "The mother of the detector volume stays", Sequence.GetDeepestVolume() == Geometry.GetVolume("A")) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(20, 0, 0));
  Passed = EvaluateTrue("ScanSetupFile()", "VirtualizeNonDetectorVolumes", "The passive volume is gone", Sequence.GetDeepestVolume() == Geometry.GetWorldVolume()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the detector keywords
bool UTDGeometry::TestDetectors()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Every detector keyword with the expected class:
  struct DetectorCase {
    MString Keyword;
    MString Base;
    int Type;
  };
  vector<DetectorCase> Cases = {
    {"Strip2D", "Strip2D", MDDetector::c_Strip2D}, {"MDStrip2D", "Strip2D", MDDetector::c_Strip2D},
    {"Strip3D", "Strip3D", MDDetector::c_Strip3D}, {"MDStrip3D", "Strip3D", MDDetector::c_Strip3D},
    {"Strip3DDirectional", "Strip3DDirectional", MDDetector::c_Strip3DDirectional}, {"MDStrip3DDirectional", "Strip3DDirectional", MDDetector::c_Strip3DDirectional},
    {"DriftChamber", "DriftChamber", MDDetector::c_DriftChamber}, {"MDDriftChamber", "DriftChamber", MDDetector::c_DriftChamber},
    {"AngerCamera", "AngerCamera", MDDetector::c_AngerCamera}, {"MDAngerCamera", "AngerCamera", MDDetector::c_AngerCamera},
    {"Scintillator", "Scintillator", MDDetector::c_ACS}, {"Simple", "Scintillator", MDDetector::c_ACS}, {"ACS", "Scintillator", MDDetector::c_ACS}, {"MDACS", "Scintillator", MDDetector::c_ACS},
    {"Calorimeter", "Calorimeter", MDDetector::c_Calorimeter}, {"MDCalorimeter", "Calorimeter", MDDetector::c_Calorimeter},
    {"Voxel3D", "Voxel3D", MDDetector::c_Voxel3D}, {"MDVoxel3D", "Voxel3D", MDDetector::c_Voxel3D}
  };
  for (const DetectorCase& Case: Cases) {
    MString Text = FullDetector(Case.Base);
    Text.ReplaceAll(Case.Base + " D\n", Case.Keyword + " D\n");
    bool Result = ScanGeometry(Geometry, GetGeometryHead() + Wafer() + Text);
    Passed = EvaluateTrue("ScanSetupFile()", Case.Keyword, "The complete detector is accepted", Result) && Passed;
    if (Result == true) {
      Passed = Evaluate("MDDetector::GetType()", Case.Keyword, "The detector has the expected class", Geometry.GetDetector("D")->GetType(), Case.Type) && Passed;
      Passed = EvaluateTrue("MDVolume::IsSensitive()", Case.Keyword, "The volume is the sensitive volume", Geometry.GetVolume("Wafer")->IsSensitive()) && Passed;
      Passed = EvaluateTrue("MDVolume::GetDetector()", Case.Keyword, "The volume knows its detector", Geometry.GetVolume("Wafer")->GetDetector() == Geometry.GetDetector("D")) && Passed;
    }
  }

  // The common values:
  MString Common = FullDetector("Strip2D");
  Common.ReplaceAll("D.TriggerThreshold 30\n", "D.TriggerThreshold 30 3\n");
  Common.ReplaceAll("D.NoiseThreshold 15\n", "D.NoiseThreshold 15 2\n");
  Common += "D.FailureRate 0.25\nD.Overflow 1000 10\nD.TimeResolution 100 1E-6\nD.TimeResolutionAt 200 2E-6\nD.BlockTrigger 1 1\nD.StructuralPitch 0.5 0.25 1.0\nD.StructuralOffset 0.25 0.5 0.75\n";
  Passed = EvaluateTrue("ScanSetupFile()", "common detector options", "All common options are accepted", ScanGeometry(Geometry, GetGeometryHead() + Wafer() + Common)) && Passed;
  MDDetector* D = Geometry.GetDetector("D");
  if (EvaluateTrue("GetDetector()", "common detector options", "The detector exists", D != nullptr) == true) {
    Passed = EvaluateNear("TriggerThreshold", "30 3", "The trigger threshold is stored", D->GetTriggerThreshold(), 30.0, 1e-12) && Passed;
    Passed = EvaluateNear("TriggerThreshold", "30 3", "The sigma of the trigger threshold is stored", D->GetTriggerThresholdSigma(), 3.0, 1e-12) && Passed;
    Passed = EvaluateNear("NoiseThreshold", "15 2", "The noise threshold is stored", D->GetNoiseThreshold(), 15.0, 1e-12) && Passed;
    Passed = EvaluateNear("NoiseThreshold", "15 2", "The sigma of the noise threshold is stored", D->GetNoiseThresholdSigma(), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("FailureRate", "0.25", "The failure rate is stored", D->GetFailureRate(), 0.25, 1e-12) && Passed;
    Passed = EvaluateNear("Overflow", "1000 10", "The overflow is stored", D->GetOverflow(), 1000.0, 1e-12) && Passed;
    Passed = EvaluateNear("Overflow", "1000 10", "The sigma of the overflow is stored", D->GetOverflowSigma(), 10.0, 1e-12) && Passed;
    Passed = EvaluateNear("TimeResolution", "at 100", "The time resolution is stored", D->GetTimeResolution(100.0), 1E-6, 1e-15) && Passed;
    Passed = EvaluateNear("TimeResolutionAt", "at 200", "The time resolution is stored", D->GetTimeResolution(200.0), 2E-6, 1e-15) && Passed;
    Passed = EvaluateVectorNear("StructuralPitch", "0.5 0.25 1.0", "The pitch is stored", D->GetStructuralPitch(), MVector(0.5, 0.25, 1.0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("StructuralOffset", "0.25 0.5 0.75", "The offset is stored", D->GetStructuralOffset(), MVector(0.25, 0.5, 0.75), 1e-12) && Passed;
    MDStrip2D* Strip = dynamic_cast<MDStrip2D*>(D);
    if (EvaluateTrue("Strip2D", "cast", "The detector is a strip detector", Strip != nullptr) == true) {
      Passed = Evaluate("StripNumber", "9 9", "The number of strips in x is stored", Strip->GetNStripsX(), 9) && Passed;
      Passed = EvaluateNear("Offset", "0.5 0.5", "The offset in x is stored", Strip->GetOffsetX(), 0.5, 1e-12) && Passed;
    }
  }

  // The different energy resolutions:
  for (const MString Line: {"D.EnergyResolution Ideal", "D.EnergyResolution Perfect", "D.EnergyResolution None", "D.EnergyResolution No", "D.EnergyResolution Gauss 10 10 1\nD.EnergyResolution Gauss 100 100 1", "D.EnergyResolution Gaus 10 10 1\nD.EnergyResolution Gaus 100 100 1", "D.EnergyResolutionAt 10 1\nD.EnergyResolutionAt 100 1", "D.EnergyResolutionAt 10 1 0 0\nD.EnergyResolutionAt 100 1 0 0"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Line, "The energy resolution line is accepted", Accepts(Wafer() + "Scintillator D\nD.SensitiveVolume Wafer\nD.DetectorVolume Wafer\nD.TriggerThreshold 1\nD.NoiseThreshold 1\n" + Line + "\n")) && Passed;
  }
  Passed = EvaluateTrue("ScanSetupFile()", "Lorentz", "A Lorentz energy resolution is accepted", ScanGeometry(Geometry, GetGeometryHead() + Wafer() + "Scintillator D\nD.SensitiveVolume Wafer\nD.DetectorVolume Wafer\nD.TriggerThreshold 1\nD.NoiseThreshold 1\nD.EnergyResolution Lorentz 10 10 1\nD.EnergyResolution Lorentz 100 100 1\n")) && Passed;
  Passed = EvaluateNear("EnergyResolution", "Lorentz", "The type is stored", Geometry.GetDetector("D")->GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeLorentz, 1e-12) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "GaussLandau", "A Gauss-Landau energy resolution is accepted", ScanGeometry(Geometry, GetGeometryHead() + Wafer() + "Scintillator D\nD.SensitiveVolume Wafer\nD.DetectorVolume Wafer\nD.TriggerThreshold 1\nD.NoiseThreshold 1\nD.EnergyResolution GaussLandau 10 10 1 10 1 0.5\nD.EnergyResolution GaussLandau 100 100 1 100 1 0.5\n")) && Passed;
  Passed = EvaluateNear("EnergyResolution", "GaussLandau", "The type is stored", Geometry.GetDetector("D")->GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeGaussLandau, 1e-12) && Passed;

  // Rejected lines, for every detector:
  MString Scintillator = Wafer() + FullDetector("Scintillator");
  for (const MString Line: {"D.SensitiveVolume Unknown", "D.DetectorVolume Unknown", "D.NoiseThresholdEqualsTriggerThreshold", "D.NoiseThreshold", "D.NoiseThreshold 1 2 3", "D.TriggerThreshold", "D.TriggerThreshold 1 2 3",
    "D.FailureRate", "D.FailureRate 1 2", "D.Overflow", "D.Overflow 1 2 3", "D.EnergyLossMap", "D.EnergyLossMap DoesNotExist.dat", "D.EnergyLossMap a b", "D.EnergyResolution", "D.EnergyResolution 1", "D.EnergyResolution 1 2 3 4 5", "D.EnergyResolution Foo 1 2",
    "D.EnergyResolution Gauss 1 2", "D.EnergyResolution Lorentz 1 2", "D.EnergyResolution GaussLandau 1 2", "D.EnergyResolution Lorentz 10 10 1", "D.EnergyResolutionType", "D.EnergyResolutionType Foo", "D.EnergyCalibration", "D.EnergyCalibration DoesNotExist.dat",
    "D.TimeResolution", "D.TimeResolution 1", "D.TimeResolution 1 2 3", "D.PulseShape 1", "D.StructuralOffset 1 2", "D.StructuralPitch 1 2",
    "D.Offset 1 2", "D.StripNumber 1 2", "D.Pixels 1 2 3", "D.Voxels 1 2 3", "D.GuardRingTriggerThreshold 1", "D.GuardRingEnergyResolution 1 2", "D.DepthResolution 1 2", "D.DepthResolutionThreshold 1", "D.EnergyResolutionDepthCorrection 1 2", "D.TriggerThresholdDepthCorrection 1 2", "D.NoiseThresholdDepthCorrection 1 2",
    "D.DirectionalResolution 1 2", "D.PositionResolution 1 2", "D.Positioning XY", "D.LightSpeed 1", "D.LightDetectorPosition 1", "D.DriftConstant 1", "D.EnergyPerElectron 1", "D.LightEnergyResolution 1 2", "D.Unknown 1", "D.Assign 1 2 3"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad or wrong line for a scintillator is rejected", Accepts(Scintillator + Line + "\n")) && Passed;
  }

  // Deprecated and outdated keywords are accepted:
  for (const MString Line: {"D.VoxelSimulation true", "D.StructuralSize 1 2 3", "D.Width 1 2", "D.Pitch 1 2", "D.StripLength 1 2", "D.Orientation 2", "D.NoiseAxis 2", "D.HitPosition 1"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Line, "The deprecated keyword is ignored", Accepts(Scintillator + Line + "\n")) && Passed;
  }
  // The trigger threshold and the noise threshold:
  Passed = EvaluateTrue("ScanSetupFile()", "NoiseThresholdEqualsTriggerThreshold", "The flag is accepted", ScanGeometry(Geometry, GetGeometryHead() + Wafer() + "Scintillator D\nD.SensitiveVolume Wafer\nD.DetectorVolume Wafer\nD.TriggerThreshold 12\nD.NoiseThresholdEqualsTriggerThreshold true\nD.EnergyResolution Ideal\n")) && Passed;
  Passed = EvaluateTrue("NoiseThresholdEqualsTriggerThreshold", "true", "The flag is stored", Geometry.GetDetector("D")->GetNoiseThresholdEqualsTriggerThreshold()) && Passed;

  // Options of the strip detectors:
  MString Strip = Wafer() + FullDetector("Strip2D");
  for (const MString Line: {"D.Offset 1", "D.Offset 1 2 3", "D.StripNumber 1", "D.StripNumber 1 2 3", "D.Pixels 1 2 3", "D.DepthResolution 1 2", "D.GuardRingTriggerThreshold", "D.GuardRingTriggerThreshold 1 2 3", "D.GuardRingEnergyResolution 1", "D.GuardRingEnergyResolution 1 2 3"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad line for a strip detector is rejected", Accepts(Strip + Line + "\n")) && Passed;
  }

  // Guard rings:
  Passed = EvaluateTrue("ScanSetupFile()", "guard ring", "A guard ring is accepted", ScanGeometry(Geometry, GetGeometryHead() + Strip + "D.GuardRingTriggerThreshold 30 2\nD.GuardRingEnergyResolution 10 1\nD.GuardRingEnergyResolution 1000 1\n")) && Passed;
  Passed = EvaluateTrue("MDDetector::HasGuardRing()", "guard ring", "The detector has a guard ring", Geometry.GetDetector("D")->HasGuardRing()) && Passed;
  Passed = Evaluate("GetNDetectors()", "guard ring", "The guard ring is in the list", Geometry.GetNDetectors(), 2U) && Passed;
  Passed = EvaluateTrue("GetDetector()", "guard ring", "The guard ring has the name of the detector with an extension", Geometry.GetDetector("D_GuardRing") != nullptr) && Passed;
  Passed = EvaluateNear("GuardRingTriggerThreshold", "30 2", "The threshold of the guard ring is stored", Geometry.GetDetector("D_GuardRing")->GetTriggerThreshold(), 30.0, 1e-12) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "guard ring, other spelling", "The other spelling is accepted", Accepts(Strip + "D.GuardringTriggerThreshold 30\nD.GuardringEnergyResolutionAt 10 1\nD.GuardringEnergyResolution 1000 1\n")) && Passed;
  MString VoxelGuard = Wafer() + FullDetector("Voxel3D") + "D.GuardRingTriggerThreshold 30\nD.GuardRingEnergyResolution 10 1\nD.GuardRingEnergyResolution 1000 1\n";
  Passed = EvaluateTrue("ScanSetupFile()", "guard ring of Voxel3D", "A guard ring of a voxel detector is accepted", ScanGeometry(Geometry, GetGeometryHead() + VoxelGuard)) && Passed;
  Passed = Evaluate("GetNDetectors()", "guard ring of Voxel3D", "The guard ring is in the list", Geometry.GetNDetectors(), 2U) && Passed;

  // Options of the Strip3D:
  MString Strip3D = Wafer() + FullDetector("Strip3D");
  for (const MString Line: {"D.DepthResolutionThreshold 25.0", "D.TriggerThresholdDepthCorrection 0.0 1.0", "D.TriggerThresholdDepthCorrection 1.0 1.0", "D.TriggerThresholdDepthCorrectionAt 0.0 1.0", "D.NoiseThresholdDepthCorrection 0.0 1.0", "D.NoiseThresholdDepthCorrection 1.0 1.0", "D.NoiseThresholdDepthCorrectionAt 0.0 1.0", "D.EnergyResolutionDepthCorrection 0.0 1.0", "D.EnergyResolutionDepthCorrectionAt Gauss 0.0 1.0 1.0"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Line, "The Strip3D option is accepted", Accepts(Strip3D + Line + "\n")) && Passed;
  }
  for (const MString Line: {"D.DepthResolutionThreshold", "D.DepthResolutionThreshold 1 2", "D.TriggerThresholdDepthCorrection 1", "D.TriggerThresholdDepthCorrection 1 2 3", "D.NoiseThresholdDepthCorrection 1", "D.NoiseThresholdDepthCorrection 1 2 3", "D.EnergyResolutionDepthCorrection", "D.EnergyResolutionDepthCorrection 1", "D.EnergyResolutionDepthCorrection Foo 1 2 3", "D.EnergyResolutionDepthCorrection Gauss 1 2", "D.EnergyResolutionDepthCorrection Lorentz 1 2 3"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad Strip3D option is rejected", Accepts(Strip3D + Line + "\n")) && Passed;
  }
  // Defect GEO-A-26: A DepthResolution line with a wrong number of values is silently ignored
  for (const MString Line: {"D.DepthResolution 1", "D.DepthResolution 1 2 3 4"}) {
    Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", Line, "The depth resolution with a wrong number of values is rejected", Accepts(Strip3D + Line + "\n"))) && Passed;
  }

  // Options of the directional detector:
  MString Directional = Wafer() + FullDetector("Strip3DDirectional");
  Passed = EvaluateFalse("ScanSetupFile()", "DirectionalResolution 1", "A directional resolution with one value is rejected", Accepts(Directional + "D.DirectionalResolution 1\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "DirectionalResolution with sigma", "A directional resolution with sigma is accepted", Accepts(Directional + "D.DirectionalResolutionAt 500 1 1E-5\n")) && Passed;

  // Options of the Anger camera:
  MString Anger = Wafer() + FullDetector("AngerCamera");
  for (const MString Line: {"D.Positioning XYZ", "D.Positioning XYZIndependent", "D.Positioning XY"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Line, "The positioning is accepted", Accepts(Anger + Line + "\n")) && Passed;
  }
  for (const MString Line: {"D.Positioning", "D.Positioning Foo", "D.PositionResolution 1", "D.PositionResolution 1 2 3"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad Anger camera line is rejected", Accepts(Anger + Line + "\n")) && Passed;
  }
  MString AngerXYZ = Wafer() + "AngerCamera D\n" + DetectorCommon("D") + "D.Positioning XYZIndependent\nD.PositionResolution 10 0.5 0.5 0.5\nD.PositionResolution 1000 0.5 0.5 0.5\n";
  Passed = EvaluateTrue("ScanSetupFile()", "PositionResolution with 3 sigmas", "Independent resolutions are accepted", Accepts(AngerXYZ)) && Passed;

  // Options of the drift chamber:
  MString Drift = Wafer() + FullDetector("DriftChamber");
  Passed = EvaluateTrue("ScanSetupFile()", "drift chamber", "The drift chamber is accepted", ScanGeometry(Geometry, GetGeometryHead() + Drift)) && Passed;
  MDDriftChamber* Chamber = dynamic_cast<MDDriftChamber*>(Geometry.GetDetector("D"));
  if (EvaluateTrue("DriftChamber", "cast", "The detector is a drift chamber", Chamber != nullptr) == true) {
    Passed = EvaluateNear("LightSpeed", "18E+9", "The light speed is stored", Chamber->GetLightSpeed(), 18E+9, 1e-3) && Passed;
    Passed = Evaluate("LightDetectorPosition", "3", "The light detector position is stored", Chamber->GetLightDetectorPosition(), 3) && Passed;
    Passed = EvaluateNear("DriftConstant", "1E-6", "The drift constant is stored", Chamber->GetDriftConstant(), 1E-6, 1e-15) && Passed;
    Passed = EvaluateNear("EnergyPerElectron", "15.6", "The energy per electron is stored", Chamber->GetEnergyPerElectron(), 15.6, 1e-12) && Passed;
  }
  for (const MString Line: {"D.LightSpeed", "D.LightSpeed 1 2", "D.LightDetectorPosition", "D.LightDetectorPosition 1 2", "D.DriftConstant", "D.DriftConstant 1 2", "D.EnergyPerElectron", "D.EnergyPerElectron 1 2", "D.LightEnergyResolution 1", "D.LightEnergyResolution 1 2 3"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad drift chamber line is rejected", Accepts(Drift + Line + "\n")) && Passed;
  }

  // Voxel detector:
  MString Voxel = Wafer() + FullDetector("Voxel3D");
  Passed = EvaluateTrue("ScanSetupFile()", "Voxel3D", "The voxel detector is accepted", ScanGeometry(Geometry, GetGeometryHead() + Voxel)) && Passed;
  Passed = Evaluate("VoxelNumber", "9 9 4", "The number of voxels in z is stored", dynamic_cast<MDVoxel3D*>(Geometry.GetDetector("D"))->GetNVoxelsZ(), 4) && Passed;
  for (const MString Other: {"D.Pixels 9 9 4", "D.Pixel 9 9 4", "D.PixelNumber 9 9 4", "D.Voxels 9 9 4", "D.Voxel 9 9 4"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Other, "The spelling of the voxel number is accepted", Accepts(Voxel + Other + "\n")) && Passed;
  }
  for (const MString Line: {"D.Voxels 1 2", "D.Voxels 1 2 3 4", "D.Offset 1 2", "D.Offset 1 2 3 4"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad voxel line is rejected", Accepts(Voxel + Line + "\n")) && Passed;
  }

  // Energy loss map and energy calibration, read relative to the setup file:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(GetTemporaryFileName("Loss.dat"), "IP LIN\nDP 0 0\nDP 1000 1000\nEN\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "EnergyLossMap", "An existing energy loss map is accepted", Accepts(Scintillator + "D.EnergyLossMap Loss.dat\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "EnergyCalibration", "An existing energy calibration is accepted", Accepts(Scintillator + "D.EnergyCalibration Loss.dat\n")) && Passed;

  // The pulse shape:
  Passed = EvaluateTrue("ScanSetupFile()", "PulseShape", "A pulse shape with all parameters is accepted", Accepts(Scintillator + "D.PulseShape 1 2 3 4 5 6 7 8 9 10 0 100\n")) && Passed;

  // Several sensitive volumes need a common volume:
  MString Multi = Marker("Common") + "Common.Virtual false\nVolume S1\nS1.Material Silicon\nS1.Shape BRIK 0.2 0.2 0.2\nS1.Position 0.5 0 0\nS1.Mother Common\nVolume S2\nS2.Material Silicon\nS2.Shape BRIK 0.2 0.2 0.2\nS2.Position -0.5 0 0\nS2.Mother Common\n";
  MString MultiDetector = "Scintillator D\nD.SensitiveVolume S1\nD.SensitiveVolume S2\nD.DetectorVolume Common\nD.TriggerThreshold 1\nD.NoiseThreshold 1\nD.EnergyResolution Gauss 10 10 1\nD.EnergyResolution Gauss 100 100 1\n";
  Passed = EvaluateTrue("ScanSetupFile()", "two sensitive volumes", "Two sensitive volumes in one volume are accepted", ScanGeometry(Geometry, GetGeometryHead() + Multi + MultiDetector)) && Passed;
  Passed = EvaluateTrue("MDDetector::GetCommonVolume()", "two sensitive volumes", "The common volume is the mother", Geometry.GetDetector("D")->GetCommonVolume() == Geometry.GetVolume("Common")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "two sensitive volumes and a third one", "A third sensitive volume in the common volume is rejected", Accepts(Multi + MultiDetector + "Volume S3\nS3.Material Silicon\nS3.Shape BRIK 0.1 0.1 0.1\nS3.Position 0 0.5 0\nS3.Mother Common\nScintillator E\nE.SensitiveVolume S3\nE.DetectorVolume S3\nE.TriggerThreshold 1\nE.NoiseThreshold 1\nE.EnergyResolution Gauss 10 10 1\nE.EnergyResolution Gauss 100 100 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "copy as sensitive volume", "Copies as one of several sensitive volumes are rejected", Accepts(Multi + "S1.Copy S1C\nS1C.Position 0 0.5 0\nS1C.Mother Common\n" + MultiDetector)) && Passed;

  // Blocked channels need two values:
  Passed = EvaluateFalse("ScanSetupFile()", "BlockTrigger 1", "BlockTrigger with one value is rejected", Accepts(Strip + "D.BlockTrigger 1\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test named detectors and Assign
bool UTDGeometry::TestNamedDetectors()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Two wafers, and a detector with the two named versions: positions by construction
  MString Base = Wafer("WaferA", 5.0) + "WaferA.Copy WaferB\nWaferB.Position 0 0 -5\nWaferB.Mother World\n" + FullDetector("Strip2D", "D", "WaferA");
  MString Named = Base + "D.Named N1\nN1.Assign 0 0 5\nD.NamedDetector N2\nN2.Assign World.WaferB\n";
  Passed = EvaluateTrue("ScanSetupFile()", "named detectors", "Named detectors are accepted", ScanGeometry(Geometry, GetGeometryHead() + Named)) && Passed;
  Passed = Evaluate("GetNDetectors()", "named detectors", "The named detectors are in the list", Geometry.GetNDetectors(), 3U) && Passed;
  MDDetector* N1 = Geometry.GetDetector("N1");
  if (EvaluateTrue("GetDetector()", "N1", "The named detector exists", N1 != nullptr) == true) {
    Passed = EvaluateTrue("MDDetector::IsNamedDetector()", "N1", "It is named", N1->IsNamedDetector()) && Passed;
    Passed = EvaluateTrue("MDDetector::GetNamedAfterDetector()", "N1", "It is named after the template", N1->GetNamedAfterDetector() == Geometry.GetDetector("D")) && Passed;
    // Inherited properties:
    Passed = EvaluateNear("CopyDataToNamedDetectors", "N1", "The threshold is inherited", N1->GetTriggerThreshold(), 30.0, 1e-12) && Passed;
  }
  Passed = EvaluateTrue("MDDetector::HasNamedDetectors()", "D", "The template has named detectors", Geometry.GetDetector("D")->HasNamedDetectors()) && Passed;
  Passed = Evaluate("MDDetector::GetNNamedDetectors()", "D", "The template has two named detectors", Geometry.GetDetector("D")->GetNNamedDetectors(), 2U) && Passed;

  // An own threshold of a named detector replaces the one of the template:
  MString Override = Named + "N1.TriggerThreshold 44\n";
  Passed = EvaluateTrue("ScanSetupFile()", "named detector with own threshold", "An own threshold is accepted", ScanGeometry(Geometry, GetGeometryHead() + Override)) && Passed;
  Passed = EvaluateNear("TriggerThreshold", "N1", "The named detector keeps its own threshold", Geometry.GetDetector("N1")->GetTriggerThreshold(), 44.0, 1e-12) && Passed;
  Passed = EvaluateNear("TriggerThreshold", "N2", "The other one has the threshold of the template", Geometry.GetDetector("N2")->GetTriggerThreshold(), 30.0, 1e-12) && Passed;

  // Rejections:
  for (const MString Line: {"D.Named", "D.Named N3 N4", "D.Named Bad-Name", "D.Named World", "N1.Named N3", "D.Assign 0 0 5", "N1.Assign 0 0", "N1.Assign 0 0 5 0", "N1.Assign 0 0 50", "N1.Assign World.Unknown", "N1.Assign Wrong.WaferA", "N1.Assign World", "N1.SensitiveVolume WaferA", "N1.DetectorVolume WaferA", "N1.StructuralOffset 0 0 0", "N1.StructuralPitch 0 0 0", "N1.Offset 1 1", "N1.StripNumber 1 1", "N1.EnergyLossMap Loss.dat"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad line of a named detector is rejected", Accepts(Base + "D.Named N1\n" + Line + "\n")) && Passed;
  }
  Passed = EvaluateFalse("ScanSetupFile()", "Pixels on named", "A voxel number of a named detector is rejected", Accepts(Wafer() + FullDetector("Voxel3D") + "D.Named N1\nN1.Assign 0 0 0\nN1.Voxels 1 1 1\n")) && Passed;

  // Defect GEO-A-27: An Assign which points into a different detector than the template of the named detector is accepted
  MString TwoDetectors = Wafer("WaferA", 5.0) + Wafer("WaferC", -5.0) + FullDetector("Strip2D", "D", "WaferA") + FullDetector("Strip2D", "E", "WaferC") + "D.Named N1\nN1.Assign 0 0 -5\n";
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "Assign into another detector", "An absolute position in another detector is rejected", Accepts(TwoDetectors))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the trigger keywords
bool UTDGeometry::TestTriggers()
{
  bool Passed = true;
  MDGeometry Geometry;

  MString Det = Wafer() + FullDetector("Strip2D");

  // The basic trigger:
  MString Basic = Det + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector D 2\n";
  Passed = EvaluateTrue("ScanSetupFile()", "basic trigger", "A basic trigger is accepted", ScanGeometry(Geometry, GetGeometryHead() + Basic)) && Passed;
  Passed = Evaluate("GetNTriggers()", "basic trigger", "One trigger exists", Geometry.GetNTriggers(), 1U) && Passed;
  MDTriggerBasic* T = dynamic_cast<MDTriggerBasic*>(Geometry.GetTrigger("T"));
  if (EvaluateTrue("GetTrigger()", "basic trigger", "The trigger exists", T != nullptr) == true) {
    Passed = EvaluateFalse("Veto", "false", "The veto flag is stored", T->IsVeto()) && Passed;
    Passed = EvaluateTrue("TriggerByChannel", "true", "The trigger by channel flag is stored", T->IsTriggerByChannel()) && Passed;
    Passed = EvaluateSize("Detector", "D 2", "The detector is stored", T->GetDetectors().size(), 1) && Passed;
    Passed = Evaluate("Detector", "D 2", "The trigger type of a normal detector is 0", T->GetTriggerTypes()[0], 0) && Passed;
  }

  Passed = EvaluateTrue("ScanSetupFile()", "DetectorType", "A trigger for a detector type is accepted", ScanGeometry(Geometry, GetGeometryHead() + Det + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.DetectorType Strip2D 1\n")) && Passed;
  T = dynamic_cast<MDTriggerBasic*>(Geometry.GetTrigger("T"));
  if (EvaluateTrue("GetTrigger()", "DetectorType", "The trigger exists", T != nullptr) == true) {
    Passed = EvaluateSize("DetectorType", "Strip2D 1", "The detector type is stored", T->GetDetectorTypes().size(), 1) && Passed;
  }
  Passed = EvaluateFalse("ScanSetupFile()", "Detector and DetectorType", "Detector and DetectorType cannot be combined", Accepts(Det + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector D 1\nT.DetectorType Strip2D 1\n")) && Passed;

  Passed = EvaluateTrue("ScanSetupFile()", "TriggerBasic and veto", "The other keyword and a veto are accepted", ScanGeometry(Geometry, GetGeometryHead() + Det + "D.NoiseThresholdEqualsTriggerThreshold true\nTriggerBasic T\nT.Veto true\nT.TriggerByDetector true\nT.Detector D 1\n")) && Passed;
  T = dynamic_cast<MDTriggerBasic*>(Geometry.GetTrigger("T"));
  if (EvaluateTrue("GetTrigger()", "veto", "The trigger exists", T != nullptr) == true) {
    Passed = EvaluateTrue("Veto", "true", "The veto flag is stored", T->IsVeto()) && Passed;
    Passed = EvaluateTrue("TriggerByDetector", "true", "The trigger by detector flag is stored", T->IsTriggerByDetector()) && Passed;
  }

  // A detector with only a veto trigger needs the flag:
  Passed = EvaluateFalse("ScanSetupFile()", "veto without flag", "A veto only detector without NoiseThresholdEqualsTriggerThreshold is rejected", Accepts(Det + "Trigger T\nT.Veto true\nT.TriggerByChannel true\nT.Detector D 1\n")) && Passed;

  // The deprecated keyword: 1 is the type of a Strip2D
  Passed = EvaluateTrue("ScanSetupFile()", "PositiveDetectorType", "The deprecated keyword is still accepted", ScanGeometry(Geometry, GetGeometryHead() + Det + "Trigger T\nT.PositiveDetectorType 1 1\n")) && Passed;
  T = dynamic_cast<MDTriggerBasic*>(Geometry.GetTrigger("T"));
  if (EvaluateTrue("GetTrigger()", "PositiveDetectorType", "The trigger exists", T != nullptr) == true) {
    Passed = EvaluateFalse("PositiveDetectorType", "1 1", "The keyword means no veto", T->IsVeto()) && Passed;
    Passed = EvaluateTrue("PositiveDetectorType", "1 1", "The keyword means trigger by channel", T->IsTriggerByChannel()) && Passed;
  }

  // Guard ring triggers:
  MString Guard = Det + "D.GuardRingTriggerThreshold 30\nD.GuardRingEnergyResolution 10 1\nD.GuardRingEnergyResolution 1000 1\n";
  for (const MString Line: {"T.GuardRingDetector D 1", "T.GuardringDetector D 1"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Line, "The guard ring trigger is accepted", Accepts(Guard + "Trigger T\nT.Veto true\nT.TriggerByChannel true\n" + Line + "\n")) && Passed;
  }
  for (const MString Line: {"T.GuardRingDetectorType Strip2D 1", "T.GuardringDetectorType Strip2D 1"}) {
    // Defect GEO-E-30: The validation of a guard ring trigger for a detector type reads the empty list of detectors
    Passed = ExpectedToCrash(EvaluateTrue("ScanSetupFile()", Line, "The guard ring trigger is accepted", Accepts(Guard + "Trigger T\nT.Veto true\nT.TriggerByChannel true\n" + Line + "\n"))) && Passed;
  }

  // A guard ring trigger needs a detector with a guard ring: the same veto is accepted as normal trigger line, but not as guard ring line
  MString Scinti = Wafer("WaferS", 5.0) + FullDetector("Scintillator", "Sc", "WaferS") + "Sc.NoiseThresholdEqualsTriggerThreshold true\n";
  Passed = EvaluateTrue("ScanSetupFile()", "T.Detector Sc 1", "A veto of a scintillator is accepted", Accepts(Scinti + "Trigger T\nT.Veto true\nT.TriggerByChannel true\nT.Detector Sc 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "T.GuardRingDetector Sc 1", "A guard ring veto of a detector without guard ring is rejected", Accepts(Scinti + "Trigger T\nT.Veto true\nT.TriggerByChannel true\nT.GuardRingDetector Sc 1\n")) && Passed;

  // Rejections:
  for (const MString Line: {"T.TriggerByChannel", "T.TriggerByChannel a b", "T.TriggerByDetector", "T.TriggerByDetector a b", "T.Veto", "T.Veto a b", "T.DetectorType Strip2D", "T.DetectorType Strip2D 1 1", "T.DetectorType Foo 1", "T.DetectorType Calorimeter 1", "T.Detector D", "T.Detector D 1 1", "T.Detector Unknown 1", "T.GuardRingDetectorType Strip2D", "T.GuardRingDetectorType Foo 1", "T.GuardRingDetector D", "T.GuardRingDetector Unknown 1", "T.Map a", "T.Map", "T.NegativeDetectorType 1 1", "T.PositiveDetectorType 1", "T.PositiveDetectorType 9 1", "T.Unknown 1"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad trigger line is rejected", Accepts(Det + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector D 1\n" + Line + "\n")) && Passed;
  }

  // A map trigger:
  const MString MapFile = GetTemporaryFileName("Map.trig");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(MapFile, "DT D\nTV 1+\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "map trigger", "A map trigger is accepted", ScanGeometry(Geometry, GetGeometryHead() + Det + "TriggerMap T\nT.Map Map.trig\n")) && Passed;
  Passed = EvaluateTrue("MDTrigger::GetType()", "map trigger", "The trigger is a universal one", Geometry.GetTrigger("T")->GetType() == MDTriggerType::c_Universal) && Passed;

  for (const MString Line: {"T.TriggerByChannel true", "T.TriggerByDetector true", "T.Veto true", "T.DetectorType Strip2D 1", "T.Detector D 1", "T.GuardRingDetectorType Strip2D 1", "T.GuardRingDetector D 1", "T.PositiveDetectorType 1 1", "T.Map DoesNotExist.trig"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad line of a map trigger is rejected", Accepts(Det + "TriggerMap T\nT.Map Map.trig\n" + Line + "\n")) && Passed;
  }
  Passed = EvaluateFalse("ScanSetupFile()", "T.Map on a basic trigger", "A map on a basic trigger is rejected", Accepts(Det + "Trigger T\nT.Map Map.trig\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "mixed triggers", "Basic and map triggers cannot be mixed", Accepts(Det + "TriggerMap T\nT.Map Map.trig\nTrigger T2\nT2.Veto false\nT2.TriggerByChannel true\nT2.Detector D 1\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the system keywords
bool UTDGeometry::TestSystem()
{
  bool Passed = true;
  MDGeometry Geometry;

  Passed = EvaluateTrue("ScanSetupFile()", "system", "A system is accepted", ScanGeometry(Geometry, GetGeometryHead() + "System S\nS.TimeResolution Gauss 2.5E-6\n")) && Passed;
  Passed = EvaluateTrue("GetSystem(Name)", "S", "The system can be found by its name", Geometry.GetSystem("S") != nullptr) && Passed;
  Passed = EvaluateTrue("GetSystem()", "S", "The system is the current one", Geometry.GetSystem() == Geometry.GetSystem("S")) && Passed;
  Passed = EvaluateNear("TimeResolution", "Gauss 2.5E-6", "The type is stored", Geometry.GetSystem()->GetTimeResolutionType(), MDSystem::c_TimeResolutionTypeGauss, 1e-12) && Passed;
  Passed = EvaluateNear("TimeResolution", "Gauss 2.5E-6", "The sigma is stored", Geometry.GetSystem()->GetTimeResolutionGaussSigma(), 2.5E-6, 1e-15) && Passed;

  for (const MString Type: {"Ideal", "none", "No", "NIX", "Perfect"}) {
    Passed = EvaluateTrue("ScanSetupFile()", Type, "The ideal time resolution is accepted", ScanGeometry(Geometry, GetGeometryHead() + "System S\nS.TimeResolution " + Type + "\n")) && Passed;
    Passed = EvaluateNear("TimeResolution", Type, "The type is ideal", Geometry.GetSystem()->GetTimeResolutionType(), MDSystem::c_TimeResolutionTypeIdeal, 1e-12) && Passed;
  }
  for (const MString Line: {"S.TimeResolution", "S.TimeResolution Gauss", "S.TimeResolution Gauss 1 2", "S.TimeResolution Foo", "S.Unknown 1"}) {
    Passed = EvaluateFalse("ScanSetupFile()", Line, "The bad system line is rejected", Accepts(MString("System S\n") + Line + "\n")) && Passed;
  }
  // The default system of a second scan:
  Passed = EvaluateTrue("ScanSetupFile()", "no system", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead())) && Passed;
  Passed = EvaluateTrue("GetSystem(Name)", "no system", "The former system is gone", Geometry.GetSystem("S") == nullptr) && Passed;

  // Defect GEO-A-28: A second System keyword silently replaces the first one, whose options are then ignored
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "two systems", "A second system is rejected", Accepts("System S\nSystem Q\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the reuse of one object with several scans
bool UTDGeometry::TestReuse()
{
  bool Passed = true;
  UTDGeometryAccess Geometry;

  // A full geometry first, then a smaller one:
  MString First = GetGeometryHead() + "Orientation O\nO.Position 0 0 0\nShape BRIK S\nS.Parameters 1 1 1\nVector M\nConstant Val 3\nSystem Sys\n" + Wafer() + FullDetector("Strip2D") + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector D 1\n";
  Passed = EvaluateTrue("ScanSetupFile()", "first scan", "The first geometry is accepted", ScanGeometry(Geometry, First)) && Passed;
  Passed = Evaluate("GetNDetectors()", "first scan", "One detector exists", Geometry.GetNDetectors(), 1U) && Passed;
  long FirstHash = Geometry.GetHash();

  MString Second = "Name Second\nSurroundingSphere 7 0 0 0 7\nMaterial Mine\nMine.Density 1\nMine.Component H 1\nVolume W\nW.Material Mine\nW.Shape BRIK 10 10 10\nW.Mother 0\n";
  Passed = EvaluateTrue("ScanSetupFile()", "second scan", "The second geometry is accepted", ScanGeometry(Geometry, Second)) && Passed;
  Passed = EvaluateTrue("IsScanned()", "second scan", "The object is scanned", Geometry.IsScanned()) && Passed;
  Passed = EvaluateTrue("GetName()", "second scan", "The new name is stored", Geometry.GetName() == "Second") && Passed;
  Passed = Evaluate("GetNVolumes()", "second scan", "Only the new volume exists", Geometry.GetNVolumes(), 1U) && Passed;
  Passed = Evaluate("GetNMaterials()", "second scan", "Only the new material exists", Geometry.GetNMaterials(), 1U) && Passed;
  Passed = Evaluate("GetNDetectors()", "second scan", "The detector is gone", Geometry.GetNDetectors(), 0U) && Passed;
  Passed = Evaluate("GetNTriggers()", "second scan", "The trigger is gone", Geometry.GetNTriggers(), 0U) && Passed;
  Passed = Evaluate("GetNOrientations()", "second scan", "The orientation is gone", Geometry.GetNOrientations(), 0U) && Passed;
  Passed = Evaluate("GetNShapes()", "second scan", "Only the shape of the new volume exists", Geometry.GetNShapes(), 1U) && Passed;
  Passed = Evaluate("GetNVectors()", "second scan", "The vector is gone", Geometry.GetNVectors(), 0U) && Passed;
  Passed = Evaluate("GetNIncludes()", "second scan", "Only the new file is in the list", Geometry.GetNIncludes(), 1) && Passed;
  Passed = EvaluateNear("GetStartSphereRadius()", "second scan", "The new radius is stored", Geometry.GetStartSphereRadius(), 7.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetSystem(Name)", "second scan", "The former system is gone", Geometry.GetSystem("Sys") == nullptr) && Passed;
  Passed = EvaluateTrue("GetVolume()", "second scan", "The cached world of the first scan is gone", Geometry.GetVolume("World") == nullptr) && Passed;
  Passed = EvaluateFalse("GetHash()", "second scan", "The hash changes with the content", Geometry.GetHash() == FirstHash) && Passed;
  // The constant of the first geometry does not apply any more:
  Passed = EvaluateFalse("ScanSetupFile()", "constant of the first scan", "The former constant is not defined", ScanGeometry(Geometry, GetGeometryHead() + Marker("A") + "A.Position { Val + 1 } 0 0\n")) && Passed;

  // The same geometry again gives the same content, and the IDs start again:
  Passed = EvaluateTrue("ScanSetupFile()", "repeat", "The same file can be read again", ScanGeometry(Geometry, First)) && Passed;
  int FirstMaterialID = Geometry.GetMaterial("Silicon")->GetID();
  unsigned int FirstVolumes = Geometry.GetNVolumes();
  Passed = EvaluateTrue("ScanSetupFile()", "repeat", "The same file can be read a third time", ScanGeometry(Geometry, First)) && Passed;
  Passed = Evaluate("GetNVolumes()", "repeat", "The number of volumes is the same", Geometry.GetNVolumes(), FirstVolumes) && Passed;
  Passed = Evaluate("MDMaterial::GetID()", "repeat", "The IDs are the same again", Geometry.GetMaterial("Silicon")->GetID(), FirstMaterialID) && Passed;
  Passed = EvaluateTrue("GetHash()", "repeat", "The hash is the same for the same content", Geometry.GetHash() == FirstHash) && Passed;

  // A guard ring geometry twice, which is in the detector list and belongs to the detector:
  MString Guard = GetGeometryHead() + Wafer() + FullDetector("Strip2D") + "D.GuardRingTriggerThreshold 30\nD.GuardRingEnergyResolution 10 1\nD.GuardRingEnergyResolution 1000 1\n";
  MDGeometry* Twice = new MDGeometry();
  Passed = EvaluateTrue("ScanSetupFile()", "guard ring", "The geometry with a guard ring is read", ScanGeometry(*Twice, Guard)) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "guard ring twice", "The geometry with a guard ring is read a second time", ScanGeometry(*Twice, Guard)) && Passed;
  // Destroying the object must not crash
  delete Twice;

  // A failed scan does not leave a scanned geometry:
  Passed = EvaluateFalse("ScanSetupFile()", "failing scan", "The bad geometry is rejected", ScanGeometry(Geometry, GetGeometryHead() + "Volume\n")) && Passed;
  Passed = EvaluateFalse("IsScanned()", "failing scan", "The object is not scanned", Geometry.IsScanned()) && Passed;
  Passed = Evaluate("GetNVolumes()", "failing scan", "No volume is left", Geometry.GetNVolumes(), 0U) && Passed;
  Passed = EvaluateTrue("GetWorldVolume()", "failing scan", "No world volume is left", Geometry.GetWorldVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("GetHash()", "failing scan", "The hash is 0", Geometry.GetHash() == 0) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "scan after a failed scan", "The object can be used again", ScanGeometry(Geometry, GetGeometryHead())) && Passed;

  // The ROOT geometry of the object goes with the object:
  gGeoManager = nullptr;
  {
    MDGeometry Temporary;
    DisableDefaultStreams();
    Temporary.Reset();
    EnableDefaultStreams();
  }
  // Defect GEO-A-49: The destructor creates a new ROOT geometry in Reset, which is never deleted and stays in gGeoManager
  Passed = ExpectedToFail(EvaluateTrue("~MDGeometry()", "gGeoManager", "No ROOT geometry of a destroyed object is left", gGeoManager == nullptr)) && Passed;

  // Reset:
  Geometry.Reset();
  Passed = EvaluateFalse("Reset()", "IsScanned", "The object is not scanned after a reset", Geometry.IsScanned()) && Passed;
  Passed = Evaluate("Reset()", "GetNVolumes", "No volume is left after a reset", Geometry.GetNVolumes(), 0U) && Passed;
  Passed = EvaluateTrue("Reset()", "GetHash", "The hash is 0 after a reset", Geometry.GetHash() == 0) && Passed;
  Geometry.Reset();
  Passed = EvaluateFalse("Reset()", "twice", "A second reset does not hurt", Geometry.IsScanned()) && Passed;

  // Defect GEO-A-29: Reset does not reset the default color, ShowVolumes, and the version, so that they leak into the next scan
  Passed = EvaluateTrue("ScanSetupFile()", "DefaultColor, first", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "DefaultColor 7\nShowVolumes false\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "DefaultColor, second", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("A") + "A.Color 2\nA.Visibility 1\n")) && Passed;
  Passed = ExpectedToFail(Evaluate("ScanSetupFile()", "DefaultColor of the former scan", "The default color of the former scan does not apply", Geometry.GetVolume("A")->GetColor(), 2)) && Passed;
  // Defect GEO-A-29: Reset does not reset the default color, ShowVolumes, and the version, so that they leak into the next scan
  Passed = ExpectedToFail(Evaluate("ScanSetupFile()", "ShowVolumes of the former scan", "ShowVolumes of the former scan does not apply", Geometry.GetVolume("A")->GetVisibility(), 1)) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "Version, first", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead())) && Passed;
  MString NoVersion = "Name N\nSurroundingSphere 1 0 0 0 1\nMaterial Mine\nMine.Density 1\nMine.Component H 1\nVolume W\nW.Material Mine\nW.Shape BRIK 10 10 10\nW.Mother 0\n";
  Passed = EvaluateTrue("ScanSetupFile()", "Version, second", "The file is accepted", ScanGeometry(Geometry, NoVersion)) && Passed;
  // Defect GEO-A-29: Reset does not reset the default color, ShowVolumes, and the version, so that they leak into the next scan
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "version of the former scan", "The version of the former scan does not apply", Geometry.m_Version == "1.0")) && Passed;

  // Defect GEO-A-30: A scan which fails during the validation of the tree keeps the partial content and a hash
  Passed = EvaluateFalse("ScanSetupFile()", "volume without material", "The bad geometry is rejected", ScanGeometry(Geometry, GetGeometryHead() + "Volume A\nA.Shape BRIK 1 1 1\nA.Mother World\n")) && Passed;
  Passed = ExpectedToFail(Evaluate("GetNVolumes()", "volume without material", "No volume is left after the failed scan", Geometry.GetNVolumes(), 0U)) && Passed;
  // Defect GEO-A-30: A scan which fails during the validation of the tree keeps the partial content and a hash
  Passed = ExpectedToFail(EvaluateTrue("GetHash()", "volume without material", "The hash is 0 after the failed scan", Geometry.GetHash() == 0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test two geometry objects at the same time
bool UTDGeometry::TestTwoObjects()
{
  bool Passed = true;

  MString Small = GetGeometryHead() + Marker("A");
  MString Other = GetGeometryHead() + Marker("B");
  MDGeometry First;
  MDGeometry Second;
  Passed = EvaluateTrue("ScanSetupFile()", "first of two objects", "The first object reads its file", ScanGeometry(First, Small, "First.geo.setup")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "second of two objects", "The second object reads its file", ScanGeometry(Second, Other, "Second.geo.setup")) && Passed;
  Passed = EvaluateTrue("GetVolume()", "first of two objects", "The first object keeps its content", First.GetVolume("A") != nullptr && First.GetVolume("B") == nullptr) && Passed;
  Passed = EvaluateTrue("GetVolume()", "second of two objects", "The second object has its content", Second.GetVolume("B") != nullptr && Second.GetVolume("A") == nullptr) && Passed;
  // The IDs of the volumes are static:
  Passed = EvaluateTrue("GetVolumeSequence()", "first of two objects", "The first object still finds its volume", First.GetVolumeSequence(MVector(0, 0, 0)).GetDeepestVolume() == First.GetVolume("A")) && Passed;

  // Destroy one, and use the other one:
  MDGeometry* Destroyed = new MDGeometry();
  MDGeometry Kept;
  Passed = EvaluateTrue("ScanSetupFile()", "two objects", "Both objects read their files", ScanGeometry(*Destroyed, Small, "First.geo.setup") && ScanGeometry(Kept, Other, "Second.geo.setup")) && Passed;
  delete Destroyed;
  Passed = EvaluateTrue("~MDGeometry()", "two objects", "Destroying one object does not hurt the other one", ScanGeometry(Kept, Other, "Second.geo.setup")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test GetHash and RequiresReload
bool UTDGeometry::TestHashAndReload()
{
  bool Passed = true;
  MDGeometry Geometry;

  PrepareTemporaryDirectory("Hash");
  const MString Dir = GetTemporaryDirectoryName("Hash");
  const MString Main = Dir + "/Main.geo.setup";
  const MString Sub = Dir + "/Sub.geo";
  const MString Loss = Dir + "/Loss.dat";

  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Sub, Marker("A"))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Loss, "IP LIN\nDP 0 0\nDP 1000 1000\nEN\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Main, GetGeometryHead() + "Include Sub.geo\n" + Wafer() + FullDetector("Scintillator") + "D.EnergyLossMap Loss.dat\n")) && Passed;

  auto Scan = [&]() {
    DisableDefaultStreams();
    bool Result = Geometry.ScanSetupFile(Main, false, false, false);
    EnableDefaultStreams();
    return Result;
  };
  auto Reload = [&]() {
    bool Result = false;
    BeginCaptureOutput();
    Result = Geometry.RequiresReload();
    EndCaptureOutput();
    return Result;
  };

  Passed = EvaluateTrue("ScanSetupFile()", "hash", "The file is accepted", Scan()) && Passed;
  long Hash = Geometry.GetHash();
  Passed = EvaluateFalse("GetHash()", "scanned", "The hash of a scanned geometry is not 0", Hash == 0) && Passed;
  Passed = EvaluateFalse("RequiresReload()", "unchanged", "Unchanged files need no reload", Reload()) && Passed;

  // The same content in a second scan:
  Passed = EvaluateTrue("ScanSetupFile()", "hash, repeated", "The file is accepted again", Scan()) && Passed;
  Passed = EvaluateTrue("GetHash()", "repeated", "The hash is reproducible", Geometry.GetHash() == Hash) && Passed;

  // A change of an included file:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Sub, Marker("A") + "// Changed\n")) && Passed;
  Passed = EvaluateTrue("RequiresReload()", "included file changed", "A change in an included file needs a reload", Reload()) && Passed;
  Passed = EvaluateTrue("GetHash()", "included file changed", "The stored hash does not change without a scan", Geometry.GetHash() == Hash) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "included file changed", "The file is accepted", Scan()) && Passed;
  Passed = EvaluateFalse("GetHash()", "included file changed", "The hash changes", Geometry.GetHash() == Hash) && Passed;
  Passed = EvaluateFalse("RequiresReload()", "after reload", "After a scan no reload is needed", Reload()) && Passed;

  // A change of the main file:
  long Hash2 = Geometry.GetHash();
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Main, GetGeometryHead() + "Include Sub.geo\n" + Wafer() + FullDetector("Scintillator") + "D.EnergyLossMap Loss.dat\n// Changed\n")) && Passed;
  Passed = EvaluateTrue("RequiresReload()", "main file changed", "A change in the main file needs a reload", Reload()) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "main file changed", "The file is accepted", Scan()) && Passed;
  Passed = EvaluateFalse("GetHash()", "main file changed", "The hash changes", Geometry.GetHash() == Hash2) && Passed;

  // Defect GEO-A-31: Files read by detectors and triggers are not part of the reload check and the hash
  long Hash3 = Geometry.GetHash();
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Loss, "IP LIN\nDP 0 0\nDP 1000 500\nEN\n")) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("RequiresReload()", "energy loss map changed", "A change in the energy loss map needs a reload", Reload())) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "energy loss map changed", "The file is accepted", Scan()) && Passed;
  // Defect GEO-A-31: Files read by detectors and triggers are not part of the reload check and the hash
  Passed = ExpectedToFail(EvaluateFalse("GetHash()", "energy loss map changed", "The hash changes", Geometry.GetHash() == Hash3)) && Passed;

  // A file which disappears:
  RemoveTemporaryFile(Sub);
  Passed = EvaluateTrue("RequiresReload()", "included file removed", "A removed file needs a reload", Reload()) && Passed;

  // An order of includes is part of the hash:
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/A.geo", Marker("InA"))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Dir + "/B.geo", Marker("InB", 3.0))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Main, GetGeometryHead() + "Include A.geo\nInclude B.geo\n")) && Passed;
  Scan();
  long OrderAB = Geometry.GetHash();
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Main, GetGeometryHead() + "Include B.geo\nInclude A.geo\n")) && Passed;
  Scan();
  Passed = EvaluateFalse("GetHash()", "order of includes", "Also the main file differs", Geometry.GetHash() == OrderAB) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the index, name, and list getters
bool UTDGeometry::TestGetters()
{
  bool Passed = true;
  MDGeometry Geometry;

  MString Text = GetGeometryHead() + "Orientation O\nO.Position 0 0 0\nShape BRIK S\nS.Parameters 1 1 1\nVector V\n" + Wafer() + FullDetector("Strip2D") + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector D 1\n";
  Passed = EvaluateTrue("ScanSetupFile()", "getters", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;

  // Volumes: world first, then in the order of the file
  Passed = Evaluate("GetNVolumes()", "two volumes", "Two volumes exist", Geometry.GetNVolumes(), 2U) && Passed;
  Passed = EvaluateTrue("GetVolumeAt()", "0", "The world is the first", Geometry.GetVolumeAt(0)->GetName() == "World") && Passed;
  Passed = EvaluateTrue("GetVolumeAt()", "1", "The wafer is the last", Geometry.GetVolumeAt(1)->GetName() == "Wafer") && Passed;
  Passed = Evaluate("GetVolumeIndex()", "Wafer", "The wafer has the index 1", Geometry.GetVolumeIndex("Wafer"), 1U) && Passed;
  Passed = Evaluate("GetVolumeIndex()", "unknown", "An unknown name has no index", Geometry.GetVolumeIndex("Unknown"), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("GetVolume()", "unknown", "An unknown name returns nullptr", Geometry.GetVolume("Unknown") == nullptr) && Passed;
  Passed = EvaluateTrue("GetVolume()", "wrong case", "The search is case sensitive", Geometry.GetVolume("WAFER") == nullptr) && Passed;
  Passed = EvaluateSize("GetVolumeList()", "two volumes", "The list has two entries", Geometry.GetVolumeList().size(), 2) && Passed;
  DisableDefaultStreams();
  MDVolume* OutVolume = Geometry.GetVolumeAt(2);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetVolumeAt()", "n", "The index after the end returns nullptr", OutVolume == nullptr) && Passed;

  // Materials:
  unsigned int NMaterials = Geometry.GetNMaterials();
  Passed = EvaluateTrue("GetNMaterials()", "file", "The material file has been read", NMaterials > 0) && Passed;
  Passed = EvaluateTrue("GetMaterialAt()", "index of Silicon", "The index and the name match", Geometry.GetMaterialAt(Geometry.GetMaterialIndex("Silicon")) == Geometry.GetMaterial("Silicon")) && Passed;
  Passed = Evaluate("GetMaterialIndex()", "unknown", "An unknown name has no index", Geometry.GetMaterialIndex("Unknown"), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "unknown", "An unknown name returns nullptr", Geometry.GetMaterial("Unknown") == nullptr) && Passed;
  Passed = EvaluateSize("GetMaterialList()", "file", "The list has all materials", Geometry.GetMaterialList().size(), NMaterials) && Passed;
  DisableDefaultStreams();
  MDMaterial* OutMaterial = Geometry.GetMaterialAt(NMaterials);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetMaterialAt()", "n", "The index after the end returns nullptr", OutMaterial == nullptr) && Passed;

  // Detectors:
  Passed = Evaluate("GetNDetectors()", "one", "One detector exists", Geometry.GetNDetectors(), 1U) && Passed;
  Passed = EvaluateTrue("GetDetectorAt()", "0", "The detector is at index 0", Geometry.GetDetectorAt(0)->GetName() == "D") && Passed;
  Passed = Evaluate("GetDetectorIndex()", "D", "The detector has the index 0", Geometry.GetDetectorIndex("D"), 0U) && Passed;
  Passed = Evaluate("GetDetectorIndex()", "unknown", "An unknown name has no index", Geometry.GetDetectorIndex("Unknown"), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("GetDetector()", "unknown", "An unknown name returns nullptr", Geometry.GetDetector(MString("Unknown")) == nullptr) && Passed;
  Passed = EvaluateSize("GetDetectorList()", "one", "The list has one entry", Geometry.GetDetectorList().size(), 1) && Passed;
  DisableDefaultStreams();
  MDDetector* OutDetector = Geometry.GetDetectorAt(1);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetDetectorAt()", "n", "The index after the end returns nullptr", OutDetector == nullptr) && Passed;

  // Shapes: named shape, world shape, wafer shape
  Passed = Evaluate("GetNShapes()", "three", "Three shapes exist", Geometry.GetNShapes(), 3U) && Passed;
  Passed = EvaluateTrue("GetShapeAt()", "index of S", "The index and the name match", Geometry.GetShapeAt(Geometry.GetShapeIndex("S")) == Geometry.GetShape("S")) && Passed;
  Passed = Evaluate("GetShapeIndex()", "unknown", "An unknown name has no index", Geometry.GetShapeIndex("Unknown"), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("GetShape()", "unknown", "An unknown name returns nullptr", Geometry.GetShape("Unknown") == nullptr) && Passed;
  Passed = EvaluateSize("GetShapeList()", "three", "The list has three entries", Geometry.GetShapeList().size(), 3) && Passed;
  DisableDefaultStreams();
  MDShape* OutShape = Geometry.GetShapeAt(3);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetShapeAt()", "n", "The index after the end returns nullptr", OutShape == nullptr) && Passed;

  // Orientations:
  Passed = Evaluate("GetNOrientations()", "one", "One orientation exists", Geometry.GetNOrientations(), 1U) && Passed;
  Passed = EvaluateTrue("GetOrientationAt()", "0", "The orientation is at index 0", Geometry.GetOrientationAt(0) == Geometry.GetOrientation("O")) && Passed;
  Passed = Evaluate("GetOrientationIndex()", "O", "The orientation has the index 0", Geometry.GetOrientationIndex("O"), 0U) && Passed;
  Passed = Evaluate("GetOrientationIndex()", "unknown", "An unknown name has no index", Geometry.GetOrientationIndex("Unknown"), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("GetOrientation()", "unknown", "An unknown name returns nullptr", Geometry.GetOrientation("Unknown") == nullptr) && Passed;
  Passed = EvaluateSize("GetOrientationList()", "one", "The list has one entry", Geometry.GetOrientationList().size(), 1) && Passed;
  DisableDefaultStreams();
  MDOrientation* OutOrientation = Geometry.GetOrientationAt(1);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetOrientationAt()", "n", "The index after the end returns nullptr", OutOrientation == nullptr) && Passed;

  // Triggers:
  Passed = Evaluate("GetNTriggers()", "one", "One trigger exists", Geometry.GetNTriggers(), 1U) && Passed;
  Passed = EvaluateTrue("GetTriggerAt()", "0", "The trigger is at index 0", Geometry.GetTriggerAt(0) == Geometry.GetTrigger("T")) && Passed;
  Passed = Evaluate("GetTriggerIndex()", "T", "The trigger has the index 0", Geometry.GetTriggerIndex("T"), 0U) && Passed;
  Passed = Evaluate("GetTriggerIndex()", "unknown", "An unknown name has no index", Geometry.GetTriggerIndex("Unknown"), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("GetTrigger()", "unknown", "An unknown name returns nullptr", Geometry.GetTrigger("Unknown") == nullptr) && Passed;
  Passed = EvaluateSize("GetTriggerList()", "one", "The list has one entry", Geometry.GetTriggerList().size(), 1) && Passed;
  DisableDefaultStreams();
  MDTrigger* OutTrigger = Geometry.GetTriggerAt(1);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetTriggerAt()", "n", "The index after the end returns nullptr", OutTrigger == nullptr) && Passed;
  Passed = EvaluateTrue("GetTriggerUnit()", "scanned", "The trigger unit validates the geometry", Geometry.GetTriggerUnit()->Validate()) && Passed;

  // Vectors:
  Passed = Evaluate("GetNVectors()", "one", "One vector exists", Geometry.GetNVectors(), 1U) && Passed;
  Passed = EvaluateTrue("GetVectorAt()", "0", "The vector is at index 0", Geometry.GetVectorAt(0) == Geometry.GetVector("V")) && Passed;
  Passed = Evaluate("GetVectorIndex()", "V", "The vector has the index 0", Geometry.GetVectorIndex("V"), 0U) && Passed;
  Passed = Evaluate("GetVectorIndex()", "unknown", "An unknown name has no index", Geometry.GetVectorIndex("Unknown"), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("GetVector()", "unknown", "An unknown name returns nullptr", Geometry.GetVector("Unknown") == nullptr) && Passed;
  DisableDefaultStreams();
  MDVector* OutVector = Geometry.GetVectorAt(1);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetVectorAt()", "n", "The index after the end returns nullptr", OutVector == nullptr) && Passed;

  // Start volume name:
  Geometry.SetStartVolumeName("Something");
  Passed = EvaluateTrue("SetStartVolumeName()", "Something", "The start volume name is stored", Geometry.GetStartVolumeName() == "Something") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the add functions
bool UTDGeometry::TestAdders()
{
  bool Passed = true;

  // The geometry owns everything which has been added - the destructor deletes them:
  MDGeometry* Owner = new MDGeometry();
  Owner->AddVolume(new MDVolume("V"));
  Owner->AddMaterial(new MDMaterial("M"));
  Owner->AddOrientation(new MDOrientation("O"));
  Owner->AddVector(new MDVector("W"));
  Owner->AddShape(new MDShapeBRIK("S"));
  Owner->AddTrigger(new MDTriggerBasic("T"));
  Owner->AddDetector(new MDStrip2D("D"));
  Passed = EvaluateTrue("AddDetector()", "one each", "The detector is stored", Owner->GetDetector(MString("D")) != nullptr && Owner->GetNDetectors() == 1) && Passed;
  Passed = EvaluateTrue("AddVolume(), AddMaterial(), AddOrientation(), AddVector(), AddShape(), AddTrigger()", "one each", "One object of each kind is counted", Owner->GetNVolumes() == 1 && Owner->GetNMaterials() == 1 && Owner->GetNOrientations() == 1 && Owner->GetNVectors() == 1 && Owner->GetNShapes() == 1 && Owner->GetNTriggers() == 1) && Passed;
  Passed = EvaluateTrue("AddVolume(), AddMaterial(), AddOrientation(), AddVector(), AddShape(), AddTrigger()", "one each", "The objects are found by their names", Owner->GetVolume("V") != nullptr && Owner->GetMaterial("M") != nullptr && Owner->GetOrientation("O") != nullptr && Owner->GetVector("W") != nullptr && Owner->GetShape("S") != nullptr && Owner->GetTrigger("T") != nullptr) && Passed;
  // The destructor deletes them, which must not crash
  delete Owner;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the includes interface
bool UTDGeometry::TestIncludeInterface()
{
  bool Passed = true;
  MDGeometry Geometry;

  const MString File = GetTemporaryFileName("IncludeInterface.geo");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(File, "// Text\nSecond line\n")) && Passed;

  // Reading a file: two lines and one empty line at the end
  UTDGeometryAccess Access;
  list<MDDebugInfo> Content;
  Passed = EvaluateTrue("AddFile()", "file with two lines", "The file is read", Access.AddFile(File, Content)) && Passed;
  Passed = EvaluateSize("AddFile()", "file with two lines", "The two lines and one empty line at the end are stored", Content.size(), 3) && Passed;
  if (Content.size() == 3) {
    Passed = EvaluateTrue("AddFile()", "second line", "The text of the line is stored", next(Content.begin(), 1)->GetText() == "Second line") && Passed;
    Passed = EvaluateTrue("AddFile()", "second line", "The file name is stored", next(Content.begin(), 1)->GetFileName() == File) && Passed;
  }
  Passed = EvaluateTrue("AddFile()", "file with two lines", "The file is in the list of includes", Access.IsIncluded(File)) && Passed;
  Passed = EvaluateTrue("AddFile()", "the same file again", "A second read returns true", Access.AddFile(File, Content)) && Passed;
  Passed = EvaluateSize("AddFile()", "the same file again", "A second read returns nothing", Content.size(), 0) && Passed;
  DisableDefaultStreams();
  bool MissingFile = Access.AddFile(GetTemporaryFileName("DoesNotExist.geo"), Content);
  EnableDefaultStreams();
  Passed = EvaluateFalse("AddFile()", "missing file", "A missing file is rejected", MissingFile) && Passed;

  Passed = EvaluateFalse("IsIncluded()", "not added", "The file is not included", Geometry.IsIncluded(File)) && Passed;
  Geometry.AddInclude(File);
  Passed = EvaluateTrue("IsIncluded()", "added", "The file is included", Geometry.IsIncluded(File)) && Passed;
  Passed = Evaluate("GetNIncludes()", "added", "One file is included", Geometry.GetNIncludes(), 1) && Passed;
  Geometry.AddInclude(File);
  Passed = Evaluate("GetNIncludes()", "added twice", "The file only counts once", Geometry.GetNIncludes(), 1) && Passed;
  Passed = EvaluateFalse("GetHash()", "added", "The hash is not 0", Geometry.GetHash() == 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test ValidName, MakeValidName, and NameExists
bool UTDGeometry::TestNameFunctions()
{
  bool Passed = true;
  MDGeometry Geometry;

  Passed = EvaluateTrue("ValidName()", "alphanumeric", "Letters, digits, and underscores are valid", Geometry.ValidName("Abc_123")) && Passed;
  Passed = EvaluateTrue("ValidName()", "single letter", "A single letter is valid", Geometry.ValidName("A")) && Passed;
  DisableDefaultStreams();
  bool Minus = Geometry.ValidName("A-B");
  bool Dot = Geometry.ValidName("A.B");
  bool Space = Geometry.ValidName("A B");
  EnableDefaultStreams();
  Passed = EvaluateFalse("ValidName()", "minus", "A minus sign is not valid", Minus) && Passed;
  Passed = EvaluateFalse("ValidName()", "dot", "A dot is not valid", Dot) && Passed;
  Passed = EvaluateFalse("ValidName()", "space", "A space is not valid", Space) && Passed;

  Passed = EvaluateTrue("MakeValidName()", "valid name", "A valid name stays the same", MDGeometry::MakeValidName("Abc_123") == "Abc_123") && Passed;
  Passed = EvaluateTrue("MakeValidName()", "bad characters", "Dots and spaces are removed", MDGeometry::MakeValidName("A.B C") == "ABC") && Passed;
  Passed = EvaluateTrue("MakeValidName()", "empty", "An empty name stays empty", MDGeometry::MakeValidName("") == "") && Passed;
  Passed = EvaluateTrue("MakeValidName()", "only bad characters", "A name out of only bad characters gets empty", MDGeometry::MakeValidName(".. /") == "") && Passed;

  // Defect GEO-A-32: MakeValidName keeps a minus sign, which ValidName rejects
  DisableDefaultStreams();
  bool MadeValid = Geometry.ValidName(MDGeometry::MakeValidName("A-B"));
  EnableDefaultStreams();
  Passed = ExpectedToFail(EvaluateTrue("MakeValidName()", "A-B", "The result of MakeValidName is a valid name", MadeValid)) && Passed;
  // Defect GEO-A-33: The empty name is accepted as valid
  Passed = ExpectedToFail(EvaluateFalse("ValidName()", "empty", "The empty name is not valid", Geometry.ValidName(""))) && Passed;

  // NameExists:
  UTDGeometryAccess Filled;
  Passed = EvaluateTrue("ScanSetupFile()", "NameExists", "The file is accepted", ScanGeometry(Filled, GetGeometryHead() + Marker("A"))) && Passed;
  Passed = EvaluateFalse("NameExists()", "new name", "An unknown name does not exist", Filled.NameExists("Unknown")) && Passed;
  Passed = Evaluate("GetNVolumes()", "after a negative NameExists", "The geometry is untouched", Filled.GetNVolumes(), 2U) && Passed;

  DisableDefaultStreams();
  bool ExistsInOtherCase = Filled.NameExists("a");
  bool Exists = Filled.NameExists("A");
  EnableDefaultStreams();
  // Defect GEO-A-69: The check for existing names is case sensitive, while its comment says that names are case insensitive
  Passed = ExpectedToFail(EvaluateTrue("NameExists()", "other case", "The check is case insensitive", ExistsInOtherCase)) && Passed;
  Passed = EvaluateTrue("NameExists()", "existing name", "An existing name is found", Exists) && Passed;
  // Defect GEO-A-34: The public query NameExists resets the whole geometry if the name exists
  Passed = ExpectedToFail(Evaluate("GetNVolumes()", "after a positive NameExists", "The query leaves the geometry untouched", Filled.GetNVolumes(), 2U)) && Passed;

  // Typo prints the message and resets:
  MDGeometry Typos;
  Passed = EvaluateTrue("ScanSetupFile()", "Typo", "The file is accepted", ScanGeometry(Typos, GetGeometryHead())) && Passed;
  BeginCaptureOutput();
  Typos.Typo("My special message");
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("Typo()", "message", "The message is shown", Output.Contains("My special message")) && Passed;
  Passed = EvaluateFalse("Typo()", "message", "The geometry is reset", Typos.IsScanned()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the protected text helpers
bool UTDGeometry::TestTextHelpers()
{
  bool Passed = true;
  UTDGeometryAccess Geometry;

  // Replacement of whole words:
  MString Text = "A AB BA A_ _A A.B B.A {A+1} A";
  Geometry.ReplaceWholeWords(Text, "A", "XY");
  Passed = EvaluateTrue("ReplaceWholeWords()", "several cases", "Only the whole words are replaced", Text == "XY AB BA A_ _A XY.B B.XY {XY+1} XY") && Passed;
  Text = "";
  Geometry.ReplaceWholeWords(Text, "A", "B");
  Passed = EvaluateTrue("ReplaceWholeWords()", "empty text", "An empty text stays empty", Text == "") && Passed;
  Text = "A A";
  Geometry.ReplaceWholeWords(Text, "A", "A A");
  Passed = EvaluateTrue("ReplaceWholeWords()", "new word contains the old one", "The new word is not replaced again", Text == "A A A A") && Passed;

  // The check for constants:
  Passed = EvaluateTrue("ContainsReplacableConstant()", "whole word", "A whole word is found", Geometry.ContainsReplacableConstant("{ A + 1 }", "A")) && Passed;
  Passed = EvaluateFalse("ContainsReplacableConstant()", "part of a word", "A part of a word is not found", Geometry.ContainsReplacableConstant("{ AB + 1 }", "A")) && Passed;
  Passed = EvaluateFalse("ContainsReplacableConstant()", "not there", "A word which is not there is not found", Geometry.ContainsReplacableConstant("{ B + 1 }", "A")) && Passed;
  Passed = EvaluateFalse("ContainsReplacableConstant()", "empty text", "An empty text contains nothing", Geometry.ContainsReplacableConstant("", "A")) && Passed;
  // Defect GEO-A-09: The search for a constant in a maths expression stops at the first partial match such as the n in sin
  Passed = ExpectedToFail(EvaluateTrue("ContainsReplacableConstant()", "part of a word, then the whole word", "The whole word after the partial match is found", Geometry.ContainsReplacableConstant("{ AB + A }", "A"))) && Passed;

  // Wrapping for the FORTRAN line length of 72 characters:
  Passed = EvaluateTrue("WFS()", "short line", "A short line stays", Geometry.WFS("Short\n") == "Short\n") && Passed;
  MString Long;
  for (unsigned int i = 0; i < 100; ++i) {
    Long += "x";
  }
  MString Wrapped = Geometry.WFS(Long + "\n");
  Passed = EvaluateTrue("WFS()", "long line", "A long line is broken into continuation lines", Wrapped.Contains("\n     &")) && Passed;
  vector<MString> Lines = Wrapped.Tokenize("\n");
  bool AllShort = true;
  unsigned int Characters = 0;
  for (const MString& Line: Lines) {
    if (Line.Length() > 72) {
      AllShort = false;
    }
    Characters += Line.Length();
    if (Line.BeginsWith("     &") == true) {
      Characters -= 6;
    }
  }
  Passed = EvaluateTrue("WFS()", "long line", "No line is longer than 72 characters", AllShort) && Passed;
  Passed = Evaluate("WFS()", "long line", "No character is lost", Characters, 100U) && Passed;
  MString Comment = "*";
  for (unsigned int i = 0; i < 100; ++i) {
    Comment += "x";
  }
  Comment += "\n";
  Passed = EvaluateTrue("WFS()", "long comment", "A comment line is not wrapped", Geometry.WFS(Comment) == Comment) && Passed;
  // Defect GEO-A-35: The check for text beyond the limit is always true, thus a line with only trailing blanks is broken
  MString Blanks;
  for (unsigned int i = 0; i < 71; ++i) {
    Blanks += "x";
  }
  for (unsigned int i = 0; i < 20; ++i) {
    Blanks += " ";
  }
  Blanks += "\n";
  Passed = ExpectedToFail(EvaluateTrue("WFS()", "only blanks after the limit", "A line with only blanks after the limit is not broken", Geometry.WFS(Blanks) == Blanks)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test ToString and DumpInformation
bool UTDGeometry::TestDescriptions()
{
  bool Passed = true;
  MDGeometry Geometry;

  MString Text = GetGeometryHead() + "Orientation O\nO.Position 0 0 0\nShape BRIK NamedShape\nNamedShape.Parameters 1 1 1\n" + Wafer() + FullDetector("Strip2D") + "Trigger Trig\nTrig.Veto false\nTrig.TriggerByChannel true\nTrig.Detector D 1\n";
  Passed = EvaluateTrue("ScanSetupFile()", "description", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;

  MString Description = Geometry.ToString();
  for (const MString Token: {"UnitTest", "version: 1.0", "NamedShape", "Description of individually defined orientations", "Wafer", "Description of volume-tree", "Aluminium", "Description of detectors", "Description of triggers", "Trig"}) {
    Passed = EvaluateTrue("ToString()", Token, "The description contains the token", Description.Contains(Token)) && Passed;
  }
  BeginCaptureOutput();
  Geometry.DumpInformation();
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("DumpInformation()", "scanned", "The dump shows the description", Output.Contains(Description)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test CalculateMasses
bool UTDGeometry::TestMasses()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Two materials of known density by hand: box of 2 x 2 x 2 = 8 cm3 at 2 g/cm3 with a daughter of 1 x 1 x 1 = 1 cm3 at 5 g/cm3 inside
  MString Text = "Name N\nSurroundingSphere 1 0 0 0 1\n";
  Text += "Material Va\nVa.Density 1E-12\nVa.Component H 1\nMaterial Light\nLight.Density 2\nLight.Component Si 1\nMaterial Heavy\nHeavy.Density 5\nHeavy.Component Fe 1\n";
  Text += "Volume World\nWorld.Material Va\nWorld.Shape BRIK 10 10 10\nWorld.Mother 0\n";
  Text += "Volume Outer\nOuter.Material Light\nOuter.Shape BRIK 1 1 1\nOuter.Mother World\n";
  Text += "Volume Inner\nInner.Material Heavy\nInner.Shape BRIK 0.5 0.5 0.5\nInner.Mother Outer\n";
  Passed = EvaluateTrue("ScanSetupFile()", "masses", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  BeginCaptureOutput();
  Geometry.CalculateMasses();
  MString Output = EndCaptureOutput();

  // Light: (8 - 1) * 2 = 14 g, Heavy: 1 * 5 = 5 g, the world in vacuum: about 8E6 * 1E-12 g = 0.000 g
  Passed = EvaluateTrue("CalculateMasses()", "Light", "The mass of the outer volume does not contain the daughter", Output.Contains("Light :        14.000 g")) && Passed;
  Passed = EvaluateTrue("CalculateMasses()", "Heavy", "The mass of the inner volume is shown", Output.Contains("Heavy :         5.000 g")) && Passed;
  Passed = EvaluateTrue("CalculateMasses()", "Total", "The total mass is the sum", Output.Contains("Total :        19.000 g")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test CheckOverlaps
bool UTDGeometry::TestOverlaps()
{
  bool Passed = true;

  // No overlap:
  MDGeometry Clean;
  Passed = EvaluateTrue("ScanSetupFile()", "no overlap", "The file is accepted", ScanGeometry(Clean, GetGeometryHead() + Marker("A", -5) + Marker("B", 5))) && Passed;
  ostringstream Diagnostics;
  Int_t ErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kError;
  bool Result = false;
  BeginCaptureOutput();
  Result = Clean.CheckOverlaps(Diagnostics);
  EndCaptureOutput();
  gErrorIgnoreLevel = ErrorLevel;
  Passed = EvaluateTrue("CheckOverlaps(Diagnostics)", "no overlap", "No overlap is found", Result) && Passed;
  Passed = EvaluateTrue("CheckOverlaps(Diagnostics)", "no overlap", "The diagnostics say so", MString(Diagnostics.str().c_str()).Contains("No extrusions or overlaps")) && Passed;

  // An overlap of two boxes, each with half length 1, with an offset on all axes - the corner of one box is inside of the other one:
  MDGeometry Overlap;
  Passed = EvaluateTrue("ScanSetupFile()", "overlap", "The file is accepted", ScanGeometry(Overlap, GetGeometryHead() + Marker("A", 0) + Marker("B", 1) + "B.Position 1 0.5 0.5\n")) && Passed;
  ostringstream OverlapDiagnostics;
  gErrorIgnoreLevel = kError;
  BeginCaptureOutput();
  Result = Overlap.CheckOverlaps(OverlapDiagnostics);
  EndCaptureOutput();
  gErrorIgnoreLevel = ErrorLevel;
  MString OverlapText = OverlapDiagnostics.str().c_str();
  Passed = EvaluateFalse("CheckOverlaps(Diagnostics)", "overlap", "The overlap is found", Result) && Passed;
  Passed = EvaluateTrue("CheckOverlaps(Diagnostics)", "overlap", "The diagnostics name the overlap", OverlapText.Contains("Overlap: ")) && Passed;

  // An extrusion: half length 1 at a distance of 100 from the center of a world of half length 100
  MDGeometry Extrusion;
  Passed = EvaluateTrue("ScanSetupFile()", "extrusion", "The file is accepted", ScanGeometry(Extrusion, GetGeometryHead() + Marker("A", 100))) && Passed;
  ostringstream ExtrusionDiagnostics;
  gErrorIgnoreLevel = kError;
  BeginCaptureOutput();
  Result = Extrusion.CheckOverlaps(ExtrusionDiagnostics);
  EndCaptureOutput();
  gErrorIgnoreLevel = ErrorLevel;
  Passed = EvaluateFalse("CheckOverlaps(Diagnostics)", "extrusion", "The extrusion is found", Result) && Passed;
  Passed = EvaluateTrue("CheckOverlaps(Diagnostics)", "extrusion", "The diagnostics name the extrusion", MString(ExtrusionDiagnostics.str().c_str()).Contains("Extrusion: ")) && Passed;

  // The version with output on the screen:
  gErrorIgnoreLevel = kError;
  // The diagnostics have no end of line of their own, which flushes the stream:
  BeginCaptureOutput();
  Result = Overlap.CheckOverlaps();
  mout<<endl;
  MString Output = EndCaptureOutput();
  gErrorIgnoreLevel = ErrorLevel;
  Passed = EvaluateFalse("CheckOverlaps()", "overlap", "The overlap is found", Result) && Passed;
  Passed = EvaluateTrue("CheckOverlaps()", "overlap", "The diagnostics are on the screen", Output.Contains("Overlap: ")) && Passed;

  // A second geometry object alive, and the check on the first one
  MDGeometry FirstObject;
  MString Small = GetGeometryHead() + Marker("A", 0) + Marker("B", 1) + "B.Position 1 0.5 0.5\n";
  Passed = EvaluateTrue("ScanSetupFile()", "first of two objects", "The file is accepted", ScanGeometry(FirstObject, Small, "OverlapFirst.geo.setup")) && Passed;
  MDGeometry SecondObject;
  Passed = EvaluateTrue("ScanSetupFile()", "second of two objects", "The file is accepted", ScanGeometry(SecondObject, GetGeometryHead() + Marker("C", 0), "OverlapSecond.geo.setup")) && Passed;
  // Defect GEO-A-48: The ROOT volumes are created in the global gGeoManager of the newest object, thus the check of an older object crashes
  ostringstream FirstDiagnostics;
  Passed = ExpectedToCrash(EvaluateFalse("CheckOverlaps()", "second object exists", "The overlap of the first object is found with a second object alive", FirstObject.CheckOverlaps(FirstDiagnostics))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test DrawGeometry and StoreViewParameters
bool UTDGeometry::TestDrawing()
{
  bool Passed = true;

  // Nothing visible cannot be drawn:
  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "no visible volume", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead())) && Passed;
  DisableDefaultStreams();
  bool Result = Geometry.DrawGeometry(nullptr, false, "");
  EnableDefaultStreams();
  Passed = EvaluateFalse("DrawGeometry()", "no visible volume", "There is nothing to draw", Result) && Passed;

  // In batch mode a visible volume can be drawn twice:
  MString Text = GetGeometryHead() + Marker("A") + "A.Visibility 1\n";
  MDGeometry Visible;
  Passed = EvaluateTrue("ScanSetupFile()", "visible volume", "The file is accepted", ScanGeometry(Visible, Text)) && Passed;
  // Defect GEO-A-68: The drawing always opens the OpenGL viewer, which crashes in batch mode
  Passed = ExpectedToCrash(EvaluateTrue("DrawGeometry()", "visible volume", "The geometry can be drawn in batch mode", Visible.DrawGeometry(nullptr, false, ""))) && Passed;
  Visible.StoreViewParameters();
  // Defect GEO-A-68: The drawing always opens the OpenGL viewer, which crashes in batch mode
  Passed = ExpectedToCrash(EvaluateTrue("DrawGeometry()", "visible volume, twice", "The geometry can be drawn a second time in batch mode", Visible.DrawGeometry(nullptr, true, ""))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test GetListOfUnusedMaterials
bool UTDGeometry::TestUnusedMaterials()
{
  bool Passed = true;
  MDGeometry Geometry;

  MString Text = "Name N\nSurroundingSphere 1 0 0 0 1\nMaterial Used\nUsed.Density 1\nUsed.Component H 1\nMaterial Unused\nUnused.Density 1\nUnused.Component H 1\nUnused.Copy UnusedToo\nVolume W\nW.Material Used\nW.Shape BRIK 1 1 1\nW.Mother 0\n";
  Passed = EvaluateTrue("ScanSetupFile()", "unused materials", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  vector<MDMaterial*> Unused = Geometry.GetListOfUnusedMaterials();
  Passed = EvaluateSize("GetListOfUnusedMaterials()", "two unused", "Two materials are unused", Unused.size(), 2) && Passed;
  if (Unused.size() == 2) {
    Passed = EvaluateTrue("GetListOfUnusedMaterials()", "first", "The first one is the unused one", Unused[0] == Geometry.GetMaterial("Unused")) && Passed;
    Passed = EvaluateTrue("GetListOfUnusedMaterials()", "second", "The second one is the copy", Unused[1] == Geometry.GetMaterial("UnusedToo")) && Passed;
  }

  // An unscanned geometry has no material at all:
  MDGeometry Empty;
  Passed = EvaluateSize("GetListOfUnusedMaterials()", "not scanned", "The list is empty", Empty.GetListOfUnusedMaterials().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume sequence and position lookups
bool UTDGeometry::TestVolumeSequences()
{
  bool Passed = true;
  MDGeometry Geometry;

  // A wafer with half lengths 5, 5, 0.5 at z = 2 in the world, rotated by 90 deg around z, and a passive box at x = 30:
  MString Text = GetGeometryHead() + Wafer("Wafer", 2.0) + FullDetector("Strip2D") + Marker("Passive", 30.0);
  Passed = EvaluateTrue("ScanSetupFile()", "volume sequence", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;

  MDVolumeSequence Sequence = Geometry.GetVolumeSequence(MVector(1, 1, 2));
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The deepest volume is the wafer", Sequence.GetDeepestVolume() == Geometry.GetVolume("Wafer")) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The detector is found", Sequence.GetDetector() == Geometry.GetDetector("D")) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "in the wafer", "The sensitive volume is found", Sequence.GetSensitiveVolume() == Geometry.GetVolume("Wafer")) && Passed;
  Passed = EvaluateVectorNear("GetVolumeSequence()", "in the wafer", "The position in the detector is relative to the wafer", Sequence.GetPositionInDetector(), MVector(1, 1, 0), 1e-12) && Passed;

  MDVolumeSequence* Pointer = Geometry.GetVolumeSequencePointer(MVector(30, 0, 0));
  Passed = EvaluateTrue("GetVolumeSequencePointer()", "passive", "The passive volume is found", Pointer->GetDeepestVolume() == Geometry.GetVolume("Passive")) && Passed;
  Passed = EvaluateTrue("GetVolumeSequencePointer()", "passive", "There is no detector", Pointer->GetDetector() == nullptr) && Passed;
  delete Pointer;

  Sequence = Geometry.GetVolumeSequence(MVector(50, 50, 50));
  Passed = EvaluateTrue("GetVolumeSequence()", "in the world", "The deepest volume is the world", Sequence.GetDeepestVolume() == Geometry.GetWorldVolume()) && Passed;
  Sequence = Geometry.GetVolumeSequence(MVector(500, 0, 0));
  Passed = EvaluateTrue("GetVolumeSequence()", "outside of the world", "There is no volume", Sequence.GetDeepestVolume() == nullptr) && Passed;

  Passed = EvaluateTrue("GetDetector(Position)", "in the wafer", "The detector is found", Geometry.GetDetector(MVector(1, 1, 2)) == Geometry.GetDetector("D")) && Passed;
  Passed = EvaluateTrue("GetDetector(Position)", "passive", "No detector is found", Geometry.GetDetector(MVector(30, 0, 0)) == nullptr) && Passed;
  Passed = Evaluate("GetDetectorType()", "in the wafer", "The type is the one of the detector", Geometry.GetDetectorType(MVector(1, 1, 2)), MDDetector::c_Strip2D) && Passed;
  Passed = Evaluate("GetDetectorType()", "passive", "The type is 0 without a detector", Geometry.GetDetectorType(MVector(30, 0, 0)), 0) && Passed;

  // The search within the tolerance: the wafer ends at z = 2.5, the default tolerance is 1E-6
  MVector Outside(0, 0, 2.5 + 5E-7);
  Sequence = Geometry.GetVolumeSequence(Outside);
  Passed = EvaluateTrue("GetVolumeSequence()", "outside, no force", "Without force no detector is searched", Sequence.GetDetector() == nullptr) && Passed;
  BeginCaptureOutput();
  Sequence = Geometry.GetVolumeSequence(Outside, true, false);
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("GetVolumeSequence()", "5E-7 cm outside, force detector", "The detector is found within the tolerance", Sequence.GetDetector() == Geometry.GetDetector("D")) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "5E-7 cm outside, force detector", "No warning is printed after a success", Output == "") && Passed;
  BeginCaptureOutput();
  Sequence = Geometry.GetVolumeSequence(Outside, false, true);
  Output = EndCaptureOutput();
  Passed = EvaluateTrue("GetVolumeSequence()", "5E-7 cm outside, force sensitive volume", "The sensitive volume is found within the tolerance", Sequence.GetSensitiveVolume() == Geometry.GetVolume("Wafer")) && Passed;

  // Just outside of the tolerance: nothing is found, and a warning is shown
  MVector Beyond(0, 0, 2.5 + 2E-6);
  BeginCaptureOutput();
  Sequence = Geometry.GetVolumeSequence(Beyond, true, false);
  Output = EndCaptureOutput();
  Passed = EvaluateTrue("GetVolumeSequence()", "2E-6 cm outside, force detector", "No detector is found outside of the tolerance", Sequence.GetDetector() == nullptr) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "2E-6 cm outside, force detector", "A warning is shown", Output.Contains("No suitable volume found")) && Passed;

  // All six directions of the search:
  vector<MVector> Directions = {MVector(5 + 5E-7, 0, 2), MVector(-5 - 5E-7, 0, 2), MVector(0, 5 + 5E-7, 2), MVector(0, -5 - 5E-7, 2), MVector(0, 0, 2.5 + 5E-7), MVector(0, 0, 1.5 - 5E-7)};
  for (unsigned int d = 0; d < Directions.size(); ++d) {
    MString Input = "direction "; Input += d;
    BeginCaptureOutput();
    Sequence = Geometry.GetVolumeSequence(Directions[d], true, false);
    EndCaptureOutput();
    Passed = EvaluateTrue("GetVolumeSequence()", Input, "The detector is found within the tolerance", Sequence.GetDetector() == Geometry.GetDetector("D")) && Passed;
  }

  // A larger tolerance from the file:
  Passed = EvaluateTrue("ScanSetupFile()", "DetectorSearchTolerance", "The file is accepted", ScanGeometry(Geometry, Text + "DetectorSearchTolerance 0.25\n")) && Passed;
  BeginCaptureOutput();
  Sequence = Geometry.GetVolumeSequence(MVector(0, 0, 2.7), true, false);
  EndCaptureOutput();
  Passed = EvaluateTrue("GetVolumeSequence()", "0.2 cm outside, tolerance 0.25", "The detector is found within the larger tolerance", Sequence.GetDetector() == Geometry.GetDetector("D")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test GetGlobalPosition
bool UTDGeometry::TestGlobalPosition()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Two copies of a wafer, at z = 5 and z = -5:
  MString Text = GetGeometryHead() + Wafer("WaferA", 5.0) + "WaferA.Copy WaferB\nWaferB.Position 0 0 -5\nWaferB.Mother World\n" + FullDetector("Strip2D", "D", "WaferA") + "D.Named N1\nN1.Assign 0 0 5\nD.Named N2\nN2.Assign 0 0 -5\n";
  Passed = EvaluateTrue("ScanSetupFile()", "global position", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  Passed = EvaluateVectorNear("GetGlobalPosition()", "N1", "A position in the first named detector moves by +5 in z", Geometry.GetGlobalPosition(MVector(1, 2, 0.25), "N1"), MVector(1, 2, 5.25), 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetGlobalPosition()", "N2", "A position in the second named detector moves by -5 in z", Geometry.GetGlobalPosition(MVector(1, 2, 0.25), "N2"), MVector(1, 2, -4.75), 1e-12) && Passed;

  DisableDefaultStreams();
  MVector Unknown = Geometry.GetGlobalPosition(MVector(1, 2, 0.25), "Unknown");
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetGlobalPosition()", "unknown name", "An unknown name returns g_VectorNotDefined", Unknown == g_VectorNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test GetRandomPositionInVolume
bool UTDGeometry::TestRandomPositions()
{
  bool Passed = true;
  MDGeometry Geometry;

  // A box with half length 2 around a daughter of half length 1, the volume fraction which is left is 7/8
  MString Text = GetGeometryHead() + "Volume Outer\nOuter.Material Aluminium\nOuter.Shape BRIK 2 2 2\nOuter.Position 10 0 0\nOuter.Mother World\nVolume Inner\nInner.Material Aluminium\nInner.Shape BRIK 1 1 1\nInner.Mother Outer\n";
  Passed = EvaluateTrue("ScanSetupFile()", "random position", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  const unsigned int N = 4000;
  unsigned int InOuterOnly = 0;
  double MeanX = 0;
  for (unsigned int n = 0; n < N; ++n) {
    MVector P = Geometry.GetRandomPositionInVolume("Outer");
    MDVolume* V = Geometry.GetVolumeSequence(P).GetDeepestVolume();
    if (V == Geometry.GetVolume("Outer")) {
      ++InOuterOnly;
    }
    MeanX += P.X();
  }
  MeanX /= N;
  // Defect GEO-A-47: The positions are also in the daughter volumes, against the description of the function, which moves volume sources of cosima
  Passed = ExpectedToFail(Evaluate("GetRandomPositionInVolume()", "mother with daughter", "All positions are in the mother and not in the daughter", InOuterOnly, N)) && Passed;
  // Uniform in a box of half length 2 without the center: sigma(x) < 2, mean is the center of the box at 10 with a tolerance of 5 sigma / sqrt(N)
  Passed = EvaluateNear("GetRandomPositionInVolume()", "mean x", "The mean x is the center of the box", MeanX, 10.0, 5.0*1.4/sqrt(N)) && Passed;

  // Two copies of a volume: half of the samples in each of the two copies, tolerance 5 sigma of the binomial distribution
  Text = GetGeometryHead() + "Volume T\nT.Material Aluminium\nT.Shape BRIK 1 1 1\nT.Copy A\nA.Position 10 0 0\nA.Mother World\nT.Copy B\nB.Position -10 0 0\nB.Mother World\n";
  Passed = EvaluateTrue("ScanSetupFile()", "copies", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  unsigned int InA = 0;
  unsigned int InB = 0;
  for (unsigned int n = 0; n < N; ++n) {
    MVector P = Geometry.GetRandomPositionInVolume("A");
    MDVolume* V = Geometry.GetVolumeSequence(P).GetDeepestVolume();
    if (V == Geometry.GetVolume("A")) {
      ++InA;
    } else if (V == Geometry.GetVolume("B")) {
      ++InB;
    }
  }
  Passed = Evaluate("GetRandomPositionInVolume()", "copies", "All positions are in one of the copies", InA + InB, N) && Passed;
  Passed = EvaluateNear("GetRandomPositionInVolume()", "copies", "Half of the positions are in the first copy", static_cast<double>(InA)/N, 0.5, 5.0*0.5/sqrt(N)) && Passed;

  // Nested copies: 2 copies of a mother, each with 2 copies of a daughter - 4 placements
  Text = GetGeometryHead() + "Volume M\nM.Material Vacuum\nM.Shape BRIK 5 5 5\nM.Copy M1\nM1.Position 20 0 0\nM1.Mother World\nM.Copy M2\nM2.Position -20 0 0\nM2.Mother World\n";
  Text += "Volume T\nT.Material Aluminium\nT.Shape BRIK 1 1 1\nT.Copy A\nA.Position 2 0 0\nA.Mother M\nT.Copy B\nB.Position -2 0 0\nB.Mother M\n";
  Passed = EvaluateTrue("ScanSetupFile()", "nested copies", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  vector<unsigned int> Counts(4, 0);
  for (unsigned int n = 0; n < N; ++n) {
    MVector P = Geometry.GetRandomPositionInVolume("T");
    unsigned int Bin = 0;
    if (P.X() > 0) {
      Bin += 2;
    }
    if (fabs(P.X()) > 20) {
      Bin += 1;
    }
    ++Counts[Bin];
  }
  for (unsigned int b = 0; b < 4; ++b) {
    MString Input = "nested copies, placement "; Input += b;
    // Binomial with p = 1/4: sigma = sqrt(p (1 - p) / N)
    Passed = EvaluateNear("GetRandomPositionInVolume()", Input, "Each placement has a quarter of the positions", static_cast<double>(Counts[b])/N, 0.25, 5.0*sqrt(0.25*0.75/N)) && Passed;
  }

  // Defect GEO-A-46: A template which is placed together with its copies only counts as one placement, thus its copies are never sampled
  Text = GetGeometryHead() + "Volume T\nT.Material Aluminium\nT.Shape BRIK 1 1 1\nT.Position 10 0 0\nT.Mother World\nT.Copy C\nC.Position -10 0 0\nC.Mother World\n";
  Passed = EvaluateTrue("ScanSetupFile()", "placed template and copy", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  unsigned int InCopy = 0;
  for (unsigned int n = 0; n < N; ++n) {
    if (Geometry.GetRandomPositionInVolume("T").X() < 0) {
      ++InCopy;
    }
  }
  Passed = ExpectedToFail(EvaluateNear("GetRandomPositionInVolume()", "placed template and copy", "Half of the positions are in the copy", static_cast<double>(InCopy)/N, 0.5, 5.0*0.5/sqrt(N))) && Passed;

  // The second call uses the cached placements and gives valid data, also after another volume:
  Passed = EvaluateTrue("ScanSetupFile()", "cache", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("A", 10) + Marker("B", -10))) && Passed;
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "A", "A position is in A", Geometry.GetRandomPositionInVolume("A").X() > 8) && Passed;
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "A again", "A position is in A", Geometry.GetRandomPositionInVolume("A").X() > 8) && Passed;
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "B", "A position is in B", Geometry.GetRandomPositionInVolume("B").X() < -8) && Passed;

  // The cache does not survive a second scan:
  Passed = EvaluateTrue("ScanSetupFile()", "cache after a scan", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("B", 10))) && Passed;
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "B after a second scan", "A position is in the new B", Geometry.GetRandomPositionInVolume("B").X() > 8) && Passed;

  // Unknown volume:
  DisableDefaultStreams();
  MVector Unknown = Geometry.GetRandomPositionInVolume("Unknown");
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetRandomPositionInVolume()", "unknown", "An unknown volume returns g_VectorNotDefined", Unknown == g_VectorNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the start volume
bool UTDGeometry::TestStartVolume()
{
  bool Passed = true;

  // The start volume keeps its position in the world coordinate system: a box at (10, 0, 0) with a daughter at (0, 2, 0)
  MString Text = GetGeometryHead() + "Volume M\nM.Material Vacuum\nM.Shape BRIK 5 5 5\nM.Position 10 0 0\nM.Mother World\nVolume D\nD.Material Aluminium\nD.Shape BRIK 1 2 3\nD.Position 0 2 0\nD.Mother M\n" + Marker("Other", -20);

  MDGeometry Geometry;
  Geometry.SetStartVolumeName("D");
  Passed = EvaluateTrue("ScanSetupFile()", "start volume", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  Passed = EvaluateVectorNear("SetStartVolumeName()", "D", "The start volume has its position in the world", Geometry.GetVolume("D")->GetPosition(), MVector(10, 2, 0), 1e-12) && Passed;
  Passed = EvaluateTrue("SetStartVolumeName()", "D", "The start volume is in the world", Geometry.GetVolume("D")->GetMother() == Geometry.GetWorldVolume()) && Passed;
  Passed = EvaluateTrue("SetStartVolumeName()", "D", "All others are gone from the tree", Geometry.GetVolumeSequence(MVector(-20, 0, 0)).GetDeepestVolume() == Geometry.GetWorldVolume()) && Passed;
  Passed = EvaluateTrue("SetStartVolumeName()", "D", "The start volume is found at its old position", Geometry.GetVolumeSequence(MVector(10, 2, 0)).GetDeepestVolume() == Geometry.GetVolume("D")) && Passed;

  // Not existing, not placed, and virtual start volumes are ignored:
  MDGeometry Unknown;
  Unknown.SetStartVolumeName("Unknown");
  Passed = EvaluateTrue("ScanSetupFile()", "unknown start volume", "The file is accepted", ScanGeometry(Unknown, Text)) && Passed;
  Passed = EvaluateTrue("SetStartVolumeName()", "unknown", "All volumes stay", Unknown.GetVolumeSequence(MVector(-20, 0, 0)).GetDeepestVolume() == Unknown.GetVolume("Other")) && Passed;
  MDGeometry NotPlaced;
  NotPlaced.SetStartVolumeName("T");
  Passed = EvaluateTrue("ScanSetupFile()", "not placed start volume", "The file is accepted", ScanGeometry(NotPlaced, Text + "Volume T\nT.Material Vacuum\nT.Shape BRIK 1 1 1\nT.Copy C\nC.Position 40 0 0\nC.Mother World\n")) && Passed;
  Passed = EvaluateTrue("SetStartVolumeName()", "not placed", "All volumes stay", NotPlaced.GetVolumeSequence(MVector(-20, 0, 0)).GetDeepestVolume() == NotPlaced.GetVolume("Other")) && Passed;

  // The name survives a scan:
  Passed = EvaluateTrue("GetStartVolumeName()", "after a scan", "The start volume name stays", Geometry.GetStartVolumeName() == "D") && Passed;

  // A rotated start volume: box with half lengths 3, 1, 1, rotated by +30 deg around z, thus the point 2.5 cm along the direction of 30 deg is inside
  MVector PointOnAxis(2.5*cos(30*c_Rad), 2.5*sin(30*c_Rad), 0.0);
  MString Rotated = GetGeometryHead() + "Volume M\nM.Material Vacuum\nM.Shape BRIK 5 5 5\nM.Mother World\nVolume D\nD.Material Aluminium\nD.Shape BRIK 3 1 1\nD.Rotation 0 0 30\nD.Mother M\n";
  MDGeometry WithoutStart;
  Passed = EvaluateTrue("ScanSetupFile()", "rotated volume", "The file is accepted", ScanGeometry(WithoutStart, Rotated)) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "rotated volume", "Without start volume the box is found on its rotated axis", WithoutStart.GetVolumeSequence(PointOnAxis).GetDeepestVolume() == WithoutStart.GetVolume("D")) && Passed;
  MDGeometry WithStart;
  WithStart.SetStartVolumeName("D");
  Passed = EvaluateTrue("ScanSetupFile()", "rotated start volume", "The file is accepted", ScanGeometry(WithStart, Rotated)) && Passed;
  // Defect GEO-A-36: The start volume gets the inverse of its rotation
  Passed = ExpectedToFail(EvaluateTrue("GetVolumeSequence()", "rotated start volume", "With start volume the box is at the same place", WithStart.GetVolumeSequence(PointOnAxis).GetDeepestVolume() == WithStart.GetVolume("D"))) && Passed;

  // Defect GEO-A-37: The world volume as start volume removes everything and makes the world its own mother
  Passed = ExpectedToCrash(EvaluateTrue("SetStartVolumeName()", "World", "The world as start volume shows the whole geometry", IsWholeGeometryFoundFromWorld(Text))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the preferred visible volumes
bool UTDGeometry::TestPreferredVisibleVolumes()
{
  bool Passed = true;

  MDGeometry Geometry;
  Geometry.AddPreferredVisibleVolume("A");
  MString Text = GetGeometryHead() + Marker("A") + "A.Visibility 0\n" + Marker("B", 3) + "B.Visibility 1\nA.Copy C\nC.Position 6 0 0\nC.Mother World\n";
  Passed = EvaluateTrue("ScanSetupFile()", "preferred", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  Passed = Evaluate("AddPreferredVisibleVolume()", "A", "The preferred volume is visible", Geometry.GetVolume("A")->GetVisibility(), 1) && Passed;
  Passed = Evaluate("AddPreferredVisibleVolume()", "B", "The other volume is not visible", Geometry.GetVolume("B")->GetVisibility(), 0) && Passed;

  MDGeometry NotThere;
  NotThere.AddPreferredVisibleVolume("Unknown");
  const MString FileName = GetTemporaryFileName("Preferred.geo.setup");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(FileName, Text)) && Passed;
  bool Result = false;
  BeginCaptureOutput();
  Result = NotThere.ScanSetupFile(FileName, false, false, false);
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("ScanSetupFile()", "unknown preferred", "The file is accepted", Result) && Passed;
  Passed = EvaluateTrue("AddPreferredVisibleVolume()", "unknown", "The problem is reported", Output.Contains("None of your preferred visible volumes")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword Import
bool UTDGeometry::TestImport()
{
  bool Passed = true;
  UTDGeometryAccess Geometry;

  Passed = EvaluateFalse("ScanSetupFile()", "Import without file", "An import without a file is rejected", Accepts("Import GDML\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Import with too many entries", "An import with too many entries is rejected", Accepts("Import GDML A B\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "missing GDML file", "An import of a missing file is rejected", Accepts("Import GDML DoesNotExist.gdml\n")) && Passed;

  // Defect GEO-A-38: An import of an unknown type prints an error and resets the state, but the scan goes on and returns true
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "unknown import type", "An import of an unknown type is rejected", Accepts("Import FOO File\n"))) && Passed;

  // Defect GEO-A-39: The imported GDML content is never added to the geometry and the scan succeeds
  const MString GDML = GetTemporaryFileName("Import.gdml");
  MString Content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<gdml xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n<define/>\n";
  Content += "<materials><material name=\"Al\" Z=\"13\"><D value=\"2.7\"/><atom value=\"26.98\"/></material></materials>\n";
  Content += "<solids><box name=\"WorldBox\" x=\"100\" y=\"100\" z=\"100\" lunit=\"cm\"/><box name=\"InnerBox\" x=\"10\" y=\"10\" z=\"10\" lunit=\"cm\"/></solids>\n";
  Content += "<structure><volume name=\"GDMLInner\"><materialref ref=\"Al\"/><solidref ref=\"InnerBox\"/></volume>\n";
  Content += "<volume name=\"GDMLWorld\"><materialref ref=\"Al\"/><solidref ref=\"WorldBox\"/><physvol><volumeref ref=\"GDMLInner\"/></physvol></volume></structure>\n";
  Content += "<setup name=\"Default\" version=\"1.0\"><world ref=\"GDMLWorld\"/></setup>\n</gdml>\n";
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(GDML, Content)) && Passed;
  // Defect GEO-A-67: The import of a GDML file sometimes crashes
  Passed = ExpectedToCrash(EvaluateTrue("ScanSetupFile()", "GDML with a volume", "The volume of the GDML file is in the geometry or the import is rejected", IsGDMLVolumeImportedOrRejected())) && Passed;

  // Directly:
  list<MDDebugInfo> Content2;
  DisableDefaultStreams();
  bool MissingGDML = Geometry.ImportGDML(GetTemporaryFileName("DoesNotExist.gdml"), Content2);
  EnableDefaultStreams();
  Passed = EvaluateFalse("ImportGDML()", "missing file", "A missing file is rejected", MissingGDML) && Passed;
  // Defect GEO-A-39: The imported GDML content is never added to the geometry and the scan succeeds
  // Defect GEO-A-67: The import of a GDML file sometimes crashes
  Passed = ExpectedToCrash(EvaluateTrue("ImportGDML()", "file with a volume", "The file gives geomega lines", HasGDMLImportLines(GDML))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the cross section file interface
bool UTDGeometry::TestCrossSections()
{
  bool Passed = true;

  // A new material has no cross sections and nothing is created without the permission:
  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "no cross section creation", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + "Material NewOne\nNewOne.Density 1.0\nNewOne.Component H 1\n")) && Passed;
  Passed = EvaluateFalse("AreCrossSectionsPresent()", "new material", "The cross sections of a new material do not exist", Geometry.AreCrossSectionsPresent()) && Passed;

  // Two independent programs of one kind which start at the same time, with a private cosima which records its source file name:
  PrepareTemporaryDirectory("FakeMegalib");
  const MString Root = GetTemporaryDirectoryName("FakeMegalib");
  MFile::CreateDirectory(Root + "/bin");
  MFile::CreateDirectory(Root + "/tmp");
  const MString Log = Root + "/Names.log";
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Root + "/bin/source-megalib.sh", "export PATH=" + MSystem::GetShellQuoted(Root + "/bin") + ":$PATH\n")) && Passed;
  // The private cosima waits until the second program has started its cosima too, so that the two runs overlap
  const MString NoOverlap = Root + "/NoOverlap.log";
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Root + "/bin/cosima", "#!/bin/bash\necho \"$1\" >> " + MSystem::GetShellQuoted(Log) + "\nfor i in $(seq 1 600); do [ $(wc -l < " + MSystem::GetShellQuoted(Log) + ") -ge 2 ] && exit 0; sleep 0.1; done\necho timeout >> " + MSystem::GetShellQuoted(NoOverlap) + "\n")) && Passed;
  chmod((Root + "/bin/cosima").Data(), 0755);
  const MString Setup = Root + "/CrossSection.geo.setup";
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Setup, "Name N\nSurroundingSphere 1 0 0 0 1\nMaterial Mine\nMine.Density 1\nMine.Component H 1\nVolume W\nW.Material Mine\nW.Shape BRIK 1 1 1\nW.Mother 0\n")) && Passed;

  // A new program starts with the same state as any new program: this is required for the state of gRandom
  const MString Program = filesystem::read_symlink("/proc/self/exe").string().c_str();
  const MString Arguments = MString("MEGALIB=") + MSystem::GetShellQuoted(Root) + " TMPDIR=" + MSystem::GetShellQuoted(Root + "/tmp") + " " + MSystem::GetShellQuoted(Program) + " --cross-section-child " + MSystem::GetShellQuoted(Setup);
  vector<pid_t> Children;
  for (unsigned int Run = 0; Run < 2; ++Run) {
    MString Output = Root + "/Child";
    Output += Run;
    Output += ".log";
    Children.push_back(MProcessManager::StartInBackground("env", Arguments, Output, "", true));
  }
  for (pid_t Child: Children) {
    if (EvaluateTrue("StartInBackground()", "child process", "The child process can be started", Child > 0) == true) {
      // The raw wait status: a program which runs too long is killed and has no exit code
      const int Status = MProcessManager::WaitForBackground(Child, 120);
      Passed = EvaluateTrue("CreateCrossSectionFiles()", "child process", "The child process ends with the exit code 0 within its time limit", Status >= 0 && WIFEXITED(Status) != 0 && WEXITSTATUS(Status) == 0) && Passed;
    } else {
      Passed = false;
    }
  }
  Passed = EvaluateFalse("CreateCrossSectionFiles()", "overlap", "The two programs run at the same time", filesystem::exists(NoOverlap.Data())) && Passed;

  vector<MString> Names = ReadTextFile(Log).Tokenize("\n");
  Passed = EvaluateSize("CreateCrossSectionFiles()", "two programs", "Each program calls cosima once", Names.size(), 2) && Passed;
  if (Names.size() == 2) {
    Passed = EvaluateTrue("CreateCrossSectionFiles()", "file name", "The file is in the temporary directory", Names[0].BeginsWith(Root + "/tmp/MEGAlib_") && Names[0].EndsWith("_CrossSections.source")) && Passed;
    Passed = EvaluateFalse("CreateCrossSectionFiles()", "two programs", "The two programs use different temporary files", Names[0] == Names[1]) && Passed;
  }

  // The creation of the cross section files does not change the state of the global random number generator
  const MString Fake = Root + "/bin/cosima";

  // The creation uses MEGALIB and TMPDIR: they point to the fixture until the end of this test
  const char* MegalibSetting = getenv("MEGALIB");
  const MString OldMegalib = (MegalibSetting != nullptr) ? MegalibSetting : "";
  const char* TmpDirSetting = getenv("TMPDIR");
  const bool HasOldTmpDir = (TmpDirSetting != nullptr);
  const MString OldTmpDir = (HasOldTmpDir == true) ? TmpDirSetting : "";
  setenv("MEGALIB", Root.Data(), 1);
  setenv("TMPDIR", (Root + "/tmp").Data(), 1);

  gRandom->SetSeed(4711);
  double Expected = gRandom->Rndm();
  gRandom->SetSeed(4711);
  MDGeometry RandomUser;
  DisableDefaultStreams();
  bool IsAccepted = RandomUser.ScanSetupFile(Setup, false, false, false);
  RandomUser.CreateCrossSectionFiles();
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "gRandom", "The file is accepted", IsAccepted) && Passed;
  Passed = EvaluateNear("CreateCrossSectionFiles()", "gRandom", "The global random number generator is not changed", gRandom->Rndm(), Expected, 1e-15) && Passed;

  // The scan starts the creation when it is allowed, and the working directory stays:
  const MString CountLog = Log;
  unsigned int NamesBefore = ReadTextFile(CountLog).Tokenize("\n").size();
  const MString Working = gSystem->WorkingDirectory();
  MDGeometry Creator;
  DisableDefaultStreams();
  Creator.ScanSetupFile(Setup, false, false, true);
  EnableDefaultStreams();
  Passed = Evaluate("ScanSetupFile()", "cross section creation allowed", "The working directory is back after the creation", MString(gSystem->WorkingDirectory()), Working) && Passed;
  Passed = EvaluateSize("ScanSetupFile()", "cross section creation allowed", "Cosima has been started once", ReadTextFile(CountLog).Tokenize("\n").size(), NamesBefore + 1) && Passed;

  // The clean up of the creation removes only its own source file, not the temporary files of other programs running at the same time
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Root + "/tmp/DelMe.Other.sim", "Belongs to another program\n")) && Passed;
  MDGeometry Cleaner;
  DisableDefaultStreams();
  IsAccepted = Cleaner.ScanSetupFile(Setup, false, false, false);
  Cleaner.CreateCrossSectionFiles();
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "file of another program", "The file is accepted", IsAccepted) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "file of another program", "The temporary file of another program stays", MFile::Exists(Root + "/tmp/DelMe.Other.sim")) && Passed;
  // The fake cosima wrote the name of the source file into the log, the last line is the one of this call
  vector<MString> CleanerNames = ReadTextFile(Log).Tokenize("\n");
  if (EvaluateTrue("CreateCrossSectionFiles()", "own source file", "Cosima has been started with a source file", CleanerNames.empty() == false) == true) {
    Passed = EvaluateFalse("CreateCrossSectionFiles()", "own source file", "The own source file is removed after the creation", MFile::Exists(CleanerNames.back())) && Passed;
  } else {
    Passed = false;
  }

  // Defect GEO-A-51: The path is not quoted in the command, which breaks with a space in the temporary directory
  MFile::CreateDirectory(Root + "/tmp with space");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Root + "/bin/cosima", "#!/bin/bash\necho \"$#\" > " + Root + "/Count.log\n")) && Passed;
  chmod((Root + "/bin/cosima").Data(), 0755);
  // The unquoted path gives error messages of the shell, thus this runs as its own program with its output in a file
  const MString SpaceArguments = MString("MEGALIB=") + MSystem::GetShellQuoted(Root) + " TMPDIR=" + MSystem::GetShellQuoted(Root + "/tmp with space") + " " + MSystem::GetShellQuoted(Program) + " --cross-section-child " + MSystem::GetShellQuoted(Setup);
  const int SpaceStatus = MProcessManager::Run("env", SpaceArguments, Root + "/Space.log", "", 120);
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "space in the temporary directory", "The program ends with the exit code 0 within its time limit", SpaceStatus >= 0 && WIFEXITED(SpaceStatus) != 0 && WEXITSTATUS(SpaceStatus) == 0) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("CreateCrossSectionFiles()", "space in the temporary directory", "Cosima gets the source file name as one argument", ReadTextFile(Root + "/Count.log").BeginsWith("1"))) && Passed;

  // Without cosima no creation is possible:
  RemoveTemporaryFile(Fake);
  if (HasOldTmpDir == true) {
    setenv("TMPDIR", OldTmpDir.Data(), 1);
  } else {
    unsetenv("TMPDIR");
  }
  MDGeometry NoCosima;
  DisableDefaultStreams();
  IsAccepted = NoCosima.ScanSetupFile(Setup, false, false, false);
  const bool IsCreated = NoCosima.CreateCrossSectionFiles();
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "no cosima", "The file is accepted", IsAccepted) && Passed;
  Passed = EvaluateFalse("CreateCrossSectionFiles()", "no cosima", "The function returns false without cosima", IsCreated) && Passed;

  setenv("MEGALIB", OldMegalib.Data(), 1);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Run one geometry with the cross section creation - the child process of TestCrossSections
int UTDGeometry::RunCrossSectionChild(const MString& FileName)
{
  MDGeometry Geometry;
  if (Geometry.ScanSetupFile(FileName, false, false, false) == false) {
    return 2;
  }
  Geometry.CreateCrossSectionFiles();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the error messages
bool UTDGeometry::TestErrorMessages()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Defect GEO-A-42: The line numbers in the error messages start with 0
  const MString FileName = GetTemporaryFileName("Messages.geo.setup");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(FileName, "// Line one\n// Line two\nName\n")) && Passed;
  bool Result = true;
  BeginCaptureOutput();
  Result = Geometry.ScanSetupFile(FileName, false, false, false);
  MString Output = EndCaptureOutput();
  Passed = EvaluateFalse("ScanSetupFile()", "error in line 3", "The file is rejected", Result) && Passed;
  Passed = EvaluateTrue("Typo()", "error in line 3", "The name of the file is shown", Output.Contains("Messages.geo.setup")) && Passed;
  Passed = EvaluateTrue("Typo()", "error in line 3", "The text of the line is shown", Output.Contains("\"Name\"")) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("Typo()", "error in line 3", "The third line is called line 3", Output.Contains("at line 3:"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that constants are evaluated independently of the order of their definition
bool UTDGeometry::TestConstantsOrder()
{
  bool Passed = true;

  // Outer = 2 * Middle = 2 * (Inner + 1) = 2 * (1.5 + 1) = 5 in all orders of the definitions, and with the use before and after them
  const vector<MString> Definitions = { "Constant Outer { 2 * Middle }\n", "Constant Middle { Inner + 1 }\n", "Constant Inner 1.5\n" };
  const vector<vector<unsigned int>> Orders = { { 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 } };
  for (const vector<unsigned int>& Order: Orders) {
    MString Constants = Definitions[Order[0]] + Definitions[Order[1]] + Definitions[Order[2]];
    MString Input = MString("order ") + Order[0] + " " + Order[1] + " " + Order[2];

    MDGeometry Before;
    Passed = EvaluateTrue("ScanSetupFile()", Input + ", use after the definitions", "The geometry is accepted", ScanGeometry(Before, GetGeometryHead() + Constants + Marker("A") + "A.Position Outer Middle Inner\n")) && Passed;
    Passed = CheckPosition(Before, "A", MVector(5.0, 2.5, 1.5), Input + ", use after the definitions") && Passed;

    MDGeometry After;
    Passed = EvaluateTrue("ScanSetupFile()", Input + ", use before the definitions", "The geometry is accepted", ScanGeometry(After, GetGeometryHead() + Marker("A") + "A.Position Outer Middle Inner\n" + Constants)) && Passed;
    Passed = CheckPosition(After, "A", MVector(5.0, 2.5, 1.5), Input + ", use before the definitions") && Passed;
  }

  // A chain of constants without maths
  MDGeometry Alias;
  Passed = EvaluateTrue("ScanSetupFile()", "chain without maths", "The geometry is accepted", ScanGeometry(Alias, GetGeometryHead() + "Constant First Second\nConstant Second Third\nConstant Third 2.5\n" + Marker("A") + "A.Position First Second Third\n")) && Passed;
  Passed = CheckPosition(Alias, "A", MVector(2.5, 2.5, 2.5), "chain without maths") && Passed;

  // A deep chain with maths: Level5 = Level0 + 5 = 6
  MString Deep = "Constant Level0 1.0\n";
  for (unsigned int l = 1; l <= 5; ++l) {
    Deep = MString("Constant Level") + l + " { Level" + (l - 1) + " + 1 }\n" + Deep;
  }
  MDGeometry DeepGeometry;
  Passed = EvaluateTrue("ScanSetupFile()", "five levels of maths", "The geometry is accepted", ScanGeometry(DeepGeometry, GetGeometryHead() + Deep + Marker("A") + "A.Position Level5 Level3 Level0\n")) && Passed;
  Passed = CheckPosition(DeepGeometry, "A", MVector(6.0, 4.0, 1.0), "five levels of maths") && Passed;

  // Names which contain each other: only whole words are replaced, in the constants and in the lines
  MDGeometry Nested;
  Passed = EvaluateTrue("ScanSetupFile()", "names containing each other", "The geometry is accepted", ScanGeometry(Nested, GetGeometryHead() + "Constant Len 1.5\nConstant LenX { Len + 1 }\nConstant LenXY { LenX + Len }\n" + Marker("LenBox") + "LenBox.Position Len LenX LenXY\n")) && Passed;
  Passed = CheckPosition(Nested, "LenBox", MVector(1.5, 2.5, 4.0), "names containing each other") && Passed;

  // One constant used several times in one expression and on one line
  MDGeometry Repeated;
  Passed = EvaluateTrue("ScanSetupFile()", "repeated constant", "The geometry is accepted", ScanGeometry(Repeated, GetGeometryHead() + "Constant Base 1.5\nConstant Square { Base * Base + Base }\n" + Marker("A") + "A.Position Square Base Base\n")) && Passed;
  Passed = CheckPosition(Repeated, "A", MVector(3.75, 1.5, 1.5), "repeated constant") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a file with more than 2000 lines, where the constants are replaced in parallel
bool UTDGeometry::TestLargeFile()
{
  bool Passed = true;

  // 2500 constants with the values 0.00, 0.01, ..., some of them with maths, and volumes which use them
  MString Constants;
  for (unsigned int c = 0; c < 2500; ++c) {
    if (c % 100 == 99) {
      Constants += MString("Constant Value") + c + " { Value" + (c - 1) + " + 0.01 }\n";
    } else {
      Constants += MString("Constant Value") + c + " " + 0.01*c + "\n";
    }
  }
  MString Volumes;
  const vector<unsigned int> Used = { 0, 1, 99, 100, 1234, 1999, 2000, 2499 };
  for (unsigned int u = 0; u < Used.size(); ++u) {
    MString Name = MString("Box") + Used[u];
    Volumes += Marker(Name) + Name + ".Position Value" + Used[u] + " " + (3.0*u) + " { 2 * Value" + Used[u] + " }\n";
  }

  // The same content as a small file is the reference
  MString SmallConstants;
  for (unsigned int u = 0; u < Used.size(); ++u) {
    SmallConstants += MString("Constant Value") + Used[u] + " " + 0.01*Used[u] + "\n";
  }

  MDGeometry Small;
  Passed = EvaluateTrue("ScanSetupFile()", "small file", "The geometry is accepted", ScanGeometry(Small, GetGeometryHead() + SmallConstants + Volumes)) && Passed;

  MDGeometry Large;
  BeginCaptureOutput();
  bool IsScanned = ScanGeometry(Large, GetGeometryHead() + Constants + Volumes);
  MString Output = EndCaptureOutput();
  Passed = EvaluateTrue("ScanSetupFile()", "file with more than 2000 lines", "The geometry is accepted", IsScanned) && Passed;
  if (IsScanned == false) {
    return false;
  }

  for (unsigned int u = 0; u < Used.size(); ++u) {
    MString Name = MString("Box") + Used[u];
    MVector Expected(0.01*Used[u], 3.0*u, 0.02*Used[u]);
    Passed = CheckPosition(Small, Name, Expected, MString("small file, constant ") + Used[u]) && Passed;
    Passed = CheckPosition(Large, Name, Expected, MString("file with more than 2000 lines, constant ") + Used[u]) && Passed;
  }
  Passed = Evaluate("GetNVolumes()", "file with more than 2000 lines", "The large file has the same number of volumes as the small one", Large.GetNVolumes(), Small.GetNVolumes()) && Passed;

  // Defect GEO-A-70: A scan of more than 2000 lines prints the state of OpenMP to the standard output
  Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", "file with more than 2000 lines", "Nothing is written to the standard output", Output.IsEmpty())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that each object is found by its name
bool UTDGeometry::TestNameLookups()
{
  bool Passed = true;
  MDGeometry Geometry;

  // Copies of a volume with a daughter, a detector with guard ring and a named detector, and one object of each other kind
  MString Text = GetGeometryHead();
  Text += "Orientation O\nO.Position 0 0 0\nShape BRIK S\nS.Parameters 1 1 1\nVector V\n";
  Text += "Volume Holder\nHolder.Material Vacuum\nHolder.Shape BRIK 6.0 6.0 1.0\n";
  Text += "Volume Wafer\nWafer.Material Silicon\nWafer.Shape BRIK 5.0 5.0 0.5\nWafer.Position 0.0 0.0 0.0\nWafer.Mother Holder\n";
  Text += "Holder.Copy HolderA\nHolderA.Position 0.0 0.0 -20.0\nHolderA.Mother World\n";
  Text += "Holder.Copy HolderB\nHolderB.Position 0.0 0.0 20.0\nHolderB.Mother World\n";
  Text += FullDetector("Strip2D");
  Text += "D.GuardRingTriggerThreshold 40\nD.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 1000 1\n";
  Text += "D.Named DNamed\nDNamed.Assign World.HolderA.Wafer\n";
  Text += "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector D 1\n";
  Passed = EvaluateTrue("ScanSetupFile()", "name lookups", "The file is accepted", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == false) {
    return false;
  }

  unsigned int NWrong = 0;
  MString Wrong;
  for (unsigned int i = 0; i < Geometry.GetNVolumes(); ++i) {
    if (Geometry.GetVolume(Geometry.GetVolumeAt(i)->GetName()) != Geometry.GetVolumeAt(i)) {
      ++NWrong;
      Wrong += Geometry.GetVolumeAt(i)->GetName() + " ";
    }
  }
  Passed = EvaluateTrue("GetNVolumes()", "name lookups", "There are the world, the holder, the wafer, and the copies", Geometry.GetNVolumes() >= 5) && Passed;
  Passed = Evaluate("GetVolume()", "name of each volume in the list", "Each volume is found by its name (not found: " + Wrong + ")", NWrong, 0U) && Passed;

  NWrong = 0;
  Wrong = "";
  for (unsigned int i = 0; i < Geometry.GetNMaterials(); ++i) {
    if (Geometry.GetMaterial(Geometry.GetMaterialAt(i)->GetName()) != Geometry.GetMaterialAt(i)) {
      ++NWrong;
      Wrong += Geometry.GetMaterialAt(i)->GetName() + " ";
    }
  }
  Passed = Evaluate("GetMaterial()", "name of each material in the list", "Each material is found by its name (not found: " + Wrong + ")", NWrong, 0U) && Passed;

  NWrong = 0;
  Wrong = "";
  bool HasGuardRing = false;
  for (unsigned int i = 0; i < Geometry.GetNDetectors(); ++i) {
    if (Geometry.GetDetectorAt(i)->GetType() == MDDetector::c_GuardRing) {
      HasGuardRing = true;
    }
    if (Geometry.GetDetector(Geometry.GetDetectorAt(i)->GetName()) != Geometry.GetDetectorAt(i)) {
      ++NWrong;
      Wrong += Geometry.GetDetectorAt(i)->GetName() + " ";
    }
  }
  Passed = EvaluateTrue("GetDetectorAt()", "detector with guard ring", "The guard ring is in the list of the detectors", HasGuardRing) && Passed;
  Passed = Evaluate("GetDetector()", "name of each detector in the list", "Each detector is found by its name (not found: " + Wrong + ")", NWrong, 0U) && Passed;

  NWrong = 0;
  for (unsigned int i = 0; i < Geometry.GetNShapes(); ++i) {
    if (Geometry.GetShape(Geometry.GetShapeAt(i)->GetName()) != Geometry.GetShapeAt(i)) {
      ++NWrong;
    }
  }
  Passed = EvaluateTrue("GetNShapes()", "name lookups", "There is at least the shape of the file", Geometry.GetNShapes() >= 1) && Passed;
  Passed = Evaluate("GetShape()", "name of each shape in the list", "Each shape is found by its name", NWrong, 0U) && Passed;

  NWrong = 0;
  for (unsigned int i = 0; i < Geometry.GetNOrientations(); ++i) {
    if (Geometry.GetOrientation(Geometry.GetOrientationAt(i)->GetName()) != Geometry.GetOrientationAt(i)) {
      ++NWrong;
    }
  }
  Passed = EvaluateTrue("GetNOrientations()", "name lookups", "There is at least the orientation of the file", Geometry.GetNOrientations() >= 1) && Passed;
  Passed = Evaluate("GetOrientation()", "name of each orientation in the list", "Each orientation is found by its name", NWrong, 0U) && Passed;

  NWrong = 0;
  for (unsigned int i = 0; i < Geometry.GetNTriggers(); ++i) {
    if (Geometry.GetTrigger(Geometry.GetTriggerAt(i)->GetName()) != Geometry.GetTriggerAt(i)) {
      ++NWrong;
    }
  }
  Passed = Evaluate("GetNTriggers()", "name lookups", "There is one trigger", Geometry.GetNTriggers(), 1U) && Passed;
  Passed = Evaluate("GetTrigger()", "name of each trigger in the list", "Each trigger is found by its name", NWrong, 0U) && Passed;

  NWrong = 0;
  for (unsigned int i = 0; i < Geometry.GetNVectors(); ++i) {
    if (Geometry.GetVector(Geometry.GetVectorAt(i)->GetName()) != Geometry.GetVectorAt(i)) {
      ++NWrong;
    }
  }
  Passed = Evaluate("GetNVectors()", "name lookups", "There is one vector", Geometry.GetNVectors(), 1U) && Passed;
  Passed = Evaluate("GetVector()", "name of each vector in the list", "Each vector is found by its name", NWrong, 0U) && Passed;

  // A second scan of another file: the names of the first file are gone
  Passed = EvaluateTrue("ScanSetupFile()", "second file", "The file is accepted", ScanGeometry(Geometry, GetGeometryHead() + Marker("Other"))) && Passed;
  Passed = EvaluateTrue("GetVolume()", "second file", "The volume of the second file is found", Geometry.GetVolume("Other") != nullptr) && Passed;
  Passed = EvaluateTrue("GetVolume()", "second file, volume of the first file", "The volume is no longer found", Geometry.GetVolume("Wafer") == nullptr) && Passed;
  Passed = EvaluateTrue("GetVolume()", "second file, copy of the first file", "The volume is no longer found", Geometry.GetVolume("HolderA") == nullptr) && Passed;
  Passed = EvaluateTrue("GetDetector()", "second file, detector of the first file", "The detector is no longer found", Geometry.GetDetector(MString("D")) == nullptr) && Passed;
  Passed = EvaluateTrue("GetDetector()", "second file, guard ring of the first file", "The guard ring is no longer found", Geometry.GetDetector(MString("D_GuardRing")) == nullptr) && Passed;
  Passed = EvaluateTrue("GetTrigger()", "second file, trigger of the first file", "The trigger is no longer found", Geometry.GetTrigger("T") == nullptr) && Passed;
  Passed = EvaluateTrue("GetShape()", "second file, shape of the first file", "The shape is no longer found", Geometry.GetShape("S") == nullptr) && Passed;
  Passed = EvaluateTrue("GetOrientation()", "second file, orientation of the first file", "The orientation is no longer found", Geometry.GetOrientation("O") == nullptr) && Passed;
  Passed = EvaluateTrue("GetVector()", "second file, vector of the first file", "The vector is no longer found", Geometry.GetVector("V") == nullptr) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "second file, material of the include", "The material is found again", Geometry.GetMaterial("Aluminium") != nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  // The child process of the test of the temporary cross section files:
  if (argc == 3 && MString(argv[1]) == "--cross-section-child") {
    return UTDGeometry::RunCrossSectionChild(argv[2]);
  }

  UTDGeometry Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDGeometry.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
