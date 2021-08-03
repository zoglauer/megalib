/*
 * MSimBinaryOptions.cxx
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
#include "MSimBinaryOptions.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSimBinaryOptions)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MSimBinaryOptions::MSimBinaryOptions() : m_Data(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MSimBinaryOptions::MSimBinaryOptions(uint32_t Data) : m_Data(Data)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MSimBinaryOptions::~MSimBinaryOptions()
{
}


// MSimBinaryOptions.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
