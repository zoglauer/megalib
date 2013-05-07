/*
 * MREAMStartInformation.cxx
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
// MREAMStartInformation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREAMStartInformation.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MREAMStartInformation)
#endif


////////////////////////////////////////////////////////////////////////////////


MREAMStartInformation::MREAMStartInformation() : MREAM()
{
  // Construct an instance of MREAMStartInformation

  m_Type = c_StartInformation;

  m_Position = MVector(0.0, 0.0, 0.0);
  m_Direction = MVector(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


MREAMStartInformation::MREAMStartInformation(const MREAMStartInformation& REAM)
{
  // Construct an instance of MREAMStartInformation

  m_Type = REAM.m_Type;

  m_Position = REAM.m_Position;
  m_Direction = REAM.m_Direction;
}


////////////////////////////////////////////////////////////////////////////////


MREAMStartInformation::~MREAMStartInformation()
{
  // Delete this instance of MREAMStartInformation
}


////////////////////////////////////////////////////////////////////////////////


MREAM* MREAMStartInformation::Clone()
{
  // Duplicate this REAM

  massert(this != 0);
  return new MREAMStartInformation(*this);
}


////////////////////////////////////////////////////////////////////////////////


MString MREAMStartInformation::ToString(int Level) 
{
  // Dump the essential content of this REAM

  // First determine the level:
  MString Space("");
  for (int i = 0; i < Level; i++) {
    Space += MString("   ");
  }

  ostringstream out;

  out<<Space<<"REAMStartInformation: original position="<<m_Position<<", original direction="<<m_Direction<<endl;

  return out.str().c_str();
}


// MREAMStartInformation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
