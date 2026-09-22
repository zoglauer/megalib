/*
 * MBPData.cxx
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
// MBPData.cxx
//
//
// This is the base class for all backprojected events:
// It describes only the type of event:
//
//  1:  Compton event data
//  5:  Pair event data
//
// 10:  image
// 11:  sparse image
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBPData.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBPData)
#endif


////////////////////////////////////////////////////////////////////////////////


MBPData::MBPData()
{
  // standard constructor

  m_Type = c_Unknown;
}


////////////////////////////////////////////////////////////////////////////////


MBPData::~MBPData()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


int MBPData::GetUsedBytes() const
{
  // Return the number of bytes used by this image
  
  int Bytes = 0;

  Bytes += sizeof(void*); // Pointer to this
  Bytes += sizeof(int); // m_Type

  return Bytes;
}


// MBPData: the end...
////////////////////////////////////////////////////////////////////////////////
