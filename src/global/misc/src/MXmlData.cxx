/*
 * MXmlData.cxx
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
// MXmlData
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MXmlData.h"

// Standard libs:
#include <sstream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MXmlData)
#endif


////////////////////////////////////////////////////////////////////////////////


MXmlData::MXmlData()
{
  // Construct an instance of MXmlData

  m_Name = "";
  m_Value = "";
}


////////////////////////////////////////////////////////////////////////////////


MXmlData::MXmlData(MString Name)
{
  //! Constructor

  m_Name = Name;
  m_Value = "";
}


////////////////////////////////////////////////////////////////////////////////


MXmlData::MXmlData(MString Name, MString Value)
{
  //! Constructor

  m_Name = Name;
  m_Value = Value;
}


////////////////////////////////////////////////////////////////////////////////


MXmlData::MXmlData(MString Name, int Value)
{
  //! Constructor

  m_Name = Name;
  ostringstream out;
  out<<Value;
  m_Value = out.str();
}


////////////////////////////////////////////////////////////////////////////////


MXmlData::MXmlData(MString Name, long Value)
{
  //! Constructor

  m_Name = Name;
  ostringstream out;
  out<<Value;
  m_Value = out.str();
}


////////////////////////////////////////////////////////////////////////////////


MXmlData::MXmlData(MString Name, unsigned int Value)
{
  //! Constructor

  m_Name = Name;
  ostringstream out;
  out<<Value;
  m_Value = out.str();
}


////////////////////////////////////////////////////////////////////////////////


MXmlData::MXmlData(MString Name, double Value)
{
  //! Constructor

  m_Name = Name;
  ostringstream out;
  out.precision(15);
  out<<Value;
  m_Value = out.str();
}


////////////////////////////////////////////////////////////////////////////////


MXmlData::MXmlData(MString Name, bool Value)
{
  //! Constructor

  m_Name = Name;
  m_Value = ((Value == true) ? "true" : "false");
}


////////////////////////////////////////////////////////////////////////////////


MXmlData::~MXmlData()
{
}


////////////////////////////////////////////////////////////////////////////////


void MXmlData::Clear()
{
  // Reset the node

  m_Name = "";
  m_Value = "";
}


////////////////////////////////////////////////////////////////////////////////


bool MXmlData::GetValueAsBoolean() const
{
  //! Return the value of the node as boolean
  
  if (m_Value == "true" || m_Value == "TRUE") {
    return true;
  }

  return false;
}


// MXmlData.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
