/*
 * MPhysicalEvent.cxx
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
// MPhysicalEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPhysicalEvent.h"

// Standard libs:
#include <cstdlib>
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MPhysicalEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MPhysicalEvent::c_Unknown        = -1;
const int MPhysicalEvent::c_Compton        =  0;
const int MPhysicalEvent::c_Pair           =  1;
const int MPhysicalEvent::c_Muon           =  2;
const int MPhysicalEvent::c_Shower         =  3;
const int MPhysicalEvent::c_Photo          =  4;
const int MPhysicalEvent::c_Decay          =  5;
const int MPhysicalEvent::c_PET            =  6;
const int MPhysicalEvent::c_Multi          =  7;
const int MPhysicalEvent::c_Unidentifiable =  100;


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent::MPhysicalEvent() : MRotationInterface(), m_Time(0)
{
  // default constructor

  m_EventType = c_Unknown;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent::~MPhysicalEvent()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


MString MPhysicalEvent::GetTypeString() const
{
  // Return the event type:
  
  MString String;
  if (m_EventType == c_Compton) {
    String = "Compton";
  } else if  (m_EventType == c_Pair) {
    String = "Pair";
  } else if  (m_EventType == c_Muon) {
    String = "Muon";
  } else if  (m_EventType == c_Shower) {
    String = "Shower";
  } else if  (m_EventType == c_Photo) {
    String = "Photo";
  } else if  (m_EventType == c_Decay) {
    String = "Decay";
  } else if  (m_EventType == c_PET) {
    String = "PET";
  } else if  (m_EventType == c_Multi) {
    String = "Multi";
  } else if  (m_EventType == c_Unidentifiable) {
    String = "Unidentifiable";
  } else {
    String = "Unknown"; 
  }
  
  return String;
}


////////////////////////////////////////////////////////////////////////////////


bool MPhysicalEvent::Assimilate(MPhysicalEvent* E)
{
  //

  Set(*dynamic_cast<MRotationInterface*>(E));
  
  m_EventType = E->m_EventType;
  m_TimeWalk = E->m_TimeWalk;
  m_Time = E->m_Time;
  m_Id = E->m_Id;
  m_AllHitsGood = E->m_AllHitsGood;
  m_Decay = E->m_Decay;
  m_Bad = E->m_Bad;
  m_BadString = E->m_BadString;
  m_Comments = E->m_Comments;

  m_OIPosition = E->m_OIPosition;
  m_OIDirection = E->m_OIDirection;
  m_OIPolarization = E->m_OIPolarization;
  m_OIEnergy = E->m_OIEnergy;  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MPhysicalEvent::Duplicate()
{
  // Duplicate this event

  MPhysicalEvent* Event = new MPhysicalEvent();
  Event->Assimilate(this);

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEvent::Reset()
{
  MRotationInterface::Reset();

  m_IsGoodEvent = false;
  m_AllHitsGood = true;

  m_Time = 10000.0;
  m_Id = 0;
  m_TimeWalk = -1;
  m_Decay = false;
  m_Bad = false;
  m_BadString = "";

  m_OIPosition = g_VectorNotDefined;
  m_OIDirection = g_VectorNotDefined;
  m_OIPolarization = g_VectorNotDefined;
  m_OIEnergy = g_DoubleNotDefined;

  m_Lines.clear();
  m_Comments.clear();
}


////////////////////////////////////////////////////////////////////////////////


MString MPhysicalEvent::ToString() const
{
  //

  return "I am a physical event!";
}


////////////////////////////////////////////////////////////////////////////////


double MPhysicalEvent::GetEnergy() const 
{
  // Return the total (measured) energy of this event

  return g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MVector MPhysicalEvent::GetPosition() const 
{
  // Return the position of the event = the first detected interaction

  return g_VectorNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MVector MPhysicalEvent::GetOrigin() const
{
  // Get the origin direction of the event - if it has none return g_VectorNotDefined
  // In detector coordinates - this is the reverse travel direction!

  return g_VectorNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MString MPhysicalEvent::GetComment(unsigned int i) const
{
  //! Get the specific comment
  
  if (i < m_Comments.size()) {
    return m_Comments[i]; 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_Comments.size(), i);
  
  return "";
}


////////////////////////////////////////////////////////////////////////////////


const MPhysicalEventHit& MPhysicalEvent::GetHit(unsigned int i) const
{
  //! Get the specific comment
  
  if (i < m_Hits.size()) {
    return m_Hits[i]; 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_Hits.size(), i);
  
  // We never reach here, thus, this should not be a problem, or just throw another exception
  return m_Hits.front();
}


////////////////////////////////////////////////////////////////////////////////


MString MPhysicalEvent::ToTraString() const
{
  //! Stream the content into a tra-file compatible string

  ostringstream S;
  switch (m_EventType) {
  case c_Compton:
    S<<"ET CO"<<endl;
    break;
  case c_Pair:
    S<<"ET PA"<<endl;
    break;
  case c_Photo:
    S<<"ET PH"<<endl;
    break;
  case c_Muon:
    S<<"ET MU"<<endl;
    break;
  case c_Decay:
    S<<"ET DY"<<endl;
    break;
  case c_PET:
    S<<"ET PT"<<endl;
    break;
  case c_Multi:
    S<<"ET MT"<<endl;
    break;
  case c_Unidentifiable:
    S<<"ET UN"<<endl;
    break;
  default:
    massert(false);
    S<<"ET Unkown"<<endl;
    break;
  }
  S<<"ID "<<m_Id<<endl;
  S<<"TI "<<m_Time.GetLongIntsString()<<endl;
  if (m_TimeWalk != -1) {
    S<<"TW "<<m_TimeWalk<<endl;
  }

  MRotationInterface::Stream(S);

  if (m_Bad == true) {
    S<<"BD "<<m_BadString<<endl;
  }
  if (m_Decay == true) {
    S<<"DC"<<endl;
  }
  if (m_OIPosition != g_VectorNotDefined && m_OIDirection != g_VectorNotDefined && m_OIPolarization != g_VectorNotDefined) {
    S<<"OI "<<m_OIPosition.X()<<" "<<m_OIPosition.Y()<<" "<<m_OIPosition.Z()<<" "<<m_OIDirection.X()<<" "<<m_OIDirection.Y()<<" "<<m_OIDirection.Z()<<" "<<m_OIPolarization.X()<<" "<<m_OIPolarization.Y()<<" "<<m_OIPolarization.Z()<<" "<<m_OIEnergy<<endl;
  }
  for (unsigned int c = 0; c < m_Comments.size(); ++c) {
    S<<"CC "<<m_Comments[c]<<endl;
  }

  return S.str();
}

  
////////////////////////////////////////////////////////////////////////////////


bool MPhysicalEvent::Stream(MFile& File, int Version, bool Read, bool Fast, bool DelayedRead)
{
  // Hopefully a faster way to stream data from and to a file than ROOT...

  if (Read == false) {
    File.Write(ToTraString());
    File.Flush();
  } else {
    // Read each line until we reach the end of the file or a new SE...
    Reset();
    int Ret;


    if (DelayedRead == true) {
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
      // Doing it purely in C is faster than using string
      const int LineLength = 1000;
      char LineBuffer[LineLength];
      while (File.ReadLine(LineBuffer, LineLength, '\n')) {
        Ret = ParseLine(LineBuffer, Fast);
        if (Ret >= 0) {
          continue;
        } else if (Ret == -1) {
          // end of event reached
          Validate();
          return true;
        }
      }
      Validate();
      
      // end of file reached ... so return false
    } 
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MPhysicalEvent::ParseDelayed(bool Fast)
{
  for (unsigned int l = 0; l < m_Lines.size(); ++l) {
    ParseLine(m_Lines[l], Fast);
  }

  Validate();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MPhysicalEvent::ParseLine(const char* Line, bool Fast)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached
 
  int Ret = 0;
  
  if (Line[0] == 'E' && Line[1] == 'T') {
    if (Line[3] == 'C' && Line[4] == 'O') {
      if (m_EventType != c_Compton) {
        cout<<"int MPhysicalEvent::ParseLine: Event is no Compton event as suggested but a "<<GetTypeString()<<"!"<<endl;
        Ret = 1;
      }
    } else if (Line[3] == 'P' && Line[4] == 'A') {
      if (m_EventType != c_Pair) {
        cout<<"int MPhysicalEvent::ParseLine: Event is no Pair event as suggested but a "<<GetTypeString()<<"!"<<endl;
        Ret = 1;
      }
    } else if (Line[3] == 'P' && Line[4] == 'H') {
      if (m_EventType != c_Photo) {
        cout<<"int MPhysicalEvent::ParseLine: Event is no Photo event as suggested but a "<<GetTypeString()<<"!"<<endl;
        Ret = 1;
      }
    } else if (Line[3] == 'M' && Line[4] == 'U') {
      if (m_EventType != c_Muon) {
        cout<<"int MPhysicalEvent::ParseLine: Event is no Muon event as suggested but a "<<GetTypeString()<<"!"<<endl;
        Ret = 1;
      }
    } else if (Line[3] == 'D' && Line[4] == 'Y') {
      if (m_EventType != c_Decay) {
        cout<<"int MPhysicalEvent::ParseLine: Event is no Decay event as suggested but a "<<GetTypeString()<<"!"<<endl;
        Ret = 1;
      }
    } else if (Line[3] == 'P' && Line[4] == 'T') {
      if (m_EventType != c_PET) {
        cout<<"int MPhysicalEvent::ParseLine: Event is no PET event as suggested but a "<<GetTypeString()<<"!"<<endl;
        Ret = 1;
      }
    } else if (Line[3] == 'M' && Line[4] == 'T') {
      if (m_EventType != c_Multi) {
        cout<<"int MPhysicalEvent::ParseLine: Event is no Multi event as suggested but a "<<GetTypeString()<<"!"<<endl;
        Ret = 1;
      }
    } else if (Line[3] == 'U' && Line[4] == 'N') {
      if (m_EventType != c_Unidentifiable) {
        cout<<"int MPhysicalEvent::ParseLine: Event is no Unidentifiable event as suggested but a "<<GetTypeString()<<"!"<<endl;
        Ret = 1;
      }
    } else {
      cout<<"int MPhysicalEvent::ParseLine: Unkown event type..."<<endl;
      Ret = 1;
    }
  } else if (Line[0] == 'T' && Line[1] == 'I') {
    if (Fast == true) {
      if (m_Time.Set(Line) == false) {
        Ret = 1;
      }
    } else {
      if (m_Time.Set(MString(Line)) == false) {
        Ret = 1;
      }
    }
  } else if (Line[0] == 'I' && Line[1] == 'D') {
    if (Fast == true) {
      m_Id = strtol(Line+3, NULL, 10);
    } else {
      if (sscanf(Line, "ID %li", &m_Id) != 1) {
        Ret = 1;
      }
    }
    MRotationInterface::m_Id = m_Id;
  } else if (Line[0] == 'T' && Line[1] == 'W') {
    if (Fast == true) {
      m_TimeWalk = strtol(Line+3, NULL, 10);
    } else {
      if (sscanf(Line, "TW %d", &m_TimeWalk) != 1) {
        Ret = 1;
      }
    }
  } else if (Line[0] == 'R' && Line[1] == 'X') {
    MRotationInterface::ParseLine(Line, Fast);
  } else if (Line[0] == 'R' && Line[1] == 'Z') {
    MRotationInterface::ParseLine(Line, Fast);
  } else if (Line[0] == 'G' && Line[1] == 'X') {
    MRotationInterface::ParseLine(Line, Fast);
  } else if (Line[0] == 'G' && Line[1] == 'Z') {
    MRotationInterface::ParseLine(Line, Fast);
  } else if (Line[0] == 'H' && Line[1] == 'X') {
    MRotationInterface::ParseLine(Line, Fast);
  } else if (Line[0] == 'H' && Line[1] == 'Z') {
    MRotationInterface::ParseLine(Line, Fast);
  } else if (Line[0] == 'B' && Line[1] == 'D') {
    m_BadString = Line;
    m_BadString = m_BadString.Remove(0, 3);
    m_BadString = m_BadString.ReplaceAll("\n", "");

    /*
    m_BadString.erase(0, 3);
    while (true) {
      const int pos = m_BadString.find("\n");
      if (pos==-1) break;
      m_BadString.replace(pos, 1, "");
    }
    */

    m_Bad = true;
  } else if (Line[0] == 'C' && Line[1] == 'C') {
    MString Comment = Line;
    Comment = Comment.Remove(0, 3);
    Comment = Comment.ReplaceAll("\n", "");
    m_Comments.push_back(Comment);

    /*
    m_BadString.erase(0, 3);
    while (true) {
      const int pos = m_BadString.find("\n");
      if (pos==-1) break;
      m_BadString.replace(pos, 1, "");
    }
    */

  } else if (Line[0] == 'O' && Line[1] == 'I') {
    if (Fast == true) {
      char* p;
      m_OIPosition[0] = strtod(Line+3, &p);
      m_OIPosition[1] = strtod(p, &p);
      m_OIPosition[2] = strtod(p, &p);
      m_OIDirection[0] = strtod(p, &p);
      m_OIDirection[1] = strtod(p, &p);
      m_OIDirection[2] = strtod(p, &p);
      m_OIPolarization[0] = strtod(p, &p);
      m_OIPolarization[1] = strtod(p, &p);
      m_OIPolarization[2] = strtod(p, &p);
      m_OIEnergy = strtod(p, NULL);
    } else {
      if (sscanf(Line, "OI %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &m_OIPosition[0], &m_OIPosition[1], &m_OIPosition[2], &m_OIDirection[0], &m_OIDirection[1], &m_OIDirection[2], &m_OIPolarization[0], &m_OIPolarization[1], &m_OIPolarization[2], &m_OIEnergy) != 10) {
        Ret = 1;
      }
    }
  } else if (Line[0] == 'D' && Line[1] == 'C') {
    m_Decay = true;
  } else if (Line[0] == 'S' && Line[1] == 'E') {
    Ret = -1;
  } else if (Line[0] == 'S' && Line[1] == 'F') {
    Ret = -1;
  } else if (Line[0] == 'E' && Line[1] == 'N') {
    Ret = -1;
  } else if (Line[0] == 'N' && Line[1] == 'F') {
    Ret = -1;
  } else {
    Ret = 2;
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


bool MPhysicalEvent::Validate() 
{ 
  //! Validate the event and calculate all high level data...
  
  MRotationInterface::Validate();
  
  return false; 
}


// MPhysicalEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
