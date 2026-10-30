/*
 * MRealTimeEvent.cxx
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
