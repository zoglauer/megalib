/*
 * MReadOutData.cxx
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
// MReadOutData
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutData.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutData)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutData::MReadOutData() : m_Wrapped(nullptr)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Standard constructor - do the decoration
MReadOutData::MReadOutData(MReadOutData* Data) : m_Wrapped(Data) 
{
}


////////////////////////////////////////////////////////////////////////////////


//! Simple default destructor
MReadOutData::~MReadOutData() 
{
  delete m_Wrapped; 
}


////////////////////////////////////////////////////////////////////////////////


//! Return the combined type of this read-out data and its wrapped data
MString MReadOutData::GetCombinedType() const
{
  MString Return;
  if (m_Wrapped != 0) {
    Return = m_Wrapped->GetCombinedType();
  }
  if (Return != "") {
    Return += "-"; 
  }
  Return += GetType();
 
  return Return;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Return this read-out data as the given type - return nullptr if not possible
MReadOutData* MReadOutData::Get(long TypeID)
{
  if (GetTypeID() == TypeID) {
    return this; 
  }
  
  if (m_Wrapped != nullptr) {
    return m_Wrapped->Get(TypeID);
  }
  
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutData::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) { // calls the derived classes one so we are right
    merr<<GetType()<<": Not enough elements to parse"<<show;
    return false;
  }
  
  if (m_Wrapped != 0) return m_Wrapped->Parse(T, StartElement);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutData& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutData.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
