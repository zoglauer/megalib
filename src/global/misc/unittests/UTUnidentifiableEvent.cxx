/*
 * UTUnidentifiableEvent.cxx
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
#include "MPhysicalEvent.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MUnidentifiableEvent.h"


//! Unit test class for MUnidentifiableEvent
class UTUnidentifiableEvent : public MUnitTest
{
public:
  //! Default constructor
  UTUnidentifiableEvent() : MUnitTest("UTUnidentifiableEvent") {}
  //! Default destructor
  virtual ~UTUnidentifiableEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Parse a tra-string line-by-line into an event object
  bool ParseTraString(MUnidentifiableEvent& Event, const MString& Tra, bool Fast = false);
  //! Test basic state and validation
  bool TestBasics();
  //! Test text and tra round-trips plus duplication
  bool TestRoundTrips();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTUnidentifiableEvent::Run()
{
  bool Passed = true;

  Passed = TestBasics() && Passed;
  Passed = TestRoundTrips() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a tra-string line-by-line into an event object
bool UTUnidentifiableEvent::ParseTraString(MUnidentifiableEvent& Event, const MString& Tra, bool Fast)
{
  istringstream In(Tra.ToString());
  string Line;
  while (getline(In, Line)) {
    if (Line.empty() == true) {
      continue;
    }
    int Ret = Event.ParseLine(Line.c_str(), Fast);
    if (Ret == 1) {
      return false;
    }
  }
  return Event.Validate();
}


////////////////////////////////////////////////////////////////////////////////


//! Test basic state and validation
bool UTUnidentifiableEvent::TestBasics()
{
  bool Passed = true;

  MUnidentifiableEvent Event;
  Passed = Evaluate("GetType()", "default", "Unidentifiable events report the correct type", Event.GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
  Passed = Evaluate("GetTypeString()", "default", "Unidentifiable events report the correct type string", Event.GetTypeString(), MString("Unidentifiable")) && Passed;
  Passed = EvaluateFalse("IsGoodEvent()", "default", "Unidentifiable events are not marked good by default", Event.IsGoodEvent()) && Passed;

  Event.SetEnergy(12.5);
  Passed = Evaluate("GetEnergy()", "set/get", "SetEnergy stores the deposited energy", Event.GetEnergy(), 12.5) && Passed;
  Passed = Evaluate("ParseLine()", "set/get parse", "ParseLine stores deposited energies from representative PE lines", Event.ParseLine("PE 14.5", false), 0) && Passed;
  Passed = Evaluate("GetEnergy()", "set/get parse", "ParseLine stores the parsed deposited energy", Event.GetEnergy(), 14.5) && Passed;
  Event.SetEnergy(12.5);
  Passed = EvaluateTrue("Validate()", "set/get", "Unidentifiable events validate successfully", Event.Validate()) && Passed;
  Passed = EvaluateFalse("IsGoodEvent()", "set/get", "Unidentifiable events remain marked as not good", Event.IsGoodEvent()) && Passed;
  Passed = Evaluate("Data()", "set/get", "Data() returns the concrete event pointer", Event.Data(), dynamic_cast<MPhysicalEvent*>(&Event)) && Passed;
  Passed = Evaluate("ToString()", "set/get", "The human-readable description is formatted deterministically", Event.ToString(), MString("The data of the Unidentifiable-event:\nEnergy: 12.500")) && Passed;

  MUnidentifiableEvent Interior;
  Interior.SetEnergy(37.125);
  Passed = Evaluate("GetEnergy()", "interior energy", "Representative interior deposited energies are stored exactly", Interior.GetEnergy(), 37.125) && Passed;
  Passed = EvaluateTrue("Validate()", "interior energy", "Representative interior deposited energies validate successfully", Interior.Validate()) && Passed;
  Passed = Evaluate("ToString()", "interior energy", "Representative interior deposited energies are formatted deterministically", Interior.ToString(), MString("The data of the Unidentifiable-event:\nEnergy: 37.125")) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = Evaluate("Duplicate()->GetEnergy()", "unidentifiable duplicate", "Duplicate preserves the deposited energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Duplicate()->GetType()", "unidentifiable duplicate", "Duplicate preserves the event type", Duplicate->GetType(), Event.GetType()) && Passed;
  delete Duplicate;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test text and tra round-trips plus duplication
bool UTUnidentifiableEvent::TestRoundTrips()
{
  bool Passed = true;

  MUnidentifiableEvent Event;
  Event.SetId(7);
  Event.SetTime(MTime(2.5));
  Event.SetEnergy(12.5);
  Passed = EvaluateTrue("Validate()", "seeded unidentifiable", "The seeded unidentifiable event validates", Event.Validate()) && Passed;

  MString Tra = Event.ToTraString();
  MString ExpectedTra = MString("ET UN\nID 7\nTI ") + Event.GetTime().GetLongIntsString() + "\nPE 12.5\n";
  Passed = Evaluate("ToTraString()", "unidentifiable tra exact", "The tra-string serialization is deterministic for a representative unidentifiable event", Tra, ExpectedTra) && Passed;
  MUnidentifiableEvent ParsedTra;
  Passed = EvaluateTrue("ParseTraString()", "unidentifiable tra", "The tra-string round-trips through ParseLine", ParseTraString(ParsedTra, Tra)) && Passed;
  Passed = Evaluate("ParsedTra.GetId()", "unidentifiable tra", "The tra round-trip preserves the event id", ParsedTra.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("ParsedTra.GetEnergy()", "unidentifiable tra", "The tra round-trip preserves the energy", ParsedTra.GetEnergy(), Event.GetEnergy()) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "unidentifiable", "Duplicate keeps the concrete unidentifiable type", dynamic_cast<MUnidentifiableEvent*>(Duplicate) != nullptr) && Passed;
  Passed = Evaluate("Duplicate()->GetEnergy()", "unidentifiable", "Duplicate preserves the deposited energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Duplicate;

  MUnidentifiableEvent Copy;
  Passed = EvaluateTrue("Assimilate(MUnidentifiableEvent*)", "unidentifiable copy", "Copy assimilation works on the same type", Copy.Assimilate(&Event)) && Passed;
  Passed = Evaluate("Copy.GetEnergy()", "unidentifiable copy", "Copy assimilation preserves the energy", Copy.GetEnergy(), Event.GetEnergy()) && Passed;

  MPhysicalEvent* Generic = new MUnidentifiableEvent();
  Passed = EvaluateTrue("Assimilate(MPhysicalEvent*)", "unidentifiable physical", "Assimilation works through the base pointer", Generic->Assimilate(&Event)) && Passed;
  Passed = Evaluate("Generic->GetEnergy()", "unidentifiable physical", "Base-pointer assimilation preserves the energy", Generic->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Generic;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTUnidentifiableEvent Test;
  return Test.Run() == true ? 0 : 1;
}
