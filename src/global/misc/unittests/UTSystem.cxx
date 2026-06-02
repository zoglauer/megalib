/*
 * UTSystem.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <ctime>
#include <sys/time.h>
#include <sys/wait.h>

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
  int ChildStatus = MSystem::RunChildProcess("/bin/sh", "-c 'echo UTSystem child'", ChildLog);
  Passed = EvaluateTrue("RunChildProcess()", "successful child", "RunChildProcess returns a normal zero exit status for a successful child", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 0) && Passed;
  MString ChildLogContent = ReadTextFile(ChildLog);
  Passed = EvaluateTrue("RunChildProcess()", "redirected output", "RunChildProcess redirects child output to the requested file", ChildLogContent.IsEmpty() == false) && Passed;
  Passed = EvaluateTrue("RunChildProcess()", "redirected output", "The redirected child output contains the expected marker", ChildLogContent.Contains("UTSystem child")) && Passed;
  const MString MissingChildLog = TempDirectory + "/missing-child.log";
  ChildStatus = MSystem::RunChildProcess(GetTemporaryFileName("missing_child_executable"), "", MissingChildLog);
  Passed = EvaluateTrue("RunChildProcess()", "missing child", "RunChildProcess returns the shell command-not-found status for a missing child executable", WIFEXITED(ChildStatus) && WEXITSTATUS(ChildStatus) == 127) && Passed;

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
