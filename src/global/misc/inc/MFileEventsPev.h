/*
 * MFileEventsPev.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileEventsPev__
#define __MFileEventsPev__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEventsPev : public MFileEvents
{
  // public interface:
 public:
  MFileEventsPev(MString FileName, unsigned int Way = 1);
  virtual ~MFileEventsPev();

  virtual bool OpenRoot(unsigned int Way = 1);

  MPhysicalEvent* GetNextEvent();
  void AddEvent(MPhysicalEvent* Pev);


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MFileEventsPev, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
