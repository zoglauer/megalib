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
#include <cstring>
#include <cstddef>
#include <cerrno>
#include <ctime>
#include <chrono>
#include <future>
#include <memory>
#include <system_error>
#include <thread>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
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


//! Check whether a usable X11 display is reachable from this process.
//!
//! Returns true if the X11 socket named by $DISPLAY accepts a connection,
//! false otherwise. Three DISPLAY forms are recognised:
//!
//!   - ":<n>" or "unix:<n>"       -> /tmp/.X11-unix/X<n> (standard Linux/BSD).
//!                                   On Linux, the abstract-socket variant
//!                                   (sun_path[0] = '\0') is tried as a
//!                                   fallback, matching libX11: some hardened
//!                                   X server configurations expose only the
//!                                   abstract socket.
//!   - "/<absolute path>:<n>"     -> literally "<path>:<n>" (XQuartz on macOS
//!                                   publishes DISPLAY this way, e.g.
//!                                   "/private/tmp/com.apple.launchd.XXX/org.xquartz:0")
//!   - "<host>:<n>" or "<ip>:<n>" -> TCP to (host, 6000 + n). Used by SSH
//!                                   X11 forwarding, remote sessions, VNC
//!                                   with X. A non-blocking connect() bounded
//!                                   by a 2 s select() correctly distinguishes
//!                                   a live tunnel from a stale one (e.g. a
//!                                   screen session whose SSH connection has
//!                                   gone away). Every address returned by
//!                                   getaddrinfo is tried, not just the
//!                                   first -- the order can prefer IPv6 even
//!                                   when (e.g.) SSH only listens on IPv4.
//!
//! Why no libX11 / no fork: calling XOpenDisplay can block for many
//! seconds on a stale DISPLAY, and forking around it is fragile in a
//! multithreaded parent (inherited locked mutexes, Apple's documented
//! fork-without-exec restriction). libX11 is also not part of a default
//! macOS install. A direct connect() probe avoids both problems.
//!
//! What this does *not* detect: a successful connect proves only that
//! the X *socket* is alive. It does not run the X11 protocol handshake
//! or verify authentication (e.g. a stale .Xauthority); it also cannot
//! tell an actual X server from an unrelated service squatting on TCP
//! port 6000 + n. If the caller later passes the display to libX11 /
//! ROOT and the handshake fails, that failure is the caller's to handle.
//!
//! DNS handling: getaddrinfo() has no built-in timeout, so it is run on
//! a detached background thread and waited on with a 2 s timeout. If
//! the resolver doesn't answer in time we report no display and let the
//! thread finish on its own; if its answer arrives after we've given up
//! the resulting addrinfo* is abandoned (a small, bounded, rare leak).
bool MSystem::HasDisplay()
{
#ifdef ___UNIX___
  const char* DisplayEnv = getenv("DISPLAY");
  if (DisplayEnv == nullptr || DisplayEnv[0] == '\0') {
    cout<<"Display not found: DISPLAY environment variable is unset or empty"<<endl;
    return false;
  }

  // Parse [host]:display[.screen]. Empty host or "unix" -> local UNIX
  // socket; absolute-path host -> XQuartz-style local socket; anything
  // else -> TCP target.
  string Display(DisplayEnv);
  string::size_type Colon = Display.rfind(':');
  if (Colon == string::npos) {
    cout<<"Display not found: DISPLAY=\""<<DisplayEnv<<"\" does not contain a ':' separator"<<endl;
    return false;
  }
  string Host = Display.substr(0, Colon);
  string Rest = Display.substr(Colon + 1);
  string::size_type Dot = Rest.find('.');
  string DisplayNumStr = (Dot == string::npos) ? Rest : Rest.substr(0, Dot);
  char* End = nullptr;
  long DisplayNum = strtol(DisplayNumStr.c_str(), &End, 10);
  if (End == DisplayNumStr.c_str() || *End != '\0' || DisplayNum < 0) {
    cout<<"Display not found: DISPLAY=\""<<DisplayEnv<<"\" has a malformed display number \""<<DisplayNumStr<<"\""<<endl;
    return false;
  }

  // -------------------------------------------------------------------
  // Local UNIX socket path. A blocking connect() is fine here: the
  // kernel answers immediately for a local socket -- either success or
  // ECONNREFUSED / ENOENT -- so no timeout machinery is needed.
  // -------------------------------------------------------------------
  if (Host.empty() || Host == "unix" || Host[0] == '/') {
    string SocketPath;
    if (Host.empty() || Host == "unix") {
      // Standard /tmp/.X11-unix/X<n> layout.
      SocketPath = "/tmp/.X11-unix/X" + DisplayNumStr;
    } else {
      // Absolute-path host: XQuartz on macOS publishes DISPLAY this way,
      // e.g. "/private/tmp/com.apple.launchd.XXX/org.xquartz:0". The
      // actual socket file is the host with the ":<n>" suffix appended.
      SocketPath = Host + ":" + DisplayNumStr;
    }

    // Reject paths that would have to be truncated into sun_path; a silent
    // truncation would connect to the wrong socket or none. (libX11 hits
    // the same kernel limit, so this is not stricter than Xlib.)
    sockaddr_un Probe;
    if (SocketPath.size() + 1 > sizeof(Probe.sun_path)) {
      cout<<"Display not found: resolved socket path \""<<SocketPath<<"\" is too long for sockaddr_un.sun_path"<<endl;
      return false;
    }

    bool LocalOk = false;

    // Attempt 1: filesystem socket at the resolved path.
    {
      int Sock = socket(AF_UNIX, SOCK_STREAM, 0);
      if (Sock < 0) {
        cout<<"Display not found: failed to create AF_UNIX socket"<<endl;
        return false;
      }
      sockaddr_un Addr;
      memset(&Addr, 0, sizeof(Addr));
      Addr.sun_family = AF_UNIX;
      memcpy(Addr.sun_path, SocketPath.c_str(), SocketPath.size() + 1);
      socklen_t AddrLen = (socklen_t) (offsetof(sockaddr_un, sun_path)
                                       + SocketPath.size() + 1);
      LocalOk = (connect(Sock, reinterpret_cast<sockaddr*>(&Addr), AddrLen) == 0);
      close(Sock);
    }

#ifdef __linux__
    // Attempt 2 (Linux only): abstract socket. The leading NUL byte in
    // sun_path marks the address as living in the abstract namespace; the
    // rest of sun_path is the literal abstract name (not NUL-terminated --
    // the kernel uses AddrLen as the boundary). libX11 falls back to this
    // when the filesystem socket isn't reachable, and some hardened X
    // server configurations expose only the abstract socket. Only
    // meaningful for the standard /tmp/.X11-unix/X<n> form; XQuartz's
    // launchd-path DISPLAY has no abstract counterpart.
    if (LocalOk == false && (Host.empty() || Host == "unix")) {
      int Sock = socket(AF_UNIX, SOCK_STREAM, 0);
      if (Sock >= 0) {
        sockaddr_un Addr;
        memset(&Addr, 0, sizeof(Addr));
        Addr.sun_family = AF_UNIX;
        Addr.sun_path[0] = '\0';
        memcpy(&Addr.sun_path[1], SocketPath.c_str(), SocketPath.size());
        socklen_t AddrLen = (socklen_t) (offsetof(sockaddr_un, sun_path)
                                         + 1 + SocketPath.size());
        LocalOk = (connect(Sock, reinterpret_cast<sockaddr*>(&Addr), AddrLen) == 0);
        close(Sock);
      }
    }
#endif

    if (LocalOk == false) {
      cout<<"Display not found: connect() to UNIX socket \""<<SocketPath<<"\" failed"
#ifdef __linux__
          <<" (filesystem and abstract)"
#endif
          <<endl;
    }
    return LocalOk;
  }

  // -------------------------------------------------------------------
  // TCP target: hostname or IP literal. Used by SSH X11 forwarding and
  // remote sessions. Must use a non-blocking connect() bounded by
  // select() so a stale tunnel can't hang startup; the kernel's TCP SYN
  // timeout (~75 s on Linux) is far too long for a startup probe.
  // -------------------------------------------------------------------
  addrinfo Hints;
  memset(&Hints, 0, sizeof(Hints));
  Hints.ai_family = AF_UNSPEC;        // accept IPv4 or IPv6
  Hints.ai_socktype = SOCK_STREAM;

  char PortStr[16];
  snprintf(PortStr, sizeof(PortStr), "%ld", 6000 + DisplayNum);

  // getaddrinfo() has no built-in timeout, so a misbehaving DNS resolver
  // can hang us for many seconds. Run it on a detached background thread
  // and wait at most 2 s for the answer via a packaged_task + future.
  // On timeout we report no display and let the thread finish on its
  // own; if the lookup eventually succeeds in the background, its
  // addrinfo* result is never consumed and leaks. Rare, bounded, and
  // acceptable for a startup probe.
  addrinfo* Resolved = nullptr;
  {
    struct ResolverResult { int Rc; addrinfo* Out; };
    const string ThreadHost = Host;
    const string ThreadPort = PortStr;
    const addrinfo ThreadHints = Hints;

    auto Task = make_shared<packaged_task<ResolverResult()>>(
      [ThreadHost, ThreadPort, ThreadHints]() {
        ResolverResult R{0, nullptr};
        R.Rc = getaddrinfo(ThreadHost.c_str(), ThreadPort.c_str(),
                           &ThreadHints, &R.Out);
        return R;
      });
    future<ResolverResult> Future = Task->get_future();
    try {
      thread([Task]() { (*Task)(); }).detach();
    } catch (const system_error&) {
      // Thread construction can throw on resource exhaustion (very rare).
      // Fail closed rather than letting the exception escape startup.
      cout<<"Display not found: failed to spawn DNS resolver thread"<<endl;
      return false;
    }

    if (Future.wait_for(chrono::seconds(2)) != future_status::ready) {
      cout<<"Display not found: getaddrinfo(\""<<Host<<"\") timed out after 2 s"<<endl;
      return false;
    }
    ResolverResult R = Future.get();
    if (R.Rc != 0 || R.Out == nullptr) {
      cout<<"Display not found: getaddrinfo(\""<<Host<<"\") failed"<<endl;
      return false;
    }
    Resolved = R.Out;
  }

  // Try every address returned by getaddrinfo. The first one isn't
  // guaranteed to be the one the X server is listening on (SSH X11
  // forwarding, for instance, commonly listens only on 127.0.0.1 even
  // when getaddrinfo prefers ::1). A single 2 s wall-clock deadline is
  // shared across all addresses, so total TCP probe time is bounded
  // regardless of how many addresses getaddrinfo returned. Both connect()
  // and select() retry on EINTR until the deadline expires.
  auto Deadline = chrono::steady_clock::now() + chrono::seconds(2);

  bool Ok = false;
  for (addrinfo* It = Resolved; It != nullptr && Ok == false; It = It->ai_next) {
    if (chrono::steady_clock::now() >= Deadline) break;

    int Sock = socket(It->ai_family, It->ai_socktype, It->ai_protocol);
    if (Sock < 0) continue;

    // Fail closed if fcntl fails: a blocking connect() would reintroduce
    // the startup hang this function exists to avoid.
    int Flags = fcntl(Sock, F_GETFL, 0);
    if (Flags < 0 || fcntl(Sock, F_SETFL, Flags | O_NONBLOCK) < 0) {
      close(Sock);
      continue;
    }

    // Retry connect() on EINTR; EINPROGRESS means "wait via select()".
    int ConnectResult;
    do {
      ConnectResult = connect(Sock, It->ai_addr, It->ai_addrlen);
    } while (ConnectResult < 0 && errno == EINTR);

    if (ConnectResult == 0) {
      // Completed synchronously (rare for TCP, but possible on loopback).
      Ok = true;
      close(Sock);
      break;
    }
    if (errno != EINPROGRESS) {
      close(Sock);
      continue;
    }

    // Wait for the asynchronous connect against the shared deadline.
    // Retry select() on EINTR until the deadline expires, so a spurious
    // signal doesn't falsely report no display.
    while (true) {
      auto Remaining = Deadline - chrono::steady_clock::now();
      if (Remaining <= chrono::steady_clock::duration::zero()) break;
      auto Secs = chrono::duration_cast<chrono::seconds>(Remaining);
      auto Usec = chrono::duration_cast<chrono::microseconds>(Remaining - Secs);
      timeval Timeout;
      Timeout.tv_sec = (time_t) Secs.count();
      Timeout.tv_usec = (suseconds_t) Usec.count();
      fd_set WriteSet;
      FD_ZERO(&WriteSet);
      FD_SET(Sock, &WriteSet);
      int SelectResult = select(Sock + 1, nullptr, &WriteSet, nullptr, &Timeout);
      if (SelectResult > 0) {
        // Writable either on success or on async failure; ask SO_ERROR.
        int SockErr = 0;
        socklen_t SockErrLen = sizeof(SockErr);
        if (getsockopt(Sock, SOL_SOCKET, SO_ERROR, &SockErr, &SockErrLen) == 0
            && SockErr == 0) {
          Ok = true;
        }
        break;
      }
      if (SelectResult < 0 && errno == EINTR) continue;
      break;  // 0 (timeout) or unrecoverable error
    }

    close(Sock);
  }

  freeaddrinfo(Resolved);
  if (Ok == false) {
    cout<<"Display not found: no resolved TCP address for "<<Host<<":"<<(6000 + DisplayNum)<<" was reachable within 2 s total"<<endl;
  }
  return Ok;

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
