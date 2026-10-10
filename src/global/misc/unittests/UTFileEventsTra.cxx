/*
 * UTFileEventsTra.cxx
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
#include <cstring>
#include <limits>
using namespace std;

// ROOT libs:
#include "TSystem.h"

// MEGAlib:
#include "MFileEventsTra.h"
#include "MStreams.h"
#include "MUnidentifiableEvent.h"
#include "MUnitTest.h"


//! Unit test class for MFileEventsTra
class UTFileEventsTra : public MUnitTest
{
public:
  //! Default constructor
  UTFileEventsTra() : MUnitTest("UTFileEventsTra") {}
  //! Default destructor
  virtual ~UTFileEventsTra() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the normal external file-processing workflow
  bool TestNormalOperationObservationTime();
  //! Test open and mode guards
  bool TestOpenAndGuards();
  //! Test a simple read/write round-trip
  bool TestRoundTrip();
  //! Test include-file reading
  bool TestIncludeFiles();
  //! Test EOF and no-event behavior
  bool TestEOFBehavior();
  //! Test threaded and parser-mode variants
  bool TestParserModesAndThreading();
  //! Test the header and footer information (Geant4 version, seed, beam, spectrum, times, simulated events)
  bool TestHeaderAndFooterInformation();

  //! Return the data directory
  MString GetDataDirectory() const;
  //! Read the observation time from a tra file
  bool ReadObservationTime(const MString& FileName, double& ObservationTime) const;
  //! Create a minimal unidentifiable event
  MUnidentifiableEvent CreateUnidentifiableEvent(long Id, double Time, double Energy) const;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTFileEventsTra::Run()
{
  bool Passed = true;

  Passed = TestNormalOperationObservationTime() && Passed;
  Passed = TestOpenAndGuards() && Passed;
  Passed = TestRoundTrip() && Passed;
  Passed = TestIncludeFiles() && Passed;
  Passed = TestEOFBehavior() && Passed;
  Passed = TestParserModesAndThreading() && Passed;
  Passed = TestHeaderAndFooterInformation() && Passed;

  Summarize();

  return Passed;
}


//! Return the data directory
MString UTFileEventsTra::GetDataDirectory() const
{
  return MString(gSystem->pwd()) + "/src/global/misc/unittests/data/UTFileEventsTra";
}


////////////////////////////////////////////////////////////////////////////////


//! Read the observation time from a tra file
bool UTFileEventsTra::ReadObservationTime(const MString& FileName, double& ObservationTime) const
{
  ObservationTime = numeric_limits<double>::quiet_NaN();

  MFileEventsTra Reader;
  if (Reader.Open(FileName) == false) {
    return false;
  }

  ObservationTime = Reader.GetObservationTime().GetAsDouble();
  Reader.Close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the normal external file-processing workflow
bool UTFileEventsTra::TestNormalOperationObservationTime()
{
  bool Passed = true;

  MString BaseDirectory = GetDataDirectory();
  Passed = EvaluateTrue("AccessPathName()", "normal data dir", "The fixture directory for MFileEventsTra exists", gSystem->AccessPathName(BaseDirectory) == false) && Passed;

  MString SourceFile = BaseDirectory + "/Max.source";
  MString ConfigFile = BaseDirectory + "/Max.cfg";
  Passed = EvaluateTrue("AccessPathName()", "normal source fixture", "The COSIMA source fixture exists", gSystem->AccessPathName(SourceFile) == false) && Passed;
  Passed = EvaluateTrue("AccessPathName()", "normal cfg fixture", "The Revan configuration fixture exists", gSystem->AccessPathName(ConfigFile) == false) && Passed;

  MString CompleteTra1 = BaseDirectory + "/ObsTime_1sec_complete.inc1.id1.tra.gz";
  MString CompleteTra2 = BaseDirectory + "/ObsTime_2sec_complete.inc1.id1.tra.gz";
  MString CompleteTra4 = BaseDirectory + "/ObsTime_4sec_complete.inc1.id1.tra.gz";
  MString CompleteTraX = BaseDirectory + "/ObsTime_Xsec_complete.tra.gz";
  MString IncompleteTra1 = BaseDirectory + "/ObsTime_1sec_incomplete.inc1.id1.tra.gz";
  MString IncompleteTra2 = BaseDirectory + "/ObsTime_2sec_incomplete.inc1.id1.tra.gz";
  MString IncompleteTra4 = BaseDirectory + "/ObsTime_4sec_incomplete.inc1.id1.tra.gz";
  MString IncompleteTraX = BaseDirectory + "/ObsTime_Xsec_incomplete.tra.gz";

  Passed = EvaluateTrue("AccessPathName()", "normal complete fixtures", "The complete tra fixtures exist",
    gSystem->AccessPathName(CompleteTra1) == false &&
    gSystem->AccessPathName(CompleteTra2) == false &&
    gSystem->AccessPathName(CompleteTra4) == false &&
    gSystem->AccessPathName(CompleteTraX) == false) && Passed;
  Passed = EvaluateTrue("AccessPathName()", "normal incomplete fixtures", "The incomplete tra fixtures exist",
    gSystem->AccessPathName(IncompleteTra1) == false &&
    gSystem->AccessPathName(IncompleteTra2) == false &&
    gSystem->AccessPathName(IncompleteTra4) == false &&
    gSystem->AccessPathName(IncompleteTraX) == false) && Passed;

  double Time1 = 0.0;
  double Time2 = 0.0;
  double Time4 = 0.0;
  double TimeX = 0.0;
  double TimeIncomplete1 = 0.0;
  double TimeIncomplete2 = 0.0;
  double TimeIncomplete4 = 0.0;
  double TimeIncompleteX = 0.0;
  Passed = EvaluateTrue("ReadObservationTime()", "normal tra1 open", "The 1-second complete tra fixture can be opened", ReadObservationTime(CompleteTra1, Time1)) && Passed;
  Passed = EvaluateTrue("ReadObservationTime()", "normal tra2 open", "The 2-second complete tra fixture can be opened", ReadObservationTime(CompleteTra2, Time2)) && Passed;
  Passed = EvaluateTrue("ReadObservationTime()", "normal tra4 open", "The 4-second complete tra fixture can be opened", ReadObservationTime(CompleteTra4, Time4)) && Passed;
  Passed = EvaluateTrue("ReadObservationTime()", "normal trax open", "The concatenated complete tra fixture can be opened", ReadObservationTime(CompleteTraX, TimeX)) && Passed;
  Passed = EvaluateTrue("ReadObservationTime()", "normal incomplete tra1 open", "The 1-second incomplete tra fixture can be opened", ReadObservationTime(IncompleteTra1, TimeIncomplete1)) && Passed;
  Passed = EvaluateTrue("ReadObservationTime()", "normal incomplete tra2 open", "The 2-second incomplete tra fixture can be opened", ReadObservationTime(IncompleteTra2, TimeIncomplete2)) && Passed;
  Passed = EvaluateTrue("ReadObservationTime()", "normal incomplete tra4 open", "The 4-second incomplete tra fixture can be opened", ReadObservationTime(IncompleteTra4, TimeIncomplete4)) && Passed;
  Passed = EvaluateTrue("ReadObservationTime()", "normal incomplete trax open", "The concatenated incomplete tra fixture can be opened", ReadObservationTime(IncompleteTraX, TimeIncompleteX)) && Passed;

  Passed = EvaluateNear("GetObservationTime()", "normal tra1 observation time", "The 1-second complete tra fixture preserves its observation time", Time1, 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal tra2 observation time", "The 2-second complete tra fixture preserves its observation time", Time2, 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal tra4 observation time", "The 4-second complete tra fixture preserves its observation time", Time4, 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal trax observation time", "The concatenated complete tra fixture preserves the summed observation time", TimeX, 7.0, 1e-12) && Passed;

  Passed = EvaluateNear("GetObservationTime()", "normal incomplete tra1 observation time", "The 1-second incomplete tra fixture falls back to the last TI value", TimeIncomplete1, 0.909178909, 1e-12) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal incomplete tra2 observation time", "The 2-second incomplete tra fixture falls back to the last TI value", TimeIncomplete2, 1.974501093, 1e-12) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal incomplete tra4 observation time", "The 4-second incomplete tra fixture falls back to the last TI value", TimeIncomplete4, 3.868021586, 1e-12) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal incomplete trax observation time", "The concatenated incomplete tra fixture preserves the summed observation time", TimeIncompleteX, 6.751701588, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the header and footer information (Geant4 version, seed, beam, spectrum, times, simulated events)
bool UTFileEventsTra::TestHeaderAndFooterInformation()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "information temp dir", "The temporary directory for the header and footer information tests can be created", PrepareTemporaryDirectory("information")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("information");

  MString FullFileName = TemporaryDirectory + "/full.tra";
  MString ReusedFileName = TemporaryDirectory + "/reused.tra";

  MFileEventsTra Writer;
  Writer.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
  Writer.SetGeant4Version("11.2.2");
  Writer.SetSimulationStartAreaFarField(123.5);
  Writer.SetSimulationSeed(4242);
  Writer.SetBeamType("FarFieldPointSource 0 0");
  Writer.SetSpectralType("Mono 511");
  Writer.SetStartObservationTime(MTime(10.0));
  Writer.SetEndObservationTime(MTime(30.0));
  Writer.SetSimulatedEvents(1000);
  Passed = EvaluateTrue("Open(write)", "full open", "The tra file with all header and footer information opens in write mode", Writer.Open(FullFileName, MFile::c_Write)) && Passed;
  Passed = EvaluateTrue("WriteHeader()", "full header", "WriteHeader succeeds with all header information set", Writer.WriteHeader()) && Passed;
  MUnidentifiableEvent Event = CreateUnidentifiableEvent(1, 12.0, 100.0);
  Passed = EvaluateTrue("AddEvent()", "full event", "An event can be added to the tra file with all information", Writer.AddEvent(&Event)) && Passed;
  Passed = EvaluateTrue("AddFooter()", "full footer text", "AddFooter accepts the footer text", Writer.AddFooter("FooterText")) && Passed;
  Passed = EvaluateTrue("WriteFooter()", "full footer", "WriteFooter succeeds", Writer.WriteFooter()) && Passed;
  Passed = EvaluateTrue("Close()", "full close", "The tra file with all information closes cleanly", Writer.Close()) && Passed;

  MString Text = ReadTextFile(FullFileName);
  // The TB line is written once: count the lines starting with TB
  unsigned int TBLines = 0;
  for (const char* Position = strstr(Text.Data(), "\nTB "); Position != nullptr; Position = strstr(Position + 1, "\nTB ")) {
    ++TBLines;
  }
  Passed = Evaluate("WriteHeader()", "start time line count", "The start of the observation time is written exactly once (one TB line)", TBLines, 1U) && Passed;
  // Footer: EN, two blank lines (EN + 2 line breaks), TE end time, TS simulated events, blank line, FT block, blank line
  Passed = EvaluateTrue("WriteFooter()", "footer content", "The file ends with EN, the end time (TE), the simulated events (TS), and the footer text block",
                        Text.EndsWith("EN\n\n\nTE 30.000000000\nTS 1000\n\nFT START\nFooterText\nFT STOP\n\n")) && Passed;

  // The information survives a read
  MFileEventsTra Reader;
  Passed = EvaluateTrue("Open(read)", "full read", "The tra file with all information opens in read mode", Reader.Open(FullFileName)) && Passed;
  Passed = Evaluate("HasGeant4Version()", "full read", "The Geant4 version is read from the header", Reader.HasGeant4Version(), true) && Passed;
  Passed = Evaluate("GetGeant4Version()", "full read", "The Geant4 version is read exactly", Reader.GetGeant4Version(), MString("11.2.2")) && Passed;
  Passed = Evaluate("HasSimulationStartAreaFarField()", "full read", "The far-field start area is read from the header", Reader.HasSimulationStartAreaFarField(), true) && Passed;
  Passed = EvaluateNear("GetSimulationStartAreaFarField()", "full read", "The far-field start area is read exactly", Reader.GetSimulationStartAreaFarField(), 123.5, 1e-12) && Passed;
  Passed = Evaluate("HasSimulationSeed()", "full read", "The simulation seed is read from the header", Reader.HasSimulationSeed(), true) && Passed;
  Passed = Evaluate("GetSimulationSeed()", "full read", "The simulation seed is read exactly", Reader.GetSimulationSeed(), 4242UL) && Passed;
  Passed = Evaluate("HasBeamType()", "full read", "The beam type is read from the header", Reader.HasBeamType(), true) && Passed;
  Passed = Evaluate("GetBeamType()", "full read", "The beam type is read exactly", Reader.GetBeamType(), MString("FarFieldPointSource 0 0")) && Passed;
  Passed = Evaluate("HasSpectralType()", "full read", "The spectral type is read from the header", Reader.HasSpectralType(), true) && Passed;
  Passed = Evaluate("GetSpectralType()", "full read", "The spectral type is read exactly", Reader.GetSpectralType(), MString("Mono 511")) && Passed;
  Passed = Evaluate("HasStartObservationTime()", "full read", "The start of the observation time is read from the header", Reader.HasStartObservationTime(), true) && Passed;
  Passed = EvaluateNear("GetStartObservationTime()", "full read", "The start of the observation time is read exactly", Reader.GetStartObservationTime().GetAsSeconds(), 10.0, 1e-12) && Passed;
  // End time minus start time = 30 - 10
  Passed = EvaluateNear("GetObservationTime()", "full read", "The observation time is the end minus the start time", Reader.GetObservationTime().GetAsSeconds(), 20.0, 1e-12) && Passed;
  Passed = Evaluate("HasEndObservationTime()", "full read", "The end of the observation time is read from the footer", Reader.HasEndObservationTime(), true) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "full read", "The number of simulated events is read from the footer", Reader.GetSimulatedEvents(), 1000L) && Passed;

  // TransferInformation copies everything, including the number of simulated events from the not yet read footer
  {
    MFileEventsTra Source;
    Passed = EvaluateTrue("Open(read)", "transfer source", "The source of the information transfer opens", Source.Open(FullFileName)) && Passed;
    MFileEventsTra Target;
    Target.TransferInformation(&Source);
    Passed = Evaluate("TransferInformation()", "Geant4 version", "The Geant4 version is transferred", Target.GetGeant4Version(), MString("11.2.2")) && Passed;
    Passed = EvaluateNear("TransferInformation()", "far-field area", "The far-field start area is transferred", Target.GetSimulationStartAreaFarField(), 123.5, 1e-12) && Passed;
    Passed = Evaluate("TransferInformation()", "seed", "The simulation seed is transferred", Target.GetSimulationSeed(), 4242UL) && Passed;
    Passed = Evaluate("TransferInformation()", "beam type", "The beam type is transferred", Target.GetBeamType(), MString("FarFieldPointSource 0 0")) && Passed;
    Passed = Evaluate("TransferInformation()", "spectral type", "The spectral type is transferred", Target.GetSpectralType(), MString("Mono 511")) && Passed;
    Passed = EvaluateNear("TransferInformation()", "start time", "The start of the observation time is transferred", Target.GetStartObservationTime().GetAsSeconds(), 10.0, 1e-12) && Passed;
    const bool HasSimulatedEvents = Target.HasSimulatedEvents();
    Passed = Evaluate("TransferInformation()", "simulated events flag", "The information that the number of simulated events is known is transferred", HasSimulatedEvents, true) && Passed;
    if (HasSimulatedEvents == true) {
      Passed = Evaluate("TransferInformation()", "simulated events", "The number of simulated events is transferred", Target.GetSimulatedEvents(), 1000L) && Passed;
    }
  }

  // A reader which is reused for a second file does not keep the information of the first one
  {
    MFileEventsTra Plain;
    Plain.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "plain open", "The tra file without simulation information opens in write mode", Plain.Open(ReusedFileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "plain header", "WriteHeader succeeds without simulation information", Plain.WriteHeader()) && Passed;
    Passed = EvaluateTrue("AddEvent()", "plain event", "An event can be added to the tra file without simulation information", Plain.AddEvent(&Event)) && Passed;
    Passed = EvaluateTrue("WriteFooter()", "plain footer", "WriteFooter succeeds without simulation information", Plain.WriteFooter()) && Passed;
    Passed = EvaluateTrue("Close()", "plain close", "The tra file without simulation information closes cleanly", Plain.Close()) && Passed;

    Passed = EvaluateTrue("Close()", "reader close", "The reader of the full file closes", Reader.Close()) && Passed;
    Passed = EvaluateTrue("Open(read)", "reused open", "The reader opens a second file", Reader.Open(ReusedFileName)) && Passed;
    Passed = Evaluate("HasGeant4Version()", "reused read", "A reused reader forgets the Geant4 version of the previous file", Reader.HasGeant4Version(), false) && Passed;
    Passed = Evaluate("HasSimulationSeed()", "reused read", "A reused reader forgets the simulation seed of the previous file", Reader.HasSimulationSeed(), false) && Passed;
    Passed = Evaluate("HasStartObservationTime()", "reused read", "A reused reader forgets the start time of the previous file", Reader.HasStartObservationTime(), false) && Passed;
    Passed = Evaluate("GetSimulatedEvents()", "reused read", "A reused reader forgets the number of simulated events of the previous file", Reader.GetSimulatedEvents(), 0L) && Passed;
    Passed = Evaluate("HasSimulatedEvents()", "reused read", "A reused reader knows no number of simulated events for a file without TS", Reader.HasSimulatedEvents(), false) && Passed;
    Reader.Close();
  }

  // A writer which is reused for a second file does not write the footer text of the first one
  {
    MString SecondFileName = TemporaryDirectory + "/second.tra";
    Passed = EvaluateTrue("Open(write)", "second open", "The writer opens a second file", Writer.Open(SecondFileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "second header", "WriteHeader succeeds for the second file", Writer.WriteHeader()) && Passed;
    Passed = EvaluateTrue("WriteFooter()", "second footer", "WriteFooter succeeds for the second file", Writer.WriteFooter()) && Passed;
    Passed = EvaluateTrue("Close()", "second close", "The second file closes cleanly", Writer.Close()) && Passed;
    Passed = EvaluateFalse("WriteFooter()", "second footer text", "The footer text of the first file is not written into the second file", ReadTextFile(SecondFileName).Contains("FT START")) && Passed;
  }

  // Without an end time, the footer uses the observation time (the time at which the simulation finished) as TE
  {
    MFileEventsTra OnlyObservationTime;
    MString OnlyObservationTimeFileName = TemporaryDirectory + "/only_observation_time.tra";
    OnlyObservationTime.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    OnlyObservationTime.SetObservationTime(MTime(9.0));
    Passed = EvaluateTrue("Open(write)", "observation time open", "The tra file with only an observation time opens in write mode", OnlyObservationTime.Open(OnlyObservationTimeFileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "observation time header", "WriteHeader succeeds with only an observation time", OnlyObservationTime.WriteHeader()) && Passed;
    Passed = EvaluateTrue("WriteFooter()", "observation time footer", "WriteFooter succeeds with only an observation time", OnlyObservationTime.WriteFooter()) && Passed;
    Passed = EvaluateTrue("Close()", "observation time close", "The tra file with only an observation time closes cleanly", OnlyObservationTime.Close()) && Passed;
    Passed = EvaluateTrue("WriteFooter()", "TE from observation time", "Without start and end time, TE is the observation time", ReadTextFile(OnlyObservationTimeFileName).Contains("\nTE 9.000000000\n")) && Passed;

    MFileEventsTra StartAndObservationTime;
    MString StartAndObservationTimeFileName = TemporaryDirectory + "/start_and_observation_time.tra";
    StartAndObservationTime.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    StartAndObservationTime.SetStartObservationTime(MTime(10.0));
    StartAndObservationTime.SetObservationTime(MTime(20.0));
    Passed = EvaluateTrue("Open(write)", "start and observation time open", "The tra file with a start and an observation time opens in write mode", StartAndObservationTime.Open(StartAndObservationTimeFileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "start and observation time header", "WriteHeader succeeds with a start and an observation time", StartAndObservationTime.WriteHeader()) && Passed;
    Passed = EvaluateTrue("WriteFooter()", "start and observation time footer", "WriteFooter succeeds with a start and an observation time", StartAndObservationTime.WriteFooter()) && Passed;
    Passed = EvaluateTrue("Close()", "start and observation time close", "The tra file with a start and an observation time closes cleanly", StartAndObservationTime.Close()) && Passed;
    // End time = start + observation time = 10 + 20
    Passed = EvaluateTrue("WriteFooter()", "TE from start and observation time", "Without end time, TE is the start time plus the observation time", ReadTextFile(StartAndObservationTimeFileName).Contains("\nTE 30.000000000\n")) && Passed;
  }

  // CloseEventList: without an end time, TE is the start time plus the observation time
  {
    MFileEventsTra NoEnd;
    MString NoEndFileName = TemporaryDirectory + "/close_no_end.tra";
    NoEnd.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    NoEnd.SetStartObservationTime(MTime(10.0));
    NoEnd.SetObservationTime(MTime(20.0));
    Passed = EvaluateTrue("Open(write)", "close no end open", "The tra file without end time opens in write mode", NoEnd.Open(NoEndFileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "close no end header", "WriteHeader succeeds without an end time", NoEnd.WriteHeader()) && Passed;
    Passed = EvaluateTrue("CloseEventList()", "close no end list", "CloseEventList succeeds without an end time", NoEnd.CloseEventList()) && Passed;
    Passed = EvaluateTrue("Close()", "close no end close", "The tra file without end time closes cleanly", NoEnd.Close()) && Passed;
    // End time = start + observation time = 10 + 20
    Passed = EvaluateTrue("CloseEventList()", "close no end TE", "TE is the start time plus the observation time", ReadTextFile(NoEndFileName).Contains("\nTE 30.000000000\n")) && Passed;

    MFileEventsTra NoEndReader;
    Passed = EvaluateTrue("Open(read)", "close no end read", "The tra file without end time opens in read mode", NoEndReader.Open(NoEndFileName)) && Passed;
    Passed = EvaluateNear("GetObservationTime()", "close no end read", "The observation time survives writing without an end time", NoEndReader.GetObservationTime().GetAsSeconds(), 20.0, 1e-12) && Passed;
    NoEndReader.Close();
  }

  // A footer added after the event list has been closed is still written (ConvertMGGPOD does it in this order)
  {
    MString LateFileName = TemporaryDirectory + "/late.tra";
    MFileEventsTra Late;
    Late.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "late open", "The tra file for the late footer opens in write mode", Late.Open(LateFileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "late header", "WriteHeader succeeds for the late footer", Late.WriteHeader()) && Passed;
    Passed = EvaluateTrue("CloseEventList()", "late close list", "CloseEventList succeeds before the late footer", Late.CloseEventList()) && Passed;
    Passed = EvaluateTrue("AddFooter()", "late footer", "AddFooter succeeds after CloseEventList", Late.AddFooter("LateText")) && Passed;
    Passed = EvaluateTrue("Close()", "late close", "The tra file for the late footer closes cleanly", Late.Close()) && Passed;
    Passed = EvaluateTrue("AddFooter()", "late footer content", "A footer added after CloseEventList is in the file", ReadTextFile(LateFileName).Contains("FT START\nLateText\nFT STOP")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Create a minimal unidentifiable event
MUnidentifiableEvent UTFileEventsTra::CreateUnidentifiableEvent(long Id, double Time, double Energy) const
{
  MUnidentifiableEvent Event;
  Event.SetId(Id);
  Event.SetTime(MTime(Time));
  Event.SetEnergy(Energy);
  return Event;
}


////////////////////////////////////////////////////////////////////////////////


//! Test open and mode guards
bool UTFileEventsTra::TestOpenAndGuards()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "open temp dir", "The temporary directory for MFileEventsTra tests can be created", PrepareTemporaryDirectory("open")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("open");

  {
    MFileEventsTra File;
    DisableDefaultStreams();
    bool Opened = File.Open(TemporaryDirectory + "/invalid.txt");
    EnableDefaultStreams();
    Passed = EvaluateFalse("Open(invalid extension)", "invalid extension", "Files without tra extension are rejected", Opened) && Passed;
  }

  {
    MString FileName = TemporaryDirectory + "/guards.tra";
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "guards open write", "A tra file opens in write mode", Writer.Open(FileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "guards header", "WriteHeader succeeds for tra files", Writer.WriteHeader()) && Passed;

    MUnidentifiableEvent Event = CreateUnidentifiableEvent(7, 5.5, 12.5);
    Passed = EvaluateTrue("AddEvent()", "guards add event", "A valid physical event can be added in write mode", Writer.AddEvent(&Event)) && Passed;
    Passed = EvaluateTrue("AddText()", "guards add text", "Additional text can be added in write mode", Writer.AddText("CC extra\n")) && Passed;
    Passed = EvaluateFalse("AddEvent(nullptr)", "guards null event", "A null event is rejected", Writer.AddEvent(nullptr)) && Passed;
    Writer.SetObservationTime(MTime(9.0));
    Passed = EvaluateTrue("CloseEventList()", "guards close list", "CloseEventList succeeds in write mode", Writer.CloseEventList()) && Passed;
    Passed = EvaluateTrue("Close()", "guards close", "The written tra file closes cleanly", Writer.Close()) && Passed;

    MString Text = ReadTextFile(FileName);
    MString ExpectedTail = MString("SE\nET UN\nID 7\nTI 5.500000000\nPE 12.5\nCC extra\nEN\n\nTE 9.000000000\n\n");
    Passed = EvaluateTrue("AddEvent()", "guards exact event tail", "The written tra file ends with the exact deterministic event payload and trailer", Text.EndsWith(ExpectedTail)) && Passed;

    MFileEventsTra Reader;
    Passed = EvaluateTrue("Open(read)", "guards open read", "The file reopens in read mode", Reader.Open(FileName)) && Passed;
#ifdef NDEBUG
    DisableDefaultStreams();
    Passed = EvaluateFalse("AddText()", "guards read add text", "AddText is rejected in read mode", Reader.AddText("CC fail\n")) && Passed;
    Passed = EvaluateFalse("AddEvent()", "guards read add event", "AddEvent is rejected in read mode", Reader.AddEvent(&Event)) && Passed;
    EnableDefaultStreams();
#endif
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a simple read/write round-trip
bool UTFileEventsTra::TestRoundTrip()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "round-trip temp dir", "The temporary directory can be recreated for round-trip tests", PrepareTemporaryDirectory("roundtrip")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("roundtrip");

  MString FileName = TemporaryDirectory + "/roundtrip.tra";

  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "round-trip open write", "The round-trip tra file opens in write mode", Writer.Open(FileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "round-trip header", "The round-trip file header is written", Writer.WriteHeader()) && Passed;

    MUnidentifiableEvent Event = CreateUnidentifiableEvent(42, 11.25, 77.5);
    Passed = EvaluateTrue("AddEvent()", "round-trip add event", "The event is written to the tra file", Writer.AddEvent(&Event)) && Passed;

    Writer.SetObservationTime(MTime(11.25));
    Passed = EvaluateTrue("CloseEventList()", "round-trip close list", "The event list is closed cleanly", Writer.CloseEventList()) && Passed;
    Writer.Close();
  }

  {
    MFileEventsTra Reader;
    Passed = EvaluateTrue("Open(read)", "round-trip open read", "The round-trip file opens in read mode", Reader.Open(FileName)) && Passed;
    MPhysicalEvent* Physical = Reader.GetNextEvent();
    Passed = EvaluateTrue("GetNextEvent()", "round-trip first event", "GetNextEvent returns the stored event", Physical != nullptr) && Passed;
    if (Physical != nullptr) {
      Passed = Evaluate("GetType()", "round-trip type", "The stored event type round-trips through the tra file", Physical->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
      Passed = Evaluate("GetId()", "round-trip id", "The stored event id round-trips through the tra file", Physical->GetId(), 42L) && Passed;
      Passed = EvaluateNear("GetTime()", "round-trip time", "The stored event time round-trips through the tra file", Physical->GetTime().GetAsDouble(), 11.25, 1e-12) && Passed;

      MUnidentifiableEvent* Un = dynamic_cast<MUnidentifiableEvent*>(Physical);
      Passed = EvaluateTrue("dynamic_cast<MUnidentifiableEvent*>", "round-trip cast", "The returned event can be cast back to MUnidentifiableEvent", Un != nullptr) && Passed;
      if (Un != nullptr) {
        Passed = EvaluateNear("GetEnergy()", "round-trip energy", "The stored unidentifiable-event energy round-trips through the tra file", Un->GetEnergy(), 77.5, 1e-12) && Passed;
      }
      delete Physical;
    }

    Passed = Evaluate("GetObservationTime()", "round-trip observation time", "The written observation time is available from the file footer", Reader.GetObservationTime().GetAsDouble(), 11.25) && Passed;
    Passed = Evaluate("GetNextEvent()", "round-trip eof", "After the only event, GetNextEvent returns null", Reader.GetNextEvent() == nullptr, true) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test include-file reading
bool UTFileEventsTra::TestIncludeFiles()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "include temp dir", "The temporary directory can be recreated for include-file tests", PrepareTemporaryDirectory("include")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("include");

  MString MainFile = TemporaryDirectory + "/main.tra";
  MString IncludeFile = TemporaryDirectory + "/include.tra";

  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "include main open", "The main tra file opens in write mode", Writer.Open(MainFile, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "include main header", "The main tra header is written", Writer.WriteHeader()) && Passed;
    Passed = EvaluateTrue("AddText(IN)", "include main include line", "The main tra file can reference an include file", Writer.AddText("IN include.tra\n")) && Passed;
    Writer.SetObservationTime(MTime(5.0));
    Passed = EvaluateTrue("CloseEventList()", "include main close", "The main tra file closes its event list", Writer.CloseEventList()) && Passed;
    Writer.Close();
  }

  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "include child open", "The include tra file opens in write mode", Writer.Open(IncludeFile, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "include child header", "The include tra header is written", Writer.WriteHeader()) && Passed;
    MUnidentifiableEvent Event = CreateUnidentifiableEvent(3, 7.0, 22.0);
    Passed = EvaluateTrue("AddEvent()", "include child add event", "The include tra file stores one event", Writer.AddEvent(&Event)) && Passed;
    Writer.SetObservationTime(MTime(7.0));
    Passed = EvaluateTrue("CloseEventList()", "include child close", "The include tra file closes its event list", Writer.CloseEventList()) && Passed;
    Writer.Close();
  }

  {
    MFileEventsTra Reader;
    Passed = EvaluateTrue("Open(read)", "include reader open", "The main tra file opens in read mode", Reader.Open(MainFile)) && Passed;
    DisableDefaultStreams();
    MPhysicalEvent* Physical = Reader.GetNextEvent();
    EnableDefaultStreams();
    Passed = EvaluateTrue("GetNextEvent()", "include first event", "GetNextEvent follows the include-file directive and returns the child event", Physical != nullptr) && Passed;
    if (Physical != nullptr) {
      Passed = Evaluate("GetId()", "include event id", "The event id from the include file is returned", Physical->GetId(), 3L) && Passed;
      Passed = EvaluateNear("GetTime()", "include event time", "The event time from the include file is returned", Physical->GetTime().GetAsDouble(), 7.0, 1e-12) && Passed;
      delete Physical;
    }
    Passed = EvaluateNear("GetObservationTime()", "include observation time", "Observation time accumulates the main and include-file observation times", Reader.GetObservationTime().GetAsDouble(), 12.0, 1e-12) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test EOF and no-event behavior
bool UTFileEventsTra::TestEOFBehavior()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "eof temp dir", "The temporary directory can be recreated for EOF tests", PrepareTemporaryDirectory("eof")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("eof");

  MString FileName = TemporaryDirectory + "/empty.tra";
  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "eof open write", "The empty tra file opens in write mode", Writer.Open(FileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "eof header", "The empty tra file header is written", Writer.WriteHeader()) && Passed;
    Writer.SetObservationTime(MTime(3.0));
    Passed = EvaluateTrue("CloseEventList()", "eof close list", "The empty tra file still writes EN and TE", Writer.CloseEventList()) && Passed;
    Writer.Close();
  }

  {
    MFileEventsTra Reader;
    Passed = EvaluateTrue("Open(read)", "eof open read", "The empty tra file opens in read mode", Reader.Open(FileName)) && Passed;
    Passed = Evaluate("GetNextEvent()", "eof no event", "A tra file with no events returns null immediately", Reader.GetNextEvent() == nullptr, true) && Passed;
    Passed = EvaluateNear("GetObservationTime()", "eof observation time", "Observation time is still available from the footer in an empty file", Reader.GetObservationTime().GetAsDouble(), 3.0, 1e-12) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test threaded and parser-mode variants
bool UTFileEventsTra::TestParserModesAndThreading()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "mode temp dir", "The temporary directory can be recreated for parser-mode tests", PrepareTemporaryDirectory("mode")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("mode");

  MString FileName = TemporaryDirectory + "/modes.tra";
  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "mode open write", "The mode test tra file opens in write mode", Writer.Open(FileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "mode header", "The mode test header is written", Writer.WriteHeader()) && Passed;
    MUnidentifiableEvent Event = CreateUnidentifiableEvent(9, 2.5, 19.0);
    Passed = EvaluateTrue("AddEvent()", "mode add event", "The mode test event is written", Writer.AddEvent(&Event)) && Passed;
    Writer.SetObservationTime(MTime(2.5));
    Passed = EvaluateTrue("CloseEventList()", "mode close list", "The mode test file closes its event list", Writer.CloseEventList()) && Passed;
    Writer.Close();
  }

  {
    MFileEventsTra Reader;
    Reader.SetFastFileParsing(true);
    Passed = EvaluateTrue("Open(read fast)", "mode fast open", "The file opens in fast parsing mode", Reader.Open(FileName)) && Passed;
    MPhysicalEvent* Physical = Reader.GetNextEvent();
    Passed = EvaluateTrue("GetNextEvent() fast", "mode fast event", "Fast parsing still returns the stored event", Physical != nullptr) && Passed;
    if (Physical != nullptr) {
      Passed = Evaluate("GetId() fast", "mode fast id", "Fast parsing preserves the event id", Physical->GetId(), 9L) && Passed;
      delete Physical;
    }
    Reader.Close();
  }

  {
    MFileEventsTra Reader;
    Reader.SetDelayedFileParsing(true);
    Passed = EvaluateTrue("Open(read delayed)", "mode delayed open", "The file opens in delayed parsing mode", Reader.Open(FileName)) && Passed;
    MPhysicalEvent* Physical = Reader.GetNextEvent();
    Passed = EvaluateTrue("GetNextEvent() delayed", "mode delayed event", "Delayed parsing still returns the stored event", Physical != nullptr) && Passed;
    if (Physical != nullptr) {
      Passed = EvaluateTrue("ParseDelayed() delayed", "mode delayed parse", "Delayed parsing can be completed explicitly by the caller", Physical->ParseDelayed()) && Passed;
      MUnidentifiableEvent* Un = dynamic_cast<MUnidentifiableEvent*>(Physical);
      Passed = EvaluateTrue("dynamic_cast delayed", "mode delayed cast", "Delayed parsing still constructs the correct event subclass", Un != nullptr) && Passed;
      if (Un != nullptr) {
        Passed = EvaluateNear("GetEnergy() delayed", "mode delayed energy", "Delayed parsing preserves the event energy", Un->GetEnergy(), 19.0, 1e-12) && Passed;
      }
      delete Physical;
    }
    Reader.Close();
  }

  {
    MFileEventsTra Reader;
    Passed = EvaluateTrue("Open(read threaded)", "mode threaded open", "The file opens for threaded reading", Reader.Open(FileName)) && Passed;
    DisableDefaultStreams();
    Passed = EvaluateTrue("StartThread()", "mode start thread", "The reader thread starts successfully", Reader.StartThread()) && Passed;
    MPhysicalEvent* Physical = Reader.GetNextEvent();
    EnableDefaultStreams();
    Passed = EvaluateTrue("GetNextEvent() threaded", "mode threaded event", "Threaded reading returns the stored event", Physical != nullptr) && Passed;
    if (Physical != nullptr) {
      Passed = Evaluate("GetId() threaded", "mode threaded id", "Threaded reading preserves the event id", Physical->GetId(), 9L) && Passed;
      delete Physical;
    }
    Passed = Evaluate("GetNextEvent() threaded", "mode threaded eof", "After the only event, threaded reading also returns null", Reader.GetNextEvent() == nullptr, true) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileEventsTra Test;
  return Test.Run() == true ? 0 : 1;
}
