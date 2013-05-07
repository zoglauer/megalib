/*
 * MREAMDriftChamberEnergy.cxx
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


#ifdef ___CINT___
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
