/*
 * MSettingsRevan.cxx
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
// MSettingsRevan.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsRevan.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettingsRevan)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsRevan::MSettingsRevan(bool AutoLoad) : MSettings("RevanConfigurationFile"), MSettingsSpectralOptions()
{
  // default constructor
  m_DefaultSettingsFileName = MString(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".revan.cfg"));
  m_SettingsFileName = m_DefaultSettingsFileName;

  // Options for exporting the spectrum
  m_ExportSpectrumBins = 100;
  m_ExportSpectrumLog = false;
  m_ExportSpectrumMin = 0.0;
  m_ExportSpectrumMax = 1000.0;
  m_ExportSpectrumFileName = "Export.dat";
  
  if (AutoLoad == true) {
    Read();
  }
}

////////////////////////////////////////////////////////////////////////////////


MSettingsRevan::~MSettingsRevan()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsRevan::WriteXml(MXmlNode* Node)
{
  // Write the content to an XML tree

  MSettings::WriteXml(Node);
  MSettingsEventReconstruction::WriteXml(Node);
  MSettingsSpectralOptions::WriteXml(Node);

  new MXmlNode(Node, "ExportSpectrumBins", m_ExportSpectrumBins);
  new MXmlNode(Node, "ExportSpectrumLog", m_ExportSpectrumLog);
  new MXmlNode(Node, "ExportSpectrumMin", m_ExportSpectrumMin);
  new MXmlNode(Node, "ExportSpectrumMax", m_ExportSpectrumMax);
  new MXmlNode(Node, "ExportSpectrumFileName", MSettings::CleanPath(m_ExportSpectrumFileName));

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsRevan::ReadXml(MXmlNode* Node)
{
  // Retrieve the content from an XML tree

  MSettings::ReadXml(Node);
  MSettingsEventReconstruction::ReadXml(Node);
  MSettingsSpectralOptions::ReadXml(Node);

  MXmlNode* aNode = 0;

  if ((aNode = Node->GetNode("ExportSpectrumBins")) != 0) {
    m_ExportSpectrumBins = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("ExportSpectrumLog")) != 0) {
    m_ExportSpectrumLog = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("ExportSpectrumMin")) != 0) {
    m_ExportSpectrumMin = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("ExportSpectrumMax")) != 0) {
    m_ExportSpectrumMax = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("ExportSpectrumFileName")) != 0) {
    m_ExportSpectrumFileName = aNode->GetValueAsString();
  }

  return true;
}
  

// MSettingsRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
