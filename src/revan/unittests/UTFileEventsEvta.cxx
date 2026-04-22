/*
 * UTFileEventsEvta.cxx
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
#include "MFileEventsEvta.h"
#include "MREAMStartInformation.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for MFileEventsEvta
class UTFileEventsEvta : public MUnitTest
{
public:
  //! Default constructor
  UTFileEventsEvta() : MUnitTest("UTFileEventsEvta") {}
  //! Default destructor
  virtual ~UTFileEventsEvta() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test open guards and geometry requirements
  bool TestOpenAndGuards();
  //! Test reading the committed sim fixtures through the evta reader
  bool TestReadSimFixtures();
  //! Test include-file observation time summing
  bool TestIncludeObservationTime();
  //! Test footer-only evta observation-time fallback
  bool TestFooterOnlyObservationTime();
  //! Test rewind behavior on the reader path used by transmitters
  bool TestRewindBehavior();
  //! Test SaveOI support used by the revan analyzer
  bool TestSaveOI();

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
  //! Scan the revan geometry
  bool LoadGeometry(MGeometryRevan& Geometry) const;
};


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::Run()
{
  bool Passed = true;

  Passed = TestOpenAndGuards() && Passed;
  Passed = TestReadSimFixtures() && Passed;
  Passed = TestIncludeObservationTime() && Passed;
  Passed = TestFooterOnlyObservationTime() && Passed;
  Passed = TestRewindBehavior() && Passed;
  Passed = TestSaveOI() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsEvta::GetTempDirectory() const
{
  return "/tmp/UTFileEventsEvta";
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsEvta::GetDataDirectory() const
{
  return MString(gSystem->pwd()) + "/src/global/misc/unittests/data/UTFileEventsTra";
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsEvta::GetGeometryFileName() const
{
  return MString(gSystem->pwd()) + "/resource/examples/geomega/special/Max.geo.setup";
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::PrepareTempDirectory() const
{
  gSystem->Exec(("rm -rf " + GetTempDirectory()).Data());
  return gSystem->mkdir(GetTempDirectory(), true) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::WriteTextFile(const MString& FileName, const MString& Content) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) return false;
  Out << Content.Data();
  return Out.good();
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::LoadGeometry(MGeometryRevan& Geometry) const
{
  return Geometry.ScanSetupFile(GetGeometryFileName());
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::TestOpenAndGuards()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "evta temp dir", "The temporary directory for MFileEventsEvta tests can be created", PrepareTempDirectory()) && Passed;
  Passed = EvaluateTrue("AccessPathName()", "evta geometry fixture", "The Max geometry fixture exists", gSystem->AccessPathName(GetGeometryFileName()) == false) && Passed;

  MString SimFixture = GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz";
  Passed = EvaluateTrue("AccessPathName()", "evta sim fixture", "The shared 1-second sim fixture exists", gSystem->AccessPathName(SimFixture) == false) && Passed;

  {
    MGeometryRevan Geometry;
    MFileEventsEvta Reader(&Geometry);
    mout.Enable(false);
    bool Opened = Reader.Open(SimFixture);
    mout.Enable(true);
    Passed = EvaluateFalse("Open()", "evta unscanned geometry", "Opening requires a scanned revan geometry", Opened) && Passed;
  }

  {
    MGeometryRevan Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "evta scan geometry", "The revan geometry can be scanned from the fixture", LoadGeometry(Geometry)) && Passed;
    MFileEventsEvta Reader(&Geometry);
    mgui.Enable(false);
    bool Opened = Reader.Open(GetTempDirectory() + "/invalid.txt");
    mgui.Enable(true);
    Passed = EvaluateFalse("Open()", "evta invalid extension", "Files without sim/evta extension are rejected", Opened) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::TestReadSimFixtures()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "evta read scan geometry", "The revan geometry can be scanned for read tests", LoadGeometry(Geometry)) && Passed;

  MString SimFixture = GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz";
  MFileEventsEvta Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "evta read open", "The 1-second sim fixture opens in the evta reader", Reader.Open(SimFixture)) && Passed;

  Passed = EvaluateNear("GetObservationTime()", "evta direct observation time", "Observation time can be read directly from the sim footer", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-9) && Passed;

  MRERawEvent* Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta first event", "The first raw event can be reconstructed from the sim fixture", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetEventID()", "evta first event id", "The first reconstructed raw event has the expected id", Event->GetEventID(), 1UL) && Passed;
    Passed = EvaluateNear("GetEventTime()", "evta first event time", "The first reconstructed raw event has the expected time", Event->GetEventTime().GetAsDouble(), 0.011003748, 1e-9) && Passed;
    Passed = Evaluate("GetNRESEs()", "evta first event rese count", "The first reconstructed raw event has the expected number of RESEs", Event->GetNRESEs(), 4) && Passed;
    delete Event;
  }

  long EventCount = 0;
  while ((Event = Reader.GetNextEvent()) != nullptr) {
    ++EventCount;
    delete Event;
  }

  Passed = EvaluateTrue("GetNextEvent()", "evta full scan count", "Scanning the full sim fixture yields more reconstructed events", EventCount > 0) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "evta full scan observation time", "Observation time remains correct after scanning all raw events", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-9) && Passed;

  Passed = EvaluateTrue("Rewind()", "evta rewind", "The evta reader can rewind to the start of the file", Reader.Rewind()) && Passed;
  Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta rewind first event", "After rewinding, the first event can be read again", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetEventID()", "evta rewind first event id", "After rewinding, the first event id matches the original first event", Event->GetEventID(), 1UL) && Passed;
    Passed = EvaluateNear("GetEventTime()", "evta rewind first event time", "After rewinding, the first event time matches the original first event", Event->GetEventTime().GetAsDouble(), 0.011003748, 1e-9) && Passed;
    delete Event;
  }

  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::TestIncludeObservationTime()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "evta include temp dir", "The temporary directory can be recreated for evta include tests", PrepareTempDirectory()) && Passed;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "evta include scan geometry", "The revan geometry can be scanned for include tests", LoadGeometry(Geometry)) && Passed;

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

  Passed = EvaluateTrue("WriteTextFile()", "evta include fixture", "The temporary include sim file can be written", WriteTextFile(IncludeFile, Content.str().c_str())) && Passed;

  MFileEventsEvta Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "evta include open", "The include sim file opens in the evta reader", Reader.Open(IncludeFile)) && Passed;

  long EventCount = 0;
  MRERawEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent()) != nullptr) {
    ++EventCount;
    delete Event;
  }

  Passed = EvaluateTrue("GetNextEvent()", "evta include event count", "The include sim file yields reconstructed events from its children", EventCount > 0) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "evta include observation time", "Observation time sums across included sim files", Reader.GetObservationTime().GetAsDouble(), 7.0, 1e-9) && Passed;

  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::TestFooterOnlyObservationTime()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "evta footer temp dir", "The temporary directory can be recreated for evta footer tests", PrepareTempDirectory()) && Passed;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "evta footer scan geometry", "The revan geometry can be scanned for footer tests", LoadGeometry(Geometry)) && Passed;

  MString FileName = GetTempDirectory() + "/footer_only.evta";

  ostringstream Content;
  Content<<"Type       evta"<<endl;
  Content<<"Version    1"<<endl;
  Content<<"Geometry   "<<GetGeometryFileName()<<endl;
  Content<<endl;
  Content<<"Date       2026-04-20 00:00:00"<<endl;
  Content<<"MEGAlib    "<<g_VersionString<<endl;
  Content<<endl;
  Content<<"TB 1.250000000"<<endl;
  Content<<"EN"<<endl;
  Content<<endl;
  Content<<"TE 4.750000000"<<endl;
  Content<<endl;

  Passed = EvaluateTrue("WriteTextFile()", "evta footer fixture", "The footer-only evta fixture can be written", WriteTextFile(FileName, Content.str().c_str())) && Passed;

  MFileEventsEvta Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "evta footer open", "The footer-only evta fixture opens", Reader.Open(FileName)) && Passed;

  MRERawEvent* Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta footer no event", "A footer-only evta file contains no raw events", Event == nullptr) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "evta footer observation time", "Observation time falls back to TE-TB for footer-only evta files", Reader.GetObservationTime().GetAsDouble(), 3.5, 1e-9) && Passed;

  Passed = EvaluateTrue("Rewind()", "evta footer rewind", "The footer-only evta file can be rewound", Reader.Rewind()) && Passed;
  Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta footer rewind no event", "Rewinding does not create phantom events in a footer-only evta file", Event == nullptr) && Passed;

  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::TestRewindBehavior()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "evta rewind temp dir", "The temporary directory can be recreated for evta rewind tests", PrepareTempDirectory()) && Passed;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "evta rewind scan geometry", "The revan geometry can be scanned for rewind tests", LoadGeometry(Geometry)) && Passed;

  MString SimFixture = GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz";
  MFileEventsEvta Reader(&Geometry);
  Passed = EvaluateTrue("Open()", "evta rewind open", "The evta reader opens the sim fixture for rewind tests", Reader.Open(SimFixture)) && Passed;

  MRERawEvent* Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta rewind first read", "The first event can be read before rewinding", Event != nullptr) && Passed;
  if (Event != nullptr) {
    delete Event;
  }

  Passed = EvaluateTrue("Rewind()", "evta rewind", "The evta reader can rewind during a normal scan", Reader.Rewind()) && Passed;
  Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta rewind second read", "After rewinding, the first event can be read again", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetEventID()", "evta rewind id", "Rewinding preserves the first event id", Event->GetEventID(), 1UL) && Passed;
    delete Event;
  }

  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::TestSaveOI()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "evta saveoi temp dir", "The temporary directory can be recreated for evta saveoi tests", PrepareTempDirectory()) && Passed;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "evta saveoi scan geometry", "The revan geometry can be scanned for saveoi tests", LoadGeometry(Geometry)) && Passed;

  MString SimFixture = GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz";
  MFileEventsEvta Reader(&Geometry);
  Reader.SaveOI(true);
  Passed = EvaluateTrue("Open()", "evta saveoi open", "The evta reader opens the sim fixture with SaveOI enabled", Reader.Open(SimFixture)) && Passed;

  MRERawEvent* Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta saveoi event", "The first raw event can be reconstructed with origin information saved", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = EvaluateTrue("GetNREAMs()", "evta saveoi ream count", "SaveOI adds at least one additional measurement", Event->GetNREAMs() > 0) && Passed;
    MREAM* REAM = Event->GetREAMAt(Event->GetNREAMs() - 1);
    Passed = EvaluateTrue("GetREAMAt()", "evta saveoi ream type", "The saved origin information is a start-information REAM", REAM != nullptr && REAM->GetType() == MREAM::c_StartInformation) && Passed;
    MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(REAM);
    Passed = EvaluateTrue("dynamic_cast", "evta saveoi cast", "The saved origin information can be cast to MREAMStartInformation", Start != nullptr) && Passed;
    if (Start != nullptr) {
      Passed = EvaluateTrue("GetEnergy()", "evta saveoi energy", "The saved origin information carries a non-zero source energy", Start->GetEnergy() > 0.0) && Passed;
    }
    delete Event;
  }

  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileEventsEvta Test;
  return Test.Run() ? 0 : 1;
}
