/*
 * MTestDriver.h
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

#ifndef __MTestDriver__
#define __MTestDriver__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
#include <memory>
#include <set>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MSettingsTesting.h"

// Forward declarations:
class MEndToEndCampaign;
class MEndToEndCampaignAnalysis;
class MManagedProcess;


////////////////////////////////////////////////////////////////////////////////


//! The status of a test
enum class MTestStatus {
  c_Pending = 0,  //!< not yet started
  c_Running,      //!< the test process is running
  c_Failed,       //!< failed checks or unexpected passes, an exit code which contradicts the summary, timed out, or aborted before any check
  c_Crashed,      //!< ended by a signal, or did not end normally
  c_Known,        //!< exited normally and all failed checks are known failures (marked with ExpectedToFail)
  c_Passed        //!< exited normally without any failed check
};


//! Which tests run if no test is requested by name
enum class MTestingScope {
  c_UnitTests = 0,
  c_All,
  c_EndToEnd
};


//! Class representing the driver which finds the test programs (UT*, ET*) in the bin directory and runs them in parallel with a dashboard
//! * By default the unit tests run, --all (-a) adds the end-to-end tests, --endtoend (-e) runs only them, tests named on the command line always run
//! * A test whose failed checks are all known failures (marked with ExpectedToFail) does not fail the run, unless --strict (-s) is given
//! * Before the end-to-end tests it runs testdrivercalibration if the machine is not calibrated yet (or with --calibrate (-c)): the time out scales with it
//! * Every end-to-end test writes summary pages (PDF and the canvases in a ROOT file) into the log directory, the driver puts them with an overview into <driver>_report.pdf
//! * All logs and the working files of failed tests go to /tmp/$USER/megalib_testing_logs, or to the directory of --logdir (-l) for this run only
//! * With --reference-campaign (-r) it runs end-to-end tests many times (--runs, default 50) to make their references: see MEndToEndCampaign
class MTestDriver
{
  // public interface:
 public:
  //! Create a driver with the given name (it is also the name of the program, which is not a test) for tests with the given prefixes
  MTestDriver(const MString& DriverName, const MString& DashboardTitle, const MString& UnitTestPrefix = "UT", const MString& EndToEndTestPrefix = "ET");
  //! Default destructor
  virtual ~MTestDriver();

  //! Execute all discovered tests of the selected scope or the requested subset, return the exit code of the program
  //! Options: see PrintUsage(), everything which is no option is the name of a test
  int Execute(int argc, char** argv);


  // protected methods:
 protected:

  // private methods:
 private:
  // Command line, settings, and the list of tests:
  //! Print the command line options
  void PrintUsage() const;
  //! Create the log directory (for the user only if the driver makes it, with its permissions if it exists), and write it and the time out into the testing settings file, return false on failure
  bool PrepareLogDirectory();
  //! Return the directory with the test programs: the directory of the driver, else $(MEGALIB)/bin, else bin of the working directory
  MString GetBinDirectory(const char* Argv0) const;
  //! Return the name of the file with the run times of the last runs (in the configuration directory of the user)
  MString GetTimingFile() const;
  //! Return the name of the test program which a request means (the prefix is optional), an empty string if there is none
  MString ResolveRequest(const MString& Input) const;
  //! Find the test programs in the bin directory (all executable programs with the prefix of the unit or the end-to-end tests)
  bool DiscoverTests();
  //! Create the list of tests to run: the named tests, or all tests of the scope if none is named, return false if a name is unknown
  bool BuildRequestedTests(const vector<MString>& Names, MTestingScope Scope, vector<MString>& RequestedTests) const;
  //! Sort the tests: the slowest of the last run first
  void SortRequestedTests(vector<MString>& RequestedTests, const map<MString, double>& Timings) const;
  //! Read the run times of the last runs from the timing file
  void LoadTimings(map<MString, double>& Timings) const;
  //! Write the run times to the timing file
  void SaveTimings(const map<MString, double>& Timings) const;


  // Calibration of the speed of this machine:

  //! Return the path of the calibration program: it measures the speed of this machine, the time out of all tests depends on its result
  MString GetCalibrationProgram() const { return m_BinDirectory + "/testdrivercalibration"; }
  //! Create the identifier of this machine and this MEGAlib version: a hash of the CPU model, the number of cores, and the version
  MString CreateMachineId() const;
  //! Return true if the calibration in the settings was made on this machine with this version of MEGAlib
  bool IsSystemCalibrated(const MSettingsTesting& Settings) const;
  //! Return the time out in seconds adapted to the speed of this machine: the time out times the slowdown of the calibration, if it is current (0: no time out)
  double GetScaledTimeout(const MSettingsTesting& Settings) const;
  //! Run the calibration program, wait for it, and store the identifier of this machine and the scaled time out in the settings file, return false if one of that failed
  bool Calibrate() const;


  // Running the tests:

  //! Remove the working files of failed tests of an earlier run for the tests which run now, but not those of tests which are running
  void RemoveStaleTestFiles(const vector<MString>& Tests) const;


  // The results:

  //! Return the status of an ended test and its metric, from its exit status and its output
  MTestStatus ClassifyResult(const MManagedProcess& Process, const MString& Output, double Timeout, MString& Metric) const;
  //! Return the counters of the output of a test ("done" if it has none): passed, failed, known failures, and unexpected passes if there are any
  MString ExtractMetric(const MString& Output) const;
  //! Return the metric in the short form of the dashboard: "F/P failed", the known failures, and the detail of the metric
  MString FormatFailureMetric(const MString& Metric) const;
  //! Return the label of a status of five characters: "PASS ", "KNOWN", "FAIL ", "CRASH", or blanks if the test is not done
  MString GetStatusLabel(MTestStatus Status) const;
  //! Count the lines of the output which begin with the prefix
  unsigned int CountLines(const MString& Output, const MString& Prefix) const;
  //! Return the FAILED, UNEXPECTED PASS, and ABORTED reports of the output of a test, each with its indented detail lines
  vector<MString> ExtractFailureReports(const MString& Output) const;
  //! Return the last lines of the output of a test, e.g. to show the cause of a crash
  vector<MString> ExtractLastLines(const MString& Output) const;
  //! Return the time in seconds with one decimal and the unit
  MString FormatRuntime(double Seconds) const;
  //! Write the summary report: the overview of the tests and the summary pages of the end-to-end tests as one PDF in the log directory, return its file name (empty if no test has summary pages)
  MString WriteSummaryReport(const vector<MString>& RequestedTests, const vector<MTestStatus>& Statuses, const vector<MString>& Metrics, const map<MString, double>& Timings) const;
  //! Run the reference campaign for the tests (the programs which are named, or all end-to-end tests with references): the runs, the analysis, and with --install the references, return the exit code of the program
  int RunReferenceCampaign(const vector<MString>& Names);
  //! Fill the tests of the reference campaign: the end-to-end tests which are named (once), or all with references, return false if a name is no end-to-end test or there are none
  bool GetCampaignTests(const vector<MString>& Names, vector<MString>& Tests) const;
  //! Tell about the analysis of a test of the campaign: the runs, the findings (the first warnings), and the files
  void ReportCampaign(const MString& Test, const MEndToEndCampaignAnalysis& Analysis, const MEndToEndCampaign& Campaign) const;
  //! Write the report with the output of the failed tests into the log directory, return its file name (empty if it could not be written)
  MString WriteFailureReport(const vector<MString>& RequestedTests, const vector<MTestStatus>& Statuses,
                             const vector<MString>& Metrics, const vector<MString>& Outputs) const;


  // protected members:
 protected:

  // private members:
 private:
  //! The name of the driver (and of the program)
  MString m_DriverName;
  //! The title of the dashboard
  MString m_DashboardTitle;
  //! The prefix of the unit tests
  MString m_UnitTestPrefix;
  //! The prefix of the end-to-end tests
  MString m_EndToEndTestPrefix;
  //! The directory with the test programs
  MString m_BinDirectory;
  //! The file with the cache of the run times
  MString m_TimingFile;
  //! The directory for all logs and the working files of failed tests
  MString m_LogDirectory;
  //! All discovered tests: name -> path
  map<MString, MString> m_AllTests;
  //! Kill a test after this many seconds on the reference machine (0: no timeout)
  double m_TimeoutSeconds;
  //! True if the calibration of this machine is made again
  bool m_Calibrate;
  //! True if tests with known failures fail the run (--strict)
  bool m_Strict;
  //! True if the reference campaign runs instead of the tests
  bool m_ReferenceCampaign;
  //! The number of runs of every test of the reference campaign
  unsigned int m_CampaignRuns;
  //! The smallest number of runs which makes references
  unsigned int m_CampaignMinimumRuns;
  //! The base seed of the reference campaign
  unsigned int m_CampaignSeed;
  //! The directory of the reference campaign (empty: campaign in the log directory)
  MString m_CampaignDirectory;
  //! True if the references of the campaign are installed
  bool m_CampaignInstall;
  //! The reason which is written into the record of the references
  MString m_CampaignReason;
  //! The directory into which the references are installed (empty: $(MEGALIB)/resource/endtoend)
  MString m_ReferencesDirectory;
  //! The open lock file next to the testing settings file: only one driver at a time per user (-1: none)
  int m_DriverLock;

};

#endif


////////////////////////////////////////////////////////////////////////////////
