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
#include "UTFixtureShared.h"

// Geant4:
#include "G4IonConstructor.hh"
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
  //! Test that a second call gives the same files
  bool TestRepeatedCall();
  //! Test a path which cannot be created
  bool TestFailure();

  //! Read one file
  XSFile ReadFile(const MString& FileName);
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
    Passed = EvaluateSize("CreateCrossSectionFiles()", m_Types[t], MString("The ") + m_Types[t] + " file has one value per energy bin: 401 energies from 1 keV to 100 GeV", m_Files[t].m_Energies.size(), 401) && Passed;
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
    if (F.m_Energies.size() != 401) continue;
    Passed = EvaluateNear("CreateCrossSectionFiles()", m_Types[t], "The first energy is 1 keV", F.m_Energies.front(), 1.0, 1e-5) && Passed;
    Passed = EvaluateNear("CreateCrossSectionFiles()", m_Types[t], "The last energy is 100 GeV = 1e8 keV", F.m_Energies.back()/1e8, 1.0, 1e-6) && Passed;
    // The grid is logarithmic - the ratio of neighboring bins is constant:
    const double Ratio = pow(1e8, 1.0/400.0);
    bool Logarithmic = true;
    for (unsigned int e = 1; e < F.m_Energies.size(); ++e) {
      if (fabs(F.m_Energies[e]/F.m_Energies[e-1]/Ratio - 1.0) > 1e-4) Logarithmic = false;
    }
    Passed = EvaluateTrue("CreateCrossSectionFiles()", m_Types[t], "The energy grid is logarithmic with 400 bins", Logarithmic) && Passed;
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
    if (m_Files[t].m_Values.size() != 401) return false;
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
  unsigned int Index1MeV = 0;
  for (unsigned int e = 0; e < Energies.size(); ++e) {
    if (fabs(log(Energies[e]/1000.0)) < fabs(log(Energies[Index1MeV]/1000.0))) Index1MeV = e;
  }
  const double ElectronDensity = 2.33 * 14.0/28.0855 * 6.02214076e23;
  const double Expected = 2.112e-25 * ElectronDensity * (Energies[Index1MeV] > 0 ? 1.0 : 0.0);
  Passed = EvaluateNear("CreateCrossSectionFiles()", "1 MeV", "The Compton cross section at 1 MeV agrees with Klein-Nishina within 3% (bin energy 1.0471 keV grid: bin within 2.4% of 1 MeV)", Compton[Index1MeV]/Expected, 1.0, 0.06) && Passed;

  // At low energies the photo effect dominates, at 1 MeV Compton:
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "10 keV", "The photo effect dominates at 10 keV", Photo[50] > Compton[50] && Photo[50] > Rayleigh[50]) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "1 MeV", "The Compton effect dominates at 1 MeV", Compton[Index1MeV] > Photo[Index1MeV] && Compton[Index1MeV] > Rayleigh[Index1MeV]) && Passed;

  // The photo effect falls between 10 keV and 100 keV:
  Passed = EvaluateTrue("CreateCrossSectionFiles()", "photo", "The photo absorption falls from 10 keV to 100 keV", Photo[50] > Photo[100]) && Passed;

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
