/*
 * MReadOutData.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
