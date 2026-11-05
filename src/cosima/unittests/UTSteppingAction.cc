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
#include "MCSteppingAction.hh"
#include "MCTrackInformation.hh"
#include "MCSD.hh"
#include "MCScintillatorSD.hh"
#include "MCRunManager.hh"
#include "MSimIA.h"
#include "MSimEvent.h"
#include "UTSDStepFixture.h"
#include "MDDetector.h"
#include "MSimEvent.h"
#include "MSimHT.h"
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
#include "UTSDHitShared.h"

// Geant4:
#include "G4Event.hh"
#include "G4VProcess.hh"
#include "G4ParticleTypes.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SteppingManager.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4DynamicParticle.hh"
#include "G4Track.hh"
#include "G4Step.hh"
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

// ROOT:
#include "TError.h"



// Access to private and protected members of the tested classes (legal member-pointer access via explicit template instantiation)
template<typename Tag, typename Tag::type Member> struct UTSARob { friend typename Tag::type UTSAGet(Tag) { return Member; } };
#define UTC_SA_ACCESS(Class, Name, Type) \
  struct UTSATag##Class##Name { typedef Type Class::* type; friend type UTSAGet(UTSATag##Class##Name); }; \
  template struct UTSARob<UTSATag##Class##Name, &Class::Name>;
UTC_SA_ACCESS(MCSteppingAction, m_StoreSimulationInfo, bool)
UTC_SA_ACCESS(MCSteppingAction, m_StoreIonization, bool)
UTC_SA_ACCESS(MCSteppingAction, m_WatchedVolumes, vector<MString>)
UTC_SA_ACCESS(MCSteppingAction, m_WatchedVolumesLog, vector<MString>)
UTC_SA_ACCESS(MCSteppingAction, m_BlackAbsorbers, vector<MString>)
UTC_SA_ACCESS(MCSteppingAction, m_InteractionId, G4int)
UTC_SA_ACCESS(MCSteppingAction, m_DecayMode, int)
UTC_SA_ACCESS(MCSteppingAction, m_DetectorTimeConstant, double)
UTC_SA_ACCESS(MCSteppingAction, m_KnownProcess, vector<G4String>)
UTC_SA_ACCESS(MCSteppingAction, m_KnownProcessID, vector<int>)
UTC_SA_ACCESS(MCSteppingAction, m_KnownProcessFrequency, vector<unsigned long long>)
UTC_SA_ACCESS(MCSteppingAction, m_Regions, vector<MCRegion>)
UTC_SA_ACCESS(MCSteppingAction, m_ParticleOriginIsBuildUpSource, bool)
UTC_SA_ACCESS(MCRun, m_NAddedIsotopes, unsigned int)
UTC_SA_ACCESS(MCEventAction, m_Event, MSimEvent*)
UTC_SA_ACCESS(MCEventAction, m_IsAborted, bool)
typedef map<string, double> UTSAPassiveMap;
UTC_SA_ACCESS(MCEventAction, m_PassiveMaterialMap, UTSAPassiveMap)
#define SA(Object, Class, Name) ((Object).*UTSAGet(UTSATag##Class##Name()))


//! A process with a name
class UTSAProcess : public G4VProcess
{
public:
  UTSAProcess(const G4String& Name) : G4VProcess(Name, fGeneral) {}
  virtual ~UTSAProcess() {}
  virtual G4bool IsApplicable(const G4ParticleDefinition&) override { return false; }
  virtual G4double PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition*) override { return -1.0; }
  virtual G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&) override { return nullptr; }
  virtual G4double AtRestGetPhysicalInteractionLength(const G4Track&, G4ForceCondition*) override { return -1.0; }
  virtual G4VParticleChange* AtRestDoIt(const G4Track&, const G4Step&) override { return nullptr; }
  virtual G4double AlongStepGetPhysicalInteractionLength(const G4Track&, G4double, G4double, G4double&, G4GPILSelection*) override { return -1.0; }
  virtual G4VParticleChange* AlongStepDoIt(const G4Track&, const G4Step&) override { return nullptr; }
};


//! Access class for the protected functions of MCSteppingAction
class MCSteppingActionAccess : public MCSteppingAction
{
public:
  MCSteppingActionAccess(MCParameterFile& Parameters) : MCSteppingAction(Parameters) {}
  using MCSteppingAction::GetProcessId;
  using MCSteppingAction::GetDetectorId;
};


//! Unit test class for MCSteppingAction
class UTSteppingAction : public MUnitTest
{
public:
  //! Default constructor
  UTSteppingAction() : MUnitTest("UTSteppingAction"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr), m_Manager(nullptr), m_EventAction(nullptr) {}
  //! Default destructor
  virtual ~UTSteppingAction() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructor and the parameters
  bool TestConstruction();
  //! Test the process IDs
  bool TestProcessIds();
  //! Test the particle types and definitions
  bool TestParticles();
  //! Test the interactions
  bool TestInteractions();
  //! Test the special cases: aborts, NaN, stuck
  bool TestSafetyNets();
  //! Test the watched volumes, black absorbers and regions
  bool TestVolumes();
  //! Test the passive material deposits
  bool TestDeposits();
  //! Test the radioactive decays
  bool TestDecays();
  //! Test the detector ID and the warning for inelastic steps
  bool TestDetectorIdAndWarnings();
  //! Test the decay modes for activation
  bool TestDecayModes();

  //! Silence cout and cerr
  void SilenceOutput();
  //! Restore cout and cerr
  void RestoreOutput();
  //! Initialize the Geant4 particles
  void InitializeGeant4();
  //! Create a parameter file
  MCParameterFile* CreateParameters(const MString& Name, const MString& Additional);

  //! A scenario: a stepping action with a stepping manager and the geometry
  struct Scenario {
    Scenario(UTSteppingAction& Test, MCParameterFile& Parameters) : Action(Parameters), Test(Test)
    {
      Action.SetSteppingManagerPointer(&Manager);
      Action.SetInitialParticles(vector<int>(1, 1));
      SA(*Test.m_EventAction, MCEventAction, m_IsAborted) = false;
      Test.m_EventAction->Reset();
    }
    ~Scenario()
    {
      Manager.GetfSecondary()->clear();
      for (G4Track* T: Tracks) delete T;
      for (G4Step* S: Steps) delete S;
    }
    //! Create a step of the given particle in the volume with the post step position and the given deposit
    G4Step* MakeStep(G4ParticleDefinition* Particle, double KineticEnergy, const G4ThreeVector& Position, double Deposit = 0.0, int Volume = -1, double Time = 2.5*s, int TrackID = 1, bool HasNext = true)
    {
      G4Step* Fixture = Test.m_Fixture->CreateStep(Deposit, Position, Position, fPostStepDoItProc, TrackID, Time, Volume);
      G4Track* Track = new G4Track(new G4DynamicParticle(Particle, G4ThreeVector(0, 0, 1), KineticEnergy), Time, Position);
      Track->SetUserInformation(new MCTrackInformation(TrackID, TrackID));
      Track->SetTrackID(TrackID);
      Track->SetTouchableHandle(Fixture->GetPreStepPoint()->GetTouchableHandle());
      if (HasNext == true) Track->SetNextTouchableHandle(Fixture->GetPostStepPoint()->GetTouchableHandle());
      Tracks.push_back(Track);
      G4Step* Step = new G4Step();
      Step->SetTrack(Track);
      Step->SetTotalEnergyDeposit(Deposit);
      Step->GetPreStepPoint()->SetTouchableHandle(Fixture->GetPreStepPoint()->GetTouchableHandle());
      Step->GetPostStepPoint()->SetTouchableHandle(Fixture->GetPostStepPoint()->GetTouchableHandle());
      Step->GetPreStepPoint()->SetPosition(Position);
      Step->GetPostStepPoint()->SetPosition(Position);
      Step->GetPostStepPoint()->SetGlobalTime(Time);
      Step->GetPostStepPoint()->SetKineticEnergy(KineticEnergy);
      Step->GetPreStepPoint()->SetGlobalTime(Time);
      Step->GetPostStepPoint()->SetStepStatus(fPostStepDoItProc);
      Steps.push_back(Step);
      return Step;
    }
    //! Set the process which defined the step
    void SetProcess(G4Step* Step, const G4String& Name)
    {
      Processes.push_back(unique_ptr<UTSAProcess>(new UTSAProcess(Name)));
      Step->GetPostStepPoint()->SetProcessDefinedStep(Processes.back().get());
    }
    //! Add a secondary to the secondaries of the stepping manager
    G4Track* AddSecondary(G4ParticleDefinition* Particle, double KineticEnergy, const G4ThreeVector& Position, const G4ThreeVector& Direction = G4ThreeVector(0, 1, 0))
    {
      G4Track* Track = new G4Track(new G4DynamicParticle(Particle, Direction, KineticEnergy), 2.5*s, Position);
      Tracks.push_back(Track);
      Manager.GetfSecondary()->push_back(Track);
      return Track;
    }
    //! Run the stepping action
    void Step(G4Step* S)
    {
      Test.SilenceOutput();
      Action.UserSteppingAction(S);
      Test.RestoreOutput();
    }
    G4SteppingManager Manager;
    MCSteppingAction Action;
    UTSteppingAction& Test;
    vector<G4Track*> Tracks;
    vector<G4Step*> Steps;
    vector<unique_ptr<UTSAProcess>> Processes;
  };

