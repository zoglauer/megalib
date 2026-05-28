/*
 * MSystem.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MSystem
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSystem.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TSystem.h"

// MEGAlib libs:
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
    void* Lib = dlopen("libX11.so.6", RTLD_LAZY | RTLD_LOCAL);
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
          cout<<"Display not found: libX11 is not installed (dlopen of libX11.so.6 failed)"<<endl;
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


int MSystem::RunChildProcess(const MString& Executable, const MString& Argument, const MString& OutputFileName)
{
  pid_t Child = fork();
  if (Child == 0) {
    if (OutputFileName.IsEmpty() == false) {
      int Log = open(OutputFileName.Data(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (Log >= 0) {
        dup2(Log, STDOUT_FILENO);
        dup2(Log, STDERR_FILENO);
        close(Log);
      }
    }

    MString Command = Executable + " " + Argument;
    execl("/bin/sh", "sh", "-c", Command.Data(), static_cast<char*>(0));
    // If execl() returns, the child failed to start the command. Use _exit()
    // after fork() to avoid running parent-owned C++ cleanup/stdio flushing.
    // Exit code 127 is the shell convention for "command could not be run".
    _exit(127);
  }

  if (Child < 0) {
    return -1;
  }

  int ChildStatus = 0;
  if (waitpid(Child, &ChildStatus, 0) < 0) {
    return -1;
  }

  return ChildStatus;
}


////////////////////////////////////////////////////////////////////////////////


void MSystem::Reset()
{
  // Reset all values to default:

  m_RAM = -1;
  m_FreeRAM = -1;
  m_Swap = -1;
  m_FreeSwap = -1;
}


////////////////////////////////////////////////////////////////////////////////


bool MSystem::FreeMemory(int &Free)
{
#ifdef ___UNIX___

  /*
   * The amount of total and used memory is read from the /proc/meminfo.
   * It also contains the information about the swap space.
   * The 'file' looks like this:
   *
   *         total:    used:    free:  shared: buffers:  cached:
   * Mem:  64593920 60219392  4374528 49426432  6213632 33689600
   * Swap: 69636096   761856 68874240
   * MemTotal:     63080 kB
   * MemFree:       4272 kB
   * MemShared:    48268 kB
   * Buffers:       6068 kB
   * Cached:       32900 kB
   * SwapTotal:    68004 kB
   * SwapFree:     67260 kB
   */

  int total, used, mfree, buffers, cached;
  
  FILE* meminfo;
  
  if ((meminfo = fopen("/proc/meminfo", "r")) == NULL) {
    Warning("bool MSystem::FreeMemory(int &Free)",
            "Cannot open file \'/proc/meminfo\'!\n"
            "The kernel needs to be compiled with support\n"
            "for /proc filesystem enabled!");
    Free = -1;
    return false;
  }

  if (fscanf(meminfo, "%*[^\n]\n") == EOF) {
    Warning("bool MSystem::FreeMemory(int &Free)",
            "Cannot read memory info file \'/proc/meminfo\'!\n");
    Free = -1;
    fclose(meminfo);
    return false;
  }

  /*
   * The following works only on systems with 4GB or less. Currently this
   * is no problem but what happens if Linus changes his mind?
   */
  if (fscanf(meminfo, "%*s %d %d %d %*d %d %d\n",
             &total, &used, &mfree, &buffers, &cached) != 5) {
    Free = -1;
    fclose(meminfo);
    return false;
  }
  
  total /= 1024;
  mfree /= 1024;
  used /= 1024;
  buffers /= 1024;
  cached /= 1024;
  
  fclose(meminfo);
  
  Free = mfree + buffers + cached;
  return true;

#else

  // If we do not have a Linux-system
  Free = -1;
  return false;

#endif
}


////////////////////////////////////////////////////////////////////////////////


bool MSystem::GetMemory()
{
  // Fill all memory variables 
  // Return false if an error occurred

  // Check if it's update time:
  //cout<<(gSystem->Now() - m_LastCheck).AsString()<<"!"<<m_CheckInterval.AsString()<<endl;
  //if ((long) (gSystem->Now() - m_LastCheck) < (long) m_CheckInterval) {
  //cout<<(gSystem->Now() - m_LastCheck).AsString()<<"!"<<m_CheckInterval.AsString()<<endl;
  //return true;
  //} 
  //cout<<gSystem->Now().AsString()<<"!"<<m_LastCheck.AsString()<<"!"<<m_CheckInterval.AsString()<<endl;

#ifdef ___UNIX___

  /*
   * The amount of total and used memory is read from the /proc/meminfo.
   * It also contains the information about the swap space.
   * The 'file' looks like this:
   *
   *         total:    used:    free:  shared: buffers:  cached:
   * Mem:  64593920 60219392  4374528 49426432  6213632 33689600
   * Swap: 69636096   761856 68874240
   * MemTotal:     63080 kB
   * MemFree:       4272 kB
   * MemShared:    48268 kB
   * Buffers:       6068 kB
   * Cached:       32900 kB
   * SwapTotal:    68004 kB
   * SwapFree:     67260 kB
   */

  int total, used, mfree, buffers, cached;
  
  FILE* meminfo;
  
  if ((meminfo = fopen("/proc/meminfo", "r")) == NULL) {
    Warning("bool MSystem::FreeMemory(int &Free)",
            "Cannot open file \'/proc/meminfo\'!\n"
            "The kernel needs to be compiled with support\n"
            "for /proc filesystem enabled!");
    Reset();
    return false;
  }

  if (fscanf(meminfo, "%*[^\n]\n") == EOF) {
    Warning("bool MSystem::FreeMemory(int &Free)",
            "Cannot read memory info file \'/proc/meminfo\'!\n");
    Reset();
    fclose(meminfo);
    return false;
  }

  // Read the RAM information:
  if (fscanf(meminfo, "%*s %d %d %d %*d %d %d\n",
             &total, &used, &mfree, &buffers, &cached) != 5) {
    Reset();
    fclose(meminfo);
    return false;
    merr<<"Unable to read /proc/meminfo... What Kernel are you using???"<<endl;
  }

  total /= 1048576;
  mfree /= 1048576;
  used /= 1048576;
  buffers /= 1048576;
  cached /= 1048576;

  m_RAM = total;
  m_FreeRAM = mfree + buffers + cached;

  // Read the swap information:
  if (fscanf(meminfo, "%*s %d %d %d\n",
             &total, &used, &mfree) != 3) {
    Reset();
    fclose(meminfo);
    return false;
    merr<<"Unable to read /proc/meminfo... What Kernel are you using???"<<endl;    
  }

  total /= 1048576;
  mfree /= 1048576;
  used /= 1048576;

  m_Swap = total;
  m_FreeSwap = mfree;
  
  fclose(meminfo);
  
  return true;

#else

  // If we do not have a Linux-system
  Reset();
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
    Warning("bool MSystem::GetProcessInfo(int ProcessID)",
            "Cannot open file \'%s\'!\n"
            "The kernel needs to be compiled with support\n"
            "for /proc filesystem enabled!", S.str().c_str());

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
  //  *Directory = MString(Filename.Replace(0, Filename.Last('/'), ""));
  *Directory = MString(gSystem->BaseName((char *) Filename.Data()));

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
  // Return true if the file exists in the current directory (selected in the dialog)

  if (Filename == gSystem->DirName((char *) Filename.Data())) {
    return false;
  }

  FILE *File;
  if ((File = fopen((char *) Filename.Data(), "r")) == NULL)
    return false;

  fclose(File);
  return true;
}


// MSystem: the end...
////////////////////////////////////////////////////////////////////////////////
