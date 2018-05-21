/*
 * MReadOutDataOrigins.cxx
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
// MReadOutDataOrigins
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataOrigins.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutDataOrigins)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The type name --- must be unique
const MString MReadOutDataOrigins::m_Type = "origins";
//! The type name ID --- must be unique
const long MReadOutDataOrigins::m_TypeID = m_Type.GetHash();


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataOrigins::MReadOutDataOrigins() : MReadOutData(nullptr), m_Origins()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataOrigins::MReadOutDataOrigins(MReadOutData* Data) : MReadOutData(Data), m_Origins()
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataOrigins::~MReadOutDataOrigins()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataOrigins* MReadOutDataOrigins::Clone() const
{
  MReadOutDataOrigins* ROD = new MReadOutDataOrigins();
  ROD->SetOrigins(m_Origins);
  if (m_Wrapped != 0) {
    ROD->SetWrapped(m_Wrapped->Clone());
  }
  return ROD;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataOrigins::Clear()
{
  MReadOutData::Clear();
  m_Origins.clear();
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataOrigins::GetNumberOfParsableElements() const
{
  return MReadOutData::GetNumberOfParsableElements() + 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataOrigins::Parse(const MTokenizer& T, unsigned int StartElement)
{
  // Go deep first:
  if (MReadOutData::Parse(T, StartElement) == false) return false;
  
  // Then here - we have one string in form 2;3;6 - parse it!
  MString OriginString = T.GetTokenAtAsString(StartElement + MReadOutData::GetNumberOfParsableElements());
  vector<MString> OriginStrings = OriginString.Tokenize(";");
  m_Origins.clear();
  for (MString S: OriginStrings) {
    m_Origins.push_back(atoi(S));
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataOrigins::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString();
  for (unsigned int o = 0; o < m_Origins.size(); ++o) {
    if (o != 0) os<<";";
    os<<m_Origins[o];
  }
  os<<" ";
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataOrigins::ToParsableString(bool WithDescriptor) const 
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
ostream& operator<<(ostream& os, const MReadOutDataOrigins& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataOrigins.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
