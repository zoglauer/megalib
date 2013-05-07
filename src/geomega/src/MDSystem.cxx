/*
 * MDSystem.cxx
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
// MDSystem
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDSystem.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDDetector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDSystem)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDSystem::c_TimeResolutionTypeUnknown = 0;
const int MDSystem::c_TimeResolutionTypeIdeal   = 1;
const int MDSystem::c_TimeResolutionTypeGauss   = 2;


////////////////////////////////////////////////////////////////////////////////


MDSystem::MDSystem(const MString& Name)
{
  // Construct an instance of MDSystem

  m_Name = Name;

  m_TimeResolutionType = c_TimeResolutionTypeIdeal;
  
  m_TimeResolutionGaussSigma = 1E-10;
}


////////////////////////////////////////////////////////////////////////////////


MDSystem::~MDSystem()
{
  // Delete this instance of MDSystem
}


////////////////////////////////////////////////////////////////////////////////


MString MDSystem::GetName() const
{
  // Return the name of this trigger condition

  return m_Name;
}


////////////////////////////////////////////////////////////////////////////////


void MDSystem::NoiseTime(MTime& Time)
{
  // Noise the system time

  if (m_TimeResolutionType == c_TimeResolutionTypeGauss) {
    MTime Noise;
    double Diff = gRandom->Gaus(0.0, m_TimeResolutionGaussSigma);
    Noise.Set(Diff);
    Time += Noise;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDSystem::SetTimeResolutionType(const int TimeResolutionType) 
{ 
  //! Set the energy resolution type, return false if you try to overwrite an existing type

  if (m_TimeResolutionType == TimeResolutionType) return true;

  if (TimeResolutionType == c_TimeResolutionTypeIdeal ||
      TimeResolutionType == c_TimeResolutionTypeGauss) {
    m_TimeResolutionType = TimeResolutionType; 
  } else {
    mout<<"   ***  Error  ***  in system "<<m_Name<<endl;
    mout<<"Unknown time resolution type: "<<TimeResolutionType<<endl;
    return false;    
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDSystem::SetTimeResolutionGaussSigma(const double Sigma)
{
  // Set the Time resolution

  if (Sigma <= 0) {
    mout<<"   ***  Error  ***  in system "<<m_Name<<endl;
    mout<<"Input time resolution needs to be psoitive!"<<endl;
    return; 
  }

  m_TimeResolutionGaussSigma = Sigma;
}


////////////////////////////////////////////////////////////////////////////////


MString MDSystem::GetGeomega() const
{
  // Return as Geomega type volume tree

  ostringstream out;
  out<<"System "<<m_Name<<endl;
  
  if (m_TimeResolutionType == c_TimeResolutionTypeGauss) {
    out<<m_Name<<".TimeResolutionType Gauss"<<endl;
    out<<m_Name<<".Time Resolution "<<m_TimeResolutionGaussSigma<<endl;
  } else {
    out<<m_Name<<".TimeResolutionType Ideal"<<endl;    
  }

  return out.str().c_str(); 
}


////////////////////////////////////////////////////////////////////////////////


MString MDSystem::ToString() const
{
  // Return the Geant4 code of this trigger condition

  ostringstream out;

  out<<(*this);

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


bool MDSystem::Validate()
{
  // Make sure everything is reasonable:

  return true;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MDSystem& R)
{
  os<<"System "<<R.m_Name<<endl;

  return os;
}


// MDSystem.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
