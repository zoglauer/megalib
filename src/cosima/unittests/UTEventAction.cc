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

// ROOT:
#include "TError.h"

#include "MCRun.hh"


// Access to the private and protected members of the tested classes (legal member-pointer access via explicit template instantiation)
template<typename Tag, typename Tag::type Member> struct UTEARob { friend typename Tag::type UTEAGet(Tag) { return Member; } };
#define UTC_EA_ACCESS(Class, Name, Type) \
  struct UTEATag##Name { typedef Type Class::* type; friend type UTEAGet(UTEATag##Name); }; \
  template struct UTEARob<UTEATag##Name, &Class::Name>;
UTC_EA_ACCESS(MCEventAction, m_Event, MSimEvent*)
UTC_EA_ACCESS(MCEventAction, m_ID, long)
UTC_EA_ACCESS(MCEventAction, m_StoreBinary, bool)
UTC_EA_ACCESS(MCEventAction, m_SaveEvents, bool)
UTC_EA_ACCESS(MCEventAction, m_PreTriggerMode, int)
UTC_EA_ACCESS(MCEventAction, m_StoreMaximumEnergyLoss, double)
UTC_EA_ACCESS(MCEventAction, m_StoreMinimumEnergy, double)
UTC_EA_ACCESS(MCEventAction, m_StoreOneHitPerEvent, bool)
UTC_EA_ACCESS(MCEventAction, m_LostEnergy, double)
UTC_EA_ACCESS(MCEventAction, m_TimeOut, double)
UTC_EA_ACCESS(MCEventAction, m_OutFileName, string)
UTC_EA_ACCESS(MCEventAction, m_OutFile, MFileEvents)
UTC_EA_ACCESS(MCEventAction, m_2DStripCollNames, vector<G4String>)
UTC_EA_ACCESS(MCEventAction, m_CalorimeterCollNames, vector<G4String>)
UTC_EA_ACCESS(MCEventAction, m_3DStripCollNames, vector<G4String>)
UTC_EA_ACCESS(MCEventAction, m_Voxel3DCollNames, vector<G4String>)
UTC_EA_ACCESS(MCEventAction, m_ScintillatorCollNames, vector<G4String>)
UTC_EA_ACCESS(MCEventAction, m_DriftChamberCollNames, vector<G4String>)
UTC_EA_ACCESS(MCEventAction, m_AngerCameraCollNames, vector<G4String>)
UTC_EA_ACCESS(MFile, m_MaxFileLength, streamsize)
UTC_EA_ACCESS(G4RunManager, currentEvent, G4Event*)
#define EA(Object, Class, Name) ((Object).*UTEAGet(UTEATag##Name()))


static int gRelegated = 0;
static void Relegator(MSimEvent*) { ++gRelegated; }


//! Unit test class for MCEventAction
class UTEventAction : public MUnitTest
{
public:
  //! Default constructor
  UTEventAction() : MUnitTest("UTEventAction"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr), m_Parameters(nullptr), m_Manager(nullptr), m_Action(nullptr) {}
  //! Default destructor
  virtual ~UTEventAction() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructor
  bool TestConstruction();
  //! Test the registration of the collection names
  bool TestCollectionNames();
  //! Test the preparation of a run
  bool TestNextRun();
  //! Test the functions adding data to the event
  bool TestEventData();
  //! Test the energy loss and the abort
  bool TestEnergyLoss();
  //! Test storing events
  bool TestEndOfEvent();
  //! Test the trigger modes
  bool TestPreTriggerModes();
  //! Test the relegation
  bool TestRelegator();
  //! Test the interrupt and the end of the run
  bool TestInterrupt();
  //! Test the file roll-over
  bool TestFileRollOver();

  //! Silence cout and cerr
  void SilenceOutput();
  //! Restore cout and cerr
  void RestoreOutput();
  //! Initialize the Geant4 particles
  void InitializeGeant4();
  //! Create the parameter file
  bool CreateParameters();
  //! Return the content of the file
  MString Content(const MString& FileName);
  //! Return the number of occurrences of the text
  unsigned int Count(const MString& Text, const MString& Search);
  //! Return the sim string of the current event
  MString EventString();
  //! Call the end of the event silently
  void EndEvent(MCEventAction& Action);
  //! Add a hit with an energy to the event of the action
  void AddHit(MCEventAction& Action, double Energy);
  //! Create a new action with its own output file and prepare the run
  MCEventAction* Scenario(const MString& Name, bool Binary = false, int ParallelID = 0);
  //! Interrupt the run and end the event, which closes the file
  void Finish(MCEventAction& Action);
  //! Return the sim file of a scenario
  MString SimFile(const MString& Name, int ID = 1, bool Binary = false, int ParallelID = 0);

  streambuf* m_CoutBuffer;
  streambuf* m_CerrBuffer;
  ostringstream m_Sink;
  MCParameterFile* m_Parameters;
  MCRunManager* m_Manager;
  MCEventAction* m_Action;
  MString m_Directory;
};


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::Run()
{
  bool Passed = true;

  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  if (RequireFixture(*this, Passed, "src/cosima/unittests/GiganticSphere.geo.setup") == false) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return Passed;
  }

  Passed = EvaluateTrue("CreateParameters()", "parameter file", "The parameter file with a run can be opened", CreateParameters()) && Passed;
  if (m_Parameters == nullptr) {
    gErrorIgnoreLevel = OldErrorLevel;
    Summarize();
    return false;
  }

  SilenceOutput();
  m_Manager = new MCRunManager(*m_Parameters);
  RestoreOutput();
  InitializeGeant4();

  SilenceOutput();
  // The detector construction registers its collection names in the action of the run manager, thus this one is only a placeholder:
  MCEventAction* RegisteredAction = new MCEventAction(*m_Parameters, false, 12345, true);
  m_Action = new MCEventAction(*m_Parameters, false, 12345, true);
  MCTrackingAction* TrackingAction = new MCTrackingAction();
  MCSteppingAction* SteppingAction = new MCSteppingAction(*m_Parameters);
  MCDetectorConstruction* Detector = new MCDetectorConstruction(*m_Parameters);
  m_Manager->SetUserAction(RegisteredAction);
  m_Manager->SetUserAction(TrackingAction);
  m_Manager->SetUserAction(SteppingAction);
  const bool GeometryLoaded = Detector->Initialize();
  m_Manager->SetUserInitialization(Detector);
  RestoreOutput();
  Passed = EvaluateTrue("MCDetectorConstruction::Initialize()", "test geometry", "The test geometry can be loaded", GeometryLoaded) && Passed;

  Passed = TestConstruction() && Passed;
  Passed = TestCollectionNames() && Passed;
  Passed = TestNextRun() && Passed;
  Passed = TestEventData() && Passed;
  Passed = TestEnergyLoss() && Passed;
  Passed = TestEndOfEvent() && Passed;
  Passed = TestPreTriggerModes() && Passed;
  Passed = TestRelegator() && Passed;
  Passed = TestFileRollOver() && Passed;
  Passed = TestInterrupt() && Passed;

  SilenceOutput();
  delete m_Action;
  delete m_Manager;
  delete m_Parameters;
  RestoreOutput();

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTEventAction::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTEventAction::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


