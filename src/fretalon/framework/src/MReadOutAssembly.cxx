/*
 * MReadOutAssembly.cxx
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
// MReadOutAssembly
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutAssembly.h"

// Standard libs:
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutAssembly)*m_ReadOuts[i]
#endif


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly::MReadOutAssembly()
{
  // Construct an instance of MReadOutAssembly
  
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly::~MReadOutAssembly()
{
  // Nothing to do yet
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::Clear()
{
  //! Reset all data

  m_ID = g_UnsignedIntNotDefined;
  m_Time = 0;
}


////////////////////////////////////////////////////////////////////////////////


MReadOut& MReadOutAssembly::GetReadOut(unsigned int i) 
{ 
  //! Return read out i

  if (i < m_ReadOuts.size()) {
    return *m_ReadOuts[i];
  }
  
  throw MExceptionIndexOutOfBounds(0, m_ReadOuts.size(), i);

  return *m_ReadOuts.at(0); // never reached so should never crash...
}

////////////////////////////////////////////////////////////////////////////////

void MReadOutAssembly::AddReadOut(MReadOut& ReadOut)
{
  //! Add a read out
  
  return m_ReadOuts.push_back(new MReadOut(ReadOut));
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveReadOut(unsigned int i)
{
  //! Remove a read out
  
  if (i < m_ReadOuts.size()) {
    vector<MReadOut*>::iterator it;
    it = m_ReadOuts.begin()+i;
    m_ReadOuts.erase(it);
    delete (*it);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamEvta(ostream& S)
{
  //! Stream the content in MEGAlib's evta format 

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;
  S<<"TI "<<m_Time<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamRoa(ostream& S, bool WithDescriptor)
{
  //! Stream the content in MEGAlib's roa format 

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;
  S<<"TI "<<m_Time<<endl;
  
  for (auto RO: m_ReadOuts) {
    S<<RO->ToParsableString(WithDescriptor)<<endl;
  }
}
  

// MReadOutAssembly.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
