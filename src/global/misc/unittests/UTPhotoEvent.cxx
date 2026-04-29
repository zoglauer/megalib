/*
 * UTPhotoEvent.cxx
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
#include "MPhotoEvent.h"
#include "MFile.h"
#include "MPhysicalEvent.h"
#include "MString.h"
#include "MUnitTest.h"


//! Unit test class for MPhotoEvent
class UTPhotoEvent : public MUnitTest
{
public:
  //! Default constructor
  UTPhotoEvent() : MUnitTest("UTPhotoEvent") {}
  //! Default destructor
  virtual ~UTPhotoEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Parse a tra-string line-by-line into an event object
  bool ParseTraString(MPhotoEvent& Event, const MString& Tra, bool Fast = false);
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
bool UTPhotoEvent::Run()
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
bool UTPhotoEvent::ParseTraString(MPhotoEvent& Event, const MString& Tra, bool Fast)
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
bool UTPhotoEvent::TestBasics()
{
  bool Passed = true;

  MPhotoEvent Event;
  Passed = Evaluate("GetType()", "default", "Photo events report the correct type", Event.GetType(), MPhysicalEvent::c_Photo) && Passed;
  Passed = Evaluate("GetTypeString()", "default", "Photo events report the correct type string", Event.GetTypeString(), MString("Photo")) && Passed;

  Passed = EvaluateTrue("Assimilate(MVector,Energy,Weight)", "photo", "A valid photo event can be created from basic inputs", Event.Assimilate(MVector(1.0, 2.0, 3.0), 5.0, 0.25)) && Passed;
  Passed = Evaluate("GetEnergy()", "photo", "The photo-event energy is stored", Event.GetEnergy(), 5.0) && Passed;
  Passed = Evaluate("GetPosition()", "photo", "The photo-event position is stored", Event.GetPosition(), MVector(1.0, 2.0, 3.0)) && Passed;
  Passed = Evaluate("GetWeight()", "photo", "The photo-event weight is stored", Event.GetWeight(), 0.25) && Passed;
  Passed = EvaluateTrue("Validate()", "photo", "A positive-energy photo event validates successfully", Event.Validate()) && Passed;

  MPhotoEvent Interior;
  Passed = EvaluateTrue("Assimilate(MVector,Energy,Weight)", "photo interior", "A representative interior photo event can also be created from nontrivial inputs", Interior.Assimilate(MVector(-1.25, 2.5, 0.375), 7.75, 0.625)) && Passed;
  Passed = Evaluate("GetEnergy()", "photo interior", "Interior photo energies are stored exactly", Interior.GetEnergy(), 7.75) && Passed;
  Passed = Evaluate("GetPosition()", "photo interior", "Interior photo positions are stored exactly", Interior.GetPosition(), MVector(-1.25, 2.5, 0.375)) && Passed;
  Passed = Evaluate("GetWeight()", "photo interior", "Interior photo weights are stored exactly", Interior.GetWeight(), 0.625) && Passed;
  Passed = EvaluateTrue("Validate()", "photo interior", "Representative interior photo events validate successfully", Interior.Validate()) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = Evaluate("Duplicate()->GetEnergy()", "photo", "Duplicate preserves the photo energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Duplicate()->GetType()", "photo", "Duplicate preserves the photo type", Duplicate->GetType(), Event.GetType()) && Passed;
  delete Duplicate;

  MPhotoEvent Invalid;
  Passed = EvaluateFalse("Validate()", "photo negative energy", "Negative energies are rejected", Invalid.Assimilate(MVector(0.0, 0.0, 0.0), -1.0, 1.0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy-style assimilation and file-stream round-trip
bool UTPhotoEvent::TestCopyAndStream()
{
  bool Passed = true;

  MPhotoEvent Event;
  Event.SetId(12);
  Event.SetTime(MTime(8.5));
  Event.Assimilate(MVector(1.0, 2.0, 3.0), 5.0, 0.25);
  Passed = EvaluateTrue("Validate()", "seeded photo", "The seeded photo event validates", Event.Validate()) && Passed;

  MPhotoEvent Copied;
  Passed = EvaluateTrue("Assimilate(MPhotoEvent*)", "copy", "Copy-style assimilation from the concrete type works", Copied.Assimilate(&Event)) && Passed;
  Passed = Evaluate("Copied.GetEnergy()", "copy", "Copy-style assimilation preserves the energy", Copied.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Copied.GetPosition()", "copy", "Copy-style assimilation preserves the position", Copied.GetPosition(), Event.GetPosition()) && Passed;

  MPhysicalEvent* Generic = new MPhotoEvent();
  Passed = EvaluateTrue("Assimilate(MPhysicalEvent*)", "copy", "Copy-style assimilation from a physical pointer works", Generic->Assimilate(&Event)) && Passed;
  Passed = Evaluate("Generic->GetEnergy()", "copy", "Base-pointer assimilation preserves the energy", Generic->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Generic;

  MString Basic = Event.ToString();
  Passed = EvaluateTrue("Assimilate(char*)", "basic line", "The basic text assimilation path is exercised", Event.Assimilate((char*) Basic.Data())) && Passed;

  MFile File;
  MString FileName = "/tmp/UTPhotoEvent.tra";
  Passed = EvaluateTrue("Open()", "stream write", "The temporary photo file can be opened for writing", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream write", "The photo event write-stream completes at EOF", Event.Stream(File, 0, false, false, false)) && Passed;
  File.Close();

  MPhotoEvent Streamed;
  Passed = EvaluateTrue("Open()", "stream read", "The temporary photo file can be reopened for reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream read", "The photo event read-stream also returns false at EOF", Streamed.Stream(File, 0, true, false, false)) && Passed;
  Passed = Evaluate("Streamed.GetId()", "stream read", "The streamed copy preserves the event id", Streamed.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("Streamed.GetEnergy()", "stream read", "The streamed copy preserves the energy", Streamed.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Streamed.GetPosition()", "stream read", "The streamed copy preserves the position", Streamed.GetPosition(), Event.GetPosition()) && Passed;
  File.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test helper methods and reset behavior
bool UTPhotoEvent::TestHelpers()
{
  bool Passed = true;

  MPhotoEvent Event;
  Event.SetEnergy(7.5);
  Event.SetPosition(MVector(1.0, 2.0, 3.0));
  Event.SetWeight(0.5);
  Passed = EvaluateTrue("Validate()", "helpers", "The seeded photo helper validates", Event.Validate()) && Passed;
  Passed = Evaluate("ParseLine()", "helpers energy", "ParseLine stores photo energies from representative PE lines", Event.ParseLine("PE 8.25", false), 0) && Passed;
  Passed = Evaluate("GetEnergy()", "helpers energy", "ParseLine stores the parsed photo energy", Event.GetEnergy(), 8.25) && Passed;
  Passed = Evaluate("ParseLine()", "helpers position", "ParseLine stores photo positions from representative PP lines", Event.ParseLine("PP 4 5 6", false), 0) && Passed;
  Passed = Evaluate("GetPosition()", "helpers position", "ParseLine stores the parsed photo position", Event.GetPosition(), MVector(4.0, 5.0, 6.0)) && Passed;
  Passed = Evaluate("ParseLine()", "helpers weight", "ParseLine stores photo weights from representative PW lines", Event.ParseLine("PW 0.75", false), 0) && Passed;
  Passed = Evaluate("GetWeight()", "helpers weight", "ParseLine stores the parsed photo weight", Event.GetWeight(), 0.75) && Passed;
  Event.SetEnergy(7.5);
  Event.SetPosition(MVector(1.0, 2.0, 3.0));
  Event.SetWeight(0.5);

  Passed = Evaluate("Data()", "helpers", "Data() returns the concrete photo pointer", Event.Data(), dynamic_cast<MPhysicalEvent*>(&Event)) && Passed;
  Passed = Evaluate("ToString()", "helpers", "The photo string representation is formatted deterministically", Event.ToString(), MString("The data of the Photo-event:\nEnergy: 7.500\nPosition: 1.000, 2.000, 3.000\nWeight: 0.500\n")) && Passed;

  MString Basic = Event.ToString();
  Passed = EvaluateTrue("Assimilate(char*)", "helpers", "The stub char* assimilator returns true", Event.Assimilate((char*) Basic.Data())) && Passed;
  Passed = Evaluate("GetEnergy()", "helpers", "The stub char* assimilator resets the energy", Event.GetEnergy(), 0.0) && Passed;
  Passed = Evaluate("GetWeight()", "helpers", "The stub char* assimilator resets the weight", Event.GetWeight(), 0.0) && Passed;

  Event.Reset();
  Passed = Evaluate("GetEnergy()", "reset", "Reset clears the photo energy", Event.GetEnergy(), 0.0) && Passed;
  Passed = Evaluate("GetWeight()", "reset", "Reset clears the photo weight", Event.GetWeight(), 0.0) && Passed;
  Passed = Evaluate("GetPosition()", "reset", "Reset clears the photo position", Event.GetPosition(), MVector(0.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("Data()", "reset", "Data() still returns the concrete pointer after reset", Event.Data(), dynamic_cast<MPhysicalEvent*>(&Event)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test text and tra round-trips plus duplication
bool UTPhotoEvent::TestRoundTrips()
{
  bool Passed = true;

  MPhotoEvent Event;
  Event.SetId(12);
  Event.SetTime(MTime(8.5));
  Event.Assimilate(MVector(1.0, 2.0, 3.0), 5.0, 0.25);
  Passed = EvaluateTrue("Validate()", "seeded photo", "The seeded photo event validates", Event.Validate()) && Passed;

  MString Tra = Event.ToTraString();
  MString ExpectedTra = MString("ET PH\nID 0\nTI ") + Event.GetTime().GetLongIntsString() + "\nPE 5\nPP 1 2 3\nPW 0.25\n";
  Passed = Evaluate("ToTraString()", "photo tra exact", "The tra-string serialization is deterministic for a representative photo event", Tra, ExpectedTra) && Passed;
  MPhotoEvent ParsedTra;
  Passed = EvaluateTrue("ParseTraString()", "photo tra", "The tra-string round-trips through ParseLine", ParseTraString(ParsedTra, Tra)) && Passed;
  Passed = Evaluate("ParsedTra.GetId()", "photo tra", "The tra round-trip preserves the event id", ParsedTra.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("ParsedTra.GetEnergy()", "photo tra", "The tra round-trip preserves the energy", ParsedTra.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("ParsedTra.GetPosition()", "photo tra", "The tra round-trip preserves the position", ParsedTra.GetPosition(), Event.GetPosition()) && Passed;
  Passed = Evaluate("ParsedTra.GetWeight()", "photo tra", "The tra round-trip preserves the weight", ParsedTra.GetWeight(), Event.GetWeight()) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "photo", "Duplicate keeps the concrete photo type", dynamic_cast<MPhotoEvent*>(Duplicate) != nullptr) && Passed;
  Passed = Evaluate("Duplicate()->GetEnergy()", "photo", "Duplicate preserves the photo energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Duplicate;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTPhotoEvent Test;
  return Test.Run() == true ? 0 : 1;
}
