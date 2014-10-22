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


#ifdef ___CINT___
ClassImp(MSettingsMimrec)
#endif


///////////////////////////////////////////////////////////////////////////////


MSettingsMimrec::MSettingsMimrec(bool AutoLoad) : MSettings("MimrecConfigurationFile"), MSettingsEventSelections(), MSettingsImaging(), MSettingsSpectralOptions()
{
  // default constructor

  m_DefaultSettingsFileName = MString(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".mimrec.cfg"));
  m_SettingsFileName = m_DefaultSettingsFileName;


  // Test positions for ARM's etc.
  m_TPTheta = 0;
  m_TPPhi = 180;
  m_TPGalLongitude = 0;
  m_TPGalLatitude = 180;
  m_TPX = 0;
  m_TPY = 0;
  m_TPZ = 100;
  m_TPDistanceTrans = 10;
  m_TPDistanceLong = 30;

  // Bin sizes for certain histograms:
  m_HistBinsARMGamma = 181;
  m_HistBinsARMElectron = 181;
  m_HistBinsSpectrum = 200;
  m_HistBinsPolarization = 90;

  // Polarization
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

  MSettings::WriteXml(Node);
  MSettingsImaging::WriteXml(Node);
  MSettingsEventSelections::WriteXml(Node);
  MSettingsSpectralOptions::WriteXml(Node);

  MXmlNode* aNode = 0;
  MXmlNode* bNode = 0;


  // Menu Response
  ////////////////

  // Menu ARM Gamma
  aNode = new MXmlNode(Node, "TestPositions");
  bNode = new MXmlNode(aNode, "CoordinateSystemSpherical");
  new MXmlNode(bNode, "Theta", m_TPTheta);
  new MXmlNode(bNode, "Phi", m_TPPhi);
  bNode = new MXmlNode(aNode, "CoordinateSystemGalactic");
  new MXmlNode(bNode, "Longitude", m_TPGalLongitude);
  new MXmlNode(bNode, "Latitude", m_TPGalLatitude);
  bNode = new MXmlNode(aNode, "CoordinateSystemCartesian");
  new MXmlNode(bNode, "X", m_TPX);
  new MXmlNode(bNode, "Y", m_TPY);
  new MXmlNode(bNode, "Z", m_TPZ);
  new MXmlNode(aNode, "DistanceTrans", m_TPDistanceTrans);
  new MXmlNode(aNode, "DistanceLong", m_TPDistanceLong);

  // Bin sizes for certain histograms
  aNode = new MXmlNode(Node, "HistogramBins");
  new MXmlNode(aNode, "ARMGamma", m_HistBinsARMGamma);
  new MXmlNode(aNode, "ARMElectron", m_HistBinsARMElectron);
  new MXmlNode(aNode, "Spectrum", m_HistBinsSpectrum);
  new MXmlNode(aNode, "Polarization", m_HistBinsPolarization);

  // Menu Polarization
  aNode = new MXmlNode(Node, "Polarization");
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
  MSettingsSpectralOptions::ReadXml(Node);
  
  MXmlNode* aNode = 0;
  MXmlNode* bNode = 0;
  MXmlNode* cNode = 0;


  if ((aNode = Node->GetNode("TestPositions")) != 0) {
    if ((bNode = aNode->GetNode("CoordinateSystemSpherical")) != 0) {
      if ((cNode = bNode->GetNode("Theta")) != 0) {
        m_TPTheta = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("Phi")) != 0) {
        m_TPPhi = cNode->GetValueAsDouble();
      }
    }
    if ((bNode = aNode->GetNode("CoordinateSystemGalactic")) != 0) {
      if ((cNode = bNode->GetNode("Longitude")) != 0) {
        m_TPGalLongitude = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("Latitude")) != 0) {
        m_TPGalLatitude = cNode->GetValueAsDouble();
      }   
    }
    if ((bNode = aNode->GetNode("CoordinateSystemCartesian")) != 0) {
      if ((cNode = bNode->GetNode("X")) != 0) {
        m_TPX = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("Y")) != 0) {
        m_TPY = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("Z")) != 0) {
        m_TPZ = cNode->GetValueAsDouble();
      }        
    }
    if ((bNode = aNode->GetNode("DistanceTrans")) != 0) {
      m_TPDistanceTrans = bNode->GetValueAsDouble();
    }
    if ((bNode = aNode->GetNode("DistanceLong")) != 0) {
      m_TPDistanceLong = bNode->GetValueAsDouble();
    }
  }


  if ((aNode = Node->GetNode("HistogramBins")) != 0) {
    if ((bNode = aNode->GetNode("ARMGamma")) != 0) {
      m_HistBinsARMGamma = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("ARMElectron")) != 0) {
      m_HistBinsARMElectron = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("Spectrum")) != 0) {
      m_HistBinsSpectrum = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("Polarization")) != 0) {
      m_HistBinsPolarization = bNode->GetValueAsInt();
    }
  }


  if ((aNode = Node->GetNode("Polarization")) != 0) {
    if ((bNode = aNode->GetNode("BackgroundFile")) != 0) {
      m_PolarizationBackgroundFileName = bNode->GetValueAsString(); 
    }
    if ((bNode = aNode->GetNode("ARMCut")) != 0) {
      m_PolarizationArmCut = bNode->GetValueAsDouble();
    }
  }
    

  return true;
}


// MSettingsMimrec.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
