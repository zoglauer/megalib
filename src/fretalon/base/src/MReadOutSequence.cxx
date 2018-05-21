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


#ifdef ___CLING___
ClassImp(MReadOutSequence)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutSequence::MReadOutSequence() : m_ID(0), m_Time(0)
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
  m_ID = 0;
  m_Time = 0;
  m_ReadOuts.clear();
  m_SimIAs.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Get a specific read out
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


//! Remove a read out
void MReadOutSequence::RemoveReadOut(unsigned int i)
{
  if (i < m_ReadOuts.size()) {
    vector<MReadOut>::iterator it;
    it = m_ReadOuts.begin()+i;
    m_ReadOuts.erase(it);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if all read-out elements are of the same type
bool MReadOutSequence::HasIdenticalReadOutElementTypes() const
{
  if (m_ReadOuts.size() == 0) return false;
  if (m_ReadOuts.size() == 1) return true;
  
  for (unsigned int i = 1; i < m_ReadOuts.size(); ++i) {
    if (m_ReadOuts[i].GetReadOutElement().AreOfSameType(m_ReadOuts[0].GetReadOutElement()) == false) {
      return false;
    }
  }
  
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


//! Return simulated interaction i
const MSimIA& MReadOutSequence::GetSimIA(unsigned int i) const
{   
  if (i < m_SimIAs.size()) return m_SimIAs[i];
  
  throw MExceptionIndexOutOfBounds(0, m_SimIAs.size(), i);
  
  // We still need a return value...
  return m_SimIAs[0];
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a line - return true if the line was processed (irrelevant if successful)
bool MReadOutSequence::Parse(MString& Line, int Version)
{
  if (Line.BeginsWith("TI")) {
    m_Time.Set(Line);
    return true;
  }
      
  if (Line.BeginsWith("ID")) {
    unsigned long ID = 0;
    if (sscanf(Line.Data(), "ID %lu\n", &ID) == 1) {
      m_ID = ID;
    } else {
      mout<<"MReadOutSequence: Error parsing ID"<<endl;
    }
    return true;
  }
   
  if (Line.BeginsWith("IA")) {
    MSimIA IA;
    if (IA.AddRawInput(Line, 25) == true) {
      m_SimIAs.push_back(IA);
    } else {
      mout<<"MReadOutSequence: Error during parsing IA"<<endl;
    }
    return true;
  }

  return true;
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
