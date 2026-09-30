/*
 * UTCActivatorAllNuclides.cc
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


// Cosima:
#include "MCActivator.hh"
#include "MCIsotopeStore.hh"

// MEGAlib:
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4EnvironmentUtils.hh"
#include "G4GenericIon.hh"
#include "G4IonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4NuclideTable.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

// ROOT:
#include "TRandom.h"

// Standard lib:
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <thread>
#include <vector>
using namespace std;

// POSIX - C++ has no process control; separate processes isolate crashes and hangs, and Geant4 is not thread safe here:
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>


//! Unit test class running MCActivator for all nuclides and levels with Geant4 radioactive decay data
class UTCActivatorAllNuclides : public MUnitTest
{
public:
  //! Default constructor
  UTCActivatorAllNuclides() : MUnitTest("UTCActivatorAllNuclides"), m_TimeLimit(300), m_MaximumZ(200) {}
  //! Default destructor
  virtual ~UTCActivatorAllNuclides() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One nuclide or level to test
  struct Nuclide {
    //! The isotope ID (1000*Z+A)
    int m_ID;
    //! The excitation (keV)
    double m_Excitation;
  };

  //! The result of one nuclide
  struct Result {
    //! Flag indicating if the nuclide has been processed
    bool m_Done = false;
    //! Flag indicating if Geant4 cannot create the level
    bool m_Skipped = false;
    //! Flag indicating a crash
    bool m_Crashed = false;
    //! Flag indicating a time out
    bool m_TimedOut = false;
    //! Flag indicating if all activator calls succeeded
    bool m_OK = false;
    //! Flag indicating NaN, infinite, or negative activities
    bool m_Bad = false;
    //! The maximum activity relative to the production rate
    double m_MaximumActivity = 0;
    //! Number of "branching ratio not close to one" messages
    unsigned int m_TotalBranchingErrors = 0;
    //! Number of "branching ratio larger than one" messages
    unsigned int m_LargerThanOneErrors = 0;
    //! Number of "activation calculation failed" messages
    unsigned int m_FailedErrors = 0;
  };

  //! Initialize the Geant4 particles and the nuclide table as done in cosima
  void InitializeGeant4();
  //! Create the list of all nuclides and levels in the radioactive decay data
  bool CreateNuclideList();
  //! Start a worker for the given slice, beginning at the given position within the slice
  pid_t StartWorker(unsigned int Worker, unsigned int Position);
  //! Run the activator for all nuclides of a slice - never returns
  void RunWorker(unsigned int Worker, unsigned int Position);
  //! Run all workers and supervise them for crashes and time outs
  bool RunAllWorkers();
  //! Read the results written by the workers
  void ReadResults();
  //! Evaluate the results of all nuclides
  bool EvaluateResults();

  //! Return the name of a nuclide for the test output
  MString GetName(const Nuclide& N) const;

  //! The time limit per nuclide (s)
  unsigned int m_TimeLimit;
  //! The maximum atomic number to test
  int m_MaximumZ;
  //! The number of workers
  unsigned int m_NWorkers;
  //! List of all nuclides
  vector<Nuclide> m_Nuclides;
  //! The results of all nuclides
  vector<Result> m_Results;
};


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorAllNuclides::Run()
{
  bool Passed = true;

  InitializeGeant4();

  Passed = EvaluateTrue("CreateNuclideList()", "RadioactiveDecay data", "The nuclide list can be created from the Geant4 radioactive decay data", CreateNuclideList()) && Passed;
  Passed = EvaluateTrue("CreateNuclideList()", "RadioactiveDecay data", "The nuclide list is not empty", m_Nuclides.size() > 0) && Passed;
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "workers", "The temporary directory for the workers can be created", PrepareTemporaryDirectory("Workers")) && Passed;

  if (Passed == true) {
    Passed = EvaluateTrue("RunAllWorkers()", MString(m_NWorkers) + " workers", "All workers finish", RunAllWorkers()) && Passed;
    ReadResults();
    Passed = EvaluateResults() && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTCActivatorAllNuclides::InitializeGeant4()
{
  // Silence Geant4 during the initialization:
  ostringstream Sink;
  streambuf* CoutBuffer = cout.rdbuf(Sink.rdbuf());
  DisableDefaultStreams();

  G4BosonConstructor().ConstructParticle();
  G4LeptonConstructor().ConstructParticle();
  G4BaryonConstructor().ConstructParticle();
  G4IonConstructor().ConstructParticle();

  // The generic ion needs a process manager before ions can be created:
  G4ParticleDefinition* GenericIon = G4GenericIon::Definition();
  GenericIon->SetProcessManager(new G4ProcessManager(GenericIon));
  G4ParticleTable::GetParticleTable()->SetReadiness();

  // Same settings as in the cosima physics list:
  G4NuclideTable::GetInstance()->SetLevelTolerance(100*eV);
  G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0.0001*ns);

  EnableDefaultStreams();
  cout.rdbuf(CoutBuffer);
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorAllNuclides::CreateNuclideList()
{
  const char* Directory = G4FindDataDir("G4RADIOACTIVEDATA");
  if (Directory == nullptr) return false;

  // Each data file z<Z>.a<A> contains one "P <excitation> ..." line per level with decay data:
  regex FileName("z([0-9]+)\\.a([0-9]+)");
  for (const auto& Entry: filesystem::directory_iterator(Directory)) {
    smatch Match;
    string Name = Entry.path().filename().string();
    if (regex_match(Name, Match, FileName) == false) continue;
    int Z = stoi(Match[1]);
    int A = stoi(Match[2]);
    if (Z > m_MaximumZ) continue;

    ifstream In(Entry.path());
    if (In.is_open() == false) return false;
    string Line;
    while (getline(In, Line)) {
      istringstream Tokens(Line);
      string Type;
      double Excitation;
      if ((Tokens>>Type>>Excitation) && Type == "P") {
        m_Nuclides.push_back({ 1000*Z + A, Excitation });
      }
    }
  }

  // Sort for a reproducible order:
  sort(m_Nuclides.begin(), m_Nuclides.end(), [](const Nuclide& A, const Nuclide& B) {
    return A.m_ID != B.m_ID ? A.m_ID < B.m_ID : A.m_Excitation < B.m_Excitation;
  });
  m_Results.resize(m_Nuclides.size());

  m_NWorkers = max(1U, thread::hardware_concurrency());

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString UTCActivatorAllNuclides::GetName(const Nuclide& N) const
{
  return MString(N.m_ID) + " [" + N.m_Excitation + " keV]";
}


////////////////////////////////////////////////////////////////////////////////


pid_t UTCActivatorAllNuclides::StartWorker(unsigned int Worker, unsigned int Position)
{
  pid_t PID = fork();
  if (PID == 0) {
    RunWorker(Worker, Position);
  }
  return PID;
}


////////////////////////////////////////////////////////////////////////////////


void UTCActivatorAllNuclides::RunWorker(unsigned int Worker, unsigned int Position)
{
  // Worker w handles the nuclides w, w + NWorkers, w + 2 NWorkers, ...
  const MString Directory = GetTemporaryDirectoryName("Workers");
  const MString ProgressFile = Directory + "/Progress" + Worker + ".txt";
  const MString ResultsFile = Directory + "/Results" + Worker + ".txt";
  const MString CountsFile = Directory + "/Counts" + Worker + ".dat";
  const MString OutputFile = Directory + "/Output" + Worker + ".act";

  // The activator prints a lot - capture it to search for error messages:
  ostringstream Sink;
  cout.rdbuf(Sink.rdbuf());
  cerr.rdbuf(Sink.rdbuf());
  DisableDefaultStreams();

  ofstream Results(ResultsFile.Data(), ios::app);

  for (unsigned int n = Worker + Position*m_NWorkers; n < m_Nuclides.size(); n += m_NWorkers, ++Position) {
    // Tell the supervisor which nuclide we are working on:
    {
      ofstream Progress(ProgressFile.Data(), ios::trunc);
      Progress<<Position<<endl;
    }

    const Nuclide& N = m_Nuclides[n];

    // Levels Geant4 cannot create never appear in counts files:
    if (MCIsotopeStore::GetParticleDefinition(N.m_ID, N.m_Excitation*keV) == nullptr) {
      Results<<n<<" SKIP"<<endl;
      continue;
    }

    {
      MCIsotopeStore Store;
      Store.SetTime(1*s);
      Store.Add("Volume", N.m_ID, N.m_Excitation*keV, 1.0);
      Store.Save(CountsFile);
    }

    // Reproducible random numbers for the partial simulation:
    gRandom->SetSeed(n + 1);
    CLHEP::HepRandom::setTheSeed(n + 1);

    // Production rate 1/s, long constant irradiation:
    Sink.str("");
    MCActivator A;
    A.SetConstantIrradiation(1e5*s);
    bool OK = A.AddCountsFile(CountsFile);
    OK = A.LoadCountsFiles() && OK;
    OK = A.CalculateEquilibriumRates() && OK;
    A.SetOutputFileName(OutputFile);
    OK = A.SaveOutputFile() && OK;
    string Log = Sink.str();

    // Check the stored activities:
    MCIsotopeStore Out;
    Out.Load(OutputFile);
    double Maximum = 0.0;
    bool Bad = (Log.find("nan") != string::npos || Log.find("inf Bq") != string::npos);
    for (unsigned int v = 0; v < Out.GetNVolumes(); ++v) {
      for (unsigned int i = 0; i < Out.GetNIDs(v); ++i) {
        for (unsigned int e = 0; e < Out.GetNExcitations(v, i); ++e) {
          double Value = Out.GetValue(v, i, e);
          if (std::isfinite(Value) == false || Value < 0) Bad = true;
          if (Value > Maximum) Maximum = Value;
        }
      }
    }

    auto Count = [&Log](const string& Text) {
      unsigned int C = 0;
      for (size_t Pos = Log.find(Text); Pos != string::npos; Pos = Log.find(Text, Pos + 1)) ++C;
      return C;
    };

    Results<<n<<" DONE "<<(OK == true ? 1 : 0)<<" "<<(Bad == true ? 1 : 0)<<" "<<Maximum<<" "
           <<Count("not close to one")<<" "<<Count("larger than one")<<" "<<Count("failed utterly")<<endl;
  }

  // Do not run any destructor of the parent's objects, e.g. the one removing the temporary directory:
  _exit(0);
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorAllNuclides::RunAllWorkers()
{
  const MString Directory = GetTemporaryDirectoryName("Workers");

  // Supervision data per worker:
  vector<pid_t> PIDs(m_NWorkers, 0);
  vector<string> LastProgress(m_NWorkers, "");
  vector<chrono::steady_clock::time_point> LastChange(m_NWorkers, chrono::steady_clock::now());

  for (unsigned int w = 0; w < m_NWorkers; ++w) {
    PIDs[w] = StartWorker(w, 0);
    if (PIDs[w] < 0) return false;
  }

  unsigned int Running = m_NWorkers;
  while (Running > 0) {
    this_thread::sleep_for(chrono::milliseconds(200));

    for (unsigned int w = 0; w < m_NWorkers; ++w) {
      if (PIDs[w] == 0) continue;

      // Read the current position of the worker:
      string Progress;
      {
        ifstream In((Directory + "/Progress" + w + ".txt").Data());
        getline(In, Progress);
      }
      if (Progress != LastProgress[w]) {
        LastProgress[w] = Progress;
        LastChange[w] = chrono::steady_clock::now();
      }
      unsigned int Position = (Progress.empty() == true) ? 0 : stoi(Progress);
      unsigned int n = w + Position*m_NWorkers;

      int Status = 0;
      pid_t Done = waitpid(PIDs[w], &Status, WNOHANG);
      bool Finished = (Done == PIDs[w]);
      bool TimedOut = (Finished == false && chrono::steady_clock::now() - LastChange[w] > chrono::seconds(m_TimeLimit));

      if (Finished == true && WIFEXITED(Status) == true && WEXITSTATUS(Status) == 0) {
        PIDs[w] = 0;
        --Running;
        continue;
      }

      if (Finished == true || TimedOut == true) {
        if (TimedOut == true) {
          kill(PIDs[w], SIGKILL);
          waitpid(PIDs[w], &Status, 0);
          m_Results[n].m_TimedOut = true;
        } else {
          m_Results[n].m_Crashed = true;
        }

        // Continue with the next nuclide of this slice:
        LastProgress[w] = "";
        LastChange[w] = chrono::steady_clock::now();
        if (n + m_NWorkers < m_Nuclides.size()) {
          PIDs[w] = StartWorker(w, Position + 1);
          if (PIDs[w] < 0) return false;
        } else {
          PIDs[w] = 0;
          --Running;
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void UTCActivatorAllNuclides::ReadResults()
{
  const MString Directory = GetTemporaryDirectoryName("Workers");

  for (unsigned int w = 0; w < m_NWorkers; ++w) {
    ifstream In((Directory + "/Results" + w + ".txt").Data());
    string Line;
    while (getline(In, Line)) {
      istringstream Tokens(Line);
      unsigned int n;
      string Type;
      if (!(Tokens>>n>>Type) || n >= m_Results.size()) continue;
      Result& R = m_Results[n];
      R.m_Done = true;
      if (Type == "SKIP") {
        R.m_Skipped = true;
      } else {
        int OK, Bad;
        Tokens>>OK>>Bad>>R.m_MaximumActivity>>R.m_TotalBranchingErrors>>R.m_LargerThanOneErrors>>R.m_FailedErrors;
        R.m_OK = (OK == 1);
        R.m_Bad = (Bad == 1);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorAllNuclides::EvaluateResults()
{
  bool Passed = true;

  unsigned int NTested = 0;
  for (unsigned int n = 0; n < m_Nuclides.size(); ++n) {
    const Result& R = m_Results[n];
    if (R.m_Skipped == true) continue;
    ++NTested;
    MString Name = GetName(m_Nuclides[n]);

    Passed = EvaluateFalse("CalculateEquilibriumRates()", Name, "The activation does not crash", R.m_Crashed) && Passed;
    Passed = EvaluateFalse("CalculateEquilibriumRates()", Name, MString("The activation finishes within ") + m_TimeLimit + " s", R.m_TimedOut) && Passed;
    if (R.m_Crashed == true || R.m_TimedOut == true) continue;

    Passed = EvaluateTrue("CalculateEquilibriumRates()", Name, "The nuclide has been processed", R.m_Done) && Passed;
    Passed = EvaluateTrue("CalculateEquilibriumRates()", Name, "Loading, calculating, and saving succeed", R.m_OK) && Passed;
    Passed = EvaluateFalse("CalculateEquilibriumRates()", Name, "All activities are finite and not negative", R.m_Bad) && Passed;
    // No activity can exceed the production rate - 3% margin, since the partial simulation has a statistical accuracy of about 0.5%:
    Passed = EvaluateTrue("CalculateEquilibriumRates()", Name, "No activity exceeds the production rate", R.m_MaximumActivity <= 1.03) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", Name, "The branching ratios of all chains add up to one", R.m_TotalBranchingErrors, 0U) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", Name, "No branching ratio is larger than one", R.m_LargerThanOneErrors, 0U) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", Name, "The activation calculation does not fail", R.m_FailedErrors, 0U) && Passed;
  }

  Passed = EvaluateTrue("EvaluateResults()", "all nuclides", "At least one nuclide has been tested", NTested > 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCActivatorAllNuclides Test;
  return Test.Run() == true ? 0 : 1;
}
