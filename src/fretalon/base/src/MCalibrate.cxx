/*
 * MCalibrate.cxx
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
#include "MCalibrate.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MReadOutDataInterfaceADCValue.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrate)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrate::MCalibrate() : m_DiagnosticsMode(false)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrate::~MCalibrate()
{
}


// MCalibrate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
