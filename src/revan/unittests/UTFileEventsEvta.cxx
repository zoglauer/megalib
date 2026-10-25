/*
 * UTFileEventsEvta.cxx
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
  //! Test parsing of OI lines with and without particle ID
  bool TestOILineParsing();
  //! Test the AT keyword (accumulated include times)
  bool TestAccumulatedIncludeTimes();

  //! Write an include master for the three observation-time fixtures with an extra header line
  bool WriteIncludeMaster(const MString& FileName, const MString& ExtraHeaderLine);
  //! Read all event times of a sim file through the evta reader
  vector<double> ReadEventTimes(MGeometryRevan& Geometry, const MString& FileName);

  //! Return the temp directory
  MString GetTempDirectory() const;
  //! Return the shared data directory
  MString GetDataDirectory() const;
  //! Return the test geometry file
  MString GetGeometryFileName() const;
  //! Prepare the temp directory
  bool PrepareTempDirectory() const;
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
  Passed = TestOILineParsing() && Passed;
  Passed = TestAccumulatedIncludeTimes() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTFileEventsEvta::GetTempDirectory() const
{
  return GetTemporaryDirectoryName("files");
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
  return PrepareTemporaryDirectory("files");
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

  Passed = EvaluateNear("GetObservationTime()", "evta direct observation time", "Observation time can be read directly from the sim footer", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;

  MRERawEvent* Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta first event", "The first raw event can be reconstructed from the sim fixture", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetEventID()", "evta first event id", "The first reconstructed raw event has the expected id", Event->GetEventID(), 1UL) && Passed;
    Passed = EvaluateNear("GetEventTime()", "evta first event time", "The first reconstructed raw event has the expected time", Event->GetEventTime().GetAsDouble(), 0.011003748, 1e-12) && Passed;
    Passed = Evaluate("GetNRESEs()", "evta first event rese count", "The first reconstructed raw event has the expected number of RESEs", Event->GetNRESEs(), 4) && Passed;
    delete Event;
  }

  long EventCount = 0;
  while ((Event = Reader.GetNextEvent()) != nullptr) {
    ++EventCount;
    delete Event;
  }

  // Expected: 21 SE blocks (zcat | grep -c '^SE') minus the first one read above
  Passed = Evaluate("GetNextEvent()", "evta full scan count", "Scanning the rest of the sim fixture yields the 20 remaining reconstructed events", EventCount, 20L) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "evta full scan observation time", "Observation time remains correct after scanning all raw events", Reader.GetObservationTime().GetAsDouble(), 1.0, 1e-12) && Passed;

  Passed = EvaluateTrue("Rewind()", "evta rewind", "The evta reader can rewind to the start of the file", Reader.Rewind()) && Passed;
  Event = Reader.GetNextEvent();
  Passed = EvaluateTrue("GetNextEvent()", "evta rewind first event", "After rewinding, the first event can be read again", Event != nullptr) && Passed;
  if (Event != nullptr) {
    Passed = Evaluate("GetEventID()", "evta rewind first event id", "After rewinding, the first event id matches the original first event", Event->GetEventID(), 1UL) && Passed;
    Passed = EvaluateNear("GetEventTime()", "evta rewind first event time", "After rewinding, the first event time matches the original first event", Event->GetEventTime().GetAsDouble(), 0.011003748, 1e-12) && Passed;
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

  // Expected: 21 + 31 + 73 SE blocks in the children (zcat | grep -c '^SE')
  Passed = Evaluate("GetNextEvent()", "evta include event count", "The include sim file yields the 21 + 31 + 73 = 125 reconstructed events of its children", EventCount, 125L) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "evta include observation time", "Observation time sums across included sim files", Reader.GetObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;

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
  Passed = EvaluateNear("GetObservationTime()", "evta footer observation time", "Observation time falls back to TE-TB for footer-only evta files", Reader.GetObservationTime().GetAsDouble(), 3.5, 1e-12) && Passed;

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
    Passed = Evaluate("GetNREAMs()", "evta saveoi ream count", "SaveOI adds exactly one additional measurement", Event->GetNREAMs(), 1U) && Passed;
    MREAM* REAM = Event->GetREAMAt(Event->GetNREAMs() - 1);
    Passed = EvaluateTrue("GetREAMAt()", "evta saveoi ream type", "The saved origin information is a start-information REAM", REAM != nullptr && REAM->GetType() == MREAM::c_StartInformation) && Passed;
    MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(REAM);
    Passed = EvaluateTrue("dynamic_cast", "evta saveoi cast", "The saved origin information can be cast to MREAMStartInformation", Start != nullptr) && Passed;
    if (Start != nullptr) {
      Passed = EvaluateNear("GetEnergy()", "evta saveoi energy", "The saved origin information carries the source energy of the IA INIT line (511 keV)", Start->GetEnergy(), 511.0, 1e-12) && Passed;
      Passed = Evaluate("GetParticleID()", "evta saveoi particle id", "The saved origin information carries the particle ID of the IA INIT line (photon)", Start->GetParticleID(), 1) && Passed;
    }
    delete Event;
  }

  Reader.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::TestOILineParsing()
{
  bool Passed = true;

  // Old OI line without particle ID
  MRERawEvent Old;
  Passed = Evaluate("ParseLine()", "evta OI 10 values", "OI lines without particle ID (older files) are accepted", Old.ParseLine("OI 1;2;3;0;0;-1;1;0;0;511", 1), 0) && Passed;
  Passed = Evaluate("GetNREAMs()", "evta OI 10 values", "An OI line without particle ID adds one measurement", Old.GetNREAMs(), 1U) && Passed;
  if (Old.GetNREAMs() == 1) {
    MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(Old.GetREAMAt(0));
    Passed = EvaluateTrue("dynamic_cast", "evta OI 10 values", "The OI line is stored as start information", Start != nullptr) && Passed;
    if (Start != nullptr) {
      Passed = Evaluate("GetEnergy()", "evta OI 10 values", "The energy is read from OI lines without particle ID", Start->GetEnergy(), 511.0) && Passed;
      Passed = Evaluate("GetParticleID()", "evta OI 10 values", "OI lines without particle ID leave the ID undefined", Start->GetParticleID(), g_IntNotDefined) && Passed;
    }
  }

  // New OI line with particle ID
  MRERawEvent New;
  Passed = Evaluate("ParseLine()", "evta OI 11 values", "OI lines with particle ID are accepted", New.ParseLine("OI 1;2;3;0;0;-1;1;0;0;511;26056", 1), 0) && Passed;
  Passed = Evaluate("GetNREAMs()", "evta OI 11 values", "An OI line with particle ID adds one measurement", New.GetNREAMs(), 1U) && Passed;
  if (New.GetNREAMs() == 1) {
    MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(New.GetREAMAt(0));
    Passed = EvaluateTrue("dynamic_cast", "evta OI 11 values", "The OI line is stored as start information", Start != nullptr) && Passed;
    if (Start != nullptr) {
      Passed = Evaluate("GetEnergy()", "evta OI 11 values", "The energy is read from OI lines with particle ID", Start->GetEnergy(), 511.0) && Passed;
      Passed = Evaluate("GetParticleID()", "evta OI 11 values", "The particle ID is read from OI lines with particle ID", Start->GetParticleID(), 26056) && Passed;

      MREAMStartInformation* Clone = dynamic_cast<MREAMStartInformation*>(Start->Clone());
      Passed = EvaluateTrue("Clone()", "evta OI clone", "Start information can be cloned", Clone != nullptr) && Passed;
      if (Clone != nullptr) {
        Passed = Evaluate("Clone()->GetParticleID()", "evta OI clone", "Cloning preserves the particle ID", Clone->GetParticleID(), 26056) && Passed;
        delete Clone;
      }
    }
  }

  // Broken OI line
  MRERawEvent Broken;
  Passed = Evaluate("ParseLine()", "evta OI 9 values", "OI lines with too few values are rejected", Broken.ParseLine("OI 1;2;3;0;0;-1;1;0;0", 1), 1) && Passed;
  Passed = Evaluate("GetNREAMs()", "evta OI 9 values", "A rejected OI line adds no measurement", Broken.GetNREAMs(), 0U) && Passed;

  // SetOriginInformation with and without particle ID
  MRERawEvent Origin;
  Origin.SetOriginInformation(MVector(1, 2, 3), MVector(0, 0, -1), MVector(1, 0, 0), 511.0);
  Origin.SetOriginInformation(MVector(1, 2, 3), MVector(0, 0, -1), MVector(1, 0, 0), 511.0, 3);
  Passed = Evaluate("GetNREAMs()", "evta SetOriginInformation", "Each SetOriginInformation call adds one measurement", Origin.GetNREAMs(), 2U) && Passed;
  if (Origin.GetNREAMs() == 2) {
    MREAMStartInformation* Without = dynamic_cast<MREAMStartInformation*>(Origin.GetREAMAt(0));
    MREAMStartInformation* With = dynamic_cast<MREAMStartInformation*>(Origin.GetREAMAt(1));
    Passed = EvaluateTrue("GetParticleID()", "evta SetOriginInformation default", "SetOriginInformation without particle ID leaves the ID undefined", Without != nullptr && Without->GetParticleID() == g_IntNotDefined) && Passed;
    Passed = EvaluateTrue("GetParticleID()", "evta SetOriginInformation id", "SetOriginInformation stores the particle ID", With != nullptr && With->GetParticleID() == 3) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::WriteIncludeMaster(const MString& FileName, const MString& ExtraHeaderLine)
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


vector<double> UTFileEventsEvta::ReadEventTimes(MGeometryRevan& Geometry, const MString& FileName)
{
  vector<double> Times;

  MFileEventsEvta Reader(&Geometry);
  if (Reader.Open(FileName) == false) return Times;

  MRERawEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent()) != nullptr) {
    Times.push_back(Event->GetEventTime().GetAsDouble());
    delete Event;
  }
  Reader.Close();

  return Times;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFileEventsEvta::TestAccumulatedIncludeTimes()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "evta AT temp dir", "The temporary directory can be recreated for AT tests", PrepareTempDirectory()) && Passed;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "evta AT scan geometry", "The revan geometry can be scanned for AT tests", LoadGeometry(Geometry)) && Passed;

  // Event times of the children read on their own:
  vector<double> Times1 = ReadEventTimes(Geometry, GetDataDirectory() + "/ObsTime_1sec_complete.inc1.id1.sim.gz");
  vector<double> Times2 = ReadEventTimes(Geometry, GetDataDirectory() + "/ObsTime_2sec_complete.inc1.id1.sim.gz");
  vector<double> Times4 = ReadEventTimes(Geometry, GetDataDirectory() + "/ObsTime_4sec_complete.inc1.id1.sim.gz");
  Passed = EvaluateSize("GetNextEvent()", "evta AT child event count", "The three children hold 21 + 31 + 73 = 125 events", Times1.size() + Times2.size() + Times4.size(), (size_t) 125) && Passed;

  // Expected times without and with the offsets 0, 1, and 1 + 2 s of the preceding children:
  vector<double> Unshifted;
  vector<double> Shifted;
  for (double Time: Times1) { Unshifted.push_back(Time); Shifted.push_back(Time); }
  for (double Time: Times2) { Unshifted.push_back(Time); Shifted.push_back(Time + 1.0); }
  for (double Time: Times4) { Unshifted.push_back(Time); Shifted.push_back(Time + 3.0); }

  // AT true:
  MString MasterTrue = GetTempDirectory() + "/ObsTime_AT_true.sim";
  Passed = EvaluateTrue("WriteTextFile()", "evta AT true fixture", "The include master with AT true can be written", WriteIncludeMaster(MasterTrue, "AT         true")) && Passed;

  {
    MFileEventsEvta Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "evta AT true open", "The include master with AT true opens in the evta reader", Reader.Open(MasterTrue)) && Passed;
    Passed = EvaluateTrue("HasAccumulatedIncludeTimes()", "evta AT true flag", "The AT true header line sets the accumulated include times flag", Reader.HasAccumulatedIncludeTimes()) && Passed;
    Passed = EvaluateNear("GetIncludeTimeOffset()", "evta AT true initial offset", "The include time offset starts at zero", Reader.GetIncludeTimeOffset().GetAsDouble(), 0.0, 1e-12) && Passed;

    vector<double> Times;
    MRERawEvent* Event = nullptr;
    while ((Event = Reader.GetNextEvent()) != nullptr) {
      Times.push_back(Event->GetEventTime().GetAsDouble());
      delete Event;
    }
    Passed = EvaluateSize("GetNextEvent()", "evta AT true event count", "All 125 events of the children are read with AT true", Times.size(), Shifted.size()) && Passed;
    if (Times.size() == Shifted.size()) {
      for (size_t i = 0; i < Times.size(); ++i) {
        ostringstream Input;
        Input<<"evta AT true event "<<i;
        Passed = EvaluateNear("GetEventTime()", Input.str().c_str(), "The event time is offset by the observation time of the preceding children", Times[i], Shifted[i], 1e-9) && Passed;
      }
    }
    bool Monotonic = true;
    for (size_t i = 1; i < Times.size(); ++i) {
      if (Times[i] < Times[i-1]) Monotonic = false;
    }
    Passed = EvaluateTrue("GetEventTime()", "evta AT true monotonic", "With AT true the event times never decrease across the included files", Monotonic) && Passed;
    Passed = EvaluateNear("GetIncludeTimeOffset()", "evta AT true final offset", "After all children are closed the offset is the sum of their observation times", Reader.GetIncludeTimeOffset().GetAsDouble(), 7.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetObservationTime()", "evta AT true observation time", "The observation time still sums across the included files", Reader.GetObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;

    Passed = EvaluateTrue("Rewind()", "evta AT true rewind", "The include master with AT true can be rewound", Reader.Rewind()) && Passed;
    Passed = EvaluateNear("GetIncludeTimeOffset()", "evta AT true rewind offset", "Rewinding resets the include time offset", Reader.GetIncludeTimeOffset().GetAsDouble(), 0.0, 1e-12) && Passed;
    Event = Reader.GetNextEvent();
    Passed = EvaluateTrue("GetNextEvent()", "evta AT true rewind first event", "After rewinding the first event can be read again", Event != nullptr) && Passed;
    if (Event != nullptr && Shifted.empty() == false) {
      Passed = EvaluateNear("GetEventTime()", "evta AT true rewind first time", "After rewinding the first event is unshifted again", Event->GetEventTime().GetAsDouble(), Shifted[0], 1e-9) && Passed;
      delete Event;
    }
    Reader.Close();
  }

  // AT false:
  MString MasterFalse = GetTempDirectory() + "/ObsTime_AT_false.sim";
  Passed = EvaluateTrue("WriteTextFile()", "evta AT false fixture", "The include master with AT false can be written", WriteIncludeMaster(MasterFalse, "AT         false")) && Passed;

  {
    MFileEventsEvta Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "evta AT false open", "The include master with AT false opens in the evta reader", Reader.Open(MasterFalse)) && Passed;
    Passed = EvaluateFalse("HasAccumulatedIncludeTimes()", "evta AT false flag", "The AT false header line leaves the accumulated include times flag unset", Reader.HasAccumulatedIncludeTimes()) && Passed;

    vector<double> Times;
    MRERawEvent* Event = nullptr;
    while ((Event = Reader.GetNextEvent()) != nullptr) {
      Times.push_back(Event->GetEventTime().GetAsDouble());
      delete Event;
    }
    Passed = EvaluateSize("GetNextEvent()", "evta AT false event count", "All 125 events of the children are read with AT false", Times.size(), Unshifted.size()) && Passed;
    if (Times.size() == Unshifted.size()) {
      for (size_t i = 0; i < Times.size(); ++i) {
        ostringstream Input;
        Input<<"evta AT false event "<<i;
        Passed = EvaluateNear("GetEventTime()", Input.str().c_str(), "The event time is not offset with AT false", Times[i], Unshifted[i], 1e-9) && Passed;
      }
    }
    Passed = EvaluateNear("GetIncludeTimeOffset()", "evta AT false final offset", "The include time offset stays zero with AT false", Reader.GetIncludeTimeOffset().GetAsDouble(), 0.0, 1e-12) && Passed;
    Reader.Close();
  }

  // No AT line:
  MString MasterNone = GetTempDirectory() + "/ObsTime_AT_none.sim";
  Passed = EvaluateTrue("WriteTextFile()", "evta AT none fixture", "The include master without AT can be written", WriteIncludeMaster(MasterNone, "")) && Passed;

  {
    MFileEventsEvta Reader(&Geometry);
    Passed = EvaluateTrue("Open()", "evta AT none open", "The include master without AT opens in the evta reader", Reader.Open(MasterNone)) && Passed;
    Passed = EvaluateFalse("HasAccumulatedIncludeTimes()", "evta AT none flag", "Without an AT header line the accumulated include times flag is unset", Reader.HasAccumulatedIncludeTimes()) && Passed;
    Reader.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileEventsEvta Test;
  return Test.Run() ? 0 : 1;
}
