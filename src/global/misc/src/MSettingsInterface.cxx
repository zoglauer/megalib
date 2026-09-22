/*
 * MSettingsInterface.cxx
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
// MSettingsInterface.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsInterface.h"

// Standard libs:

// ROOT libs:

// MEGAlib:
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSettingsInterface)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsInterface::MSettingsInterface() : m_ModificationLevel(0)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MSettingsInterface::~MSettingsInterface()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MSettingsInterface::Modify(int m)
{
  // Modify the modification level

  if (m_ModificationLevel < m) {
    m_ModificationLevel = m;
  }
}


////////////////////////////////////////////////////////////////////////////////


int MSettingsInterface::GetModificationLevel(bool Reset) 
{
  // Return the modification level of the file

  int Level = m_ModificationLevel;
  if (Reset == true) {
    m_ModificationLevel = 0;
  }
    
  return Level;
}


////////////////////////////////////////////////////////////////////////////////


MString MSettingsInterface::CleanPath(MString Path)
{
  //! Clean the path, i.e. exchange absolute with relative path $(MEGALIB)
  
  vector<MString> AllToBeReplaced;
  AllToBeReplaced.push_back("$(MEGALIB)");
  //AllToBeReplaced.push_back("$(Nuclearizer)");
  
  vector<MString> Additions;
  Additions.push_back("");
  Additions.push_back("_trunk");
  Additions.push_back("_master");
  Additions.push_back("_github");
  Additions.push_back("_github_master");
  Additions.push_back("_github_experimental");
  
  for (MString ToBeReplaced: AllToBeReplaced) {
    if (ToBeReplaced == "" || ToBeReplaced == "/") return Path;

    MString Expanded = ToBeReplaced;
    MFile::ExpandFileName(Expanded);
  
    for (MString Add: Additions) {
      Path = Path.ReplaceAll(Expanded + Add + "/", ToBeReplaced + "/");
    }
  }

  return Path;  
}


// MSettingsInterface.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
