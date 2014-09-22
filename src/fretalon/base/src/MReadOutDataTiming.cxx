/*
 * MReadOutDataTiming.cxx
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
// MReadOutDataTiming
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataTiming.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutDataTiming)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The type name --- must be unique
const MString MReadOutDataTiming::m_Type = "timing";
//! The type name ID --- must be unique
const long MReadOutDataTiming::m_TypeID = m_Type.GetHash();


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataTiming::MReadOutDataTiming() : MReadOutData(nullptr), m_Timing(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataTiming::MReadOutDataTiming(MReadOutData* Data) : MReadOutData(Data), m_Timing(0)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataTiming::~MReadOutDataTiming()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataTiming* MReadOutDataTiming::Clone() const
{
  MReadOutDataTiming* ROD = new MReadOutDataTiming();
  ROD->SetTiming(m_Timing);
  if (m_Wrapped != 0) {
    ROD->SetWrapped(m_Wrapped->Clone());
  }
  return ROD;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataTiming::Clear()
{
  MReadOutData::Clear();
  m_Timing = 0;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataTiming::GetNumberOfParsableElements() const
{
  return MReadOutData::GetNumberOfParsableElements() + 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataTiming::Parse(const MTokenizer& T, unsigned int StartElement)
{
  // Go deep first:
  if (MReadOutData::Parse(T, StartElement) == false) return false;
  
  // Then here:
  m_Timing = T.GetTokenAtAsUnsignedIntFast(StartElement + MReadOutData::GetNumberOfParsableElements());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataTiming::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString()<<m_Timing<<" ";
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataTiming::ToParsableString(bool WithDescriptor) const 
{
  ostringstream os;
  if (WithDescriptor == true) {
    os<<GetCombinedType()<<" ";
  }
  os<<ToString();
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutDataTiming& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataTiming.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
