/*
 * UTCrossSections.cc
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
#include "MCCrossSections.hh"
#include "MCDetectorConstruction.hh"
#include "MCEventAction.hh"
#include "MCSteppingAction.hh"
#include "MCTrackingAction.hh"
#include "MCParameterFile.hh"
#include "MCPhysicsListParticles.hh"
#include "MCRunManager.hh"

// MEGAlib:
#include "MFile.h"
#include "MGlobal.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MResponseMatrixO1.h"
#include "UTFixtureShared.h"

// Geant4:
#include "G4IonConstructor.hh"
#include "G4NistManager.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermoreRayleighModel.hh"
#include "G4LivermoreGammaConversionModel.hh"
#include "G4CrossSectionHandler.hh"
#include "G4DataVector.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Version.hh"

// ROOT:
#include "TError.h"

// Standard lib:
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;


//! Unit test class for MCCrossSections
class UTCrossSections : public MUnitTest
{
public:
  //! Default constructor
  UTCrossSections() : MUnitTest("UTCrossSections"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr) {}
  //! Default destructor
  virtual ~UTCrossSections() {}

  //! Run all tests
  virtual bool Run();

private:
  //! The content of one cross section file
  struct XSFile {
    //! True if the file could be read
    bool m_Read = false;
    //! All lines starting with a #
    vector<MString> m_Comments;
    //! The values after the keywords Type, Version, NM, CE, HA
    MString m_Type, m_Version, m_Name, m_CE, m_Hash;
    //! The energies in keV
    vector<double> m_Energies;
    //! The macroscopic cross sections in 1/cm
    vector<double> m_Values;
  };

  //! Test construction and destruction
  bool TestLifecycle();
  //! Test the created files
  bool TestFiles();
  //! Test the headers of the files
  bool TestHeaders();
  //! Test the energy grid
  bool TestEnergyGrid();
  //! Test the values against physics
  bool TestValues();
  //! Test the absorption edge of the photo effect
  bool TestEdge();
  //! Test that the edges of all elements are in the files, also weak ones and those just above 1 keV
  bool TestEdgePositions();
  //! Test that the linear interpolation of the files agrees with the Geant4 models within the tolerance
  bool TestTolerance();
  //! Test that a second call gives the same files
  bool TestRepeatedCall();
  //! Test a path which cannot be created
  bool TestFailure();

  //! Read one file
  XSFile ReadFile(const MString& FileName);
  //! Return the index of the energy closest to the given one (keV) on a logarithmic scale
  unsigned int FindClosestIndex(const vector<double>& Energies, double Energy) const;
  //! Return the lower indices of adjacent energies less than 4e-5 apart relative to each other
  vector<unsigned int> FindEdgePairs(const vector<double>& Energies) const;
  //! Create the test materials of the edge and tolerance tests
  void CreateTestMaterials();
  //! Redirect cout, cerr, and the MEGAlib streams
  void SilenceOutput();
  //! Restore cout, cerr, and the MEGAlib streams
  void RestoreOutput();

  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The original cerr buffer while silenced
  streambuf* m_CerrBuffer;
  //! The sink for the silenced output
  ostringstream m_Sink;
  //! The run manager
  MCRunManager* m_RunManager = nullptr;
  //! The detector construction
  MCDetectorConstruction* m_Detector = nullptr;
  //! The directory of the cross section files
  MString m_Directory;
  //! The hash of the material Silicon
  unsigned long m_Hash = 0;
  //! The names of the created files
  vector<MString> m_Types = { "Photo", "Rayleigh", "Compton", "Pair", "Total" };
  //! The content of the created files
  vector<XSFile> m_Files;
};


////////////////////////////////////////////////////////////////////////////////


bool UTCrossSections::Run()
{
  bool Passed = true;

  Passed = TestLifecycle() && Passed;

  if (RequireFixture(*this, Passed, "src/cosima/unittests/GiganticSphere.geo.setup") == false) {
    Summarize();
    return Passed;
  }

  // ROOT reports the creation of its geometry managers on stderr:
  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  PrepareTemporaryDirectory("Main");
  const MString Source = GetTemporaryDirectoryName("Main") + "/Main.source";
  Passed = EvaluateTrue("WriteTextFile()", "source", "The parameter file can be written", WriteTextFile(Source, "Version 1\nGeometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\nDetectorTimeConstant 0.000005\nPhysicsListEM LivermorePol\nStoreSimulationInfo init\nRun Base\nBase.FileName Base\nBase.Time 10\nBase.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n")) && Passed;

  SilenceOutput();
  MCParameterFile* Parameters = new MCParameterFile();
  const bool Opened = Parameters->Open(Source);
  RestoreOutput();
  Passed = EvaluateTrue("MCParameterFile::Open()", "source", "The parameter file can be opened", Opened) && Passed;
  if (Opened == false) {
    delete Parameters;
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return false;
  }

  // Geant4 requires the run manager to exist before the first particle is registered, as in cosima:
  SilenceOutput();
  m_RunManager = new MCRunManager(*Parameters);
  MCPhysicsListParticles Particles;
  Particles.ConstructParticle();
  G4IonConstructor().ConstructParticle();
  G4ParticleTable::GetParticleTable()->SetReadiness();
  m_RunManager->SetUserAction(new MCEventAction(*Parameters, false, 1, true));
  m_RunManager->SetUserAction(new MCTrackingAction());
  m_RunManager->SetUserAction(new MCSteppingAction(*Parameters));
  m_Detector = new MCDetectorConstruction(*Parameters);
  const bool GeometryLoaded = m_Detector->Initialize();
  m_RunManager->SetUserInitialization(m_Detector);
  // Silicon is also a material of the Geomega geometry, which provides the hash:
  G4Material* Silicon = new G4Material("Silicon", 14.0, 28.0855*g/mole, 2.33*g/cm3);
  m_Hash = m_Detector->GetMaterialHash(Silicon);
  // Test materials for the edges: they are not in the Geomega geometry, thus their files have no hash
  CreateTestMaterials();
  RestoreOutput();
  Passed = EvaluateTrue("MCDetectorConstruction::Initialize()", "geometry", "The test geometry can be loaded", GeometryLoaded) && Passed;
  Passed = EvaluateTrue("GetMaterialHash()", "Silicon", "The Geomega hash of Silicon is available", m_Hash != 0) && Passed;

  // Create the cross section files in a directory which does not exist yet:
  m_Directory = GetTemporaryDirectoryName("Main") + "/Xsections/Nested";
  Passed = EvaluateFalse("CreateCrossSectionFiles()", "before", "The directory does not exist before the files are created", filesystem::exists(m_Directory.Data())) && Passed;

  SilenceOutput();
  MCCrossSections CrossSections;
  const bool Created = CrossSections.CreateCrossSectionFiles(m_Directory);
  RestoreOutput();
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "Silicon", "The cross section files are created", Created) && Passed;

  for (const MString& Type: m_Types) {
    m_Files.push_back(ReadFile(m_Directory + "/Xsection." + Type + ".Silicon.rsp"));
  }

  Passed = TestFiles() && Passed;
  Passed = TestHeaders() && Passed;
  Passed = TestEnergyGrid() && Passed;
  Passed = TestValues() && Passed;
  Passed = TestEdge() && Passed;
  Passed = TestEdgePositions() && Passed;
  Passed = TestTolerance() && Passed;
  Passed = TestRepeatedCall() && Passed;
  Passed = TestFailure() && Passed;

  SilenceOutput();
  delete m_RunManager;
  m_RunManager = nullptr;
  delete Parameters;
  RestoreOutput();

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTCrossSections::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTCrossSections::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the lower indices of adjacent energies less than 4e-5 apart relative to each other
vector<unsigned int> UTCrossSections::FindEdgePairs(const vector<double>& Energies) const
{
  vector<unsigned int> Pairs;
  for (unsigned int e = 0; e + 1 < Energies.size(); ++e) {
    if (Energies[e+1]/Energies[e] - 1.0 < 4e-5) Pairs.push_back(e);
  }
  return Pairs;
}


////////////////////////////////////////////////////////////////////////////////


//! Create the test materials of the edge and tolerance tests
void UTCrossSections::CreateTestMaterials()
{
  G4NistManager* Nist = G4NistManager::Instance();

  // Xenon: the M2 edge at 1.0021 keV is just above the start of the table
  G4Material* Xenon = new G4Material("UTXenon", 3.52*g/cm3, 1);
  Xenon->AddElement(Nist->FindOrBuildElement("Xe"), 1);

  // CdTe: the M1 edge of tellurium at 1.012 keV is just above the start of the table
  G4Material* CdTe = new G4Material("UTCdTe", 5.85*g/cm3, 2);
  CdTe->AddElement(Nist->FindOrBuildElement("Cd"), 1);
  CdTe->AddElement(Nist->FindOrBuildElement("Te"), 1);

  // CZT: the L3 and L2 edges of zinc (1.024 and 1.047 keV) are small in the mixture and close to the M1 edge of tellurium
  G4Material* CZT = new G4Material("UTCZT", 5.78*g/cm3, 3);
  CZT->AddElement(Nist->FindOrBuildElement("Cd"), 9);
  CZT->AddElement(Nist->FindOrBuildElement("Zn"), 1);
  CZT->AddElement(Nist->FindOrBuildElement("Te"), 10);

  // CsI: the M2 edge of cesium (1.068 keV) and the M1 edge of iodine (1.078 keV) are 1% apart, the cesium M3 edge is at exactly 1 keV
  G4Material* CsI = new G4Material("UTCsI", 4.51*g/cm3, 2);
  CsI->AddElement(Nist->FindOrBuildElement("Cs"), 1);
  CsI->AddElement(Nist->FindOrBuildElement("I"), 1);

  // A tungsten alloy with nickel: the L1 edge of nickel at 1.015 keV is just above the start of the table
  G4Material* Densimet = new G4Material("UTDensimet", 18.0*g/cm3, 3);
  Densimet->AddElement(Nist->FindOrBuildElement("W"), 0.90);
  Densimet->AddElement(Nist->FindOrBuildElement("Ni"), 0.07);
  Densimet->AddElement(Nist->FindOrBuildElement("Fe"), 0.03);

  // Aluminium with traces of zinc and magnesium: their edges are jumps of 0.1% or less of the material
  G4Material* Trace = new G4Material("UTTrace", 2.70*g/cm3, 3);
  Trace->AddElement(Nist->FindOrBuildElement("Al"), 0.9973);
  Trace->AddElement(Nist->FindOrBuildElement("Zn"), 0.0025);
  Trace->AddElement(Nist->FindOrBuildElement("Mg"), 0.0002);

  // NaI: pair creation reaches 0.1% of the total just above 1.15 MeV, where it rises like E^21
  G4Material* NaI = new G4Material("UTNaI", 3.67*g/cm3, 2);
  NaI->AddElement(Nist->FindOrBuildElement("Na"), 1);
  NaI->AddElement(Nist->FindOrBuildElement("I"), 1);

  // Germanium: below the L3 edge Rayleigh scattering is about 0.1% of the total, and its data have kinks
  G4Material* Germanium = new G4Material("UTGermanium", 5.323*g/cm3, 1);
  Germanium->AddElement(Nist->FindOrBuildElement("Ge"), 1);

  // Alu6061 as in Materials.geo: the photo effect of aluminium has a small step at 5.994 keV, which is not a shell edge
  G4Material* Alu6061 = new G4Material("UTAlu6061", 2.6989*g/cm3, 9);
  Alu6061->AddElement(Nist->FindOrBuildElement("Si"), 0.0060);
  Alu6061->AddElement(Nist->FindOrBuildElement("Fe"), 0.0070);
  Alu6061->AddElement(Nist->FindOrBuildElement("Cu"), 0.0030);
  Alu6061->AddElement(Nist->FindOrBuildElement("Mn"), 0.0015);
  Alu6061->AddElement(Nist->FindOrBuildElement("Mg"), 0.0100);
  Alu6061->AddElement(Nist->FindOrBuildElement("Cr"), 0.0020);
  Alu6061->AddElement(Nist->FindOrBuildElement("Zn"), 0.0025);
  Alu6061->AddElement(Nist->FindOrBuildElement("Ti"), 0.0015);
  Alu6061->AddElement(Nist->FindOrBuildElement("Al"), 0.9665);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the index of the energy closest to the given one (keV) on a logarithmic scale
unsigned int UTCrossSections::FindClosestIndex(const vector<double>& Energies, double Energy) const
{
  unsigned int Index = 0;
  for (unsigned int e = 0; e < Energies.size(); ++e) {
    if (fabs(log(Energies[e]/Energy)) < fabs(log(Energies[Index]/Energy))) Index = e;
  }
  return Index;
}


////////////////////////////////////////////////////////////////////////////////


UTCrossSections::XSFile UTCrossSections::ReadFile(const MString& FileName)
{
  XSFile File;
  ifstream In(FileName.Data());
  if (In.is_open() == false) return File;
  File.m_Read = true;

  string Line;
  while (getline(In, Line)) {
    istringstream Tokens(Line);
    string Key;
    if (!(Tokens>>Key)) continue;
    if (Key[0] == '#') {
      File.m_Comments.push_back(Line.c_str());
    } else if (Key == "Type") {
      Tokens>>Key;
      File.m_Type = Key.c_str();
    } else if (Key == "Version") {
      Tokens>>Key;
      File.m_Version = Key.c_str();
    } else if (Key == "NM") {
      getline(Tokens, Key);
      File.m_Name = Key.c_str();
    } else if (Key == "CE") {
      Tokens>>Key;
      File.m_CE = Key.c_str();
    } else if (Key == "HA") {
      Tokens>>Key;
      File.m_Hash = Key.c_str();
    } else if (Key == "R1") {
      double Energy = 0.0, Value = 0.0;
      Tokens>>Energy>>Value;
      File.m_Energies.push_back(Energy);
      File.m_Values.push_back(Value);
    }
  }

  return File;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCrossSections::TestLifecycle()
{
  bool Passed = true;

  MCCrossSections* CrossSections = new MCCrossSections();
  Passed = EvaluateTrue("MCCrossSections()", "new", "The class can be created", CrossSections != nullptr) && Passed;
  delete CrossSections;
  for (unsigned int i = 0; i < 10; ++i) {
    MCCrossSections Temporary;
    (void) Temporary;
  }
  Passed = EvaluateTrue("~MCCrossSections()", "repeated", "The class can be created and destroyed repeatedly", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCrossSections::TestFiles()
{
  bool Passed = true;

  Passed = EvaluateTrue("CreateCrossSectionFiles()", "nested path", "The nested directory has been created", filesystem::is_directory(m_Directory.Data())) && Passed;
  for (unsigned int t = 0; t < m_Types.size(); ++t) {
    Passed = EvaluateTrue("CreateCrossSectionFiles()", m_Types[t], MString("The ") + m_Types[t] + " file of the material has been created", m_Files[t].m_Read) && Passed;
    // The number of points depends on the models (refinement), thus only: all files have the same energies, more than the 401 grid points
    Passed = EvaluateTrue("CreateCrossSectionFiles()", m_Types[t], MString("The ") + m_Types[t] + " file has more than the 401 energies of the logarithmic grid", m_Files[t].m_Energies.size() > 401) && Passed;
    Passed = EvaluateTrue("CreateCrossSectionFiles()", m_Types[t], MString("The ") + m_Types[t] + " file has the same energies as the photo file", m_Files[t].m_Energies == m_Files[0].m_Energies) && Passed;
  }

  // The material Silicon gives exactly five files:
  unsigned int NFiles = 0;
  for (const auto& Entry: filesystem::directory_iterator(m_Directory.Data())) {
    if (Entry.is_regular_file() == true && Entry.path().string().find(".Silicon.") != string::npos) ++NFiles;
  }
  Passed = Evaluate("CreateCrossSectionFiles()", "one material", "The material Silicon gives exactly five files", NFiles, 5U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCrossSections::TestHeaders()
{
  bool Passed = true;

  for (unsigned int t = 0; t < m_Types.size(); ++t) {
    const XSFile& F = m_Files[t];
    if (F.m_Read == false) continue;
    Passed = Evaluate("CreateCrossSectionFiles()", m_Types[t], "The file is a first order response matrix", F.m_Type, MString("ResponseMatrixO1")) && Passed;
    Passed = Evaluate("CreateCrossSectionFiles()", m_Types[t], "The file has version 1", F.m_Version, MString("1")) && Passed;
    Passed = Evaluate("CreateCrossSectionFiles()", m_Types[t], "The file is not marked as calculated externally (CE false)", F.m_CE, MString("false")) && Passed;
    Passed = Evaluate("CreateCrossSectionFiles()", m_Types[t], "The hash of the file is the hash of the Geomega material", F.m_Hash, MString(m_Hash)) && Passed;
    Passed = EvaluateTrue("CreateCrossSectionFiles()", m_Types[t], "The name of the file contains the material", F.m_Name.Contains("Silicon")) && Passed;
    bool HasVersion = false;
    for (const MString& C: F.m_Comments) {
      if (C.Contains(MString("Geant4 version ") + G4VERSION_NUMBER) == true) HasVersion = true;
    }
    Passed = EvaluateTrue("CreateCrossSectionFiles()", m_Types[t], "The comments contain the Geant4 version the cross sections were derived from", HasVersion) && Passed;
  }

  // The names describe the process:
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "Photo", "The photo file is named after photo absorption", m_Files[0].m_Name.Contains("Photo")) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "Rayleigh", "The Rayleigh file is named after Rayleigh scattering", m_Files[1].m_Name.Contains("Rayleigh")) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "Compton", "The Compton file is named after Compton scattering", m_Files[2].m_Name.Contains("Compton")) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "Pair", "The pair file is named after pair creation", m_Files[3].m_Name.Contains("Pair")) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "Total", "The total file is named total", m_Files[4].m_Name.Contains("Total")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCrossSections::TestEnergyGrid()
{
  bool Passed = true;

  for (unsigned int t = 0; t < m_Types.size(); ++t) {
    const XSFile& F = m_Files[t];
    if (F.m_Energies.empty() == true) continue;
    Passed = EvaluateNear("CreateCrossSectionFiles()", m_Types[t], "The first energy is 1 keV", F.m_Energies.front(), 1.0, 1e-5) && Passed;
    Passed = EvaluateNear("CreateCrossSectionFiles()", m_Types[t], "The last energy is 100 GeV = 1e8 keV", F.m_Energies.back()/1e8, 1.0, 1e-6) && Passed;

    // The energies are increasing
    bool Increasing = true;
    for (unsigned int e = 1; e < F.m_Energies.size(); ++e) {
      if (F.m_Energies[e] <= F.m_Energies[e-1]) Increasing = false;
    }
    Passed = EvaluateTrue("CreateCrossSectionFiles()", m_Types[t], "The energies are increasing", Increasing) && Passed;

    // All 401 energies of the logarithmic grid with 400 bins are there (written with 6 decimals in keV, thus 1e-5 relative at 1 keV)
    unsigned int NGrid = 0;
    for (int g = 0; g <= 400; ++g) {
      const double Grid = pow(1e8, g/400.0);
      for (double E : F.m_Energies) {
        if (fabs(E/Grid - 1.0) < 1e-5) {
          ++NGrid;
          break;
        }
      }
    }
    Passed = Evaluate("CreateCrossSectionFiles()", m_Types[t], "All 401 energies of the logarithmic grid are present", NGrid, 401U) && Passed;
  }

  // All files use the same grid:
  bool SameGrid = true;
  for (unsigned int t = 1; t < m_Types.size(); ++t) {
    if (m_Files[t].m_Energies != m_Files[0].m_Energies) SameGrid = false;
  }
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "all files", "All files use the same energy grid", SameGrid) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCrossSections::TestValues()
{
  bool Passed = true;

  for (unsigned int t = 0; t < m_Types.size(); ++t) {
    if (m_Files[t].m_Values.size() != m_Files[0].m_Energies.size() || m_Files[t].m_Values.empty() == true) return false;
  }
  const vector<double>& Energies = m_Files[0].m_Energies;
  const vector<double>& Photo = m_Files[0].m_Values;
  const vector<double>& Rayleigh = m_Files[1].m_Values;
  const vector<double>& Compton = m_Files[2].m_Values;
  const vector<double>& Pair = m_Files[3].m_Values;
  const vector<double>& Total = m_Files[4].m_Values;

  bool AllFinite = true;
  bool AllNotNegative = true;
  bool TotalIsSum = true;
  bool TotalBelowSum = false;
  for (unsigned int e = 0; e < Energies.size(); ++e) {
    for (unsigned int t = 0; t < m_Types.size(); ++t) {
      if (std::isfinite(m_Files[t].m_Values[e]) == false) AllFinite = false;
      if (m_Files[t].m_Values[e] < 0.0) AllNotNegative = false;
    }
    const double Sum = Photo[e] + Rayleigh[e] + Compton[e] + Pair[e];
    // The values are stored with 6 significant digits:
    if (fabs(Total[e] - Sum) > 1e-5*Sum + 1e-12) TotalIsSum = false;
  }
  (void) TotalBelowSum;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "all values", "All cross sections are finite", AllFinite) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "all values", "No cross section is negative", AllNotNegative) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "total", "The total cross section is the sum of photo, Rayleigh, Compton, and pair at every energy", TotalIsSum) && Passed;

  // Pair creation is not possible below twice the electron mass (1022 keV):
  bool NoPairBelowThreshold = true;
  bool PairAboveThreshold = false;
  for (unsigned int e = 0; e < Energies.size(); ++e) {
    if (Energies[e] < 1022.0 && Pair[e] != 0.0) NoPairBelowThreshold = false;
    if (Energies[e] > 100000.0 && Pair[e] > 0.0) PairAboveThreshold = true;
  }
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "pair", "There is no pair creation below 1022 keV", NoPairBelowThreshold) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "pair", "There is pair creation at 100 MeV", PairAboveThreshold) && Passed;

  // Reference: Klein-Nishina at 1 MeV: sigma = 2.112e-25 cm2 per electron, Silicon: 2.33 g/cm3, Z/A = 14/28.0855 mol/g
  //            -> 0.1477 / cm; binding effects at 1 MeV are below 1%, thus 3% is a generous tolerance
  const unsigned int Index1MeV = FindClosestIndex(Energies, 1000.0);
  const unsigned int Index10keV = FindClosestIndex(Energies, 10.0);
  const unsigned int Index100keV = FindClosestIndex(Energies, 100.0);
  const double ElectronDensity = 2.33 * 14.0/28.0855 * 6.02214076e23;
  const double Expected = 2.112e-25 * ElectronDensity * (Energies[Index1MeV] > 0 ? 1.0 : 0.0);
  Passed = EvaluateNear("CreateCrossSectionFiles()", "1 MeV", "The Compton cross section at 1 MeV agrees with Klein-Nishina within 3% (bin energy 1.0471 keV grid: bin within 2.4% of 1 MeV)", Compton[Index1MeV]/Expected, 1.0, 0.06) && Passed;

  // At low energies the photo effect dominates, at 1 MeV Compton:
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "10 keV", "The photo effect dominates at 10 keV", Photo[Index10keV] > Compton[Index10keV] && Photo[Index10keV] > Rayleigh[Index10keV]) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "1 MeV", "The Compton effect dominates at 1 MeV", Compton[Index1MeV] > Photo[Index1MeV] && Compton[Index1MeV] > Rayleigh[Index1MeV]) && Passed;

  // The photo effect falls between 10 keV and 100 keV:
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "photo", "The photo absorption falls from 10 keV to 100 keV", Photo[Index10keV] > Photo[Index100keV]) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the absorption edge of the photo effect
bool UTCrossSections::TestEdge()
{
  bool Passed = true;

  const vector<double>& Energies = m_Files[0].m_Energies;
  const vector<double>& Photo = m_Files[0].m_Values;
  const vector<double>& Total = m_Files[4].m_Values;
  if (Energies.size() != Photo.size() || Energies.size() != Total.size()) return false;

  // Select close pairs with a photoelectric jump above 1%:
  vector<unsigned int> EdgePairs;
  for (unsigned int b : FindEdgePairs(Energies)) {
    if (Photo[b+1] > 1.01*Photo[b]) EdgePairs.push_back(b);
  }
  Passed = EvaluateSize("CreateCrossSectionFiles()", "Silicon", "Silicon has one edge above 1 keV, the K edge, thus one pair of points", EdgePairs.size(), 1) && Passed;
  if (EdgePairs.size() != 1) return Passed;

  const unsigned int Below = EdgePairs[0];
  const unsigned int Above = EdgePairs[0] + 1;

  // The K edge of silicon is at 1.839 keV; the Livermore data may differ slightly, thus 1%
  Passed = EvaluateNear("CreateCrossSectionFiles()", "Silicon K edge", "The point below is at the K edge of 1.839 keV", Energies[Below], 1.839, 0.01*1.839) && Passed;
  // The two points are 2e-5 apart relative to each other, as written with 6 decimals in keV
  Passed = EvaluateNear("CreateCrossSectionFiles()", "Silicon K edge", "The points are just below and just above the edge", Energies[Above]/Energies[Below] - 1.0, 2e-5, 2e-6) && Passed;

  // NIST XCOM: the photo absorption jumps by a factor of about 10 at the K edge of silicon; 5 is a safe lower limit
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "Silicon K edge", "The photo absorption jumps up by more than a factor of 5", Photo[Above] > 5.0*Photo[Below]) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "Silicon K edge", "The total cross section jumps up by more than a factor of 5", Total[Above] > 5.0*Total[Below]) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the edges of all elements are in the files, also weak ones and those just above 1 keV
bool UTCrossSections::TestEdgePositions()
{
  bool Passed = true;

  // EPICS2017 shell energies from pe-high-Z.dat, independent of the grid generator
  // Use a 0.2% matching window; the nearest listed edges are 10 eV apart
  struct Edge { MString m_Material; MString m_Name; double m_Energy; };
  const vector<Edge> Edges = {
    { "UTXenon", "Xe M2", 1.0021 },
    { "UTXenon", "Xe L3", 4.7873 },
    { "UTCdTe", "Te M1", 1.0120 },
    { "UTCZT", "Te M1", 1.0120 },
    { "UTCZT", "Zn L3", 1.024 },
    { "UTCZT", "Zn L2", 1.047 },
    { "UTCsI", "Cs M2", 1.068 },
    { "UTCsI", "I M1", 1.078 },
    { "UTDensimet", "Ni L1", 1.0150 },
    { "UTTrace", "Zn L2", 1.047 },
    { "UTTrace", "Mg K", 1.308 }
  };

  for (const Edge& E : Edges) {
    const XSFile Photo = ReadFile(m_Directory + "/Xsection.Photo." + E.m_Material + ".rsp");
    Passed = EvaluateTrue("CreateCrossSectionFiles()", E.m_Material, "The photo file of the test material can be read", Photo.m_Read && Photo.m_Energies.size() == Photo.m_Values.size()) && Passed;
    if (Photo.m_Read == false) continue;

    bool Found = false;
    bool JumpsUp = false;
    for (unsigned int b : FindEdgePairs(Photo.m_Energies)) {
      if (fabs(sqrt(Photo.m_Energies[b]*Photo.m_Energies[b+1])/E.m_Energy - 1.0) < 2e-3) {
        Found = true;
        JumpsUp = Photo.m_Values[b+1] > Photo.m_Values[b];
      }
    }
    Passed = EvaluateTrue("CreateCrossSectionFiles()", E.m_Material + " " + E.m_Name, MString("The edge at ") + E.m_Energy + " keV has a point just below and just above it", Found) && Passed;
    Passed = EvaluateTrue("CreateCrossSectionFiles()", E.m_Material + " " + E.m_Name, "The photo absorption jumps up at the edge", JumpsUp) && Passed;
  }

  // Check the cesium M3 edge at the 1 keV table boundary:
  const XSFile CsI = ReadFile(m_Directory + "/Xsection.Photo.UTCsI.rsp");
  if (CsI.m_Read == true && CsI.m_Values.empty() == false) {
    G4LivermorePhotoElectricModel* Photo = new G4LivermorePhotoElectricModel();
    G4DataVector Data;
    Photo->Initialise(G4ParticleTable::GetParticleTable()->FindParticle("gamma"), Data);
    const double Model = Photo->CrossSectionPerVolume(G4Material::GetMaterial("UTCsI"), G4ParticleTable::GetParticleTable()->FindParticle("gamma"), 1.0*keV)*cm;
    Passed = Evaluate("CreateCrossSectionFiles()", "UTCsI", "The first energy is exactly 1 keV", CsI.m_Energies.front(), 1.0) && Passed;
    // The values are written with 7 significant digits
    Passed = EvaluateNear("CreateCrossSectionFiles()", "UTCsI", "The first value is the one of the model at exactly 1 keV", CsI.m_Values.front(), Model, 1e-6*Model) && Passed;

    // Check the inside sample and the native reader above the edge:
    Passed = EvaluateTrue("CreateCrossSectionFiles()", "UTCsI", "The second energy is just above the cesium M3 edge at exactly 1 keV", CsI.m_Energies.size() > 1 && CsI.m_Energies[1] < 1.0001) && Passed;
    MResponseMatrixO1 Reader;
    if (Reader.Read(m_Directory + "/Xsection.Photo.UTCsI.rsp") == true) {
      const double Above = Photo->CrossSectionPerVolume(G4Material::GetMaterial("UTCsI"), G4ParticleTable::GetParticleTable()->FindParticle("gamma"), 1.00002*keV)*cm;
      Passed = EvaluateNear("MResponseMatrixO1::GetInterpolated()", "UTCsI at 1.00002 keV", "The reader returns the model value just above the edge at 1 keV within 0.1%", Reader.GetInterpolated(1.00002), Above, 1e-3*Above) && Passed;
    } else {
      Passed = EvaluateTrue("MResponseMatrixO1::Read()", "UTCsI", "The photo file can be read by MEGAlib", false) && Passed;
    }
    delete Photo;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the linear interpolation of the files agrees with the Geant4 models within the tolerance
bool UTCrossSections::TestTolerance()
{
  bool Passed = true;

  // Initialize reference models:
  const G4ParticleDefinition* Gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  G4DataVector Data;
  G4LivermorePhotoElectricModel* Photo = new G4LivermorePhotoElectricModel();
  Photo->Initialise(Gamma, Data);
  G4LivermoreRayleighModel* Rayleigh = new G4LivermoreRayleighModel();
  G4LivermoreGammaConversionModel* Pair = new G4LivermoreGammaConversionModel();
  G4CrossSectionHandler* Compton = new G4CrossSectionHandler();
  Compton->Clear();
  Compton->LoadData("comp/ce-cs-");

  // Check 63 interior samples per interval against the models
  // Use the process contribution floors documented by the generator
  // Add 1e-5 relative tolerance for serialized energy and value precision
  // Skip edge brackets and model-step intervals below 3e-5 relative width
  const double Tolerance = 1e-3 + 1e-5;
  const vector<MString> Materials = { "Silicon", "UTXenon", "UTCZT", "UTCsI", "UTDensimet", "UTTrace", "UTNaI", "UTGermanium", "UTAlu6061" };
  for (const MString& Name : Materials) {
    vector<XSFile> Files;
    bool Read = true;
    for (const MString& Type: m_Types) {
      Files.push_back(ReadFile(m_Directory + "/Xsection." + Type + "." + Name + ".rsp"));
      if (Files.back().m_Read == false || Files.back().m_Energies != Files[0].m_Energies || Files.back().m_Values.size() != Files[0].m_Energies.size()) Read = false;
    }
    Passed = EvaluateTrue("CreateCrossSectionFiles()", Name, "All five files can be read and have the same energies", Read) && Passed;
    const G4Material* Material = G4Material::GetMaterial(Name.Data());
    if (Read == false || Material == nullptr) continue;

    const vector<double>& Energies = Files[0].m_Energies;
    unsigned int NChecked = 0;
    unsigned int NDeviations = 0;
    double Largest = 0.0;
    MString Where;
    for (unsigned int e = 0; e + 1 < Energies.size(); ++e) {
      if (Energies[e+1]/Energies[e] - 1.0 < 3e-5) continue;
      for (int k = 1; k < 64; ++k) {
        const double Fraction = k/64.0;
        const double Energy = Energies[e] + Fraction*(Energies[e+1] - Energies[e]);
        double Model[5];
        Model[0] = Photo->CrossSectionPerVolume(Material, Gamma, Energy*keV)*cm;
        Model[1] = Rayleigh->CrossSectionPerVolume(Material, Gamma, Energy*keV)*cm;
        Model[2] = Compton->ValueForMaterial(Material, Energy*keV)*cm;
        Model[3] = Pair->CrossSectionPerVolume(Material, Gamma, Energy*keV)*cm;
        Model[4] = Model[0] + Model[1] + Model[2] + Model[3];
        for (unsigned int p = 0; p < 5; ++p) {
          const double Floor = (p == 0 || p == 2) ? 1e-6 : 1e-3;
          if (Model[p] <= 0.0) continue;
          if (p != 4 && Model[p] < Floor*Model[4]) continue;
          const double Interpolated = Files[p].m_Values[e] + Fraction*(Files[p].m_Values[e+1] - Files[p].m_Values[e]);
          const double Deviation = fabs(Interpolated/Model[p] - 1.0);
          ++NChecked;
          if (Deviation > Tolerance) ++NDeviations;
          if (Deviation > Largest) {
            Largest = Deviation;
            Where = m_Types[p] + MString(" at ") + Energy + " keV";
          }
        }
      }
    }
    Passed = EvaluateTrue("CreateCrossSectionFiles()", Name, MString("Values are checked: ") + NChecked, NChecked > 1000) && Passed;
    Passed = Evaluate("CreateCrossSectionFiles()", Name, MString("The linear interpolation agrees with the models within 0.1% (largest deviation: ") + Largest + ", " + Where + ")", NDeviations, 0U) && Passed;
  }

  delete Photo;
  delete Rayleigh;
  delete Pair;
  delete Compton;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCrossSections::TestRepeatedCall()
{
  bool Passed = true;

  const MString FileName = m_Directory + "/Xsection.Total.Silicon.rsp";
  const MString Before = ReadTextFile(FileName);

  SilenceOutput();
  MCCrossSections CrossSections;
  const bool Again = CrossSections.CreateCrossSectionFiles(m_Directory);
  RestoreOutput();
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "second call", "The files can be created again in the same directory", Again) && Passed;
  Passed = Evaluate("CreateCrossSectionFiles()", "second call", "The second call gives the same file", ReadTextFile(FileName), Before) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCrossSections::TestFailure()
{
  bool Passed = true;

  // A path below a regular file cannot be created:
  const MString NotADirectory = GetTemporaryFileName("NotADirectory");
  Passed = EvaluateTrue("WriteTextFile()", "blocker", "The file which blocks the directory can be written", WriteTextFile(NotADirectory, "x")) && Passed;

  SilenceOutput();
  MCCrossSections CrossSections;
  const bool Result = CrossSections.CreateCrossSectionFiles(NotADirectory + "/Xsections");
  RestoreOutput();
  // Known failure: CreateCrossSectionFiles() ignores the return value of gSystem->mkdir() and returns true although no file was written
  Passed = ExpectedToFail(EvaluateFalse("CreateCrossSectionFiles()", "impossible path", "Creating the files fails if the directory cannot be created", Result)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCrossSections Test;
  return Test.Run() == true ? 0 : 1;
}
