/*
 * MCRegion.cxx
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
  m_Name(Name), m_VolumeName(g_StringNotDefined), m_RangeCut(g_DoubleNotDefined)
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
