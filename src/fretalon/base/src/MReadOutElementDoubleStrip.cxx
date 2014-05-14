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


#ifdef ___CINT___
ClassImp(MReadOutElementDoubleStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutElementDoubleStrip::MReadOutElementDoubleStrip(): MReadOutElementStrip(), m_IsPositiveStrip(true)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Read out element of a DOUBLE-SIDED strip detector  
MReadOutElementDoubleStrip::MReadOutElementDoubleStrip(unsigned int DetectorID, unsigned int StripID, bool IsPositiveStrip) : MReadOutElementStrip(DetectorID, StripID), m_IsPositiveStrip(IsPositiveStrip)
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
  MReadOutElementDoubleStrip* R = new MReadOutElementDoubleStrip(m_DetectorID, m_StripID, m_IsPositiveStrip);
  return R;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutElementDoubleStrip::Clear()
{
  MReadOutElementStrip::Clear();
  m_IsPositiveStrip = true;
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
  if (MReadOutElementStrip::operator==(R) == false) return false;
  if (m_IsPositiveStrip != S->m_IsPositiveStrip) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Smaller than operator
bool MReadOutElementDoubleStrip::operator<(const MReadOutElement& R) const
{
  const MReadOutElementDoubleStrip* S = dynamic_cast<const MReadOutElementDoubleStrip*>(&R);
  
  if (S == 0) return false;
  if (MReadOutElementStrip::operator<(R) == true) return true;
  if (m_DetectorID == S->m_DetectorID && m_StripID == S->m_StripID) {
    if (m_IsPositiveStrip == false && S->m_IsPositiveStrip == true) return true;
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
  
  m_DetectorID = T.GetTokenAtAsUnsignedInt(StartElement);
  m_StripID = T.GetTokenAtAsUnsignedInt(StartElement+1);
  m_IsPositiveStrip = (T.GetTokenAt(StartElement+2) == "p") ? true : false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the data as parsable string
MString MReadOutElementDoubleStrip::ToParsableString(bool WithDescriptor) 
{
  MString Return;
  if (WithDescriptor == true) {
    Return += "dss "; 
  }
  Return += m_DetectorID;
  Return += " ";
  Return += m_StripID;
  Return += " ";
  Return += (m_IsPositiveStrip ? "p" : "n");

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a string
MString MReadOutElementDoubleStrip::ToString() const
{
  ostringstream os;
  os<<"Detector: "<<m_DetectorID<<", side: "<<((m_IsPositiveStrip == true) ? "pos" : "neg")<<", strip: "<<m_StripID;
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
