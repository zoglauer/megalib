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


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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
const int MPhysicalEvent::c_Unidentifiable =  10;


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent::MPhysicalEvent() : m_Time(0)
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
  } else if  (m_EventType == c_Unidentifiable) {
    String = "Unidentifiable";
  } else {
    String = "Unknown"; 
  }
  
  return String;
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEvent::SetGalacticPointingXAxis(const double Longitude, const double Latitude)
{
  // Set the X axis of the LEFT-handed galactic coordinate system:
  // Left handedness is applied via y-axis

  m_HasGalacticPointing = true;
  m_GalacticPointingXAxis.SetMagThetaPhi(1.0, (90+Latitude)*c_Rad, Longitude*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEvent::SetGalacticPointingZAxis(const double Longitude, const double Latitude)
{
  // Set the Z axis of the LEFT-handed galactic coordinate system:
  // Left handedness is applied via y-axis

  m_HasGalacticPointing = true;
  m_GalacticPointingZAxis.SetMagThetaPhi(1.0, (90+Latitude)*c_Rad, Longitude*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEvent::SetDetectorRotationXAxis(const MVector Rot)
{
  // Set the X axis of the right-handed Cartesian coordinate system:

  m_HasDetectorRotation = true;
  m_DetectorRotationXAxis = Rot;
}


////////////////////////////////////////////////////////////////////////////////


MVector MPhysicalEvent::GetDetectorRotationXAxis() const
{
  // Get the X axis of the right-handed Cartesian coordinate system:

  return m_DetectorRotationXAxis;
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEvent::SetDetectorRotationZAxis(const MVector Rot)
{
  // Set the Z axis of the right-handed Cartesian coordinate system:

  m_HasDetectorRotation = true;
  m_DetectorRotationZAxis = Rot;
}


////////////////////////////////////////////////////////////////////////////////


MVector MPhysicalEvent::GetDetectorRotationZAxis() const
{
  // Get the Z axis of the right-handed Cartesian coordinate system:

  return m_DetectorRotationZAxis;
}


////////////////////////////////////////////////////////////////////////////////


TMatrix MPhysicalEvent::GetHorizonPointingRotationMatrix() const
{
  // Return the rotation matrix of this event

  // Verify that x and z axis are at right angle:
  if (fabs(m_HorizonPointingXAxis.Angle(m_HorizonPointingZAxis) - c_Pi/2.0)*c_Deg > 0.1) {
    cout<<"Event "<<m_Id<<": Horizon axes are not at right angle, but: "<<m_HorizonPointingXAxis.Angle(m_HorizonPointingZAxis)*c_Deg<<" deg"<<endl;
  }

  // First compute the y-Axis vector:
  MVector m_HorizonPointingYAxis = m_HorizonPointingZAxis.Cross(m_HorizonPointingXAxis);

  TMatrix M(3,3);
  M(0,0) = m_HorizonPointingXAxis.X();
  M(1,0) = m_HorizonPointingXAxis.Y();
  M(2,0) = m_HorizonPointingXAxis.Z();
  M(0,1) = m_HorizonPointingYAxis.X();
  M(1,1) = m_HorizonPointingYAxis.Y();
  M(2,1) = m_HorizonPointingYAxis.Z();
  M(0,2) = m_HorizonPointingZAxis.X();
  M(1,2) = m_HorizonPointingZAxis.Y();
  M(2,2) = m_HorizonPointingZAxis.Z();

  return M; 
}


////////////////////////////////////////////////////////////////////////////////


TMatrix MPhysicalEvent::GetDetectorRotationMatrix() const
{
  // Return the rotation matrix of this event

  // Verify that x and z axis are at right angle:
  if (fabs(m_DetectorRotationXAxis.Angle(m_DetectorRotationZAxis) - c_Pi/2.0)*c_Deg > 0.1) {
    cout<<"Event "<<m_Id<<": DetectorRotation axes are not at right angle, but: "<<m_DetectorRotationXAxis.Angle(m_DetectorRotationZAxis)*c_Deg<<" deg"<<endl;
  }


  TMatrix M(3,3);
  MVector m_DetectorRotationYAxis;

  // First compute the y-Axis vector:
  m_DetectorRotationYAxis = m_DetectorRotationZAxis.Cross(m_DetectorRotationXAxis);

  M(0,0) = m_DetectorRotationXAxis.X();
  M(1,0) = m_DetectorRotationXAxis.Y();
  M(2,0) = m_DetectorRotationXAxis.Z();
  M(0,1) = m_DetectorRotationYAxis.X();
  M(1,1) = m_DetectorRotationYAxis.Y();
  M(2,1) = m_DetectorRotationYAxis.Z();
  M(0,2) = m_DetectorRotationZAxis.X();
  M(1,2) = m_DetectorRotationZAxis.Y();
  M(2,2) = m_DetectorRotationZAxis.Z();

  return M; 
}


////////////////////////////////////////////////////////////////////////////////


TMatrix MPhysicalEvent::GetGalacticPointingRotationMatrix() const
{
  // Return the rotation matrix of this event

  // Verify that x and z axis are at right angle:
  if (fabs(m_GalacticPointingXAxis.Angle(m_GalacticPointingZAxis) - c_Pi/2.0)*c_Deg > 0.1) {
    cout<<"Event "<<m_Id<<": GalacticPointing axes are not at right angle, but: "<<m_GalacticPointingXAxis.Angle(m_GalacticPointingZAxis)*c_Deg<<" deg"<<endl;
  }


  TMatrix M(3,3);
  MVector m_GalacticPointingYAxis;

  // First compute the y-Axis vector:
  m_GalacticPointingYAxis = m_GalacticPointingZAxis.Cross(m_GalacticPointingXAxis);

  // We need a minus here since the Galactic coordinate system in left-handed!!!!
  if (m_HasGalacticPointing == true) {
    m_GalacticPointingYAxis *= -1;
  }

  M(0,0) = m_GalacticPointingXAxis.X();
  M(1,0) = m_GalacticPointingXAxis.Y();
  M(2,0) = m_GalacticPointingXAxis.Z();
  M(0,1) = m_GalacticPointingYAxis.X();
  M(1,1) = m_GalacticPointingYAxis.Y();
  M(2,1) = m_GalacticPointingYAxis.Z();
  M(0,2) = m_GalacticPointingZAxis.X();
  M(1,2) = m_GalacticPointingZAxis.Y();
  M(2,2) = m_GalacticPointingZAxis.Z();

  return M; 
}


////////////////////////////////////////////////////////////////////////////////


bool MPhysicalEvent::Assimilate(MPhysicalEvent* E)
{
  //

  if (E->HasGalacticPointing() == true) { 
    m_GalacticPointingXAxis = E->m_GalacticPointingXAxis;
    m_GalacticPointingZAxis = E->m_GalacticPointingZAxis;
    m_HasGalacticPointing = true;
  } else {
    m_GalacticPointingXAxis = MVector(1.0, 0.0, 0.0);
    m_GalacticPointingZAxis = MVector(0.0, 0.0, 1.0);
    m_HasGalacticPointing = false; 
  }

  if (E->HasDetectorRotation() == true) { 
    m_DetectorRotationXAxis = E->m_DetectorRotationXAxis;
    m_DetectorRotationZAxis = E->m_DetectorRotationZAxis;
    m_HasDetectorRotation = true;  
  } else {
    m_DetectorRotationXAxis = MVector(1.0, 0.0, 0.0);
    m_DetectorRotationZAxis = MVector(0.0, 0.0, 1.0);
    m_HasDetectorRotation = false; 
  }

  if (E->HasHorizonPointing() == true) { 
    m_HorizonPointingXAxis = E->m_HorizonPointingXAxis;
    m_HorizonPointingZAxis = E->m_HorizonPointingZAxis;
    m_HasHorizonPointing = true; 
  } else {
    m_HorizonPointingXAxis = MVector(1.0, 0.0, 0.0);
    m_HorizonPointingZAxis = MVector(0.0, 0.0, 1.0);
    m_HasHorizonPointing = false; 
  }
  
  m_EventType = E->m_EventType;
  m_TimeWalk = E->m_TimeWalk;
  m_Time = E->m_Time;
  m_Id = E->m_Id;
  m_AllHitsGood = E->m_AllHitsGood;
  m_Decay = E->m_Decay;
  m_Bad = E->m_Bad;
  m_BadString = E->m_BadString;
  m_Comments = E->m_Comments;

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
  m_IsGoodEvent = false;
  m_AllHitsGood = true;

  m_GalacticPointingXAxis = MVector(1.0, 0.0, 0.0);
  m_GalacticPointingZAxis = MVector(0.0, 0.0, 1.0);
  m_HasGalacticPointing = false; 

  m_DetectorRotationXAxis = MVector(1.0, 0.0, 0.0);
  m_DetectorRotationZAxis = MVector(0.0, 0.0, 1.0);
  m_HasDetectorRotation = false; 
  
  m_HorizonPointingXAxis = MVector(1.0, 0.0, 0.0);
  m_HorizonPointingZAxis = MVector(0.0, 0.0, 1.0);
  m_HasHorizonPointing = false; 

  m_Time = 10000.0;
  m_Id = 0;;
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


MString MPhysicalEvent::GetComment(unsigned int i)
{
  //! Get the specific comment

  if (i < m_Comments.size()) {
    return m_Comments[i]; 
  }
  
  mout<<"Error: Index for comment out of bounds"<<endl;
  
  return "";
}
  
  
////////////////////////////////////////////////////////////////////////////////


bool MPhysicalEvent::Stream(MFile& File, int Version, bool Read, bool Fast, bool DelayedRead)
{
  // Hopefully a faster way to stream data from and to a file than ROOT...

  if (Read == false) {
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
    if (m_HasGalacticPointing == true) {
      double phi = m_GalacticPointingXAxis.Phi()*c_Deg;
      while (phi < 0.0) phi += 360.0;
      S<<"GX "<<phi<<" "<<m_GalacticPointingXAxis.Theta()*c_Deg - 90<<endl;
      phi = m_GalacticPointingZAxis.Phi()*c_Deg;
      while (phi < 0.0) phi += 360.0;
      S<<"GZ "<<phi<<" "<<m_GalacticPointingZAxis.Theta()*c_Deg - 90<<endl;
    } 
    if (m_HasDetectorRotation == true) {
      S<<"RX "<<m_DetectorRotationXAxis.X()<<" "<<m_DetectorRotationXAxis.Y()<<" "<<m_DetectorRotationXAxis.Z()<<endl;
      S<<"RZ "<<m_DetectorRotationZAxis.X()<<" "<<m_DetectorRotationZAxis.Y()<<" "<<m_DetectorRotationZAxis.Z()<<endl;
    }
    if (m_HasHorizonPointing == true) {
      S<<"HX "<<m_HorizonPointingXAxis.Phi()*c_Deg<<" "<<90 - m_HorizonPointingXAxis.Theta()*c_Deg<<endl;
      S<<"HZ "<<m_HorizonPointingZAxis.Phi()*c_Deg<<" "<<90 - m_HorizonPointingZAxis.Theta()*c_Deg<<endl;
    }
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
    
    File.Write(S);
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
      if (sscanf(Line, "ID %u", &m_Id) != 1) {
        Ret = 1;
      }
    }
  } else if (Line[0] == 'T' && Line[1] == 'W') {
    if (Fast == true) {
      m_TimeWalk = strtol(Line+3, NULL, 10);
    } else {
      if (sscanf(Line, "TW %d", &m_TimeWalk) != 1) {
        Ret = 1;
      }
    }
  } else if (Line[0] == 'R' && Line[1] == 'X') {
    if (Fast == true) {
      char* p;
      m_DetectorRotationXAxis[0] = strtod(Line+3, &p);
      m_DetectorRotationXAxis[1] = strtod(p, &p);
      m_DetectorRotationXAxis[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "RX %lf %lf %lf", &m_DetectorRotationXAxis[0], &m_DetectorRotationXAxis[1], &m_DetectorRotationXAxis[2]) != 3) {
        Ret = 1;
      }
    }
    m_HasDetectorRotation = true;
  } else if (Line[0] == 'R' && Line[1] == 'Z') {
    if (Fast == true) {
      char* p;
      m_DetectorRotationZAxis[0] = strtod(Line+3, &p);
      m_DetectorRotationZAxis[1] = strtod(p, &p);
      m_DetectorRotationZAxis[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "RZ %lf %lf %lf", &m_DetectorRotationZAxis[0], &m_DetectorRotationZAxis[1], &m_DetectorRotationZAxis[2]) != 3) {
        Ret = 1;
      }
    }
    m_HasDetectorRotation = true;
  } else if (Line[0] == 'G' && Line[1] == 'X') {
    double Longitude, Latitude;
    if (Fast == true) {
      char* p;
      Longitude = strtod(Line+3, &p);
      Latitude = strtod(p, NULL);
    } else {
      if (sscanf(Line, "GX %lf %lf", &Longitude, &Latitude) != 2) {
        Ret = 1;
      }
    }
    SetGalacticPointingXAxis(Longitude, Latitude);
  } else if (Line[0] == 'G' && Line[1] == 'Z') {
    double Longitude, Latitude;
    if (Fast == true) {
      char* p;
      Longitude = strtod(Line+3, &p);
      Latitude = strtod(p, NULL);
    } else {
      if (sscanf(Line, "GZ %lf %lf", &Longitude, &Latitude) != 2) {
        Ret = 1;
      }
    }
    SetGalacticPointingZAxis(Longitude, Latitude);
  } else if (Line[0] == 'H' && Line[1] == 'X') {
    double Azimuth, Elevation;
    if (Fast == true) {
      char* p;
      Azimuth = strtod(Line+3, &p);
      Elevation = strtod(p, NULL);
    } else {
      if (sscanf(Line, "HX %lf %lf", &Azimuth, &Elevation) != 2) {
        Ret = 1;
      }
    }
    SetHorizonPointingXAxis(Azimuth, Elevation);
  } else if (Line[0] == 'H' && Line[1] == 'Z') {
    double Azimuth, Elevation;
    if (Fast == true) {
      char* p;
      Azimuth = strtod(Line+3, &p);
      Elevation = strtod(p, NULL);
    } else {
      if (sscanf(Line, "HZ %lf %lf", &Azimuth, &Elevation) != 2) {
        Ret = 1;
      }
    }
    SetHorizonPointingZAxis(Azimuth, Elevation);
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
  } else if (Line[0] == 'E' && Line[1] == 'N') {
    Ret = -1;
  } else if (Line[0] == 'N' && Line[1] == 'F') {
    Ret = -1;
  } else {
    Ret = 2;
  }

  return Ret;
}


// MPhysicalEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
