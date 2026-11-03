/*
 * MEndToEndCampaign.cxx
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
#include "MEndToEndCampaign.h"

// Standard libs:
#include <filesystem>
#include <iomanip>
#include <map>
#include <sstream>
#include <thread>
using namespace std;

// POSIX libs: the wait status of a run and the sleep between the polls have no equivalent in the C++ standard library
#include <sys/wait.h>
#include <unistd.h>

// MEGAlib libs:
#include "MEndToEndFingerprint.h"
#include "MFile.h"
#include "MStreams.h"
#include "MProcessManager.h"
#include "MSystem.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Create a campaign for the test programs in the bin directory, the runs and the results are in the campaign directory
MEndToEndCampaign::MEndToEndCampaign(const MString& BinDirectory, const MString& CampaignDirectory) : m_BinDirectory(BinDirectory), m_ReferencesDirectory("$(MEGALIB)/resource/endtoend"), m_NumberOfRuns(50), m_MinimumRuns(30), m_Timeout(0.0), m_BaseSeed(1)
{
  // Absolute paths: the runs change their working directory
  error_code Error;
  m_BinDirectory = filesystem::absolute(BinDirectory.Data(), Error).lexically_normal().string().c_str();
  m_CampaignDirectory = filesystem::absolute(CampaignDirectory.Data(), Error).lexically_normal().string().c_str();
  m_MaximumParallel = thread::hardware_concurrency();
  if (m_MaximumParallel == 0) {
    m_MaximumParallel = 1;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the directory of a run: <campaign>/<test>/run_<number>
MString MEndToEndCampaign::GetRunDirectory(const MString& Test, unsigned int Run) const
{
  ostringstream Number;
  Number<<setfill('0')<<setw(3)<<Run;
  return m_CampaignDirectory + "/" + Test + "/run_" + Number.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the base seed of a run: it depends on the base seed of the campaign, the test, and the run
unsigned int MEndToEndCampaign::GetSeed(const MString& Test, unsigned int Run) const
{
  const unsigned long MaximumSeed = 2000000000;
  return 1 + static_cast<unsigned long>(MString(Test + "/" + Run + "/" + m_BaseSeed).GetHash()) % MaximumSeed;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the record of the run in the directory, return false if there is none or it is incomplete
bool ETCampaignRecord::Read(const MString& Directory)
{
  MEndToEndRecord Done;
  if (MFile::Exists(Directory + "/run.done") == false || Done.Read(Directory + "/run.done") == false) {
    return false;
  }
  if (Done.Get("failed_checks").IsEmpty() == true || Done.Get("exit_code").IsEmpty() == true || Done.Get("timed_out").IsEmpty() == true) {
    return false;
  }
  m_ReportedFailedChecks = static_cast<unsigned int>(Done.Get("failed_checks").ToInt());
  m_ExitCode = Done.Get("exit_code").ToInt();
  m_TimedOut = (Done.Get("timed_out") != "0");
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of failed checks of the run: at least one if the run did not end with the exit code 0 or timed out
unsigned int ETCampaignRecord::GetFailedChecks() const
{
  if (m_ExitCode != 0 || m_TimedOut == true) {
    return max(m_ReportedFailedChecks, 1U);
  }
  return m_ReportedFailedChecks;
}


////////////////////////////////////////////////////////////////////////////////


//! Return all runs of the tests
vector<ETCampaignJob> MEndToEndCampaign::GetJobs(const vector<MString>& Tests) const
{
  vector<ETCampaignJob> Jobs;
  for (const MString& Test: Tests) {
    for (unsigned int r = 0; r < m_NumberOfRuns; ++r) {
      ETCampaignJob Job;
      Job.m_Test = Test;
      Job.m_Run = r;
      Job.m_Directory = GetRunDirectory(Test, r);
      Job.m_Seed = GetSeed(Test, r);
      Jobs.push_back(Job);
    }
  }
  return Jobs;
}


////////////////////////////////////////////////////////////////////////////////


//! Run the jobs, at most the maximum number at the same time, return false if the campaign was interrupted, a run could not be started or waited for, or the file which says how a run ended could not be written (a run which fails its checks is no failure here, the analysis finds it)
bool MEndToEndCampaign::RunJobs(const vector<ETCampaignJob>& Jobs)
{
  const double PollSeconds = 0.2;

  // The runs do not receive Ctrl-C: the manager stops them if this program is interrupted
  MProcessManager::ClearInterrupt();
  MProcessManager::InstallInterruptHandlers();
  MProcessManager Manager(m_MaximumParallel);

  bool Success = true;
  map<unsigned int, unsigned int> JobOfId;
  unsigned int Next = 0;
  unsigned int Finished = 0;
  while (Next < Jobs.size() || Manager.HasRunning() == true) {
    if (MProcessManager::WasInterrupted() == true) {
      Manager.StopAll();
      mout<<"Interrupted - all running runs have been stopped"<<endl;
      Success = false;
      break;
    }

    // Start runs until the maximum number is running
    while (Next < Jobs.size() && Manager.CanStart() == true) {
      const int Id = StartJob(Jobs[Next], Manager);
      if (Id < 0) {
        merr<<"Unable to start the run "<<Jobs[Next].m_Run<<" of "<<Jobs[Next].m_Test<<show;
        Success = false;
      } else {
        JobOfId[Id] = Next;
      }
      ++Next;
    }
    if (Manager.HasRunning() == false) {
      continue;
    }

    // The runs which ended get their record, the ones which take too long are stopped by the manager
    const bool Waited = Manager.Poll();
    for (const MManagedProcess& Process: Manager.TakeEnded()) {
      const ETCampaignJob& Job = Jobs[JobOfId[Process.m_Id]];
      if (Process.m_Lost == true) {
        merr<<"The run "<<Job.m_Run<<" of "<<Job.m_Test<<" is lost: it cannot be waited for"<<show;
        continue;
      }
      if (FinishJob(Job, Process.m_Status, Process.m_Seconds, Process.m_TimedOut, ++Finished, Jobs.size()) == false) {
        Success = false;
      }
    }
    if (Waited == false) {
      // A run which cannot be waited for (reaped by someone else, e.g. a handler of SIGCHLD): the campaign has failed
      Manager.StopAll();
      Success = false;
      break;
    }
    if (Manager.HasRunning() == true) {
      usleep(static_cast<useconds_t>(PollSeconds*1000000.0));
    }
  }

  MProcessManager::RestoreInterruptHandlers();
  return Success;
}


////////////////////////////////////////////////////////////////////////////////


//! Read how a run ended, write its record, and tell about it: the number of finished runs includes this one, return false if the record cannot be written
bool MEndToEndCampaign::FinishJob(const ETCampaignJob& Job, int ProcessStatus, double Seconds, bool TimedOut, unsigned int Finished, unsigned int Total) const
{
  // A run which does not report its checks, or has none, has failed (as for the normal execution of tests)
  MString Output;
  MFile::ReadTextFile(Job.m_Directory + "/" + Job.m_Test + ".log", Output);
  unsigned int PassedChecks = 0;
  unsigned int FailedChecks = 0;
  if (MUnitTest::ParseSummary(Output, PassedChecks, FailedChecks) == false || PassedChecks == 0) {
    FailedChecks = max(FailedChecks, 1U);
  }
  int ExitCode = -1;
  if (WIFEXITED(ProcessStatus) != 0) {
    ExitCode = WEXITSTATUS(ProcessStatus);
  }
  const bool Written = WriteRunDone(Job, FailedChecks, ExitCode, TimedOut, Seconds);
  if (Written == false) {
    merr<<"Unable to write the record of run "<<Job.m_Run<<" of "<<Job.m_Test<<show;
  }

  mout<<"["<<Finished<<"/"<<Total<<"] "<<Job.m_Test<<" run "<<Job.m_Run<<": "<<FailedChecks<<" failed checks, exit code "<<ExitCode<<", "<<fixed<<setprecision(1)<<Seconds<<" s";
  if (TimedOut == true) {
    mout<<", timed out";
  }
  mout<<endl;
  return Written;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze the runs of a test: read all its runs, write the candidate references and the report into the campaign directory, return false if they cannot be written
bool MEndToEndCampaign::Analyze(const MString& Test, const MString& Reason, MEndToEndCampaignAnalysis& Analysis) const
{
  Analysis = MEndToEndCampaignAnalysis();
  Analysis.SetMinimumRuns(m_MinimumRuns);

  // Every run counts: a run without a complete record or without key results is a run with an error
  for (unsigned int r = 0; r < m_NumberOfRuns; ++r) {
    const MString Directory = GetRunDirectory(Test, r);
    ETCampaignRecord Record;
    ETCampaignRun Run;
    if (Record.Read(Directory) == false) {
      merr<<"Run "<<r<<" of "<<Test<<" has no complete record"<<show;
      Run.m_FailedChecks = 1;
      Analysis.AddRun(Run);
      continue;
    }
    const unsigned int FailedChecks = Record.GetFailedChecks();
    if (MEndToEndCampaignAnalysis::ReadRun(Directory, Test, FailedChecks, Run) == false) {
      Run.m_FailedChecks = max(FailedChecks, 1U);
    }
    Analysis.AddRun(Run);
  }
  Analysis.Analyze();

  bool Success = MakeReferences(Test, Reason, Analysis).Write(GetCandidateFileName(Test));
  if (Analysis.WriteReport(Test, GetReportFileName(Test)) == false) {
    Success = false;
  }
  return Success;
}


////////////////////////////////////////////////////////////////////////////////


//! Make the references of a test from its analysis: the record has the reason and the base seed of the campaign
MEndToEndReferenceList MEndToEndCampaign::MakeReferences(const MString& Test, const MString& Reason, const MEndToEndCampaignAnalysis& Analysis) const
{
  MEndToEndReferenceList References = Analysis.MakeReferences(Test, Reason);
  References.GetHeader().Set("base_seed", MString(m_BaseSeed));
  return References;
}


////////////////////////////////////////////////////////////////////////////////


//! Install the references of a test (with the reason in the record) into the references directory, return false if the analysis has errors or the file cannot be written
bool MEndToEndCampaign::Install(const MString& Test, const MString& Reason, const MEndToEndCampaignAnalysis& Analysis) const
{
  if (Analysis.HasErrors() == true) {
    merr<<"The campaign of "<<Test<<" has errors: the references are not installed"<<show;
    return false;
  }
  return MakeReferences(Test, Reason, Analysis).Write(m_ReferencesDirectory + "/" + Test + ".references.tsv");
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the directory of the job is <campaign>/<test>/run_<number>, and neither the test directory nor the run directory is a symbolic link
bool MEndToEndCampaign::IsRunDirectory(const ETCampaignJob& Job) const
{
  // Without a campaign directory the run directory would start at the root
  if (m_CampaignDirectory.IsEmpty() == true) {
    return false;
  }

  // The name of the test is one name: no path, no "." or ".."
  if (Job.m_Test.IsEmpty() == true || Job.m_Test == "." || Job.m_Test == ".." || Job.m_Test.Contains("/") == true) {
    return false;
  }

  // The directory is the directory of its run, whatever else the job says
  if (Job.m_Directory != GetRunDirectory(Job.m_Test, Job.m_Run)) {
    return false;
  }

  // Nothing is deleted through a link: the campaign directory is the user's choice, the two directories below it are ours
  error_code Error;
  for (const MString& Directory: { m_CampaignDirectory + "/" + Job.m_Test, Job.m_Directory }) {
    if (filesystem::is_symlink(filesystem::symlink_status(Directory.Data(), Error)) == true) {
      return false;
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Prepare the directory of a run (empty, with the settings file) and start the test with the manager, return the id of the manager or -1
int MEndToEndCampaign::StartJob(const ETCampaignJob& Job, MProcessManager& Manager) const
{
  // Only the directory of this run is emptied, never one outside of the campaign
  if (IsRunDirectory(Job) == false) {
    merr<<"The directory of the run is not <campaign>/<test>/run_<number> in the campaign directory (a link, or no campaign directory?): "<<Job.m_Directory<<show;
    return -1;
  }

  // Every run starts from nothing
  error_code Error;
  filesystem::remove_all(Job.m_Directory.Data(), Error);
  if (Error.value() != 0) {
    merr<<"Unable to remove the old run directory "<<Job.m_Directory<<": "<<Error.message().c_str()<<show;
    return -1;
  }
  if (MFile::CreateDirectory(Job.m_Directory) == false) {
    return -1;
  }

  // The settings of the run: the log directory is the directory of the run, and the run has it as its home
  MSettingsTesting Settings = m_Settings;
  Settings.SetLogDirectory(Job.m_Directory);
  if (Settings.Write(Job.m_Directory + "/.testdrive.cfg") == false) {
    return -1;
  }
  if (MFile::Exists(Job.m_Directory + "/.testdrive.cfg") == false) {
    return -1;
  }

  // The test finds its settings in its home and its base seed in the environment
  const MString Arguments = MString("HOME=") + MSystem::GetShellQuoted(Job.m_Directory) + " " + MEndToEndFingerprint::GetSeedVariable() + "=" + Job.m_Seed + " " + MEndToEndFingerprint::GetCampaignVariable() + "=1 " + MSystem::GetShellQuoted(m_BinDirectory + "/" + Job.m_Test);
  return Manager.Start("env", Arguments, Job.m_Directory + "/" + Job.m_Test + ".log", Job.m_Directory, m_Timeout);
}


////////////////////////////////////////////////////////////////////////////////


//! Write the file which says how a run ended, return false if it cannot be written
bool MEndToEndCampaign::WriteRunDone(const ETCampaignJob& Job, unsigned int FailedChecks, int ExitCode, bool TimedOut, double Seconds) const
{
  MEndToEndRecord Done;
  Done.Set("test", Job.m_Test);
  Done.Set("run", MString(Job.m_Run));
  Done.Set("seed", MString(Job.m_Seed));
  Done.Set("failed_checks", MString(FailedChecks));
  Done.Set("exit_code", MString(ExitCode));
  MString TimedOutFlag = "0";
  if (TimedOut == true) {
    TimedOutFlag = "1";
  }
  Done.Set("timed_out", TimedOutFlag);
  Done.Set("seconds", MString(Seconds, 3U));
  return Done.Write(Job.m_Directory + "/run.done", "MEGAlib end-to-end campaign run");
}


// MEndToEndCampaign.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
