/*
 * MREAM.cxx
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
// MREAM
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREAM.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MREAM)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MREAM::c_Unknown = 0;
const unsigned int MREAM::c_DriftChamberEnergy = 1;
const unsigned int MREAM::c_GuardringHit = 2;
const unsigned int MREAM::c_Directional = 3;
const unsigned int MREAM::c_StartInformation = 4;
const unsigned int MREAM::c_Min = 1;
const unsigned int MREAM::c_Max = 4;


////////////////////////////////////////////////////////////////////////////////


MREAM::MREAM()
{
  // Construct an instance of MREAM

  m_Type = c_Unknown;
  m_VolumeSequence = 0;
}


////////////////////////////////////////////////////////////////////////////////


MREAM::MREAM(const MREAM& REAM)
{
  // Construct an instance of MREAM

  m_Type = REAM.m_Type;
  m_VolumeSequence = new MDVolumeSequence(*(REAM.m_VolumeSequence));
}


////////////////////////////////////////////////////////////////////////////////


MREAM::~MREAM()
{
  // Delete this instance of MREAM

  delete m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


const MREAM& MREAM::operator=(const MREAM& REAM)
{
  // Assign all values of the left hand side hit to this one 

  if (this != &REAM) {
    m_Type = REAM.m_Type;
    m_VolumeSequence = new MDVolumeSequence(*(REAM.m_VolumeSequence));
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MREAM* MREAM::Clone()
{
  // Duplicate this REAM

  massert(this != 0);
  return new MREAM(*this);
}


////////////////////////////////////////////////////////////////////////////////


MString MREAM::ToString(int Level) 
{
  // Dump the essential content of this REAM

  // First determine the level:
  MString Space("");
  for (int i = 0; i < Level; i++) {
    Space += MString("   ");
  }

  ostringstream out;

  out<<Space<<"Basic REAM of Type "<<m_Type<<endl;

  return out.str().c_str();
}


// MREAM.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
