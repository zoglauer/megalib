/*
 * MSettings.cxx
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
// MSettings.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettings.h"

// Standard libs:
#include <iostream>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MFile.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettings)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettings::MSettings(MString NameMasterNode) : m_NameMasterNode(NameMasterNode)
{
  // default constructor

  m_Version = 2;
  m_MEGAlibVersion = g_Version;
  m_DefaultSettingsFileName = ".megalib.cfg";
}


////////////////////////////////////////////////////////////////////////////////


MSettings::~MSettings()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSettings::Read()
{
  // Load the default configuration-file

  return Read(m_DefaultSettingsFileName);
}



////////////////////////////////////////////////////////////////////////////////


bool MSettings::Read(MString FileName)
{
  // Load the configuration-file

  if (FileName == g_StringNotDefined || FileName == "") {
    m_SettingsFileName = m_DefaultSettingsFileName;
  } else {
    m_SettingsFileName = FileName;
  }

  MFile::ExpandFileName(m_SettingsFileName);
  
  // Read the first line, if it begins with "<" we have an xml file
  ifstream in;
  in.open(m_SettingsFileName);
  if (in.is_open() == false) {
    mout<<"Can't find the configuration file "<<m_SettingsFileName<<endl;
    mout<<"Starting with the default configuration!"<<endl;
    return false;
  }

  const int LineLength = 1000;
  char LineBuffer[LineLength];

  if (in.getline(LineBuffer, LineLength, '\n')) {
    MString Line(LineBuffer);
    Line.ReplaceAll("\r", "");
    Line.StripFront(' ');
    Line.StripBack(' ');
    if (Line.BeginsWith(MString("<")) == false || Line.EndsWith(MString(">")) == false) {
      in.close();
      mout<<"The non-XML configuration file format is no longer supported!"<<endl;
      mout<<"Starting with the default configuration!"<<endl;
      return true;
    }
  } else {
    mout<<"Error reading configuration file "<<m_SettingsFileName<<endl;
    mout<<"Starting with the default configuration!"<<endl;
    return false;
  }


  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument();
  Document->Load(m_SettingsFileName);

  if (m_NameMasterNode != Document->GetName()) {
    mout<<"Error reading configuration file "<<m_SettingsFileName<<endl;
    mout<<"The loaded configuration file is of type \""<<Document->GetName()<<"\", but expected is: \""<<m_NameMasterNode<<"\""<<endl;
    mout<<"Starting with the default configuration!"<<endl;
    return false;    
  }

  ReadXml(Document);

  delete Document;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettings::Write()
{
  // Save the current configuration

  return Write(m_DefaultSettingsFileName);
}


////////////////////////////////////////////////////////////////////////////////


bool MSettings::Write(MString FileName)
{
  // Save the current configuration

  if (FileName == "" || FileName == g_StringNotDefined) {
    FileName = m_DefaultSettingsFileName;
  }

  // We always add the suffix if it doesn't exist:
  if (FileName.EndsWith(".cfg") == false) {
    FileName += ".cfg";
  }

  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument(m_NameMasterNode);

  WriteXml(Document);

  // Store the module content
  MFile::ExpandFileName(FileName);
  Document->Save(FileName);
  
  delete Document;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettings::ReadXml(MXmlNode* Node)
{
  //! Read all data from an XML tree

  MXmlNode* aNode = 0;

  if ((aNode = Node->GetNode("Version")) != 0) {
    m_Version = aNode->GetValueAsInt();
  }

  if ((aNode = Node->GetNode("MEGAlibVersion")) != 0) {
    m_MEGAlibVersion = aNode->GetValueAsInt();
  }

  MSettingsBasicFiles::ReadXml(Node);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettings::WriteXml(MXmlNode* Node)
{
  //! Writes all data to an XML tree
  
  new MXmlNode(Node, "Version", m_Version);
  new MXmlNode(Node, "MEGAlibVersion", m_MEGAlibVersion);

  MSettingsBasicFiles::WriteXml(Node); 
  
  return true;
}


// MSettings.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
