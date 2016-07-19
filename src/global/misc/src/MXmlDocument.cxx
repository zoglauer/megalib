/*
 * MXmlDocument.cxx
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
// MXmlDocument
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MXmlDocument.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MXmlDocument)
#endif


////////////////////////////////////////////////////////////////////////////////


MXmlDocument::MXmlDocument()
{
  // Construct an instance of MXmlDocument
}


////////////////////////////////////////////////////////////////////////////////


MXmlDocument::MXmlDocument(MString Name)
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
  //! Load an xml document

  MString AllContent;

  ifstream in(FileName); 
  if (in.is_open() == false) { 
    cout<<"Xml parser: Error opening file: "<<FileName<<endl; 
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
      cout<<"Xml parser: Error with comments!"<<endl; 
      return false; 
    }
    End = AllContent.Index("-->", Begin);
    AllContent = AllContent.Replace(Begin, End-Begin+3, "");
  }
  while ((Begin = AllContent.Index("<?", 0)) != MString::npos) {
    if (AllContent.Index(">", Begin) < AllContent.Index("?>", Begin)) {
      cout<<"Xml parser: Error with <? ... ?>!"<<endl; 
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
  size_t FirstAttribute = m_Name.First(' ');
  if (FirstAttribute != MString::npos) {
    m_Name = m_Name.GetSubString(0, FirstAttribute);
  }

  size_t LastBegin = AllContent.Index(MString("</") + m_Name + MString(">"));

  Parse(AllContent.GetSubString(FirstEnd+1, LastBegin-FirstEnd-1));

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MXmlDocument::Save(MString FileName)
{
  //! Save an Xml ldocument

  ofstream out;
  out.open(FileName);
  out<<ToString()<<endl;
  out.close();

  return true;
}


// MXmlDocument.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
