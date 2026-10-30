/*
 * MCRegion.cc
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


// Cosima:
#include "MCRegion.hh"

// MEGAlib:
#include "MGlobal.h"

// Geant4:

// Standard lib:


/******************************************************************************
 * Standard constructor
 */
MCRegion::MCRegion(const MString& Name) : 
  m_Name(Name), m_VolumeName(g_StringNotDefined), m_RangeCut(g_DoubleNotDefined), m_CutAllAllSecondaries(false)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCRegion::~MCRegion()
{
  // Intentionally left blank
}


/*
 * MCRegion.cc: the end...
 ******************************************************************************/
