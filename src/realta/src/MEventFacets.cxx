/*
 * MEventFacets.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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


#ifdef ___CLING___
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
