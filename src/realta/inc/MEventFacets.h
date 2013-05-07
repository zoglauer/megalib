/*
 * MEventFacets.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEventFacets__
#define __MEventFacets__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MHitEvent;
class MPhysicalEvent;
class MBPData;

////////////////////////////////////////////////////////////////////////////////


class MEventFacets : public TObject
{
  // public interface:
 public:
  MEventFacets();
  ~MEventFacets();

  void SetHitEvent(MHitEvent *Event);
  MHitEvent* GetHitEvent();

  void SetPhysicalEvent(MPhysicalEvent *Event);
  MPhysicalEvent* GetPhysicalEvent();

  void SetBackprojectionEvent(MBPData *Event);
  MBPData* GetBackprojectionEvent();

  Bool_t IsFullyAnalyzed();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MHitEvent *m_HitEvent;
  MPhysicalEvent *m_PhysicalEvent;
  MBPData *m_BackprojectionEvent;

  Bool_t m_FullyAnalyzed;


#ifdef ___CINT___
 public:
  ClassDef(MEventFacets, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
