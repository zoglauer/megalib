/*
 * MRealTimeEvent.cxx
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
// MRealTimeEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRealTimeEvent.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MRealTimeEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MRealTimeEvent::MRealTimeEvent()
{
  // Construct an instance of MRealTimeEvent
  
  m_IsInitialized = false;
  m_IsCoincident = false;
  m_IsReconstructed = false;
  m_IsImaged = false;
  
  m_IsDropped = false;
  m_IsMerged = false;
  
  m_InitialRawEvent = 0;
  m_CoincidentRawEvent = 0;
  m_PhysicalEvent = 0;
  m_Backprojection = 0;
}


////////////////////////////////////////////////////////////////////////////////


MRealTimeEvent::~MRealTimeEvent()
{
  // Delete this instance of MRealTimeEvent
  
  delete m_InitialRawEvent;
  delete m_CoincidentRawEvent;
  delete m_PhysicalEvent;
  delete m_Backprojection; 
}


// MRealTimeEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
