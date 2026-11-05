/*
 * UTSDHitShared.h
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


#ifndef ___UTSDHitShared___
#define ___UTSDHitShared___


// Standard lib:
#include <chrono>
#include <functional>
#include <thread>
using namespace std;

// POSIX - C++ has no process control; a forked child isolates crashes of the code under test, so that only one check fails and all others still run:
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>


//! Helper shared by the unit tests of the hits and sensitive detectors of cosima
class UTSDHitShared
{
public:
  //! The outcome of a callable which has been run in a child process
  enum class ChildOutcome { c_ReturnedTrue, c_ReturnedFalse, c_Crashed, c_TimedOut, c_ForkFailed };

  //! Run the callable in a forked child process without any output and return how it ended
  //! The callable must not use the Evaluate functions - they would count in the child only
  static ChildOutcome RunInChild(const function<bool()>& Callable, unsigned int TimeLimit = 20)
  {
    pid_t PID = fork();
    if (PID < 0) {
      return ChildOutcome::c_ForkFailed;
    }

    if (PID == 0) {
      // Die immediately on a crash - the ROOT handler would print a long stack trace:
      signal(SIGSEGV, SIG_DFL);
      signal(SIGABRT, SIG_DFL);
      signal(SIGBUS, SIG_DFL);
      signal(SIGFPE, SIG_DFL);
      signal(SIGILL, SIG_DFL);

      // Discard all output of the child:
      int Null = open("/dev/null", O_WRONLY);
      if (Null >= 0) {
        dup2(Null, STDOUT_FILENO);
        dup2(Null, STDERR_FILENO);
        close(Null);
      }

      bool Result = false;
      try {
        Result = Callable();
      } catch (...) {
        Result = false;
      }
      // Do not run destructors or flush buffers of the parent's objects:
      _exit(Result == true ? 0 : 1);
    }

    // Wait for the child with a time limit:
    auto Start = chrono::steady_clock::now();
    int Status = 0;
    while (true) {
      pid_t Done = waitpid(PID, &Status, WNOHANG);
      if (Done == PID) break;
      if (chrono::steady_clock::now() - Start > chrono::seconds(TimeLimit)) {
        kill(PID, SIGKILL);
        waitpid(PID, &Status, 0);
        return ChildOutcome::c_TimedOut;
      }
      this_thread::sleep_for(chrono::milliseconds(2));
    }

    if (WIFEXITED(Status) == true) {
      return (WEXITSTATUS(Status) == 0) ? ChildOutcome::c_ReturnedTrue : ChildOutcome::c_ReturnedFalse;
    }

    return ChildOutcome::c_Crashed;
  }

  //! Return true if the callable ran through in a child process and returned true
  static bool RunsCleanlyInChild(const function<bool()>& Callable, unsigned int TimeLimit = 20)
  {
    return RunInChild(Callable, TimeLimit) == ChildOutcome::c_ReturnedTrue;
  }
};


#endif


/*
 * UTSDHitShared.h: the end...
 ******************************************************************************/
