/*
 * MSettingsGeomega.cxx
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
// MSettingsGeomega.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsGeomega.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettingsGeomega)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsGeomega::MSettingsGeomega(bool AutoLoad) : MSettings("GeomegaConfigurationFile")
{
  // default constructor

  m_DefaultSettingsFileName = MString(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".geomega.cfg"));
  m_SettingsFileName = m_DefaultSettingsFileName;

  
  m_MGeantOutputMode = 0;
  m_MGeantFileName = "MGeantGeo";

  m_StoreIAs = false;
  m_StoreVetoes = true;

  
  if (AutoLoad == true) {
    Read();
  }
}


////////////////////////////////////////////////////////////////////////////////


MSettingsGeomega::~MSettingsGeomega()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsGeomega::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MSettings::WriteXml(Node);

  new MXmlNode(Node, "MGeantOutputMode", m_MGeantOutputMode);
  new MXmlNode(Node, "MGeantFile", CleanPath(m_MGeantFileName));
  new MXmlNode(Node, "Position",  m_Position);
  new MXmlNode(Node, "StoreIAs", m_StoreIAs);
  new MXmlNode(Node, "StoreVetoes", m_StoreVetoes);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsGeomega::ReadXml(MXmlNode* Node)
{  
  // Retrieve the content from an XML tree

  MSettings::ReadXml(Node);
  
  MXmlNode* aNode = 0;
  
  if ((aNode = Node->GetNode("MGeantOutputMode")) != 0) {
    m_MGeantOutputMode = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("MGeantFile")) != 0) {
    m_MGeantFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("Position")) != 0) {
    m_Position = aNode->GetValueAsVector();
  }
  if ((aNode = Node->GetNode("StoreIAs")) != 0) {
    m_StoreIAs = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("StoreVetoes")) != 0) {
    m_StoreVetoes = aNode->GetValueAsBoolean();
  }

  return true;
}


// MSettingsGeomega.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