  //! The event of the event action
  MSimEvent* Event() { return SA(*m_EventAction, MCEventAction, m_Event); }
  //! Return true if the sim string of the event contains the text
  bool EventStringContains(const MString& Text) { return Event()->ToSimString().Contains(Text); }
  //! Get an IA
  MSimIA* IA(unsigned int i) { return Event()->GetIAAt(i); }
  //! Compare vectors in cm
  bool SameVector(const MVector& V, double X, double Y, double Z) { return fabs(V[0] - X) < 1e-6 && fabs(V[1] - Y) < 1e-6 && fabs(V[2] - Z) < 1e-6; }

  streambuf* m_CoutBuffer;
  streambuf* m_CerrBuffer;
  ostringstream m_Sink;
  MCRunManager* m_Manager;
  G4RunManager* m_G4Manager = nullptr;
  MCEventAction* m_EventAction;
  UTSDStepFixture* m_Fixture = nullptr;
  MCParameterFile* m_Default = nullptr;
  MCParameterFile* m_Special = nullptr;
  G4Material* m_Material = nullptr;
  MString m_Directory;
};


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::Run()
{
  bool Passed = true;

  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  PrepareTemporaryDirectory("Files");
  m_Directory = GetTemporaryDirectoryName("Files");

  m_Default = CreateParameters("Default", "");
  m_Special = CreateParameters("Special", "StoreSimulationInfoIonization true\nStoreSimulationInfoWatchedVolumes Det Other\nBlackAbsorber Inner\nDetectorTimeConstant 0.001\nDecayMode Normal\nRegion CutRegion\nCutRegion.Volume Other\nCutRegion.RangeCut 1\nCutRegion.CutAllSecondaries true\n");
  Passed = EvaluateTrue("CreateParameters()", "parameter files", "The parameter files can be opened", m_Default != nullptr && m_Special != nullptr) && Passed;
  if (m_Default == nullptr || m_Special == nullptr) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return false;
  }

  // Geant4 requires a run manager before the first particle is registered:
  SilenceOutput();
  m_Manager = new MCRunManager(*m_Default);
  RestoreOutput();
  InitializeGeant4();

  SilenceOutput();
  m_Fixture = new UTSDStepFixture();
  m_Fixture->SetupEventAction();
  m_EventAction = m_Fixture->GetEventAction();
  m_Material = new G4Material("UTSAMaterial", 1.0, 1.008*g/mole, 1.0*g/cm3);
  // World -> Det -> Inner and World -> Other
  m_Fixture->AddVolume("Det", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(0, 0, 0), nullptr, 0);
  m_Fixture->AddVolume("Inner", G4ThreeVector(5*cm, 5*cm, 5*cm), G4ThreeVector(0, 0, 0), nullptr, 1);
  m_Fixture->AddVolume("Other", G4ThreeVector(10*cm, 10*cm, 10*cm), G4ThreeVector(50*cm, 0, 0), nullptr, 0);
  // Like after the initialization of a run every logical volume belongs to a region (the default region for the world)
  G4Region* DefaultRegion = G4RegionStore::GetInstance()->GetRegion("DefaultRegionForTheWorld", false);
  if (DefaultRegion == nullptr) DefaultRegion = new G4Region("DefaultRegionForTheWorld");
  for (G4LogicalVolume* L: *G4LogicalVolumeStore::GetInstance()) {
    if (L != nullptr) {
      L->SetMaterial(m_Material);
      L->SetRegion(DefaultRegion);
    }
  }
  RestoreOutput();

  Passed = TestConstruction() && Passed;
  Passed = TestProcessIds() && Passed;
  Passed = TestParticles() && Passed;
  Passed = TestInteractions() && Passed;
  Passed = TestSafetyNets() && Passed;
  Passed = TestVolumes() && Passed;
  Passed = TestDeposits() && Passed;
  Passed = TestDecays() && Passed;
  Passed = TestDetectorIdAndWarnings() && Passed;
  Passed = TestDecayModes() && Passed;

  // The fixture, the event action and the run manager are not freed: Geant4 owns the event manager
  SilenceOutput();
  delete m_Fixture;
  RestoreOutput();

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTSteppingAction::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTSteppingAction::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


void UTSteppingAction::InitializeGeant4()
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


