/*
 * MReadOutDataADCValueWithTiming.cxx
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
// MReadOutDataADCValueWithTiming
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataADCValueWithTiming.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutDataADCValueWithTiming)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataADCValueWithTiming::MReadOutDataADCValueWithTiming() : MReadOutData(), MReadOutDataInterfaceADCValue(0), MReadOutDataInterfaceTiming(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataADCValueWithTiming::MReadOutDataADCValueWithTiming(double Data, double Timing) : MReadOutData(), MReadOutDataInterfaceADCValue(Data), MReadOutDataInterfaceTiming(Timing)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataADCValueWithTiming::~MReadOutDataADCValueWithTiming()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out data is of the given type
bool MReadOutDataADCValueWithTiming::IsOfType(const MString& String) const
{ 
  if (String == "adcwithtiming") return true;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the type
MString MReadOutDataADCValueWithTiming::GetType() const
{
  return "adcwithtiming";
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataADCValueWithTiming* MReadOutDataADCValueWithTiming::Clone() const
{
  return new MReadOutDataADCValueWithTiming(m_ADCValue, m_Timing);
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataADCValueWithTiming::Clear()
{
  MReadOutDataInterfaceADCValue::Clear();
  MReadOutDataInterfaceTiming::Clear();
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataADCValueWithTiming::GetNumberOfParsableElements() const
{
  return MReadOutDataInterfaceADCValue::GetNumberOfParsableElements() + MReadOutDataInterfaceTiming::GetNumberOfParsableElements(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataADCValueWithTiming::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) {
    merr<<GetType()<<": Not enough elements to parse"<<show;
    return false;
  }
   
  if (MReadOutDataInterfaceADCValue::Parse(T, StartElement) == false) return false;
  StartElement += MReadOutDataInterfaceADCValue::GetNumberOfParsableElements();
  if (MReadOutDataInterfaceTiming::Parse(T, StartElement) == false) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataADCValueWithTiming::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString()<<" "<<MReadOutDataInterfaceADCValue::ToString()<<" "<<MReadOutDataInterfaceTiming::ToString();
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataADCValueWithTiming::ToParsableString(bool WithDescriptor) const 
{
  ostringstream os;
  if (WithDescriptor == true) {
    os<<GetType()<<" ";
  }
  os<<MReadOutDataInterfaceADCValue::ToParsableString()<<" "<<MReadOutDataInterfaceTiming::ToParsableString();
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutDataADCValueWithTiming& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataADCValueWithTiming.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
