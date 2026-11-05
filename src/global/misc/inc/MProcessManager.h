/*
 * MProcessManager.h
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


#ifndef __MProcessManager__
#define __MProcessManager__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <chrono>
#include <csignal>
#include <vector>
using namespace std;

// POSIX libs:
#include <sys/types.h>

// ROOT libs:

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a managed process, its runtime, and its exit status
class MManagedProcess
{
public:
  //! The manager ID returned by Start
  unsigned int m_Id = 0;
  //! The operating system process ID
  pid_t m_ProcessId = -1;
  //! The program which was started
  MString m_Executable;
  //! The start time measured with a monotonic clock
  chrono::steady_clock::time_point m_Start;
  //! The timeout in seconds (zero disables it)
  double m_Timeout = 0.0;
  //! True if the process was killed after exceeding its timeout
  bool m_TimedOut = false;
  //! True if the process has finished, was stopped, or can no longer be waited for
  bool m_Ended = false;
  //! True if waiting for the process failed and its exit status is unknown
  bool m_Lost = false;
  //! The raw wait status (see WIFEXITED), -1 if unknown
  int m_Status = -1;
  //! The exit code if it ended normally, -1 otherwise
  int m_ExitCode = -1;
  //! The elapsed time in seconds from the start until its end was recorded
  double m_Seconds = 0.0;
};


//! Class representing a manager for starting, monitoring, and stopping child processes
//! Run up to MaximumParallel programs at once, each in its own process group
//! Poll to collect exit statuses and stop programs that exceed their timeouts
//! Install the interrupt handlers to let waiting stop all managed programs on SIGINT or SIGTERM
//! Use the static functions to start or wait for individual programs without a manager
class MProcessManager
{
  // public interface:
 public:
  //! Create a manager that runs up to MaximumParallel programs at once
  MProcessManager(unsigned int MaximumParallel = 1);
  //! Default destructor: stop all remaining programs
  virtual ~MProcessManager();
  //! Disable copying because the manager owns and cleans up its child processes
  MProcessManager(const MProcessManager&) = delete;
  //! Disable assignment because the manager owns and cleans up its child processes
  MProcessManager& operator=(const MProcessManager&) = delete;

  // Starting:

  //! Start a program in its own process group and return its manager ID
  //! Use the given output file and working directory when provided
  //! Set the timeout in seconds (zero disables it); Poll enforces the timeout
  //! Return -1 if starting fails or the parallel process limit has been reached
  int Start(const MString& Executable, const MString& Arguments, const MString& OutputFile, const MString& WorkingDirectory, double TimeoutSeconds);
  //! Return true if there is room to start another program
  bool CanStart() const;
  //! Return the number of programs currently tracked as running
  unsigned int GetNumberOfRunning() const { return m_Running.size(); }
  //! Return true if any programs are currently tracked as running
  bool HasRunning() const { return m_Running.empty() == false; }

  // Watching:

  //! Collect the exit statuses of finished programs and stop process groups that have exceeded their timeouts
  //! Return false if a child process can no longer be waited for and its status is unknown
  bool Poll();
  //! Return and clear the collected results for finished programs, in collection order
  //! Each result includes the exit status, timeout flag, and elapsed time in seconds
  vector<MManagedProcess> TakeEnded();
  //! Wait for all managed programs to finish, sleeping PollSeconds between checks
  //! Return false if a child process status is lost or an interrupt is detected
  //! Stop all managed programs when an interrupt is detected
  bool WaitForAll(double PollSeconds);
  //! Kill all managed process groups and wait until the programs started by the manager are reaped
  void StopAll();

  // Interrupts - one flag for the program, set by SIGINT and SIGTERM:

  //! Install SIGINT and SIGTERM handlers that set the interrupt flag, saving the previous handlers
  static void InstallInterruptHandlers();
  //! Restore the previous SIGINT and SIGTERM handlers
  static void RestoreInterruptHandlers();
  //! Return true if SIGINT or SIGTERM has been received since the interrupt flag was cleared
  static bool WasInterrupted();
  //! Clear the interrupt flag
  static void ClearInterrupt();

  // Single programs:

  //! Launch a program in the background and return its process ID, or -1 on failure
  //! Start a new process group if OwnProcessGroup is true; otherwise use the caller's process group
  //! Pass Arguments to the shell, allowing redirection, quoted arguments, and operators such as &&
  //! Without Arguments the program runs without a shell: it is the started process itself, so a signal which ends it is seen as that signal
  //! Redirect stdout and stderr to OutputFile when provided
  //! Use WorkingDirectory when provided
  static pid_t StartInBackground(const MString& Executable, const MString& Arguments, const MString& OutputFile = "", const MString& WorkingDirectory = "", bool OwnProcessGroup = false);
  //! Wait for a background process and return its raw wait status (see WIFEXITED), or -1 on failure
  //! Kill the process if TimeOut seconds elapse (zero disables the timeout) or the optional stop flag is set
  //! Kill its process group if it leads one; otherwise try to kill its descendants as well
  static int WaitForBackground(pid_t Process, unsigned int TimeOut = 0, const volatile sig_atomic_t* Stop = nullptr);
  //! Run a program to completion and return its raw wait status (see WIFEXITED), or -1 on failure
  static int Run(const MString& Executable, const MString& Arguments, const MString& OutputFile = "", const MString& WorkingDirectory = "", unsigned int TimeOut = 0);

  // private methods:
 private:
  //! Set the interrupt flag when SIGINT or SIGTERM is received
  static void HandleInterrupt(int Signal);
  //! Kill descendants first, then the process
  //! Reparented descendants or children started during the search may survive
  static void KillTree(pid_t Process);

  // private members:
 private:
  //! The interrupt flag set by SIGINT and SIGTERM
  static volatile sig_atomic_t m_Interrupted;
  //! The previous SIGINT handler
  static void (*m_OldInterruptHandler)(int);
  //! The previous SIGTERM handler
  static void (*m_OldTerminateHandler)(int);
  //! True while the handlers are installed
  static bool m_HandlersInstalled;

  //! The maximum number of programs allowed to run at once
  unsigned int m_MaximumParallel;
  //! The manager ID to assign to the next program
  unsigned int m_NextId;
  //! The programs currently tracked as running
  vector<MManagedProcess> m_Running;
  //! The collected results waiting to be retrieved
  vector<MManagedProcess> m_Ended;
};

#endif


////////////////////////////////////////////////////////////////////////////////
