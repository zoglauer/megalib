/*
 * MREAMDirectional.cxx
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
// MREAMDirectional
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREAMDirectional.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MREAMDirectional)
#endif


////////////////////////////////////////////////////////////////////////////////


MREAMDirectional::MREAMDirectional() : MREAM()
{
  // Construct an instance of MREAMDirectional

  m_Type = c_Directional;
  
  m_Direction = g_VectorNotDefined;

  m_RESE = 0;
}


////////////////////////////////////////////////////////////////////////////////


MREAMDirectional::MREAMDirectional(const MREAMDirectional& REAM)
{
  // Construct an instance of MREAMDirectional

  m_Type = REAM.m_Type;
  m_VolumeSequence = new MDVolumeSequence(*(REAM.m_VolumeSequence));
  m_Direction = REAM.m_Direction;
  m_RESE = REAM.m_RESE;
}


////////////////////////////////////////////////////////////////////////////////


MREAMDirectional::~MREAMDirectional()
{
  // Delete this instance of MREAMDirectional
}


////////////////////////////////////////////////////////////////////////////////


MREAM* MREAMDirectional::Clone()
{
  // Duplicate this REAM

  massert(this != 0);
  return new MREAMDirectional(*this);
}


////////////////////////////////////////////////////////////////////////////////


MString MREAMDirectional::ToString(int Level) 
{
  // Dump the essential content of this REAM

  // First determine the level:
  MString Space("");
  for (int i = 0; i < Level; i++) {
    Space += MString("   ");
  }

  ostringstream out;

  out<<Space<<"REAMDirectional: Dir:"<<m_Direction<<", Energy:"<<m_Energy<<", RESE-ID:";
  if (m_RESE != 0) {
    out<<m_RESE->GetID()<<endl;
  } else {
    out<<"none"<<endl;
  }

  return out.str().c_str();
}


// MREAMDirectional.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
