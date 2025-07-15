/*
 * MDecayEvent.cxx
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
// MDecayEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDecayEvent.h"

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
ClassImp(MDecayEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MDecayEvent::MDecayEvent()
{
  // standard constructor

  m_EventType = c_DecayEvent;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MDecayEvent::~MDecayEvent()
{
  //standard destructor
}


////////////////////////////////////////////////////////////////////////////////


MString MDecayEvent::ToString()
{
  // Display the compton-data of this event

  //char Text[1000];
  MString String("The data of the Decay-event:\n"); 

//   for (unsigned int i = 0; i < m_Energies.size(); ++i) {
//     sprintf(Text, "Hit %i: Energy: %.3f", i, m_Energies[i]);
//     String += MString(Text);
//     sprintf(Text, "Position: %.3f, %.3f, %.3f\n", 
//             m_Positions[i].X(), m_Positions[i].Y(), 
//             m_Positions[i].Z());
//     String += MString(Text);
//   }

//   sprintf(Text, "Detector pointing: x(%.3f, %.3f, %.3f) z(%.3f, %.3f, %.3f) \n", 
//           m_RotationXAxis.X(), m_RotationXAxis.Y(), m_RotationXAxis.Z(),
//           m_RotationZAxis.X(), m_RotationZAxis.Y(), m_RotationZAxis.Z());
//   String += MString(Text);

  return String;
}


////////////////////////////////////////////////////////////////////////////////


bool MDecayEvent::Assimilate(MDecayEvent* Decay)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray...
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton scattering, we return false.
  
  Reset();
  
  // Get only the basic data and calculate the rest:
  MPhysicalEvent::Assimilate((MPhysicalEvent *) Decay);

//   m_Energies = Decay->m_Energies;       
//   m_Positions = Decay->m_Positions;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDecayEvent::Assimilate(MPhysicalEvent* Event)
{
  // Simply Call: MDecayEvent::Assimilate(const MDecayEventData *DecayEventData)

  if (Event->GetType() == c_DecayEvent) {
    return Assimilate((MDecayEvent *) Event);
  } else {
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MDecayEvent::Duplicate()
{
  // Duplicate this event

  MPhysicalEvent* Event = new MDecayEvent();
  Event->Assimilate(this);

  return dynamic_cast<MPhysicalEvent*>(Event);
}


////////////////////////////////////////////////////////////////////////////////


void MDecayEvent::AddPhysicalEvent(const MPhysicalEvent& Event)
{
  // Add abother physical event (most likely a Compton) to this event

  m_PhysicalEvents.push_back[Event];
}


////////////////////////////////////////////////////////////////////////////////


const MPhysicalEvent& MDecayEvent::GetPhysicalEvent(unsigned int i) const
{
  // Return the physical event at position i

  massert(i < GetNPhysicalEvents());

  return m_PhysicalEvents[i];
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDecayEvent::GetNPhysicalEvents() const
{
  // Return the number of stored events

  return m_PhysicalEvents.size();
}


////////////////////////////////////////////////////////////////////////////////


MString MDecayEvent::ToTraString() const
{
  //! Stream the content into a tra-file compatible string

  MString T;
  T += MPhysicalEvent::ToTraString();

  ostringstream S;
  S<<"DB "<<m_BetaEnergy<<" "<<m_BetaPosition[0]<<" "<<m_BetaPosition[1]<<" "<<m_BetaPosition[2]<<endl;
  for (unsigned int i = 0; i < GetNPhysicalEvents(); ++i) {
    S<<"DA "<<endl;
    //TODO: This seems to be unfinshed...
  }

  T += S.str();

  return T;
}


////////////////////////////////////////////////////////////////////////////////


int MDecayEvent::ParseLine(const char* Line, bool Fast)
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

  double Energy;
  double x, y, z;

  if (Line[0] == 'H' && Line[1] == 'T') {
//     if (sscanf(Line, "HT %lf %lf %lf %lf", 
//                &Energy, &x, &y, &z) != 4) {
//       mout<<"Unable to parse HT!"<<endl;
//       Ret = 1;
//     } else {
//       SetHit(Energy, MVector(x, y, z));
//     }
  } else {
    Ret = 2;
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


void MDecayEvent::Reset() 
{
  // Reset all values to zero:
  
  MPhysicalEvent::Reset();
  
  m_BetaEnergy = 0.0;
  m_BetaPosition = MVector(0, 0, 0);
  m_PhysicalEvents.clear();

  return;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MDecayEvent::Data() 
{
  // Return a pointer to this event
 
  return (MPhysicalEvent *) this; 
} 


////////////////////////////////////////////////////////////////////////////////



// MDecayEvent: the end...
////////////////////////////////////////////////////////////////////////////////
