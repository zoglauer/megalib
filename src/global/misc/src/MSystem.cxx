/*
 * MSystem.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MSystem
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSystem.h"

// Standard libs:
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <vector>

// POSIX libs:
#include <dlfcn.h>
#include <fcntl.h>
#ifdef __APPLE__
#include <sys/sysctl.h>
#include <mach/mach.h>
#endif
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TSystem.h"

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"

// Special libs:
#ifdef ___UNIX___
#include <sys/time.h>
#endif


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSystem)
#endif


////////////////////////////////////////////////////////////////////////////////


MSystem::MSystem()
{
  // standard constructor

  Reset();

  m_LastCheck = TTime(0);
  m_CheckInterval = TTime(2000); // 2 seconds
}


////////////////////////////////////////////////////////////////////////////////


MSystem::~MSystem()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


//! Check whether an X11 display can be opened from this process.
//!
//! INTERNAL: private to MSystem and friended only to MGlobal. The single
//! intended caller is MGlobal::Initialize(), which runs before MEGAlib
//! spawns any worker threads. The Linux implementation fork()s and the
//! child calls dlopen() / XOpenDisplay(); after fork() in a multithreaded
//! parent both can deadlock on inherited library/runtime locks, so the
//! "called early at startup" constraint is a precondition, not advice.
//!
//! Linux: tries to open the display in a child process so a stale or
//! unreachable DISPLAY cannot block startup; the parent polls for the
//! child's exit for up to 2 s and SIGKILLs the probe if it doesn't
//! finish in time. libX11 is loaded at runtime via dlopen, so
//! libCommonMisc carries no link-time dependency on it -- the library
//! still works on systems where libX11 is not installed at all
//! (minimal containers, headless servers, pure-Wayland desktops that
//! ship without XWayland). Wayland sessions that include XWayland (the
//! common case) look like ordinary X sessions and succeed via
//! /tmp/.X11-unix/X<n>.
//!
//! macOS: libX11 is not part of a default install (it ships with
//! XQuartz, whose SDK is not assumed at build time), so we do not link
//! or call X here. We trust the DISPLAY environment variable instead:
//! if it is set we assume an X server (typically XQuartz) is available;
//! otherwise we report no display and let ROOT pick its Cocoa back-end
//! or batch mode on its own. We deliberately do not dlopen XQuartz to
//! probe further: requiring XQuartz at runtime is exactly the assumption
//! we want to avoid. A stale macOS $DISPLAY will therefore still register
//! as a usable display here; ROOT discovers the failure when it tries.
bool MSystem::HasDisplay()
{
#if defined(___LINUX___)
  const char* DisplayEnv = getenv("DISPLAY");
  if (DisplayEnv == nullptr || DisplayEnv[0] == '\0') {
    // Pure-Wayland session with no XWayland is the most common reason
    // DISPLAY is empty on a logged-in Linux desktop. Give a useful hint.
    const char* WaylandEnv = getenv("WAYLAND_DISPLAY");
    if (WaylandEnv != nullptr && WaylandEnv[0] != '\0') {
      cout<<"Display not found: only WAYLAND_DISPLAY is set (Wayland session without XWayland); ROOT requires an X server"<<endl;
    } else {
      cout<<"Display not found: DISPLAY environment variable is unset or empty"<<endl;
    }
    return false;
  }

  pid_t Child = fork();
  if (Child == 0) {
    // Load libX11 at runtime so libCommonMisc has no link-time
    // dependency on it. Distinct exit codes let the parent give a
    // useful diagnostic:
    //   0 = display opened successfully
    //   1 = XOpenDisplay returned nullptr (server unreachable)
    //   2 = libX11 is not installed (dlopen failed)
    //   3 = libX11 loaded but expected symbols are missing
    // libX11 SONAME has been .6 for ~20 years; .7 is purely defensive
    // in case a future ABI bump ever happens. The unversioned "libX11.so"
    // is the -dev symlink and only exists where development packages are
    // installed, so it goes last.
    static const char* const LibCandidates[] = {
      "libX11.so.6",
      "libX11.so.7",
      "libX11.so",
      nullptr
    };
    void* Lib = nullptr;
    for (int i = 0; LibCandidates[i] != nullptr; ++i) {
      Lib = dlopen(LibCandidates[i], RTLD_LAZY | RTLD_LOCAL);
      if (Lib != nullptr) break;
    }
    if (Lib == nullptr) _exit(2);

    typedef void* (*OpenFn)(const char*);
    typedef int (*CloseFn)(void*);
    OpenFn OpenDisplay = (OpenFn) dlsym(Lib, "XOpenDisplay");
    CloseFn CloseDisplay = (CloseFn) dlsym(Lib, "XCloseDisplay");
    if (OpenDisplay == nullptr || CloseDisplay == nullptr) _exit(3);

    void* Handle = OpenDisplay(nullptr);
    if (Handle == nullptr) _exit(1);

    CloseDisplay(Handle);
    _exit(0);
  }

  if (Child < 0) {
    cout<<"Display not found: failed to fork display probe"<<endl;
    return false;
  }

  int ChildStatus = 0;
  for (unsigned int i = 0; i < 200; ++i) {
    pid_t Result = waitpid(Child, &ChildStatus, WNOHANG);
    if (Result == Child) {
      if (WIFEXITED(ChildStatus)) {
        int Code = WEXITSTATUS(ChildStatus);
        if (Code == 0) return true;
        if (Code == 2) {
          cout<<"Display not found: libX11 is not installed (no libX11.so.{6,7,*} could be dlopen'd)"<<endl;
        } else if (Code == 3) {
          cout<<"Display not found: libX11 is missing expected symbols (XOpenDisplay / XCloseDisplay)"<<endl;
        } else {
          cout<<"Display not found: XOpenDisplay failed for DISPLAY=\""<<DisplayEnv<<"\""<<endl;
        }
      } else {
        cout<<"Display not found: display probe died abnormally for DISPLAY=\""<<DisplayEnv<<"\""<<endl;
      }
      return false;
    }

    if (Result < 0) {
      // EINTR: signal interruption, not a real failure -- retry on the
      // next iteration so we don't leave the probe child running while
      // falsely reporting no display.
      if (errno == EINTR) continue;
      cout<<"Display not found: failed to wait for display probe"<<endl;
      return false;
    }

    usleep(10000);
  }

  kill(Child, SIGKILL);
  // Reap the probe, retrying on EINTR so it never lingers as a zombie.
  while (waitpid(Child, &ChildStatus, 0) < 0 && errno == EINTR) {}
  cout<<"Display not found: XOpenDisplay probe timed out for DISPLAY=\""<<DisplayEnv<<"\""<<endl;
  return false;

#elif defined(___MACOSX___)
  // libX11 is not part of a default macOS install (it ships with XQuartz,
  // whose SDK is not assumed at build time), so we do not call XOpenDisplay
  // here. Trust the DISPLAY environment variable: if it is set we assume an
  // X server (typically XQuartz) is available; otherwise ROOT picks its
  // Cocoa back-end or batch mode on its own.
  const char* DisplayEnv = getenv("DISPLAY");
  if (DisplayEnv == nullptr || DisplayEnv[0] == '\0') {
    cout<<"Display not found: DISPLAY environment variable is unset or empty"<<endl;
    return false;
  }
  return true;

#else
  return true;
#endif
}


////////////////////////////////////////////////////////////////////////////////


//! Return the argument quoted for a POSIX shell: it is one word whatever it contains (spaces, quotes, $, backticks, ...), the empty string gives ''
MString MSystem::GetShellQuoted(const MString& Argument)
{
  // One shell word: in single quotes, a single quote inside becomes '\''
  MString Quoted = "'";
  for (unsigned int c = 0; c < Argument.Length(); ++c) {
    if (Argument[c] == '\'') {
      Quoted += "'\\''";
    } else {
      Quoted += Argument[c];
    }
  }
  Quoted += "'";
  return Quoted;
}


////////////////////////////////////////////////////////////////////////////////


//! Launch a program in the background and return its process ID, or -1 on failure
//! With OwnProcessGroup it starts a new process group, otherwise it stays in the process group of the caller
//! The arguments are a shell fragment and may contain redirects, &&, quoted words, etc.
//! The output (stdout and stderr) goes to the output file
//! The program runs in the working directory if one is given
pid_t MSystem::StartProcessInBackground(const MString& Executable, const MString& Arguments, const MString& OutputFile, const MString& WorkingDirectory, bool OwnProcessGroup)
{
  // Everything which allocates memory is done before the fork: the process may have other threads
  const MString Command = GetShellQuoted(Executable) + " " + Arguments;

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
    // If execl() returns, the process failed to start the command. Use _exit()
    // after fork() to avoid running parent-owned C++ cleanup/stdio flushing.
    // Exit code 127 is the shell convention for "command could not be run".
    _exit(127);
  }

  if (OwnProcessGroup == true) {
    setpgid(Process, Process); // Also set here to avoid a race with the new process
  }
  return Process;
}


////////////////////////////////////////////////////////////////////////////////


//! Wait for a background process and return its raw wait status (see WIFEXITED), or -1 on failure.
//! After the time out in seconds (0: none), or as soon as the stop flag (if given) is set, the process is killed, with its group if it has its own, otherwise with its descendants (best effort)
int MSystem::WaitForBackgroundProcess(pid_t Process, unsigned int TimeOut, const volatile sig_atomic_t* Stop)
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
        // Kill the whole group if the process leads its own, otherwise the process and its descendants
        if (getpgid(Process) == Process) {
          kill(-Process, SIGKILL);
        } else {
          KillProcessTree(Process);
        }
        Killed = true;
      }
    }
    usleep(10000);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Kill the process and all its descendants, the descendants first (best effort: reparented or newly started ones can survive)
void MSystem::KillProcessTree(pid_t Process)
{
  if (Process <= 0) { // Zero and negative values would address groups
    return;
  }

  // Freeze the process - it cannot start new children while we look for them
  kill(Process, SIGSTOP);

  // Read the process table with POSIX ps - the C++ library cannot list processes
  FILE* Table = popen("ps -A -o pid= -o ppid=", "r");
  if (Table == nullptr) {
    merr<<"KillProcessTree: cannot read the process table, only process "<<Process<<" is killed"<<endl;
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
    const int Status = pclose(Table); // Always close, also after a read error
    if (ReadFailed == true || Status != 0) {
      merr<<"KillProcessTree: reading the process table failed, descendants of "<<Process<<" may survive"<<endl;
    }
    // Kill the children first - once the parent is dead they are reparented and cannot be found any more
    for (pid_t Child: Children) {
      KillProcessTree(Child);
    }
  }

  kill(Process, SIGKILL);
}


////////////////////////////////////////////////////////////////////////////////


//! Start a program, wait for it and return its raw wait status, or -1 on failure (see StartProcessInBackground and WaitForBackgroundProcess)
int MSystem::RunProcess(const MString& Executable, const MString& Arguments, const MString& OutputFile, const MString& WorkingDirectory, unsigned int TimeOut)
{
  const pid_t Process = StartProcessInBackground(Executable, Arguments, OutputFile, WorkingDirectory);
  if (Process < 0) {
    return -1;
  }
  return WaitForBackgroundProcess(Process, TimeOut);
}


void MSystem::Reset()
{
  // Reset all values to default:

  m_RAM = -1;
  m_FreeRAM = -1;
  m_Swap = -1;
  m_FreeSwap = -1;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the free RAM in MB, return false and set it to -1 if unknown
bool MSystem::FreeMemory(int &Free)
{
  bool Success = GetMemory();
  Free = m_FreeRAM;

  return Success;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the RAM and swap values in MB (rounded down), return false and set them to -1 if unknown
bool MSystem::GetMemory()
{
  Reset();

#if defined(__APPLE__)

  // Get the installed RAM (bytes):
  uint64_t MemSize = 0;
  size_t MemSizeLength = sizeof(MemSize);
  if (sysctlbyname("hw.memsize", &MemSize, &MemSizeLength, nullptr, 0) != 0) {
    merr<<"Unable to read hw.memsize"<<endl;
    return false;
  }

  // Get the free RAM - free plus inactive pages:
  mach_port_t Host = mach_host_self();
  vm_size_t PageSize = 0;
  vm_statistics64_data_t VM;
  mach_msg_type_number_t VMCount = HOST_VM_INFO64_COUNT;
  kern_return_t PageSizeResult = host_page_size(Host, &PageSize);
  kern_return_t StatisticsResult = host_statistics64(Host, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&VM), &VMCount);
  // Release the host port - mach_host_self() added a send right
  mach_port_deallocate(mach_task_self(), Host);
  if (PageSizeResult != KERN_SUCCESS || StatisticsResult != KERN_SUCCESS) {
    merr<<"Unable to read the virtual memory statistics"<<endl;
    return false;
  }
  uint64_t FreeBytes = (static_cast<uint64_t>(VM.free_count) + VM.inactive_count) * PageSize;

  // Get the swap (bytes):
  struct xsw_usage SwapUsage;
  size_t SwapUsageLength = sizeof(SwapUsage);
  if (sysctlbyname("vm.swapusage", &SwapUsage, &SwapUsageLength, nullptr, 0) != 0) {
    merr<<"Unable to read vm.swapusage"<<endl;
    return false;
  }

  m_RAM = MemSize/1048576;
  m_FreeRAM = FreeBytes/1048576;
  m_Swap = SwapUsage.xsw_total/1048576;
  m_FreeSwap = SwapUsage.xsw_avail/1048576;
  return true;

#elif defined(__linux__)

  FILE* MemInfo = fopen("/proc/meminfo", "r");
  if (MemInfo == nullptr) {
    merr<<"Cannot open file '/proc/meminfo'!"<<endl;
    return false;
  }

  // Read the values (kB), -1: not found:
  long MemTotal = -1, MemFree = -1, MemAvailable = -1, Buffers = -1, Cached = -1, SwapTotal = -1, SwapFree = -1;
  char Line[256];
  while (fgets(Line, sizeof(Line), MemInfo) != nullptr) {
    char Name[64];
    long Value = 0;
    if (sscanf(Line, "%63[^:]: %ld", Name, &Value) != 2) continue;
    MString Key(Name);
    if (Key == "MemTotal") MemTotal = Value;
    else if (Key == "MemFree") MemFree = Value;
    else if (Key == "MemAvailable") MemAvailable = Value;
    else if (Key == "Buffers") Buffers = Value;
    else if (Key == "Cached") Cached = Value;
    else if (Key == "SwapTotal") SwapTotal = Value;
    else if (Key == "SwapFree") SwapFree = Value;
  }
  fclose(MemInfo);

  if (MemTotal < 0 || MemFree < 0 || SwapTotal < 0 || SwapFree < 0) {
    merr<<"Unable to read the memory values from /proc/meminfo"<<endl;
    return false;
  }

  // Free RAM - MemAvailable if the kernel has it (>= 3.14), otherwise free plus buffers and cache:
  long FreeRAM = MemFree + (Buffers > 0 ? Buffers : 0) + (Cached > 0 ? Cached : 0);
  if (MemAvailable >= 0) FreeRAM = MemAvailable;

  m_RAM = MemTotal/1024;
  m_FreeRAM = FreeRAM/1024;
  m_Swap = SwapTotal/1024;
  m_FreeSwap = SwapFree/1024;
  return true;

#else

  return false;

#endif
}


////////////////////////////////////////////////////////////////////////////////


void MSystem::BusyWait(int musec)
{
  // Do a busy wait (== calling thread is active NOT sleeping!) 
  // for several microseconds
  // Sleep for roughly [musec..musec+1] microseconds


#ifdef ___UNIX___
  long long currenttime = 0, stoptime;
  struct timeval tv;
  gettimeofday(&tv, 0);
  stoptime = (long long)tv.tv_sec * (long long)1000000;
  stoptime += (long long)tv.tv_usec;
  stoptime += (long long)(musec+1);
  while (stoptime > currenttime) {
    gettimeofday(&tv, 0);
    currenttime = (long long)tv.tv_sec * (long long)1000000;
    currenttime += (long long)tv.tv_usec;
  }
#else
  // Principially this routine should work for all POSIX compatible systems,
  // but not tested yet (20050222 - RA)
  merr<<"There is no BusyWait function implemented for this OS!"<<endl;
#endif
}


////////////////////////////////////////////////////////////////////////////////


bool MSystem::GetTime(long int& Seconds, long int& NanoSeconds)
{
  // Return the current time in seconds/nanoseconds 

#ifdef ___UNIX___
  time_t t;
  struct timeval tv;
  struct tm *tp;

  // Initializing the time has to be done with two functions since we want both
  // microsecond precision and the current date:
  while (true) {
    // get microsecond precision:
    gettimeofday(&tv, 0);
    
    // and date:
    t = time(0);
    tp = localtime(&t);
    
    // Test if we have overlap i.e. we got the data within the same second:
    if ((tp->tm_min == (tv.tv_sec % 3600) / 60) && (tp->tm_sec == tv.tv_sec % 60)) {
      Seconds = t;
      NanoSeconds = tv.tv_usec*1000;
      break;
    }
  }
#else
  Seconds = time(NULL);
  NanoSeconds = 0;
  mimp<<"No support for nanoseconds on windows!"<<show;
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////


//! Return the model name of the CPU, empty if it is unknown
MString MSystem::GetCpuModel()
{
#ifdef __APPLE__
  char Brand[256];
  size_t Size = sizeof(Brand);
  if (sysctlbyname("machdep.cpu.brand_string", Brand, &Size, nullptr, 0) == 0) {
    return Brand;
  }
  return "";
#else
  ifstream In("/proc/cpuinfo");
  string Line;
  while (getline(In, Line)) {
    if (Line.compare(0, 10, "model name") == 0) {
      size_t Colon = Line.find(':');
      if (Colon != string::npos) {
        return Line.substr(Colon + 1).c_str();
      }
    }
  }
  return "";
#endif
}

////////////////////////////////////////////////////////////////////////////////


int MSystem::GetRAM()
{
  // Return the amount of installed RAM or -1 if it can not be detrmined

  GetMemory();

  return m_RAM;
}


////////////////////////////////////////////////////////////////////////////////


int MSystem::GetFreeRAM()
{
  // Return the amount of free RAM or -1 if it can not be detrmined

  GetMemory();

  return m_FreeRAM;
}


////////////////////////////////////////////////////////////////////////////////


int MSystem::GetSwap()
{
  // Return the amount of installed Swap or -1 if it can not be detrmined

  GetMemory();   

  return m_Swap;
}


////////////////////////////////////////////////////////////////////////////////


int MSystem::GetFreeSwap()
{
  // Return the amount of free swap or -1 if it can not be determined

  GetMemory();

  return m_FreeSwap;
}


////////////////////////////////////////////////////////////////////////////////


bool MSystem::GetProcessInfo(int ProcessID)
{
  // Fill some info about this process, e.g. its memory
  //
  // Further development:
  // Method has to return something like MProcessInfo

  ostringstream S;
  S<<"/proc/"<<ProcessID<<"/status";

  // Open the file - c-mode - sorry...
  FILE *PIDStatus;
  if ((PIDStatus = fopen(S.str().c_str(), "r")) == 0) {
    merr<<"Cannot open file '"<<S.str()<<"'! The kernel needs to be compiled with support for the /proc filesystem"<<endl;

    return false;
  }

  int Result = 0; // Storing result required by some compilers
  Result += fscanf(PIDStatus, "%*s %*s");
  Result += fscanf(PIDStatus, "%*s %*c %*s");
  Result += fscanf(PIDStatus, "%*s %*d");
  Result += fscanf(PIDStatus, "%*s %*d");
  Result += fscanf(PIDStatus, "%*s %*d %*d %*d %*d");
  Result += fscanf(PIDStatus, "%*s %*d %*d %*d %*d");
  Result += fscanf(PIDStatus, "%*s %*d %*d %*d %*d");
  Result += fscanf(PIDStatus, "%*s %*d %*s"); // VmSize
  Result += fscanf(PIDStatus, "%*s %*d %*s"); // VmLck
  Result += fscanf(PIDStatus, "%*s %d %*s", &m_ProcessMemory);  // VmRSS
  Result += fscanf(PIDStatus, "%*s %*d %*s"); // VmData
  Result += fscanf(PIDStatus, "%*s %*d %*s"); // VmStk
  Result += fscanf(PIDStatus, "%*s %*d %*s"); // VmExe
  Result += fscanf(PIDStatus, "%*s %*d %*s"); // VmLib
  if (Result != 1) {
    merr<<"Problem scanning process memory..."<<endl;
  }
  
  m_ProcessMemory /= 1024;

  fclose(PIDStatus);



  // The remainings are not needed right now
  /*
  buf.sprintf("/proc/%s/stat", (const char *)info);
  if ((fd = fopen(buf, "r")) == 0)
  {
    error = true;
    errMessage.sprintf(i18n("Cannot open %s!\n"), buf.data());
    return (false);
  }

  fscanf(fd, "%d %*s %c %d %d %*d %d %*d %*u %*u %*u %*u %*u %d %d"
       "%*d %*d %*d %d %*u %*u %*d %u %u",
       (int*) &pid, &status, (int*) &ppid, (int*) &gid, &ttyNo,
       &userTime, &sysTime, &niceLevel, &vm_size, &vm_rss);

  vm_rss = (vm_rss + 3) * PAGE_SIZE;

  fclose(fd);

    buf.sprintf("/proc/%s/cmdline", (const char *)info);
  if ((fd = fopen(buf, "r")) == 0)
  {
    error = true;
    errMessage.sprintf(i18n("Cannot open %s!\n"), buf.data());
    return (false);
  }
  cbuf[0] = '\0';
  fscanf(fd, "%1023[^\n]", cbuf);
  cbuf[1023] = '\0';
  cmdline = cbuf;
  fclose(fd);

  switch (status)
  {
  case 'R':
    statusTxt = i18n("Run");
    break;
  case 'S':
    statusTxt = i18n("Sleep");
    break;
  case 'D': 
    statusTxt = i18n("Disk");
    break;
  case 'Z':
    statusTxt = i18n("Zombie");
    break;
  case 'T': 
    statusTxt = i18n("Stop");
    break;
  case 'W':
    statusTxt = i18n("Swap");
    break;
  default:
    statusTxt = i18n("????");
    break;
  }

  // find out user name with the process uid
  struct passwd* pwent = getpwuid(uid);
  if (pwent)
    userName = pwent->pw_name;
  */

  return true; 
}


