/*
 * UTPhysicsList.cc
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
#include "MCPhysicsList.hh"
#include "MCParameterFile.hh"
#include "MCRunManager.hh"
#include "UTSDHitShared.h"

// MEGAlib:
#include "MFile.h"
#include "MGlobal.h"
#include "MString.h"
#include "MUnitTest.h"
#include "UTFixtureShared.h"

// Geant4:
#include "G4EmParameters.hh"
#include "G4IonConstructor.hh"
#include "G4NuclideTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4ProductionCuts.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicsConstructor.hh"

// ROOT:
#include "TError.h"

// Standard lib:
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;


//! Unit test class for MCPhysicsList
class UTPhysicsList : public MUnitTest
{
public:
  //! Default constructor
  UTPhysicsList() : MUnitTest("UTPhysicsList"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr) {}
  //! Default destructor
  virtual ~UTPhysicsList() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constants
  bool TestConstants();
  //! Test the selection of the EM physics
  bool TestEMSelection();
  //! Test the selection of the hadron physics
  bool TestHDSelection();
  //! Test the cuts
  bool TestCuts();
  //! Test the processes
  bool TestConstructProcess();
  //! Test that the modification of the processes also works with minimal EM lists (in child processes)
  bool TestConstructProcessMinimalEM();

  //! Create a parameter file
  MCParameterFile* CreateParameters(const MString& Name, const MString& Lines);
  //! Return the names of all registered physics constructors
  vector<MString> GetNames(MCPhysicsList& List);
  //! Return true if any of the names contains the text
  bool HasName(const vector<MString>& Names, const MString& Text);

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
  //! The parameter files
  vector<MCParameterFile*> m_Parameters;
};


////////////////////////////////////////////////////////////////////////////////


void UTPhysicsList::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTPhysicsList::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


MCParameterFile* UTPhysicsList::CreateParameters(const MString& Name, const MString& Lines)
{
  PrepareTemporaryDirectory("Sources");
  const MString FileName = GetTemporaryDirectoryName("Sources") + "/" + Name + ".source";
  MString Content = "Version 1\nGeometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\nDetectorTimeConstant 0.000005\nStoreSimulationInfo init\n";
  Content += Lines;
  Content += "Run Base\nBase.FileName UTPLBase\nBase.Time 10\nBase.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n";
  if (WriteTextFile(FileName, Content) == false) return nullptr;

  SilenceOutput();
  MCParameterFile* Parameters = new MCParameterFile();
  const bool Opened = Parameters->Open(FileName);
  RestoreOutput();
  if (Opened == false) {
    delete Parameters;
    return nullptr;
  }
  m_Parameters.push_back(Parameters);
  return Parameters;
}


////////////////////////////////////////////////////////////////////////////////


vector<MString> UTPhysicsList::GetNames(MCPhysicsList& List)
{
  vector<MString> Names;
  for (G4int i = 0; ; ++i) {
    const G4VPhysicsConstructor* P = List.GetPhysics(i);
    if (P == nullptr) break;
    Names.push_back(P->GetPhysicsName().c_str());
  }
  return Names;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsList::HasName(const vector<MString>& Names, const MString& Text)
{
  for (const MString& N: Names) {
    if (N.Contains(Text) == true) return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsList::Run()
{
  bool Passed = true;

  Passed = TestConstants() && Passed;

  if (RequireFixture(*this, Passed, "src/cosima/unittests/GiganticSphere.geo.setup") == false) {
    Summarize();
    return Passed;
  }

  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  // Geant4 requires the run manager to exist before the first particle is registered, as in cosima:
  MCParameterFile* Base = CreateParameters("Base", "");
  Passed = EvaluateTrue("MCParameterFile::Open()", "base", "The base parameter file can be opened", Base != nullptr) && Passed;
  if (Base == nullptr) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return false;
  }
  SilenceOutput();
  m_RunManager = new MCRunManager(*Base);
  RestoreOutput();

  Passed = TestEMSelection() && Passed;
  Passed = TestHDSelection() && Passed;
  Passed = TestCuts() && Passed;
  // The children must run before the parent registers any particles:
  Passed = TestConstructProcessMinimalEM() && Passed;
  Passed = TestConstructProcess() && Passed;

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsList::TestConstants()
{
  bool Passed = true;

  Passed = Evaluate("MCPhysicsList", "c_EMNone", "The EM id for none is 0", MCPhysicsList::c_EMNone, 0) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_EMStandard", "The EM id for standard is 1", MCPhysicsList::c_EMStandard, 1) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_EMLivermore", "The EM id for Livermore is 2", MCPhysicsList::c_EMLivermore, 2) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_EMLivermorePolarized", "The EM id for polarized Livermore is 3", MCPhysicsList::c_EMLivermorePolarized, 3) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_EMLivermoreG4LECS", "The EM id for G4LECS is 4", MCPhysicsList::c_EMLivermoreG4LECS, 4) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_EMPenelope", "The EM id for Penelope is 5", MCPhysicsList::c_EMPenelope, 5) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_EMMin/Max", "The EM ids range from 0 to 5", MCPhysicsList::c_EMMax - MCPhysicsList::c_EMMin, 5) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_HDNone", "The HD id for none is 0", MCPhysicsList::c_HDNone, 0) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_HDMin/Max", "The HD ids range from 0 to 5", MCPhysicsList::c_HDMax - MCPhysicsList::c_HDMin, 5) && Passed;
  Passed = Evaluate("MCPhysicsList", "c_HD ids", "The HD ids are consecutive", MCPhysicsList::c_HDQGSP_BIC_HP + MCPhysicsList::c_HDQGSP_BERT_HP + MCPhysicsList::c_HDFTFP_BERT_HP + MCPhysicsList::c_HDFTFP_INCLXX_HP + MCPhysicsList::c_HDQGSP_INCLXX_HP, 15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsList::TestEMSelection()
{
  bool Passed = true;

  struct Case { int m_Id; const char* m_Key; const char* m_Expected; };
  const vector<Case> Cases = {
    { MCPhysicsList::c_EMStandard, "standard", "G4EmStandard" },
    { MCPhysicsList::c_EMLivermore, "Livermore", "G4EmLivermore" },
    { MCPhysicsList::c_EMLivermorePolarized, "LivermorePol", "G4EmLivermorePolarized" },
    { MCPhysicsList::c_EMLivermoreG4LECS, "", "Livermore" },
    { MCPhysicsList::c_EMPenelope, "Penelope", "G4EmPenelope" } };

  for (const Case& C: Cases) {
    // The id 4 is only reachable through the legacy "Package" keyword:
    const MString Line = (C.m_Id == MCPhysicsList::c_EMLivermoreG4LECS) ? MString("Package 4\n") : MString("PhysicsListEM ") + C.m_Key + "\n";
    MCParameterFile* P = CreateParameters(MString("EM") + C.m_Id, Line);
    if (P == nullptr) continue;
    SilenceOutput();
    MCPhysicsList* List = new MCPhysicsList(*P);
    RestoreOutput();
    const vector<MString> Names = GetNames(*List);
    Passed = Evaluate("MCPhysicsList()", MString("EM ") + C.m_Id, "The parameter file holds the requested EM id", P->GetPhysicsListEM(), C.m_Id) && Passed;
    // Known failure: Register() has no branch for c_EMLivermoreG4LECS (4), which "Package 4" accepts, and falls back to Penelope instead of Livermore
    if (C.m_Id == MCPhysicsList::c_EMLivermoreG4LECS) {
      Passed = ExpectedToFail(EvaluateTrue("Register()", MString("EM ") + C.m_Id, MString("The EM physics with id ") + C.m_Id + " is registered (" + C.m_Expected + ")", HasName(Names, C.m_Expected))) && Passed;
    } else {
      Passed = EvaluateTrue("Register()", MString("EM ") + C.m_Id, MString("The EM physics with id ") + C.m_Id + " is registered (" + C.m_Expected + ")", HasName(Names, C.m_Expected)) && Passed;
    }
    Passed = EvaluateTrue("Register()", MString("EM ") + C.m_Id, "The particles are registered", HasName(Names, "Particles") || Names.size() >= 3) && Passed;
    Passed = EvaluateTrue("Register()", MString("EM ") + C.m_Id, "The radioactive decay is always registered", HasName(Names, "RadioactiveDecay")) && Passed;
    if (C.m_Id == MCPhysicsList::c_EMLivermore) {
      Passed = EvaluateFalse("Register()", "Livermore", "Livermore does not register the polarized list", HasName(Names, "Polarized")) && Passed;
    }
    delete List;
  }

  // No EM and no hadrons: only particles and decay
  MCParameterFile* None = CreateParameters("EMNone", "PhysicsListEM none\n");
  if (None != nullptr) {
    SilenceOutput();
    MCPhysicsList List(*None);
    RestoreOutput();
    const vector<MString> Names = GetNames(List);
    Passed = Evaluate("Register()", "EM none", "Without EM and hadron physics only the particles and the decay are registered", Names.size(), 2UL) && Passed;
  }

  // The default cut:
  MCParameterFile* Cut = CreateParameters("Cut", "DefaultRangeCut 0.02\n");
  if (Cut != nullptr) {
    SilenceOutput();
    MCPhysicsList List(*Cut);
    RestoreOutput();
    Passed = EvaluateNear("MCPhysicsList()", "default cut", "The default cut is taken from the parameter file (0.02 cm = 0.2 mm)", List.GetDefaultCutValue()/mm, 0.2, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsList::TestHDSelection()
{
  bool Passed = true;

  const vector<MString> HD = { "none", "QGSP-BIC-HP", "QGSP-BERT-HP", "FTFP-BERT-HP", "FTFP-INCLXX-HP", "QGSP-INCLXX-HP" };
  for (unsigned int h = 0; h < HD.size(); ++h) {
    MCParameterFile* P = CreateParameters(MString("HD") + h, MString("PhysicsListEM Livermore\nPhysicsListHD ") + HD[h] + "\n");
    if (P == nullptr) {
      Passed = EvaluateTrue("MCParameterFile::Open()", HD[h], "The parameter file can be opened", false) && Passed;
      continue;
    }
    SilenceOutput();
    MCPhysicsList* List = new MCPhysicsList(*P);
    RestoreOutput();
    const vector<MString> Names = GetNames(*List);
    Passed = Evaluate("MCPhysicsList()", HD[h], "The parameter file holds the requested HD id", P->GetPhysicsListHD(), int(h)) && Passed;
    if (h == 0) {
      Passed = Evaluate("Register()", HD[h], "Without hadrons: particles, EM, and decay", Names.size(), 3UL) && Passed;
    } else {
      Passed = EvaluateTrue("Register()", HD[h], "Hadron physics adds constructors", Names.size() > 3) && Passed;
      Passed = EvaluateTrue("Register()", HD[h], "Hadronic inelastic physics is registered", HasName(Names, "inelastic") || HasName(Names, "Inelastic") || HasName(Names, "Binary") || HasName(Names, "INCL")) && Passed;
    }

    // Each EM type is registered only once, and the hadron list must not bring in a second one:
    unsigned int NEM = 0;
    for (const MString& N: Names) {
      if (N.BeginsWith("G4Em") == true && N.Contains("Extra") == false) ++NEM;
    }
    Passed = Evaluate("Register()", HD[h], "Exactly one EM physics is registered", NEM, 1U) && Passed;
    Passed = EvaluateTrue("Register()", HD[h], "The chosen Livermore physics is kept", HasName(Names, "G4EmLivermore")) && Passed;

    // No constructor is registered twice:
    bool Unique = true;
    for (unsigned int a = 0; a < Names.size(); ++a) {
      for (unsigned int b = a+1; b < Names.size(); ++b) {
        if (Names[a] == Names[b]) Unique = false;
      }
    }
    // Known failure: the HP lists already contain G4RadioactiveDecayPhysics, the duplicate check skips constructors of physics type 0, and Register() adds a second G4RadioactiveDecayPhysics
    if (h == 0) {
      Passed = EvaluateTrue("Register()", HD[h], "No physics constructor is registered twice", Unique) && Passed;
    } else {
      Passed = ExpectedToFail(EvaluateTrue("Register()", HD[h], "No physics constructor is registered twice", Unique)) && Passed;
    }
    delete List;
  }

  // "No EM package" with hadrons: the hadron lists bring in their own EM physics
  MCParameterFile* NoEM = CreateParameters("NoEMHD", "PhysicsListEM none\nPhysicsListHD QGSP_BIC_HP\n");
  if (NoEM != nullptr) {
    SilenceOutput();
    MCPhysicsList List(*NoEM);
    RestoreOutput();
    const vector<MString> Names = GetNames(List);
    Passed = EvaluateFalse("Register()", "no EM + HD", "c_EMNone means no EM package: no G4EmStandard is registered, even when hadron physics is chosen", HasName(Names, "G4EmStandard")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsList::TestCuts()
{
  bool Passed = true;

  MCParameterFile* P = CreateParameters("Cuts", "PhysicsListEM Livermore\nDefaultRangeCut 0.05\nRegion Foo\nFoo.Volume FooVolume\nFoo.RangeCut 0.01\n");
  if (P == nullptr) {
    return EvaluateTrue("MCParameterFile::Open()", "cuts", "The parameter file with a region can be opened", false);
  }
  Passed = Evaluate("GetRegionList()", "cuts", "The parameter file has one region", P->GetRegionList().size(), 1UL) && Passed;

  // The regions of the parameter file which do not exist in Geant4 must not crash (in a child):
  MCPhysicsList* ListForChild = nullptr;
  SilenceOutput();
  ListForChild = new MCPhysicsList(*P);
  RestoreOutput();
  const bool Clean = UTSDHitShared::RunsCleanlyInChild([&]() { ListForChild->SetCuts(); return true; });
  // Known failure: SetCuts() dereferences the null pointer which G4RegionStore::GetRegion() returns for a region that does not exist
  Passed = ExpectedToFail(EvaluateTrue("SetCuts()", "missing region", "SetCuts does not crash if a region of the parameter file does not exist in Geant4", Clean)) && Passed;
  delete ListForChild;

  // With the region present the cut is applied:
  G4Region* Region = new G4Region("Foo");
  SilenceOutput();
  MCPhysicsList List(*P);
  List.SetCuts();
  RestoreOutput();
  Passed = EvaluateNear("SetCuts()", "region cut", "The range cut of the region is 0.01 cm = 0.1 mm", Region->GetProductionCuts()->GetProductionCut("gamma")/mm, 0.1, 1e-9) && Passed;
  Passed = EvaluateNear("SetCuts()", "default cut", "The default cut is 0.05 cm = 0.5 mm", List.GetDefaultCutValue()/mm, 0.5, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsList::TestConstructProcessMinimalEM()
{
  bool Passed = true;

  const vector<MString> Lists = { "none", "standard" };
  for (const MString& EM: Lists) {
    MCParameterFile* P = CreateParameters(MString("Minimal") + EM, MString("PhysicsListEM ") + EM + "\n");
    if (P == nullptr) continue;
    SilenceOutput();
    MCPhysicsList* List = new MCPhysicsList(*P);
    RestoreOutput();
    const bool Clean = UTSDHitShared::RunsCleanlyInChild([&]() {
      List->ConstructParticle();
      G4IonConstructor().ConstructParticle();
      G4ParticleTable::GetParticleTable()->SetReadiness();
      List->Construct();
      return true;
    }, 60);
    Passed = EvaluateTrue("ConstructProcess()", EM, MString("ConstructProcess runs through with EM physics '") + EM + "' (no atomic de-excitation is available)", Clean) && Passed;
    delete List;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsList::TestConstructProcess()
{
  bool Passed = true;

  MCParameterFile* P = CreateParameters("Process", "PhysicsListEM Livermore\n");
  if (P == nullptr) return EvaluateTrue("MCParameterFile::Open()", "process", "The parameter file can be opened", false);

  SilenceOutput();
  MCPhysicsList* List = new MCPhysicsList(*P);
  List->ConstructParticle();
  G4IonConstructor().ConstructParticle();
  G4ParticleTable::GetParticleTable()->SetReadiness();
  List->Construct();
  RestoreOutput();

  G4ParticleTable* Table = G4ParticleTable::GetParticleTable();
  // Every particle has a process manager, and gamma has its EM processes:
  G4ProcessManager* Gamma = Table->FindParticle("gamma")->GetProcessManager();
  Passed = EvaluateTrue("ConstructProcess()", "gamma", "Gamma has a process manager", Gamma != nullptr) && Passed;
  if (Gamma != nullptr) {
    Passed = EvaluateTrue("ConstructProcess()", "gamma", "Gamma has photo effect, Compton, Rayleigh, and conversion (at least 4 processes plus transportation)", Gamma->GetProcessList()->size() >= 5) && Passed;
  }
  G4ProcessManager* Electron = Table->FindParticle("e-")->GetProcessManager();
  Passed = EvaluateTrue("ConstructProcess()", "e-", "The electron has ionisation, bremsstrahlung, and multiple scattering", Electron != nullptr && Electron->GetProcessList()->size() >= 4) && Passed;

  // Radioactive decay is attached to the ions:
  bool DecayFound = false;
  G4ProcessManager* Ion = Table->FindParticle("GenericIon")->GetProcessManager();
  if (Ion != nullptr) {
    for (G4int i = 0; i < int(Ion->GetProcessList()->size()); ++i) {
      if ((*Ion->GetProcessList())[i]->GetProcessName().find("adio") != string::npos) DecayFound = true;
    }
  }
  Passed = EvaluateTrue("ConstructProcess()", "GenericIon", "The generic ion has a radioactive decay process", DecayFound) && Passed;

  // Atomic de-excitation:
  G4EmParameters* Em = G4EmParameters::Instance();
  Passed = EvaluateTrue("ConstructProcess()", "fluo", "Fluorescence is on", Em->Fluo()) && Passed;
  Passed = EvaluateTrue("ConstructProcess()", "auger", "Auger is on", Em->Auger()) && Passed;
  Passed = EvaluateTrue("ConstructProcess()", "pixe", "PIXE is on", Em->Pixe()) && Passed;
  Passed = EvaluateFalse("ConstructProcess()", "ignore cut", "The de-excitation does not ignore the cuts", Em->DeexcitationIgnoreCut()) && Passed;

  // The nuclide table is configured for isomers:
  Passed = EvaluateNear("ConstructProcess()", "level tolerance", "The level tolerance of the nuclide table is 100 eV", G4NuclideTable::GetInstance()->GetLevelTolerance()/eV, 100.0, 1e-9) && Passed;
  Passed = EvaluateNear("ConstructProcess()", "half-life threshold", "The half-life threshold of the nuclide table is 0.0001 ns", G4NuclideTable::GetInstance()->GetThresholdOfHalfLife()/ns, 0.0001, 1e-9) && Passed;

  SilenceOutput();
  delete List;
  RestoreOutput();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTPhysicsList Test;
  return Test.Run() == true ? 0 : 1;
}
