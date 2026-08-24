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
    bool ReadyForUILoop = false;
    bool HadError = Assembly.ParseCommandLine(2, HelpArgv, ReadyForUILoop);
    cout.rdbuf(OriginalCout);
    Passed = EvaluateFalse("ParseCommandLine()", MString(HelpOption), "Help-style options do not ask main() to enter the UI loop", ReadyForUILoop) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", MString(HelpOption) + " usage", "Help-style options print the usage text", MString(HelpOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
    Passed = EvaluateFalse("ParseCommandLine()", MString(HelpOption) + " error", "A help request is a clean, requested exit, not an error", HadError) && Passed;
  }

  char ConfigurationOption[] = "-c";
  char* MissingConfigurationArgv[] = { Program, ConfigurationOption };
  ostringstream MissingConfigurationOutput;
  streambuf* OriginalCout = cout.rdbuf(MissingConfigurationOutput.rdbuf());
  bool MissingConfigurationReadyForUILoop = false;
  bool MissingConfigurationHadError = Assembly.ParseCommandLine(2, MissingConfigurationArgv, MissingConfigurationReadyForUILoop);
  cout.rdbuf(OriginalCout);
  Passed = EvaluateFalse("ParseCommandLine()", "-c missing value", "A missing configuration-file argument does not ask main() to enter the UI loop", MissingConfigurationReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-c missing value output", "The missing configuration-file argument prints an error and usage text", MString(MissingConfigurationOutput.str()).Contains("needs a second argument") && MString(MissingConfigurationOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-c missing value error", "A missing required argument is an actual error, not a clean exit", MissingConfigurationHadError) && Passed;

  char MultithreadingOption[] = "--multithreading";
  char* MissingMultithreadingArgv[] = { Program, MultithreadingOption };
  ostringstream MissingMultithreadingOutput;
  streambuf* MissingMultithreadingOriginal = cout.rdbuf(MissingMultithreadingOutput.rdbuf());
  bool MissingMultithreadingReadyForUILoop = false;
  bool MissingMultithreadingHadError = Assembly.ParseCommandLine(2, MissingMultithreadingArgv, MissingMultithreadingReadyForUILoop);
  cout.rdbuf(MissingMultithreadingOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "--multithreading missing value", "A missing multithreading argument does not ask main() to enter the UI loop", MissingMultithreadingReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "--multithreading missing value output", "The missing multithreading argument prints an error and usage text", MString(MissingMultithreadingOutput.str()).Contains("needs a second argument") && MString(MissingMultithreadingOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "--multithreading missing value option name", "The error names the long option that was actually used, not just its second character (which for a long option is itself only a dash)", MString(MissingMultithreadingOutput.str()).Contains("Option --multithreading needs")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "--multithreading missing value error", "A missing required argument is an actual error, not a clean exit", MissingMultithreadingHadError) && Passed;

  // -v/--verbosity takes a value the same way -c and -m do; atoi(argv[++i]) would read argv[argc],
  // which the C++ standard guarantees is a null pointer, if the validation below did not catch it
  char VerbosityOption[] = "-v";
  char* MissingVerbosityArgv[] = { Program, VerbosityOption };
  ostringstream MissingVerbosityOutput;
  streambuf* MissingVerbosityOriginal = cout.rdbuf(MissingVerbosityOutput.rdbuf());
  bool MissingVerbosityReadyForUILoop = false;
  bool MissingVerbosityHadError = Assembly.ParseCommandLine(2, MissingVerbosityArgv, MissingVerbosityReadyForUILoop);
  cout.rdbuf(MissingVerbosityOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "-v missing value", "A missing verbosity argument is rejected before it can be dereferenced", MissingVerbosityReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v missing value output", "The missing verbosity argument prints an error and usage text", MString(MissingVerbosityOutput.str()).Contains("needs a second argument") && MString(MissingVerbosityOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v missing value error", "A missing required argument is an actual error, not a clean exit", MissingVerbosityHadError) && Passed;

  // An unrecognized option (a typo, for example) must be rejected rather than silently ignored
  char UnknownOption[] = "--verbositty";
  char* UnknownOptionArgv[] = { Program, UnknownOption };
  ostringstream UnknownOptionOutput;
  streambuf* UnknownOptionOriginal = cout.rdbuf(UnknownOptionOutput.rdbuf());
  bool UnknownOptionReadyForUILoop = false;
  bool UnknownOptionHadError = Assembly.ParseCommandLine(2, UnknownOptionArgv, UnknownOptionReadyForUILoop);
  cout.rdbuf(UnknownOptionOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "unknown option", "An unrecognized option does not ask main() to enter the UI loop", UnknownOptionReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "unknown option output", "The unrecognized option prints an error naming it and usage text", MString(UnknownOptionOutput.str()).Contains("Unknown option") && MString(UnknownOptionOutput.str()).Contains(UnknownOption) && MString(UnknownOptionOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "unknown option error", "An unrecognized option is an actual error, not a clean exit", UnknownOptionHadError) && Passed;

  // A non-numeric value must be rejected rather than silently read as 0 by atoi()
  char VerbosityOptionAgain[] = "-v";
  char NonNumericValue[] = "abc";
  char* NonNumericVerbosityArgv[] = { Program, VerbosityOptionAgain, NonNumericValue };
  ostringstream NonNumericOutput;
  streambuf* NonNumericOriginal = cout.rdbuf(NonNumericOutput.rdbuf());
  bool NonNumericReadyForUILoop = false;
  bool NonNumericHadError = Assembly.ParseCommandLine(3, NonNumericVerbosityArgv, NonNumericReadyForUILoop);
  cout.rdbuf(NonNumericOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "-v non-numeric value", "A non-numeric verbosity argument is rejected rather than silently read as zero", NonNumericReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v non-numeric value output", "The non-numeric verbosity argument prints an error and usage text", MString(NonNumericOutput.str()).Contains("needs a plain integer second argument") && MString(NonNumericOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v non-numeric value error", "A non-numeric required argument is an actual error, not a clean exit", NonNumericHadError) && Passed;

  // A floating-point value must be rejected too: it passes a lenient "is this a number" check, but
  // atoi() below would silently truncate it -- "0.5" becoming verbosity 0 without any warning
  char FloatingPointValue[] = "0.5";
  char* FloatingPointVerbosityArgv[] = { Program, VerbosityOptionAgain, FloatingPointValue };
  ostringstream FloatingPointOutput;
  streambuf* FloatingPointOriginal = cout.rdbuf(FloatingPointOutput.rdbuf());
  bool FloatingPointReadyForUILoop = false;
  bool FloatingPointHadError = Assembly.ParseCommandLine(3, FloatingPointVerbosityArgv, FloatingPointReadyForUILoop);
  cout.rdbuf(FloatingPointOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "-v floating-point value", "A floating-point verbosity argument is rejected rather than silently truncated by atoi()", FloatingPointReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v floating-point value error", "A floating-point required argument is an actual error, not a clean exit", FloatingPointHadError) && Passed;

  // A value outside the range of int must be rejected too: atoi()'s behavior on overflow is
  // undefined, so this has to be caught before it ever reaches atoi()
  char OutOfRangeValue[] = "99999999999999999999";
  char* OutOfRangeVerbosityArgv[] = { Program, VerbosityOptionAgain, OutOfRangeValue };
  ostringstream OutOfRangeOutput;
  streambuf* OutOfRangeOriginal = cout.rdbuf(OutOfRangeOutput.rdbuf());
  bool OutOfRangeReadyForUILoop = false;
  bool OutOfRangeHadError = Assembly.ParseCommandLine(3, OutOfRangeVerbosityArgv, OutOfRangeReadyForUILoop);
  cout.rdbuf(OutOfRangeOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "-v out-of-range value", "A verbosity argument too large for int is rejected rather than handed to atoi() with undefined results", OutOfRangeReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v out-of-range value error", "An out-of-range required argument is an actual error, not a clean exit", OutOfRangeHadError) && Passed;

  // Already caught by the "does the next token look like another option" check, since every
  // negative number starts with '-' -- confirms end-to-end rejection, not a dedicated branch
  char NegativeValue[] = "-1";
  char* NegativeVerbosityArgv[] = { Program, VerbosityOptionAgain, NegativeValue };
  ostringstream NegativeOutput;
  streambuf* NegativeOriginal = cout.rdbuf(NegativeOutput.rdbuf());
  bool NegativeReadyForUILoop = false;
  bool NegativeHadError = Assembly.ParseCommandLine(3, NegativeVerbosityArgv, NegativeReadyForUILoop);
  cout.rdbuf(NegativeOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "-v negative value", "A negative verbosity argument is rejected", NegativeReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v negative value output", "The negative verbosity argument is rejected as a missing argument, since it is indistinguishable from another option", MString(NegativeOutput.str()).Contains("needs a second argument") && MString(NegativeOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v negative value error", "A negative required argument is an actual error, not a clean exit", NegativeHadError) && Passed;

  // Not caught by that heuristic: the first character is a space, not '-', and MString::Is<int>()
  // skips leading whitespace, so this parses as a valid negative integer -- caught instead by the
  // range check.
  char WhitespaceNegativeValue[] = " -1";
  char* WhitespaceNegativeVerbosityArgv[] = { Program, VerbosityOptionAgain, WhitespaceNegativeValue };
  ostringstream WhitespaceNegativeOutput;
  streambuf* WhitespaceNegativeOriginal = cout.rdbuf(WhitespaceNegativeOutput.rdbuf());
  bool WhitespaceNegativeReadyForUILoop = false;
  bool WhitespaceNegativeHadError = Assembly.ParseCommandLine(3, WhitespaceNegativeVerbosityArgv, WhitespaceNegativeReadyForUILoop);
  cout.rdbuf(WhitespaceNegativeOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "-v whitespace-prefixed negative value", "A negative verbosity argument disguised by leading whitespace is still rejected", WhitespaceNegativeReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v whitespace-prefixed negative value output", "The whitespace-prefixed negative verbosity argument is rejected by the range check, not treated as a missing argument", MString(WhitespaceNegativeOutput.str()).Contains("needs a verbosity value between") && MString(WhitespaceNegativeOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v whitespace-prefixed negative value error", "A whitespace-prefixed negative required argument is an actual error, not a clean exit", WhitespaceNegativeHadError) && Passed;

  // The valid range's upper end is g_Verbosity's actual highest level (Chatty/Extreme), not the
  // four the usage text used to stop at. A valid value lets the parser reach LaunchUI(), so this
  // needs a headless ROOT, same as the multithreading checks below.
  if (gClient == nullptr || gClient->GetRoot() == nullptr) {
    char ExtremeValue[] = "5";
    char* ExtremeVerbosityArgv[] = { Program, VerbosityOptionAgain, ExtremeValue };
    ostringstream ExtremeOutput;
    streambuf* ExtremeOriginal = cout.rdbuf(ExtremeOutput.rdbuf());
    bool ExtremeReadyForUILoop = false;
    bool ExtremeHadError = Assembly.ParseCommandLine(3, ExtremeVerbosityArgv, ExtremeReadyForUILoop);
    cout.rdbuf(ExtremeOriginal);
    Passed = EvaluateTrue("ParseCommandLine()", "-v Extreme value output", "The highest defined verbosity level is read and reported like any other valid value", MString(ExtremeOutput.str()).Contains("Verbosity 5")) && Passed;
    Passed = EvaluateFalse("ParseCommandLine()", "-v Extreme value", "The value itself is well-formed and accepted, but the run still fails because no GUI could be launched", ExtremeReadyForUILoop) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "-v Extreme value error", "The highest defined verbosity level is not itself an error, but the resulting GUI-launch failure is", ExtremeHadError) && Passed;
  } else {
    mout<<"UTAssembly: a ROOT graphics client is available, skipping the -v 5 argument check"<<endl;
  }

  char BeyondExtremeValue[] = "6";
  char* BeyondExtremeVerbosityArgv[] = { Program, VerbosityOptionAgain, BeyondExtremeValue };
  ostringstream BeyondExtremeOutput;
  streambuf* BeyondExtremeOriginal = cout.rdbuf(BeyondExtremeOutput.rdbuf());
  bool BeyondExtremeReadyForUILoop = false;
  bool BeyondExtremeHadError = Assembly.ParseCommandLine(3, BeyondExtremeVerbosityArgv, BeyondExtremeReadyForUILoop);
  cout.rdbuf(BeyondExtremeOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "-v beyond Extreme value", "A verbosity level beyond the highest defined one is rejected", BeyondExtremeReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v beyond Extreme value output", "The out-of-range verbosity level is rejected by the range check", MString(BeyondExtremeOutput.str()).Contains("needs a verbosity value between") && MString(BeyondExtremeOutput.str()).Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v beyond Extreme value error", "A verbosity level beyond the highest defined one is an actual error, not a clean exit", BeyondExtremeHadError) && Passed;

  // The three value-validation errors name the option through the same code path, once affected by
  // the same bug (naming a long option by its second character, itself just a dash). Exercised here
  // via the long form to confirm none regressed.
  char LongVerbosityOption[] = "--verbosity";
  char* LongMissingVerbosityArgv[] = { Program, LongVerbosityOption };
  ostringstream LongMissingVerbosityOutput;
  streambuf* LongMissingVerbosityOriginal = cout.rdbuf(LongMissingVerbosityOutput.rdbuf());
  bool LongMissingVerbosityReadyForUILoop = false;
  Assembly.ParseCommandLine(2, LongMissingVerbosityArgv, LongMissingVerbosityReadyForUILoop);
  cout.rdbuf(LongMissingVerbosityOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "--verbosity missing value", "A missing verbosity argument is rejected the same way through the long option spelling", LongMissingVerbosityReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "--verbosity missing value option name", "The error names the long option that was actually used", MString(LongMissingVerbosityOutput.str()).Contains("Option --verbosity needs a second argument")) && Passed;

  char LongNonNumericValue[] = "abc";
  char* LongNonNumericVerbosityArgv[] = { Program, LongVerbosityOption, LongNonNumericValue };
  ostringstream LongNonNumericOutput;
  streambuf* LongNonNumericOriginal = cout.rdbuf(LongNonNumericOutput.rdbuf());
  bool LongNonNumericReadyForUILoop = false;
  Assembly.ParseCommandLine(3, LongNonNumericVerbosityArgv, LongNonNumericReadyForUILoop);
  cout.rdbuf(LongNonNumericOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "--verbosity non-numeric value", "A non-numeric verbosity argument is rejected the same way through the long option spelling", LongNonNumericReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "--verbosity non-numeric value option name", "The error names the long option that was actually used", MString(LongNonNumericOutput.str()).Contains("Option --verbosity needs a plain integer second argument")) && Passed;

  char LongOutOfRangeValue[] = "6";
  char* LongOutOfRangeVerbosityArgv[] = { Program, LongVerbosityOption, LongOutOfRangeValue };
  ostringstream LongOutOfRangeOutput;
  streambuf* LongOutOfRangeOriginal = cout.rdbuf(LongOutOfRangeOutput.rdbuf());
  bool LongOutOfRangeReadyForUILoop = false;
  Assembly.ParseCommandLine(3, LongOutOfRangeVerbosityArgv, LongOutOfRangeReadyForUILoop);
  cout.rdbuf(LongOutOfRangeOriginal);
  Passed = EvaluateFalse("ParseCommandLine()", "--verbosity out-of-range value", "An out-of-range verbosity argument is rejected the same way through the long option spelling", LongOutOfRangeReadyForUILoop) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "--verbosity out-of-range value option name", "The error names the long option that was actually used", MString(LongOutOfRangeOutput.str()).Contains("Option --verbosity needs a verbosity value between")) && Passed;

  // A value-carrying option has to be read from its argument, not from the option itself. With a
  // valid argument the parser runs on to the user interface, so this needs a headless ROOT -- on a
  // machine with a display LaunchUI() would open a real window.
  if (gClient == nullptr || gClient->GetRoot() == nullptr) {
    char MultithreadingValueOption[] = "--multithreading";
    char EnabledValue[] = "1";
    char* EnabledArgv[] = { Program, MultithreadingValueOption, EnabledValue };
    ostringstream EnabledOutput;
    OriginalCout = cout.rdbuf(EnabledOutput.rdbuf());
    bool EnabledReadyForUILoop = false;
    bool EnabledHadError = Assembly.ParseCommandLine(3, EnabledArgv, EnabledReadyForUILoop);
    cout.rdbuf(OriginalCout);
    Passed = EvaluateTrue("ParseCommandLine()", "--multithreading 1", "The multithreading option reads its argument and enables multithreading", MString(EnabledOutput.str()).Contains("Using multithreading: yes")) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "--multithreading 1 error", "The option value itself is well-formed, but the run still fails because no GUI could be launched, which is a genuine error", EnabledHadError) && Passed;

    char DisabledValue[] = "0";
    char* DisabledArgv[] = { Program, MultithreadingValueOption, DisabledValue };
    ostringstream DisabledOutput;
    OriginalCout = cout.rdbuf(DisabledOutput.rdbuf());
    bool DisabledReadyForUILoop = false;
    Assembly.ParseCommandLine(3, DisabledArgv, DisabledReadyForUILoop);
    cout.rdbuf(OriginalCout);
    Passed = EvaluateTrue("ParseCommandLine()", "--multithreading 0", "The multithreading option reads its argument and disables multithreading", MString(DisabledOutput.str()).Contains("Using multithreading: no")) && Passed;
  } else {
    mout<<"UTAssembly: a ROOT graphics client is available, skipping the multithreading argument check"<<endl;
  }

  // -C/--change-configuration changes a field in the already-loaded configuration, and can be
  // given more than once; each occurrence is applied. Reaching this pass needs a headless ROOT,
  // same as the multithreading checks above -- with a display, ParseCommandLine() would go on to
  // open a real LaunchUI() window.
  if (gClient == nullptr || gClient->GetRoot() == nullptr) {
    char ChangeConfigOption[] = "-C";
    char GeometryChange[] = "GeometryFileName=/tmp/UTAssembly_changed.geo.setup";
    char* ChangeConfigArgv[] = { Program, ChangeConfigOption, GeometryChange };
    ostringstream ChangeConfigOutput;
    OriginalCout = cout.rdbuf(ChangeConfigOutput.rdbuf());
    bool ChangeConfigReadyForUILoop = false;
    Assembly.ParseCommandLine(3, ChangeConfigArgv, ChangeConfigReadyForUILoop);
    cout.rdbuf(OriginalCout);
    Passed = EvaluateTrue("ParseCommandLine()", "-C changes a field", "The field named by -C is reported as changed", MString(ChangeConfigOutput.str()).Contains("Changing this configuration value")) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "-C changes GeometryFileName", "The named field is actually updated on the shared supervisor", MSupervisor::GetSupervisor()->GetGeometryFileName() == "/tmp/UTAssembly_changed.geo.setup") && Passed;

    // -C can be used multiple times; both occurrences target the same field, so the final value
    // proves the second one was applied on top of the first rather than the first one winning
    char SecondChangeConfigOption[] = "-C";
    char FirstOfTwoChanges[] = "GeometryFileName=/tmp/UTAssembly_first.geo.setup";
    char SecondOfTwoChanges[] = "GeometryFileName=/tmp/UTAssembly_second.geo.setup";
    char* TwoChangesArgv[] = { Program, ChangeConfigOption, FirstOfTwoChanges, SecondChangeConfigOption, SecondOfTwoChanges };
    bool TwoChangesReadyForUILoop = false;
    DisableDefaultStreams();
    Assembly.ParseCommandLine(5, TwoChangesArgv, TwoChangesReadyForUILoop);
    EnableDefaultStreams();
    Passed = EvaluateTrue("ParseCommandLine()", "-C given twice", "The second -C occurrence is applied after the first", MSupervisor::GetSupervisor()->GetGeometryFileName() == "/tmp/UTAssembly_second.geo.setup") && Passed;

    // An unresolvable field name is reported but does not abort command-line parsing, matching
    // how mimrec's -C already behaves
    char UnknownFieldChange[] = "NoSuchField=5";
    char* UnknownFieldArgv[] = { Program, ChangeConfigOption, UnknownFieldChange };
    ostringstream UnknownFieldOutput;
    OriginalCout = cout.rdbuf(UnknownFieldOutput.rdbuf());
    bool UnknownFieldReadyForUILoop = false;
    Assembly.ParseCommandLine(3, UnknownFieldArgv, UnknownFieldReadyForUILoop);
    cout.rdbuf(OriginalCout);
    Passed = EvaluateTrue("ParseCommandLine()", "-C unresolvable field", "An unresolvable field is reported as an error", MString(UnknownFieldOutput.str()).Contains("Unable to change this configuration value")) && Passed;
  } else {
    mout<<"UTAssembly: a ROOT graphics client is available, skipping the change-configuration check"<<endl;
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
