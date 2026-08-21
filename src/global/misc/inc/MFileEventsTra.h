/*
 * MFileEventsTra.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileEventsTra__
#define __MFileEventsTra__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TThread.h"

// Standrad libs
#include <list>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEventsTra : public MFileEvents
{
  // public interface:
 public:
  MFileEventsTra();
  virtual ~MFileEventsTra();

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read, bool IsBinary = false);

  //! Close the file and kill the thread (if it (still) exists)
  virtual bool Close();

  //! Enable or disable fast file (and thus unsecure) file parsing
  void SetFastFileParsing(bool Fast) { m_Fast = Fast; }

  //! Enable or disable delayed file parsing
  void SetDelayedFileParsing(bool ParseDelayed) { m_ParseDelayed = ParseDelayed; }
  
  //! If you want this class to be multi-threaded, call this function
  bool StartThread();

  MPhysicalEvent* GetNextEvent();

  void* ThreadedReading();

  bool AddEvent(MPhysicalEvent* P);
  bool AddText(const MString& Text);

  //! Return the number of simulated events
  //! Attention: This information is only complete after all files have been scanned,
  //! i.e. after GetNextEvent() has been called for all events!
  //! In addition, this is the number for the total file(s), NOT up to the just read event!!
  virtual long GetSimulatedEvents();

  // protected methods:
 protected:
  MPhysicalEvent* ReadNextEvent();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  int m_EventType;

  bool m_Threaded;
  bool m_StopThread;
  TThread* m_Thread;
  
  //! True if the event sshould be read in fast mode (i.e. without fault tolerance)
  bool m_Fast;

  //! True if the event parsing should happen delayed
  bool m_ParseDelayed;

  //! True as long as EOF is not reached, or cancel was not pressed.
  bool m_MoreEvents;

  //! The data set store
  list<MPhysicalEvent*> m_DataSets;
  //! the current number of data sets (stored to avoid frequent calls to m_DataSets.size())
  unsigned int m_NDataSets;
  //! The aspired minimum number of events in the data set - if below we start reading events again
  unsigned int m_MinimumNDataSets;
  //! The maximum allow "minimum allowed number of data sets" --- to prevent an unlimited allocation
  //! This is the maximum number to which m_MinimumNDataSets is allowed to grow
  unsigned int m_MinimumNDataSetsMax;


#ifdef ___CLING___
 public:
  ClassDef(MFileEventsTra, 0) // no description
#endif

};

////////////////////////////////////////////////////////////////////////////////

class MFileEventsTra_ThreadCaller
{
 public:
  //! Standard constructor
  MFileEventsTra_ThreadCaller(MFileEventsTra* File) {
    m_File = File;
  }

  //! Return the calling class
  MFileEventsTra* GetThreadCaller() { return m_File; }

 private:
  //! Store the calling class for retrieval
  MFileEventsTra* m_File;
};

////////////////////////////////////////////////////////////////////////////////

void MFileEventsTra_CallThread(void* address);


#endif


////////////////////////////////////////////////////////////////////////////////
