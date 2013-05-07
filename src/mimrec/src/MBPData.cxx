/*
 * MBPData.cxx
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


#ifdef ___CINT___
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
