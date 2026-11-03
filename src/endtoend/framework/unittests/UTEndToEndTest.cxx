/*
 * UTEndToEndTest.cxx
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


// MEGAlib libs:
#include "MEndToEndTest.h"
#include "MStatistics.h"
#include "MEndToEndFingerprint.h"
#include "MFile.h"
#include "MSettingsTesting.h"
#include "MSystem.h"

// Standard libs:
#include <filesystem>
#include <chrono>
#include <cmath>
#include <limits>
#include <thread>
#include <fstream>
using namespace std;

// ROOT libs:
#include "TFile.h"
#include "TH1D.h"


//! Unit test class for the helpers of MEndToEndTest (no external program is run)
class UTEndToEndTest : public MUnitTest
{
public:
  UTEndToEndTest() : MUnitTest("UTEndToEndTest") {}
  virtual ~UTEndToEndTest() {}

  virtual bool Run();

private:
  //! Expose the protected functionality of MEndToEndTest
  class Probe : public MEndToEndTest
  {
  public:
    Probe() : MEndToEndTest("Probe") {}
    virtual bool RunTest() { return true; }
    bool Execute10(const MString& Directory, const MString& Executable, const MString& Arguments) { return Execute(Directory, Executable, Arguments, 10); }
    vector<MString> Lines(const MString& FileName) { return ReadLines(FileName); }
    ETTraFileCoreData Tra(const MString& FileName) { return ReadTraFile(FileName); }
    ETSimFileCoreData Read(const ETSimScenario& Scenario) { return ReadSim(Scenario); }
    bool Prepare(ETSimScenario& Scenario) { return PrepareScenario(Scenario); }
    bool PrepareSim(ETSimScenario& Scenario) { return PrepareSimulation(Scenario); }
    bool Sim(const ETSimScenario& Scenario) { return Simulate(Scenario); }
    bool Verify() { return VerifyEnvironment(); }
    unique_ptr<TH1D> First1D(const MString& FileName) { return FirstHistogram<TH1D>(FileName); }
    bool Peak(const TH2* Image, double Range, double& CentroidX, double& CentroidY) { return ImagePeak(Image, Range, CentroidX, CentroidY); }
    unsigned int Budget() { return GetRemainingTimeBudget(); }
    MString Home() { return GetTemporaryDirectoryName("home"); }
    bool RunIn(const MString& Directory, const MString& Executable, const MString& Arguments, unsigned int Timeout) { return Execute(Directory, Executable, Arguments, Timeout); }
    bool PrepareAll(vector<ETSimScenario>& Scenarios) { return PrepareSimulations(Scenarios); }
    double Arm(const MComptonEvent& Event, const MVector& ToSource) { return ARM(Event, ToSource); }
    ETMeasuredValue Measured(const vector<double>& Values, const MStatisticFunction& Statistic, unsigned int NResamples) { return Measure(Values, Statistic, NResamples); }
    void References(const map<MString, pair<double, double>>& Values)
    {
      MEndToEndReferenceList Made;
      for (const pair<const MString, pair<double, double>>& Value: Values) {
        MEndToEndReference Reference(Value.first);
        Reference.SetMean(Value.second.first);
        Reference.SetError(Value.second.second);
        Made.SetReference(Reference);
      }
      SetReferences(Made);
    }
    bool Load(const MString& FileName) { return LoadReferences(FileName); }
    map<MString, MString> Fingerprint() { return GetFingerprint(); }
    vector<MString> Differences() { return CheckReferences(); }
    MString ScenarioHash(const ETSimScenario& Scenario) { return GetScenarioHash(Scenario); }
    bool Prepare(const ETSimScenario& Scenario) { ETSimScenario Copy = Scenario; return PrepareScenario(Copy); }
    const vector<MString>& Titles() { return GetReport().GetTitleLines(); }
    void NoReportDirectory() { GetReport().SetDirectory(""); }
    bool Compare(const ETMeasuredValue& Value) { return CompareToReference("S", "O", "Test", Value); }
    unsigned int Seed() { return RandomSeed(); }
    bool CompareKey(const MString& Scenario, const MString& Observable, const ETMeasuredValue& Value) { return CompareToReference(Scenario, Observable, "Test", Value); }
    bool CompareLimit(const ETMeasuredValue& Value, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical) { return CompareValues("Probe", "limit", "Test", "Limit", Value, Expected, Tolerance, IsStatistical); }
    static double MaximumY(const TGraph* Graph) { return GetMaximumY(Graph); }
    const MEndToEndResult& Last() { return GetResults().GetResults().back(); }
    size_t Recorded() { return GetResults().GetResults().size(); }
    void SummaryDirectory(const MString& Directory) { GetReport().SetDirectory(Directory); }
    ETMeasuredValue Reference(const MString& Scenario, const MString& Observable) { return GetReference(Scenario, Observable); }
    void Key(const MString& Name, double Value, double Error) { GetResults().AddResult(Name, Value, Error); }
    void Page() { TPad* Pad = GetReport().AddPage("Page", 1, 1); Pad->cd(1); GetReport().DrawHistogram("Test;x;y", vector<double>{ 1.0, 2.0, 2.0 }, 4, 0.0, 4.0); }
  };

  //! Write a (gzipped if the name ends with .gz) text file
  bool Write(const MString& FileName, const MString& Content);

  bool TestPathsWithSpaces();
  bool TestReaders();
  bool TestSeeds();
  bool TestGeometry();
  bool TestExecution();
  bool TestRootHelpers();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndTest::Write(const MString& FileName, const MString& Content)
{
  {
    ofstream Out(FileName.Data());
    if (Out.is_open() == false) {
      return false;
    }
    Out<<Content;
  }
  if (FileName.EndsWith(".gz") == false) {
    return true;
  }
  // gzip the plain file under its own name
  const MString Plain = FileName.GetSubString(0, FileName.Length() - 3);
  error_code Error;
  filesystem::rename(FileName.Data(), Plain.Data(), Error);
  Probe Tester;
  if (system((MString("gzip -f ") + MSystem::GetShellQuoted(Plain)).Data()) != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndTest::Run()
{
  bool Passed = true;
  Passed = TestPathsWithSpaces() && Passed;
  Passed = TestReaders() && Passed;
  Passed = TestSeeds() && Passed;
  Passed = TestGeometry() && Passed;
  Passed = TestExecution() && Passed;
  Passed = TestRootHelpers() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndTest::TestPathsWithSpaces()
{
  bool Passed = true;
  Probe Tester;

  const MString Base = GetTemporaryDirectoryName("with spaces and 'quotes'");
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "directory", "A directory with spaces and quotes can be created", PrepareTemporaryDirectory("with spaces and 'quotes'")) && Passed;

  // Execute: the directory is one word
  Passed = EvaluateTrue("Execute()", "spaces", "A command runs in a directory with spaces and quotes", Tester.Execute10(Base, "touch", "executed.txt")) && Passed;
  Passed = EvaluateTrue("Execute()", "result", "The command ran in that directory", filesystem::exists((Base + "/executed.txt").Data())) && Passed;

  // The programs have a private home directory, not the one of the user
  Passed = EvaluateTrue("Execute()", "home", "A program can write the home directory it sees", Tester.Execute10(Base, "sh", "-c 'echo \"$HOME\" > home.txt'")) && Passed;
  MString SeenHome;
  MFile::ReadTextFile(Base + "/home.txt", SeenHome);
  SeenHome.RemoveAllInPlace("\n");
  Passed = Evaluate("Execute()", "private home", "The programs see a private home directory below the temporary root of the test, not the one of the user", SeenHome, Tester.Home()) && Passed;
  const char* UserHome = getenv("HOME");
  if (UserHome != nullptr) {
    Passed = EvaluateTrue("Execute()", "not the user home", "The home directory of the programs is not the home directory of the user", SeenHome != MString(UserHome)) && Passed;
  }
  Passed = EvaluateTrue("Execute()", "home exists", "The private home directory exists", filesystem::is_directory(Tester.Home().Data())) && Passed;

  // ReadLines: a plain file in such a directory
  Passed = EvaluateTrue("WriteTextFile()", "plain", "A file can be written", Write(Base + "/plain file.txt", "one\ntwo\n")) && Passed;
  const vector<MString> Lines = Tester.Lines(Base + "/plain file.txt");
  Passed = EvaluateSize("ReadLines()", "plain, number", "A plain file with spaces in its path has two lines", Lines.size(), 2) && Passed;
  if (Lines.size() == 2) {
    Passed = Evaluate("ReadLines()", "plain, first", "The first line of the plain file is one", Lines[0], MString("one")) && Passed;
    Passed = Evaluate("ReadLines()", "plain, second", "The second line of the plain file is two", Lines[1], MString("two")) && Passed;
  }
  Passed = EvaluateSize("ReadLines()", "missing", "A missing file has no lines", Tester.Lines(Base + "/missing.txt").size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndTest::TestReaders()
{
  bool Passed = true;
  Probe Tester;

  // Use the real cosima and revan fixture: 21 events, 19 Compton and 2 photo events
  MString Directory = "$(MEGALIB)/src/global/misc/unittests/data/UTFileEventsTra";
  MFile::ExpandFileName(Directory);
  ETSimScenario Scenario;
  Scenario.m_Name = "ObsTime_1sec_complete";
  Scenario.m_Directory = Directory;

  const ETSimFileCoreData Sim = Tester.Read(Scenario);
  Passed = EvaluateTrue("ReadSim()", "valid", "The sim file of the fixture is read", Sim.m_FileSuccessfullyRead) && Passed;
  Passed = EvaluateNear("ReadSim()", "time", "The simulated time is 1 second", Sim.m_SimulationTime, 1.0, 1e-9) && Passed;
  Passed = Evaluate("ReadSim()", "generated", "The number of generated particles is the TS of the file", Sim.m_SimulatedParticles, 310UL) && Passed;
  Passed = EvaluateNear("ReadSim()", "start area", "The start area of the far field source is the one of the file header", Sim.m_StartArea, 314.159, 1e-6) && Passed;
  Passed = EvaluateSize("ReadSim()", "events", "The sim file has 21 triggered events", Sim.m_Events.size(), 21) && Passed;
  if (Sim.m_Events.size() > 0) {
    const shared_ptr<MSimEvent>& First = Sim.m_Events[0];
    Passed = EvaluateNear("ReadSim()", "first energy", "The first event starts a 511 keV particle", First->GetIAAt(0)->GetSecondaryEnergy(), 511.0, 1e-6) && Passed;
    Passed = EvaluateVectorNear("ReadSim()", "first direction", "The first event flies along -z", First->GetIAAt(0)->GetSecondaryDirection(), MVector(0.0, 0.0, -1.0), 1e-6) && Passed;
    Passed = Evaluate("ReadSim()", "first ID", "The first event has ID 1", First->GetID(), 1L) && Passed;
  }

  const ETTraFileCoreData Tra = Tester.Tra(Directory + "/ObsTime_1sec_complete.inc1.id1.tra.gz");
  Passed = EvaluateTrue("ReadTraFile()", "valid", "The tra file of the fixture is read", Tra.m_FileSuccessfullyRead) && Passed;
  const vector<shared_ptr<MComptonEvent>> Comptons = Tra.Compton();
  const vector<shared_ptr<MPhotoEvent>> Photos = Tra.Photo();
  Passed = EvaluateSize("ReadTraFile()", "Compton", "The tra file has 19 Compton events", Comptons.size(), 19) && Passed;
  Passed = EvaluateSize("ReadTraFile()", "photo", "The tra file has 2 photo events", Photos.size(), 2) && Passed;
  Passed = Evaluate("ReadTraFile()", "unknown", "The tra file has no unidentifiable events", Tra.Count(MPhysicalEvent::c_Unidentifiable), 0UL) && Passed;
  Passed = EvaluateSize("ReadTraFile()", "all", "The tra file has 21 events", Tra.m_Events.size(), 21) && Passed;
  if (Comptons.size() > 0) {
    const shared_ptr<MComptonEvent> FirstEvent = Comptons[0];
    Passed = Evaluate("ReadTraFile()", "first ID", "The first Compton event has ID 1", FirstEvent->GetId(), 1L) && Passed;
    Passed = EvaluateNear("ReadTraFile()", "first time", "The first Compton event has the time of the file", FirstEvent->GetTime().GetAsSeconds(), 0.011003748, 1e-9) && Passed;
    Passed = EvaluateNear("ReadTraFile()", "Eg", "The energy of the scattered photon is the one of the file", FirstEvent->Eg(), 386.326, 1e-3) && Passed;
    Passed = EvaluateNear("ReadTraFile()", "Ee", "The energy of the recoil electron is the one of the file", FirstEvent->Ee(), 123.94, 1e-3) && Passed;
    Passed = EvaluateVectorNear("ReadTraFile()", "first position", "The first interaction position is the one of the file", FirstEvent->C1(), MVector(-1.3875, 1.0175, 4.72085), 1e-4) && Passed;
    Passed = EvaluateVectorNear("ReadTraFile()", "second position", "The second interaction position is the one of the file", FirstEvent->C2(), MVector(-1.3875, 0.2775, 4.0748), 1e-4) && Passed;
  }
  if (Photos.size() > 0) {
    Passed = Evaluate("ReadTraFile()", "photo ID", "The first photo event has ID 7", Photos[0]->GetId(), 7L) && Passed;
    Passed = EvaluateNear("ReadTraFile()", "photo energy", "The first photo event has the energy of the file", Photos[0]->GetEnergy(), 510.953, 1e-3) && Passed;
  }

  // Files which can not be read are not valid
  Passed = EvaluateFalse("ReadTraFile()", "missing", "A missing tra file is not valid", Tester.Tra(Directory + "/missing.tra").m_FileSuccessfullyRead) && Passed;
  ETSimScenario Missing;
  Missing.m_Name = "Missing";
  Missing.m_Directory = Directory;
  Passed = EvaluateFalse("ReadSim()", "missing", "A missing sim file is not valid", Tester.Read(Missing).m_FileSuccessfullyRead) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndTest::TestSeeds()
{
  bool Passed = true;
  Probe Tester;

  ETSimScenario Given;
  Given.m_Name = "Given";
  Given.m_Seed = 4242;
  Passed = EvaluateTrue("PrepareSimulation()", "given", "The scenario can be prepared", Tester.PrepareSim(Given)) && Passed;
  Passed = Evaluate("PrepareSimulation()", "given seed", "An explicitly given seed is kept", Given.m_Seed, 4242u) && Passed;

  ETSimScenario Random;
  Random.m_Name = "Random";
  Passed = EvaluateTrue("PrepareSimulation()", "random", "The scenario can be prepared", Tester.PrepareSim(Random)) && Passed;
  // The seed is random - only its range (1 to 2e9) can be checked
  Passed = EvaluateTrue("PrepareSimulation()", "random seed", "Without a seed a random seed in the valid range is chosen", Random.m_Seed >= 1 && Random.m_Seed <= 2000000000u) && Passed;

  // A scenario which does not simulate has no seed
  ETSimScenario Data;
  Data.m_Name = "Data";
  Passed = EvaluateTrue("PrepareScenario()", "data", "The scenario can be prepared", Tester.Prepare(Data)) && Passed;
  Passed = Evaluate("PrepareScenario()", "no seed", "A scenario which does not simulate gets no seed", Data.m_Seed, 0u) && Passed;
  DisableDefaultStreams();
  const bool Simulated = Tester.Sim(Data);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Simulate()", "no seed", "A simulation without a seed is refused", Simulated) && Passed;

  // Check the geometry: it fails for a missing file
  {
    Probe Existing;
    Passed = EvaluateTrue("VerifyEnvironment()", "default", "The geometry, the programs, and the program which limits the run time are there", Existing.Verify()) && Passed;
    Probe Missing;
    Missing.SetGeometry(GetTemporaryDirectoryName("missing.geo.setup"));
    DisableDefaultStreams();
    const bool Found = Missing.Verify();
    EnableDefaultStreams();
    Passed = EvaluateFalse("VerifyEnvironment()", "missing", "A geometry file which does not exist fails the check", Found) && Passed;
  }

  // All scenarios of a simulation are prepared and get a seed
  {
    vector<ETSimScenario> Scenarios(2);
    Scenarios[0].m_Name = "First";
    Scenarios[1].m_Name = "Second";
    Passed = EvaluateTrue("PrepareSimulations()", "all", "All scenarios can be prepared", Tester.PrepareAll(Scenarios)) && Passed;
    // The seeds are random - only their range (1 to 2e9) can be checked
    for (const ETSimScenario& Scenario : Scenarios) {
      Passed = EvaluateTrue("PrepareSimulations()", "seed of " + Scenario.m_Name, "Every scenario has a seed between 1 and 2e9", Scenario.m_Seed >= 1 && Scenario.m_Seed <= 2000000000u) && Passed;
    }
  }

  // Check the far field point source
  {
    ETSimScenario Scenario;
    Scenario.SetFarFieldPointSource(20, 45, "Mono 662", 2.5);
    Passed = EvaluateSize("SetFarFieldPointSource()", "lines", "The source has four lines", Scenario.m_SourceLines.size(), 4) && Passed;
    if (Scenario.m_SourceLines.size() == 4) {
      Passed = Evaluate("SetFarFieldPointSource()", "particle", "The particles are photons", Scenario.m_SourceLines[0], MString("ParticleType 1")) && Passed;
      Passed = Evaluate("SetFarFieldPointSource()", "beam", "The beam is a far field point source in the given direction", Scenario.m_SourceLines[1], MString("Beam FarFieldPointSource 20 45")) && Passed;
      Passed = Evaluate("SetFarFieldPointSource()", "spectrum", "The spectrum is the given one", Scenario.m_SourceLines[2], MString("Spectrum Mono 662")) && Passed;
      Passed = Evaluate("SetFarFieldPointSource()", "flux", "The flux is the given one", Scenario.m_SourceLines[3], MString("Flux 2.5")) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndTest::TestGeometry()
{
  bool Passed = true;
  Probe Tester;

  const double Degree = c_Rad;

  // Check the ARM of a consistent event (662 keV, scattered by 60 degrees, from +z): zero
  // cos = 1 - c_E0*(1/Eg - 1/662) with Eg = Etot - Ee
  const double Total = 662.0;
  const double CosScatter = 0.5;
  const double Scattered = Total/(1.0 + Total/c_E0*(1.0 - CosScatter));
  MComptonEvent Event;
  Event.SetEg(Scattered);
  Event.SetEe(Total - Scattered);
  Event.SetC1(MVector(0.0, 0.0, 0.0));
  Event.SetC2(MVector(sin(60*Degree), 0.0, -cos(60*Degree)));
  Passed = EvaluateNear("ARM()", "exact", "A Compton event which is consistent with the source direction has an ARM of zero", Tester.Arm(Event, MVector(0.0, 0.0, 1.0)), 0.0, 1e-9) && Passed;
  MVector Toward5;
  Toward5.SetMagThetaPhi(1.0, 5*Degree, 0.0);
  Passed = EvaluateNear("ARM()", "off by 5 degrees", "A source direction which is 5 degrees off gives an ARM of 5 degrees in size", fabs(Tester.Arm(Event, Toward5)), 5.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndTest::TestExecution()
{
  bool Passed = true;
  Probe Tester;

  // Check the time limit - the time out of the driver, adapted to the speed of this machine, which is used up
  MSettingsTesting Settings;
  Settings.Read();
  const double Timeout = Settings.GetTimeout();
  if (Timeout > 0.0) {
    const unsigned int Expected = max(1U, static_cast<unsigned int>(ceil(Timeout)));
    const unsigned int First = Tester.Budget();
    // Expected: the time out rounded up, at most 0.5 s less (wall clock)
    const unsigned int Lowest = max(1U, static_cast<unsigned int>(ceil(Timeout - 0.5)));
    Passed = EvaluateTrue("GetRemainingTimeBudget()", "start", "At the start the time budget is the scaled time out of the test rounded up (at most 0.5 s used)", First <= Expected && First >= Lowest) && Passed;
    if (Timeout >= 4.0) {
      // Expected: time out less 1.5 s to 2.0 s, rounded up (sleep of 1.5 s, up to 0.5 s scheduling)
      this_thread::sleep_for(chrono::milliseconds(1500));
      const unsigned int Second = Tester.Budget();
      Passed = EvaluateTrue("GetRemainingTimeBudget()", "used up", "After 1.5 s (at most 2.0 s) the time budget is the time out less that time, rounded up", Second <= static_cast<unsigned int>(ceil(Timeout - 1.5)) && Second >= static_cast<unsigned int>(ceil(Timeout - 2.0))) && Passed;
    }
  } else {
    Passed = Evaluate("GetRemainingTimeBudget()", "no time out", "Without a time out of the test there is no time limit", Tester.Budget(), 0U) && Passed;
  }

  // Stop a program which runs too long, report one which fails
  PrepareTemporaryDirectory("execution");
  const MString Directory = GetTemporaryDirectoryName("execution");
  Passed = EvaluateTrue("Execute()", "success", "A command which succeeds is reported as success", Tester.RunIn(Directory, "true", "", 10)) && Passed;
  Passed = EvaluateFalse("Execute()", "failure", "A command which fails is reported as failure", Tester.RunIn(Directory, "false", "", 10)) && Passed;

  // Run a program with a display set in the environment of the test - the program does not see it:
  const char* OldDisplay = getenv("DISPLAY");
  MString OldDisplayCopy;
  if (OldDisplay != nullptr) {
    OldDisplayCopy = OldDisplay;
  }
  setenv("DISPLAY", ":99", 1);
  Passed = EvaluateTrue("Execute()", "no display", "A program does not see the display of the test", Tester.RunIn(Directory, "sh", "-c 'test -z \"$DISPLAY\"'", 10)) && Passed;
  if (OldDisplay == nullptr) {
    unsetenv("DISPLAY");
  } else {
    setenv("DISPLAY", OldDisplayCopy.Data(), 1);
  }
  const auto Start = chrono::steady_clock::now();
  const bool Stopped = Tester.RunIn(Directory, "sleep", "20", 1);
  const double Seconds = chrono::duration<double>(chrono::steady_clock::now() - Start).count();
  Passed = EvaluateFalse("Execute()", "time limit", "A command which runs longer than its time limit fails", Stopped) && Passed;
  // Expected: between the limit of 1 s and 5 s, far below the sleep of 20 s (wall clock)
  Passed = EvaluateTrue("Execute()", "time limit duration", "It is stopped at the time limit of 1 s, not before it and not after the full run time of 20 s", Seconds >= 1.0 && Seconds < 5.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the helpers which read the ROOT files of mimrec and analyze histograms
bool UTEndToEndTest::TestRootHelpers()
{
  bool Passed = true;
  Probe Tester;

  // Write a canvas with a histogram like mimrec
  PrepareTemporaryDirectory("root");
  const MString FileName = GetTemporaryDirectoryName("root") + "/histogram.root";
  {
    TH1D Histogram("UTHistogram", "UTHistogram", 10, 0.0, 10.0);
    Histogram.SetDirectory(nullptr);
    Histogram.SetBinContent(5, 7.0);
    TCanvas Canvas("UTCanvas", "UTCanvas");
    Histogram.Draw();
    TFile File(FileName.Data(), "RECREATE");
    Canvas.Write();
    File.Close();
  }
  unique_ptr<TH1D> Read = Tester.First1D(FileName);
  Passed = EvaluateTrue("FirstHistogram()", "read", "The histogram of the first canvas of a ROOT file is read", Read != nullptr) && Passed;
  if (Read != nullptr) {
    Passed = EvaluateNear("FirstHistogram()", "content", "The copy has the content of the histogram", Read->GetBinContent(5), 7.0, 1e-12) && Passed;
    Passed = Evaluate("FirstHistogram()", "bins", "The copy has the binning of the histogram", Read->GetNbinsX(), 10) && Passed;
  }
  Passed = EvaluateTrue("FirstHistogram()", "missing", "A missing file gives no histogram", Tester.First1D(FileName + ".missing") == nullptr) && Passed;

  // Use a peak of two bins
  TH2D Image("UTImage", "UTImage", 20, 0.0, 20.0, 20, 0.0, 20.0);
  Image.SetDirectory(nullptr);
  Image.SetBinContent(Image.GetXaxis()->FindBin(10.5), Image.GetYaxis()->FindBin(5.5), 3.0);
  Image.SetBinContent(Image.GetXaxis()->FindBin(11.5), Image.GetYaxis()->FindBin(5.5), 1.0);
  double CentroidX = 0.0;
  double CentroidY = 0.0;
  Passed = EvaluateTrue("ImagePeak()", "found", "The peak of an image is found", Tester.Peak(&Image, 2.0, CentroidX, CentroidY)) && Passed;
  Passed = EvaluateNear("ImagePeak()", "x", "The centroid is weighted by the bin contents: (3*10.5 + 1*11.5)/4", CentroidX, 10.75, 1e-9) && Passed;
  Passed = EvaluateNear("ImagePeak()", "y", "The centroid in y", CentroidY, 5.5, 1e-9) && Passed;
  TH2D EmptyImage("UTEmptyImage", "UTEmptyImage", 4, 0.0, 4.0, 4, 0.0, 4.0);
  EmptyImage.SetDirectory(nullptr);
  Passed = EvaluateFalse("ImagePeak()", "empty", "An empty image has no peak", Tester.Peak(&EmptyImage, 2.0, CentroidX, CentroidY)) && Passed;

  // Test the statistics of the detector effects tests:
  {
    Probe Tester;
    // Alternating 0 and 1 (400 values): the mean is 0.5, its bootstrap error is the one of the statistics
    vector<double> Alternating;
    for (int i = 0; i < 200; ++i) {
      Alternating.push_back(0.0);
      Alternating.push_back(1.0);
    }
    const MStatisticFunction Mean = [](const vector<double>& V) { return MStatistics::GetMean(V); };
    const ETMeasuredValue Measured = Tester.Measured(Alternating, Mean, 50);
    Passed = EvaluateNear("Measure()", "value", "The value is the statistic of the data", Measured.m_Value, 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("Measure()", "error", "The error is the bootstrap error", Measured.m_Error, MStatistics::GetBootstrapError(Alternating, Mean, 50), 0.0) && Passed;

    // Reference 10 +- 0.6 and measurement +- 0.8: the combined error is 1.0, the tolerance five
    Tester.References({ { "S/O", { 10.0, 0.6 } } });
    ETMeasuredValue Value;
    Value.m_Error = 0.8;
    Value.m_Value = 14.9;
    Passed = EvaluateTrue("CompareToReference()", "just inside", "A measurement 4.9 combined errors from the reference passes", Tester.Compare(Value)) && Passed;
    Value.m_Value = 5.1;
    Passed = EvaluateTrue("CompareToReference()", "just inside, below", "A measurement 4.9 combined errors below the reference passes", Tester.Compare(Value)) && Passed;
    Value.m_Value = 15.1;
    Passed = EvaluateFalse("CompareToReference()", "just outside", "A measurement 5.1 combined errors from the reference fails", Tester.Compare(Value)) && Passed;
    Value.m_Value = 4.9;
    Passed = EvaluateFalse("CompareToReference()", "just outside, below", "A measurement 5.1 combined errors below the reference fails", Tester.Compare(Value)) && Passed;
    Value.m_Value = 100.0;
    Value.m_Error = numeric_limits<double>::quiet_NaN();
    Passed = EvaluateFalse("CompareToReference()", "NaN error", "A NaN uncertainty does not accept a wrong measurement", Tester.Compare(Value)) && Passed;
    Value.m_Error = numeric_limits<double>::infinity();
    Passed = EvaluateFalse("CompareToReference()", "infinite error", "An infinite uncertainty does not accept a wrong measurement", Tester.Compare(Value)) && Passed;
    Value.m_Value = numeric_limits<double>::quiet_NaN();
    Value.m_Error = 0.8;
    Passed = EvaluateFalse("CompareToReference()", "NaN value", "A NaN measurement fails", Tester.Compare(Value)) && Passed;
    Tester.References({ { "S/O", { 10.0, numeric_limits<double>::quiet_NaN() } } });
    Value.m_Value = 100.0;
    Passed = EvaluateFalse("CompareToReference()", "NaN reference error", "A NaN reference uncertainty does not accept a wrong measurement", Tester.Compare(Value)) && Passed;
    // A comparison which rejects its input records nothing and leaves the preceding key result as it is
    {
      Probe Fresh;
      Fresh.References({ { "S/O", { 10.0, numeric_limits<double>::quiet_NaN() } } });
      ETMeasuredValue First;
      First.m_Value = 10.0;
      First.m_Error = 0.8;
      Passed = EvaluateFalse("CompareToReference()", "invalid first", "An invalid first comparison fails", Fresh.Compare(First)) && Passed;
      Passed = Evaluate("CompareToReference()", "invalid first recorded", "An invalid first comparison records nothing", Fresh.Recorded(), static_cast<size_t>(0)) && Passed;
      Fresh.References({ { "S/O", { 10.0, 0.6 }, }, { "S/P", { 10.0, numeric_limits<double>::quiet_NaN() } } });
      Passed = EvaluateTrue("CompareToReference()", "valid first", "A valid comparison passes", Fresh.Compare(First)) && Passed;
      ETMeasuredValue Other;
      Other.m_Value = 10.0;
      Other.m_Error = 0.3;
      Passed = EvaluateFalse("CompareToReference()", "invalid second", "An invalid second comparison fails", Fresh.CompareKey("S", "P", Other)) && Passed;
      Passed = Evaluate("CompareToReference()", "invalid second recorded", "An invalid second comparison records nothing", Fresh.Recorded(), static_cast<size_t>(1)) && Passed;
      Passed = Evaluate("CompareToReference()", "preceding error", "The preceding key result keeps its error", Fresh.Last().GetError(), 0.8) && Passed;
    }
    Tester.References({ });
    Value.m_Value = 10.0;
    Passed = EvaluateFalse("CompareToReference()", "no reference", "Without a reference the comparison fails", Tester.Compare(Value)) && Passed;

    // A run of a reference campaign makes the new references: the old ones neither pass nor fail it
    setenv(MEndToEndFingerprint::GetCampaignVariable().Data(), "1", 1);
    Passed = EvaluateTrue("CompareToReference()", "campaign no reference", "In a campaign a missing reference does not fail", Tester.Compare(Value)) && Passed;
    Tester.References({ { "S/O", { 10.0, 0.6 } } });
    Value.m_Value = 100.0;
    Passed = EvaluateTrue("CompareToReference()", "campaign mismatch", "In a campaign a measurement far from the reference does not fail", Tester.Compare(Value)) && Passed;
    // A measurement which is not valid fails the test also in a campaign, and is not recorded
    const size_t Recorded = Tester.Recorded();
    Value.m_Value = numeric_limits<double>::quiet_NaN();
    Passed = EvaluateFalse("CompareToReference()", "campaign NaN", "In a campaign a NaN measurement fails", Tester.Compare(Value)) && Passed;
    Value.m_Value = 1.0;
    Value.m_Error = numeric_limits<double>::infinity();
    Passed = EvaluateFalse("CompareToReference()", "campaign infinite error", "In a campaign a measurement with an infinite uncertainty fails", Tester.Compare(Value)) && Passed;
    Passed = Evaluate("CompareToReference()", "campaign not recorded", "A measurement which is not valid is not recorded", Tester.Recorded(), Recorded) && Passed;
    Value.m_Value = 100.0;
    Value.m_Error = 0.8;
    unsetenv(MEndToEndFingerprint::GetCampaignVariable().Data());
    Passed = EvaluateFalse("CompareToReference()", "after campaign", "Outside a campaign the same measurement fails", Tester.Compare(Value)) && Passed;
  }

  // Test the summary: Run() writes the pages (title, key results, pulls, and the page of the test) as PDF and ROOT file, and the key results as text
  {
    const MString Directory = GetTemporaryDirectoryName("summary");
    Passed = EvaluateTrue("MFile::CreateDirectory()", "summary", "The directory for the summary can be created", MFile::CreateDirectory(Directory)) && Passed;
    bool Result = false;
    {
      Probe Tester;
      Tester.SummaryDirectory(Directory);
      Tester.References({ { "S/O", { 10.0, 0.5 } } });
      ETMeasuredValue Value;
      Value.m_Value = 10.2;
      Value.m_Error = 0.3;
      Tester.Compare(Value);
      Tester.Key("Free", 1.0, 0.1);
      Tester.Page();
      DisableDefaultStreams();
      Result = Tester.Run();
      EnableDefaultStreams();
    }
    Passed = EvaluateTrue("Run()", "result", "Run() returns the result of the test", Result) && Passed;
    Passed = EvaluateTrue("Run()", "PDF", "Run() writes the summary PDF", MFile::Exists(Directory + "/Probe.pdf")) && Passed;
    Passed = EvaluateTrue("Run()", "ROOT file", "Run() writes the ROOT file with the canvases", MFile::Exists(Directory + "/Probe.summary.root")) && Passed;
    Passed = EvaluateTrue("Run()", "key results", "Run() writes the key results as text", MFile::Exists(Directory + "/Probe.keyresults.tsv")) && Passed;
    MEndToEndResultList Written;
    Passed = EvaluateTrue("Run()", "key results read", "The key results of Run() can be read", Written.Read(Directory + "/Probe.keyresults.tsv")) && Passed;
    Passed = EvaluateSize("Run()", "key results number", "The comparison and the free key result are written", Written.GetResults().size(), 2) && Passed;
    Passed = EvaluateTrue("Run()", "fingerprint", "Run() writes the environment", MFile::Exists(Directory + "/Probe.fingerprint.tsv")) && Passed;
    MEndToEndRecord Environment;
    Passed = EvaluateTrue("Run()", "fingerprint read", "The file of the environment can be read", Environment.Read(Directory + "/Probe.fingerprint.tsv")) && Passed;
    Passed = Evaluate("Run()", "fingerprint test", "The environment names the test", Environment.Get("test"), MString("Probe")) && Passed;
    Passed = Evaluate("Run()", "fingerprint root", "The environment has the ROOT version", Environment.Get("root"), MString(gROOT->GetVersion())) && Passed;
    Passed = EvaluateSize("Run()", "fingerprint keys", "The environment has the test and the four keys of the fingerprint", Environment.GetFields().size(), 5) && Passed;

    // A test with pages but without key results writes the summary and no file of key results
    const MString PageDirectory = GetTemporaryDirectoryName("pageonly");
    Passed = EvaluateTrue("MFile::CreateDirectory()", "pageonly", "The directory for the summary of a test without key results can be created", MFile::CreateDirectory(PageDirectory)) && Passed;
    {
      Probe PageOnly;
      PageOnly.SummaryDirectory(PageDirectory);
      PageOnly.Page();
      DisableDefaultStreams();
      PageOnly.Run();
      EnableDefaultStreams();
    }
    Passed = EvaluateTrue("Run()", "page only PDF", "A test without key results writes the summary PDF", MFile::Exists(PageDirectory + "/Probe.pdf")) && Passed;
    Passed = EvaluateFalse("Run()", "page only key results", "A test without key results writes no file of key results", MFile::Exists(PageDirectory + "/Probe.keyresults.tsv")) && Passed;
  }
  {
    // Without pages and key results nothing is written
    const MString Directory = GetTemporaryDirectoryName("empty");
    Passed = EvaluateTrue("MFile::CreateDirectory()", "empty", "The directory for the empty summary can be created", MFile::CreateDirectory(Directory)) && Passed;
    {
      Probe Tester;
      Tester.SummaryDirectory(Directory);
      DisableDefaultStreams();
      Tester.Run();
      EnableDefaultStreams();
    }
    Passed = EvaluateFalse("Run()", "empty", "A test without pages and key results writes no summary", MFile::Exists(Directory + "/Probe.pdf")) && Passed;
    Passed = EvaluateFalse("Run()", "empty fingerprint", "A test without key results writes no environment", MFile::Exists(Directory + "/Probe.fingerprint.tsv")) && Passed;
  }

  // Test the comparison with a limit: the outcome of the check is the outcome in the key results
  {
    Probe Tester;
    ETMeasuredValue Expected, Value;
    Expected.m_Value = 4.1;
    Value.m_Value = 4.5;
    Passed = EvaluateTrue("CompareValues()", "inside", "A difference of 0.4 is inside the tolerance 0.5", Tester.CompareLimit(Value, Expected, 0.5, false)) && Passed;
    Passed = EvaluateTrue("CompareValues()", "recorded inside", "The key result records the passed check", Tester.Last().IsPassed()) && Passed;
    Value.m_Value = 4.7;
    Passed = EvaluateFalse("CompareValues()", "outside", "A difference of 0.6 is outside the tolerance 0.5", Tester.CompareLimit(Value, Expected, 0.5, false)) && Passed;
    Passed = EvaluateFalse("CompareValues()", "recorded outside", "The key result records the failed check", Tester.Last().IsPassed()) && Passed;
    Passed = EvaluateNear("CompareValues()", "recorded tolerance", "The key result records the tolerance of the check", Tester.Last().GetTolerance(), 0.5, 0.0) && Passed;
    const size_t Before = Tester.Recorded();
    Value.m_Error = numeric_limits<double>::quiet_NaN();
    Passed = EvaluateFalse("CompareValues()", "NaN", "A NaN uncertainty fails the comparison", Tester.CompareLimit(Value, Expected, 100.0, true)) && Passed;
    Passed = Evaluate("CompareValues()", "not recorded", "A comparison with a NaN uncertainty is not recorded as a key result", Tester.Recorded(), Before) && Passed;
  }

  // Test the maximum of a graph: an empty graph has no maximum
  {
    TGraph Empty;
    Passed = EvaluateNear("GetMaximumY()", "empty", "A graph without points has the maximum zero", Probe::MaximumY(&Empty), 0.0, 0.0) && Passed;
    TGraph Points;
    Points.SetPoint(0, 1.0, 2.5);
    Points.SetPoint(1, 2.0, 7.5);
    Points.SetPoint(2, 3.0, 4.0);
    Passed = EvaluateNear("GetMaximumY()", "points", "The maximum of the y values", Probe::MaximumY(&Points), 7.5, 0.0) && Passed;
  }

  // Test the references from a file: the first comparison tells if they were made in another environment
  {
    const MString Directory = GetTemporaryDirectoryName("references");
    Passed = EvaluateTrue("MFile::CreateDirectory()", "references", "The directory for the references can be created", MFile::CreateDirectory(Directory)) && Passed;
    const MString FileName = Directory + "/Probe.references.tsv";
    Probe Tester;
    MEndToEndReferenceList Made;
    MEndToEndReference Reference("S/O");
    Reference.SetMean(10.0);
    Reference.SetError(0.5);
    Reference.SetScatter(1.7);
    Reference.SetRuns(12);
    Made.SetReference(Reference);
    // A calibration of two: the bootstrap error of a run counts twice
    Reference.SetName("S/C");
    Reference.SetCalibration(2.0);
    Made.SetReference(Reference);
    // A relative floor of 0.5% of 100 and an absolute floor of 0.2 for references which are very precise
    MEndToEndReference Precise("S/F");
    Precise.SetMean(100.0);
    Precise.SetError(0.01);
    Precise.SetRuns(12);
    Precise.SetRelativeFloor(0.005);
    Made.SetReference(Precise);
    MEndToEndReference Zero("S/A");
    Zero.SetAbsoluteFloor(0.2);
    Zero.SetRuns(12);
    Made.SetReference(Zero);
    Made.GetHeader().Set("runs", "12");
    Made.GetHeader().Set("date", "2026-10-12");
    Made.GetHeader().Set("geant4", "0.0.0 - not this one");
    Passed = EvaluateTrue("MEndToEndReferenceList::Write()", "file", "The references can be written", Made.Write(FileName)) && Passed;
    Passed = EvaluateTrue("LoadReferences()", "file", "The references are loaded from the file", Tester.Load(FileName)) && Passed;
    Passed = EvaluateNear("GetReference()", "loaded value", "The mean of the loaded reference", Tester.Reference("S", "O").m_Value, 10.0, 0.0) && Passed;
    Passed = EvaluateNear("GetReference()", "loaded error", "The error of the loaded reference", Tester.Reference("S", "O").m_Error, 0.5, 0.0) && Passed;
    // A header which does not match this environment: the differences are warnings, the comparison goes on
    const vector<MString> Differences = Tester.Differences();
    Passed = EvaluateTrue("CheckReferences()", "differences", "The record of the Geant4 version does not match", Differences.size() >= 1 && Differences[0].BeginsWith("geant4: references 0.0.0") == true) && Passed;
    Passed = EvaluateSize("CheckReferences()", "all fields", "The Geant4 version is recorded wrongly, ROOT, geometry, and scenarios are not recorded", Differences.size(), 4) && Passed;
    ETMeasuredValue Value;
    Value.m_Value = 10.2;
    Value.m_Error = 0.3;
    Passed = EvaluateTrue("CompareToReference()", "from file", "The comparison with the loaded reference passes", Tester.Compare(Value)) && Passed;
    Passed = EvaluateSize("CompareToReference()", "title lines", "The first comparison adds the note on the references to the title page, the environment is checked at the end of the test", Tester.Titles().size(), 1) && Passed;
    Passed = EvaluateTrue("CompareToReference()", "from references", "The key result of a comparison with a reference is marked", Tester.Last().IsFromReferences()) && Passed;
    Passed = EvaluateTrue("CompareToReference()", "note", "The first title line names the file and the number of runs", Tester.Titles()[0].Contains("Probe.references.tsv, 12 runs") == true) && Passed;
    Tester.Compare(Value);
    Passed = EvaluateSize("CompareToReference()", "once", "The second comparison adds no further lines", Tester.Titles().size(), 1) && Passed;
    // The calibration of two: the error 0.3 of the run counts as 0.6, the combined error is sqrt(0.36 + 0.25) = 0.781 and the tolerance 3.905: 13.8 is inside, without the calibration (error 0.3) it is not
    Value.m_Value = 13.8;
    Passed = EvaluateTrue("CompareToReference()", "calibrated inside", "A difference of 3.8 is inside five calibrated errors", Tester.CompareKey("S", "C", Value)) && Passed;
    Passed = EvaluateNear("CompareToReference()", "raw error", "The key result keeps the error of the run as it was measured", Tester.Last().GetError(), 0.3, 0.0) && Passed;
    Passed = EvaluateFalse("CompareToReference()", "uncalibrated", "The same difference is outside five errors without the calibration", Tester.CompareKey("S", "O", Value)) && Passed;
    Value.m_Value = 14.0;
    Passed = EvaluateFalse("CompareToReference()", "calibrated outside", "A difference of 4.0 is outside five calibrated errors", Tester.CompareKey("S", "C", Value)) && Passed;

    // The floors: 5 combined errors of 100 +- 0.01 and 0.01 are 0.07, the relative floor 0.5% is 0.5
    Value.m_Value = 100.4;
    Value.m_Error = 0.01;
    Passed = EvaluateTrue("CompareToReference()", "relative floor inside", "A difference of 0.4 is inside the relative floor of 0.5", Tester.CompareKey("S", "F", Value)) && Passed;
    Passed = EvaluateFalse("CompareToReference()", "floor is no pull", "A comparison which is decided by the floor is not a statistical pull", Tester.Last().IsStatistical()) && Passed;
    Passed = EvaluateNear("CompareToReference()", "floor tolerance", "The tolerance is the floor", Tester.Last().GetTolerance(), 0.5, 1e-12) && Passed;
    Value.m_Value = 100.6;
    Passed = EvaluateFalse("CompareToReference()", "relative floor outside", "A difference of 0.6 is outside the relative floor of 0.5", Tester.CompareKey("S", "F", Value)) && Passed;
    Value.m_Value = 0.15;
    Passed = EvaluateTrue("CompareToReference()", "absolute floor inside", "A difference of 0.15 is inside the absolute floor of 0.2", Tester.CompareKey("S", "A", Value)) && Passed;
    Value.m_Value = 0.25;
    Passed = EvaluateFalse("CompareToReference()", "absolute floor outside", "A difference of 0.25 is outside the absolute floor of 0.2", Tester.CompareKey("S", "A", Value)) && Passed;
    Value.m_Value = 10.2;
    Value.m_Error = 0.3;

    // The environment is checked when the test ends: the four differences (Geant4 recorded wrongly, ROOT, geometry, and scenarios not recorded) go to the title page
    Tester.NoReportDirectory();
    Tester.Run();
    Passed = EvaluateSize("Run()", "differences", "The end of the test adds the four differences of the environment to the title page", Tester.Titles().size(), 5) && Passed;

    // The scenarios of the fingerprint are the ones which are prepared when the test ends, also those prepared after the first comparison
    {
      Probe Both;
      vector<ETSimScenario> AB(2);
      AB[0].m_Name = "A";
      AB[1].m_Name = "B";
      Passed = EvaluateTrue("PrepareSimulations()", "A and B", "Both scenarios can be prepared", Both.PrepareAll(AB)) && Passed;
      const map<MString, MString> Environment = Both.Fingerprint();
      MEndToEndReferenceList Late;
      MEndToEndReference LateReference("A/O");
      LateReference.SetMean(10.0);
      LateReference.SetError(0.5);
      LateReference.SetRuns(12);
      Late.SetReference(LateReference);
      Late.GetHeader().Set("runs", "12");
      Late.GetHeader().Set("date", "2026-10-19");
      for (const pair<const MString, MString>& Entry: Environment) {
        Late.GetHeader().Set(Entry.first, Entry.second);
      }
      Passed = EvaluateTrue("MEndToEndReferenceList::Write()", "late", "The references with the environment of both scenarios can be written", Late.Write(Directory + "/Late.references.tsv")) && Passed;
      Probe Tester2;
      Tester2.NoReportDirectory();
      Passed = EvaluateTrue("LoadReferences()", "late", "The references are loaded", Tester2.Load(Directory + "/Late.references.tsv")) && Passed;
      vector<ETSimScenario> OnlyA(1);
      OnlyA[0].m_Name = "A";
      Passed = EvaluateTrue("PrepareSimulations()", "A first", "The first scenario can be prepared", Tester2.PrepareAll(OnlyA)) && Passed;
      Passed = EvaluateTrue("CompareToReference()", "A compared", "The comparison of the first scenario passes", Tester2.CompareKey("A", "O", Value)) && Passed;
      vector<ETSimScenario> OnlyB(1);
      OnlyB[0].m_Name = "B";
      Passed = EvaluateTrue("PrepareSimulations()", "B after", "The second scenario can be prepared after the first comparison", Tester2.PrepareAll(OnlyB)) && Passed;
      Tester2.Run();
      Passed = EvaluateSize("Run()", "late scenario", "The environment of the references is the one of both scenarios: no difference, only the note on the references", Tester2.Titles().size(), 1) && Passed;
    }

    DisableDefaultStreams();
    const bool Missing = Tester.Load(Directory + "/Missing.references.tsv");
    EnableDefaultStreams();
    Passed = EvaluateFalse("LoadReferences()", "missing file", "A missing file of references is reported", Missing) && Passed;
    Passed = EvaluateFalse("CompareToReference()", "after missing file", "Without references a comparison fails", Tester.Compare(Value)) && Passed;
  }

  // Test the random seeds: a base seed in the environment makes them reproducible, setenv and unsetenv are POSIX, the C++ standard library has no equivalent
  {
    const char* Old = getenv(MEndToEndFingerprint::GetSeedVariable().Data());
    MString OldValue;
    if (Old != nullptr) {
      OldValue = Old;
    }
    setenv(MEndToEndFingerprint::GetSeedVariable().Data(), "4711", 1);
    Probe First, Second;
    const unsigned int A1 = First.Seed();
    const unsigned int A2 = First.Seed();
    const unsigned int B1 = Second.Seed();
    Passed = Evaluate("RandomSeed()", "reproducible", "The same base seed gives the same first seed", A1, B1) && Passed;
    Passed = EvaluateTrue("RandomSeed()", "sequence", "The next seed is another one", A1 != A2) && Passed;
    Passed = EvaluateTrue("RandomSeed()", "range", "The seeds are between 1 and 2000000000", A1 >= 1 && A1 <= 2000000000U && A2 >= 1 && A2 <= 2000000000U) && Passed;
    setenv(MEndToEndFingerprint::GetSeedVariable().Data(), "4712", 1);
    Probe Other;
    Passed = EvaluateTrue("RandomSeed()", "other base", "Another base seed gives another first seed", Other.Seed() != A1) && Passed;
    unsetenv(MEndToEndFingerprint::GetSeedVariable().Data());
    Probe Random1, Random2;
    Passed = EvaluateTrue("RandomSeed()", "random", "Without a base seed the seeds are random", Random1.Seed() != Random2.Seed()) && Passed;
    if (Old != nullptr) {
      setenv(MEndToEndFingerprint::GetSeedVariable().Data(), OldValue.Data(), 1);
    }
  }

  // Test the hash of a scenario: it depends on the definition, not on the seed and the directory
  {
    Probe Tester;
    ETSimScenario Scenario;
    Scenario.m_Name = "Line";
    Scenario.SetFarFieldPointSource(0.0, 0.0, "Mono 100", 1.0);
    Scenario.m_Time = 100.0;
    const MString Hash = Tester.ScenarioHash(Scenario);
    Passed = EvaluateTrue("GetScenarioHash()", "length", "The hash is a number", Hash.IsEmpty() == false && Hash.IsNumber() == true) && Passed;
    ETSimScenario Seeded = Scenario;
    Seeded.m_Seed = 4711;
    Seeded.m_Directory = "/somewhere/else";
    Passed = Evaluate("GetScenarioHash()", "seed and directory", "The seed and the directory do not change the hash", Tester.ScenarioHash(Seeded), Hash) && Passed;
    ETSimScenario Other = Scenario;
    Other.m_Time = 200.0;
    Passed = EvaluateTrue("GetScenarioHash()", "time", "A different time changes the hash", Tester.ScenarioHash(Other) != Hash) && Passed;
    Other = Scenario;
    Other.SetFarFieldPointSource(0.0, 0.0, "Mono 101", 1.0);
    Passed = EvaluateTrue("GetScenarioHash()", "source", "A different source changes the hash", Tester.ScenarioHash(Other) != Hash) && Passed;
    Other = Scenario;
    Other.m_Files.push_back(make_pair(MString("a.dat"), MString("content")));
    Passed = EvaluateTrue("GetScenarioHash()", "files", "An additional file changes the hash", Tester.ScenarioHash(Other) != Hash) && Passed;

    // The fingerprint of this environment: the scenarios which were prepared are part of it
    const map<MString, MString> Before = Tester.Fingerprint();
    Passed = EvaluateSize("GetFingerprint()", "keys", "Geant4, ROOT, geometry, and scenarios", Before.size(), 4) && Passed;
    Passed = EvaluateTrue("GetFingerprint()", "ROOT", "The ROOT version is the one of the running program", Before.at("root") == MString(gROOT->GetVersion())) && Passed;
    // The geometry is its file and the files which it includes
    const MString GeometryDirectory = GetTemporaryDirectoryName("fingerprint_geometry");
    MFile::CreateDirectory(GeometryDirectory);
    { ofstream Out((GeometryDirectory + "/Top.geo.setup").Data()); Out<<"Name Mini\nVersion 1.0\nSurroundingSphere 100 0 0 0 100\nInclude $(MEGALIB)/resource/examples/geomega/materials/Materials.geo\nInclude Part.geo\n"<<endl; }
    { ofstream Out((GeometryDirectory + "/Part.geo").Data()); Out<<"Volume World\nWorld.Material Vacuum\nWorld.Visibility 0\nWorld.Shape BRIK 1000 1000 1000\nWorld.Mother 0\n// A"<<endl; }
    const MString OriginalGeometry = Tester.GetGeometry();
    Tester.SetGeometry(GeometryDirectory + "/Top.geo.setup");
    const MString TopHash = Tester.Fingerprint().at("geometry");
    { ofstream Out((GeometryDirectory + "/Part.geo").Data()); Out<<"Volume World\nWorld.Material Vacuum\nWorld.Visibility 0\nWorld.Shape BRIK 1000 1000 1000\nWorld.Mother 0\n// B"<<endl; }
    // The geometry is read once: a change of a file after the first fingerprint is not seen until the geometry is set again
    Passed = Evaluate("GetFingerprint()", "geometry read once", "The hash of the geometry is the one of the first reading, the geometry does not change during a test", Tester.Fingerprint().at("geometry"), TopHash) && Passed;
    Tester.SetGeometry(GeometryDirectory + "/Top.geo.setup");
    Passed = EvaluateTrue("GetFingerprint()", "included geometry", "A change of an included geometry file changes the hash of the geometry", Tester.Fingerprint().at("geometry") != TopHash) && Passed;
    Tester.SetGeometry(GeometryDirectory + "/None.geo.setup");
    Passed = Evaluate("GetFingerprint()", "missing geometry", "A missing geometry file is unknown", Tester.Fingerprint().at("geometry"), MString("unknown")) && Passed;
    Tester.SetGeometry(OriginalGeometry);

    Passed = EvaluateTrue("PrepareScenario()", "prepared", "The scenario can be prepared", Tester.Prepare(Scenario)) && Passed;
    Passed = EvaluateTrue("GetFingerprint()", "scenarios", "A prepared scenario changes the hash of the scenarios", Tester.Fingerprint().at("scenarios") != Before.at("scenarios")) && Passed;
  }

  // Test the references
  {
    Probe Tester;
    Tester.References({ { "S/O", { 10.0, 0.5 } } });
    Passed = EvaluateNear("GetReference()", "value", "The reference value of a known key", Tester.Reference("S", "O").m_Value, 10.0, 0.0) && Passed;
    Passed = EvaluateNear("GetReference()", "error", "The reference error of a known key", Tester.Reference("S", "O").m_Error, 0.5, 0.0) && Passed;
    Passed = EvaluateNear("GetReference()", "unknown", "An unknown key has the reference zero", Tester.Reference("S", "X").m_Value, 0.0, 0.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("UTEndToEndTest", "unit test of the end-to-end test base class") == false) {
    return 1;
  }

  UTEndToEndTest Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


////////////////////////////////////////////////////////////////////////////////
