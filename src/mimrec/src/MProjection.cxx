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

  if (Event->GetType() == c_ComptonEvent) {
    m_C = dynamic_cast<MComptonEvent*>(Event);
  } else if (Event->GetType() == c_PairEvent) {
    m_P = dynamic_cast<MPairEvent*>(Event);
  } else if (Event->GetType() == c_PhotoEvent) {
    m_Photo = dynamic_cast<MPhotoEvent*>(Event);
  } else if (Event->GetType() == c_PETEvent) {
    m_PET = dynamic_cast<MPETEvent*>(Event);
  } else if (Event->GetType() == c_MultiEvent) {
    m_Multi = dynamic_cast<MMultiEvent*>(Event);
  } else {
    cout<<"Unhandled event type: "<<Event->GetTypeString()<<endl;
    return false;
  }

  return true;
}


// MProjection: the end...
////////////////////////////////////////////////////////////////////////////////
