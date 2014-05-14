/*
 * MReadOutElementStrip.cxx
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
// MReadOutElementStrip
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutElementStrip.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutElementStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutElementStrip::MReadOutElementStrip() : MReadOutElement(), m_StripID(g_UnsignedIntNotDefined)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Read out element of a SINGLE-SIDED strip detector  
MReadOutElementStrip::MReadOutElementStrip(unsigned int DetectorID, unsigned int StripID) : MReadOutElement(DetectorID), m_StripID(StripID) 
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutElementStrip::~MReadOutElementStrip()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this read-out element - the returned element must be deleted!
MReadOutElementStrip* MReadOutElementStrip::Clone() const
{
  MReadOutElementStrip* R = new MReadOutElementStrip(m_DetectorID, m_StripID);
  return R;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutElementStrip::Clear()
{
  MReadOutElement::Clear();
  m_StripID = g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out element is of the given type
bool MReadOutElementStrip::IsOfType(const MString& String) const
{ 
  if (String == "singlessidedstrip") return true;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the type of this read-out element
MString MReadOutElementStrip::GetType() const
{
  return "singlessidedstrip";
}


////////////////////////////////////////////////////////////////////////////////


//! Test for equality
bool MReadOutElementStrip::operator==(const MReadOutElement& R) const
{
  const MReadOutElementStrip* S = dynamic_cast<const MReadOutElementStrip*>(&R);
  
  if (S == 0) return false;
  if (MReadOutElement::operator==(R) == false) return false;
  if (m_StripID != S->m_StripID) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Smaller than operator
bool MReadOutElementStrip::operator<(const MReadOutElement& R) const
{
  const MReadOutElementStrip* S = dynamic_cast<const MReadOutElementStrip*>(&R);
  
  if (S == 0) return false;
  if (MReadOutElement::operator<(R) == true) return true;
  if (m_DetectorID == S->m_DetectorID) {
    if (m_StripID < S->m_StripID) return true;
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutElementStrip::GetNumberOfParsableElements() const
{
  return 2; 
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutElementStrip::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) {
    merr<<GetType()<<": Not enough elements to parse"<<show;
    return false;
  }
  
  m_DetectorID = T.GetTokenAtAsUnsignedInt(StartElement);
  m_StripID = T.GetTokenAtAsUnsignedInt(StartElement+1);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the data as parsable string
MString MReadOutElementStrip::ToParsableString(bool WithDescriptor) 
{
  MString Return;
  if (WithDescriptor == true) {
    Return += "sss "; 
  }
  Return += m_DetectorID;
  Return += " ";
  Return += m_StripID;

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert content to a string
MString MReadOutElementStrip::ToString() const
{
  ostringstream os;
  os<<"Detector: "<<m_DetectorID<<", strip: "<<m_StripID;
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Stream the data
ostream& operator<<(ostream& os, const MReadOutElementStrip& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutElementStrip.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
