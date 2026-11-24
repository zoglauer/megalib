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
#include "MFile.h"
#include "MFileEventsSim.h"
#include "MTokenizer.h"
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
  bool TestAccumulateTimesAndIDsAcrossFiles();
  //! Test the AT keyword with an included file whose last event has no hits
  bool TestAccumulatedIncludeIDsWithEmptyLastEvent();
  //! Test the number of simulated events up to the last read event
  bool TestLastReadSimulationEventID();
  //! Test the observation time up to the last read event
  bool TestLastReadObservationTime();
  //! Test the last read values for a binary sim file
  bool TestLastReadBinary();
  //! Test the last read values for an include master which includes another include master
  bool TestLastReadNestedIncludes();
  //! Test the observation time up to the last read event for a file without TB and with the footer read first
  bool TestLastReadWithoutTBAndFooterFirst();
  //! Test the number of simulated events up to the last read event for a file without simulation IDs
  bool TestLastReadWithoutSimulationIDs();

  //! Write a variant of the 1-second fixture, optionally without the TB line and without the simulation IDs on the ID lines
  bool WriteFixtureVariant(const MString& FileName, bool WithTB, bool WithSimulationIDs);
  //! Write the 1-second fixture as binary sim file with TB 0, TE 1, and TS 310
  bool WriteBinaryFixture(MDGeometryQuest& Geometry, const MString& FileName);

  //! Write the 1-second fixture with an additional last event without hits (ID 22)
  bool WriteChildWithEmptyLastEvent(const MString& FileName);

  //! Write an include master for the three observation-time fixtures with an extra header line
  bool WriteIncludeMaster(const MString& FileName, const MString& ExtraHeaderLine);
  //! Read the times, IDs, and simulation IDs of all events of a sim file
  void ReadEvents(MDGeometryQuest& Geometry, const MString& FileName, vector<double>& Times, vector<long>& IDs, vector<long>& SimulationIDs);

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
  Passed = TestAccumulateTimesAndIDsAcrossFiles() && Passed;
  Passed = TestAccumulatedIncludeIDsWithEmptyLastEvent() && Passed;
  Passed = TestLastReadSimulationEventID() && Passed;
  Passed = TestLastReadObservationTime() && Passed;
  Passed = TestLastReadBinary() && Passed;
  Passed = TestLastReadNestedIncludes() && Passed;
  Passed = TestLastReadWithoutTBAndFooterFirst() && Passed;
  Passed = TestLastReadWithoutSimulationIDs() && Passed;

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
  MString Directory = "$(MEGALIB)/src/global/misc/unittests/data/UTFileEventsTra";
  MFile::ExpandFileName(Directory);
  return Directory;
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsSim::GetGeometryFileName() const
{
  MString FileName = "$(MEGALIB)/resource/examples/geomega/special/Max.geo.setup";
  MFile::ExpandFileName(FileName);
  return FileName;
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

  // Without an end time, TE is the start time plus the observation time
  {
    MString NoEndFileName = GetTempDirectory() + "/footer_no_end.sim";
    MFileEventsSim Writer(&Geometry);
    Passed = EvaluateTrue("Open()", "sim no end open", "The sim file without end time opens in write mode", Writer.Open(NoEndFileName, MFile::c_Write)) && Passed;
    Writer.SetGeometryFileName(GetGeometryFileName());
    Writer.SetStartObservationTime(MTime(1.25));
    Writer.SetObservationTime(MTime(3.5));
    Writer.SetSimulatedEvents(12);
    Passed = EvaluateTrue("WriteHeader()", "sim no end header", "The sim header without end time can be written", Writer.WriteHeader()) && Passed;
    Passed = EvaluateTrue("CloseEventList()", "sim no end footer", "The sim footer without end time can be written", Writer.CloseEventList()) && Passed;
    Passed = EvaluateTrue("Close()", "sim no end close", "The sim file without end time closes cleanly", Writer.Close()) && Passed;

    // End time = start + observation time = 1.25 + 3.5
    Passed = EvaluateTrue("CloseEventList()", "sim no end TE", "TE is the start time plus the observation time", ReadTextFile(NoEndFileName).Contains("TE 4.750000000")) && Passed;
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


void UTFileEventsSim::ReadEvents(MDGeometryQuest& Geometry, const MString& FileName, vector<double>& Times, vector<long>& IDs, vector<long>& SimulationIDs)
{
  Times.clear();
  IDs.clear();
  SimulationIDs.clear();

  MFileEventsSim Reader(&Geometry);
  if (Reader.Open(FileName) == false) return;

  MSimEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    Times.push_back(Event->GetTime().GetAsDouble());
    IDs.push_back(Event->GetID());
    SimulationIDs.push_back(Event->GetSimulationEventID());
    delete Event;
  }
  Reader.Close();
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestAccumulateTimesAndIDsAcrossFiles()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim AT temp dir", "The temporary directory can be recreated for AT tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim AT scan geometry", "The quest geometry can be scanned for AT tests", LoadGeometry(Geometry)) && Passed;

  // Times and IDs of the children read on their own:
  vector<double> Times1;
  vector<double> Times2;
  vector<double> Times4;
  vector<long> IDs1;
  vector<long> IDs2;
  vector<long> IDs4;
  vector<long> SimulationIDs1;
  vector<long> SimulationIDs2;
  vector<long> SimulationIDs4;
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz", Times1, IDs1, SimulationIDs1);
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_2sec_complete.inc1.id1.sim.gz", Times2, IDs2, SimulationIDs2);
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_4sec_complete.inc1.id1.sim.gz", Times4, IDs4, SimulationIDs4);
  Passed = EvaluateSize("GetNextEvent()", "sim AT child event count", "The three children hold 21 + 31 + 73 = 125 events", Times1.size() + Times2.size() + Times4.size(), (size_t) 125) && Passed;

  // Expected values without and with the offsets of the preceding children: times 0, 1, 1 + 2 s, event IDs 0, 21, 21 + 31, simulation IDs 0, 310, 310 + 631:
  vector<double> Unshifted;
  vector<double> Shifted;
  vector<long> UnshiftedIDs;
  vector<long> ShiftedIDs;
  vector<long> UnshiftedSimulationIDs;
  vector<long> ShiftedSimulationIDs;
  for (size_t i = 0; i < Times1.size(); ++i) {
    Unshifted.push_back(Times1[i]);
    Shifted.push_back(Times1[i]);
    UnshiftedIDs.push_back(IDs1[i]);
    ShiftedIDs.push_back(IDs1[i]);
    UnshiftedSimulationIDs.push_back(SimulationIDs1[i]);
    ShiftedSimulationIDs.push_back(SimulationIDs1[i]);
  }
  for (size_t i = 0; i < Times2.size(); ++i) {
    Unshifted.push_back(Times2[i]);
    Shifted.push_back(Times2[i] + 1.0);
    UnshiftedIDs.push_back(IDs2[i]);
    ShiftedIDs.push_back(IDs2[i] + 21);
    UnshiftedSimulationIDs.push_back(SimulationIDs2[i]);
    ShiftedSimulationIDs.push_back(SimulationIDs2[i] + 310);
  }
  for (size_t i = 0; i < Times4.size(); ++i) {
    Unshifted.push_back(Times4[i]);
    Shifted.push_back(Times4[i] + 3.0);
    UnshiftedIDs.push_back(IDs4[i]);
    ShiftedIDs.push_back(IDs4[i] + 52);
    UnshiftedSimulationIDs.push_back(SimulationIDs4[i]);
    ShiftedSimulationIDs.push_back(SimulationIDs4[i] + 941);
  }

  // AT true:
  MString MasterTrue = GetTempDirectory() + "/ObsTime_AT_true.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim AT true fixture", "The include master with AT true can be written", WriteIncludeMaster(MasterTrue, "AT         true")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT true open", "The include master with AT true opens in the sim reader", Reader.Open(MasterTrue)) && Passed;
    Passed = EvaluateTrue("GetAccumulateTimesAndIDsAcrossFiles()", "sim AT true flag", "The AT true header line sets the accumulated include times flag", Reader.GetAccumulateTimesAndIDsAcrossFiles()) && Passed;
    Passed = EvaluateNear("GetIncludeOffsetObservationTime()", "sim AT true initial offset", "The include time offset starts at zero", Reader.GetIncludeOffsetObservationTime().GetAsDouble(), 0.0, 1e-12) && Passed;

    vector<double> Times;
    vector<long> IDs;
    vector<long> SimulationIDs;
    MSimEvent* Event = nullptr;
    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      Times.push_back(Event->GetTime().GetAsDouble());
      IDs.push_back(Event->GetID());
      SimulationIDs.push_back(Event->GetSimulationEventID());
      delete Event;
    }
    Passed = EvaluateSize("GetNextEvent()", "sim AT true event count", "All 125 events of the children are read with AT true", Times.size(), Shifted.size()) && Passed;
    if (Times.size() == Shifted.size()) {
      for (size_t i = 0; i < Times.size(); ++i) {
        ostringstream Input;
        Input<<"sim AT true event "<<i;
        Passed = EvaluateNear("GetTime()", Input.str().c_str(), "The event time is offset by the observation time of the preceding children", Times[i], Shifted[i], 1e-9) && Passed;
        Passed = Evaluate("GetID()", Input.str().c_str(), "The event ID is offset by the last event ID of the preceding children", IDs[i], ShiftedIDs[i]) && Passed;
        Passed = Evaluate("GetSimulationEventID()", Input.str().c_str(), "The simulation event ID is offset by the simulated events of the preceding children", SimulationIDs[i], ShiftedSimulationIDs[i]) && Passed;
      }
    }
    bool Monotonic = true;
    for (size_t i = 1; i < Times.size(); ++i) {
      if (Times[i] < Times[i-1]) Monotonic = false;
    }
    Passed = EvaluateTrue("GetTime()", "sim AT true monotonic", "With AT true the event times never decrease across the included files", Monotonic) && Passed;
    Passed = EvaluateNear("GetIncludeOffsetObservationTime()", "sim AT true final offset", "After all children are closed the offset is the sum of their observation times", Reader.GetIncludeOffsetObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;
    Passed = Evaluate("GetIncludeOffsetEventID()", "sim AT true final ID offset", "After all children are closed the ID offset is the last event ID", Reader.GetIncludeOffsetEventID(), 125L) && Passed;
    Passed = EvaluateNear("GetObservationTime()", "sim AT true observation time", "The observation time still sums across the included files", Reader.GetObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;

    Passed = EvaluateTrue("Rewind()", "sim AT true rewind", "The include master with AT true can be rewound", Reader.Rewind()) && Passed;
    Passed = EvaluateNear("GetIncludeOffsetObservationTime()", "sim AT true rewind offset", "Rewinding resets the include time offset", Reader.GetIncludeOffsetObservationTime().GetAsDouble(), 0.0, 1e-12) && Passed;
    Passed = Evaluate("GetIncludeOffsetEventID()", "sim AT true rewind ID offset", "Rewinding resets the include ID offset", Reader.GetIncludeOffsetEventID(), 0L) && Passed;
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
    Passed = EvaluateFalse("GetAccumulateTimesAndIDsAcrossFiles()", "sim AT false flag", "The AT false header line leaves the accumulated include times flag unset", Reader.GetAccumulateTimesAndIDsAcrossFiles()) && Passed;

    vector<double> Times;
    vector<long> IDs;
    vector<long> SimulationIDs;
    MSimEvent* Event = nullptr;
    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      Times.push_back(Event->GetTime().GetAsDouble());
      IDs.push_back(Event->GetID());
      SimulationIDs.push_back(Event->GetSimulationEventID());
      delete Event;
    }
    Passed = EvaluateSize("GetNextEvent()", "sim AT false event count", "All 125 events of the children are read with AT false", Times.size(), Unshifted.size()) && Passed;
    if (Times.size() == Unshifted.size()) {
      for (size_t i = 0; i < Times.size(); ++i) {
        ostringstream Input;
        Input<<"sim AT false event "<<i;
        Passed = EvaluateNear("GetTime()", Input.str().c_str(), "The event time is not offset with AT false", Times[i], Unshifted[i], 1e-9) && Passed;
        Passed = Evaluate("GetID()", Input.str().c_str(), "The event ID is not offset with AT false", IDs[i], UnshiftedIDs[i]) && Passed;
        Passed = Evaluate("GetSimulationEventID()", Input.str().c_str(), "The simulation event ID is not offset with AT false", SimulationIDs[i], UnshiftedSimulationIDs[i]) && Passed;
      }
    }
    Passed = EvaluateNear("GetIncludeOffsetObservationTime()", "sim AT false final offset", "The include time offset is accumulated with AT false too (1 + 2 + 4 s), it is only not applied to the events", Reader.GetIncludeOffsetObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;
    Passed = Evaluate("GetIncludeOffsetEventID()", "sim AT false final ID offset", "The include ID offset is accumulated with AT false too (21 + 31 + 73), it is only not applied to the events", Reader.GetIncludeOffsetEventID(), 125L) && Passed;
    Passed = Evaluate("GetLastReadEventID()", "sim AT false final last ID", "The last event ID counts across the included files with AT false too", Reader.GetLastReadEventID(), 125L) && Passed;
    Reader.Close();
  }

  // No AT line:
  MString MasterNone = GetTempDirectory() + "/ObsTime_AT_none.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim AT none fixture", "The include master without AT can be written", WriteIncludeMaster(MasterNone, "")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT none open", "The include master without AT opens in the sim reader", Reader.Open(MasterNone)) && Passed;
    Passed = EvaluateFalse("GetAccumulateTimesAndIDsAcrossFiles()", "sim AT none flag", "Without an AT header line the accumulated include times flag is unset", Reader.GetAccumulateTimesAndIDsAcrossFiles()) && Passed;
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
    bool Flag = Reader.GetAccumulateTimesAndIDsAcrossFiles();
    Passed = EvaluateTrue("Open()", "sim AT malformed open", "The include master with a malformed AT line still opens", Opened) && Passed;
    Passed = EvaluateFalse("GetAccumulateTimesAndIDsAcrossFiles()", "sim AT malformed flag", "A malformed AT line leaves the accumulated include times flag unset", Flag) && Passed;
    Reader.Close();
  }

  // Write side:
  MString Written = GetTempDirectory() + "/AT_header.sim";
  {
    MFileEventsSim Writer(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT write open", "A sim file for the AT header opens in write mode", Writer.Open(Written, MFile::c_Write)) && Passed;
    Writer.SetGeometryFileName(GetGeometryFileName());
    Writer.SetAccumulateTimesAndIDsAcrossFiles(true);
    Passed = EvaluateTrue("GetAccumulateTimesAndIDsAcrossFiles()", "sim AT write flag", "SetAccumulateTimesAndIDsAcrossFiles(true) sets the flag", Writer.GetAccumulateTimesAndIDsAcrossFiles()) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "sim AT write header", "The header with the AT keyword can be written", Writer.WriteHeader()) && Passed;
    Passed = EvaluateTrue("CloseEventList()", "sim AT write footer", "The footer can be written after the AT header", Writer.CloseEventList()) && Passed;
    Passed = EvaluateTrue("Close()", "sim AT write close", "The sim file with the AT header closes cleanly", Writer.Close()) && Passed;
  }

  MString Text = ReadTextFile(Written);
  Passed = EvaluateTrue("WriteHeader()", "sim AT write header text", "The written header contains the AT keyword", Text.Contains("AT        true")) && Passed;

  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT write reopen", "The sim file with the AT header reopens in read mode", Reader.Open(Written)) && Passed;
    Passed = EvaluateTrue("GetAccumulateTimesAndIDsAcrossFiles()", "sim AT write reopen flag", "The AT keyword round-trips through WriteHeader()", Reader.GetAccumulateTimesAndIDsAcrossFiles()) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::WriteChildWithEmptyLastEvent(const MString& FileName)
{
  MFile In;
  if (In.Open(GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz") == false) return false;

  ostringstream Content;
  MString Line;
  while (In.ReadLine(Line) == true) {
    if (Line.BeginsWith("EN") == true) {
      Content<<"SE"<<endl;
      Content<<"ID 22 309"<<endl;
      Content<<"TI 0.95"<<endl;
    }
    Content<<Line<<endl;
  }
  In.Close();

  return WriteTextFile(FileName, Content.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestAccumulatedIncludeIDsWithEmptyLastEvent()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim AT empty last event temp dir", "The temporary directory can be recreated for the empty last event test", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim AT empty last event geometry", "The quest geometry can be scanned for the empty last event test", LoadGeometry(Geometry)) && Passed;

  MString Child1 = GetTempDirectory() + "/ObsTime_1sec_empty_last.sim";
  MString Child2 = GetDataDirectory() + "/ObsTime_2sec_complete.inc1.id1.sim.gz";
  Passed = EvaluateTrue("WriteTextFile()", "sim AT empty last event child", "The child with an empty last event can be written", WriteChildWithEmptyLastEvent(Child1)) && Passed;

  // Child with the empty last event read on its own:
  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim AT empty last event child open", "The child with an empty last event opens", Reader.Open(Child1)) && Passed;
    MSimEvent* Event = nullptr;
    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      delete Event;
    }
    Passed = Evaluate("GetLastReadEventID()", "sim AT empty last event child last ID", "The last read event ID is the ID of the empty last event", Reader.GetLastReadEventID(), 22L) && Passed;
    Reader.Close();
  }

  vector<double> Times2;
  vector<long> IDs2;
  vector<long> SimulationIDs2;
  ReadEvents(Geometry, Child2, Times2, IDs2, SimulationIDs2);

  ostringstream Content;
  Content<<"Type       sim"<<endl;
  Content<<"Version    101"<<endl;
  Content<<"Geometry   "<<GetGeometryFileName()<<endl;
  Content<<"AT         true"<<endl;
  Content<<endl;
  Content<<"IN "<<Child1<<endl;
  Content<<"IN "<<Child2<<endl;
  Content<<"EN"<<endl;
  MString Master = GetTempDirectory() + "/ObsTime_AT_empty_last.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim AT empty last event master", "The include master can be written", WriteTextFile(Master, Content.str().c_str())) && Passed;

  MFileEventsSim Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim AT empty last event master open", "The include master opens", Reader.Open(Master)) && Passed;

  vector<long> IDs;
  MSimEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    IDs.push_back(Event->GetID());
    delete Event;
  }
  Passed = EvaluateSize("GetNextEvent()", "sim AT empty last event count", "The master yields the 22 events of the first child and all events of the second", IDs.size(), 22 + IDs2.size()) && Passed;
  if (IDs.size() == 22 + IDs2.size()) {
    for (size_t i = 0; i < IDs2.size(); ++i) {
      ostringstream Input;
      Input<<"sim AT empty last event, second child event "<<i;
      Passed = Evaluate("GetID()", Input.str().c_str(), "The event IDs of the second child continue after the empty last event of the first child", IDs[22 + i], IDs2[i] + 22) && Passed;
    }
  }
  Passed = Evaluate("GetIncludeOffsetEventID()", "sim AT empty last event final ID offset", "The final ID offset is the sum of the last event IDs of both children", Reader.GetIncludeOffsetEventID(), 53L) && Passed;
  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestLastReadSimulationEventID()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim last read temp dir", "The temporary directory can be recreated for the last read tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim last read scan geometry", "The quest geometry can be scanned for the last read tests", LoadGeometry(Geometry)) && Passed;

  // Simulation IDs of the first two children read on their own (TS 310 and 631):
  vector<double> Times1;
  vector<double> Times2;
  vector<long> IDs1;
  vector<long> IDs2;
  vector<long> SimulationIDs1;
  vector<long> SimulationIDs2;
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz", Times1, IDs1, SimulationIDs1);
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_2sec_complete.inc1.id1.sim.gz", Times2, IDs2, SimulationIDs2);
  Passed = EvaluateSize("GetNextEvent()", "sim last read child 1 event count", "The first child holds 21 events", SimulationIDs1.size(), (size_t) 21) && Passed;
  Passed = EvaluateSize("GetNextEvent()", "sim last read child 2 event count", "The second child holds 31 events", SimulationIDs2.size(), (size_t) 31) && Passed;

  // Single file:
  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim last read single open", "The 1-second sim fixture opens", Reader.Open(GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz")) && Passed;
    Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read single before first event", "Before the first event no simulated events are counted", Reader.GetLastReadSimulationEventID(), 0L) && Passed;

    unsigned int NRead = 0;
    MSimEvent* Event = nullptr;
    while (NRead < 5 && (Event = Reader.GetNextEvent(false)) != nullptr) {
      ++NRead;
      delete Event;
    }
    Passed = Evaluate("GetNextEvent()", "sim last read single 5 events", "Five events can be read", NRead, 5U) && Passed;
    if (NRead == 5 && SimulationIDs1.size() >= 5) {
      Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read single after 5 events", "After 5 events the count is the simulation ID of the 5th event", Reader.GetLastReadSimulationEventID(), SimulationIDs1[4]) && Passed;
    }

    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      delete Event;
    }
    Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read single all events", "After all events the count is the total TS 310", Reader.GetLastReadSimulationEventID(), 310L) && Passed;

    Passed = EvaluateTrue("Rewind()", "sim last read single rewind", "The sim fixture can be rewound", Reader.Rewind()) && Passed;
    Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read single after rewind", "Rewinding resets the count to zero", Reader.GetLastReadSimulationEventID(), 0L) && Passed;
    Reader.Close();
  }

  // Include masters stopped in the second child, with AT false and AT true:
  vector<MString> HeaderLines = { "AT         false", "AT         true" };
  for (const MString& HeaderLine: HeaderLines) {
    MString Master = GetTempDirectory() + "/LastRead_" + (HeaderLine.Contains("true") == true ? "AT_true" : "AT_false") + ".sim";
    MString Input = MString("sim last read master ") + HeaderLine;
    Passed = EvaluateTrue("WriteTextFile()", Input, "The include master can be written", WriteIncludeMaster(Master, HeaderLine)) && Passed;

    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", Input, "The include master opens", Reader.Open(Master)) && Passed;

    // All 21 events of the first child and 3 of the second:
    unsigned int NRead = 0;
    MSimEvent* Event = nullptr;
    while (NRead < 24 && (Event = Reader.GetNextEvent(false)) != nullptr) {
      ++NRead;
      delete Event;
    }
    Passed = Evaluate("GetNextEvent()", Input + " 24 events", "24 events can be read", NRead, 24U) && Passed;
    if (NRead == 24 && SimulationIDs2.size() >= 3) {
      Passed = Evaluate("GetLastReadSimulationEventID()", Input + " stopped in second child", "The count is TS 310 of the first child plus the simulation ID of the 3rd event of the second child", Reader.GetLastReadSimulationEventID(), 310L + SimulationIDs2[2]) && Passed;
    }
    if (NRead == 24 && IDs2.size() >= 3) {
      Passed = Evaluate("GetLastReadEventID()", Input + " stopped in second child", "The last event ID is the last ID 21 of the first child plus the ID of the 3rd event of the second child", Reader.GetLastReadEventID(), 21L + IDs2[2]) && Passed;
    }

    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      delete Event;
    }
    Passed = Evaluate("GetLastReadSimulationEventID()", Input + " all events", "After all events the count is the total 310 + 631 + 1284 = 2225", Reader.GetLastReadSimulationEventID(), 2225L) && Passed;
    Passed = Evaluate("GetLastReadEventID()", Input + " all events", "After all events the last event ID is the sum of the last IDs 21 + 31 + 73", Reader.GetLastReadEventID(), 125L) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestLastReadObservationTime()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim last read time temp dir", "The temporary directory can be recreated for the last read time tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim last read time scan geometry", "The geometry can be scanned for the last read time tests", LoadGeometry(Geometry)) && Passed;

  // Event times of the first two children read on their own (TB 0, TE 1 and 2 s):
  vector<double> Times1;
  vector<double> Times2;
  vector<long> IDs1;
  vector<long> IDs2;
  vector<long> SimulationIDs1;
  vector<long> SimulationIDs2;
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz", Times1, IDs1, SimulationIDs1);
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_2sec_complete.inc1.id1.sim.gz", Times2, IDs2, SimulationIDs2);
  Passed = EvaluateSize("GetNextEvent()", "sim last read time child 1 event count", "The first child holds 21 events", Times1.size(), (size_t) 21) && Passed;
  Passed = EvaluateSize("GetNextEvent()", "sim last read time child 2 event count", "The second child holds 31 events", Times2.size(), (size_t) 31) && Passed;

  // Single file:
  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim last read time single open", "The 1-second sim fixture opens", Reader.Open(GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz")) && Passed;
    Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read time single before first event", "Before the first event the observation time is zero", Reader.GetLastReadObservationTime().GetAsDouble(), 0.0, 1e-12) && Passed;

    unsigned int NRead = 0;
    MSimEvent* Event = nullptr;
    while (NRead < 5 && (Event = Reader.GetNextEvent(false)) != nullptr) {
      ++NRead;
      delete Event;
    }
    Passed = Evaluate("GetNextEvent()", "sim last read time single 5 events", "Five events can be read", NRead, 5U) && Passed;
    if (NRead == 5 && Times1.size() >= 5) {
      Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read time single after 5 events", "After 5 events the observation time is the time of the 5th event (TB 0)", Reader.GetLastReadObservationTime().GetAsDouble(), Times1[4], 1e-9) && Passed;
    }

    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      delete Event;
    }
    Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read time single all events", "After all events the observation time is the total TE - TB = 1 s", Reader.GetLastReadObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;

    Passed = EvaluateTrue("Rewind()", "sim last read time single rewind", "The sim fixture can be rewound", Reader.Rewind()) && Passed;
    Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read time single after rewind", "Rewinding resets the observation time to zero", Reader.GetLastReadObservationTime().GetAsDouble(), 0.0, 1e-12) && Passed;
    Reader.Close();
  }

  // Include masters stopped in the second child, with AT false and AT true:
  vector<MString> HeaderLines = { "AT         false", "AT         true" };
  for (const MString& HeaderLine: HeaderLines) {
    MString Master = GetTempDirectory() + "/LastReadTime_" + (HeaderLine.Contains("true") == true ? "AT_true" : "AT_false") + ".sim";
    MString Input = MString("sim last read time master ") + HeaderLine;
    Passed = EvaluateTrue("WriteTextFile()", Input, "The include master can be written", WriteIncludeMaster(Master, HeaderLine)) && Passed;

    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", Input, "The include master opens", Reader.Open(Master)) && Passed;

    // All 21 events of the first child and 3 of the second:
    unsigned int NRead = 0;
    MSimEvent* Event = nullptr;
    while (NRead < 24 && (Event = Reader.GetNextEvent(false)) != nullptr) {
      ++NRead;
      delete Event;
    }
    Passed = Evaluate("GetNextEvent()", Input + " 24 events", "24 events can be read", NRead, 24U) && Passed;
    if (NRead == 24 && Times2.size() >= 3) {
      Passed = EvaluateNear("GetLastReadObservationTime()", Input + " stopped in second child", "The observation time is the 1 s of the first child plus the time of the 3rd event of the second child", Reader.GetLastReadObservationTime().GetAsDouble(), 1.0 + Times2[2], 1e-9) && Passed;
    }

    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      delete Event;
    }
    Passed = EvaluateNear("GetLastReadObservationTime()", Input + " all events", "After all events the observation time is the total 1 + 2 + 4 = 7 s", Reader.GetLastReadObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::WriteFixtureVariant(const MString& FileName, bool WithTB, bool WithSimulationIDs)
{
  MFile In;
  if (In.Open(GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz") == false) return false;

  ostringstream Content;
  MString Line;
  while (In.ReadLine(Line) == true) {
    if (WithTB == false && Line.BeginsWith("TB") == true) continue;
    if (WithSimulationIDs == false && Line.BeginsWith("ID") == true) {
      MTokenizer Tokens(Line);
      if (Tokens.GetNTokens() == 3) Line = MString("ID ") + Tokens.GetTokenAtAsString(1);
    }
    Content<<Line<<endl;
  }
  In.Close();

  return WriteTextFile(FileName, Content.str().c_str());
}

////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::WriteBinaryFixture(MDGeometryQuest& Geometry, const MString& FileName)
{
  MFileEventsSim Reader(&Geometry);
  if (Reader.Open(GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz") == false) return false;

  MFileEventsSim Writer(&Geometry);
  if (Writer.Open(FileName, MFile::c_Write, true) == false) return false;
  Writer.SetGeometryFileName(GetGeometryFileName());
  Writer.SetVersion(101);
  Writer.SetStartObservationTime(0.0);
  if (Writer.WriteHeader() == false) return false;

  MSimEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    Writer.AddEvent(Event);
    delete Event;
  }
  Reader.Close();

  Writer.SetEndObservationTime(1.0);
  Writer.SetSimulatedEvents(310);
  if (Writer.CloseEventList() == false) return false;
  return Writer.Close();
}

////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestLastReadBinary()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim last read binary temp dir", "The temporary directory can be recreated for the binary last read tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim last read binary geometry", "The geometry can be scanned for the binary last read tests", LoadGeometry(Geometry)) && Passed;

  // Event times and IDs of the ASCII fixture:
  vector<double> Times;
  vector<long> IDs;
  vector<long> SimulationIDs;
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz", Times, IDs, SimulationIDs);

  MString BinaryFile = GetTempDirectory() + "/LastRead_binary.sim";
  Passed = EvaluateTrue("WriteBinaryFixture()", "sim last read binary fixture", "The binary fixture can be written", WriteBinaryFixture(Geometry, BinaryFile)) && Passed;

  MFileEventsSim Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim last read binary open", "The binary fixture opens", Reader.Open(BinaryFile)) && Passed;

  unsigned int NRead = 0;
  MSimEvent* Event = nullptr;
  while (NRead < 5 && (Event = Reader.GetNextEvent(false)) != nullptr) {
    ++NRead;
    delete Event;
  }
  Passed = Evaluate("GetNextEvent()", "sim last read binary 5 events", "Five events can be read from the binary fixture", NRead, 5U) && Passed;
  if (NRead == 5 && Times.size() >= 5) {
    Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read binary after 5 events", "After 5 events the observation time is the time of the 5th event (TB 0)", Reader.GetLastReadObservationTime().GetAsDouble(), Times[4], 1e-9) && Passed;
    Passed = Evaluate("GetLastReadEventID()", "sim last read binary after 5 events", "After 5 events the last event ID is the ID of the 5th event", Reader.GetLastReadEventID(), IDs[4]) && Passed;
  }
  if (NRead == 5 && SimulationIDs.size() >= 5) {
    Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read binary after 5 events", "After 5 events the count is the simulation ID of the 5th event", Reader.GetLastReadSimulationEventID(), SimulationIDs[4]) && Passed;
  }

  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    delete Event;
  }
  Passed = EvaluateNear("GetObservationTime()", "sim last read binary all events", "After all events of a binary file the observation time is TE - TB = 1 s", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read binary all events", "After all events the observation time is the total TE - TB = 1 s", Reader.GetLastReadObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;
  Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read binary all events", "After all events the count is the total TS 310", Reader.GetLastReadSimulationEventID(), 310L) && Passed;

  Reader.Close();

  return Passed;
}

////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestLastReadWithoutTBAndFooterFirst()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim last read without TB temp dir", "The temporary directory can be recreated for the tests without TB", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim last read without TB geometry", "The geometry can be scanned for the tests without TB", LoadGeometry(Geometry)) && Passed;

  vector<double> Times;
  vector<long> IDs;
  vector<long> SimulationIDs;
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz", Times, IDs, SimulationIDs);

  MString NoTBFile = GetTempDirectory() + "/LastRead_without_TB.sim";
  Passed = EvaluateTrue("WriteFixtureVariant()", "sim last read without TB fixture", "The fixture without TB can be written", WriteFixtureVariant(NoTBFile, false, true)) && Passed;

  // Without TB, read event by event: the observation starts at the first event
  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim last read without TB open", "The fixture without TB opens", Reader.Open(NoTBFile)) && Passed;
    unsigned int NRead = 0;
    MSimEvent* Event = nullptr;
    while (NRead < 5 && (Event = Reader.GetNextEvent(false)) != nullptr) {
      ++NRead;
      delete Event;
    }
    if (NRead == 5 && Times.size() >= 5) {
      Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read without TB after 5 events", "Without TB the observation time after 5 events is the time of the 5th event minus the one of the first event", Reader.GetLastReadObservationTime().GetAsDouble(), Times[4] - Times[0], 1e-9) && Passed;
    }
    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      delete Event;
    }
    if (Times.size() >= 1) {
      Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read without TB all events", "Without TB the observation time after all events is TE minus the time of the first event", Reader.GetLastReadObservationTime().GetAsDouble(), 1.0 - Times[0], 1e-9) && Passed;
      Passed = EvaluateNear("GetObservationTime()", "sim last read without TB all events", "Without TB the observation time after all events is TE minus the time of the first event", Reader.GetObservationTime().GetAsDouble(), 1.0 - Times[0], 1e-9) && Passed;
    }
    Reader.Close();
  }

  // Without TB, the footer read first:
  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim last read without TB footer first open", "The fixture without TB opens", Reader.Open(NoTBFile)) && Passed;
    if (Times.size() >= 1) {
      Passed = EvaluateNear("GetObservationTime()", "sim last read without TB footer first", "Without TB the observation time read first is TE minus the time of the first event, as when the events are read first", Reader.GetObservationTime().GetAsDouble(), 1.0 - Times[0], 1e-9) && Passed;
    }
    unsigned int NRead = 0;
    MSimEvent* Event = nullptr;
    while (NRead < 5 && (Event = Reader.GetNextEvent(false)) != nullptr) {
      ++NRead;
      delete Event;
    }
    if (NRead == 5 && Times.size() >= 5) {
      Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read without TB footer first after 5 events", "Without TB and with the footer read first, the observation time after 5 events is the time of the 5th event minus the one of the first event", Reader.GetLastReadObservationTime().GetAsDouble(), Times[4] - Times[0], 1e-9) && Passed;
    }
    Reader.Close();
  }

  // With TB, the footer read first:
  {
    MFileEventsSim Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "sim last read footer first open", "The 1-second fixture opens", Reader.Open(GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz")) && Passed;
    Passed = EvaluateNear("GetObservationTime()", "sim last read footer first", "The observation time read first is TE - TB = 1 s", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;
    unsigned int NRead = 0;
    MSimEvent* Event = nullptr;
    while (NRead < 5 && (Event = Reader.GetNextEvent(false)) != nullptr) {
      ++NRead;
      delete Event;
    }
    if (NRead == 5 && Times.size() >= 5) {
      Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read footer first after 5 events", "With the footer read first, the observation time after 5 events is the time of the 5th event (TB 0)", Reader.GetLastReadObservationTime().GetAsDouble(), Times[4], 1e-9) && Passed;
    }
    while ((Event = Reader.GetNextEvent(false)) != nullptr) {
      delete Event;
    }
    Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read footer first all events", "With the footer read first, the observation time after all events is 1 s", Reader.GetLastReadObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;
    Reader.Close();
  }

  return Passed;
}

////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestLastReadWithoutSimulationIDs()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim last read without simulation IDs temp dir", "The temporary directory can be recreated for the test without simulation IDs", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim last read without simulation IDs geometry", "The geometry can be scanned for the test without simulation IDs", LoadGeometry(Geometry)) && Passed;

  MString FileName = GetTempDirectory() + "/LastRead_without_simulation_IDs.sim";
  Passed = EvaluateTrue("WriteFixtureVariant()", "sim last read without simulation IDs fixture", "The fixture with ID lines without simulation IDs can be written", WriteFixtureVariant(FileName, true, false)) && Passed;

  MFileEventsSim Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim last read without simulation IDs open", "The fixture without simulation IDs opens", Reader.Open(FileName)) && Passed;

  unsigned int NRead = 0;
  MSimEvent* Event = nullptr;
  while (NRead < 5 && (Event = Reader.GetNextEvent(false)) != nullptr) {
    ++NRead;
    delete Event;
  }
  Passed = Evaluate("GetNextEvent()", "sim last read without simulation IDs 5 events", "Five events can be read", NRead, 5U) && Passed;
  // Without simulation IDs MSimEvent uses the event ID as simulation ID, i.e. the count is a lower bound
  Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read without simulation IDs after 5 events", "Without simulation IDs the count is the ID of the 5th event", Reader.GetLastReadSimulationEventID(), 5L) && Passed;

  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    delete Event;
  }
  Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read without simulation IDs all events", "After all events the count is the total TS 310", Reader.GetLastReadSimulationEventID(), 310L) && Passed;
  Reader.Close();

  return Passed;
}

