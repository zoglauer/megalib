/*
 * MSimGR.cxx
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


// Include the header:
#include "MSimGR.h"

// Standard libs:
#include <sstream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGuardRing.h"
#include "MStreams.h"

#ifdef ___CINT___
ClassImp(MSimGR)
#endif


////////////////////////////////////////////////////////////////////////////////


MSimGR::MSimGR(MDGeometryQuest* Geo)
{
  // standard constructor

  m_DetectorType = MDDetector::c_NoDetectorType;
  m_Position = g_VectorNotDefined;
  m_Energy = 0;
  m_OriginalEnergy = m_Energy;
  m_VolumeSequence = 0;
  m_Geometry = Geo;
}


////////////////////////////////////////////////////////////////////////////////


MSimGR::MSimGR(const int DetectorType, const MVector& Position, const double Energy, MDGeometryQuest* Geo) : 
  m_DetectorType(DetectorType), m_Position(Position), m_Energy(Energy), m_Geometry(Geo)
{
  // extended constructor
  
  if (m_Geometry != 0) {
    m_VolumeSequence = m_Geometry->GetVolumeSequencePointer(m_Position, true, true);
    if (m_VolumeSequence->GetDetector() == 0) {
      mout<<"Error: This guard ring hit at "<<m_Position<<" has no corresponding detector volume!"<<endl;
    }
    if (m_VolumeSequence->GetSensitiveVolume() == 0) {
      mout<<"Error: This guard ring hit at "<<m_Position<<" has no corresponding sensitive volume!"<<endl;
    }
  }
  
  m_OriginalEnergy = m_Energy;
}


////////////////////////////////////////////////////////////////////////////////


MSimGR::~MSimGR()
{
  // standard destructor
  
  delete m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimGR::AddRawInput(MString LineBuffer, const int Version)
{
  // Analyze one line of text input...
  
  if (sscanf(LineBuffer.Data(), "GR %lf;%lf;%lf;%lf",
             &m_Position[0], &m_Position[1], &m_Position[2],
             &m_Energy) != 4) {
    if (sscanf(LineBuffer.Data(), "GRsim %d;%lf;%lf;%lf;%lf",
               &m_DetectorType,
               &m_Position[0], &m_Position[1], &m_Position[2],
               &m_Energy) != 5) {
      return false;
    }
  }

  m_OriginalEnergy = m_Energy; 
  
  if (Noise(true) == false) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimGR::ToSimString(const int WhatToStore, const int ScientificPrecision, const int Version) const
{
  // Convert this SimEvent to the original *.sim file format...

  // If there is a significant change, make sure you make new sim version,
  // and update all reading functions

  ostringstream S;

  int WidthPos;
  int WidthEnergy;
  //int WidthTime;
  int Precision;
  if (ScientificPrecision > 0) {
    WidthPos = ScientificPrecision+7;
    WidthEnergy = ScientificPrecision+6;
    //WidthTime = ScientificPrecision+6;
    Precision = ScientificPrecision;
    S.setf(ios_base::scientific, ios_base::floatfield);
  } else {
    WidthPos = 10;
    WidthEnergy = 10;
    //WidthTime = 11;
    Precision = 5;
    S.setf(ios_base::fixed, ios_base::floatfield);
  }

  S<<"GRsim "<<m_DetectorType<<";";
  S<<setprecision(Precision);
  S<<setw(WidthPos)<<m_Position[0]<<";";
  S<<setw(WidthPos)<<m_Position[1]<<";";
  S<<setw(WidthPos)<<m_Position[2]<<";";
  S<<setw(WidthEnergy)<<m_Energy;

  return S;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimGR::Noise(bool RecalculateVolumeSequence) 
{ 
  // Recalculate the volume sequence:

  // Make sure we noise not twice:
  m_Energy = m_OriginalEnergy;

  //cout<<"Before: "<<m_Energy<<" --> ";
  if (m_Geometry != 0) {
    if (RecalculateVolumeSequence == true) {
      delete m_VolumeSequence;
      m_VolumeSequence = m_Geometry->GetVolumeSequencePointer(m_Position, true, true);
      if (m_VolumeSequence->GetDetector() == 0) {
        mout<<"This guard ring hit at "<<m_Position<<" has no corresponding detector volume!"<<endl;
        mout<<"  --> It will not show up in the later analysis!"<<endl;
        return false;
      }
      if (m_VolumeSequence->GetSensitiveVolume() == 0) {
        mout<<"This guard ring hit at "<<m_Position<<" has no corresponding sensitive volume!"<<endl;
        mout<<"  --> It will not show up in the later analysis!"<<endl;
        return false;
      }
    }
    if (m_Geometry->GetActivateNoising() == true) {
      if (m_VolumeSequence->GetDetector()->HasGuardRing() == true) {
        double DummyTime = 0.0;
        m_VolumeSequence->GetDetector()->GetGuardRing()->Noise(m_Position, m_Energy, DummyTime, nullptr);
      } else {
        mout<<"The detector of this guard ring hit at "<<m_Position<<" has no guard ring detector!"<<endl;
        mout<<"  --> It will not show up in the later analysis!"<<endl;
        return false;  
      }

      if (m_Energy <= 0.0) return false; // Below trigger threshold of guard ring
    }    
  }
  //cout<<"After "<<m_Energy<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSimGR::SetVolumeSequence(MDVolumeSequence* VolumeSequence)
{
  // Set the volume, inside which this hit lies

  delete m_VolumeSequence;
  m_VolumeSequence = VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence* MSimGR::GetVolumeSequence()
{
  // Return the volume, in which this hit lies, or zero

  if (m_VolumeSequence == 0 && m_Geometry != 0) {
    m_VolumeSequence = m_Geometry->GetVolumeSequencePointer(m_Position, true, true);
    if (m_VolumeSequence->GetDetector() == 0) {
      mout<<"Error: This guard ring hit at "<<m_Position<<" has no corresponding detector volume!"<<endl;
    }
    if (m_VolumeSequence->GetSensitiveVolume() == 0) {
      mout<<"Error: This guard ring hit at "<<m_Position<<" has no corresponding sensitive volume!"<<endl;
    }
  }
  
  return m_VolumeSequence;
}


// MSimGR: the end...
////////////////////////////////////////////////////////////////////////////////
