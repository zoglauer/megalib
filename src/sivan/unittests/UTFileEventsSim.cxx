/*
 * UTFileEventsSim.cxx
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

  //! Return the temp directory
  MString GetTempDirectory() const;
  //! Return the shared data directory
  MString GetDataDirectory() const;
  //! Return the test geometry file
  MString GetGeometryFileName() const;
  //! Prepare the temp directory
  bool PrepareTempDirectory() const;
  //! Write a text file
  bool WriteTextFile(const MString& FileName, const MString& Content) const;
  //! Read file text
  MString ReadTextFile(const MString& FileName) const;
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

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsSim::GetTempDirectory() const
{
  return "/tmp/UTFileEventsSim";
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
  gSystem->Exec(("rm -rf " + GetTempDirectory()).Data());
  return gSystem->mkdir(GetTempDirectory(), true) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsSim::WriteTextFile(const MString& FileName, const MString& Content) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) return false;
  Out << Content.Data();
  return Out.good();
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsSim::ReadTextFile(const MString& FileName) const
{
  ifstream In(FileName.Data());
  ostringstream Buffer;
  Buffer << In.rdbuf();
  return Buffer.str().c_str();
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

  Passed = EvaluateNear("GetObservationTime()", "sim direct observation time", "Observation time can be read directly from the footer", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-9) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "sim direct simulated events", "The total simulated-event count is read from TS", Reader.GetSimulatedEvents(), 310L) && Passed;

  MSimEvent* Event = Reader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim first event", "The first simulation event can be read from the fixture", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetID()", "sim first event id", "The first simulation event has the expected id", Event->GetID(), 1L) && Passed;
    Passed = EvaluateNear("GetTime()", "sim first event time", "The first simulation event has the expected time", Event->GetTime().GetAsDouble(), 0.011003748, 1e-9) && Passed;
    Passed = Evaluate("GetNIAs()", "sim first event ia count", "The first simulation event has the expected IA count", Event->GetNIAs(), 7U) && Passed;
    Passed = Evaluate("GetNHTs()", "sim first event ht count", "The first simulation event has the expected HT count", Event->GetNHTs(), 4U) && Passed;
    delete Event;
  }

  long EventCount = 0;
  while ((Event = Reader.GetNextEvent()) != nullptr) {
    ++EventCount;
    delete Event;
  }

  Passed = EvaluateTrue("GetNextEvent()", "sim full scan count", "Scanning the full sim fixture yields more simulation events", EventCount > 0) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "sim full scan observation time", "Observation time remains correct after scanning all simulation events", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-9) && Passed;

  Passed = EvaluateTrue("Rewind()", "sim rewind", "The sim reader can rewind to the start of the file", Reader.Rewind()) && Passed;
  Event = Reader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim rewind first event", "After rewinding, the first event can be read again", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetID()", "sim rewind first event id", "After rewinding, the first event id matches the original first event", Event->GetID(), 1L) && Passed;
    Passed = EvaluateNear("GetTime()", "sim rewind first event time", "After rewinding, the first event time matches the original first event", Event->GetTime().GetAsDouble(), 0.011003748, 1e-9) && Passed;
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

  Passed = EvaluateTrue("GetNextEvent()", "sim include event count", "The include sim file yields simulation events from its children", EventCount > 0) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "sim include observation time", "Observation time sums across included sim files", Reader.GetObservationTime().GetAsDouble(), 7.0, 1e-9) && Passed;
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
    Passed = EvaluateNear("GetObservationTime()", "sim write reopen observation time", "TB and TE round-trip through a footer-only sim file", Reader.GetObservationTime().GetAsDouble(), 3.5, 1e-9) && Passed;
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

  Passed = EvaluateNear("GetObservationTime()", "sim binary input observation time", "The input observation time is available before rewriting", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-9) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "sim binary input simulated events", "The input simulated-event count is available before rewriting", Reader.GetSimulatedEvents(), 310L) && Passed;

  MSimEvent* FirstEvent = Reader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim binary first event", "The first input event can be read before rewinding", FirstEvent != nullptr) && Passed;
  if (FirstEvent != nullptr) {
    Passed = Evaluate("GetID()", "sim binary first event id", "The first input event has the expected id", FirstEvent->GetID(), 1L) && Passed;
    Passed = EvaluateNear("GetTime()", "sim binary first event time", "The first input event has the expected time", FirstEvent->GetTime().GetAsDouble(), 0.011003748, 1e-9) && Passed;
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
  Passed = EvaluateTrue("AddEvent()", "sim binary event count", "The binary round-trip reads at least one event", Count > 0) && Passed;

  Writer.SetObservationTime(Reader.GetObservationTime());
  Writer.SetSimulatedEvents(Reader.GetSimulatedEvents());
  Passed = EvaluateTrue("CloseEventList()", "sim binary close list", "The binary sim output file can be closed", Writer.CloseEventList()) && Passed;
  Passed = EvaluateTrue("Close()", "sim binary close", "The binary sim output file closes cleanly", Writer.Close()) && Passed;
  Reader.Close();

  MFileEventsSim BinaryReader(&Geometry);
  Passed = EvaluateTrue("Open()", "sim binary reopen", "The binary sim output file reopens in read mode", BinaryReader.Open(OutputFile)) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "sim binary round-trip observation time", "Observation time survives the binary round-trip", BinaryReader.GetObservationTime().GetAsDouble(), 1.0, 1e-9) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "sim binary round-trip simulated events", "TS survives the binary round-trip", BinaryReader.GetSimulatedEvents(), 310L) && Passed;

  Event = BinaryReader.GetNextEvent(false);
  Passed = EvaluateTrue("GetNextEvent()", "sim binary reopen first event", "The first event can be read back from the binary output file", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetID()", "sim binary reopen first event id", "The binary round-trip preserves the event id", Event->GetID(), 1L) && Passed;
    Passed = EvaluateNear("GetTime()", "sim binary reopen first event time", "The binary round-trip preserves the event time", Event->GetTime().GetAsDouble(), 0.011003748, 1e-9) && Passed;
    delete Event;
  }

  BinaryReader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileEventsSim Test;
  return Test.Run() ? 0 : 1;
}
