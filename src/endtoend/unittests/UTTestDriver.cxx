/*
 * UTTestDriver.cxx
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

// Unit test of MTestDriver with fake test programs (shell scripts) in a child process

// Include the header:
#include "MTestDriver.h"
#include "MFile.h"
#include "MSettingsTesting.h"
#include "MUnitTest.h"

// Standard libs:
#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>

// POSIX libs:
#include <fcntl.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;


//! Unit test class for the test driver
class UTTestDriver : public MUnitTest
{
public:
  //! Default constructor
  UTTestDriver() : MUnitTest("UTTestDriver") {}
  //! Default destructor
  virtual ~UTTestDriver() {}

  //! Run all tests
  virtual bool Run();

private:
  //! The result of a run of the driver
  struct Result {
    //! The exit code of the driver, -1 if it did not end normally
    int m_ExitCode = -1;
    //! The log directory of the run
    MString m_LogDirectory;
  };

  //! Write an executable shell script
  bool WriteScript(const MString& Directory, const MString& Name, const MString& Body) const;
  //! Write the fake tests into the (existing) directory: unit tests UTAlpha (passes), UTBeta (fails), UTEnvironment (prints the log directory), UTSlow (sleeps), and
  //! end-to-end tests ETGamma and ETDelta (pass)
  bool CreateFakeTests(const MString& Directory) const;
  //! Run the driver with the arguments in a child process (stdout and stderr discarded), with the given log directory and a private configuration directory
  Result Drive(const MString& BinDirectory, const vector<MString>& Arguments, const MString& LogDirectory) const;
  //! Read a text file
  static MString ReadAll(const MString& FileName);
  //! True if the log directory has a file with this name
  static bool Has(const MString& Directory, const MString& Name) { return filesystem::exists((Directory + "/" + Name).Data()); }
  //! True if the log directory has a failure report
  static bool HasReport(const MString& Directory);

  //! Test which tests run by default, with --all, with --endtoend, and by name
  bool TestScopes();
  //! Test the files of the log directory, and the log directory option
  bool TestLogDirectory();
  //! Test the invalid command lines and the help
  bool TestCommandLine();
  //! Test the time out, and the cache of the run times
  bool TestTimeoutAndTimings();
  //! Test the machine in the testing settings: identifier, slowdown, scaled time out, storage
  bool TestMachineSettings();
  //! Test that the calibration test runs first and alone, and that the time out scales with its result
  bool TestCalibration();
  //! Test that a test only passes if it exits normally, reports checks, and none failed
  bool TestStrictResults();
};


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::Run()
{
  bool Passed = true;

  Passed = TestScopes() && Passed;
  Passed = TestLogDirectory() && Passed;
  Passed = TestCommandLine() && Passed;
  Passed = TestTimeoutAndTimings() && Passed;
  Passed = TestMachineSettings() && Passed;
  Passed = TestCalibration() && Passed;
  Passed = TestStrictResults() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::WriteScript(const MString& Directory, const MString& Name, const MString& Body) const
{
  const MString FileName = Directory + "/" + Name;
  {
    ofstream Out(FileName.Data());
    if (Out.is_open() == false) {
      return false;
    }
    Out<<"#!/bin/sh"<<endl<<Body<<endl;
  }
  error_code Error;
  filesystem::permissions(FileName.Data(), filesystem::perms::owner_all, filesystem::perm_options::add, Error);
  if (Error.value() != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::CreateFakeTests(const MString& Directory) const
{
  error_code Error;
  filesystem::create_directories(Directory.Data(), Error);
  bool OK = true;
  OK = WriteScript(Directory, "UTAlpha", "echo \"Passed tests: 3\"; echo \"Failed tests: 0\"") && OK;
  OK = WriteScript(Directory, "UTBeta", "echo \"output of the failing test\"; echo \"Passed tests: 1\"; echo \"Failed tests: 2\"; exit 1") && OK;
  OK = WriteScript(Directory, "UTEnvironment", "cat \"$HOME/.testdrive.cfg\"; echo \"Passed tests: 1\"; echo \"Failed tests: 0\"") && OK;
  OK = WriteScript(Directory, "UTSlow", "sleep 20; echo \"Passed tests: 1\"; echo \"Failed tests: 0\"") && OK;
  OK = WriteScript(Directory, "ETGamma", "echo \"Passed tests: 5\"; echo \"Failed tests: 0\"") && OK;
  OK = WriteScript(Directory, "ETDelta", "echo \"Passed tests: 7\"; echo \"Failed tests: 0\"") && OK;
  // Files which are no tests: no prefix, or not executable
  OK = WriteScript(Directory, "Helper", "echo \"not a test\"") && OK;
  {
    ofstream NotExecutable((Directory + "/UTNotExecutable").Data());
    NotExecutable<<"#!/bin/sh"<<endl<<"exit 0"<<endl;
  }
  return OK;
}


////////////////////////////////////////////////////////////////////////////////


UTTestDriver::Result UTTestDriver::Drive(const MString& BinDirectory, const vector<MString>& Arguments, const MString& LogDirectory) const
{
  Result Outcome;
  Outcome.m_LogDirectory = LogDirectory;

  vector<MString> Words = { BinDirectory + "/driver" };
  if (LogDirectory.IsEmpty() == false) {
    Words.push_back("--logdir");
    Words.push_back(LogDirectory);
  }
  for (const MString& Argument : Arguments) {
    Words.push_back(Argument);
  }
  vector<char*> Argv;
  for (MString& Word : Words) {
    Argv.push_back(const_cast<char*>(Word.Data()));
  }
  Argv.push_back(nullptr);

  pid_t Pid = fork();
  if (Pid < 0) {
    return Outcome;
  }
  if (Pid == 0) {
    // The child: no output, private configuration directory, run the driver
    int Null = open("/dev/null", O_WRONLY);
    if (Null >= 0) {
      dup2(Null, STDOUT_FILENO);
      dup2(Null, STDERR_FILENO);
      close(Null);
    }
    setenv("XDG_CONFIG_HOME", (BinDirectory + "/config").Data(), 1);
    setenv("HOME", BinDirectory.Data(), 1);
    MTestDriver Driver("driver", "test");
    _exit(Driver.Execute(static_cast<int>(Words.size()), Argv.data()));
  }

  int Status = 0;
  waitpid(Pid, &Status, 0);
  if (WIFEXITED(Status)) {
    Outcome.m_ExitCode = WEXITSTATUS(Status);
  }
  return Outcome;
}


////////////////////////////////////////////////////////////////////////////////


MString UTTestDriver::ReadAll(const MString& FileName)
{
  MString Content;
  MFile::ReadTextFile(FileName, Content);
  return Content;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::HasReport(const MString& Directory)
{
  error_code Error;
  for (const filesystem::directory_entry& Entry : filesystem::directory_iterator(Directory.Data(), Error)) {
    if (Entry.path().filename().string().compare(0, 14, "driver_failed_") == 0) {
      return true;
    }
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::TestScopes()
{
  bool Passed = true;

  PrepareTemporaryDirectory("scopes_bin");
  const MString Bin = GetTemporaryDirectoryName("scopes_bin");
  Passed = EvaluateTrue("CreateFakeTests()", "scopes", "The fake tests can be created", CreateFakeTests(Bin)) && Passed;
  const MString Logs = GetTemporaryDirectoryName("scopes_logs");

  // Default scope: only the unit tests - UTSlow is removed for these runs
  filesystem::remove((Bin + "/UTSlow").Data());

  {
    const Result Outcome = Drive(Bin, { }, Logs + "/default");
    Passed = Evaluate("Execute()", "default scope, exit code", "A failing unit test gives the exit code 1", Outcome.m_ExitCode, 1) && Passed;
    Passed = EvaluateTrue("Execute()", "default scope, unit tests", "The default scope runs the unit tests", Has(Outcome.m_LogDirectory, "UTAlpha.log") && Has(Outcome.m_LogDirectory, "UTBeta.log") && Has(Outcome.m_LogDirectory, "UTEnvironment.log")) && Passed;
    Passed = EvaluateFalse("Execute()", "default scope, end-to-end tests", "The default scope does not run the end-to-end tests", Has(Outcome.m_LogDirectory, "ETGamma.log") || Has(Outcome.m_LogDirectory, "ETDelta.log")) && Passed;
    Passed = EvaluateFalse("Execute()", "default scope, no tests", "Programs which are no tests (no prefix, or not executable) do not run", Has(Outcome.m_LogDirectory, "Helper.log") || Has(Outcome.m_LogDirectory, "UTNotExecutable.log")) && Passed;
  }
  {
    const Result Outcome = Drive(Bin, { "--all" }, Logs + "/all");
    Passed = Evaluate("Execute()", "--all, exit code", "A failing test gives the exit code 1 with --all", Outcome.m_ExitCode, 1) && Passed;
    Passed = EvaluateTrue("Execute()", "--all", "--all runs the unit tests and the end-to-end tests", Has(Outcome.m_LogDirectory, "UTAlpha.log") && Has(Outcome.m_LogDirectory, "UTBeta.log") && Has(Outcome.m_LogDirectory, "ETGamma.log") && Has(Outcome.m_LogDirectory, "ETDelta.log")) && Passed;
  }
  {
    // The short options
    const Result All = Drive(Bin, { "-a" }, Logs + "/short_all");
    Passed = Evaluate("Execute()", "-a, exit code", "-a is --all: a failing test gives the exit code 1", All.m_ExitCode, 1) && Passed;
    Passed = EvaluateTrue("Execute()", "-a", "-a runs the unit tests and the end-to-end tests", Has(All.m_LogDirectory, "UTAlpha.log") && Has(All.m_LogDirectory, "UTBeta.log") && Has(All.m_LogDirectory, "ETGamma.log") && Has(All.m_LogDirectory, "ETDelta.log")) && Passed;
    const Result EndToEnd = Drive(Bin, { "-e" }, Logs + "/short_endtoend");
    Passed = Evaluate("Execute()", "-e, exit code", "-e is --endtoend: the passing end-to-end tests give the exit code 0", EndToEnd.m_ExitCode, 0) && Passed;
    Passed = EvaluateTrue("Execute()", "-e", "-e runs the end-to-end tests", Has(EndToEnd.m_LogDirectory, "ETGamma.log") && Has(EndToEnd.m_LogDirectory, "ETDelta.log")) && Passed;
    Passed = EvaluateFalse("Execute()", "-e, unit tests", "-e does not run the unit tests", Has(EndToEnd.m_LogDirectory, "UTAlpha.log") || Has(EndToEnd.m_LogDirectory, "UTBeta.log")) && Passed;
  }
  {
    const Result Outcome = Drive(Bin, { "--endtoend" }, Logs + "/endtoend");
    Passed = Evaluate("Execute()", "--endtoend, exit code", "The passing end-to-end tests give the exit code 0", Outcome.m_ExitCode, 0) && Passed;
    Passed = EvaluateTrue("Execute()", "--endtoend", "--endtoend runs the end-to-end tests", Has(Outcome.m_LogDirectory, "ETGamma.log") && Has(Outcome.m_LogDirectory, "ETDelta.log")) && Passed;
    Passed = EvaluateFalse("Execute()", "--endtoend, unit tests", "--endtoend does not run the unit tests", Has(Outcome.m_LogDirectory, "UTAlpha.log") || Has(Outcome.m_LogDirectory, "UTBeta.log")) && Passed;
    Passed = EvaluateFalse("Execute()", "--endtoend, no report", "A run without failure writes no failure report", HasReport(Outcome.m_LogDirectory)) && Passed;
  }
  {
    // Named tests run in any scope, the prefix is optional, short names with M work
    const Result Outcome = Drive(Bin, { "ETGamma", "Alpha" }, Logs + "/named");
    Passed = Evaluate("Execute()", "named tests, exit code", "Named passing tests give the exit code 0", Outcome.m_ExitCode, 0) && Passed;
    Passed = EvaluateTrue("Execute()", "named tests", "An end-to-end test which is named runs in the default scope, a short name finds the unit test", Has(Outcome.m_LogDirectory, "ETGamma.log") && Has(Outcome.m_LogDirectory, "UTAlpha.log")) && Passed;
    Passed = EvaluateFalse("Execute()", "named tests, others", "Only the named tests run", Has(Outcome.m_LogDirectory, "UTBeta.log") || Has(Outcome.m_LogDirectory, "ETDelta.log")) && Passed;
  }
  {
    const Result Outcome = Drive(Bin, { "--endtoend", "UTAlpha" }, Logs + "/named_scope");
    Passed = EvaluateTrue("Execute()", "named test with --endtoend", "A named unit test runs with --endtoend, and only it", Has(Outcome.m_LogDirectory, "UTAlpha.log") && Has(Outcome.m_LogDirectory, "ETGamma.log") == false) && Passed;
  }
  {
    const Result Outcome = Drive(Bin, { "Unknown" }, Logs + "/unknown");
    Passed = Evaluate("Execute()", "unknown test", "An unknown test name gives the exit code 1", Outcome.m_ExitCode, 1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::TestLogDirectory()
{
  bool Passed = true;

  PrepareTemporaryDirectory("log_bin");
  const MString Bin = GetTemporaryDirectoryName("log_bin");
  Passed = EvaluateTrue("CreateFakeTests()", "log directory", "The fake tests can be created", CreateFakeTests(Bin)) && Passed;
  filesystem::remove((Bin + "/UTSlow").Data());
  const MString Logs = GetTemporaryDirectoryName("log_logs");

  // Check the log directory: created, with the output of every test and the report of the failed test
  const MString Directory = Logs + "/a/b/c";
  {
    const Result Outcome = Drive(Bin, { }, Directory);
    Passed = EvaluateTrue("Execute()", "created log directory", "The log directory is created with its parents", filesystem::is_directory(Directory.Data())) && Passed;
    Passed = EvaluateTrue("Execute()", "log of a passing test", "The log of a passing test has its output", ReadAll(Directory + "/UTAlpha.log").Contains("Passed tests: 3")) && Passed;
    Passed = EvaluateTrue("Execute()", "log of a failing test", "The log of a failing test has its output", ReadAll(Directory + "/UTBeta.log").Contains("output of the failing test")) && Passed;
    Passed = EvaluateTrue("Execute()", "failure report", "A run with a failing test writes a failure report into the log directory", HasReport(Directory)) && Passed;
    MString Report;
    error_code Error;
    for (const filesystem::directory_entry& Entry : filesystem::directory_iterator(Directory.Data(), Error)) {
      if (Entry.path().filename().string().compare(0, 14, "driver_failed_") == 0) {
        Report = ReadAll(Entry.path().string().c_str());
      }
    }
    Passed = EvaluateTrue("Execute()", "report content", "The failure report has the output of the failed test", Report.Contains("Test: UTBeta") && Report.Contains("output of the failing test")) && Passed;
    Passed = EvaluateFalse("Execute()", "report of passing tests", "The failure report has nothing of the passing tests", Report.Contains("Test: UTAlpha")) && Passed;
    // The tests read the absolute log directory from the settings file
    Passed = EvaluateTrue("Execute()", "settings file", "The driver writes the log directory into the testing settings file ~/.testdrive.cfg", ReadAll(Bin + "/.testdrive.cfg").Contains(MString("<LogDirectory>") + Directory + "</LogDirectory>")) && Passed;
    Passed = EvaluateTrue("Execute()", "tests read the settings", "The tests see the settings file with the log directory", ReadAll(Directory + "/UTEnvironment.log").Contains(MString("<LogDirectory>") + Directory + "</LogDirectory>")) && Passed;

    // The time out is in the settings file too
    Drive(Bin, { "--timeout", "77", "UTAlpha" }, Directory);
    Passed = EvaluateTrue("Execute()", "timeout in settings", "The driver writes its time out into the testing settings file", ReadAll(Bin + "/.testdrive.cfg").Contains("<Timeout>77</Timeout>")) && Passed;

    // Without --logdir the directory of the settings file is used again
    filesystem::remove((Directory + "/UTAlpha.log").Data(), Error);
    Drive(Bin, { }, "");
    Passed = EvaluateTrue("Execute()", "stored log directory", "Without --logdir the log directory of the settings file is used", filesystem::exists((Directory + "/UTAlpha.log").Data())) && Passed;
    (void) Outcome;
  }

  // Remove the stale files only of the tests which run, not of tests with the name as prefix
  {
    const MString Stale = Logs + "/stale";
    error_code Error;
    for (const char* Name : { "MEGAlib_abcdefghij_UTAlpha", "MEGAlib_abcdefghij_UTAlphaOther", "MEGAlib_abcdefghij_UTOther", "MEGAlib_ab_cdefghi_UTAlpha", "MEGAlib_abcdefghij_UTAlpha.txt", "Other_abcdefghij_UTAlpha" }) {
      filesystem::create_directories((Stale + "/" + Name).Data(), Error);
    }
    {
      ofstream File((Stale + "/MEGAlib_abcdefghij_UTBeta").Data());
      File<<"a file and not a directory";
    }
    // The root of a running test holds a lock: it is kept, also if the test runs again
    filesystem::create_directories((Stale + "/MEGAlib_klmnopqrst_UTAlpha").Data(), Error);
    const int ActiveRoot = open((Stale + "/MEGAlib_klmnopqrst_UTAlpha").Data(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    Passed = EvaluateTrue("flock()", "active root", "The root of a running test can be locked", ActiveRoot >= 0 && flock(ActiveRoot, LOCK_EX | LOCK_NB) == 0) && Passed;
    const Result Outcome = Drive(Bin, { }, Stale);
    if (ActiveRoot >= 0) {
      close(ActiveRoot);
    }
    Passed = EvaluateTrue("Execute()", "active root", "The working files of a test which is running (with a lock on its root) are kept", Has(Stale, "MEGAlib_klmnopqrst_UTAlpha")) && Passed;
    Passed = EvaluateFalse("Execute()", "stale files of a test which runs", "The working files of an earlier run of a test which runs again are removed", Has(Stale, "MEGAlib_abcdefghij_UTAlpha")) && Passed;
    Passed = EvaluateTrue("Execute()", "stale files of other tests", "The working files of tests which do not run, and of tests with a longer name, are kept", Has(Stale, "MEGAlib_abcdefghij_UTAlphaOther") && Has(Stale, "MEGAlib_abcdefghij_UTOther")) && Passed;
    Passed = EvaluateTrue("Execute()", "other names", "Entries which do not have the form of a working directory are kept", Has(Stale, "MEGAlib_ab_cdefghi_UTAlpha") && Has(Stale, "MEGAlib_abcdefghij_UTAlpha.txt") && Has(Stale, "Other_abcdefghij_UTAlpha")) && Passed;
    Passed = EvaluateTrue("Execute()", "files are not removed", "A file with the name of a working directory is not removed", Has(Stale, "MEGAlib_abcdefghij_UTBeta")) && Passed;
    (void) Outcome;
  }

  // Only one driver at a time uses a log directory: a second one waits. Another process holds the lock for 2 s
  {
    const MString Locked = Logs + "/locked";
    error_code Error;
    filesystem::create_directories(Locked.Data(), Error);
    const pid_t Holder = fork();
    if (Holder == 0) {
      const int Lock = open((Locked + "/.testdrive.lock").Data(), O_CREAT | O_RDWR, 0666);
      flock(Lock, LOCK_EX);
      {
        ofstream Ready((Locked + "/ready").Data());
        Ready<<"locked"<<endl;
      }
      sleep(2);
      _exit(0);
    }
    for (unsigned int Attempt = 0; Attempt < 50 && Has(Locked, "ready") == false; ++Attempt) {
      usleep(100000);
    }
    Passed = EvaluateTrue("flock()", "log directory", "Another process can hold the lock of a log directory", Has(Locked, "ready")) && Passed;
    const chrono::steady_clock::time_point Start = chrono::steady_clock::now();
    const Result Waiting = Drive(Bin, { "UTAlpha" }, Locked);
    const double Seconds = chrono::duration<double>(chrono::steady_clock::now() - Start).count();
    waitpid(Holder, nullptr, 0);
    Passed = EvaluateTrue("Execute()", "waits", "A driver waits until the other driver is done with the log directory, then it runs", Waiting.m_ExitCode == 0 && Has(Locked, "UTAlpha.log") && Seconds >= 1.0) && Passed;
    Passed = Evaluate("Execute()", "other directory", "A driver with another log directory does not wait", Drive(Bin, { "UTAlpha" }, Logs + "/unlocked").m_ExitCode, 0) && Passed;
  }

  // A log directory which cannot be created
  {
    const MString Blocker = Logs + "/blocker";
    {
      ofstream File(Blocker.Data());
      File<<"a file where the log directory should be";
    }
    const Result Outcome = Drive(Bin, { }, Blocker + "/logs");
    Passed = Evaluate("Execute()", "impossible log directory", "A log directory which cannot be created gives the exit code 1", Outcome.m_ExitCode, 1) && Passed;
  }

  // The default log directory of the settings: /tmp/$USER/megalib_testing_logs
  {
    const char* Old = getenv("USER");
    string OldUser;
    if (Old != nullptr) {
      OldUser = Old;
    }
    bool HadUser = false;
    if (Old != nullptr) {
      HadUser = true;
    }
    setenv("USER", "someone", 1);
    MSettingsTesting Settings;
    Passed = Evaluate("MSettingsTesting()", "user", "The default log directory is /tmp/$USER/megalib_testing_logs", Settings.GetLogDirectory(), MString("/tmp/someone/megalib_testing_logs")) && Passed;
    if (HadUser == true) {
      setenv("USER", OldUser.c_str(), 1);
    } else {
      unsetenv("USER");
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::TestCommandLine()
{
  bool Passed = true;

  PrepareTemporaryDirectory("cmd_bin");
  const MString Bin = GetTemporaryDirectoryName("cmd_bin");
  Passed = EvaluateTrue("CreateFakeTests()", "command line", "The fake tests can be created", CreateFakeTests(Bin)) && Passed;
  filesystem::remove((Bin + "/UTSlow").Data());
  const MString Logs = GetTemporaryDirectoryName("cmd_logs");

  {
    // Short options: -l creates the log directory, -t writes the time out
    const MString ShortLogs = Logs + "/short_options";
    Drive(Bin, { "-l", ShortLogs, "-t", "55", "UTAlpha" }, "");
    Passed = EvaluateTrue("Execute()", "-l", "-l is --logdir: the log of the test is in the given directory", filesystem::exists((ShortLogs + "/UTAlpha.log").Data())) && Passed;
    Passed = EvaluateTrue("Execute()", "-t", "-t is --timeout: the time out is in the testing settings file", ReadAll(Bin + "/.testdrive.cfg").Contains("<Timeout>55</Timeout>")) && Passed;
    Passed = Evaluate("Execute()", "-t without a time", "-t without a time gives the exit code 1", Drive(Bin, { "-t" }, "").m_ExitCode, 1) && Passed;
    Passed = Evaluate("Execute()", "-l without a directory", "-l without a directory gives the exit code 1", Drive(Bin, { "-l" }, "").m_ExitCode, 1) && Passed;
  }
  Passed = Evaluate("Execute()", "-a -e", "-a and -e exclude each other", Drive(Bin, { "-a", "-e" }, Logs + "/exclusive_short").m_ExitCode, 1) && Passed;
  Passed = Evaluate("Execute()", "--all --endtoend", "--all and --endtoend exclude each other", Drive(Bin, { "--all", "--endtoend" }, Logs + "/conflict").m_ExitCode, 1) && Passed;
  Passed = Evaluate("Execute()", "unknown option", "An unknown option gives the exit code 1", Drive(Bin, { "--nonsense" }, Logs + "/option").m_ExitCode, 1) && Passed;
  Passed = Evaluate("Execute()", "--timeout without value", "--timeout without a value gives the exit code 1", Drive(Bin, { "--timeout" }, Logs + "/timeout_missing").m_ExitCode, 1) && Passed;
  Passed = Evaluate("Execute()", "--timeout with text", "--timeout with a value which is no number gives the exit code 1", Drive(Bin, { "--timeout", "soon" }, Logs + "/timeout_text").m_ExitCode, 1) && Passed;
  for (const MString& Value: vector<MString>({ "nan", "inf", "-inf", "1e9999", "-1", "3601", "1e12" })) {
    Passed = Evaluate("Execute()", "--timeout " + Value, "--timeout with the value " + Value + " (not finite, negative, or above 3600 s) gives the exit code 1", Drive(Bin, { "--timeout", Value }, Logs + "/timeout_" + Value).m_ExitCode, 1) && Passed;
  }
  Passed = Evaluate("Execute()", "--help", "--help gives the exit code 0", Drive(Bin, { "--help" }, Logs + "/help").m_ExitCode, 0) && Passed;
  Passed = EvaluateFalse("Execute()", "--help runs nothing", "--help does not run any test", Has(Logs + "/help", "UTAlpha.log")) && Passed;

  // --logdir without a directory: the driver runs without the helper
  {
    vector<MString> Words = { Bin + "/driver", "--logdir" };
    vector<char*> Argv;
    for (MString& Word : Words) {
      Argv.push_back(const_cast<char*>(Word.Data()));
    }
    Argv.push_back(nullptr);
    pid_t Pid = fork();
    if (Pid == 0) {
      int Null = open("/dev/null", O_WRONLY);
      if (Null >= 0) {
        dup2(Null, STDOUT_FILENO);
        dup2(Null, STDERR_FILENO);
        close(Null);
      }
      MTestDriver Driver("driver", "test");
      _exit(Driver.Execute(static_cast<int>(Words.size()), Argv.data()));
    }
    int Status = 0;
    waitpid(Pid, &Status, 0);
    int ExitCode = -1;
    if (WIFEXITED(Status)) {
      ExitCode = WEXITSTATUS(Status);
    }
    Passed = Evaluate("Execute()", "--logdir without a directory", "--logdir without a directory gives the exit code 1", ExitCode, 1) && Passed;
  }

  // A directory without tests
  {
    const MString Empty = GetTemporaryDirectoryName("cmd_empty");
    error_code Error;
    filesystem::create_directories(Empty.Data(), Error);
    Passed = Evaluate("Execute()", "no tests", "A directory without tests gives the exit code 1", Drive(Empty, { }, Logs + "/empty").m_ExitCode, 1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::TestTimeoutAndTimings()
{
  bool Passed = true;

  PrepareTemporaryDirectory("timeout_bin");
  const MString Bin = GetTemporaryDirectoryName("timeout_bin");
  Passed = EvaluateTrue("CreateFakeTests()", "time out", "The fake tests can be created", CreateFakeTests(Bin)) && Passed;
  const MString Logs = GetTemporaryDirectoryName("timeout_logs");

  // A test which runs longer than the time out is killed and fails
  const Result Outcome = Drive(Bin, { "--timeout", "1", "UTSlow", "UTAlpha" }, Logs + "/slow");
  Passed = Evaluate("Execute()", "time out, exit code", "A test which exceeds the time out gives the exit code 1", Outcome.m_ExitCode, 1) && Passed;
  MString Report;
  error_code Error;
  for (const filesystem::directory_entry& Entry : filesystem::directory_iterator((Logs + "/slow").Data(), Error)) {
    if (Entry.path().filename().string().compare(0, 14, "driver_failed_") == 0) {
      Report = ReadAll(Entry.path().string().c_str());
    }
  }
  Passed = EvaluateTrue("Execute()", "time out report", "The failure report names the test which timed out", Report.Contains("Test: UTSlow") && Report.Contains("timed out")) && Passed;
  Passed = EvaluateFalse("Execute()", "time out, passing test", "The passing test is not in the failure report", Report.Contains("Test: UTAlpha")) && Passed;

  // A program which a test started dies with the test when the test is stopped
  Passed = EvaluateTrue("WriteScript()", "nested", "The fake test which starts a program can be written", WriteScript(Bin, "UTNested", "sleep 57 & echo $! > \"$HOME/nested_pid\"; wait")) && Passed;
  const Result Nested = Drive(Bin, { "--timeout", "1", "UTNested" }, Logs + "/nested");
  Passed = Evaluate("Execute()", "nested, exit code", "A test which exceeds the time out gives the exit code 1", Nested.m_ExitCode, 1) && Passed;
  const pid_t NestedProcess = static_cast<pid_t>(atoi(ReadAll(Bin + "/nested_pid").Data()));
  bool NestedGone = false;
  for (unsigned int Attempt = 0; Attempt < 30 && NestedGone == false; ++Attempt) {
    if (NestedProcess > 0 && kill(NestedProcess, 0) != 0) {
      NestedGone = true;
    }
    usleep(100000);
  }
  Passed = EvaluateTrue("Execute()", "nested killed", "The program which the stopped test started is stopped with it", NestedGone) && Passed;
  if (NestedGone == false && NestedProcess > 0) {
    kill(NestedProcess, SIGKILL);
  }

  // The timing cache is private to this test and has every test which ran
  const MString Cache = Bin + "/config/MEGAlib/driver.timings";
  const MString Timings = ReadAll(Cache);
  Passed = EvaluateTrue("Execute()", "timings", "The cache of the run times has the tests which ran", Timings.Contains("UTAlpha ") && Timings.Contains("UTSlow ")) && Passed;
  Passed = EvaluateFalse("Execute()", "timings of others", "The cache of the run times has no test which did not run", Timings.Contains("ETGamma")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::TestMachineSettings()
{
  bool Passed = true;

  MSettingsTesting Settings;
  Passed = Evaluate("GetMachineId()", "default", "Without a calibration there is no identifier of a machine", Settings.GetMachineId(), MString("")) && Passed;
  Passed = EvaluateNear("GetMachineSlowdown()", "default", "Without a calibration the slowdown is 1", Settings.GetMachineSlowdown(), 1.0, 1e-12) && Passed;

  PrepareTemporaryDirectory("machine_settings");
  const MString FileName = GetTemporaryDirectoryName("machine_settings") + "/machine.cfg";
  Settings.SetMachineId("0123456789abcdef");
  Settings.SetMachineSlowdown(1.75);
  Passed = EvaluateTrue("Write()", "machine", "The settings with the machine can be written", Settings.Write(FileName)) && Passed;
  MSettingsTesting Read;
  Passed = EvaluateTrue("Read()", "machine", "The settings with the machine can be read", Read.Read(FileName)) && Passed;
  Passed = Evaluate("GetMachineId()", "stored", "The identifier of the machine is stored", Read.GetMachineId(), MString("0123456789abcdef")) && Passed;
  Passed = EvaluateNear("GetMachineSlowdown()", "stored", "The slowdown is stored", Read.GetMachineSlowdown(), 1.75, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::TestCalibration()
{
  bool Passed = true;

  PrepareTemporaryDirectory("calibration_bin");
  const MString Bin = GetTemporaryDirectoryName("calibration_bin");
  const MString Logs = GetTemporaryDirectoryName("calibration_logs");
  Passed = EvaluateTrue("CreateFakeTests()", "calibration", "The fake tests can be created", CreateFakeTests(Bin)) && Passed;

  // The calibration program takes a while, stores a slowdown of 3, and marks that it is done (and how often it ran):
  // ETAfter fails if it starts before, ETLong needs 8 s: it only passes with the time out of 4 s scaled by 3
  Passed = EvaluateTrue("WriteScript()", "calibration", "The fake calibration can be written", WriteScript(Bin, "testdrivercalibration",
    "sleep 2; sed -i \"s|<MachineSlowdown>[^<]*|<MachineSlowdown>3|\" \"$HOME/.testdrive.cfg\"; touch \"$HOME/calibration_done\"; echo run >> \"$HOME/calibration_runs\"; echo \"Passed tests: 1\"; echo \"Failed tests: 0\"")) && Passed;
  Passed = EvaluateTrue("WriteScript()", "after", "The fake test which needs the calibration can be written", WriteScript(Bin, "ETAfter",
    "test -f \"$HOME/calibration_done\" || exit 1; echo \"Passed tests: 1\"; echo \"Failed tests: 0\"")) && Passed;
  Passed = EvaluateTrue("WriteScript()", "long", "The fake test which needs the scaled time out can be written", WriteScript(Bin, "ETLong",
    "test -f \"$HOME/calibration_done\" || exit 1; sleep 8; echo \"Passed tests: 1\"; echo \"Failed tests: 0\"")) && Passed;
  auto Runs = [&]() {
    const MString Log = ReadAll(Bin + "/calibration_runs");
    return static_cast<unsigned int>(count(Log.Data(), Log.Data() + Log.Length(), '\n'));
  };

  const Result Outcome = Drive(Bin, { "--endtoend", "--timeout", "4" }, Logs + "/first");
  Passed = Evaluate("Execute()", "calibration, exit code", "The tests which need the calibration pass: they start after it, with the time out scaled by its result", Outcome.m_ExitCode, 0) && Passed;
  Passed = Evaluate("Execute()", "calibration, runs", "The calibration runs once before the end-to-end tests", Runs(), 1U) && Passed;
  Passed = EvaluateTrue("Execute()", "calibration, log", "The output of the calibration is in the log directory, it is no test", Has(Logs + "/first", "testdrivercalibration.log") && Has(Logs + "/first", "ETAfter.log")) && Passed;
  Passed = EvaluateTrue("Execute()", "calibration, stored", "The calibration is in the testing settings", ReadAll(Bin + "/.testdrive.cfg").Contains("<MachineSlowdown>3</MachineSlowdown>")) && Passed;
  Passed = EvaluateTrue("Execute()", "calibration, time out", "The time out in the testing settings is the scaled one: 4 s times 3", ReadAll(Bin + "/.testdrive.cfg").Contains("<Timeout>12</Timeout>")) && Passed;
  const MString Config = ReadAll(Bin + "/.testdrive.cfg");
  const unsigned int IdBegin = Config.Index("<MachineId>") + 11;
  const MString Id = Config.GetSubString(IdBegin, Config.Index("</MachineId>") - IdBegin);
  Passed = EvaluateTrue("Execute()", "calibration, machine", "The driver stores the identifier of the machine: a hash value", Id.IsNumber()) && Passed;

  // A current calibration is not made again
  const Result Again = Drive(Bin, { "--endtoend", "--timeout", "4" }, Logs + "/again");
  Passed = EvaluateTrue("Execute()", "current calibration", "A calibration which was made on this machine is not repeated, and its slowdown is used", Again.m_ExitCode == 0 && Runs() == 1U) && Passed;

  // The unit tests do not need it
  const Result Unit = Drive(Bin, { }, Logs + "/unit");
  Passed = Evaluate("Execute()", "unit scope", "The unit tests run (one fails by design)", Unit.m_ExitCode, 1) && Passed;
  Passed = Evaluate("Execute()", "unit scope, calibration", "The unit tests do not start the calibration", Runs(), 1U) && Passed;

  // --calibrate runs it even without end-to-end tests
  const Result Forced = Drive(Bin, { "--calibrate", "UTAlpha" }, Logs + "/forced");
  Passed = EvaluateTrue("Execute()", "--calibrate", "--calibrate runs the calibration", Forced.m_ExitCode == 0 && Runs() == 2U && Has(Logs + "/forced", "testdrivercalibration.log")) && Passed;
  const Result Short = Drive(Bin, { "-c", "UTAlpha" }, Logs + "/short");
  Passed = EvaluateTrue("Execute()", "-c", "-c is --calibrate", Short.m_ExitCode == 0 && Runs() == 3U) && Passed;

  // A calibration of another machine (or version) is made again
  {
    MString OtherConfig = ReadAll(Bin + "/.testdrive.cfg");
    OtherConfig.ReplaceAllInPlace(Id, "123456789");
    ofstream Out((Bin + "/.testdrive.cfg").Data());
    Out<<OtherConfig;
  }
  const Result Other = Drive(Bin, { "ETAfter" }, Logs + "/other");
  Passed = EvaluateTrue("Execute()", "other machine", "A calibration of another machine or version of MEGAlib is made again", Other.m_ExitCode == 0 && Runs() == 4U) && Passed;

  // A faster machine gets a shorter time out: 4 s times 0.25 is 1 s, a test which needs 3 s is stopped
  PrepareTemporaryDirectory("calibration_faster_bin");
  const MString Faster = GetTemporaryDirectoryName("calibration_faster_bin");
  Passed = EvaluateTrue("CreateFakeTests()", "faster", "The fake tests can be created", CreateFakeTests(Faster)) && Passed;
  Passed = EvaluateTrue("WriteScript()", "faster calibration", "The fake calibration of a fast machine can be written", WriteScript(Faster, "testdrivercalibration",
    "sed -i \"s|<MachineSlowdown>[^<]*|<MachineSlowdown>0.25|\" \"$HOME/.testdrive.cfg\"; echo \"Passed tests: 1\"; echo \"Failed tests: 0\"")) && Passed;
  Passed = EvaluateTrue("WriteScript()", "faster test", "The fake test of 3 s can be written", WriteScript(Faster, "ETThreeSeconds", "sleep 3; echo \"Passed tests: 1\"; echo \"Failed tests: 0\"")) && Passed;
  Passed = Evaluate("Execute()", "faster machine", "A test which needs 3 s fails on a machine with a time out of 4 s times 0.25", Drive(Faster, { "--timeout", "4", "ETThreeSeconds" }, Logs + "/faster").m_ExitCode, 1) && Passed;
  Passed = EvaluateTrue("Execute()", "faster time out", "The time out in the testing settings is the scaled one: 4 s times 0.25", ReadAll(Faster + "/.testdrive.cfg").Contains("<Timeout>1</Timeout>")) && Passed;

  // A failing calibration fails the run, the tests run anyway
  PrepareTemporaryDirectory("calibration_failing_bin");
  const MString Failing = GetTemporaryDirectoryName("calibration_failing_bin");
  Passed = EvaluateTrue("CreateFakeTests()", "failing calibration", "The fake tests can be created", CreateFakeTests(Failing)) && Passed;
  Passed = EvaluateTrue("WriteScript()", "failing calibration", "The failing fake calibration can be written", WriteScript(Failing, "testdrivercalibration", "echo \"no cosima\"; exit 1")) && Passed;
  const Result Broken = Drive(Failing, { "ETGamma" }, Logs + "/broken");
  Passed = Evaluate("Execute()", "failing calibration, exit code", "A calibration which fails gives the exit code 1", Broken.m_ExitCode, 1) && Passed;
  Passed = EvaluateTrue("Execute()", "failing calibration, tests", "The tests run although the calibration failed", ReadAll(Logs + "/broken/ETGamma.log").Contains("Passed tests: 5")) && Passed;

  // Without the program --calibrate cannot work, but the end-to-end tests run
  PrepareTemporaryDirectory("calibration_missing_bin");
  const MString Missing = GetTemporaryDirectoryName("calibration_missing_bin");
  Passed = EvaluateTrue("CreateFakeTests()", "no calibration", "The fake tests can be created", CreateFakeTests(Missing)) && Passed;
  Passed = Evaluate("Execute()", "--calibrate without program", "--calibrate without the calibration program gives the exit code 1", Drive(Missing, { "--calibrate" }, Logs + "/missing").m_ExitCode, 1) && Passed;
  Passed = Evaluate("Execute()", "without program", "Without the calibration program the end-to-end tests run", Drive(Missing, { "--endtoend" }, Logs + "/without").m_ExitCode, 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTestDriver::TestStrictResults()
{
  bool Passed = true;

  PrepareTemporaryDirectory("strict_bin");
  const MString Bin = GetTemporaryDirectoryName("strict_bin");
  const MString Logs = GetTemporaryDirectoryName("strict_logs");
  Passed = EvaluateTrue("CreateDirectory()", "strict", "The directory of the fake tests can be created", MFile::CreateDirectory(Bin)) && Passed;
  Passed = EvaluateTrue("WriteScript()", "good", "The good fake test can be written", WriteScript(Bin, "UTGood", "echo \"Passed tests: 4\"; echo \"Failed tests: 0\"")) && Passed;
  Passed = EvaluateTrue("WriteScript()", "no checks", "The fake test without checks can be written", WriteScript(Bin, "UTNoChecks", "echo \"Passed tests: 0\"; echo \"Failed tests: 0\"")) && Passed;
  Passed = EvaluateTrue("WriteScript()", "silent", "The fake test without summary can be written", WriteScript(Bin, "UTSilent", "echo \"nothing to see\"")) && Passed;
  Passed = EvaluateTrue("WriteScript()", "failing checks", "The fake test with failed checks and exit code 0 can be written", WriteScript(Bin, "UTFailedChecks", "echo \"Passed tests: 3\"; echo \"Failed tests: 2\"")) && Passed;

  Passed = Evaluate("Execute()", "good", "A test with checks which all passed passes", Drive(Bin, { "UTGood" }, Logs + "/good").m_ExitCode, 0) && Passed;
  Passed = Evaluate("Execute()", "no checks", "A test which exits normally without any check fails", Drive(Bin, { "UTNoChecks" }, Logs + "/nochecks").m_ExitCode, 1) && Passed;
  Passed = Evaluate("Execute()", "silent", "A test which exits normally without a summary fails", Drive(Bin, { "UTSilent" }, Logs + "/silent").m_ExitCode, 1) && Passed;
  Passed = Evaluate("Execute()", "failed checks", "A test which reports failed checks fails although it exits normally", Drive(Bin, { "UTFailedChecks" }, Logs + "/failedchecks").m_ExitCode, 1) && Passed;

  const Result All = Drive(Bin, { }, Logs + "/all");
  Passed = Evaluate("Execute()", "all", "A run with failing tests fails", All.m_ExitCode, 1) && Passed;
  MString Report;
  error_code Error;
  for (const filesystem::directory_entry& Entry : filesystem::directory_iterator((Logs + "/all").Data(), Error)) {
    if (Entry.path().filename().string().compare(0, 14, "driver_failed_") == 0) {
      Report = ReadAll(Entry.path().string().c_str());
    }
  }
  Passed = EvaluateTrue("Execute()", "report", "The report names the tests without checks, without a summary, and with failed checks, and the reason", Report.Contains("Test: UTNoChecks") && Report.Contains("Metric: no checks") && Report.Contains("Test: UTSilent") && Report.Contains("Metric: no test summary") && Report.Contains("Test: UTFailedChecks")) && Passed;
  Passed = EvaluateFalse("Execute()", "report good", "The report does not name the good test", Report.Contains("Test: UTGood")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTTestDriver Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


////////////////////////////////////////////////////////////////////////////////
