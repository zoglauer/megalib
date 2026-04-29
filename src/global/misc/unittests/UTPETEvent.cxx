/*
 * UTPETEvent.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib:
#include "MPETEvent.h"
#include "MFile.h"
#include "MPhysicalEvent.h"
#include "MString.h"
#include "MUnitTest.h"


//! Unit test class for MPETEvent
class UTPETEvent : public MUnitTest
{
public:
  //! Default constructor
  UTPETEvent() : MUnitTest("UTPETEvent") {}
  //! Default destructor
  virtual ~UTPETEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Parse a tra-string line-by-line into an event object
  bool ParseTraString(MPETEvent& Event, const MString& Tra, bool Fast = false);
  //! Test basic state and validation
  bool TestBasics();
  //! Test text and tra round-trips plus duplication
  bool TestRoundTrips();
  //! Test copy-style assimilation and file-stream round-trip
  bool TestCopyAndStream();
  //! Test helper methods and rejection branches
  bool TestHelpers();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTPETEvent::Run()
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
bool UTPETEvent::ParseTraString(MPETEvent& Event, const MString& Tra, bool Fast)
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
bool UTPETEvent::TestBasics()
{
  bool Passed = true;

  MPETEvent Event;
  Passed = Evaluate("GetType()", "default", "PET events report the correct type", Event.GetType(), MPhysicalEvent::c_PET) && Passed;
  Passed = Evaluate("GetTypeString()", "default", "PET events report the correct type string", Event.GetTypeString(), MString("PET")) && Passed;

  Event.SetEnergy1(511.0);
  Event.SetEnergyResolution1(5.0);
  Event.SetPosition1(MVector(1.0, 2.0, 3.0));
  Event.SetPositionResolution1(MVector(0.1, 0.2, 0.3));
  Event.SetTiming1(1.5);
  Event.SetTimingResolution1(0.05);

  Event.SetEnergy2(512.0);
  Event.SetEnergyResolution2(6.0);
  Event.SetPosition2(MVector(4.0, 5.0, 6.0));
  Event.SetPositionResolution2(MVector(0.4, 0.5, 0.6));
  Event.SetTiming2(2.5);
  Event.SetTimingResolution2(0.06);

  Passed = EvaluateTrue("Validate()", "PET valid", "Positive PET inputs validate successfully", Event.Validate()) && Passed;
  Passed = Evaluate("GetEnergy()", "PET valid", "The PET energy is the sum of both hits", Event.GetEnergy(), 1023.0) && Passed;
  Passed = Evaluate("GetEnergy1()", "PET valid", "The first PET energy is stored", Event.GetEnergy1(), 511.0) && Passed;
  Passed = Evaluate("GetEnergy2()", "PET valid", "The second PET energy is stored", Event.GetEnergy2(), 512.0) && Passed;
  Passed = Evaluate("GetPosition1()", "PET valid", "The first PET hit position is stored", Event.GetPosition1(), MVector(1.0, 2.0, 3.0)) && Passed;
  Passed = Evaluate("GetPosition2()", "PET valid", "The second PET hit position is stored", Event.GetPosition2(), MVector(4.0, 5.0, 6.0)) && Passed;
  Passed = Evaluate("GetEnergyResolution1()", "PET valid", "The first PET energy resolution is stored", Event.GetEnergyResolution1(), 5.0) && Passed;
  Passed = Evaluate("GetEnergyResolution2()", "PET valid", "The second PET energy resolution is stored", Event.GetEnergyResolution2(), 6.0) && Passed;
  Passed = Evaluate("GetTiming1()", "PET valid", "The first PET timing is stored", Event.GetTiming1(), 1.5) && Passed;
  Passed = Evaluate("GetTiming2()", "PET valid", "The second PET timing is stored", Event.GetTiming2(), 2.5) && Passed;
  Passed = Evaluate("GetTimingResolution1()", "PET valid", "The first PET timing resolution is stored", Event.GetTimingResolution1(), 0.05) && Passed;
  Passed = Evaluate("GetTimingResolution2()", "PET valid", "The second PET timing resolution is stored", Event.GetTimingResolution2(), 0.06) && Passed;
  Passed = Evaluate("GetPositionResolution1()", "PET valid", "The first PET position resolution is stored", Event.GetPositionResolution1(), MVector(0.1, 0.2, 0.3)) && Passed;
  Passed = Evaluate("GetPositionResolution2()", "PET valid", "The second PET position resolution is stored", Event.GetPositionResolution2(), MVector(0.4, 0.5, 0.6)) && Passed;

  MPETEvent Interior;
  Interior.SetEnergy1(300.5);
  Interior.SetEnergyResolution1(3.25);
  Interior.SetPosition1(MVector(-1.5, 2.25, 0.75));
  Interior.SetPositionResolution1(MVector(0.12, 0.23, 0.34));
  Interior.SetTiming1(1.234);
  Interior.SetTimingResolution1(0.015);
  Interior.SetEnergy2(455.25);
  Interior.SetEnergyResolution2(4.75);
  Interior.SetPosition2(MVector(3.5, -4.25, 1.125));
  Interior.SetPositionResolution2(MVector(0.21, 0.32, 0.43));
  Interior.SetTiming2(2.468);
  Interior.SetTimingResolution2(0.025);
  Passed = EvaluateTrue("Validate()", "PET interior", "Representative interior PET inputs validate successfully", Interior.Validate()) && Passed;
  Passed = Evaluate("GetEnergy()", "PET interior", "Interior PET energies still sum correctly", Interior.GetEnergy(), 755.75) && Passed;
  Passed = Evaluate("GetPosition1()", "PET interior", "Interior first PET hit positions are stored exactly", Interior.GetPosition1(), MVector(-1.5, 2.25, 0.75)) && Passed;
  Passed = Evaluate("GetPosition2()", "PET interior", "Interior second PET hit positions are stored exactly", Interior.GetPosition2(), MVector(3.5, -4.25, 1.125)) && Passed;
  Passed = Evaluate("GetTiming1()", "PET interior", "Interior first PET timings are stored exactly", Interior.GetTiming1(), 1.234) && Passed;
  Passed = Evaluate("GetTiming2()", "PET interior", "Interior second PET timings are stored exactly", Interior.GetTiming2(), 2.468) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = Evaluate("Duplicate()->GetEnergy()", "PET duplicate", "Duplicate preserves the PET energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Duplicate()->GetType()", "PET duplicate", "Duplicate preserves the PET type", Duplicate->GetType(), Event.GetType()) && Passed;
  delete Duplicate;

  MPETEvent InvalidTiming;
  InvalidTiming.SetEnergy1(511.0);
  InvalidTiming.SetEnergyResolution1(5.0);
  InvalidTiming.SetPosition1(MVector(1.0, 2.0, 3.0));
  InvalidTiming.SetPositionResolution1(MVector(0.1, 0.2, 0.3));
  InvalidTiming.SetTiming1(1.5);
  InvalidTiming.SetTimingResolution1(0.05);
  InvalidTiming.SetEnergy2(512.0);
  InvalidTiming.SetEnergyResolution2(6.0);
  InvalidTiming.SetPosition2(MVector(4.0, 5.0, 6.0));
  InvalidTiming.SetPositionResolution2(MVector(0.4, 0.5, 0.6));
  InvalidTiming.SetTiming2(2.5);
  InvalidTiming.SetTimingResolution2(-0.06);
  Passed = EvaluateFalse("Validate()", "PET negative timing2 resolution", "Negative timing resolution for the second hit must be rejected", InvalidTiming.Validate()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy-style assimilation and file-stream round-trip
bool UTPETEvent::TestCopyAndStream()
{
  bool Passed = true;

  MPETEvent Event;
  Event.SetId(4);
  Event.SetTime(MTime(3.0));
  Event.SetEnergy1(511.0);
  Event.SetEnergyResolution1(5.0);
  Event.SetPosition1(MVector(1.0, 2.0, 3.0));
  Event.SetPositionResolution1(MVector(0.1, 0.2, 0.3));
  Event.SetTiming1(1.5);
  Event.SetTimingResolution1(0.05);
  Event.SetEnergy2(512.0);
  Event.SetEnergyResolution2(6.0);
  Event.SetPosition2(MVector(4.0, 5.0, 6.0));
  Event.SetPositionResolution2(MVector(0.4, 0.5, 0.6));
  Event.SetTiming2(2.5);
  Event.SetTimingResolution2(0.06);
  Passed = EvaluateTrue("Validate()", "seeded PET", "The seeded PET event validates", Event.Validate()) && Passed;

  MPETEvent Copied;
  Passed = EvaluateTrue("Assimilate(MPETEvent*)", "copy", "Copy-style assimilation from the concrete type works", Copied.Assimilate(&Event)) && Passed;
  Passed = Evaluate("Copied.GetEnergy()", "copy", "Copy-style assimilation preserves the energy", Copied.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Copied.GetTiming2()", "copy", "Copy-style assimilation preserves the second timing", Copied.GetTiming2(), Event.GetTiming2()) && Passed;

  MPhysicalEvent* Generic = new MPETEvent();
  Passed = EvaluateTrue("Assimilate(MPhysicalEvent*)", "copy", "Copy-style assimilation from a physical pointer works", Generic->Assimilate(&Event)) && Passed;
  Passed = Evaluate("Generic->GetEnergy()", "copy", "Base-pointer assimilation preserves the energy", Generic->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Generic;

  Passed = EvaluateNear("GetResolutionMeasure()", "PET", "A point on the detector line has zero resolution measure", Event.GetResolutionMeasure(MVector(2.0, 3.0, 4.0)), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetResolutionMeasure()", "PET off line", "A representative point away from the detector line has a positive resolution measure", Event.GetResolutionMeasure(MVector(2.0, 3.5, 4.0)) > 0.0) && Passed;

  MFile File;
  MString FileName = "/tmp/UTPETEvent.tra";
  Passed = EvaluateTrue("Open()", "stream write", "The temporary PET file can be opened for writing", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream write", "The PET event write-stream completes at EOF", Event.Stream(File, 0, false, false, false)) && Passed;
  File.Close();

  MPETEvent Streamed;
  Passed = EvaluateTrue("Open()", "stream read", "The temporary PET file can be reopened for reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream read", "The PET event read-stream also returns false at EOF", Streamed.Stream(File, 0, true, false, false)) && Passed;
  Passed = Evaluate("Streamed.GetId()", "stream read", "The streamed copy preserves the event id", Streamed.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("Streamed.GetEnergy()", "stream read", "The streamed copy preserves the energy", Streamed.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Streamed.GetTiming2()", "stream read", "The streamed copy preserves the second timing", Streamed.GetTiming2(), Event.GetTiming2()) && Passed;
  File.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test helper methods and rejection branches
bool UTPETEvent::TestHelpers()
{
  bool Passed = true;

  MPETEvent Event;
  Event.SetId(9);
  Event.SetEnergy1(510.0);
  Event.SetEnergyResolution1(4.0);
  Event.SetPosition1(MVector(1.0, 0.0, 0.0));
  Event.SetPositionResolution1(MVector(0.1, 0.1, 0.1));
  Event.SetTiming1(1.0);
  Event.SetTimingResolution1(0.01);
  Event.SetEnergy2(512.0);
  Event.SetEnergyResolution2(6.0);
  Event.SetPosition2(MVector(0.0, 1.0, 0.0));
  Event.SetPositionResolution2(MVector(0.2, 0.2, 0.2));
  Event.SetTiming2(2.0);
  Event.SetTimingResolution2(0.02);
  Passed = EvaluateTrue("Validate()", "helpers", "The PET helper event validates", Event.Validate()) && Passed;

  Passed = Evaluate("Data()", "helpers", "Data() returns the concrete event pointer", Event.Data(), dynamic_cast<MPhysicalEvent*>(&Event)) && Passed;
  Passed = Evaluate("ToString()", "helpers", "The PET string representation is formatted deterministically", Event.ToString(), MString("Position-emmision-tomography event:\n  Energy1: 510.000 +- 4.000 \n  Position1: 1.000, 0.000, 0.000  +- 0.100 0.100  0.100 \n  Timing1: 1.000000000 +- 0.010 \n  Energy2: 512.000 +- 6.000 \n  Position2: 0.000, 1.000, 0.000 +- 0.200  0.200  0.200 \n  Timing2: 2.000000000 +- 0.020000000 \n")) && Passed;

  Event.Reset();
  Passed = EvaluateFalse("Validate()", "reset", "Reset PET events are no longer valid", Event.Validate()) && Passed;
  Passed = Evaluate("Data()", "reset", "Data() still returns the concrete pointer after reset", Event.Data(), dynamic_cast<MPhysicalEvent*>(&Event)) && Passed;
  Passed = Evaluate("GetEnergy()", "reset", "Reset clears the PET energies", Event.GetEnergy(), 0.0) && Passed;

  DisableDefaultStreams();
  Passed = Evaluate("GetResolutionMeasure()", "reject", "The galactic coordinate-system rejection returns the undefined value", Event.GetResolutionMeasure(MVector(1.0, 2.0, 3.0), MCoordinateSystem::c_Galactic), g_DoubleNotDefined) && Passed;
  EnableDefaultStreams();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test text and tra round-trips plus duplication
bool UTPETEvent::TestRoundTrips()
{
  bool Passed = true;

  MPETEvent Event;
  Event.SetId(4);
  Event.SetTime(MTime(3.0));
  Event.SetEnergy1(511.0);
  Event.SetEnergyResolution1(5.0);
  Event.SetPosition1(MVector(1.0, 2.0, 3.0));
  Event.SetPositionResolution1(MVector(0.1, 0.2, 0.3));
  Event.SetTiming1(1.5);
  Event.SetTimingResolution1(0.05);
  Event.SetEnergy2(512.0);
  Event.SetEnergyResolution2(6.0);
  Event.SetPosition2(MVector(4.0, 5.0, 6.0));
  Event.SetPositionResolution2(MVector(0.4, 0.5, 0.6));
  Event.SetTiming2(2.5);
  Event.SetTimingResolution2(0.06);
  Passed = EvaluateTrue("Validate()", "seeded PET", "The seeded PET event validates", Event.Validate()) && Passed;

  MString Tra = Event.ToTraString();
  MPETEvent ParsedTra;
  Passed = EvaluateTrue("ParseTraString()", "PET tra", "The tra-string round-trips through ParseLine", ParseTraString(ParsedTra, Tra)) && Passed;
  Passed = Evaluate("ParsedTra.GetId()", "PET tra", "The tra round-trip preserves the event id", ParsedTra.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("ParsedTra.GetEnergy()", "PET tra", "The tra round-trip preserves the total energy", ParsedTra.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("ParsedTra.GetTiming2()", "PET tra", "The tra round-trip preserves the second timing", ParsedTra.GetTiming2(), Event.GetTiming2()) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "PET", "Duplicate keeps the concrete PET type", dynamic_cast<MPETEvent*>(Duplicate) != nullptr) && Passed;
  Passed = Evaluate("Duplicate()->GetEnergy()", "PET", "Duplicate preserves the PET energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Duplicate;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTPETEvent Test;
  return Test.Run() == true ? 0 : 1;
}
