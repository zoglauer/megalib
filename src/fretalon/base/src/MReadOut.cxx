/*
 * MReadOut.cxx
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
// MReadOut
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOut.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOut)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOut::MReadOut()
{
  m_ROE = 0;
  m_ROD = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Copy constructor
MReadOut::MReadOut(const MReadOut& RO)
{
  m_ROE = RO.m_ROE->Clone();
  m_ROD = RO.m_ROD->Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor setting the read out element and data
MReadOut::MReadOut(const MReadOutElement& ROE, const MReadOutData& ROD)
{
  m_ROE = ROE.Clone();
  m_ROD = ROD.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOut::~MReadOut()
{
  delete m_ROE;
  delete m_ROD;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Assignment operator
MReadOut& MReadOut::operator=(const MReadOut& RO)
{
  delete m_ROE;
  m_ROE = RO.m_ROE->Clone();
  delete m_ROD;
  m_ROD = RO.m_ROD->Clone();
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Set the read-out element
void MReadOut::SetReadOutElement(const MReadOutElement& ROE)
{
  delete m_ROE;    
  m_ROE = ROE.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! Set the read-out data
void MReadOut::SetReadOutData(const MReadOutData& ROD)
{
  delete m_ROD;
  m_ROD = ROD.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOut::ToString() const
{
  ostringstream os;
  os<<"RO with "<<m_ROE->ToParsableString(true)<<" and "<<m_ROD->ToParsableString(true);
  return os.str();
}

  

////////////////////////////////////////////////////////////////////////////////


//! Return the data as parsable string
MString MReadOut::ToParsableString(bool WithDescriptor)
{
  ostringstream os;
  os<<"UH "<<m_ROE->ToParsableString(WithDescriptor)<<" "<<m_ROD->ToParsableString(WithDescriptor);
  return os.str();
}

  
////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
std::ostream& operator<<(std::ostream& os, const MReadOut& R)
{
  os<<R.ToString();
  return os;
}


// MReadOut.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
