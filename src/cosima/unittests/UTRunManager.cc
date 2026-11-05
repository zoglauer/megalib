/*
 * UTRunManager.cc
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
#include "MCActivator.hh"
#include "MCDetectorConstruction.hh"
#include "MCEventAction.hh"
#include "MCIsotopeStore.hh"
#include "MCParameterFile.hh"
#include "MCPhysicsListParticles.hh"
#include "MCRun.hh"
#include "MCRunManager.hh"
#include "MCSource.hh"
#include "MCSteppingAction.hh"
#include "MCTrackingAction.hh"

// MEGAlib:
#include "MFile.h"
#include "MGlobal.h"
#include "MString.h"
#include "MUnitTest.h"
#include "UTFixtureShared.h"

// Geant4:
#include "G4GenericIon.hh"
#include "G4IonConstructor.hh"
#include "G4NuclideTable.hh"
#include "G4ProcessManager.hh"
#include "G4ParticleTable.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

// ROOT:
#include "TError.h"

// Standard lib:
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;


//! Unit test class for MCRunManager
class UTRunManager : public MUnitTest
{
public:
  //! Default constructor
  UTRunManager() : MUnitTest("UTRunManager"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr), m_Parameters(nullptr) {}
  //! Default destructor
  virtual ~UTRunManager() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the singleton before the run manager exists
  bool TestSingletonBefore(bool& NoInstance);
  //! Test the access to the singleton and to the actions
  bool TestSingletonAndGetters(MCRunManager* Manager);
  //! Test the run list access
  bool TestRuns(MCRunManager* Manager);
  //! Test the preparation of the next run
  bool TestPrepareNextRun(MCRunManager* Manager);
  //! Test aborting a run which is not running
  bool TestAbortRun(MCRunManager* Manager);
  //! Test starting the beam for the activators
  bool TestStartBeamActivator(MCRunManager* Manager);
  //! Test the singleton after the run manager has been deleted
  bool TestSingletonAfter();

  //! Initialize the Geant4 particles
  void InitializeGeant4();
  //! Write the parameter file and open it
  bool CreateParameters();
  //! Redirect cout, cerr, and the MEGAlib streams
  void SilenceOutput();
  //! Restore cout, cerr, and the MEGAlib streams
  void RestoreOutput();
  //! Return true if the file exists and contains the text
  bool FileContains(const MString& FileName, const MString& Text);

  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The original cerr buffer while silenced
  streambuf* m_CerrBuffer;
  //! The sink for the silenced output
  ostringstream m_Sink;
  //! The parameter file
  MCParameterFile* m_Parameters;
  //! Name of the counts file
  MString m_CountsFile;
  //! Name of the activation file
  MString m_ActivationFile;
  //! Base name of the sim file of the run
  MString m_RunBaseName;
};


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::Run()
{
  bool Passed = true;

  // ROOT reports the creation of its geometry managers on stderr:
  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  bool NoInstance = false;
  Passed = TestSingletonBefore(NoInstance) && Passed;

  if (RequireFixture(*this, Passed, "src/cosima/unittests/GiganticSphere.geo.setup") == false) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return Passed;
  }

  Passed = EvaluateTrue("CreateParameters()", "parameter file", "The parameter file with a run and an activator can be opened", CreateParameters()) && Passed;
  if (m_Parameters == nullptr) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return false;
  }

  // Geant4 requires the run manager to exist before the first particle is registered, as in cosima:
  SilenceOutput();
  MCRunManager* Manager = new MCRunManager(*m_Parameters);
  RestoreOutput();
  InitializeGeant4();

  Passed = TestSingletonAndGetters(Manager) && Passed;
  Passed = TestRuns(Manager) && Passed;
  Passed = TestPrepareNextRun(Manager) && Passed;
  Passed = TestAbortRun(Manager) && Passed;
  Passed = TestStartBeamActivator(Manager) && Passed;

  // The run manager owns the event, tracking, and stepping action and the detector construction:
  SilenceOutput();
  delete Manager;
  RestoreOutput();
  Passed = TestSingletonAfter() && Passed;

  SilenceOutput();
  delete m_Parameters;
  RestoreOutput();

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTRunManager::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTRunManager::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


void UTRunManager::InitializeGeant4()
{
  SilenceOutput();
  MCPhysicsListParticles Particles;
  Particles.ConstructParticle();
  G4IonConstructor().ConstructParticle();
  // The run initialization of cosima gives the generic ion its process manager - without it Geant4 cannot create ions:
  G4ParticleDefinition* GenericIon = G4GenericIon::Definition();
  if (GenericIon->GetProcessManager() == nullptr) GenericIon->SetProcessManager(new G4ProcessManager(GenericIon));
  G4ParticleTable::GetParticleTable()->SetReadiness();
  // Same settings as in the cosima physics list:
  G4NuclideTable::GetInstance()->SetLevelTolerance(100*eV);
  G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0.0001*ns);
  RestoreOutput();
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::FileContains(const MString& FileName, const MString& Text)
{
  ifstream In(FileName.Data());
  if (In.is_open() == false) return false;
  string Line;
  while (getline(In, Line)) {
    if (MString(Line.c_str()).Contains(Text) == true) return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::CreateParameters()
{
  PrepareTemporaryDirectory("Files");
  const MString Directory = GetTemporaryDirectoryName("Files");
  m_CountsFile = Directory + "/Counts.dat";
  m_ActivationFile = Directory + "/Activation.dat";
  m_RunBaseName = Directory + "/UTRMRun";

  // Na24 - 10 counts in 10 s:
  if (WriteTextFile(m_CountsFile, "TT 10\nVN Volume\nRP 11024 0.0 10.0\nEN\n") == false) return false;

  MString Content = "Version 1\nGeometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\nDetectorTimeConstant 0.000005\nPhysicsListEM LivermorePol\nStoreSimulationInfo init\n";
  // The output name in the parameter file is relative to the private temporary root, which is the working directory of the test:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  Content += "Run Base\nBase.FileName Files/UTRMRun\nBase.Time 10\nBase.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n";
  Content += MString("Activator A\nA.IsotopeProductionFile ") + m_CountsFile + "\nA.ActivationMode ConstantIrradiation 3600\nA.ActivationFile " + m_ActivationFile + "\n";
  const MString FileName = Directory + "/Main.source";
  if (WriteTextFile(FileName, Content) == false) return false;

  SilenceOutput();
  m_Parameters = new MCParameterFile();
  bool Opened = m_Parameters->Open(FileName);
  RestoreOutput();
  if (Opened == false) {
    delete m_Parameters;
    m_Parameters = nullptr;
  }

  return Opened;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::TestSingletonBefore(bool& NoInstance)
{
  bool Passed = true;

  NoInstance = (MCRunManager::GetMCRunManager() == nullptr);
  Passed = EvaluateTrue("GetMCRunManager()", "before creation", "There is no run manager before one is created", NoInstance) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::TestSingletonAndGetters(MCRunManager* Manager)
{
  bool Passed = true;

  Passed = EvaluateTrue("GetMCRunManager()", "after creation", "The singleton is the created run manager", MCRunManager::GetMCRunManager() == Manager) && Passed;
  G4RunManager* Base = Manager;
  Passed = EvaluateTrue("MCRunManager()", "base class", "The manager is a Geant4 run manager which can be cast back", dynamic_cast<MCRunManager*>(Base) == Manager) && Passed;

  // No actions yet:
  Passed = EvaluateTrue("GetEventAction()", "not set", "Without an event action nullptr is returned", Manager->GetEventAction() == nullptr) && Passed;
  Passed = EvaluateTrue("GetTrackingAction()", "not set", "Without a tracking action nullptr is returned", Manager->GetTrackingAction() == nullptr) && Passed;
  Passed = EvaluateTrue("GetSteppingAction()", "not set", "Without a stepping action nullptr is returned", Manager->GetSteppingAction() == nullptr) && Passed;
  Passed = EvaluateTrue("GetPrimaryGeneratorAction()", "not set", "Without a primary generator action nullptr is returned", Manager->GetPrimaryGeneratorAction() == nullptr) && Passed;
  Passed = EvaluateTrue("GetDetectorConstruction()", "not set", "Without a detector construction nullptr is returned", Manager->GetDetectorConstruction() == nullptr) && Passed;
  Passed = EvaluateTrue("GetPhysicsList()", "not set", "Without a physics list nullptr is returned", Manager->GetPhysicsList() == nullptr) && Passed;

  // Set the actions the way MCMain does:
  SilenceOutput();
  MCEventAction* EventAction = new MCEventAction(*m_Parameters, false, 1, true);
  MCTrackingAction* TrackingAction = new MCTrackingAction();
  MCSteppingAction* SteppingAction = new MCSteppingAction(*m_Parameters);
  MCDetectorConstruction* Detector = new MCDetectorConstruction(*m_Parameters);
  Manager->SetUserAction(EventAction);
  Manager->SetUserAction(TrackingAction);
  Manager->SetUserAction(SteppingAction);
  const bool GeometryLoaded = Detector->Initialize();
  Manager->SetUserInitialization(Detector);
  RestoreOutput();

  Passed = EvaluateTrue("MCDetectorConstruction::Initialize()", "test geometry", "The test geometry can be loaded", GeometryLoaded) && Passed;
  Passed = EvaluateTrue("GetEventAction()", "set", "The event action is returned", Manager->GetEventAction() == EventAction) && Passed;
  Passed = EvaluateTrue("GetTrackingAction()", "set", "The tracking action is returned", Manager->GetTrackingAction() == TrackingAction) && Passed;
  Passed = EvaluateTrue("GetSteppingAction()", "set", "The stepping action is returned", Manager->GetSteppingAction() == SteppingAction) && Passed;
  Passed = EvaluateTrue("GetDetectorConstruction()", "set", "The detector construction is returned", Manager->GetDetectorConstruction() == Detector) && Passed;
  Passed = EvaluateTrue("GetPrimaryGeneratorAction()", "still not set", "The primary generator action is still not set", Manager->GetPrimaryGeneratorAction() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::TestRuns(MCRunManager* Manager)
{
  bool Passed = true;

  vector<MCRun>& Runs = Manager->GetRuns();
  Passed = EvaluateSize("GetRuns()", "one run", "The run list has the one run of the parameter file", Runs.size(), 1) && Passed;
  Passed = EvaluateTrue("GetRuns()", "same list", "The run list is the one of the parameter file", &Runs == &m_Parameters->GetRunList()) && Passed;
  Passed = Evaluate("GetCurrentRun()", "first run", "The current run is the first run", Manager->GetCurrentRun().GetName(), MString("Base")) && Passed;
  Passed = EvaluateTrue("GetCurrentRun()", "same run", "The current run is the one of the parameter file", &Manager->GetCurrentRun() == &m_Parameters->GetCurrentRun()) && Passed;
  Passed = EvaluateSize("GetCurrentRun()", "sources", "The current run has its source", Manager->GetCurrentRun().GetNSources(), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::TestPrepareNextRun(MCRunManager* Manager)
{
  bool Passed = true;

  // The event action of this test does not zip the file, and the file is empty until the first event is stored:
  const MString SimFile = m_RunBaseName + ".inc0.id1.sim";
  Passed = EvaluateFalse("PrepareNextRun()", "before", "The sim file of the run does not exist before the run is prepared", filesystem::exists(SimFile.Data())) && Passed;

  SilenceOutput();
  bool Prepared = Manager->PrepareNextRun();
  RestoreOutput();
  Passed = EvaluateTrue("PrepareNextRun()", "first run", "The first run can be prepared", Prepared) && Passed;
  Passed = EvaluateTrue("PrepareNextRun()", "first run", "The preparation opens the sim file of the run", filesystem::exists(SimFile.Data())) && Passed;
  // The sources of the run are initialized, i.e. their particle definitions exist:
  MCSource* Source = Manager->GetCurrentRun().GetSource(0);
  Passed = EvaluateTrue("PrepareNextRun()", "first run", "The run is initialized: its source has a particle definition", Source != nullptr && Source->GetParticleDefinition() != nullptr) && Passed;

  // Preparing again restarts the run:
  SilenceOutput();
  bool PreparedAgain = Manager->PrepareNextRun();
  RestoreOutput();
  Passed = EvaluateTrue("PrepareNextRun()", "again", "The run can be prepared a second time", PreparedAgain) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::TestAbortRun(MCRunManager* Manager)
{
  bool Passed = true;

  // Without a running simulation Geant4 ignores the request:
  SilenceOutput();
  Manager->AbortRun(false);
  Manager->AbortRun(true);
  RestoreOutput();
  Passed = EvaluateTrue("AbortRun()", "idle", "Aborting a run which is not running does not crash", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::TestStartBeamActivator(MCRunManager* Manager)
{
  bool Passed = true;

  // Only test the activators - the simulation of the runs needs a fully initialized Geant4:
  Manager->GetRuns().clear();
  Passed = Evaluate("StartBeam()", "no runs", "The run list is empty for the activator tests", (int) m_Parameters->GetNRuns(), 0) && Passed;
  Passed = Evaluate("GetNActivators()", "parameter file", "The parameter file has one activator", (int) m_Parameters->GetNActivators(), 1) && Passed;
  Passed = Evaluate("GetActivator()", "name", "The activator has the name of the parameter file", m_Parameters->GetActivator(0).GetName(), MString("A")) && Passed;

  // (1) A valid counts file: Na24 - 10 counts in 10 s = 1/s, activated for 1 h
  Passed = EvaluateFalse("StartBeam()", "before", "The activation file does not exist before the activator ran", MFile::Exists(m_ActivationFile)) && Passed;
  SilenceOutput();
  Manager->StartBeam();
  RestoreOutput();
  Passed = EvaluateTrue("StartBeam()", "valid counts", "The activator writes the activation file", MFile::Exists(m_ActivationFile)) && Passed;
  Passed = EvaluateTrue("StartBeam()", "valid counts", "The activation file contains the activated isotope Na24 (ID 11024)", FileContains(m_ActivationFile, "RP 11024")) && Passed;

  // (2) The activation file cannot be written: the failure is reported and the beam is not started
  Passed = EvaluateTrue("RemoveTemporaryFile()", "activation file", "The activation file can be removed", RemoveTemporaryFile(m_ActivationFile)) && Passed;
  const MString MissingDirectory = GetTemporaryDirectoryName("Files") + "/DoesNotExist/Activation.dat";
  m_Parameters->GetActivator(0).SetOutputFileName(MissingDirectory);
  SilenceOutput();
  Manager->StartBeam();
  RestoreOutput();
  Passed = EvaluateFalse("StartBeam()", "unwritable output", "Nothing is written if the activation file cannot be saved", MFile::Exists(MissingDirectory) || MFile::Exists(m_ActivationFile)) && Passed;

  // (3) A counts file without time makes the load fail: the activator does not calculate and does not save
  m_Parameters->GetActivator(0).SetOutputFileName(m_ActivationFile);
  const MString BadCountsFile = GetTemporaryDirectoryName("Files") + "/NoTime.dat";
  Passed = EvaluateTrue("WriteTextFile()", "bad counts", "The counts file without time can be written", WriteTextFile(BadCountsFile, "VN Volume\nRP 11024 0.0 10.0\nEN\n")) && Passed;
  SilenceOutput();
  bool Added = m_Parameters->GetActivator(0).AddCountsFile(BadCountsFile);
  Manager->StartBeam();
  RestoreOutput();
  Passed = EvaluateTrue("AddCountsFile()", "bad counts", "The counts file without time can be added", Added) && Passed;
  Passed = EvaluateFalse("StartBeam()", "counts without time", "An activator whose counts cannot be loaded does not write the activation file", MFile::Exists(m_ActivationFile)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRunManager::TestSingletonAfter()
{
  bool Passed = true;

  // The manager has been deleted - the singleton must not point to it anymore:
  // Known failure: the destructor of MCRunManager does not reset s_RunManager, the singleton pointer dangles after the deletion
  Passed = ExpectedToFail(EvaluateTrue("GetMCRunManager()", "after deletion", "After the run manager is deleted the singleton is nullptr instead of a dangling pointer", MCRunManager::GetMCRunManager() == nullptr)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTRunManager Test;
  return Test.Run() == true ? 0 : 1;
}
