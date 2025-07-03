/*
 * MCalibrationSpectralPoint.cxx
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
#include "MCalibrationSpectralPoint.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MCalibrationSpectralPoint)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationSpectralPoint::MCalibrationSpectralPoint() : m_Peak(0), m_FWHM(-1), m_Counts(0), m_LowEdge(0), m_HighEdge(0), m_Fit(0), m_IsGood(true)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationSpectralPoint::~MCalibrationSpectralPoint()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Get the fit, if it doesn't exist throw MExceptionObjectDoesNotExist
//! Bad design since a new fit can be set any time...
const MCalibrationFit& MCalibrationSpectralPoint::GetFit() const 
{ 
  if (m_Fit == nullptr) {
    throw MExceptionObjectDoesNotExist("Fit does not exist!");
  }
  
  return *m_Fit; 
}


////////////////////////////////////////////////////////////////////////////////


//! Get the fit, if it doesn't exist throw MExceptionObjectDoesNotExist
//! Bad design since a new fit can be set any time...
MCalibrationFit& MCalibrationSpectralPoint::GetFit() 
{ 
  if (m_Fit == nullptr) {
    throw MExceptionObjectDoesNotExist("Fit does not exist!");
  }
  
  return *m_Fit; 
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a string
MString MCalibrationSpectralPoint::ToString() const
{
  ostringstream os;
  if (IsGood() == true) {
    os<<"Good ";
  } else {
    os<<"Bad ";
  }
  os<<"peak at "<<m_Peak<<" ["<<m_LowEdge<<", "<<m_HighEdge<<"] with ~"<<m_Counts<<" counts";
  if (IsGood() == true) {
    os<<", corresponding to an energy of "<<m_Energy<<" keV with a FWHM of "<<GetEnergyFWHM()<<" keV";
  }
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a string
MString MCalibrationSpectralPoint::ToParsableString(const MString& Mode, bool WithDescriptor)
{
  ostringstream out;

  if (Mode == "param" || Mode == "param+error") {
    if (m_Fit != nullptr) {
      out<<m_Fit->ToParsableString(Mode, WithDescriptor);
    }
  } else {
    throw MExceptionUnknownMode(Mode);
    return "";
  }

  return out.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Stream the content
ostream& operator<<(ostream& os, const MCalibrationSpectralPoint& P)
{
  os<<P.ToString();
  return os;
}


// MCalibrationSpectralPoint.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
