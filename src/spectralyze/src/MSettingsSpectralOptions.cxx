/*
 * MSettingsSpectralOptions.cxx
 *
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Michelle Galloway & Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
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


#ifdef ___CINT___
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
