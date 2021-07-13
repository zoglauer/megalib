/*
 * MPhotoEvent.cxx
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
// MPhotoEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPhotoEvent.h"

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
ClassImp(MPhotoEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MPhotoEvent::MPhotoEvent()
{
  // standard constructor

  m_EventType = c_Photo;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MPhotoEvent::~MPhotoEvent()
{
  //standard destructor
}


////////////////////////////////////////////////////////////////////////////////


MString MPhotoEvent::ToString() const
{
  // Display the compton-data of this event

  char Text[1000];
  MString String("The data of the Photo-event:\n"); 

  sprintf(Text, "Energy: %.3f\n", m_Energy);
  String += MString(Text);

  sprintf(Text, "Position: %.3f, %.3f, %.3f\n", 
          m_Position.X(), m_Position.Y(), 
          m_Position.Z());
  String += MString(Text);

  sprintf(Text, "Weight: %.3f\n", m_Weight);
  String += MString(Text);

  return String;
}


////////////////////////////////////////////////////////////////////////////////


bool MPhotoEvent::Assimilate(MPhotoEvent* Photo)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray...
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton scattering, we return false.
  
  Reset();
  
  // Get only the basic data and calculate the rest:
  MPhysicalEvent::Assimilate(dynamic_cast<MPhysicalEvent*>(Photo));

  m_Energy = Photo->GetEnergy();       
  m_Position = Photo->GetPosition();
  m_Weight = Photo->GetWeight();       

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MPhotoEvent::Assimilate(MPhysicalEvent* Event)
{
  // Simply Call: MPhotoEvent::Assimilate(const MPhotoEventData *PhotoEventData)

  if (Event->GetType() == MPhysicalEvent::c_Photo) {
    return Assimilate(dynamic_cast<MPhotoEvent*>(Event));
  } else {
    merr<<"Trying to assimilate a non photo event!"<<endl; 
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MPhotoEvent::Assimilate(MVector Position, double Energy, double Weight)
{
  //

  Reset();
  
  // Get only the basic data and calculate the rest:
  m_Energy = Energy;       
  m_Position = Position;     
  m_Weight = Weight;       
  
  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MPhotoEvent::Duplicate()
{
  // Duplicate this event

  MPhysicalEvent* Event = new MPhotoEvent();
  Event->Assimilate(this);

  return dynamic_cast<MPhysicalEvent*>(Event);
}


////////////////////////////////////////////////////////////////////////////////


bool MPhotoEvent::Validate()
{
  // Do some sanity checks on the event and calculate all high level data:

  if (m_Energy < 0) return false;

  m_IsGoodEvent = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MPhotoEvent::Assimilate(char *LineBuffer)
{
  // Assimilate an event from a text-line

  Reset();

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MPhotoEvent::Stream(MFile& File, int Version, bool Read, bool Fast, bool ReadDelayed)
{
  // Hopefully a faster way to stream data from and to a file than ROOT...

  bool Return = MPhysicalEvent::Stream(File, Version, Read, Fast, ReadDelayed);

  if (Read == false) {
    // Write Photo specific infos:
    ostringstream S;
    S<<"PE "<<m_Energy<<endl;
    S<<"PP "<<m_Position[0]<<" "<<m_Position[1]<<" "<<m_Position[2]<<endl;
    S<<"PW "<<m_Weight<<endl;
    File.Write(S);
    File.Flush();
  }

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


int MPhotoEvent::ParseLine(const char* Line, bool Fast)
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
        cout<<"Unable to parse PE of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'P' && Line[1] == 'P') {
    if (Fast == true) {
      char* p;
      m_Position[0] = strtod(Line+3, &p);
      m_Position[1] = strtod(p, &p);
      m_Position[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "PP %lf %lf %lf", 
                 &m_Position[0], &m_Position[1], &m_Position[2]) != 3) {
        cout<<"Unable to parse PP of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'P' && Line[1] == 'W') {
    if (Fast == true) {
      m_Weight = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "PW %lf", &m_Weight) != 1) {
        cout<<"Unable to parse PW of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else {
    Ret = 2;
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


void MPhotoEvent::Reset() 
{
  // Reset all values to zero:
  
  MPhysicalEvent::Reset();
  
  m_Energy = 0.0;       
  m_Position = MVector(0.0, 0.0, 0.0);     
  m_Weight = 0.0;       
  m_IsGoodEvent = false;

  return;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MPhotoEvent::Data() 
{
  // Return a pointer to this event
 
  return dynamic_cast<MPhysicalEvent*>(this); 
} 


////////////////////////////////////////////////////////////////////////////////



// MPhotoEvent: the end...
////////////////////////////////////////////////////////////////////////////////
