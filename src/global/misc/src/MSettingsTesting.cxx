/*
 * MSettingsTesting.cxx
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


// Include the header:
#include "MSettingsTesting.h"

// Standard libs:
#include <cstdlib>

// POSIX libs:
#include <unistd.h>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSettingsTesting)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor with the default settings (the settings file is not read)
MSettingsTesting::MSettingsTesting() : MSettings("TestingConfigurationFile")
{
  // Create the settings with their defaults

  m_DefaultSettingsFileName = "~/.testdrive.cfg";
  m_SettingsFileName = m_DefaultSettingsFileName;

  m_LogDirectory = GetDefaultLogDirectory();
  m_Timeout = 120.0;
  m_MachineId = "";
  m_MachineSlowdown = 1.0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MSettingsTesting::~MSettingsTesting()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


//! Read all data from an XML tree
bool MSettingsTesting::ReadXml(MXmlNode* Node)
{
  // Read all data from an XML tree

  MSettings::ReadXml(Node);

  MXmlNode* aNode = 0;
  if ((aNode = Node->GetNode("LogDirectory")) != 0) {
    if (aNode->GetValueAsString().IsEmpty() == false) {
      m_LogDirectory = aNode->GetValueAsString();
    }
  }

  if ((aNode = Node->GetNode("Timeout")) != 0) {
    if (aNode->GetValueAsDouble() >= 0.0) {
      m_Timeout = aNode->GetValueAsDouble();
    }
  }

  if ((aNode = Node->GetNode("MachineId")) != 0) {
    m_MachineId = aNode->GetValueAsString();
  }

  if ((aNode = Node->GetNode("MachineSlowdown")) != 0) {
    if (aNode->GetValueAsDouble() > 0.0) {
      m_MachineSlowdown = aNode->GetValueAsDouble();
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Write all data to an XML tree
bool MSettingsTesting::WriteXml(MXmlNode* Node)
{
  // Write all data to an XML tree

  MSettings::WriteXml(Node);

  new MXmlNode(Node, "LogDirectory", m_LogDirectory);
  new MXmlNode(Node, "Timeout", m_Timeout);
  new MXmlNode(Node, "MachineId", m_MachineId);
  new MXmlNode(Node, "MachineSlowdown", m_MachineSlowdown);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the default log directory /tmp/$USER/megalib_testing_logs
MString MSettingsTesting::GetDefaultLogDirectory() const
{
  // Return the default log directory

  const char* User = getenv("USER");
  if (User == nullptr || User[0] == '\0') {
    User = getenv("LOGNAME");
  }
  MString Name;
  if (User != nullptr && User[0] != '\0') {
    Name = User;
  } else {
    Name = MString(static_cast<long>(getuid()));
  }
  return MString("/tmp/") + Name + "/megalib_testing_logs";
}


// MSettingsTesting.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
