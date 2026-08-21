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
      Module->m_PreceedingModulesHardRequirement = m_PreceedingModulesHardRequirement;
      Module->m_SucceedingModules = m_SucceedingModules;
      Module->m_Modules = m_Modules;
      Module->m_IsStartModule = m_IsStartModule;
      return Module;
    }

    virtual bool Initialize()
    {
      ++m_InitializeCalls;
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
    void AddProvidedType(uint64_t Type) { AddModuleType(Type); }
    void SetConfigValue(const MString& ConfigValue) { m_ConfigValue = ConfigValue; }
    void SetMaxGeneratedEvents(int MaxGeneratedEvents) { m_MaxGeneratedEvents = MaxGeneratedEvents; }
    void SetInitializeResult(bool InitializeResult) { m_InitializeResult = InitializeResult; }
    void SetMarkNotOKDuringAnalysis(bool MarkNotOKDuringAnalysis) { m_MarkNotOKDuringAnalysis = MarkNotOKDuringAnalysis; }
    void SetAllowMultiThreading(bool Flag) { m_AllowMultiThreading = Flag; }
    void ResetProbeState()
    {
      m_MaxGeneratedEvents = -1;
      m_InitializeResult = true;
      m_MarkNotOKDuringAnalysis = false;
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

  Passed = EvaluateFalse("SetModule()", "append loader", "SetModule appends at the next sequence position; Validate currently returns false even for valid sequences", m_Supervisor->SetModule(m_Loader, 0)) && Passed;
  Passed = Evaluate("GetNModules()", "loader only", "The loader is stored as the first sequence module", m_Supervisor->GetNModules(), 1U) && Passed;
  Passed = EvaluateTrue("GetModule()", "loader", "The first configured module can be retrieved", m_Supervisor->GetModule(0) == m_Loader) && Passed;
  Passed = EvaluateTrue("GetModule()", "out of range", "Out-of-range configured-module lookup returns nullptr", m_Supervisor->GetModule(99) == nullptr) && Passed;

  vector<MModule*> PossibleAfterLoader = m_Supervisor->ReturnPossibleVolumes(1);
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "filter after loader", "Modules whose hard predecessor is already present are offered", find(PossibleAfterLoader.begin(), PossibleAfterLoader.end(), m_Filter) != PossibleAfterLoader.end()) && Passed;
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "loader excluded", "The existing module instance is not offered again", find(PossibleAfterLoader.begin(), PossibleAfterLoader.end(), m_Loader) == PossibleAfterLoader.end()) && Passed;
  Passed = EvaluateTrue("ReturnPossibleVolumes()", "saver blocked", "Modules with missing hard predecessors are not offered", find(PossibleAfterLoader.begin(), PossibleAfterLoader.end(), m_Saver) == PossibleAfterLoader.end()) && Passed;

  Passed = EvaluateFalse("SetModule()", "append filter", "Appending a valid second module keeps the sequence", m_Supervisor->SetModule(m_Filter, 1)) && Passed;
  Passed = EvaluateFalse("SetModule()", "append saver", "Appending a valid third module keeps the sequence", m_Supervisor->SetModule(m_Saver, 2)) && Passed;
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

  Passed = EvaluateFalse("RemoveModule()", "existing loader", "Removing an existing module clears it from the sequence", m_Supervisor->RemoveModule(0)) && Passed;
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
  DisableDefaultStreams();
  const bool MissingLoadResult = m_Supervisor->Load(MissingFile);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Load()", "missing configuration", "Loading a missing configuration file reports failure and leaves an empty configuration", MissingLoadResult) && Passed;
  Passed = Evaluate("GetNModules()", "after missing configuration", "A missing configuration load clears the current module sequence", m_Supervisor->GetNModules(), 0U) && Passed;

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

  m_Supervisor->Clear();
  ResetProbeStates();
  m_Supervisor->SetGeometryFileName(GeometryFileName);
  DisableDefaultStreams();
  const bool EmptyAnalysisResult = m_Supervisor->Analyze(false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Analyze()", "no modules", "Analysis fails after geometry loading when no modules are configured", EmptyAnalysisResult) && Passed;
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

  // Exit() is deliberately not exercised: with no analysis running it calls Terminate(), which ends
  // the process through gApplication->Terminate(0) and would take the test run down with it.

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
