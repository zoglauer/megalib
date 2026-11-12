/*
 * UTRun.cc
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


// Standard lib:
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Cosima:
#include "MCDetectorConstruction.hh"
#include "MDDetector.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MDGeometryQuest.h"
#include "MDVolume.h"
#include "MC2DStripSD.hh"
#include "MCCalorBarSD.hh"
#include "MCScintillatorSD.hh"
#include "MCSD.hh"
#include "MCEventAction.hh"
#include "MCIsotopeStore.hh"
#include "MCOrientation.hh"
#include "MCParameterFile.hh"
#include "MCPhysicsListParticles.hh"
#include "MCRunManager.hh"
#include "MCSource.hh"
#include "MCSteppingAction.hh"
#include "MCTrackingAction.hh"

// MEGAlib:
#include "MFile.h"
#include "MGlobal.h"
#include "MStreams.h"
#include "MString.h"
#include "MTokenizer.h"
#include "MUnitTest.h"
#include "UTFixtureShared.h"
#include "UTSDHitShared.h"

// Geant4:
#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4GenericIon.hh"
#include "G4Ions.hh"
#include "G4IonConstructor.hh"
#include "G4IonTable.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NavigationHistory.hh"
#include "G4NuclideTable.hh"
#include "G4PVPlacement.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4TouchableHistory.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Cons.hh"
#include "G4Trd.hh"
#include "G4Trap.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4DisplacedSolid.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4Material.hh"

// ROOT:
#include "TError.h"



// Access to the private and protected members of the tested classes (legal member-pointer access via explicit template instantiation)
template<typename Tag, typename Tag::type Member> struct UTDCRob { friend typename Tag::type UTDCGet(Tag) { return Member; } };
#define UTC_DC_ACCESS(Class, Name, Type) \
  struct UTDCTag##Class##Name { typedef Type Class::* type; friend type UTDCGet(UTDCTag##Class##Name); }; \
  template struct UTDCRob<UTDCTag##Class##Name, &Class::Name>;
UTC_DC_ACCESS(MCDetectorConstruction, m_Geometry, MDGeometryQuest*)
UTC_DC_ACCESS(MCDetectorConstruction, m_WorldVolume, G4VPhysicalVolume*)
UTC_DC_ACCESS(MCSD, m_StructuralSize, G4ThreeVector)
UTC_DC_ACCESS(MCSD, m_StructuralPitch, G4ThreeVector)
UTC_DC_ACCESS(MCSD, m_StructuralOffset, G4ThreeVector)
UTC_DC_ACCESS(MCSD, m_StructuralDimension, G4ThreeVector)
UTC_DC_ACCESS(MCSD, m_DetectorVolumeName, G4String)
UTC_DC_ACCESS(MCSD, m_DiscretizeHits, bool)
UTC_DC_ACCESS(MCSD, m_HasTimeResolution, bool)
UTC_DC_ACCESS(MCSD, m_IsNeverTriggering, bool)
UTC_DC_ACCESS(MC2DStripSD, m_XSize, double)
UTC_DC_ACCESS(MC2DStripSD, m_YSize, double)
UTC_DC_ACCESS(MC2DStripSD, m_XOffset, double)
UTC_DC_ACCESS(MC2DStripSD, m_YOffset, double)
UTC_DC_ACCESS(MC2DStripSD, m_XPitch, double)
UTC_DC_ACCESS(MC2DStripSD, m_YPitch, double)
UTC_DC_ACCESS(MC2DStripSD, m_XNStrips, int)
UTC_DC_ACCESS(MC2DStripSD, m_YNStrips, int)
UTC_DC_ACCESS(MC2DStripSD, m_Is3D, bool)
UTC_DC_ACCESS(MCEventAction, m_2DStripCollNames, vector<G4String>)
UTC_DC_ACCESS(MCEventAction, m_CalorimeterCollNames, vector<G4String>)
UTC_DC_ACCESS(MCEventAction, m_ScintillatorCollNames, vector<G4String>)
#define DC(Object, Class, Name) ((Object).*UTDCGet(UTDCTag##Class##Name()))


//! Unit test class for MCDetectorConstruction
class UTDetectorConstruction : public MUnitTest
{
public:
  //! Default constructor
  UTDetectorConstruction() : MUnitTest("UTDetectorConstruction"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr), m_Manager(nullptr) {}
  //! Default destructor
  virtual ~UTDetectorConstruction() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the construction and destruction of an uninitialized object
  bool TestConstruction();
  //! Test the detectors of the prototype geometry
  bool TestDetectors(MCDetectorConstruction* Detector);
  //! Test the world, the volumes, the queries
  bool TestQueries(MCDetectorConstruction* Detector);
  //! Test the conversion of the shapes
  bool TestShapes(MCDetectorConstruction* Detector);
  //! Test the conversion of the materials
  bool TestMaterials(MCDetectorConstruction* Detector);
  //! Test the positioning of the volumes
  bool TestPlacements(MCDetectorConstruction* Detector);
  //! Test the regions
  bool TestRegions(MCDetectorConstruction* Detector);
  //! Test failing initializations
  bool TestFailures();


  //! Silence cout and cerr
  void SilenceOutput();
  //! Restore cout and cerr
  void RestoreOutput();
  //! Initialize the Geant4 particles
  void InitializeGeant4();
  //! Create a parameter file with the given geometry and additional lines
  MCParameterFile* CreateParameters(const MString& Name, const MString& Geometry, const MString& Additional);
  //! Write the shapes geometry and return its file name
  MString WriteShapesGeometry(const MString& Name, const MString& Trap = "TRAP 5 10 20 4 3 2 5 4 3 2 5");
  //! Find a logical volume
  G4LogicalVolume* Logical(const G4String& Name);
  //! Find a physical volume
  G4VPhysicalVolume* Physical(const G4String& Name);
  //! Initialize a detector silently
  bool InitializeSilently(MCDetectorConstruction* Detector);
  //! Return the solid of a logical volume
  template <typename T> T* Solid(const G4String& VolumeName)
  {
    G4LogicalVolume* L = Logical(VolumeName);
    if (L == nullptr) return nullptr;
    return dynamic_cast<T*>(L->GetSolid());
  }

  streambuf* m_CoutBuffer;
  streambuf* m_CerrBuffer;
  ostringstream m_Sink;
  MCRunManager* m_Manager;
  MCEventAction* m_Action = nullptr;
  MString m_Directory;
};


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::Run()
{
  bool Passed = true;

  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  if (RequireFixture(*this, Passed, "resource/examples/geomega/simplifiedprototype/Prototype.geo.setup") == false) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return Passed;
  }

  PrepareTemporaryDirectory("Files");
  m_Directory = GetTemporaryDirectoryName("Files");

  Passed = TestConstruction() && Passed;

  MCParameterFile* Prototype = CreateParameters("Prototype", "$(MEGALIB)/resource/examples/geomega/simplifiedprototype/Prototype.geo.setup", "");
  Passed = EvaluateTrue("CreateParameters()", "prototype", "The parameter file of the prototype can be opened", Prototype != nullptr) && Passed;
  if (Prototype == nullptr) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return false;
  }

  // The test geometry with all shapes and a region:
  const MString Shapes = WriteShapesGeometry("Shapes");
  MCParameterFile* ShapesParameters = CreateParameters("Shapes", Shapes, "Region TestRegion\nTestRegion.Volume ShBox\nTestRegion.RangeCut 0.5\n");
  Passed = EvaluateTrue("CreateParameters()", "shapes", "The parameter file of the shapes can be opened", ShapesParameters != nullptr) && Passed;
  if (ShapesParameters == nullptr) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return false;
  }

  SilenceOutput();
  m_Manager = new MCRunManager(*Prototype);
  RestoreOutput();
  InitializeGeant4();

  SilenceOutput();
  m_Action = new MCEventAction(*Prototype, false, 1, true);
  MCTrackingAction* TrackingAction = new MCTrackingAction();
  MCSteppingAction* SteppingAction = new MCSteppingAction(*Prototype);
  m_Manager->SetUserAction(m_Action);
  m_Manager->SetUserAction(TrackingAction);
  m_Manager->SetUserAction(SteppingAction);
  RestoreOutput();

  // Geometry 1: the prototype with strip detectors and calorimeters
  MCDetectorConstruction* Detector = new MCDetectorConstruction(*Prototype);
  const bool Loaded = InitializeSilently(Detector);
  Passed = EvaluateTrue("Initialize()", "prototype", "The prototype geometry can be initialized", Loaded) && Passed;
  if (Loaded == true) {
    Passed = TestDetectors(Detector) && Passed;
  }
  SilenceOutput();
  delete Detector;
  RestoreOutput();

  // Geometry 2: the shapes
  Detector = new MCDetectorConstruction(*ShapesParameters);
  const bool ShapesLoaded = InitializeSilently(Detector);
  Passed = EvaluateTrue("Initialize()", "shapes", "The shapes geometry can be initialized", ShapesLoaded) && Passed;
  if (ShapesLoaded == true) {
    Passed = TestQueries(Detector) && Passed;
    Passed = TestShapes(Detector) && Passed;
    Passed = TestMaterials(Detector) && Passed;
    Passed = TestPlacements(Detector) && Passed;
    Passed = TestRegions(Detector) && Passed;
  }
  SilenceOutput();
  delete Detector;
  RestoreOutput();

  Passed = TestFailures() && Passed;

  SilenceOutput();
  delete m_Manager;
  delete Prototype;
  delete ShapesParameters;
  RestoreOutput();

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTDetectorConstruction::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTDetectorConstruction::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


void UTDetectorConstruction::InitializeGeant4()
{
  SilenceOutput();
  MCPhysicsListParticles Particles;
  Particles.ConstructParticle();
  G4IonConstructor().ConstructParticle();
  G4ParticleDefinition* GenericIon = G4GenericIon::Definition();
  if (GenericIon->GetProcessManager() == nullptr) GenericIon->SetProcessManager(new G4ProcessManager(GenericIon));
  G4ParticleTable::GetParticleTable()->SetReadiness();
  G4NuclideTable::GetInstance()->SetLevelTolerance(100*eV);
  G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0.0001*ns);
  RestoreOutput();
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::InitializeSilently(MCDetectorConstruction* Detector)
{
  SilenceOutput();
  const bool Result = Detector->Initialize();
  RestoreOutput();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


MCParameterFile* UTDetectorConstruction::CreateParameters(const MString& Name, const MString& Geometry, const MString& Additional)
{
  MString Content = MString("Version 1\nGeometry ") + Geometry + "\nDetectorTimeConstant 0.000005\nPhysicsListEM LivermorePol\n" + Additional;
  // The output name in the parameter file is relative to the private temporary root, which is the working directory of the test:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  Content += MString("Run Base\nBase.FileName Files/") + Name + "\nBase.Time 10\nBase.Source S\n";
  Content += "S.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n";
  const MString FileName = m_Directory + "/" + Name + ".source";
  if (WriteTextFile(FileName, Content) == false) return nullptr;

  SilenceOutput();
  MCParameterFile* Parameters = new MCParameterFile();
  bool Opened = Parameters->Open(FileName);
  RestoreOutput();
  if (Opened == false) {
    delete Parameters;
    return nullptr;
  }
  return Parameters;
}


////////////////////////////////////////////////////////////////////////////////


MString UTDetectorConstruction::WriteShapesGeometry(const MString& Name, const MString& Trap)
{
  MString Content;
  Content += "Name Shapes\nVersion 1.0\nSurroundingSphere 200 0 0 0 200\n";
  Content += "Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo\n\n";
  Content += "Trigger TN01\nTN01.Veto false\nTN01.TriggerByChannel true\nTN01.Detector DScint 1\n\n";
  Content += "Volume WorldVolume\nWorldVolume.Material Vacuum\nWorldVolume.Visibility 0\nWorldVolume.Shape BRIK 100 100 100\nWorldVolume.Mother 0\n\n";

  // Stand-alone shapes for the boolean volumes
  Content += "Shape BRIK SubA\nSubA.Parameters 3 3 3\nShape TUBS SubB\nSubB.Parameters 0 1 4 0 360\nShape BRIK UniA\nUniA.Parameters 2 2 2\nShape BRIK UniB\nUniB.Parameters 1 1 4\nShape BRIK IntA\nIntA.Parameters 3 3 3\nShape SPHE IntB\nIntB.Parameters 0 3.5\n";
  Content += "Orientation SubOri\nSubOri.Position 1 0.5 0\nSubOri.Rotation 0 0 90\n";
  Content += "Orientation UniOri\nUniOri.Position 0 0 3\nOrientation IntOri\nIntOri.Position 0.5 0 0\n\n";

  struct Shape { const char* Name; const char* Definition; double X; };
  const Shape List[] = {
    { "ShBox", "BRIK 1 2 3", 10 },
    { "ShTubs", "TUBS 1 3 5 30 120", 20 },
    { "ShSphe", "SPHE 1 4 20 100 10 200", 30 },
    { "ShCone", "CONE 5 1 2 3 4", 40 },
    { "ShCons", "CONS 5 1 2 3 4 30 120", 50 },
    { "ShTrd1", "TRD1 1 2 3 4", 60 },
    { "ShTrd2", "TRD2 1 2 3 4 5", 70 },
    { "ShTrap", Trap.Data(), 80 },
    { "ShPcon", "PCON 10 200 3 -2 0 2 0 1 3 3 0 1", -10 },
    { "ShPgon", "PGON 0 360 6 2 -1 0 2 1 0 2", -20 },
    { "ShSub", "Subtraction SubA SubB SubOri", -30 },
    { "ShUni", "Union UniA UniB UniOri", -40 },
    { "ShInt", "Intersection IntA IntB IntOri", -50 }
  };
  for (const Shape& S: List) {
    Content += MString("Volume ") + S.Name + "\n" + S.Name + ".Material Aluminium\n" + S.Name + ".Shape " + S.Definition + "\n";
    Content += MString(S.Name) + ".Position " + S.X + " 0 0\n" + S.Name + ".Mother WorldVolume\n\n";
  }

  // A copy and a rotated volume:
  Content += "ShBox.Copy ShBoxCopy\nShBoxCopy.Position 10 30 0\nShBoxCopy.Rotation 20 30 40\nShBoxCopy.Mother WorldVolume\n\n";
  Content += "Volume Rotated\nRotated.Material Aluminium\nRotated.Shape BRIK 1 2 3\nRotated.Position -10 30 5\nRotated.Rotation 15 25 35\nRotated.Mother WorldVolume\n\n";

  // A volume with a daughter - random positions in the mother must avoid the daughter:
  Content += "Volume ShHollow\nShHollow.Material Aluminium\nShHollow.Shape BRIK 5 5 5\nShHollow.Position 0 60 0\nShHollow.Mother WorldVolume\n\n";
  Content += "Volume ShHollowCore\nShHollowCore.Material Vacuum\nShHollowCore.Shape BRIK 4 4 4\nShHollowCore.Position 1 0 0\nShHollowCore.Mother ShHollow\n\n";

  // The detector:
  Content += "Volume Scint\nScint.Material Silicon\nScint.Shape BRIK 1 1 1\nScint.Position 0 -30 0\nScint.Mother WorldVolume\n\n";
  Content += "Scintillator DScint\nDScint.DetectorVolume Scint\nDScint.SensitiveVolume Scint\nDScint.NoiseThreshold 0.000001\nDScint.TriggerThreshold 0.000001\nDScint.EnergyResolution ideal\nDScint.FailureRate 0.0\n";

  const MString FileName = m_Directory + "/" + Name + ".geo.setup";
  WriteTextFile(FileName, Content);
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


G4LogicalVolume* UTDetectorConstruction::Logical(const G4String& Name)
{
  G4LogicalVolumeStore* Store = G4LogicalVolumeStore::GetInstance();
  for (G4LogicalVolume* L: *Store) {
    if (L != nullptr && L->GetName() == Name + "Log") return L;
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


G4VPhysicalVolume* UTDetectorConstruction::Physical(const G4String& Name)
{
  G4PhysicalVolumeStore* Store = G4PhysicalVolumeStore::GetInstance();
  for (G4VPhysicalVolume* P: *Store) {
    if (P != nullptr && P->GetName() == Name) return P;
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::TestConstruction()
{
  bool Passed = true;

  MCParameterFile* Parameters = CreateParameters("Construction", "$(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup", "");
  if (Parameters == nullptr) return EvaluateTrue("CreateParameters()", "construction", "The parameter file can be opened", false);

  // A constructed but not initialized object has no geometry (and can be destroyed):
  const size_t Size = sizeof(MCDetectorConstruction);
  vector<unsigned char> Buffer(Size + 64, 0xFF);
  void* Pointer = (void*) (((uintptr_t) Buffer.data() + 31) & ~((uintptr_t) 31));
  memset(Pointer, 0xFF, Size);
  MCDetectorConstruction* Detector = new (Pointer) MCDetectorConstruction(*Parameters);
  MDGeometryQuest* Geometry = Detector->GetGeometry();
  // Known failure: the constructor does not initialize m_Geometry, GetGeometry() returns whatever was in memory
  Passed = ExpectedToFail(EvaluateTrue("MCDetectorConstruction()", "memory pattern", "Before Initialize() there is no geometry", Geometry == nullptr)) && Passed;
  Passed = EvaluateTrue("Construct()", "no geometry", "Without an initialized geometry no world volume exists", DC(*Detector, MCDetectorConstruction, m_WorldVolume) == nullptr) && Passed;

  // The destructor deletes the geometry pointer, which must be valid:
  const bool Clean = UTSDHitShared::RunsCleanlyInChild([&]() {
    vector<unsigned char> Buffer2(Size + 64, 0xFF);
    void* Pointer2 = (void*) (((uintptr_t) Buffer2.data() + 31) & ~((uintptr_t) 31));
    memset(Pointer2, 0xFF, Size);
    MCDetectorConstruction* D = new (Pointer2) MCDetectorConstruction(*Parameters);
    D->~MCDetectorConstruction();
    return true;
  });
  // Known failure: the destructor deletes m_Geometry, which the constructor never initialized, and crashes on the garbage pointer
  Passed = ExpectedToFail(EvaluateTrue("~MCDetectorConstruction()", "never initialized", "An object which was never initialized can be destroyed", Clean)) && Passed;

  delete Parameters;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::TestDetectors(MCDetectorConstruction* Detector)
{
  bool Passed = true;

  G4SDManager* SDManager = G4SDManager::GetSDMpointer();
  G4VSensitiveDetector* Base = SDManager->FindSensitiveDetector("SStripSD", false);
  MC2DStripSD* Strip = dynamic_cast<MC2DStripSD*>(Base);
  Passed = EvaluateTrue("ConstructDetectors()", "strip", "The strip detector is registered under its name + SD", Strip != nullptr) && Passed;
  if (Strip != nullptr) {
    Passed = Evaluate("ConstructDetectors()", "strip", "The type is a 2D strip detector", Strip->GetType(), MCSD::c_2DStrip) && Passed;
    Passed = Evaluate("ConstructDetectors()", "strip", "The number of strips in x", DC(*Strip, MC2DStripSD, m_XNStrips), 128) && Passed;
    Passed = Evaluate("ConstructDetectors()", "strip", "The number of strips in y", DC(*Strip, MC2DStripSD, m_YNStrips), 128) && Passed;
    Passed = EvaluateNear("ConstructDetectors()", "strip", "The x size is half the width in cm (wafer half size 3.15 cm)", DC(*Strip, MC2DStripSD, m_XSize), 3.15*cm, 1e-6) && Passed;
    Passed = EvaluateNear("ConstructDetectors()", "strip", "The y size is half the width in cm", DC(*Strip, MC2DStripSD, m_YSize), 3.15*cm, 1e-6) && Passed;
    Passed = EvaluateNear("ConstructDetectors()", "strip", "The x offset is in cm", DC(*Strip, MC2DStripSD, m_XOffset), 0.142*cm, 1e-9) && Passed;
    Passed = EvaluateNear("ConstructDetectors()", "strip", "The y offset is in cm", DC(*Strip, MC2DStripSD, m_YOffset), 0.142*cm, 1e-9) && Passed;
    Passed = EvaluateNear("ConstructDetectors()", "strip", "The x pitch is the width divided by the strips, in cm", DC(*Strip, MC2DStripSD, m_XPitch), (6.3 - 2*0.142)/128*cm, 1e-4) && Passed;
    Passed = EvaluateFalse("ConstructDetectors()", "strip", "A 2D strip detector is not 3D", DC(*Strip, MC2DStripSD, m_Is3D)) && Passed;
    const G4ThreeVector Pitch = DC(*Strip, MCSD, m_StructuralPitch);
    const G4ThreeVector Offset = DC(*Strip, MCSD, m_StructuralOffset);
    Passed = EvaluateNear("ConstructDetectors()", "strip", "The structural pitch is in cm", (Pitch - G4ThreeVector(0.1*cm, 0.1*cm, 1.0*cm)).mag(), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("ConstructDetectors()", "strip", "The structural offset is in cm", (Offset - G4ThreeVector(4.2*cm, 4.2*cm, 0.0*cm)).mag(), 0.0, 1e-9) && Passed;
    const G4ThreeVector Dimension = DC(*Strip, MCSD, m_StructuralDimension);
    Passed = EvaluateNear("ConstructDetectors()", "strip", "The structural dimension is the size of the detector volume in cm", (Dimension - G4ThreeVector(11.7*cm, 11.7*cm, 0.025*cm)).mag(), 0.0, 1e-9) && Passed;
    Passed = Evaluate("ConstructDetectors()", "strip", "The detector volume is the logical volume of the detector", MString(DC(*Strip, MCSD, m_DetectorVolumeName).c_str()), MString("SiStripDetectorLog")) && Passed;
    Passed = EvaluateTrue("ConstructDetectors()", "strip", "Hits are discretized by default", DC(*Strip, MCSD, m_DiscretizeHits)) && Passed;
    Passed = EvaluateFalse("ConstructDetectors()", "strip", "The strip detector has no time resolution", DC(*Strip, MCSD, m_HasTimeResolution)) && Passed;
    Passed = EvaluateFalse("ConstructDetectors()", "strip", "The strip detector triggers", DC(*Strip, MCSD, m_IsNeverTriggering)) && Passed;

    G4LogicalVolume* Wafer = Logical("Wafer");
    Passed = EvaluateTrue("ConstructDetectors()", "strip", "The sensitive volume has the detector attached", Wafer != nullptr && Wafer->GetSensitiveDetector() == Strip) && Passed;
    G4LogicalVolume* Holder = Logical("SiStripDetector");
    Passed = EvaluateTrue("ConstructDetectors()", "strip", "The detector volume itself is not sensitive", Holder != nullptr && Holder->GetSensitiveDetector() == nullptr) && Passed;
  }

  // Calorimeters:
  const char* Names[3] = { "DCal2SD", "DCal4SD", "DCal8SD" };
  for (const char* N: Names) {
    MCCalorBarSD* Calorimeter = dynamic_cast<MCCalorBarSD*>(SDManager->FindSensitiveDetector(N, false));
    Passed = EvaluateTrue("ConstructDetectors()", N, "The calorimeter is registered", Calorimeter != nullptr) && Passed;
    if (Calorimeter != nullptr) {
      Passed = Evaluate("ConstructDetectors()", N, "The type is a calorimeter", Calorimeter->GetType(), MCSD::c_Calorimeter) && Passed;
    }
  }
  MCCalorBarSD* Calorimeter4 = dynamic_cast<MCCalorBarSD*>(SDManager->FindSensitiveDetector("DCal4SD", false));
  if (Calorimeter4 != nullptr) {
    const G4ThreeVector Pitch = DC(*Calorimeter4, MCSD, m_StructuralPitch);
    const G4ThreeVector Offset = DC(*Calorimeter4, MCSD, m_StructuralOffset);
    Passed = EvaluateNear("ConstructDetectors()", "DCal4", "The structural pitch is in cm", (Pitch - G4ThreeVector(0.07*cm, 0.07*cm, 0.0)).mag(), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("ConstructDetectors()", "DCal4", "The structural offset is in cm", (Offset - G4ThreeVector(0.235*cm, 0.235*cm, 0.415*cm)).mag(), 0.0, 1e-9) && Passed;
    G4LogicalVolume* Crystal = Logical("CsICrystal4");
    Passed = EvaluateTrue("ConstructDetectors()", "DCal4", "The crystals have the calorimeter attached", Crystal != nullptr && Crystal->GetSensitiveDetector() == Calorimeter4) && Passed;
  }

  // The event action knows all collections:
  const vector<G4String>& Strips = DC(*m_Action, MCEventAction, m_2DStripCollNames);
  const vector<G4String>& Calorimeters = DC(*m_Action, MCEventAction, m_CalorimeterCollNames);
  Passed = Evaluate("ConstructDetectors()", "collections", "The strip collection is registered in the event action", Strips.size(), (size_t) 1) && Passed;
  Passed = Evaluate("ConstructDetectors()", "collections", "The three calorimeter collections are registered in the event action", Calorimeters.size(), (size_t) 3) && Passed;
  Passed = EvaluateTrue("ConstructDetectors()", "collections", "The collection is called like the SD", Strips.size() == 1 && Strips[0] == "SStripSD") && Passed;

  // Queries on the prototype:
  Passed = EvaluateTrue("HasVolume()", "copy", "A copy of a volume exists", Detector->HasVolume("hermes")) && Passed;
  Passed = EvaluateTrue("IsValidVolume()", "copy", "A copy is a valid volume", Detector->IsValidVolume("hermes")) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "prototype", "The geometry is scanned", Detector->GetGeometry() != nullptr && Detector->GetGeometry()->IsScanned()) && Passed;
  Passed = EvaluateTrue("Initialize()", "trigger", "The trigger unit ignores vetoes and thresholds", Detector->GetGeometry()->GetTriggerUnit() != nullptr) && Passed;

  // Start area of the source: set from the geometry
  MCSource* Source = m_Manager->GetCurrentRun().GetSource(0);
  Passed = Evaluate("Initialize()", "start area", "The start area of the source is a sphere", Source->GetStartAreaType(), MCSource::c_StartAreaSphere) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::TestQueries(MCDetectorConstruction* Detector)
{
  bool Passed = true;

  Passed = EvaluateTrue("Construct()", "initialized", "The initialized detector returns the world volume", Detector->Construct() != nullptr && Detector->Construct() == DC(*Detector, MCDetectorConstruction, m_WorldVolume)) && Passed;
  Passed = Evaluate("Construct()", "world", "The world volume is called WorldVolume", MString(Detector->Construct()->GetName().c_str()), MString("WorldVolume")) && Passed;
  Passed = EvaluateTrue("Construct()", "world", "The world has no mother", Detector->Construct()->GetMotherLogical() == nullptr) && Passed;

  Passed = EvaluateTrue("HasVolume()", "existing", "An existing volume is found", Detector->HasVolume("ShBox")) && Passed;
  Passed = EvaluateFalse("HasVolume()", "missing", "A missing volume is not found", Detector->HasVolume("DoesNotExist")) && Passed;
  Passed = EvaluateTrue("HasVolume()", "copy", "A copy is found", Detector->HasVolume("ShBoxCopy")) && Passed;
  Passed = EvaluateTrue("IsValidVolume()", "existing", "An existing volume is valid", Detector->IsValidVolume("ShBox")) && Passed;
  Passed = EvaluateFalse("IsValidVolume()", "missing", "A missing volume is not valid", Detector->IsValidVolume("DoesNotExist")) && Passed;

  Passed = EvaluateTrue("IsInsideWorldVolume()", "origin", "The origin is inside", Detector->IsInsideWorldVolume(G4ThreeVector(0, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInsideWorldVolume()", "99 cm", "99 cm is inside", Detector->IsInsideWorldVolume(G4ThreeVector(99*cm, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInsideWorldVolume()", "101 cm", "101 cm is outside", Detector->IsInsideWorldVolume(G4ThreeVector(101*cm, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInsideWorldVolume()", "-101 cm z", "-101 cm in z is outside", Detector->IsInsideWorldVolume(G4ThreeVector(0, 0, -101*cm))) && Passed;
  Passed = EvaluateTrue("IsInsideWorldVolume()", "1 m in meters", "The unit is handled: 0.9 m is inside", Detector->IsInsideWorldVolume(G4ThreeVector(0.9*m, 0.9*m, 0.9*m))) && Passed;

  // Random positions in a volume are in the volume (world coordinates, Geant4 units)
  bool AllInside = true;
  for (unsigned int i = 0; i < 100; ++i) {
    G4ThreeVector P = Detector->GetRandomPosition("ShTrd1");
    if (fabs(P.x() - 60*cm) > 2*cm + 1e-9 || fabs(P.y()) > 3*cm + 1e-9 || fabs(P.z()) > 4*cm + 1e-9) AllInside = false;
  }
  Passed = EvaluateTrue("GetRandomPosition()", "ShTrd1", "Random positions are inside the volume, in Geant4 units", AllInside) && Passed;

  // Random positions in a mother volume are not inside its daughter volumes
  bool AllInMother = true;
  bool NoneInDaughter = true;
  for (unsigned int i = 0; i < 1000; ++i) {
    G4ThreeVector P = Detector->GetRandomPosition("ShHollow");
    if (fabs(P.x()) > 5*cm + 1e-9 || fabs(P.y() - 60*cm) > 5*cm + 1e-9 || fabs(P.z()) > 5*cm + 1e-9) AllInMother = false;
    if (fabs(P.x() - 1*cm) < 4*cm - 1e-9 && fabs(P.y() - 60*cm) < 4*cm - 1e-9 && fabs(P.z()) < 4*cm - 1e-9) NoneInDaughter = false;
  }
  Passed = EvaluateTrue("GetRandomPosition()", "ShHollow", "Random positions are inside the mother volume", AllInMother) && Passed;
  Passed = EvaluateTrue("GetRandomPosition()", "ShHollow", "Random positions are not inside the daughter volume", NoneInDaughter) && Passed;

  // Random positions in the daughter volume are inside the daughter
  bool AllInCore = true;
  for (unsigned int i = 0; i < 100; ++i) {
    G4ThreeVector P = Detector->GetRandomPosition("ShHollowCore");
    if (fabs(P.x() - 1*cm) > 4*cm + 1e-9 || fabs(P.y() - 60*cm) > 4*cm + 1e-9 || fabs(P.z()) > 4*cm + 1e-9) AllInCore = false;
  }
  Passed = EvaluateTrue("GetRandomPosition()", "ShHollowCore", "Random positions in a daughter are inside the daughter", AllInCore) && Passed;

  // Material hash
  G4Material* Aluminium = G4Material::GetMaterial("Aluminium", false);
  Passed = EvaluateTrue("GetMaterialHash()", "setup", "The material Aluminium exists in Geant4", Aluminium != nullptr) && Passed;
  if (Aluminium != nullptr) {
    const unsigned long Hash = Detector->GetMaterialHash(Aluminium);
    MDMaterial* M = Detector->GetGeometry()->GetMaterial("Aluminium");
    Passed = EvaluateTrue("GetMaterialHash()", "Aluminium", "The hash is the Geomega hash", M != nullptr && Hash == M->GetHash() && Hash != 0) && Passed;
  }
  G4Material* Unknown = new G4Material("UTDCUnknown", 1.0, 1.008*g/mole, 1.0*g/cm3);
  SilenceOutput();
  const unsigned long UnknownHash = Detector->GetMaterialHash(Unknown);
  RestoreOutput();
  Passed = Evaluate("GetMaterialHash()", "unknown material", "An unknown material has hash 0", UnknownHash, 0UL) && Passed;

  // The surrounding start sphere is set for the source:
  MCSource* Source = m_Manager->GetCurrentRun().GetSource(0);
  Passed = Evaluate("Initialize()", "start area", "The start area type of the source is a sphere", Source->GetStartAreaType(), MCSource::c_StartAreaSphere) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::TestShapes(MCDetectorConstruction*)
{
  bool Passed = true;

  auto Near = [](double A, double B) { return fabs(A - B) < 1e-6; };

  if (G4Box* S = Solid<G4Box>("ShBox")) {
    Passed = EvaluateTrue("ConstructVolumes()", "BRIK", "A box has the half lengths in cm", Near(S->GetXHalfLength(), 1*cm) && Near(S->GetYHalfLength(), 2*cm) && Near(S->GetZHalfLength(), 3*cm)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "BRIK", "A BRIK becomes a G4Box", false) && Passed;
  }

  if (G4Tubs* S = Solid<G4Tubs>("ShTubs")) {
    Passed = EvaluateTrue("ConstructVolumes()", "TUBS", "A tube has radii and half height in cm and the phi range", Near(S->GetInnerRadius(), 1*cm) && Near(S->GetOuterRadius(), 3*cm) && Near(S->GetZHalfLength(), 5*cm) && Near(S->GetStartPhiAngle(), 30*deg) && Near(S->GetDeltaPhiAngle(), 90*deg)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "TUBS", "A TUBS becomes a G4Tubs", false) && Passed;
  }

  if (G4Sphere* S = Solid<G4Sphere>("ShSphe")) {
    Passed = EvaluateTrue("ConstructVolumes()", "SPHE", "A sphere has radii in cm, theta 20-100 deg and phi 10-200 deg", Near(S->GetInnerRadius(), 1*cm) && Near(S->GetOuterRadius(), 4*cm) && Near(S->GetStartThetaAngle(), 20*deg) && Near(S->GetDeltaThetaAngle(), 80*deg) && Near(S->GetStartPhiAngle(), 10*deg) && Near(S->GetDeltaPhiAngle(), 190*deg)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "SPHE", "A SPHE becomes a G4Sphere", false) && Passed;
  }

  if (G4Cons* S = Solid<G4Cons>("ShCone")) {
    Passed = EvaluateTrue("ConstructVolumes()", "CONE", "A cone has the half height and the radii at -z and +z in cm, and a full circle", Near(S->GetZHalfLength(), 5*cm) && Near(S->GetInnerRadiusMinusZ(), 1*cm) && Near(S->GetOuterRadiusMinusZ(), 2*cm) && Near(S->GetInnerRadiusPlusZ(), 3*cm) && Near(S->GetOuterRadiusPlusZ(), 4*cm) && Near(S->GetDeltaPhiAngle(), 360*deg)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "CONE", "A CONE becomes a G4Cons", false) && Passed;
  }

  if (G4Cons* S = Solid<G4Cons>("ShCons")) {
    Passed = EvaluateTrue("ConstructVolumes()", "CONS radii", "A cone segment has the half height and the radii in cm", Near(S->GetZHalfLength(), 5*cm) && Near(S->GetInnerRadiusMinusZ(), 1*cm) && Near(S->GetOuterRadiusMinusZ(), 2*cm) && Near(S->GetInnerRadiusPlusZ(), 3*cm) && Near(S->GetOuterRadiusPlusZ(), 4*cm)) && Passed;
    Passed = EvaluateNear("ConstructVolumes()", "CONS phi 30-120 deg", "A cone segment starts at phi min", S->GetStartPhiAngle()/deg, 30.0, 1e-6) && Passed;
    // Known failure: ConstructVolumes() passes phi max (120 deg) as the delta phi of the G4Cons instead of phi max - phi min
    Passed = ExpectedToFail(EvaluateNear("ConstructVolumes()", "CONS phi 30-120 deg", "A cone segment spans phi max - phi min = 90 deg", S->GetDeltaPhiAngle()/deg, 90.0, 1e-6)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "CONS", "A CONS becomes a G4Cons", false) && Passed;
  }

  if (G4Trd* S = Solid<G4Trd>("ShTrd1")) {
    Passed = EvaluateTrue("ConstructVolumes()", "TRD1", "A TRD1 has dx1, dx2, y for both ends, and z in cm", Near(S->GetXHalfLength1(), 1*cm) && Near(S->GetXHalfLength2(), 2*cm) && Near(S->GetYHalfLength1(), 3*cm) && Near(S->GetYHalfLength2(), 3*cm) && Near(S->GetZHalfLength(), 4*cm)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "TRD1", "A TRD1 becomes a G4Trd", false) && Passed;
  }

  if (G4Trd* S = Solid<G4Trd>("ShTrd2")) {
    Passed = EvaluateTrue("ConstructVolumes()", "TRD2", "A TRD2 has dx1, dx2, dy1, dy2, z in cm", Near(S->GetXHalfLength1(), 1*cm) && Near(S->GetXHalfLength2(), 2*cm) && Near(S->GetYHalfLength1(), 3*cm) && Near(S->GetYHalfLength2(), 4*cm) && Near(S->GetZHalfLength(), 5*cm)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "TRD2", "A TRD2 becomes a G4Trd", false) && Passed;
  }

  if (G4Trap* S = Solid<G4Trap>("ShTrap")) {
    Passed = EvaluateTrue("ConstructVolumes()", "TRAP", "A trapezoid has the half lengths in cm", Near(S->GetZHalfLength(), 5*cm) && Near(S->GetYHalfLength1(), 4*cm) && Near(S->GetXHalfLength1(), 3*cm) && Near(S->GetXHalfLength2(), 2*cm) && Near(S->GetYHalfLength2(), 4*cm) && Near(S->GetXHalfLength3(), 3*cm) && Near(S->GetXHalfLength4(), 2*cm)) && Passed;
    Passed = EvaluateTrue("ConstructVolumes()", "TRAP angles", "A trapezoid has the angles in deg", Near(atan(S->GetTanAlpha1()), 5*deg) && Near(atan(S->GetTanAlpha2()), 5*deg) && Near(S->GetSymAxis().theta(), 10*deg) && Near(S->GetSymAxis().phi(), 20*deg)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "TRAP", "A TRAP becomes a G4Trap", false) && Passed;
  }

  if (G4Polycone* S = Solid<G4Polycone>("ShPcon")) {
    const G4PolyconeHistorical* P = S->GetOriginalParameters();
    Passed = EvaluateTrue("ConstructVolumes()", "PCON", "A polycone has the angles in deg and z, rmin, rmax in cm", P->Num_z_planes == 3 && Near(P->Start_angle, 10*deg) && Near(P->Opening_angle, 200*deg) && Near(P->Z_values[0], -2*cm) && Near(P->Z_values[2], 3*cm) && Near(P->Rmin[1], 1*cm) && Near(P->Rmax[0], 2*cm) && Near(P->Rmax[1], 3*cm) && Near(P->Rmax[2], 1*cm)) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "PCON", "A PCON becomes a G4Polycone", false) && Passed;
  }

  if (G4Polyhedra* S = Solid<G4Polyhedra>("ShPgon")) {
    // A hexagon with a distance to the sides of 2 cm has the area 6 * 2^2 * tan(30 deg), the height is 2 cm
    const double Expected = 2*cm * 6 * 2*cm * 2*cm * tan(30*deg);
    Passed = EvaluateNear("ConstructVolumes()", "PGON", "A hexagon with 2 cm distance to the sides has the volume of a hexagonal prism", S->GetCubicVolume()/cm3, Expected/cm3, 0.01) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "PGON", "A PGON becomes a G4Polyhedra", false) && Passed;
  }

  // Boolean volumes:
  if (G4SubtractionSolid* S = Solid<G4SubtractionSolid>("ShSub")) {
    Passed = EvaluateTrue("ConstructVolumes()", "Subtraction", "The subtraction has both constituents", S->GetConstituentSolid(0) != nullptr && S->GetConstituentSolid(1) != nullptr) && Passed;
    G4VSolid* A = S->GetConstituentSolid(0);
    Passed = EvaluateTrue("ConstructVolumes()", "Subtraction", "The minuend is a box of 3 cm half length", dynamic_cast<G4Box*>(A) != nullptr && Near(dynamic_cast<G4Box*>(A)->GetXHalfLength(), 3*cm)) && Passed;
    // The subtrahend is displaced by (1, 0.5, 0) cm: a point at (1, 0.5, 0) cm is in the tube axis - outside the subtraction; (-2, 0, 0) cm is inside
    Passed = Evaluate("ConstructVolumes()", "Subtraction", "A point on the axis of the displaced tube is outside of the solid", (int) S->Inside(G4ThreeVector(1*cm, 0.5*cm, 0)), (int) kOutside) && Passed;
    Passed = Evaluate("ConstructVolumes()", "Subtraction", "A point away from the tube is inside of the solid", (int) S->Inside(G4ThreeVector(-2*cm, 0, 0)), (int) kInside) && Passed;
    // The tube has radius 1 cm: a point 0.9 cm off its axis is outside, 1.5 cm off is inside
    Passed = Evaluate("ConstructVolumes()", "Subtraction", "A point 0.9 cm from the tube axis is cut out", (int) S->Inside(G4ThreeVector(1.9*cm, 0.5*cm, 0)), (int) kOutside) && Passed;
    Passed = Evaluate("ConstructVolumes()", "Subtraction", "A point 1.5 cm from the tube axis stays", (int) S->Inside(G4ThreeVector(2.5*cm, 0.5*cm, 0)), (int) kInside) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "Subtraction", "A Subtraction becomes a G4SubtractionSolid", false) && Passed;
  }

  if (G4UnionSolid* S = Solid<G4UnionSolid>("ShUni")) {
    // The second box (1, 1, 4) is shifted by 3 cm in z: it reaches z = 7 cm, the first box 2 cm
    Passed = Evaluate("ConstructVolumes()", "Union", "The addend reaches z = 6.9 cm", (int) S->Inside(G4ThreeVector(0, 0, 6.9*cm)), (int) kInside) && Passed;
    Passed = Evaluate("ConstructVolumes()", "Union", "Nothing is beyond z = 7.1 cm", (int) S->Inside(G4ThreeVector(0, 0, 7.1*cm)), (int) kOutside) && Passed;
    Passed = Evaluate("ConstructVolumes()", "Union", "The augend is kept", (int) S->Inside(G4ThreeVector(1.9*cm, 1.9*cm, 0)), (int) kInside) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "Union", "A Union becomes a G4UnionSolid", false) && Passed;
  }

  if (G4IntersectionSolid* S = Solid<G4IntersectionSolid>("ShInt")) {
    // The box (3 cm) intersects the sphere (3.5 cm) displaced by 0.5 cm in x
    Passed = Evaluate("ConstructVolumes()", "Intersection", "A point in both solids is inside", (int) S->Inside(G4ThreeVector(0, 0, 0)), (int) kInside) && Passed;
    Passed = Evaluate("ConstructVolumes()", "Intersection", "A point in the box and outside of the displaced sphere is outside: (-2.9, 0, 0) is 3.4 from the sphere centre (inside), (2.9, 2.9, 0) is outside", (int) S->Inside(G4ThreeVector(2.9*cm, 2.9*cm, 0)), (int) kOutside) && Passed;
    Passed = Evaluate("ConstructVolumes()", "Intersection", "A point at -3.2 cm in x is outside of the displaced sphere (3.7 cm away)", (int) S->Inside(G4ThreeVector(-3.2*cm, 0, 0)), (int) kOutside) && Passed;
  } else {
    Passed = EvaluateTrue("ConstructVolumes()", "Intersection", "An Intersection becomes a G4IntersectionSolid", false) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::TestMaterials(MCDetectorConstruction*)
{
  bool Passed = true;

  G4Material* Aluminium = G4Material::GetMaterial("Aluminium", false);
  Passed = EvaluateTrue("ConstructMaterials()", "Aluminium", "A used material exists", Aluminium != nullptr) && Passed;
  if (Aluminium != nullptr) {
    Passed = EvaluateNear("ConstructMaterials()", "Aluminium", "The density is in g/cm3", Aluminium->GetDensity()/(g/cm3), 2.7, 1e-6) && Passed;
    Passed = Evaluate("ConstructMaterials()", "Aluminium", "Aluminium has one element", (int) Aluminium->GetNumberOfElements(), 1) && Passed;
    Passed = EvaluateNear("ConstructMaterials()", "Aluminium", "The element is aluminium", Aluminium->GetElement(0)->GetZ(), 13.0, 1e-9) && Passed;
  }
  G4Material* Vacuum = G4Material::GetMaterial("Vacuum", false);
  Passed = EvaluateTrue("ConstructMaterials()", "Vacuum", "The vacuum of the world exists", Vacuum != nullptr) && Passed;
  G4Material* Silicon = G4Material::GetMaterial("Silicon", false);
  Passed = EvaluateTrue("ConstructMaterials()", "Silicon", "The silicon of the detector exists", Silicon != nullptr) && Passed;
  Passed = EvaluateTrue("ConstructMaterials()", "Germanium", "A material which is not used by any volume is not created", G4Material::GetMaterial("Germanium", false) == nullptr) && Passed;

  // The logical volumes have the material of the geometry
  G4LogicalVolume* Box = Logical("ShBox");
  Passed = EvaluateTrue("ConstructVolumes()", "material", "The logical volume has its material", Box != nullptr && Box->GetMaterial() == Aluminium) && Passed;
  G4LogicalVolume* World = Logical("WorldVolume");
  Passed = EvaluateTrue("ConstructVolumes()", "world", "The world has the vacuum", World != nullptr && World->GetMaterial() == Vacuum) && Passed;

  // A material with components by mass:
  G4Material* Air = G4Material::GetMaterial("Air", false);
  Passed = EvaluateTrue("ConstructMaterials()", "Air", "A material which is not used is not created even if it has several components", Air == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::TestPlacements(MCDetectorConstruction* Detector)
{
  bool Passed = true;

  auto Near = [](double A, double B) { return fabs(A - B) < 1e-6; };

  G4VPhysicalVolume* Box = Physical("ShBox");
  Passed = EvaluateTrue("PositionVolumes()", "ShBox", "The box is placed", Box != nullptr) && Passed;
  if (Box != nullptr) {
    Passed = EvaluateTrue("PositionVolumes()", "ShBox", "The box is placed at x = 10 cm", Near(Box->GetObjectTranslation().x(), 10*cm) && Near(Box->GetObjectTranslation().y(), 0) && Near(Box->GetObjectTranslation().z(), 0)) && Passed;
    Passed = EvaluateTrue("PositionVolumes()", "ShBox", "The mother of the box is the world", Box->GetMotherLogical() == Logical("WorldVolume")) && Passed;
    Passed = EvaluateTrue("PositionVolumes()", "ShBox", "The box is placed with its logical volume", Box->GetLogicalVolume() == Logical("ShBox")) && Passed;
  }

  // A copy uses the logical volume of the template:
  G4VPhysicalVolume* Copy = Physical("ShBoxCopy");
  Passed = EvaluateTrue("PositionVolumes()", "ShBoxCopy", "The copy is placed", Copy != nullptr) && Passed;
  if (Copy != nullptr) {
    Passed = EvaluateTrue("PositionVolumes()", "ShBoxCopy", "The copy uses the logical volume of the template", Copy->GetLogicalVolume() == Logical("ShBox")) && Passed;
    Passed = EvaluateTrue("PositionVolumes()", "ShBoxCopy", "The copy is placed at (10, 30, 0) cm", Near(Copy->GetObjectTranslation().x(), 10*cm) && Near(Copy->GetObjectTranslation().y(), 30*cm) && Near(Copy->GetObjectTranslation().z(), 0)) && Passed;
  }

  // The rotations: a local point transformed into the mother must give the same result as Geomega
  const char* Rotated[2] = { "Rotated", "ShBoxCopy" };
  for (const char* Name: Rotated) {
    G4VPhysicalVolume* P = Physical(Name);
    MDVolume* V = Detector->GetGeometry()->GetVolume(Name);
    if (P == nullptr || V == nullptr) {
      Passed = EvaluateTrue("PositionVolumes()", Name, "The volume exists in Geomega and Geant4", false) && Passed;
      continue;
    }
    const MVector Local(1.0, 2.0, 3.0);
    const MVector Expected = V->GetPositionInMotherVolume(Local);
    const G4ThreeVector Result = P->GetObjectRotationValue() * G4ThreeVector(1*cm, 2*cm, 3*cm) + P->GetObjectTranslation();
    const double Difference = (Result - G4ThreeVector(Expected[0]*cm, Expected[1]*cm, Expected[2]*cm)).mag();
    Passed = EvaluateNear("PositionVolumes()", Name, "A point transformed into the mother volume is at the same place as in Geomega", Difference/cm, 0.0, 1e-5) && Passed;

    // And a second point for the direction (x axis of the volume in the mother)
    const MVector Local2(1.0, 0.0, 0.0);
    const MVector Expected2 = V->GetPositionInMotherVolume(Local2);
    const G4ThreeVector Result2 = P->GetObjectRotationValue() * G4ThreeVector(1*cm, 0, 0) + P->GetObjectTranslation();
    Passed = EvaluateNear("PositionVolumes()", MString(Name) + " x", "The x axis of the volume is at the same place as in Geomega", (Result2 - G4ThreeVector(Expected2[0]*cm, Expected2[1]*cm, Expected2[2]*cm)).mag()/cm, 0.0, 1e-5) && Passed;
  }

  // Volume counts: every Geomega volume has a physical volume
  unsigned int NPhysical = 0;
  for (G4VPhysicalVolume* P: *G4PhysicalVolumeStore::GetInstance()) if (P != nullptr) ++NPhysical;
  Passed = EvaluateTrue("PositionVolumes()", "count", "Every Geomega volume is placed", NPhysical >= Detector->GetGeometry()->GetNVolumes()) && Passed;

  // The detector of the geometry: a scintillator on the volume Scint
  G4VSensitiveDetector* SD = G4SDManager::GetSDMpointer()->FindSensitiveDetector("DScintSD", false);
  Passed = EvaluateTrue("ConstructDetectors()", "scintillator", "The scintillator is registered", dynamic_cast<MCScintillatorSD*>(SD) != nullptr) && Passed;
  if (SD != nullptr) {
    Passed = EvaluateTrue("ConstructDetectors()", "scintillator", "The sensitive volume has the scintillator", Logical("Scint") != nullptr && Logical("Scint")->GetSensitiveDetector() == SD) && Passed;
    Passed = Evaluate("ConstructDetectors()", "scintillator", "The type is a scintillator", dynamic_cast<MCSD*>(SD)->GetType(), MCSD::c_Scintillator) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::TestRegions(MCDetectorConstruction*)
{
  bool Passed = true;

  G4Region* Region = nullptr;
  for (G4Region* R: *G4RegionStore::GetInstance()) {
    if (R != nullptr && R->GetName() == "TestRegion") Region = R;
  }
  Passed = EvaluateTrue("ConstructRegions()", "TestRegion", "The region of the parameter file exists", Region != nullptr) && Passed;
  if (Region != nullptr) {
    Passed = Evaluate("ConstructRegions()", "TestRegion", "The region has one root volume", (int) Region->GetNumberOfRootVolumes(), 1) && Passed;
    Passed = EvaluateTrue("ConstructRegions()", "TestRegion", "The root volume is the volume of the parameter file", Logical("ShBox") != nullptr && Logical("ShBox")->GetRegion() == Region) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDetectorConstruction::TestFailures()
{
  bool Passed = true;

  // A geometry which does not exist:
  {
    MCParameterFile* Parameters = CreateParameters("Missing", m_Directory + "/DoesNotExist.geo.setup", "");
    if (Parameters != nullptr) {
      MCDetectorConstruction* Detector = new MCDetectorConstruction(*Parameters);
      Passed = EvaluateFalse("Initialize()", "missing geometry", "A missing geometry file fails", InitializeSilently(Detector)) && Passed;
      SilenceOutput();
      delete Detector;
      delete Parameters;
      RestoreOutput();
    }
  }

  // A region with a missing volume:
  {
    const MString Shapes = WriteShapesGeometry("ShapesBadRegion");
    MCParameterFile* Parameters = CreateParameters("BadRegion", Shapes, "Region BadRegion\nBadRegion.Volume NoSuchVolume\nBadRegion.RangeCut 0.5\n");
    if (Parameters != nullptr) {
      MCDetectorConstruction* Detector = new MCDetectorConstruction(*Parameters);
      Passed = EvaluateFalse("Initialize()", "region with missing volume", "A region with a missing volume fails", InitializeSilently(Detector)) && Passed;
      SilenceOutput();
      delete Detector;
      delete Parameters;
      RestoreOutput();
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDetectorConstruction Test;
  return Test.Run() == true ? 0 : 1;
}
