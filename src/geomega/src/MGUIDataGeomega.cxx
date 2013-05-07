/*
 * MGUIDataGeomega.cxx
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
// MGUIDataGeomega.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIDataGeomega.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIDataGeomega)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIDataGeomega::MGUIDataGeomega() : MGUIData()
{
  // default constructor
  m_DefaultFileName = TString(gSystem->ConcatFileName(gSystem->HomeDirectory(),
                                                      ".geomega.cfg"));
  m_DataFileName = m_DefaultFileName;

  m_MGeantOutputMode = 0;
  m_MGeantFileName = "MGeantGeo";

  m_StoreIAs = false;
  m_StoreVetoes = true;

  m_MasterNodeName = "GeomegaConfigurationFile";

  ReadData();
}


////////////////////////////////////////////////////////////////////////////////


MGUIDataGeomega::~MGUIDataGeomega()
{
  // default destructor

  SaveData();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDataGeomega::ReadDataLocal(FILE* Input)
{
  // Load the configuration-file

  const int LineLength = 1000;
  char LineBuffer[LineLength];
  TString Str;

  // Step two find keywords and the belonging data:
  while (fgets(LineBuffer, LineLength, Input) != NULL) {

    if (strstr(LineBuffer, "[MGeantOutputMode]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_MGeantOutputMode);
    }

    if (strstr(LineBuffer, "[MGeantFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = TString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_MGeantFileName = Str;
    }

    if (strstr(LineBuffer, "[Position]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      double x, y, z;
      sscanf(LineBuffer, "%lf %lf %lf\n", &x, &y, &z);
      m_Position.SetXYZ(x, y, z);
    }
  
    if (strstr(LineBuffer, "[StoreIAs]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_StoreIAs = false : m_StoreIAs = true;
    }
  
    if (strstr(LineBuffer, "[StoreVetoes]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_StoreVetoes = false : m_StoreVetoes = true;
    }

  } 
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataGeomega::SaveDataLocal(MXmlDocument* Document)
{
  // Step two: save the keywords and the belonging data:

  new MXmlNode(Document, "MGeantOutputMode", m_MGeantOutputMode);
  new MXmlNode(Document, "MGeantFile", m_MGeantFileName);
  new MXmlNode(Document, "Position",  m_Position);
  new MXmlNode(Document, "StoreIAs", m_StoreIAs);
  new MXmlNode(Document, "StoreVetoes", m_StoreVetoes);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataGeomega::ReadDataLocal(MXmlDocument* Document)
{  
  MXmlNode* aNode = 0;
  
  if ((aNode = Document->GetNode("MGeantOutputMode")) != 0) {
    m_MGeantOutputMode = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("MGeantFile")) != 0) {
    m_MGeantFileName = aNode->GetValueAsString();
  }
  if ((aNode = Document->GetNode("Position")) != 0) {
    m_Position = aNode->GetValueAsVector();
  }
  if ((aNode = Document->GetNode("StoreIAs")) != 0) {
    m_StoreIAs = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("StoreVetoes")) != 0) {
    m_StoreVetoes = aNode->GetValueAsBoolean();
  }

  return true;
}


// MGUIDataGeomega.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