////////////////////////////////////////////////////////////////////////////////


bool MSystem::GetFileSuffix(MString Filename, MString* Suffix)
{
  // extract the suffix from a filename

  if (Filename.Last('.') < Filename.Last('/')) {
    *Suffix = MString("");
    return true;
  }
      
  *Suffix = MString(Filename.Replace(0, Filename.Last('.')+1, ""));
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSystem::GetFileDirectory(MString Filename, MString* Directory)
{
  *Directory = MFile::GetDirectoryName(Filename);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSystem::GetFileWithoutSuffix(MString Filename, MString* NewFilename)
{
  //*NewFilename = gSystem->DirName((char *) Filename.Data());
  *NewFilename = MString(Filename.Replace(Filename.Last('.'), Filename.Length(), ""));
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSystem::FileExist(MString Filename)
{
  return MFile::Exists(Filename);
}


////////////////////////////////////////////////////////////////////////////////


MString MSystem::GetOS()
{
  MString Result;

  array<char, 128> Buffer;
  unique_ptr<FILE, decltype(&pclose)> Pipe(popen("uname -sr", "r"), pclose);
  if (!Pipe) {
     mout<<"Error: Unable to open pipe"<<endl;
     return Result;
  }
  while (fgets(Buffer.data(), Buffer.size(), Pipe.get()) != nullptr) {
    Result += Buffer.data();
  }

  Result.TrimInPlace();

  return Result;
}


// MSystem: the end...
////////////////////////////////////////////////////////////////////////////////
