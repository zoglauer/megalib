/*
 * MPhysicalEventHit.cxx
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
