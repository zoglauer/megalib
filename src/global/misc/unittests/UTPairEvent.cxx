/*
 * UTPairEvent.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <fstream>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib:
#include "MPairEvent.h"
#include "MFile.h"
#include "MString.h"
#include "MUnitTest.h"


//! Unit test class for MPairEvent
class UTPairEvent : public MUnitTest
{
public:
  //! Default constructor
  UTPairEvent() : MUnitTest("UTPairEvent") {}
  //! Default destructor
  virtual ~UTPairEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Parse a tra-string line-by-line into an event object
  bool ParseTraString(MPairEvent& Event, const MString& Tra, bool Fast = false);
  //! Test basic state, validation, and helpers
  bool TestBasics();
  //! Test text and tra round-trips plus duplication
  bool TestRoundTrips();
  //! Test copy-style assimilation and file-stream round-trip
  bool TestCopyAndStream();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTPairEvent::Run()
{
  bool Passed = true;

  Passed = TestBasics() && Passed;
  Passed = TestRoundTrips() && Passed;
  Passed = TestCopyAndStream() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a tra-string line-by-line into an event object
bool UTPairEvent::ParseTraString(MPairEvent& Event, const MString& Tra, bool Fast)
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


//! Test basic state, validation, and helpers
bool UTPairEvent::TestBasics()
{
  bool Passed = true;

  MPairEvent Event;
  Passed = Evaluate("GetType()", "default", "Pair events report the correct type", Event.GetType(), MPhysicalEvent::c_Pair) && Passed;
  Passed = Evaluate("GetTypeString()", "default", "Pair events report the correct type string", Event.GetTypeString(), MString("Pair")) && Passed;
  Event.SetPairCreationIA(MVector(1.0, 2.0, 3.0));
  Event.SetElectronDirection(MVector(1.0, 0.0, 0.0));
  Event.SetPositronDirection(MVector(0.0, 1.0, 0.0));
  Event.SetEnergyElectron(100.0);
  Event.SetEnergyErrorElectron(1.0);
  Event.SetEnergyPositron(150.0);
  Event.SetEnergyErrorPositron(1.5);
  Event.SetInitialEnergyDeposit(25.0);
  Event.SetTrackQualityFactor(0.75);

  Passed = EvaluateTrue("Validate()", "pair", "A basic pair event validates successfully", Event.Validate()) && Passed;
  Passed = Evaluate("GetEnergy()", "pair", "The pair-event energy is the sum of both leptons", Event.GetEnergy(), 250.0) && Passed;
  Passed = Evaluate("GetPosition()", "pair", "The pair creation point is the event position", Event.GetPosition(), MVector(1.0, 2.0, 3.0)) && Passed;
  Passed = Evaluate("GetOrigin()", "pair", "The incoming gamma direction is stored as the origin", Event.GetOrigin(), Event.m_IncomingGammaDirection) && Passed;
  Passed = Evaluate("GetEnergyElectron()", "pair", "The electron energy is stored", Event.GetEnergyElectron(), 100.0) && Passed;
  Passed = Evaluate("GetEnergyErrorElectron()", "pair", "The electron energy error is stored", Event.GetEnergyErrorElectron(), 1.0) && Passed;
  Passed = Evaluate("GetEnergyPositron()", "pair", "The positron energy is stored", Event.GetEnergyPositron(), 150.0) && Passed;
  Passed = Evaluate("GetEnergyErrorPositron()", "pair", "The positron energy error is stored", Event.GetEnergyErrorPositron(), 1.5) && Passed;
  Passed = Evaluate("GetInitialEnergyDeposit()", "pair", "The initial deposit is stored", Event.GetInitialEnergyDeposit(), 25.0) && Passed;
  Passed = Evaluate("GetTrackQualityFactor()", "pair", "The pair track quality factor is stored", Event.GetTrackQualityFactor(), 0.75) && Passed;
  Passed = Evaluate("GetOpeningAngle()", "pair", "Orthogonal lepton directions produce a right angle", Event.GetOpeningAngle(), c_Pi / 2.0) && Passed;
  Passed = EvaluateTrue("MostProbableDirectionIncomingGamma()", "pair", "The incoming gamma direction can be estimated", Event.MostProbableDirectionIncomingGamma()) && Passed;
  Passed = Evaluate("m_IncomingGammaDirection", "pair", "The incoming gamma direction is the weighted average of the two lepton directions", Event.m_IncomingGammaDirection, MVector(-0.4, -0.6, 0.0)) && Passed;
  Passed = EvaluateNear("GetARMGamma()", "pair", "Pair ARM is zero on the incoming gamma line", Event.GetARMGamma(Event.GetPosition() + Event.GetOrigin()), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAzimuthalScatterAngle()", "pair", "Pair azimuthal scatter angle is pi/4 for a simple reference geometry", Event.GetAzimuthalScatterAngle(MVector(0.0, 0.0, 1.0)), c_Pi / 4.0, 1e-12) && Passed;
  Passed = Evaluate("Ei()", "pair", "Ei() mirrors the total pair energy", Event.Ei(), 250.0) && Passed;
  Passed = Evaluate("Data()", "pair", "Data() returns the concrete pair-event pointer", Event.Data(), static_cast<MPhysicalEvent*>(&Event)) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = Evaluate("Duplicate()->GetEnergy()", "pair", "Duplicate preserves the pair energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Duplicate()->GetType()", "pair", "Duplicate preserves the pair type", Duplicate->GetType(), Event.GetType()) && Passed;
  delete Duplicate;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test text and tra round-trips plus duplication
bool UTPairEvent::TestRoundTrips()
{
  bool Passed = true;

  MPairEvent Event;
  Event.SetId(21);
  Event.SetTime(MTime(9.0));
  Event.SetPairCreationIA(MVector(1.0, 2.0, 3.0));
  Event.SetElectronDirection(MVector(1.0, 0.0, 0.0));
  Event.SetPositronDirection(MVector(0.0, 1.0, 0.0));
  Event.SetEnergyElectron(100.0);
  Event.SetEnergyErrorElectron(1.0);
  Event.SetEnergyPositron(150.0);
  Event.SetEnergyErrorPositron(1.5);
  Event.SetInitialEnergyDeposit(25.0);
  Event.SetTrackQualityFactor(0.75);
  Passed = EvaluateTrue("Validate()", "seeded pair", "The seeded pair event validates", Event.Validate()) && Passed;

  MString Basic = Event.ToBasicString();
  Passed = Evaluate("ToBasicString()", "basic string", "The compact pair representation is formatted deterministically", Basic, MString("P;1.000000;2.000000;3.000000;1.000000;0.000000;0.000000;0.000000;1.000000;0.000000;100.000000;150.000000;1.000000;1.500000\n")) && Passed;
  MPairEvent ParsedBasic;
  string BasicStd = Basic.ToString();
  vector<char> BasicBuffer(BasicStd.begin(), BasicStd.end());
  BasicBuffer.push_back('\0');
  Passed = EvaluateTrue("Assimilate(char*)", "basic string", "The compact pair representation round-trips", ParsedBasic.Assimilate(BasicBuffer.data())) && Passed;
  Passed = Evaluate("ParsedBasic.GetEnergy()", "basic string", "The basic representation preserves the total energy", ParsedBasic.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("ParsedBasic.GetPosition()", "basic string", "The basic representation preserves the creation point", ParsedBasic.GetPosition(), Event.GetPosition()) && Passed;

  MString Tra = Event.ToTraString();
  MPairEvent ParsedTra;
  Passed = EvaluateTrue("ParseTraString()", "pair tra", "The tra-string round-trips through ParseLine", ParseTraString(ParsedTra, Tra)) && Passed;
  Passed = Evaluate("ParsedTra.GetId()", "pair tra", "The tra round-trip preserves the event id", ParsedTra.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("ParsedTra.GetEnergy()", "pair tra", "The tra round-trip preserves the total energy", ParsedTra.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("ParsedTra.GetPosition()", "pair tra", "The tra round-trip preserves the creation point", ParsedTra.GetPosition(), Event.GetPosition()) && Passed;
  Passed = Evaluate("ParsedTra.GetOpeningAngle()", "pair tra", "The tra round-trip preserves the opening angle", ParsedTra.GetOpeningAngle(), Event.GetOpeningAngle()) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "pair", "Duplicate keeps the concrete pair type", dynamic_cast<MPairEvent*>(Duplicate) != nullptr) && Passed;
  Passed = Evaluate("Duplicate()->GetEnergy()", "pair", "Duplicate preserves the pair energy", Duplicate->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Duplicate;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy-style assimilation and file-stream round-trip
bool UTPairEvent::TestCopyAndStream()
{
  bool Passed = true;

  MPairEvent Event;
  Event.SetId(19);
  Event.SetTime(MTime(11.0));
  Event.SetPairCreationIA(MVector(1.0, 2.0, 3.0));
  Event.SetElectronDirection(MVector(1.0, 0.0, 0.0));
  Event.SetPositronDirection(MVector(0.0, 1.0, 0.0));
  Event.SetEnergyElectron(100.0);
  Event.SetEnergyErrorElectron(1.0);
  Event.SetEnergyPositron(150.0);
  Event.SetEnergyErrorPositron(1.5);
  Event.SetInitialEnergyDeposit(25.0);
  Event.SetTrackQualityFactor(0.75);
  Passed = EvaluateTrue("Validate()", "seeded pair", "The seeded pair event validates", Event.Validate()) && Passed;

  MPairEvent Copied;
  Passed = EvaluateTrue("Assimilate(MPairEvent*)", "copy", "Copy-style assimilation from the concrete type works", Copied.Assimilate(&Event)) && Passed;
  Passed = Evaluate("Copied.GetEnergy()", "copy", "Copy-style assimilation preserves the total energy", Copied.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Copied.GetOpeningAngle()", "copy", "Copy-style assimilation preserves the opening angle", Copied.GetOpeningAngle(), Event.GetOpeningAngle()) && Passed;

  MPhysicalEvent* Generic = new MPairEvent();
  Passed = EvaluateTrue("Assimilate(MPhysicalEvent*)", "copy", "Copy-style assimilation from a physical pointer works", Generic->Assimilate(&Event)) && Passed;
  Passed = Evaluate("Generic->GetEnergy()", "copy", "Base-pointer assimilation preserves the total energy", Generic->GetEnergy(), Event.GetEnergy()) && Passed;
  delete Generic;

  MFile File;
  MString FileName = "/tmp/UTPairEvent.tra";
  Passed = EvaluateTrue("Open()", "stream write", "The temporary pair file can be opened for writing", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream write", "The pair event write-stream completes at EOF", Event.Stream(File, 0, false, false, false)) && Passed;
  File.Close();

  MPairEvent Streamed;
  Passed = EvaluateTrue("Open()", "stream read", "The temporary pair file can be reopened for reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream read", "The pair event read-stream also returns false at EOF", Streamed.Stream(File, 0, true, false, false)) && Passed;
  Passed = Evaluate("Streamed.GetId()", "stream read", "The streamed copy preserves the event id", Streamed.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("Streamed.GetEnergy()", "stream read", "The streamed copy preserves the total energy", Streamed.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Streamed.GetPosition()", "stream read", "The streamed copy preserves the creation point", Streamed.GetPosition(), Event.GetPosition()) && Passed;
  File.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTPairEvent Test;
  Test.Run();

  return 0;
}
