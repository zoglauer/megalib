/*
 * MReadOutSequence.cxx
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
// MReadOutSequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutSequence.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutSequence)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutSequence::MReadOutSequence()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor 
MReadOutSequence::~MReadOutSequence()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Remove all content
void MReadOutSequence::Clear()
{
  m_ReadOuts.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Get a specific read-outs
const MReadOut& MReadOutSequence::GetReadOut(unsigned int R) const
{
  if (R < m_ReadOuts.size()) return m_ReadOuts[R];
  
  throw MExceptionIndexOutOfBounds(0, m_ReadOuts.size(), R);
  
  // We still need a return value...
  return m_ReadOuts[0];
}


////////////////////////////////////////////////////////////////////////////////


//! Find a read out with the given read-out element and return its position
//! Return g_UnsignedIntNotDefined in case it is not found
unsigned int MReadOutSequence::FindReadOut(const MReadOutElement& ROE) const
{
  for (unsigned int i = 0; i < m_ReadOuts.size(); ++i) {
    if (m_ReadOuts[i].GetReadOutElement() == ROE) {
      return i; 
    }
  }
  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a string
MString MReadOutSequence::ToString() const
{
  ostringstream os;
  os<<"RO Sequence - number of read outs = "<<m_ReadOuts.size();
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutSequence& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutSequence.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
