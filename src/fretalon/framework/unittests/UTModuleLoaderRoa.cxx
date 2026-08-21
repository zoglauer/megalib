/*
 * UTModuleLoaderRoa.cxx
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
#include "MReadOut.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutElementStrip.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Standard libs:
#include <sstream>
using namespace std;


//! Unit test class for MModuleLoaderRoa
class UTModuleLoaderRoa : public MUnitTest
{
public:
  //! Default constructor
  UTModuleLoaderRoa() : MUnitTest("UTModuleLoaderRoa") {}
  //! Default destructor
  virtual ~UTModuleLoaderRoa() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Write a minimal representative ROA fixture and return its path
  MString WriteRoaFixture(const MString& Name);
  //! Test construction-time module metadata and file-name interface behavior
  bool TestMetadataAndFileName();
  //! Test XML configuration helpers
  bool TestXmlConfiguration();
  //! Test file initialization and event loading
  bool TestLoading();
};


////////////////////////////////////////////////////////////////////////////////


bool UTModuleLoaderRoa::Run()
{
  bool Passed = true;

  Passed = TestMetadataAndFileName() && Passed;
  Passed = TestXmlConfiguration() && Passed;
  Passed = TestLoading() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTModuleLoaderRoa::WriteRoaFixture(const MString& Name)
{
  const MString FileName = GetTemporaryFileName(Name);

  MReadOutElementStrip Element(7, 11);
  MReadOutDataADCValue Data;
  Data.SetADCValue(1234);
  MReadOut ReadOut(Element, Data);

  ostringstream Content;
  Content<<"Type roa\n";
  Content<<"Version 1\n";
  Content<<"UF "<<Element.GetType()<<" "<<Data.GetType()<<"\n";
  Content<<"CB 0\n";
  Content<<"SE\n";
  Content<<"ID 42\n";
  Content<<"TI 12.5\n";
  Content<<ReadOut.ToParsableString(false)<<"\n";
  Content<<"SE\n";
  Content<<"EN\n";

  WriteTextFile(FileName, Content.str().c_str());
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleLoaderRoa::TestMetadataAndFileName()
{
  bool Passed = true;

  MModuleLoaderRoa Module;
  Passed = Evaluate("GetName()", "default", "The ROA loader exposes its representative module name", Module.GetName(), MString("Loader for Roa files")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The ROA loader exposes its representative XML tag", Module.GetXmlTag(), MString("XmlTagLoaderRoa")) && Passed;

  Passed = EvaluateSize("GetNPreceedingModuleTypes()", "loader", "The ROA loader has no predecessor requirements", Module.GetNPreceedingModuleTypes(), static_cast<size_t>(0)) && Passed;
  Passed = EvaluateSize("GetNModuleTypes()", "loader", "The ROA loader registers one provided module type", Module.GetNModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetModuleType()", "event loader", "The ROA loader provides the event-loader module type", Module.GetModuleType(0), static_cast<uint64_t>(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "event loader", "The ROA loader reports that it provides event loading", Module.ProvidesModuleType(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateFalse("ProvidesModuleType()", "missing type", "The ROA loader rejects a representative unregistered module type", Module.ProvidesModuleType(0x80000000ULL)) && Passed;

  Passed = EvaluateSize("GetNSucceedingModuleTypes()", "loader", "The ROA loader registers one succeeding module type", Module.GetNSucceedingModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetSucceedingModuleType()", "no restriction", "The ROA loader allows unrestricted succeeding modules", Module.GetSucceedingModuleType(0), static_cast<uint64_t>(MAssembly::c_NoRestriction)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "loader", "The ROA loader exposes an options GUI", Module.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("IsStartModule()", "loader", "The ROA loader is a start module", Module.IsStartModule()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "loader", "The ROA loader allows multi-threading", Module.AllowsMultiThreading()) && Passed;
  Passed = EvaluateFalse("AllowsMultipleInstances()", "loader", "The ROA loader does not allow multiple instances", Module.AllowsMultipleInstances()) && Passed;

  Module.SetFileName("representative.roa");
  Passed = Evaluate("SetFileName()/GetFileName()", "representative.roa", "The file-name interface stores the configured ROA file name", Module.GetFileName(), MString("representative.roa")) && Passed;

  MModuleLoaderRoa* Clone = Module.Clone();
  Passed = EvaluateTrue("Clone()", "loader", "Clone returns a distinct ROA loader instance", Clone != nullptr && Clone != &Module) && Passed;
  Passed = Evaluate("Clone()", "loader name", "The cloned ROA loader has the same representative module name", Clone->GetName(), MString("Loader for Roa files")) && Passed;
  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleLoaderRoa::TestXmlConfiguration()
{
  bool Passed = true;

  MModuleLoaderRoa Module;
  MXmlNode Node(nullptr, "XmlTagLoaderRoa");
  new MXmlNode(&Node, "FileName", MString("configured.roa"));
  Passed = EvaluateTrue("ReadXmlConfiguration()", "FileName node", "The ROA loader reads its file name from XML", Module.ReadXmlConfiguration(&Node)) && Passed;
  Passed = Evaluate("GetFileName()", "after XML read", "The XML reader stores the configured file name", Module.GetFileName(), MString("configured.roa")) && Passed;

  MXmlNode EmptyNode(nullptr, "XmlTagLoaderRoa");
  Passed = EvaluateTrue("ReadXmlConfiguration()", "missing FileName node", "The ROA loader accepts XML without a FileName node", Module.ReadXmlConfiguration(&EmptyNode)) && Passed;
  Passed = Evaluate("GetFileName()", "after missing FileName", "Missing FileName XML leaves the existing file name unchanged", Module.GetFileName(), MString("configured.roa")) && Passed;

  Module.SetFileName("written.roa");
  MXmlNode* WrittenNode = Module.CreateXmlConfiguration();
  Passed = Evaluate("CreateXmlConfiguration()", "node name", "The ROA loader XML writer creates a node with the loader XML tag", WrittenNode->GetName(), MString("XmlTagLoaderRoa")) && Passed;
  Passed = EvaluateSize("CreateXmlConfiguration()", "child nodes", "The ROA loader XML writer creates one FileName child", WrittenNode->GetNNodes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "FileName child name", "The XML child stores the FileName tag", WrittenNode->GetNode(0)->GetName(), MString("FileName")) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "FileName child value", "The XML child stores the configured file name", WrittenNode->GetNode(0)->GetValue(), MString("written.roa")) && Passed;
  delete WrittenNode;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleLoaderRoa::TestLoading()
{
  bool Passed = true;

  MModuleLoaderRoa MissingModule;
  MissingModule.SetFileName(GetTemporaryFileName("missing.roa"));
  // The missing-file path reports through mgui. Silence only the noisy call itself -- an Evaluate*
  // made while the MEGAlib streams are disabled would hide its own failure message, which mout carries.
  DisableDefaultStreams();
  const bool MissingInitialized = MissingModule.Initialize();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Initialize()", "missing file", "Initialization fails when the configured ROA file does not exist", MissingInitialized) && Passed;

  const MString RoaFileName = WriteRoaFixture("loader_fixture.roa");

  MModuleLoaderRoa Module;
  Module.SetFileName(RoaFileName);
  Passed = EvaluateTrue("Initialize()", "valid ROA file", "Initialization succeeds for a representative ROA file", Module.Initialize()) && Passed;

  MReadOutAssembly Event;
  Event.SetFilteredOut(false);
  Passed = EvaluateTrue("AnalyzeEvent()", "first event", "AnalyzeEvent reads the first event from the ROA file", Module.AnalyzeEvent(&Event)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "event-loader progress", "The loaded event is marked with event-loader progress", Event.HasAnalysisProgress(MAssembly::c_EventLoader)) && Passed;
  Passed = Evaluate("GetID()", "first event", "The event ID is read from the ROA file", Event.GetID(), static_cast<unsigned long>(42)) && Passed;
  Passed = EvaluateSize("GetNumberOfReadOuts()", "first event", "The event contains the representative read-out from the ROA file", Event.GetNumberOfReadOuts(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetReadOutElement().GetDetectorID()", "first read-out", "The read-out detector ID is read from the ROA file", Event.GetReadOut(0).GetReadOutElement().GetDetectorID(), 7U) && Passed;

  DisableDefaultStreams();
  const bool HasSecondEvent = Module.AnalyzeEvent(&Event);
  EnableDefaultStreams();
  Passed = EvaluateFalse("AnalyzeEvent()", "end of file", "AnalyzeEvent returns false after the final ROA event", HasSecondEvent) && Passed;
  Passed = EvaluateTrue("IsFinished()", "end of file", "The ROA loader marks itself finished after reaching end of file", Module.IsFinished()) && Passed;

  Module.Finalize();
  RemoveTemporaryFile(RoaFileName);

  const MString ReopenFirstFileName = WriteRoaFixture("loader_reopen_first.roa");
  const MString ReopenSecondFileName = WriteRoaFixture("loader_reopen_second.roa");
  MModuleLoaderRoa ReopenModule;
  ReopenModule.SetFileName(ReopenFirstFileName);
  Passed = EvaluateTrue("Initialize()", "first reopen fixture", "The ROA loader initializes with the first representative file before finalization", ReopenModule.Initialize()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "first reopen fixture", "The ROA loader reads an event from the first representative file", ReopenModule.AnalyzeEvent(&Event)) && Passed;
  ReopenModule.Finalize();
  ReopenModule.SetFileName(ReopenSecondFileName);
  Passed = EvaluateTrue("Initialize()", "second reopen fixture", "The ROA loader can initialize a second representative file after finalization closes the first", ReopenModule.Initialize()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "second reopen fixture", "The ROA loader reads an event from the second representative file after reinitialization", ReopenModule.AnalyzeEvent(&Event)) && Passed;
  // Regression: loading into an already-used assembly must not accumulate the earlier read-outs
  Passed = EvaluateSize("AnalyzeEvent()", "reused assembly", "Loading into a reused assembly replaces rather than appends its read-outs", Event.GetNumberOfReadOuts(), static_cast<size_t>(1)) && Passed;
  ReopenModule.Finalize();
  RemoveTemporaryFile(ReopenFirstFileName);
  RemoveTemporaryFile(ReopenSecondFileName);

  MModuleLoaderRoa FlowModule;
  FlowModule.SetFileName(WriteRoaFixture("loader_flow_fixture.roa"));
  Passed = EvaluateTrue("Initialize()", "flow fixture", "Initialization succeeds for the inherited start-module flow fixture", FlowModule.Initialize()) && Passed;
  Passed = EvaluateTrue("DoSingleAnalysis()", "start-module flow", "The inherited start-module flow creates and loads one event", FlowModule.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "start-module flow", "One loaded event passed through the inherited analysis flow", FlowModule.GetNumberOfAnalyzedEvents(), 1L) && Passed;
  Passed = EvaluateTrue("HasAnalyzedReadOutAssemblies()", "start-module flow", "The inherited analysis flow queues the loaded event", FlowModule.HasAnalyzedReadOutAssemblies()) && Passed;
  delete FlowModule.GetAnalyzedReadOutAssembly();
  DisableDefaultStreams();
  const bool FlowTerminalAnalyzed = FlowModule.DoSingleAnalysis();
  EnableDefaultStreams();
  Passed = EvaluateTrue("DoSingleAnalysis()", "flow end of file", "The inherited flow returns true for the terminal generated event it deletes", FlowTerminalAnalyzed) && Passed;
  Passed = EvaluateTrue("IsFinished()", "flow end of file", "The inherited flow marks the loader finished at end of file", FlowModule.IsFinished()) && Passed;
  Passed = EvaluateFalse("HasAnalyzedReadOutAssemblies()", "flow end of file", "The terminal generated event is not queued", FlowModule.HasAnalyzedReadOutAssemblies()) && Passed;
  RemoveTemporaryFile(FlowModule.GetFileName());

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTModuleLoaderRoa Test;
  return Test.Run() == true ? 0 : 1;
}
