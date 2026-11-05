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


// Access to the private members of MCRun (legal member-pointer access via explicit template instantiation)
template<typename Tag, typename Tag::type Member> struct UTRunRob { friend typename Tag::type UTRunGet(Tag) { return Member; } };
#define UTC_RUN_ACCESS(Name, Type) \
  struct UTRunTag##Name { typedef Type MCRun::* type; friend type UTRunGet(UTRunTag##Name); }; \
  template struct UTRunRob<UTRunTag##Name, &MCRun::Name>;
UTC_RUN_ACCESS(m_ActivationMode, int)
UTC_RUN_ACCESS(m_NSkippedEvents, unsigned long)
UTC_RUN_ACCESS(m_NAddedIsotopes, unsigned int)
typedef set<MCSource*, NextEmissionSort> UTRunEmissionSet;
UTC_RUN_ACCESS(m_NextEmission, UTRunEmissionSet)
UTC_RUN_ACCESS(m_SourceList, vector<MCSource*>)
#define PRIV(Run, Name) ((Run).*UTRunGet(UTRunTag##Name()))


//! Unit test class for MCRun
class UTRun : public MUnitTest
{
public:
  //! Default constructor
  UTRun() : MUnitTest("UTRun"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr), m_Parameters(nullptr) {}
  //! Default destructor
  virtual ~UTRun() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default values
  bool TestDefaults();
  //! Test all simple setters and getters
  bool TestSettersAndGetters();
  //! Test the incarnation determination
  bool TestIncarnation();
  //! Test the isotope store handling
  bool TestIsotopeStore();
  //! Test the stop conditions
  bool TestStopConditions();
  //! Test the run statistics
  bool TestStatistics();
  //! Test the skipping of events
  bool TestSkipOneEvent();
  //! Test Initialize
  bool TestInitialize();
  //! Test the event list of the build-up sources
  bool TestBuildUpEventList();
  //! Test the generation of primaries
  bool TestGeneratePrimaries();

  //! Silence cout and cerr
  void SilenceOutput();
  //! Restore cout and cerr
  void RestoreOutput();
  //! Initialize the Geant4 particles
  void InitializeGeant4();
  //! Create the parameter file
  bool CreateParameters();
  //! Return true if the file contains the text
  bool FileContains(const MString& FileName, const MString& Text);
  //! Parse an orientation file in galactic coordinates
  bool ParseOrientation(MCOrientation& Orientation, const MString& Name, const MString& Content, const MString& Mode);

  //! The buffers of the silenced streams
  streambuf* m_CoutBuffer;
  streambuf* m_CerrBuffer;
  ostringstream m_Sink;
  //! The parameters
  MCParameterFile* m_Parameters;
  //! The manager
  MCRunManager* m_Manager = nullptr;
  //! Base name of the run files
  MString m_RunBaseName;
};


////////////////////////////////////////////////////////////////////////////////


