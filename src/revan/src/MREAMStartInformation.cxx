/*
 * MREAMStartInformation.cxx
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
// MREAMStartInformation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREAMStartInformation.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MREAMStartInformation)
#endif


////////////////////////////////////////////////////////////////////////////////


MREAMStartInformation::MREAMStartInformation() : MREAM()
{
  // Construct an instance of MREAMStartInformation

  m_Type = c_StartInformation;

  m_Position = MVector(0.0, 0.0, 0.0);
  m_Direction = MVector(0.0, 0.0, 0.0);
  m_Polarization = MVector(0.0, 0.0, 0.0);
  m_Energy = 0.0;
}


////////////////////////////////////////////////////////////////////////////////


MREAMStartInformation::MREAMStartInformation(const MREAMStartInformation& REAM)
{
  // Construct an instance of MREAMStartInformation

  m_Type = REAM.m_Type;

  m_Position = REAM.m_Position;
  m_Direction = REAM.m_Direction;
  m_Polarization = REAM.m_Polarization;
  m_Energy = REAM.m_Energy;
}


////////////////////////////////////////////////////////////////////////////////


MREAMStartInformation::~MREAMStartInformation()
{
  // Delete this instance of MREAMStartInformation
}


////////////////////////////////////////////////////////////////////////////////


MREAM* MREAMStartInformation::Clone()
{
  // Duplicate this REAM

  massert(this != 0);
  return new MREAMStartInformation(*this);
}


////////////////////////////////////////////////////////////////////////////////


MString MREAMStartInformation::ToString(int Level) 
{
  // Dump the essential content of this REAM

  // First determine the level:
  MString Space("");
  for (int i = 0; i < Level; i++) {
    Space += MString("   ");
  }

  ostringstream out;

  out<<Space<<"REAMStartInformation: original position="<<m_Position<<", original direction="<<m_Direction<<endl;

  return out.str().c_str();
}


// MREAMStartInformation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
