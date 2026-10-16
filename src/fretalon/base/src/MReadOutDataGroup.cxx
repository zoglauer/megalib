/*
 * MReadOutDataGroup.cxx
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
// MReadOutDataGroup
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataGroup.h"

// Standard libs:
#include <algorithm>
#include <atomic>
using namespace std;

// ROOT libs:
#include "TH1.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MExceptions.h"
#include "MReadOutDataADCValue.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutDataGroup)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Global switch for the compact storage of plain ADC values
static std::atomic<bool> g_CompactStorage(true);

//! Per-thread temporary which is returned for compactly stored ADC values
static MReadOutDataADCValue& TemporaryADC(unsigned int Value)
{
  static thread_local MReadOutDataADCValue Temporary;
  Temporary.SetADCValue(Value);
  return Temporary;
}

void MReadOutDataGroup::SetCompactStorage(bool Compact) { g_CompactStorage = Compact; }
bool MReadOutDataGroup::GetCompactStorage() { return g_CompactStorage; }


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataGroup::MReadOutDataGroup() 
{
  m_Name = "";
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataGroup::MReadOutDataGroup(const MString & Name) 
{
  m_Name = Name;
}


////////////////////////////////////////////////////////////////////////////////


//! The copy constructor 
MReadOutDataGroup::MReadOutDataGroup(const MReadOutDataGroup& ReadOutDataGroup)
{
  m_Name = ReadOutDataGroup.m_Name;
  
  m_ADCs = ReadOutDataGroup.m_ADCs;
  for (unsigned int d = 0; d < ReadOutDataGroup.m_RODs.size(); ++d) {
    m_RODs.push_back(ReadOutDataGroup.m_RODs[d]->Clone());
  }
}


////////////////////////////////////////////////////////////////////////////////


//! The move constructor
MReadOutDataGroup::MReadOutDataGroup(MReadOutDataGroup&& RODG) noexcept :
  m_Name(std::move(RODG.m_Name)), m_ADCs(std::move(RODG.m_ADCs)), m_RODs(std::move(RODG.m_RODs))
{
  RODG.m_ADCs.clear();
  RODG.m_RODs.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Move assignment operator
MReadOutDataGroup& MReadOutDataGroup::operator=(MReadOutDataGroup&& RODG) noexcept
{
  if (this != &RODG) {
    for (unsigned int i = 0; i < m_RODs.size(); ++i) {
      delete m_RODs[i];
    }
    m_Name = std::move(RODG.m_Name);
    m_ADCs = std::move(RODG.m_ADCs);
    RODG.m_ADCs.clear();
    m_RODs = std::move(RODG.m_RODs);
    RODG.m_RODs.clear();
  }
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataGroup::~MReadOutDataGroup()
{
  for (unsigned int i = 0; i < m_RODs.size(); ++i) {
    delete m_RODs[i];
  }
  m_RODs.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Assignment operator
MReadOutDataGroup& MReadOutDataGroup::operator=(const MReadOutDataGroup& ReadOutDataGroup)
{
  m_Name = ReadOutDataGroup.m_Name;
  
  for (unsigned int i = 0; i < m_RODs.size(); ++i) {
    delete m_RODs[i];
  }
  m_RODs.clear();
  
  m_ADCs = ReadOutDataGroup.m_ADCs;
  for (unsigned int d = 0; d < ReadOutDataGroup.m_RODs.size(); ++d) {
    m_RODs.push_back(ReadOutDataGroup.m_RODs[d]->Clone());
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Add this read-out data 
void MReadOutDataGroup::Add(const MReadOutData& ROD)
{
  // Plain ADC values (no wrapped data) are stored compactly
  if (g_CompactStorage.load(std::memory_order_relaxed) == true && ROD.IsWrapping() == false) {
    const MReadOutDataADCValue* ADC = dynamic_cast<const MReadOutDataADCValue*>(&ROD);
    if (ADC != nullptr) {
      m_ADCs.push_back(ADC->GetADCValue());
      return;
    }
  }
  
  m_RODs.push_back(ROD.Clone());
}


////////////////////////////////////////////////////////////////////////////////


//! Move the content from the given into this read-out data group
void MReadOutDataGroup::Move(MReadOutDataGroup& RODG)
{
  m_ADCs.insert(m_ADCs.end(), RODG.m_ADCs.begin(), RODG.m_ADCs.end());
  RODG.m_ADCs.clear();
  m_RODs.insert(m_RODs.end(), RODG.m_RODs.begin(), RODG.m_RODs.end());
  RODG.m_RODs.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Return a specific read-out data, in case it doesn't exist throw the exception MExceptionIndexOutOfBounds
MReadOutData& MReadOutDataGroup::GetReadOutData(unsigned int d)
{
  if (d < m_ADCs.size()) {
    return TemporaryADC(m_ADCs[d]);
  }
  d -= m_ADCs.size();
  
  if (d < m_RODs.size()) {
    return *m_RODs[d]; 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_ADCs.size() + m_RODs.size(), d + m_ADCs.size());
  
  // The following is wrong and might crash, but we still need a return value... 
  return *m_RODs[0];
} 


////////////////////////////////////////////////////////////////////////////////


//! Return a specific read-out data, in case it doesn't exist throw the exception MExceptionIndexOutOfBounds
const MReadOutData& MReadOutDataGroup::GetReadOutData(unsigned int d) const
{
  if (d < m_ADCs.size()) {
    return TemporaryADC(m_ADCs[d]);
  }
  d -= m_ADCs.size();
  
  if (d < m_RODs.size()) {
    return *m_RODs[d]; 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_ADCs.size() + m_RODs.size(), d + m_ADCs.size());
  
  // The following is wrong and might crash, but we still need a return value... 
  return *m_RODs[0];
} 


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataGroup::ToString() const
{
  ostringstream os;
  os<<"Data group with "<<m_ADCs.size() + m_RODs.size()<<" entries";
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutDataGroup& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutDataGroup.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
