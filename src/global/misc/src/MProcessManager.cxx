/*
 * MProcessManager.cxx
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
#include "MProcessManager.h"

// Standard libs:
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <vector>
using namespace std;

// POSIX libs: processes and signals have no equivalent in the C++ standard library
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MSystem.h"


////////////////////////////////////////////////////////////////////////////////


//! The interrupt flag set by SIGINT and SIGTERM
volatile sig_atomic_t MProcessManager::m_Interrupted = 0;
//! The previous SIGINT handler
void (*MProcessManager::m_OldInterruptHandler)(int) = SIG_DFL;
//! The previous SIGTERM handler
void (*MProcessManager::m_OldTerminateHandler)(int) = SIG_DFL;
//! True while the handlers are installed
bool MProcessManager::m_HandlersInstalled = false;


////////////////////////////////////////////////////////////////////////////////


//! Create a manager that runs up to MaximumParallel programs at once
MProcessManager::MProcessManager(unsigned int MaximumParallel) : m_MaximumParallel(MaximumParallel), m_NextId(0)
{
  if (m_MaximumParallel == 0) {
    m_MaximumParallel = 1;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor: stop all remaining programs
MProcessManager::~MProcessManager()
{
  StopAll();
}


////////////////////////////////////////////////////////////////////////////////


//! Start a program in its own process group and return its manager ID
//! Use the given output file and working directory when provided
//! Set the timeout in seconds (zero disables it); Poll enforces the timeout
//! Return -1 if starting fails or the parallel process limit has been reached
int MProcessManager::Start(const MString& Executable, const MString& Arguments, const MString& OutputFile, const MString& WorkingDirectory, double TimeoutSeconds)
{
  if (CanStart() == false) {
    merr<<"Unable to start "<<Executable<<": "<<m_MaximumParallel<<" programs run already"<<show;
    return -1;
  }
  const pid_t ProcessId = StartInBackground(Executable, Arguments, OutputFile, WorkingDirectory, true);
  if (ProcessId < 0) {
    return -1;
  }
  MManagedProcess Process;
  Process.m_Id = m_NextId++;
  Process.m_ProcessId = ProcessId;
  Process.m_Executable = Executable;
  Process.m_Start = chrono::steady_clock::now();
  Process.m_Timeout = TimeoutSeconds;
  m_Running.push_back(Process);
  return Process.m_Id;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if there is room to start another program
bool MProcessManager::CanStart() const
{
  if (m_Running.size() < m_MaximumParallel) {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Collect the exit statuses of finished programs and stop process groups that have exceeded their timeouts
//! Return false if a child process can no longer be waited for and its status is unknown
bool MProcessManager::Poll()
{
  bool Success = true;
  const chrono::steady_clock::time_point Now = chrono::steady_clock::now();
  vector<MManagedProcess> StillRunning;
  for (MManagedProcess& Process: m_Running) {
    // Check only this child process, leaving unrelated children alone
    int Status = 0;
    const pid_t Done = waitpid(Process.m_ProcessId, &Status, WNOHANG);
    if (Done == Process.m_ProcessId) {
      Process.m_Ended = true;
      Process.m_Status = Status;
      if (WIFEXITED(Status) != 0) {
        Process.m_ExitCode = WEXITSTATUS(Status);
      }
      Process.m_Seconds = chrono::duration<double>(Now - Process.m_Start).count();
      // Kill any descendants still in the finished program's process group
      kill(-Process.m_ProcessId, SIGKILL);
      m_Ended.push_back(Process);
      continue;
    }
    if (Done < 0 && errno != EINTR) {
      // We cannot retrieve the exit status, for example if another handler already collected it
      merr<<"Unable to wait for "<<Process.m_Executable<<" ("<<strerror(errno)<<"): the program is lost"<<show;
      Process.m_Ended = true;
      Process.m_Lost = true;
      Process.m_Seconds = chrono::duration<double>(Now - Process.m_Start).count();
      m_Ended.push_back(Process);
      Success = false;
      continue;
    }
    // Kill the process group if the timeout has expired
    if (Process.m_TimedOut == false && Process.m_Timeout > 0.0 && chrono::duration<double>(Now - Process.m_Start).count() > Process.m_Timeout) {
      kill(-Process.m_ProcessId, SIGKILL);
      Process.m_TimedOut = true;
    }
    StillRunning.push_back(Process);
  }
  m_Running = StillRunning;
  return Success;
}


////////////////////////////////////////////////////////////////////////////////


//! Return and clear the collected results for finished programs, in collection order
//! Each result includes the exit status, timeout flag, and elapsed time in seconds
vector<MManagedProcess> MProcessManager::TakeEnded()
{
  vector<MManagedProcess> Ended = m_Ended;
  m_Ended.clear();
  return Ended;
}


////////////////////////////////////////////////////////////////////////////////


//! Wait for all managed programs to finish, sleeping PollSeconds between checks
//! Return false if a child process status is lost or an interrupt is detected
//! Stop all managed programs when an interrupt is detected
bool MProcessManager::WaitForAll(double PollSeconds)
{
  while (true) {
    if (WasInterrupted() == true) {
      StopAll();
      return false;
    }
    if (Poll() == false) {
      return false;
    }
    if (HasRunning() == false) {
      return true;
    }
    usleep(static_cast<useconds_t>(PollSeconds*1000000.0));
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Kill all managed process groups and wait until the programs started by the manager are reaped
void MProcessManager::StopAll()
{
  for (MManagedProcess& Process: m_Running) {
    kill(-Process.m_ProcessId, SIGKILL);
  }
  const chrono::steady_clock::time_point Now = chrono::steady_clock::now();
  for (MManagedProcess& Process: m_Running) {
    int Status = 0;
    pid_t Done = -1;
    do {
      Done = waitpid(Process.m_ProcessId, &Status, 0);
    } while (Done < 0 && errno == EINTR);
    Process.m_Ended = true;
    if (Done == Process.m_ProcessId) {
      Process.m_Status = Status;
    } else {
      Process.m_Lost = true;
    }
    Process.m_Seconds = chrono::duration<double>(Now - Process.m_Start).count();
    m_Ended.push_back(Process);
  }
  m_Running.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Install SIGINT and SIGTERM handlers that set the interrupt flag, saving the previous handlers
void MProcessManager::InstallInterruptHandlers()
{
  if (m_HandlersInstalled == true) {
    return;
  }
  m_OldInterruptHandler = signal(SIGINT, HandleInterrupt);
  m_OldTerminateHandler = signal(SIGTERM, HandleInterrupt);
  m_HandlersInstalled = true;
}


////////////////////////////////////////////////////////////////////////////////


//! Restore the previous SIGINT and SIGTERM handlers
void MProcessManager::RestoreInterruptHandlers()
{
  if (m_HandlersInstalled == false) {
    return;
  }
  signal(SIGINT, m_OldInterruptHandler);
  signal(SIGTERM, m_OldTerminateHandler);
  m_HandlersInstalled = false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if SIGINT or SIGTERM has been received since the interrupt flag was cleared
bool MProcessManager::WasInterrupted()
{
  if (m_Interrupted != 0) {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the interrupt flag
void MProcessManager::ClearInterrupt()
{
  m_Interrupted = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the interrupt flag when SIGINT or SIGTERM is received
void MProcessManager::HandleInterrupt(int)
{
  m_Interrupted = 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Launch a program in the background and return its process ID, or -1 on failure
//! Start a new process group if OwnProcessGroup is true; otherwise use the caller's process group
//! Pass Arguments to the shell, allowing redirection, quoted arguments, and operators such as &&
//! Redirect stdout and stderr to OutputFile when provided
//! Use WorkingDirectory when provided
pid_t MProcessManager::StartInBackground(const MString& Executable, const MString& Arguments, const MString& OutputFile, const MString& WorkingDirectory, bool OwnProcessGroup)
{
  // Allocate memory before forking - other threads may hold allocator locks
  const MString Command = MSystem::GetShellQuoted(Executable) + " " + Arguments;

  pid_t Process = fork();
  if (Process < 0) {
    return -1;
  }
  if (Process == 0) {
    if (OwnProcessGroup == true) {
      setpgid(0, 0);
    }
    if (WorkingDirectory.IsEmpty() == false && chdir(WorkingDirectory.Data()) != 0) {
      _exit(126);
    }
    if (OutputFile.IsEmpty() == false) {
      int Log = open(OutputFile.Data(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (Log >= 0) {
        dup2(Log, STDOUT_FILENO);
        dup2(Log, STDERR_FILENO);
        close(Log);
      }
    }
    execl("/bin/sh", "sh", "-c", Command.Data(), static_cast<char*>(0));
    // Exit after a failed exec without running inherited cleanup or flushing buffers
    // Use exit code 127 for a command that could not be run
    _exit(127);
  }

  if (OwnProcessGroup == true) {
    setpgid(Process, Process); // Also set here to avoid a race with the new process
  }
  return Process;
}


////////////////////////////////////////////////////////////////////////////////


//! Wait for a background process and return its raw wait status (see WIFEXITED), or -1 on failure
//! Kill the process if TimeOut seconds elapse (zero disables the timeout) or the optional stop flag is set
//! Kill its process group if it leads one; otherwise try to kill its descendants as well
int MProcessManager::WaitForBackground(pid_t Process, unsigned int TimeOut, const volatile sig_atomic_t* Stop)
{
  const chrono::steady_clock::time_point Start = chrono::steady_clock::now();
  bool Killed = false;
  while (true) {
    int Status = 0;
    const pid_t Done = waitpid(Process, &Status, WNOHANG);
    if (Done == Process) {
      return Status;
    }
    if (Done < 0 && errno != EINTR) {
      return -1;
    }
    if (Killed == false) {
      const double Elapsed = chrono::duration<double>(chrono::steady_clock::now() - Start).count();
      if ((TimeOut > 0 && Elapsed > TimeOut) || (Stop != nullptr && *Stop != 0)) {
        // Kill the process group if this process leads it; otherwise kill its process tree
        if (getpgid(Process) == Process) {
          kill(-Process, SIGKILL);
        } else {
          KillTree(Process);
        }
        Killed = true;
      }
    }
    usleep(10000);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Kill descendants first, then the process
//! Reparented descendants or children started during the search may survive
void MProcessManager::KillTree(pid_t Process)
{
  if (Process <= 0) { // Nonpositive IDs could signal a process group
    return;
  }

  // Pause the process to prevent it from starting children during the search
  kill(Process, SIGSTOP);

  // Read the process table with POSIX ps - the C++ library cannot list processes
  FILE* Table = popen("ps -A -o pid= -o ppid=", "r");
  if (Table == nullptr) {
    merr<<"KillTree: cannot read the process table, only process "<<Process<<" is killed"<<endl;
  } else {
    vector<pid_t> Children;
    long Id = 0;
    long Parent = 0;
    while (fscanf(Table, "%ld %ld", &Id, &Parent) == 2) {
      if (Parent == Process) {
        Children.push_back(static_cast<pid_t>(Id));
      }
    }
    bool ReadFailed = false;
    if (ferror(Table) != 0) {
      ReadFailed = true;
    }
    const int Status = pclose(Table); // Close the pipe even if reading failed
    if (ReadFailed == true || Status != 0) {
      merr<<"KillTree: reading the process table failed, descendants of "<<Process<<" may survive"<<endl;
    }
    // Kill children before the parent so their parent IDs remain available during the search
    for (pid_t Child: Children) {
      KillTree(Child);
    }
  }

  kill(Process, SIGKILL);
}


////////////////////////////////////////////////////////////////////////////////


//! Run a program to completion and return its raw wait status (see WIFEXITED), or -1 on failure
int MProcessManager::Run(const MString& Executable, const MString& Arguments, const MString& OutputFile, const MString& WorkingDirectory, unsigned int TimeOut)
{
  const pid_t Process = StartInBackground(Executable, Arguments, OutputFile, WorkingDirectory);
  if (Process < 0) {
    return -1;
  }
  return WaitForBackground(Process, TimeOut);
}


// MProcessManager.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
