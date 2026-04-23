/*
 * UTMultiEvent.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib:
#include "MExceptions.h"
#include "MMultiEvent.h"
#include "MFile.h"
#include "MPhysicalEvent.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MUnidentifiableEvent.h"


//! Unit test class for MMultiEvent
class UTMultiEvent : public MUnitTest
{
public:
  //! Default constructor
  UTMultiEvent() : MUnitTest("UTMultiEvent") {}
  //! Default destructor
  virtual ~UTMultiEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test basic state and validation
  bool TestBasics();
  //! Test assimilation, duplication, reset, and tra round-trips
  bool TestRoundTrips();
  //! Test wrapper methods and edge cases
  bool TestHelpers();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTMultiEvent::Run()
{
  bool Passed = true;

  Passed = TestBasics() && Passed;
  Passed = TestRoundTrips() && Passed;
  Passed = TestHelpers() && Passed;

  Summarize();

  return Passed;
}
//! Test basic state and validation
bool UTMultiEvent::TestBasics()
{
  bool Passed = true;

  MMultiEvent Event;
  Passed = Evaluate("GetType()", "default", "Multi events report the correct type", Event.GetType(), MPhysicalEvent::c_Multi) && Passed;
  Passed = Evaluate("GetTypeString()", "default", "Multi events report the correct type string", Event.GetTypeString(), MString("Multi")) && Passed;
  Passed = EvaluateSize("GetNumberOfEvents()", "default", "A default multi event has no sub-events", Event.GetNumberOfEvents(), 0U) && Passed;
  Passed = EvaluateFalse("IsGoodEvent()", "default", "A default multi event is not yet valid", Event.IsGoodEvent()) && Passed;

  MUnidentifiableEvent* One = new MUnidentifiableEvent();
  One->SetEnergy(4.0);
  Event.Add(One);
  MUnidentifiableEvent* Two = new MUnidentifiableEvent();
  Two->SetEnergy(3.0);
  Event.Add(Two);

  Passed = EvaluateSize("GetNumberOfEvents()", "add", "Two sub-events are stored", Event.GetNumberOfEvents(), 2U) && Passed;
  Passed = Evaluate("GetEnergy()", "add", "The multi-event energy is the sum of the sub-events", Event.GetEnergy(), 7.0) && Passed;
  Passed = Evaluate("GetEvent(0)->GetType()", "add", "The first sub-event is accessible", Event.GetEvent(0)->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
  Passed = Evaluate("GetEvent(1)->GetType()", "add", "The second sub-event is accessible", Event.GetEvent(1)->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
  Passed = EvaluateTrue("Validate()", "add", "The populated multi event validates", Event.Validate()) && Passed;
  Passed = EvaluateTrue("ToString()", "add", "The human-readable multi-event description includes a sub-event", Event.ToString().Contains("Unidentifiable-event")) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "multi", "Duplicate keeps the concrete multi-event type", dynamic_cast<MMultiEvent*>(Duplicate) != nullptr) && Passed;
  Passed = EvaluateSize("Duplicate()->GetNumberOfEvents()", "multi", "Duplicate preserves the number of sub-events", dynamic_cast<MMultiEvent*>(Duplicate)->GetNumberOfEvents(), 2U) && Passed;
  Passed = Evaluate("Duplicate()->GetEnergy()", "multi", "Duplicate preserves the total energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Duplicate;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test assimilation, duplication, reset, and tra round-trips
bool UTMultiEvent::TestRoundTrips()
{
  bool Passed = true;

  MMultiEvent Event;
  MUnidentifiableEvent* One = new MUnidentifiableEvent();
  One->SetEnergy(4.0);
  Event.Add(One);
  MUnidentifiableEvent* Two = new MUnidentifiableEvent();
  Two->SetEnergy(3.0);
  Event.Add(Two);
  Passed = EvaluateTrue("Validate()", "seeded multi", "The seeded multi event validates", Event.Validate()) && Passed;

  MString Tra = Event.ToTraString();
  Passed = EvaluateTrue("ToTraString()", "multi tra", "The tra string contains the multi-event separators", Tra.Contains("SI") && Tra.Contains("SF")) && Passed;
  MFile File;
  MString FileName = "/tmp/UTMultiEvent.tra";
  Passed = EvaluateTrue("Open()", "multi tra", "The temporary tra file can be opened for writing", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateTrue("Stream()", "multi tra", "The event can be streamed to a file", Event.Stream(File, 0, false, false, false)) && Passed;
  File.Close();

  MMultiEvent Streamed;
  Passed = EvaluateTrue("Open()", "multi read", "The temporary tra file can be reopened for reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateFalse("Stream()", "multi read", "The multi event can be streamed back from file", Streamed.Stream(File, 0, true, false, false)) && Passed;
  Passed = EvaluateSize("GetNumberOfEvents()", "multi read", "The streamed multi-event preserves the number of sub-events", Streamed.GetNumberOfEvents(), Event.GetNumberOfEvents()) && Passed;
  Passed = Evaluate("GetEnergy()", "multi read", "The streamed multi-event preserves the total energy", Streamed.GetEnergy(), Event.GetEnergy()) && Passed;
  File.Close();

  MMultiEvent Delayed;
  Passed = EvaluateTrue("Open()", "multi delayed", "The temporary tra file can be reopened for delayed reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateFalse("Stream()", "multi delayed", "Delayed reading buffers the multi-event for later parsing", Delayed.Stream(File, 0, true, false, true)) && Passed;
  Passed = EvaluateTrue("ParseDelayed()", "multi delayed", "The delayed multi-event parses successfully", Delayed.ParseDelayed()) && Passed;
  Passed = EvaluateSize("GetNumberOfEvents()", "multi delayed", "The delayed parse preserves the number of sub-events", Delayed.GetNumberOfEvents(), Event.GetNumberOfEvents()) && Passed;
  Passed = Evaluate("GetEnergy()", "multi delayed", "The delayed parse preserves the total energy", Delayed.GetEnergy(), Event.GetEnergy()) && Passed;
  File.Close();

  MMultiEvent FromMulti;
  Passed = EvaluateTrue("Assimilate(MMultiEvent*)", "multi copy", "A multi-event can assimilate from another multi-event", FromMulti.Assimilate(&Event)) && Passed;
  Passed = Evaluate("FromMulti.GetNumberOfEvents()", "multi copy", "Assimilation preserves the number of sub-events", FromMulti.GetNumberOfEvents(), Event.GetNumberOfEvents()) && Passed;

  MPhysicalEvent* FromPhysical = new MMultiEvent();
  Passed = EvaluateTrue("Assimilate(MPhysicalEvent*)", "multi physical", "A multi-event can assimilate from a physical event pointer", FromPhysical->Assimilate(&Event)) && Passed;
  Passed = Evaluate("FromPhysical->GetEnergy()", "multi physical", "Assimilation preserves the total energy", FromPhysical->GetEnergy(), Event.GetEnergy()) && Passed;
  delete FromPhysical;

  Event.Reset();
  Passed = EvaluateSize("Reset()", "multi reset", "Reset clears all owned sub-events", Event.GetNumberOfEvents(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test wrapper methods and edge cases
bool UTMultiEvent::TestHelpers()
{
  bool Passed = true;

  MMultiEvent Event;
  Passed = Evaluate("Data()", "helpers", "Data() returns the concrete multi-event pointer", Event.Data(), dynamic_cast<MPhysicalEvent*>(&Event)) && Passed;
  Passed = Evaluate("GetTypeString()", "helpers", "Multi events report the expected type string", Event.GetTypeString(), MString("Multi")) && Passed;
  Passed = EvaluateTrue("ParseLine()", "helpers", "The base-line parser accepts a multi-event type line", Event.ParseLine("ET MT", false) == 0) && Passed;
  Passed = EvaluateTrue("ParseDelayed()", "helpers", "An empty delayed parse still validates", Event.ParseDelayed()) && Passed;

  MUnidentifiableEvent* One = new MUnidentifiableEvent();
  One->SetEnergy(4.0);
  Event.Add(One);
  MUnidentifiableEvent* Two = new MUnidentifiableEvent();
  Two->SetEnergy(3.0);
  Event.Add(Two);
  Passed = Evaluate("ToString()", "helpers", "The string representation concatenates the child events deterministically", Event.ToString(), MString("The data of the Unidentifiable-event:\nEnergy: 4.000The data of the Unidentifiable-event:\nEnergy: 3.000")) && Passed;

  EvaluateException<MExceptionIndexOutOfBounds>("GetEvent()", "helpers", "Out-of-bounds sub-event access throws", [&](){ Event.GetEvent(2); });

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTMultiEvent Test;
  Test.Run();

  return 0;
}
