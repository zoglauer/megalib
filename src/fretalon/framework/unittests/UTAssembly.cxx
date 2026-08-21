/*
 * UTAssembly.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MAssembly.h"
#include "MModule.h"
#include "MSupervisor.h"
#include "MUnitTest.h"

// The application implementation defines its own main(). Rename it only inside
// this test translation unit so production code and the generic test rule stay unchanged.
#define main MAssemblyApplicationMain
#include "../src/MAssembly.cxx"
#undef main

// ROOT libs:
#include "TGClient.h"

// Standard libs:
#include <sstream>
using namespace std;


//! Unit test class for MAssembly
class UTAssembly : public MUnitTest
{
public:
  //! Default constructor
  UTAssembly() : MUnitTest("UTAssembly") {}
  //! Default destructor
  virtual ~UTAssembly() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the public module-type constants
  bool TestModuleTypeConstants();
  //! Test construction and supervisor setup side effects
  bool TestConstructionAndInterrupt();
  //! Test command-line branches that return before launching the GUI
  bool TestSafeCommandLineParsing();
};


////////////////////////////////////////////////////////////////////////////////


bool UTAssembly::Run()
{
  bool Passed = true;

  Passed = TestModuleTypeConstants() && Passed;
  Passed = TestConstructionAndInterrupt() && Passed;
  Passed = TestSafeCommandLineParsing() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAssembly::TestModuleTypeConstants()
{
  bool Passed = true;

  Passed = Evaluate("c_EventLoader", "constant", "The event-loader module type keeps its documented value", MAssembly::c_EventLoader, 1) && Passed;
  Passed = Evaluate("c_EventLoaderMeasurement", "constant", "The measurement-loader module type keeps its documented value", MAssembly::c_EventLoaderMeasurement, 2) && Passed;
  Passed = Evaluate("c_EventLoaderSimulation", "constant", "The simulation-loader module type keeps its documented value", MAssembly::c_EventLoaderSimulation, 3) && Passed;
  Passed = Evaluate("c_DetectorEffectsEngine", "constant", "The detector-effects module type keeps its documented value", MAssembly::c_DetectorEffectsEngine, 4) && Passed;
  Passed = Evaluate("c_EventCoincidence", "constant", "The event-coincidence module type keeps its documented value", MAssembly::c_EventCoincidence, 5) && Passed;
  Passed = Evaluate("c_EnergyCalibration", "constant", "The energy-calibration module type keeps its documented value", MAssembly::c_EnergyCalibration, 6) && Passed;
  Passed = Evaluate("c_ChargeSharingCorrection", "constant", "The charge-sharing module type keeps its documented value", MAssembly::c_ChargeSharingCorrection, 7) && Passed;
  Passed = Evaluate("c_CrosstalkCorrection", "constant", "The crosstalk module type keeps its documented value", MAssembly::c_CrosstalkCorrection, 8) && Passed;
  Passed = Evaluate("c_DepthCorrection", "constant", "The depth-correction module type keeps its documented value", MAssembly::c_DepthCorrection, 9) && Passed;
  Passed = Evaluate("c_StripPairing", "constant", "The strip-pairing module type keeps its documented value", MAssembly::c_StripPairing, 10) && Passed;
  Passed = Evaluate("c_PositionDetermiation", "constant", "The position-determination module type keeps its documented value", MAssembly::c_PositionDetermiation, 11) && Passed;
  Passed = Evaluate("c_AspectReconstruction", "constant", "The aspect-reconstruction module type keeps its documented value", MAssembly::c_AspectReconstruction, 12) && Passed;
  Passed = Evaluate("c_EventReconstruction", "constant", "The event-reconstruction module type keeps its documented value", MAssembly::c_EventReconstruction, 13) && Passed;
  Passed = Evaluate("c_EventSaver", "constant", "The event-saver module type keeps its documented value", MAssembly::c_EventSaver, 14) && Passed;
  Passed = Evaluate("c_EventTransmitter", "constant", "The event-transmitter module type keeps its documented value", MAssembly::c_EventTransmitter, 15) && Passed;
  Passed = Evaluate("c_EventFilter", "constant", "The event-filter module type keeps its documented value", MAssembly::c_EventFilter, 16) && Passed;
  Passed = Evaluate("c_NoRestriction", "constant", "The no-restriction module type keeps its documented value", MAssembly::c_NoRestriction, 17) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAssembly::TestConstructionAndInterrupt()
{
  bool Passed = true;

  MSupervisor* Supervisor = MSupervisor::GetSupervisor();
  Supervisor->Clear();
  Supervisor->SetHardInterrupt(false);
  const unsigned int AvailableBefore = Supervisor->GetNAvailableModules();

  // Construction reports its missing default configuration file through mout.
  DisableDefaultStreams();
  MAssembly Assembly;
  EnableDefaultStreams();

  Passed = EvaluateTrue("MAssembly()", "supervisor singleton", "Construction uses the shared supervisor singleton", MSupervisor::GetSupervisor() == Supervisor) && Passed;
  Passed = Evaluate("MAssembly()", "available modules added", "Construction registers the three default framework modules", Supervisor->GetNAvailableModules(), AvailableBefore + 3) && Passed;
  Passed = EvaluateTrue("MAssembly()", "loader module", "Construction registers the default ROA loader module", Supervisor->GetAvailableModuleByXmlTag("XmlTagLoaderRoa") != nullptr) && Passed;
  Passed = EvaluateTrue("MAssembly()", "transmitter module", "Construction registers the default Realta transmitter module", Supervisor->GetAvailableModuleByXmlTag("XmlTagTransmitterRealta") != nullptr) && Passed;
  Passed = EvaluateTrue("MAssembly()", "saver module", "Construction registers the default event saver module", Supervisor->GetAvailableModuleByXmlTag("XmlTagEventSaver") != nullptr) && Passed;

  Assembly.SetInterrupt();
  Passed = EvaluateTrue("SetInterrupt()", "default true", "The assembly interrupt forwards to the supervisor hard interrupt", Supervisor->GetHardInterrupt()) && Passed;
  Passed = EvaluateTrue("SetInterrupt()", "default true soft", "A hard interrupt forwarded through the assembly also sets the supervisor soft interrupt", Supervisor->GetSoftInterrupt()) && Passed;
  Assembly.SetInterrupt(false);
  Passed = EvaluateFalse("SetInterrupt(false)", "clear hard", "Clearing the assembly interrupt clears the supervisor hard interrupt", Supervisor->GetHardInterrupt()) && Passed;
  Passed = EvaluateTrue("SetInterrupt(false)", "soft unchanged", "Clearing only the hard interrupt leaves the supervisor soft interrupt unchanged", Supervisor->GetSoftInterrupt()) && Passed;
  Supervisor->SetSoftInterrupt(false);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTAssembly::TestSafeCommandLineParsing()
{
  bool Passed = true;

  // Construction reports its missing default configuration file through mout.
  DisableDefaultStreams();
  MAssembly Assembly;
  EnableDefaultStreams();

  char Program[] = "UTAssembly";

  const char* HelpOptions[] = { "-h", "--help", "?", "-?" };
  for (const char* HelpOption: HelpOptions) {
    char HelpOptionBuffer[16];
    snprintf(HelpOptionBuffer, sizeof(HelpOptionBuffer), "%s", HelpOption);
    char* HelpArgv[] = { Program, HelpOptionBuffer };
    ostringstream HelpOutput;
    streambuf* OriginalCout = cout.rdbuf(HelpOutput.rdbuf());
    bool Result = Assembly.ParseCommandLine(2, HelpArgv);
    cout.rdbuf(OriginalCout);
    Passed = EvaluateFalse("ParseCommandLine()", MString(HelpOption), "Help-style options return false before launching the GUI", Result) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", MString(HelpOption) + " usage", "Help-style options print the usage text", MString(HelpOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  }

  char ConfigurationOption[] = "-c";
  char* MissingConfigurationArgv[] = { Program, ConfigurationOption };
  ostringstream MissingConfigurationOutput;
  streambuf* OriginalCout = cout.rdbuf(MissingConfigurationOutput.rdbuf());
  bool MissingConfigurationResult = Assembly.ParseCommandLine(2, MissingConfigurationArgv);
  cout.rdbuf(OriginalCout);
  Passed = EvaluateFalse("ParseCommandLine()", "-c missing value", "A missing configuration-file argument is rejected before launching the GUI", MissingConfigurationResult) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-c missing value output", "The missing configuration-file argument prints an error and usage text", MString(MissingConfigurationOutput.str()).Contains("needs a second argument") && MString(MissingConfigurationOutput.str()).Contains("Usage: Nuclearizer")) && Passed;

  char MultithreadingOption[] = "--multithreading";
  char* MissingMultithreadingArgv[] = { Program, MultithreadingOption };
  ostringstream MissingMultithreadingOutput;
  streambuf* MissingMultithreadingOriginal = cout.rdbuf(MissingMultithreadingOutput.rdbuf());
  bool MissingMultithreadingResult = Assembly.ParseCommandLine(2, MissingMultithreadingArgv);
  cout.rdbuf(MissingMultithreadingOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "--multithreading missing value", "A missing multithreading argument is rejected before launching the GUI", MissingMultithreadingResult) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "--multithreading missing value output", "The missing multithreading argument prints an error and usage text", MString(MissingMultithreadingOutput.str()).Contains("needs a second argument") && MString(MissingMultithreadingOutput.str()).Contains("Usage: Nuclearizer")) && Passed;

  // A value-carrying option has to be read from its argument, not from the option itself. With a
  // valid argument the parser runs on to the user interface, so this needs a headless ROOT -- on a
  // machine with a display LaunchUI() would open a real window.
  if (gClient == nullptr || gClient->GetRoot() == nullptr) {
    char MultithreadingValueOption[] = "--multithreading";
    char EnabledValue[] = "1";
    char* EnabledArgv[] = { Program, MultithreadingValueOption, EnabledValue };
    ostringstream EnabledOutput;
    OriginalCout = cout.rdbuf(EnabledOutput.rdbuf());
    Assembly.ParseCommandLine(3, EnabledArgv);
    cout.rdbuf(OriginalCout);
    Passed = EvaluateTrue("ParseCommandLine()", "--multithreading 1", "The multithreading option reads its argument and enables multithreading", MString(EnabledOutput.str()).Contains("Using multithreading: yes")) && Passed;

    char DisabledValue[] = "0";
    char* DisabledArgv[] = { Program, MultithreadingValueOption, DisabledValue };
    ostringstream DisabledOutput;
    OriginalCout = cout.rdbuf(DisabledOutput.rdbuf());
    Assembly.ParseCommandLine(3, DisabledArgv);
    cout.rdbuf(OriginalCout);
    Passed = EvaluateTrue("ParseCommandLine()", "--multithreading 0", "The multithreading option reads its argument and disables multithreading", MString(DisabledOutput.str()).Contains("Using multithreading: no")) && Passed;
  } else {
    mout<<"UTAssembly: a ROOT graphics client is available, skipping the multithreading argument check"<<endl;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTAssembly Test;
  return Test.Run() == true ? 0 : 1;
}


// UTAssembly.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
