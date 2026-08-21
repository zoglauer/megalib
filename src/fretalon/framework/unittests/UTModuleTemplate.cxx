/*
 * UTModuleTemplate.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MAssembly.h"
#include "MModuleTemplate.h"
#include "MReadOutAssembly.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Standard libs:
using namespace std;


//! Unit test class for MModuleTemplate
class UTModuleTemplate : public MUnitTest
{
public:
  //! Default constructor
  UTModuleTemplate() : MUnitTest("UTModuleTemplate") {}
  //! Default destructor
  virtual ~UTModuleTemplate() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test construction-time module metadata
  bool TestMetadata();
  //! Test lifecycle and analysis hooks
  bool TestLifecycleAndAnalysis();
  //! Test XML configuration helpers
  bool TestXmlConfiguration();
};


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTemplate::Run()
{
  bool Passed = true;

  Passed = TestMetadata() && Passed;
  Passed = TestLifecycleAndAnalysis() && Passed;
  Passed = TestXmlConfiguration() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTemplate::TestMetadata()
{
  bool Passed = true;

  MModuleTemplate Module;
  Passed = Evaluate("GetName()", "default", "The template module exposes its representative module name", Module.GetName(), MString("Template")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The template module exposes its representative XML tag", Module.GetXmlTag(), MString("XmlTagTemplate")) && Passed;

  Passed = EvaluateSize("GetNPreceedingModuleTypes()", "template", "The template module registers the expected predecessor module types", Module.GetNPreceedingModuleTypes(), static_cast<size_t>(5)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "detector effects", "The first predecessor is detector effects", Module.GetPreceedingModuleType(0), static_cast<uint64_t>(MAssembly::c_DetectorEffectsEngine)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "energy calibration", "The second predecessor is energy calibration", Module.GetPreceedingModuleType(1), static_cast<uint64_t>(MAssembly::c_EnergyCalibration)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "charge sharing", "The third predecessor is charge-sharing correction", Module.GetPreceedingModuleType(2), static_cast<uint64_t>(MAssembly::c_ChargeSharingCorrection)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "depth correction", "The fourth predecessor is depth correction", Module.GetPreceedingModuleType(3), static_cast<uint64_t>(MAssembly::c_DepthCorrection)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "strip pairing", "The fifth predecessor is strip pairing", Module.GetPreceedingModuleType(4), static_cast<uint64_t>(MAssembly::c_StripPairing)) && Passed;
  for (unsigned int i = 0; i < Module.GetNPreceedingModuleTypes(); ++i) {
    Passed = EvaluateTrue("GetPreceedingModuleHardRequirement()", MString(i), "All template predecessors are hard requirements", Module.GetPreceedingModuleHardRequirement(i)) && Passed;
  }

  Passed = EvaluateSize("GetNModuleTypes()", "template", "The template module registers the expected provided module types", Module.GetNModuleTypes(), static_cast<size_t>(6)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "detector effects", "The template module provides detector effects", Module.ProvidesModuleType(MAssembly::c_DetectorEffectsEngine)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "energy calibration", "The template module provides energy calibration", Module.ProvidesModuleType(MAssembly::c_EnergyCalibration)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "charge sharing", "The template module provides charge-sharing correction", Module.ProvidesModuleType(MAssembly::c_ChargeSharingCorrection)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "depth correction", "The template module provides depth correction", Module.ProvidesModuleType(MAssembly::c_DepthCorrection)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "strip pairing", "The template module provides strip pairing", Module.ProvidesModuleType(MAssembly::c_StripPairing)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "event reconstruction", "The template module provides event reconstruction", Module.ProvidesModuleType(MAssembly::c_EventReconstruction)) && Passed;
  Passed = EvaluateFalse("ProvidesModuleType()", "missing module type", "The template module rejects a representative unregistered module type", Module.ProvidesModuleType(0x80000000ULL)) && Passed;

  Passed = EvaluateSize("GetNSucceedingModuleTypes()", "template", "The template module does not register succeeding module types", Module.GetNSucceedingModuleTypes(), static_cast<size_t>(0)) && Passed;
  Passed = EvaluateFalse("HasOptionsGUI()", "template", "The template module does not enable its options GUI by default", Module.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "template", "The template module allows multi-threading", Module.AllowsMultiThreading()) && Passed;
  Passed = EvaluateFalse("AllowsMultipleInstances()", "template", "The template module does not allow multiple instances", Module.AllowsMultipleInstances()) && Passed;

  MModuleTemplate* Clone = Module.Clone();
  Passed = EvaluateTrue("Clone()", "template", "Clone returns a distinct template module instance", Clone != nullptr && Clone != &Module) && Passed;
  Passed = Evaluate("Clone()", "template name", "The cloned template module has the same representative name", Clone->GetName(), MString("Template")) && Passed;
  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTemplate::TestLifecycleAndAnalysis()
{
  bool Passed = true;

  MModuleTemplate Module;
  Passed = EvaluateTrue("Initialize()", "template", "The template module initialize hook succeeds", Module.Initialize()) && Passed;

  MReadOutAssembly Event;
  Event.SetFilteredOut(false);
  const uint64_t InitialAnalysisProgress = Event.GetAnalysisProgress();
  Passed = EvaluateTrue("AnalyzeEvent()", "representative event", "The template analysis hook accepts a representative event", Module.AnalyzeEvent(&Event)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "representative event", "The template analysis hook does not modify analysis progress", Event.GetAnalysisProgress(), InitialAnalysisProgress) && Passed;

  Module.AddReadOutAssembly(new MReadOutAssembly());
  Passed = EvaluateTrue("HasAddedReadOutAssemblies()", "before Finalize", "The inherited queue contains a representative event before finalization", Module.HasAddedReadOutAssemblies()) && Passed;
  Module.Finalize();
  Passed = EvaluateFalse("HasAddedReadOutAssemblies()", "after Finalize", "The template finalizer clears inherited input queues through MModule", Module.HasAddedReadOutAssemblies()) && Passed;
  Passed = EvaluateFalse("HasAnalyzedReadOutAssemblies()", "after Finalize", "The template finalizer clears inherited output queues through MModule", Module.HasAnalyzedReadOutAssemblies()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTemplate::TestXmlConfiguration()
{
  bool Passed = true;

  MModuleTemplate Module;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "nullptr", "The template XML reader accepts the representative default configuration", Module.ReadXmlConfiguration(nullptr)) && Passed;

  MXmlNode* Node = Module.CreateXmlConfiguration();
  Passed = Evaluate("CreateXmlConfiguration()", "XmlTagTemplate", "The template XML writer creates a node with the template XML tag", Node->GetName(), MString("XmlTagTemplate")) && Passed;
  Passed = EvaluateSize("CreateXmlConfiguration()", "child nodes", "The template XML writer does not create child nodes by default", Node->GetNNodes(), static_cast<size_t>(0)) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTModuleTemplate Test;
  return Test.Run() == true ? 0 : 1;
}
