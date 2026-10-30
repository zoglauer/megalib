/*
 * MSettingsSpectralOptions.cxx
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
// MSettingsMimrec.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsSpectralOptions.h"

// Standard libs:
#include <limits>
using namespace std;
#include <iomanip>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSettingsSpectralOptions)
#endif


///////////////////////////////////////////////////////////////////////////////


MSettingsSpectralOptions::MSettingsSpectralOptions() : MSettingsInterface()
{
  // default constructor

  // Spectral analyzer
  m_SpectralSignaltoNoiseRatio = 3;
  m_SpectralPoissonLimit = 20;
  m_SpectralIsotopeFileName = "$(MEGALIB)/resource/libraries/IsotopeLibrary_HomelandSecurity_Short.isotopes";
  m_SpectralEnergyRange = 1;

}


////////////////////////////////////////////////////////////////////////////////


MSettingsSpectralOptions::~MSettingsSpectralOptions()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsSpectralOptions::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MXmlNode* aNode = 0;

  // Spectral analyzer
  aNode = new MXmlNode(Node, "SpectralAnalyzer");
  new MXmlNode(aNode, "SpectralSignaltoNoiseRatio", m_SpectralSignaltoNoiseRatio);
  new MXmlNode(aNode, "SpectralPoissonLimit", m_SpectralPoissonLimit);  
  new MXmlNode(aNode, "SpectralIsotopeFileName", CleanPath(m_SpectralIsotopeFileName));
  new MXmlNode(aNode, "SpectralEnergyRange", m_SpectralEnergyRange);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsSpectralOptions::ReadXml(MXmlNode* Node)
{
  // Retrieve the content from an XML tree
  
  MXmlNode* aNode = 0;
  MXmlNode* bNode = 0;

  if ((aNode = Node->GetNode("SpectralAnalyzer")) != 0) {
    if ((bNode = aNode->GetNode("SpectralSignaltoNoiseRatio")) != 0) {
      m_SpectralSignaltoNoiseRatio = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("SpectralPoissonLimit")) != 0) {
      m_SpectralPoissonLimit = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("SpectralIsotopeFileName")) != 0) {
      m_SpectralIsotopeFileName = bNode->GetValueAsString();
    }
    if ((bNode = aNode->GetNode("SpectralEnergyRange")) != 0) {
      m_SpectralEnergyRange = bNode->GetValueAsDouble();
    }
  }

  return true;
}


// MSettingsSpectralOptions.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
