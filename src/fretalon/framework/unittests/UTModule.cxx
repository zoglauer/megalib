/*
 * UTModule.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MModule.h"
#include "MReadOutAssembly.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Standard libs:
#include <sstream>
#include <stdexcept>
#include <cstdint>
using namespace std;


//! Unit test class for MModule
class UTModule : public MUnitTest
{
public:
  //! Default constructor
  UTModule() : MUnitTest("UTModule") {}
  //! Default destructor
  virtual ~UTModule() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test helper exposing protected MModule functionality
  class ModuleProbe : public MModule
  {
  public:
    ModuleProbe() : MModule(), m_AnalyzeCalls(0), m_WasAnalyzingDuringAnalysis(false), m_FinishDuringAnalysis(false) {}
    virtual ~ModuleProbe() {}

    virtual MModule* Clone() { return new ModuleProbe(); }
    virtual bool AnalyzeEvent(MReadOutAssembly* Event)
    {
      ++m_AnalyzeCalls;
      m_WasAnalyzingDuringAnalysis = IsAnalyzing();
      if (Event != nullptr) {
        Event->SetAnalysisProgress(c_OutputProgress);
      }
      if (m_FinishDuringAnalysis == true) {
        m_IsFinished = true;
      }
      return true;
    }

    void SetProbeName(const MString& Name) { SetName(Name); }
    void SetProbeXmlTag(const MString& XmlTag) { m_XmlTag = XmlTag; }
    void AddHardPreceedingType(uint64_t Type) { AddPreceedingModuleType(Type, true); }
    void AddSoftPreceedingType(uint64_t Type) { AddPreceedingModuleType(Type, false); }
    void AddImmediatePreceedingType(uint64_t Type, bool Hard = true) { AddPreceedingModuleType(Type, Hard, true); }
    void SetProbeTypeExclusive(bool Flag) { SetTypeExclusive(Flag); }
    void ClearProbePreceedingTypes() { ClearPreceedingModuleTypes(); }
    void AddProvidedType(uint64_t Type) { AddModuleType(Type); }
    void AddExpectedSucceedingType(uint64_t Type) { AddSucceedingModuleType(Type); }
    void SetStartModule(bool Flag) { m_IsStartModule = Flag; }
    void SetReady(bool Flag) { m_IsReady = Flag; }
    void SetOK(bool Flag) { m_IsOK = Flag; }
    void SetFinished(bool Flag) { m_IsFinished = Flag; }
    void SetAllowPausing(bool Flag) { m_AllowPausing = Flag; }
    void SetHasOptions(bool Flag) { m_HasOptionsGUI = Flag; }
    void SetAllowMultiThreading(bool Flag) { m_AllowMultiThreading = Flag; }
    void SetAllowMultipleInstances(bool Flag) { m_AllowMultipleInstances = Flag; }
    void SetFinishDuringAnalysis(bool Flag) { m_FinishDuringAnalysis = Flag; }
    void AddExpo(MGUIExpo* Expo) { m_Expos.push_back(Expo); }
    MDGeometryQuest* GetGeometry() const { return m_Geometry; }
    unsigned int GetAnalyzeCalls() const { return m_AnalyzeCalls; }
    bool WasAnalyzingDuringAnalysis() const { return m_WasAnalyzingDuringAnalysis; }

    static const uint64_t c_InputProgress = 0x1;
    static const uint64_t c_OutputProgress = 0x2;
    static const uint64_t c_SoftProgress = 0x4;

  private:
    unsigned int m_AnalyzeCalls;
    bool m_WasAnalyzingDuringAnalysis;
    bool m_FinishDuringAnalysis;
  };

  //! Test constructor defaults and protected metadata helpers
  bool TestDefaultsAndMetadata();
  //! Test pausing, initialization, and configuration helpers
  bool TestStateAndConfiguration();
  //! Test requirement checks and queue based event flow
  bool TestAnalysisFlow();
  //! Test start-module event generation and finish handling
  bool TestStartModuleFlow();
  //! Test remaining public API with side-effect-free helper paths
  bool TestAdditionalPublicAPI();
  //! Test the immediate and exclusivity flags used for GUI sequence building
  bool TestOrderingFlags();
};


////////////////////////////////////////////////////////////////////////////////


bool UTModule::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndMetadata() && Passed;
  Passed = TestStateAndConfiguration() && Passed;
  Passed = TestAnalysisFlow() && Passed;
  Passed = TestStartModuleFlow() && Passed;
  Passed = TestAdditionalPublicAPI() && Passed;
  Passed = TestOrderingFlags() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModule::TestDefaultsAndMetadata()
{
  bool Passed = true;

  ModuleProbe Module;
  Passed = Evaluate("GetName()", "default", "The base module starts with the representative default name", Module.GetName(), MString("Base class...")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The base module starts with the representative default XML tag", Module.GetXmlTag(), MString("BaseClass")) && Passed;
  Passed = EvaluateSize("GetNPreceedingModuleTypes()", "default", "No preceding module types are registered by default", Module.GetNPreceedingModuleTypes(), static_cast<size_t>(0)) && Passed;
  Passed = EvaluateSize("GetNModuleTypes()", "default", "No provided module types are registered by default", Module.GetNModuleTypes(), static_cast<size_t>(0)) && Passed;
  Passed = EvaluateSize("GetNSucceedingModuleTypes()", "default", "No succeeding module types are registered by default", Module.GetNSucceedingModuleTypes(), static_cast<size_t>(0)) && Passed;
  Passed = EvaluateFalse("AllowsMultiThreading()", "default", "The base module does not allow multi-threading by default", Module.AllowsMultiThreading()) && Passed;
  Passed = EvaluateFalse("AllowsMultipleInstances()", "default", "The base module does not allow multiple instances by default", Module.AllowsMultipleInstances()) && Passed;
  Passed = EvaluateFalse("IsMultiThreaded()", "default", "The base module is not running a worker thread by default", Module.IsMultiThreaded()) && Passed;
  Passed = EvaluateFalse("IsStartModule()", "default", "The base module is not a start module by default", Module.IsStartModule()) && Passed;
  Passed = EvaluateTrue("AllowPausing()", "default", "The base module can be paused by default", Module.AllowPausing()) && Passed;
  Passed = EvaluateFalse("IsPaused()", "default", "The base module starts unpaused", Module.IsPaused()) && Passed;
  Passed = EvaluateTrue("IsReady()", "default", "The base module starts ready", Module.IsReady()) && Passed;
  Passed = EvaluateTrue("IsOK()", "default", "The base module starts with OK status", Module.IsOK()) && Passed;
  Passed = EvaluateFalse("IsFinished()", "default", "The base module is not finished by default", Module.IsFinished()) && Passed;
  Passed = EvaluateFalse("IsAnalyzing()", "default", "The base module is not analyzing by default", Module.IsAnalyzing()) && Passed;
  Passed = EvaluateFalse("HasOptionsGUI()", "default", "The base module has no options GUI by default", Module.HasOptionsGUI()) && Passed;
  Passed = EvaluateFalse("HasExpos()", "default", "The base module has no expos by default", Module.HasExpos()) && Passed;
  Passed = EvaluateSize("GetExpos()", "default", "The expo list is empty by default", Module.GetExpos().size(), static_cast<size_t>(0)) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "default", "No events have passed through analysis by default", Module.GetNumberOfAnalyzedEvents(), 0L) && Passed;
  Passed = EvaluateNear("GetSleepingTime()", "default", "No sleeping time is recorded by default", Module.GetSleepingTime(), 0.0, 1.0e-12) && Passed;
  Passed = EvaluateNear("GetProcessingTime()", "default", "No processing time is recorded by default", Module.GetProcessingTime(), 0.0, 1.0e-6) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "default", "A freshly constructed module starts without a geometry pointer, not an uninitialized one", Module.GetGeometry() == nullptr) && Passed;

  Module.SetProbeName("ProbeModule");
  Module.SetProbeXmlTag("ProbeXml");
  Module.AddHardPreceedingType(ModuleProbe::c_InputProgress);
  Module.AddSoftPreceedingType(ModuleProbe::c_SoftProgress);
  Module.AddProvidedType(ModuleProbe::c_OutputProgress);
  Module.AddExpectedSucceedingType(0x8);

  Passed = Evaluate("SetName()", "ProbeModule", "The protected name setter updates the public module name", Module.GetName(), MString("ProbeModule")) && Passed;
  Passed = Evaluate("m_XmlTag", "ProbeXml", "The XML tag stored by a derived module is returned publicly", Module.GetXmlTag(), MString("ProbeXml")) && Passed;
  Passed = EvaluateSize("GetNPreceedingModuleTypes()", "two predecessors", "Both hard and soft predecessor types are registered", Module.GetNPreceedingModuleTypes(), static_cast<size_t>(2)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "first predecessor", "The first predecessor type is returned in insertion order", Module.GetPreceedingModuleType(0), ModuleProbe::c_InputProgress) && Passed;
  Passed = EvaluateTrue("GetPreceedingModuleHardRequirement()", "hard predecessor", "The hard predecessor stores its hard-requirement flag", Module.GetPreceedingModuleHardRequirement(0)) && Passed;
  Passed = EvaluateFalse("GetPreceedingModuleHardRequirement()", "soft predecessor", "The soft predecessor stores its soft-requirement flag", Module.GetPreceedingModuleHardRequirement(1)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "hard predecessor", "Hard predecessor lookup identifies registered hard types", Module.IsHardPreceedingModule(ModuleProbe::c_InputProgress)) && Passed;
  Passed = EvaluateTrue("IsSoftPreceedingModule()", "soft predecessor", "Soft predecessor lookup identifies registered soft types", Module.IsSoftPreceedingModule(ModuleProbe::c_SoftProgress)) && Passed;
  Passed = EvaluateFalse("IsHardPreceedingModule()", "missing predecessor", "Hard predecessor lookup rejects missing types", Module.IsHardPreceedingModule(0x80)) && Passed;
  Passed = EvaluateFalse("IsSoftPreceedingModule()", "hard predecessor", "Soft predecessor lookup rejects hard-only types", Module.IsSoftPreceedingModule(ModuleProbe::c_InputProgress)) && Passed;
  Passed = EvaluateSize("GetNModuleTypes()", "one provided type", "One provided module type is registered", Module.GetNModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetModuleType()", "provided type", "The provided module type is returned in insertion order", Module.GetModuleType(0), ModuleProbe::c_OutputProgress) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "provided type", "Provided module lookup accepts registered types", Module.ProvidesModuleType(ModuleProbe::c_OutputProgress)) && Passed;
  Passed = EvaluateFalse("ProvidesModuleType()", "missing type", "Provided module lookup rejects missing types", Module.ProvidesModuleType(0x80)) && Passed;
  Passed = EvaluateSize("GetNSucceedingModuleTypes()", "one succeeding type", "One succeeding module type is registered", Module.GetNSucceedingModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetSucceedingModuleType()", "succeeding type", "The succeeding module type is returned in insertion order", Module.GetSucceedingModuleType(0), static_cast<uint64_t>(0x8)) && Passed;

  // The four indexed accessors use vector::at(), so an out-of-range index is a documented error path
  Passed = EvaluateException<out_of_range>("GetPreceedingModuleType()", "index equal to the size", "Requesting a predecessor type one past the last registered one throws", [&]() { Module.GetPreceedingModuleType(Module.GetNPreceedingModuleTypes()); }) && Passed;
  Passed = EvaluateException<out_of_range>("GetPreceedingModuleHardRequirement()", "index equal to the size", "Requesting a predecessor requirement one past the last registered one throws", [&]() { Module.GetPreceedingModuleHardRequirement(Module.GetNPreceedingModuleTypes()); }) && Passed;
  Passed = EvaluateException<out_of_range>("GetModuleType()", "index equal to the size", "Requesting a provided type one past the last registered one throws", [&]() { Module.GetModuleType(Module.GetNModuleTypes()); }) && Passed;
  Passed = EvaluateException<out_of_range>("GetSucceedingModuleType()", "index equal to the size", "Requesting a succeeding type one past the last registered one throws", [&]() { Module.GetSucceedingModuleType(Module.GetNSucceedingModuleTypes()); }) && Passed;

  // An empty module has no registered types at all, so even index zero is out of range
  ModuleProbe EmptyMetadataModule;
  Passed = EvaluateException<out_of_range>("GetPreceedingModuleType()", "empty module", "Requesting a predecessor type from a module without predecessors throws", [&]() { EmptyMetadataModule.GetPreceedingModuleType(0); }) && Passed;
  Passed = EvaluateException<out_of_range>("GetModuleType()", "empty module", "Requesting a provided type from a module without provided types throws", [&]() { EmptyMetadataModule.GetModuleType(0); }) && Passed;

  MDGeometryQuest* Geometry = reinterpret_cast<MDGeometryQuest*>(static_cast<uintptr_t>(0x1));
  Module.SetGeometry(Geometry);
  Passed = EvaluateTrue("SetGeometry()", "representative pointer", "The base module stores the geometry pointer supplied by the framework", Module.GetGeometry() == Geometry) && Passed;

  MModule* Clone = Module.Clone();
  Passed = EvaluateTrue("Clone()", "ModuleProbe", "The derived clone implementation returns a distinct module instance", Clone != nullptr && Clone != &Module) && Passed;
  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModule::TestStateAndConfiguration()
{
  bool Passed = true;

  ModuleProbe Module;
  Module.Pause();
  Passed = EvaluateTrue("Pause()", "default", "Pause sets the paused flag when pausing is allowed", Module.IsPaused()) && Passed;
  Module.Pause(false);
  Passed = EvaluateFalse("Pause(false)", "default", "Pause(false) clears the paused flag when pausing is allowed", Module.IsPaused()) && Passed;

  Module.SetAllowPausing(false);
  Module.Pause();
  Passed = EvaluateFalse("Pause()", "pausing disabled", "Pause leaves the module unpaused when pausing is disabled", Module.IsPaused()) && Passed;

  Module.SetReady(false);
  Module.SetOK(false);
  Module.SetFinished(true);
  Module.Pause();
  Passed = EvaluateTrue("Initialize()", "state reset", "Initialize succeeds for the representative base module", Module.Initialize()) && Passed;
  Passed = EvaluateFalse("IsPaused()", "after Initialize", "Initialize clears the paused state", Module.IsPaused()) && Passed;
  Passed = EvaluateTrue("IsOK()", "after Initialize", "Initialize resets the OK status", Module.IsOK()) && Passed;
  Passed = EvaluateFalse("IsFinished()", "after Initialize", "Initialize clears the finished state", Module.IsFinished()) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "after Initialize", "Initialize resets the analyzed-event counter", Module.GetNumberOfAnalyzedEvents(), 0L) && Passed;
  Passed = EvaluateNear("GetSleepingTime()", "after Initialize", "Initialize resets the sleeping-time counter", Module.GetSleepingTime(), 0.0, 1.0e-12) && Passed;

  Module.SetHasOptions(true);
  Passed = EvaluateTrue("HasOptionsGUI()", "options enabled", "Derived modules can expose that an options GUI is available", Module.HasOptionsGUI()) && Passed;
  Module.ShowOptionsGUI();
  Module.CreateExpos();
  Passed = EvaluateTrue("ShowOptionsGUI()/CreateExpos()", "default hooks", "The base GUI hooks are callable no-ops", true) && Passed;

  MGUIExpo* SentinelExpo = reinterpret_cast<MGUIExpo*>(static_cast<uintptr_t>(0x1));
  Module.AddExpo(SentinelExpo);
  Passed = EvaluateTrue("HasExpos()", "one sentinel expo", "A derived module with an expo pointer reports that expos are available", Module.HasExpos()) && Passed;
  Passed = EvaluateSize("GetExpos()", "one sentinel expo", "The expo getter returns the registered expo pointer", Module.GetExpos().size(), static_cast<size_t>(1)) && Passed;
  Passed = EvaluateTrue("GetExpos()", "one sentinel expo", "The registered expo pointer is preserved", Module.GetExpos()[0] == SentinelExpo) && Passed;

  Module.SetProbeXmlTag("ModuleProbe");
  Passed = EvaluateTrue("ReadXmlConfiguration()", "nullptr", "The base XML reader accepts the representative default configuration", Module.ReadXmlConfiguration(nullptr)) && Passed;
  MXmlNode* Node = Module.CreateXmlConfiguration();
  Passed = Evaluate("CreateXmlConfiguration()", "ModuleProbe", "The base XML writer creates a node with the module XML tag", Node->GetName(), MString("ModuleProbe")) && Passed;
  delete Node;

  Module.AddReadOutAssembly(new MReadOutAssembly());
  Passed = EvaluateTrue("HasAddedReadOutAssemblies()", "before Finalize", "The representative input queue contains the added event before finalization", Module.HasAddedReadOutAssemblies()) && Passed;
  Module.Finalize();
  Passed = EvaluateFalse("HasAddedReadOutAssemblies()", "after Finalize", "Finalize clears the input queue", Module.HasAddedReadOutAssemblies()) && Passed;
  Passed = EvaluateFalse("HasAnalyzedReadOutAssemblies()", "after Finalize", "Finalize clears the output queue", Module.HasAnalyzedReadOutAssemblies()) && Passed;

  ModuleProbe CapabilityModule;
  CapabilityModule.SetAllowMultiThreading(true);
  CapabilityModule.SetAllowMultipleInstances(true);
  Passed = EvaluateTrue("AllowsMultiThreading()", "enabled by derived module", "Derived modules can report that multi-threading is allowed", CapabilityModule.AllowsMultiThreading()) && Passed;
  Passed = EvaluateTrue("AllowsMultipleInstances()", "enabled by derived module", "Derived modules can report that multiple instances are allowed", CapabilityModule.AllowsMultipleInstances()) && Passed;
  CapabilityModule.UseMultiThreading(false);
  Passed = EvaluateTrue("Initialize()", "multiple instances enabled", "Initialize succeeds when multiple-instance sorting is enabled", CapabilityModule.Initialize()) && Passed;
  Passed = EvaluateFalse("IsMultiThreaded()", "multi-threading disabled by request", "Disabling multi-threading prevents starting a worker thread even when the module allows it", CapabilityModule.IsMultiThreaded()) && Passed;

  ModuleProbe ThreadRequestOnlyModule;
  ThreadRequestOnlyModule.UseMultiThreading(true);
  Passed = EvaluateTrue("Initialize()", "multi-threading requested but not allowed", "Initialize succeeds when multi-threading is requested but the module does not allow worker threads", ThreadRequestOnlyModule.Initialize()) && Passed;
  Passed = EvaluateFalse("IsMultiThreaded()", "multi-threading requested but not allowed", "Requesting multi-threading alone does not start a worker thread", ThreadRequestOnlyModule.IsMultiThreaded()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModule::TestAnalysisFlow()
{
  bool Passed = true;

  ModuleProbe Module;
  MReadOutAssembly* Event = new MReadOutAssembly();
  Event->SetFilteredOut(false);
  Passed = EvaluateTrue("AddReadOutAssembly()", "representative event", "The module accepts a representative input event", Module.AddReadOutAssembly(Event)) && Passed;
  Passed = EvaluateTrue("HasAddedReadOutAssemblies()", "representative event", "The input queue reports the added event", Module.HasAddedReadOutAssemblies()) && Passed;
  Passed = EvaluateTrue("DoSingleAnalysis()", "representative event", "A ready module processes one queued event", Module.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "representative event", "The derived analysis method is called for a processable event", Module.GetAnalyzeCalls(), 1U) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "representative event", "One event passed through the module", Module.GetNumberOfAnalyzedEvents(), 1L) && Passed;
  Passed = EvaluateFalse("HasAddedReadOutAssemblies()", "after DoSingleAnalysis", "The input queue is empty after processing one event", Module.HasAddedReadOutAssemblies()) && Passed;
  Passed = EvaluateTrue("HasAnalyzedReadOutAssemblies()", "after DoSingleAnalysis", "The output queue receives the processed event", Module.HasAnalyzedReadOutAssemblies()) && Passed;
  MReadOutAssembly* Output = Module.GetAnalyzedReadOutAssembly();
  Passed = EvaluateTrue("GetAnalyzedReadOutAssembly()", "representative event", "The output queue returns the same event pointer that was analyzed", Output == Event) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "analysis progress", "The representative analysis marks its output progress on the event", Output->HasAnalysisProgress(ModuleProbe::c_OutputProgress)) && Passed;
  Passed = EvaluateTrue("IsAnalyzing()", "during AnalyzeEvent", "The module reports that analysis is active while the derived analysis method runs", Module.WasAnalyzingDuringAnalysis()) && Passed;
  Passed = EvaluateFalse("IsAnalyzing()", "after DoSingleAnalysis", "The module clears the analyzing flag after the event has passed through", Module.IsAnalyzing()) && Passed;
  Passed = EvaluateTrue("GetProcessingTime()", "after DoSingleAnalysis", "Processing time remains a non-negative value after analysis", Module.GetProcessingTime() >= 0.0) && Passed;
  delete Output;

  ModuleProbe RequirementModule;
  RequirementModule.AddHardPreceedingType(ModuleProbe::c_InputProgress);
  MReadOutAssembly RequiredEvent;
  RequiredEvent.SetFilteredOut(false);
  RequiredEvent.SetAnalysisProgress(ModuleProbe::c_InputProgress);
  Passed = EvaluateTrue("FullfillsRequirements()", "required progress present", "The module accepts an event containing the required hard predecessor progress", RequirementModule.FullfillsRequirements(&RequiredEvent)) && Passed;
  MReadOutAssembly MissingRequiredEvent;
  MissingRequiredEvent.SetFilteredOut(false);
  Passed = EvaluateFalse("FullfillsRequirements()", "required progress missing", "The module rejects an event missing a registered hard predecessor progress", RequirementModule.FullfillsRequirements(&MissingRequiredEvent)) && Passed;

  ModuleProbe SoftRequirementModule;
  SoftRequirementModule.AddSoftPreceedingType(ModuleProbe::c_SoftProgress);
  MReadOutAssembly MissingSoftEvent;
  MissingSoftEvent.SetFilteredOut(false);
  Passed = EvaluateTrue("FullfillsRequirements()", "soft progress missing", "A missing soft predecessor progress does not block an event, because the providing module need not be in the sequence", SoftRequirementModule.FullfillsRequirements(&MissingSoftEvent)) && Passed;
  MissingSoftEvent.SetAnalysisProgress(ModuleProbe::c_SoftProgress);
  Passed = EvaluateTrue("FullfillsRequirements()", "soft progress present", "The base requirement check accepts an event containing the registered soft predecessor progress", SoftRequirementModule.FullfillsRequirements(&MissingSoftEvent)) && Passed;

  // A hard requirement still gates the event even when a soft one is registered alongside it
  ModuleProbe MixedRequirementModule;
  MixedRequirementModule.AddHardPreceedingType(ModuleProbe::c_InputProgress);
  MixedRequirementModule.AddSoftPreceedingType(ModuleProbe::c_SoftProgress);
  MReadOutAssembly MixedEvent;
  MixedEvent.SetFilteredOut(false);
  Passed = EvaluateFalse("FullfillsRequirements()", "mixed, hard missing", "A missing hard predecessor still blocks the event when a soft predecessor is also registered", MixedRequirementModule.FullfillsRequirements(&MixedEvent)) && Passed;
  MixedEvent.SetAnalysisProgress(ModuleProbe::c_InputProgress);
  Passed = EvaluateTrue("FullfillsRequirements()", "mixed, hard present", "Satisfying only the hard predecessor is enough when the other is a soft requirement", MixedRequirementModule.FullfillsRequirements(&MixedEvent)) && Passed;

  ModuleProbe FilterModule;
  MReadOutAssembly* FilteredEvent = new MReadOutAssembly();
  FilteredEvent->SetFilteredOut(true);
  Passed = EvaluateTrue("AddReadOutAssembly()", "filtered event", "The module accepts a representative filtered input event", FilterModule.AddReadOutAssembly(FilteredEvent)) && Passed;
  Passed = EvaluateTrue("DoSingleAnalysis()", "filtered event", "Filtered events still pass through the module queues", FilterModule.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "filtered event", "Filtered events bypass the derived analysis method", FilterModule.GetAnalyzeCalls(), 0U) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "filtered event", "Filtered events still count as passed through the module", FilterModule.GetNumberOfAnalyzedEvents(), 1L) && Passed;
  delete FilterModule.GetAnalyzedReadOutAssembly();

  ModuleProbe NotReadyModule;
  MReadOutAssembly* QueuedEvent = new MReadOutAssembly();
  QueuedEvent->SetFilteredOut(false);
  NotReadyModule.AddReadOutAssembly(QueuedEvent);
  NotReadyModule.SetReady(false);
  Passed = EvaluateFalse("DoSingleAnalysis()", "not ready", "A module that is not ready does not consume queued events", NotReadyModule.DoSingleAnalysis()) && Passed;
  Passed = EvaluateTrue("HasAddedReadOutAssemblies()", "not ready", "The event remains queued when the module is not ready", NotReadyModule.HasAddedReadOutAssemblies()) && Passed;
  NotReadyModule.ClearQueues();

  ModuleProbe NotOKModule;
  NotOKModule.SetOK(false);
  Passed = EvaluateFalse("DoSingleAnalysis()", "not OK", "A module with a non-OK status does not analyze", NotOKModule.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "not OK", "The derived analysis method is not called when status is not OK", NotOKModule.GetAnalyzeCalls(), 0U) && Passed;

  ModuleProbe EmptyModule;
  Passed = EvaluateFalse("DoSingleAnalysis()", "empty queue", "A non-start module without queued input has nothing to analyze", EmptyModule.DoSingleAnalysis()) && Passed;
  Passed = EvaluateTrue("GetAnalyzedReadOutAssembly()", "empty output queue", "An empty output queue returns no analyzed event", EmptyModule.GetAnalyzedReadOutAssembly() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModule::TestStartModuleFlow()
{
  bool Passed = true;

  ModuleProbe Module;
  Module.SetStartModule(true);
  Passed = EvaluateTrue("DoSingleAnalysis()", "start module", "A start module creates and processes a representative event without queued input", Module.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "start module", "The derived analysis method is called for the generated start-module event", Module.GetAnalyzeCalls(), 1U) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "start module", "Generated start-module events count as passed through the module", Module.GetNumberOfAnalyzedEvents(), 1L) && Passed;
  Passed = EvaluateTrue("HasAnalyzedReadOutAssemblies()", "start module", "The generated event is added to the output queue", Module.HasAnalyzedReadOutAssemblies()) && Passed;
  delete Module.GetAnalyzedReadOutAssembly();

  ModuleProbe FinishedModule;
  FinishedModule.SetStartModule(true);
  FinishedModule.SetFinished(true);
  Passed = EvaluateFalse("DoSingleAnalysis()", "already finished start module", "A finished start module does not generate new events", FinishedModule.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "already finished start module", "No analysis is attempted after a start module is already finished", FinishedModule.GetAnalyzeCalls(), 0U) && Passed;

  ModuleProbe FinishDuringAnalysisModule;
  FinishDuringAnalysisModule.SetStartModule(true);
  FinishDuringAnalysisModule.SetFinishDuringAnalysis(true);
  Passed = EvaluateTrue("DoSingleAnalysis()", "finish during analysis", "A start module can finish while analyzing its generated event", FinishDuringAnalysisModule.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "finish during analysis", "The generated event is analyzed before the module notices it has finished", FinishDuringAnalysisModule.GetAnalyzeCalls(), 1U) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "finish during analysis", "A terminal generated event is not counted as passed through", FinishDuringAnalysisModule.GetNumberOfAnalyzedEvents(), 0L) && Passed;
  Passed = EvaluateFalse("HasAnalyzedReadOutAssemblies()", "finish during analysis", "A terminal generated event is deleted instead of being queued", FinishDuringAnalysisModule.HasAnalyzedReadOutAssemblies()) && Passed;

  ModuleProbe InterruptedModule;
  InterruptedModule.SetStartModule(true);
  InterruptedModule.SetInterrupt();
  Passed = EvaluateFalse("DoSingleAnalysis()", "interrupted start module", "An interrupted start module does not generate a new event", InterruptedModule.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "interrupted start module", "The derived analysis method is not called while interrupted before generation", InterruptedModule.GetAnalyzeCalls(), 0U) && Passed;
  InterruptedModule.SetInterrupt(false);
  Passed = EvaluateTrue("DoSingleAnalysis()", "interrupt cleared", "Clearing the interrupt allows a start module to generate events again", InterruptedModule.DoSingleAnalysis()) && Passed;
  delete InterruptedModule.GetAnalyzedReadOutAssembly();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModule::TestAdditionalPublicAPI()
{
  bool Passed = true;

  ModuleProbe ClearModule;
  ClearModule.AddReadOutAssembly(new MReadOutAssembly());
  Passed = EvaluateTrue("HasAddedReadOutAssemblies()", "before ClearQueues", "The input queue contains a representative event before clearing", ClearModule.HasAddedReadOutAssemblies()) && Passed;
  ClearModule.ClearQueues();
  Passed = EvaluateFalse("HasAddedReadOutAssemblies()", "after ClearQueues", "ClearQueues removes queued input events", ClearModule.HasAddedReadOutAssemblies()) && Passed;
  Passed = EvaluateFalse("HasAnalyzedReadOutAssemblies()", "after ClearQueues", "ClearQueues leaves no queued output events", ClearModule.HasAnalyzedReadOutAssemblies()) && Passed;

  ModuleProbe SharingSource;
  ModuleProbe SharingTarget;
  SharingSource.ShareQueues(&SharingTarget);
  MReadOutAssembly* SharedEvent = new MReadOutAssembly();
  SharedEvent->SetFilteredOut(false);
  SharingSource.AddReadOutAssembly(SharedEvent);
  Passed = EvaluateTrue("ShareQueues()", "incoming source", "The source module reports an event added to the shared queue", SharingSource.HasAddedReadOutAssemblies()) && Passed;
  Passed = EvaluateTrue("ShareQueues()", "incoming target", "The target module sees incoming events from the shared queue", SharingTarget.HasAddedReadOutAssemblies()) && Passed;
  Passed = EvaluateTrue("DoSingleAnalysis()", "shared target", "The target module can process an event from the shared queue", SharingTarget.DoSingleAnalysis()) && Passed;
  Passed = EvaluateFalse("HasAddedReadOutAssemblies()", "shared queue consumed", "The source observes that the shared input queue has been consumed", SharingSource.HasAddedReadOutAssemblies()) && Passed;
  Passed = EvaluateTrue("HasAnalyzedReadOutAssemblies()", "shared output source", "The source sees output events produced through the shared queue", SharingSource.HasAnalyzedReadOutAssemblies()) && Passed;
  delete SharingSource.GetAnalyzedReadOutAssembly();

  ModuleProbe InterruptedLoopModule;
  InterruptedLoopModule.SetInterrupt();
  Passed = EvaluateTrue("MModuleKickstartThread()", "pre-interrupted module", "The thread entry point returns after an already-interrupted analysis loop", MModuleKickstartThread(&InterruptedLoopModule) == nullptr) && Passed;
  Passed = EvaluateFalse("IsMultiThreaded()", "after interrupted AnalysisLoop", "The analysis loop clears the running flag before returning", InterruptedLoopModule.IsMultiThreaded()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModule::TestOrderingFlags()
{
  bool Passed = true;

  // The immediate flag is stored per requirement, alongside the hard/soft flag
  ModuleProbe Module;
  Module.AddHardPreceedingType(ModuleProbe::c_InputProgress);
  Module.AddImmediatePreceedingType(ModuleProbe::c_OutputProgress);
  Passed = EvaluateSize("AddPreceedingModuleType()", "two requirements", "Both requirements are registered", Module.GetNPreceedingModuleTypes(), static_cast<size_t>(2)) && Passed;
  Passed = EvaluateFalse("GetPreceedingModuleImmediateRequirement()", "plain requirement", "A requirement added without the immediate flag is not immediate", Module.GetPreceedingModuleImmediateRequirement(0)) && Passed;
  Passed = EvaluateTrue("GetPreceedingModuleImmediateRequirement()", "immediate requirement", "A requirement added with the immediate flag is immediate", Module.GetPreceedingModuleImmediateRequirement(1)) && Passed;
  Passed = EvaluateException<out_of_range>("GetPreceedingModuleImmediateRequirement()", "index equal to the size", "Requesting the immediate flag one past the last requirement throws", [&]() { Module.GetPreceedingModuleImmediateRequirement(Module.GetNPreceedingModuleTypes()); }) && Passed;

  // An immediate requirement is meaningless unless the type is present at all, so a soft one is
  // raised to a hard one rather than being stored as a contradictory combination
  const int PreviousVerbosity = g_Verbosity;
  ModuleProbe ContradictoryModule;
  g_Verbosity = c_Quiet;
  ContradictoryModule.AddImmediatePreceedingType(ModuleProbe::c_SoftProgress, false);
  g_Verbosity = PreviousVerbosity;
  Passed = EvaluateTrue("AddPreceedingModuleType()", "soft and immediate", "A soft requirement asked to be immediate is raised to a hard requirement", ContradictoryModule.GetPreceedingModuleHardRequirement(0)) && Passed;
  Passed = EvaluateTrue("AddPreceedingModuleType()", "soft and immediate", "The immediate flag itself is kept", ContradictoryModule.GetPreceedingModuleImmediateRequirement(0)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "soft and immediate", "The raised requirement is reported as a hard predecessor", ContradictoryModule.IsHardPreceedingModule(ModuleProbe::c_SoftProgress)) && Passed;
  Passed = EvaluateFalse("IsSoftPreceedingModule()", "soft and immediate", "The raised requirement is no longer reported as a soft predecessor", ContradictoryModule.IsSoftPreceedingModule(ModuleProbe::c_SoftProgress)) && Passed;

  // Raising the requirement silently would hide a mistake from the caller, so it is reported at
  // warning level. Capture the raw stream: the message content itself is what is being asserted.
  ModuleProbe WarningModule;
  WarningModule.SetProbeXmlTag("XmlTagWarningProbe");
  g_Verbosity = c_Warning;
  ostringstream ContradictoryOutput;
  streambuf* OriginalCout = cout.rdbuf(ContradictoryOutput.rdbuf());
  WarningModule.AddImmediatePreceedingType(ModuleProbe::c_SoftProgress, false);
  cout.rdbuf(OriginalCout);
  ostringstream LegitimateOutput;
  OriginalCout = cout.rdbuf(LegitimateOutput.rdbuf());
  WarningModule.AddImmediatePreceedingType(ModuleProbe::c_InputProgress, true);
  WarningModule.AddSoftPreceedingType(ModuleProbe::c_OutputProgress);
  cout.rdbuf(OriginalCout);
  g_Verbosity = PreviousVerbosity;

  Passed = EvaluateTrue("AddPreceedingModuleType()", "soft and immediate warning", "Raising a soft requirement to a hard one is reported at warning level", MString(ContradictoryOutput.str()).Contains("cannot be immediate")) && Passed;
  Passed = EvaluateTrue("AddPreceedingModuleType()", "soft and immediate warning", "The warning names the module it came from", MString(ContradictoryOutput.str()).Contains("XmlTagWarningProbe")) && Passed;
  Passed = EvaluateTrue("AddPreceedingModuleType()", "legitimate combinations", "Neither a hard immediate nor a plain soft requirement produces a warning", LegitimateOutput.str().empty()) && Passed;

  // Clearing has to drop the type, the hard flag and the immediate flag together, so that a module
  // rebuilding its requirements cannot end up with flags left over from the previous set
  ModuleProbe RebuiltModule;
  RebuiltModule.AddImmediatePreceedingType(ModuleProbe::c_InputProgress);
  RebuiltModule.AddHardPreceedingType(ModuleProbe::c_OutputProgress);
  RebuiltModule.ClearProbePreceedingTypes();
  Passed = EvaluateSize("ClearPreceedingModuleTypes()", "after clearing", "Clearing removes every registered requirement", RebuiltModule.GetNPreceedingModuleTypes(), static_cast<size_t>(0)) && Passed;
  RebuiltModule.AddSoftPreceedingType(ModuleProbe::c_SoftProgress);
  Passed = EvaluateSize("ClearPreceedingModuleTypes()", "rebuilt", "A requirement added after clearing is the only one left", RebuiltModule.GetNPreceedingModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "rebuilt", "The rebuilt requirement stores the new type", RebuiltModule.GetPreceedingModuleType(0), ModuleProbe::c_SoftProgress) && Passed;
  Passed = EvaluateFalse("GetPreceedingModuleHardRequirement()", "rebuilt", "The rebuilt requirement does not inherit the previous hard flag", RebuiltModule.GetPreceedingModuleHardRequirement(0)) && Passed;
  Passed = EvaluateFalse("GetPreceedingModuleImmediateRequirement()", "rebuilt", "The rebuilt requirement does not inherit the previous immediate flag", RebuiltModule.GetPreceedingModuleImmediateRequirement(0)) && Passed;

  // Type exclusivity
  ModuleProbe ExclusiveModule;
  Passed = EvaluateTrue("IsTypeExclusive()", "default", "A module is type exclusive by default", ExclusiveModule.IsTypeExclusive()) && Passed;
  ExclusiveModule.SetProbeTypeExclusive(false);
  Passed = EvaluateFalse("SetTypeExclusive()", "cleared", "Type exclusivity can be switched off", ExclusiveModule.IsTypeExclusive()) && Passed;
  ExclusiveModule.SetProbeTypeExclusive(true);
  Passed = EvaluateTrue("SetTypeExclusive()", "restored", "Type exclusivity can be switched back on", ExclusiveModule.IsTypeExclusive()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTModule Test;
  return Test.Run() == true ? 0 : 1;
}
