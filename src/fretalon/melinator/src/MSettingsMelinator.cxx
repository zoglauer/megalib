/*
 * MSettingsMelinator.cxx
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
// MSettingsMelinator.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsMelinator.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettingsMelinator)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The default constructor
MSettingsMelinator::MSettingsMelinator(bool AutoLoad) : MSettings("MelinatorConfigurationFile")
{
  m_DefaultSettingsFileName = MString(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".melinator.cfg"));
  m_SettingsFileName = m_DefaultSettingsFileName;
  
  m_CalibrationFile1 = "";
  m_CalibrationFile1Isotope1 = "None";
  m_CalibrationFile1Isotope2 = "None";
  m_CalibrationFile1Isotope3 = "None";
  
  m_CalibrationFile2 = "";
  m_CalibrationFile2Isotope1 = "None";
  m_CalibrationFile2Isotope2 = "None";
  m_CalibrationFile2Isotope3 = "None";
  
  m_CalibrationFile3 = "";
  m_CalibrationFile3Isotope1 = "None";
  m_CalibrationFile3Isotope2 = "None";
  m_CalibrationFile3Isotope3 = "None";
  
  m_CalibrationFile4 = "";
  m_CalibrationFile4Isotope1 = "None";
  m_CalibrationFile4Isotope2 = "None";
  m_CalibrationFile4Isotope3 = "None";
  
  m_CalibrationFile5 = "";
  m_CalibrationFile5Isotope1 = "None";
  m_CalibrationFile5Isotope2 = "None";
  m_CalibrationFile5Isotope3 = "None";
  
  m_HistogramMin = 0;
  m_HistogramMax = 1024;
  m_HistogramBinningMode = 0;
  m_HistogramBinningModeValue = 100;
  m_HistogramLogX = false;
  m_HistogramLogY = false;

  m_PeakHistogramBinningMode = 0;
  m_PeakHistogramBinningModeValue = 100;

  m_PeakParametrizationMethod = 0;

  m_SaveAsFileName = "Out.ecal";
  
  if (AutoLoad == true) {
    Read();
  }
}


////////////////////////////////////////////////////////////////////////////////


MSettingsMelinator::~MSettingsMelinator()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsMelinator::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MSettings::WriteXml(Node);

  new MXmlNode(Node, "CalibrationFile1", m_CalibrationFile1);
  new MXmlNode(Node, "CalibrationFile1Isotope1", m_CalibrationFile1Isotope1);
  new MXmlNode(Node, "CalibrationFile1Isotope2", m_CalibrationFile1Isotope2);
  new MXmlNode(Node, "CalibrationFile1Isotope3", m_CalibrationFile1Isotope3);

  new MXmlNode(Node, "CalibrationFile2", m_CalibrationFile2);
  new MXmlNode(Node, "CalibrationFile2Isotope1", m_CalibrationFile2Isotope1);
  new MXmlNode(Node, "CalibrationFile2Isotope2", m_CalibrationFile2Isotope2);
  new MXmlNode(Node, "CalibrationFile2Isotope3", m_CalibrationFile2Isotope3);

  new MXmlNode(Node, "CalibrationFile3", m_CalibrationFile3);
  new MXmlNode(Node, "CalibrationFile3Isotope1", m_CalibrationFile3Isotope1);
  new MXmlNode(Node, "CalibrationFile3Isotope2", m_CalibrationFile3Isotope2);
  new MXmlNode(Node, "CalibrationFile3Isotope3", m_CalibrationFile3Isotope3);

  new MXmlNode(Node, "CalibrationFile4", m_CalibrationFile4);
  new MXmlNode(Node, "CalibrationFile4Isotope1", m_CalibrationFile4Isotope1);
  new MXmlNode(Node, "CalibrationFile4Isotope2", m_CalibrationFile4Isotope2);
  new MXmlNode(Node, "CalibrationFile4Isotope3", m_CalibrationFile4Isotope3);

  new MXmlNode(Node, "CalibrationFile5", m_CalibrationFile5);
  new MXmlNode(Node, "CalibrationFile5Isotope1", m_CalibrationFile5Isotope1);
  new MXmlNode(Node, "CalibrationFile5Isotope2", m_CalibrationFile5Isotope2);
  new MXmlNode(Node, "CalibrationFile5Isotope3", m_CalibrationFile5Isotope3);
  
  new MXmlNode(Node, "HistogramMin", m_HistogramMin);
  new MXmlNode(Node, "HistogramMax", m_HistogramMax);
  new MXmlNode(Node, "HistogramBinningMode", m_HistogramBinningMode);
  new MXmlNode(Node, "HistogramBinningModeValue", m_HistogramBinningModeValue);
  new MXmlNode(Node, "HistogramLogX", m_HistogramLogX);
  new MXmlNode(Node, "HistogramLogY", m_HistogramLogY);

  new MXmlNode(Node, "PeakHistogramBinningMode", m_PeakHistogramBinningMode);
  new MXmlNode(Node, "PeakHistogramBinningModeValue", m_PeakHistogramBinningModeValue);

  new MXmlNode(Node, "PeakParametrizationMethod", m_PeakParametrizationMethod);

  new MXmlNode(Node, "SaveAsFileName", m_SaveAsFileName);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsMelinator::ReadXml(MXmlNode* Node)
{  
  // Retrieve the content from an XML tree

  MSettings::ReadXml(Node);
  
  MXmlNode* aNode = 0;

  if ((aNode = Node->GetNode("CalibrationFile1")) != 0) {
    m_CalibrationFile1 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile1Isotope1")) != 0) {
    m_CalibrationFile1Isotope1 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile1Isotope2")) != 0) {
    m_CalibrationFile1Isotope2 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile1Isotope3")) != 0) {
    m_CalibrationFile1Isotope3 = aNode->GetValueAsString();
  }


  if ((aNode = Node->GetNode("CalibrationFile2")) != 0) {
    m_CalibrationFile2 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile2Isotope1")) != 0) {
    m_CalibrationFile2Isotope1 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile2Isotope2")) != 0) {
    m_CalibrationFile2Isotope2 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile2Isotope3")) != 0) {
    m_CalibrationFile2Isotope3 = aNode->GetValueAsString();
  }


  if ((aNode = Node->GetNode("CalibrationFile3")) != 0) {
    m_CalibrationFile3 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile3Isotope1")) != 0) {
    m_CalibrationFile3Isotope1 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile3Isotope2")) != 0) {
    m_CalibrationFile3Isotope2 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile3Isotope3")) != 0) {
    m_CalibrationFile3Isotope3 = aNode->GetValueAsString();
  }


  if ((aNode = Node->GetNode("CalibrationFile4")) != 0) {
    m_CalibrationFile4 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile4Isotope1")) != 0) {
    m_CalibrationFile4Isotope1 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile4Isotope2")) != 0) {
    m_CalibrationFile4Isotope2 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile4Isotope3")) != 0) {
    m_CalibrationFile4Isotope3 = aNode->GetValueAsString();
  }


  if ((aNode = Node->GetNode("CalibrationFile5")) != 0) {
    m_CalibrationFile5 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile5Isotope1")) != 0) {
    m_CalibrationFile5Isotope1 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile5Isotope2")) != 0) {
    m_CalibrationFile5Isotope2 = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("CalibrationFile5Isotope3")) != 0) {
    m_CalibrationFile5Isotope3 = aNode->GetValueAsString();
  }

  if ((aNode = Node->GetNode("HistogramMin")) != 0) {
    m_HistogramMin = aNode->GetValueAsDouble();;
  }
  if ((aNode = Node->GetNode("HistogramMax")) != 0) {
    m_HistogramMax = aNode->GetValueAsDouble();;
  }
  if ((aNode = Node->GetNode("HistogramBinningMode")) != 0) {
    m_HistogramBinningMode = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("HistogramBinningModeValue")) != 0) {
    m_HistogramBinningModeValue = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("HistogramLogX")) != 0) {
    m_HistogramLogX = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("HistogramLogY")) != 0) {
    m_HistogramLogY = aNode->GetValueAsBoolean();
  }
 
  if ((aNode = Node->GetNode("PeakHistogramBinningMode")) != 0) {
    m_PeakHistogramBinningMode = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("PeakHistogramBinningModeValue")) != 0) {
    m_PeakHistogramBinningModeValue = aNode->GetValueAsDouble();
  }

  if ((aNode = Node->GetNode("PeakParametrizationMethod")) != 0) {
    m_PeakParametrizationMethod = aNode->GetValueAsUnsignedInt();
  }

  if ((aNode = Node->GetNode("SaveAsFileName")) != 0) {
    m_SaveAsFileName = aNode->GetValueAsString();
  }
 
  return true;
}


// MSettingsMelinator.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
