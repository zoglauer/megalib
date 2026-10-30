/*
 * MREAM.cxx
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
// MREAM
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREAM.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MREAM)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MREAM::c_Unknown = 0;
const unsigned int MREAM::c_DriftChamberEnergy = 1;
const unsigned int MREAM::c_GuardRingHit = 2;
const unsigned int MREAM::c_Directional = 3;
const unsigned int MREAM::c_StartInformation = 4;
const unsigned int MREAM::c_Min = 1;
const unsigned int MREAM::c_Max = 4;


////////////////////////////////////////////////////////////////////////////////


MREAM::MREAM()
{
  // Construct an instance of MREAM

  m_Type = c_Unknown;
  m_VolumeSequence = 0;
}


////////////////////////////////////////////////////////////////////////////////


MREAM::MREAM(const MREAM& REAM)
{
  // Construct an instance of MREAM

  m_Type = REAM.m_Type;
  m_VolumeSequence = new MDVolumeSequence(*(REAM.m_VolumeSequence));
}


////////////////////////////////////////////////////////////////////////////////


MREAM::~MREAM()
{
  // Delete this instance of MREAM

  delete m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


const MREAM& MREAM::operator=(const MREAM& REAM)
{
  // Assign all values of the left hand side hit to this one 

  if (this != &REAM) {
    m_Type = REAM.m_Type;
    m_VolumeSequence = new MDVolumeSequence(*(REAM.m_VolumeSequence));
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MREAM* MREAM::Clone()
{
  // Duplicate this REAM

  massert(this != 0);
  return new MREAM(*this);
}


////////////////////////////////////////////////////////////////////////////////


MString MREAM::ToString(int Level) 
{
  // Dump the essential content of this REAM

  // First determine the level:
  MString Space("");
  for (int i = 0; i < Level; i++) {
    Space += MString("   ");
  }

  ostringstream out;

  out<<Space<<"Basic REAM of Type "<<m_Type<<endl;

  return out.str().c_str();
}


// MREAM.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
