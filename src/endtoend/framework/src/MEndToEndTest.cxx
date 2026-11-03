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
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;

// ROOT libs:
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TLatex.h"
#include "TBox.h"
#include "TColor.h"
#include "TLine.h"
#include "TMath.h"
#include "TPaveText.h"
#include "TPad.h"
#include "TError.h"
#include "TStyle.h"

// MEGAlib libs:
#include "MEndToEndFingerprint.h"
#include "MFile.h"
#include "MSettingsTesting.h"
#include "MProcessManager.h"
#include "MSystem.h"


////////////////////////////////////////////////////////////////////////////////


//! Standard constructor giving the name of the test
MEndToEndTest::MEndToEndTest(const MString& Name) : MUnitTest(Name), m_Report(Name)
{
  // Read the time out of the test driver (it is adapted to the speed of this machine), it counts from now
  MSettingsTesting Settings;
  Settings.Read();
  m_Timeout = Settings.GetTimeout();
  m_Report.SetDirectory(Settings.GetLogDirectory());
  m_StartTime = chrono::steady_clock::now();

  // The seeds are random, unless a campaign gives the base seed
  random_device Device;
  m_SeedGenerator.seed(Device());
  const char* BaseSeed = getenv(MEndToEndFingerprint::GetSeedVariable().Data());
  if (BaseSeed != nullptr && MString(BaseSeed).IsEmpty() == false) {
    m_SeedGenerator.seed(static_cast<unsigned int>(strtoul(BaseSeed, nullptr, 10)));
  }

  // Run ROOT without graphics - batch mode first, otherwise ROOT tries to connect to the display
  gROOT->SetBatch(true);
  if (gROOT->GetApplication() == nullptr) {
    new TApplication("ROOT", 0, 0);
  }
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
  // The programs run with a private home directory (they load their default configuration from there, not the one of the user) and without display
  const MString Home = GetTemporaryDirectoryName("home");
  if (MFile::CreateDirectory(Home) == false) {
    return false;
  }
  const int Status = MProcessManager::Run("env", MString("-u DISPLAY HOME=") + MSystem::GetShellQuoted(Home) + " " + MSystem::GetShellQuoted(Executable) + " " + Arguments, "", Directory, Timeout);
  if (WIFEXITED(Status) == false || WEXITSTATUS(Status) != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a random seed (1 .. 2e9) which is printed by the tests: any seed has to pass
unsigned int MEndToEndTest::RandomSeed() const
{
  const unsigned int MaximumSeed = 2000000000;
  return 1 + m_SeedGenerator() % MaximumSeed;
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
  m_ScenarioHashes[Scenario.m_Name] = GetScenarioHash(Scenario);
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


//! Return the hash of the definition of a scenario: the name, the source and run lines, the time or the triggers, and the files (not the seed and the directory)
MString MEndToEndTest::GetScenarioHash(const ETSimScenario& Scenario) const
{
  // Everything which the written source and the files depend on, in a fixed order
  MString Definition = Scenario.m_Name + "\n";
  for (const MString& Line: Scenario.m_SourceLines) {
    Definition += "S " + Line + "\n";
  }
  for (const MString& Line: Scenario.m_RunLines) {
    Definition += "R " + Line + "\n";
  }
  Definition += MString("T ") + Scenario.m_Time + " " + Scenario.m_Triggers + "\n";
  for (const pair<MString, MString>& File: Scenario.m_Files) {
    Definition += "F " + File.first + "\n" + File.second + "\n";
  }
  return MEndToEndFingerprint::GetHash(Definition);
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
vector<MString> MEndToEndTest::ReadLines(const MString& FileName) const
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


//! Return the angular resolution measure (ARM) in degree of a Compton event relative to the unit vector to the source
double MEndToEndTest::ARM(const MComptonEvent& Event, const MVector& ToSource) const
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


//! Measure a statistic of the values together with its bootstrap uncertainty
ETMeasuredValue MEndToEndTest::Measure(const vector<double>& Values, const MStatisticFunction& Statistic, unsigned int NResamples)
{
  ETMeasuredValue Measured;
  Measured.m_Value = Statistic(Values);
  Measured.m_Error = MStatistics::GetBootstrapError(Values, Statistic, NResamples);
  return Measured;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the largest y value of the graph, zero if it has no points
double MEndToEndTest::GetMaximumY(const TGraph* Graph)
{
  // TMath::MaxElement reads beyond the end of an empty graph
  if (Graph->GetN() == 0) {
    return 0.0;
  }
  return TMath::MaxElement(Graph->GetN(), Graph->GetY());
}


////////////////////////////////////////////////////////////////////////////////


//! Check that the measured value is the expected value within the tolerance, add the comparison with the outcome of this check to the key results under the name, return the outcome
bool MEndToEndTest::CompareValues(const MString& Function, const MString& Input, const MString& Description, const MString& Name, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  // A measurement, an expectation, or a tolerance which is not valid (not finite, uncertainties negative) fails the test and is not recorded as a key result
  if (EvaluateTrue(Function, Input, "The measurement and the expectation are valid (finite, uncertainties not negative)", Measured.IsValid() == true && Expected.IsValid() == true && isfinite(Tolerance) == true) == false) {
    return false;
  }

  MEndToEndResult& Result = m_Results.AddComparison(Name, Measured, Expected, Tolerance, IsStatistical);
  Result.SetPassed(EvaluateNear(Function, Input, Description, Measured.m_Value, Expected.m_Value, Tolerance));
  return Result.IsPassed();
}


////////////////////////////////////////////////////////////////////////////////


//! Load the references of this test from the default file: $(MEGALIB)/resource/endtoend/<name of the test>.references.tsv, return false if it cannot be read
bool MEndToEndTest::LoadReferences()
{
  return LoadReferences(MEndToEndReferenceList::GetFileName(GetName()));
}


////////////////////////////////////////////////////////////////////////////////


//! Load the references from the file, return false if it cannot be read
bool MEndToEndTest::LoadReferences(const MString& FileName)
{
  m_ReferenceFile = FileName;
  m_ReferencesChecked = false;
  return m_References.Read(FileName);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the environment of this run which the references depend on: Geant4, ROOT, the geometry, and the scenarios (keys of MEndToEndFingerprint::GetKeys())
//! The geometry is read once per geometry file: it does not change during a test
map<MString, MString> MEndToEndTest::GetFingerprint() const
{
  map<MString, MString> Fingerprint;
  Fingerprint["geant4"] = MEndToEndReport::ReadConfiguration("GEANT4");
  Fingerprint["root"] = gROOT->GetVersion();

  // The geometry is the hash of its file and of the files which it includes: from the geometry of the reader if it is loaded, else read once
  if (m_GeometryHash.IsEmpty() == true) {
    m_GeometryHash = "unknown";
    if (m_ReaderGeometry != nullptr && m_ReaderGeometry->GetHash() != 0) {
      m_GeometryHash = MString(m_ReaderGeometry->GetHash());
    } else if (MFile::Exists(GetGeometry()) == true) {
      m_GeometryHash = MEndToEndFingerprint::GetFilesHash(GetGeometry());
    }
  }
  Fingerprint["geometry"] = m_GeometryHash;

  // The scenarios are the hash of the hashes of the prepared scenarios, sorted by name
  MString Scenarios;
  for (const pair<const MString, MString>& Entry: m_ScenarioHashes) {
    Scenarios += Entry.first + " " + Entry.second + "\n";
  }
  Fingerprint["scenarios"] = MEndToEndFingerprint::GetHash(Scenarios);
  return Fingerprint;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the differences between the record of the references and the environment of this run, empty if they match (a difference is a warning, not a failure)
vector<MString> MEndToEndTest::CheckReferences() const
{
  return MEndToEndFingerprint::Compare(m_References.GetHeader(), GetFingerprint());
}


////////////////////////////////////////////////////////////////////////////////


//! Write the key results as tab separated text into the directory of the summary, if the test has key results
void MEndToEndTest::WriteResults() const
{
  if (m_Report.GetDirectory().IsEmpty() == true || m_Results.IsEmpty() == true) {
    return;
  }
  m_Results.Write(m_Report.GetDirectory() + "/" + GetName() + ".keyresults.tsv");
}


////////////////////////////////////////////////////////////////////////////////


//! Write the environment of this run (the fingerprint) as file with a header only into the directory of the summary, if the test has key results
void MEndToEndTest::WriteFingerprint() const
{
  if (m_Report.GetDirectory().IsEmpty() == true || m_Results.IsEmpty() == true) {
    return;
  }
  MEndToEndRecord Environment;
  Environment.Set("test", GetName());
  for (const pair<const MString, MString>& Entry: GetFingerprint()) {
    Environment.Set(Entry.first, Entry.second);
  }
  Environment.Write(m_Report.GetDirectory() + "/" + GetName() + ".fingerprint.tsv", "MEGAlib end-to-end fingerprint of a run");
}


////////////////////////////////////////////////////////////////////////////////


//! Tell if the references were made in another environment: at the end of the test, when every scenario is prepared and the fingerprint is the one which is written
void MEndToEndTest::ReportReferenceEnvironment()
{
  if (m_ReferencesChecked == false || m_ReferenceFile.IsEmpty() == true) {
    return;
  }
  for (const MString& Difference: CheckReferences()) {
    mout<<"The references were made in another environment - "<<Difference<<endl;
    m_Report.AddTitleLine(MString("The references were made in another environment - ") + Difference);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Compare a measured value with its reference: inside five combined standard errors, prints a REFERENCE line
bool MEndToEndTest::CompareToReference(const MString& Scenario, const MString& Observable, const MString& Description, const ETMeasuredValue& Measured)
{
  // The first comparison names the references on the title page - the environment is checked at the end of the test, when every scenario is prepared
  if (m_ReferencesChecked == false) {
    m_ReferencesChecked = true;
    if (m_ReferenceFile.IsEmpty() == false) {
      m_Report.AddTitleLine(MString("References: ") + MFile::GetBaseName(m_ReferenceFile) + ", " + m_References.GetHeader().Get("runs") + " runs, " + m_References.GetHeader().Get("date"));
    }
  }

  const MString Key = Scenario + "/" + Observable;
  mout<<"REFERENCE "<<GetName()<<"/"<<Key<<" "<<Measured.m_Value<<" "<<Measured.m_Error<<endl;

  // A measurement which is not valid fails the test, in a reference campaign too, and is not recorded as a key result
  if (EvaluateTrue("Reference", Scenario + ", " + Observable, "The measurement is valid (finite, uncertainty not negative)", Measured.IsValid()) == false) {
    return false;
  }

  // A run of a reference campaign makes the new references: the old ones neither pass nor fail it
  const char* Campaign = getenv(MEndToEndFingerprint::GetCampaignVariable().Data());
  if (Campaign != nullptr && MString(Campaign).IsEmpty() == false) {
    m_Results.AddResult(Key, Measured.m_Value, Measured.m_Error).SetFromReferences(true);
    return true;
  }

  if (m_References.HasReference(Key) == false) {
    MEndToEndResult& Missing = m_Results.AddResult(Key, Measured.m_Value, Measured.m_Error);
    Missing.SetPassed(false);
    Missing.SetFromReferences(true);
    return EvaluateTrue("Reference", Scenario + ", " + Observable, "A reference value exists for this observable", false);
  }

  // The bootstrap error of the run is calibrated with the scatter of the runs of the campaign
  const MEndToEndReference Reference = m_References.GetReference(Key);
  ETMeasuredValue Calibrated = Measured;
  if (Reference.GetCalibration() > 0.0) {
    Calibrated.m_Error = Reference.GetCalibration()*Measured.m_Error;
  }
  ETMeasuredValue Expected;
  Expected.m_Value = Reference.GetMean();
  Expected.m_Error = Reference.GetError();
  double Tolerance = MEndToEndResultList::GetStatisticalTolerance(Calibrated, Expected);

  // A reference which is more precise than the changes which matter: the floors are the smallest tolerance
  const double Floor = max(Reference.GetRelativeFloor()*fabs(Reference.GetMean()), Reference.GetAbsoluteFloor());
  if (Floor > Tolerance) {
    Tolerance = Floor;
  }
  const size_t Recorded = m_Results.GetResults().size();
  const bool Passed = CompareValues("Reference", Scenario + ", " + Observable, Description + " (five combined standard errors)", Key, Calibrated, Expected, Tolerance, true);

  // The key result shows the error of the run as it was measured, the campaign needs it - but only if the comparison recorded a result
  if (m_Results.GetResults().size() > Recorded) {
    MEndToEndResult& Result = m_Results.GetLastResult();
    Result.SetError(Measured.m_Error);
    Result.SetFromReferences(true);
  }
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the reference value of "scenario/observable" (zero if there is none)
ETMeasuredValue MEndToEndTest::GetReference(const MString& Scenario, const MString& Observable) const
{
  ETMeasuredValue Reference;
  const MEndToEndReference Stored = m_References.GetReference(Scenario + "/" + Observable);
  Reference.m_Value = Stored.GetMean();
  Reference.m_Error = Stored.GetError();
  return Reference;
}


////////////////////////////////////////////////////////////////////////////////




// MEndToEndTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
