/*
 * MFretalonRegistry.cxx
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


// Include the header:
#include "MFretalonRegistry.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"
#include "MReadOutElementStrip.h"
#include "MReadOutElementDoubleStrip.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutDataTiming.h"
#include "MReadOutDataTemperature.h"
#include "MReadOutDataOrigins.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFretalonRegistry)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MFretalonRegistry::MFretalonRegistry()
{
  // Register the default read-out elements
  m_ROEs.push_back(new MReadOutElement());
  m_ROEs.push_back(new MReadOutElementStrip());
  m_ROEs.push_back(new MReadOutElementDoubleStrip());
  
  // Register the default read-out datas
  m_RODs.push_back(new MReadOutDataADCValue());
  m_RODs.push_back(new MReadOutDataTiming());
  m_RODs.push_back(new MReadOutDataTemperature());
  m_RODs.push_back(new MReadOutDataOrigins());
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MFretalonRegistry::~MFretalonRegistry()
{
  for (unsigned int e = 0; e < m_ROEs.size(); ++e) {
    delete m_ROEs[e];
  }
  for (unsigned int d = 0; d < m_RODs.size(); ++d) {
    delete m_RODs[d];
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the single instance of this class
MFretalonRegistry& MFretalonRegistry::Instance()
{
  static MFretalonRegistry FR;
  return FR;
}


////////////////////////////////////////////////////////////////////////////////


//! Add another read-out element type
void MFretalonRegistry::Register(const MReadOutElement& ROE)
{
  m_ROEs.push_back(ROE.Clone());
  m_ROEs.back()->Clear();
}

////////////////////////////////////////////////////////////////////////////////


//! Add another read-out data type
void MFretalonRegistry::Register(const MReadOutData& ROD)
{
  m_RODs.push_back(ROD.Clone()); 
  m_RODs.back()->Clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this kind of read-out element is registered
bool MFretalonRegistry::IsReadOutElementRegistered(const MString& Name)
{
  for (unsigned int e = 0; e < m_ROEs.size(); ++e) {
    if (m_ROEs[e]->IsOfType(Name) == true) {
      return true;
    }
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this kind of read-out data is registered
bool MFretalonRegistry::IsReadOutDataRegistered(const MString& Name)
{
  for (unsigned int e = 0; e < m_RODs.size(); ++e) {
    if (m_RODs[e]->IsOfType(Name) == true) {
      return true;
    }
  }
  return false;
}
  

////////////////////////////////////////////////////////////////////////////////


//! Return the new instance of this read-out element
//! Return zero if it doesn't exist, it must be deleted
MReadOutElement* MFretalonRegistry::GetReadOutElement(const MString& Name)
{
  for (unsigned int e = 0; e < m_ROEs.size(); ++e) {
    if (m_ROEs[e]->IsOfType(Name) == true) {
      return m_ROEs[e]->Clone();
    }
  }
  return 0;
}
  

////////////////////////////////////////////////////////////////////////////////


//! Return the new instance of this read-out data
//! Return zero if it doesn't exist, it must be deleted
MReadOutData* MFretalonRegistry::GetReadOutData(const MString& Name)
{
  for (unsigned int e = 0; e < m_RODs.size(); ++e) {
    if (m_RODs[e]->IsOfType(Name) == true) {
      return m_RODs[e]->Clone();
    }
  }
  
  return 0;
}


// MFretalonRegistry.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
