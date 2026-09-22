/*
 * MDVector.cxx
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
// MDVector
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDVector.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDVector)
#endif


////////////////////////////////////////////////////////////////////////////////


MDVector::MDVector(const MString Name): m_Name(Name)
{
  // Construct an instance of MDVector
}


////////////////////////////////////////////////////////////////////////////////


MDVector::~MDVector()
{
  // Delete this instance of MDVector
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVector::GetPosition(unsigned int i) 
{
  //! Return the position at pos i - g_VectorNotDefined if out of bounds

  if (i < m_Positions.size()) {
    return m_Positions[i];
  }

  return g_VectorNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


double MDVector::GetValue(unsigned int i)
{
  //! Return the value at pos i - g_DoubleNotDefined id out of bounds

  if (i < m_Values.size()) {
    return m_Values[i];
  }
  
  return g_DoubleNotDefined;
}


// MDVector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
