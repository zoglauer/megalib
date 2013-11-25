/*
 * MFileEventsRawa.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileEventsRawa__
#define __MFileEventsRawa__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using std::vector;

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEventsRawa : public MFileEvents
{
  // public interface:
 public:
  MFileEventsRawa();
  virtual ~MFileEventsRawa();

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read);

  bool LoadNextEvent();
 
  // Interface to the raw event data:
  int GetEventId() { return m_EventId; }

  int GetTimeWalk() { return m_TimeWalk; }
  int GetTimeTillD2Trigger() { return m_TimeTillD2Trigger; }
  vector<MString> GetTriggerPattern() { return m_TriggerMask; }
  bool HasTriggered(MString Chip);

  unsigned int GetNPulseFormAdcs() { return m_PulseFormAdcs.size(); }
  MString GetPulseFormAdcAt(unsigned int i) { return m_PulseFormAdcs[i]; } // no bound check!
  vector<int> GetPulseFormAt(unsigned int i) { return m_PulseForms[i]; } // no bound check!
  vector<int> GetPulseFormFor(MString Adc);
  


  // protected methods:
 protected:
  bool OpenIncludeFile(const MString& Line);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  int m_EventId;
  int m_TimeWalk;
  int m_TimeTillD2Trigger;

  vector<MString> m_PulseFormAdcs;
  vector<vector<int> > m_PulseForms;

  vector<MString> m_TriggerMask;

#ifdef ___CINT___
 public:
  ClassDef(MFileEventsRawa, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
