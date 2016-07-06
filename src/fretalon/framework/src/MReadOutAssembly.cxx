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


MReadOutAssembly::MReadOutAssembly() : MReadOutSequence()
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


bool MReadOutAssembly::Parse(MString& Line, int Version)
{
  // Returns true if something has been read (sucessful or not)
  
  if (MReadOutSequence::Parse(Line) == true) return true;
  
  if (Line.BeginsWith("BD")) {
    // set a bad flag
    // too lazy RN to go thru each flag.  the following should do::
    m_FilteredOut = true;
    return true;
  }

  return false;
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
  
  for (MReadOut& RO: m_ReadOuts) {
    S<<RO.ToParsableString(WithDescriptor)<<endl;
  }
}
  

// MReadOutAssembly.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
