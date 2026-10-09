/*
 * MTestDriver.cxx
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
#include "MTestDriver.h"

// Standard libs:
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// POSIX libs:
#include <fcntl.h>
#include <signal.h>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// MEGAlib libs:
#include "MFile.h"
#include "MSettingsTesting.h"
#include "MSystem.h"
#include "MUnitTest.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Set when the driver receives SIGINT or SIGTERM
volatile sig_atomic_t MTestDriver::m_Interrupted = 0;


////////////////////////////////////////////////////////////////////////////////


//! Create a driver with the given name (it is also the name of the program, which is not a test) for tests with the given prefixes
MTestDriver::MTestDriver(const MString& DriverName, const MString& DashboardTitle, const MString& UnitTestPrefix, const MString& EndToEndTestPrefix) :
  m_DriverName(DriverName),
  m_DashboardTitle(DashboardTitle),
  m_UnitTestPrefix(UnitTestPrefix),
  m_EndToEndTestPrefix(EndToEndTestPrefix),
  m_TimeoutSeconds(120.0),
  m_Calibrate(false),
  m_LogDirectoryLock(-1)
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MTestDriver::~MTestDriver()
{
  if (m_LogDirectoryLock >= 0) {
    close(m_LogDirectoryLock);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Execute all discovered tests of the selected scope or the requested subset, return the exit code of the program
//! Options: see PrintUsage(), everything which is no option is the name of a test
int MTestDriver::Execute(int argc, char** argv)
{
  const unsigned int PollIntervalMicroseconds = 1000000; // Check the running tests once per second
  const int InterruptedExitCode = 130; // The shell convention for a program stopped by Ctrl-C
  const unsigned int DefaultTerminalWidth = 100; // If the width of the terminal is unknown
  const unsigned int MinimumPaneWidth = 24; // Characters of a pane of the dashboard

  const char* Argv0 = nullptr;
  if (argc > 0) {
    Argv0 = argv[0];
  }
  m_BinDirectory = GetBinDirectory(Argv0);
  m_TimingFile = GetTimingFile();
  if (DiscoverTests() == false || m_AllTests.empty() == true) {
    merr<<"No test executables ("<<m_UnitTestPrefix<<"*, "<<m_EndToEndTestPrefix<<"*) found in "<<m_BinDirectory<<show;
    return 1;
  }

  // Separate the options from the requested test names
  vector<MString> Names;
  MTestingScope Scope = MTestingScope::c_UnitTests;
  bool AllSwitch = false, EndToEndSwitch = false;
  for (int a = 1; a < argc; ++a) {
    MString Option = argv[a];
    if (Option == "--timeout" || Option == "-t") {
      char* End = nullptr;
      double Timeout = -1.0;
      if (a+1 < argc) {
        Timeout = strtod(argv[a+1], &End);
      }
      if (a+1 >= argc || End == argv[a+1] || *End != '\0' || Timeout < 0.0) {
        merr<<"Usage: --timeout (-t) <seconds>, with 0 meaning no timeout"<<show;
        return 1;
      }
      m_TimeoutSeconds = Timeout;
      ++a;
    } else if (Option == "--logdir" || Option == "-l") {
      if (a+1 >= argc || argv[a+1][0] == '\0') {
        merr<<"Usage: --logdir (-l) <directory>"<<show;
        return 1;
      }
      m_LogDirectory = argv[a+1];
      ++a;
    } else if (Option == "--all" || Option == "-a") {
      AllSwitch = true;
    } else if (Option == "--endtoend" || Option == "-e") {
      EndToEndSwitch = true;
    } else if (Option == "--calibrate" || Option == "-c") {
      m_Calibrate = true;
    } else if (Option == "-h" || Option == "--help") {
      PrintUsage();
      return 0;
    } else if (Option.BeginsWith("-") == true) {
      merr<<"Unknown option: "<<Option<<show;
      PrintUsage();
      return 1;
    } else {
      Names.push_back(Option);
    }
  }
  if (AllSwitch == true && EndToEndSwitch == true) {
    merr<<"The options --all (-a) and --endtoend (-e) exclude each other"<<show;
    return 1;
  }
  if (PrepareLogDirectory() == false) {
    return 1;
  }
  if (AllSwitch == true) {
    Scope = MTestingScope::c_All;
  }
  if (EndToEndSwitch == true) {
    Scope = MTestingScope::c_EndToEnd;
  }

  map<MString, double> Timings;
  LoadTimings(Timings);
  vector<MString> Tests;
  if (BuildRequestedTests(Names, Scope, Tests) == false) {
    return 1;
  }
  if (m_Calibrate == true && MFile::IsExecutable(GetCalibrationProgram()) == false) {
    merr<<"The option --calibrate (-c) needs the program "<<GetCalibrationProgram()<<show;
    return 1;
  }
  if (Tests.empty() == true) {
    merr<<"No tests to run: no test of the requested scope is in "<<m_BinDirectory<<show;
    return 1;
  }
  SortRequestedTests(Tests, Timings);
  RemoveStaleTestFiles(Tests);

  // The dashboard tells which tests run
  MString Title = m_DashboardTitle;
  if (Names.empty() == false) {
    Title += " (requested tests)";
  } else if (Scope == MTestingScope::c_All) {
    Title += " (unit and end-to-end tests)";
  } else if (Scope == MTestingScope::c_EndToEnd) {
    Title += " (end-to-end tests)";
  } else {
    Title += " (unit tests)";
  }

  vector<MTestStatus> Statuses(Tests.size(), MTestStatus::c_Pending);
  vector<MString> Outputs(Tests.size(), "");
  vector<MString> OutputFiles(Tests.size(), "");
  vector<MString> Metrics(Tests.size(), "");
  vector<chrono::steady_clock::time_point> StartTimes(Tests.size());
  vector<bool> TimedOut(Tests.size(), false);
  map<pid_t, unsigned int> PidToIndex;
  const chrono::steady_clock::time_point SuiteStart = chrono::steady_clock::now();
  unsigned int MaxParallel = 0;
#ifdef __APPLE__
  size_t CpuSize = sizeof(MaxParallel);
  sysctlbyname("hw.physicalcpu", &MaxParallel, &CpuSize, nullptr, 0);
#else
  MaxParallel = thread::hardware_concurrency();
#endif
  if (MaxParallel == 0) {
    MaxParallel = 1;
  }
  bool UseTTY = false;
  if (isatty(STDOUT_FILENO) != 0) {
    UseTTY = true;
  }
  unsigned int Spinner = 0;
  const MString SpinnerChars = "|/-\\";
  MSettingsTesting Settings;
  Settings.Read();
  double Timeout = GetScaledTimeout(Settings);

  unsigned int TerminalWidth = DefaultTerminalWidth;
  if (UseTTY == true) {
    winsize Size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &Size) == 0 && Size.ws_col > 0) {
      TerminalWidth = Size.ws_col;
    }
  }

  auto Clip = [](const MString& Text, unsigned int Width) {
    if (Text.Length() <= Width) {
      return Text;
    }
    if (Width > 3) {
      return Text.GetSubString(0, Width - 3) + "...";
    }
    return Text.GetSubString(0, Width);
  };
  auto Render = [&]() {
    if (UseTTY == false) {
      return;
    }
    unsigned int Done = 0;
    unsigned int Running = 0;
    unsigned int Failed = 0;
    vector<MString> Left = {"", "", "", "", "Failed runs:"};
    vector<MString> Right = {"Running tests:"};
    for (unsigned int i = 0; i < Tests.size(); ++i) {
      if (Statuses[i] != MTestStatus::c_Pending && Statuses[i] != MTestStatus::c_Running) {
        ++Done;
      }
      if (Statuses[i] == MTestStatus::c_Running) {
        ++Running;
        Right.push_back(MString(SpinnerChars[Spinner % SpinnerChars.Length()]) + " " + Tests[i]);
      } else if (Statuses[i] == MTestStatus::c_Failed) {
        ++Failed;
        Left.push_back(Tests[i] + " (" + FormatFailureMetric(Metrics[i]) + ")");
      }
    }
    Left[0] = MString("Done: ") + Done + "/" + Tests.size();
    Left[1] = MString("Running: ") + Running;
    Left[2] = MString("Failed: ") + Failed;
    if (Failed == 0) {
      Left.push_back("none");
    }
    if (Running == 0) {
      Right.push_back("none");
    }
    while (Right.size() < MaxParallel + 1) {
      Right.push_back("");
    }

    const unsigned int ProgressWidth = 32;
    unsigned int Filled = ProgressWidth;
    if (Tests.empty() == false) {
      Filled = Done * ProgressWidth / Tests.size();
    }
    MString Bar = "[";
    for (unsigned int i = 0; i < ProgressWidth; ++i) {
      if (i < Filled) {
        Bar += "#";
      } else {
        Bar += ".";
      }
    }
    Bar += "]";
    double Elapsed = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - SuiteStart).count();
    MString Progress = MString("Progress: ") + Bar + "  " + Done + "/" + Tests.size() + "  " + FormatRuntime(Elapsed);

    unsigned int PaneWidth = MinimumPaneWidth;
    for (const MString& Line : Left) {
      PaneWidth = max(PaneWidth, static_cast<unsigned int>(Line.Length()));
    }
    for (const MString& Line : Right) {
      PaneWidth = max(PaneWidth, static_cast<unsigned int>(Line.Length()));
    }
    PaneWidth = max(PaneWidth, static_cast<unsigned int>(Progress.Length() / 2));
    unsigned int MaximumPaneWidth = 1;
    if (TerminalWidth > 7) {
      MaximumPaneWidth = (TerminalWidth - 7) / 2;
    }
    PaneWidth = min(PaneWidth, MaximumPaneWidth);
    const unsigned int InnerWidth = PaneWidth * 2 + 5;
    auto Pad = [](const MString& Text, unsigned int Width) { return Text + MString(string(Width - Text.Length(), ' ')); };
    auto Border = [&]() { return MString("+") + MString(string(InnerWidth, '-')) + "+"; };
    auto Center = [&](const MString& Text) {
      MString Value = Clip(Text, InnerWidth);
      unsigned int LeftPad = (InnerWidth - Value.Length()) / 2;
      return MString("|") + MString(string(LeftPad, ' ')) + Value
           + MString(string(InnerWidth - LeftPad - Value.Length(), ' ')) + "|";
    };

    mout<<"\x1b[2J\x1b[H"<<Border()<<endl<<Center(Title)<<endl<<Border()<<endl;
    for (unsigned int i = 0; i < max(Left.size(), Right.size()); ++i) {
      MString LeftText;
      if (i < Left.size()) {
        LeftText = Left[i];
      }
      MString RightText;
      if (i < Right.size()) {
        RightText = Right[i];
      }
      LeftText = Clip(LeftText, PaneWidth);
      RightText = Clip(RightText, PaneWidth);
      mout<<"| "<<Pad(LeftText, PaneWidth)<<" | "<<Pad(RightText, PaneWidth)<<" |"<<endl;
    }
    mout<<Border()<<endl<<Center(Progress)<<endl<<Border()<<endl<<flush;
  };

  // Stop the tests ourselves - they do not receive Ctrl-C
  signal(SIGINT, HandleInterrupt);
  signal(SIGTERM, HandleInterrupt);

  // Measure the speed of this machine before the first test starts if the time out of the end-to-end tests depends on it
  bool CalibrationFailed = false;
  bool HasEndToEndTest = false;
  for (const MString& Test : Tests) {
    if (Test.BeginsWith(m_EndToEndTestPrefix) == true) {
      HasEndToEndTest = true;
    }
  }
  if (m_Calibrate == true || (HasEndToEndTest == true && MFile::IsExecutable(GetCalibrationProgram()) == true && IsSystemCalibrated(Settings) == false)) {
    mout<<"Measuring the speed of this machine ..."<<endl;
    if (Calibrate() == false) {
      CalibrationFailed = true;
      mout<<"The calibration failed - the time out is not scaled, see "<<m_LogDirectory<<"/testdrivercalibration.log"<<endl;
    }
    Settings.Read();
    Timeout = GetScaledTimeout(Settings);
  }

  if (UseTTY == true) {
    mout<<"\x1b[?25l"<<flush;
  }
  Render();
  unsigned int Next = 0;
  unsigned int Running = 0;
  while (Next < Tests.size() || Running > 0) {
    if (m_Interrupted != 0) {
      for (const pair<const pid_t, unsigned int>& Entry : PidToIndex) {
        kill(-Entry.first, SIGKILL);
      }
      while (waitpid(-1, nullptr, 0) > 0) {
        // Intentionally left blank: wait for all stopped tests
      }
      if (UseTTY == true) {
        mout<<"\x1b[?25h"<<endl<<flush;
      }
      mout<<"Interrupted - all running tests have been stopped"<<endl;
      return InterruptedExitCode;
    }
    while (Next < Tests.size() && Running < MaxParallel) {
      // The complete output of the test stays in the log directory
      OutputFiles[Next] = m_LogDirectory + "/" + Tests[Next] + ".log";
      const int ProcessId = MSystem::StartProcessInBackground(m_AllTests[Tests[Next]], "", OutputFiles[Next], "", true);
      if (ProcessId < 0) {
        Statuses[Next] = MTestStatus::c_Failed;
        Metrics[Next] = "Failed to launch test process";
        OutputFiles[Next] = "";
        ++Next;
        continue;
      }
      PidToIndex[ProcessId] = Next;
      StartTimes[Next] = chrono::steady_clock::now();
      Statuses[Next++] = MTestStatus::c_Running;
      ++Running;
    }
    Render();

    int ProcessStatus = 0;
    pid_t Process = waitpid(-1, &ProcessStatus, WNOHANG);
    if (Process < 0) {
      if (errno == EINTR) {
        continue;
      }
      break;
    }
    if (Process == 0) {
      // Kill tests which run longer than the timeout
      if (Timeout > 0.0) {
        for (const pair<const pid_t, unsigned int>& Entry : PidToIndex) {
          double Elapsed = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - StartTimes[Entry.second]).count();
          if (TimedOut[Entry.second] == false && Elapsed > Timeout) {
            kill(-Entry.first, SIGKILL);
            TimedOut[Entry.second] = true;
          }
        }
      }
      ++Spinner;
      usleep(PollIntervalMicroseconds);
      continue;
    }
    const auto Found = PidToIndex.find(Process);
    if (Found == PidToIndex.end()) {
      continue;
    }
    const unsigned int Index = Found->second;
    PidToIndex.erase(Found);
    MFile::ReadTextFile(OutputFiles[Index], Outputs[Index]);
    Metrics[Index] = ExtractMetric(Outputs[Index]);
    if (TimedOut[Index] == true) {
      Metrics[Index] = MString("timed out after ") + FormatRuntime(Timeout);
    }
    Timings[Tests[Index]] = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - StartTimes[Index]).count();
    // A test passes if it exits normally, reports its checks, and none of them failed
    Statuses[Index] = MTestStatus::c_Failed;
    if (WIFEXITED(ProcessStatus) != 0 && WEXITSTATUS(ProcessStatus) == 0) {
      unsigned int PassedChecks = 0;
      unsigned int FailedChecks = 0;
      if (MUnitTest::ParseSummary(Outputs[Index], PassedChecks, FailedChecks) == false) {
        Metrics[Index] = "no test summary";
      } else if (FailedChecks > 0) {
        Metrics[Index] = ExtractMetric(Outputs[Index]);
      } else if (PassedChecks == 0) {
        Metrics[Index] = "no checks";
      } else {
        Statuses[Index] = MTestStatus::c_Passed;
      }
    }
    --Running;
  }

  unsigned int Failed = 0;
  for (MTestStatus Status : Statuses) {
    if (Status != MTestStatus::c_Passed) {
      ++Failed;
    }
  }
  Render();
  if (UseTTY == true) {
    mout<<"\x1b[?25h"<<endl<<flush;
  }
  if (UseTTY == false) {
    for (unsigned int i = 0; i < Tests.size(); ++i) {
      if (Statuses[i] == MTestStatus::c_Passed) {
        mout<<"PASS ";
      }
      mout<<Tests[i]<<" ("<<Metrics[i]<<")"<<endl;
    }
  }
  if (Failed > 0) {
    MString Report = WriteFailureReport(Tests, Statuses, Metrics, Outputs);
    if (Report.IsEmpty() == false) {
      mout<<"Failed test output report: "<<Report<<endl;
    } else {
      mout<<"Failed test output report: unable to write report file"<<endl;
    }
  }
  mout<<"Logs of all tests and the working files of failed tests: "<<m_LogDirectory<<endl;
  SaveTimings(Timings);
  if (Failed == 0 && CalibrationFailed == false) {
    return 0;
  }
  return 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Print the command line options
void MTestDriver::PrintUsage() const
{
  mout<<"Usage: "<<m_DriverName<<" [-a | -e] [-c] [-l <directory>] [-t <seconds>] [test ...]"<<endl
      <<"  Runs the test programs "<<m_UnitTestPrefix<<"* (unit tests) and "<<m_EndToEndTestPrefix<<"* (end-to-end tests) of the bin directory in parallel."<<endl
      <<"  (no option)         Run the unit tests"<<endl
      <<"  -a, --all           Run the unit tests and the end-to-end tests"<<endl
      <<"  -e, --endtoend      Run only the end-to-end tests"<<endl
      <<"  -c, --calibrate     Measure the speed of this machine again (the program testdrivercalibration): it runs before the first test, the time out is scaled with the result"<<endl
      <<"  -l, --logdir <directory> All logs and the working files of failed tests go here (default: the one in ~/.testdrive.cfg, initially /tmp/$USER/megalib_testing_logs)"<<endl
      <<"  -t, --timeout <seconds> Kill a test after this time, 0 means no timeout (default: 120)"<<endl
      <<"  test                The name of a test (the prefix is optional): the tests which are named run independent of the options above"<<endl
      <<"  -h, --help          This help"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


//! Create the log directory, and write it and the time out into the testing settings file, return false on failure
bool MTestDriver::PrepareLogDirectory()
{
  // Use the log directory of the settings file without --logdir
  MSettingsTesting Settings;
  MString SettingsFileName = Settings.GetSettingsFileName();
  MFile::ExpandFileName(SettingsFileName);
  if (MFile::Exists(SettingsFileName) == true) {
    Settings.Read(SettingsFileName);
  }
  if (m_LogDirectory.IsEmpty() == true) {
    m_LogDirectory = Settings.GetLogDirectory();
  }

  // An absolute path: the tests may change their working directory
  error_code Error;
  m_LogDirectory = filesystem::absolute(m_LogDirectory.Data(), Error).lexically_normal().string().c_str();
  if (Error.value() != 0 || MFile::CreateDirectory(m_LogDirectory) == false) {
    merr<<"Unable to create the log directory: "<<m_LogDirectory<<show;
    return false;
  }

  // Only one driver at a time can use a log directory
  m_LogDirectoryLock = open((m_LogDirectory + "/.testdrive.lock").Data(), O_CREAT | O_RDWR | O_CLOEXEC, 0666);
  if (m_LogDirectoryLock < 0) {
    merr<<"Unable to create the lock file in the log directory: "<<m_LogDirectory<<show;
    return false;
  }
  if (flock(m_LogDirectoryLock, LOCK_EX | LOCK_NB) != 0) {
    mout<<"Another "<<m_DriverName<<" uses the log directory "<<m_LogDirectory<<": waiting for it to finish ..."<<endl;
    flock(m_LogDirectoryLock, LOCK_EX);
  }

  // Write the log directory and the time out for the tests
  Settings.SetLogDirectory(m_LogDirectory);
  if (m_Calibrate == true) {
    Settings.SetMachineId("");
    Settings.SetMachineSlowdown(1.0);
  }
  Settings.SetTimeout(GetScaledTimeout(Settings));
  if (Settings.Write() == false) {
    merr<<"Unable to write the testing settings file: "<<Settings.GetSettingsFileName()<<show;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the directory with the test programs: the directory of the driver, else $(MEGALIB)/bin, else bin of the working directory
MString MTestDriver::GetBinDirectory(const char* Argv0) const
{
  MString Executable;
  if (Argv0 != nullptr) {
    Executable = Argv0;
  }
  if (Executable.Contains("/") == true || Executable.Contains("\\") == true) {
    MString Directory = MFile::GetDirectoryName(Executable);
    if (Directory.IsEmpty() == false) {
      return Directory;
    }
  }

  MString MEGAlibBin = "$(MEGALIB)/bin";
  if (MFile::ExpandFileName(MEGAlibBin) == true) {
    return MEGAlibBin;
  }

  return MFile::GetWorkingDirectory() + "/bin";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the name of the file with the run times of the last runs (in the configuration directory of the user)
MString MTestDriver::GetTimingFile() const
{
  const char* Home = getenv("HOME");
  MString BaseDirectory;
#ifdef __APPLE__
  if (Home != nullptr && Home[0] != '\0') {
    BaseDirectory = MString(Home) + "/Library/Application Support";
  }
#else
  const char* XdgConfigHome = getenv("XDG_CONFIG_HOME");
  if (XdgConfigHome != nullptr && XdgConfigHome[0] != '\0') {
    BaseDirectory = XdgConfigHome;
  } else if (Home != nullptr && Home[0] != '\0') {
    BaseDirectory = MString(Home) + "/.config";
  }
#endif
  if (BaseDirectory.IsEmpty() == true) {
    return "";
  }

  MString ConfigDirectory = BaseDirectory + "/MEGAlib";
  if (MFile::CreateDirectory(ConfigDirectory) == false) {
    return "";
  }
  return ConfigDirectory + "/" + m_DriverName + ".timings";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the name of the test program which a request means (the prefix is optional), an empty string if there is none
MString MTestDriver::ResolveRequest(const MString& Input) const
{
  // The plain name: without path and extension
  MString Name = MFile::GetBaseName(Input);
  if (Name.EndsWith(".cxx") == true) {
    Name = Name.GetSubString(0, Name.Length() - 4);
  }
  vector<MString> Candidates;
  if (Name.BeginsWith(m_UnitTestPrefix) == true || Name.BeginsWith(m_EndToEndTestPrefix) == true) {
    Candidates.push_back(Name);
  } else {
    // Accept a short name (e.g. Rotation or MRotation) of a unit or an end-to-end test
    for (const MString& Prefix : { m_UnitTestPrefix, m_EndToEndTestPrefix }) {
      Candidates.push_back(Prefix + Name);
      if (Name.BeginsWith("M") == true && Name.Length() > 1) {
        Candidates.push_back(Prefix + Name.GetSubString(1));
      }
    }
  }

  for (const MString& Candidate : Candidates) {
    if (m_AllTests.find(Candidate) != m_AllTests.end()) {
      return Candidate;
    }
  }
  if (Candidates.empty() == false) {
    return Candidates.front();
  }
  return Name;
}


////////////////////////////////////////////////////////////////////////////////


//! Find the test programs in the bin directory (all executable programs with the prefix of the unit or the end-to-end tests)
bool MTestDriver::DiscoverTests()
{
  m_AllTests.clear();
  error_code Error;
  filesystem::directory_iterator Directory(m_BinDirectory.Data(), Error);
  if (Error.value() != 0) {
    return false;
  }

  for (const filesystem::directory_entry& Entry : Directory) {
    MString Name = Entry.path().filename().string().c_str();
    if ((Name.BeginsWith(m_UnitTestPrefix) == false && Name.BeginsWith(m_EndToEndTestPrefix) == false) || Name == m_DriverName) {
      continue;
    }
    MString Path = m_BinDirectory + "/" + Name;
    if (MFile::IsExecutable(Path) == true) {
      m_AllTests[Name] = Path;
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Create the list of tests to run: the named tests, or all tests of the scope if none is named, return false if a name is unknown
bool MTestDriver::BuildRequestedTests(const vector<MString>& Names, MTestingScope Scope, vector<MString>& RequestedTests) const
{
  RequestedTests.clear();
  if (Names.empty() == true) {
    // All tests of the scope
    for (const pair<const MString, MString>& Entry : m_AllTests) {
      const bool EndToEnd = Entry.first.BeginsWith(m_EndToEndTestPrefix);
      if (Scope == MTestingScope::c_All || (Scope == MTestingScope::c_EndToEnd && EndToEnd == true) || (Scope == MTestingScope::c_UnitTests && EndToEnd == false)) {
        RequestedTests.push_back(Entry.first);
      }
    }
    return true;
  }

  // Tests which are requested by name run independent of the scope
  set<MString> Seen;
  for (const MString& Name : Names) {
    MString Requested = ResolveRequest(Name);
    if (Seen.insert(Requested).second == false) {
      continue;
    }
    if (m_AllTests.find(Requested) == m_AllTests.end()) {
      merr<<"Unknown test request: "<<Name<<" -> "<<Requested<<show;
      return false;
    }
    RequestedTests.push_back(Requested);
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Sort the tests: the slowest of the last run first
void MTestDriver::SortRequestedTests(vector<MString>& RequestedTests, const map<MString, double>& Timings) const
{
  stable_sort(RequestedTests.begin(), RequestedTests.end(), [&](const MString& First, const MString& Second) {
    const auto Ta = Timings.find(First);
    const auto Tb = Timings.find(Second);
    double TimeA = -1.0;
    if (Ta != Timings.end()) {
      TimeA = Ta->second;
    }
    double TimeB = -1.0;
    if (Tb != Timings.end()) {
      TimeB = Tb->second;
    }
    if (TimeA != TimeB) {
      if (TimeA > TimeB) {
        return true;
      }
      return false;
    }
    if (First < Second) {
      return true;
    }
    return false;
  });
}


////////////////////////////////////////////////////////////////////////////////


//! Read the run times of the last runs from the timing file
void MTestDriver::LoadTimings(map<MString, double>& Timings) const
{
  Timings.clear();
  if (m_TimingFile.IsEmpty() == true) {
    return;
  }

  ifstream Input(m_TimingFile.Data());
  string Line;
  while (getline(Input, Line).fail() == false) {
    if (Line.empty() == true || Line[0] == '#') {
      continue;
    }
    string Name;
    double Seconds = 0.0;
    stringstream Stream(Line);
    if ((Stream>>Name>>Seconds).fail() == false && Seconds >= 0.0) {
      Timings[Name.c_str()] = Seconds;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Write the run times to the timing file
void MTestDriver::SaveTimings(const map<MString, double>& Timings) const
{
  if (m_TimingFile.IsEmpty() == true) {
    return;
  }
  ofstream Output(m_TimingFile.Data(), ios::trunc);
  if (Output.is_open() == false) {
    return;
  }

  Output<<"# "<<m_DriverName<<" timing cache"<<endl;
  Output<<"# test_name seconds"<<endl;
  for (const pair<const MString, double>& Entry : Timings) {
    Output<<Entry.first<<" "<<Entry.second<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Create the identifier of this machine and this MEGAlib version: a hash of the CPU model, the number of cores, and the version
MString MTestDriver::CreateMachineId() const
{
  const MString Description = MSystem::GetCpuModel() + "|" + thread::hardware_concurrency() + "|" + g_VersionString;
  return MString(Description.GetHash());
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the calibration in the settings was made on this machine with this version of MEGAlib
bool MTestDriver::IsSystemCalibrated(const MSettingsTesting& Settings) const
{
  if (Settings.GetMachineId().IsEmpty() == true) {
    return false;
  }
  if (Settings.GetMachineId() != CreateMachineId()) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the time out in seconds adapted to the speed of this machine: the time out times the slowdown of the calibration, if it is current (0: no time out)
double MTestDriver::GetScaledTimeout(const MSettingsTesting& Settings) const
{
  if (m_TimeoutSeconds <= 0.0) {
    return 0.0;
  }
  if (IsSystemCalibrated(Settings) == false) {
    return m_TimeoutSeconds;
  }
  return m_TimeoutSeconds*Settings.GetMachineSlowdown();
}


////////////////////////////////////////////////////////////////////////////////


//! Run the calibration program, wait for it, and store the identifier of this machine and the scaled time out in the settings file, return false if one of that failed
bool MTestDriver::Calibrate() const
{
  const pid_t Process = MSystem::StartProcessInBackground(GetCalibrationProgram(), "", m_LogDirectory + "/testdrivercalibration.log", "", true);
  if (Process < 0) {
    return false;
  }
  MSettingsTesting Settings;
  Settings.Read();
  const int Status = MSystem::WaitForBackgroundProcess(Process, static_cast<unsigned int>(ceil(GetScaledTimeout(Settings))), &m_Interrupted);
  if (WIFEXITED(Status) == false || WEXITSTATUS(Status) != 0) {
    return false;
  }

  // The program stored the slowdown, the identifier of this machine and the time out for it are stored now
  Settings.Read();
  Settings.SetMachineId(CreateMachineId());
  Settings.SetTimeout(GetScaledTimeout(Settings));
  if (Settings.Write() == false) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Signal handler for SIGINT and SIGTERM: only flag the interrupt, the main loop stops the tests
void MTestDriver::HandleInterrupt(int)
{
  m_Interrupted = 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Remove the working files of failed tests of an earlier run for the tests which run now, but not those of tests which are running
void MTestDriver::RemoveStaleTestFiles(const vector<MString>& Tests) const
{
  // Remove the files MEGAlib_<random>_<test> of failed earlier runs for the tests which run now
  const unsigned int PrefixLength = 8; // MEGAlib_
  const unsigned int RandomLength = 10; // The random characters in the name
  const unsigned int NameLength = PrefixLength + RandomLength + 1; // MEGAlib_<random>_
  error_code Error;
  vector<filesystem::path> Stale;
  for (const filesystem::directory_entry& Entry : filesystem::directory_iterator(m_LogDirectory.Data(), Error)) {
    if (Entry.is_directory(Error) == false) {
      continue;
    }
    const string Name = Entry.path().filename().string();
    if (Name.compare(0, PrefixLength, "MEGAlib_") != 0 || Name.size() <= NameLength || Name[NameLength - 1] != '_') {
      continue;
    }
    bool Random = true;
    for (unsigned int c = PrefixLength; c < PrefixLength + RandomLength; ++c) {
      if (isalnum(static_cast<unsigned char>(Name[c])) == 0) {
        Random = false;
      }
    }
    if (Random == false) {
      continue;
    }
    const MString Test = Name.substr(NameLength).c_str();
    for (const MString& Candidate : Tests) {
      if (Candidate == Test) {
        Stale.push_back(Entry.path());
      }
    }
  }
  for (const filesystem::path& Path : Stale) {
    // A running test (of this or of another driver) holds a lock on its root
    const int Root = open(Path.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (Root < 0) {
      continue;
    }
    if (flock(Root, LOCK_EX | LOCK_NB) == 0) {
      filesystem::remove_all(Path, Error);
    }
    close(Root);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the passed and failed counters of the output of a test ("done" if it has none)
MString MTestDriver::ExtractMetric(const MString& Output) const
{
  unsigned int Passed = 0;
  unsigned int Failed = 0;
  if (MUnitTest::ParseSummary(Output, Passed, Failed) == false) {
    return "done";
  }
  return MString("Passed tests: ") + Passed + ", Failed tests: " + Failed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the metric in the form which is shown for a failed test
MString MTestDriver::FormatFailureMetric(const MString& Metric) const
{
  unsigned int Passed = 0;
  unsigned int Failed = 0;
  if (MUnitTest::ParseSummary(Metric, Passed, Failed) == false) {
    return Metric;
  }
  return MString(Failed) + "/" + Passed + " failed";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the time in seconds with one decimal and the unit
MString MTestDriver::FormatRuntime(double Seconds) const
{
  ostringstream Output;
  Output.setf(ios::fixed);
  Output.precision(1);
  Output<<Seconds<<"s";
  return Output.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Write the report with the output of the failed tests into the log directory, return its file name (empty if it could not be written)
MString MTestDriver::WriteFailureReport(const vector<MString>& Tests, const vector<MTestStatus>& Statuses,
                                        const vector<MString>& Metrics, const vector<MString>& Outputs) const
{
  time_t Now = time(nullptr);
  tm LocalTime;
  localtime_r(&Now, &LocalTime);
  const unsigned int TimeTagLength = 32;
  vector<char> TimeTag(TimeTagLength);
  strftime(TimeTag.data(), TimeTag.size(), "%Y%m%dT%H%M%S", &LocalTime);

  MString FileName = m_LogDirectory + "/" + m_DriverName + "_failed_" + TimeTag.data() + ".log";
  ofstream Report(FileName.Data(), ios::trunc);
  if (Report.is_open() == false) {
    return "";
  }

  Report<<m_DriverName<<" failed test output report"<<endl<<"Created: "<<TimeTag.data()<<endl;
  for (unsigned int i = 0; i < Tests.size(); ++i) {
    if (Statuses[i] == MTestStatus::c_Passed) {
      continue;
    }
    Report<<endl<<"================================================================================"<<endl;
    Report<<"Test: "<<Tests[i]<<endl<<"Metric: "<<Metrics[i]<<endl;
    Report<<"--------------------------------------------------------------------------------"<<endl;
    if (Outputs[i].IsEmpty() == false) {
      Report<<Outputs[i];
    } else {
      Report<<"No output captured."<<endl;
    }
    if (Outputs[i].IsEmpty() == false && Outputs[i].EndsWith("\n") == false) {
      Report<<endl;
    }
  }
  return FileName;
}


// MTestDriver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
