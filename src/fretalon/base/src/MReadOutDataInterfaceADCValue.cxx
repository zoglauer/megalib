/*
 * MReadOutDataInterfaceADCValue.cxx
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
// MReadOutDataInterfaceADCValue
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataInterfaceADCValue.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutDataInterfaceADCValue)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataInterfaceADCValue::MReadOutDataInterfaceADCValue() : m_ADCValue(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataInterfaceADCValue::MReadOutDataInterfaceADCValue(double ADCValue) : m_ADCValue(ADCValue)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataInterfaceADCValue::~MReadOutDataInterfaceADCValue()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataInterfaceADCValue::Clear()
{
  m_ADCValue = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true is all values are zero
bool MReadOutDataInterfaceADCValue::IsZero() const
{
  return ((m_ADCValue == 0) ? true : false);
}


////////////////////////////////////////////////////////////////////////////////


//! Return true is the value is positive
bool MReadOutDataInterfaceADCValue::IsPositive() const
{
  return ((m_ADCValue > 0) ? true : false);
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataInterfaceADCValue::GetNumberOfParsableElements() const
{
  return 1; 
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataInterfaceADCValue::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + MReadOutDataInterfaceADCValue::GetNumberOfParsableElements()) {
    merr<<"adc: Not enough elements to parse"<<show;
    return false;
  }
  
  m_ADCValue = T.GetTokenAtAsDouble(StartElement);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataInterfaceADCValue::ToString() const
{
  ostringstream os;
  os<<"ADC = "<<m_ADCValue;
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataInterfaceADCValue::ToParsableString() const
{
  ostringstream os;
  os<<m_ADCValue;
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutDataInterfaceADCValue& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataInterfaceADCValue.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
