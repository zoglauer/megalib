/*
 * UTFileEventsSim.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


// Standard libs:
#include <sstream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TSystem.h"

// MEGAlib:
#include "MFileEventsSim.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for MFileEventsSim
class UTFileEventsSim : public MUnitTest
{
public:
  //! Default constructor
  UTFileEventsSim() : MUnitTest("UTFileEventsSim") {}
  //! Default destructor
  virtual ~UTFileEventsSim() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test open guards and geometry requirements
  bool TestOpenAndGuards();
  //! Test reading the committed sim fixtures
  bool TestReadFixtures();
  //! Test include-file observation time and simulated-event summing
  bool TestIncludeObservationTime();
  //! Test write-side header/footer contract
  bool TestWriteHeaderAndFooter();
  //! Test binary round-trip and rewind behavior
  bool TestBinaryRoundTripAndRewind();
  //! Test the AT keyword (accumulated include times)
  bool TestAccumulatedIncludeTimes();

  //! Write an include master for the three observation-time fixtures with an extra header line
  bool WriteIncludeMaster(const MString& FileName, const MString& ExtraHeaderLine);
  //! Read all event times of a sim file
  vector<double> ReadEventTimes(MDGeometryQuest& Geometry, const MString& FileName);

  //! Return the temp directory
  MString GetTempDirectory() const;
  //! Return the shared data directory
  MString GetDataDirectory() const;
  //! Return the test geometry file
  MString GetGeometryFileName() const;
  //! Prepare the temp directory
  bool PrepareTempDirectory() const;
  //! Scan the quest geometry
  bool LoadGeometry(MDGeometryQuest& Geometry) const;
};


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::Run()
{
  bool Passed = true;

  Passed = TestOpenAndGuards() && Passed;
  Passed = TestReadFixtures() && Passed;
  Passed = TestIncludeObservationTime() && Passed;
  Passed = TestWriteHeaderAndFooter() && Passed;
  Passed = TestBinaryRoundTripAndRewind() && Passed;
  Passed = TestAccumulatedIncludeTimes() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsSim::GetTempDirectory() const
{
  return GetTemporaryDirectoryName("files");
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsSim::GetDataDirectory() const
{
  return MString(gSystem->pwd()) + "/src/global/misc/unittests/data/UTFileEventsTra";
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsSim::GetGeometryFileName() const
{
  return MString(gSystem->pwd()) + "/resource/examples/geomega/special/Max.geo.setup";
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::PrepareTempDirectory() const
{
  return PrepareTemporaryDirectory("files");
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::LoadGeometry(MDGeometryQuest& Geometry) const
{
  return Geometry.ScanSetupFile(GetGeometryFileName());
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestOpenAndGuards()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim temp dir", "The temporary directory for MFileEventsSim tests can be created", PrepareTempDirectory()) && Passed;
  Passed = EvaluateTrue("AccessPathName()", "sim geometry fixture", "The Max geometry fixture exists", gSystem->AccessPathName(GetGeometryFileName()) == false) && Passed;

  MString SimFixture = GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz";
  Passed = EvaluateTrue("AccessPathName()", "sim fixture", "The shared 1-second sim fixture exists", gSystem->AccessPathName(SimFixture) == false) && Passed;

  {
    MFileEventsSim Reader;
    mout.Enable(false);
    bool Opened = Reader.Open(SimFixture);
    mout.Enable(true);
    Passed = EvaluateFalse("Open()", "sim missing geometry", "Opening requires a geometry pointer", Opened) && Passed;
  }

  {
    MDGeometryQuest Geometry;
    MFileEventsSim Reader(&Geometry);
    mout.Enable(false);
    bool Opened = Reader.Open(SimFixture);
    mout.Enable(true);
    Passed = EvaluateFalse("Open()", "sim unscanned geometry", "Opening requires a scanned geometry", Opened) && Passed;
  }

  {
    MDGeometryQuest Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "sim scan geometry", "The quest geometry can be scanned from the fixture", LoadGeometry(Geometry)) && Passed;
    MFileEventsSim Reader(&Geometry);
    mgui.Enable(false);
    bool Opened = Reader.Open(GetTempDirectory() + "/invalid.txt");
    mgui.Enable(true);
    Passed = EvaluateFalse("Open()", "sim invalid file", "Opening a non-sim file fails", Opened) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestReadFixtures()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim read scan geometry", "The quest geometry can be scanned for read tests", LoadGeometry(Geometry)) && Passed;

  MString SimFixture = GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz";
  MFileEventsSim Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim read open", "The 1-second sim fixture opens in the sim reader", Reader.Open(SimFixture)) && Passed;

  Passed = EvaluateNear("GetObservationTime()", "sim direct observation time", "Observation time can be read directly from the footer", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "sim direct simulated events", "The total simulated-event count is read from TS", Reader.GetSimulatedEvents(), 310L) && Passed;

  MSimEvent* Event = Reader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim first event", "The first simulation event can be read from the fixture", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetID()", "sim first event id", "The first simulation event has the expected id", Event->GetID(), 1L) && Passed;
    Passed = EvaluateNear("GetTime()", "sim first event time", "The first simulation event has the expected time", Event->GetTime().GetAsDouble(), 0.011003748, 1e-12) && Passed;
    Passed = Evaluate("GetNIAs()", "sim first event ia count", "The first simulation event has the expected IA count", Event->GetNIAs(), 7U) && Passed;
    Passed = Evaluate("GetNHTs()", "sim first event ht count", "The first simulation event has the expected HT count", Event->GetNHTs(), 4U) && Passed;
    delete Event;
  }

  long EventCount = 0;
  while ((Event = Reader.GetNextEvent()) != nullptr) {
    ++EventCount;
    delete Event;
  }

  // Expected: 21 SE blocks (zcat | grep -c '^SE') minus the first one read above
  Passed = Evaluate("GetNextEvent()", "sim full scan count", "Scanning the rest of the sim fixture yields the 20 remaining simulation events", EventCount, 20L) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "sim full scan observation time", "Observation time remains correct after scanning all simulation events", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;

  Passed = EvaluateTrue("Rewind()", "sim rewind", "The sim reader can rewind to the start of the file", Reader.Rewind()) && Passed;
  Event = Reader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim rewind first event", "After rewinding, the first event can be read again", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetID()", "sim rewind first event id", "After rewinding, the first event id matches the original first event", Event->GetID(), 1L) && Passed;
    Passed = EvaluateNear("GetTime()", "sim rewind first event time", "After rewinding, the first event time matches the original first event", Event->GetTime().GetAsDouble(), 0.011003748, 1e-12) && Passed;
    delete Event;
  }

  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestIncludeObservationTime()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim include temp dir", "The temporary directory can be recreated for sim include tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim include scan geometry", "The quest geometry can be scanned for include tests", LoadGeometry(Geometry)) && Passed;

  MString Sim1 = GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz";
  MString Sim2 = GetDataDirectory() + "/ObsTime_2sec_complete.inc1.id1.sim.gz";
  MString Sim4 = GetDataDirectory() + "/ObsTime_4sec_complete.inc1.id1.sim.gz";
  MString IncludeFile = GetTempDirectory() + "/ObsTime_Xsec_complete.sim";

  ostringstream Content;
  Content<<"Type       sim"<<endl;
  Content<<"Version    101"<<endl;
  Content<<"Geometry   "<<GetGeometryFileName()<<endl;
  Content<<endl;
  Content<<"Date       2026-04-20 00:00:00"<<endl;
  Content<<"MEGAlib    "<<g_VersionString<<endl;
  Content<<endl;
  Content<<"IN "<<Sim1<<endl;
  Content<<"IN "<<Sim2<<endl;
  Content<<"IN "<<Sim4<<endl;
  Content<<"EN"<<endl;
  Content<<endl;

  Passed = EvaluateTrue("WriteTextFile()", "sim include fixture", "The temporary include sim file can be written", WriteTextFile(IncludeFile, Content.str().c_str())) && Passed;

  MFileEventsSim Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim include open", "The include sim file opens in the sim reader", Reader.Open(IncludeFile)) && Passed;

  long EventCount = 0;
  MSimEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent()) != nullptr) {
    ++EventCount;
    delete Event;
  }

  // Expected: 21 + 31 + 73 SE blocks in the children (zcat | grep -c '^SE')
  Passed = Evaluate("GetNextEvent()", "sim include event count", "The include sim file yields the 21 + 31 + 73 = 125 simulation events of its children", EventCount, 125L) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "sim include observation time", "Observation time sums across included sim files", Reader.GetObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "sim include simulated events", "Simulated-event counts sum across included sim files", Reader.GetSimulatedEvents(), 2225L) && Passed;

  Passed = EvaluateTrue("Rewind()", "sim include rewind", "The include sim file can be rewound", Reader.Rewind()) && Passed;
  Event = Reader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim include rewind first event", "After rewinding, the include sim file can be read again from the start", Event != nullptr) && Passed;
  if (Event != nullptr) {
    delete Event;
  }

  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestWriteHeaderAndFooter()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim write temp dir", "The temporary directory can be recreated for sim write tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim write scan geometry", "The quest geometry can be scanned for sim write tests", LoadGeometry(Geometry)) && Passed;

  MString FileName = GetTempDirectory() + "/footer_only.sim";

  {
    MFileEventsSim Writer(&Geometry);
    Passed = EvaluateTrue("Open()", "sim write open", "The footer-only sim file opens in write mode", Writer.Open(FileName, MFile::c_Write)) && Passed;
    Writer.SetGeometryFileName(GetGeometryFileName());
    Writer.SetStartObservationTime(MTime(1.25));
    Writer.SetEndObservationTime(MTime(4.75));
    Writer.SetObservationTime(MTime(3.5));
    Writer.SetSimulatedEvents(12);
    Passed = EvaluateTrue("WriteHeader()", "sim write header", "The sim header can be written", Writer.WriteHeader()) && Passed;
    Passed = EvaluateTrue("CloseEventList()", "sim write footer", "The sim footer can be written without any events", Writer.CloseEventList()) && Passed;
    Passed = EvaluateTrue("Close()", "sim write close", "The footer-only sim file closes cleanly", Writer.Close()) && Passed;
  }

  MString Text = ReadTextFile(FileName);
  Passed = EvaluateTrue("WriteHeader()", "sim write header text", "The written sim header contains TB", Text.Contains("TB 1.250000000")) && Passed;
  Passed = EvaluateTrue("CloseEventList()", "sim write footer text", "The written sim footer contains TE and TS", Text.Contains("TE 4.750000000") && Text.Contains("TS 12")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim write reopen", "The footer-only sim file reopens in read mode", Reader.Open(FileName)) && Passed;
    Passed = EvaluateNear("GetObservationTime()", "sim write reopen observation time", "TB and TE round-trip through a footer-only sim file", Reader.GetObservationTime().GetAsDouble(), 3.5, 1e-12) && Passed;
    Passed = Evaluate("GetSimulatedEvents()", "sim write reopen simulated events", "TS round-trips through a footer-only sim file", Reader.GetSimulatedEvents(), 12L) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestBinaryRoundTripAndRewind()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim binary temp dir", "The temporary directory can be recreated for binary sim tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim binary scan geometry", "The quest geometry can be scanned for binary tests", LoadGeometry(Geometry)) && Passed;

  MString InputFile = GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz";
  MString OutputFile = GetTempDirectory() + "/binary_roundtrip.sim";

  MFileEventsSim Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim binary open read", "The sim fixture can be opened for binary round-trip tests", Reader.Open(InputFile)) && Passed;

  Passed = EvaluateNear("GetObservationTime()", "sim binary input observation time", "The input observation time is available before rewriting", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "sim binary input simulated events", "The input simulated-event count is available before rewriting", Reader.GetSimulatedEvents(), 310L) && Passed;

  MSimEvent* FirstEvent = Reader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim binary first event", "The first input event can be read before rewinding", FirstEvent != nullptr) && Passed;
  if (FirstEvent != nullptr) {
    Passed = Evaluate("GetID()", "sim binary first event id", "The first input event has the expected id", FirstEvent->GetID(), 1L) && Passed;
    Passed = EvaluateNear("GetTime()", "sim binary first event time", "The first input event has the expected time", FirstEvent->GetTime().GetAsDouble(), 0.011003748, 1e-12) && Passed;
    delete FirstEvent;
  }

  Passed = EvaluateTrue("Rewind()", "sim binary rewind input", "The input sim file can be rewound before rewriting", Reader.Rewind()) && Passed;

  MFileEventsSim Writer(&Geometry);
  Passed = EvaluateTrue("Open()", "sim binary open write", "A binary sim output file can be opened", Writer.Open(OutputFile, MFile::c_Write, true)) && Passed;
  Writer.SetGeometryFileName(GetGeometryFileName());
  Writer.SetVersion(101);
  Passed = EvaluateTrue("WriteHeader()", "sim binary write header", "The binary sim header can be written", Writer.WriteHeader()) && Passed;

  MSimEvent* Event = nullptr;
  long Count = 0;
  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    ++Count;
    Passed = EvaluateTrue("AddEvent()", "sim binary add event", "Each input event can be written to the binary output file", Writer.AddEvent(Event)) && Passed;
    delete Event;
  }
  // Expected: 21 SE blocks (zcat | grep -c '^SE')
  Passed = Evaluate("AddEvent()", "sim binary event count", "The binary round-trip writes all 21 events of the fixture", Count, 21L) && Passed;

  Writer.SetObservationTime(Reader.GetObservationTime());
  Writer.SetSimulatedEvents(Reader.GetSimulatedEvents());
  Passed = EvaluateTrue("CloseEventList()", "sim binary close list", "The binary sim output file can be closed", Writer.CloseEventList()) && Passed;
  Passed = EvaluateTrue("Close()", "sim binary close", "The binary sim output file closes cleanly", Writer.Close()) && Passed;
  Reader.Close();

  MFileEventsSim BinaryReader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim binary reopen", "The binary sim output file reopens in read mode", BinaryReader.Open(OutputFile)) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "sim binary round-trip observation time", "Observation time survives the binary round-trip", BinaryReader.GetObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "sim binary round-trip simulated events", "TS survives the binary round-trip", BinaryReader.GetSimulatedEvents(), 310L) && Passed;

  Event = BinaryReader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim binary reopen first event", "The first event can be read back from the binary output file", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetID()", "sim binary reopen first event id", "The binary round-trip preserves the event id", Event->GetID(), 1L) && Passed;
    Passed = EvaluateNear("GetTime()", "sim binary reopen first event time", "The binary round-trip preserves the event time", Event->GetTime().GetAsDouble(), 0.011003748, 1e-12) && Passed;
    delete Event;
  }

  long ReadBackCount = 0;
  while ((Event = BinaryReader.GetNextEvent(false)) != nullptr) {
    ++ReadBackCount;
    delete Event;
  }
  Passed = Evaluate("GetNextEvent()", "sim binary reopen event count", "The binary output file holds the remaining 20 events after the first one", ReadBackCount, 20L) && Passed;

  BinaryReader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::WriteIncludeMaster(const MString& FileName, const MString& ExtraHeaderLine)
{
  ostringstream Content;
  Content<<"Type       sim"<<endl;
  Content<<"Version    101"<<endl;
  Content<<"Geometry   "<<GetGeometryFileName()<<endl;
  Content<<endl;
  Content<<"Date       2026-04-20 00:00:00"<<endl;
  Content<<"MEGAlib    "<<g_VersionString<<endl;
  if (ExtraHeaderLine.IsEmpty() == false) {
    Content<<ExtraHeaderLine<<endl;
  }
  Content<<endl;
  Content<<"IN "<<GetDataDirectory()<<"/ObsTime_1sec_complete.inc1.id1.sim.gz"<<endl;
  Content<<"IN "<<GetDataDirectory()<<"/ObsTime_2sec_complete.inc1.id1.sim.gz"<<endl;
  Content<<"IN "<<GetDataDirectory()<<"/ObsTime_4sec_complete.inc1.id1.sim.gz"<<endl;
  Content<<"EN"<<endl;
  Content<<endl;

  return WriteTextFile(FileName, Content.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


vector<double> UTFileEventsSim::ReadEventTimes(MDGeometryQuest& Geometry, const MString& FileName)
{
  vector<double> Times;

  MFileEventsSim Reader(&Geometry);
  if (Reader.Open(FileName) == false) return Times;

  MSimEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    Times.push_back(Event->GetTime().GetAsDouble());
    delete Event;
  }
  Reader.Close();

  return Times;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestAccumulatedIncludeTimes()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim AT temp dir", "The temporary directory can be recreated for AT tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim AT scan geometry", "The quest geometry can be scanned for AT tests", LoadGeometry(Geometry)) && Passed;

  // Event times of the children read on their own:
  vector<double> Times1 = ReadEventTimes(Geometry, GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz");
  vector<double> Times2 = ReadEventTimes(Geometry, GetDataDirectory() + "/ObsTime_2sec_complete.inc1.id1.sim.gz");
  vector<double> Times4 = ReadEventTimes(Geometry, GetDataDirectory() + "/ObsTime_4sec_complete.inc1.id1.sim.gz");
  Passed = EvaluateSize("GetNextEvent()", "sim AT child event count", "The three children hold 21 + 31 + 73 = 125 events", Times1.size() + Times2.size() + Times4.size(), (size_t) 125) && Passed;

  // Expected times without and with the offsets 0, 1, and 1 + 2 s of the preceding children:
  vector<double> Unshifted;
  vector<double> Shifted;
  for (double Time: Times1) { Unshifted.push_back(Time); Shifted.push_back(Time); }
  for (double Time: Times2) { Unshifted.push_back(Time); Shifted.push_back(Time + 1.0); }
  for (double Time: Times4) { Unshifted.push_back(Time); Shifted.push_back(Time + 3.0); }

  // AT true:
  MString MasterTrue = GetTempDirectory() + "/ObsTime_AT_true.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim AT true fixture", "The include master with AT true can be written", WriteIncludeMaster(MasterTrue, "AT         true")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT true open", "The include master with AT true opens in the sim reader", Reader.Open(MasterTrue)) && Passed;
    Passed = EvaluateTrue("HasAccumulatedIncludeTimes()", "sim AT true flag", "The AT true header line sets the accumulated include times flag", Reader.HasAccumulatedIncludeTimes()) && Passed;
    Passed = EvaluateNear("GetIncludeTimeOffset()", "sim AT true initial offset", "The include time offset starts at zero", Reader.GetIncludeTimeOffset().GetAsDouble(), 0.0, 1e-12) && Passed;

    vector<double> Times;
    MSimEvent* Event = nullptr;
    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      Times.push_back(Event->GetTime().GetAsDouble());
      delete Event;
    }
    Passed = EvaluateSize("GetNextEvent()", "sim AT true event count", "All 125 events of the children are read with AT true", Times.size(), Shifted.size()) && Passed;
    if (Times.size() == Shifted.size()) {
      for (size_t i = 0; i < Times.size(); ++i) {
        ostringstream Input;
        Input<<"sim AT true event "<<i;
        Passed = EvaluateNear("GetTime()", Input.str().c_str(), "The event time is offset by the observation time of the preceding children", Times[i], Shifted[i], 1e-9) && Passed;
      }
    }
    bool Monotonic = true;
    for (size_t i = 1; i < Times.size(); ++i) {
      if (Times[i] < Times[i-1]) Monotonic = false;
    }
    Passed = EvaluateTrue("GetTime()", "sim AT true monotonic", "With AT true the event times never decrease across the included files", Monotonic) && Passed;
    Passed = EvaluateNear("GetIncludeTimeOffset()", "sim AT true final offset", "After all children are closed the offset is the sum of their observation times", Reader.GetIncludeTimeOffset().GetAsDouble(), 7.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetObservationTime()", "sim AT true observation time", "The observation time still sums across the included files", Reader.GetObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;

    Passed = EvaluateTrue("Rewind()", "sim AT true rewind", "The include master with AT true can be rewound", Reader.Rewind()) && Passed;
    Passed = EvaluateNear("GetIncludeTimeOffset()", "sim AT true rewind offset", "Rewinding resets the include time offset", Reader.GetIncludeTimeOffset().GetAsDouble(), 0.0, 1e-12) && Passed;
    Event = Reader.GetNextEvent(false);
    Passed = EvaluateTrue("GetNextEvent()", "sim AT true rewind first event", "After rewinding the first event can be read again", Event != nullptr) && Passed;
    if (Event != nullptr && Shifted.empty() == false) {
      Passed = EvaluateNear("GetTime()", "sim AT true rewind first time", "After rewinding the first event is unshifted again", Event->GetTime().GetAsDouble(), Shifted[0], 1e-9) && Passed;
      delete Event;
    }
    Reader.Close();
  }

  // AT false:
  MString MasterFalse = GetTempDirectory() + "/ObsTime_AT_false.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim AT false fixture", "The include master with AT false can be written", WriteIncludeMaster(MasterFalse, "AT         false")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT false open", "The include master with AT false opens in the sim reader", Reader.Open(MasterFalse)) && Passed;
    Passed = EvaluateFalse("HasAccumulatedIncludeTimes()", "sim AT false flag", "The AT false header line leaves the accumulated include times flag unset", Reader.HasAccumulatedIncludeTimes()) && Passed;

    vector<double> Times;
    MSimEvent* Event = nullptr;
    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      Times.push_back(Event->GetTime().GetAsDouble());
      delete Event;
    }
    Passed = EvaluateSize("GetNextEvent()", "sim AT false event count", "All 125 events of the children are read with AT false", Times.size(), Unshifted.size()) && Passed;
    if (Times.size() == Unshifted.size()) {
      for (size_t i = 0; i < Times.size(); ++i) {
        ostringstream Input;
        Input<<"sim AT false event "<<i;
        Passed = EvaluateNear("GetTime()", Input.str().c_str(), "The event time is not offset with AT false", Times[i], Unshifted[i], 1e-9) && Passed;
      }
    }
    Passed = EvaluateNear("GetIncludeTimeOffset()", "sim AT false final offset", "The include time offset stays zero with AT false", Reader.GetIncludeTimeOffset().GetAsDouble(), 0.0, 1e-12) && Passed;
    Reader.Close();
  }

  // No AT line:
  MString MasterNone = GetTempDirectory() + "/ObsTime_AT_none.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim AT none fixture", "The include master without AT can be written", WriteIncludeMaster(MasterNone, "")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT none open", "The include master without AT opens in the sim reader", Reader.Open(MasterNone)) && Passed;
    Passed = EvaluateFalse("HasAccumulatedIncludeTimes()", "sim AT none flag", "Without an AT header line the accumulated include times flag is unset", Reader.HasAccumulatedIncludeTimes()) && Passed;
    Reader.Close();
  }

  // Malformed AT line:
  MString MasterBad = GetTempDirectory() + "/ObsTime_AT_bad.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim AT malformed fixture", "The include master with a malformed AT line can be written", WriteIncludeMaster(MasterBad, "AT")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    DisableDefaultStreams();
    bool Opened = Reader.Open(MasterBad);
    EnableDefaultStreams();
    bool Flag = Reader.HasAccumulatedIncludeTimes();
    Passed = EvaluateTrue("Open()", "sim AT malformed open", "The include master with a malformed AT line still opens", Opened) && Passed;
    Passed = EvaluateFalse("HasAccumulatedIncludeTimes()", "sim AT malformed flag", "A malformed AT line leaves the accumulated include times flag unset", Flag) && Passed;
    Reader.Close();
  }

  // Write side:
  MString Written = GetTempDirectory() + "/AT_header.sim";
  {
    MFileEventsSim Writer(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT write open", "A sim file for the AT header opens in write mode", Writer.Open(Written, MFile::c_Write)) && Passed;
    Writer.SetGeometryFileName(GetGeometryFileName());
    Writer.SetAccumulatedIncludeTimes(true);
    Passed = EvaluateTrue("HasAccumulatedIncludeTimes()", "sim AT write flag", "SetAccumulatedIncludeTimes(true) sets the flag", Writer.HasAccumulatedIncludeTimes()) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "sim AT write header", "The header with the AT keyword can be written", Writer.WriteHeader()) && Passed;
    Passed = EvaluateTrue("CloseEventList()", "sim AT write footer", "The footer can be written after the AT header", Writer.CloseEventList()) && Passed;
    Passed = EvaluateTrue("Close()", "sim AT write close", "The sim file with the AT header closes cleanly", Writer.Close()) && Passed;
  }

  MString Text = ReadTextFile(Written);
  Passed = EvaluateTrue("WriteHeader()", "sim AT write header text", "The written header contains the AT keyword", Text.Contains("AT        true")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT write reopen", "The sim file with the AT header reopens in read mode", Reader.Open(Written)) && Passed;
    Passed = EvaluateTrue("HasAccumulatedIncludeTimes()", "sim AT write reopen flag", "The AT keyword round-trips through WriteHeader()", Reader.HasAccumulatedIncludeTimes()) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileEventsSim Test;
  return Test.Run() ? 0 : 1;
}
