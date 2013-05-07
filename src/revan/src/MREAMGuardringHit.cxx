/*
 * MREAMGuardringHit.cxx
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
// MREAMGuardringHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREAMGuardringHit.h"

// Standard libs:
#include <iostream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MREAMGuardringHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MREAMGuardringHit::MREAMGuardringHit() : MREAM()
{
  // Construct an instance of MREAMGuardringHit

  m_Type = c_GuardringHit;
  
  m_Energy = g_DoubleNotDefined;
  m_EnergyResolution = g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MREAMGuardringHit::MREAMGuardringHit(const MREAMGuardringHit& REAM)
{
  // Construct an instance of MREAMGuardringHit

  m_Type = REAM.m_Type;
  m_VolumeSequence = new MDVolumeSequence(*(REAM.m_VolumeSequence));
  m_Energy = REAM.m_Energy;
  m_EnergyResolution = REAM.m_EnergyResolution;
}


////////////////////////////////////////////////////////////////////////////////


MREAMGuardringHit::~MREAMGuardringHit()
{
  // Delete this instance of MREAMGuardringHit
}


////////////////////////////////////////////////////////////////////////////////


MREAM* MREAMGuardringHit::Clone()
{
  // Duplicate this REAM

  massert(this != 0);
  return new MREAMGuardringHit(*this);
}


////////////////////////////////////////////////////////////////////////////////



MString MREAMGuardringHit::ToEvtaString(const int ScientificPrecision, const int Version)
{  
  //! Convert to a string in the evta file

  merr<<"This function has never ever been tested!"<<endl;

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
  
  
  MVector Pos = m_VolumeSequence->GetPositionInSensitiveVolume();
  S<<"GR "<<m_VolumeSequence->GetDetector()->GetType()<<";";
  S<<setprecision(Precision);
  S<<setw(WidthPos)<<Pos[0]<<";";
  S<<setw(WidthPos)<<Pos[1]<<";";
  S<<setw(WidthPos)<<Pos[2]<<";";
  S<<setw(WidthEnergy)<<m_Energy;
  S<<endl;

  return S.str().c_str();
}


// MREAMGuardringHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
