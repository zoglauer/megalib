/*
 * MEndToEndCampaign.h
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

#ifndef __MEndToEndCampaign__
#define __MEndToEndCampaign__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MEndToEndCampaignAnalysis.h"
#include "MSettingsTesting.h"
#include "MString.h"

// Forward declarations:
class MProcessManager;


////////////////////////////////////////////////////////////////////////////////


//! One run of a test in a campaign
class ETCampaignJob
{
public:
  //! The name of the test (the program in the bin directory)
  MString m_Test;
  //! The number of the run
  unsigned int m_Run = 0;
  //! The directory of the run
  MString m_Directory;
  //! The base seed of the random seeds of the run
  unsigned int m_Seed = 0;
};


//! How a run ended: the record in the file run.done of its directory
class ETCampaignRecord
{
public:
  //! Read the record of the run in the directory, return false if there is none or it is incomplete
  bool Read(const MString& Directory);
  //! Return the number of failed checks of the run: at least one if the run did not end with the exit code 0 or timed out
  unsigned int GetFailedChecks() const;
  //! Return true if the run ended without a failed check
  bool Succeeded() const { return GetFailedChecks() == 0; }

  //! The number of the failed checks which the run reported, its known failures included
  unsigned int m_ReportedFailedChecks = 0;
  //! The exit code of the run
  int m_ExitCode = -1;
  //! True if the run was stopped because it took too long
  bool m_TimedOut = false;
};


//! Class representing a campaign of independent runs of end-to-end tests which makes references
//! * Every run of a test has its own directory <campaign>/<test>/run_<number> which is also its log directory and its home directory
//! * A campaign always starts over: the directory of every run is emptied before the run starts, nothing of an earlier campaign is reused
//! * The base seed of the run is derived from the base seed of the campaign, the test, and the run: the runs are reproducible
//! * After the runs of a test the analysis (MEndToEndCampaignAnalysis) makes the candidate references and the report, which are installed if there is no error
class MEndToEndCampaign
{
  // public interface:
 public:
  //! Create a campaign for the test programs in the bin directory, the runs and the results are in the campaign directory
  MEndToEndCampaign(const MString& BinDirectory, const MString& CampaignDirectory);
  //! Default destructor
  ~MEndToEndCampaign() = default;

  //! Set the number of runs of every test
  void SetNumberOfRuns(unsigned int NumberOfRuns) { m_NumberOfRuns = NumberOfRuns; }
  //! Set the smallest number of runs which makes references
  void SetMinimumRuns(unsigned int MinimumRuns) { m_MinimumRuns = MinimumRuns; }
  //! Set the number of runs which are made at the same time
  void SetMaximumParallel(unsigned int MaximumParallel) { m_MaximumParallel = MaximumParallel; }
  //! Set the time in seconds after which a run is stopped, zero for no time out
  void SetTimeout(double Timeout) { m_Timeout = Timeout; }
  //! Set the base seed of the campaign
  void SetBaseSeed(unsigned int BaseSeed) { m_BaseSeed = BaseSeed; }
  //! Set the testing settings which the runs get (the log directory is the one of the run)
  void SetSettings(const MSettingsTesting& Settings) { m_Settings = Settings; }
  //! Set the directory into which the references are installed, the default is $(MEGALIB)/resource/endtoend
  void SetReferencesDirectory(const MString& Directory) { m_ReferencesDirectory = Directory; }

  //! Return the directory of a run: <campaign>/<test>/run_<number>
  MString GetRunDirectory(const MString& Test, unsigned int Run) const;
  //! Return the base seed of a run: it depends on the base seed of the campaign, the test, and the run
  unsigned int GetSeed(const MString& Test, unsigned int Run) const;
  //! Return all runs of the tests
  vector<ETCampaignJob> GetJobs(const vector<MString>& Tests) const;

  //! Run the jobs, at most the maximum number at the same time, return false if the campaign was interrupted, a run could not be started or waited for, or the file which says how a run ended could not be written (a run which fails its checks is no failure here, the analysis finds it)
  bool RunJobs(const vector<ETCampaignJob>& Jobs);

  //! Analyze the runs of a test: read all its runs, write the candidate references and the report into the campaign directory, return false if they cannot be written
  //! The reason is the reason in the record of the references
  bool Analyze(const MString& Test, const MString& Reason, MEndToEndCampaignAnalysis& Analysis) const;
  //! Install the references of a test (with the reason in the record) into the references directory, return false if the analysis has errors or the file cannot be written
  bool Install(const MString& Test, const MString& Reason, const MEndToEndCampaignAnalysis& Analysis) const;

  //! Return the name of the file with the candidate references of a test
  MString GetCandidateFileName(const MString& Test) const { return m_CampaignDirectory + "/" + Test + ".references.tsv"; }
  //! Return the name of the file with the report of the analysis of a test
  MString GetReportFileName(const MString& Test) const { return m_CampaignDirectory + "/" + Test + ".campaign.pdf"; }

  // private methods:
 private:
  //! Return true if the directory of the job is <campaign>/<test>/run_<number> and neither the test directory nor the run directory is a symbolic link: only such a directory may be emptied
  bool IsRunDirectory(const ETCampaignJob& Job) const;
  //! Prepare the directory of a run (empty, with the settings file) and start the test with the manager, return the id of the manager or -1
  int StartJob(const ETCampaignJob& Job, MProcessManager& Manager) const;
  //! Make the references of a test from its analysis: the record has the reason and the base seed of the campaign
  MEndToEndReferenceList MakeReferences(const MString& Test, const MString& Reason, const MEndToEndCampaignAnalysis& Analysis) const;
  //! Read how a run ended, write its record, and tell about it: the number of finished runs includes this one, return false if the record cannot be written
  bool FinishJob(const ETCampaignJob& Job, int ProcessStatus, double Seconds, bool TimedOut, unsigned int Finished, unsigned int Total) const;
  //! Write the file which says how a run ended, return false if it cannot be written
  bool WriteRunDone(const ETCampaignJob& Job, unsigned int FailedChecks, int ExitCode, bool TimedOut, double Seconds) const;

  // private members:
 private:
  //! The directory with the test programs
  MString m_BinDirectory;
  //! The directory of the campaign
  MString m_CampaignDirectory;
  //! The directory into which the references are installed
  MString m_ReferencesDirectory;
  //! The number of runs of every test
  unsigned int m_NumberOfRuns;
  //! The smallest number of runs which makes references
  unsigned int m_MinimumRuns;
  //! The number of runs which are made at the same time
  unsigned int m_MaximumParallel;
  //! The time in seconds after which a run is stopped, zero for no time out
  double m_Timeout;
  //! The base seed of the campaign
  unsigned int m_BaseSeed;
  //! The testing settings which the runs get
  MSettingsTesting m_Settings;
};

#endif


////////////////////////////////////////////////////////////////////////////////
