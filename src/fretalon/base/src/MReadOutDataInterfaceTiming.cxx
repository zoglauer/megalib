/*
 * MReadOutDataInterfaceTiming.cxx
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
// MReadOutDataInterfaceTiming
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataInterfaceTiming.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutDataInterfaceTiming)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataInterfaceTiming::MReadOutDataInterfaceTiming() : m_Timing(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataInterfaceTiming::MReadOutDataInterfaceTiming(double Timing) : m_Timing(Timing)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataInterfaceTiming::~MReadOutDataInterfaceTiming()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataInterfaceTiming::Clear()
{
  m_Timing = 0;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataInterfaceTiming::GetNumberOfParsableElements() const
{
  return 1; 
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataInterfaceTiming::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + MReadOutDataInterfaceTiming::GetNumberOfParsableElements()) {
    merr<<"timing: Not enough elements to parse"<<show;
    return false;
  }
  
  m_Timing = T.GetTokenAtAsDouble(StartElement);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataInterfaceTiming::ToString() const
{
  ostringstream os;
  os<<"Timing = "<<m_Timing;
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutDataInterfaceTiming& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataInterfaceTiming.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
