/*
 * MERConstruction.cxx
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
// MERConstruction
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERConstruction.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERConstruction)
#endif


////////////////////////////////////////////////////////////////////////////////


MERConstruction::MERConstruction()
{
  // Construct an instance of MERConstruction

  m_List = 0;
  m_Geometry = 0;
}


////////////////////////////////////////////////////////////////////////////////


MERConstruction::~MERConstruction()
{
  // Delete this instance of MERConstruction

  m_List = 0; // do not delete
  m_Geometry = 0; // do not delete
}


////////////////////////////////////////////////////////////////////////////////


bool MERConstruction::Analyze(MRawEventIncarnations* List)
{
  // Analyze the raw event...
  m_List = List;

  ModifyEventList();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MERConstruction::ModifyEventList()
{
  // Modification routine at the beginning of the analysis to add/modify 
  // the events in the list before the analysis

  return;
}


// MERConstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
