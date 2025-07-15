/*
 * MFileEventsType.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileEventsType__
#define __MFileEventsType__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEventsType : public MFileEvents
{
  // public interface:
 public:
  MFileEventsType(MString GeometryFileName="None");
  virtual ~MFileEventsType();

  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read, bool IsBinary = false);
  virtual bool Close();

  bool AddEvent(long EventId, int EventType, double EventTypeProbability);
  bool GetNextEvent();// fills private members with file content

  // Access private members
  long GetEventId();
  int GetEventType();
  double GetEventTypeProbability();
  // private methods:
 private:
  void Reset();

  // private members:
 private:
  MString m_GeometryFileName;

  long m_EventId;
  int m_EventType;
  double m_EventTypeProbability;

  static const long c_NoId;
  bool m_IsFirstEvent;


#ifdef ___CLING___
 public:
  ClassDef(MFileEventsType, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
