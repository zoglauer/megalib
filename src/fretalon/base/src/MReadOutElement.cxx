/*
 * MReadOutElement.cxx
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
// MReadOutElement
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutElement.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutElement)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutElement::MReadOutElement() : m_DetectorID(g_UnsignedIntNotDefined)
{
}


////////////////////////////////////////////////////////////////////////////////


//! A basic read out element - only knows a detector
MReadOutElement::MReadOutElement(unsigned int DetectorID) : m_DetectorID(DetectorID) 
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutElement::~MReadOutElement()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this read-out element - the returned element must be deleted!
MReadOutElement* MReadOutElement::Clone() const
{
  MReadOutElement* R = new MReadOutElement(m_DetectorID);
  return R;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutElement::Clear()
{
  m_DetectorID = g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Check for equality
bool MReadOutElement::operator==(const MReadOutElement& R) const
{
  if (m_DetectorID != R.m_DetectorID) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Smaller than operator
bool MReadOutElement::operator<(const MReadOutElement& R) const
{
  if (m_DetectorID < R.m_DetectorID) return true;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out element is of the given type
bool MReadOutElement::IsOfType(const MString& String) const
{ 
  if (String == "single") return true;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the type of this read-out element
MString MReadOutElement::GetType() const
{
  return "single";
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutElement::GetNumberOfParsableElements() const
{
  return 1; 
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutElement::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) {
    merr<<GetType()<<": Not enough elements to parse"<<show;
    return false;
  }
  
  m_DetectorID = T.GetTokenAtAsUnsignedIntFast(StartElement);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the data as parsable string
MString MReadOutElement::ToParsableString(bool WithDescriptor) const 
{
  MString Return;
  if (WithDescriptor == true) {
    Return += "d "; 
  }
  Return += m_DetectorID;

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a string
MString MReadOutElement::ToString() const
{
  ostringstream os;
  os<<"Detector: "<<m_DetectorID;
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Stream the content
ostream& operator<<(ostream& os, const MReadOutElement& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutElement.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
