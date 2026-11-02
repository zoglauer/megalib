/*
 * UTGlobal.cxx
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
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"


//! Unit test class for MGlobal: Initialize() is tested in child processes, it can only succeed once per process
class UTGlobal : public MUnitTest
{
public:
  //! Default constructor
  UTGlobal() : MUnitTest("UTGlobal") {}
  //! Default destructor
  virtual ~UTGlobal() {}

  //! Run all tests
  virtual bool Run();

  //! Helper entry point: MEGALIB is not set, Initialize() has to fail
  static int WithoutMEGAlib()
  {
    unsetenv("MEGALIB");
    if (MGlobal::Initialize("UTGlobal", "unit test of MGlobal") == false) {
      return 0;
    }
    return 1;
  }
  //! Helper entry point: MEGALIB is empty, Initialize() has to fail
  static int EmptyMEGAlib()
  {
    setenv("MEGALIB", "", 1);
    if (MGlobal::Initialize() == false) {
      return 0;
    }
    return 1;
  }
  //! Helper entry point: MEGALIB is set as in the parent, Initialize() has to succeed
  static int WithMEGAlib()
  {
    if (MGlobal::Initialize() == true) {
      return 0;
    }
    return 1;
  }

private:
  typedef int (*TCheckFunction)();

  //! Run a helper function in a child process and capture its output
  static int RunChild(TCheckFunction Function, const MString& LogFileName)
  {
    pid_t Child = fork();
    if (Child < 0) {
      return -1;
    }

    if (Child == 0) {
      int Log = open(LogFileName.Data(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (Log < 0) {
        _exit(127);
      }
      (void)dup2(Log, STDOUT_FILENO);
      (void)dup2(Log, STDERR_FILENO);
      close(Log);
      int ReturnValue = Function();
      _exit(ReturnValue);
    }

    int Status = 0;
    while (waitpid(Child, &Status, 0) < 0) {
      if (errno != EINTR) {
        return -1;
      }
    }
    if (WIFEXITED(Status) != 0) {
      return WEXITSTATUS(Status);
    }
    return -1;
  }
};


////////////////////////////////////////////////////////////////////////////////


bool UTGlobal::Run()
{
  bool Passed = true;

  // Without MEGALIB nothing of MEGAlib can be found: Initialize() fails and says why, the programs end with it
  const MString NoneLog = GetTemporaryFileName("none.log");
  RemoveTemporaryFile(NoneLog);
  Passed = Evaluate("MGlobal::Initialize()", "no MEGALIB status", "Without MEGALIB the initialization fails", RunChild(WithoutMEGAlib, NoneLog), 0) && Passed;
  Passed = EvaluateTrue("MGlobal::Initialize()", "no MEGALIB message", "Without MEGALIB the initialization says that MEGALIB is not set", ReadTextFile(NoneLog).Contains("MEGALIB environment variable is not set")) && Passed;
  RemoveTemporaryFile(NoneLog);

  const MString EmptyLog = GetTemporaryFileName("empty.log");
  RemoveTemporaryFile(EmptyLog);
  Passed = Evaluate("MGlobal::Initialize()", "empty MEGALIB status", "With an empty MEGALIB the initialization fails", RunChild(EmptyMEGAlib, EmptyLog), 0) && Passed;
  Passed = EvaluateTrue("MGlobal::Initialize()", "empty MEGALIB message", "With an empty MEGALIB the initialization says that MEGALIB is not set", ReadTextFile(EmptyLog).Contains("MEGALIB environment variable is not set")) && Passed;
  RemoveTemporaryFile(EmptyLog);

  // With MEGALIB the initialization works - if the environment of this test has it
  if (getenv("MEGALIB") == nullptr) {
    mout<<"UTGlobal: MEGALIB is not set - skipping the test of the initialization with MEGALIB"<<endl;
  } else {
    const MString WithLog = GetTemporaryFileName("with.log");
    RemoveTemporaryFile(WithLog);
    Passed = Evaluate("MGlobal::Initialize()", "MEGALIB status", "With MEGALIB the initialization succeeds", RunChild(WithMEGAlib, WithLog), 0) && Passed;
    Passed = EvaluateFalse("MGlobal::Initialize()", "MEGALIB message", "With MEGALIB there is no error message", ReadTextFile(WithLog).Contains("INITIALIZATION ERROR")) && Passed;
    RemoveTemporaryFile(WithLog);
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int, char**)
{
  UTGlobal Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTGlobal.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
