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
#include <cmath>
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
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
#include <sys/file.h>
#include <sys/ioctl.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TCanvas.h"
#include "TFile.h"
#include "TKey.h"
#include "TLatex.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MEndToEndCampaign.h"
#include "MEndToEndReferenceList.h"
#include "MEndToEndReport.h"
#include "MFile.h"
#include "MSettingsTesting.h"
#include "MProcessManager.h"
#include "MSystem.h"
#include "MUnitTest.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Create a driver with the given name (it is also the name of the program, which is not a test) for tests with the given prefixes
MTestDriver::MTestDriver(const MString& DriverName, const MString& DashboardTitle, const MString& UnitTestPrefix, const MString& EndToEndTestPrefix) :
  m_DriverName(DriverName),
  m_DashboardTitle(DashboardTitle),
  m_UnitTestPrefix(UnitTestPrefix),
  m_EndToEndTestPrefix(EndToEndTestPrefix),
  m_TimeoutSeconds(120.0),
  m_Calibrate(false),
  m_ReferenceCampaign(false),
  m_CampaignRuns(50),
  m_CampaignMinimumRuns(30),
  m_CampaignSeed(1),
  m_CampaignInstall(false),
  m_DriverLock(-1)
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MTestDriver::~MTestDriver()
{
  if (m_DriverLock >= 0) {
    close(m_DriverLock);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Execute all discovered tests of the selected scope or the requested subset, return the exit code of the program
//! Options: see PrintUsage(), everything which is no option is the name of a test
int MTestDriver::Execute(int argc, char** argv)
{
  const unsigned int PollIntervalMicroseconds = 1000000; // Check the running tests once per second
  const int InterruptedExitCode = 130; // The shell convention for a program stopped by Ctrl-C
  const double MaximumTimeout = 3600.0; // Seconds
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
      // Reject text, negative values, and nan, inf, or overflows (1e9999) - they would never reach the deadline
      if (a+1 >= argc || End == argv[a+1] || *End != '\0' || isfinite(Timeout) == false || Timeout < 0.0 || Timeout > MaximumTimeout) {
        merr<<"Usage: --timeout (-t) T, with T = [0...3600] seconds and T = 0: no timeout"<<show;
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
    } else if (Option == "--reference-campaign" || Option == "-r") {
      m_ReferenceCampaign = true;
    } else if (Option == "--runs" || Option == "--minimum-runs" || Option == "--seed") {
      // Positive numbers (the seed may be zero)
      char* End = nullptr;
      unsigned long Number = 0;
      if (a+1 < argc) {
        Number = strtoul(argv[a+1], &End, 10);
      }
      const bool IsSeed = (Option == "--seed");
      if (a+1 >= argc || End == argv[a+1] || *End != '\0' || argv[a+1][0] == '-' || Number > 2000000000UL || (Number == 0 && IsSeed == false)) {
        merr<<"Usage: "<<Option<<" N, with N = [1...2000000000] ([0...2000000000] for --seed)"<<show;
        return 1;
      }
      if (Option == "--runs") {
        m_CampaignRuns = Number;
      } else if (Option == "--minimum-runs") {
        m_CampaignMinimumRuns = Number;
      } else {
        m_CampaignSeed = Number;
      }
      ++a;
    } else if (Option == "--campaign-dir" || Option == "--references-dir" || Option == "--reason") {
      if (a+1 >= argc || argv[a+1][0] == '\0') {
        merr<<"Usage: "<<Option<<" <text>"<<show;
        return 1;
      }
      if (Option == "--campaign-dir") {
        m_CampaignDirectory = argv[a+1];
      } else if (Option == "--references-dir") {
        m_ReferencesDirectory = argv[a+1];
      } else {
        m_CampaignReason = argv[a+1];
      }
      ++a;
    } else if (Option == "--install") {
      m_CampaignInstall = true;
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
  if (m_ReferenceCampaign == true && (AllSwitch == true || EndToEndSwitch == true)) {
    merr<<"The option --reference-campaign (-r) runs the tests which are named, or all end-to-end tests with references: it excludes --all (-a) and --endtoend (-e)"<<show;
    return 1;
  }
  if (m_ReferenceCampaign == false && (m_CampaignInstall == true || m_CampaignReason.IsEmpty() == false || m_CampaignDirectory.IsEmpty() == false || m_ReferencesDirectory.IsEmpty() == false)) {
    merr<<"The options --install, --reason, --campaign-dir, and --references-dir need --reference-campaign (-r)"<<show;
    return 1;
  }
  if (m_CampaignInstall == true && m_CampaignReason.IsEmpty() == true) {
    merr<<"The option --install needs the reason for new references: --reason <text>"<<show;
    return 1;
  }
  if (PrepareLogDirectory() == false) {
    return 1;
  }
  if (m_ReferenceCampaign == true) {
    return RunReferenceCampaign(Names);
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
  map<unsigned int, unsigned int> IndexOfId;
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
  MProcessManager Manager(MaxParallel);
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
  MProcessManager::ClearInterrupt();
  MProcessManager::InstallInterruptHandlers();

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
  while (Next < Tests.size() || Manager.HasRunning() == true) {
    if (MProcessManager::WasInterrupted() == true) {
      Manager.StopAll();
      if (UseTTY == true) {
        mout<<"\x1b[?25h"<<endl<<flush;
      }
      mout<<"Interrupted - all running tests have been stopped"<<endl;
      MProcessManager::RestoreInterruptHandlers();
      return InterruptedExitCode;
    }
    while (Next < Tests.size() && Manager.CanStart() == true) {
      // The complete output of the test stays in the log directory
      OutputFiles[Next] = m_LogDirectory + "/" + Tests[Next] + ".log";
      const int Id = Manager.Start(m_AllTests[Tests[Next]], "", OutputFiles[Next], "", Timeout);
      if (Id < 0) {
        Statuses[Next] = MTestStatus::c_Failed;
        Metrics[Next] = "Failed to launch test process";
        OutputFiles[Next] = "";
        ++Next;
        continue;
      }
      IndexOfId[Id] = Next;
      Statuses[Next++] = MTestStatus::c_Running;
    }
    Render();

    // The tests which ended get their result, the ones which run longer than the time out are stopped by the manager
    const bool Waited = Manager.Poll();
    const vector<MManagedProcess> Ended = Manager.TakeEnded();
    for (const MManagedProcess& Process: Ended) {
      const unsigned int Index = IndexOfId[Process.m_Id];
      MFile::ReadTextFile(OutputFiles[Index], Outputs[Index]);
      Metrics[Index] = ExtractMetric(Outputs[Index]);
      if (Process.m_TimedOut == true) {
        Metrics[Index] = MString("timed out after ") + FormatRuntime(Timeout);
      }
      if (Process.m_Lost == true) {
        Metrics[Index] = "lost: the test could not be waited for";
      }
      Timings[Tests[Index]] = Process.m_Seconds;
      // A test passes if it exits normally, reports its checks, and none of them failed
      Statuses[Index] = MTestStatus::c_Failed;
      if (Process.m_ExitCode == 0) {
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
    }
    if (Waited == false) {
      // A test which cannot be waited for (reaped by someone else): the others are stopped, the run ends with what it has
      merr<<"Unable to wait for the tests: the tests which still run are stopped"<<show;
      Manager.StopAll();
      break;
    }
    if (Ended.empty() == true && Manager.HasRunning() == true) {
      ++Spinner;
      usleep(PollIntervalMicroseconds);
    }
  }
  MProcessManager::RestoreInterruptHandlers();

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
  const MString SummaryReport = WriteSummaryReport(Tests, Statuses, Metrics, Timings);
  if (SummaryReport.IsEmpty() == false) {
    mout<<"Summary report of the tests: "<<SummaryReport<<endl;
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
      <<"       "<<m_DriverName<<" -r [--runs N] [--minimum-runs N] [--seed S] [--campaign-dir <directory>] [--install --reason <text> [--references-dir <directory>]] [test ...]"<<endl
      <<"  Runs the test programs "<<m_UnitTestPrefix<<"* (unit tests) and "<<m_EndToEndTestPrefix<<"* (end-to-end tests) of the bin directory in parallel."<<endl
      <<"  (no option)         Run the unit tests"<<endl
      <<"  -a, --all           Run the unit tests and the end-to-end tests"<<endl
      <<"  -e, --endtoend      Run only the end-to-end tests"<<endl
      <<"  -c, --calibrate     Measure the speed of this machine again (the program testdrivercalibration): it runs before the first test, the time out is scaled with the result"<<endl
      <<"  -l, --logdir <directory> All logs and the working files of failed tests go here, for this run only (default: /tmp/$USER/megalib_testing_logs)"<<endl
      <<"  -t, --timeout <T>       Kill a test after T = [0...3600] seconds, T = 0: no timeout (default: 120)"<<endl
      <<"  -r, --reference-campaign Make the references of end-to-end tests: every test runs N times in its own directory, the runs are analyzed (campaign report and candidate references in the campaign directory)"<<endl
      <<"      --runs N            The number of runs of every test (default: 50), the runs of an earlier campaign in the campaign directory are removed"<<endl
      <<"      --minimum-runs N    The smallest number of runs which makes references (default: 30)"<<endl
      <<"      --seed S            The base seed of the campaign: the runs are reproducible (default: 1), another seed makes independent runs"<<endl
      <<"      --campaign-dir <directory> The directory of the campaign (default: <logdir>/campaign)"<<endl
      <<"      --install           Install the references of the tests whose campaign has no error into $(MEGALIB)/resource/endtoend, it needs --reason"<<endl
      <<"      --reason <text>     The reason for new references: it is written into their record"<<endl
      <<"      --references-dir <directory> Install into this directory instead"<<endl
      <<"  test                The name of a test (the prefix is optional): the tests which are named run independent of the options above"<<endl
      <<"  -h, --help          This help"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


//! Create the log directory, and write it and the time out into the testing settings file, return false on failure
bool MTestDriver::PrepareLogDirectory()
{
  // Only one driver at a time per user: the tests of a driver read the one settings file of the user
  MSettingsTesting Settings;
  MString SettingsFileName = Settings.GetSettingsFileName();
  MFile::ExpandFileName(SettingsFileName);
  m_DriverLock = open((SettingsFileName + ".lock").Data(), O_CREAT | O_RDWR | O_CLOEXEC, 0666);
  if (m_DriverLock < 0) {
    merr<<"Unable to create the lock file next to the testing settings file: "<<SettingsFileName<<show;
    return false;
  }
  if (flock(m_DriverLock, LOCK_EX | LOCK_NB) != 0) {
    merr<<"Another "<<m_DriverName<<" is running: only one can run at a time"<<show;
    return false;
  }

  // The log directory is the default, unless --logdir overwrites it for this run
  if (MFile::Exists(SettingsFileName) == true) {
    Settings.Read(SettingsFileName);
  }
  Settings.SetLogDirectory(Settings.GetDefaultLogDirectory());
  if (m_LogDirectory.IsEmpty() == true) {
    m_LogDirectory = Settings.GetLogDirectory();
  }

  // An absolute path: the tests may change their working directory
  error_code Error;
  m_LogDirectory = filesystem::absolute(m_LogDirectory.Data(), Error).lexically_normal().string().c_str();
  if (Error.value() != 0) {
    merr<<"Unable to create the log directory: "<<m_LogDirectory<<show;
    return false;
  }
  const bool Existed = MFile::IsDirectory(m_LogDirectory);
  if (MFile::CreateDirectory(m_LogDirectory) == false) {
    merr<<"Unable to create the log directory: "<<m_LogDirectory<<show;
    return false;
  }

  // A log directory which the driver makes is for the user only (the logs and the working files of the tests), one which exists already
  // keeps its permissions - it is not the driver's to change (a home, a shared directory), the user is told if others can read the logs
  if (Existed == false) {
    filesystem::permissions(m_LogDirectory.Data(), filesystem::perms::owner_all, filesystem::perm_options::replace, Error);
    if (Error.value() != 0) {
      mout<<"Warning: unable to restrict the log directory to its owner: "<<m_LogDirectory<<endl;
    }
  } else if ((filesystem::status(m_LogDirectory.Data(), Error).permissions() & (filesystem::perms::group_read | filesystem::perms::others_read)) != filesystem::perms::none) {
    mout<<"The log directory can be read by others: "<<m_LogDirectory<<endl;
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
  // The calibration runs in its own process group: the manager stops it at the time out, or if this driver is interrupted (the handlers are installed)
  MSettingsTesting Settings;
  Settings.Read();
  MProcessManager Manager(1);
  if (Manager.Start(GetCalibrationProgram(), "", m_LogDirectory + "/testdrivercalibration.log", "", GetScaledTimeout(Settings)) < 0) {
    return false;
  }
  if (Manager.WaitForAll(0.01) == false) {
    return false;
  }
  const vector<MManagedProcess> Ended = Manager.TakeEnded();
  if (Ended.size() != 1 || Ended[0].m_ExitCode != 0) {
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
  // The summaries of an earlier run of the tests which run now
  for (const MString& Test : Tests) {
    filesystem::remove((m_LogDirectory + "/" + Test + ".summary.root").Data(), Error);
    filesystem::remove((m_LogDirectory + "/" + Test + ".pdf").Data(), Error);
    filesystem::remove((m_LogDirectory + "/" + Test + ".keyresults.tsv").Data(), Error);
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


//! Write the summary report: the overview of the tests and the summary pages of the end-to-end tests as one PDF in the log directory, return its file name (empty if no test has summary pages)
MString MTestDriver::WriteSummaryReport(const vector<MString>& RequestedTests, const vector<MTestStatus>& Statuses, const vector<MString>& Metrics, const map<MString, double>& Timings) const
{
  // Collect the pages of the tests: every test which has pages wrote its canvases into a ROOT file
  vector<unique_ptr<TCanvas>> Pages;
  for (const MString& Test : RequestedTests) {
    const MString FileName = m_LogDirectory + "/" + Test + ".summary.root";
    if (MFile::Exists(FileName) == false) {
      continue;
    }
    TFile File(FileName.Data());
    if (File.IsZombie() == true) {
      continue;
    }
    TIter NextKey(File.GetListOfKeys());
    TKey* Key = nullptr;
    while ((Key = static_cast<TKey*>(NextKey())) != nullptr) {
      TCanvas* Page = dynamic_cast<TCanvas*>(Key->ReadObj());
      if (Page != nullptr) {
        Pages.push_back(unique_ptr<TCanvas>(Page));
      }
    }
  }
  if (Pages.empty() == true) {
    return "";
  }

  // The pulls of the key results of all tests together
  MEndToEndReport Combined("All tests");
  MEndToEndResultList CombinedResults;
  for (const MString& Test : RequestedTests) {
    MEndToEndResultList Results;
    Results.Read(m_LogDirectory + "/" + Test + ".keyresults.tsv");
    for (const MEndToEndResult& Result : Results.GetResults()) {
      CombinedResults.AddResult(Result);
    }
  }
  Combined.AddPullsPage(CombinedResults);

  // The overview page: every test with its result and run time
  gROOT->SetBatch(true);
  vector<unique_ptr<TCanvas>> Overview;
  const unsigned int RowsPerPage = 30;
  for (unsigned int Start = 0; Start < RequestedTests.size(); Start += RowsPerPage) {
    unique_ptr<TCanvas> Page(new TCanvas(MString("Overview") + Start, "Overview", MEndToEndReport::c_PageWidth, MEndToEndReport::c_PageHeight));
    TLatex Text;
    Text.SetNDC();
    Text.SetTextFont(62);
    Text.SetTextSize(0.035);
    Text.DrawLatex(0.02, 0.945, (m_DashboardTitle + ": overview").Data());
    Text.SetTextFont(82);
    Text.SetTextSize(0.02);
    Text.DrawLatex(0.02, 0.89, "test");
    Text.DrawLatex(0.50, 0.89, "result");
    Text.DrawLatex(0.62, 0.89, "checks");
    Text.DrawLatex(0.90, 0.89, "time");
    for (unsigned int t = Start; t < RequestedTests.size() && t < Start + RowsPerPage; ++t) {
      const double Y = 0.85 - 0.0275*(t - Start);
      if (Statuses[t] == MTestStatus::c_Passed) {
        Text.SetTextColor(kGreen + 2);
        Text.DrawLatex(0.50, Y, "passed");
      } else {
        Text.SetTextColor(kRed);
        Text.DrawLatex(0.50, Y, "FAILED");
      }
      Text.SetTextColor(kBlack);
      Text.DrawLatex(0.02, Y, RequestedTests[t].Data());
      Text.DrawLatex(0.62, Y, Metrics[t].Data());
      const auto Time = Timings.find(RequestedTests[t]);
      if (Time != Timings.end()) {
        Text.DrawLatex(0.90, Y, FormatRuntime(Time->second).Data());
      }
    }
    Overview.push_back(std::move(Page));
  }

  // One PDF with the overview first, then the pulls of all tests, then the pages of the tests
  vector<TCanvas*> All;
  for (unique_ptr<TCanvas>& Page : Overview) {
    All.push_back(Page.get());
  }
  for (TCanvas* Page : Combined.GetPages()) {
    All.push_back(Page);
  }
  for (unique_ptr<TCanvas>& Page : Pages) {
    All.push_back(Page.get());
  }
  const MString Report = m_LogDirectory + "/" + m_DriverName + "_report.pdf";
  MEndToEndReport::PrintPdf(All, Report);
  return Report;
}


////////////////////////////////////////////////////////////////////////////////


//! Run the reference campaign for the tests (the programs which are named, or all end-to-end tests with references): the runs, the analysis, and with --install the references, return the exit code of the program
int MTestDriver::RunReferenceCampaign(const vector<MString>& Names)
{
  vector<MString> Tests;
  if (GetCampaignTests(Names, Tests) == false) {
    return 1;
  }
  // Fewer runs than the minimum cannot make references: say so before the runs start, not after hours of them
  if (m_CampaignRuns < m_CampaignMinimumRuns) {
    merr<<"--runs "<<m_CampaignRuns<<" is below --minimum-runs "<<m_CampaignMinimumRuns<<": no references can be made"<<show;
    return 1;
  }

  // The speed of this machine scales the time out of the runs
  MSettingsTesting Settings;
  Settings.Read();
  if (MFile::IsExecutable(GetCalibrationProgram()) == true && IsSystemCalibrated(Settings) == false) {
    // The calibration does not receive Ctrl-C: the manager stops it if this driver is interrupted
    MProcessManager::ClearInterrupt();
    MProcessManager::InstallInterruptHandlers();
    mout<<"Measuring the speed of this machine ..."<<endl;
    const bool Calibrated = Calibrate();
    MProcessManager::RestoreInterruptHandlers();
    // An interrupted calibration is not a failed one: stop here instead of starting the campaign
    if (MProcessManager::WasInterrupted() == true) {
      mout<<"Interrupted - the calibration has been stopped"<<endl;
      return 1;
    }
    if (Calibrated == false) {
      mout<<"The calibration failed - the time out is not scaled"<<endl;
    }
    Settings.Read();
  }

  MString CampaignDirectory = m_CampaignDirectory;
  if (CampaignDirectory.IsEmpty() == true) {
    CampaignDirectory = m_LogDirectory + "/campaign";
  }
  MEndToEndCampaign Campaign(m_BinDirectory, CampaignDirectory);
  Campaign.SetNumberOfRuns(m_CampaignRuns);
  Campaign.SetMinimumRuns(m_CampaignMinimumRuns);
  Campaign.SetBaseSeed(m_CampaignSeed);
  Campaign.SetTimeout(GetScaledTimeout(Settings));
  Campaign.SetSettings(Settings);
  if (m_ReferencesDirectory.IsEmpty() == false) {
    Campaign.SetReferencesDirectory(m_ReferencesDirectory);
  }

  const vector<ETCampaignJob> Jobs = Campaign.GetJobs(Tests);
  mout<<"Reference campaign: "<<Tests.size()<<" tests, "<<m_CampaignRuns<<" runs each, "<<Jobs.size()<<" runs to do, base seed "<<m_CampaignSeed<<", directory "<<CampaignDirectory<<endl;
  if (Campaign.RunJobs(Jobs) == false) {
    return 1;
  }

  // The analysis of every test
  MString Reason = m_CampaignReason;
  if (Reason.IsEmpty() == true) {
    Reason = MString("Reference campaign of ") + m_CampaignRuns + " runs";
  }
  bool Success = true;
  for (const MString& Test: Tests) {
    MEndToEndCampaignAnalysis Analysis;
    const bool Analyzed = Campaign.Analyze(Test, Reason, Analysis);
    ReportCampaign(Test, Analysis, Campaign);
    if (Analyzed == false) {
      merr<<"The candidate references or the report of "<<Test<<" could not be written: the references are not installed"<<show;
      Success = false;
    } else if (Analysis.HasErrors() == true) {
      Success = false;
    } else if (m_CampaignInstall == true) {
      if (Campaign.Install(Test, Reason, Analysis) == true) {
        MString Target = "$(MEGALIB)/resource/endtoend";
        if (m_ReferencesDirectory.IsEmpty() == false) {
          Target = m_ReferencesDirectory;
        }
        mout<<"  Installed into "<<Target<<endl;
      } else {
        Success = false;
      }
    }
  }
  if (Success == true) {
    return 0;
  }
  return 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the tests of the reference campaign: the end-to-end tests which are named (once), or all with references, return false if a name is no end-to-end test or there are none
bool MTestDriver::GetCampaignTests(const vector<MString>& Names, vector<MString>& Tests) const
{
  if (BuildRequestedTests(Names, MTestingScope::c_EndToEnd, Tests) == false) {
    return false;
  }

  // Named tests have to be end-to-end tests, without names the end-to-end tests with references are the tests
  for (const MString& Test: Tests) {
    if (Names.empty() == false && Test.BeginsWith(m_EndToEndTestPrefix) == false) {
      merr<<"The reference campaign needs end-to-end tests ("<<m_EndToEndTestPrefix<<"*), not "<<Test<<show;
      return false;
    }
  }
  if (Names.empty() == true) {
    Tests.erase(remove_if(Tests.begin(), Tests.end(), [](const MString& Test) { return MFile::Exists(MEndToEndReferenceList::GetFileName(Test)) == false; }), Tests.end());
  }
  if (Tests.empty() == true) {
    merr<<"No end-to-end tests with references: name the tests"<<show;
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Tell about the analysis of a test of the campaign: the runs, the findings (the first warnings), and the files
void MTestDriver::ReportCampaign(const MString& Test, const MEndToEndCampaignAnalysis& Analysis, const MEndToEndCampaign& Campaign) const
{
  const unsigned int MaximumShownWarnings = 10;
  unsigned int Warnings = 0;
  mout<<endl<<Test<<": "<<Analysis.GetNumberOfRuns()<<" runs, "<<Analysis.GetObservables().size()<<" observables"<<endl;
  for (const ETCampaignFinding& Finding: Analysis.GetFindings()) {
    if (Finding.m_Severity == MCampaignSeverity::c_Error) {
      mout<<"  ERROR   "<<Finding.m_Message<<endl;
    } else if (++Warnings <= MaximumShownWarnings) {
      mout<<"  WARNING "<<Finding.m_Message<<endl;
    }
  }
  if (Warnings > MaximumShownWarnings) {
    mout<<"  ... and "<<Warnings - MaximumShownWarnings<<" more warnings in the report"<<endl;
  }
  mout<<"  Candidate references: "<<Campaign.GetCandidateFileName(Test)<<endl<<"  Report: "<<Campaign.GetReportFileName(Test)<<endl;
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
