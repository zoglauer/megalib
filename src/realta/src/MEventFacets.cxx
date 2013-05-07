/*
 * MEventFacets.cxx                                   v0.1  01/01/2001
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MEventFacets
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEventFacets.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MHitEvent.h"
#include "MPhysicalEvent.h"
#include "MBPData.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MEventFacets)
#endif


////////////////////////////////////////////////////////////////////////////////


MEventFacets::MEventFacets()
{
  // Construct an instance of MEventFacets

  m_HitEvent = 0;
  m_PhysicalEvent = 0;
  m_BackprojectionEvent = 0;

  m_FullyAnalyzed = kFALSE;
}


////////////////////////////////////////////////////////////////////////////////


MEventFacets::~MEventFacets()
{
  // Delete this instance of MEventFacets

  delete m_HitEvent;
  delete m_PhysicalEvent;
  delete m_BackprojectionEvent;
}


////////////////////////////////////////////////////////////////////////////////


void MEventFacets::SetHitEvent(MHitEvent *Event)
{ 
  //

  m_HitEvent = Event;
}

////////////////////////////////////////////////////////////////////////////////


MHitEvent* MEventFacets::GetHitEvent()
{
  //

  return m_HitEvent;
}


////////////////////////////////////////////////////////////////////////////////


void MEventFacets::SetPhysicalEvent(MPhysicalEvent *Event)
{
  //

  m_PhysicalEvent = Event;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MEventFacets::GetPhysicalEvent()
{
  //

  return m_PhysicalEvent;
}


////////////////////////////////////////////////////////////////////////////////


void MEventFacets::SetBackprojectionEvent(MBPData *Event)
{
  //

  m_BackprojectionEvent = Event;
}


////////////////////////////////////////////////////////////////////////////////


MBPData* MEventFacets::GetBackprojectionEvent()
{
  //

  return m_BackprojectionEvent;
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MEventFacets::IsFullyAnalyzed()
{
  //

  if (m_HitEvent != 0 &&
      m_PhysicalEvent != 0 &&
      m_BackprojectionEvent != 0) {
    return kTRUE;
  } else {
    return kFALSE;
  }
}


////////////////////////////////////////////////////////////////////////////////



// MEventFacets.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
