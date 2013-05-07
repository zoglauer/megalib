/*
 * MGUIDataEview.cxx
 *
 *
 * Copyright (C) 1998-2010 by Andreas Zoglauer.
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
// MGUIDataEview.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIDataEview.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIDataEview)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIDataEview::MGUIDataEview()
  : MGUIData()
{
  // default constructor
  m_DefaultFileName = TString(gSystem->ConcatFileName(gSystem->HomeDirectory(),
                                                      ".eview.cfg"));
  m_DataFileName = m_DefaultFileName;

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


  m_MasterNodeName = "EviewConfigurationFile";

  ReadData();
}


////////////////////////////////////////////////////////////////////////////////


MGUIDataEview::~MGUIDataEview()
{
  // default destructor

  SaveData();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDataEview::ReadDataLocal(FILE* Input)
{
  // Load the configuration-file

  const int LineLength = 1000; 
  char LineBuffer[LineLength];
  TString Str;

  // Step two find keywords and the belonging data:
  while (fgets(LineBuffer, LineLength, Input) != NULL) {

    if (strstr(LineBuffer, "[GeometryFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = TString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_GeometryFileName = Str;
    }

    if (strstr(LineBuffer, "[EventFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = TString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_CurrentFile = Str;
    }

    if (strstr(LineBuffer, "[Angle]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_AngleTheta);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_AnglePhi);
    }

    if (strstr(LineBuffer, "[Rotation]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_RotationTheta);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_RotationPhi);
    }

    if (strstr(LineBuffer, "[Position]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_PositionXNeg);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_PositionXPos);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_PositionYNeg);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_PositionYPos);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_PositionZNeg);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_PositionZPos);
    }

    if (strstr(LineBuffer, "[Energy]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_EnergyMin);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_EnergyMax);
    }

    if (strstr(LineBuffer, "[MinHits]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%d\n", &m_MinHitsTotal);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%d\n", &m_MinHitsD1);
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%d\n", &m_MinHitsD2);
    }

    if (strstr(LineBuffer, "[Display]") != NULL) {
      int i = 0;
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &i);
      m_Reconstruct = (bool) i;
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &i);
      m_DisplaySequence = (bool) i;
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &i);
      m_DisplayOrigin = (bool) i;
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &i);
      m_DisplayOnlyGoodEvents = (bool) i;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataEview::SaveDataLocal(MXmlDocument* Document)
{
  // Step two: save the keywords and the belonging data:

  new MXmlNode(Document, "GeometryFile", m_GeometryFileName);
  new MXmlNode(Document, "EventFile", m_CurrentFile);
  new MXmlNode(Document, "AngleTheta", m_AngleTheta);
  new MXmlNode(Document, "AnglePhi", m_AnglePhi);
  new MXmlNode(Document, "RotationTheta", m_RotationTheta);
  new MXmlNode(Document, "RotationPhi", m_RotationPhi);
  new MXmlNode(Document, "PositionXNeg", m_PositionXNeg);
  new MXmlNode(Document, "PositionXPos", m_PositionXPos); 
  new MXmlNode(Document, "PositionYNeg", m_PositionYNeg);
  new MXmlNode(Document, "PositionYPos", m_PositionYPos); 
  new MXmlNode(Document, "PositionZNeg", m_PositionZNeg);
  new MXmlNode(Document, "PositionZPos", m_PositionZPos);
  new MXmlNode(Document, "EnergyMin", m_EnergyMin);
  new MXmlNode(Document, "EnergyMax", m_EnergyMax);
  new MXmlNode(Document, "MinHitsTotal", m_MinHitsTotal); 
  new MXmlNode(Document, "MinHitsD1", m_MinHitsD1);  
  new MXmlNode(Document, "MinHitsD2", m_MinHitsD2);
  new MXmlNode(Document, "DoReconstruction", m_Reconstruct);
  new MXmlNode(Document, "DisplaySequence", m_DisplaySequence); 
  new MXmlNode(Document, "DisplayOrigin", m_DisplayOrigin);
  new MXmlNode(Document, "DisplayOnlyGoodEvents", m_DisplayOnlyGoodEvents);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataEview::ReadDataLocal(MXmlDocument* Document)
{  
  MXmlNode* aNode = 0;
  
  if ((aNode = Document->GetNode("GeometryFile")) != 0) {
    m_GeometryFileName = aNode->GetValueAsString();
  }
  if ((aNode = Document->GetNode("EventFile")) != 0) {
    m_CurrentFile = aNode->GetValueAsString();
  }
  if ((aNode = Document->GetNode("AngleTheta")) != 0) {
    m_AngleTheta = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("AnglePhi")) != 0) {
    m_AnglePhi = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("RotationTheta")) != 0) {
    m_RotationTheta = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("RotationPhi")) != 0) {
    m_RotationPhi = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("PositionXNeg")) != 0) {
    m_PositionXNeg = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("PositionXPos")) != 0) {
    m_PositionXPos = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("PositionYNeg")) != 0) {
    m_PositionYNeg = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("PositionYPos")) != 0) {
    m_PositionYPos = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("PositionZNeg")) != 0) {
    m_PositionZNeg = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("PositionZPos")) != 0) {
    m_PositionZPos = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("EnergyMin")) != 0) {
    m_EnergyMin = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("EnergyMax")) != 0) {
    m_EnergyMax = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("MinHitsTotal")) != 0) {
    m_MinHitsTotal = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("MinHitsD1")) != 0) {
    m_MinHitsD1 = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("MinHitsD2")) != 0) {
    m_MinHitsD2 = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("DoReconstruction")) != 0) {
    m_Reconstruct = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("DisplaySequence")) != 0) {
    m_DisplaySequence = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("DisplayOrigin")) != 0) {
    m_DisplayOrigin = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("DisplayOnlyGoodEvents")) != 0) {
    m_DisplayOnlyGoodEvents = aNode->GetValueAsBoolean();
  }

  return true;
}


// MGUIDataEview.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
