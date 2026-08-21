/*
 * UTSubModule.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MSubModule.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Standard libs:
#include <type_traits>
using namespace std;


//! Unit test class for MSubModule
class UTSubModule : public MUnitTest
{
public:
  //! Default constructor
  UTSubModule() : MUnitTest("UTSubModule") {}
  //! Default destructor
  virtual ~UTSubModule() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test helper exposing protected MSubModule state
  class SubModuleProbe : public MSubModule
  {
  public:
    SubModuleProbe() : MSubModule(), m_AnalyzeCalls(0), m_ClearCalls(0), m_FinalizeCalls(0) {}
    virtual ~SubModuleProbe() {}

    virtual bool AnalyzeEvent(MReadOutAssembly* Event)
    {
      ++m_AnalyzeCalls;
      if (Event != nullptr) {
        Event->SetAnalysisProgress(c_ProbeProgress);
      }
      return true;
    }

    virtual void Clear()
    {
      ++m_ClearCalls;
      MSubModule::Clear();
    }

    virtual void Finalize()
    {
      ++m_FinalizeCalls;
      MSubModule::Finalize();
    }

    MString GetProbeName() const { return m_Name; }
    void SetProbeName(const MString& Name) { m_Name = Name; }
    unsigned int GetAnalyzeCalls() const { return m_AnalyzeCalls; }
    unsigned int GetClearCalls() const { return m_ClearCalls; }
    unsigned int GetFinalizeCalls() const { return m_FinalizeCalls; }

    static const uint64_t c_ProbeProgress = 0x100;

  private:
    unsigned int m_AnalyzeCalls;
    unsigned int m_ClearCalls;
    unsigned int m_FinalizeCalls;
  };

  //! Test construction-time defaults and exposed protected state
  bool TestDefaults();
  //! Test lifecycle hooks and derived analysis behavior
  bool TestLifecycleAndAnalysis();
  //! Test XML configuration helpers
  bool TestXmlConfiguration();
  //! Test the compile-time copy and move restrictions of the interface
  bool TestCopyAndMoveRestrictions();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSubModule::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestLifecycleAndAnalysis() && Passed;
  Passed = TestXmlConfiguration() && Passed;
  Passed = TestCopyAndMoveRestrictions() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSubModule::TestDefaults()
{
  bool Passed = true;

  SubModuleProbe Module;
  Passed = Evaluate("MSubModule()", "default", "The sub-module base starts with the representative default name", Module.GetProbeName(), MString("Unnamed sub-module")) && Passed;

  Module.SetProbeName("ProbeSubModule");
  Passed = Evaluate("m_Name", "ProbeSubModule", "A derived sub-module can maintain its protected name", Module.GetProbeName(), MString("ProbeSubModule")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSubModule::TestLifecycleAndAnalysis()
{
  bool Passed = true;

  SubModuleProbe Module;
  Passed = EvaluateTrue("Initialize()", "default", "The base sub-module initialize hook succeeds", Module.Initialize()) && Passed;

  Module.Clear();
  Passed = Evaluate("Clear()", "default", "The derived clear hook can call the base no-op clear implementation", Module.GetClearCalls(), 1U) && Passed;

  MReadOutAssembly Event;
  Event.SetFilteredOut(false);
  Passed = EvaluateTrue("AnalyzeEvent()", "representative event", "The probe analysis hook accepts a representative event", Module.AnalyzeEvent(&Event)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "representative event", "The probe analysis hook is called once", Module.GetAnalyzeCalls(), 1U) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "analysis progress", "The probe analysis hook can update the event progress", Event.HasAnalysisProgress(SubModuleProbe::c_ProbeProgress)) && Passed;

  Module.Finalize();
  Passed = Evaluate("Finalize()", "default", "The derived finalizer can call the base no-op finalizer implementation", Module.GetFinalizeCalls(), 1U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSubModule::TestXmlConfiguration()
{
  bool Passed = true;

  SubModuleProbe Module;
  MXmlNode Node(nullptr, "SubModuleNode");
  Passed = EvaluateTrue("ReadXmlConfiguration()", "representative node", "The base sub-module XML reader accepts the representative configuration", Module.ReadXmlConfiguration(&Node)) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "nullptr", "The base sub-module XML reader accepts a null configuration pointer", Module.ReadXmlConfiguration(nullptr)) && Passed;

  MXmlNode* ReturnedNode = Module.CreateXmlConfiguration(&Node);
  Passed = EvaluateTrue("CreateXmlConfiguration()", "representative node", "The base sub-module XML writer returns the supplied node", ReturnedNode == &Node) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "representative node", "The supplied XML node is not renamed by the base sub-module writer", ReturnedNode->GetName(), MString("SubModuleNode")) && Passed;
  Passed = EvaluateSize("CreateXmlConfiguration()", "representative node", "The base sub-module XML writer does not add child nodes", ReturnedNode->GetNNodes(), static_cast<size_t>(0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSubModule::TestCopyAndMoveRestrictions()
{
  bool Passed = true;

  // The copy and move members are declared deleted, so they cannot be called at run time. They are
  // still part of the public interface, so assert the guarantee they provide through type traits.
  Passed = EvaluateFalse("MSubModule(const MSubModule&)", "deleted copy constructor", "A sub-module cannot be copy constructed", is_copy_constructible<MSubModule>::value) && Passed;
  Passed = EvaluateFalse("operator=(const MSubModule&)", "deleted copy assignment", "A sub-module cannot be copy assigned", is_copy_assignable<MSubModule>::value) && Passed;
  Passed = EvaluateFalse("MSubModule(MSubModule&&)", "deleted move constructor", "A sub-module cannot be move constructed", is_move_constructible<MSubModule>::value) && Passed;
  Passed = EvaluateFalse("operator=(MSubModule&&)", "deleted move assignment", "A sub-module cannot be move assigned", is_move_assignable<MSubModule>::value) && Passed;

  // The restriction has to reach derived modules, otherwise it could be bypassed by deriving
  Passed = EvaluateFalse("MSubModule(const MSubModule&)", "derived copy constructor", "A derived sub-module inherits the copy restriction", is_copy_constructible<SubModuleProbe>::value) && Passed;
  Passed = EvaluateFalse("operator=(const MSubModule&)", "derived copy assignment", "A derived sub-module inherits the copy-assignment restriction", is_copy_assignable<SubModuleProbe>::value) && Passed;

  // AnalyzeEvent is pure virtual, so the base cannot be instantiated on its own
  Passed = EvaluateTrue("AnalyzeEvent()", "pure virtual", "The sub-module base class is abstract and cannot be instantiated directly", is_abstract<MSubModule>::value) && Passed;
  Passed = EvaluateTrue("~MSubModule()", "virtual destructor", "The sub-module base has a virtual destructor so derived modules are destroyed correctly", has_virtual_destructor<MSubModule>::value) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSubModule Test;
  return Test.Run() == true ? 0 : 1;
}
