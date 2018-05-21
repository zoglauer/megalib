/*
 * MSettingsSpectralyzer.cxx
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
