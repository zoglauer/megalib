/*
 * MProjection.cxx
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
  
  m_Event = 0;
  m_C = 0;
  m_P = 0;
  m_Photo = 0;

  m_ApproximatedMaths = false;
}


////////////////////////////////////////////////////////////////////////////////


MProjection::~MProjection()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MProjection::Assimilate(MPhysicalEvent* Event)
{
  // Now we have to check which kind of data we want to backproject
  massert(Event != 0);
  m_Event = Event;

  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    m_C = dynamic_cast<MComptonEvent*>(Event);
  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    m_P = dynamic_cast<MPairEvent*>(Event);
  } else if (Event->GetType() == MPhysicalEvent::c_Photo) {
    m_Photo = dynamic_cast<MPhotoEvent*>(Event);
  } else {
    cout<<"Unhandled event type: "<<Event->GetTypeString()<<endl;
    return false;
  }

  return true;
}


// MProjection: the end...
////////////////////////////////////////////////////////////////////////////////
