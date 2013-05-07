/*
 * MPairEventData.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPairEventData__
#define __MPairEventData__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPairEventData : public MPhysicalEvent
{
  // Public Interface:
 public:
  MPairEventData();
  virtual ~MPairEventData();



  // protected members:
 public:



#ifdef ___CINT___
 public:
  ClassDef(MPairEventData, 0)        // basic data of a pair event
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
