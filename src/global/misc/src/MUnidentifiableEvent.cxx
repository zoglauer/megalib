/*
 * MUnidentifiableEvent.cxx
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
// MUnidentifiableEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MUnidentifiableEvent.h"

// Standard libs:
#include <cstdlib>
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MUnidentifiableEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MUnidentifiableEvent::MUnidentifiableEvent()
{
  // standard constructor

  m_EventType = c_UnidentifiableEvent;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MUnidentifiableEvent::~MUnidentifiableEvent()
{
  //standard destructor
}


////////////////////////////////////////////////////////////////////////////////


MString MUnidentifiableEvent::ToString() const
{
  // Display the compton-data of this event

  const int Length = 1000;
  char Text[Length];
  MString String("The data of the Unidentifiable-event:\n"); 

  snprintf(Text, Length, "Energy: %.3f", m_Energy);
  String += MString(Text);

  return String;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnidentifiableEvent::Assimilate(MUnidentifiableEvent* Unidentifiable)
{
  // Take over all the necessary event data and perform some elementary computations
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton scattering, we return false.
  
  Reset();
  
  // Get only the basic data and calculate the rest:
  MPhysicalEvent::Assimilate((MPhysicalEvent *) Unidentifiable);

  m_Energy = Unidentifiable->m_Energy;       
  
  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MUnidentifiableEvent::Assimilate(MPhysicalEvent* Event)
{
  // Simply Call: MUnidentifiableEvent::Assimilate(const MUnidentifiableEventData *UnidentifiableEventData)

  if (Event->GetType() == c_UnidentifiableEvent) {
    return Assimilate((MUnidentifiableEvent *) Event);
  } else {
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MUnidentifiableEvent::Duplicate()
{
  // Duplicate this event

  MPhysicalEvent* Event = new MUnidentifiableEvent();
  Event->Assimilate(this);

  return dynamic_cast<MPhysicalEvent*>(Event);
}


////////////////////////////////////////////////////////////////////////////////


bool MUnidentifiableEvent::Validate()
{
  // Do some sanity checks on the event and calculate all high level data:

  m_IsGoodEvent = false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MUnidentifiableEvent::ToTraString() const
{
  //! Stream the content into a tra-file compatible string

  MString T;
  T += MPhysicalEvent::ToTraString();

  ostringstream S;
  S<<"PE "<<m_Energy<<endl;

  T += S.str();

  return T;
}


////////////////////////////////////////////////////////////////////////////////


int MUnidentifiableEvent::ParseLine(const char* Line, bool Fast)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached

  int Ret = MPhysicalEvent::ParseLine(Line, Fast);
  if (Ret != 2) {
    return Ret;
  }

  Ret = 0;

  if (Line[0] == 'P' && Line[1] == 'E') {
    if (Fast == true) {
      m_Energy = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "PE %lf", &m_Energy) != 1) {
        mout<<"Unable to parse PE of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else {
    Ret = 2;
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


void MUnidentifiableEvent::Reset() 
{
  // Reset all values to zero:
  
  MPhysicalEvent::Reset();
  
  m_Energy = 0;

  return;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MUnidentifiableEvent::Data() 
{
  // Return a pointer to this event
 
  return (MPhysicalEvent *) this; 
} 


// MUnidentifiableEvent: the end...
////////////////////////////////////////////////////////////////////////////////
