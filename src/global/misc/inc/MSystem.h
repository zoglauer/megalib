/*
 * MSystem.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  
  //! Return the OS version and name via "uname -rs"
  static MString GetOS();

  // protected methods:
 protected:
  bool GetMemory();
  bool GetProcessInfo(int ProcessID);
  void Reset();


  // private methods:
 private:



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
