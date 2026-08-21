/*
 * UTModuleSaver.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MAssembly.h"
#include "MModuleLoaderRoa.h"
#include "MModuleSaver.h"
#include "MReadOut.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutElementStrip.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Standard libs:
using namespace std;


//! Unit test class for MModuleSaver
class UTModuleSaver : public MUnitTest
{
public:
  //! Default constructor
  UTModuleSaver() : MUnitTest("UTModuleSaver") {}
  //! Default destructor
  virtual ~UTModuleSaver() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Create a representative read-out assembly
  MReadOutAssembly* CreateRepresentativeEvent(unsigned long ID = 42);
  //! Test construction-time module metadata and file-name interface behavior
  bool TestMetadataAndFileName();
  //! Test XML configuration helpers
  bool TestXmlConfiguration();
  //! Test initialization failure cases and ROA/EVTA writing
  bool TestSaving();
  //! Test that a written roa file can be read back and rewritten unchanged
  bool TestRoaRoundTrip();
  //! Test inherited queue processing through DoSingleAnalysis
  bool TestInheritedFlow();
};


////////////////////////////////////////////////////////////////////////////////


bool UTModuleSaver::Run()
{
  bool Passed = true;

  Passed = TestMetadataAndFileName() && Passed;
  Passed = TestXmlConfiguration() && Passed;
  Passed = TestSaving() && Passed;
  Passed = TestInheritedFlow() && Passed;
  Passed = TestRoaRoundTrip() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* UTModuleSaver::CreateRepresentativeEvent(unsigned long ID)
{
  MReadOutAssembly* Event = new MReadOutAssembly();
  Event->SetFilteredOut(false);
  Event->SetID(ID);

  MReadOutElementStrip Element(7, 11);
  MReadOutDataADCValue Data;
  Data.SetADCValue(1234);
  MReadOut ReadOut(Element, Data);
  Event->AddReadOut(ReadOut);

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleSaver::TestMetadataAndFileName()
{
  bool Passed = true;

  MModuleSaver Module;
  Passed = Evaluate("GetName()", "default", "The event saver exposes its representative module name", Module.GetName(), MString("Save events (roa or evta format)")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The event saver exposes its representative XML tag", Module.GetXmlTag(), MString("XmlTagEventSaver")) && Passed;

  Passed = EvaluateSize("GetNPreceedingModuleTypes()", "saver", "The event saver registers one predecessor requirement", Module.GetNPreceedingModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "event loader", "The event saver requires event-loader progress first", Module.GetPreceedingModuleType(0), static_cast<uint64_t>(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("GetPreceedingModuleHardRequirement()", "event loader", "The event-loader predecessor is a hard requirement", Module.GetPreceedingModuleHardRequirement(0)) && Passed;

  Passed = EvaluateSize("GetNModuleTypes()", "saver", "The event saver registers one provided module type", Module.GetNModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetModuleType()", "event saver", "The event saver provides the event-saver module type", Module.GetModuleType(0), static_cast<uint64_t>(MAssembly::c_EventSaver)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "event saver", "The event saver reports that it provides event saving", Module.ProvidesModuleType(MAssembly::c_EventSaver)) && Passed;
  Passed = EvaluateFalse("ProvidesModuleType()", "missing type", "The event saver rejects a representative unregistered module type", Module.ProvidesModuleType(0x80000000ULL)) && Passed;

  Passed = EvaluateSize("GetNSucceedingModuleTypes()", "saver", "The event saver registers one succeeding module type", Module.GetNSucceedingModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetSucceedingModuleType()", "no restriction", "The event saver allows unrestricted succeeding modules", Module.GetSucceedingModuleType(0), static_cast<uint64_t>(MAssembly::c_NoRestriction)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "saver", "The event saver exposes an options GUI", Module.HasOptionsGUI()) && Passed;
  Passed = EvaluateFalse("IsStartModule()", "saver", "The event saver is not a start module", Module.IsStartModule()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "saver", "The event saver allows multi-threading", Module.AllowsMultiThreading()) && Passed;
  Passed = EvaluateFalse("AllowsMultipleInstances()", "saver", "The event saver does not allow multiple instances", Module.AllowsMultipleInstances()) && Passed;

  Module.SetFileName("representative.roa");
  Passed = Evaluate("SetFileName()/GetFileName()", "representative.roa", "The file-name interface stores the configured output file name", Module.GetFileName(), MString("representative.roa")) && Passed;

  MModuleSaver* Clone = Module.Clone();
  Passed = EvaluateTrue("Clone()", "saver", "Clone returns a distinct event saver instance", Clone != nullptr && Clone != &Module) && Passed;
  Passed = Evaluate("Clone()", "saver name", "The cloned event saver has the same representative module name", Clone->GetName(), MString("Save events (roa or evta format)")) && Passed;
  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleSaver::TestXmlConfiguration()
{
  bool Passed = true;

  MModuleSaver Module;
  MXmlNode Node(nullptr, "XmlTagEventSaver");
  new MXmlNode(&Node, "FileName", MString("configured.roa"));
  new MXmlNode(&Node, "Mode", static_cast<unsigned int>(MModuleSaver::c_RoaFile));
  Passed = EvaluateTrue("ReadXmlConfiguration()", "FileName and Mode nodes", "The event saver reads file name and mode from XML", Module.ReadXmlConfiguration(&Node)) && Passed;
  Passed = Evaluate("GetFileName()", "after XML read", "The XML reader stores the configured file name", Module.GetFileName(), MString("configured.roa")) && Passed;

  MXmlNode EmptyNode(nullptr, "XmlTagEventSaver");
  Passed = EvaluateTrue("ReadXmlConfiguration()", "missing nodes", "The event saver accepts XML without optional nodes", Module.ReadXmlConfiguration(&EmptyNode)) && Passed;
  Passed = Evaluate("GetFileName()", "after missing nodes", "Missing XML nodes leave the existing file name unchanged", Module.GetFileName(), MString("configured.roa")) && Passed;

  Module.SetFileName("written.evta");
  MXmlNode* WrittenNode = Module.CreateXmlConfiguration();
  Passed = Evaluate("CreateXmlConfiguration()", "node name", "The event saver XML writer creates a node with the saver XML tag", WrittenNode->GetName(), MString("XmlTagEventSaver")) && Passed;
  Passed = EvaluateSize("CreateXmlConfiguration()", "child nodes", "The event saver XML writer creates FileName and Mode children", WrittenNode->GetNNodes(), static_cast<size_t>(2)) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "FileName child name", "The first XML child stores the FileName tag", WrittenNode->GetNode(0)->GetName(), MString("FileName")) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "FileName child value", "The first XML child stores the configured file name", WrittenNode->GetNode(0)->GetValue(), MString("written.evta")) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "Mode child name", "The second XML child stores the Mode tag", WrittenNode->GetNode(1)->GetName(), MString("Mode")) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "Mode child value", "The second XML child stores the configured mode", WrittenNode->GetNode(1)->GetValueAsUnsignedInt(), MModuleSaver::c_RoaFile) && Passed;
  delete WrittenNode;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleSaver::TestSaving()
{
  bool Passed = true;

  MModuleSaver InvalidModule;
  InvalidModule.SetFileName(GetTemporaryFileName("invalid.txt"));

  // The unsupported-extension path prints a diagnostic gated by the global verbosity. Silence only
  // the noisy call itself -- an Evaluate* made while output is suppressed would hide its own failure.
  const int PreviousVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  const bool InvalidInitialized = InvalidModule.Initialize();
  g_Verbosity = PreviousVerbosity;
  Passed = EvaluateFalse("Initialize()", "invalid extension", "Initialization fails for unsupported output file extensions", InvalidInitialized) && Passed;

  MModuleSaver RoaModule;
  const MString RoaFileName = GetTemporaryFileName("saved.roa");
  RemoveTemporaryFile(RoaFileName);
  RoaModule.SetFileName(RoaFileName);
  Passed = EvaluateTrue("Initialize()", "ROA output", "Initialization succeeds for ROA output files", RoaModule.Initialize()) && Passed;
  MReadOutAssembly* RoaEvent = CreateRepresentativeEvent(101);
  Passed = EvaluateTrue("AnalyzeEvent()", "ROA output", "The event saver writes a representative event in ROA mode", RoaModule.AnalyzeEvent(RoaEvent)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "ROA progress", "The event saver marks the saved event with event-saver progress", RoaEvent->HasAnalysisProgress(MAssembly::c_EventSaver)) && Passed;
  RoaModule.Finalize();
  delete RoaEvent;
  const MString RoaText = ReadTextFile(RoaFileName);
  Passed = EvaluateTrue("Initialize()", "ROA header", "ROA output contains the representative ROA header", RoaText.Contains("TYPE ROA")) && Passed;
  Passed = EvaluateTrue("Initialize()", "ROA read-out format", "ROA output declares the read-out element and data types MFileReadOuts needs", RoaText.Contains("UF singlesidedstrip adc")) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "ROA event", "ROA output contains the representative event ID and read-out", RoaText.Contains("ID 101") && RoaText.Contains("UH 7 11 1234")) && Passed;
  Passed = EvaluateTrue("Finalize()", "ROA trailer", "ROA output is closed with the EN trailer", RoaText.EndsWith("EN\n")) && Passed;
  RemoveTemporaryFile(RoaFileName);

  MModuleSaver EvtaModule;
  const MString EvtaFileName = GetTemporaryFileName("saved.evta");
  RemoveTemporaryFile(EvtaFileName);
  EvtaModule.SetFileName(EvtaFileName);
  Passed = EvaluateTrue("Initialize()", "EVTA output", "Initialization succeeds for EVTA output files", EvtaModule.Initialize()) && Passed;
  MReadOutAssembly* EvtaEvent = CreateRepresentativeEvent(202);
  Passed = EvaluateTrue("AnalyzeEvent()", "EVTA output", "The event saver writes a representative event in EVTA mode", EvtaModule.AnalyzeEvent(EvtaEvent)) && Passed;
  EvtaModule.Finalize();
  delete EvtaEvent;
  const MString EvtaText = ReadTextFile(EvtaFileName);
  Passed = EvaluateTrue("Initialize()", "EVTA header", "EVTA output contains the representative EVTA header", EvtaText.Contains("Version 21") && EvtaText.Contains("Type EVTA")) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "EVTA event", "EVTA output contains the representative event ID", EvtaText.Contains("ID 202")) && Passed;
  Passed = EvaluateTrue("Finalize()", "EVTA trailer", "EVTA output is closed with the EN trailer", EvtaText.EndsWith("EN\n")) && Passed;
  RemoveTemporaryFile(EvtaFileName);

  MModuleSaver ReopenModule;
  const MString ReopenFirstFileName = GetTemporaryFileName("reopen_first.roa");
  const MString ReopenSecondFileName = GetTemporaryFileName("reopen_second.roa");
  RemoveTemporaryFile(ReopenFirstFileName);
  RemoveTemporaryFile(ReopenSecondFileName);
  ReopenModule.SetFileName(ReopenFirstFileName);
  Passed = EvaluateTrue("Initialize()", "first reopen output", "The event saver initializes with the first representative output file before finalization", ReopenModule.Initialize()) && Passed;
  MReadOutAssembly* ReopenEvent = CreateRepresentativeEvent(303);
  Passed = EvaluateTrue("AnalyzeEvent()", "first reopen output", "The event saver writes to the first representative output file", ReopenModule.AnalyzeEvent(ReopenEvent)) && Passed;
  ReopenModule.Finalize();
  ReopenModule.SetFileName(ReopenSecondFileName);
  Passed = EvaluateTrue("Initialize()", "second reopen output", "The event saver can initialize a second output file after finalization closes the first", ReopenModule.Initialize()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "second reopen output", "The event saver writes to the second representative output file after reinitialization", ReopenModule.AnalyzeEvent(ReopenEvent)) && Passed;
  ReopenModule.Finalize();
  delete ReopenEvent;
  Passed = EvaluateTrue("Finalize()", "first reopen output", "The first reopen output was written and closed", ReadTextFile(ReopenFirstFileName).Contains("ID 303") && ReadTextFile(ReopenFirstFileName).EndsWith("EN\n")) && Passed;
  Passed = EvaluateTrue("Finalize()", "second reopen output", "The second reopen output was written and closed", ReadTextFile(ReopenSecondFileName).Contains("ID 303") && ReadTextFile(ReopenSecondFileName).EndsWith("EN\n")) && Passed;
  RemoveTemporaryFile(ReopenFirstFileName);
  RemoveTemporaryFile(ReopenSecondFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleSaver::TestInheritedFlow()
{
  bool Passed = true;

  MModuleSaver FlowModule;
  const MString FlowFileName = GetTemporaryFileName("flow.roa");
  RemoveTemporaryFile(FlowFileName);
  FlowModule.SetFileName(FlowFileName);
  Passed = EvaluateTrue("Initialize()", "flow output", "Initialization succeeds for inherited flow output", FlowModule.Initialize()) && Passed;

  MReadOutAssembly* FlowEvent = CreateRepresentativeEvent(404);
  FlowEvent->SetAnalysisProgress(MAssembly::c_EventLoader);
  Passed = EvaluateTrue("AddReadOutAssembly()", "flow event", "The inherited input queue accepts a representative event to save", FlowModule.AddReadOutAssembly(FlowEvent)) && Passed;
  Passed = EvaluateTrue("DoSingleAnalysis()", "flow event", "The inherited analysis flow saves one queued event", FlowModule.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "flow event", "One saved event passed through the inherited analysis flow", FlowModule.GetNumberOfAnalyzedEvents(), 1L) && Passed;
  Passed = EvaluateTrue("HasAnalyzedReadOutAssemblies()", "flow event", "The inherited analysis flow queues the saved event for downstream modules", FlowModule.HasAnalyzedReadOutAssemblies()) && Passed;
  MReadOutAssembly* SavedEvent = FlowModule.GetAnalyzedReadOutAssembly();
  Passed = EvaluateTrue("GetAnalyzedReadOutAssembly()", "flow event", "The saved event remains available downstream", SavedEvent == FlowEvent) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "flow event progress", "The saved event receives event-saver progress", SavedEvent->HasAnalysisProgress(MAssembly::c_EventSaver)) && Passed;
  delete SavedEvent;
  FlowModule.Finalize();

  const MString FlowText = ReadTextFile(FlowFileName);
  Passed = EvaluateTrue("DoSingleAnalysis()", "flow output content", "The inherited analysis flow writes the representative event to disk", FlowText.Contains("ID 404") && FlowText.EndsWith("EN\n")) && Passed;
  RemoveTemporaryFile(FlowFileName);

  MModuleSaver MissingRequirementModule;
  const MString MissingRequirementFileName = GetTemporaryFileName("missing_requirement.roa");
  RemoveTemporaryFile(MissingRequirementFileName);
  MissingRequirementModule.SetFileName(MissingRequirementFileName);
  Passed = EvaluateTrue("Initialize()", "missing predecessor progress", "Initialization succeeds before testing inherited requirement filtering", MissingRequirementModule.Initialize()) && Passed;
  MReadOutAssembly* MissingRequirementEvent = CreateRepresentativeEvent(505);
  Passed = EvaluateTrue("AddReadOutAssembly()", "missing predecessor progress", "The inherited queue accepts an event without event-loader progress", MissingRequirementModule.AddReadOutAssembly(MissingRequirementEvent)) && Passed;
  Passed = EvaluateTrue("DoSingleAnalysis()", "missing predecessor progress", "Events missing predecessor progress still pass through inherited queues", MissingRequirementModule.DoSingleAnalysis()) && Passed;
  MReadOutAssembly* UnsavedEvent = MissingRequirementModule.GetAnalyzedReadOutAssembly();
  Passed = EvaluateFalse("AnalyzeEvent()", "missing predecessor progress", "The event saver does not mark or save events missing event-loader progress", UnsavedEvent->HasAnalysisProgress(MAssembly::c_EventSaver)) && Passed;
  delete UnsavedEvent;
  MissingRequirementModule.Finalize();
  Passed = EvaluateFalse("DoSingleAnalysis()", "missing predecessor output", "The output file does not contain the event that missed required predecessor progress", ReadTextFile(MissingRequirementFileName).Contains("ID 505")) && Passed;
  RemoveTemporaryFile(MissingRequirementFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleSaver::TestRoaRoundTrip()
{
  bool Passed = true;

  // A roa file whose first event carries no read-outs has no determinable read-out format, so the
  // saver has to refuse it rather than produce a file MFileReadOuts cannot reopen
  const MString FormatlessFileName = GetTemporaryFileName("roundtrip_formatless.roa");
  RemoveTemporaryFile(FormatlessFileName);

  MModuleSaver FormatlessSaver;
  FormatlessSaver.SetFileName(FormatlessFileName);
  Passed = EvaluateTrue("Initialize()", "formatless writer", "The saver initializes for a file whose first event has no read-outs", FormatlessSaver.Initialize()) && Passed;
  Passed = EvaluateTrue("IsOK()", "before formatless event", "The saver reports itself OK before the undeterminable event arrives", FormatlessSaver.IsOK()) && Passed;

  MReadOutAssembly Formatless;
  Formatless.SetFilteredOut(false);
  Formatless.SetID(1);
  const int PreviousVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  const bool FormatlessAccepted = FormatlessSaver.AnalyzeEvent(&Formatless);
  g_Verbosity = PreviousVerbosity;
  Passed = EvaluateFalse("AnalyzeEvent()", "first event without read-outs", "The saver rejects a first event whose read-out format cannot be determined", FormatlessAccepted) && Passed;
  Passed = EvaluateFalse("IsOK()", "after formatless event", "The saver reports itself no longer OK so the supervisor shuts the sequence down", FormatlessSaver.IsOK()) && Passed;
  FormatlessSaver.Finalize();
  RemoveTemporaryFile(FormatlessFileName);

  // Write a representative event, read it back with the loader, and compare every field that was
  // set. The roa format is text, so the comparison is field-exact rather than byte-exact.
  const MString WrittenFileName = GetTemporaryFileName("roundtrip_written.roa");
  RemoveTemporaryFile(WrittenFileName);

  MModuleSaver Saver;
  Saver.SetFileName(WrittenFileName);
  Passed = EvaluateTrue("Initialize()", "round-trip writer", "The saver initializes for the round-trip file", Saver.Initialize()) && Passed;

  MReadOutAssembly Written;
  Written.SetFilteredOut(false);
  Written.SetID(909);
  Written.SetTime(MTime(2.25));
  MReadOutElementStrip Element(7, 11);
  MReadOutDataADCValue Data;
  Data.SetADCValue(1234);
  Written.AddReadOut(MReadOut(Element, Data));
  Passed = EvaluateTrue("AnalyzeEvent()", "round-trip event", "The representative event is written to the roa file", Saver.AnalyzeEvent(&Written)) && Passed;
  Saver.Finalize();

  MModuleLoaderRoa Loader;
  Loader.SetFileName(WrittenFileName);
  Passed = EvaluateTrue("Initialize()", "round-trip reader", "A roa file written by the saver can be opened by the loader", Loader.Initialize()) && Passed;

  MReadOutAssembly ReadBack;
  ReadBack.SetFilteredOut(false);
  Passed = EvaluateTrue("AnalyzeEvent()", "round-trip read", "The written event can be read back from the roa file", Loader.AnalyzeEvent(&ReadBack)) && Passed;
  Passed = Evaluate("GetID()", "round trip", "The event ID survives the write-read round trip", ReadBack.GetID(), Written.GetID()) && Passed;
  Passed = EvaluateNear("GetTime()", "round trip", "The event time survives the write-read round trip", ReadBack.GetTime().GetAsSeconds(), Written.GetTime().GetAsSeconds(), 1.0e-9) && Passed;
  Passed = EvaluateSize("GetNumberOfReadOuts()", "round trip", "The number of read-outs survives the write-read round trip", ReadBack.GetNumberOfReadOuts(), static_cast<size_t>(Written.GetNumberOfReadOuts())) && Passed;
  if (ReadBack.GetNumberOfReadOuts() == 1) {
    Passed = Evaluate("GetDetectorID()", "round trip", "The read-out detector ID survives the write-read round trip", ReadBack.GetReadOut(0).GetReadOutElement().GetDetectorID(), 7U) && Passed;
    Passed = Evaluate("ToParsableString()", "round trip", "The whole read-out survives the write-read round trip unchanged", ReadBack.GetReadOut(0).ToParsableString(false), Written.GetReadOut(0).ToParsableString(false)) && Passed;
  }
  Loader.Finalize();

  // Write the event that came back out again: a format the loader and saver agree on has to
  // reproduce the original file exactly.
  const MString RewrittenFileName = GetTemporaryFileName("roundtrip_rewritten.roa");
  RemoveTemporaryFile(RewrittenFileName);

  MModuleSaver Rewriter;
  Rewriter.SetFileName(RewrittenFileName);
  Passed = EvaluateTrue("Initialize()", "round-trip rewriter", "The saver initializes for the rewritten round-trip file", Rewriter.Initialize()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "round-trip rewrite", "The event read back from disk can be written again", Rewriter.AnalyzeEvent(&ReadBack)) && Passed;
  Rewriter.Finalize();

  Passed = EvaluateFilesIdentical("StreamRoa()", "round trip", "Writing, reading and rewriting an event reproduces the original roa file", RewrittenFileName, WrittenFileName) && Passed;

  Passed = EvaluateTrue("RemoveTemporaryFile()", "round-trip files", "The round-trip fixture files can be removed", RemoveTemporaryFile(WrittenFileName) && RemoveTemporaryFile(RewrittenFileName)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTModuleSaver Test;
  return Test.Run() == true ? 0 : 1;
}
