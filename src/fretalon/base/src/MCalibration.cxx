/*
 * MCalibration.cxx
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
#include "MCalibration.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibration::MCalibration()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibration::~MCalibration()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this calibration - the returned calibration must be deleted!
MCalibration* MCalibration::Clone() const
{
  MCalibration* C = new MCalibration();
  C->m_IsCalibrated = m_IsCalibrated;
  
  return C;
}


// MCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
