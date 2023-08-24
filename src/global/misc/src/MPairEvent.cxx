/*
 * MPairEvent.cxx
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
// MPairEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPairEvent.h"

// Standard libs:
#include <cstdlib>
#include <iostream>
using namespace std;

// ROOT libs:
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MPairEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


const double MPairEvent::m_RestEnergyElectron = 511.004;


////////////////////////////////////////////////////////////////////////////////


MPairEvent::MPairEvent()
{
  // standard constructor

  m_EventType = c_Pair;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MPairEvent::~MPairEvent()
{
  // standard destructor
}

////////////////////////////////////////////////////////////////////////////////


void MPairEvent::Reset() 
{
  // Reset all values to zero:
  
  MPhysicalEvent::Reset();

  m_PairCreationIA = MVector(0.0, 0.0, 0.0);     
  m_ElectronDirection = MVector(0.0, 0.0, 0.0);  
  m_PositronDirection = MVector(0.0, 0.0, 0.0);      

  m_EnergyElectron = 0.0;
  m_EnergyPositron = 0.0; 

  m_EnergyErrorElectron = 0.0; 
  m_EnergyErrorPositron = 0.0; 

  m_InitialEnergyDeposit = 0.0;

  m_TrackQualityFactor = 0.0;

  m_EPDistinguishable = false;
}


////////////////////////////////////////////////////////////////////////////////


bool MPairEvent::MostProbableDirectionIncomingGamma()
{
  // Let's start simple:
  
  // For the MEGA prototype:
  m_IncomingGammaDirection = -(m_ElectronDirection + m_PositronDirection);

  // For all the rest:
  m_IncomingGammaDirection = 
    -(m_EnergyElectron*m_ElectronDirection + m_EnergyPositron*m_PositronDirection)*
    (1.0/(m_EnergyElectron+m_EnergyPositron));

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MPairEvent::Validate()
{
  // Do some initial computations after assimilation:
  // Test if the event is ok, compute the scatter angles and the 
  // compton-axis-Vector, etc.

  // Sometimes the real data is so false that we are unable to compute a compton-angle: 
  MostProbableDirectionIncomingGamma();
 
  m_IsGoodEvent = true;

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MPairEvent::Assimilate(MPairEvent* PairEventData)
{
  if (MPhysicalEvent::Assimilate(PairEventData) == false) return false;

  SetPairCreationIA(PairEventData->GetPairCreationIA());
  SetElectronDirection(PairEventData->GetElectronDirection());
  SetPositronDirection(PairEventData->GetPositronDirection());
  SetEnergyElectron(PairEventData->GetEnergyElectron());
  SetEnergyErrorElectron(PairEventData->GetEnergyErrorElectron());
  SetEnergyPositron(PairEventData->GetEnergyPositron());
  SetEnergyErrorPositron(PairEventData->GetEnergyErrorPositron());
  SetInitialEnergyDeposit(PairEventData->GetInitialEnergyDeposit());
  SetTrackQualityFactor(PairEventData->GetTrackQualityFactor());

  if (Validate() == false) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MPairEvent::Assimilate(MPhysicalEvent* Event)
{
  if (Event->GetType() == MPhysicalEvent::c_Pair) {
    return Assimilate(dynamic_cast<MPairEvent*>(Event));
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MPairEvent::Duplicate()
{
  // Duplicate this event

  MPhysicalEvent* Event = new MPairEvent();
  Event->Assimilate(this);

  return dynamic_cast<MPhysicalEvent*>(Event);
}


////////////////////////////////////////////////////////////////////////////////


MVector MPairEvent::GetPairCreationIA() const
{
  // Return the pair creation interaction position

  return m_PairCreationIA;
}


////////////////////////////////////////////////////////////////////////////////


void MPairEvent::SetPairCreationIA(const MVector& PCIA)
{
  // Set the pair creation interaction position

  m_PairCreationIA = PCIA;
}


////////////////////////////////////////////////////////////////////////////////


MVector MPairEvent::GetElectronDirection() const
{
  // Return the direction of the electron

  return m_ElectronDirection;
}


////////////////////////////////////////////////////////////////////////////////


void MPairEvent::SetElectronDirection(const MVector& ED)
{
  // Set the direction of the electron

  m_ElectronDirection = ED;
}


////////////////////////////////////////////////////////////////////////////////


MVector MPairEvent::GetPositronDirection() const
{
  // Return the direction of the positron

  return m_PositronDirection;
}


////////////////////////////////////////////////////////////////////////////////


void MPairEvent::SetPositronDirection(const MVector& PD)
{
  // Set the direction of the positron

  m_PositronDirection = PD;
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetEnergyElectron() const
{
  // Return the energy of the electron 

  return m_EnergyElectron;
}


////////////////////////////////////////////////////////////////////////////////


void MPairEvent::SetEnergyElectron(const double Energy)
{
  // Set the energy of the electron

  m_EnergyElectron = Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetEnergyErrorElectron() const
{
  // Return the energy error of the electron 

  return m_EnergyErrorElectron;
}


////////////////////////////////////////////////////////////////////////////////


void MPairEvent::SetEnergyErrorElectron(const double Energy)
{
  // Set the energy error of the electron 

  m_EnergyErrorElectron = Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetEnergyPositron() const
{
  // Return the energy of the positron

  return m_EnergyPositron;
}


////////////////////////////////////////////////////////////////////////////////


void MPairEvent::SetEnergyPositron(const double Energy)
{
  // Set energy of the positron

  m_EnergyPositron = Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetEnergyErrorPositron() const
{
  // Return the energy error of the positron

  return m_EnergyErrorPositron;
}


////////////////////////////////////////////////////////////////////////////////


void MPairEvent::SetEnergyErrorPositron(const double Energy)
{
  // Set the energy error of the positron

  m_EnergyErrorPositron = Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetInitialEnergyDeposit() const
{
  // Get the energy deposit in the first layer

  return m_InitialEnergyDeposit;
}


////////////////////////////////////////////////////////////////////////////////


void MPairEvent::SetInitialEnergyDeposit(const double Energy)
{
  // Set the energy deposit in the first layer

  m_InitialEnergyDeposit = Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetOpeningAngle() const
{
  // Return the angle between electron ans positron

  return m_ElectronDirection.Angle(m_PositronDirection);
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetARMGamma(const MVector& Position, const MCoordinateSystem& CS)
{
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-Spitze with the 
  // (Test-)position

  // Rotate the position into event coordinates
  
  MVector RotPosition = Position;
  if (m_HasDetectorRotation == true) RotPosition = GetDetectorRotationMatrix().Invert()*RotPosition;
  if (CS == MCoordinateSystem::c_Galactic && m_HasGalacticPointing == true) RotPosition = GetGalacticPointingInverseRotationMatrix()*RotPosition;
  
  return m_IncomingGammaDirection.Angle(RotPosition - m_PairCreationIA);
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetAzimuthalScatterAngle(const MVector& Position, const MCoordinateSystem& CS)
{
  //! Return the azimuthal scatter angle value for the given test position in the given coordinate system

  // Rotate the position into event coordinates
  MVector RotPosition = Position;
  if (m_HasDetectorRotation == true) RotPosition = GetDetectorRotationMatrix().Invert()*RotPosition;
  if (CS == MCoordinateSystem::c_Galactic && m_HasGalacticPointing == true) RotPosition = GetGalacticPointingInverseRotationMatrix()*RotPosition;

  MVector Plain = GetElectronDirection() + GetPositronDirection();
  Plain.RotateZ(-RotPosition.Phi());
  Plain.RotateY(-RotPosition.Theta());

  return Plain.Phi();
}


////////////////////////////////////////////////////////////////////////////////


bool MPairEvent::Assimilate(char* LineBuffer)
{
  // Takeover the event from a data-line

  int i;
  double m_xPairCreationD1, m_yPairCreationD1, m_zPairCreationD1; 
  double m_xFirstElectronDir, m_yFirstElectronDir, m_zFirstElectronDir;         
  double m_xSecondElectronDir, m_ySecondElectronDir, m_zSecondElectronDir;

  if (LineBuffer[0] == 'P') {
    i = sscanf(LineBuffer, 
         "P;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf\n", 
         &m_xPairCreationD1,
         &m_yPairCreationD1,
         &m_zPairCreationD1, 
         &m_xFirstElectronDir,
         &m_yFirstElectronDir,
         &m_zFirstElectronDir,          
         &m_xSecondElectronDir,
         &m_ySecondElectronDir,
         &m_zSecondElectronDir,
         &m_EnergyElectron,
         &m_EnergyPositron,
         &m_EnergyErrorElectron,
         &m_EnergyErrorPositron);
    if (i == 13) {

      SetPairCreationIA(MVector(m_xPairCreationD1, m_yPairCreationD1, m_zPairCreationD1));
      SetElectronDirection(MVector(m_xFirstElectronDir, m_yFirstElectronDir, m_zFirstElectronDir));
      SetPositronDirection(MVector(m_xSecondElectronDir, m_ySecondElectronDir, m_zSecondElectronDir));

      if (Validate() == false) return false;
      
      return true;
    }
    else {
      return false;
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MPairEvent::ToTraString() const
{
  //! Stream the content into a tra-file compatible string

  MString T;
  T += MPhysicalEvent::ToTraString();

  ostringstream S;
  S<<"PC "<<m_PairCreationIA.X()<<" "<<m_PairCreationIA.Y()<<" "<<m_PairCreationIA.Z()<<endl;
  S<<"PE "<<m_EnergyElectron<<" "<<m_EnergyErrorElectron<<" "<<m_ElectronDirection.X()<<" "<<m_ElectronDirection.Y()<<" "<<m_ElectronDirection.Z()<<endl;
  S<<"PP "<<m_EnergyPositron<<" "<<m_EnergyErrorPositron<<" "<<m_PositronDirection.X()<<" "<<m_PositronDirection.Y()<<" "<<m_PositronDirection.Z()<<endl;
  S<<"PI "<<m_InitialEnergyDeposit<<endl;
  S<<"TQ "<<m_TrackQualityFactor<<endl;

  T += S.str();

  return T;
}


////////////////////////////////////////////////////////////////////////////////


int MPairEvent::ParseLine(const char* Line, bool Fast)
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

  if (Line[0] == 'P' && Line[1] == 'C') {
    if (Fast == true) {
      char* p;
      m_PairCreationIA[0] = strtod(Line+3, &p);
      m_PairCreationIA[1] = strtod(p, &p);
      m_PairCreationIA[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "PC %lf %lf %lf", &m_PairCreationIA[0], &m_PairCreationIA[1], &m_PairCreationIA[2]) != 3) {
        cout<<"Unable to parse PC of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'P' && Line[1] == 'E') {
    if (Fast == true) {
      char* p;
      m_EnergyElectron = strtod(Line+3, &p);
      m_EnergyErrorElectron = strtod(p, &p);
      m_ElectronDirection[0] = strtod(p, &p);
      m_ElectronDirection[1] = strtod(p, &p);
      m_ElectronDirection[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "PE %lf %lf %lf %lf %lf", 
                 &m_EnergyElectron, &m_EnergyErrorElectron, 
                 &m_ElectronDirection[0], &m_ElectronDirection[1], &m_ElectronDirection[2]) != 5) {
        cout<<"Unable to parse PE of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'P' && Line[1] == 'P') {
    if (Fast == true) {
      char* p;
      m_EnergyPositron = strtod(Line+3, &p);
      m_EnergyErrorPositron = strtod(p, &p);
      m_PositronDirection[0] = strtod(p, &p);
      m_PositronDirection[1] = strtod(p, &p);
      m_PositronDirection[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "PP %lf %lf %lf %lf %lf", 
                 &m_EnergyPositron, &m_EnergyErrorPositron, 
                 &m_PositronDirection[0], &m_PositronDirection[1], &m_PositronDirection[2]) != 5) {
        cout<<"Unable to parse PP of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'P' && Line[1] == 'I') {
    if (Fast == true) {
      m_InitialEnergyDeposit = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "PI %lf", 
                 &m_InitialEnergyDeposit) != 1) {
        cout<<"Unable to parse PI of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'T' && Line[1] == 'Q') {
    if (Fast == true) {
      m_TrackQualityFactor = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "TQ %lf", 
                 &m_TrackQualityFactor) != 1) {
        cout<<"Unable to parse TQ of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else {
    Ret = 2;
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


MString MPairEvent::ToBasicString()
{
  // Transform the data to one line of text

  const int Length = 1000;
  char LineBuffer[Length];

  snprintf(LineBuffer, Length, "P;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f\n",
    m_PairCreationIA.X(), m_PairCreationIA.Y(), m_PairCreationIA.Z(), 
    m_ElectronDirection.X(), m_ElectronDirection.Y(), m_ElectronDirection.Z(), 
    m_PositronDirection.X(), m_PositronDirection.Y(), m_PositronDirection.Z(), 
    m_EnergyElectron, m_EnergyPositron, 
    m_EnergyErrorElectron, m_EnergyErrorPositron);

  return MString(LineBuffer);
}


////////////////////////////////////////////////////////////////////////////////


double MPairEvent::GetEnergy() const
{
  // Return the total energy of this event

  return m_EnergyElectron + m_EnergyPositron;
}


// MPairEvent: the end...
////////////////////////////////////////////////////////////////////////////////
