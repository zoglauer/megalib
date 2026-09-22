/*
 * MXmlDocument.cxx
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
// MXmlDocument
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MXmlDocument.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MXmlDocument)
#endif


////////////////////////////////////////////////////////////////////////////////


MXmlDocument::MXmlDocument()
{
  // Construct an instance of MXmlDocument
}


////////////////////////////////////////////////////////////////////////////////


MXmlDocument::MXmlDocument(const MString& Name)
{
  // Construct an instance of MXmlDocument

  m_Name = Name;
}


////////////////////////////////////////////////////////////////////////////////


MXmlDocument::~MXmlDocument()
{
  // Delete this instance of MXmlDocument
}


////////////////////////////////////////////////////////////////////////////////


bool MXmlDocument::Load(MString FileName)
{
  // Load an XML document

  Clear();

  MString AllContent;

  ifstream in(FileName); 
  if (in.is_open() == false) { 
    mout<<"Xml parser: Error opening file: "<<FileName<<endl;
    return false; 
  } 
 
  string tmp;
  while (getline(in, tmp)) {
    AllContent += tmp.c_str();
    AllContent += "\n";
  }

  // Remove all comments:
  size_t Begin;
  size_t End;
  while ((Begin = AllContent.Index("<!--", 0)) != MString::npos) {
    if (AllContent.Index(">", Begin) < AllContent.Index("-->", Begin)) {
      mout<<"Xml parser: Error with comments!"<<endl;
      return false; 
    }
    End = AllContent.Index("-->", Begin);
    AllContent = AllContent.Replace(Begin, End-Begin+3, "");
  }
  while ((Begin = AllContent.Index("<?", 0)) != MString::npos) {
    if (AllContent.Index(">", Begin) < AllContent.Index("?>", Begin)) {
      mout<<"Xml parser: Error with <? ... ?>!"<<endl;
      return false; 
    }
    End = AllContent.Index("?>", Begin);
    AllContent = AllContent.Replace(Begin, End-Begin+2, "");
  }

  // Remove all returns:
  AllContent = AllContent.ReplaceAll("\n", "");
  AllContent = AllContent.ReplaceAll("\r", "");

  // Detect the outer name:
  size_t FirstBegin = AllContent.Index("<", 0);
  size_t FirstEnd = AllContent.Index(">", FirstBegin);

  m_Name = AllContent.GetSubString(FirstBegin+1, FirstEnd-FirstBegin-1);

  // Make sure to ignore attributes
  MString Attributes("");
  size_t FirstAttribute = m_Name.First(' ');
  if (FirstAttribute != MString::npos) {
    Attributes = m_Name.GetSubString(FirstAttribute+1, m_Name.Length() - FirstAttribute - 1);
    m_Name = m_Name.GetSubString(0, FirstAttribute);
  }

  Attributes = Attributes.Strip();
  size_t Equal = MString::npos;
  while ((Equal = Attributes.Index("=")) != MString::npos) {
    MString AttributeName = Attributes.GetSubString(0, Equal);
    AttributeName = AttributeName.Strip();

    size_t FirstQuote = Attributes.Index("\"");
    if (FirstQuote == MString::npos) {
      mout<<"Xml parser: Error with root attributes!"<<endl;
      return false;
    }
    size_t SecondQuote = Attributes.Index("\"", FirstQuote+1);
    if (SecondQuote == MString::npos) {
      mout<<"Xml parser: Error with root attributes!"<<endl;
      return false;
    }
    MString AttributeValue = Attributes.GetSubString(FirstQuote+1, SecondQuote-FirstQuote-1);
    AttributeValue.ReplaceAll("&quot;", "\"");
    AttributeValue.ReplaceAll("&lt;", "<");
    AttributeValue.ReplaceAll("&gt;", ">");
    AttributeValue.ReplaceAll("&amp;", "&");

    new MXmlAttribute(this, AttributeName, AttributeValue);

    Attributes = Attributes.GetSubString(SecondQuote+1, Attributes.Length() - SecondQuote - 1);
    Attributes = Attributes.Strip();
  }

  size_t LastBegin = AllContent.Index(MString("</") + m_Name + MString(">"));

  if (Parse(AllContent.GetSubString(FirstEnd+1, LastBegin-FirstEnd-1)) == false) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MXmlDocument::Save(MString FileName)
{
  // Save an XML document

  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    return false;
  }
  out<<ToString()<<endl;
  if (out.good() == false) {
    out.close();
    return false;
  }
  out.close();

  return true;
}


// MXmlDocument.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
