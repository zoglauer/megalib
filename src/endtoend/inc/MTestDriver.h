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
#include <csignal>
#include <map>
#include <set>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MSettingsTesting.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The status of a test
enum class MTestStatus {
  c_Pending = 0,
  c_Running,
  c_Failed,
  c_Passed
};


//! Which tests run if no test is requested by name
enum class MTestingScope {
  c_UnitTests = 0,
  c_All,
  c_EndToEnd
};


//! Class representing the driver which finds the test programs (UT*, ET*) in the bin directory and runs them in parallel with a dashboard
//! * By default the unit tests run, --all (-a) adds the end-to-end tests, --endtoend (-e) runs only them, tests named on the command line always run
//! * Before the end-to-end tests it runs testdrivercalibration if the machine is not calibrated yet (or with --calibrate (-c)): the time out scales with it
//! * All logs and the working files of failed tests go to the log directory of ~/.testdrive.cfg (--logdir (-l))
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
  //! Create the log directory, and write it and the time out into the testing settings file, return false on failure
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
  //! Signal handler for SIGINT and SIGTERM: only flag the interrupt, the main loop stops the tests
  static void HandleInterrupt(int Signal);
  //! Remove the working files of failed tests of an earlier run for the tests which run now, but not those of tests which are running
  void RemoveStaleTestFiles(const vector<MString>& Tests) const;

  // The results:
  //! Return the passed and failed counters of the output of a test ("done" if it has none)
  MString ExtractMetric(const MString& Output) const;
  //! Return the metric in the form which is shown for a failed test
  MString FormatFailureMetric(const MString& Metric) const;
  //! Return the time in seconds with one decimal and the unit
  MString FormatRuntime(double Seconds) const;
  //! Write the report with the output of the failed tests into the log directory, return its file name (empty if it could not be written)
  MString WriteFailureReport(const vector<MString>& RequestedTests, const vector<MTestStatus>& Statuses,
                             const vector<MString>& Metrics, const vector<MString>& Outputs) const;


  // protected members:
 protected:

  // private members:
 private:
  //! Set when the driver receives SIGINT or SIGTERM
  static volatile sig_atomic_t m_Interrupted;
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
  //! The open lock file of the log directory: only one driver at a time can use a log directory (-1: none)
  int m_LogDirectoryLock;

};

#endif


////////////////////////////////////////////////////////////////////////////////
