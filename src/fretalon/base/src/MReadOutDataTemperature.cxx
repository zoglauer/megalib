/*
 * MReadOutDataTemperature.cxx
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
// MReadOutDataTemperature
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataTemperature.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutDataTemperature)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The type name --- must be unique
const MString MReadOutDataTemperature::m_Type = "temperature";
//! The type name ID --- must be unique
const long MReadOutDataTemperature::m_TypeID = m_Type.GetHash();


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataTemperature::MReadOutDataTemperature() : MReadOutData(nullptr), m_Temperature(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataTemperature::MReadOutDataTemperature(MReadOutData* Data) : MReadOutData(Data), m_Temperature(0)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataTemperature::~MReadOutDataTemperature()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataTemperature* MReadOutDataTemperature::Clone() const
{
  MReadOutDataTemperature* ROD = new MReadOutDataTemperature();
  ROD->SetTemperature(m_Temperature);
  if (m_Wrapped != 0) {
    ROD->SetWrapped(m_Wrapped->Clone());
  }
  return ROD;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataTemperature::Clear()
{
  MReadOutData::Clear();
  m_Temperature = 0;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataTemperature::GetNumberOfParsableElements() const
{
  return MReadOutData::GetNumberOfParsableElements() + 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataTemperature::Parse(const MTokenizer& T, unsigned int StartElement)
{
  // Go deep first:
  if (MReadOutData::Parse(T, StartElement) == false) return false;
  
  // Then here:
  m_Temperature = T.GetTokenAtAsDouble(StartElement + MReadOutData::GetNumberOfParsableElements());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataTemperature::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString()<<m_Temperature<<" ";
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataTemperature::ToParsableString(bool WithDescriptor) const 
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
ostream& operator<<(ostream& os, const MReadOutDataTemperature& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataTemperature.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
