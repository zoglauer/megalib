/*
 * UTProcessManager.cxx
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


// Standard libs:
#include <algorithm>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <type_traits>

// POSIX libs: processes and signals have no equivalent in the C++ standard library
#include <sys/wait.h>
#include <unistd.h>

// MEGAlib:
#include "MFile.h"
#include "MProcessManager.h"
#include "MSystem.h"
#include "MUnitTest.h"


//! Unit test class for MProcessManager
class UTProcessManager : public MUnitTest
{
public:
  UTProcessManager() : MUnitTest("UTProcessManager") {}
  virtual ~UTProcessManager() {}

  virtual bool Run();

private:
  bool TestRun();
  bool TestTimeouts();
  bool TestProcessGroups();
  bool TestBackground();
  bool TestManaged();
  bool TestManagedTimeout();
  bool TestParallelLimit();
  bool TestInterrupt();
  bool TestLost();
  bool TestForeignChild();
  bool TestOwnership();
  bool TestLeftBehind();

  //! A handler of SIGINT which does nothing: it is installed before the handlers of the manager, it has to be back after they are restored
  static void IgnoreInterrupt(int) {}
};


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "temp dir", "A temporary directory can be created", PrepareTemporaryDirectory()) && Passed;
  Passed = TestRun() && Passed;
  Passed = TestTimeouts() && Passed;
  Passed = TestProcessGroups() && Passed;
  Passed = TestBackground() && Passed;
  Passed = TestManaged() && Passed;
  Passed = TestManagedTimeout() && Passed;
  Passed = TestParallelLimit() && Passed;
  Passed = TestInterrupt() && Passed;
  Passed = TestLost() && Passed;
  Passed = TestForeignChild() && Passed;
  Passed = TestOwnership() && Passed;
  Passed = TestLeftBehind() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestRun()
{
  bool Passed = true;
  const MString TempDirectory = GetTemporaryDirectoryName();

  const MString ChildLog = TempDirectory + "/child.log";
  int ChildStatus = MProcessManager::Run("/bin/sh", "-c 'echo UTProcessManager child'", ChildLog);
  Passed = EvaluateTrue("Run()", "successful child", "Run returns a normal zero exit status for a successful child", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 0) && Passed;
  MString ChildLogContent = ReadTextFile(ChildLog);
  Passed = Evaluate("Run()", "redirected output", "Run redirects the output of the child to the requested file", ChildLogContent, MString("UTProcessManager child\n")) && Passed;
  const MString MissingChildLog = TempDirectory + "/missing-child.log";
  ChildStatus = MProcessManager::Run(GetTemporaryFileName("missing_child_executable"), "", MissingChildLog);
  Passed = EvaluateTrue("Run()", "missing child", "Run returns the shell command-not-found status for a missing child executable", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 127) && Passed;

  const MString Directory = TempDirectory + "/work here";
  Passed = EvaluateTrue("CreateDirectory()", "working directory", "The working directory with a space can be created", MFile::CreateDirectory(Directory)) && Passed;
  const MString WorkLog = TempDirectory + "/work.log";
  ChildStatus = MProcessManager::Run("pwd", "", WorkLog, Directory);
  Passed = EvaluateTrue("Run()", "working directory", "The program runs in the working directory", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 0 && ReadTextFile(WorkLog) == Directory + "\n") && Passed;
  ChildStatus = MProcessManager::Run("true", "", "", TempDirectory + "/does not exist");
  Passed = EvaluateTrue("Run()", "missing working directory", "A working directory which does not exist fails the program", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 126) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "child log cleanup", "The child-process log can be removed", RemoveTemporaryFile(ChildLog)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "missing child log cleanup", "The missing-child log can be removed", RemoveTemporaryFile(MissingChildLog)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestTimeouts()
{
  bool Passed = true;
  const MString TempDirectory = GetTemporaryDirectoryName();
  int ChildStatus = 0;

  const chrono::steady_clock::time_point Start = chrono::steady_clock::now();
  ChildStatus = MProcessManager::Run("sleep", "20", "", "", 1);
  const double Seconds = chrono::duration<double>(chrono::steady_clock::now() - Start).count();
  // Bounds: 1 s time out exact, upper bound of 10 s is well below the 20 s sleep and allows for a loaded machine
  Passed = EvaluateTrue("Run()", "time out", "A program which exceeds the time out of 1 s is killed by SIGKILL", WIFSIGNALED(ChildStatus) && WTERMSIG(ChildStatus) == SIGKILL && Seconds >= 1.0 && Seconds < 10.0) && Passed;

  // Without its own process group, the descendants are killed too: every parent waits, so only the time out ends them
  const MString TreeMarker = TempDirectory + "/tree_marker";
  const MString Delayed = "(sleep 3; touch " + MSystem::GetShellQuoted(TreeMarker) + ") & wait";
  ChildStatus = MProcessManager::Run("sh", MString("-c ") + MSystem::GetShellQuoted(Delayed), "", "", 1);
  Passed = EvaluateTrue("Run()", "descendant time out", "A shell which waits for a delayed descendant is ended by SIGKILL at the time out", WIFSIGNALED(ChildStatus) && WTERMSIG(ChildStatus) == SIGKILL) && Passed;
  const MString GrandMarker = TempDirectory + "/grandchild_marker";
  const MString Nested = "sh -c " + MSystem::GetShellQuoted("(sleep 3; touch " + MSystem::GetShellQuoted(GrandMarker) + ") & wait") + " & wait";
  ChildStatus = MProcessManager::Run("sh", MString("-c ") + MSystem::GetShellQuoted(Nested), "", "", 1);
  Passed = EvaluateTrue("Run()", "grandchild time out", "A nested shell is ended by SIGKILL at the time out", WIFSIGNALED(ChildStatus) && WTERMSIG(ChildStatus) == SIGKILL) && Passed;
  const MString ControlMarker = TempDirectory + "/control_marker";
  ChildStatus = MProcessManager::Run("sh", MString("-c ") + MSystem::GetShellQuoted("(sleep 1; touch " + MSystem::GetShellQuoted(ControlMarker) + ") & wait"), "", "", 10);
  Passed = EvaluateTrue("Run()", "control", "Without a time out the delayed descendant creates its file", MFile::Exists(ControlMarker)) && Passed;
  this_thread::sleep_for(chrono::milliseconds(3500));
  Passed = EvaluateFalse("Run()", "descendant", "The time out also stops the descendants of a program without its own process group", MFile::Exists(TreeMarker)) && Passed;
  Passed = EvaluateFalse("Run()", "grandchild", "The time out also stops the grandchildren", MFile::Exists(GrandMarker)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestProcessGroups()
{
  bool Passed = true;
  const MString TempDirectory = GetTemporaryDirectoryName();

  // A program in its own process group is killed with its group, also the children of the shell:
  const MString Marker = TempDirectory + "/group_marker";
  const pid_t Group = MProcessManager::StartInBackground("sh", MString("-c ") + MSystem::GetShellQuoted("sleep 3 && touch " + MSystem::GetShellQuoted(Marker) + " & wait"), "", "", true);
  Passed = EvaluateTrue("StartInBackground()", "own group", "A program with its own process group leads that group", Group > 0 && getpgid(Group) == Group) && Passed;
  MProcessManager::WaitForBackground(Group, 1);
  this_thread::sleep_for(chrono::milliseconds(3500));
  Passed = EvaluateFalse("WaitForBackground()", "process group", "The time out also stops the processes which a program with its own group started", MFile::Exists(Marker)) && Passed;

  // Without an own group the program stays in the group of the caller
  const pid_t Same = MProcessManager::StartInBackground("sleep", "20");
  Passed = EvaluateTrue("StartInBackground()", "same group", "A program without OwnProcessGroup stays in the process group of the caller", Same > 0 && getpgid(Same) == getpgrp()) && Passed;
  kill(Same, SIGKILL);
  waitpid(Same, nullptr, 0);

  // The programs which a process starts die with the group of that process: a stand-in for a test and the programs it starts
  {
    const MString NestedFile = TempDirectory + "/nested_pid.txt";
    const pid_t Middle = fork();
    if (Middle == 0) {
      setpgid(0, 0);
      const pid_t Nested = MProcessManager::StartInBackground("sleep", "30");
      {
        ofstream Out(NestedFile.Data());
        Out<<Nested<<endl;
      }
      sleep(60);
      _exit(0);
    }
    setpgid(Middle, Middle);
    MString NestedText;
    for (unsigned int Attempt = 0; Attempt < 50 && NestedText.IsEmpty() == true; ++Attempt) {
      this_thread::sleep_for(chrono::milliseconds(100));
      MFile::ReadTextFile(NestedFile, NestedText);
    }
    const pid_t Nested = static_cast<pid_t>(atoi(NestedText.Data()));
    Passed = EvaluateTrue("StartInBackground()", "nested", "The stand-in test started a program", Nested > 0 && kill(Nested, 0) == 0) && Passed;
    kill(-Middle, SIGKILL);
    waitpid(Middle, nullptr, 0);
    bool Gone = false;
    for (unsigned int Attempt = 0; Attempt < 50 && Gone == false; ++Attempt) {
      this_thread::sleep_for(chrono::milliseconds(100));
      if (kill(Nested, 0) != 0) {
        Gone = true;
      }
    }
    Passed = EvaluateTrue("StartInBackground()", "nested killed", "Stopping the process group of a test also stops the programs it started", Gone) && Passed;
    if (Gone == false) {
      kill(Nested, SIGKILL);
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestBackground()
{
  bool Passed = true;
  int ChildStatus = 0;

  const pid_t Background = MProcessManager::StartInBackground("sh", "-c 'exit 3'");
  Passed = EvaluateTrue("StartInBackground()", "started", "A background process can be started", Background > 0) && Passed;
  ChildStatus = MProcessManager::WaitForBackground(Background);
  Passed = EvaluateTrue("WaitForBackground()", "status", "The wait returns the exit status of the background process", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 3) && Passed;

  volatile sig_atomic_t Stop = 0;
  const pid_t Long = MProcessManager::StartInBackground("sleep", "20");
  Stop = 1;
  ChildStatus = MProcessManager::WaitForBackground(Long, 0, &Stop);
  Passed = EvaluateTrue("WaitForBackground()", "stop flag", "A set stop flag kills the background process with SIGKILL", WIFSIGNALED(ChildStatus) && WTERMSIG(ChildStatus) == SIGKILL) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestManaged()
{
  bool Passed = true;
  const MString TempDirectory = GetTemporaryDirectoryName();

  // Two programs at most: a third is refused until one ended
  MProcessManager Manager(2);
  Passed = EvaluateTrue("CanStart()", "fresh", "A fresh manager can start a program", Manager.CanStart()) && Passed;
  Passed = Evaluate("GetNumberOfRunning()", "fresh", "Nothing runs", Manager.GetNumberOfRunning(), 0U) && Passed;
  const MString Directory = TempDirectory + "/managed work";
  Passed = EvaluateTrue("CreateDirectory()", "working directory", "The working directory can be created", MFile::CreateDirectory(Directory)) && Passed;
  const int First = Manager.Start("sh", "-c 'pwd; echo managed'", TempDirectory + "/first.log", Directory, 10.0);
  const int Second = Manager.Start("sh", "-c 'exit 3'", TempDirectory + "/second.log", "", 10.0);
  Passed = EvaluateTrue("Start()", "two", "Two programs can be started", First >= 0 && Second >= 0 && First != Second) && Passed;
  Passed = EvaluateFalse("CanStart()", "full", "With the maximum number running nothing more can be started", Manager.CanStart()) && Passed;
  DisableDefaultStreams();
  const int Third = Manager.Start("true", "", "", "", 10.0);
  EnableDefaultStreams();
  Passed = Evaluate("Start()", "refused", "A third program is refused", Third, -1) && Passed;
  Passed = Evaluate("GetNumberOfRunning()", "two", "Two programs run", Manager.GetNumberOfRunning(), 2U) && Passed;

  Passed = EvaluateTrue("WaitForAll()", "ended", "Both programs end by themselves", Manager.WaitForAll(0.02)) && Passed;
  Passed = EvaluateFalse("HasRunning()", "ended", "Nothing runs after the wait", Manager.HasRunning()) && Passed;
  Passed = EvaluateTrue("CanStart()", "ended", "A program can be started again", Manager.CanStart()) && Passed;
  const vector<MManagedProcess> Ended = Manager.TakeEnded();
  Passed = EvaluateSize("TakeEnded()", "two", "Both programs are reported as ended", Ended.size(), 2) && Passed;
  for (const MManagedProcess& Process: Ended) {
    if (static_cast<int>(Process.m_Id) == First) {
      Passed = Evaluate("TakeEnded()", "first exit code", "The first program ended with 0", Process.m_ExitCode, 0) && Passed;
      Passed = EvaluateTrue("TakeEnded()", "first status", "The first program ended normally", WIFEXITED(Process.m_Status) != 0) && Passed;
      Passed = Evaluate("TakeEnded()", "first executable", "The record names the program", Process.m_Executable, MString("sh")) && Passed;
      Passed = EvaluateTrue("TakeEnded()", "first seconds", "The seconds are measured", Process.m_Seconds >= 0.0 && Process.m_Seconds < 10.0) && Passed;
      Passed = EvaluateFalse("TakeEnded()", "first flags", "The first program neither timed out nor was lost", Process.m_TimedOut || Process.m_Lost) && Passed;
    } else {
      Passed = Evaluate("TakeEnded()", "second exit code", "The second program ended with 3", Process.m_ExitCode, 3) && Passed;
    }
  }
  Passed = Evaluate("TakeEnded()", "first output", "The output of the first program went into its file, and it ran in the working directory", ReadTextFile(TempDirectory + "/first.log"), Directory + "\nmanaged\n") && Passed;
  Passed = EvaluateSize("TakeEnded()", "taken", "The ended programs are reported once", Manager.TakeEnded().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestManagedTimeout()
{
  bool Passed = true;
  const MString TempDirectory = GetTemporaryDirectoryName();

  // A program which runs too long is stopped with its whole process group: its child goes with it
  const MString ChildFile = TempDirectory + "/timeout_child.txt";
  MProcessManager Manager(1);
  const MString Script = "sleep 30 & echo $! > " + MSystem::GetShellQuoted(ChildFile) + "; wait";
  const int Id = Manager.Start("sh", MString("-c ") + MSystem::GetShellQuoted(Script), "", "", 1.0);
  Passed = EvaluateTrue("Start()", "slow", "The slow program can be started", Id >= 0) && Passed;
  Passed = EvaluateTrue("WaitForAll()", "stopped", "The program is stopped at its time out and the wait ends", Manager.WaitForAll(0.02)) && Passed;
  const vector<MManagedProcess> Ended = Manager.TakeEnded();
  Passed = EvaluateSize("TakeEnded()", "stopped", "The stopped program is reported", Ended.size(), 1) && Passed;
  if (Ended.size() == 1) {
    Passed = EvaluateTrue("TakeEnded()", "timed out", "The program is marked as timed out", Ended[0].m_TimedOut) && Passed;
    Passed = EvaluateTrue("TakeEnded()", "killed", "The program was ended by SIGKILL", WIFSIGNALED(Ended[0].m_Status) != 0 && WTERMSIG(Ended[0].m_Status) == SIGKILL) && Passed;
    Passed = Evaluate("TakeEnded()", "no exit code", "A program which was killed has no exit code", Ended[0].m_ExitCode, -1) && Passed;
    // The time out is one second, the poll interval 20 ms: the program ran between 1 and 2 seconds (coarse, this machine may be loaded)
    Passed = EvaluateTrue("TakeEnded()", "seconds", "The program ran for about its time out", Ended[0].m_Seconds >= 1.0 && Ended[0].m_Seconds < 5.0) && Passed;
  }
  const MString ChildText = ReadTextFile(ChildFile);
  const pid_t Child = static_cast<pid_t>(atoi(ChildText.Data()));
  Passed = EvaluateTrue("Start()", "child known", "The program wrote the process id of its child", Child > 0) && Passed;
  bool ChildGone = false;
  for (unsigned int Attempt = 0; Attempt < 50 && ChildGone == false; ++Attempt) {
    if (kill(Child, 0) != 0) {
      ChildGone = true;
    } else {
      this_thread::sleep_for(chrono::milliseconds(100));
    }
  }
  Passed = EvaluateTrue("Poll()", "child stopped", "The child of the stopped program is gone with its process group", ChildGone) && Passed;
  if (ChildGone == false) {
    kill(Child, SIGKILL);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestParallelLimit()
{
  bool Passed = true;
  const MString TempDirectory = GetTemporaryDirectoryName();

  // Five programs with a limit of two: every program leaves a marker while it runs, never more than two markers exist at the same time
  const MString Markers = TempDirectory + "/markers";
  Passed = EvaluateTrue("CreateDirectory()", "markers", "The directory for the markers can be created", MFile::CreateDirectory(Markers)) && Passed;
  MProcessManager Manager(2);
  unsigned int Started = 0;
  unsigned int MostAtOnce = 0;
  unsigned int Ended = 0;
  while (Started < 5 || Manager.HasRunning() == true) {
    while (Started < 5 && Manager.CanStart() == true) {
      const MString Script = "touch " + MSystem::GetShellQuoted(Markers + "/running." + Started) + "; sleep 0.3; rm " + MSystem::GetShellQuoted(Markers + "/running." + Started);
      Passed = EvaluateTrue("Start()", MString("program ") + Started, "The program can be started", Manager.Start("sh", MString("-c ") + MSystem::GetShellQuoted(Script), "", "", 10.0) >= 0) && Passed;
      ++Started;
    }
    unsigned int AtOnce = 0;
    for (unsigned int p = 0; p < 5; ++p) {
      if (MFile::Exists(Markers + "/running." + p) == true) {
        ++AtOnce;
      }
    }
    MostAtOnce = max(MostAtOnce, AtOnce);
    Manager.Poll();
    Ended += Manager.TakeEnded().size();
    this_thread::sleep_for(chrono::milliseconds(10));
  }
  Passed = Evaluate("Start()", "all ended", "All five programs ended", Ended, 5U) && Passed;
  Passed = EvaluateTrue("CanStart()", "at most two", "Never more than two programs ran at the same time", MostAtOnce <= 2) && Passed;
  Passed = EvaluateTrue("CanStart()", "two at once", "Two programs did run at the same time", MostAtOnce == 2) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestInterrupt()
{
  bool Passed = true;

  // The handlers of the manager are installed in front of the ones which exist, and those are back after the restore
  signal(SIGINT, IgnoreInterrupt);
  MProcessManager::ClearInterrupt();
  Passed = EvaluateFalse("WasInterrupted()", "cleared", "No interrupt after the flag was cleared", MProcessManager::WasInterrupted()) && Passed;
  MProcessManager::InstallInterruptHandlers();
  MProcessManager Manager(1);
  const int Id = Manager.Start("sleep", "30", "", "", 0.0);
  Passed = EvaluateTrue("Start()", "long", "The long program can be started", Id >= 0) && Passed;
  // The signal to this program sets the flag, the wait stops everything
  raise(SIGTERM);
  Passed = EvaluateTrue("WasInterrupted()", "raised", "SIGTERM sets the flag", MProcessManager::WasInterrupted()) && Passed;
  Passed = EvaluateFalse("WaitForAll()", "interrupted", "The wait returns false after the interrupt", Manager.WaitForAll(0.02)) && Passed;
  Passed = EvaluateFalse("HasRunning()", "interrupted", "Nothing runs after the interrupt", Manager.HasRunning()) && Passed;
  const vector<MManagedProcess> Ended = Manager.TakeEnded();
  Passed = EvaluateSize("TakeEnded()", "interrupted", "The stopped program is reported", Ended.size(), 1) && Passed;
  if (Ended.size() == 1) {
    Passed = EvaluateTrue("StopAll()", "killed", "The program was ended by SIGKILL", WIFSIGNALED(Ended[0].m_Status) != 0 && WTERMSIG(Ended[0].m_Status) == SIGKILL) && Passed;
    Passed = EvaluateFalse("StopAll()", "not timed out", "An interrupted program did not time out", Ended[0].m_TimedOut) && Passed;
  }
  MProcessManager::ClearInterrupt();
  Passed = EvaluateFalse("ClearInterrupt()", "cleared again", "The flag is cleared", MProcessManager::WasInterrupted()) && Passed;
  MProcessManager::RestoreInterruptHandlers();
  void (*Restored)(int) = signal(SIGINT, SIG_DFL);
  Passed = EvaluateTrue("RestoreInterruptHandlers()", "old handler", "The handler of SIGINT which was installed before is back", Restored == IgnoreInterrupt) && Passed;
  signal(SIGTERM, SIG_DFL);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestLost()
{
  bool Passed = true;

  // With SIGCHLD ignored the system reaps the program itself: it cannot be waited for and is reported as lost
  void (*OldChild)(int) = signal(SIGCHLD, SIG_IGN);
  MProcessManager Manager(1);
  const int Id = Manager.Start("true", "", "", "", 10.0);
  Passed = EvaluateTrue("Start()", "lost", "The program can be started", Id >= 0) && Passed;
  DisableDefaultStreams();
  const bool Waited = Manager.WaitForAll(0.02);
  EnableDefaultStreams();
  signal(SIGCHLD, OldChild);
  Passed = EvaluateFalse("WaitForAll()", "lost", "A program which cannot be waited for ends the wait with false", Waited) && Passed;
  const vector<MManagedProcess> Ended = Manager.TakeEnded();
  Passed = EvaluateSize("TakeEnded()", "lost", "The lost program is reported", Ended.size(), 1) && Passed;
  if (Ended.size() == 1) {
    Passed = EvaluateTrue("TakeEnded()", "lost flag", "The program is marked as lost", Ended[0].m_Lost) && Passed;
    Passed = Evaluate("TakeEnded()", "lost status", "A lost program has no status", Ended[0].m_Status, -1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestForeignChild()
{
  bool Passed = true;

  // A child which this test started itself is not reaped by the manager: it is still there to be waited for afterwards
  const pid_t Foreign = fork();
  if (Foreign == 0) {
    _exit(7);
  }
  Passed = EvaluateTrue("fork()", "foreign child", "The foreign child can be started", Foreign > 0) && Passed;
  this_thread::sleep_for(chrono::milliseconds(200));
  MProcessManager Manager(1);
  Manager.Start("true", "", "", "", 10.0);
  Passed = EvaluateTrue("WaitForAll()", "own program", "The own program ends", Manager.WaitForAll(0.02)) && Passed;
  int Status = 0;
  const pid_t Waited = waitpid(Foreign, &Status, 0);
  Passed = EvaluateTrue("Poll()", "foreign child kept", "The foreign child was not reaped by the manager: it can be waited for with its exit code 7", Waited == Foreign && WIFEXITED(Status) != 0 && WEXITSTATUS(Status) == 7) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestOwnership()
{
  bool Passed = true;

  // A manager owns its processes: a copy would stop and reap the processes of the original, which then reports them as lost
  Passed = EvaluateFalse("MProcessManager", "copy", "A manager cannot be copied", is_copy_constructible<MProcessManager>::value) && Passed;
  Passed = EvaluateFalse("MProcessManager", "assign", "A manager cannot be assigned", is_copy_assignable<MProcessManager>::value) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTProcessManager::TestLeftBehind()
{
  bool Passed = true;
  const MString TempDirectory = GetTemporaryDirectoryName();

  // A program which leaves a child behind and ends by itself: the child is stopped with the process group when the program is reaped
  const MString ChildFile = TempDirectory + "/left_behind_child.txt";
  MProcessManager Manager(1);
  const MString Script = "sleep 30 & echo $! > " + MSystem::GetShellQuoted(ChildFile);
  const int Id = Manager.Start("sh", MString("-c ") + MSystem::GetShellQuoted(Script), "", "", 10.0);
  Passed = EvaluateTrue("Start()", "leaving", "The program which leaves a child behind can be started", Id >= 0) && Passed;
  Passed = EvaluateTrue("WaitForAll()", "leaving", "The program ends by itself", Manager.WaitForAll(0.02)) && Passed;
  const vector<MManagedProcess> Ended = Manager.TakeEnded();
  Passed = EvaluateSize("TakeEnded()", "leaving", "The program is reported", Ended.size(), 1) && Passed;
  if (Ended.size() == 1) {
    Passed = Evaluate("TakeEnded()", "leaving exit code", "The program ended normally with 0", Ended[0].m_ExitCode, 0) && Passed;
    Passed = EvaluateFalse("TakeEnded()", "leaving flags", "The program neither timed out nor was lost", Ended[0].m_TimedOut || Ended[0].m_Lost) && Passed;
  }
  const pid_t Child = static_cast<pid_t>(atoi(ReadTextFile(ChildFile).Data()));
  Passed = EvaluateTrue("Start()", "left child known", "The program wrote the process id of the child it left behind", Child > 0) && Passed;
  bool ChildGone = false;
  for (unsigned int Attempt = 0; Attempt < 50 && ChildGone == false; ++Attempt) {
    if (kill(Child, 0) != 0) {
      ChildGone = true;
    } else {
      this_thread::sleep_for(chrono::milliseconds(100));
    }
  }
  Passed = EvaluateTrue("Poll()", "left child stopped", "The child which the ended program left behind is stopped with its process group", ChildGone) && Passed;
  if (ChildGone == false) {
    kill(Child, SIGKILL);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTProcessManager Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTProcessManager.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
