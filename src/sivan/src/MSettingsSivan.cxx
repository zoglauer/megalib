/*
 * MSettingsSivan.cxx
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


#ifdef ___CLING___
ClassImp(MSettingsSivan)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsSivan::MSettingsSivan(bool AutoLoad) : MSettings("SivanConfigurationFile")
{
  // default constructor

  m_DefaultSettingsFileName = "~/.sivan.cfg";
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
