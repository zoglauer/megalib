/*
 * MREAMDriftChamberEnergy.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MREAMDriftChamberEnergy
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREAMDriftChamberEnergy.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MREAMDriftChamberEnergy)
#endif


////////////////////////////////////////////////////////////////////////////////


MREAMDriftChamberEnergy::MREAMDriftChamberEnergy() : MREAM()
{
  // Construct an instance of MREAMDriftChamberEnergy

  m_Type = c_DriftChamberEnergy;
  
  m_Energy = g_DoubleNotDefined;
  m_EnergyResolution = g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MREAMDriftChamberEnergy::MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy& REAM)
{
  // Construct an instance of MREAMDriftChamberEnergy

  m_Type = REAM.m_Type;
  m_VolumeSequence = new MDVolumeSequence(*(REAM.m_VolumeSequence));
  m_Energy = REAM.m_Energy;
  m_EnergyResolution = REAM.m_EnergyResolution;
}


////////////////////////////////////////////////////////////////////////////////


MREAMDriftChamberEnergy::~MREAMDriftChamberEnergy()
{
  // Delete this instance of MREAMDriftChamberEnergy
}


////////////////////////////////////////////////////////////////////////////////


MREAM* MREAMDriftChamberEnergy::Clone()
{
  // Duplicate this REAM

  massert(this != 0);
  return new MREAMDriftChamberEnergy(*this);
}


// MREAMDriftChamberEnergy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
