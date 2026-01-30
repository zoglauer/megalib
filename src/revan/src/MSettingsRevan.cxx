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


#ifdef ___CLING___
ClassImp(MSettingsRevan)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsRevan::MSettingsRevan(bool AutoLoad) : MSettings("RevanConfigurationFile"), MSettingsSpectralOptions()
{
  // default constructor
  m_DefaultSettingsFileName = "~/.revan.cfg";
  m_SettingsFileName = m_DefaultSettingsFileName;

  // Options for exporting the spectrum
  m_ExportSpectrumBins = 100;
  m_ExportSpectrumLog = false;
  m_ExportSpectrumMin = 0.0;
  m_ExportSpectrumMax = 1000.0;
  m_ExportSpectrumFileName = "Export.dat";
  
  // Options for displaying the spectrum
  m_SpectrumBefore = true;
  m_SpectrumAfter = false;
  
  m_SpectrumSortByInstrument = true;
  m_SpectrumSortByDetectorType = false;
  m_SpectrumSortByNamedDetector = false;
  m_SpectrumSortByDetector = false;
  
  m_SpectrumCombine = true;
  
  m_SpectrumTotalDeposit = false;

  m_SpectrumOutputToScreen = true;
  m_SpectrumOutputToFile = false;
  
  m_SpectrumBins = 100;
  m_SpectrumLog = false;
  m_SpectrumMin = 0.0;
  m_SpectrumMax = 1000.0;  
  
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

  new MXmlNode(Node, "SpectrumBefore)", m_SpectrumBefore);
  new MXmlNode(Node, "SpectrumAfter", m_SpectrumAfter);
  
  new MXmlNode(Node, "SpectrumSortByInstrument", m_SpectrumSortByInstrument);
  new MXmlNode(Node, "SpectrumSortByDetectorType", m_SpectrumSortByDetectorType);
  new MXmlNode(Node, "SpectrumSortByNamedDetector", m_SpectrumSortByNamedDetector);
  new MXmlNode(Node, "SpectrumSortByDetector", m_SpectrumSortByDetector);

  new MXmlNode(Node, "SpectrumCombine", m_SpectrumCombine);

  new MXmlNode(Node, "SpectrumTotalDeposit", m_SpectrumTotalDeposit);

  new MXmlNode(Node, "SpectrumOutputToScreen", m_SpectrumOutputToScreen);
  new MXmlNode(Node, "SpectrumOutputToFile", m_SpectrumOutputToFile);

  new MXmlNode(Node, "SpectrumBins", m_SpectrumBins);
  new MXmlNode(Node, "SpectrumLog", m_SpectrumLog);
  new MXmlNode(Node, "SpectrumMin", m_SpectrumMin);
  new MXmlNode(Node, "SpectrumMax", m_SpectrumMax);

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

  
  if ((aNode = Node->GetNode("SpectrumBefore")) != 0) {
    m_SpectrumBefore = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SpectrumAfter")) != 0) {
    m_SpectrumAfter = aNode->GetValueAsBoolean();
  }
  
  if ((aNode = Node->GetNode("SpectrumSortByInstrument")) != 0) {
    m_SpectrumSortByInstrument = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SpectrumSortByDetectorType")) != 0) {
    m_SpectrumSortByDetectorType = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SpectrumSortByNamedDetector")) != 0) {
    m_SpectrumSortByNamedDetector = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SpectrumSortByDetector")) != 0) {
    m_SpectrumSortByDetector = aNode->GetValueAsBoolean();
  }
  
  if ((aNode = Node->GetNode("SpectrumOutputToScreen")) != 0) {
    m_SpectrumOutputToScreen = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SpectrumOutputToFile")) != 0) {
    m_SpectrumOutputToFile = aNode->GetValueAsBoolean();
  }

  if ((aNode = Node->GetNode("SpectrumCombine")) != 0) {
    m_SpectrumCombine = aNode->GetValueAsBoolean();
  }

  if ((aNode = Node->GetNode("SpectrumTotalDeposit")) != 0) {
    m_SpectrumTotalDeposit = aNode->GetValueAsBoolean();
  }

  if ((aNode = Node->GetNode("SpectrumBins")) != 0) {
    m_SpectrumBins = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("SpectrumLog")) != 0) {
    m_SpectrumLog = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SpectrumMin")) != 0) {
    m_SpectrumMin = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("SpectrumMax")) != 0) {
    m_SpectrumMax = aNode->GetValueAsDouble();
  }

  return true;
}
  

// MSettingsRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
