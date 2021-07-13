/*
 * MReadOutDataADCValue.cxx
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
// MReadOutDataADCValue
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataADCValue.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutDataADCValue)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The type name --- must be unique
const MString MReadOutDataADCValue::m_Type = "adc";
//! The type name ID --- must be unique
const long MReadOutDataADCValue::m_TypeID = m_Type.GetHash();


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataADCValue::MReadOutDataADCValue() : MReadOutData(nullptr), m_ADCValue(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataADCValue::MReadOutDataADCValue(MReadOutData* Data) : MReadOutData(Data), m_ADCValue(0)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataADCValue::~MReadOutDataADCValue()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataADCValue::Clear()
{
  MReadOutData::Clear();
  m_ADCValue = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true is all values are zero
bool MReadOutDataADCValue::IsZero() const
{
  return ((m_ADCValue == 0) ? true : false);
}


////////////////////////////////////////////////////////////////////////////////


//! Return true is the value is positive
bool MReadOutDataADCValue::IsPositive() const
{
  return ((m_ADCValue > 0) ? true : false);
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataADCValue* MReadOutDataADCValue::Clone() const
{
  MReadOutDataADCValue* ROD = new MReadOutDataADCValue();
  ROD->SetADCValue(m_ADCValue);
  if (m_Wrapped != 0) {
    ROD->SetWrapped(m_Wrapped->Clone());
  }
  return ROD;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataADCValue::GetNumberOfParsableElements() const
{
  return MReadOutData::GetNumberOfParsableElements() + 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataADCValue::Parse(const MTokenizer& T, unsigned int StartElement)
{
  // Go deep first:
  if (MReadOutData::Parse(T, StartElement) == false) return false;
  
  // Then here:
  m_ADCValue = T.GetTokenAtAsUnsignedIntFast(StartElement + MReadOutData::GetNumberOfParsableElements());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataADCValue::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString()<<m_ADCValue<<" ";
  
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataADCValue::ToParsableString(bool WithDescriptor) const 
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
ostream& operator<<(ostream& os, const MReadOutDataADCValue& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataADCValue.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
