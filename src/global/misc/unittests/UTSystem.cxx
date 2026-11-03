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
#include <algorithm>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

// POSIX libs:
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

// ROOT libs:
#include "TError.h"

// MEGAlib:
#include "MFile.h"
#include "MProcessManager.h"
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

  // GetCpuModel
  {
    const MString Cpu = MSystem::GetCpuModel();
#ifdef __linux__
    Passed = EvaluateTrue("GetCpuModel()", "model", "The model of the CPU is known on Linux", Cpu.IsEmpty() == false) && Passed;
    Passed = EvaluateFalse("GetCpuModel()", "label", "The model of the CPU is the value without the label of /proc/cpuinfo", Cpu.Contains("model name")) && Passed;
#endif
    Passed = Evaluate("GetCpuModel()", "stable", "The model of the CPU is the same on every call", MSystem::GetCpuModel(), Cpu) && Passed;
  }

  // GetOS: the same as the output of "uname -sr"
  {
    const MString UnameFile = GetTemporaryFileName("uname.txt");
    const int UnameStatus = MProcessManager::Run("uname", "-sr", UnameFile);
    Passed = EvaluateTrue("RunProcess()", "uname", "The reference command uname -sr runs", WIFEXITED(UnameStatus) && WEXITSTATUS(UnameStatus) == 0) && Passed;
    MString Reference = ReadTextFile(UnameFile);
    Reference.TrimInPlace();
    Passed = Evaluate("GetOS()", "uname", "The OS is the output of uname -sr (kernel name and release)", MSystem::GetOS(), Reference) && Passed;
  }

  // GetShellQuoted: the shell sees exactly the argument, whatever it contains
  Passed = Evaluate("GetShellQuoted()", "plain", "A plain word is put in single quotes", MSystem::GetShellQuoted("word"), MString("'word'")) && Passed;
  Passed = Evaluate("GetShellQuoted()", "empty", "The empty string gives two single quotes: an empty word", MSystem::GetShellQuoted(""), MString("''")) && Passed;
  Passed = Evaluate("GetShellQuoted()", "single quote", "A single quote is closed, escaped, and opened again", MSystem::GetShellQuoted("it's"), MString("'it'\\''s'")) && Passed;
  const vector<MString> Arguments = { "plain", "with space", "it's", "$HOME `date` \"double\" ; & | > < * ? \\ !", "two  spaces", "'", "''", "a\nb" };
  for (const MString& Argument: Arguments) {
    const MString EchoLog = TempDirectory + "/quoted.log";
    const int ChildStatus = MProcessManager::Run("printf", MString("%s ") + MSystem::GetShellQuoted(Argument), EchoLog);
    Passed = EvaluateTrue("GetShellQuoted()", Argument, "The shell passes the quoted argument on unchanged", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 0 && ReadTextFile(EchoLog) == Argument) && Passed;
  }
  const MString SpaceDirectory = TempDirectory + "/a directory with spaces";
  const MString SpaceFile = SpaceDirectory + "/a file's name";
  Passed = EvaluateTrue("CreateDirectory()", "spaces", "The directory with spaces can be created", MFile::CreateDirectory(SpaceDirectory)) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "spaces", "The file with spaces and a single quote in its name can be written", WriteTextFile(SpaceFile, "UTSystem spaces")) && Passed;
  const MString SpaceLog = TempDirectory + "/spaces.log";
  const int SpaceStatus = MProcessManager::Run("cat", MSystem::GetShellQuoted(SpaceFile), SpaceLog);
  Passed = EvaluateTrue("GetShellQuoted()", "path", "A quoted path with spaces and a single quote is one argument", WIFEXITED(SpaceStatus) && WEXITSTATUS(SpaceStatus) == 0 && ReadTextFile(SpaceLog) == MString("UTSystem spaces")) && Passed;

  // Compare the memory statistics with an independent reading of the platform source (MB, rounded down)
  struct MemoryReference {
    bool Valid = false;
    double RAM = 0.0;
    double FreeRAM = 0.0;
    double Swap = 0.0;
    double FreeSwap = 0.0;
  };
  // Free RAM and free swap change while reading - allowed difference to the reference
  const double FreeSlack = 64.0; // MB, far above the change within microseconds
#if defined(__linux__)
  const double SwapSlack = 0.0; // MB, the installed swap is constant
  auto ReadReference = [&]() -> MemoryReference {
    MemoryReference Reference;
    ifstream In("/proc/meminfo");
    string Line;
    double Total = -1.0, Free = -1.0, Available = -1.0, SwapTotal = -1.0, SwapFree = -1.0;
    while (getline(In, Line)) {
      size_t Colon = Line.find(':');
      if (Colon == string::npos) continue;
      string Key = Line.substr(0, Colon);
      double Value = atof(Line.c_str() + Colon + 1); // kB
      if (Key == "MemTotal") Total = Value;
      if (Key == "MemFree") Free = Value;
      if (Key == "MemAvailable") Available = Value;
      if (Key == "SwapTotal") SwapTotal = Value;
      if (Key == "SwapFree") SwapFree = Value;
    }
    if (Total < 0.0 || Free < 0.0 || Available < 0.0 || SwapTotal < 0.0 || SwapFree < 0.0) return Reference;
    Reference.Valid = true;
    Reference.RAM = floor(Total/1024.0);
    Reference.FreeRAM = floor(Available/1024.0);
    Reference.Swap = floor(SwapTotal/1024.0);
    Reference.FreeSwap = floor(SwapFree/1024.0);
    return Reference;
  };
