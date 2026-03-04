/*
 * MSettingsMimrec.cxx
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
// MSettingsMimrec.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsMimrec.h"

// Standard libs:
#include <limits>
using namespace std;
#include <iomanip>

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MEarthHorizon.h"
#include "MProjection.h"
#include "MLMLAlgorithms.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSettingsMimrec)
#endif


///////////////////////////////////////////////////////////////////////////////


MSettingsMimrec::MSettingsMimrec(bool AutoLoad) : MSettings("MimrecConfigurationFile"), MSettingsEventSelections(), MSettingsImaging(), MSettingsSpectralOptions()
{
  // default constructor

  m_DefaultSettingsFileName = "~/.mimrec.cfg";
  m_SettingsFileName = m_DefaultSettingsFileName;

  // Polarization
  m_HistBinsPolarization = 90;
  m_PolarizationBackgroundFileName = "";
  m_PolarizationArmCut = 10;
  
  if (AutoLoad == true) {
    Read();
  }
}


////////////////////////////////////////////////////////////////////////////////


MSettingsMimrec::~MSettingsMimrec()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsMimrec::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  // Read included objects
  MSettings::WriteXml(Node);
  MSettingsImaging::WriteXml(Node);
  MSettingsEventSelections::WriteXml(Node);
  MSettingsResolutions::WriteXml(Node);
  MSettingsSpectralOptions::WriteXml(Node);

  MXmlNode* aNode = nullptr;

  // Menu Polarization
  aNode = new MXmlNode(Node, "Polarization");
  new MXmlNode(aNode, "Bins", m_HistBinsPolarization);
  new MXmlNode(aNode, "BackgroundFile", MSettings::CleanPath(m_PolarizationBackgroundFileName));
  new MXmlNode(aNode, "ARMCut", m_PolarizationArmCut);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsMimrec::ReadXml(MXmlNode* Node)
{
  // Retrieve the content from an XML tree

  MSettings::ReadXml(Node);
  MSettingsImaging::ReadXml(Node);
  MSettingsEventSelections::ReadXml(Node);
  MSettingsResolutions::ReadXml(Node);
  MSettingsSpectralOptions::ReadXml(Node);
  
  MXmlNode* aNode = nullptr;
  MXmlNode* bNode = nullptr;

  if ((aNode = Node->GetNode("Polarization")) != nullptr) {
    if ((bNode = aNode->GetNode("Bins")) != nullptr) {
      m_HistBinsPolarization = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("BackgroundFile")) != nullptr) {
      m_PolarizationBackgroundFileName = bNode->GetValueAsString(); 
    }
    if ((bNode = aNode->GetNode("ARMCut")) != nullptr) {
      m_PolarizationArmCut = bNode->GetValueAsDouble();
    }
  }

  return true;
}


// MSettingsMimrec.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