void UTEventAction::InitializeGeant4()
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


bool UTEventAction::CreateParameters()
{
  PrepareTemporaryDirectory("Files");
  m_Directory = GetTemporaryDirectoryName("Files");

  MString Content = "Version 1\nGeometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\nDetectorTimeConstant 0.000005\nPhysicsListEM LivermorePol\nPreTriggerMode Everything\n";
  // The output name in the parameter file is relative to the private temporary root, which is the working directory of the test:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  Content += "Run Base\nBase.FileName Files/UTEARun\nBase.Triggers 1000000\nBase.Source S\n";
  Content += "S.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n";
  const MString FileName = m_Directory + "/Main.source";
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


MString UTEventAction::Content(const MString& FileName)
{
  ifstream In(FileName.Data());
  stringstream Buffer;
  Buffer<<In.rdbuf();
  return MString(Buffer.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTEventAction::Count(const MString& Text, const MString& Search)
{
  unsigned int N = 0;
  string T = Text.Data();
  string S = Search.Data();
  size_t Pos = 0;
  while ((Pos = T.find(S, Pos)) != string::npos) {
    ++N;
    Pos += S.size();
  }
  return N;
}


////////////////////////////////////////////////////////////////////////////////


MString UTEventAction::EventString()
{
  return EA(*m_Action, MCEventAction, m_Event)->ToSimString();
}


////////////////////////////////////////////////////////////////////////////////


void UTEventAction::EndEvent(MCEventAction& Action)
{
  G4Event Event(0);
  SilenceOutput();
  Action.EndOfEventAction(&Event);
  RestoreOutput();
}


////////////////////////////////////////////////////////////////////////////////


void UTEventAction::AddHit(MCEventAction& Action, double Energy)
{
  MSimHT* HT = new MSimHT();
  HT->SetDetectorType(1);
  HT->SetPosition(MVector(1.0, 2.0, 3.0));
  HT->SetEnergy(Energy);
  HT->SetTime(0.0);
  EA(Action, MCEventAction, m_Event)->AddHT(HT);
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestConstruction()
{
  bool Passed = true;

  Passed = EvaluateFalse("GetInterrupt()", "new", "A new event action has no interrupt", m_Action->GetInterrupt()) && Passed;
  Passed = EvaluateNear("MCEventAction()", "new", "The lost energy starts at 0", EA(*m_Action, MCEventAction, m_LostEnergy), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCEventAction()", "parameters", "The pre-trigger mode comes from the parameter file", EA(*m_Action, MCEventAction, m_PreTriggerMode), (int) MCParameterFile::c_PreTriggerEverything) && Passed;
  Passed = EvaluateFalse("MCEventAction()", "parameters", "Binary storage is off by default", EA(*m_Action, MCEventAction, m_StoreBinary)) && Passed;
  Passed = EvaluateTrue("MCEventAction()", "parameters", "The default minimum energy stores everything", EA(*m_Action, MCEventAction, m_StoreMinimumEnergy) < 0) && Passed;
  Passed = EvaluateTrue("MCEventAction()", "parameters", "The default maximum energy loss is huge", EA(*m_Action, MCEventAction, m_StoreMaximumEnergyLoss) > 1e30) && Passed;
  Passed = EvaluateFalse("MCEventAction()", "parameters", "One hit per event is off by default", EA(*m_Action, MCEventAction, m_StoreOneHitPerEvent)) && Passed;

  m_Action->Interrupt();
  Passed = EvaluateTrue("Interrupt()", "called", "An interrupt is remembered", m_Action->GetInterrupt()) && Passed;

  // A run which was not prepared yet must not claim that an event is aborted (uninitialized member):
  const size_t Size = sizeof(MCEventAction);
  vector<unsigned char> BufferA(Size + 64, 0x00);
  vector<unsigned char> BufferB(Size + 64, 0xFF);
  void* PointerA = (void*) (((uintptr_t) BufferA.data() + 31) & ~((uintptr_t) 31));
  void* PointerB = (void*) (((uintptr_t) BufferB.data() + 31) & ~((uintptr_t) 31));
  memset(PointerA, 0x00, Size);
  memset(PointerB, 0xFF, Size);
  SilenceOutput();
  MCEventAction* ActionA = new (PointerA) MCEventAction(*m_Parameters, false, 1, true);
  MCEventAction* ActionB = new (PointerB) MCEventAction(*m_Parameters, false, 1, true);
  RestoreOutput();
  const bool AbortedA = ActionA->IsAborted();
  const bool AbortedB = ActionB->IsAborted();
  SilenceOutput();
  ActionA->~MCEventAction();
  ActionB->~MCEventAction();
  RestoreOutput();
  // Known failure: m_IsAborted is not initialized by the constructor, only by BeginOfEventAction()
  Passed = ExpectedToFail(EvaluateTrue("MCEventAction()", "memory pattern", "The abort flag is initialized by the constructor", AbortedA == AbortedB)) && Passed;
  // Known failure: the uninitialized m_IsAborted keeps the memory pattern, so a new event action can claim to be aborted
  Passed = ExpectedToFail(EvaluateFalse("MCEventAction()", "memory pattern", "A new event action is not aborted", AbortedA || AbortedB)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestCollectionNames()
{
  bool Passed = true;

  SilenceOutput();
  MCEventAction Action(*m_Parameters, false, 1, true);
  Action.SetCollectionName("A", MDDetector::c_Strip2D);
  Action.SetCollectionName("B", MDDetector::c_Calorimeter);
  Action.SetCollectionName("C", MDDetector::c_Strip3D);
  Action.SetCollectionName("D", MDDetector::c_Voxel3D);
  Action.SetCollectionName("E", MDDetector::c_ACS);
  Action.SetCollectionName("F", MDDetector::c_DriftChamber);
  Action.SetCollectionName("G", MDDetector::c_AngerCamera);
  Action.SetCollectionName("H", MDDetector::c_GuardRing);
  Action.SetCollectionName("I", -12345);
  Action.SetCollectionName("J", MDDetector::c_Strip2D);
  RestoreOutput();

  Passed = Evaluate("SetCollectionName()", "2D strip", "Two 2D strip collections are stored", EA(Action, MCEventAction, m_2DStripCollNames).size(), (size_t) 2) && Passed;
  Passed = Evaluate("SetCollectionName()", "2D strip", "The 2D strip collections are in order", EA(Action, MCEventAction, m_2DStripCollNames).back(), G4String("J")) && Passed;
  Passed = Evaluate("SetCollectionName()", "calorimeter", "The calorimeter is stored", EA(Action, MCEventAction, m_CalorimeterCollNames).size(), (size_t) 1) && Passed;
  Passed = Evaluate("SetCollectionName()", "3D strip", "The 3D strip is stored", EA(Action, MCEventAction, m_3DStripCollNames).size(), (size_t) 1) && Passed;
  Passed = Evaluate("SetCollectionName()", "voxel", "The voxel detector is stored", EA(Action, MCEventAction, m_Voxel3DCollNames).size(), (size_t) 1) && Passed;
  Passed = Evaluate("SetCollectionName()", "scintillator", "The scintillator is stored", EA(Action, MCEventAction, m_ScintillatorCollNames).size(), (size_t) 1) && Passed;
  Passed = Evaluate("SetCollectionName()", "drift chamber", "The drift chamber is stored", EA(Action, MCEventAction, m_DriftChamberCollNames).size(), (size_t) 1) && Passed;
  Passed = Evaluate("SetCollectionName()", "anger camera", "The anger camera is stored", EA(Action, MCEventAction, m_AngerCameraCollNames).size(), (size_t) 1) && Passed;
  Passed = EvaluateTrue("SetCollectionName()", "unknown and guard ring", "Guard rings and unknown types add no collection", EA(Action, MCEventAction, m_2DStripCollNames).size() + EA(Action, MCEventAction, m_CalorimeterCollNames).size() + EA(Action, MCEventAction, m_3DStripCollNames).size() + EA(Action, MCEventAction, m_Voxel3DCollNames).size() + EA(Action, MCEventAction, m_ScintillatorCollNames).size() + EA(Action, MCEventAction, m_DriftChamberCollNames).size() + EA(Action, MCEventAction, m_AngerCameraCollNames).size() == 8) && Passed;

  // The action of the tests has none (the collection IDs would be looked up at the end of every event)
  Passed = EvaluateTrue("SetCollectionName()", "test action", "The action of the tests has no collections", EA(*m_Action, MCEventAction, m_2DStripCollNames).empty() && EA(*m_Action, MCEventAction, m_CalorimeterCollNames).empty() && EA(*m_Action, MCEventAction, m_3DStripCollNames).empty() && EA(*m_Action, MCEventAction, m_Voxel3DCollNames).empty() && EA(*m_Action, MCEventAction, m_ScintillatorCollNames).empty() && EA(*m_Action, MCEventAction, m_DriftChamberCollNames).empty() && EA(*m_Action, MCEventAction, m_AngerCameraCollNames).empty()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestNextRun()
{
  bool Passed = true;

  const MString SimFile = m_Directory + "/UTEARun.inc0.id1.sim";
  Passed = EvaluateFalse("NextRun()", "before", "The sim file does not exist before the run is prepared", filesystem::exists(SimFile.Data())) && Passed;

  SilenceOutput();
  const bool Prepared = m_Action->NextRun();
  RestoreOutput();
  Passed = EvaluateTrue("NextRun()", "with file name", "The run can be prepared", Prepared) && Passed;
  Passed = EvaluateTrue("NextRun()", "with file name", "The sim file is opened", filesystem::exists(SimFile.Data())) && Passed;
  Passed = EvaluateTrue("NextRun()", "with file name", "The events are saved", EA(*m_Action, MCEventAction, m_SaveEvents)) && Passed;
  // The run keeps the file name as it is in the parameter file: relative to the working directory of the test
  Passed = Evaluate("NextRun()", "with file name", "The file name is stored", MString(EA(*m_Action, MCEventAction, m_OutFileName).c_str()), MString("Files/UTEARun.inc0.id1.sim")) && Passed;
  Passed = Evaluate("NextRun()", "with file name", "The event counter is reset", EA(*m_Action, MCEventAction, m_ID), 0L) && Passed;
  Passed = EvaluateFalse("NextRun()", "with file name", "The run is not aborted", m_Action->IsAborted()) && Passed;
  Passed = EvaluateFalse("NextRun()", "with file name", "The interrupt of a previous run is cleared", m_Action->GetInterrupt()) && Passed;

  // Flush the header by closing is not possible here - check the header after the first event was stored
  const G4Event Event(0);
  m_Action->BeginOfEventAction(&Event);
  m_Action->BeginOfEventAction(&Event);
  Passed = Evaluate("BeginOfEventAction()", "two events", "The event counter counts the events", EA(*m_Action, MCEventAction, m_ID), 2L) && Passed;

  // Without a file name nothing is saved:
  MCRun& Run = m_Manager->GetCurrentRun();
  const MString OldName = Run.GetFileName();
  Run.SetFileName("");
  SilenceOutput();
  MCEventAction NoFile(*m_Parameters, false, 1, true);
  const bool NoFilePrepared = NoFile.NextRun();
  RestoreOutput();
  Passed = EvaluateTrue("NextRun()", "no file name", "A run without a file name can be prepared", NoFilePrepared) && Passed;
  Passed = EvaluateFalse("NextRun()", "no file name", "Without a file name nothing is saved", EA(NoFile, MCEventAction, m_SaveEvents)) && Passed;
  Run.SetFileName(OldName);

  // A file which cannot be opened is reported:
  Run.SetFileName(m_Directory + "/DoesNotExist/Run");
  SilenceOutput();
  MCEventAction BadFile(*m_Parameters, false, 1, true);
  const bool BadPrepared = BadFile.NextRun();
  RestoreOutput();
  Passed = EvaluateFalse("NextRun()", "bad file name", "A file in a missing directory fails", BadPrepared) && Passed;
  Run.SetFileName(OldName);

  // Parallel and zipped names:
  Run.SetParallelID(3);
  Run.SetIncarnationID(2);
  SilenceOutput();
  MCEventAction Zipped(*m_Parameters, true, 1, true);
  Zipped.NextRun();
  RestoreOutput();
  Passed = Evaluate("NextRun()", "parallel, zipped", "The file name contains the parallel ID, the incarnation and the zip suffix", MString(EA(Zipped, MCEventAction, m_OutFileName).c_str()), MString("Files/UTEARun.p3.inc2.id1.sim.gz")) && Passed;
  Run.SetParallelID(0);
  Run.SetIncarnationID(0);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestEventData()
{
  bool Passed = true;

  SilenceOutput();
  m_Action->Reset();
  RestoreOutput();

  auto Numbers = [](const MString& Text, const string& Prefix) {
    // Returns the numbers of the first line with the prefix, separated by ';' and rounded to 1e-6
    string Result = "<missing>";
    istringstream In(Text.Data());
    string Line;
    while (getline(In, Line)) {
      if (Line.compare(0, Prefix.size(), Prefix) != 0) continue;
      Result = "";
      string Rest = Line.substr(Prefix.size());
      replace(Rest.begin(), Rest.end(), ';', ' ');
      istringstream Fields(Rest);
      double N;
      while (Fields>>N) {
        double R = round(N*1e6)/1e6;
        if (R == 0.0) R = 0.0;
        ostringstream Out;
        Out<<R;
        if (Result.empty() == false) Result += " ";
        Result += Out.str();
      }
      break;
    }
    return MString(Result.c_str());
  };

  // An IA: time in seconds as passed by the stepping action, position in cm, energies in keV
  m_Action->AddIA("INIT", 1, 0, 0, 2.5, G4ThreeVector(10*mm, 20*mm, 30*mm), 0, G4ThreeVector(0, 0, 1), G4ThreeVector(1, 0, 0), 0.0, 1, G4ThreeVector(0, 1, 0), G4ThreeVector(0, 0, 1), 1.5*MeV);
  MString Text = EventString();
  Passed = Evaluate("AddIA()", "INIT", "The IA has the IDs, time, position in cm, directions, polarizations and energies in keV", Numbers(Text, "IA INIT"), MString("1 0 0 2.5 1 2 3 0 0 0 1 1 0 0 0 1 0 1 0 0 0 1 1500")) && Passed;
  Passed = Evaluate("AddIA()", "INIT", "The number of IAs is one", EA(*m_Action, MCEventAction, m_Event)->GetNIAs(), 1U) && Passed;
  Passed = EvaluateNear("AddIA()", "INIT", "A normal IA is no energy loss", EA(*m_Action, MCEventAction, m_LostEnergy), 0.0, 1e-12) && Passed;

  // A second IA with an escape adds the kinetic energy to the energy loss:
  m_Action->AddIA("ESCP", 2, 1, 0, 3.0, G4ThreeVector(), 1, G4ThreeVector(1, 0, 0), G4ThreeVector(), 200*keV, 0, G4ThreeVector(), G4ThreeVector(), 0.0);
  Passed = Evaluate("AddIA()", "ESCP", "The number of IAs is two", EA(*m_Action, MCEventAction, m_Event)->GetNIAs(), 2U) && Passed;
  Passed = EvaluateNear("AddIA()", "ESCP", "An escape adds the energy to the energy loss", EA(*m_Action, MCEventAction, m_LostEnergy), 200*keV, 1e-9) && Passed;

  // Comments
  m_Action->AddComment("Hello comment");
  Passed = EvaluateTrue("AddComment()", "text", "The comment is part of the event", EventString().Contains("CC Hello comment")) && Passed;

  // Passive material: energy adds to the loss
  m_Action->AddDepositPassiveMaterial(100*keV, "Aluminum");
  m_Action->AddDepositPassiveMaterial(50*keV, "Aluminum");
  m_Action->AddDepositPassiveMaterial(10*keV, "Lead");
  Passed = EvaluateNear("AddDepositPassiveMaterial()", "160 keV", "Passive deposits count as energy loss", EA(*m_Action, MCEventAction, m_LostEnergy), 360*keV, 1e-9) && Passed;

  // Pointing: the stepping action passes radians (from MCOrientation), the event stores degrees
  m_Action->SetGalacticPointing(30*deg, 40*deg, 10*deg, 130*deg);
  MSimEvent* Event = EA(*m_Action, MCEventAction, m_Event);
  Passed = EvaluateTrue("SetGalacticPointing()", "set", "The event has a galactic pointing", Event->HasGalacticPointing()) && Passed;
  Passed = EvaluateNear("SetGalacticPointing()", "x axis", "The galactic x axis has the given longitude", Event->GetGalacticPointingXAxis().Phi(), 40*deg, 1e-9) && Passed;
  Passed = EvaluateNear("SetGalacticPointing()", "x axis", "The galactic x axis has the given latitude", Event->GetGalacticPointingXAxis().Theta(), 90*deg + 30*deg, 1e-9) && Passed;
  Passed = EvaluateNear("SetGalacticPointing()", "z axis", "The galactic z axis has the given longitude", Event->GetGalacticPointingZAxis().Phi(), 130*deg, 1e-9) && Passed;
  Passed = EvaluateNear("SetGalacticPointing()", "z axis", "The galactic z axis has the given latitude", Event->GetGalacticPointingZAxis().Theta(), 90*deg + 10*deg, 1e-9) && Passed;

  m_Action->SetDetectorPointing(60*deg, 20*deg, 10*deg, 70*deg);
  Passed = EvaluateTrue("SetDetectorPointing()", "set", "The event has a detector pointing", Event->HasDetectorRotation()) && Passed;
  Passed = EvaluateNear("SetDetectorPointing()", "x axis", "The detector x axis has the given theta", Event->GetDetectorRotationXAxis().Theta(), 60*deg, 1e-9) && Passed;
  Passed = EvaluateNear("SetDetectorPointing()", "x axis", "The detector x axis has the given phi", Event->GetDetectorRotationXAxis().Phi(), 20*deg, 1e-9) && Passed;
  Passed = EvaluateNear("SetDetectorPointing()", "z axis", "The detector z axis has the given theta", Event->GetDetectorRotationZAxis().Theta(), 10*deg, 1e-9) && Passed;
  Passed = EvaluateNear("SetDetectorPointing()", "z axis", "The detector z axis has the given phi", Event->GetDetectorRotationZAxis().Phi(), 70*deg, 1e-9) && Passed;

  // Reset removes everything of the event
  SilenceOutput();
  m_Action->Reset();
  RestoreOutput();
  Passed = Evaluate("Reset()", "after data", "Reset removes the IAs", Event->GetNIAs(), 0U) && Passed;
  Passed = EvaluateNear("Reset()", "after data", "Reset removes the energy loss", EA(*m_Action, MCEventAction, m_LostEnergy), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("Reset()", "after data", "Reset removes the comment", EventString().Contains("CC Hello comment")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MCEventAction* UTEventAction::Scenario(const MString& Name, bool Binary, int ParallelID)
{
  MCRun& Run = m_Manager->GetCurrentRun();
  Run.SetFileName(m_Directory + "/" + Name);
  Run.SetParallelID(ParallelID);
  Run.SetIncarnationID(0);
  SilenceOutput();
  MCEventAction* Action = new MCEventAction(*m_Parameters, false, 1, true);
  EA(*Action, MCEventAction, m_StoreBinary) = Binary;
  const bool Prepared = Action->NextRun();
  RestoreOutput();
  if (Prepared == false) {
    delete Action;
    return nullptr;
  }
  return Action;
}


////////////////////////////////////////////////////////////////////////////////


void UTEventAction::Finish(MCEventAction& Action)
{
  // The last event only closes the run, it must not be stored
  EA(Action, MCEventAction, m_PreTriggerMode) = MCParameterFile::c_PreTriggerFull;
  Action.Interrupt();
  EndEvent(Action);
}


////////////////////////////////////////////////////////////////////////////////


MString UTEventAction::SimFile(const MString& Name, int ID, bool Binary, int ParallelID)
{
  ostringstream Out;
  Out<<m_Directory.Data()<<"/"<<Name.Data();
  if (ParallelID != 0) Out<<".p"<<ParallelID;
  Out<<".inc0.id"<<ID;
  if (Binary == true) Out<<".bin";
  Out<<".sim";
  return MString(Out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestEnergyLoss()
{
  bool Passed = true;

  G4RunManager* Base = m_Manager;
  G4Event* Current = new G4Event(77);
  EA(*Base, G4RunManager, currentEvent) = Current;
  const double OldMax = EA(*m_Action, MCEventAction, m_StoreMaximumEnergyLoss);
  EA(*m_Action, MCEventAction, m_StoreMaximumEnergyLoss) = 100*keV;

  SilenceOutput();
  m_Action->Reset();
  m_Action->AddEnergyLoss(60*keV);
  RestoreOutput();
  Passed = EvaluateFalse("AddEnergyLoss()", "60 of 100 keV", "A loss below the maximum does not abort the event", m_Action->IsAborted()) && Passed;
  Passed = EvaluateFalse("AddEnergyLoss()", "60 of 100 keV", "The Geant4 event is not aborted", Current->IsAborted()) && Passed;
  SilenceOutput();
  m_Action->AddEnergyLoss(30*keV);
  RestoreOutput();
  Passed = EvaluateFalse("AddEnergyLoss()", "90 of 100 keV", "The summed loss below the maximum does not abort the event", m_Action->IsAborted()) && Passed;
  SilenceOutput();
  m_Action->AddEnergyLoss(20*keV);
  RestoreOutput();
  Passed = EvaluateTrue("AddEnergyLoss()", "110 of 100 keV", "A loss above the maximum aborts the event", m_Action->IsAborted()) && Passed;
  Passed = EvaluateTrue("AddEnergyLoss()", "110 of 100 keV", "The Geant4 event is aborted too", Current->IsAborted()) && Passed;
  SilenceOutput();
  m_Action->Reset();
  RestoreOutput();
  Passed = EvaluateFalse("Reset()", "after abort", "Reset clears the abort flag", m_Action->IsAborted()) && Passed;

  // Passive material and an escaping particle count as loss as well
  delete Current;
  Current = new G4Event(78);
  EA(*Base, G4RunManager, currentEvent) = Current;
  SilenceOutput();
  m_Action->AddDepositPassiveMaterial(150*keV, "Lead");
  RestoreOutput();
  Passed = EvaluateTrue("AddDepositPassiveMaterial()", "150 of 100 keV", "A deposit in passive material above the maximum aborts the event", m_Action->IsAborted() && Current->IsAborted()) && Passed;
  delete Current;
  Current = new G4Event(79);
  EA(*Base, G4RunManager, currentEvent) = Current;
  SilenceOutput();
  m_Action->Reset();
  m_Action->AddIA("ESCP", 2, 1, 0, 3.0, G4ThreeVector(), 1, G4ThreeVector(1, 0, 0), G4ThreeVector(), 200*keV, 0, G4ThreeVector(), G4ThreeVector(), 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("AddIA()", "escape of 200 keV", "An escape above the maximum aborts the event", m_Action->IsAborted() && Current->IsAborted()) && Passed;

  // Direct abort:
  delete Current;
  Current = new G4Event(80);
  EA(*Base, G4RunManager, currentEvent) = Current;
  SilenceOutput();
  m_Action->Reset();
  m_Action->AbortEvent();
  RestoreOutput();
  Passed = EvaluateTrue("AbortEvent()", "direct", "An aborted event is reported", m_Action->IsAborted() && Current->IsAborted()) && Passed;

  SilenceOutput();
  m_Action->Reset();
  RestoreOutput();
  EA(*m_Action, MCEventAction, m_StoreMaximumEnergyLoss) = OldMax;
  EA(*Base, G4RunManager, currentEvent) = nullptr;
  delete Current;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestEndOfEvent()
{
  bool Passed = true;

  MCRun& Run = m_Manager->GetCurrentRun();
  MCEventAction* Action = Scenario("Stored");
  Passed = EvaluateTrue("NextRun()", "scenario", "The scenario can be prepared", Action != nullptr) && Passed;
  if (Action == nullptr) return false;

  const long TriggeredBefore = Run.GetNTriggeredEvents();
  Run.SetSimulatedTime(12.5*s);
  G4Event Event(0);
  Action->BeginOfEventAction(&Event);
  Action->BeginOfEventAction(&Event);
  Action->AddIA("INIT", 1, 0, 0, 0.0, G4ThreeVector(), 0, G4ThreeVector(), G4ThreeVector(), 0.0, 1, G4ThreeVector(0, 0, 1), G4ThreeVector(), 511*keV);
  Action->AddComment("Stored comment");
  Action->AddDepositPassiveMaterial(100*keV, "Aluminum");
  Action->AddDepositPassiveMaterial(50*keV, "Aluminum");
  Action->AddDepositPassiveMaterial(0.5*eV, "Tiny");
  AddHit(*Action, 123.0);
  EndEvent(*Action);

  Passed = Evaluate("EndOfEventAction()", "stored event", "The triggered event is counted", Run.GetNTriggeredEvents(), TriggeredBefore + 1) && Passed;
  Passed = Evaluate("EndOfEventAction()", "stored event", "The event is reset after it was stored", EA(*Action, MCEventAction, m_Event)->GetNIAs() + EA(*Action, MCEventAction, m_Event)->GetNHTs(), 0U) && Passed;
  Passed = EvaluateNear("EndOfEventAction()", "stored event", "The lost energy is reset", EA(*Action, MCEventAction, m_LostEnergy), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("EndOfEventAction()", "stored event", "The event is not aborted", Action->IsAborted()) && Passed;

  // Second event without hits: stored as well since everything is pre-triggered
  Action->BeginOfEventAction(&Event);
  Action->AddIA("INIT", 1, 0, 0, 0.0, G4ThreeVector(), 0, G4ThreeVector(), G4ThreeVector(), 0.0, 1, G4ThreeVector(0, 0, 1), G4ThreeVector(), 662*keV);
  EndEvent(*Action);
  Passed = Evaluate("EndOfEventAction()", "event without hits", "With the pre-trigger mode everything an event without hits is stored", Run.GetNTriggeredEvents(), TriggeredBefore + 2) && Passed;

  Finish(*Action);
  const MString Text = Content(SimFile("Stored"));
  Passed = Evaluate("EndOfEventAction()", "file", "Two events are in the file", Count(Text, "\nSE\n"), 2U) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "file", "The event has the triggered ID and the simulation ID", Text.Contains(MString("ID ") + (TriggeredBefore + 1) + " 2")) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "file", "The event has the simulated time in seconds", Text.Contains("TI 12.5")) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "file", "The IA is in the file", Text.Contains("IA INIT")) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "file", "The comment is in the file", Text.Contains("CC Stored comment")) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "file", "The passive material deposit is summed and in keV", Text.Contains("PM Aluminum  150")) && Passed;
  Passed = EvaluateFalse("EndOfEventAction()", "file", "A deposit below 1 eV is not stored", Text.Contains("PM Tiny")) && Passed;
  Passed = Evaluate("EndOfEventAction()", "file", "The hit is in the file once", Count(Text, "\nHTsim "), 1U) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "file", "The passive material of the first event is not repeated in the second", Count(Text, "PM Aluminum") == 1) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "file", "The comment of the first event is not repeated in the second", Count(Text, "CC Stored comment") == 1) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "footer", "The footer has the observation end time", Text.Contains("TE 12.5")) && Passed;
  Passed = EvaluateTrue("EndOfEventAction()", "footer", "The footer has the number of simulated events", Text.Contains("TS ")) && Passed;
  delete Action;

  // Aborted events are not stored:
  Action = Scenario("Aborted");
  if (Action != nullptr) {
    G4Event* Current = new G4Event(5);
    G4RunManager* Base = m_Manager;
    EA(*Base, G4RunManager, currentEvent) = Current;
    const long Before = Run.GetNTriggeredEvents();
    Action->BeginOfEventAction(&Event);
    Action->AddIA("INIT", 1, 0, 0, 0.0, G4ThreeVector(), 0, G4ThreeVector(), G4ThreeVector(), 0.0, 1, G4ThreeVector(0, 0, 1), G4ThreeVector(), 511*keV);
    AddHit(*Action, 10.0);
    SilenceOutput();
    Action->AbortEvent();
    RestoreOutput();
    EndEvent(*Action);
    EA(*Base, G4RunManager, currentEvent) = nullptr;
    delete Current;
    Passed = Evaluate("EndOfEventAction()", "aborted event", "An aborted event is not counted as triggered", Run.GetNTriggeredEvents(), Before) && Passed;
    Passed = EvaluateFalse("EndOfEventAction()", "aborted event", "The abort flag is cleared at the end of the event", Action->IsAborted()) && Passed;
    Passed = EvaluateTrue("EndOfEventAction()", "aborted event", "The data of an aborted event is removed", EA(*Action, MCEventAction, m_Event)->GetNIAs() == 0 && EA(*Action, MCEventAction, m_Event)->GetNHTs() == 0) && Passed;
    Finish(*Action);
    Passed = EvaluateFalse("EndOfEventAction()", "aborted event", "An aborted event is not in the file", Content(SimFile("Aborted")).Contains("\nSE\n")) && Passed;
    delete Action;
  }

  // Minimum energy: events below are not stored
  Action = Scenario("MinimumEnergy");
  if (Action != nullptr) {
    EA(*Action, MCEventAction, m_StoreMinimumEnergy) = 100*keV;
    Action->BeginOfEventAction(&Event);
    AddHit(*Action, 50.0);
    EndEvent(*Action);
    Action->BeginOfEventAction(&Event);
    AddHit(*Action, 150.0);
    EndEvent(*Action);
    Finish(*Action);
    const MString MinText = Content(SimFile("MinimumEnergy"));
    // The hits are given in keV like the minimum energy of the parameter file (StoreMinimumEnergy 100 is 100 keV)
    // Known failure: m_StoreMinimumEnergy is in Geant4 units (100 keV = 0.1) but compared with the MSimEvent energy in keV, so the threshold is a factor 1000 too low
    Passed = ExpectedToFail(Evaluate("EndOfEventAction()", "minimum energy", "Only the event above the minimum energy (100 keV) is stored - the 50 keV event is not", Count(MinText, "\nSE\n"), 1U)) && Passed;
    delete Action;
  }

  // One hit per event:
  Action = Scenario("OneHit");
  if (Action != nullptr) {
    EA(*Action, MCEventAction, m_StoreOneHitPerEvent) = true;
    const long Before = Run.GetNTriggeredEvents();
    Action->BeginOfEventAction(&Event);
    AddHit(*Action, 10.0);
    AddHit(*Action, 20.0);
    AddHit(*Action, 30.0);
    EndEvent(*Action);
    Passed = Evaluate("EndOfEventAction()", "one hit per event", "Every hit is a triggered event", Run.GetNTriggeredEvents(), Before + 3) && Passed;
    Finish(*Action);
    const MString HitText = Content(SimFile("OneHit"));
    Passed = Evaluate("EndOfEventAction()", "one hit per event", "Three events are stored", Count(HitText, "\nSE\n"), 3U) && Passed;
    Passed = Evaluate("EndOfEventAction()", "one hit per event", "Every event has one hit", Count(HitText, "\nHTsim "), 3U) && Passed;
    Passed = EvaluateTrue("EndOfEventAction()", "one hit per event", "The triggered IDs are consecutive", HitText.Contains(MString("ID ") + (Before + 1) + " ") && HitText.Contains(MString("ID ") + (Before + 2) + " ") && HitText.Contains(MString("ID ") + (Before + 3) + " ")) && Passed;
    delete Action;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestPreTriggerModes()
{
  bool Passed = true;

  MCRun& Run = m_Manager->GetCurrentRun();
  G4Event Event(0);

  struct Case { int Mode; const char* Name; long Expected; };
  const Case Cases[4] = {
    { MCParameterFile::c_PreTriggerEverything, "Everything", 1 },
    { MCParameterFile::c_PreTriggerEveryEventWithHits, "EveryEventWithHits", 0 },
    { MCParameterFile::c_PreTriggerFull, "Full", 0 },
    { 99, "Unknown", 0 }
  };

  for (const Case& C: Cases) {
    MCEventAction* Action = Scenario(MString("Mode") + C.Name);
    if (Action == nullptr) {
      Passed = EvaluateTrue("NextRun()", C.Name, "The scenario can be prepared", false) && Passed;
      continue;
    }
    EA(*Action, MCEventAction, m_PreTriggerMode) = C.Mode;
    const long Before = Run.GetNTriggeredEvents();
    Action->BeginOfEventAction(&Event);
    AddHit(*Action, 100.0);
    EndEvent(*Action);
    // Without sensitive detectors there is no sensitive energy and no trigger
    Passed = Evaluate("EndOfEventAction()", MString("mode ") + C.Name, "The mode decides if an event without sensitive detector hits and without trigger is stored", Run.GetNTriggeredEvents() - Before, C.Expected) && Passed;
    Finish(*Action);
    delete Action;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestRelegator()
{
  bool Passed = true;

  G4Event Event(0);
  MCEventAction* Action = Scenario("Relegated");
  if (Action == nullptr) return false;

  gRelegated = 0;
  Action->SetRelegator(Relegator);
  Action->BeginOfEventAction(&Event);
  AddHit(*Action, 100.0);
  EndEvent(*Action);
  Passed = Evaluate("SetRelegator()", "one event", "A stored event is handed to the relegator", gRelegated, 1) && Passed;

  // Single hit events are relegated one by one
  EA(*Action, MCEventAction, m_StoreOneHitPerEvent) = true;
  Action->BeginOfEventAction(&Event);
  AddHit(*Action, 10.0);
  AddHit(*Action, 20.0);
  EndEvent(*Action);
  Passed = Evaluate("SetRelegator()", "two hits", "Single hit events are relegated one by one", gRelegated, 3) && Passed;

  Finish(*Action);
  delete Action;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestInterrupt()
{
  bool Passed = true;

  MCRun& Run = m_Manager->GetCurrentRun();
  G4Event Event(0);

  // The end of the run closes the file and writes the footer:
  MCEventAction* Action = Scenario("Interrupted");
  if (Action == nullptr) return false;
  Run.SetSimulatedTime(3.0*s);
  Action->BeginOfEventAction(&Event);
  AddHit(*Action, 100.0);
  EndEvent(*Action);
  Passed = EvaluateFalse("GetInterrupt()", "before", "There is no interrupt before it is requested", Action->GetInterrupt()) && Passed;
  Finish(*Action);
  Passed = EvaluateTrue("GetInterrupt()", "after", "The interrupt stays", Action->GetInterrupt()) && Passed;
  const MString Text = Content(SimFile("Interrupted"));
  Passed = EvaluateTrue("Interrupt()", "file", "The file contains the stored event", Text.Contains("\nSE\n")) && Passed;
  Passed = EvaluateTrue("Interrupt()", "file", "The footer has the end time", Text.Contains("TE 3")) && Passed;
  Passed = EvaluateTrue("Interrupt()", "file", "The file is complete (ends with EN)", Text.Contains("\nEN")) && Passed;
  delete Action;

  // Time out: no stored events after the time out stops the simulation, stored events prevent it
  {
    SilenceOutput();
    Run.SetFileName(m_Directory + "/TimeOut");
    MCEventAction* NoEvents = new MCEventAction(*m_Parameters, false, 1, false);
    NoEvents->NextRun();
    RestoreOutput();
    EA(*NoEvents, MCEventAction, m_PreTriggerMode) = MCParameterFile::c_PreTriggerFull;
    EA(*NoEvents, MCEventAction, m_TimeOut) = -1.0;
    NoEvents->BeginOfEventAction(&Event);
    EndEvent(*NoEvents);
    Passed = EvaluateTrue("EndOfEventAction()", "time out", "No stored events after the time out interrupts the run", NoEvents->GetInterrupt()) && Passed;
    delete NoEvents;

    SilenceOutput();
    Run.SetFileName(m_Directory + "/NoTimeOut");
    MCEventAction* WithEvents = new MCEventAction(*m_Parameters, false, 1, false);
    WithEvents->NextRun();
    RestoreOutput();
    EA(*WithEvents, MCEventAction, m_TimeOut) = -1.0;
    WithEvents->BeginOfEventAction(&Event);
    AddHit(*WithEvents, 100.0);
    EndEvent(*WithEvents);
    Passed = EvaluateFalse("EndOfEventAction()", "time out with stored events", "Stored events prevent the time out", WithEvents->GetInterrupt()) && Passed;
    delete WithEvents;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEventAction::TestFileRollOver()
{
  bool Passed = true;

  G4Event Event(0);

  // Text files:
  MCEventAction* Action = Scenario("Roll");
  if (Action != nullptr) {
    MFileEvents& File = EA(*Action, MCEventAction, m_OutFile);
    File.*UTEAGet(UTEATagm_MaxFileLength()) = 1;
    Action->BeginOfEventAction(&Event);
    AddHit(*Action, 100.0);
    EndEvent(*Action);
    Passed = EvaluateTrue("EndOfEventAction()", "file too long", "A second file is opened if the file is too long", filesystem::exists(SimFile("Roll", 2).Data())) && Passed;
    Passed = Evaluate("EndOfEventAction()", "file too long", "The action knows the new file", MString(EA(*Action, MCEventAction, m_OutFileName).c_str()), SimFile("Roll", 2)) && Passed;
    Finish(*Action);
    const MString First = Content(SimFile("Roll", 1));
    Passed = EvaluateTrue("EndOfEventAction()", "first file", "The first file points to the second", First.Contains(MString("NF ") + SimFile("Roll", 2))) && Passed;
    Passed = EvaluateTrue("EndOfEventAction()", "first file", "The first file contains the event", First.Contains("\nSE\n")) && Passed;
    const MString Second = Content(SimFile("Roll", 2));
    Passed = EvaluateTrue("EndOfEventAction()", "second file", "The second file is marked as continuation", Second.Contains("# Continued file...")) && Passed;
    delete Action;
  }

  // Parallel:
  Action = Scenario("RollParallel", false, 2);
  if (Action != nullptr) {
    MFileEvents& File = EA(*Action, MCEventAction, m_OutFile);
    File.*UTEAGet(UTEATagm_MaxFileLength()) = 1;
    Action->BeginOfEventAction(&Event);
    AddHit(*Action, 100.0);
    EndEvent(*Action);
    Passed = EvaluateTrue("EndOfEventAction()", "parallel", "The second file contains the parallel ID", filesystem::exists(SimFile("RollParallel", 2, false, 2).Data())) && Passed;
    Finish(*Action);
    delete Action;
    m_Manager->GetCurrentRun().SetParallelID(0);
  }

  // Binary files: the second file has the same format and naming as the first
  Action = Scenario("RollBinary", true);
  if (Action != nullptr) {
    Passed = EvaluateTrue("NextRun()", "binary", "The first binary file is called .id1.bin.sim", filesystem::exists(SimFile("RollBinary", 1, true).Data())) && Passed;
    MFileEvents& File = EA(*Action, MCEventAction, m_OutFile);
    File.*UTEAGet(UTEATagm_MaxFileLength()) = 1;
    Action->BeginOfEventAction(&Event);
    AddHit(*Action, 100.0);
    EndEvent(*Action);
    // Known failure: the continuation file is named .idN.sim without .bin and opened as a text file - m_StoreBinary is ignored
    Passed = ExpectedToFail(EvaluateTrue("EndOfEventAction()", "binary file too long", "The continuation of a binary file is a binary file (.id2.bin.sim)", filesystem::exists(SimFile("RollBinary", 2, true).Data()))) && Passed;
    Finish(*Action);
    delete Action;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTEventAction Test;
  return Test.Run() == true ? 0 : 1;
}
