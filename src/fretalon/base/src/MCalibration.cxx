/*
 * MCalibration.cxx
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
