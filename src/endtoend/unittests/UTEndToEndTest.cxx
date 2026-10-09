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
#include "MFile.h"
#include "MSettingsTesting.h"
#include "MSystem.h"

// Standard libs:
#include <filesystem>
#include <chrono>
#include <cmath>
#include <thread>
#include <fstream>
using namespace std;


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
    virtual bool Run() { return true; }
    bool Run(const MString& Directory, const MString& Executable, const MString& Arguments) { return Execute(Directory, Executable, Arguments, 10); }
    vector<MString> Lines(const MString& FileName) { return ReadLines(FileName); }
    ETTraFileCoreData Tra(const MString& FileName) { return ReadTraFile(FileName); }
    ETSimFileCoreData Read(const ETSimScenario& Scenario) { return ReadSim(Scenario); }
    bool Prepare(ETSimScenario& Scenario) { return PrepareScenario(Scenario); }
    bool PrepareSim(ETSimScenario& Scenario) { return PrepareSimulation(Scenario); }
    bool Sim(const ETSimScenario& Scenario) { return Simulate(Scenario); }
    bool Verify() { return VerifyEnvironment(); }
    unique_ptr<TH1D> First1D(const MString& FileName) { return FirstHistogram<TH1D>(FileName); }
    bool Peak(const TH2* Image, double Range, double& CentroidX, double& CentroidY) { return ImagePeak(Image, Range, CentroidX, CentroidY); }
    double Width(const TH1* Histogram) { return FullWidthAtHalfMaximum(Histogram); }
    unsigned int Budget() { return GetRemainingTimeBudget(); }
    MString Home() { return GetTemporaryDirectoryName("home"); }
    bool RunIn(const MString& Directory, const MString& Executable, const MString& Arguments, unsigned int Timeout) { return Execute(Directory, Executable, Arguments, Timeout); }
    bool PrepareAll(vector<ETSimScenario>& Scenarios) { return PrepareSimulations(Scenarios); }
    double Arm(const MComptonEvent& Event, const MVector& ToSource) { return ARM(Event, ToSource); }
    double Median(const vector<double>& Values) { return GetMedian(Values); }
    double MedianError(const vector<double>& Values) { return GetMedianError(Values); }
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
  Passed = EvaluateTrue("Execute()", "spaces", "A command runs in a directory with spaces and quotes", Tester.Run(Base, "touch", "executed.txt")) && Passed;
  Passed = EvaluateTrue("Execute()", "result", "The command ran in that directory", filesystem::exists((Base + "/executed.txt").Data())) && Passed;

  // The programs have a private home directory, not the one of the user
  Passed = EvaluateTrue("Execute()", "home", "A program can write the home directory it sees", Tester.Run(Base, "sh", "-c 'echo \"$HOME\" > home.txt'")) && Passed;
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
  const MString OldDisplayCopy = (OldDisplay == nullptr) ? "" : OldDisplay;
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

  // Use a triangle at x = 5 with the base from 2 to 8 and bin width 0.1 - highest bin centers 4.95 and 5.05 at 9.8333, half maximum 4.91667
  // Expected: interpolation between bin centers 3.45 (4.8333) and 3.55 (5.1667) crosses at 3.475, symmetrically at 6.525, width 3.05
  TH1D Triangle("UTTriangle", "UTTriangle", 100, 0.0, 10.0);
  Triangle.SetDirectory(nullptr);
  for (int b = 1; b <= 100; ++b) {
    const double CentroidX = Triangle.GetXaxis()->GetBinCenter(b);
    const double Height = 10.0*(1.0 - fabs(CentroidX - 5.0)/3.0);
    if (Height > 0.0) {
      Triangle.SetBinContent(b, Height);
    }
  }
  Passed = EvaluateNear("FullWidthAtHalfMaximum()", "triangle", "The width of a triangle at half its maximum of the bin centers", Tester.Width(&Triangle), 3.05, 1e-9) && Passed;
  TH1D Empty("UTEmpty", "UTEmpty", 10, 0.0, 10.0);
  Empty.SetDirectory(nullptr);
  Passed = EvaluateNear("FullWidthAtHalfMaximum()", "empty", "An empty histogram has no width", Tester.Width(&Empty), 0.0, 1e-12) && Passed;

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

  // Test the median of the absolute values and its standard error 1/(2 f sqrt(N)) with f from the sqrt(N) values on each side of the median:
  {
    Probe Tester;
    // Values (j + 0.5)/10000 for j = 0 ... 4999, each twice - median 0.25 (mean of j = 2499 and 2500)
    // Expected error: 1/(2*2*sqrt(10000)) = 0.0025 with density 2 on [0, 0.5]
    vector<double> Uniform;
    for (int i = 0; i < 5000; ++i) {
      Uniform.push_back((i + 0.5)/10000.0);
      Uniform.push_back((i + 0.5)/10000.0);
    }
    Passed = EvaluateNear("GetMedian()", "uniform", "The median of a uniform distribution", Tester.Median(Uniform), 0.25, 1e-12) && Passed;
    Passed = EvaluateNear("GetMedianError()", "uniform", "The error of a uniform distribution follows its density", Tester.MedianError(Uniform), 0.0025, 1e-9) && Passed;

    // {1, 2, 3} - median 2, f = 2/6 = 1/3, error 1/(2*(1/3)*sqrt(3)) = sqrt(3)/2
    Passed = EvaluateNear("GetMedian()", "three", "The median of an odd number of values", Tester.Median(vector<double>{3.0, 1.0, 2.0}), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMedianError()", "three", "The error of three values", Tester.MedianError(vector<double>{3.0, 1.0, 2.0}), sqrt(3.0)/2.0, 1e-12) && Passed;

    // {1, 2, 3, 4} - median (2 + 3)/2, f = 3/(4*3) = 1/4, error 1/(2*(1/4)*2) = 1
    Passed = EvaluateNear("GetMedian()", "four", "The median of an even number of values is the mean of the two in the middle", Tester.Median(vector<double>{4.0, 1.0, 3.0, 2.0}), 2.5, 1e-12) && Passed;
    Passed = EvaluateNear("GetMedianError()", "four", "The error of four values", Tester.MedianError(vector<double>{4.0, 1.0, 3.0, 2.0}), 1.0, 1e-12) && Passed;

    // {-3, -1, 2, 4} - median (-1 + 2)/2
    Passed = EvaluateNear("GetMedian()", "negative", "The median of values with a sign", Tester.Median(vector<double>{-1.0, 2.0, -3.0, 4.0}), 0.5, 1e-12) && Passed;

    // Identical values have no width - error 0; no values - median 0 and error 0
    Passed = EvaluateNear("GetMedian()", "identical", "The median of identical values", Tester.Median(vector<double>{5.0, 5.0, 5.0, 5.0}), 5.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMedianError()", "identical", "The error of identical values is zero", Tester.MedianError(vector<double>{5.0, 5.0, 5.0, 5.0}), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("GetMedian()", "empty", "The median of no values is zero", Tester.Median(vector<double>()), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("GetMedianError()", "empty", "The error of no values is zero", Tester.MedianError(vector<double>()), 0.0, 0.0) && Passed;
  }

  return Passed;
}


////


int main()
{
  UTEndToEndTest Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


////////////////////////////////////////////////////////////////////////////////
