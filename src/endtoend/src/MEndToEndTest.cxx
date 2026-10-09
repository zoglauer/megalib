/*
 * MEndToEndTest.cxx
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

// Include the header:
#include "MEndToEndTest.h"

// Standard libs:
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <random>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MFile.h"
#include "MSettingsTesting.h"
#include "MSystem.h"


////////////////////////////////////////////////////////////////////////////////


//! Standard constructor giving the name of the test
MEndToEndTest::MEndToEndTest(const MString& Name) : MUnitTest(Name)
{
  // Read the time out of the test driver (it is adapted to the speed of this machine), it counts from now
  MSettingsTesting Settings;
  Settings.Read();
  m_Timeout = Settings.GetTimeout();
  m_StartTime = chrono::steady_clock::now();

  // ROOT without any graphics, needed to read the ROOT files of mimrec
  if (gROOT->GetApplication() == nullptr) {
    new TApplication("ROOT", 0, 0);
  }
  gROOT->SetBatch(true);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MEndToEndTest::~MEndToEndTest()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Return the geometry file: the one which was set, otherwise the default Max.geo.setup of the examples, empty only if MEGALIB is not defined
MString MEndToEndTest::GetGeometry() const
{
  if (m_Geometry != "") {
    return m_Geometry;
  }

  MString FileName = "$(MEGALIB)/resource/examples/geomega/special/Max.geo.setup";
  if (MFile::ExpandFileName(FileName) == false) {
    return "";
  }
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


//! Check (as a test) that MEGAlib is set up ($(MEGALIB), the geometry file, and the programs cosima, revan, and mimrec in the path)
bool MEndToEndTest::VerifyEnvironment()
{
  bool Passed = true;
  Passed = EvaluateTrue("VerifyEnvironment()", "MEGALIB", "MEGAlib is set up: the environment variable MEGALIB is defined (source bin/source-megalib.sh)", getenv("MEGALIB") != nullptr) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "geometry", "The geometry file is known ($(MEGALIB) is set or a geometry was given) and exists", GetGeometry().IsEmpty() == false && MFile::Exists(GetGeometry()) == true) && Passed;
  for (const MString& Program: vector<MString>({ "cosima", "revan", "mimrec" })) {
    Passed = EvaluateTrue("ProgramExists()", Program, MString("The program ") + Program + " of MEGAlib is found (MEGAlib is set up: source bin/source-megalib.sh)", MFile::ProgramExists(Program)) && Passed;
  }
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the time in seconds which is left of the time out of this test, at least one second, 0 without time out
unsigned int MEndToEndTest::GetRemainingTimeBudget() const
{
  if (m_Timeout <= 0.0) {
    return 0;
  }
  const double Elapsed = chrono::duration<double>(chrono::steady_clock::now() - m_StartTime).count();
  const double Left = m_Timeout - Elapsed;
  if (Left < 1.0) {
    return 1;
  }
  return static_cast<unsigned int>(ceil(Left));
}


////////////////////////////////////////////////////////////////////////////////


//! Execute a program with its arguments (a shell fragment, e.g. with a redirect) in a directory with a private home directory and a timeout in seconds (0: none), return true if the exit status is zero
bool MEndToEndTest::Execute(const MString& Directory, const MString& Executable, const MString& Arguments, unsigned int Timeout) const
{
  // The programs run with a private home directory: they load their default configuration (~/.revan.cfg, ~/.mimrec.cfg) from there, not the one of the user
  const MString Home = GetTemporaryDirectoryName("home");
  if (MFile::CreateDirectory(Home) == false) {
    return false;
  }
  const int Status = MSystem::RunProcess("env", MString("HOME=") + MSystem::GetShellQuoted(Home) + " " + MSystem::GetShellQuoted(Executable) + " " + Arguments, "", Directory, Timeout);
  if (WIFEXITED(Status) == false || WEXITSTATUS(Status) != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a random seed (1 .. 2e9) which is printed by the tests: any seed has to pass
unsigned int MEndToEndTest::RandomSeed()
{
  const unsigned int MaximumSeed = 2000000000;
  static random_device Device;
  return 1 + Device() % MaximumSeed;
}


////////////////////////////////////////////////////////////////////////////////


//! Prepare the directory of a scenario (no random numbers are involved: e.g. a scenario which starts from stored data)
bool MEndToEndTest::PrepareScenario(ETSimScenario& Scenario)
{
  const MString Name = MString("EndToEnd_") + Scenario.m_Name;
  if (PrepareTemporaryDirectory(Name) == false) {
    return false;
  }
  Scenario.m_Directory = GetTemporaryDirectoryName(Name);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Prepare the directory of a scenario which is simulated: as PrepareScenario, and give it a random seed unless it has one already
bool MEndToEndTest::PrepareSimulation(ETSimScenario& Scenario)
{
  if (PrepareScenario(Scenario) == false) {
    return false;
  }
  if (Scenario.m_Seed == 0) {
    Scenario.m_Seed = RandomSeed(); // an explicitly given seed is kept
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Prepare the directories of the scenarios of a simulation and give them their seeds (as a test for each scenario), the seeds are printed
bool MEndToEndTest::PrepareSimulations(vector<ETSimScenario>& Scenarios)
{
  bool Passed = true;
  for (ETSimScenario& Scenario: Scenarios) {
    Passed = EvaluateTrue("PrepareSimulation()", Scenario.m_Name, "The directory of the scenario can be created and the scenario gets a random seed", PrepareSimulation(Scenario)) && Passed;
    mout<<Scenario.m_Name<<": seed "<<Scenario.m_Seed<<endl;
  }
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the source file (and the files of the scenario) of a scenario
bool MEndToEndTest::WriteSource(const ETSimScenario& Scenario) const
{
  for (const pair<MString, MString>& Entry: Scenario.m_Files) {
    ofstream File((Scenario.m_Directory + "/" + Entry.first).Data());
    if (File.is_open() == false) {
      return false;
    }
    File<<Entry.second;
  }

  ofstream Out((Scenario.m_Directory + "/" + Scenario.m_Name + ".source").Data());
  if (Out.is_open() == false) {
    return false;
  }
  Out<<"Version 1"<<endl;
  Out<<"Geometry "<<GetGeometry()<<endl;
  Out<<"PhysicsListEM LivermorePol"<<endl;
  Out<<"StoreSimulationInfo all"<<endl;
  Out<<"Run R"<<endl;
  Out<<"R.FileName "<<Scenario.m_Name<<endl;
  if (Scenario.m_Triggers > 0) {
    Out<<"R.Triggers "<<Scenario.m_Triggers<<endl;
  } else {
    Out<<"R.Time "<<Scenario.m_Time<<endl;
  }
  for (const MString& Line: Scenario.m_RunLines) {
    Out<<"R."<<Line<<endl;
  }
  Out<<"R.Source S"<<endl;
  for (const MString& Line: Scenario.m_SourceLines) {
    Out<<"S."<<Line<<endl;
  }
  return Out.good();
}


////////////////////////////////////////////////////////////////////////////////


//! Simulate with cosima
bool MEndToEndTest::Simulate(const ETSimScenario& Scenario) const
{
  if (Scenario.m_Seed == 0) {
    merr<<"Error in MEndToEndTest::Simulate: the scenario "<<Scenario.m_Name<<" has no random seed, prepare it with PrepareSimulation()"<<endl;
    return false;
  }
  if (WriteSource(Scenario) == false) {
    return false;
  }
  return Execute(Scenario.m_Directory, "cosima", MString("-v 0 -s ") + Scenario.m_Seed + " -z " + MSystem::GetShellQuoted(Scenario.m_Name + ".source") + " > cosima.log 2>&1", GetRemainingTimeBudget());
}


////////////////////////////////////////////////////////////////////////////////


//! Create the default revan configuration (revan.cfg) in the given directory
bool MEndToEndTest::CreateRevanConfiguration(const MString& Directory) const
{
  Execute(Directory, "revan", MString("-g ") + MSystem::GetShellQuoted(GetGeometry()) + " --save-cfg -o revan.cfg -n > revan.save.log 2>&1", GetRemainingTimeBudget());
  return Execute(Directory, "test", "-s revan.cfg", GetRemainingTimeBudget());
}


////////////////////////////////////////////////////////////////////////////////


//! Reconstruct with revan using the given configuration file
bool MEndToEndTest::Reconstruct(const ETSimScenario& Scenario, const MString& Configuration) const
{
  return Execute(Scenario.m_Directory, "revan", MString("-f ") + MSystem::GetShellQuoted(Scenario.m_Name + ".inc1.id1.sim.gz") + " -g " + MSystem::GetShellQuoted(GetGeometry()) + " -c " + MSystem::GetShellQuoted(Configuration) + " -a -n > revan.log 2>&1", GetRemainingTimeBudget());
}


////////////////////////////////////////////////////////////////////////////////


//! Simulate with cosima and reconstruct the result with revan using the given configuration file
bool MEndToEndTest::SimulateAndReconstruct(const ETSimScenario& Scenario, const MString& Configuration) const
{
  if (Simulate(Scenario) == false) {
    return false;
  }
  if (Reconstruct(Scenario, Configuration) == false) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Create the default mimrec configuration (mimrec.cfg) in the given directory
bool MEndToEndTest::CreateMimrecConfiguration(const MString& Directory) const
{
  Execute(Directory, "mimrec", MString("-g ") + MSystem::GetShellQuoted(GetGeometry()) + " --save-cfg -o mimrec.cfg -n > mimrec.save.log 2>&1", GetRemainingTimeBudget());
  return Execute(Directory, "test", "-s mimrec.cfg", GetRemainingTimeBudget());
}


////////////////////////////////////////////////////////////////////////////////


//! Run mimrec on the tra file of a scenario with the analysis option, the output file, and the configuration changes ("Key=Value")
bool MEndToEndTest::Mimrec(const ETSimScenario& Scenario, const MString& Configuration, const MString& Option, const MString& Output, const vector<MString>& Changes) const
{
  MString Arguments = MString("-g ") + MSystem::GetShellQuoted(GetGeometry()) + " -f " + MSystem::GetShellQuoted(Scenario.m_Name + ".inc1.id1.tra.gz") + " -c " + MSystem::GetShellQuoted(Configuration) +
                    " -n " + MSystem::GetShellQuoted(Option) + " -o " + MSystem::GetShellQuoted(Output);
  for (const MString& Change: Changes) {
    Arguments += " -C " + MSystem::GetShellQuoted(Change);
  }
  Arguments += MString(" > ") + MSystem::GetShellQuoted(MString("mimrec_") + Output + ".log") + " 2>&1";
  return Execute(Scenario.m_Directory, "mimrec", Arguments, GetRemainingTimeBudget());
}


////////////////////////////////////////////////////////////////////////////////


//! Read the sim file of a scenario with MFileEventsSim
ETSimFileCoreData MEndToEndTest::ReadSim(const ETSimScenario& Scenario)
{
  ETSimFileCoreData Sim;

  // Load the geometry once, with its output suppressed - the reader needs it
  if (m_ReaderGeometry == nullptr) {
    shared_ptr<MDGeometryQuest> Geometry = make_shared<MDGeometryQuest>();
    DisableDefaultStreams();
    const bool Loaded = Geometry->ScanSetupFile(GetGeometry());
    EnableDefaultStreams();
    if (Loaded == false) {
      return Sim;
    }
    m_ReaderGeometry = Geometry;
  }
  Sim.m_Geometry = m_ReaderGeometry;
  MFileEventsSim Reader(Sim.m_Geometry.get());
  if (Reader.Open(Scenario.m_Directory + "/" + Scenario.m_Name + ".inc1.id1.sim.gz") == false) {
    return Sim;
  }

  Sim.m_SimulationTime = Reader.GetObservationTime().GetAsSeconds();
  Sim.m_SimulatedParticles = Reader.GetSimulatedEvents();
  Sim.m_StartArea = Reader.GetSimulationStartAreaFarField();

  bool InitialInteractions = true;
  MSimEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent(false)) != nullptr) {
    // The first interaction INIT has the truth of the initial particle
    if (Event->GetNIAs() == 0 || Event->GetIAAt(0)->GetProcess() != "INIT") {
      InitialInteractions = false;
    }
    Sim.m_Events.push_back(shared_ptr<MSimEvent>(Event));
  }
  Reader.Close();

  Sim.m_FileSuccessfullyRead = false;
  if (Sim.m_SimulationTime > 0 && Sim.m_SimulatedParticles > 0 && InitialInteractions == true) {
    Sim.m_FileSuccessfullyRead = true;
  }

  return Sim;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the tra file of a scenario with MFileEventsTra
ETTraFileCoreData MEndToEndTest::ReadTra(const ETSimScenario& Scenario) const
{
  return ReadTraFile(Scenario.m_Directory + "/" + Scenario.m_Name + ".inc1.id1.tra.gz");
}


////////////////////////////////////////////////////////////////////////////////


//! Read any tra file (gzipped or not) with MFileEventsTra
ETTraFileCoreData MEndToEndTest::ReadTraFile(const MString& FileName) const
{
  ETTraFileCoreData Tra;

  MFileEventsTra Reader;
  if (Reader.Open(FileName) == false) {
    return Tra;
  }

  MPhysicalEvent* Event = nullptr;
  while ((Event = Reader.GetNextEvent()) != nullptr) {
    Tra.m_Events.push_back(shared_ptr<MPhysicalEvent>(Event));
  }
  Reader.Close();
  Tra.m_FileSuccessfullyRead = true;

  return Tra;
}


////////////////////////////////////////////////////////////////////////////////


//! Read a plain text file (e.g. the log of a program) line by line
vector<MString> MEndToEndTest::ReadLines(const MString& FileName)
{
  vector<MString> Lines;
  ifstream In(FileName.Data());
  string Line;
  while (getline(In, Line).fail() == false) {
    while (Line.empty() == false && Line.back() == '\r') {
      Line.pop_back();
    }
    Lines.push_back(Line.c_str());
  }
  return Lines;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the centroid of an image around its maximum (all bins within the range in both axes), return false if the image is empty
bool MEndToEndTest::ImagePeak(const TH2* Image, double Range, double& CentroidX, double& CentroidY) const
{
  int BinX = 0, BinY = 0, BinZ = 0;
  const_cast<TH2*>(Image)->GetMaximumBin(BinX, BinY, BinZ);
  const double MaxX = Image->GetXaxis()->GetBinCenter(BinX);
  const double MaxY = Image->GetYaxis()->GetBinCenter(BinY);
  double Sum = 0.0, SumX = 0.0, SumY = 0.0;
  for (int i = 1; i <= Image->GetNbinsX(); ++i) {
    for (int j = 1; j <= Image->GetNbinsY(); ++j) {
      const double PositionX = Image->GetXaxis()->GetBinCenter(i);
      const double PositionY = Image->GetYaxis()->GetBinCenter(j);
      if (fabs(PositionX - MaxX) < Range && fabs(PositionY - MaxY) < Range) {
        const double Weight = Image->GetBinContent(i, j);
        Sum += Weight;
        SumX += Weight*PositionX;
        SumY += Weight*PositionY;
      }
    }
  }
  if (Sum <= 0) {
    return false;
  }
  CentroidX = SumX/Sum;
  CentroidY = SumY/Sum;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the full width at half maximum of the peak of a histogram, 0 if it cannot be determined
double MEndToEndTest::FullWidthAtHalfMaximum(const TH1* Histogram) const
{
  const int Peak = const_cast<TH1*>(Histogram)->GetMaximumBin();
  const double Half = 0.5*Histogram->GetBinContent(Peak);
  if (Half <= 0) {
    return 0.0;
  }
  double Left = Histogram->GetXaxis()->GetBinCenter(1), Right = Histogram->GetXaxis()->GetBinCenter(Histogram->GetNbinsX());
  for (int b = Peak; b > 1; --b) {
    if (Histogram->GetBinContent(b - 1) < Half) {
      const double Low = Histogram->GetBinContent(b - 1), High = Histogram->GetBinContent(b);
      Left = Histogram->GetXaxis()->GetBinCenter(b - 1) + (Half - Low)/(High - Low)*Histogram->GetXaxis()->GetBinWidth(b);
      break;
    }
  }
  for (int b = Peak; b < Histogram->GetNbinsX(); ++b) {
    if (Histogram->GetBinContent(b + 1) < Half) {
      const double Low = Histogram->GetBinContent(b + 1), High = Histogram->GetBinContent(b);
      Right = Histogram->GetXaxis()->GetBinCenter(b + 1) - (Half - Low)/(High - Low)*Histogram->GetXaxis()->GetBinWidth(b);
      break;
    }
  }
  return Right - Left;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the angular resolution measure (ARM) in degree of a Compton event relative to the unit vector to the source
double MEndToEndTest::ARM(const MComptonEvent& Event, const MVector& ToSource)
{
  const double ElectronMass = c_E0; // keV
  const double CosScatter = 1.0 - ElectronMass*(1.0/Event.Eg() - 1.0/(Event.Eg() + Event.Ee()));
  if (CosScatter < -1.0 || CosScatter > 1.0) {
    const double InvalidARM = 1000.0; // deg: no Compton cone exists
    return InvalidARM;
  }
  const MVector Scattered = (Event.C2() - Event.C1()).Unit();
  const double GeometricCos = max(-1.0, min(1.0, (ToSource*(-1.0)).Dot(Scattered)));
  return (acos(GeometricCos) - acos(CosScatter))*c_Deg;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the median of a vector
double MEndToEndTest::Median(vector<double> Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  sort(Values.begin(), Values.end());
  if (Values.size() % 2 == 1) {
    return Values[Values.size()/2];
  }
  return 0.5*(Values[Values.size()/2 - 1] + Values[Values.size()/2]);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the significance in sigma of the ratio of two Poisson counts CountA/CountB relative to the expected ratio
double MEndToEndTest::RatioSigma(double CountA, double CountB, double ExpectedRatio)
{
  // Var(A/B) ~ (A/B)^2 * (1/A + 1/B)
  if (CountA <= 0 || CountB <= 0) {
    const double NoSignificance = 1e9; // sigma: the ratio is undefined
    return NoSignificance;
  }
  return fabs(CountA/CountB - ExpectedRatio)/(CountA/CountB*sqrt(1.0/CountA + 1.0/CountB));
}


////////////////////////////////////////////////////////////////////////////////


//! Return the two-proportion z value of the efficiencies K1/N1 and K2/N2
double MEndToEndTest::ProportionSigma(double K1, double N1, double K2, double N2)
{
  const double Proportion = (K1 + K2)/(N1 + N2);
  return fabs(K1/N1 - K2/N2)/sqrt(Proportion*(1.0 - Proportion)*(1.0/N1 + 1.0/N2));
}


// MEndToEndTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
