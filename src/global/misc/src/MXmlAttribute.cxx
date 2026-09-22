/*
 * MXmlAttribute.cxx
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
// MXmlAttribute
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MXmlAttribute.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MXmlNode.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MXmlAttribute)
#endif


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute() : MXmlData()
{
  // Construct an instance of MXmlAttribute
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, MString Name) : MXmlData(Name)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, MString Name, MString Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, MString Name, int Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, MString Name, unsigned int Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, MString Name, double Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, MString Name, bool Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::~MXmlAttribute()
{
  // Delete this instance of MXmlAttribute
}


////////////////////////////////////////////////////////////////////////////////


MString MXmlAttribute::ToString()
{
  //! Returns the XML text

  MString Xml;
  Xml += m_Name;
  Xml += "=\"";
  Xml += m_Value;
  Xml += "\"";

  return Xml;
}


// MXmlAttribute.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
