/*
 * MSettingsEview.cxx
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
// MSettingsEview.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsEview.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettingsEview)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsEview::MSettingsEview(bool AutoLoad) : MSettings("EviewConfigurationFile")
{
  // default constructor

  m_DefaultSettingsFileName = MString(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".eview.cfg"));
  m_SettingsFileName = m_DefaultSettingsFileName;

  
  m_AngleTheta = 0;
  m_AnglePhi = 0;
  m_RotationTheta = 5;
  m_RotationPhi = 5;

  m_Reconstruct = false;
  m_DisplaySequence = false;
  m_DisplayOrigin = false;
  m_DisplayOnlyGoodEvents = false;

  m_PositionXNeg = -15;
  m_PositionYNeg = -15;
  m_PositionZNeg = 0;
  m_PositionXPos = 15;
  m_PositionYPos = 15;
  m_PositionZPos = 30;

  m_EnergyMin = 0;
  m_EnergyMax = 10000;

  m_MinHitsTotal = 0;
  m_MinHitsD1 = 0;
  m_MinHitsD2 = 0;
  
  
  if (AutoLoad == true) {
    Read();
  }
}


////////////////////////////////////////////////////////////////////////////////


MSettingsEview::~MSettingsEview()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsEview::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MSettings::WriteXml(Node);
  MSettingsEventReconstruction::WriteXml(Node);

  new MXmlNode(Node, "AngleTheta", m_AngleTheta);
  new MXmlNode(Node, "AnglePhi", m_AnglePhi);
  new MXmlNode(Node, "RotationTheta", m_RotationTheta);
  new MXmlNode(Node, "RotationPhi", m_RotationPhi);
  new MXmlNode(Node, "PositionXNeg", m_PositionXNeg);
  new MXmlNode(Node, "PositionXPos", m_PositionXPos); 
  new MXmlNode(Node, "PositionYNeg", m_PositionYNeg);
  new MXmlNode(Node, "PositionYPos", m_PositionYPos); 
  new MXmlNode(Node, "PositionZNeg", m_PositionZNeg);
  new MXmlNode(Node, "PositionZPos", m_PositionZPos);
  new MXmlNode(Node, "EnergyMin", m_EnergyMin);
  new MXmlNode(Node, "EnergyMax", m_EnergyMax);
  new MXmlNode(Node, "MinHitsTotal", m_MinHitsTotal); 
  new MXmlNode(Node, "MinHitsD1", m_MinHitsD1);  
  new MXmlNode(Node, "MinHitsD2", m_MinHitsD2);
  new MXmlNode(Node, "DoReconstruction", m_Reconstruct);
  new MXmlNode(Node, "DisplaySequence", m_DisplaySequence); 
  new MXmlNode(Node, "DisplayOrigin", m_DisplayOrigin);
  new MXmlNode(Node, "DisplayOnlyGoodEvents", m_DisplayOnlyGoodEvents);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsEview::ReadXml(MXmlNode* Node)
{  
  // Retrieve the content from an XML tree

  MSettings::ReadXml(Node);
  MSettingsEventReconstruction::ReadXml(Node);
  
  MXmlNode* aNode = 0;
  
  if ((aNode = Node->GetNode("AngleTheta")) != 0) {
    m_AngleTheta = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("AnglePhi")) != 0) {
    m_AnglePhi = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("RotationTheta")) != 0) {
    m_RotationTheta = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("RotationPhi")) != 0) {
    m_RotationPhi = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("PositionXNeg")) != 0) {
    m_PositionXNeg = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("PositionXPos")) != 0) {
    m_PositionXPos = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("PositionYNeg")) != 0) {
    m_PositionYNeg = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("PositionYPos")) != 0) {
    m_PositionYPos = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("PositionZNeg")) != 0) {
    m_PositionZNeg = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("PositionZPos")) != 0) {
    m_PositionZPos = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("EnergyMin")) != 0) {
    m_EnergyMin = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("EnergyMax")) != 0) {
    m_EnergyMax = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("MinHitsTotal")) != 0) {
    m_MinHitsTotal = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("MinHitsD1")) != 0) {
    m_MinHitsD1 = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("MinHitsD2")) != 0) {
    m_MinHitsD2 = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("DoReconstruction")) != 0) {
    m_Reconstruct = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("DisplaySequence")) != 0) {
    m_DisplaySequence = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("DisplayOrigin")) != 0) {
    m_DisplayOrigin = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("DisplayOnlyGoodEvents")) != 0) {
    m_DisplayOnlyGoodEvents = aNode->GetValueAsBoolean();
  }

  return true;
}


// MSettingsEview.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
