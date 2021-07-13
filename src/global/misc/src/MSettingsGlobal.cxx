/*
 * MSettingsGlobal.cxx
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
// MSettingsGlobal.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsGlobal.h"

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


#ifdef ___CLING___
ClassImp(MSettingsGlobal)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsGlobal::MSettingsGlobal()
{
  // default constructor

  m_SettingsFileName = "~/.megalib.cfg";
  MFile::ExpandFileName(m_SettingsFileName);

  m_NameMasterNode = "MEGAlib";
  
  m_LicenseHash = 0;
  m_ChangeLogHash = 0;
  m_FontScaler = "normal";
}


////////////////////////////////////////////////////////////////////////////////


MSettingsGlobal::~MSettingsGlobal()
{
  // default destructor
}



////////////////////////////////////////////////////////////////////////////////


bool MSettingsGlobal::Read()
{
  // Load the configuration-file
  
  // We do not have this file when we start up the first time
  if (MFile::Exists(m_SettingsFileName) == false) return true;
  
  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument();
  Document->Load(m_SettingsFileName);

  if (Document->GetName() != m_NameMasterNode) {
    mout<<"Error reading configuration file "<<m_SettingsFileName<<endl;
    mout<<"The loaded configuration file is of type \""<<Document->GetName()<<"\", but expected is: MEGAlib"<<endl;
    mout<<"Starting with the default configuration!"<<endl;
    return false;    
  }

  ReadXml(Document);

  delete Document;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsGlobal::Write()
{
  // Save the current configuration

  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument(m_NameMasterNode);
  WriteXml(Document);

  // Store the module content
  Document->Save(m_SettingsFileName);

  delete Document;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsGlobal::ReadXml(MXmlNode* Node)
{
  //! Read all data from an XML tree

  MXmlNode* aNode = 0;

  if ((aNode = Node->GetNode("LicenseHash")) != 0) {
    m_LicenseHash = aNode->GetValueAsLong();
  }

  if ((aNode = Node->GetNode("ChangeLogHash")) != 0) {
    m_ChangeLogHash = aNode->GetValueAsLong();
  }

  if ((aNode = Node->GetNode("FontScaler")) != 0) {
    m_FontScaler = aNode->GetValueAsString();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsGlobal::WriteXml(MXmlNode* Node)
{
  //! Writes all data to an XML tree
  
  new MXmlNode(Node, "LicenseHash", m_LicenseHash);
  new MXmlNode(Node, "ChangeLogHash", m_ChangeLogHash);
  new MXmlNode(Node, "FontScaler", m_FontScaler);
  
  return true;
}


// MSettingsGlobal.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
