/*
 * MProjection.cxx
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
// MProjection
//
//
// This is the base class for all Back/Forward-Projection classes of image
// reconstruction. It has two objects: a Compton-event and a Pair-event, but 
// only the right one is active at one time.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MProjection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MProjection)
#endif


////////////////////////////////////////////////////////////////////////////////


MProjection::MProjection(MCoordinateSystem CoordinateSystem)
{
  // Initialize a MProjection
  
  m_CoordinateSystem = CoordinateSystem;
  
  m_Event = nullptr;
  m_C = nullptr;
  m_P = nullptr;
  m_Photo = nullptr;
  m_PET = nullptr;
  m_Multi = nullptr;

  m_ApproximatedMaths = false;
}


////////////////////////////////////////////////////////////////////////////////


MProjection::~MProjection()
{
  // Default destructor
  
  // We don't own any pointers, thus we do nothing.
}


////////////////////////////////////////////////////////////////////////////////


bool MProjection::Assimilate(MPhysicalEvent* Event)
{
  // Now we have to check which kind of data we want to backproject
  
  massert(Event != 0);
  
  m_Event = Event;
  
  // Since we do not own the old events, nullptr them!
  m_C = nullptr;
  m_P = nullptr;
  m_Photo = nullptr;
  m_PET = nullptr;
  m_Multi = nullptr;

  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    m_C = dynamic_cast<MComptonEvent*>(Event);
  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    m_P = dynamic_cast<MPairEvent*>(Event);
  } else if (Event->GetType() == MPhysicalEvent::c_Photo) {
    m_Photo = dynamic_cast<MPhotoEvent*>(Event);
  } else if (Event->GetType() == MPhysicalEvent::c_PET) {
    m_PET = dynamic_cast<MPETEvent*>(Event);
  } else if (Event->GetType() == MPhysicalEvent::c_Multi) {
    m_Multi = dynamic_cast<MMultiEvent*>(Event);
  } else {
    cout<<"Unhandled event type: "<<Event->GetTypeString()<<endl;
    return false;
  }

  return true;
}


// MProjection: the end...
////////////////////////////////////////////////////////////////////////////////
