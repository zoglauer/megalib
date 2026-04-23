/*
 * UTMuonEvent.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib:
#include "MMuonEvent.h"
#include "MFile.h"
#include "MPhysicalEvent.h"
#include "MString.h"
#include "MUnitTest.h"


//! Unit test class for MMuonEvent
class UTMuonEvent : public MUnitTest
{
public:
  //! Default constructor
  UTMuonEvent() : MUnitTest("UTMuonEvent") {}
  //! Default destructor
  virtual ~UTMuonEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Parse a tra-string line-by-line into an event object
  bool ParseTraString(MMuonEvent& Event, const MString& Tra, bool Fast = false);
  //! Test basic state and validation
  bool TestBasics();
  //! Test text and tra round-trips plus duplication
  bool TestRoundTrips();
  //! Test copy-style assimilation and file-stream round-trip
  bool TestCopyAndStream();
  //! Test helper methods and reset behavior
  bool TestHelpers();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTMuonEvent::Run()
{
  bool Passed = true;

  Passed = TestBasics() && Passed;
  Passed = TestRoundTrips() && Passed;
  Passed = TestCopyAndStream() && Passed;
  Passed = TestHelpers() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a tra-string line-by-line into an event object
bool UTMuonEvent::ParseTraString(MMuonEvent& Event, const MString& Tra, bool Fast)
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
bool UTMuonEvent::TestBasics()
{
  bool Passed = true;

  MMuonEvent Event;
  Passed = Evaluate("GetType()", "default", "Muon events report the correct type", Event.GetType(), MPhysicalEvent::c_Muon) && Passed;
  Passed = Evaluate("GetTypeString()", "default", "Muon events report the correct type string", Event.GetTypeString(), MString("Muon")) && Passed;

  Passed = EvaluateTrue("Assimilate(Direction,CoG,Energy)", "muon", "A valid muon event can be created from basic inputs", Event.Assimilate(MVector(1.0, 0.0, 0.0), MVector(0.0, 1.0, 0.0), 2.5)) && Passed;
  Passed = Evaluate("GetEnergy()", "muon", "The muon-event energy is stored", Event.GetEnergy(), 2.5) && Passed;
  Passed = Evaluate("GetDirection()", "muon", "The muon direction is stored", Event.GetDirection(), MVector(1.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("GetCenterOfGravity()", "muon", "The muon center of gravity is stored", Event.GetCenterOfGravity(), MVector(0.0, 1.0, 0.0)) && Passed;
  Passed = EvaluateTrue("Validate()", "muon", "A positive-energy muon event validates successfully", Event.Validate()) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = Evaluate("Duplicate()->GetEnergy()", "muon", "Duplicate preserves the muon energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Duplicate()->GetType()", "muon", "Duplicate preserves the muon type", Duplicate->GetType(), Event.GetType()) && Passed;
  delete Duplicate;

  MMuonEvent Invalid;
  Passed = EvaluateFalse("Validate()", "muon negative energy", "Negative muon energies are rejected", Invalid.Assimilate(MVector(1.0, 0.0, 0.0), MVector(0.0, 1.0, 0.0), -2.5)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy-style assimilation and file-stream round-trip
bool UTMuonEvent::TestCopyAndStream()
{
  bool Passed = true;

  MMuonEvent Event;
  Event.SetId(8);
  Event.SetTime(MTime(4.0));
  Event.Assimilate(MVector(1.0, 0.0, 0.0), MVector(0.0, 1.0, 0.0), 2.5);
  Passed = EvaluateTrue("Validate()", "seeded muon", "The seeded muon event validates", Event.Validate()) && Passed;

  MMuonEvent Copied;
  Passed = EvaluateTrue("Assimilate(MMuonEvent*)", "copy", "Copy-style assimilation from the concrete type works", Copied.Assimilate(&Event)) && Passed;
  Passed = Evaluate("Copied.GetEnergy()", "copy", "Copy-style assimilation preserves the energy", Copied.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Copied.GetDirection()", "copy", "Copy-style assimilation preserves the direction", Copied.GetDirection(), Event.GetDirection()) && Passed;

  MPhysicalEvent* Generic = new MMuonEvent();
  Passed = EvaluateTrue("Assimilate(MPhysicalEvent*)", "copy", "Copy-style assimilation from a physical pointer works", Generic->Assimilate(&Event)) && Passed;
  Passed = Evaluate("Generic->GetEnergy()", "copy", "Base-pointer assimilation preserves the energy", Generic->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Generic;

  MString Basic = Event.ToString();
  Passed = EvaluateTrue("Assimilate(char*)", "basic line", "The basic text assimilation path is exercised", Event.Assimilate((char*) Basic.Data())) && Passed;

  MFile File;
  MString FileName = "/tmp/UTMuonEvent.tra";
  Passed = EvaluateTrue("Open()", "stream write", "The temporary muon file can be opened for writing", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream write", "The muon event write-stream completes at EOF", Event.Stream(File, 0, false, false, false)) && Passed;
  File.Close();

  MMuonEvent Streamed;
  Passed = EvaluateTrue("Open()", "stream read", "The temporary muon file can be reopened for reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream read", "The muon event read-stream also returns false at EOF", Streamed.Stream(File, 0, true, false, false)) && Passed;
  Passed = Evaluate("Streamed.GetId()", "stream read", "The streamed copy preserves the event id", Streamed.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("Streamed.GetEnergy()", "stream read", "The streamed copy preserves the energy", Streamed.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Streamed.GetDirection()", "stream read", "The streamed copy preserves the direction", Streamed.GetDirection(), Event.GetDirection()) && Passed;
  File.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test helper methods and reset behavior
bool UTMuonEvent::TestHelpers()
{
  bool Passed = true;

  MMuonEvent Event;
  Event.SetEnergy(4.5);
  Event.SetDirection(MVector(1.0, 0.0, 0.0));
  Event.SetCenterOfGravity(MVector(0.0, 1.0, 0.0));
  Passed = EvaluateTrue("Validate()", "helpers", "The seeded muon helper validates", Event.Validate()) && Passed;

  Passed = Evaluate("Data()", "helpers", "Data() returns the concrete muon pointer", Event.Data(), dynamic_cast<MPhysicalEvent*>(&Event)) && Passed;
  Passed = Evaluate("ToString()", "helpers", "The muon string representation is formatted deterministically", Event.ToString(), MString("The data of the Muon-event:\nEnergy: 4.500\nDirection: 1.000, 0.000, 0.000\nCenterOfGravity: 0.000, 1.000, 0.000\n")) && Passed;

  MString Basic = Event.ToString();
  Passed = EvaluateTrue("Assimilate(char*)", "helpers", "The stub char* assimilator returns true", Event.Assimilate((char*) Basic.Data())) && Passed;
  Passed = Evaluate("GetEnergy()", "helpers", "The stub char* assimilator resets the energy", Event.GetEnergy(), 0.0) && Passed;
  Passed = Evaluate("GetDirection()", "helpers", "The stub char* assimilator resets the direction", Event.GetDirection(), MVector(0.0, 0.0, 0.0)) && Passed;

  Event.Reset();
  Passed = Evaluate("GetEnergy()", "reset", "Reset clears the muon energy", Event.GetEnergy(), 0.0) && Passed;
  Passed = Evaluate("GetDirection()", "reset", "Reset clears the muon direction", Event.GetDirection(), MVector(0.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("GetCenterOfGravity()", "reset", "Reset clears the muon center of gravity", Event.GetCenterOfGravity(), MVector(0.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("Data()", "reset", "Data() still returns the concrete pointer after reset", Event.Data(), dynamic_cast<MPhysicalEvent*>(&Event)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test text and tra round-trips plus duplication
bool UTMuonEvent::TestRoundTrips()
{
  bool Passed = true;

  MMuonEvent Event;
  Event.SetId(8);
  Event.SetTime(MTime(4.0));
  Event.Assimilate(MVector(1.0, 0.0, 0.0), MVector(0.0, 1.0, 0.0), 2.5);
  Passed = EvaluateTrue("Validate()", "seeded muon", "The seeded muon event validates", Event.Validate()) && Passed;

  MString Tra = Event.ToTraString();
  MMuonEvent ParsedTra;
  Passed = EvaluateTrue("ParseTraString()", "muon tra", "The tra-string round-trips through ParseLine", ParseTraString(ParsedTra, Tra)) && Passed;
  Passed = Evaluate("ParsedTra.GetId()", "muon tra", "The tra round-trip preserves the event id", ParsedTra.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("ParsedTra.GetEnergy()", "muon tra", "The tra round-trip preserves the energy", ParsedTra.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("ParsedTra.GetDirection()", "muon tra", "The tra round-trip preserves the direction", ParsedTra.GetDirection(), Event.GetDirection()) && Passed;
  Passed = Evaluate("ParsedTra.GetCenterOfGravity()", "muon tra", "The tra round-trip preserves the center of gravity", ParsedTra.GetCenterOfGravity(), Event.GetCenterOfGravity()) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "muon", "Duplicate keeps the concrete muon type", dynamic_cast<MMuonEvent*>(Duplicate) != nullptr) && Passed;
  Passed = Evaluate("Duplicate()->GetEnergy()", "muon", "Duplicate preserves the muon energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Duplicate;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTMuonEvent Test;
  Test.Run();

  return 0;
}
