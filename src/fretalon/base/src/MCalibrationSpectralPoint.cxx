/*
 * MCalibrationSpectralPoint.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
  if (m_Fit == 0) {
    throw MExceptionObjectDoesNotExist("Fit does not exist!");
  }
  
  return *m_Fit; 
}


////////////////////////////////////////////////////////////////////////////////


//! Get the fit, if it doesn't exist throw MExceptionObjectDoesNotExist
//! Bad design since a new fit can be set any time...
MCalibrationFit& MCalibrationSpectralPoint::GetFit() 
{ 
  if (m_Fit == 0) {
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


//! Stream the content
ostream& operator<<(ostream& os, const MCalibrationSpectralPoint& P)
{
  os<<P.ToString();
  return os;
}


// MCalibrationSpectralPoint.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
