/*
 * UTSystem.cxx
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
#include <thread>
#include <chrono>
#include <csignal>
#include <ctime>

// POSIX libs:
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

// ROOT libs:
#include "TError.h"

// MEGAlib:
#include "MFile.h"
#include "MSystem.h"
#include "MUnitTest.h"


//! Unit test class for MSystem
class UTSystem : public MUnitTest
{
public:
  UTSystem() : MUnitTest("UTSystem") {}
  virtual ~UTSystem() {}

  virtual bool Run();

private:
};


////////////////////////////////////////////////////////////////////////////////


bool UTSystem::Run()
{
  bool Passed = true;

  const MString TempDirectory = GetTemporaryDirectoryName();
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "temp dir", "A temporary directory can be created for MSystem tests", PrepareTemporaryDirectory()) && Passed;
  if (MFile::CreateDirectory(TempDirectory) == false) {
    Summarize();
    return false;
  }

  const MString FileName = TempDirectory + "/example.geo.setup";
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The representative file for MSystem tests can be written", WriteTextFile(FileName, "UTSystem")) && Passed;

  MSystem System;

  MString Suffix;
  Passed = EvaluateTrue("GetFileSuffix()", "suffix", "GetFileSuffix returns the suffix from a representative MEGAlib-style filename", System.GetFileSuffix(FileName, &Suffix)) && Passed;
  Passed = Evaluate("GetFileSuffix()", "suffix value", "GetFileSuffix extracts the representative suffix", Suffix, MString("setup")) && Passed;

  Passed = EvaluateTrue("FileExist()", "existing file", "FileExist reports the representative file as existing", System.FileExist(FileName)) && Passed;
  Passed = EvaluateFalse("FileExist()", "missing file", "FileExist reports a missing file as absent", System.FileExist(TempDirectory + "/does_not_exist")) && Passed;

  const MString ChildLog = TempDirectory + "/child.log";
  int ChildStatus = MSystem::RunProcess("/bin/sh", "-c 'echo UTSystem child'", ChildLog);
  Passed = EvaluateTrue("RunProcess()", "successful child", "RunProcess returns a normal zero exit status for a successful child", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 0) && Passed;
  MString ChildLogContent = ReadTextFile(ChildLog);
  Passed = EvaluateTrue("RunProcess()", "redirected output", "RunProcess redirects child output to the requested file", ChildLogContent.IsEmpty() == false) && Passed;
  Passed = EvaluateTrue("RunProcess()", "redirected output", "The redirected child output contains the expected marker", ChildLogContent.Contains("UTSystem child")) && Passed;
  const MString MissingChildLog = TempDirectory + "/missing-child.log";
  ChildStatus = MSystem::RunProcess(GetTemporaryFileName("missing_child_executable"), "", MissingChildLog);
  Passed = EvaluateTrue("RunProcess()", "missing child", "RunProcess returns the shell command-not-found status for a missing child executable", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 127) && Passed;

  // Working directory, time out, background processes
  {
    const MString Directory = TempDirectory + "/work here";
    Passed = EvaluateTrue("CreateDirectory()", "working directory", "The working directory with a space can be created", MFile::CreateDirectory(Directory)) && Passed;
    const MString WorkLog = TempDirectory + "/work.log";
    ChildStatus = MSystem::RunProcess("pwd", "", WorkLog, Directory);
    Passed = EvaluateTrue("RunProcess()", "working directory", "The program runs in the working directory", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 0 && ReadTextFile(WorkLog).Contains("work here")) && Passed;
    ChildStatus = MSystem::RunProcess("true", "", "", TempDirectory + "/does not exist");
    Passed = EvaluateTrue("RunProcess()", "missing working directory", "A working directory which does not exist fails the program", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 126) && Passed;

    const chrono::steady_clock::time_point Start = chrono::steady_clock::now();
    ChildStatus = MSystem::RunProcess("sleep", "20", "", "", 1);
    const double Seconds = chrono::duration<double>(chrono::steady_clock::now() - Start).count();
    Passed = EvaluateTrue("RunProcess()", "time out", "A program which exceeds the time out is killed", WIFSIGNALED(ChildStatus) && Seconds < 10.0) && Passed;

    // Without its own process group, the descendants are killed too: every parent waits, so only the time out ends them
    const MString TreeMarker = TempDirectory + "/tree_marker";
    const MString Delayed = "(sleep 3; touch " + MSystem::GetShellQuoted(TreeMarker) + ") & wait";
    ChildStatus = MSystem::RunProcess("sh", MString("-c ") + MSystem::GetShellQuoted(Delayed), "", "", 1);
    Passed = EvaluateTrue("RunProcess()", "descendant time out", "A shell which waits for a delayed descendant is ended by the time out", WIFSIGNALED(ChildStatus)) && Passed;
    const MString GrandMarker = TempDirectory + "/grandchild_marker";
    const MString Nested = "sh -c " + MSystem::GetShellQuoted("(sleep 3; touch " + MSystem::GetShellQuoted(GrandMarker) + ") & wait") + " & wait";
    ChildStatus = MSystem::RunProcess("sh", MString("-c ") + MSystem::GetShellQuoted(Nested), "", "", 1);
    Passed = EvaluateTrue("RunProcess()", "grandchild time out", "A nested shell is ended by the time out", WIFSIGNALED(ChildStatus)) && Passed;
    const MString ControlMarker = TempDirectory + "/control_marker";
    ChildStatus = MSystem::RunProcess("sh", MString("-c ") + MSystem::GetShellQuoted("(sleep 1; touch " + MSystem::GetShellQuoted(ControlMarker) + ") & wait"), "", "", 10);
    Passed = EvaluateTrue("RunProcess()", "control", "Without a time out the delayed descendant creates its file", MFile::Exists(ControlMarker)) && Passed;
    this_thread::sleep_for(chrono::milliseconds(3500));
    Passed = EvaluateFalse("RunProcess()", "descendant", "The time out also stops the descendants of a program without its own process group", MFile::Exists(TreeMarker)) && Passed;
    Passed = EvaluateFalse("RunProcess()", "grandchild", "The time out also stops the grandchildren", MFile::Exists(GrandMarker)) && Passed;

    // A program in its own process group is killed with its group, also the children of the shell:
    const MString Marker = TempDirectory + "/group_marker";
    const pid_t Group = MSystem::StartProcessInBackground("sh", MString("-c ") + MSystem::GetShellQuoted("sleep 3 && touch " + MSystem::GetShellQuoted(Marker) + " & wait"), "", "", true);
    Passed = EvaluateTrue("StartProcessInBackground()", "own group", "A program with its own process group leads that group", Group > 0 && getpgid(Group) == Group) && Passed;
    MSystem::WaitForBackgroundProcess(Group, 1);
    this_thread::sleep_for(chrono::milliseconds(3500));
    Passed = EvaluateFalse("WaitForBackgroundProcess()", "process group", "The time out also stops the processes which a program with its own group started", MFile::Exists(Marker)) && Passed;

    // Without an own group the program stays in the group of the caller
    const pid_t Same = MSystem::StartProcessInBackground("sleep", "20");
    Passed = EvaluateTrue("StartProcessInBackground()", "same group", "A program without OwnProcessGroup stays in the process group of the caller", Same > 0 && getpgid(Same) == getpgrp()) && Passed;
    kill(Same, SIGKILL);
    waitpid(Same, nullptr, 0);

    // The programs which a process starts die with the group of that process: a stand-in for a test and the programs it starts
    {
      const MString NestedFile = TempDirectory + "/nested_pid.txt";
      const pid_t Middle = fork();
      if (Middle == 0) {
        setpgid(0, 0);
        const pid_t Nested = MSystem::StartProcessInBackground("sleep", "30");
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
      Passed = EvaluateTrue("StartProcessInBackground()", "nested", "The stand-in test started a program", Nested > 0 && kill(Nested, 0) == 0) && Passed;
      kill(-Middle, SIGKILL);
      waitpid(Middle, nullptr, 0);
      bool Gone = false;
      for (unsigned int Attempt = 0; Attempt < 50 && Gone == false; ++Attempt) {
        this_thread::sleep_for(chrono::milliseconds(100));
        if (kill(Nested, 0) != 0) {
          Gone = true;
        }
      }
      Passed = EvaluateTrue("StartProcessInBackground()", "nested killed", "Stopping the process group of a test also stops the programs it started", Gone) && Passed;
      if (Gone == false) {
        kill(Nested, SIGKILL);
      }
    }

    const pid_t Background = MSystem::StartProcessInBackground("sh", "-c 'exit 3'");
    Passed = EvaluateTrue("StartProcessInBackground()", "started", "A background process can be started", Background > 0) && Passed;
    ChildStatus = MSystem::WaitForBackgroundProcess(Background);
    Passed = EvaluateTrue("WaitForBackgroundProcess()", "status", "The wait returns the exit status of the background process", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 3) && Passed;

    volatile sig_atomic_t Stop = 0;
    const pid_t Long = MSystem::StartProcessInBackground("sleep", "20");
    Stop = 1;
    ChildStatus = MSystem::WaitForBackgroundProcess(Long, 0, &Stop);
    Passed = EvaluateTrue("WaitForBackgroundProcess()", "stop flag", "A set stop flag kills the background process", WIFSIGNALED(ChildStatus)) && Passed;
  }

  // GetCpuModel
  {
    const MString Cpu = MSystem::GetCpuModel();
#ifdef __linux__
    Passed = EvaluateTrue("GetCpuModel()", "model", "The model of the CPU is known on Linux", Cpu.IsEmpty() == false) && Passed;
    Passed = EvaluateFalse("GetCpuModel()", "label", "The model of the CPU is the value without the label of /proc/cpuinfo", Cpu.Contains("model name")) && Passed;
#endif
    Passed = Evaluate("GetCpuModel()", "stable", "The model of the CPU is the same on every call", MSystem::GetCpuModel(), Cpu) && Passed;
  }

  // GetShellQuoted: the shell sees exactly the argument, whatever it contains
  Passed = Evaluate("GetShellQuoted()", "plain", "A plain word is put in single quotes", MSystem::GetShellQuoted("word"), MString("'word'")) && Passed;
  Passed = Evaluate("GetShellQuoted()", "empty", "The empty string gives two single quotes: an empty word", MSystem::GetShellQuoted(""), MString("''")) && Passed;
  Passed = Evaluate("GetShellQuoted()", "single quote", "A single quote is closed, escaped, and opened again", MSystem::GetShellQuoted("it's"), MString("'it'\\''s'")) && Passed;
  const vector<MString> Arguments = { "plain", "with space", "it's", "$HOME `date` \"double\" ; & | > < * ? \\ !", "two  spaces", "'", "''", "a\nb" };
  for (const MString& Argument: Arguments) {
    const MString EchoLog = TempDirectory + "/quoted.log";
    ChildStatus = MSystem::RunProcess("printf", MString("%s ") + MSystem::GetShellQuoted(Argument), EchoLog);
    Passed = EvaluateTrue("GetShellQuoted()", Argument, "The shell passes the quoted argument on unchanged", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 0 && ReadTextFile(EchoLog) == Argument) && Passed;
  }
  const MString SpaceDirectory = TempDirectory + "/a directory with spaces";
  const MString SpaceFile = SpaceDirectory + "/a file's name";
  Passed = EvaluateTrue("CreateDirectory()", "spaces", "The directory with spaces can be created", MFile::CreateDirectory(SpaceDirectory)) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "spaces", "The file with spaces and a single quote in its name can be written", WriteTextFile(SpaceFile, "UTSystem spaces")) && Passed;
  const MString SpaceLog = TempDirectory + "/spaces.log";
  ChildStatus = MSystem::RunProcess("cat", MSystem::GetShellQuoted(SpaceFile), SpaceLog);
  Passed = EvaluateTrue("GetShellQuoted()", "path", "A quoted path with spaces and a single quote is one argument", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 0 && ReadTextFile(SpaceLog).Contains("UTSystem spaces")) && Passed;

  int Free = -1;
  int ErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  DisableDefaultStreams();
  bool HasFreeMemory = System.FreeMemory(Free);
  int RAM = System.GetRAM();
  int FreeRAM = System.GetFreeRAM();
  int Swap = System.GetSwap();
  int FreeSwap = System.GetFreeSwap();
  EnableDefaultStreams();
  gErrorIgnoreLevel = ErrorIgnoreLevel;
  if (HasFreeMemory == true) {
    Passed = EvaluateTrue("FreeMemory()", "free mem value", "FreeMemory returns a non-negative amount of free memory when platform memory statistics are available", Free >= 0) && Passed;
    Passed = EvaluateTrue("GetRAM()", "ram", "GetRAM returns a non-negative amount of installed memory when platform memory statistics are available", RAM >= 0) && Passed;
    Passed = EvaluateTrue("GetFreeRAM()", "free ram", "GetFreeRAM returns a non-negative amount of free memory when platform memory statistics are available", FreeRAM >= 0) && Passed;
    Passed = EvaluateTrue("GetSwap()", "swap", "GetSwap returns a non-negative amount of installed swap when platform memory statistics are available", Swap >= 0) && Passed;
    Passed = EvaluateTrue("GetFreeSwap()", "free swap", "GetFreeSwap returns a non-negative amount of free swap when platform memory statistics are available", FreeSwap >= 0) && Passed;
  } else {
    Passed = Evaluate("FreeMemory()", "free mem value", "FreeMemory returns -1 when platform memory statistics are unavailable", Free, -1) && Passed;
    Passed = Evaluate("GetRAM()", "ram", "GetRAM returns -1 when platform memory statistics are unavailable", RAM, -1) && Passed;
    Passed = Evaluate("GetFreeRAM()", "free ram", "GetFreeRAM returns -1 when platform memory statistics are unavailable", FreeRAM, -1) && Passed;
    Passed = Evaluate("GetSwap()", "swap", "GetSwap returns -1 when platform memory statistics are unavailable", Swap, -1) && Passed;
    Passed = Evaluate("GetFreeSwap()", "free swap", "GetFreeSwap returns -1 when platform memory statistics are unavailable", FreeSwap, -1) && Passed;
  }

  long int Seconds = 0;
  long int NanoSeconds = 0;
  time_t Now = time(nullptr);
  Passed = EvaluateTrue("GetTime()", "current time", "GetTime returns the current system time", MSystem::GetTime(Seconds, NanoSeconds)) && Passed;
  Passed = EvaluateTrue("GetTime()", "seconds", "GetTime returns a timestamp close to the current second", Seconds >= Now - 1 && Seconds <= Now + 1) && Passed;
  Passed = EvaluateTrue("GetTime()", "nanoseconds", "GetTime returns a nanosecond component within the valid range", NanoSeconds >= 0 && NanoSeconds < 1000000000L) && Passed;

  timeval StartTime{};
  timeval EndTime{};
  gettimeofday(&StartTime, nullptr);
  MSystem::BusyWait(5000);
  gettimeofday(&EndTime, nullptr);
  long long ElapsedMicroseconds = (static_cast<long long>(EndTime.tv_sec) - static_cast<long long>(StartTime.tv_sec))*1000000LL
                                + (static_cast<long long>(EndTime.tv_usec) - static_cast<long long>(StartTime.tv_usec));
  Passed = EvaluateTrue("BusyWait()", "elapsed", "BusyWait waits at least the requested time", ElapsedMicroseconds >= 3000LL) && Passed;

  Passed = EvaluateTrue("RemoveTemporaryFile()", "cleanup", "The representative MSystem file can be removed", RemoveTemporaryFile(FileName)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "child log cleanup", "The representative child-process log can be removed", RemoveTemporaryFile(ChildLog)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "missing child log cleanup", "The representative missing-child log can be removed", RemoveTemporaryFile(MissingChildLog)) && Passed;
  Passed = EvaluateFalse("FileExist()", "cleanup", "The representative MSystem file is gone after cleanup", System.FileExist(FileName)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "temp cleanup", "The temporary MSystem directory can be removed", RemoveTemporaryDirectory(TempDirectory)) && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSystem Test;
  return Test.Run() == true ? 0 : 1;
}
