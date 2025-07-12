/*
 * MEREventTypeExternal.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEREventTypeExternal__
#define __MEREventTypeExternal__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MEREventType.h"
#include "MFileEventsType.h"
#include "MRawEventIncarnations.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MEREventTypeExternal : public MEREventType
{
  // public interface:
 public:
  MEREventTypeExternal();
  virtual ~MEREventTypeExternal();

  //! Global parameters used by all electron tracking algorithms
  virtual void SetParameters(MString EventTypeFileName);
  bool Analyze(MRawEventIncarnations* List);

  bool PostAnalysis();
  MString ToString(bool CoreOnly = false) const;


  // protected members:
 protected:
  MString m_EventTypeFileName;
  MFileEventsType* m_FileEventsType;


#ifdef ___CLING___
 public:
  ClassDef(MEREventTypeExternal, 0) // no description
#endif

};

#endif




