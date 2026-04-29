/*
 * UTFileEventsTra.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <fstream>
#include <limits>
#include <sstream>
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

  //! Return the temp directory
  MString GetTempDirectory() const;
  //! Return the data directory
  MString GetDataDirectory() const;
  //! Prepare the temp directory
  bool PrepareTempDirectory() const;
  //! Read file text
  MString ReadTextFile(const MString& FileName) const;
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

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the temp directory
MString UTFileEventsTra::GetTempDirectory() const
{
  return "/tmp/UTFileEventsTra";
}


////////////////////////////////////////////////////////////////////////////////


//! Prepare the temp directory
bool UTFileEventsTra::PrepareTempDirectory() const
{
  gSystem->Exec(("rm -rf " + GetTempDirectory()).Data());
  return gSystem->mkdir(GetTempDirectory(), true) == 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the data directory
MString UTFileEventsTra::GetDataDirectory() const
{
  return MString(gSystem->pwd()) + "/src/global/misc/unittests/data/UTFileEventsTra";
}


////////////////////////////////////////////////////////////////////////////////


//! Read file text
MString UTFileEventsTra::ReadTextFile(const MString& FileName) const
{
  ifstream In(FileName.Data());
  ostringstream Buffer;
  Buffer << In.rdbuf();
  return Buffer.str().c_str();
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

  Passed = EvaluateNear("GetObservationTime()", "normal tra1 observation time", "The 1-second complete tra fixture preserves its observation time", Time1, 1.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal tra2 observation time", "The 2-second complete tra fixture preserves its observation time", Time2, 2.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal tra4 observation time", "The 4-second complete tra fixture preserves its observation time", Time4, 4.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal trax observation time", "The concatenated complete tra fixture preserves the summed observation time", TimeX, 7.0, 1e-9) && Passed;

  Passed = EvaluateNear("GetObservationTime()", "normal incomplete tra1 observation time", "The 1-second incomplete tra fixture falls back to the last TI value", TimeIncomplete1, 0.909178909, 1e-9) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal incomplete tra2 observation time", "The 2-second incomplete tra fixture falls back to the last TI value", TimeIncomplete2, 1.974501093, 1e-9) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal incomplete tra4 observation time", "The 4-second incomplete tra fixture falls back to the last TI value", TimeIncomplete4, 3.868021586, 1e-9) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "normal incomplete trax observation time", "The concatenated incomplete tra fixture preserves the summed observation time", TimeIncompleteX, 6.751701588, 1e-8) && Passed;

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

  Passed = EvaluateTrue("PrepareTempDirectory()", "open temp dir", "The temporary directory for MFileEventsTra tests can be created", PrepareTempDirectory()) && Passed;

  {
    MFileEventsTra File;
    DisableDefaultStreams();
    bool Opened = File.Open(GetTempDirectory() + "/invalid.txt");
    EnableDefaultStreams();
    Passed = EvaluateFalse("Open(invalid extension)", "invalid extension", "Files without tra extension are rejected", Opened) && Passed;
  }

  {
    MString FileName = GetTempDirectory() + "/guards.tra";
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
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
    Passed = EvaluateTrue("AddEvent()", "guards event content", "The file contains the unidentifiable-event markers", Text.Contains("ET UN") && Text.Contains("PE 12.5")) && Passed;
    Passed = EvaluateTrue("AddText()", "guards text content", "The file contains the appended raw text", Text.Contains("CC extra")) && Passed;

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

  Passed = EvaluateTrue("PrepareTempDirectory()", "round-trip temp dir", "The temporary directory can be recreated for round-trip tests", PrepareTempDirectory()) && Passed;

  MString FileName = GetTempDirectory() + "/roundtrip.tra";

  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
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

  Passed = EvaluateTrue("PrepareTempDirectory()", "include temp dir", "The temporary directory can be recreated for include-file tests", PrepareTempDirectory()) && Passed;

  MString MainFile = GetTempDirectory() + "/main.tra";
  MString IncludeFile = GetTempDirectory() + "/include.tra";

  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "include main open", "The main tra file opens in write mode", Writer.Open(MainFile, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "include main header", "The main tra header is written", Writer.WriteHeader()) && Passed;
    Passed = EvaluateTrue("AddText(IN)", "include main include line", "The main tra file can reference an include file", Writer.AddText("IN include.tra\n")) && Passed;
    Writer.SetObservationTime(MTime(5.0));
    Passed = EvaluateTrue("CloseEventList()", "include main close", "The main tra file closes its event list", Writer.CloseEventList()) && Passed;
    Writer.Close();
  }

  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
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

  Passed = EvaluateTrue("PrepareTempDirectory()", "eof temp dir", "The temporary directory can be recreated for EOF tests", PrepareTempDirectory()) && Passed;

  MString FileName = GetTempDirectory() + "/empty.tra";
  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
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

  Passed = EvaluateTrue("PrepareTempDirectory()", "mode temp dir", "The temporary directory can be recreated for parser-mode tests", PrepareTempDirectory()) && Passed;

  MString FileName = GetTempDirectory() + "/modes.tra";
  {
    MFileEventsTra Writer;
    Writer.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
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
