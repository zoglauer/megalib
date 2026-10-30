/*
 * MSettingsSpectralyzer.cxx
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
// MSettingsSpectralyzer.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsSpectralyzer.h"

// Standard libs:
#include <limits>
using namespace std;
#include <iomanip>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSettingsSpectralyzer)
#endif


///////////////////////////////////////////////////////////////////////////////


MSettingsSpectralyzer::MSettingsSpectralyzer() : MSettings("SpectralyzerConfigurationFile")
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MSettingsSpectralyzer::~MSettingsSpectralyzer()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsSpectralyzer::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MSettings::WriteXml(Node);
  MSettingsSpectralOptions::WriteXml(Node);


  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsSpectralyzer::ReadXml(MXmlNode* Node)
{
  // Retrieve the content from an XML tree

  MSettings::ReadXml(Node);
  MSettingsSpectralOptions::ReadXml(Node);
  


  return true;
}


// MSettingsSpectralyzer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
