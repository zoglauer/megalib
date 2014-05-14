/*
 * MCallibrationCollection.cxx
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
// MCallibrationCollection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MCallibrationCollection.h"

// Standard libs:
#include <algorithm>

// ROOT libs:
#include "TH1.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MExceptions.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCallibrationCollection)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCallibrationCollection::MCallibrationCollection()
{
  m_ROE = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor with a reference to a read-out element
MCallibrationCollection::MCallibrationCollection(const MCallibrationElement& ROE)
{
  m_ROE = ROE.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! The copy constructor 
MCallibrationCollection::MCallibrationCollection(const MCallibrationCollection& CallibrationCollection)
{
  m_ROE = CallibrationCollection.m_ROE->Clone();

  for (unsigned int d = 0; d < CallibrationCollection.m_ROGs.size(); ++d) {
    m_ROGs.push_back(CallibrationCollection.m_ROGs[d]);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCallibrationCollection::~MCallibrationCollection()
{
  delete m_ROE;
}


////////////////////////////////////////////////////////////////////////////////


//! Assignment operator
MCallibrationCollection& MCallibrationCollection::operator=(const MCallibrationCollection& ROC)
{
  delete m_ROE;
  m_ROE = ROC.m_ROE->Clone();  
  for (unsigned int d = 0; d < ROC.m_ROGs.size(); ++d) {
    m_ROGs.push_back(ROC.m_ROGs[d]);
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the read-out element
void MCallibrationCollection::SetCallibrationElement(const MCallibrationElement& ROE)
{
  delete m_ROE;
  m_ROE = ROE.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a read-out group and return its ID
void MCallibrationCollection::AddCallibrationDataGroup(const MString& Name)
{
  MCallibrationDataGroup G(Name);
  m_ROGs.push_back(G);
}


////////////////////////////////////////////////////////////////////////////////


//! Add a read-out group and return its ID
void MCallibrationCollection::AddCallibrationDataGroups(vector<MString> Names)
{
  for (unsigned int g = 0; g < Names.size(); ++g) {
    MCallibrationDataGroup G(Names[g]);
    m_ROGs.push_back(G);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Add this read-out data 
bool MCallibrationCollection::Add(const MCallibrationData& ROD, unsigned int g)
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
void MCallibrationCollection::Add(const MCallibrationSequence& ROS, unsigned int g)
{
  for (unsigned int d = 0; d < ROS.GetNumberOfCallibrations(); ++d) { 
    Add(ROS.GetCallibration(d), g);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Check if the read-out elements are identical
bool MCallibrationCollection::HasIdenticalCallibrationElement(const MCallibrationElement& ROE) const
{
  return m_ROE->operator==(ROE);
}


////////////////////////////////////////////////////////////////////////////////


//! If the read out has the correct ROE add it
bool MCallibrationCollection::Add(const MCallibration& RO, unsigned int g)
{
  if (HasIdenticalCallibrationElement(RO.GetCallibrationElement()) == true) {
    Add(RO.GetCallibrationData(), g);
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Get a read-out data group by its index
//! Use GetNumberOfCallibrationDataGroup first to check if it exists, because
//! if it doesn't exist an exception MExceptionIndexOutOfBounds is returned
MCallibrationDataGroup& MCallibrationCollection::GetCallibrationDataGroup(unsigned int g)
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
MString MCallibrationCollection::ToString() const
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
ostream& operator<<(ostream& os, const MCallibrationCollection& R)
{
  os<<R.ToString();
  return os;
}


// MCallibrationCollection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
