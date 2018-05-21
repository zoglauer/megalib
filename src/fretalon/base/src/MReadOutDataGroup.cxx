/*
 * MReadOutDataGroup.cxx
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
// MReadOutDataGroup
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataGroup.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TH1.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutDataGroup)
#endif


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
  
  for (unsigned int d = 0; d < ReadOutDataGroup.m_RODs.size(); ++d) {
    m_RODs.push_back(ReadOutDataGroup.m_RODs[d]->Clone());
  }
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
  
  for (unsigned int d = 0; d < ReadOutDataGroup.m_RODs.size(); ++d) {
    m_RODs.push_back(ReadOutDataGroup.m_RODs[d]->Clone());
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Add this read-out data 
void MReadOutDataGroup::Add(const MReadOutData& ROD)
{
  m_RODs.push_back(ROD.Clone());
}


////////////////////////////////////////////////////////////////////////////////


//! Move the content from the given into this read-out data group
void MReadOutDataGroup::Move(MReadOutDataGroup& RODG)
{
  m_RODs.insert(m_RODs.end(), RODG.m_RODs.begin(), RODG.m_RODs.end());
  RODG.m_RODs.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Return a specific read-out data, in case it doesn't exist throw the exception MExceptionIndexOutOfBounds
MReadOutData& MReadOutDataGroup::GetReadOutData(unsigned int d)
{
  if (d < m_RODs.size()) {
    return *m_RODs[d]; 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_RODs.size(), d);
  
  // The following is wrong and might crash, but we still need a return value... 
  return *m_RODs[0];
} 


////////////////////////////////////////////////////////////////////////////////


//! Return a specific read-out data, in case it doesn't exist throw the exception MExceptionIndexOutOfBounds
const MReadOutData& MReadOutDataGroup::GetReadOutData(unsigned int d) const
{
  if (d < m_RODs.size()) {
    return *m_RODs[d]; 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_RODs.size(), d);
  
  // The following is wrong and might crash, but we still need a return value... 
  return *m_RODs[0];
} 


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataGroup::ToString() const
{
  ostringstream os;
  os<<"Data group with "<<m_RODs.size()<<" entries";
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
