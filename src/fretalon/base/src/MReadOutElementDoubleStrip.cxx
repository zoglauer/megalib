/*
 * MReadOutElementDoubleStrip.cxx
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
// MReadOutElementDoubleStrip
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutElementDoubleStrip.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutElementDoubleStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutElementDoubleStrip::MReadOutElementDoubleStrip(): MReadOutElementStrip(), m_IsLowVoltageStrip(true)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Read out element of a DOUBLE-SIDED strip detector  
MReadOutElementDoubleStrip::MReadOutElementDoubleStrip(unsigned int DetectorID, unsigned int StripID, bool IsLowVoltageStrip) : MReadOutElementStrip(DetectorID, StripID), m_IsLowVoltageStrip(IsLowVoltageStrip)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutElementDoubleStrip::~MReadOutElementDoubleStrip()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this read-out element - the returned element must be deleted!
MReadOutElementDoubleStrip* MReadOutElementDoubleStrip::Clone() const
{
  MReadOutElementDoubleStrip* R = new MReadOutElementDoubleStrip(m_DetectorID, m_StripID, m_IsLowVoltageStrip);
  return R;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutElementDoubleStrip::Clear()
{
  MReadOutElementStrip::Clear();
  m_IsLowVoltageStrip = true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out element is of the given type
bool MReadOutElementDoubleStrip::IsOfType(const MString& String) const
{ 
  if (String == "doublesidedstrip") return true;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the type of this read-out element
MString MReadOutElementDoubleStrip::GetType() const
{
  return "doublesidedstrip";
}


////////////////////////////////////////////////////////////////////////////////


//! Test for equality
bool MReadOutElementDoubleStrip::operator==(const MReadOutElement& R) const
{
  const MReadOutElementDoubleStrip* S = dynamic_cast<const MReadOutElementDoubleStrip*>(&R);
  if (S == 0) return false;
  
  if (m_StripID != S->m_StripID) return false;
  if (m_DetectorID != S->m_DetectorID) return false;
  if (m_IsLowVoltageStrip != S->m_IsLowVoltageStrip) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Smaller than operator
bool MReadOutElementDoubleStrip::operator<(const MReadOutElement& R) const
{
  const MReadOutElementDoubleStrip* S = dynamic_cast<const MReadOutElementDoubleStrip*>(&R);
  if (S == 0) return false;
  
  if (m_DetectorID < S->m_DetectorID) return true;
  if (m_DetectorID == S->m_DetectorID) {
    if (m_StripID < S->m_StripID) return true;
    if (m_StripID == S->m_StripID) {
      if (m_IsLowVoltageStrip == false && S->m_IsLowVoltageStrip == true) return true;
    }  
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutElementDoubleStrip::GetNumberOfParsableElements() const
{
  return 3; 
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutElementDoubleStrip::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) {
    merr<<GetType()<<": Not enough elements to parse"<<show;
    return false;
  }
  
  m_DetectorID = T.GetTokenAtAsUnsignedIntFast(StartElement);
  m_StripID = T.GetTokenAtAsUnsignedIntFast(StartElement+1);
  m_IsLowVoltageStrip = (T.GetTokenAt(StartElement+2) == "l" || T.GetTokenAt(StartElement+2) == "p") ? true : false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the data as parsable string
MString MReadOutElementDoubleStrip::ToParsableString(bool WithDescriptor) const
{
  MString Return;
  if (WithDescriptor == true) {
    Return += "dss "; 
  }
  Return += m_DetectorID;
  Return += " ";
  Return += m_StripID;
  Return += " ";
  Return += (m_IsLowVoltageStrip ? "l" : "h");

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a string
MString MReadOutElementDoubleStrip::ToString() const
{
  ostringstream os;
  os<<"Detector: "<<m_DetectorID<<", side: "<<((m_IsLowVoltageStrip == true) ? "LV" : "HV")<<", strip: "<<m_StripID;
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Stream the content
ostream& operator<<(ostream& os, const MReadOutElementDoubleStrip& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutElementDoubleStrip.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