#elif defined(__APPLE__)
  const double SwapSlack = 1.0; // MB, sysctl prints two digits and macOS grows the swap in large steps
  const MString MemoryLog = TempDirectory + "/memory.log";
  auto RunAndRead = [&](const MString& Program, const MString& Arguments) -> string {
    int Status = MProcessManager::Run(Program, Arguments, MemoryLog);
    if (WIFEXITED(Status) == false || WEXITSTATUS(Status) != 0) return "";
    return string(ReadTextFile(MemoryLog).Data());
  };
  auto ReadReference = [&]() -> MemoryReference {
    MemoryReference Reference;
    string MemSize = RunAndRead("sysctl", "-n hw.memsize");
    string VM = RunAndRead("vm_stat", "");
    string SwapUsage = RunAndRead("sysctl", "-n vm.swapusage");
    // vm_stat: "Mach Virtual Memory Statistics: (page size of 16384 bytes)", "Pages free:   12345." ...
    auto Pages = [&](const string& Key) -> double {
      size_t Position = VM.find(Key);
      return (Position == string::npos) ? -1.0 : atof(VM.c_str() + Position + Key.size());
    };
    size_t PageSizePosition = VM.find("page size of ");
    double PageSize = (PageSizePosition == string::npos) ? -1.0 : atof(VM.c_str() + PageSizePosition + 13);
    double FreePages = Pages("Pages free:");
    double InactivePages = Pages("Pages inactive:");
    // sysctl: "total = 2048.00M  used = 1025.25M  free = 1022.75M  (encrypted)"
    double SwapTotal = -1.0, SwapUsed = -1.0, SwapFree = -1.0;
    if (MemSize.empty() || PageSize < 0.0 || FreePages < 0.0 || InactivePages < 0.0) return Reference;
    if (sscanf(SwapUsage.c_str(), " total = %lfM used = %lfM free = %lfM", &SwapTotal, &SwapUsed, &SwapFree) != 3) return Reference;
    Reference.Valid = true;
    Reference.RAM = floor(strtod(MemSize.c_str(), nullptr)/1048576.0);
    Reference.FreeRAM = floor((FreePages + InactivePages)*PageSize/1048576.0);
    Reference.Swap = floor(SwapTotal);
    Reference.FreeSwap = floor(SwapFree);
    return Reference;
  };
#else
  const double SwapSlack = 0.0;
  auto ReadReference = [&]() -> MemoryReference {
    return MemoryReference();
  };
#endif
  auto InRange = [](double Value, double First, double Second, double Slack) -> bool {
    return Value >= min(First, Second) - Slack && Value <= max(First, Second) + Slack;
  };

  int Free = -1;
  int ErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  DisableDefaultStreams();
  MemoryReference Before = ReadReference();
  bool HasFreeMemory = System.FreeMemory(Free);
  int RAM = System.GetRAM();
  int FreeRAM = System.GetFreeRAM();
  int Swap = System.GetSwap();
  int FreeSwap = System.GetFreeSwap();
  MemoryReference After = ReadReference();
  EnableDefaultStreams();
  gErrorIgnoreLevel = ErrorIgnoreLevel;
  if (Before.Valid == true && After.Valid == true) {
    Passed = EvaluateTrue("FreeMemory()", "success", "FreeMemory succeeds when platform memory statistics are available", HasFreeMemory) && Passed;
    Passed = EvaluateTrue("FreeMemory()", "free mem value", "FreeMemory returns the free memory of the platform (within the change while reading)", InRange(Free, Before.FreeRAM, After.FreeRAM, FreeSlack)) && Passed;
    Passed = Evaluate("GetRAM()", "ram", "GetRAM returns the installed memory of the platform in MB", RAM, static_cast<int>(Before.RAM)) && Passed;
    Passed = EvaluateTrue("GetFreeRAM()", "free ram", "GetFreeRAM returns the free memory of the platform (within the change while reading)", InRange(FreeRAM, Before.FreeRAM, After.FreeRAM, FreeSlack)) && Passed;
    Passed = EvaluateTrue("GetFreeRAM()", "at most installed", "GetFreeRAM is not larger than the installed memory", FreeRAM <= RAM) && Passed;
    Passed = EvaluateTrue("GetSwap()", "swap", "GetSwap returns the installed swap of the platform in MB", InRange(Swap, Before.Swap, After.Swap, SwapSlack)) && Passed;
    Passed = EvaluateTrue("GetFreeSwap()", "free swap", "GetFreeSwap returns the free swap of the platform (within the change while reading)", InRange(FreeSwap, Before.FreeSwap, After.FreeSwap, FreeSlack)) && Passed;
    Passed = EvaluateTrue("GetFreeSwap()", "at most installed", "GetFreeSwap is not larger than the installed swap", FreeSwap <= Swap) && Passed;
  } else {
    Passed = EvaluateTrue("FreeMemory()", "failure", "FreeMemory fails when platform memory statistics are unavailable", HasFreeMemory == false) && Passed;
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
  Passed = EvaluateTrue("BusyWait()", "elapsed", "BusyWait waits at least the requested 5000 microseconds", ElapsedMicroseconds >= 5000LL) && Passed;

  Passed = EvaluateTrue("RemoveTemporaryFile()", "cleanup", "The representative MSystem file can be removed", RemoveTemporaryFile(FileName)) && Passed;
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
