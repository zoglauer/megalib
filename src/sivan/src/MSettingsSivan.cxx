/*
 * MSettingsSivan.cxx
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
// MSettingsSivan.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsSivan.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettingsSivan)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsSivan::MSettingsSivan(bool AutoLoad) : MSettings("SivanConfigurationFile")
{
  // default constructor

  m_DefaultSettingsFileName = MString(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".sivan.cfg"));
  m_SettingsFileName = m_DefaultSettingsFileName;

  
  m_Realism = 2;
  m_NInitializationEvents = 2500;

  m_SpecialMode = false;
  
  
  if (AutoLoad == true) {
    Read();
  }
}


////////////////////////////////////////////////////////////////////////////////


MSettingsSivan::~MSettingsSivan()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


void MSettingsSivan::SetRealism(const int Realism)
{
  // Set the realism of the simulated event analysis:
  // 0: ideal data
  // 1: 
  // 2: noised data

  m_Realism = Realism;
}


////////////////////////////////////////////////////////////////////////////////


int MSettingsSivan::GetRealism() const
{
  // Return the realism of the simulated event analysis:

  return m_Realism;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsSivan::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MSettings::WriteXml(Node);

  new MXmlNode(Node, "Realism", m_Realism);
  new MXmlNode(Node, "NInitializationEvents", m_NInitializationEvents);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsSivan::ReadXml(MXmlNode* Node)
{  
  // Retrieve the content from an XML tree

  MSettings::ReadXml(Node);
  
  MXmlNode* aNode = 0;
  
  if ((aNode = Node->GetNode("Realism")) != 0) {
    m_Realism = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("NInitializationEvents")) != 0) {
    m_NInitializationEvents = aNode->GetValueAsInt();
  }
    
  return true;
}


// MSettingsSivan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
