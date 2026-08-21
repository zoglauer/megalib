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


#ifdef ___CLING___
ClassImp(MReadOutAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


atomic<unsigned long> MReadOutAssembly::s_NextAssemblyID(0);


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly::MReadOutAssembly() : MReadOutSequence(), m_AssemblyID(++s_NextAssemblyID)
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

  // Reset the inherited sequence data -- this also drops the stored read-outs and sim IAs, so a
  // reused assembly does not accumulate them across loads
  MReadOutSequence::Clear();

  // The assembly reports an undefined ID rather than the base class' zero
  m_ID = g_UnsignedIntNotDefined;
  m_AnalysisProgress = 0;
  m_FilteredOut = false;
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::Parse(MString& Line, int Version)
{
  // Returns true if something has been read (sucessful or not)

  // The assembly-specific keywords have to be checked first: the base sequence parser is tolerant
  // and consumes every line including unrecognized ones, so anything checked after it is unreachable

  // A BD line lists one or more bad-event flag names ("BD <flag> <flag> ..."). The assembly tracks
  // only a single filtered-out state rather than the individual flags, so any bad-data marker
  // removes the whole assembly from the analysis.
  if (Line.BeginsWith("BD")) {
    m_FilteredOut = true;
    return true;
  }

  return MReadOutSequence::Parse(Line, Version);
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
