/*
 * UTSupervisor.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MAssembly.h"
#include "MFile.h"
#include "MModule.h"
#include "MSupervisor.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// ROOT libs:
#include "TError.h"
#include "TGClient.h"
#include "TSystem.h"

// Standard libs:
#include <sstream>
using namespace std;


//! Unit test class for MSupervisor
class UTSupervisor : public MUnitTest
{
public:
  //! Default constructor
  UTSupervisor() : MUnitTest("UTSupervisor") {}
  //! Default destructor
  virtual ~UTSupervisor() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test helper exposing representative module metadata and XML options
  class SupervisorModuleProbe : public MModule
  {
  public:
    SupervisorModuleProbe(const MString& Name, const MString& XmlTag, uint64_t ProvidedType = 0, bool StartModule = false) :
      MModule(),
      m_ConfigValue("default"),
      m_MaxGeneratedEvents(-1),
      m_InitializeResult(true),
      m_MarkNotOKDuringAnalysis(false),
      m_RejectConfiguration(false),
      m_SupervisorToInterrupt(nullptr),
      m_InitializeCalls(0),
      m_FinalizeCalls(0),
      m_AnalyzeCalls(0)
    {
      SetName(Name);
      m_XmlTag = XmlTag;
      if (ProvidedType != 0) {
        AddModuleType(ProvidedType);
      }
      m_IsStartModule = StartModule;
    }
    virtual ~SupervisorModuleProbe() {}

    virtual MModule* Clone()
    {
      SupervisorModuleProbe* Module = new SupervisorModuleProbe(m_Name, m_XmlTag);
      Module->m_ConfigValue = m_ConfigValue;
      Module->m_PreceedingModules = m_PreceedingModules;
      Module->m_SucceedingModules = m_SucceedingModules;
      Module->m_Modules = m_Modules;
      Module->m_IsStartModule = m_IsStartModule;
      return Module;
    }

    virtual bool Initialize()
    {
      ++m_InitializeCalls;
      // Simulates a soft interrupt landing during this module's own Initialize() -- returns
      // false as if init failed, with the interrupt flag already set on return
      if (m_SupervisorToInterrupt != nullptr) {
        m_SupervisorToInterrupt->SetSoftInterrupt(true);
        return false;
      }
      if (m_InitializeResult == false) {
        return false;
      }
      return MModule::Initialize();
    }

    virtual void Finalize()
    {
      ++m_FinalizeCalls;
      MModule::Finalize();
    }

    virtual bool AnalyzeEvent(MReadOutAssembly* Event)
    {
      ++m_AnalyzeCalls;
      if (Event != nullptr) {
        for (unsigned int m = 0; m < GetNModuleTypes(); ++m) {
          Event->SetAnalysisProgress(GetModuleType(m));
        }
        if (GetNModuleTypes() == 0) {
          Event->SetAnalysisProgress(MAssembly::c_NoRestriction);
        }
      }
      if (m_IsStartModule == true && m_MaxGeneratedEvents >= 0 && m_AnalyzeCalls > static_cast<unsigned int>(m_MaxGeneratedEvents)) {
        m_IsFinished = true;
      }
      if (m_MarkNotOKDuringAnalysis == true) {
        m_IsOK = false;
      }
      return true;
    }

    virtual bool ReadXmlConfiguration(MXmlNode* Node)
    {
      if (m_RejectConfiguration == true) {
        return false;
      }
      MXmlNode* ConfigValue = Node->GetNode("ConfigValue");
      if (ConfigValue != nullptr) {
        m_ConfigValue = ConfigValue->GetValue();
      }
      return true;
    }

    virtual MXmlNode* CreateXmlConfiguration()
    {
      MXmlNode* Node = new MXmlNode(nullptr, m_XmlTag);
      new MXmlNode(Node, "ConfigValue", m_ConfigValue);
      return Node;
    }

    void AddHardPreceedingType(uint64_t Type) { AddPreceedingModuleType(Type, true); }
    void AddSoftPreceedingType(uint64_t Type) { AddPreceedingModuleType(Type, false); }
    void AddImmediatePreceedingType(uint64_t Type) { AddPreceedingModuleType(Type, true, true); }
    void SetProbeTypeExclusive(bool Flag) { SetTypeExclusive(Flag); }
    void AddProvidedType(uint64_t Type) { AddModuleType(Type); }
    void SetConfigValue(const MString& ConfigValue) { m_ConfigValue = ConfigValue; }
    void SetMaxGeneratedEvents(int MaxGeneratedEvents) { m_MaxGeneratedEvents = MaxGeneratedEvents; }
    void SetInitializeResult(bool InitializeResult) { m_InitializeResult = InitializeResult; }
    void SetMarkNotOKDuringAnalysis(bool MarkNotOKDuringAnalysis) { m_MarkNotOKDuringAnalysis = MarkNotOKDuringAnalysis; }
    void SetRejectConfiguration(bool Flag) { m_RejectConfiguration = Flag; }
    void SetSoftInterruptOnInitialize(MSupervisor* Supervisor) { m_SupervisorToInterrupt = Supervisor; }
    void SetAllowMultiThreading(bool Flag) { m_AllowMultiThreading = Flag; }
    void ResetProbeState()
    {
      m_MaxGeneratedEvents = -1;
      m_InitializeResult = true;
      m_MarkNotOKDuringAnalysis = false;
      m_RejectConfiguration = false;
      m_SupervisorToInterrupt = nullptr;
      m_InitializeCalls = 0;
      m_FinalizeCalls = 0;
      m_AnalyzeCalls = 0;
      m_IsOK = true;
      m_IsFinished = false;
      m_Geometry = nullptr;
      ClearQueues();
    }
    MString GetConfigValue() const { return m_ConfigValue; }
    unsigned int GetInitializeCalls() const { return m_InitializeCalls; }
    unsigned int GetFinalizeCalls() const { return m_FinalizeCalls; }
    unsigned int GetAnalyzeCalls() const { return m_AnalyzeCalls; }
    MDGeometryQuest* GetGeometryPointer() const { return m_Geometry; }

  private:
    MString m_ConfigValue;
    int m_MaxGeneratedEvents;
    bool m_InitializeResult;
    bool m_MarkNotOKDuringAnalysis;
    bool m_RejectConfiguration;
    MSupervisor* m_SupervisorToInterrupt;
    unsigned int m_InitializeCalls;
    unsigned int m_FinalizeCalls;
    unsigned int m_AnalyzeCalls;
  };

  //! Initialize the singleton and representative available modules
  bool PrepareSupervisor();
  //! Test singleton defaults and available-module lookup
  bool TestDefaultsAndAvailableModules();
  //! Test sequence editing, validation side effects, and possible-module selection
  bool TestModuleSequence();
  //! Test XML save, load, and one-field configuration updates
  bool TestConfiguration();
  //! Test interrupt state and analysis failure paths that do not enter the UI
  bool TestInterruptsAndAnalysisFailure();
  //! Test successful geometry loading and a bounded single-threaded analysis
  bool TestSuccessfulAnalysis();
  //! Test additional deterministic analysis edge cases after geometry loading
  bool TestAdditionalAnalysisBranches();
  //! Test a bounded pipeline with the multi-threaded analysis loop enabled
  bool TestMultiThreadedAnalysis();
  //! Test the user-interface settings and the headless LaunchUI path
  bool TestUserInterfaceSettings();
  //! Test the rules the GUI uses to offer modules for a sequence position
  bool TestSequenceBuildingRules();
  //! Return true if the module is among those offered after the given sequence
  bool IsOffered(vector<MModule*>& Previous, MModule* Candidate);
  //! Return the shared geometry fixture path used for supervisor integration paths
  MString GetGeometryFixtureName() const;
  //! Reset all representative module probes before a stateful analysis test
  void ResetProbeStates();

  MSupervisor* m_Supervisor = nullptr;
  SupervisorModuleProbe* m_Loader = nullptr;
  SupervisorModuleProbe* m_Filter = nullptr;
  SupervisorModuleProbe* m_Saver = nullptr;
  SupervisorModuleProbe* m_SoftConsumer = nullptr;
  SupervisorModuleProbe* m_SoftProvider = nullptr;

  static const uint64_t c_Loaded = 0x1;
  static const uint64_t c_Filtered = 0x2;
  static const uint64_t c_Saved = 0x4;
  static const uint64_t c_SoftConsumer = 0x8;
  static const uint64_t c_SoftProvider = 0x10;
};


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::Run()
{
  bool Passed = true;

  Passed = PrepareSupervisor() && Passed;
  Passed = TestDefaultsAndAvailableModules() && Passed;
  Passed = TestModuleSequence() && Passed;
  Passed = TestConfiguration() && Passed;
  Passed = TestInterruptsAndAnalysisFailure() && Passed;
  Passed = TestSuccessfulAnalysis() && Passed;
  Passed = TestAdditionalAnalysisBranches() && Passed;
  Passed = TestMultiThreadedAnalysis() && Passed;
  Passed = TestUserInterfaceSettings() && Passed;
  Passed = TestSequenceBuildingRules() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTSupervisor::GetGeometryFixtureName() const
{
  return MString(gSystem->pwd()) + "/resource/examples/geomega/special/Max.geo.setup";
}


////////////////////////////////////////////////////////////////////////////////


void UTSupervisor::ResetProbeStates()
{
  m_Loader->ResetProbeState();
  m_Filter->ResetProbeState();
  m_Saver->ResetProbeState();
  m_SoftConsumer->ResetProbeState();
  m_SoftProvider->ResetProbeState();
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::PrepareSupervisor()
{
  bool Passed = true;

  m_Supervisor = MSupervisor::GetSupervisor();
  m_Supervisor->Clear();
  m_Supervisor->UseUI(false);
  m_Supervisor->UseMultiThreading(false);
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_default.cfg"));

  m_Loader = new SupervisorModuleProbe("UT Supervisor Loader", "XmlTagUTSupervisorLoader", c_Loaded, true);
  m_Filter = new SupervisorModuleProbe("UT Supervisor Filter", "XmlTagUTSupervisorFilter", c_Filtered);
  m_Filter->AddHardPreceedingType(c_Loaded);
  m_Saver = new SupervisorModuleProbe("UT Supervisor Saver", "XmlTagUTSupervisorSaver", c_Saved);
  m_Saver->AddHardPreceedingType(c_Filtered);
  m_SoftConsumer = new SupervisorModuleProbe("UT Supervisor Soft Consumer", "XmlTagUTSupervisorSoftConsumer", c_SoftConsumer);
  m_SoftConsumer->AddSoftPreceedingType(c_SoftProvider);
  m_SoftProvider = new SupervisorModuleProbe("UT Supervisor Soft Provider", "XmlTagUTSupervisorSoftProvider", c_SoftProvider);

  m_Supervisor->AddAvailableModule(m_Loader);
  m_Supervisor->AddAvailableModule(m_Filter);
  m_Supervisor->AddAvailableModule(m_Saver);
  m_Supervisor->AddAvailableModule(m_SoftConsumer);
  m_Supervisor->AddAvailableModule(m_SoftProvider);

  Passed = EvaluateTrue("GetSupervisor()", "singleton setup", "A representative supervisor singleton is available for testing", m_Supervisor != nullptr) && Passed;
  Passed = Evaluate("GetNModules()", "after Clear", "Clear removes any previously configured module sequence", m_Supervisor->GetNModules(), 0U) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "default", "The supervisor is not analyzing after setup", m_Supervisor->IsAnalysisisRunning()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestDefaultsAndAvailableModules()
{
  bool Passed = true;

  Passed = EvaluateTrue("GetSupervisor()", "singleton identity", "GetSupervisor returns the same singleton instance on repeated calls", MSupervisor::GetSupervisor() == m_Supervisor) && Passed;
  Passed = Evaluate("GetNAvailableModules()", "registered probes", "All representative available modules are registered", m_Supervisor->GetNAvailableModules(), 5U) && Passed;
  Passed = EvaluateTrue("GetAvailableModule()", "first module", "Available modules can be retrieved by insertion index", m_Supervisor->GetAvailableModule(0) == m_Loader) && Passed;
  Passed = EvaluateTrue("GetAvailableModule()", "out of range", "Out-of-range available-module lookup returns nullptr", m_Supervisor->GetAvailableModule(999) == nullptr) && Passed;
  Passed = EvaluateTrue("GetAvailableModuleByName()", "known name", "Available modules can be looked up by representative name", m_Supervisor->GetAvailableModuleByName("UT Supervisor Filter") == m_Filter) && Passed;
  Passed = EvaluateTrue("GetAvailableModuleByName()", "missing name", "Unknown module names return nullptr", m_Supervisor->GetAvailableModuleByName("Missing supervisor module") == nullptr) && Passed;
  Passed = EvaluateTrue("GetAvailableModuleByXmlTag()", "known XML tag", "Available modules can be looked up by XML tag", m_Supervisor->GetAvailableModuleByXmlTag("XmlTagUTSupervisorSaver") == m_Saver) && Passed;
  Passed = EvaluateTrue("GetAvailableModuleByXmlTag()", "missing XML tag", "Unknown XML tags return nullptr", m_Supervisor->GetAvailableModuleByXmlTag("XmlTagMissingSupervisor") == nullptr) && Passed;

  m_Supervisor->SetGeometryFileName("representative.geo.setup");
  Passed = Evaluate("SetGeometryFileName()/GetGeometryFileName()", "representative path", "The configured geometry file name is stored", m_Supervisor->GetGeometryFileName(), MString("representative.geo.setup")) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "before load", "No geometry object is present before a successful geometry load", m_Supervisor->GetGeometry() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestModuleSequence()
{
  bool Passed = true;

  m_Supervisor->Clear();

  Passed = EvaluateTrue("SetModule()", "append loader", "Appending a valid first module reports success and keeps the sequence", m_Supervisor->SetModule(m_Loader, 0)) && Passed;
  Passed = Evaluate("GetNModules()", "loader only", "The loader is stored as the first sequence module", m_Supervisor->GetNModules(), 1U) && Passed;
  Passed = EvaluateTrue("GetModule()", "loader", "The first configured module can be retrieved", m_Supervisor->GetModule(0) == m_Loader) && Passed;
  Passed = EvaluateTrue("GetModule()", "out of range", "Out-of-range configured-module lookup returns nullptr", m_Supervisor->GetModule(99) == nullptr) && Passed;

  vector<MModule*> PossibleAfterLoader = m_Supervisor->ReturnPossibleVolumes(1);
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "filter after loader", "Modules whose hard predecessor is already present are offered", find(PossibleAfterLoader.begin(), PossibleAfterLoader.end(), m_Filter) != PossibleAfterLoader.end()) && Passed;
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "loader excluded", "The existing module instance is not offered again", find(PossibleAfterLoader.begin(), PossibleAfterLoader.end(), m_Loader) == PossibleAfterLoader.end()) && Passed;
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "saver blocked", "Modules with missing hard predecessors are not offered", find(PossibleAfterLoader.begin(), PossibleAfterLoader.end(), m_Saver) == PossibleAfterLoader.end()) && Passed;

  Passed = EvaluateTrue("SetModule()", "append filter", "Appending a valid second module reports success and keeps the sequence", m_Supervisor->SetModule(m_Filter, 1)) && Passed;
  Passed = EvaluateTrue("SetModule()", "append saver", "Appending a valid third module reports success and keeps the sequence", m_Supervisor->SetModule(m_Saver, 2)) && Passed;
  Passed = Evaluate("GetNModules()", "valid sequence", "The valid loader/filter/saver sequence is retained", m_Supervisor->GetNModules(), 3U) && Passed;

  // Silence only the noisy call itself -- an Evaluate* made while the MEGAlib streams are
  // disabled would hide its own failure message, which mout carries.
  DisableDefaultStreams();
  const bool DuplicateResult = m_Supervisor->SetModule(m_Loader, 3);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "duplicate module", "Adding a duplicate module triggers validation trimming", DuplicateResult) && Passed;
  Passed = Evaluate("GetNModules()", "duplicate trimmed", "Validation removes the duplicate module and later modules", m_Supervisor->GetNModules(), 3U) && Passed;

  DisableDefaultStreams();
  const bool MissingRequirementResult = m_Supervisor->SetModule(m_Saver, 1);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "missing hard predecessor", "Replacing a module with one whose hard predecessor is missing triggers validation trimming", MissingRequirementResult) && Passed;
  Passed = Evaluate("GetNModules()", "missing hard predecessor trimmed", "Validation removes the first module with an unmet hard predecessor and later modules", m_Supervisor->GetNModules(), 1U) && Passed;
  Passed = EvaluateTrue("GetModule()", "remaining loader", "The valid prefix remains after trimming invalid successors", m_Supervisor->GetModule(0) == m_Loader) && Passed;

  vector<MModule*> Previous;
  Previous.push_back(m_SoftConsumer);
  vector<MModule*> PossibleAfterSoftConsumer = m_Supervisor->ReturnPossibleVolumes(Previous);
  Passed = EvaluateTrue("ReturnPossibleVolumes(vector)", "soft predecessor blocks provider", "A module is not offered after a module that softly requires its type as a predecessor", find(PossibleAfterSoftConsumer.begin(), PossibleAfterSoftConsumer.end(), m_SoftProvider) == PossibleAfterSoftConsumer.end()) && Passed;

  DisableDefaultStreams();
  const bool RemoveResult = m_Supervisor->RemoveModule(9);
  EnableDefaultStreams();
  Passed = EvaluateFalse("RemoveModule()", "out of range", "Removing an out-of-range module reports validation failure and leaves the sequence unchanged", RemoveResult) && Passed;
  Passed = Evaluate("GetNModules()", "after out-of-range remove", "The module sequence is unchanged after an out-of-range remove", m_Supervisor->GetNModules(), 1U) && Passed;

  Passed = EvaluateTrue("RemoveModule()", "existing loader", "Removing an existing module reports success and clears it from the sequence", m_Supervisor->RemoveModule(0)) && Passed;
  Passed = Evaluate("GetNModules()", "empty after remove", "The sequence is empty after removing its only module", m_Supervisor->GetNModules(), 0U) && Passed;

  DisableDefaultStreams();
  const bool GapResult = m_Supervisor->SetModule(m_Loader, 2);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "gap index", "Adding beyond the next sequence position is rejected", GapResult) && Passed;
  Passed = Evaluate("GetNModules()", "after gap index", "The sequence remains unchanged after a rejected gap insertion", m_Supervisor->GetNModules(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestConfiguration()
{
  bool Passed = true;

  m_Supervisor->Clear();
  m_Loader->SetConfigValue("loader-before-save");
  m_Filter->SetConfigValue("filter-before-save");
  m_Supervisor->SetGeometryFileName("representative-before-save.geo");
  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);

  const MString SaveFile = GetTemporaryFileName("supervisor_save.cfg");
  RemoveTemporaryFile(SaveFile);
  Passed = EvaluateTrue("Save()", "representative configuration", "The supervisor writes a representative XML configuration", m_Supervisor->Save(SaveFile)) && Passed;

  MString SavedContent = ReadTextFile(SaveFile);
  Passed = EvaluateTrue("Save()", "module sequence XML", "The saved configuration contains the selected module XML tags", SavedContent.Contains("XmlTagUTSupervisorLoader") && SavedContent.Contains("XmlTagUTSupervisorFilter")) && Passed;
  Passed = EvaluateTrue("Save()", "geometry XML", "The saved configuration contains the configured geometry file name", SavedContent.Contains("representative-before-save.geo")) && Passed;
  Passed = EvaluateTrue("Save()", "module options XML", "The saved configuration contains module option values for available modules", SavedContent.Contains("loader-before-save") && SavedContent.Contains("filter-before-save")) && Passed;

  m_Supervisor->Clear();
  m_Loader->SetConfigValue("loader-after-clear");
  m_Filter->SetConfigValue("filter-after-clear");
  Passed = EvaluateTrue("Load()", "saved configuration", "The supervisor loads a previously saved representative configuration", m_Supervisor->Load(SaveFile)) && Passed;
  Passed = Evaluate("GetNModules()", "after load", "Loading restores the saved module sequence", m_Supervisor->GetNModules(), 2U) && Passed;
  Passed = EvaluateTrue("GetModule()", "loaded first module", "The first loaded sequence item resolves by XML tag", m_Supervisor->GetModule(0) == m_Loader) && Passed;
  Passed = EvaluateTrue("GetModule()", "loaded second module", "The second loaded sequence item resolves by XML tag", m_Supervisor->GetModule(1) == m_Filter) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "after load", "Loading restores the saved geometry file name", m_Supervisor->GetGeometryFileName(), MString("representative-before-save.geo")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "loader options", "Loading dispatches module options to available modules", m_Loader->GetConfigValue(), MString("loader-before-save")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "filter options", "Loading dispatches module options to all matching available modules", m_Filter->GetConfigValue(), MString("filter-before-save")) && Passed;

  const MString NameFallbackFile = GetTemporaryFileName("supervisor_name_fallback.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "old module-name configuration", "A representative old-style module-name configuration can be written",
                        WriteTextFile(NameFallbackFile,
                                      "<NuclearizerData>\n"
                                      "  <Version>2</Version>\n"
                                      "  <ModuleSequence>\n"
                                      "    <ModuleSequenceItem>UT Supervisor Loader</ModuleSequenceItem>\n"
                                      "  </ModuleSequence>\n"
                                      "  <GeometryFileName>name-fallback.geo</GeometryFileName>\n"
                                      "  <ModuleOptions>\n"
                                      "    <XmlTagUTSupervisorLoader>\n"
                                      "      <ConfigValue>name-fallback-config</ConfigValue>\n"
                                      "    </XmlTagUTSupervisorLoader>\n"
                                      "  </ModuleOptions>\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool VersionResult = m_Supervisor->Load(NameFallbackFile);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Load()", "old module-name configuration", "Loading accepts old configurations that store module names instead of XML tags", VersionResult) && Passed;
  Passed = Evaluate("GetNModules()", "old module-name configuration", "The old-style module-name configuration restores one selected module", m_Supervisor->GetNModules(), 1U) && Passed;
  Passed = EvaluateTrue("GetModule()", "old module-name configuration", "Old-style module-name sequence items resolve through the name fallback", m_Supervisor->GetModule(0) == m_Loader) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "old module-name configuration", "The version-warning configuration still loads its geometry field", m_Supervisor->GetGeometryFileName(), MString("name-fallback.geo")) && Passed;
  Passed = Evaluate("GetConfigValue()", "old module-name configuration", "The version-warning configuration still loads module options", m_Loader->GetConfigValue(), MString("name-fallback-config")) && Passed;

  Passed = EvaluateTrue("ChangeConfiguration()", "geometry field", "Changing a top-level XML field updates the in-memory configuration", m_Supervisor->ChangeConfiguration("GeometryFileName=changed-by-field.geo")) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "after ChangeConfiguration", "The changed geometry field is read back into the supervisor", m_Supervisor->GetGeometryFileName(), MString("changed-by-field.geo")) && Passed;
  Passed = EvaluateTrue("ChangeConfiguration()", "module option field", "Changing a nested module option updates the matching available module", m_Supervisor->ChangeConfiguration("ModuleOptions.XmlTagUTSupervisorLoader.ConfigValue=changed-loader-option")) && Passed;
  Passed = Evaluate("GetConfigValue()", "after ChangeConfiguration", "The changed module option is read back into the module", m_Loader->GetConfigValue(), MString("changed-loader-option")) && Passed;

  DisableDefaultStreams();
  const bool MissingFieldResult = m_Supervisor->ChangeConfiguration("ModuleOptions.XmlTagMissing.ConfigValue=unused");
  EnableDefaultStreams();
  Passed = EvaluateFalse("ChangeConfiguration()", "missing field", "Changing an unknown XML field fails without modifying existing configuration", MissingFieldResult) && Passed;
  Passed = Evaluate("GetConfigValue()", "after failed ChangeConfiguration", "A failed configuration change leaves previous values in place", m_Loader->GetConfigValue(), MString("changed-loader-option")) && Passed;

  const MString MissingFile = GetTemporaryFileName("missing_supervisor.cfg");
  RemoveTemporaryFile(MissingFile);
  // PrepareSupervisor() set the default configuration file name to this, which differs from the
  // file actually requested below, so the message can be checked for naming the right one
  const MString DefaultConfigurationFile = GetTemporaryFileName("supervisor_default.cfg");
  Passed = EvaluateFalse("GetTemporaryFileName()", "default vs. requested file", "The default configuration file name differs from the one about to be requested", DefaultConfigurationFile == MissingFile) && Passed;
  ostringstream MissingLoadOutput;
  streambuf* MissingLoadOriginal = cout.rdbuf(MissingLoadOutput.rdbuf());
  const bool MissingLoadResult = m_Supervisor->Load(MissingFile);
  cout.rdbuf(MissingLoadOriginal);
  Passed = EvaluateFalse("Load()", "missing configuration", "Loading a missing configuration file reports failure", MissingLoadResult) && Passed;
  Passed = Evaluate("GetNModules()", "after missing configuration", "A missing configuration load preserves the current module sequence instead of wiping it", m_Supervisor->GetNModules(), 1U) && Passed;
  Passed = EvaluateTrue("Load()", "missing configuration message", "The missing-file message names the file which was actually requested, not the unrelated default configuration file name", MissingLoadOutput.str().find(MissingFile.Data()) != string::npos) && Passed;
  Passed = EvaluateFalse("Load()", "missing configuration message", "The missing-file message does not misname the default configuration file instead", MissingLoadOutput.str().find(DefaultConfigurationFile.Data()) != string::npos) && Passed;

  // A file which cannot even be parsed as XML must fail the same way a missing file does: reported,
  // and without touching whatever configuration was already in memory
  m_Supervisor->Clear();
  m_Supervisor->SetModule(m_Loader, 0);
  const MString MalformedFile = GetTemporaryFileName("malformed_supervisor.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "malformed configuration", "A malformed XML configuration file can be written",
                        WriteTextFile(MalformedFile,
                                      "<NuclearizerData>\n"
                                      "  <!-- an unterminated comment\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool MalformedLoadResult = m_Supervisor->Load(MalformedFile);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Load()", "malformed configuration", "Loading a file which cannot be parsed as XML reports failure", MalformedLoadResult) && Passed;
  Passed = Evaluate("GetNModules()", "after malformed configuration", "A malformed configuration load preserves the current module sequence instead of wiping it", m_Supervisor->GetNModules(), 1U) && Passed;
  Passed = EvaluateTrue("GetModule()", "after malformed configuration", "The preserved sequence still resolves to the same module instance", m_Supervisor->GetModule(0) == m_Loader) && Passed;
  RemoveTemporaryFile(MalformedFile);

  // An unresolvable entry is dropped; the remainder (loader/filter, calibration gone) can be
  // valid on its own, so GetNValidModules() alone can't detect this
  const MString IncompleteFile = GetTemporaryFileName("incomplete_supervisor.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "incomplete configuration", "A configuration referencing an unknown module can be written",
                        WriteTextFile(IncompleteFile,
                                      "<NuclearizerData>\n"
                                      "  <Version>1</Version>\n"
                                      "  <ModuleSequence>\n"
                                      "    <ModuleSequenceItem>XmlTagUTSupervisorLoader</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagDoesNotExist</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTSupervisorFilter</ModuleSequenceItem>\n"
                                      "  </ModuleSequence>\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool IncompleteLoadResult = m_Supervisor->Load(IncompleteFile);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Load()", "incomplete configuration", "Loading a configuration with an unresolvable module reports failure", IncompleteLoadResult) && Passed;
  Passed = Evaluate("GetNModules()", "incomplete configuration", "The unresolvable entry is left out, keeping the two modules which could be resolved", m_Supervisor->GetNModules(), 2U) && Passed;
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "incomplete configuration", "The supervisor remembers that the loaded configuration is not what was stored", m_Supervisor->IsConfigurationIncomplete()) && Passed;
  Passed = Evaluate("GetNValidModules()", "incomplete configuration", "The remaining loader/filter sequence is valid entirely on its own", m_Supervisor->GetNValidModules(), m_Supervisor->GetNModules()) && Passed;

  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_incomplete_analyze.cfg"));
  DisableDefaultStreams();
  const bool AnalyzedIncomplete = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "incomplete configuration", "A valid but incomplete sequence is refused even though every remaining module is individually fine", AnalyzedIncomplete) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "incomplete configuration", "The running flag is reset after the refusal", m_Supervisor->IsAnalysisisRunning()) && Passed;

  // ChangeConfiguration() rebuilds from modules already in memory, so it can't see the dropped
  // entry -- an unrelated field edit must not silently clear the incompleteness
  Passed = EvaluateTrue("ChangeConfiguration()", "unrelated field, incomplete configuration", "Changing an unrelated field still succeeds", m_Supervisor->ChangeConfiguration("GeometryFileName=changed-while-incomplete.geo")) && Passed;
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "after unrelated field change", "An unrelated field change does not clear the incompleteness recorded by the earlier load", m_Supervisor->IsConfigurationIncomplete()) && Passed;

  // Explicitly editing the sequence through the module editor is how the user is asked to correct
  // an incomplete configuration, so that edit has to be able to clear the flag again
  Passed = EvaluateTrue("SetModule()", "repair incomplete configuration", "Appending a valid module through the editor is accepted", m_Supervisor->SetModule(m_Saver, 2)) && Passed;
  Passed = EvaluateFalse("IsConfigurationIncomplete()", "after SetModule", "An explicit edit through the module editor clears the incompleteness flag", m_Supervisor->IsConfigurationIncomplete()) && Passed;

  DisableDefaultStreams();
  m_Supervisor->Load(IncompleteFile);
  EnableDefaultStreams();
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "reloaded incomplete configuration", "Reloading the same incomplete configuration sets the flag again", m_Supervisor->IsConfigurationIncomplete()) && Passed;
  Passed = EvaluateTrue("RemoveModule()", "repair incomplete configuration", "Removing a module through the editor is accepted", m_Supervisor->RemoveModule(1)) && Passed;
  Passed = EvaluateFalse("IsConfigurationIncomplete()", "after RemoveModule", "An explicit edit through the module editor clears the incompleteness flag", m_Supervisor->IsConfigurationIncomplete()) && Passed;

  // A rejected edit hasn't repaired anything -- clearing the protection regardless of outcome
  // would let RemoveModule(99) or similar make an incomplete sequence runnable unfixed
  DisableDefaultStreams();
  m_Supervisor->Load(IncompleteFile);
  EnableDefaultStreams();
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "reloaded incomplete configuration again", "Reloading the same incomplete configuration sets the flag again", m_Supervisor->IsConfigurationIncomplete()) && Passed;
  DisableDefaultStreams();
  const bool OutOfRangeRemove = m_Supervisor->RemoveModule(99);
  EnableDefaultStreams();
  Passed = EvaluateFalse("RemoveModule()", "out of range on incomplete configuration", "Removing at an invalid position reports failure", OutOfRangeRemove) && Passed;
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "after failed RemoveModule", "A rejected edit has not repaired anything, so the incompleteness protection must remain", m_Supervisor->IsConfigurationIncomplete()) && Passed;

  // Replacing a module with the identical instance already at that position changes nothing, even
  // though the call itself reports success, so it must not clear the protection either
  const bool NoOpSet = m_Supervisor->SetModule(m_Supervisor->GetModule(0), 0);
  Passed = EvaluateTrue("SetModule()", "no-op replacement on incomplete configuration", "Replacing a module with the identical instance already there reports success", NoOpSet) && Passed;
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "after no-op SetModule", "A no-op edit has not repaired anything, so the incompleteness protection must remain", m_Supervisor->IsConfigurationIncomplete()) && Passed;

  // Now genuinely repair the sequence half, isolating the module-options half for the next check
  Passed = EvaluateTrue("RemoveModule()", "repair after a rejected attempt", "A genuinely successful edit still clears the flag after an earlier rejected attempt", m_Supervisor->RemoveModule(1)) && Passed;
  Passed = EvaluateFalse("IsConfigurationIncomplete()", "sequence repaired", "The sequence half of the incompleteness is now clean", m_Supervisor->IsConfigurationIncomplete()) && Passed;

  // A sequence edit only repairs the missing-module half -- it can't fix a module which failed
  // to restore its own options
  m_SoftProvider->SetRejectConfiguration(true);
  DisableDefaultStreams();
  m_Supervisor->ChangeConfiguration("ModuleOptions.XmlTagUTSupervisorSoftProvider.ConfigValue=still-rejected");
  EnableDefaultStreams();
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "module options rejected", "The module-options half of the incompleteness is now set", m_Supervisor->IsConfigurationIncomplete()) && Passed;
  Passed = EvaluateTrue("SetModule()", "sequence edit cannot repair module options", "A valid, unrelated sequence edit is accepted on its own terms", m_Supervisor->SetModule(m_Filter, 1)) && Passed;
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "after unrelated sequence edit", "A sequence edit cannot clear an incompleteness caused by a module rejecting its own options", m_Supervisor->IsConfigurationIncomplete()) && Passed;
  m_SoftProvider->SetRejectConfiguration(false);

  // A module which rejects the field it was just given has to make ChangeConfiguration() itself
  // fail, not just log an error internally and report success regardless
  m_SoftProvider->SetRejectConfiguration(true);
  DisableDefaultStreams();
  const bool RejectedFieldResult = m_Supervisor->ChangeConfiguration("ModuleOptions.XmlTagUTSupervisorSoftProvider.ConfigValue=rejected");
  EnableDefaultStreams();
  Passed = EvaluateFalse("ChangeConfiguration()", "module rejects its configuration", "ChangeConfiguration reports failure when a module's own reader rejects the change", RejectedFieldResult) && Passed;
  Passed = EvaluateTrue("IsConfigurationIncomplete()", "module rejects its configuration", "A module rejecting its own configuration during a field change is itself recorded as an incompleteness", m_Supervisor->IsConfigurationIncomplete()) && Passed;
  m_SoftProvider->SetRejectConfiguration(false);

  RemoveTemporaryFile(IncompleteFile);

  // A stored sequence can be invalid outright (order breaks a hard requirement) even though every
  // module resolves. Load() only reports completeness, not validity -- caught separately, before
  // Analyze() runs it.
  const MString InvalidOrderFile = GetTemporaryFileName("invalid_order_supervisor.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "invalid-order configuration", "A configuration with a broken module order can be written",
                        WriteTextFile(InvalidOrderFile,
                                      "<NuclearizerData>\n"
                                      "  <Version>1</Version>\n"
                                      "  <ModuleSequence>\n"
                                      "    <ModuleSequenceItem>XmlTagUTSupervisorFilter</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTSupervisorLoader</ModuleSequenceItem>\n"
                                      "  </ModuleSequence>\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool InvalidOrderLoadResult = m_Supervisor->Load(InvalidOrderFile);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Load()", "invalid-order configuration", "Every entry resolves, so loading itself reports success", InvalidOrderLoadResult) && Passed;
  Passed = EvaluateFalse("IsConfigurationIncomplete()", "invalid-order configuration", "The configuration is complete -- it is invalid, which is a different thing", m_Supervisor->IsConfigurationIncomplete()) && Passed;
  DisableDefaultStreams();
  const unsigned int InvalidOrderValid = m_Supervisor->GetNValidModules();
  EnableDefaultStreams();
  Passed = Evaluate("GetNValidModules()", "invalid-order configuration", "The filter is first in this sequence and its hard predecessor requirement is unmet, so no module is valid", InvalidOrderValid, 0U) && Passed;

  DisableDefaultStreams();
  const bool AnalyzedInvalidOrder = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "invalid-order configuration", "A sequence loaded from disk with a broken order is refused, reached through Load() rather than through the module editor", AnalyzedInvalidOrder) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "invalid-order configuration", "The running flag is reset after the refusal", m_Supervisor->IsAnalysisisRunning()) && Passed;
  RemoveTemporaryFile(InvalidOrderFile);

  // A valid sequence with no start module must be rejected before Save() writes it -- otherwise
  // the refused configuration overwrites the last usable one
  const MString NoStartSaveFile = GetTemporaryFileName("supervisor_no_start_save.cfg");
  RemoveTemporaryFile(NoStartSaveFile);
  m_Supervisor->Clear();
  m_Supervisor->SetConfigurationFileName(NoStartSaveFile);
  m_Supervisor->SetModule(m_SoftProvider, 0);
  Passed = Evaluate("GetNValidModules()", "no start module", "The single module has no requirements of its own, so the sequence is valid -- only the missing start module is at fault", m_Supervisor->GetNValidModules(), 1U) && Passed;
  DisableDefaultStreams();
  const bool AnalyzedNoStart = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "no start module", "A valid sequence whose first module does not generate events is refused", AnalyzedNoStart) && Passed;
  Passed = EvaluateFalse("MFile::Exists()", "no start module", "The rejected sequence is not saved to disk before being refused", MFile::Exists(NoStartSaveFile)) && Passed;

  RemoveTemporaryFile(SaveFile);
  RemoveTemporaryFile(NameFallbackFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestInterruptsAndAnalysisFailure()
{
  bool Passed = true;

  // Geometry loading makes ROOT announce every TGeoManager it builds; keep warnings and errors.
  const int PreviousErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  m_Supervisor->Clear();
  m_Supervisor->UseUI(false);
  m_Supervisor->UseMultiThreading(false);
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_analyze.cfg"));

  m_Supervisor->SetSoftInterrupt();
  Passed = EvaluateTrue("SetSoftInterrupt()", "set", "Setting the soft interrupt records the requested interrupt state", m_Supervisor->GetSoftInterrupt()) && Passed;
  Passed = EvaluateFalse("GetHardInterrupt()", "soft only", "A soft interrupt alone does not imply a hard interrupt", m_Supervisor->GetHardInterrupt()) && Passed;
  m_Supervisor->SetHardInterrupt();
  Passed = EvaluateTrue("SetHardInterrupt()", "set", "Setting the hard interrupt also records the hard interrupt state", m_Supervisor->GetHardInterrupt()) && Passed;
  Passed = EvaluateTrue("GetSoftInterrupt()", "hard implies soft", "A hard interrupt implies a soft interrupt", m_Supervisor->GetSoftInterrupt()) && Passed;
  m_Supervisor->SetSoftInterrupt(false);
  Passed = EvaluateFalse("GetSoftInterrupt()", "clear soft", "Clearing the soft interrupt also clears the hard interrupt", m_Supervisor->GetSoftInterrupt()) && Passed;
  Passed = EvaluateFalse("GetHardInterrupt()", "clear soft", "Clearing the soft interrupt clears any hard interrupt", m_Supervisor->GetHardInterrupt()) && Passed;
  m_Supervisor->SetHardInterrupt(false);
  Passed = EvaluateFalse("GetHardInterrupt()", "clear hard", "Clearing the hard interrupt leaves it unset", m_Supervisor->GetHardInterrupt()) && Passed;

  const MString MissingGeometry = GetTemporaryFileName("missing_supervisor.geo");
  RemoveTemporaryFile(MissingGeometry);
  m_Supervisor->SetGeometryFileName(MissingGeometry);
  DisableDefaultStreams();
  const bool GeometryResult = m_Supervisor->LoadGeometry();
  EnableDefaultStreams();
  Passed = EvaluateFalse("LoadGeometry()", "missing geometry", "Loading a missing geometry file fails cleanly", GeometryResult) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "after failed LoadGeometry", "A failed geometry load leaves no geometry object behind", m_Supervisor->GetGeometry() == nullptr) && Passed;
  Passed = EvaluateTrue("SetGeometry()", "after failed LoadGeometry", "A failed geometry load does not assign geometry to modules", m_Loader->GetGeometryPointer() == nullptr && m_Filter->GetGeometryPointer() == nullptr) && Passed;

  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);
  DisableDefaultStreams();
  const bool AnalyzeResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "missing geometry", "Analysis fails before module initialization when the geometry cannot be loaded", AnalyzeResult) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after failed Analyze", "The running flag is reset after analysis fails during geometry loading", m_Supervisor->IsAnalysisisRunning()) && Passed;
  Passed = Evaluate("Initialize()", "after failed Analyze", "Modules are not initialized when analysis fails while loading geometry", m_Loader->GetInitializeCalls(), 0U) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "after failed Analyze", "Modules do not analyze events when geometry loading fails", m_Loader->GetAnalyzeCalls(), 0U) && Passed;
  Passed = EvaluateFalse("GetHardInterrupt()", "after normal failed Analyze", "A normal failed analysis leaves the hard interrupt cleared", m_Supervisor->GetHardInterrupt()) && Passed;
  m_Supervisor->View();
  Passed = EvaluateTrue("View()", "without expo viewer", "Viewing without an expo viewer is a callable no-op", true) && Passed;


  gErrorIgnoreLevel = PreviousErrorIgnoreLevel;
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestSuccessfulAnalysis()
{
  bool Passed = true;

  // Geometry loading makes ROOT announce every TGeoManager it builds; keep warnings and errors.
  const int PreviousErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  const MString GeometryFileName = GetGeometryFixtureName();
  Passed = EvaluateTrue("AccessPathName()", "geometry fixture", "The representative geometry fixture exists", gSystem->AccessPathName(GeometryFileName) == false) && Passed;

  m_Supervisor->Clear();
  m_Supervisor->UseUI(false);
  m_Supervisor->UseMultiThreading(false);
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_success.cfg"));
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);

  DisableDefaultStreams();
  const bool GeometryResult = m_Supervisor->LoadGeometry();
  EnableDefaultStreams();
  Passed = EvaluateTrue("LoadGeometry()", "valid geometry", "A representative geometry fixture loads successfully", GeometryResult) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "after valid LoadGeometry", "A successful geometry load stores the geometry object", m_Supervisor->GetGeometry() != nullptr) && Passed;
  Passed = EvaluateTrue("SetGeometry()", "after valid LoadGeometry", "A successful geometry load passes the same geometry pointer to configured modules", m_Loader->GetGeometryPointer() == m_Supervisor->GetGeometry() && m_Filter->GetGeometryPointer() == m_Supervisor->GetGeometry()) && Passed;

  // A second call which fails must not discard the still-valid geometry from the first one, nor
  // leave the modules holding a pointer to something which has since been freed
  MDGeometryQuest* FirstGeometry = m_Supervisor->GetGeometry();
  m_Supervisor->SetGeometryFileName(GetTemporaryFileName("supervisor_missing_geometry.geo.setup"));
  DisableDefaultStreams();
  const bool SecondGeometryResult = m_Supervisor->LoadGeometry();
  EnableDefaultStreams();
  Passed = EvaluateFalse("LoadGeometry()", "repeated call, second load fails", "A second geometry load with a nonexistent file fails", SecondGeometryResult) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "after failed repeated LoadGeometry", "The previous, still-valid geometry is kept rather than being discarded or leaked", m_Supervisor->GetGeometry() == FirstGeometry) && Passed;
  Passed = EvaluateTrue("SetGeometry()", "after failed repeated LoadGeometry", "Modules still point at the previous, still-valid geometry rather than a dangling pointer", m_Loader->GetGeometryPointer() == FirstGeometry && m_Filter->GetGeometryPointer() == FirstGeometry) && Passed;

  // A successful replacement deletes the old geometry -- a module removed from the sequence since
  // the previous load must still be updated, or it's left pointing at freed memory
  Passed = EvaluateTrue("RemoveModule()", "before second successful geometry load", "The filter is removed from the sequence ahead of the next load", m_Supervisor->RemoveModule(1)) && Passed;
  Passed = EvaluateTrue("SetGeometry()", "removed module before second load", "The removed module still points at the first geometry immediately after removal", m_Filter->GetGeometryPointer() == FirstGeometry) && Passed;
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  DisableDefaultStreams();
  const bool ThirdGeometryResult = m_Supervisor->LoadGeometry();
  EnableDefaultStreams();
  Passed = EvaluateTrue("LoadGeometry()", "second successful load after removal", "A second successful geometry load succeeds", ThirdGeometryResult) && Passed;
  Passed = EvaluateTrue("SetGeometry()", "removed module after second load", "A module removed from the sequence since the previous load is still updated to the new geometry, not left pointing at the one just freed", m_Filter->GetGeometryPointer() == m_Supervisor->GetGeometry()) && Passed;
  Passed = EvaluateTrue("SetGeometry()", "configured module after second load", "The module still in the sequence is updated to the new geometry as well", m_Loader->GetGeometryPointer() == m_Supervisor->GetGeometry()) && Passed;

  // Clear() deletes the geometry -- any module still pointing at it would dangle otherwise.
  // Checked before ResetProbeStates(), which would itself mask a regression here.
  m_Supervisor->Clear();
  Passed = EvaluateTrue("Clear()", "geometry pointer reset", "Clear() resets the geometry pointer on every available module, not only the ones still in the sequence", m_Loader->GetGeometryPointer() == nullptr && m_Filter->GetGeometryPointer() == nullptr) && Passed;

  ResetProbeStates();
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  DisableDefaultStreams();
  const bool EmptyAnalysisResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "no modules", "Analysis fails when no modules are configured, before geometry is even loaded", EmptyAnalysisResult) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after no-module Analyze", "The running flag is reset after the no-module analysis failure", m_Supervisor->IsAnalysisisRunning()) && Passed;

  m_Supervisor->Clear();
  ResetProbeStates();
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  m_Supervisor->SetModule(m_SoftConsumer, 0);
  DisableDefaultStreams();
  const bool NonStartResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "single non-start module", "Analysis rejects a one-module sequence whose first module is not a start module", NonStartResult) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after non-start Analyze", "The running flag is reset after the single non-start analysis failure", m_Supervisor->IsAnalysisisRunning()) && Passed;

  m_Supervisor->Clear();
  ResetProbeStates();
  m_Loader->SetMaxGeneratedEvents(1);
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);
  m_Supervisor->SetModule(m_Saver, 2);

  DisableDefaultStreams();
  const bool AnalysisResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Analyze()", "bounded single-threaded pipeline", "A bounded non-UI single-threaded module sequence analyzes one representative event successfully", AnalysisResult) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after successful Analyze", "The running flag is reset after successful analysis", m_Supervisor->IsAnalysisisRunning()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "start module calls", "The start module generates one event and one shutdown sentinel", m_Loader->GetAnalyzeCalls(), 2U) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "filter calls", "The downstream filter analyzes the one generated event", m_Filter->GetAnalyzeCalls(), 1U) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "saver calls", "The downstream saver analyzes the one generated event", m_Saver->GetAnalyzeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "start module", "Successful analysis finalizes the start module", m_Loader->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "filter module", "Successful analysis finalizes the filter module", m_Filter->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "saver module", "Successful analysis finalizes the saver module", m_Saver->GetFinalizeCalls(), 1U) && Passed;
  Passed = EvaluateFalse("GetHardInterrupt()", "after successful Analyze", "A successful normal analysis leaves the hard interrupt cleared", m_Supervisor->GetHardInterrupt()) && Passed;

  m_Loader->SetMaxGeneratedEvents(-1);
  m_Supervisor->Clear();


  gErrorIgnoreLevel = PreviousErrorIgnoreLevel;
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestAdditionalAnalysisBranches()
{
  bool Passed = true;

  // Geometry loading makes ROOT announce every TGeoManager it builds; keep warnings and errors.
  const int PreviousErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  const MString GeometryFileName = GetGeometryFixtureName();

  m_Supervisor->Clear();
  ResetProbeStates();
  m_Supervisor->UseUI(false);
  m_Supervisor->UseMultiThreading(false);
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_init_failure.cfg"));
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  m_Filter->SetInitializeResult(false);
  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);
  m_Supervisor->SetModule(m_Saver, 2);

  DisableDefaultStreams();
  const bool InitializationFailureResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "module initialization failure", "Analysis fails when a module initialization fails after geometry loading", InitializationFailureResult) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after initialization failure", "The running flag is reset after module initialization failure", m_Supervisor->IsAnalysisisRunning()) && Passed;
  Passed = Evaluate("Initialize()", "loader before failure", "Modules before the failing module are initialized", m_Loader->GetInitializeCalls(), 1U) && Passed;
  Passed = Evaluate("Initialize()", "failing filter", "The failing module initialization is attempted once", m_Filter->GetInitializeCalls(), 1U) && Passed;
  Passed = Evaluate("Initialize()", "saver after failure", "Modules after the failing module are not initialized", m_Saver->GetInitializeCalls(), 0U) && Passed;
  Passed = Evaluate("Finalize()", "loader after failure", "Already initialized modules are finalized after initialization failure", m_Loader->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "failing filter", "The failing module is also finalized by the current cleanup loop", m_Filter->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "saver after failure", "Modules after the failing module are not finalized", m_Saver->GetFinalizeCalls(), 0U) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "after initialization failure", "No events are analyzed when initialization fails", m_Loader->GetAnalyzeCalls(), 0U) && Passed;
  m_Supervisor->Clear();

  // A soft interrupt during a module's own Initialize() must be treated like a genuine failure --
  // later modules were never initialized either way. The probe simulates this by setting the
  // interrupt on the supervisor from inside its own Initialize() call.
  m_Supervisor->Clear();
  ResetProbeStates();
  m_Supervisor->UseUI(false);
  m_Supervisor->UseMultiThreading(false);
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_init_interrupt.cfg"));
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  m_Filter->SetSoftInterruptOnInitialize(m_Supervisor);
  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);
  m_Supervisor->SetModule(m_Saver, 2);

  DisableDefaultStreams();
  const bool InitializationInterruptResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "module initialization interrupted", "Analysis reports failure when a soft interrupt lands during a module's own initialization", InitializationInterruptResult) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after initialization interrupted", "The running flag is reset after an interrupted initialization", m_Supervisor->IsAnalysisisRunning()) && Passed;
  Passed = Evaluate("Initialize()", "loader before interrupt", "Modules before the interrupted module are initialized", m_Loader->GetInitializeCalls(), 1U) && Passed;
  Passed = Evaluate("Initialize()", "interrupted filter", "The interrupted module's initialization is attempted once", m_Filter->GetInitializeCalls(), 1U) && Passed;
  Passed = Evaluate("Initialize()", "saver after interrupt", "Modules after the interrupted module are not initialized -- this is the regression this test guards against", m_Saver->GetInitializeCalls(), 0U) && Passed;
  Passed = Evaluate("Finalize()", "loader after interrupt", "Already initialized modules are finalized after an interrupted initialization", m_Loader->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "interrupted filter", "The interrupted module is also finalized by the cleanup loop", m_Filter->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "saver after interrupt", "Modules after the interrupted module are not finalized", m_Saver->GetFinalizeCalls(), 0U) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "after initialization interrupted", "No events are analyzed when initialization is interrupted", m_Loader->GetAnalyzeCalls(), 0U) && Passed;
  m_Filter->SetSoftInterruptOnInitialize(nullptr);
  m_Supervisor->SetSoftInterrupt(false);
  m_Supervisor->Clear();

  m_Supervisor->Clear();
  ResetProbeStates();
  m_Loader->SetMaxGeneratedEvents(1);
  m_Filter->SetMarkNotOKDuringAnalysis(true);
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_not_ok.cfg"));
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);

  DisableDefaultStreams();
  const bool ModuleNotOKResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Analyze()", "module no longer OK", "Analysis performs an orderly shutdown when a module reports not OK during processing", ModuleNotOKResult) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after module no longer OK", "The running flag is reset after module-not-OK shutdown", m_Supervisor->IsAnalysisisRunning()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "loader before not OK", "The start module generates one event and one shutdown sentinel before module-not-OK shutdown", m_Loader->GetAnalyzeCalls(), 2U) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "filter not OK", "The downstream module analyzes the generated event before marking itself not OK", m_Filter->GetAnalyzeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "loader after not OK", "Module-not-OK shutdown finalizes the start module", m_Loader->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "filter after not OK", "Module-not-OK shutdown finalizes the downstream module", m_Filter->GetFinalizeCalls(), 1U) && Passed;
  m_Supervisor->Clear();

  m_Supervisor->Clear();
  ResetProbeStates();
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_test_run.cfg"));
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);

  // The success marker itself is asserted on below, so this output has to be captured rather than
  // discarded. Only the noisy call is wrapped -- an Evaluate* inside would report into the capture.
  ostringstream TestRunOutput;
  streambuf* OriginalCout = cout.rdbuf(TestRunOutput.rdbuf());
  const bool TestRunResult = m_Supervisor->Analyze(true);
  cout.rdbuf(OriginalCout);
  Passed = EvaluateTrue("Analyze()", "test run", "Test-run analysis initializes and finalizes modules without processing events", TestRunResult) && Passed;
  Passed = EvaluateTrue("Analyze()", "test-run message", "Test-run analysis emits the documented success marker", MString(TestRunOutput.str()).Contains(">>> TEST RUN SUCCESSFUL <<<")) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after test run", "The running flag is reset after a test run", m_Supervisor->IsAnalysisisRunning()) && Passed;
  Passed = EvaluateTrue("GetHardInterrupt()", "after test run", "Test-run mode leaves the hard interrupt set by the current implementation", m_Supervisor->GetHardInterrupt()) && Passed;
  Passed = Evaluate("Initialize()", "loader test run", "The start module is initialized during a test run", m_Loader->GetInitializeCalls(), 1U) && Passed;
  Passed = Evaluate("Initialize()", "filter test run", "The downstream module is initialized during a test run", m_Filter->GetInitializeCalls(), 1U) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "loader test run", "The start module does not analyze events during a test run", m_Loader->GetAnalyzeCalls(), 0U) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "filter test run", "The downstream module does not analyze events during a test run", m_Filter->GetAnalyzeCalls(), 0U) && Passed;
  Passed = Evaluate("Finalize()", "loader test run", "The start module is finalized during a test run", m_Loader->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "filter test run", "The downstream module is finalized during a test run", m_Filter->GetFinalizeCalls(), 1U) && Passed;
  m_Supervisor->SetHardInterrupt(false);
  m_Supervisor->Clear();

  const MString SaveDirectory = GetTemporaryDirectoryName("supervisor_save_directory");
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "save directory", "A temporary directory can stand in for an unwritable configuration-file path", PrepareTemporaryDirectory("supervisor_save_directory")) && Passed;
  Passed = EvaluateTrue("Save()", "directory path", "MSupervisor::Save currently returns true even when the XML document cannot be written to a directory path", m_Supervisor->Save(SaveDirectory)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "save directory", "The temporary save-directory fixture can be removed", RemoveTemporaryDirectory(SaveDirectory)) && Passed;


  gErrorIgnoreLevel = PreviousErrorIgnoreLevel;
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestMultiThreadedAnalysis()
{
  bool Passed = true;

  // Geometry loading makes ROOT announce every TGeoManager it builds; keep warnings and errors.
  const int PreviousErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  const int GeneratedEvents = 20;

  m_Supervisor->Clear();
  ResetProbeStates();
  m_Supervisor->UseUI(false);
  m_Supervisor->UseMultiThreading(true);
  m_Loader->SetAllowMultiThreading(true);
  m_Filter->SetAllowMultiThreading(true);
  m_Loader->SetMaxGeneratedEvents(GeneratedEvents);
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_multithreaded.cfg"));
  m_Supervisor->SetGeometryFileName(GetGeometryFixtureName());
  m_Supervisor->SetModule(m_Loader, 0);
  m_Supervisor->SetModule(m_Filter, 1);

  DisableDefaultStreams();
  const bool AnalysisResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();

  Passed = EvaluateTrue("Analyze()", "multi-threaded pipeline", "A bounded module sequence analyzes successfully with the multi-threaded loop enabled", AnalysisResult) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "after multi-threaded Analyze", "The running flag is reset after multi-threaded analysis", m_Supervisor->IsAnalysisisRunning()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "multi-threaded start module", "The start module generates every event plus one shutdown sentinel", m_Loader->GetAnalyzeCalls(), static_cast<unsigned int>(GeneratedEvents + 1)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "multi-threaded downstream module", "The downstream module analyzes every generated event", m_Filter->GetAnalyzeCalls(), static_cast<unsigned int>(GeneratedEvents)) && Passed;
  Passed = Evaluate("Finalize()", "multi-threaded start module", "Multi-threaded analysis finalizes the start module", m_Loader->GetFinalizeCalls(), 1U) && Passed;
  Passed = Evaluate("Finalize()", "multi-threaded downstream module", "Multi-threaded analysis finalizes the downstream module", m_Filter->GetFinalizeCalls(), 1U) && Passed;
  Passed = EvaluateFalse("IsMultiThreaded()", "after multi-threaded Analyze", "The module analysis threads have stopped once analysis returns", m_Loader->IsMultiThreaded() || m_Filter->IsMultiThreaded()) && Passed;

  m_Supervisor->UseMultiThreading(false);
  m_Loader->SetAllowMultiThreading(false);
  m_Filter->SetAllowMultiThreading(false);
  m_Supervisor->Clear();

  gErrorIgnoreLevel = PreviousErrorIgnoreLevel;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestUserInterfaceSettings()
{
  bool Passed = true;

  // The five UI setters have no getters and are not written to the configuration file, so the only
  // consumer of the stored values is LaunchUI(), which cannot build a window in a headless test.
  // Calling them here verifies they are linkable and that the values are accepted and survive being
  // handed to the launch path below -- the values themselves are not observable through the API.
  m_Supervisor->SetUIProgramName("UT Supervisor Program");
  m_Supervisor->SetUIPicturePath("");
  m_Supervisor->SetUISubTitle("UT Supervisor sub title");
  m_Supervisor->SetUILeadAuthor("UT Supervisor lead author");
  m_Supervisor->SetUICoAuthors("UT Supervisor co-authors");
  Passed = EvaluateTrue("SetUIProgramName()/SetUIPicturePath()/SetUISubTitle()/SetUILeadAuthor()/SetUICoAuthors()", "representative UI settings", "The user-interface settings are accepted without disturbing the supervisor state", m_Supervisor->IsAnalysisisRunning() == false) && Passed;

  // LaunchUI() refuses to build a window when ROOT has no graphics client, which is the case in a
  // headless test run. Guard on that, because on a machine with a display this would open a window.
  if (gClient == nullptr || gClient->GetRoot() == nullptr) {
    m_Supervisor->UseUI(true);
    DisableDefaultStreams();
    const bool LaunchResult = m_Supervisor->LaunchUI();
    EnableDefaultStreams();
    Passed = EvaluateFalse("LaunchUI()", "no graphics client", "Launching the user interface fails cleanly when ROOT has no graphics client", LaunchResult) && Passed;
  } else {
    mout<<"UTSupervisor: a ROOT graphics client is available, skipping the headless LaunchUI() check"<<endl;
  }
  m_Supervisor->UseUI(false);

  // Exit() is not exercised: it calls Terminate() -> gApplication->Terminate(0), which would end
  // the test process. Terminate()'s save guard is IsSequenceRunnable(), shared with Analyze(), so
  // it's covered indirectly through the Analyze() tests elsewhere in this file.

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::IsOffered(vector<MModule*>& Previous, MModule* Candidate)
{
  vector<MModule*> Offered = m_Supervisor->ReturnPossibleVolumes(Previous);
  return find(Offered.begin(), Offered.end(), Candidate) != Offered.end();
}


////////////////////////////////////////////////////////////////////////////////


bool UTSupervisor::TestSequenceBuildingRules()
{
  bool Passed = true;

  // Analyze() loads the geometry, and ROOT announces every TGeoManager it builds; keep warnings
  const int PreviousErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;

  // These probes are registered last, so that the available-module counts asserted by the earlier
  // sub-tests are not disturbed
  const uint64_t c_RuleProvider = 0x1000;
  const uint64_t c_RuleShared   = 0x2000;
  const uint64_t c_RuleOptional = 0x4000;

  SupervisorModuleProbe* Provider = new SupervisorModuleProbe("UT Rule Provider", "XmlTagUTRuleProvider", c_RuleProvider, true);
  SupervisorModuleProbe* Spacer = new SupervisorModuleProbe("UT Rule Spacer", "XmlTagUTRuleSpacer", 0x8000);
  SupervisorModuleProbe* Immediate = new SupervisorModuleProbe("UT Rule Immediate", "XmlTagUTRuleImmediate", 0x10000);
  Immediate->AddImmediatePreceedingType(c_RuleProvider);
  SupervisorModuleProbe* SoftUser = new SupervisorModuleProbe("UT Rule Soft User", "XmlTagUTRuleSoftUser", 0x20000);
  SoftUser->AddSoftPreceedingType(c_RuleOptional);
  SupervisorModuleProbe* SoftProviderLate = new SupervisorModuleProbe("UT Rule Soft Provider", "XmlTagUTRuleSoftProvider", c_RuleOptional);
  SoftProviderLate->SetProbeTypeExclusive(false);
  SupervisorModuleProbe* SoftProviderSecond = new SupervisorModuleProbe("UT Rule Soft Provider 2", "XmlTagUTRuleSoftProvider2", c_RuleOptional);
  SoftProviderSecond->SetProbeTypeExclusive(false);
  SupervisorModuleProbe* ExclusiveA = new SupervisorModuleProbe("UT Rule Exclusive", "XmlTagUTRuleExclusive", c_RuleShared);
  SupervisorModuleProbe* SharedB = new SupervisorModuleProbe("UT Rule Shared B", "XmlTagUTRuleSharedB", c_RuleShared);
  SharedB->SetProbeTypeExclusive(false);
  SupervisorModuleProbe* SharedC = new SupervisorModuleProbe("UT Rule Shared C", "XmlTagUTRuleSharedC", c_RuleShared);
  SharedC->SetProbeTypeExclusive(false);
  SupervisorModuleProbe* SecondStart = new SupervisorModuleProbe("UT Rule Second Start", "XmlTagUTRuleSecondStart", 0x2000000, true);

  m_Supervisor->AddAvailableModule(Provider);
  m_Supervisor->AddAvailableModule(Spacer);
  m_Supervisor->AddAvailableModule(Immediate);
  m_Supervisor->AddAvailableModule(SoftUser);
  m_Supervisor->AddAvailableModule(SoftProviderLate);
  m_Supervisor->AddAvailableModule(SoftProviderSecond);
  m_Supervisor->AddAvailableModule(ExclusiveA);
  m_Supervisor->AddAvailableModule(SharedB);
  m_Supervisor->AddAvailableModule(SharedC);
  m_Supervisor->AddAvailableModule(SecondStart);

  vector<MModule*> AfterProvider;
  AfterProvider.push_back(Provider);
  vector<MModule*> AfterProviderSpacer;
  AfterProviderSpacer.push_back(Provider);
  AfterProviderSpacer.push_back(Spacer);

  // An immediate requirement is only satisfied by the module directly before the new position
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "immediate requirement satisfied", "A module with an immediate requirement is offered directly after its provider", IsOffered(AfterProvider, Immediate)) && Passed;
  Passed = EvaluateFalse("ReturnPossibleVolumes()", "immediate requirement one position later", "A module with an immediate requirement is not offered once another module sits in between", IsOffered(AfterProviderSpacer, Immediate)) && Passed;
  vector<MModule*> Empty;
  Passed = EvaluateFalse("ReturnPossibleVolumes()", "immediate requirement at the start", "A module with an immediate requirement is not offered as the first module of a sequence", IsOffered(Empty, Immediate)) && Passed;

  // A soft requirement must not stop a module from being offered: its provider is optional
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "soft requirement, provider absent", "A module whose soft requirement is unmet is still offered, because its provider is optional", IsOffered(AfterProvider, SoftUser)) && Passed;

  // Type exclusivity has to give the same answer whichever module was added first
  vector<MModule*> AfterExclusive;
  AfterExclusive.push_back(ExclusiveA);
  vector<MModule*> AfterShared;
  AfterShared.push_back(SharedB);
  Passed = EvaluateFalse("ReturnPossibleVolumes()", "exclusive module placed first", "A second module of the same type is not offered after a type-exclusive module", IsOffered(AfterExclusive, SharedB)) && Passed;
  Passed = EvaluateFalse("ReturnPossibleVolumes()", "exclusive module placed second", "A type-exclusive module is not offered after another module of the same type", IsOffered(AfterShared, ExclusiveA)) && Passed;
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "both non-exclusive", "Two modules of the same type are offered together when neither is type exclusive", IsOffered(AfterShared, SharedC)) && Passed;

  // The sequence setters report whether the call did what was asked
  m_Supervisor->Clear();
  Passed = EvaluateTrue("SetModule()", "valid start module", "Setting a valid module at the next position reports success", m_Supervisor->SetModule(Provider, 0)) && Passed;
  Passed = EvaluateTrue("SetModule()", "valid immediate follower", "Setting a module whose immediate requirement is met reports success", m_Supervisor->SetModule(Immediate, 1)) && Passed;
  Passed = Evaluate("GetNModules()", "valid sequence", "Both modules remain in the sequence", m_Supervisor->GetNModules(), 2U) && Passed;

  // The same pair in the wrong order breaks the immediate requirement and is trimmed
  m_Supervisor->Clear();
  m_Supervisor->SetModule(Provider, 0);
  m_Supervisor->SetModule(Spacer, 1);
  DisableDefaultStreams();
  const bool ImmediateAfterSpacer = m_Supervisor->SetModule(Immediate, 2);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "immediate requirement broken", "Placing a module whose immediate predecessor is no longer directly before it reports failure", ImmediateAfterSpacer) && Passed;
  Passed = Evaluate("GetNModules()", "after broken immediate requirement", "The offending module is trimmed from the sequence", m_Supervisor->GetNModules(), 2U) && Passed;

  // A module whose immediate predecessor is required cannot be the first one in the sequence
  m_Supervisor->Clear();
  DisableDefaultStreams();
  const bool ImmediateAtStart = m_Supervisor->SetModule(Immediate, 0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "immediate requirement at the start", "A module with an immediate requirement cannot be the first module of a sequence", ImmediateAtStart) && Passed;
  Passed = Evaluate("GetNModules()", "immediate requirement at the start", "The rejected module does not stay in the sequence", m_Supervisor->GetNModules(), 0U) && Passed;

  // A rejected position has to be distinguishable from a rejected sequence: the sequence is still
  // valid afterwards, so the very next well-placed module is accepted
  m_Supervisor->Clear();
  DisableDefaultStreams();
  const bool GapPosition = m_Supervisor->SetModule(Provider, 3);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "position beyond the end", "Setting a module beyond the next free position reports failure", GapPosition) && Passed;
  Passed = Evaluate("GetNModules()", "position beyond the end", "A rejected position leaves the sequence untouched", m_Supervisor->GetNModules(), 0U) && Passed;
  Passed = EvaluateTrue("SetModule()", "after a rejected position", "The sequence is still usable after a rejected position", m_Supervisor->SetModule(Provider, 0)) && Passed;

  // Type exclusivity is part of what makes a sequence valid, not only a rule for what the GUI
  // offers, so placing a second module of an exclusive type is rejected and trimmed
  m_Supervisor->Clear();
  m_Supervisor->SetModule(ExclusiveA, 0);
  DisableDefaultStreams();
  const bool SecondOfSameType = m_Supervisor->SetModule(SharedB, 1);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "two modules of one exclusive type", "A second module carrying a type an earlier module provides exclusively is rejected", SecondOfSameType) && Passed;
  Passed = Evaluate("GetNModules()", "two modules of one exclusive type", "The rejected module is trimmed from the sequence", m_Supervisor->GetNModules(), 1U) && Passed;

  // Two modules which both allow their type to repeat stay in the sequence
  m_Supervisor->Clear();
  m_Supervisor->SetModule(SharedB, 0);
  const bool TwoNonExclusive = m_Supervisor->SetModule(SharedC, 1);
  Passed = EvaluateTrue("SetModule()", "two modules of one shared type", "Two modules of the same type are accepted when neither is type exclusive", TwoNonExclusive) && Passed;
  Passed = Evaluate("GetNModules()", "two modules of one shared type", "Both non-exclusive modules remain in the sequence", m_Supervisor->GetNModules(), 2U) && Passed;

  // A candidate whose soft requirement is provided further down would be erased the moment it is
  // placed, so it must not be offered for that position in the first place
  m_Supervisor->Clear();
  m_Supervisor->SetModule(Spacer, 0);
  m_Supervisor->SetModule(SoftProviderLate, 1);
  vector<MModule*> OfferedBeforeSoftProvider = m_Supervisor->ReturnPossibleVolumes(0);
  Passed = EvaluateTrue("ReturnPossibleVolumes(Position)", "soft provider held later", "A module whose soft requirement is provided further down the sequence is not offered for an earlier position", find(OfferedBeforeSoftProvider.begin(), OfferedBeforeSoftProvider.end(), SoftUser) == OfferedBeforeSoftProvider.end()) && Passed;

  m_Supervisor->Clear();
  m_Supervisor->SetModule(ExclusiveA, 0);
  Passed = Evaluate("GetNValidModules()", "valid single module", "A single module forms a valid sequence", m_Supervisor->GetNValidModules(), 1U) && Passed;

  // Nothing produces events unless the first module is a start module, so the analysis loop would
  // never reach its shutdown condition. That has to be refused for any sequence length.
  m_Supervisor->Clear();
  ResetProbeStates();
  m_Supervisor->UseUI(false);
  m_Supervisor->UseMultiThreading(false);
  m_Supervisor->SetConfigurationFileName(GetTemporaryFileName("supervisor_no_start.cfg"));
  m_Supervisor->SetGeometryFileName(GetGeometryFixtureName());
  m_Supervisor->SetModule(Spacer, 0);
  m_Supervisor->SetModule(SharedB, 1);
  Passed = Evaluate("GetNModules()", "two non-start modules", "Both non-start modules are in the sequence", m_Supervisor->GetNModules(), 2U) && Passed;
  DisableDefaultStreams();
  const bool AnalyzedWithoutStartModule = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "no start module", "A sequence whose first module does not generate events is refused, however many modules follow", AnalyzedWithoutStartModule) && Passed;
  Passed = EvaluateFalse("IsAnalysisisRunning()", "no start module", "The running flag is reset after the refusal", m_Supervisor->IsAnalysisisRunning()) && Passed;

  // A soft requirement is violated by a provider behind the module even when one also sits before it
  m_Supervisor->Clear();
  m_Supervisor->SetModule(SoftProviderLate, 0);
  m_Supervisor->SetModule(SoftUser, 1);
  Passed = Evaluate("GetNValidModules()", "soft provider before", "A soft requirement met by an earlier module is valid", m_Supervisor->GetNValidModules(), 2U) && Passed;
  m_Supervisor->Clear();
  m_Supervisor->SetModule(SoftProviderLate, 0);
  m_Supervisor->SetModule(SoftUser, 1);
  DisableDefaultStreams();
  const bool ProviderOnBothSides = m_Supervisor->SetModule(SoftProviderSecond, 2);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "soft provider on both sides", "A soft requirement is violated by a provider behind the module even when one also sits before it", ProviderOnBothSides) && Passed;
  Passed = Evaluate("GetNModules()", "soft provider on both sides", "The sequence is trimmed at the module whose soft requirement is violated", m_Supervisor->GetNModules(), 1U) && Passed;

  // SetModule() replaces rather than inserts, so replacing an existing position can leave modules
  // after it unable to fulfill their requirements -- they get trimmed. Applies to any unmet hard
  // requirement, not just immediate ones.
  m_Supervisor->Clear();
  m_Supervisor->SetModule(Provider, 0);
  m_Supervisor->SetModule(Immediate, 1);
  vector<MModule*> OfferedAtStart = m_Supervisor->ReturnPossibleVolumes(0);
  Passed = EvaluateTrue("ReturnPossibleVolumes(Position)", "successors ignored", "A module is offered for a position even when choosing it invalidates the modules after it", find(OfferedAtStart.begin(), OfferedAtStart.end(), Spacer) != OfferedAtStart.end()) && Passed;
  DisableDefaultStreams();
  const bool ReplacedDependency = m_Supervisor->SetModule(Spacer, 0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "replacing a required predecessor", "Replacing a module that a later module depends on reports that modules had to be eliminated", ReplacedDependency) && Passed;
  Passed = Evaluate("GetNModules()", "replacing a required predecessor", "The dependent module is trimmed from the sequence", m_Supervisor->GetNModules(), 1U) && Passed;
  Passed = EvaluateTrue("GetModule()", "replacing a required predecessor", "Only the replacement module remains", m_Supervisor->GetModule(0) == Spacer) && Passed;

  m_Supervisor->Clear();
  m_Supervisor->SetModule(Provider, 0);
  m_Supervisor->SetModule(Spacer, 1);
  DisableDefaultStreams();
  const bool RemovedOutOfRange = m_Supervisor->RemoveModule(99);
  EnableDefaultStreams();
  Passed = EvaluateFalse("RemoveModule()", "out of range", "Removing a module at an invalid position reports failure", RemovedOutOfRange) && Passed;
  Passed = Evaluate("GetNModules()", "after out-of-range remove", "An out-of-range remove leaves the sequence untouched", m_Supervisor->GetNModules(), 2U) && Passed;

  // A module can demand several immediate predecessors, but only one module can be directly before
  // it, so they can only all be met by a single module providing every one of those types
  const uint64_t c_RuleFirst  = 0x40000;
  const uint64_t c_RuleSecond = 0x80000;

  SupervisorModuleProbe* BothProvider = new SupervisorModuleProbe("UT Rule Both Provider", "XmlTagUTRuleBothProvider", c_RuleFirst);
  BothProvider->AddProvidedType(c_RuleSecond);
  SupervisorModuleProbe* FirstOnlyProvider = new SupervisorModuleProbe("UT Rule First Only", "XmlTagUTRuleFirstOnly", c_RuleFirst);
  SupervisorModuleProbe* TwoImmediate = new SupervisorModuleProbe("UT Rule Two Immediate", "XmlTagUTRuleTwoImmediate", 0x100000);
  TwoImmediate->AddImmediatePreceedingType(c_RuleFirst);
  TwoImmediate->AddImmediatePreceedingType(c_RuleSecond);

  m_Supervisor->AddAvailableModule(BothProvider);
  m_Supervisor->AddAvailableModule(FirstOnlyProvider);
  m_Supervisor->AddAvailableModule(TwoImmediate);

  vector<MModule*> AfterBothProvider;
  AfterBothProvider.push_back(BothProvider);
  vector<MModule*> AfterFirstOnly;
  AfterFirstOnly.push_back(FirstOnlyProvider);

  Passed = EvaluateTrue("ReturnPossibleVolumes()", "two immediate requirements, both provided", "A module with two immediate requirements is offered after a single module providing both types", IsOffered(AfterBothProvider, TwoImmediate)) && Passed;
  Passed = EvaluateFalse("ReturnPossibleVolumes()", "two immediate requirements, one provided", "A module with two immediate requirements is not offered when the module before it provides only one of the types", IsOffered(AfterFirstOnly, TwoImmediate)) && Passed;

  // Exclusivity has to trigger on any shared type, not only when the whole type list matches
  const uint64_t c_RuleOverlap = 0x200000;
  SupervisorModuleProbe* OverlapA = new SupervisorModuleProbe("UT Rule Overlap A", "XmlTagUTRuleOverlapA", 0x400000);
  OverlapA->AddProvidedType(c_RuleOverlap);
  SupervisorModuleProbe* OverlapB = new SupervisorModuleProbe("UT Rule Overlap B", "XmlTagUTRuleOverlapB", c_RuleOverlap);
  OverlapB->AddProvidedType(0x800000);
  SupervisorModuleProbe* NoOverlap = new SupervisorModuleProbe("UT Rule No Overlap", "XmlTagUTRuleNoOverlap", 0x1000000);

  m_Supervisor->AddAvailableModule(OverlapA);
  m_Supervisor->AddAvailableModule(OverlapB);
  m_Supervisor->AddAvailableModule(NoOverlap);

  vector<MModule*> AfterOverlapA;
  AfterOverlapA.push_back(OverlapA);
  Passed = EvaluateFalse("ReturnPossibleVolumes()", "one shared type of several", "A module sharing a single type with one already in the sequence is not offered", IsOffered(AfterOverlapA, OverlapB)) && Passed;
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "no shared type", "A module sharing no type with the sequence is offered", IsOffered(AfterOverlapA, NoOverlap)) && Passed;
  OverlapA->SetProbeTypeExclusive(false);
  OverlapB->SetProbeTypeExclusive(false);
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "one shared type, neither exclusive", "Modules sharing a type are offered together when neither is type exclusive", IsOffered(AfterOverlapA, OverlapB)) && Passed;
  OverlapA->SetProbeTypeExclusive(true);
  OverlapB->SetProbeTypeExclusive(true);

  // Type exclusivity has to hold for the whole sequence, not only for the part before the position:
  // when an earlier position is replaced there can be modules after it holding the same type
  m_Supervisor->Clear();
  m_Supervisor->SetModule(Spacer, 0);
  m_Supervisor->SetModule(ExclusiveA, 1);
  vector<MModule*> OfferedBeforeExclusive = m_Supervisor->ReturnPossibleVolumes(0);
  Passed = EvaluateTrue("ReturnPossibleVolumes(Position)", "exclusive type held later", "A module is not offered for an earlier position when a module after it already holds the same exclusive type", find(OfferedBeforeExclusive.begin(), OfferedBeforeExclusive.end(), SharedB) == OfferedBeforeExclusive.end()) && Passed;
  ExclusiveA->SetProbeTypeExclusive(false);
  vector<MModule*> OfferedNonExclusive = m_Supervisor->ReturnPossibleVolumes(0);
  Passed = EvaluateTrue("ReturnPossibleVolumes(Position)", "non-exclusive type held later", "A module is offered for an earlier position when neither it nor the module after it is type exclusive", find(OfferedNonExclusive.begin(), OfferedNonExclusive.end(), SharedB) != OfferedNonExclusive.end()) && Passed;
  ExclusiveA->SetProbeTypeExclusive(true);

  // The exclusivity check must also see a successor unreachable only because the CURRENT occupant
  // is itself invalid (Immediate at position 0 fails its own immediate requirement, unrelated to
  // SharedB/ExclusiveA's type) -- replacing Immediate with SharedB must be refused the same way as
  // the Spacer case above. Built from XML since SetModule(Immediate, 0) would be trimmed immediately.
  const MString ExclusivityInvalidPrefixFile = GetTemporaryFileName("supervisor_exclusivity_invalid_prefix.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "exclusivity behind an invalid first module", "A configuration with an already-invalid first module and an exclusive module right behind it can be written",
                        WriteTextFile(ExclusivityInvalidPrefixFile,
                                      "<NuclearizerData>\n"
                                      "  <Version>1</Version>\n"
                                      "  <ModuleSequence>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleImmediate</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleExclusive</ModuleSequenceItem>\n"
                                      "  </ModuleSequence>\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool ExclusivityInvalidPrefixLoadResult = m_Supervisor->Load(ExclusivityInvalidPrefixFile);
  const unsigned int ExclusivityInvalidPrefixValid = m_Supervisor->GetNValidModules();
  vector<MModule*> OfferedBehindInvalidPrefix = m_Supervisor->ReturnPossibleVolumes(0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Load()", "exclusivity behind an invalid first module", "Every entry resolves, so loading itself reports success", ExclusivityInvalidPrefixLoadResult) && Passed;
  Passed = Evaluate("GetNValidModules()", "exclusivity behind an invalid first module", "Immediate is invalid on its own, so nothing is valid before any candidate is even considered", ExclusivityInvalidPrefixValid, 0U) && Passed;
  Passed = EvaluateFalse("ReturnPossibleVolumes(Position)", "exclusivity behind an invalid first module", "A candidate is not offered when it would exclusively conflict with a module still present later in the sequence, even though nothing was valid before the candidate was considered", find(OfferedBehindInvalidPrefix.begin(), OfferedBehindInvalidPrefix.end(), SharedB) != OfferedBehindInvalidPrefix.end()) && Passed;
  RemoveTemporaryFile(ExclusivityInvalidPrefixFile);

  // The bound above must not blanket-disable ALL exclusivity checking -- that would also hide a
  // conflict between two OTHER modules unrelated to the candidate. Sequence (via XML, as above):
  // [slot, ExclusiveA, SharedB, ExclusiveX] with candidate SharedX (ExclusiveX/SharedX an unrelated
  // type). ExclusiveA/SharedB collide regardless of the candidate, invalidating from position 2 on
  // -- the scan must never reach ExclusiveX, so SharedX must be offered.
  const uint64_t c_RuleSharedX = 0x4000000;
  SupervisorModuleProbe* ExclusiveX = new SupervisorModuleProbe("UT Rule Exclusive X", "XmlTagUTRuleExclusiveX", c_RuleSharedX);
  SupervisorModuleProbe* SharedX = new SupervisorModuleProbe("UT Rule Shared X", "XmlTagUTRuleSharedX", c_RuleSharedX);
  SharedX->SetProbeTypeExclusive(false);
  m_Supervisor->AddAvailableModule(ExclusiveX);
  m_Supervisor->AddAvailableModule(SharedX);

  const MString UnrelatedExclusivityFile = GetTemporaryFileName("supervisor_exclusivity_unrelated_conflict.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "unrelated exclusivity conflict later in the sequence", "A configuration with an unrelated exclusivity conflict behind the candidate's position can be written",
                        WriteTextFile(UnrelatedExclusivityFile,
                                      "<NuclearizerData>\n"
                                      "  <Version>1</Version>\n"
                                      "  <ModuleSequence>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleSpacer</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleExclusive</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleSharedB</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleExclusiveX</ModuleSequenceItem>\n"
                                      "  </ModuleSequence>\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool UnrelatedExclusivityLoadResult = m_Supervisor->Load(UnrelatedExclusivityFile);
  vector<MModule*> OfferedPastUnrelatedExclusivityConflict = m_Supervisor->ReturnPossibleVolumes(0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Load()", "unrelated exclusivity conflict later in the sequence", "Every entry resolves, so loading itself reports success", UnrelatedExclusivityLoadResult) && Passed;
  Passed = EvaluateTrue("ReturnPossibleVolumes(Position)", "unrelated exclusivity conflict later in the sequence", "A candidate is still offered when an exclusivity conflict unrelated to the candidate already invalidates the sequence before a same-type module further down", find(OfferedPastUnrelatedExclusivityConflict.begin(), OfferedPastUnrelatedExclusivityConflict.end(), SharedX) != OfferedPastUnrelatedExclusivityConflict.end()) && Passed;
  RemoveTemporaryFile(UnrelatedExclusivityFile);

  // Appending must not be affected by the check above -- there is nothing after the last position
  m_Supervisor->Clear();
  m_Supervisor->SetModule(Provider, 0);
  vector<MModule*> OfferedAtEnd = m_Supervisor->ReturnPossibleVolumes(1);
  Passed = EvaluateTrue("ReturnPossibleVolumes(Position)", "appending at the end", "Appending still offers the modules whose requirements are met", find(OfferedAtEnd.begin(), OfferedAtEnd.end(), Immediate) != OfferedAtEnd.end()) && Passed;

  // GetNValidModules reports how far a sequence is valid without changing it
  m_Supervisor->Clear();
  m_Supervisor->SetModule(Provider, 0);
  m_Supervisor->SetModule(Spacer, 1);
  Passed = Evaluate("GetNValidModules()", "valid sequence", "A completely valid sequence is valid over its whole length", m_Supervisor->GetNValidModules(), m_Supervisor->GetNModules()) && Passed;
  Passed = Evaluate("GetNModules()", "after GetNValidModules", "Querying the valid length does not change the sequence", m_Supervisor->GetNModules(), 2U) && Passed;

  // Replacing an existing position with a module that keeps the sequence valid reports success
  m_Supervisor->Clear();
  m_Supervisor->SetModule(Provider, 0);
  m_Supervisor->SetModule(Spacer, 1);
  Passed = EvaluateTrue("SetModule()", "valid replacement", "Replacing a module with one that keeps the sequence valid reports success", m_Supervisor->SetModule(NoOverlap, 1)) && Passed;
  Passed = Evaluate("GetNModules()", "valid replacement", "A valid replacement does not change the length of the sequence", m_Supervisor->GetNModules(), 2U) && Passed;
  Passed = EvaluateTrue("GetModule()", "valid replacement", "The replacement module sits at the position it was set to", m_Supervisor->GetModule(1) == NoOverlap) && Passed;
  Passed = EvaluateTrue("GetModule()", "valid replacement", "The module before the replaced position is untouched", m_Supervisor->GetModule(0) == Provider) && Passed;

  // A start module can only ever be first -- elsewhere it never drains its incoming queue
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "start module as the first module", "A module which generates its own events is offered as the first module of a sequence", IsOffered(Empty, SecondStart)) && Passed;
  Passed = EvaluateFalse("ReturnPossibleVolumes()", "start module after another module", "A module which generates its own events is not offered anywhere but as the first module", IsOffered(AfterProvider, SecondStart)) && Passed;

  m_Supervisor->Clear();
  m_Supervisor->SetModule(Provider, 0);
  DisableDefaultStreams();
  const bool SecondStartPlaced = m_Supervisor->SetModule(SecondStart, 1);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetModule()", "second start module", "A second module which generates its own events is rejected even when placed directly through SetModule", SecondStartPlaced) && Passed;
  Passed = Evaluate("GetNModules()", "second start module", "The offending module is trimmed from the sequence", m_Supervisor->GetNModules(), 1U) && Passed;

  // GetNValidModules() must judge the soft-provider check only against the hard-valid prefix -- a
  // provider in an already-invalid suffix can't be blamed for a soft violation earlier on. Loaded
  // from XML since SetModule() would trim the invalid middle module before this could be built.
  const MString SoftSuffixFile = GetTemporaryFileName("supervisor_soft_suffix.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "soft provider in invalid suffix", "A configuration with a provider stranded behind an invalid module can be written",
                        WriteTextFile(SoftSuffixFile,
                                      "<NuclearizerData>\n"
                                      "  <Version>1</Version>\n"
                                      "  <ModuleSequence>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleSoftUser</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleImmediate</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleSoftProvider</ModuleSequenceItem>\n"
                                      "  </ModuleSequence>\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool SoftSuffixLoadResult = m_Supervisor->Load(SoftSuffixFile);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Load()", "soft provider in invalid suffix", "Every entry resolves, so loading itself reports success", SoftSuffixLoadResult) && Passed;
  Passed = Evaluate("GetNModules()", "soft provider in invalid suffix", "All three modules are kept even though the sequence is not fully valid", m_Supervisor->GetNModules(), 3U) && Passed;
  DisableDefaultStreams();
  const unsigned int SoftSuffixValid = m_Supervisor->GetNValidModules();
  EnableDefaultStreams();
  Passed = Evaluate("GetNValidModules()", "soft provider in invalid suffix", "The soft consumer is valid on its own once the invalid module and the provider stranded behind it are excluded from consideration", SoftSuffixValid, 1U) && Passed;

  RemoveTemporaryFile(SoftSuffixFile);

  // ReturnPossibleVolumes(Position) must judge candidates against the same hard-valid prefix, not
  // the raw remainder -- otherwise a stranded provider can disqualify a candidate GetNValidModules()
  // would accept. Position 0 holds Spacer, not SoftUser itself, since a module already at its own
  // position is always excluded from its own candidate list regardless of this bug.
  const MString SoftSuffixOfferFile = GetTemporaryFileName("supervisor_soft_suffix_offer.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "soft provider in invalid suffix, offer check", "A configuration with a neutral first module and a provider stranded behind an invalid one can be written",
                        WriteTextFile(SoftSuffixOfferFile,
                                      "<NuclearizerData>\n"
                                      "  <Version>1</Version>\n"
                                      "  <ModuleSequence>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleSpacer</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleImmediate</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleSoftProvider</ModuleSequenceItem>\n"
                                      "  </ModuleSequence>\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool SoftSuffixOfferLoadResult = m_Supervisor->Load(SoftSuffixOfferFile);
  const unsigned int SoftSuffixOfferValid = m_Supervisor->GetNValidModules();
  vector<MModule*> OfferedWithSoftSuffix = m_Supervisor->ReturnPossibleVolumes(0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Load()", "soft provider in invalid suffix, offer check", "Every entry resolves, so loading itself reports success", SoftSuffixOfferLoadResult) && Passed;
  Passed = Evaluate("GetNValidModules()", "soft provider in invalid suffix, offer check", "Spacer alone is valid once the invalid module and the provider stranded behind it are excluded from consideration", SoftSuffixOfferValid, 1U) && Passed;
  Passed = EvaluateTrue("ReturnPossibleVolumes(Position)", "soft provider in invalid suffix, offer check", "A candidate for position 0 is not hidden by a soft-requirement provider stranded behind an already-invalid module", find(OfferedWithSoftSuffix.begin(), OfferedWithSoftSuffix.end(), SoftUser) != OfferedWithSoftSuffix.end()) && Passed;

  // Opposite direction: bounding the successor scan by validity BEFORE the candidate is placed
  // must not miss a genuine self-erasure. Immediate at position 0 is already invalid on its own
  // (empty pre-candidate prefix), but SoftProviderLate at position 1 would still sit directly
  // after SoftUser if SoftUser replaced Immediate, violating SoftUser's own ordering. Built from
  // XML, as above -- SetModule() would trim Immediate before SoftProviderLate could be added.
  const MString SelfErasureFile = GetTemporaryFileName("supervisor_self_erasure.cfg");
  Passed = EvaluateTrue("WriteTextFile()", "self-erasure via a later, still-present provider", "A configuration with an already-invalid first module and a real provider right behind it can be written",
                        WriteTextFile(SelfErasureFile,
                                      "<NuclearizerData>\n"
                                      "  <Version>1</Version>\n"
                                      "  <ModuleSequence>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleImmediate</ModuleSequenceItem>\n"
                                      "    <ModuleSequenceItem>XmlTagUTRuleSoftProvider</ModuleSequenceItem>\n"
                                      "  </ModuleSequence>\n"
                                      "</NuclearizerData>\n")) && Passed;
  DisableDefaultStreams();
  const bool SelfErasureLoadResult = m_Supervisor->Load(SelfErasureFile);
  const unsigned int SelfErasureCurrentValid = m_Supervisor->GetNValidModules();
  vector<MModule*> OfferedForSelfErasure = m_Supervisor->ReturnPossibleVolumes(0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Load()", "self-erasure via a later, still-present provider", "Every entry resolves, so loading itself reports success", SelfErasureLoadResult) && Passed;
  Passed = Evaluate("GetNValidModules()", "self-erasure via a later, still-present provider", "Immediate is invalid on its own, so nothing is valid before any candidate is even considered", SelfErasureCurrentValid, 0U) && Passed;
  Passed = EvaluateFalse("ReturnPossibleVolumes(Position)", "self-erasure via a later, still-present provider", "A candidate is not offered when placing it would violate its own soft ordering requirement against a module still present later in the sequence, even though nothing was valid before the candidate was considered", find(OfferedForSelfErasure.begin(), OfferedForSelfErasure.end(), SoftUser) != OfferedForSelfErasure.end()) && Passed;
  RemoveTemporaryFile(SelfErasureFile);

  m_Supervisor->Clear();


  gErrorIgnoreLevel = PreviousErrorIgnoreLevel;
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSupervisor Test;
  return Test.Run() == true ? 0 : 1;
}


// UTSupervisor.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
