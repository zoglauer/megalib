/*
 * MReadOutElementPixel2D.cxx
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
// MReadOutElementPixel2D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutElementPixel2D.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutElementPixel2D)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutElementPixel2D::MReadOutElementPixel2D() : MReadOutElement(), m_XPixelID(g_UnsignedIntNotDefined), m_YPixelID(g_UnsignedIntNotDefined)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Read out element of a SINGLE-SIDED strip detector  
MReadOutElementPixel2D::MReadOutElementPixel2D(unsigned int DetectorID, unsigned int XPixelID, unsigned int YPixelID) : MReadOutElement(DetectorID), m_XPixelID(XPixelID),  m_YPixelID(YPixelID)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutElementPixel2D::~MReadOutElementPixel2D()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this read-out element - the returned element must be deleted!
MReadOutElementPixel2D* MReadOutElementPixel2D::Clone() const
{
  MReadOutElementPixel2D* R = new MReadOutElementPixel2D(m_DetectorID, m_XPixelID, m_YPixelID);
  return R;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutElementPixel2D::Clear()
{
  MReadOutElement::Clear();
  m_XPixelID = g_UnsignedIntNotDefined;
  m_YPixelID = g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out element is of the given type
bool MReadOutElementPixel2D::IsOfType(const MString& String) const
{ 
  if (String == "pixel2d") return true;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the type of this read-out element
MString MReadOutElementPixel2D::GetType() const
{
  return "pixel2d";
}


////////////////////////////////////////////////////////////////////////////////


//! Test for equality
bool MReadOutElementPixel2D::operator==(const MReadOutElement& R) const
{
  const MReadOutElementPixel2D* S = dynamic_cast<const MReadOutElementPixel2D*>(&R);
  if (S == nullptr) return false;

  if (m_XPixelID != S->m_XPixelID) return false;
  if (m_YPixelID != S->m_YPixelID) return false;
  if (m_DetectorID != S->m_DetectorID) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Smaller than operator
bool MReadOutElementPixel2D::operator<(const MReadOutElement& R) const
{
  const MReadOutElementPixel2D* S = dynamic_cast<const MReadOutElementPixel2D*>(&R);
  if (S == nullptr) return false;
  
  if (m_DetectorID < S->m_DetectorID) return true;
  if (m_DetectorID == S->m_DetectorID) {
    if (m_XPixelID < S->m_XPixelID) return true;
    if (m_XPixelID == S->m_XPixelID) {
      if (m_YPixelID < S->m_YPixelID) return true;
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutElementPixel2D::GetNumberOfParsableElements() const
{
  return 3;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutElementPixel2D::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) {
    merr<<GetType()<<": Not enough elements to parse"<<show;
    return false;
  }
  
  m_DetectorID = T.GetTokenAtAsUnsignedIntFast(StartElement);
  m_XPixelID = T.GetTokenAtAsUnsignedIntFast(StartElement+1);
  m_YPixelID = T.GetTokenAtAsUnsignedIntFast(StartElement+2);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the data as a parsable string
MString MReadOutElementPixel2D::ToParsableString(bool WithDescriptor) const
{
  MString Return;
  if (WithDescriptor == true) {
    Return += "p2d ";
  }
  Return += m_DetectorID;
  Return += " ";
  Return += m_XPixelID;
  Return += " ";
  Return += m_YPixelID;

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert content to a string
MString MReadOutElementPixel2D::ToString() const
{
  ostringstream os;
  os<<"Detector: "<<m_DetectorID<<", x pixel: "<<m_XPixelID<<", y pixel: "<<m_YPixelID;
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Stream the data
ostream& operator<<(ostream& os, const MReadOutElementPixel2D& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutElementPixel2D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
