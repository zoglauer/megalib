/*
 * MHitEventAnalyzer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MHitEventAnalyzer__
#define __MHitEventAnalyzer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MHitEvent;
class MRawEventIncarnations;
class MPhysicalEvent;
class MDGeometryQuest;
class MGeometryBasic;


////////////////////////////////////////////////////////////////////////////////


class MHitEventAnalyzer : public TObject
{
  // public interface:
 public:
  MHitEventAnalyzer(MDGeometryQuest* Geo);
  ~MHitEventAnalyzer();

  Bool_t Analyze(MHitEvent* Event);
  MPhysicalEvent* GetPhysicalEvent();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MRawEventIncarnations *m_RawEvents;  // array of all "possible" events
  MDGeometryQuest *m_Geometry;
  MGeometryBasic *m_Geo;


#ifdef ___CLING___
 public:
  ClassDef(MHitEventAnalyzer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
