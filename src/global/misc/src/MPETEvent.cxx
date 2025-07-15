/*
 * MPETEvent.cxx
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
// MPETEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPETEvent.h"

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
ClassImp(MPETEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MPETEvent::MPETEvent()
{
  m_EventType = c_PETEvent;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MPETEvent::~MPETEvent()
{
  // Intentionally empty
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a human readable string
MString MPETEvent::ToString() const
{
  const int Length = 1000;
  char Text[Length];
  MString String("Position-emmision-tomography event:\n"); 
  
  snprintf(Text, Length, "  Energy1: %.3f +- %.3f \n", m_Energy1, m_EnergyResolution1);
  String += MString(Text);
  
  snprintf(Text, Length, "  Position1: %.3f, %.3f, %.3f  +- %.3f %.3f  %.3f \n",
          m_Position1.X(), m_Position1.Y(), m_Position1.Z(), 
          m_PositionResolution1.X(), m_PositionResolution1.Y(), m_PositionResolution1.Z());
  String += MString(Text);

  snprintf(Text, Length, "  Timing1: %.9f +- %.3f \n", m_Timing1, m_TimingResolution1);
  String += MString(Text);
  
  snprintf(Text, Length, "  Energy2: %.3f +- %.3f \n", m_Energy2, m_EnergyResolution2);
  String += MString(Text);
  
  snprintf(Text, Length, "  Position2: %.3f, %.3f, %.3f +- %.3f  %.3f  %.3f \n",
          m_Position2.X(), m_Position2.Y(), m_Position2.Z(), 
          m_PositionResolution2.X(), m_PositionResolution2.Y(), m_PositionResolution2.Z());
  String += MString(Text);
  
  snprintf(Text, Length, "  Timing2: %.9f +- %.9f \n", m_Timing2, m_TimingResolution2);
  String += MString(Text);
  

  return String;
}


////////////////////////////////////////////////////////////////////////////////


bool MPETEvent::Assimilate(MPETEvent* PET)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray...
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton scattering, we return false.
  
  Reset();
  
  // Get only the basic data and calculate the rest:
  MPhysicalEvent::Assimilate(dynamic_cast<MPhysicalEvent*>(PET));
  
  m_Energy1 = PET->GetEnergy1();       
  m_Position1 = PET->GetPosition1();
  m_Timing1 = PET->GetTiming1();
  m_Energy2 = PET->GetEnergy2();       
  m_Position2 = PET->GetPosition2();
  m_Timing2 = PET->GetTiming2();
  
  m_EnergyResolution1 = PET->GetEnergyResolution1();       
  m_PositionResolution1 = PET->GetPositionResolution1();
  m_TimingResolution1 = PET->GetTimingResolution1();
  m_EnergyResolution2 = PET->GetEnergyResolution2();       
  m_PositionResolution2 = PET->GetPositionResolution2();
  m_TimingResolution2 = PET->GetTimingResolution2();
  
  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MPETEvent::Assimilate(MPhysicalEvent* Event)
{
  // Simply Call: MPETEvent::Assimilate(const MPETEventData *PETEventData)

  if (Event->GetType() == c_PETEvent) {
    return Assimilate(dynamic_cast<MPETEvent*>(Event));
  } else {
    merr<<"Trying to assimilate a non-PET event!"<<endl; 
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MPETEvent::Duplicate()
{
  MPhysicalEvent* Event = new MPETEvent();
  Event->Assimilate(this);

  return dynamic_cast<MPhysicalEvent*>(Event);
}


////////////////////////////////////////////////////////////////////////////////


bool MPETEvent::Validate()
{
  // Do some sanity checks on the event and calculate all high level data:

  if (m_Energy1 <= 0) return false;
  if (m_Energy2 <= 0) return false;
  
  if (m_EnergyResolution1 < 0) return false;
  if (m_EnergyResolution2 < 0) return false;
  
  if (m_PositionResolution1[0] < 0) return false;
  if (m_PositionResolution2[0] < 0) return false;
  
  if (m_PositionResolution1[1] < 0) return false;
  if (m_PositionResolution2[1] < 0) return false;
  
  if (m_PositionResolution1[2] < 0) return false;
  if (m_PositionResolution2[2] < 0) return false;
  
  if (m_TimingResolution1 < 0) return false;
  if (m_TimingResolution1 < 0) return false;
  
  m_IsGoodEvent = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MPETEvent::ToTraString() const
{
  //! Stream the content into a tra-file compatible string

  MString T;
  T += MPhysicalEvent::ToTraString();

  ostringstream S;
  S<<"EF "<<m_Energy1<<" "<<m_EnergyResolution1<<endl;
  S<<"PF "<<m_Position1[0]<<" "<<m_Position1[1]<<" "<<m_Position1[2]<<"    "<<m_PositionResolution1[0]<<" "<<m_PositionResolution1[1]<<" "<<m_PositionResolution1[2]<<endl;
  S<<"ZF "<<m_Timing1<<" "<<m_TimingResolution1<<endl;
  S<<"ES "<<m_Energy2<<" "<<m_EnergyResolution2<<endl;
  S<<"PS "<<m_Position2[0]<<" "<<m_Position2[1]<<" "<<m_Position2[2]<<"    "<<m_PositionResolution2[0]<<" "<<m_PositionResolution2[1]<<" "<<m_PositionResolution2[2]<<endl;
  S<<"ZS "<<m_Timing2<<" "<<m_TimingResolution2<<endl;

  T += S.str();

  return T;
}


////////////////////////////////////////////////////////////////////////////////


int MPETEvent::ParseLine(const char* Line, bool Fast)
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
  
  if (Line[0] == 'E' && Line[1] == 'F') {
    if (Fast == true) {
      m_Energy1 = strtod(Line+3, NULL);
      m_EnergyResolution1 = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "EF %lf %lf", &m_Energy1, &m_EnergyResolution1) != 2) {
        cout<<"Unable to parse EF of PET event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'P' && Line[1] == 'F') {
    if (Fast == true) {
      char* p;
      m_Position1[0] = strtod(Line+3, &p);
      m_Position1[1] = strtod(p, &p);
      m_Position1[2] = strtod(p, &p);
      m_PositionResolution1[0] = strtod(p, &p);
      m_PositionResolution1[1] = strtod(p, &p);
      m_PositionResolution1[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "PF %lf %lf %lf %lf %lf %lf", &m_Position1[0], &m_Position1[1], &m_Position1[2], &m_PositionResolution1[0], &m_PositionResolution1[1], &m_PositionResolution1[2]) != 6) {
        cout<<"Unable to parse PF of PET event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'Z' && Line[1] == 'F') {
    if (Fast == true) {
      char* p;
      m_Timing1 = strtod(Line+3, &p);
      m_TimingResolution1 = strtod(p, NULL);
    } else {
      if (sscanf(Line, "ZF %lf %lf", &m_Timing1, &m_TimingResolution1) != 2) {
        mout<<"Unable to parse ZF of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'E' && Line[1] == 'S') {
    if (Fast == true) {
      m_Energy2 = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "ES %lf", &m_Energy2) != 1) {
        cout<<"Unable to parse ES of PET event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'P' && Line[1] == 'S') {
    if (Fast == true) {
      char* p;
      m_Position2[0] = strtod(Line+3, &p);
      m_Position2[1] = strtod(p, &p);
      m_Position2[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "PS %lf %lf %lf", &m_Position2[0], &m_Position2[1], &m_Position2[2]) != 3) {
        cout<<"Unable to parse PF of PET event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'Z' && Line[1] == 'S') {
    if (Fast == true) {
      char* p;
      m_Timing2 = strtod(Line+3, &p);
      m_TimingResolution2 = strtod(p, NULL);
    } else {
      if (sscanf(Line, "ZS %lf %lf", &m_Timing2, &m_TimingResolution2) != 2) {
        mout<<"Unable to parse ZS of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else {
    Ret = 2;
  }
  
  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


void MPETEvent::Reset() 
{
  // Reset all values to zero:
  
  MPhysicalEvent::Reset();
  
  m_Energy1 = 0.0;       
  m_Position1 = MVector(0.0, 0.0, 0.0);
  m_Timing1 = 0;
  m_Energy2 = 0.0;       
  m_Position2 = MVector(0.0, 0.0, 0.0);
  m_Timing2 = 0;
  
  m_EnergyResolution1 = 0.0;       
  m_PositionResolution1 = MVector(0.0, 0.0, 0.0);
  m_TimingResolution1 = 0;
  m_EnergyResolution2 = 0.0;       
  m_PositionResolution2 = MVector(0.0, 0.0, 0.0);
  m_TimingResolution2 = 0;
  
  m_IsGoodEvent = false;

  return;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MPETEvent::Data() 
{
  // Return a pointer to this event
 
  return dynamic_cast<MPhysicalEvent*>(this); 
} 


////////////////////////////////////////////////////////////////////////////////


double MPETEvent::GetResolutionMeasure(const MVector& Position, const MCoordinateSystem& CS) 
{
  // Rotate/translate the position into event coordinates
  MVector RotPosition = Position;
  if (m_HasDetectorRotation == true) RotPosition = GetDetectorInverseRotationMatrix()*RotPosition;
  if (CS == MCoordinateSystem::c_Galactic || CS == MCoordinateSystem::c_Spheric) {
    merr<<"PET resolution measure is only reasonable in the near field!"<<endl;
    return g_DoubleNotDefined;
  }
  
  return RotPosition.DistanceToLine(m_Position1, m_Position2);
}


// MPETEvent: the end...
////////////////////////////////////////////////////////////////////////////////
