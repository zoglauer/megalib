/*
 * MXmlAttribute.cxx
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


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, const MString& Name) : MXmlData(Name)
{
  // Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, const MString& Name, const MString& Value) : MXmlData(Name, Value)
{
  // Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, const MString& Name, int Value) : MXmlData(Name, Value)
{
  // Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, const MString& Name, unsigned int Value) : MXmlData(Name, Value)
{
  // Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, const MString& Name, double Value) : MXmlData(Name, Value)
{
  // Constructor

  if (MotherNode != 0) {
    MotherNode->AddAttribute(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute::MXmlAttribute(MXmlNode* MotherNode, const MString& Name, bool Value) : MXmlData(Name, Value)
{
  // Constructor

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
  // Return the XML text

  MString Value = m_Value;
  Value.ReplaceAll("&", "&amp;");
  Value.ReplaceAll("\"", "&quot;");
  Value.ReplaceAll("<", "&lt;");
  Value.ReplaceAll(">", "&gt;");

  MString Xml;
  Xml += m_Name;
  Xml += "=\"";
  Xml += Value;
  Xml += "\"";

  return Xml;
}


// MXmlAttribute.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
