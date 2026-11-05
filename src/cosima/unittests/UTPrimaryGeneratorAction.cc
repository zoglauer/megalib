/*
 * UTPrimaryGeneratorAction.cc
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
#include "MCPrimaryGeneratorAction.hh"
#include "MCDetectorConstruction.hh"
#include "MCEventAction.hh"
#include "MCParameterFile.hh"
#include "MCPhysicsListParticles.hh"
#include "MCRun.hh"
#include "MCRunManager.hh"
#include "MCSteppingAction.hh"
#include "MCTrackInformation.hh"
#include "MCTrackingAction.hh"
#include "UTFixtureShared.h"

// MEGAlib:
#include "MFile.h"
#include "MGlobal.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4DynamicParticle.hh"
#include "G4Event.hh"
#include "G4IonConstructor.hh"
#include "G4ParticleTable.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "Randomize.hh"

// ROOT:
#include "TError.h"
#include "TRandom.h"

// Standard lib:
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
using namespace std;


//! Unit test class for MCPrimaryGeneratorAction
class UTPrimaryGeneratorAction : public MUnitTest
{
public:
  //! Default constructor
  UTPrimaryGeneratorAction() : MUnitTest("UTPrimaryGeneratorAction"), m_CoutBuffer(nullptr), m_RunManager(nullptr) {}
  //! Default destructor
  virtual ~UTPrimaryGeneratorAction() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test construction and destruction
  bool TestLifecycle();
  //! Test the generation of one primary
  bool TestSinglePrimary();
  //! Test the counters and the emission time
  bool TestCounters();
  //! Test the isotropy of a point source
  bool TestIsotropy();
  //! Test two sources with different fluxes and particles
  bool TestTwoSources();
  //! Test the stop condition by time
  bool TestStopByTime();
  //! Test that the tracking action knows the number of generated particles
  bool TestTrackingActionIsInformed();

  //! Initialize the Geant4 particles
  void InitializeGeant4();
  //! Create the actions the run manager needs
  bool CreateActions(MCParameterFile& Parameters);
  //! Write a source file with the given runs and open it - returns nullptr if it does not work
  MCParameterFile* CreateParameters(const MString& Name, const MString& RunLines);
  //! Set the seeds of all random number generators
  void SetSeeds(long Seed);
  //! Redirect cout and the MEGAlib streams
  void SilenceOutput();
  //! Restore cout and the MEGAlib streams
  void RestoreOutput();

  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The sink for the silenced output
  ostringstream m_Sink;
  //! The run manager
  MCRunManager* m_RunManager;
};


////////////////////////////////////////////////////////////////////////////////


bool UTPrimaryGeneratorAction::Run()
{
  bool Passed = true;

  // ROOT reports the creation of its geometry managers on stderr:
  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  const bool HasGeometry = RequireFixture(*this, Passed, "src/cosima/unittests/GiganticSphere.geo.setup");
  MCParameterFile* Base = nullptr;
  if (HasGeometry == true) {
    // Geant4 requires the run manager to exist before the first particle is registered, as in cosima:
    Base = CreateParameters("Base", "Run Base\nBase.FileName UTPGABase\nBase.Time 100\nBase.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n");
    Passed = EvaluateTrue("MCParameterFile::Open()", "base run", "The base parameter file can be opened", Base != nullptr) && Passed;
    if (Base != nullptr) {
      SilenceOutput();
      m_RunManager = new MCRunManager(*Base);
      RestoreOutput();
    }
  }

  InitializeGeant4();

  Passed = TestLifecycle() && Passed;

  if (HasGeometry == true && m_RunManager != nullptr) {
    // The run manager owns the event, tracking, and stepping action:
    if (CreateActions(*Base) == true) {
      Passed = TestSinglePrimary() && Passed;
      Passed = TestCounters() && Passed;
      Passed = TestIsotropy() && Passed;
      Passed = TestTwoSources() && Passed;
      Passed = TestStopByTime() && Passed;
      Passed = TestTrackingActionIsInformed() && Passed;
    } else {
      Passed = EvaluateTrue("CreateActions()", "wiring", "The actions of the run manager can be created", false) && Passed;
    }
  }
  SilenceOutput();
  delete m_RunManager;
  m_RunManager = nullptr;
  delete Base;
  RestoreOutput();

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTPrimaryGeneratorAction::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTPrimaryGeneratorAction::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  m_CoutBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


void UTPrimaryGeneratorAction::SetSeeds(long Seed)
{
  CLHEP::HepRandom::setTheSeed(Seed);
  gRandom->SetSeed(Seed);
}


////////////////////////////////////////////////////////////////////////////////


void UTPrimaryGeneratorAction::InitializeGeant4()
{
  SilenceOutput();
  MCPhysicsListParticles Particles;
  Particles.ConstructParticle();
  G4IonConstructor().ConstructParticle();
  G4ParticleTable::GetParticleTable()->SetReadiness();
  RestoreOutput();
}


////////////////////////////////////////////////////////////////////////////////


MCParameterFile* UTPrimaryGeneratorAction::CreateParameters(const MString& Name, const MString& RunLines)
{
  PrepareTemporaryDirectory("Sources");
  const MString FileName = GetTemporaryDirectoryName("Sources") + "/" + Name + ".source";
  MString Content = "Version 1\nGeometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\nDetectorTimeConstant 0.000005\nPhysicsListEM LivermorePol\nStoreSimulationInfo init\n";
  Content += RunLines;
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


bool UTPrimaryGeneratorAction::CreateActions(MCParameterFile& Parameters)
{
  SilenceOutput();
  m_RunManager->SetUserAction(new MCEventAction(Parameters, false, 1, true));
  m_RunManager->SetUserAction(new MCTrackingAction());
  m_RunManager->SetUserAction(new MCSteppingAction(Parameters));
  // The sources check their positions against the world volume of the detector construction - the geometry is loaded, but Geant4 does not build it:
  MCDetectorConstruction* Detector = new MCDetectorConstruction(Parameters);
  const bool GeometryLoaded = Detector->Initialize();
  m_RunManager->SetUserInitialization(Detector);
  RestoreOutput();

  return GeometryLoaded == true && m_RunManager->GetEventAction() != nullptr && m_RunManager->GetTrackingAction() != nullptr && m_RunManager->GetSteppingAction() != nullptr && m_RunManager->GetDetectorConstruction() != nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPrimaryGeneratorAction::TestLifecycle()
{
  bool Passed = true;

  MCParameterFile Parameters;

  SilenceOutput();
  MCPrimaryGeneratorAction* Action = new MCPrimaryGeneratorAction(Parameters);
  RestoreOutput();
  G4VUserPrimaryGeneratorAction* Base = Action;
  Passed = EvaluateTrue("MCPrimaryGeneratorAction()", "base class", "The action is a Geant4 primary generator action which can be cast back", dynamic_cast<MCPrimaryGeneratorAction*>(Base) == Action) && Passed;

  SilenceOutput();
  delete Action;
  RestoreOutput();
  Passed = EvaluateTrue("~MCPrimaryGeneratorAction()", "delete", "The action can be deleted", true) && Passed;

  // Repeated creation - each action owns its own particle source:
  SilenceOutput();
  for (unsigned int i = 0; i < 10; ++i) {
    MCPrimaryGeneratorAction* Temporary = new MCPrimaryGeneratorAction(Parameters);
    delete Temporary;
  }
  RestoreOutput();
  Passed = EvaluateTrue("MCPrimaryGeneratorAction()", "10 actions", "Many actions can be created and destroyed one after the other", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPrimaryGeneratorAction::TestSinglePrimary()
{
  bool Passed = true;

  MCParameterFile* Parameters = CreateParameters("Single", "Run Single\nSingle.FileName UTPGASingle\nSingle.Time 1000\nSingle.Source S\nS.ParticleType 1\nS.Beam PointSource 1 2 3\nS.Spectrum Mono 511\nS.Flux 100\n");
  Passed = EvaluateTrue("MCParameterFile::Open()", "single source", "The parameter file with one point source can be opened", Parameters != nullptr) && Passed;
  if (Parameters == nullptr) return false;

  SetSeeds(101);
  SilenceOutput();
  bool Initialized = Parameters->GetCurrentRun().Initialize();
  MCPrimaryGeneratorAction* Action = new MCPrimaryGeneratorAction(*Parameters);
  G4Event* Event = new G4Event(0);
  Action->GeneratePrimaries(Event);
  RestoreOutput();
  Passed = EvaluateTrue("MCRun::Initialize()", "single source", "The run can be initialized", Initialized) && Passed;

  Passed = Evaluate("GeneratePrimaries()", "point source", "One event has exactly one primary vertex", Event->GetNumberOfPrimaryVertex(), 1) && Passed;
  G4PrimaryVertex* Vertex = Event->GetPrimaryVertex(0);
  Passed = EvaluateTrue("GeneratePrimaries()", "point source", "The primary vertex exists", Vertex != nullptr) && Passed;
  if (Vertex != nullptr) {
    Passed = Evaluate("GeneratePrimaries()", "point source", "The vertex has one primary particle", Vertex->GetNumberOfParticle(), 1) && Passed;
    G4PrimaryParticle* Primary = Vertex->GetPrimary();
    Passed = EvaluateTrue("GeneratePrimaries()", "point source", "The primary particle exists", Primary != nullptr) && Passed;
    if (Primary != nullptr) {
      Passed = Evaluate("GeneratePrimaries()", "photon", "The primary is a photon", string(Primary->GetG4code()->GetParticleName().c_str()), string("gamma")) && Passed;
      Passed = EvaluateNear("GeneratePrimaries()", "Mono 511", "The photon has the energy 511 keV", Primary->GetKineticEnergy()/keV, 511.0, 1e-6) && Passed;
      G4ThreeVector Direction = Primary->GetMomentumDirection();
      Passed = EvaluateNear("GeneratePrimaries()", "direction", "The direction is a unit vector", Direction.mag(), 1.0, 1e-9) && Passed;
    }
    Passed = EvaluateNear("GeneratePrimaries()", "PointSource 1 2 3", "The x position is 1 cm", Vertex->GetX0()/cm, 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GeneratePrimaries()", "PointSource 1 2 3", "The y position is 2 cm", Vertex->GetY0()/cm, 2.0, 1e-9) && Passed;
    Passed = EvaluateNear("GeneratePrimaries()", "PointSource 1 2 3", "The z position is 3 cm", Vertex->GetZ0()/cm, 3.0, 1e-9) && Passed;
  }

  SilenceOutput();
  delete Event;
  delete Action;
  delete Parameters;
  RestoreOutput();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPrimaryGeneratorAction::TestCounters()
{
  bool Passed = true;

  MCParameterFile* Parameters = CreateParameters("Counters", "Run Counters\nCounters.FileName UTPGACounters\nCounters.Time 100000\nCounters.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 662\nS.Flux 100\n");
  if (Parameters == nullptr) return EvaluateTrue("MCParameterFile::Open()", "counters", "The parameter file can be opened", false);

  SetSeeds(202);
  SilenceOutput();
  MCRun& Run = Parameters->GetCurrentRun();
  Run.Initialize();
  MCPrimaryGeneratorAction* Action = new MCPrimaryGeneratorAction(*Parameters);
  const int N = 200;
  vector<double> Times;
  int WithPrimary = 0;
  for (int i = 0; i < N; ++i) {
    G4Event* Event = new G4Event(i);
    Action->GeneratePrimaries(Event);
    if (Event->GetNumberOfPrimaryVertex() == 1) ++WithPrimary;
    Times.push_back(Run.GetSimulatedTime());
    delete Event;
  }
  RestoreOutput();

  Passed = Evaluate("GeneratePrimaries()", "200 events", "Every event has a primary", WithPrimary, N) && Passed;
  Passed = Evaluate("MCRun::GetNSimulatedEvents()", "200 events", "The run counts the simulated events", (int) Run.GetNSimulatedEvents(), N) && Passed;
  Passed = Evaluate("MCRun::GetNGeneratedParticles()", "200 events", "The run counts the generated particles", Run.GetNGeneratedParticles(), N) && Passed;

  bool Increasing = true;
  for (unsigned int i = 1; i < Times.size(); ++i) {
    if (Times[i] <= Times[i-1]) Increasing = false;
  }
  Passed = EvaluateTrue("GeneratePrimaries()", "200 events", "The simulated time increases with every event", Increasing) && Passed;
  // Mean emission rate 100 ph/s: 200 events take about 2 s (Poisson: 2 s +/- 0.14 s)
  Passed = EvaluateNear("GeneratePrimaries()", "200 events", "200 events at 100 ph/s take about 2 s", Times.back()/s, 2.0, 0.7) && Passed;

  SilenceOutput();
  delete Action;
  delete Parameters;
  RestoreOutput();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPrimaryGeneratorAction::TestIsotropy()
{
  bool Passed = true;

  MCParameterFile* Parameters = CreateParameters("Isotropy", "Run Isotropy\nIsotropy.FileName UTPGAIsotropy\nIsotropy.Time 100000\nIsotropy.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 662\nS.Flux 1000\n");
  if (Parameters == nullptr) return EvaluateTrue("MCParameterFile::Open()", "isotropy", "The parameter file can be opened", false);

  SetSeeds(303);
  SilenceOutput();
  Parameters->GetCurrentRun().Initialize();
  MCPrimaryGeneratorAction* Action = new MCPrimaryGeneratorAction(*Parameters);
  const int N = 400;
  double SumX = 0.0, SumY = 0.0, SumZ = 0.0;
  bool AllUnit = true;
  for (int i = 0; i < N; ++i) {
    G4Event* Event = new G4Event(i);
    Action->GeneratePrimaries(Event);
    if (Event->GetNumberOfPrimaryVertex() == 1) {
      G4ThreeVector Direction = Event->GetPrimaryVertex(0)->GetPrimary()->GetMomentumDirection();
      if (fabs(Direction.mag() - 1.0) > 1e-9) AllUnit = false;
      SumX += Direction.x();
      SumY += Direction.y();
      SumZ += Direction.z();
    }
    delete Event;
  }
  RestoreOutput();

  // Each component of an isotropic unit vector has the standard deviation 1/sqrt(3) - 400 events: 0.029, limit 5 sigma
  Passed = EvaluateTrue("GeneratePrimaries()", "400 events", "All directions are unit vectors", AllUnit) && Passed;
  Passed = EvaluateNear("GeneratePrimaries()", "400 events", "The mean x direction is about zero", SumX/N, 0.0, 0.15) && Passed;
  Passed = EvaluateNear("GeneratePrimaries()", "400 events", "The mean y direction is about zero", SumY/N, 0.0, 0.15) && Passed;
  Passed = EvaluateNear("GeneratePrimaries()", "400 events", "The mean z direction is about zero", SumZ/N, 0.0, 0.15) && Passed;

  SilenceOutput();
  delete Action;
  delete Parameters;
  RestoreOutput();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPrimaryGeneratorAction::TestTwoSources()
{
  bool Passed = true;

  MCParameterFile* Parameters = CreateParameters("Two", "Run Two\nTwo.FileName UTPGATwo\nTwo.Time 100000\nTwo.Source A\nA.ParticleType 1\nA.Beam PointSource 0 0 0\nA.Spectrum Mono 511\nA.Flux 300\nTwo.Source B\nB.ParticleType 3\nB.Beam PointSource 0 0 0\nB.Spectrum Mono 1000\nB.Flux 100\n");
  if (Parameters == nullptr) return EvaluateTrue("MCParameterFile::Open()", "two sources", "The parameter file with two sources can be opened", false);
  Passed = Evaluate("MCParameterFile::Open()", "two sources", "The run has two sources", (int) Parameters->GetCurrentRun().GetNSources(), 2) && Passed;

  SetSeeds(404);
  SilenceOutput();
  Parameters->GetCurrentRun().Initialize();
  MCPrimaryGeneratorAction* Action = new MCPrimaryGeneratorAction(*Parameters);
  const int N = 400;
  map<string, int> Counts;
  map<string, double> Energies;
  for (int i = 0; i < N; ++i) {
    G4Event* Event = new G4Event(i);
    Action->GeneratePrimaries(Event);
    if (Event->GetNumberOfPrimaryVertex() == 1) {
      G4PrimaryParticle* Primary = Event->GetPrimaryVertex(0)->GetPrimary();
      string Name = Primary->GetG4code()->GetParticleName().c_str();
      Counts[Name]++;
      Energies[Name] = Primary->GetKineticEnergy()/keV;
    }
    delete Event;
  }
  RestoreOutput();

  Passed = Evaluate("GeneratePrimaries()", "two sources", "All events have a primary", Counts["gamma"] + Counts["e-"], N) && Passed;
  // Fluxes 300:100 - 400 events: 300 +/- 8.7 photons, limit 5 sigma
  Passed = EvaluateNear("GeneratePrimaries()", "flux 300 : 100", "The photon source with the flux 300 produces three quarters of the events", (double) Counts["gamma"], 300.0, 45.0) && Passed;
  Passed = EvaluateNear("GeneratePrimaries()", "flux 300 : 100", "The electron source with the flux 100 produces one quarter of the events", (double) Counts["e-"], 100.0, 45.0) && Passed;
  Passed = EvaluateNear("GeneratePrimaries()", "photons", "The photons have the energy of their source", Energies["gamma"], 511.0, 1e-6) && Passed;
  Passed = EvaluateNear("GeneratePrimaries()", "electrons", "The electrons have the energy of their source", Energies["e-"], 1000.0, 1e-6) && Passed;

  SilenceOutput();
  delete Action;
  delete Parameters;
  RestoreOutput();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPrimaryGeneratorAction::TestStopByTime()
{
  bool Passed = true;

  MCParameterFile* Parameters = CreateParameters("Stop", "Run Stop\nStop.FileName UTPGAStop\nStop.Time 2\nStop.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 1000\n");
  if (Parameters == nullptr) return EvaluateTrue("MCParameterFile::Open()", "stop by time", "The parameter file can be opened", false);

  SetSeeds(505);
  SilenceOutput();
  MCRun& Run = Parameters->GetCurrentRun();
  Run.Initialize();
  MCPrimaryGeneratorAction* Action = new MCPrimaryGeneratorAction(*Parameters);
  int NWithPrimary = 0;
  bool Stopped = false;
  for (int i = 0; i < 6000; ++i) {
    G4Event* Event = new G4Event(i);
    Action->GeneratePrimaries(Event);
    bool HasPrimary = (Event->GetNumberOfPrimaryVertex() > 0);
    delete Event;
    if (HasPrimary == false) {
      Stopped = true;
      break;
    }
    ++NWithPrimary;
  }
  const bool Condition = Run.CheckStopConditions();
  RestoreOutput();

  Passed = EvaluateTrue("GeneratePrimaries()", "Time 2", "The generation stops when the run time is reached", Stopped) && Passed;
  Passed = EvaluateTrue("CheckStopConditions()", "Time 2", "The stop condition is fulfilled", Condition) && Passed;
  Passed = Evaluate("GetStopCondition()", "Time 2", "The run stops by time", Run.GetStopCondition(), (int) MCRun::c_StopByTime) && Passed;
  Passed = EvaluateNear("GetSimulatedTime()", "Time 2", "The simulated time is the duration of the run", Run.GetSimulatedTime()/s, 2.0, 1e-9) && Passed;
  // 2 s at 1000 ph/s: 2000 +/- 45 events, limit 5 sigma
  Passed = EvaluateNear("GeneratePrimaries()", "Time 2", "About 2000 events happen in 2 s at 1000 ph/s", (double) NWithPrimary, 2000.0, 225.0) && Passed;
  Passed = Evaluate("GetNSimulatedEvents()", "Time 2", "The run counts exactly the events which had primaries", (int) Run.GetNSimulatedEvents(), NWithPrimary) && Passed;

  SilenceOutput();
  delete Action;
  delete Parameters;
  RestoreOutput();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPrimaryGeneratorAction::TestTrackingActionIsInformed()
{
  bool Passed = true;

  MCParameterFile* Parameters = CreateParameters("Tracking", "Run Tracking\nTracking.FileName UTPGATracking\nTracking.Time 100000\nTracking.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n");
  if (Parameters == nullptr) return EvaluateTrue("MCParameterFile::Open()", "tracking", "The parameter file can be opened", false);

  SetSeeds(606);
  SilenceOutput();
  Parameters->GetCurrentRun().Initialize();
  MCPrimaryGeneratorAction* Action = new MCPrimaryGeneratorAction(*Parameters);
  G4Event* Event = new G4Event(0);
  Action->GeneratePrimaries(Event);

  // The tracking action assigns the ids to the primaries - one primary was generated:
  G4DynamicParticle* Particle = new G4DynamicParticle(G4ParticleTable::GetParticleTable()->FindParticle("gamma"), G4ThreeVector(0, 0, 1), 511*keV);
  G4Track* Track = new G4Track(Particle, 0.0, G4ThreeVector(0, 0, 0));
  m_RunManager->GetTrackingAction()->PreUserTrackingAction(Track);
  MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
  RestoreOutput();

  Passed = EvaluateTrue("GeneratePrimaries()", "tracking action", "The tracking action attaches information to the primary", Info != nullptr) && Passed;
  if (Info != nullptr) {
    Passed = Evaluate("GeneratePrimaries()", "tracking action", "The id of the only primary is the number of generated particles", Info->GetId(), 1) && Passed;
  }

  SilenceOutput();
  delete Track;
  delete Event;
  delete Action;
  delete Parameters;
  RestoreOutput();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTPrimaryGeneratorAction Test;
  return Test.Run() == true ? 0 : 1;
}
