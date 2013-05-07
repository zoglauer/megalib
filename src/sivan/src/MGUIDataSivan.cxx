/*
 * MGUIDataSivan.cxx
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
// MGUIDataSivan.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIDataSivan.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIDataSivan)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIDataSivan::MGUIDataSivan()
  : MGUIData()
{
  // default constructor
  m_DefaultFileName = TString(gSystem->ConcatFileName(gSystem->HomeDirectory(),
                                                      ".sivan.cfg"));
  m_DataFileName = m_DefaultFileName;

  m_Realism = 2;
  m_NInitializationEvents = 2500;

  m_MasterNodeName = "SivanConfigurationFile";

  ReadData();
}


////////////////////////////////////////////////////////////////////////////////


MGUIDataSivan::~MGUIDataSivan()
{
  // default destructor

  SaveData();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDataSivan::SetRealism(const int Realism)
{
  // Set the realism of the simulated event analysis:
  // 0: ideal data
  // 1: 
  // 2: noised data

  m_Realism = Realism;
}


////////////////////////////////////////////////////////////////////////////////


int MGUIDataSivan::GetRealism() const
{
  // Return the realism of the simulated event analysis:

  return m_Realism;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDataSivan::ReadDataLocal(FILE* Input)
{
  // Load the configuration-file

  const int LineLength = 1000;
  char LineBuffer[LineLength];
  TString Str;

  // Step two find keywords and the belonging data:
  while (fgets(LineBuffer, LineLength, Input) != NULL) {

    // Realism
    if (strstr(LineBuffer, "[Realism]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0) break;
      sscanf(LineBuffer, "%d\n", &m_Realism);
    }

    // Realism
    if (strstr(LineBuffer, "[NInitializationEvents]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0) break;
      sscanf(LineBuffer, "%d\n", &m_NInitializationEvents);
    }

    if (strstr(LineBuffer, "[Geometry]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0) break;
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

  } 
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataSivan::SaveDataLocal(MXmlDocument* Document)
{
  // Step two: save the keywords and the belonging data:

  new MXmlNode(Document, "Realism", m_Realism);
  new MXmlNode(Document, "NInitializationEvents", m_NInitializationEvents);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataSivan::ReadDataLocal(MXmlDocument* Document)
{  
  MXmlNode* aNode = 0;
  
  if ((aNode = Document->GetNode("Realism")) != 0) {
    m_Realism = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("NInitializationEvents")) != 0) {
    m_NInitializationEvents = aNode->GetValueAsInt();
  }
    
  return true;
}


// MGUIDataSivan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