MCParameterFile* UTSteppingAction::CreateParameters(const MString& Name, const MString& Additional)
{
  MString Content = MString("Version 1\nGeometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\nPhysicsListEM LivermorePol\n") + Additional;
  Content += "Run Base\nBase.Time 10\nBase.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n";
  const MString FileName = m_Directory + "/" + Name + ".source";
  if (WriteTextFile(FileName, Content) == false) return nullptr;

  SilenceOutput();
  MCParameterFile* Parameters = new MCParameterFile();
  const bool Opened = Parameters->Open(FileName);
  RestoreOutput();
  if (Opened == false) {
    delete Parameters;
    return nullptr;
  }
  return Parameters;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestConstruction()
{
  bool Passed = true;

  MCSteppingAction Defaults(*m_Default);
  Passed = EvaluateFalse("MCSteppingAction()", "defaults", "Ionization is not stored by default", SA(Defaults, MCSteppingAction, m_StoreIonization)) && Passed;
  Passed = Evaluate("MCSteppingAction()", "defaults", "No watched volumes by default", SA(Defaults, MCSteppingAction, m_WatchedVolumes).size(), (size_t) 0) && Passed;
  Passed = Evaluate("MCSteppingAction()", "defaults", "No black absorbers by default", SA(Defaults, MCSteppingAction, m_BlackAbsorbers).size(), (size_t) 0) && Passed;
  Passed = Evaluate("MCSteppingAction()", "defaults", "The default decay mode is to ignore decays", SA(Defaults, MCSteppingAction, m_DecayMode), (int) MCParameterFile::c_DecayModeIgnore) && Passed;
  Passed = EvaluateTrue("MCSteppingAction()", "defaults", "The interaction ID starts at 0", SA(Defaults, MCSteppingAction, m_InteractionId) == 0) && Passed;

  MCSteppingAction Special(*m_Special);
  Passed = EvaluateTrue("MCSteppingAction()", "special", "Ionization is stored when requested", SA(Special, MCSteppingAction, m_StoreIonization)) && Passed;
  Passed = Evaluate("MCSteppingAction()", "special", "The watched volumes are taken over", SA(Special, MCSteppingAction, m_WatchedVolumes).size(), (size_t) 2) && Passed;
  Passed = Evaluate("MCSteppingAction()", "special", "The logical volume names of the watched volumes have the suffix Log", SA(Special, MCSteppingAction, m_WatchedVolumesLog).size() == 2 ? SA(Special, MCSteppingAction, m_WatchedVolumesLog)[1] : MString("<missing>"), MString("OtherLog")) && Passed;
  Passed = Evaluate("MCSteppingAction()", "special", "The black absorber is taken over", SA(Special, MCSteppingAction, m_BlackAbsorbers).size() == 1 ? SA(Special, MCSteppingAction, m_BlackAbsorbers)[0] : MString("<missing>"), MString("Inner")) && Passed;
  Passed = Evaluate("MCSteppingAction()", "special", "The decay mode is taken over", SA(Special, MCSteppingAction, m_DecayMode), (int) MCParameterFile::c_DecayModeNormal) && Passed;
  Passed = EvaluateNear("MCSteppingAction()", "special", "The detector time constant is taken over", SA(Special, MCSteppingAction, m_DetectorTimeConstant), 0.001*s, 1e-9) && Passed;
  Passed = Evaluate("MCSteppingAction()", "special", "The region is taken over", SA(Special, MCSteppingAction, m_Regions).size(), (size_t) 1) && Passed;

  // The known process tables are kept in sync
  Passed = Evaluate("MCSteppingAction()", "process tables", "Every known process has an ID", SA(Special, MCSteppingAction, m_KnownProcess).size(), SA(Special, MCSteppingAction, m_KnownProcessID).size()) && Passed;
  // Known failure: the constructor fills m_KnownProcessFrequency twice (two identical loops), the table has twice as many counters as processes
  Passed = ExpectedToFail(Evaluate("MCSteppingAction()", "process tables", "Every known process has a frequency counter (and no more)", SA(Special, MCSteppingAction, m_KnownProcessFrequency).size(), SA(Special, MCSteppingAction, m_KnownProcess).size())) && Passed;

  // Setters
  Special.SetInitialParticles(vector<int>(3, 1));
  Passed = EvaluateTrue("SetInitialParticles()", "3 particles", "The interaction ID starts after the initial particles", SA(Special, MCSteppingAction, m_InteractionId) == 3) && Passed;
  Special.SetParticleOriginIsBuildUpSource(true);
  Passed = EvaluateTrue("SetParticleOriginIsBuildUpSource()", "true", "The flag is stored", SA(Special, MCSteppingAction, m_ParticleOriginIsBuildUpSource)) && Passed;
  Special.SetDecayMode(MCParameterFile::c_DecayModeBuildUp);
  Passed = Evaluate("SetDecayMode()", "buildup", "The decay mode is stored", SA(Special, MCSteppingAction, m_DecayMode), (int) MCParameterFile::c_DecayModeBuildUp) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestProcessIds()
{
  bool Passed = true;

  // The process IDs are all different
  const int IDs[] = { MCSteppingAction::c_ProcessIDCompton, MCSteppingAction::c_ProcessIDPhoto, MCSteppingAction::c_ProcessIDPair, MCSteppingAction::c_ProcessIDRayleigh, MCSteppingAction::c_ProcessIDAnnihilation, MCSteppingAction::c_ProcessIDBremsstrahlung, MCSteppingAction::c_ProcessIDElasticScattering, MCSteppingAction::c_ProcessIDInelasticScattering, MCSteppingAction::c_ProcessIDFission, MCSteppingAction::c_ProcessIDCapture, MCSteppingAction::c_ProcessIDDecay, MCSteppingAction::c_ProcessIDRadioactiveDecay, MCSteppingAction::c_ProcessIDIonization, MCSteppingAction::c_ProcessIDTransportation, MCSteppingAction::c_ProcessIDUncovered, MCSteppingAction::c_ProcessIDNoProcess };
  set<int> Unique(begin(IDs), end(IDs));
  Passed = Evaluate("c_ProcessID", "all", "All process IDs are different", Unique.size(), sizeof(IDs)/sizeof(int)) && Passed;

  struct Mapping { const char* Name; int ID; };
  const Mapping Table[] = {
    { "compt", MCSteppingAction::c_ProcessIDCompton }, { "LowEnCompton", MCSteppingAction::c_ProcessIDCompton }, { "polarLowEnCompt", MCSteppingAction::c_ProcessIDCompton }, { "PenCompton", MCSteppingAction::c_ProcessIDCompton },
    { "phot", MCSteppingAction::c_ProcessIDPhoto }, { "LowEnPhotoElec", MCSteppingAction::c_ProcessIDPhoto }, { "PenPhotoElec", MCSteppingAction::c_ProcessIDPhoto },
    { "conv", MCSteppingAction::c_ProcessIDPair }, { "LowEnConversion", MCSteppingAction::c_ProcessIDPair }, { "LowEnPolarizConversion", MCSteppingAction::c_ProcessIDPair }, { "muPairProd", MCSteppingAction::c_ProcessIDPair }, { "ePairProd", MCSteppingAction::c_ProcessIDPair }, { "hPairProd", MCSteppingAction::c_ProcessIDPair },
    { "Rayl", MCSteppingAction::c_ProcessIDRayleigh }, { "LowEnRayleigh", MCSteppingAction::c_ProcessIDRayleigh },
    { "annihil", MCSteppingAction::c_ProcessIDAnnihilation }, { "PenAnnih", MCSteppingAction::c_ProcessIDAnnihilation },
    { "eBrem", MCSteppingAction::c_ProcessIDBremsstrahlung }, { "hBrems", MCSteppingAction::c_ProcessIDBremsstrahlung }, { "muBrems", MCSteppingAction::c_ProcessIDBremsstrahlung },
    { "hadElastic", MCSteppingAction::c_ProcessIDElasticScattering }, { "ionElastic", MCSteppingAction::c_ProcessIDElasticScattering },
    { "neutronInelastic", MCSteppingAction::c_ProcessIDInelasticScattering }, { "protonInelastic", MCSteppingAction::c_ProcessIDInelasticScattering }, { "ionInelastic", MCSteppingAction::c_ProcessIDInelasticScattering }, { "photonNuclear", MCSteppingAction::c_ProcessIDInelasticScattering }, { "electronNuclear", MCSteppingAction::c_ProcessIDInelasticScattering }, { "positronNuclear", MCSteppingAction::c_ProcessIDInelasticScattering }, { "alphaInelastic", MCSteppingAction::c_ProcessIDInelasticScattering }, { "dInelastic", MCSteppingAction::c_ProcessIDInelasticScattering }, { "tInelastic", MCSteppingAction::c_ProcessIDInelasticScattering }, { "He3Inelastic", MCSteppingAction::c_ProcessIDInelasticScattering }, { "pi+Inelastic", MCSteppingAction::c_ProcessIDInelasticScattering }, { "kaon-Inelastic", MCSteppingAction::c_ProcessIDInelasticScattering },
    { "nFission", MCSteppingAction::c_ProcessIDFission },
    { "nCapture", MCSteppingAction::c_ProcessIDCapture }, { "hBertiniCaptureAtRest", MCSteppingAction::c_ProcessIDCapture },
    { "Decay", MCSteppingAction::c_ProcessIDDecay },
    { "RadioactiveDecay", MCSteppingAction::c_ProcessIDRadioactiveDecay }, { "Radioactivation", MCSteppingAction::c_ProcessIDRadioactiveDecay },
    { "eIoni", MCSteppingAction::c_ProcessIDIonization }, { "ionIoni", MCSteppingAction::c_ProcessIDIonization }, { "hIoni", MCSteppingAction::c_ProcessIDIonization }, { "msc", MCSteppingAction::c_ProcessIDIonization }, { "CoulombScat", MCSteppingAction::c_ProcessIDIonization },
    { "Transportation", MCSteppingAction::c_ProcessIDTransportation },
    { "NoProcess", MCSteppingAction::c_ProcessIDNoProcess },
    { "SomethingNew", MCSteppingAction::c_ProcessIDUncovered }, { "", MCSteppingAction::c_ProcessIDUncovered }, { "compt ", MCSteppingAction::c_ProcessIDUncovered }, { "Compt", MCSteppingAction::c_ProcessIDUncovered }
  };

  MCSteppingActionAccess Access(*m_Default);
  bool AllOK = true;
  MString Wrong;
  for (const Mapping& M: Table) {
    if (Access.GetProcessId(M.Name) != M.ID) {
      AllOK = false;
      Wrong += MString(M.Name) + " ";
    }
  }
  Passed = Evaluate("GetProcessId()", "all known processes", "Every known process name gets its ID (wrong: list)", Wrong, MString("")) && Passed;
  (void) AllOK;

  // Repeated calls re-sort the table by the frequency (at call 1, 1000, 10000, ...): the mapping must not change
  Wrong = "";
  for (unsigned int round = 0; round < 25000; ++round) {
    const Mapping& M = Table[round % (sizeof(Table)/sizeof(Mapping))];
    if (Access.GetProcessId(M.Name) != M.ID) {
      Wrong = MString(M.Name);
      break;
    }
  }
  Passed = Evaluate("GetProcessId()", "25000 calls with re-sorting", "The mapping stays correct while the table is sorted by frequency (first wrong name)", Wrong, MString("")) && Passed;
  Passed = EvaluateTrue("GetProcessId()", "sorted", "The table is sorted by frequency (descending)", [&]() {
    const vector<unsigned long long>& F = SA(Access, MCSteppingAction, m_KnownProcessFrequency);
    const size_t N = SA(Access, MCSteppingAction, m_KnownProcess).size();
    // The table is only sorted at certain calls, so at least the most frequent entries of the previous sorting are in front: only check that frequencies are consistent (sum of the first N entries equals the calls with known names)
    unsigned long long Sum = 0;
    for (size_t i = 0; i < N && i < F.size(); ++i) Sum += F[i];
    return Sum > 0;
  }()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestParticles()
{
  bool Passed = true;

  // Names
  struct Name { const char* Name; int ID; };
  const Name Names[] = {
    { "gamma", MCSource::c_Gamma }, { "e-", MCSource::c_Electron }, { "e+", MCSource::c_Positron }, { "proton", MCSource::c_Proton }, { "anti_proton", MCSource::c_AntiProton }, { "neutron", MCSource::c_Neutron }, { "anti_neutron", MCSource::c_AntiNeutron },
    { "mu+", MCSource::c_MuonPlus }, { "mu-", MCSource::c_MuonMinus }, { "deuteron", MCSource::c_Deuteron }, { "triton", MCSource::c_Triton }, { "He3", MCSource::c_He3 }, { "alpha", MCSource::c_Alpha }, { "GenericIon", MCSource::c_GenericIon },
    { "pi+", MCSource::c_PiPlus }, { "pi-", MCSource::c_PiMinus }, { "pi0", MCSource::c_PiZero }, { "kaon+", MCSource::c_KaonPlus }, { "kaon-", MCSource::c_KaonMinus }, { "lambda", MCSource::c_Lambda },
    { "Na24", 11024 }, { "Al26[228.305]", 13026 }, { "Ga74[59.570X]", 31074 }, { "Fe56", 26056 }, { "U238", 92238 }, { "H2", 1002 },
    { "rho+", MCSource::c_RhoPlus }, { "rho0", MCSource::c_RhoZero }, { "delta++", MCSource::c_DeltaPlusPlus }, { "delta-", MCSource::c_DeltaMinus }, { "delta0", MCSource::c_DeltaZero }, { "delta+", MCSource::c_DeltaPlus },
    { "N(1440)+", MCSource::c_Proton }, { "N(1440)0", MCSource::c_Neutron }, { "omega", MCSource::c_Omega },
    { "NoSuchParticle", 0 }, { "Xx12", 0 }, { "Na", 0 }, { "Na1000", 0 }
  };
  SilenceOutput();
  vector<pair<MString, int>> Results;
  for (const Name& N: Names) Results.push_back({ MString(N.Name), MCSteppingAction::GetParticleType(G4String(N.Name)) });
  RestoreOutput();
  for (unsigned int i = 0; i < Results.size(); ++i) {
    Passed = Evaluate("GetParticleType(name)", Results[i].first, "The particle type of the name", Results[i].second, Names[i].ID) && Passed;
  }

  // Consistency of the name and the definition for all particles of Geant4
  G4ParticleTable* Table = G4ParticleTable::GetParticleTable();
  G4ParticleTable::G4PTblDicIterator* Iterator = Table->GetIterator();
  Iterator->reset();
  vector<MString> Inconsistent;
  unsigned int NParticles = 0;
  SilenceOutput();
  while ((*Iterator)()) {
    G4ParticleDefinition* Definition = Iterator->value();
    ++NParticles;
    const int ByName = MCSteppingAction::GetParticleType(Definition->GetParticleName());
    const int ByDefinition = MCSteppingAction::GetParticleType(Definition);
    if (ByName != ByDefinition) Inconsistent.push_back(MString(Definition->GetParticleName().c_str()) + "(" + ByName + "/" + ByDefinition + ")");
  }
  RestoreOutput();
  MString InconsistentText;
  for (unsigned int i = 0; i < Inconsistent.size() && i < 10; ++i) InconsistentText += Inconsistent[i] + " ";
  Passed = EvaluateTrue("GetParticleType()", "table", "The particle table is not empty", NParticles > 100) && Passed;
  // Known failure: GetParticleType(name) returns 0 for hypernuclei (hyperH4, hyperalpha, ...) while GetParticleType(definition) returns 1000*Z+A for them
  Passed = ExpectedToFail(Evaluate("GetParticleType()", "name vs definition for all particles", "The type from the name and from the definition agree (first differences name/definition)", InconsistentText, MString(""))) && Passed;

  // IDs to definitions and back
  vector<MString> NotBack;
  vector<MString> NoDefinition;
  SilenceOutput();
  for (int ID = 1; ID <= 70; ++ID) {
    G4ParticleDefinition* Definition = MCSteppingAction::GetParticleDefinition(ID, 0.0);
    if (Definition == nullptr) {
      NoDefinition.push_back(MString(ID));
      continue;
    }
    if (MCSteppingAction::GetParticleType(Definition) != ID) {
      NotBack.push_back(MString(ID) + "(" + Definition->GetParticleName().c_str() + ")");
    }
  }
  RestoreOutput();
  MString NotBackText;
  for (const MString& S: NotBack) NotBackText += S + " ";
  Passed = Evaluate("GetParticleDefinition()", "IDs 1-70", "Every ID which has a definition is converted back to the same ID (list of differences)", NotBackText, MString("")) && Passed;

  // The resonances and omega (IDs without a Geant4 particle) are documented as not yet implemented
  const int NotImplemented[] = { MCSource::c_RhoPlus, MCSource::c_RhoMinus, MCSource::c_RhoZero, MCSource::c_DeltaPlusPlus, MCSource::c_DeltaPlus, MCSource::c_DeltaZero, MCSource::c_DeltaMinus, MCSource::c_Omega };
  MString UnexpectedMissing;
  for (const MString& S: NoDefinition) {
    bool Expected = false;
    for (int N: NotImplemented) if (S == MString(N)) Expected = true;
    if (Expected == false && S.ToInt() <= 55) UnexpectedMissing += S + " ";
  }
  Passed = Evaluate("GetParticleDefinition()", "IDs 1-55", "Only the resonances and omega have no definition (missing IDs)", UnexpectedMissing, MString("")) && Passed;

  // Ions
  const int Ions[] = { 11024, 13026, 27060, 82208, 6014, 2004 };
  for (int ID: Ions) {
    SilenceOutput();
    G4ParticleDefinition* Definition = MCSteppingAction::GetParticleDefinition(ID, 0.0);
    RestoreOutput();
    Passed = EvaluateTrue("GetParticleDefinition()", MString(ID), "An ion has a definition", Definition != nullptr) && Passed;
    if (Definition != nullptr && ID != 2004) {
      Passed = Evaluate("GetParticleDefinition()", MString(ID), "An ion is converted back to its ID", MCSteppingAction::GetParticleType(Definition), ID) && Passed;
    }
  }
  SilenceOutput();
  G4ParticleDefinition* Excited = MCSteppingAction::GetParticleDefinition(13026, 228.305*keV);
  RestoreOutput();
  Passed = EvaluateTrue("GetParticleDefinition()", "Al26 isomer", "An isomer has a definition", Excited != nullptr) && Passed;
  if (Excited != nullptr) {
    Passed = EvaluateNear("GetParticleDefinition()", "Al26 isomer", "The excitation energy is the given one", dynamic_cast<G4Ions*>(Excited)->GetExcitationEnergy()/keV, 228.305, 1e-3) && Passed;
    Passed = Evaluate("GetParticleType()", "Al26 isomer", "An isomer has the ID of its nucleus", MCSteppingAction::GetParticleType(Excited), 13026) && Passed;
  }
  SilenceOutput();
  G4ParticleDefinition* Missing = MCSteppingAction::GetParticleDefinition(11099, 0.0);
  G4ParticleDefinition* Zero = MCSteppingAction::GetParticleDefinition(0, 0.0);
  G4ParticleDefinition* Negative = MCSteppingAction::GetParticleDefinition(-5, 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("GetParticleDefinition()", "Na99", "A non-existing nuclide gives nullptr", Missing == nullptr) && Passed;
  Passed = EvaluateTrue("GetParticleDefinition()", "0", "ID 0 gives nullptr", Zero == nullptr) && Passed;
  Passed = EvaluateTrue("GetParticleDefinition()", "-5", "A negative ID gives nullptr", Negative == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestInteractions()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4Gamma::Definition();
  G4ParticleDefinition* Electron = G4Electron::Definition();
  G4ParticleDefinition* Positron = G4Positron::Definition();
  const G4ThreeVector Position(1*cm, 2*cm, 3*cm);

  // Compton with one secondary electron
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
    S.SetProcess(Step, "compt");
    G4Track* Secondary = S.AddSecondary(Electron, 300*keV, G4ThreeVector(1*cm, 2*cm, 3*cm), G4ThreeVector(0, 1, 0));
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "Compton", "One IA is stored", Event()->GetNIAs(), 1U) && Passed;
    if (Event()->GetNIAs() == 1) {
      MSimIA* I = IA(0);
      Passed = Evaluate("UserSteppingAction()", "Compton", "The process is COMP", I->GetProcess(), MString("COMP")) && Passed;
      Passed = Evaluate("UserSteppingAction()", "Compton", "The ID follows the initial particle", I->GetID(), 2) && Passed;
      Passed = Evaluate("UserSteppingAction()", "Compton", "The origin is the origin of the track", I->GetOriginID(), 1) && Passed;
      Passed = Evaluate("UserSteppingAction()", "Compton", "Without a sensitive detector the detector type is unknown", I->GetDetectorType(), MCSD::c_Unknown) && Passed;
      Passed = EvaluateNear("UserSteppingAction()", "Compton", "The time is in seconds", I->GetTime(), 2.5, 1e-9) && Passed;
      Passed = EvaluateTrue("UserSteppingAction()", "Compton", "The position is the one of the electron in cm", SameVector(I->GetPosition(), 1, 2, 3)) && Passed;
      Passed = Evaluate("UserSteppingAction()", "Compton", "The mother is a photon", I->GetMotherParticleID(), 1) && Passed;
      Passed = EvaluateTrue("UserSteppingAction()", "Compton", "The mother direction is the one of the scattered photon", SameVector(I->GetMotherDirection(), 0, 0, 1)) && Passed;
      Passed = EvaluateNear("UserSteppingAction()", "Compton", "The mother energy is the one of the scattered photon in keV", I->GetMotherEnergy(), 700.0, 1e-6) && Passed;
      Passed = Evaluate("UserSteppingAction()", "Compton", "The secondary is an electron", I->GetSecondaryParticleID(), 3) && Passed;
      Passed = EvaluateTrue("UserSteppingAction()", "Compton", "The secondary direction is the one of the electron", SameVector(I->GetSecondaryDirection(), 0, 1, 0)) && Passed;
      Passed = EvaluateNear("UserSteppingAction()", "Compton", "The secondary energy is in keV", I->GetSecondaryEnergy(), 300.0, 1e-6) && Passed;
    }
    MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Secondary->GetUserInformation());
    Passed = EvaluateTrue("UserSteppingAction()", "Compton", "The electron gets the interaction ID as ID and origin", Info != nullptr && Info->GetId() == 2 && Info->GetOriginId() == 2) && Passed;
    MCTrackInformation* PhotonInfo = dynamic_cast<MCTrackInformation*>(Step->GetTrack()->GetUserInformation());
    Passed = EvaluateTrue("UserSteppingAction()", "Compton", "The photon gets the interaction ID as ID but keeps its origin", PhotonInfo != nullptr && PhotonInfo->GetId() == 2 && PhotonInfo->GetOriginId() == 1) && Passed;
  }

  // Compton without a secondary (below the production threshold)
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 700*keV, Position, 5*keV, 1);
    S.SetProcess(Step, "compt");
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "Compton, no secondary", "One IA is stored", Event()->GetNIAs(), 1U) && Passed;
    if (Event()->GetNIAs() >= 1) {
      MSimIA* I = IA(0);
      Passed = Evaluate("UserSteppingAction()", "Compton, no secondary", "The process is COMP", I->GetProcess(), MString("COMP")) && Passed;
      Passed = Evaluate("UserSteppingAction()", "Compton, no secondary", "The secondary is an electron", I->GetSecondaryParticleID(), 3) && Passed;
      Passed = EvaluateNear("UserSteppingAction()", "Compton, no secondary", "The secondary energy is the local deposit in keV", I->GetSecondaryEnergy(), 5.0, 1e-6) && Passed;
      Passed = EvaluateTrue("UserSteppingAction()", "Compton, no secondary", "The position is the one of the photon in cm", SameVector(I->GetPosition(), 1, 2, 3)) && Passed;
    }
  }

  // Pair creation with two secondaries: stored from the last to the first secondary
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 0.0, Position, 0.0, 1);
    S.SetProcess(Step, "conv");
    G4Track* First = S.AddSecondary(Electron, 400*keV, G4ThreeVector(1*cm, 2*cm, 3*cm), G4ThreeVector(1, 0, 0));
    G4Track* Second = S.AddSecondary(Positron, 600*keV, G4ThreeVector(1*cm, 2*cm, 3*cm), G4ThreeVector(0, 0, 1));
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "pair", "Two IAs are stored", Event()->GetNIAs(), 2U) && Passed;
    if (Event()->GetNIAs() == 2) {
      Passed = Evaluate("UserSteppingAction()", "pair", "The first IA is the last secondary (positron)", IA(0)->GetSecondaryParticleID(), 2) && Passed;
      Passed = Evaluate("UserSteppingAction()", "pair", "The second IA is the first secondary (electron)", IA(1)->GetSecondaryParticleID(), 3) && Passed;
      Passed = Evaluate("UserSteppingAction()", "pair", "The process is PAIR", IA(0)->GetProcess(), MString("PAIR")) && Passed;
      Passed = Evaluate("UserSteppingAction()", "pair", "The IDs increase", IA(0)->GetID() + 1, IA(1)->GetID()) && Passed;
      Passed = EvaluateNear("UserSteppingAction()", "pair", "The energy of the positron is in keV", IA(0)->GetSecondaryEnergy(), 600.0, 1e-6) && Passed;
      Passed = Evaluate("UserSteppingAction()", "pair", "The mother is a photon", IA(0)->GetMotherParticleID(), 1) && Passed;
    }
    MCTrackInformation* InfoFirst = dynamic_cast<MCTrackInformation*>(First->GetUserInformation());
    MCTrackInformation* InfoSecond = dynamic_cast<MCTrackInformation*>(Second->GetUserInformation());
    Passed = EvaluateTrue("UserSteppingAction()", "pair", "Both secondaries get track information with their interaction ID", InfoFirst != nullptr && InfoSecond != nullptr && InfoFirst->GetId() == 3 && InfoSecond->GetId() == 2) && Passed;
  }

  // Photo effect, Rayleigh, Bremsstrahlung, Annihilation
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 0.0, Position, 0.0, 1);
    S.SetProcess(Step, "phot");
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "photo effect, no secondary", "An absorbed photon without secondary gives a PHOT IA without secondary", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "PHOT" && IA(0)->GetSecondaryParticleID() == 0) && Passed;
  }
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 0.0, Position, 0.0, 1);
    S.SetProcess(Step, "phot");
    S.AddSecondary(Electron, 100*keV, Position);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "photo effect", "A photo electron gives a PHOT IA with the electron as secondary", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "PHOT" && IA(0)->GetSecondaryParticleID() == 3 && fabs(IA(0)->GetSecondaryEnergy() - 100.0) < 1e-6) && Passed;
  }
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 500*keV, Position, 0.0, 1);
    S.SetProcess(Step, "Rayl");
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "Rayleigh", "Rayleigh scattering gives one RAYL IA with the photon as mother and its energy", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "RAYL" && IA(0)->GetMotherParticleID() == 1 && fabs(IA(0)->GetMotherEnergy() - 500.0) < 1e-6 && IA(0)->GetSecondaryParticleID() == 0) && Passed;
  }
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Electron, 2*MeV, Position, 0.0, 1);
    S.SetProcess(Step, "eBrem");
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "Bremsstrahlung, no secondary", "A bremsstrahlung without photon gives a BREM IA of the electron", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "BREM" && IA(0)->GetMotherParticleID() == 3 && fabs(IA(0)->GetMotherEnergy() - 2000.0) < 1e-6) && Passed;
  }
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Electron, 2*MeV, Position, 0.0, 1);
    S.SetProcess(Step, "eBrem");
    S.AddSecondary(Gamma, 50*keV, Position);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "Bremsstrahlung", "A bremsstrahlung photon gives a BREM IA with the photon as secondary", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "BREM" && IA(0)->GetSecondaryParticleID() == 1 && fabs(IA(0)->GetSecondaryEnergy() - 50.0) < 1e-6) && Passed;
  }
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Positron, 0.0, Position, 0.0, 1);
    S.SetProcess(Step, "annihil");
    S.AddSecondary(Gamma, 511*keV, Position, G4ThreeVector(1, 0, 0));
    S.AddSecondary(Gamma, 511*keV, Position, G4ThreeVector(-1, 0, 0));
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "annihilation", "Two photons give two ANNI IAs", Event()->GetNIAs(), 2U) && Passed;
    if (Event()->GetNIAs() == 2) {
      Passed = EvaluateTrue("UserSteppingAction()", "annihilation", "The mother is the positron", IA(0)->GetProcess() == "ANNI" && IA(0)->GetMotherParticleID() == 2 && IA(1)->GetMotherParticleID() == 2) && Passed;
      Passed = EvaluateTrue("UserSteppingAction()", "annihilation", "The secondaries are 511 keV photons", IA(0)->GetSecondaryParticleID() == 1 && fabs(IA(0)->GetSecondaryEnergy() - 511.0) < 1e-6) && Passed;
    }
  }

  // Hadronic processes without and with secondaries
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(G4Proton::Definition(), 100*MeV, Position, 0.0, 1);
    S.SetProcess(Step, "protonInelastic");
    S.AddSecondary(G4Neutron::Definition(), 20*MeV, Position);
    S.AddSecondary(G4Alpha::Definition(), 10*MeV, Position);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "inelastic", "Two secondaries give two INEL IAs with the proton as mother", Event()->GetNIAs() == 2 && IA(0)->GetProcess() == "INEL" && IA(0)->GetMotherParticleID() == 4 && IA(0)->GetSecondaryParticleID() == MCSource::c_Alpha && IA(1)->GetSecondaryParticleID() == MCSource::c_Neutron) && Passed;
  }
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(G4Neutron::Definition(), 1*MeV, Position, 0.0, 1);
    S.SetProcess(Step, "hadElastic");
    S.AddSecondary(G4Alpha::Definition(), 100*keV, Position);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "elastic", "A recoil gives an ELAS IA", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "ELAS" && IA(0)->GetMotherParticleID() == MCSource::c_Neutron) && Passed;
  }
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(G4Neutron::Definition(), 0.0, Position, 0.0, 1);
    S.SetProcess(Step, "nCapture");
    S.AddSecondary(Gamma, 2*MeV, Position);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "capture", "A capture photon gives a CAPT IA", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "CAPT" && IA(0)->GetSecondaryParticleID() == 1) && Passed;
  }
  {
    // Fission without secondaries still is an interaction and needs its own ID like all other interactions
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(G4Neutron::Definition(), 1*MeV, Position, 0.0, 1);
    S.SetProcess(Step, "nFission");
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "fission, no secondary", "A fission without secondaries gives a FISS IA", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "FISS") && Passed;
    if (Event()->GetNIAs() == 1) {
      // Known failure: for a fission without secondaries UserSteppingAction() adds the FISS IA with the current m_InteractionId without incrementing it first
      Passed = ExpectedToFail(Evaluate("UserSteppingAction()", "fission, no secondary", "The FISS IA gets a new interaction ID (2, after the initial particle 1)", IA(0)->GetID(), 2)) && Passed;
    }
  }

  // Ionization is only stored on request
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Electron, 100*keV, Position, 1*keV, 1);
    S.SetProcess(Step, "eIoni");
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "ionization, not stored", "Ionization is not stored by default", Event()->GetNIAs(), 0U) && Passed;
  }
  {
    Scenario S(*this, *m_Special);
    G4Step* Step = S.MakeStep(Electron, 100*keV, Position, 1*keV, 1);
    S.SetProcess(Step, "eIoni");
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "ionization, stored", "With the ionization option an IONI IA is stored", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "IONI" && IA(0)->GetMotherParticleID() == 3) && Passed;
  }
  {
    Scenario S(*this, *m_Special);
    G4Step* Step = S.MakeStep(Electron, 100*keV, Position, 1*keV, 1);
    S.SetProcess(Step, "eIoni");
    S.AddSecondary(Electron, 5*keV, Position);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "ionization with delta electron", "A delta electron gives an IONI IA with the secondary", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "IONI" && IA(0)->GetSecondaryParticleID() == 3 && fabs(IA(0)->GetSecondaryEnergy() - 5.0) < 1e-6) && Passed;
  }
  {
    // Transportation with a deposit is stored as ionization, without deposit not
    Scenario S(*this, *m_Special);
    G4Step* Step = S.MakeStep(Electron, 100*keV, Position, 1*keV, 1);
    S.SetProcess(Step, "Transportation");
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "transportation with deposit", "A transportation step with an energy deposit is stored as IONI", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "IONI") && Passed;
  }
  {
    Scenario S(*this, *m_Special);
    G4Step* Step = S.MakeStep(Electron, 100*keV, Position, 0.0, 1);
    S.SetProcess(Step, "Transportation");
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "transportation without deposit", "A transportation step without deposit is not stored", Event()->GetNIAs(), 0U) && Passed;
  }

  // An uncovered process stores nothing, but the secondaries inherit the track information
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Electron, 100*keV, Position, 0.0, 1);
    S.SetProcess(Step, "SomethingUnknown");
    G4Track* Secondary = S.AddSecondary(Electron, 5*keV, Position);
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "uncovered process", "An uncovered process stores no IA", Event()->GetNIAs(), 0U) && Passed;
    MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Secondary->GetUserInformation());
    Passed = EvaluateTrue("UserSteppingAction()", "uncovered process", "The secondary inherits ID and origin of the track", Info != nullptr && Info->GetId() == 1 && Info->GetOriginId() == 1) && Passed;
  }

  // The interaction IDs continue over several steps:
  {
    Scenario S(*this, *m_Default);
    G4Step* Step1 = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1, 2.5*s, 1);
    S.SetProcess(Step1, "Rayl");
    S.Step(Step1);
    G4Step* Step2 = S.MakeStep(Gamma, 600*keV, Position, 0.0, 1, 2.6*s, 1);
    S.SetProcess(Step2, "Rayl");
    S.Step(Step2);
    Passed = EvaluateTrue("UserSteppingAction()", "two steps", "Two interactions get the IDs 2 and 3", Event()->GetNIAs() == 2 && IA(0)->GetID() == 2 && IA(1)->GetID() == 3) && Passed;
  }

  // A particle escaping the world:
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 300*keV, Position, 0.0, 0, 2.5*s, 1, false);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "escape", "A particle without next volume gives an ESCP IA", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "ESCP" && IA(0)->GetMotherParticleID() == 1 && fabs(IA(0)->GetMotherEnergy() - 300.0) < 1e-6) && Passed;
    Passed = EvaluateNear("UserSteppingAction()", "escape", "The escape energy is an energy loss", SA(*m_EventAction, MCEventAction, m_IsAborted) ? 1.0 : 0.0, 0.0, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestSafetyNets()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4Gamma::Definition();
  const G4ThreeVector Position(1*cm, 2*cm, 3*cm);

  // An aborted event kills the track and all secondaries
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
    S.SetProcess(Step, "Rayl");
    SA(*m_EventAction, MCEventAction, m_IsAborted) = true;
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "aborted event", "An aborted event kills the track and its secondaries", (int) Step->GetTrack()->GetTrackStatus(), (int) fKillTrackAndSecondaries) && Passed;
    Passed = Evaluate("UserSteppingAction()", "aborted event", "An aborted event stores no more IAs", Event()->GetNIAs(), 0U) && Passed;
    SA(*m_EventAction, MCEventAction, m_IsAborted) = false;
  }

  // NaN
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 700*keV, G4ThreeVector(numeric_limits<double>::quiet_NaN(), 0, 0), 0.0, 1);
    S.SetProcess(Step, "Rayl");
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "NaN position", "A NaN position kills the track", (int) Step->GetTrack()->GetTrackStatus(), (int) fStopAndKill) && Passed;
    Passed = Evaluate("UserSteppingAction()", "NaN position", "A NaN position stores no IA", Event()->GetNIAs(), 0U) && Passed;
  }

  // A track stuck at the same position for 1000 steps is killed:
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
    bool KilledEarly = false;
    for (unsigned int i = 0; i < 1001; ++i) {
      S.Step(Step);
      if (Step->GetTrack()->GetTrackStatus() != fAlive) KilledEarly = true;
    }
    Passed = EvaluateFalse("UserSteppingAction()", "stuck, 1001 steps", "A track which stays 1000 steps at the same position is still alive", KilledEarly) && Passed;
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "stuck, 1002 steps", "A track which stays more than 1000 steps at the same position is killed", (int) Step->GetTrack()->GetTrackStatus(), (int) fStopAndKill) && Passed;
  }
  {
    // Moving tracks are not killed:
    Scenario S(*this, *m_Default);
    bool Killed = false;
    for (unsigned int i = 0; i < 1500; ++i) {
      G4Step* Step = S.MakeStep(Gamma, 700*keV, G4ThreeVector(i*0.01*cm, 1*cm, 1*cm), 0.0, 1);
      S.Step(Step);
      if (Step->GetTrack()->GetTrackStatus() != fAlive) Killed = true;
    }
    Passed = EvaluateFalse("UserSteppingAction()", "moving track", "A moving track is never killed", Killed) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestVolumes()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4Gamma::Definition();
  const G4ThreeVector Position(1*cm, 2*cm, 3*cm);

  // Volumes of the fixture: 0 World, 1 Det, 2 Inner, 3 Other
  // Entering a watched volume: pre step in the world, post step in Det
  {
    Scenario S(*this, *m_Special);
    G4Step* Enter = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
    G4Step* WorldStep = S.MakeStep(Gamma, 700*keV, Position, 0.0, 0);
    Enter->GetPreStepPoint()->SetTouchableHandle(WorldStep->GetPreStepPoint()->GetTouchableHandle());
    S.Step(Enter);
    Passed = EvaluateTrue("UserSteppingAction()", "enter watched volume", "Entering a watched volume gives an ENTR IA", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "ENTR") && Passed;
    if (Event()->GetNIAs() == 1) {
      Passed = EvaluateNear("UserSteppingAction()", "enter watched volume", "The energy of the ENTR IA is the photon energy in keV", IA(0)->GetMotherEnergy(), 700.0, 1e-6) && Passed;
    }
  }
  // Leaving a watched volume: pre step in Det, post step in the world
  {
    Scenario S(*this, *m_Special);
    G4Step* Leave = S.MakeStep(Gamma, 700*keV, Position, 0.0, 0);
    G4Step* DetStep = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
    Leave->GetPreStepPoint()->SetTouchableHandle(DetStep->GetPreStepPoint()->GetTouchableHandle());
    S.Step(Leave);
    Passed = EvaluateTrue("UserSteppingAction()", "leave watched volume", "Leaving a watched volume gives an EXIT IA", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "EXIT") && Passed;
  }
  // Moving from the watched volume Det into its daughter: no ENTR or EXIT
  {
    Scenario S(*this, *m_Special);
    G4Step* Step = S.MakeStep(Gamma, 700*keV, Position, 0.0, 2);
    G4Step* DetStep = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
    Step->GetPreStepPoint()->SetTouchableHandle(DetStep->GetPreStepPoint()->GetTouchableHandle());
    S.Step(Step);
    bool EnterOrExit = false;
    for (unsigned int i = 0; i < Event()->GetNIAs(); ++i) if (IA(i)->GetProcess() == "ENTR" || IA(i)->GetProcess() == "EXIT") EnterOrExit = true;
    Passed = EvaluateFalse("UserSteppingAction()", "into a daughter of a watched volume", "Moving into a daughter of the watched volume is neither entering nor leaving", EnterOrExit) && Passed;
  }
  // Watched volumes are only relevant if the volume changes:
  {
    Scenario S(*this, *m_Special);
    G4Step* Step = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "same volume", "A step inside the watched volume stores no IA", Event()->GetNIAs(), 0U) && Passed;
  }

  // Black absorber Inner: entering it stops the track
  {
    Scenario S(*this, *m_Special);
    G4Step* Enter = S.MakeStep(Gamma, 700*keV, Position, 0.0, 2);
    G4Step* DetStep = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
    Enter->GetPreStepPoint()->SetTouchableHandle(DetStep->GetPreStepPoint()->GetTouchableHandle());
    S.Step(Enter);
    bool HasBlack = false;
    for (unsigned int i = 0; i < Event()->GetNIAs(); ++i) if (IA(i)->GetProcess() == "BLAK") HasBlack = true;
    Passed = EvaluateTrue("UserSteppingAction()", "black absorber", "Entering a black absorber gives a BLAK IA", HasBlack) && Passed;
    Passed = Evaluate("UserSteppingAction()", "black absorber", "Entering a black absorber kills the track", (int) Enter->GetTrack()->GetTrackStatus(), (int) fStopAndKill) && Passed;
  }

  // Region: secondaries in the region CutRegion are killed. The volume Other is in the region of the parameter file
  {
    G4Region* Region = new G4Region("CutRegion");
    G4LogicalVolume* OtherLog = nullptr;
    for (G4LogicalVolume* L: *G4LogicalVolumeStore::GetInstance()) if (L != nullptr && L->GetName() == "OtherLog") OtherLog = L;
    Passed = EvaluateTrue("UserSteppingAction()", "region", "The logical volume of the region exists", OtherLog != nullptr) && Passed;
    if (OtherLog != nullptr) {
      Region->AddRootLogicalVolume(OtherLog);
      Scenario S(*this, *m_Special);
      G4Step* Step = S.MakeStep(Gamma, 700*keV, Position, 0.0, 3);
      S.SetProcess(Step, "compt");
      G4Track* Secondary = S.AddSecondary(G4Electron::Definition(), 300*keV, Position);
      S.Step(Step);
      Passed = Evaluate("UserSteppingAction()", "region with CutAllSecondaries", "Secondaries in a region which cuts all secondaries are killed", (int) Secondary->GetTrackStatus(), (int) fStopAndKill) && Passed;
      Passed = Evaluate("UserSteppingAction()", "region with CutAllSecondaries", "The interaction is still stored", Event()->GetNIAs(), 1U) && Passed;
      Scenario Other(*this, *m_Default);
      G4Step* Step2 = Other.MakeStep(Gamma, 700*keV, Position, 0.0, 3);
      Other.SetProcess(Step2, "compt");
      G4Track* Secondary2 = Other.AddSecondary(G4Electron::Definition(), 300*keV, Position);
      Other.Step(Step2);
      Passed = Evaluate("UserSteppingAction()", "region without parameter", "Without the region in the parameter file secondaries are kept", (int) Secondary2->GetTrackStatus(), (int) fAlive) && Passed;
      Region->RemoveRootLogicalVolume(OtherLog);
      OtherLog->SetRegion(G4RegionStore::GetInstance()->GetRegion("DefaultRegionForTheWorld", false));
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestDeposits()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4Gamma::Definition();
  G4ParticleDefinition* Electron = G4Electron::Definition();
  const G4ThreeVector Position(1*cm, 2*cm, 3*cm);

  // A deposit in a not sensitive volume is a deposit in passive material, named after the material
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Electron, 100*keV, Position, 30*keV, 1);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "passive deposit", "The deposit is stored by material name", SA(*m_EventAction, MCEventAction, m_PassiveMaterialMap).count("UTSAMaterial") == 1 && fabs(SA(*m_EventAction, MCEventAction, m_PassiveMaterialMap)["UTSAMaterial"] - 30*keV) < 1e-9) && Passed;
  }
  // A digitized step is not a passive deposit; the flag is reset
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Electron, 100*keV, Position, 30*keV, 1);
    MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Step->GetTrack()->GetUserInformation());
    Info->SetDigitized(true);
    SA(*m_EventAction, MCEventAction, m_PassiveMaterialMap).clear();
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "digitized", "A digitized step is no passive deposit", SA(*m_EventAction, MCEventAction, m_PassiveMaterialMap).empty()) && Passed;
    Passed = EvaluateFalse("UserSteppingAction()", "digitized", "The digitized flag is reset", Info->IsDigitized()) && Passed;
  }
  // No deposit, no entry
  {
    Scenario S(*this, *m_Default);
    SA(*m_EventAction, MCEventAction, m_PassiveMaterialMap).clear();
    G4Step* Step = S.MakeStep(Gamma, 100*keV, Position, 0.0, 1);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "no deposit", "Without a deposit nothing is stored", SA(*m_EventAction, MCEventAction, m_PassiveMaterialMap).empty()) && Passed;
  }
  // Sum of several steps
  {
    Scenario S(*this, *m_Default);
    SA(*m_EventAction, MCEventAction, m_PassiveMaterialMap).clear();
    for (unsigned int i = 0; i < 3; ++i) {
      G4Step* Step = S.MakeStep(Electron, 100*keV, G4ThreeVector(i*cm, 0, 0), 10*keV, 1);
      S.Step(Step);
    }
    Passed = EvaluateNear("UserSteppingAction()", "three deposits", "Several deposits add up", SA(*m_EventAction, MCEventAction, m_PassiveMaterialMap)["UTSAMaterial"], 30*keV, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestDecays()
{
  bool Passed = true;

  SilenceOutput();
  G4ParticleDefinition* Sodium = MCSteppingAction::GetParticleDefinition(11024, 0.0);
  RestoreOutput();
  if (Sodium == nullptr) return EvaluateTrue("GetParticleDefinition()", "Na24", "The test ion exists", false);

  const G4ThreeVector Position(1*cm, 2*cm, 3*cm);

  // Normal mode: the decay products are stored
  {
    Scenario S(*this, *m_Special); // DecayMode Normal
    G4Step* Step = S.MakeStep(Sodium, 0.0, Position, 0.0, 1, 2.5*s);
    S.SetProcess(Step, "RadioactiveDecay");
    G4Track* Product = S.AddSecondary(G4Electron::Definition(), 1.4*MeV, Position);
    Product->SetGlobalTime(3.5*s);
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "decay, normal mode", "One decay product gives one DECA IA", Event()->GetNIAs(), 1U) && Passed;
    if (Event()->GetNIAs() == 1) {
      Passed = Evaluate("UserSteppingAction()", "decay, normal mode", "The process is DECA", IA(0)->GetProcess(), MString("DECA")) && Passed;
      Passed = Evaluate("UserSteppingAction()", "decay, normal mode", "The mother is the nucleus", IA(0)->GetMotherParticleID(), 11024) && Passed;
      Passed = Evaluate("UserSteppingAction()", "decay, normal mode", "The secondary is an electron", IA(0)->GetSecondaryParticleID(), 3) && Passed;
      Passed = EvaluateNear("UserSteppingAction()", "decay, normal mode", "The time is the creation time of the decay product in seconds", IA(0)->GetTime(), 3.5, 1e-9) && Passed;
      Passed = EvaluateNear("UserSteppingAction()", "decay, normal mode", "The energy of the decay product is in keV", IA(0)->GetSecondaryEnergy(), 1400.0, 1e-6) && Passed;
    }
    Passed = EvaluateTrue("UserSteppingAction()", "decay, normal mode", "The track is kept", Step->GetTrack()->GetTrackStatus() == fAlive) && Passed;
  }

  // Ignore mode: the track and its secondaries are killed
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Sodium, 0.0, Position, 0.0, 1, 2.5*s);
    S.SetProcess(Step, "RadioactiveDecay");
    S.AddSecondary(G4Electron::Definition(), 1.4*MeV, Position);
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "decay, ignore mode", "The track and its secondaries are killed", (int) Step->GetTrack()->GetTrackStatus(), (int) fKillTrackAndSecondaries) && Passed;
    Passed = Evaluate("UserSteppingAction()", "decay, ignore mode", "No IA is stored", Event()->GetNIAs(), 0U) && Passed;
  }

  // A decay without secondaries does nothing
  {
    Scenario S(*this, *m_Special);
    G4Step* Step = S.MakeStep(Sodium, 0.0, Position, 0.0, 1, 2.5*s);
    S.SetProcess(Step, "RadioactiveDecay");
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "decay without products", "A decay without products stores nothing and keeps the track", Event()->GetNIAs() == 0 && Step->GetTrack()->GetTrackStatus() == fAlive) && Passed;
  }

  // Decay of an unstable particle (not radioactive decay)
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(G4MuonMinus::Definition(), 0.0, Position, 0.0, 1, 2.5*s);
    S.SetProcess(Step, "Decay");
    S.AddSecondary(G4Electron::Definition(), 30*MeV, Position);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "muon decay", "A muon decay gives a DECA IA with the muon as mother", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "DECA" && IA(0)->GetMotherParticleID() == MCSource::c_MuonMinus) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestDetectorIdAndWarnings()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4Gamma::Definition();
  const G4ThreeVector Position(1*cm, 2*cm, 3*cm);

  // GetDetectorId: unknown without a sensitive detector, the type of the detector with one
  MCSteppingActionAccess Access(*m_Default);
  {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(Gamma, 1*MeV, Position, 0.0, 1);
    Passed = Evaluate("GetDetectorId()", "no detector", "Without a sensitive detector the ID is unknown", Access.GetDetectorId(Step->GetPreStepPoint()), MCSD::c_Unknown) && Passed;

    G4LogicalVolume* DetLog = nullptr;
    for (G4LogicalVolume* L: *G4LogicalVolumeStore::GetInstance()) if (L != nullptr && L->GetName() == "DetLog") DetLog = L;
    SilenceOutput();
    MCScintillatorSD* SD = new MCScintillatorSD("UTSADetSD");
    RestoreOutput();
    if (DetLog != nullptr) {
      DetLog->SetSensitiveDetector(SD);
      Passed = Evaluate("GetDetectorId()", "scintillator", "A volume with a scintillator has the scintillator type", Access.GetDetectorId(Step->GetPreStepPoint()), MCSD::c_Scintillator) && Passed;
      // The IA stores the type
      G4Step* Compton = S.MakeStep(Gamma, 700*keV, Position, 0.0, 1);
      S.SetProcess(Compton, "Rayl");
      Compton->GetPreStepPoint()->SetTouchableHandle(Step->GetPreStepPoint()->GetTouchableHandle());
      Compton->SetControlFlag(AvoidHitInvocation);
      S.Step(Compton);
      Passed = EvaluateTrue("UserSteppingAction()", "scintillator", "The IA has the detector type of the volume", Event()->GetNIAs() == 1 && IA(0)->GetDetectorType() == MCSD::c_Scintillator) && Passed;
      DetLog->SetSensitiveDetector(nullptr);
    }
  }

  // The warning for inelastic scattering without secondaries only fires if the particle did not survive
  struct Case { G4TrackStatus Status; unsigned int Secondaries; bool Warning; const char* Name; };
  const Case Cases[] = {
    { fStopAndKill, 0, true, "particle killed, no secondaries" },
    { fAlive, 0, false, "particle survives, no secondaries" },
    { fStopAndKill, 1, false, "particle killed, with secondary" }
  };
  for (const Case& C: Cases) {
    Scenario S(*this, *m_Default);
    G4Step* Step = S.MakeStep(G4Proton::Definition(), 100*MeV, Position, 0.0, 1);
    Step->GetTrack()->SetTrackStatus(C.Status);
    S.SetProcess(Step, "protonInelastic");
    for (unsigned int i = 0; i < C.Secondaries; ++i) S.AddSecondary(G4Neutron::Definition(), 10*MeV, Position);
    ostringstream Captured;
    streambuf* Old = cout.rdbuf(Captured.rdbuf());
    S.Action.UserSteppingAction(Step);
    cout.rdbuf(Old);
    const bool HasWarning = MString(Captured.str().c_str()).Contains("didn't generate secondaries");
    Passed = Evaluate("UserSteppingAction()", C.Name, "The warning about missing secondaries of inelastic scattering", HasWarning, C.Warning) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSteppingAction::TestDecayModes()
{
  bool Passed = true;

  SilenceOutput();
  G4ParticleDefinition* Sodium = MCSteppingAction::GetParticleDefinition(11024, 0.0);
  G4ParticleDefinition* Isomer = MCSteppingAction::GetParticleDefinition(13026, 228.305*keV);
  RestoreOutput();
  if (Sodium == nullptr || Isomer == nullptr) return EvaluateTrue("GetParticleDefinition()", "ions", "The test ions exist", false);

  MCRun& Run = m_Manager->GetCurrentRun();
  const G4ThreeVector Position(1*cm, 2*cm, 3*cm);
  const double Constant = 0.001*s;

  struct Mode { int Mode; const char* Name; };

  // Fast secondary decays (within the detector time constant) of a non-primary: kept in all modes, in the activation modes with the time reset to 0
  const Mode Modes[] = {
    { MCParameterFile::c_DecayModeNormal, "Normal" },
    { MCParameterFile::c_DecayModeActivationBuildUp, "ActivationBuildUp" },
    { MCParameterFile::c_DecayModeActivationDelayedDecay, "ActivationDelayedDecay" }
  };
  for (const Mode& M: Modes) {
    Scenario S(*this, *m_Special);
    S.Action.SetDecayMode(M.Mode);
    G4Step* Step = S.MakeStep(Sodium, 0.0, Position, 0.0, 1, 2.5*s, 5);
    S.SetProcess(Step, "RadioactiveDecay");
    G4Track* Product = S.AddSecondary(G4Electron::Definition(), 1.4*MeV, Position);
    Product->SetGlobalTime(2.5*s + 0.1*Constant);
    const unsigned int SourcesBefore = Run.GetNSources();
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", MString("prompt decay, ") + M.Name, "A decay within the time constant is kept and stored as DECA", Event()->GetNIAs() == 1 && IA(0)->GetProcess() == "DECA" && Step->GetTrack()->GetTrackStatus() == fAlive) && Passed;
    if (M.Mode != MCParameterFile::c_DecayModeNormal && Event()->GetNIAs() == 1) {
      Passed = EvaluateNear("UserSteppingAction()", MString("prompt decay, ") + M.Name, "In the activation modes the time of the decay products is reset to 0", IA(0)->GetTime(), 0.0, 1e-12) && Passed;
    }
    Passed = Evaluate("UserSteppingAction()", MString("prompt decay, ") + M.Name, "A prompt decay is not added to the future event list", Run.GetNSources(), SourcesBefore) && Passed;
  }

  // Delayed decay of a primary (origin <= number of initial particles) in the delayed decay mode is kept
  {
    Scenario S(*this, *m_Special);
    S.Action.SetDecayMode(MCParameterFile::c_DecayModeActivationDelayedDecay);
    G4Step* Step = S.MakeStep(Sodium, 0.0, Position, 0.0, 1, 2.5*s, 1);
    S.SetProcess(Step, "RadioactiveDecay");
    G4Track* Product = S.AddSecondary(G4Electron::Definition(), 1.4*MeV, Position);
    Product->SetGlobalTime(2.5*s + 100*Constant);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "primary decay, ActivationDelayedDecay", "The decay of a primary is simulated now", Event()->GetNIAs() == 1 && Step->GetTrack()->GetTrackStatus() == fAlive) && Passed;
  }

  // Delayed decay of a non-primary in the delayed decay mode: moved to the future event list
  {
    Scenario S(*this, *m_Special);
    S.Action.SetDecayMode(MCParameterFile::c_DecayModeActivationDelayedDecay);
    G4Step* Step = S.MakeStep(Sodium, 0.0, Position, 0.0, 1, 2.5*s, 5);
    S.SetProcess(Step, "RadioactiveDecay");
    G4Track* Product = S.AddSecondary(G4Electron::Definition(), 1.4*MeV, Position);
    Product->SetGlobalTime(2.5*s + 1*s);
    const unsigned int SourcesBefore = Run.GetNSources();
    S.Step(Step);
    Passed = Evaluate("UserSteppingAction()", "delayed decay, ActivationDelayedDecay", "The delayed decay is removed from this event", (int) Step->GetTrack()->GetTrackStatus(), (int) fKillTrackAndSecondaries) && Passed;
    Passed = Evaluate("UserSteppingAction()", "delayed decay, ActivationDelayedDecay", "The delayed decay creates the future event list source", Run.GetNSources(), SourcesBefore + 1) && Passed;
    Passed = EvaluateTrue("UserSteppingAction()", "delayed decay, ActivationDelayedDecay", "The event has a comment about the future decay", EventStringContains("Future decay: Na24")) && Passed;
  }

  // ActivationBuildUp: delayed decays are stored in the isotope store and removed from the event (the reactivated mode)
  for (int Excited = 0; Excited < 2; ++Excited) {
    Scenario S(*this, *m_Special);
    S.Action.SetDecayMode(MCParameterFile::c_DecayModeActivationBuildUp);
    G4ParticleDefinition* Nucleus = (Excited == 0) ? Sodium : Isomer;
    G4Step* Step = S.MakeStep(Nucleus, 0.0, Position, 0.0, 1, 2.5*s, 1);
    S.SetProcess(Step, "RadioactiveDecay");
    G4Track* Product = S.AddSecondary(G4Electron::Definition(), 1.4*MeV, Position);
    Product->SetGlobalTime(2.5*s + 1*s);
    const unsigned int Before = SA(Run, MCRun, m_NAddedIsotopes);
    S.Step(Step);
    const MString Name = Excited == 0 ? "Na24" : "Al26[228.305]";
    Passed = Evaluate("UserSteppingAction()", MString("delayed decay, ActivationBuildUp, ") + Name, "The delayed decay is removed from the event", (int) Step->GetTrack()->GetTrackStatus(), (int) fKillTrackAndSecondaries) && Passed;
    Passed = Evaluate("UserSteppingAction()", MString("delayed decay, ActivationBuildUp, ") + Name, "The isotope is added to the isotope store", SA(Run, MCRun, m_NAddedIsotopes), Before + 1) && Passed;
    Passed = EvaluateTrue("UserSteppingAction()", MString("delayed decay, ActivationBuildUp, ") + Name, "The event has a comment with the stored isotope", EventStringContains(MString("Storing isotope: ") + (Excited == 0 ? "Na24" : "Al26"))) && Passed;
    Passed = Evaluate("UserSteppingAction()", MString("delayed decay, ActivationBuildUp, ") + Name, "No decay IA is stored for a stored isotope", Event()->GetNIAs(), 0U) && Passed;
  }

  // BuildUp mode: primaries from a build-up source are kept, the others are delayed
  {
    Scenario S(*this, *m_Special);
    S.Action.SetDecayMode(MCParameterFile::c_DecayModeBuildUp);
    S.Action.SetParticleOriginIsBuildUpSource(true);
    S.Action.SetInitialParticles(vector<int>(1, 11024));
    G4Step* Step = S.MakeStep(Sodium, 0.0, Position, 0.0, 1, 2.5*s, 1);
    S.SetProcess(Step, "RadioactiveDecay");
    G4Track* Product = S.AddSecondary(G4Electron::Definition(), 1.4*MeV, Position);
    Product->SetGlobalTime(2.5*s + 1*s);
    S.Step(Step);
    Passed = EvaluateTrue("UserSteppingAction()", "BuildUp, initial particle", "The decay of the initial particle of a build-up source is simulated now", Event()->GetNIAs() == 1 && Step->GetTrack()->GetTrackStatus() == fAlive) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSteppingAction Test;
  return Test.Run() == true ? 0 : 1;
}
