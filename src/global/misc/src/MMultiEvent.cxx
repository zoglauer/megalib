/*
 * MMultiEvent.cxx
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
// MMultiEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MMultiEvent.h"

// Standard libs:
#include <cstdlib>
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MExceptions.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"
#include "MPhotoEvent.h"
#include "MMuonEvent.h"
#include "MPETEvent.h"
#include "MUnidentifiableEvent.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MMultiEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MMultiEvent::MMultiEvent()
{
  m_EventType = c_MultiEvent;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MMultiEvent::~MMultiEvent()
{
  // Intentionally empty
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a human readable string
MString MMultiEvent::ToString() const
{
  MString String;
  for (auto E: m_Events) {
    String += E->ToString();
  }

  return String;
}


////////////////////////////////////////////////////////////////////////////////


bool MMultiEvent::Assimilate(MMultiEvent* Multi)
{
  Reset();
  
  // Get only the basic data and calculate the rest:
  MPhysicalEvent::Assimilate(dynamic_cast<MPhysicalEvent*>(Multi));

  for (auto E: Multi->m_Events) {
    Add(E->Duplicate()); 
  }

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MMultiEvent::Assimilate(MPhysicalEvent* Event)
{
  // Simply Call: MMultiEvent::Assimilate(const MMultiEventData *MultiEventData)

  if (Event->GetType() == c_MultiEvent) {
    return Assimilate(dynamic_cast<MMultiEvent*>(Event));
  } else {
    merr<<"Trying to assimilate a non-Multi event!"<<endl; 
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MMultiEvent::Duplicate()
{
  MPhysicalEvent* Event = new MMultiEvent();
  Event->Assimilate(this);

  return dynamic_cast<MPhysicalEvent*>(Event);
}


////////////////////////////////////////////////////////////////////////////////


//! Get a specific event
MPhysicalEvent* MMultiEvent::GetEvent(unsigned int i)
{
  if (i < m_Events.size()) {
    return m_Events[i]; 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_Events.size(), i);
  
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool MMultiEvent::Validate()
{
  // Do some sanity checks on the event and calculate all high level data:

  m_IsGoodEvent = true;
  
  for (auto E: m_Events) {
    if (E->Validate() == false) {
      m_IsGoodEvent = false;
    }
  }  

  return m_IsGoodEvent;
}


////////////////////////////////////////////////////////////////////////////////


MString MMultiEvent::ToTraString() const
{
  //! Stream the content into a tra-file compatible string

  MString T;
  T += MPhysicalEvent::ToTraString();

  for (unsigned int i = 0; i < m_Events.size(); ++i) {
    T += "SI";
    T += m_Events[i]->ToTraString();
    T += "SF";
  }

  return T;
}


////////////////////////////////////////////////////////////////////////////////


bool MMultiEvent::Stream(MFile& File, int Version, bool Read, bool Fast, bool ReadDelayed)
{
  // Stream data from and to a file than ROOT...

  bool Return = true;
  
  if (Read == true) {
    // That's copy and paste from MPhysicalEvent:

    Reset();
    int Ret = 0;
    
    if (ReadDelayed == true) {
      MString Line;
      while (File.IsGood() == true) {
        File.ReadLine(Line);
        if (Line.Length() < 2) continue;
        if ((Line[0] == 'S' && Line[1] == 'E') || (Line[0] == 'E' && Line[1] == 'N') || (Line[0] == 'N' && Line[1] == 'F')) {
          return true;
        }
        m_Lines.push_back(Line);
      }
    } else {
      // Doing it purely in C is faster than using strings
      const int LineLength = 1000;
      char LineBuffer[LineLength];
      while (File.ReadLine(LineBuffer, LineLength, '\n')) {
        // First do the normal parse line - nothing is happening if we have anything specific for this class
        
        //cout<<LineBuffer<<endl;
        
        Ret = 2;
        if (LineBuffer[0] != 'E' && LineBuffer[1] != 'T') {
          Ret = ParseLine(LineBuffer, Fast);
        }
        //cout<<"Ret: "<<Ret<<endl;
        if (Ret == 2) {
          // Nothing got parsed, thus look what we can do here:
          MString Line(LineBuffer);
          
          MPhysicalEvent* Phys = nullptr;
          if (Line[0] == 'E' && Line[1] == 'T') {
            if (Line.Length() < 5) {
              cout<<"Error reading event type"<<endl;
            } else if (Line[3] == 'C' && Line[4] == 'O') {
              MComptonEvent* P = new MComptonEvent();
              P->Stream(File, Version, Read, Fast, false);
              Phys = (MPhysicalEvent*) P;
            } else  if (Line[3] == 'P' && Line[4] == 'A') {
              MPairEvent* P = new MPairEvent();
              P->Stream(File, Version, Read, Fast, false);
              Phys = (MPhysicalEvent*) P;
            } else  if (Line[3] == 'P' && Line[4] == 'H') {
              MPhotoEvent* P = new MPhotoEvent();
              P->Stream(File, Version, Read, Fast, false);
              Phys = (MPhysicalEvent*) P;
            } else  if (Line[3] == 'M' && Line[4] == 'U') {
              MMuonEvent* P = new MMuonEvent();
              P->Stream(File, Version, Read, Fast, false);
              Phys = (MPhysicalEvent*) P;
            } else  if (Line[3] == 'P' && Line[4] == 'T') {
              MPETEvent* P = new MPETEvent();
              P->Stream(File, Version, Read, Fast, false);
              Phys = (MPhysicalEvent*) P;
            } else  if (Line[3] == 'M' && Line[4] == 'T') {
              MMultiEvent* P = new MMultiEvent();
              P->Stream(File, Version, Read, Fast, false);
              Phys = (MPhysicalEvent*) P;
            } else  if (Line[3] == 'U' && Line[4] == 'N') {
              MUnidentifiableEvent* P = new MUnidentifiableEvent();
              P->Stream(File, Version, Read, Fast, false);
              Phys = (MPhysicalEvent*) P;
            } else {
              cout<<"Unknown event"<<endl;
            }
          }
          if (Phys != nullptr) {
            m_Events.push_back(Phys); 
          }
        } else if (Ret >= 0) {
          continue;
        } else if (Ret == -1) {
          // end of event reached
          Validate();
          //cout<<"Good event"<<endl;
          return true;
        }
      }
      Validate();
      
      // end of file reached ... so return false
      return false;
    }     

  } else {
    // Write Multi specific infos:

    File.Write(ToTraString());
    File.Flush();
  }

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MMultiEvent::ParseDelayed(bool Fast)
{
  for (unsigned int l = 0; l < m_Lines.size(); ++l) {
    //cout<<m_Lines[l]<<endl;
    if (m_Lines[l].BeginsWith("SI") == true) {
      while (++l < m_Lines.size()) {
        MPhysicalEvent* Phys = nullptr;
        if (m_Lines[l][0] == 'E' && m_Lines[l][1] == 'T') {
          if (m_Lines[l].Length() < 5) {
            cout<<"Error reading event type"<<endl;
          } else if (m_Lines[l][3] == 'C' && m_Lines[l][4] == 'O') {
            MComptonEvent* P = new MComptonEvent();
            Phys = (MPhysicalEvent*) P;
          } else  if (m_Lines[l][3] == 'P' && m_Lines[l][4] == 'A') {
            MPairEvent* P = new MPairEvent();
            Phys = (MPhysicalEvent*) P;
          } else  if (m_Lines[l][3] == 'P' && m_Lines[l][4] == 'H') {
            MPhotoEvent* P = new MPhotoEvent();
            Phys = (MPhysicalEvent*) P;
          } else  if (m_Lines[l][3] == 'M' && m_Lines[l][4] == 'U') {
            MMuonEvent* P = new MMuonEvent();
            Phys = (MPhysicalEvent*) P;
          } else  if (m_Lines[l][3] == 'P' && m_Lines[l][4] == 'T') {
            MPETEvent* P = new MPETEvent();
            Phys = (MPhysicalEvent*) P;
          } else  if (m_Lines[l][3] == 'M' && m_Lines[l][4] == 'T') {
            MMultiEvent* P = new MMultiEvent();
            Phys = (MPhysicalEvent*) P;
          } else  if (m_Lines[l][3] == 'U' && m_Lines[l][4] == 'N') {
            MUnidentifiableEvent* P = new MUnidentifiableEvent();
            Phys = (MPhysicalEvent*) P;
          } else {
            cout<<"Unknown event"<<endl;
          }
        }
        if (Phys != nullptr) {
          m_Events.push_back(Phys);
          break;
        }
      }
      while (++l < m_Lines.size()) {
        m_Events.back()->ParseLine(m_Lines[l], Fast);
        if (m_Lines[l].BeginsWith("SF") == true) {
          break;
        }
      }
    } else {
      ParseLine(m_Lines[l], Fast);
    }
  }
  
  Validate();
  
  //cout<<ToString()<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MMultiEvent::ParseLine(const char* Line, bool Fast)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached
  
  return MPhysicalEvent::ParseLine(Line, Fast);
}


////////////////////////////////////////////////////////////////////////////////


void MMultiEvent::Reset() 
{
  // Reset all values to zero:
  
  MPhysicalEvent::Reset();
  
  for (auto E: m_Events) {
    delete E; 
  }
  m_Events.clear();
  
  m_IsGoodEvent = false;

  return;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MMultiEvent::Data() 
{
  // Return a pointer to this event
 
  return dynamic_cast<MPhysicalEvent*>(this); 
} 


// MMultiEvent: the end...
////////////////////////////////////////////////////////////////////////////////
