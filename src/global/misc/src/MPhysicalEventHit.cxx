/*
 * MPhysicalEventHit.cxx
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
#include "MPhysicalEventHit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MPhysicalEventHit)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MPhysicalEventHit::MPhysicalEventHit() : m_Position(0, 0, 0), m_PositionUncertainty(0, 0, 0), m_Energy(0), m_EnergyUncertainty(0), m_Time(0), m_TimeUncertainty(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MPhysicalEventHit::~MPhysicalEventHit()
{
}


// MPhysicalEventHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
