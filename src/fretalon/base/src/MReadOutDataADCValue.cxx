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


#ifdef ___CINT___
ClassImp(MReadOutDataADCValue)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataADCValue::MReadOutDataADCValue() : MReadOutData(), MReadOutDataInterfaceADCValue(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataADCValue::MReadOutDataADCValue(double Data) : MReadOutData(), MReadOutDataInterfaceADCValue(Data)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataADCValue::~MReadOutDataADCValue()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out element is of the given type
bool MReadOutDataADCValue::IsOfType(const MString& String) const
{ 
  if (String == "adc" || String == "data") return true;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the type
MString MReadOutDataADCValue::GetType() const
{
  return "adc";
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataADCValue* MReadOutDataADCValue::Clone() const
{
  return new MReadOutDataADCValue(m_ADCValue);
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataADCValue::Clear()
{
  MReadOutDataInterfaceADCValue::Clear();
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataADCValue::GetNumberOfParsableElements() const
{
  return MReadOutDataInterfaceADCValue::GetNumberOfParsableElements();
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataADCValue::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) {
    merr<<GetType()<<": Not enough elements to parse"<<show;
    return false;
  }
  
  if (MReadOutDataInterfaceADCValue::Parse(T, StartElement) == false) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataADCValue::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString()<<" "<<MReadOutDataInterfaceADCValue::ToString();
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataADCValue::ToParsableString(bool WithDescriptor) const 
{
  ostringstream os;
  if (WithDescriptor == true) {
    os<<GetType()<<" ";
  }
  os<<MReadOutDataInterfaceADCValue::ToParsableString();
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