////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::TestLastReadNestedIncludes()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "sim last read nested temp dir", "The temporary directory can be recreated for the nested include tests", PrepareTempDirectory()) && Passed;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "sim last read nested geometry", "The geometry can be scanned for the nested include tests", LoadGeometry(Geometry)) && Passed;

  // Times and IDs of the second fixture read on its own:
  vector<double> Times2;
  vector<long> IDs2;
  vector<long> SimulationIDs2;
  ReadEvents(Geometry, GetDataDirectory() + "/ObsTime_2sec_complete.inc1.id1.sim.gz", Times2, IDs2, SimulationIDs2);

  // Inner master (1 + 2 s) and outer master (inner master + 4 s), both without AT:
  ostringstream Inner;
  Inner<<"Type       sim"<<endl;
  Inner<<"Version    101"<<endl;
  Inner<<"Geometry   "<<GetGeometryFileName()<<endl;
  Inner<<endl;
  Inner<<"IN "<<GetDataDirectory()<<"/ObsTime_1sec_complete.inc1.id1.sim.gz"<<endl;
  Inner<<"IN "<<GetDataDirectory()<<"/ObsTime_2sec_complete.inc1.id1.sim.gz"<<endl;
  Inner<<"EN"<<endl;
  MString InnerFile = GetTempDirectory() + "/LastRead_nested_inner.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim last read nested inner", "The inner include master can be written", WriteTextFile(InnerFile, Inner.str().c_str())) && Passed;

  ostringstream Outer;
  Outer<<"Type       sim"<<endl;
  Outer<<"Version    101"<<endl;
  Outer<<"Geometry   "<<GetGeometryFileName()<<endl;
  Outer<<endl;
  Outer<<"IN "<<InnerFile<<endl;
  Outer<<"IN "<<GetDataDirectory()<<"/ObsTime_4sec_complete.inc1.id1.sim.gz"<<endl;
  Outer<<"EN"<<endl;
  MString OuterFile = GetTempDirectory() + "/LastRead_nested_outer.sim";
  Passed = EvaluateTrue("WriteTextFile()", "sim last read nested outer", "The outer include master can be written", WriteTextFile(OuterFile, Outer.str().c_str())) && Passed;

  MFileEventsSim Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim last read nested open", "The outer include master opens", Reader.Open(OuterFile)) && Passed;

  // All 21 events of the first grandchild and 3 of the second:
  unsigned int NRead = 0;
  MSimEvent* Event = nullptr;
  while (NRead < 24 && (Event = Reader.GetNextEvent(false)) != nullptr) {
    ++NRead;
    delete Event;
  }
  Passed = Evaluate("GetNextEvent()", "sim last read nested 24 events", "24 events can be read", NRead, 24U) && Passed;
  if (NRead == 24 && IDs2.size() >= 3) {
    Passed = Evaluate("GetLastReadEventID()", "sim last read nested stopped in second grandchild", "The last event ID includes the offset of the first grandchild: 21 plus the ID of the 3rd event of the second grandchild", Reader.GetLastReadEventID(), 21L + IDs2[2]) && Passed;
    Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read nested stopped in second grandchild", "The observation time is the 1 s of the first grandchild plus the time of the 3rd event of the second grandchild", Reader.GetLastReadObservationTime().GetAsDouble(), 1.0 + Times2[2], 1e-9) && Passed;
  }
  if (NRead == 24 && SimulationIDs2.size() >= 3) {
    Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read nested stopped in second grandchild", "The count is TS 310 of the first grandchild plus the simulation ID of the 3rd event of the second grandchild", Reader.GetLastReadSimulationEventID(), 310L + SimulationIDs2[2]) && Passed;
  }

  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    delete Event;
  }
  Passed = Evaluate("GetLastReadEventID()", "sim last read nested all events", "After all events the last event ID is the sum of the last IDs 21 + 31 + 73", Reader.GetLastReadEventID(), 125L) && Passed;
  Passed = EvaluateNear("GetLastReadObservationTime()", "sim last read nested all events", "After all events the observation time is the total 1 + 2 + 4 = 7 s", Reader.GetLastReadObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;
  Passed = Evaluate("GetLastReadSimulationEventID()", "sim last read nested all events", "After all events the count is the total 310 + 631 + 1284 = 2225", Reader.GetLastReadSimulationEventID(), 2225L) && Passed;
  Reader.Close();

  return Passed;
}

////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileEventsSim Test;
  return Test.Run() ? 0 : 1;
}