bool UTRun::Run()
{
  bool Passed = true;

  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  // Tests which do not need Geant4:
  Passed = TestDefaults() && Passed;
  Passed = TestSettersAndGetters() && Passed;
  Passed = TestIncarnation() && Passed;
  Passed = TestStopConditions() && Passed;
  Passed = TestStatistics() && Passed;

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
  MCEventAction* EventAction = new MCEventAction(*m_Parameters, false, 1, true);
  MCTrackingAction* TrackingAction = new MCTrackingAction();
  MCSteppingAction* SteppingAction = new MCSteppingAction(*m_Parameters);
  MCDetectorConstruction* Detector = new MCDetectorConstruction(*m_Parameters);
  m_Manager->SetUserAction(EventAction);
  m_Manager->SetUserAction(TrackingAction);
  m_Manager->SetUserAction(SteppingAction);
  const bool GeometryLoaded = Detector->Initialize();
  m_Manager->SetUserInitialization(Detector);
  RestoreOutput();
  Passed = EvaluateTrue("MCDetectorConstruction::Initialize()", "test geometry", "The test geometry can be loaded", GeometryLoaded) && Passed;

  Passed = TestSkipOneEvent() && Passed;
  Passed = TestIsotopeStore() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestBuildUpEventList() && Passed;
  Passed = TestGeneratePrimaries() && Passed;

  SilenceOutput();
  delete m_Manager;
  delete m_Parameters;
  RestoreOutput();

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTRun::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTRun::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


void UTRun::InitializeGeant4()
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


bool UTRun::FileContains(const MString& FileName, const MString& Text)
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


bool UTRun::CreateParameters()
{
  PrepareTemporaryDirectory("Files");
  const MString Directory = GetTemporaryDirectoryName("Files");
  // The output name in the parameter file is relative to the private temporary root, which is the working directory of the test:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  m_RunBaseName = "Files/UTRunRun";

  // Two sources: A has the low flux and is defined first, B has the high flux
  MString Content = "Version 1\nGeometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\nDetectorTimeConstant 0.000005\nPhysicsListEM LivermorePol\nStoreSimulationInfo init\n";
  Content += MString("Run Base\nBase.FileName ") + m_RunBaseName + "\nBase.Time 10\nBase.Source A\nBase.Source B\n";
  Content += "A.ParticleType 1\nA.Beam PointSource 0 0 0\nA.Spectrum Mono 511\nA.Flux 1\n";
  Content += "B.ParticleType 1\nB.Beam PointSource 0 0 0\nB.Spectrum Mono 662\nB.Flux 1000\n";
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


bool UTRun::ParseOrientation(MCOrientation& Orientation, const MString& Name, const MString& Content, const MString& Mode)
{
  // The file is written below the private temporary root, which is the working directory of the test, and the parser gets the relative name:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  if (WriteTextFile(GetTemporaryFileName(Name), Content) == false) return false;

  MTokenizer Tokenizer;
  Tokenizer.Analyse(MString("Run Orientation Galactic File ") + Mode + " " + Name, false);
  DisableDefaultStreams();
  bool Result = Orientation.Parse(Tokenizer);
  EnableDefaultStreams();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestDefaults()
{
  bool Passed = true;

  MCRun Run;
  Passed = Evaluate("MCRun()", "default", "The default name is empty", Run.GetName(), MString("")) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default file name is empty", Run.GetFileName(), MString("")) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default geometry file name is empty", Run.GetGeometryFileName(), MString("")) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default parallel ID is 0", Run.GetParallelID(), 0) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default incarnation ID is 0", Run.GetIncarnationID(), 0) && Passed;
  Passed = EvaluateFalse("MCRun()", "default", "The default is not to zip", Run.GetZip()) && Passed;
  Passed = EvaluateFalse("MCRun()", "default", "The default is not to store binary", Run.GetStoreBinary()) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default TCP/IP host name is empty", Run.GetTcpIpHostName(), MString("")) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default TCP/IP port is 9090", Run.GetTcpIpPort(), 9090U) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default revan configuration is empty", Run.GetRevanConfigurationFileName(), MString("")) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default tra file is empty", Run.GetTraFileName(), MString("")) && Passed;
  Passed = EvaluateNear("MCRun()", "default", "The default duration is the maximum", Run.GetDuration(), numeric_limits<double>::max(), 1.0) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default number of triggers is the maximum", Run.GetTriggers(), numeric_limits<long>::max()) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default number of events is the maximum", Run.GetEvents(), numeric_limits<long>::max()) && Passed;
  Passed = Evaluate("MCRun()", "default", "The default stop condition is by triggers", Run.GetStopCondition(), MCRun::c_StopByTriggers) && Passed;
  Passed = EvaluateNear("MCRun()", "default", "The simulated time is 0", Run.GetSimulatedTime(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCRun()", "default", "No events are simulated", Run.GetNSimulatedEvents(), 0L) && Passed;
  Passed = Evaluate("MCRun()", "default", "No events are triggered", Run.GetNTriggeredEvents(), 0L) && Passed;
  Passed = Evaluate("MCRun()", "default", "No particles are generated", Run.GetNGeneratedParticles(), 0) && Passed;
  Passed = Evaluate("MCRun()", "default", "No sources", Run.GetNSources(), 0U) && Passed;
  Passed = EvaluateFalse("MCRun()", "default", "The default sky orientation is not oriented", Run.GetSkyOrientation().IsOriented()) && Passed;
  Passed = EvaluateFalse("MCRun()", "default", "The default detector orientation is not oriented", Run.GetDetectorOrientation().IsOriented()) && Passed;
  Passed = Evaluate("MCRun()", "stop IDs", "The stop IDs are different", (MCRun::c_StopByTriggers != MCRun::c_StopByEvents) && (MCRun::c_StopByEvents != MCRun::c_StopByTime) && (MCRun::c_StopByTriggers != MCRun::c_StopByTime), true) && Passed;

  // The activation mode is never initialized in the constructor:
  // Construct the run in memory filled with two different patterns - a deterministic value must not depend on the pattern
  const size_t Size = sizeof(MCRun);
  vector<unsigned char> BufferA(Size + 64, 0x00);
  vector<unsigned char> BufferB(Size + 64, 0xFF);
  void* PointerA = (void*) (((uintptr_t) BufferA.data() + 31) & ~((uintptr_t) 31));
  void* PointerB = (void*) (((uintptr_t) BufferB.data() + 31) & ~((uintptr_t) 31));
  memset(PointerA, 0x00, Size);
  memset(PointerB, 0xFF, Size);
  MCRun* RunA = new (PointerA) MCRun();
  MCRun* RunB = new (PointerB) MCRun();
  const int ModeA = PRIV(*RunA, m_ActivationMode);
  const int ModeB = PRIV(*RunB, m_ActivationMode);
  const long SkippedA = (long) PRIV(*RunA, m_NSkippedEvents);
  const long SkippedB = (long) PRIV(*RunB, m_NSkippedEvents);
  RunA->~MCRun();
  RunB->~MCRun();
  // Known failure: m_ActivationMode is not initialized by the constructor
  Passed = ExpectedToFail(Evaluate("MCRun()", "memory pattern", "The activation mode is initialized by the constructor", ModeA == ModeB, true)) && Passed;
  Passed = Evaluate("MCRun()", "memory pattern", "The skipped events are initialized by the constructor", SkippedA == SkippedB, true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestSettersAndGetters()
{
  bool Passed = true;

  MCRun Run;
  Run.SetName("Name");
  Passed = Evaluate("SetName()", "Name", "The name is stored", Run.GetName(), MString("Name")) && Passed;
  Passed = EvaluateTrue("SetFileName()", "file", "Setting the file name returns true", Run.SetFileName("File")) && Passed;
  Passed = Evaluate("SetFileName()", "file", "The file name is stored", Run.GetFileName(), MString("File")) && Passed;
  Passed = EvaluateTrue("SetGeometryFileName()", "geometry", "Setting the geometry file name returns true", Run.SetGeometryFileName("Geo.setup")) && Passed;
  Passed = Evaluate("SetGeometryFileName()", "geometry", "The geometry file name is stored", Run.GetGeometryFileName(), MString("Geo.setup")) && Passed;
  Run.SetParallelID(7);
  Passed = Evaluate("SetParallelID()", "7", "The parallel ID is stored", Run.GetParallelID(), 7) && Passed;
  Run.SetIncarnationID(3);
  Passed = Evaluate("SetIncarnationID()", "3", "The incarnation ID is stored", Run.GetIncarnationID(), 3) && Passed;
  Passed = EvaluateTrue("SetZip()", "true", "Setting zip returns true", Run.SetZip(true)) && Passed;
  Passed = EvaluateTrue("SetZip()", "true", "Zip is stored", Run.GetZip()) && Passed;
  Passed = EvaluateTrue("SetStoreBinary()", "true", "Setting binary returns true", Run.SetStoreBinary(true)) && Passed;
  Passed = EvaluateTrue("SetStoreBinary()", "true", "Binary is stored", Run.GetStoreBinary()) && Passed;
  Passed = EvaluateTrue("SetTcpIpHostName()", "host", "Setting the host returns true", Run.SetTcpIpHostName("localhost")) && Passed;
  Passed = Evaluate("SetTcpIpHostName()", "host", "The host is stored", Run.GetTcpIpHostName(), MString("localhost")) && Passed;
  Passed = EvaluateTrue("SetTcpIpPort()", "port", "Setting the port returns true", Run.SetTcpIpPort(1234)) && Passed;
  Passed = Evaluate("SetTcpIpPort()", "port", "The port is stored", Run.GetTcpIpPort(), 1234U) && Passed;
  Passed = EvaluateTrue("SetRevanConfigurationFileName()", "revan", "Setting the revan file returns true", Run.SetRevanConfigurationFileName("Revan.cfg")) && Passed;
  Passed = Evaluate("SetRevanConfigurationFileName()", "revan", "The revan file is stored", Run.GetRevanConfigurationFileName(), MString("Revan.cfg")) && Passed;
  Passed = EvaluateTrue("SetTraFileName()", "tra", "Setting the tra file returns true", Run.SetTraFileName("File.tra")) && Passed;
  Passed = Evaluate("SetTraFileName()", "tra", "The tra file is stored", Run.GetTraFileName(), MString("File.tra")) && Passed;
  Passed = EvaluateTrue("SetActivationMode()", "mode", "Setting the activation mode returns true", Run.SetActivationMode(2)) && Passed;
  Passed = Evaluate("SetActivationMode()", "mode", "The activation mode is stored", PRIV(Run, m_ActivationMode), 2) && Passed;

  // Stop conditions:
  MCRun Stops;
  Passed = EvaluateFalse("SetDuration()", "0", "A duration of 0 is rejected", Stops.SetDuration(0.0)) && Passed;
  Passed = EvaluateFalse("SetDuration()", "-1", "A negative duration is rejected", Stops.SetDuration(-1.0)) && Passed;
  Passed = Evaluate("SetDuration()", "rejected", "A rejected duration does not change the stop condition", Stops.GetStopCondition(), MCRun::c_StopByTriggers) && Passed;
  Passed = EvaluateFalse("SetTriggers()", "0", "0 triggers are rejected", Stops.SetTriggers(0)) && Passed;
  Passed = EvaluateFalse("SetTriggers()", "-1", "Negative triggers are rejected", Stops.SetTriggers(-1)) && Passed;
  Passed = EvaluateFalse("SetEvents()", "0", "0 events are rejected", Stops.SetEvents(0)) && Passed;
  Passed = EvaluateFalse("SetEvents()", "-1", "Negative events are rejected", Stops.SetEvents(-1)) && Passed;
  Passed = Evaluate("SetEvents()", "rejected", "Rejected values do not change the defaults", Stops.GetEvents() == numeric_limits<long>::max() && Stops.GetTriggers() == numeric_limits<long>::max(), true) && Passed;

  Passed = EvaluateTrue("SetDuration()", "10 s", "A duration can be set", Stops.SetDuration(10*s)) && Passed;
  Passed = EvaluateNear("SetDuration()", "10 s", "The duration is stored", Stops.GetDuration(), 10*s, 1e-6) && Passed;
  Passed = Evaluate("SetDuration()", "10 s", "A duration sets the stop condition to time", Stops.GetStopCondition(), MCRun::c_StopByTime) && Passed;
  Passed = EvaluateTrue("SetEvents()", "5", "Events can be set", Stops.SetEvents(5)) && Passed;
  Passed = Evaluate("SetEvents()", "5", "The events are stored", Stops.GetEvents(), 5L) && Passed;
  Passed = Evaluate("SetEvents()", "5", "Events set the stop condition to events", Stops.GetStopCondition(), MCRun::c_StopByEvents) && Passed;
  Passed = EvaluateTrue("SetTriggers()", "6", "Triggers can be set", Stops.SetTriggers(6)) && Passed;
  Passed = Evaluate("SetTriggers()", "6", "The triggers are stored", Stops.GetTriggers(), 6L) && Passed;
  Passed = Evaluate("SetTriggers()", "6", "Triggers set the stop condition to triggers", Stops.GetStopCondition(), MCRun::c_StopByTriggers) && Passed;

  // Counters:
  Stops.SetNGeneratedParticles(5);
  Stops.AddNGeneratedParticles(3);
  Passed = Evaluate("AddNGeneratedParticles()", "5 + 3", "Generated particles add up", Stops.GetNGeneratedParticles(), 8) && Passed;
  Stops.AddSimulatedEvent();
  Stops.AddSimulatedEvent();
  Stops.AddTriggeredEvent();
  Passed = Evaluate("AddSimulatedEvent()", "2", "Simulated events are counted", Stops.GetNSimulatedEvents(), 2L) && Passed;
  Passed = Evaluate("AddTriggeredEvent()", "1", "Triggered events are counted", Stops.GetNTriggeredEvents(), 1L) && Passed;
  Stops.SetSimulatedTime(5*s);
  Passed = EvaluateNear("SetSimulatedTime()", "5 s", "The simulated time is stored", Stops.GetSimulatedTime(), 5*s, 1e-6) && Passed;

  // Orientations:
  MCOrientation Orientation;
  MCOrientation FileOrientation;
  Passed = EvaluateTrue("ParseOrientation()", "file", "The orientation file can be parsed", ParseOrientation(FileOrientation, "Setters.ori", "OG 10 0 0 0 90\nOG 20 0 0 0 90\n", "NoLoop")) && Passed;
  Passed = EvaluateTrue("SetSkyOrientation()", "file", "Setting the sky orientation returns true", Run.SetSkyOrientation(FileOrientation)) && Passed;
  Passed = EvaluateTrue("GetSkyOrientation()", "file", "The sky orientation is stored", Run.GetSkyOrientation().IsOriented() && Run.GetSkyOrientationReference().IsOriented()) && Passed;
  Passed = EvaluateNear("GetSkyOrientation()", "file", "The sky orientation keeps its stop time", Run.GetSkyOrientation().GetStopTime(), 20*s, 1e-3) && Passed;
  Passed = EvaluateTrue("SetDetectorOrientation()", "file", "Setting the detector orientation returns true", Run.SetDetectorOrientation(FileOrientation)) && Passed;
  Passed = EvaluateTrue("GetDetectorOrientation()", "file", "The detector orientation is stored", Run.GetDetectorOrientation().IsOriented() && Run.GetDetectorOrientationReference().IsOriented()) && Passed;
  Passed = EvaluateFalse("SetSkyOrientation()", "independent", "The sky and detector orientations are independent", MCRun().GetSkyOrientation().IsOriented()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestIncarnation()
{
  bool Passed = true;

  PrepareTemporaryDirectory("Incarnation");
  const MString Dir = GetTemporaryDirectoryName("Incarnation");

  auto Check = [&](MCRun& Run) {
    DisableDefaultStreams();
    ostringstream Sink;
    streambuf* Old = cout.rdbuf(Sink.rdbuf());
    Run.CheckIncarnationID();
    cout.rdbuf(Old);
    EnableDefaultStreams();
  };
  auto Touch = [&](const MString& Name) {
    ofstream Out(Name.Data());
    Out<<"x"<<endl;
  };

  // Without a file name nothing happens:
  {
    MCRun Run;
    Check(Run);
    Passed = Evaluate("CheckIncarnationID()", "no file name", "Without a file name the incarnation stays 0", Run.GetIncarnationID(), 0) && Passed;
  }

  // First incarnation:
  {
    MCRun Run;
    Run.SetFileName(Dir + "/Fresh");
    Check(Run);
    Passed = Evaluate("CheckIncarnationID()", "fresh", "The first incarnation is 1", Run.GetIncarnationID(), 1) && Passed;
    Passed = EvaluateTrue("CheckIncarnationID()", "fresh", "A placeholder file is created immediately", filesystem::exists((Dir + "/Fresh.inc1.id1.sim").Data())) && Passed;
    Passed = EvaluateTrue("CheckIncarnationID()", "fresh", "The placeholder tells that it can be deleted", FileContains(Dir + "/Fresh.inc1.id1.sim", "You can delete me")) && Passed;

    // The placeholder is found by the next run:
    MCRun Next;
    Next.SetFileName(Dir + "/Fresh");
    Check(Next);
    Passed = Evaluate("CheckIncarnationID()", "second run", "The second run gets incarnation 2", Next.GetIncarnationID(), 2) && Passed;
    Passed = EvaluateTrue("CheckIncarnationID()", "second run", "The second placeholder is created", filesystem::exists((Dir + "/Fresh.inc2.id1.sim").Data())) && Passed;
  }

  // Existing files with the different extensions:
  {
    Touch(Dir + "/Ext.inc1.id1.sim.gz");
    Touch(Dir + "/Ext.inc2.id1.sim.zip");
    MCRun Run;
    Run.SetFileName(Dir + "/Ext");
    Check(Run);
    Passed = Evaluate("CheckIncarnationID()", "sim.gz and sim.zip", "Existing zipped files are skipped", Run.GetIncarnationID(), 3) && Passed;
  }

  // A later file of an earlier incarnation (several files per incarnation):
  {
    Touch(Dir + "/Multi.inc1.id2.sim");
    MCRun Run;
    Run.SetFileName(Dir + "/Multi");
    Check(Run);
    Passed = Evaluate("CheckIncarnationID()", "id2 only", "An existing second file of the incarnation 1 forces incarnation 2", Run.GetIncarnationID(), 2) && Passed;
  }

  // Parallel IDs:
  {
    Touch(Dir + "/Par.p3.inc1.id1.sim");
    MCRun Run;
    Run.SetFileName(Dir + "/Par");
    Run.SetParallelID(3);
    Check(Run);
    Passed = Evaluate("CheckIncarnationID()", "parallel", "Existing files of the parallel ID are skipped", Run.GetIncarnationID(), 2) && Passed;
    Passed = EvaluateTrue("CheckIncarnationID()", "parallel", "The placeholder contains the parallel ID", filesystem::exists((Dir + "/Par.p3.inc2.id1.sim").Data())) && Passed;
    MCRun Other;
    Other.SetFileName(Dir + "/Par");
    Other.SetParallelID(4);
    Check(Other);
    Passed = Evaluate("CheckIncarnationID()", "other parallel ID", "Files of another parallel ID do not matter", Other.GetIncarnationID(), 1) && Passed;
  }

  // A fixed incarnation is not searched:
  {
    Touch(Dir + "/Fixed.inc5.id1.sim");
    MCRun Run;
    Run.SetFileName(Dir + "/Fixed");
    Run.SetIncarnationID(5);
    Check(Run);
    Passed = Evaluate("CheckIncarnationID()", "fixed", "A fixed incarnation is kept", Run.GetIncarnationID(), 5) && Passed;
    Run.SetIncarnationID(0);
    Check(Run);
    Passed = Evaluate("CheckIncarnationID()", "unfixed", "An incarnation of 0 switches to the automatic search", Run.GetIncarnationID(), 1) && Passed;
  }

  // Zipped output gets a zipped placeholder name:
  {
    MCRun Run;
    Run.SetFileName(Dir + "/Zip");
    Run.SetZip(true);
    Check(Run);
    Passed = EvaluateTrue("CheckIncarnationID()", "zip", "The placeholder of a zipped run ends with .sim.gz", filesystem::exists((Dir + "/Zip.inc1.id1.sim.gz").Data())) && Passed;
    MCRun Next;
    Next.SetFileName(Dir + "/Zip");
    Check(Next);
    Passed = Evaluate("CheckIncarnationID()", "zip", "The placeholder of a zipped run is found", Next.GetIncarnationID(), 2) && Passed;
  }

  // Binary output: the placeholder is called <name>.incN.id1.bin.sim - an existing one must be found
  {
    MCRun Run;
    Run.SetFileName(Dir + "/Bin");
    Run.SetStoreBinary(true);
    Check(Run);
    Passed = EvaluateTrue("CheckIncarnationID()", "binary", "The placeholder of a binary run is called .id1.bin.sim", filesystem::exists((Dir + "/Bin.inc1.id1.bin.sim").Data())) && Passed;
    Passed = Evaluate("CheckIncarnationID()", "binary", "The first binary incarnation is 1", Run.GetIncarnationID(), 1) && Passed;
    MCRun Next;
    Next.SetFileName(Dir + "/Bin");
    Next.SetStoreBinary(true);
    Check(Next);
    // Known failure: the incarnation search appends ".bin" after the trailing dot and looks for "<name>.incN.id1..binsim", so binary placeholders are never found
    Passed = ExpectedToFail(Evaluate("CheckIncarnationID()", "binary second run", "The placeholder of a binary run is found by the next run (otherwise the files are overwritten)", Next.GetIncarnationID(), 2)) && Passed;
  }
  {
    // The second file of a binary incarnation:
    Touch(Dir + "/BinMulti.inc1.id2.bin.sim");
    MCRun Run;
    Run.SetFileName(Dir + "/BinMulti");
    Run.SetStoreBinary(true);
    Check(Run);
    // Known failure: the search for further files of an incarnation (id2, ...) never adds ".bin" and does not see binary files
    Passed = ExpectedToFail(Evaluate("CheckIncarnationID()", "binary id2", "An existing second binary file of incarnation 1 forces incarnation 2", Run.GetIncarnationID(), 2)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestStopConditions()
{
  bool Passed = true;

  // A run without sources has no active source:
  {
    MCRun Run;
    DisableDefaultStreams();
    bool Result = Run.CheckStopConditions();
    EnableDefaultStreams();
    Passed = EvaluateTrue("CheckStopConditions()", "no sources", "A run without sources is finished", Result) && Passed;
  }

  MCRun Run;
  Run.AddSource("Active");
  auto Check = [&]() {
    DisableDefaultStreams();
    ostringstream Sink;
    streambuf* Old = cout.rdbuf(Sink.rdbuf());
    bool Result = Run.CheckStopConditions();
    cout.rdbuf(Old);
    EnableDefaultStreams();
    return Result;
  };

  // Triggers:
  Run.SetTriggers(3);
  Passed = EvaluateFalse("CheckStopConditions()", "0 of 3 triggers", "No stop before the triggers", Check()) && Passed;
  Run.AddTriggeredEvent();
  Run.AddTriggeredEvent();
  Passed = EvaluateFalse("CheckStopConditions()", "2 of 3 triggers", "No stop before the triggers", Check()) && Passed;
  Run.AddSimulatedEvent();
  Run.AddSimulatedEvent();
  Run.AddSimulatedEvent();
  Run.AddSimulatedEvent();
  Passed = EvaluateFalse("CheckStopConditions()", "many events, 2 of 3 triggers", "Simulated events do not count for the trigger condition", Check()) && Passed;
  Run.AddTriggeredEvent();
  Passed = EvaluateTrue("CheckStopConditions()", "3 of 3 triggers", "Stop at the number of triggers", Check()) && Passed;

  // Events:
  MCRun EventRun;
  EventRun.AddSource("Active");
  EventRun.SetEvents(3);
  auto CheckEvents = [&]() {
    DisableDefaultStreams();
    ostringstream Sink;
    streambuf* Old = cout.rdbuf(Sink.rdbuf());
    bool Result = EventRun.CheckStopConditions();
    cout.rdbuf(Old);
    EnableDefaultStreams();
    return Result;
  };
  EventRun.AddSimulatedEvent();
  EventRun.AddSimulatedEvent();
  Passed = EvaluateFalse("CheckStopConditions()", "2 of 3 events", "No stop before the events", CheckEvents()) && Passed;
  EventRun.AddTriggeredEvent();
  EventRun.AddTriggeredEvent();
  EventRun.AddTriggeredEvent();
  EventRun.AddTriggeredEvent();
  Passed = EvaluateFalse("CheckStopConditions()", "many triggers", "Triggers do not count for the event condition", CheckEvents()) && Passed;
  EventRun.AddSimulatedEvent();
  Passed = EvaluateTrue("CheckStopConditions()", "3 of 3 events", "Stop at the number of events", CheckEvents()) && Passed;

  // Time (internal Geant4 units):
  MCRun TimeRun;
  TimeRun.AddSource("Active");
  TimeRun.SetDuration(10*s);
  auto CheckTime = [&]() {
    DisableDefaultStreams();
    ostringstream Sink;
    streambuf* Old = cout.rdbuf(Sink.rdbuf());
    bool Result = TimeRun.CheckStopConditions();
    cout.rdbuf(Old);
    EnableDefaultStreams();
    return Result;
  };
  TimeRun.SetSimulatedTime(9.99*s);
  Passed = EvaluateFalse("CheckStopConditions()", "9.99 of 10 s", "No stop before the duration", CheckTime()) && Passed;
  TimeRun.SetSimulatedTime(10*s);
  Passed = EvaluateTrue("CheckStopConditions()", "10 of 10 s", "Stop at the duration", CheckTime()) && Passed;
  TimeRun.SetSimulatedTime(11*s);
  Passed = EvaluateTrue("CheckStopConditions()", "11 of 10 s", "Stop after the duration", CheckTime()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestStatistics()
{
  bool Passed = true;

  MCRun Run;
  Run.SetName("Stats");
  Run.SetNGeneratedParticles(10);
  Run.SetSimulatedTime(12.5*s);

  auto Dump = [&](MCRun& R, double CPU) {
    DisableDefaultStreams();
    ostringstream Sink;
    streambuf* Old = cout.rdbuf(Sink.rdbuf());
    R.DumpRunStatistics(CPU);
    cout.rdbuf(Old);
    EnableDefaultStreams();
    return MString(Sink.str().c_str());
  };

  MString Text = Dump(Run, 5.0);
  Passed = EvaluateTrue("DumpRunStatistics()", "no sources", "The run name is dumped", Text.Contains("Summary for run Stats")) && Passed;
  Passed = EvaluateTrue("DumpRunStatistics()", "10 particles", "The generated particles are dumped", Text.Contains("Total number of generated particles:     10")) && Passed;
  Passed = EvaluateTrue("DumpRunStatistics()", "5 s CPU", "The CPU time is dumped", Text.Contains("Total CPU time spent in run:             5 sec")) && Passed;
  Passed = EvaluateTrue("DumpRunStatistics()", "5 s CPU", "The time per event is the CPU time per particle", Text.Contains("Time spent per event:                    0.5 sec")) && Passed;
  Passed = EvaluateTrue("DumpRunStatistics()", "12.5 s", "The observation time is dumped in seconds", Text.Contains("Observation time:                        12.5 sec")) && Passed;

  MCRun Empty;
  Text = Dump(Empty, 5.0);
  Passed = EvaluateTrue("DumpRunStatistics()", "no particles", "Without particles the time per event is 0 instead of a division by zero", Text.Contains("Time spent per event:                    0 sec")) && Passed;

  // The sources are sorted by the number of generated particles, then by name - the source list itself stays
  Run.AddSource("Zeta");
  Run.AddSource("Alpha");
  Text = Dump(Run, 1.0);
  const int PosAlpha = (int) string(Text.Data()).find("Source Alpha");
  const int PosZeta = (int) string(Text.Data()).find("Source Zeta");
  Passed = EvaluateTrue("DumpRunStatistics()", "two sources", "Both sources are listed", PosAlpha >= 0 && PosZeta >= 0) && Passed;
  Passed = EvaluateTrue("DumpRunStatistics()", "same counts", "Sources with the same number of particles are sorted by name", PosAlpha >= 0 && PosAlpha < PosZeta) && Passed;
  Passed = Evaluate("DumpRunStatistics()", "sorted output", "The dump does not reorder the source list", Run.GetSource(0)->GetName(), MString("Zeta")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestSkipOneEvent()
{
  bool Passed = true;

  MCRun Run;
  SilenceOutput();
  Run.AddSource("Gamma1");
  Run.AddSource("Gamma2");
  Run.AddSource("Electron");
  Run.AddSource("Gamma1Copy");
  RestoreOutput();
  const MString Volumes[4] = { "VolA", "VolB", "VolA", "VolA" };
  const int Types[4] = { 1, 1, 2, 1 };
  for (unsigned int i = 0; i < 4; ++i) {
    Run.GetSource(i)->SetParticleType(Types[i]);
    Run.GetSource(i)->SetVolume(Volumes[i]);
  }
  SilenceOutput();
  bool AllGenerated = true;
  for (unsigned int i = 0; i < 4; ++i) AllGenerated = Run.GetSource(i)->GenerateParticleDefinition() && AllGenerated;
  RestoreOutput();
  Passed = EvaluateTrue("GenerateParticleDefinition()", "sources", "The particle definitions of the test sources exist", AllGenerated) && Passed;

  G4ParticleDefinition* Gamma = Run.GetSource(0)->GetParticleDefinition();
  G4ParticleDefinition* Electron = Run.GetSource(2)->GetParticleDefinition();
  Passed = EvaluateTrue("SkipOneEvent()", "definitions", "Gamma and electron are different particles", Gamma != nullptr && Electron != nullptr && Gamma != Electron) && Passed;

  SilenceOutput();
  bool Result = Run.SkipOneEvent(Gamma, "VolB");
  RestoreOutput();
  Passed = EvaluateTrue("SkipOneEvent()", "gamma in VolB", "Skipping returns true", Result) && Passed;
  Passed = Evaluate("SkipOneEvent()", "gamma in VolB", "The matching source skips one event", Run.GetSource(1)->GetNEventsToSkip(), 1U) && Passed;
  Passed = Evaluate("SkipOneEvent()", "gamma in VolB", "The other sources skip nothing", Run.GetSource(0)->GetNEventsToSkip() + Run.GetSource(2)->GetNEventsToSkip() + Run.GetSource(3)->GetNEventsToSkip(), 0U) && Passed;

  SilenceOutput();
  Run.SkipOneEvent(Gamma, "VolA");
  Run.SkipOneEvent(Gamma, "VolA");
  Run.SkipOneEvent(Electron, "VolA");
  RestoreOutput();
  Passed = Evaluate("SkipOneEvent()", "gamma in VolA twice", "The first matching source counts all skips", Run.GetSource(0)->GetNEventsToSkip(), 2U) && Passed;
  Passed = Evaluate("SkipOneEvent()", "gamma in VolA twice", "Only one of identical sources is used", Run.GetSource(3)->GetNEventsToSkip(), 0U) && Passed;
  Passed = Evaluate("SkipOneEvent()", "electron in VolA", "The electron source skips one event", Run.GetSource(2)->GetNEventsToSkip(), 1U) && Passed;

  ostringstream Captured;
  streambuf* OldCout = cout.rdbuf(Captured.rdbuf());
  Result = Run.SkipOneEvent(Electron, "VolB");
  cout.rdbuf(OldCout);
  const MString Message = Captured.str().c_str();
  Passed = EvaluateTrue("SkipOneEvent()", "not found", "A missing source is reported", Message.Contains("Source to skip not found")) && Passed;
  Passed = Evaluate("SkipOneEvent()", "not found", "A missing source changes no counter", Run.GetSource(0)->GetNEventsToSkip() + Run.GetSource(1)->GetNEventsToSkip() + Run.GetSource(2)->GetNEventsToSkip() + Run.GetSource(3)->GetNEventsToSkip(), 4U) && Passed;

  // Sources
  Passed = EvaluateTrue("AddSource()", "by name", "Adding a source by name works without a test", Run.AddSource("Gamma1", false)) && Passed;
  Passed = Evaluate("AddSource()", "duplicate without test", "Duplicate names are added without test", Run.GetNSources(), 5U) && Passed;
  Passed = EvaluateFalse("AddSource()", "duplicate with test", "A duplicate name is rejected with the test", Run.AddSource("Gamma2", true)) && Passed;
  Passed = Evaluate("AddSource()", "duplicate with test", "A rejected source is not added", Run.GetNSources(), 5U) && Passed;
  Passed = EvaluateTrue("AddSource()", "new with test", "A new name is accepted with the test", Run.AddSource("Fresh", true)) && Passed;
  MCSource* Own = new MCSource("Own");
  Passed = EvaluateTrue("AddSource()", "pointer", "A source pointer can be added", Run.AddSource(Own, true)) && Passed;
  Passed = EvaluateTrue("GetSource()", "pointer", "The source pointer is stored", Run.GetSource(Run.GetNSources() - 1) == Own) && Passed;
  Passed = EvaluateFalse("AddSource()", "pointer duplicate", "A second source with the same name is rejected by pointer too", Run.AddSource(new MCSource("Own"), true)) && Passed;
  Passed = EvaluateTrue("GetSourceList()", "list", "The list and the count agree", Run.GetSourceList().size() == Run.GetNSources()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestIsotopeStore()
{
  bool Passed = true;

  PrepareTemporaryDirectory("Isotopes");
  const MString Dir = GetTemporaryDirectoryName("Isotopes");

  // No name: no file is written and nothing crashes
  {
    MCRun Run;
    SilenceOutput();
    Run.SaveIsotopeStore();
    RestoreOutput();
    Passed = Evaluate("SaveIsotopeStore()", "no name", "Nothing is written without a name", (long) distance(filesystem::directory_iterator(Dir.Data()), filesystem::directory_iterator()), 0L) && Passed;
  }

  // With a name, the incarnation and the time are used:
  {
    MCRun Run;
    Run.SetIsotopeStoreFileName(Dir + "/Store");
    Run.SetIncarnationID(4);
    Run.SetSimulatedTime(42*s);
    SilenceOutput();
    Run.SaveIsotopeStore();
    RestoreOutput();
    Passed = EvaluateTrue("SaveIsotopeStore()", "name", "The store is saved with the incarnation", filesystem::exists((Dir + "/Store.inc4.dat").Data())) && Passed;
    Passed = EvaluateTrue("SaveIsotopeStore()", "time", "The store contains the simulated time", FileContains(Dir + "/Store.inc4.dat", "TT 42")) && Passed;
    // A second save reuses the name:
    Run.SetSimulatedTime(50*s);
    SilenceOutput();
    Run.SaveIsotopeStore();
    RestoreOutput();
    Passed = EvaluateTrue("SaveIsotopeStore()", "second save", "A second save updates the same file", FileContains(Dir + "/Store.inc4.dat", "TT 50")) && Passed;
    Passed = EvaluateFalse("SaveIsotopeStore()", "second save", "The suffix is not added twice", filesystem::exists((Dir + "/Store.inc4.dat.inc4.dat").Data())) && Passed;
  }
  {
    MCRun Run;
    Run.SetIsotopeStoreFileName(Dir + "/Par");
    Run.SetParallelID(2);
    Run.SetIncarnationID(3);
    SilenceOutput();
    Run.SaveIsotopeStore();
    RestoreOutput();
    Passed = EvaluateTrue("SaveIsotopeStore()", "parallel", "The store name contains the parallel ID", filesystem::exists((Dir + "/Par.p2.inc3.dat").Data())) && Passed;
  }

  // Adding isotopes with a touchable:
  {
    G4Material* Material = new G4Material("UTRunMaterial", 1.0, 1.008*g/mole, 1.0*g/cm3);
    G4Box* Box = new G4Box("UTRunBox", 1*cm, 1*cm, 1*cm);
    G4LogicalVolume* Logical = new G4LogicalVolume(Box, Material, "CrystalLog");
    G4PVPlacement* Placement = new G4PVPlacement(nullptr, G4ThreeVector(), Logical, "Crystal", nullptr, false, 0, false);
    G4NavigationHistory History;
    History.SetFirstEntry(Placement);
    G4TouchableHistory Touchable(History);

    SilenceOutput();
    G4ParticleDefinition* Ion = G4IonTable::GetIonTable()->GetIon(11, 24, 0.0);
    RestoreOutput();
    G4Ions* Ions = dynamic_cast<G4Ions*>(Ion);
    Passed = EvaluateTrue("GetIon()", "Na24", "The test ion exists", Ions != nullptr) && Passed;

    if (Ions != nullptr) {
      MCRun Run;
      Run.SetIsotopeStoreFileName(Dir + "/Added");
      Run.SetIncarnationID(1);
      Run.AddIsotope(Ions, &Touchable);
      Run.AddIsotope(Ions, &Touchable);
      SilenceOutput();
      Run.SaveIsotopeStore();
      RestoreOutput();
      Passed = Evaluate("AddIsotope()", "2 adds", "The added isotopes are counted", PRIV(Run, m_NAddedIsotopes), 2U) && Passed;
      Passed = EvaluateTrue("AddIsotope()", "volume name", "The volume name is stored without the suffix Log", FileContains(Dir + "/Added.inc1.dat", "VN Crystal")) && Passed;
      Passed = EvaluateTrue("AddIsotope()", "ID", "The isotope is stored as 1000*Z+A", FileContains(Dir + "/Added.inc1.dat", "RP 11024")) && Passed;
      Passed = EvaluateTrue("AddIsotope()", "count", "Two adds give a count of 2", FileContains(Dir + "/Added.inc1.dat", "2.00000e+00")) && Passed;

      // The store is saved automatically every 10000 isotopes:
      MCRun Auto;
      Auto.SetIsotopeStoreFileName(Dir + "/Auto");
      Auto.SetIncarnationID(1);
      SilenceOutput();
      for (unsigned int i = 0; i < 9999; ++i) Auto.AddIsotope(Ions, &Touchable);
      RestoreOutput();
      Passed = EvaluateFalse("AddIsotope()", "9999 adds", "No automatic save before 10000 isotopes", filesystem::exists((Dir + "/Auto.inc1.dat").Data())) && Passed;
      SilenceOutput();
      Auto.AddIsotope(Ions, &Touchable);
      RestoreOutput();
      Passed = EvaluateTrue("AddIsotope()", "10000 adds", "The store is saved after 10000 isotopes", filesystem::exists((Dir + "/Auto.inc1.dat").Data())) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestInitialize()
{
  bool Passed = true;

  MCRun& Run = m_Manager->GetCurrentRun();
  Passed = EvaluateTrue("GetCurrentRun()", "sources", "The run has its two sources", Run.GetNSources() == 2) && Passed;

  // A run with an orientation which starts after 0 and does not loop
  MCOrientation Orientation;
  Passed = EvaluateTrue("ParseOrientation()", "file", "The orientation file can be parsed", ParseOrientation(Orientation, "Initialize.ori", "OG 100 0 0 0 90\nOG 200 0 0 0 90\nOG 300 0 0 0 90\n", "NoLoop")) && Passed;
  Run.SetSkyOrientation(Orientation);

  SilenceOutput();
  bool Result = Run.Initialize();
  RestoreOutput();
  Passed = EvaluateTrue("Initialize()", "run", "The run can be initialized", Result) && Passed;

  // Sorted by flux, descending:
  Passed = Evaluate("Initialize()", "flux sorting", "The source with the highest flux is first", Run.GetSource(0)->GetName(), MString("B")) && Passed;
  Passed = Evaluate("Initialize()", "flux sorting", "The source with the lowest flux is last", Run.GetSource(1)->GetName(), MString("A")) && Passed;
  Passed = EvaluateTrue("Initialize()", "particle definitions", "The particle definitions are generated", Run.GetSource(0)->GetParticleDefinition() != nullptr && Run.GetSource(1)->GetParticleDefinition() != nullptr) && Passed;

  // The time starts at the start of the orientation
  Passed = EvaluateNear("Initialize()", "orientation", "The simulated time starts at the start time of the non-looping sky orientation", Run.GetSimulatedTime(), 100*s, 1e-3) && Passed;
  Passed = Evaluate("Initialize()", "next emissions", "All sources are scheduled", PRIV(Run, m_NextEmission).size(), (size_t) 2) && Passed;
  Passed = EvaluateTrue("Initialize()", "next emissions", "The first emissions are after the start time", Run.GetSource(0)->GetNextEmission() >= 100*s && Run.GetSource(1)->GetNextEmission() >= 100*s) && Passed;
  Passed = EvaluateTrue("Initialize()", "next emissions", "The scheduled first source is the one with the earliest emission", (*PRIV(Run, m_NextEmission).begin())->GetNextEmission() <= min(Run.GetSource(0)->GetNextEmission(), Run.GetSource(1)->GetNextEmission())) && Passed;

  // Initializing again must not schedule sources twice:
  SilenceOutput();
  Run.Initialize();
  RestoreOutput();
  Passed = Evaluate("Initialize()", "twice", "Initializing twice does not schedule a source twice", PRIV(Run, m_NextEmission).size(), (size_t) 2) && Passed;

  // The detector orientation starting later than the sky orientation:
  {
    MCRun Other;
    MCOrientation Detector;
    ParseOrientation(Detector, "InitializeDetector.ori", "OG 500 0 0 0 90\nOG 600 0 0 0 90\n", "NoLoop");
    Other.SetDetectorOrientation(Detector);
    Other.SetSkyOrientation(Orientation);
    SilenceOutput();
    Other.Initialize();
    RestoreOutput();
    Passed = EvaluateNear("Initialize()", "detector orientation", "The time starts at the latest start time of the orientations", Other.GetSimulatedTime(), 500*s, 1e-3) && Passed;
  }

  // A looping orientation does not advance the time
  {
    MCRun Other;
    MCOrientation Looping;
    ParseOrientation(Looping, "InitializeLoop.ori", "OG 100 0 0 0 90\nOG 200 0 0 0 90\n", "Loop");
    Other.SetSkyOrientation(Looping);
    SilenceOutput();
    Other.Initialize();
    RestoreOutput();
    Passed = EvaluateNear("Initialize()", "looping orientation", "A looping orientation does not advance the time", Other.GetSimulatedTime(), 0.0, 1e-3) && Passed;
  }

  // Remove the orientation again for the following tests
  Run.SetSkyOrientation(MCOrientation());
  PRIV(Run, m_NextEmission).clear();
  Run.SetSimulatedTime(0.0);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestBuildUpEventList()
{
  bool Passed = true;

  MCRun& Run = m_Manager->GetCurrentRun();
  Run.SetSimulatedTime(0.0);
  SilenceOutput();
  Run.Initialize();
  RestoreOutput();

  G4ParticleDefinition* Gamma = Run.GetSource(0)->GetParticleDefinition();
  const unsigned int NSourcesBefore = Run.GetNSources();

  // Beyond the duration nothing is stored if the run stops by time
  Run.SetDuration(10*s);
  bool Result = Run.AddToBuildUpEventList(1*MeV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 11*s, Gamma, "VolA");
  Passed = EvaluateTrue("AddToBuildUpEventList()", "beyond duration", "An event beyond the duration is dropped and the call succeeds", Result) && Passed;
  Passed = Evaluate("AddToBuildUpEventList()", "beyond duration", "No list source is created for a dropped event", Run.GetNSources(), NSourcesBefore) && Passed;

  // The simulated time is added:
  Run.SetSimulatedTime(2*s);
  SilenceOutput();
  Run.AddToBuildUpEventList(1*MeV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 7*s, Gamma, "VolA");
  RestoreOutput();
  Passed = Evaluate("AddToBuildUpEventList()", "first event", "The first event creates the list source", Run.GetNSources(), NSourcesBefore + 1) && Passed;
  MCSource* List = Run.GetSource(Run.GetNSources() - 1);
  Passed = Evaluate("AddToBuildUpEventList()", "first event", "The list source is called DelayedDecaysList", List->GetName(), MString("DelayedDecaysList")) && Passed;
  Passed = EvaluateTrue("AddToBuildUpEventList()", "first event", "The list source is a build-up event list", List->IsBuildUpEventList() && List->IsEventList()) && Passed;
  Passed = EvaluateNear("AddToBuildUpEventList()", "first event", "The emission time is the event time plus the simulated time", List->GetNextEmission(), 9*s, 1e-3) && Passed;
  Passed = EvaluateTrue("AddToBuildUpEventList()", "first event", "The list source knows the particle", List->GetEventListNextParticle() == Gamma) && Passed;
  Passed = Evaluate("AddToBuildUpEventList()", "first event", "The list source knows the volume", List->GetEventListNextVolume(), MString("VolA")) && Passed;
  Passed = EvaluateTrue("AddToBuildUpEventList()", "first event", "The list source is scheduled", PRIV(Run, m_NextEmission).count(List) == 1) && Passed;

  // A second, earlier event is added to the same source and becomes the next emission
  SilenceOutput();
  Result = Run.AddToBuildUpEventList(1*MeV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 3*s, Gamma, "VolB");
  RestoreOutput();
  Passed = EvaluateTrue("AddToBuildUpEventList()", "second event", "Adding to the existing list source succeeds", Result) && Passed;
  Passed = Evaluate("AddToBuildUpEventList()", "second event", "No second list source is created", Run.GetNSources(), NSourcesBefore + 1) && Passed;
  Passed = EvaluateNear("AddToBuildUpEventList()", "second event", "The earlier event is the next emission", List->GetNextEmission(), 5*s, 1e-3) && Passed;
  Passed = Evaluate("AddToBuildUpEventList()", "second event", "The earlier event is the next one of the list", List->GetEventListNextVolume(), MString("VolB")) && Passed;
  Passed = Evaluate("AddToBuildUpEventList()", "second event", "The list source is scheduled exactly once", PRIV(Run, m_NextEmission).count(List), (size_t) 1) && Passed;
  Passed = EvaluateTrue("AddToBuildUpEventList()", "second event", "The list source stays unique in the schedule", PRIV(Run, m_NextEmission).size() == 3) && Passed;

  // Return value: the documentation says "Return true if new source has been added"
  {
    MCRun Fresh;
    SilenceOutput();
    Result = Fresh.AddToBuildUpEventList(1*MeV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 3*s, Gamma, "VolB");
    RestoreOutput();
    // Known failure: AddToBuildUpEventList() returns false after creating the new source and true when it adds to an existing one - the reverse of its documentation
    Passed = ExpectedToFail(EvaluateTrue("AddToBuildUpEventList()", "new source", "The return value is true if a new source has been added (as documented)", Result)) && Passed;
  }

  // Clean up the schedule for the next test
  PRIV(Run, m_NextEmission).clear();
  Run.SetSimulatedTime(0.0);
  PRIV(Run, m_SourceList).pop_back();
  Run.SetTriggers(numeric_limits<long>::max() - 1);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRun::TestGeneratePrimaries()
{
  bool Passed = true;

  MCRun& Run = m_Manager->GetCurrentRun();

  // A clean state: re-initialize with events as stop condition
  Run.SetSimulatedTime(0.0);
  Run.SetEvents(2);
  SilenceOutput();
  Run.Initialize();
  RestoreOutput();

  // The earliest emission will be the next event time:
  double Earliest = numeric_limits<double>::max();
  MCSource* EarliestSource = nullptr;
  for (MCSource* S: Run.GetSourceList()) {
    if (S->GetNextEmission() < Earliest) {
      Earliest = S->GetNextEmission();
      EarliestSource = S;
    }
  }

  SilenceOutput();
  G4GeneralParticleSource* Gun = new G4GeneralParticleSource();
  RestoreOutput();
  G4Event* Event = new G4Event(0);

  // In a child first, since a crash would end the test
  const bool Clean = UTSDHitShared::RunsCleanlyInChild([&]() {
    G4Event* E = new G4Event(1);
    Run.GeneratePrimaries(E, Gun);
    return E->GetNumberOfPrimaryVertex() == 1;
  });
  Passed = EvaluateTrue("GeneratePrimaries()", "first event", "Generating the first event runs cleanly and gives one primary vertex", Clean) && Passed;
  if (Clean == false) {
    delete Event;
    return false;
  }

  SilenceOutput();
  Run.GeneratePrimaries(Event, Gun);
  RestoreOutput();
  Passed = Evaluate("GeneratePrimaries()", "first event", "The event has one primary vertex", Event->GetNumberOfPrimaryVertex(), 1) && Passed;
  Passed = Evaluate("GeneratePrimaries()", "first event", "One event is simulated", Run.GetNSimulatedEvents(), 1L) && Passed;
  Passed = Evaluate("GeneratePrimaries()", "first event", "One particle is generated", Run.GetNGeneratedParticles(), 1) && Passed;
  Passed = EvaluateNear("GeneratePrimaries()", "first event", "The simulated time is the emission time of the earliest source", Run.GetSimulatedTime(), Earliest, 1e-6) && Passed;
  Passed = EvaluateNear("GeneratePrimaries()", "first event", "The energy is the energy of that source", Event->GetPrimaryVertex()->GetPrimary()->GetKineticEnergy(), EarliestSource->GetName() == "A" ? 511*keV : 662*keV, 1e-3) && Passed;
  Passed = EvaluateTrue("GeneratePrimaries()", "next emission", "The used source is rescheduled after the event", EarliestSource->GetNextEmission() > Earliest) && Passed;
  Passed = Evaluate("GeneratePrimaries()", "next emission", "The source counts its generated particle", EarliestSource->GetNGeneratedParticles(), 1L) && Passed;

  // A second event reaches the stop condition (2 events):
  G4Event* Event2 = new G4Event(2);
  SilenceOutput();
  Run.GeneratePrimaries(Event2, Gun);
  RestoreOutput();
  Passed = Evaluate("GeneratePrimaries()", "second event", "The second event is simulated", Run.GetNSimulatedEvents(), 2L) && Passed;
  SilenceOutput();
  bool Stop = Run.CheckStopConditions();
  RestoreOutput();
  Passed = EvaluateTrue("CheckStopConditions()", "two events", "The run stops after the requested events", Stop) && Passed;

  // Generating beyond the stop condition gives no event
  G4Event* Event3 = new G4Event(3);
  SilenceOutput();
  Run.GeneratePrimaries(Event3, Gun);
  RestoreOutput();
  Passed = Evaluate("GeneratePrimaries()", "after stop", "No event is generated after the stop condition", Event3->GetNumberOfPrimaryVertex(), 0) && Passed;
  Passed = Evaluate("GeneratePrimaries()", "after stop", "The number of simulated events stays", Run.GetNSimulatedEvents(), 2L) && Passed;

  // Stop by time: the time is set to the duration
  MCRun& TimeRun = Run;
  TimeRun.SetDuration(1*ns);
  G4Event* Event4 = new G4Event(4);
  SilenceOutput();
  TimeRun.GeneratePrimaries(Event4, Gun);
  RestoreOutput();
  Passed = EvaluateNear("GeneratePrimaries()", "stop by time", "When the run stops by time, the simulated time is the duration", TimeRun.GetSimulatedTime(), 1*ns, 1e-9) && Passed;
  Passed = Evaluate("GeneratePrimaries()", "stop by time", "No event is generated when stopping by time", Event4->GetNumberOfPrimaryVertex(), 0) && Passed;

  delete Event;
  delete Event2;
  delete Event3;
  delete Event4;
  (void) Gun; // owned by Geant4 (registered in the G4 run manager singleton scope)

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTRun Test;
  return Test.Run() == true ? 0 : 1;
}
