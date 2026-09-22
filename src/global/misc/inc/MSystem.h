/*
 * MSystem.h
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


#ifndef __MSystem__
#define __MSystem__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TTime.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSystem 
{
  // Public Interface:
 public:
  MSystem();
  virtual ~MSystem();

  bool FreeMemory(int &Free);
  int GetRAM();
  int GetFreeRAM();
  int GetSwap();
  int GetFreeSwap();

  static bool GetTime(long int& Seconds, long int& NanoSeconds);
  static void BusyWait(int musec);

  int GetProcessMemory();

  bool FileExist(MString Filename);
  bool GetFileDirectory(MString Filename, MString* Directory);
  bool GetFileSuffix(MString Filename, MString* Suffix);
  bool GetFileWithoutSuffix(MString Filename, MString* NewFilename);
  
  //! Run a child process with arguments and optionally redirect its output to a file
  static int RunChildProcess(const MString& Executable, const MString& Arguments, const MString& OutputFileName = "");

  // protected methods:
 protected:
  bool GetMemory();
  bool GetProcessInfo(int ProcessID);
  void Reset();


  // private methods:
 private:
  //! Test whether an X11 display can be opened.
  //!
  //! INTERNAL: must be called exactly once during MGlobal::Initialize(),
  //! before any worker threads are spawned. The Linux implementation
  //! fork()s and the child then calls dlopen() and XOpenDisplay(); after
  //! fork() in a multithreaded parent both can deadlock on inherited
  //! library/runtime locks. The "early startup" constraint is a
  //! precondition, not advice -- which is why this is private and
  //! MGlobal is the only friended caller.
  static bool HasDisplay();
  friend class MGlobal;


  // protected members:
 protected:


  // private members:
 private:
  int m_RAM;        // Installed RAM
  int m_FreeRAM;    // Free RAM
  int m_Swap;       // Installed Swap-space
  int m_FreeSwap;   // Free Swap-Space

  TTime m_LastCheck;      // Time of last check
  TTime m_CheckInterval;  // Minimum time gap between two checks

  int m_ProcessMemory;


#ifdef ___CLING___
 public:
  ClassDef(MSystem, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
