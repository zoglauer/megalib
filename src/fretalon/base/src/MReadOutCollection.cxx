/*
 * MReadOutCollection.cxx
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
// MReadOutCollection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutCollection.h"

// Standard libs:
#include <algorithm>

// ROOT libs:
#include "TH1.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MExceptions.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutCollection)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutCollection::MReadOutCollection()
{
  m_ROE = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor with a reference to a read-out element
MReadOutCollection::MReadOutCollection(const MReadOutElement& ROE)
{
  m_ROE = ROE.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! The copy constructor 
MReadOutCollection::MReadOutCollection(const MReadOutCollection& ReadOutCollection)
{
  m_ROE = ReadOutCollection.m_ROE->Clone();

  for (unsigned int d = 0; d < ReadOutCollection.m_ROGs.size(); ++d) {
    m_ROGs.push_back(ReadOutCollection.m_ROGs[d]);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutCollection::~MReadOutCollection()
{
  delete m_ROE;
}


////////////////////////////////////////////////////////////////////////////////


//! Assignment operator
MReadOutCollection& MReadOutCollection::operator=(const MReadOutCollection& ROC)
{
  delete m_ROE;
  m_ROE = ROC.m_ROE->Clone();  
  m_ROGs.clear();
  for (unsigned int d = 0; d < ROC.m_ROGs.size(); ++d) {
    m_ROGs.push_back(ROC.m_ROGs[d]);
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Smaller than operator - comparing the read-out element
bool MReadOutCollection::operator<(const MReadOutCollection& R) const
{
  if (m_ROE != 0 && R.m_ROE != 0) {
    return m_ROE->operator<(*R.m_ROE);
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the read-out element
void MReadOutCollection::SetReadOutElement(const MReadOutElement& ROE)
{
  delete m_ROE;
  m_ROE = ROE.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a read-out group and return its ID
void MReadOutCollection::AddReadOutDataGroup(const MString& Name)
{
  MReadOutDataGroup G(Name);
  m_ROGs.push_back(G);
}


////////////////////////////////////////////////////////////////////////////////


//! Add a read-out group and return its ID
void MReadOutCollection::AddReadOutDataGroups(vector<MString> Names)
{
  for (unsigned int g = 0; g < Names.size(); ++g) {
    MReadOutDataGroup G(Names[g]);
    m_ROGs.push_back(G);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Add this read-out data 
bool MReadOutCollection::Add(const MReadOutData& ROD, unsigned int g)
{
  if (g < m_ROGs.size()) {
    m_ROGs[g].Add(ROD);
    return true;
  }
  
  throw MExceptionIndexOutOfBounds(0, m_ROGs.size(), g);
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Add all possible roud-out datas (i.e. those with identical sequence) of this read-out sequence 
void MReadOutCollection::Add(const MReadOutSequence& ROS, unsigned int g)
{
  for (unsigned int d = 0; d < ROS.GetNumberOfReadOuts(); ++d) { 
    Add(ROS.GetReadOut(d), g);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Check if the read-out elements are identical
bool MReadOutCollection::HasIdenticalReadOutElement(const MReadOutElement& ROE) const
{
  return m_ROE->operator==(ROE);
}


////////////////////////////////////////////////////////////////////////////////


//! If the read out has the correct ROE add it
bool MReadOutCollection::Add(const MReadOut& RO, unsigned int g)
{
  if (HasIdenticalReadOutElement(RO.GetReadOutElement()) == true) {
    Add(RO.GetReadOutData(), g);
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Move all read outs of this read-out data group
//! If g does not exist return an MExceptionIndexOutOfBounds exception
void MReadOutCollection::Move(MReadOutDataGroup& ROG, unsigned int g)
{
  if (g < m_ROGs.size()) {
    m_ROGs[g].Move(ROG);
    return;
  }
  
  throw MExceptionIndexOutOfBounds(0, m_ROGs.size(), g);
}


////////////////////////////////////////////////////////////////////////////////


//! Get a read-out data group by its index
//! Use GetNumberOfReadOutDataGroup first to check if it exists, because
//! if it doesn't exist an exception MExceptionIndexOutOfBounds is returned
MReadOutDataGroup& MReadOutCollection::GetReadOutDataGroup(unsigned int g)
{
  if (g < m_ROGs.size()) {
    return m_ROGs[g];
  }

  throw MExceptionIndexOutOfBounds(0, m_ROGs.size(), g);
  
  // Might crash, but will never reach this point unless exceptions are turned off
  return m_ROGs[0];
}


////////////////////////////////////////////////////////////////////////////////


//! Get a read-out data group by its index
//! Use GetNumberOfReadOutDataGroup first to check if it exists, because
//! if it doesn't exist an exception MExceptionIndexOutOfBounds is returned
const MReadOutDataGroup& MReadOutCollection::GetReadOutDataGroup(unsigned int g) const
{
  if (g < m_ROGs.size()) {
    return m_ROGs[g];
  }

  throw MExceptionIndexOutOfBounds(0, m_ROGs.size(), g);
  
  // Might crash, but will never reach this point unless exceptions are turned off
  return m_ROGs[0];
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutCollection::ToString() const
{
  ostringstream os;
  os<<"Collection for "<<m_ROE->ToString()<<" with "<<m_ROGs.size()<<" groups:";
  for (unsigned int g = 0; g < m_ROGs.size(); ++g) {
    os<<m_ROGs[g]<<endl; 
  }
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutCollection& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutCollection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
