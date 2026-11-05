/*
 * UTMain.cc
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


// The class is compiled with the visualization manager in cosima:
#ifndef G4VIS_USE
#define G4VIS_USE
#endif

// Standard lib - included first since the private members of MCMain are opened below:
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib:
#include "MFile.h"
#include "MGlobal.h"
#include "MSimEvent.h"
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"
#include "UTFixtureShared.h"

// Geant4 and ROOT:
#include "G4VisManager.hh"
#include "G4UIsession.hh"
#include "G4UImanager.hh"
#include "Randomize.hh"
#include "TError.h"
#include "TRandom.h"

// Cosima - the state of the parsed command line is only available through the private members:
#include "MCParameterFile.hh"
#include "MCRunManager.hh"
#define private public
#include "MCMain.hh"
#undef private
#include "UTSDHitShared.h"


//! The number of events which arrived at the relegator
static unsigned int g_RelegatedEvents = 0;
//! The relegator
static void CountingRelegator(MSimEvent* Event)
{
  if (Event != nullptr) ++g_RelegatedEvents;
}


//! Unit test class for MCMain
class UTMain : public MUnitTest
{
public:
  //! Default constructor
  UTMain() : MUnitTest("UTMain"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr) {}
  //! Default destructor
  virtual ~UTMain() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the state after construction
  bool TestConstruction();
  //! Test help and errors
  bool TestHelpAndErrors();
  //! Test the options
  bool TestOptions();
  //! Test the verbosity options which change global streams
  bool TestVerbosity();
  //! Test options which lack their argument
  bool TestMissingArguments();
  //! Test the parameter file handling
  bool TestParameterFile();
  //! Test initialization and execution
  bool TestExecution();

  //! Parse the arguments, return the result, silence all output
  unsigned int Parse(MCMain& Main, const vector<string>& Arguments);
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
  //! The parameter file name
  MString m_Source;
  //! The base name of the output files
  MString m_BaseName;
};


////////////////////////////////////////////////////////////////////////////////


void UTMain::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTMain::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTMain::Parse(MCMain& Main, const vector<string>& Arguments)
{
  vector<string> Storage = Arguments;
  vector<char*> Argv;
  Argv.push_back(const_cast<char*>("Cosima"));
  for (string& A: Storage) Argv.push_back(const_cast<char*>(A.c_str()));
  Argv.push_back(nullptr); // As for a real command line
  SilenceOutput();
  const unsigned int Result = Main.ParseCommandLine(int(Argv.size()) - 1, Argv.data());
  RestoreOutput();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMain::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;

  if (RequireFixture(*this, Passed, "src/cosima/unittests/GiganticSphere.geo.setup") == false) {
    Summarize();
    return Passed;
  }

  const Int_t OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  PrepareTemporaryDirectory("Main");
  // The parameter file names its output relative to the private temporary root, which is the working directory of the test:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  m_BaseName = GetTemporaryDirectoryName("Main") + "/UTMainOut";
  m_Source = GetTemporaryDirectoryName("Main") + "/Main.source";
  Passed = EvaluateTrue("WriteTextFile()", "source", "The parameter file can be written", WriteTextFile(m_Source, "Version 1\nGeometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\nDetectorTimeConstant 0.000005\nPhysicsListEM LivermorePol\nStoreSimulationInfo init\nRun Base\nBase.FileName Main/UTMainOut\nBase.Time 1\nBase.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.Spectrum Mono 511\nS.Flux 100\n")) && Passed;

  Passed = TestHelpAndErrors() && Passed;
  Passed = TestOptions() && Passed;
  Passed = TestVerbosity() && Passed;
  Passed = TestMissingArguments() && Passed;
  Passed = TestParameterFile() && Passed;
  Passed = TestExecution() && Passed;

  gErrorIgnoreLevel = OldErrorLevel;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMain::TestConstruction()
{
  bool Passed = true;

  MCMain* Main = new MCMain();
  Passed = Evaluate("MCMain()", "verbosity", "The default verbosity is 1", Main->m_Verbosity, 1) && Passed;
  Passed = EvaluateTrue("MCMain()", "zip", "Simulation files are zipped by default", Main->m_Zip) && Passed;
  Passed = EvaluateFalse("MCMain()", "interactive", "The default is not interactive", Main->m_Interactive) && Passed;
  Passed = EvaluateFalse("MCMain()", "macro", "There is no macro by default", Main->m_HasMacro) && Passed;
  Passed = EvaluateFalse("MCMain()", "timeout", "The time out is on by default", Main->m_NoTimeOut) && Passed;
  Passed = EvaluateFalse("MCMain()", "debug", "Debug is off by default", Main->m_UseDebug) && Passed;
  Passed = Evaluate("MCMain()", "parallel id", "The parallel ID is 0", Main->m_ParallelID, 0) && Passed;
  Passed = Evaluate("MCMain()", "incarnation id", "The incarnation ID is 0", Main->m_IncarnationID, 0) && Passed;
  Passed = EvaluateTrue("MCMain()", "seed", "The default seed is positive (time based)", Main->m_Seed > 0) && Passed;
  Passed = EvaluateTrue("MCMain()", "pointers", "The managers are not yet created", Main->m_RunManager == nullptr && Main->m_VisManager == nullptr && Main->m_Session == nullptr && Main->m_UI == nullptr) && Passed;

  // Without initialization nothing can be interrupted or relegated:
  Passed = EvaluateFalse("Interrupt()", "no run manager", "Interrupt reports false without a run manager", Main->Interrupt()) && Passed;
  SilenceOutput();
  Main->SetEventRelegator(CountingRelegator);
  RestoreOutput();
  Passed = EvaluateTrue("SetEventRelegator()", "no run manager", "Setting a relegator without run manager does not crash", true) && Passed;

  delete Main;
  Passed = EvaluateTrue("~MCMain()", "delete", "An uninitialized main can be deleted", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMain::TestHelpAndErrors()
{
  bool Passed = true;

  for (const string& Help: { string("-h"), string("--help"), string("?"), string("-?") }) {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", Help.c_str(), "The help option returns 1", Parse(Main, { Help }), 1U) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "-h with file", "Help is also found before a parameter file", Parse(Main, { "-h", m_Source.Data() }), 1U) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "unknown", "An unknown option returns 2", Parse(Main, { "-x", m_Source.Data() }), 2U) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "missing file", "A not existing parameter file returns 2", Parse(Main, { (GetTemporaryDirectoryName("Main") + "/DoesNotExist.source").Data() }), 2U) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "seed 0", "A seed of zero returns 2", Parse(Main, { "-s", "0", m_Source.Data() }), 2U) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "seed text", "A seed which is no number returns 2", Parse(Main, { "-s", "abc", m_Source.Data() }), 2U) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "seed negative", "A negative seed returns 2", Parse(Main, { "-s", "-5", m_Source.Data() }), 2U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMain::TestOptions()
{
  bool Passed = true;

  {
    MCMain Main;
    const unsigned int Result = Parse(Main, { "-s", "42", "-u", "-n", "-p", "3", "-f", "4", "-t", "tag", "-c", "Conv.geo", "-z", m_Source.Data() });
    Passed = Evaluate("ParseCommandLine()", "all", "A command line with all options is accepted", Result, 0U) && Passed;
    Passed = Evaluate("ParseCommandLine()", "seed", "The seed is stored", Main.m_Seed, 42L) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "-z after -u", "-z switches zipping on again (the options are applied in order)", Main.m_Zip) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "-n", "The time out can be disabled", Main.m_NoTimeOut) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-p", "The parallel ID is stored", Main.m_ParallelID, 3) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-f", "The incarnation ID is stored", Main.m_IncarnationID, 4) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-c", "The convert file name is stored", Main.m_ConvertFileName, MString("Conv.geo")) && Passed;
    Passed = Evaluate("ParseCommandLine()", "file", "The parameter file name is stored", Main.m_ParameterFileName, m_Source) && Passed;
    Passed = Evaluate("ParseCommandLine()", "ROOT seed", "The ROOT seed is set", (long) gRandom->GetSeed(), 42L) && Passed;
    Passed = Evaluate("ParseCommandLine()", "Geant4 seed", "The Geant4 seed is set", (long) CLHEP::HepRandom::getTheSeed(), 42L) && Passed;
    Passed = EvaluateFalse("ParseCommandLine()", "interactive", "Not interactive without -i or -m", Main.m_Interactive) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "-u", "-u is accepted", Parse(Main, { "-u", m_Source.Data() }), 0U) && Passed;
    Passed = EvaluateFalse("ParseCommandLine()", "-u", "-u disables zipping", Main.m_Zip) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "-i", "-i is accepted", Parse(Main, { "-i", m_Source.Data() }), 0U) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "-i", "-i starts the interactive mode", Main.m_Interactive) && Passed;
    Passed = EvaluateFalse("ParseCommandLine()", "-i", "-i has no macro", Main.m_HasMacro) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "-m", "-m is accepted", Parse(Main, { "-m", "run.mac", m_Source.Data() }), 0U) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "-m", "A macro implies interactive mode", Main.m_Interactive && Main.m_HasMacro) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-m", "The macro name is stored", Main.m_MacroFileName, MString("run.mac")) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "-r", "A restriction to an existing run is accepted", Parse(Main, { "-r", "0", m_Source.Data() }), 0U) && Passed;
  }
  {
    // Options after the parameter file name are found as well:
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "order", "Options are also accepted before the parameter file", Parse(Main, { "-s", "9", "-u", m_Source.Data() }), 0U) && Passed;
    Passed = Evaluate("ParseCommandLine()", "order", "The seed is stored", Main.m_Seed, 9L) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMain::TestVerbosity()
{
  bool Passed = true;

  const int OldVerbosity = g_Verbosity;
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "-v 3", "-v 3 is accepted", Parse(Main, { "-v", "3", m_Source.Data() }), 0U) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-v 3", "The verbosity is stored", Main.m_Verbosity, 3) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "-v 3", "Verbosity above 1 enables debugging", Main.m_UseDebug) && Passed;
  }
  {
    MCMain Main;
    Parse(Main, { "-v", "9", m_Source.Data() });
    Passed = Evaluate("ParseCommandLine()", "-v 9", "The verbosity is limited to 5", Main.m_Verbosity, 5) && Passed;
  }
  {
    MCMain Main;
    Passed = Evaluate("ParseCommandLine()", "-v 0", "-v 0 is accepted", Parse(Main, { "-v", "0", m_Source.Data() }), 0U) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-v 0", "The verbosity is stored", Main.m_Verbosity, 0) && Passed;
    Passed = EvaluateFalse("ParseCommandLine()", "-v 0", "Debugging stays off", Main.m_UseDebug) && Passed;
  }
  g_Verbosity = OldVerbosity;
  mout.Enable(true);
  mlog.Enable(true);
  mgui.Enable(true);
  mdebug.Enable(true);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMain::TestMissingArguments()
{
  bool Passed = true;

  // A missing argument must give an error, not a crash (run in a child since it may crash):
  for (const string& Option: { string("-s"), string("-m"), string("-c"), string("-v"), string("-p"), string("-r"), string("-f") }) {
    const UTSDHitShared::ChildOutcome Outcome = UTSDHitShared::RunInChild([&]() {
      MCMain Main;
      vector<char*> Argv = { const_cast<char*>("Cosima"), const_cast<char*>(Option.c_str()), nullptr };
      return Main.ParseCommandLine(2, Argv.data()) == 2;
    });
    if (Option == "-r" || Option == "-f") {
      // Known failure: ParseCommandLine() only checks -v and -p for a following argument, -r and -f read argv[++i] past the end and crash
      Passed = ExpectedToFail(EvaluateTrue("ParseCommandLine()", Option.c_str(), MString("The option ") + Option.c_str() + " without argument at the end of the command line returns 2 without crashing", Outcome == UTSDHitShared::ChildOutcome::c_ReturnedTrue)) && Passed;
    } else {
      Passed = EvaluateTrue("ParseCommandLine()", Option.c_str(), MString("The option ") + Option.c_str() + " without argument at the end of the command line returns 2 without crashing", Outcome == UTSDHitShared::ChildOutcome::c_ReturnedTrue) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMain::TestParameterFile()
{
  bool Passed = true;

  MCMain Main;
  const unsigned int Result = Parse(Main, { m_Source.Data() });
  Passed = Evaluate("ParseCommandLine()", "file only", "A parameter file alone is accepted", Result, 0U) && Passed;
  Passed = Evaluate("GetParameterFile()", "runs", "The parameter file has one run", Main.GetParameterFile().GetNRuns(), 1U) && Passed;
  Passed = Evaluate("GetParameterFile()", "EM", "The parameter file is the parsed one (Livermore polarized)", Main.GetParameterFile().GetPhysicsListEM(), 3) && Passed;

  // Without a parameter file the example is used:
  if (MFile::Exists("$(MEGALIB)/resource/examples/cosima/source/Run.source") == true) {
    MCMain Default;
    Passed = Evaluate("ParseCommandLine()", "no file", "Without a file the example parameter file is used", Parse(Default, {}), 0U) && Passed;
    Passed = Evaluate("ParseCommandLine()", "no file", "The example parameter file name is stored", Default.m_ParameterFileName, g_MEGAlibPath + "/resource/examples/cosima/source/Run.source") && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMain::TestExecution()
{
  bool Passed = true;

  MCMain* MainPointer = new MCMain();
  MCMain& Main = *MainPointer;
  Passed = Evaluate("ParseCommandLine()", "run", "The command line for the run is accepted", Parse(Main, { "-u", "-s", "5", m_Source.Data() }), 0U) && Passed;

  SilenceOutput();
  const bool Initialized = Main.Initialize();
  RestoreOutput();
  Passed = EvaluateTrue("Initialize()", "run", "The initialization works", Initialized) && Passed;
  if (Initialized == false) {
    SilenceOutput();
    delete MainPointer;
    RestoreOutput();
    return false;
  }

  Passed = EvaluateTrue("Initialize()", "managers", "The run manager, session, and UI manager exist", Main.m_RunManager != nullptr && Main.m_Session != nullptr && Main.m_UI != nullptr) && Passed;
  Passed = EvaluateTrue("Initialize()", "singleton", "The run manager of the main is the global Cosima run manager", MCRunManager::GetMCRunManager() == Main.m_RunManager) && Passed;
  Passed = EvaluateTrue("Initialize()", "actions", "The event, tracking, stepping actions and the generator exist", Main.m_RunManager->GetEventAction() != nullptr && Main.m_RunManager->GetTrackingAction() != nullptr && Main.m_RunManager->GetSteppingAction() != nullptr && Main.m_RunManager->GetUserPrimaryGeneratorAction() != nullptr) && Passed;
  Passed = EvaluateTrue("Initialize()", "zip", "The zip option of the command line reached the runs", Main.m_RunManager->GetRuns().size() == 1 && Main.m_RunManager->GetRuns()[0].GetZip() == false) && Passed;

  g_RelegatedEvents = 0;
  SilenceOutput();
  Main.SetEventRelegator(CountingRelegator);
  const bool Executed = Main.Execute();
  RestoreOutput();
  Passed = EvaluateTrue("Execute()", "run", "The execution works", Executed) && Passed;
  Passed = EvaluateTrue("Execute()", "events", "Events arrive at the relegator", g_RelegatedEvents > 0) && Passed;
  Passed = EvaluateTrue("Execute()", "file", "The simulation file exists", filesystem::exists((m_BaseName + ".inc1.id1.sim").Data())) && Passed;
  Passed = EvaluateTrue("Interrupt()", "after init", "Interrupt reports true with a run manager", Main.Interrupt()) && Passed;

  SilenceOutput();
  delete MainPointer;
  RestoreOutput();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTMain Test;
  return Test.Run() == true ? 0 : 1;
}
