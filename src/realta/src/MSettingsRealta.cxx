/*
 * MSettingsRealta.cxx
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
// MSettingsRealta.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsRealta.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettingsRealta)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsRealta::MSettingsRealta(bool AutoLoad) : MSettings("RealtaConfigurationFile"), MSettingsEventSelections(), MSettingsImaging(), MSettingsEventReconstruction(), MSettingsSpectralOptions()
{
  // Default constructor
  
  m_DefaultSettingsFileName = MString(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".realta.cfg"));
  m_SettingsFileName = m_DefaultSettingsFileName;
  
  m_HostName = "localhost";
  m_Port = 9090;
  m_ConnectOnStart = false;
  
  m_AccumulationTime = 100; //sec
  m_AccumulationFileName = "";
  m_AccumulationFileNameAddDateAndTime = true;
  
  m_BinsCountRate = 10;
  m_BinsSpectrum = 50;
  m_MinimumSpectrum = 0;
  m_MaximumSpectrum = 2000;
  
  m_TransceiverMode = 0;
  
  m_DoCoincidence = false;
  m_DoIdentification = false;

  if (AutoLoad == true) {
    Read();
  }
}


////////////////////////////////////////////////////////////////////////////////


MSettingsRealta::~MSettingsRealta()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsRealta::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MSettings::WriteXml(Node);
  MSettingsEventSelections::WriteXml(Node);
  MSettingsImaging::WriteXml(Node);
  MSettingsEventReconstruction::WriteXml(Node);
  MSettingsSpectralOptions::WriteXml(Node);

  new MXmlNode(Node, "HostName", m_HostName);
  new MXmlNode(Node, "Port", m_Port);
  new MXmlNode(Node, "ConnectOnStart", m_ConnectOnStart);
  new MXmlNode(Node, "TransceiverMode", m_TransceiverMode);

  new MXmlNode(Node, "DoCoincidence", m_DoCoincidence);
  new MXmlNode(Node, "DoIdentification", m_DoIdentification);
  
  new MXmlNode(Node, "AccumulationTime", m_AccumulationTime);
  new MXmlNode(Node, "AccumulationFileName", MSettings::CleanPath(m_AccumulationFileName));
  new MXmlNode(Node, "AccumulationFileNameAddDateAndTime", m_AccumulationFileNameAddDateAndTime);

  new MXmlNode(Node, "BinsCountRate", m_BinsCountRate);
  new MXmlNode(Node, "BinsSpectrum", m_BinsSpectrum);
  new MXmlNode(Node, "MinimumSpectrum", m_MinimumSpectrum);
  new MXmlNode(Node, "MaximumSpectrum", m_MaximumSpectrum);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsRealta::ReadXml(MXmlNode* Node)
{  
   // Read content from an XML tree

  MSettings::ReadXml(Node);
  MSettingsEventSelections::ReadXml(Node);
  MSettingsImaging::ReadXml(Node);
  MSettingsEventReconstruction::ReadXml(Node);
  MSettingsSpectralOptions::ReadXml(Node);

  MXmlNode* aNode = 0;
  if ((aNode = Node->GetNode("HostName")) != 0) {
    m_HostName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("Port")) != 0) {
    m_Port = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("ConnectOnStart")) != 0) {
    m_ConnectOnStart = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("DoIdentification")) != 0) {
    m_DoIdentification = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("DoCoincidence")) != 0) {
    m_DoCoincidence = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("TransceiverMode")) != 0) {
    m_TransceiverMode = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("AccumulationTime")) != 0) {
    m_AccumulationTime = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("AccumulationFileName")) != 0) {
    m_AccumulationFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("AccumulationFileNameAddDateAndTime")) != 0) {
    m_AccumulationFileNameAddDateAndTime = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("BinsCountRate")) != 0) {
    m_BinsCountRate = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("BinsSpectrum")) != 0) {
    m_BinsSpectrum = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("MinimumSpectrum")) != 0) {
    m_MinimumSpectrum = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("MaximumSpectrum")) != 0) {
    m_MaximumSpectrum = aNode->GetValueAsDouble();
  }
    
  return true;
}


// MSettingsRealta.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

