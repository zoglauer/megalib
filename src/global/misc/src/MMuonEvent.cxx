/*
 * MMuonEvent.cxx
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
// MMuonEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MMuonEvent.h"

// Standard libs:
#include <cstdlib>
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MMuonEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MMuonEvent::MMuonEvent()
{
  // standard constructor

  m_EventType = c_Muon;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MMuonEvent::~MMuonEvent()
{
  //standard destructor
}


////////////////////////////////////////////////////////////////////////////////


MString MMuonEvent::ToString() const
{
  // Display the compton-data of this event

  const int Length = 1000;
  char Text[Length];
  MString String("The data of the Muon-event:\n"); 

  snprintf(Text, Length, "Energy: %.3f\n", m_Energy);
  String += MString(Text);

  snprintf(Text, Length, "Direction: %.3f, %.3f, %.3f\n",
          m_Direction.X(), m_Direction.Y(), 
          m_Direction.Z());
  String += MString(Text);

  snprintf(Text, Length, "CenterOfGravity: %.3f, %.3f, %.3f\n",
          m_CenterOfGravity.X(), m_CenterOfGravity.Y(), 
          m_CenterOfGravity.Z());
  String += MString(Text);

  return String;
}


////////////////////////////////////////////////////////////////////////////////


bool MMuonEvent::Assimilate(MMuonEvent* Muon)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray...
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton scattering, we return false.
  
  Reset();
  
  // Get only the basic data and calculate the rest:
  MPhysicalEvent::Assimilate((MPhysicalEvent *) Muon);

  m_Energy = Muon->GetEnergy();       
  m_Direction = Muon->GetDirection();
  m_CenterOfGravity = Muon->GetCenterOfGravity();
  
  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MMuonEvent::Assimilate(MPhysicalEvent* Event)
{
  // Simply Call: MMuonEvent::Assimilate(const MMuonEventData *MuonEventData)

  if (Event->GetType() == MPhysicalEvent::c_Muon) {
    return Assimilate((MMuonEvent *) Event);
  } else {
    merr<<"Trying to assimilate a non muon event!"<<endl; 
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MMuonEvent::Assimilate(MVector Direction, MVector CenterOfGravity, double Energy)
{
  //

  Reset();
  
  // Get only the basic data and calculate the rest:
  m_Energy = Energy;       
  m_Direction = Direction;     
  m_CenterOfGravity = CenterOfGravity;     
  
  return Validate();
}



////////////////////////////////////////////////////////////////////////////////


bool MMuonEvent::Assimilate(char *LineBuffer)
{
  // Assimilate an event from a text-line

  Reset();

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MMuonEvent::Validate()
{
  // Do some sanity checks on the event and calculate all high level data:

  if (m_Energy < 0) return false;

  m_IsGoodEvent = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MMuonEvent::Duplicate()
{
  // Duplicate this event

  MPhysicalEvent* Event = new MMuonEvent();
  Event->Assimilate(this);

  return dynamic_cast<MPhysicalEvent*>(Event);
}


////////////////////////////////////////////////////////////////////////////////


void MMuonEvent::Reset() 
{
  // Reset all values to zero:
  
  MPhysicalEvent::Reset();
  
  m_Energy = 0.0;       
  m_Direction = MVector(0.0, 0.0, 0.0);     
  m_CenterOfGravity = MVector(0.0, 0.0, 0.0);     

  return;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MMuonEvent::Data() 
{
  // Return a pointer to this event
 
  return (MPhysicalEvent *) this; 
} 


////////////////////////////////////////////////////////////////////////////////


bool MMuonEvent::Stream(MFile& File, int Version, bool Read, bool Fast, bool ReadDelayed)
{
  // Hopefully a faster way to stream data from and to a file than ROOT...

  bool Return = MPhysicalEvent::Stream(File, Version, Read, Fast, ReadDelayed);

  if (Read == false) {
    // Write Muon specific infos:
    ostringstream S;
    S<<"ME "<<m_Energy<<endl;
    S<<"MD "<<m_Direction[0]<<" "<<m_Direction[1]<<" "<<m_Direction[2]<<endl;
    S<<"MG "<<m_CenterOfGravity[0]<<" "<<m_CenterOfGravity[1]<<" "<<m_CenterOfGravity[2]<<endl;
    File.Write(S);
    File.Flush();
  }

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


int MMuonEvent::ParseLine(const char* Line, bool Fast)
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

  if (Line[0] == 'M' && Line[1] == 'E') {
    if (Fast == true) {
      m_Energy = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "ME %lf", &m_Energy) != 1) {
        cout<<"Unable to parse ME of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'M' && Line[1] == 'D') {
    if (Fast == true) {
      char* p;
      m_Direction[0] = strtod(Line+3, &p);
      m_Direction[1] = strtod(p, &p);
      m_Direction[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "MD %lf %lf %lf", 
                 &m_Direction[0], &m_Direction[1], &m_Direction[2]) != 3) {
        cout<<"Unable to parse MD of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'M' && Line[1] == 'G') {
    if (Fast == true) {
      char* p;
      m_CenterOfGravity[0] = strtod(Line+3, &p);
      m_CenterOfGravity[1] = strtod(p, &p);
      m_CenterOfGravity[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "MG %lf %lf %lf", 
                 &m_CenterOfGravity[0], &m_CenterOfGravity[1], &m_CenterOfGravity[2]) != 3) {
        cout<<"Unable to parse MG of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else {
    Ret = 2;
  }

  return Ret;
}




// MMuonEvent: the end...
////////////////////////////////////////////////////////////////////////////////
