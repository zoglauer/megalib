/*
 * MGUIData.cxx
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
// MGUIData.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIData.h"

// Standard libs:
#include <iostream>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MFile.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIData)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MGUIData::m_MaxHistory = 20;


////////////////////////////////////////////////////////////////////////////////


MGUIData::MGUIData()
{
  // default constructor

  m_MasterNodeName = "MEGAlibConfigurationFile";

  m_DefaultFileName = ".megalib.cfg";
  m_ModificationLevel = 0;
  m_GeometryFileName = g_StringNotDefined;
  m_SpecialMode = false;
  m_MEGAlibVersion = g_Version;

  for (unsigned int i = 0; i < m_MaxHistory; ++i) {
    m_FileHistory.push_back(g_StringNotDefined);
    m_GeometryHistory.push_back(g_StringNotDefined);
  }
}


////////////////////////////////////////////////////////////////////////////////


MGUIData::~MGUIData()
{
  // default destructor

}


////////////////////////////////////////////////////////////////////////////////


bool MGUIData::ReadDataOld(MString FileName)
{
  // Load the configuration-file

  mout<<"Info: Reading old configuration file format..."<<endl;
  mout<<"      Please check if your configuration is still identical..."<<endl;

  if (FileName != g_StringNotDefined) {
    m_DataFileName = FileName;
  } else {
    m_DataFileName = m_DefaultFileName;
  }

  // Step one: Open the file from which we read the data:
  FILE* Input = 0;
  if ((Input = fopen(m_DataFileName, "r")) == 0) {
    mout<<"Can't find the configuration file "<<m_DataFileName.Data()<<endl;
    mout<<"Starting with the minimal configuration!"<<endl;
    return false;
  }

  const int LineLength = 1000;
  char LineBuffer[LineLength];
  MString Str;

  // Step two find keywords and the belonging data:
  while (fgets(LineBuffer, LineLength, Input) != NULL) {

    if (strstr(LineBuffer, "[Version]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = MString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_MEGAlibVersion = atoi(Str);
    }

    if (strstr(LineBuffer, "[GeometryFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = MString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_GeometryFileName = Str;
    }

    if (strstr(LineBuffer, "[EventFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = MString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_CurrentFile = Str;
    }

    // FileHistory:
    if (strstr(LineBuffer, "[FileHistory]") != NULL) {
      m_FileHistory.clear();
      for (unsigned int i = 0; i < m_MaxHistory; ++i) {
        if (fgets(LineBuffer, LineLength, Input) == 0) break;
        Str = MString(LineBuffer);
        Str = Str.Remove(Str.First('\n'));
        if (MFile::Exists(Str) == true) {
          m_FileHistory.push_back(Str);
        }
      }   
    }
    // FileHistory:
    if (strstr(LineBuffer, "[GeometryHistory]") != NULL) {
      m_GeometryHistory.clear();
      for (unsigned int i = 0; i < m_MaxHistory; ++i) {
        if (fgets(LineBuffer, LineLength, Input) == 0) break;
        Str = MString(LineBuffer);
        Str = Str.Remove(Str.First('\n'));
        if (MFile::Exists(Str) == true) {
          m_GeometryHistory.push_back(Str);
        }
      }   
    }
  }
    
  rewind(Input);
  ReadDataLocal(Input);

  fclose(Input);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIData::ReadData(MString FileName)
{
  // Load the configuration-file

  if (FileName != g_StringNotDefined) {
    m_DataFileName = FileName;
  } else {
    m_DataFileName = m_DefaultFileName;
  }

  // Read the first line, if it begins with "<" we have an xml file
  ifstream in;
  in.open(m_DataFileName);
  if (in.is_open() == false) {
    mout<<"Can't find the configuration file "<<m_DataFileName<<endl;
    mout<<"Starting with the default configuration!"<<endl;
    return false;
  }

  const int LineLength = 1000;
  char LineBuffer[LineLength];

  if (in.getline(LineBuffer, LineLength, '\n')) {
    MString Line(LineBuffer);
    Line.ReplaceAll("\r", "");
    Line.Strip();
    if (Line.BeginsWith("<") == false || Line.EndsWith(">") == false) {
      in.close();
      ReadDataOld(FileName);
      return true;
    }
  } else {
    mout<<"Error reading configuration file "<<m_DataFileName<<endl;
    mout<<"Starting with the default configuration!"<<endl;
    return false;
  }


  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument();
  Document->Load(m_DataFileName);

  if (m_MasterNodeName != Document->GetName()) {
    mout<<"Error reading configuration file "<<m_DataFileName<<endl;
    mout<<"The loaded configarion file is of type \""<<Document->GetName()<<"\", but expected is: \""<<m_MasterNodeName<<"\""<<endl;
    mout<<"Starting with the default configuration!"<<endl;
    return false;    
  }


  MXmlNode* aNode = 0;


  int Version = 1;
  if ((aNode = Document->GetNode("Version")) != 0) {
    Version = aNode->GetValueAsInt();
  }

  if ((aNode = Document->GetNode("MEGAlibVersion")) != 0) {
    m_MEGAlibVersion = aNode->GetValueAsInt();
  }

  if ((aNode = Document->GetNode("GeometryFileName")) != 0) {
    m_GeometryFileName = aNode->GetValueAsString();
  }
  if ((aNode = Document->GetNode("DataFileName")) != 0) {
    m_CurrentFile = aNode->GetValueAsString();
  }

  if ((aNode = Document->GetNode("DataFileHistory")) != 0) {
    m_FileHistory.clear();
    for (unsigned int n = 0; n < aNode->GetNNodes(); ++n) {
      if (aNode->GetNode(n)->GetName() == "DataFileHistoryItem") {
        if (MFile::Exists(aNode->GetNode(n)->GetValueAsString()) == true) {
          m_FileHistory.push_back(aNode->GetNode(n)->GetValueAsString());
        }
      }
    }
  }

  if ((aNode = Document->GetNode("GeometryFileHistory")) != 0) {
    m_GeometryHistory.clear();
    for (unsigned int n = 0; n < aNode->GetNNodes(); ++n) {
      if (aNode->GetNode(n)->GetName() == "GeometryFileHistoryItem") {
        if (MFile::Exists(aNode->GetNode(n)->GetValueAsString()) == true) {
          m_GeometryHistory.push_back(aNode->GetNode(n)->GetValueAsString());
        }
      }
    }
  }

  ReadDataLocal(Document);

  delete Document;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIData::SaveData(MString FileName)
{
  // Save the current configuration

  if (FileName == g_StringNotDefined) {
    FileName = m_DefaultFileName;
  }

  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument(m_MasterNodeName);

  // Version string: 1
  new MXmlNode(Document, "Version", 1);

  // Version string: 1
  new MXmlNode(Document, "MEGAlibVersion", m_MEGAlibVersion);

  // Store the file names
  new MXmlNode(Document, "GeometryFileName", m_GeometryFileName);
  new MXmlNode(Document, "DataFileName", m_CurrentFile);


  MXmlNode* DataFileHistory = new MXmlNode(Document, "DataFileHistory");
  for (unsigned int i = 0; i < m_FileHistory.size(); ++i) {
    if (m_FileHistory[i] == g_StringNotDefined) break;
    new MXmlNode(DataFileHistory, "DataFileHistoryItem", m_FileHistory[i]);
  }

  MXmlNode* GeometryFileHistory = new MXmlNode(Document, "GeometryFileHistory");
  for (unsigned int i = 0; i < m_GeometryHistory.size(); ++i) {
    if (m_GeometryHistory[i] == g_StringNotDefined) break;
    new MXmlNode(GeometryFileHistory, "GeometryFileHistoryItem", m_GeometryHistory[i]);
  }

  SaveDataLocal(Document);

  // Store the module content
  Document->Save(FileName);

  delete Document;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIData::Modify(int m)
{
  //

  if (m_ModificationLevel < m) {
    m_ModificationLevel = m;
  }
}


////////////////////////////////////////////////////////////////////////////////


int MGUIData::GetModificationLevel(bool Reset) 
{
  // Copy the data of this object to the object data and return the modification-level
  // between this class and the old one
  //
  // 4:  backprojection, active file etc. has been modified 
  // 3:  iterations
  // 2:  post-computation
  // 1:  display 
  // 0:  nothing has been modified since last question

  int Level = m_ModificationLevel;
  if (Reset == true)
    m_ModificationLevel = 0;

  return Level;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIData::GetCurrentFileName()
{
  return m_CurrentFile;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIData::SetCurrentFileName(MString FileName)
{ 
  MFile::ExpandFileName(FileName);
  if (MFile::Exists(FileName) == false) {
    return false;
  }
  m_CurrentFile = FileName; 
  AddFileHistory(FileName);
  Modify(4);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIData::SetGeometryFileName(MString FileName)
{
  MFile::ExpandFileName(FileName);
  if (MFile::Exists(FileName) == false) {
    return false;
  }
  m_GeometryFileName = FileName;
  AddGeometryHistory(FileName);
  Modify(4);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIData::GetGeometryFileName()
{
  return m_GeometryFileName;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIData::GetDataFileName()
{
  return m_DataFileName;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIData::SetDataFileName(MString FileName)
{
  MFile::ExpandFileName(FileName);
  if (MFile::Exists(FileName) == false) {
    return false;
  }
  m_DataFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MGUIData::GetNFileHistories()
{
  for (unsigned int i = 0; i < m_FileHistory.size(); ++i) {
    if (m_FileHistory[i] == g_StringNotDefined) {
      return i;
    }
  }

  return m_FileHistory.size();
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIData::GetFileHistoryAt(unsigned int i)
{
  massert(i < GetNFileHistories());

  return m_FileHistory[i];
}


////////////////////////////////////////////////////////////////////////////////


void MGUIData::AddFileHistory(MString FileName)
{
  if (FileName == g_StringNotDefined) return;

  vector<MString>::iterator Iter;
  Iter = find(m_FileHistory.begin(), m_FileHistory.end(), FileName);
  if (Iter != m_FileHistory.end()) {
    m_FileHistory.erase(Iter);
  }

  if (m_FileHistory.size() > 0) {
    m_FileHistory.push_back(m_FileHistory[m_FileHistory.size()-1]);
    for (unsigned int i = m_FileHistory.size()-1; i > 0; --i) {
      m_FileHistory[i] = m_FileHistory[i-1];
    }
    m_FileHistory[0] = FileName;
  } else {
    m_FileHistory.push_back(FileName);
  }

  while (GetNFileHistories() > m_MaxHistory) {
    m_FileHistory.pop_back();
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MGUIData::GetNGeometryHistories()
{
  for (unsigned int i = 0; i < m_GeometryHistory.size(); ++i) {
    if (m_GeometryHistory[i] == g_StringNotDefined) {
      return i;
    }
  }

  return m_GeometryHistory.size();
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIData::GetGeometryHistoryAt(unsigned int i)
{
  massert(i < GetNGeometryHistories());

  return m_GeometryHistory[i];
}


////////////////////////////////////////////////////////////////////////////////


void MGUIData::AddGeometryHistory(MString FileName)
{
  if (FileName == g_StringNotDefined) return;

  vector<MString>::iterator Iter;
  Iter = find(m_GeometryHistory.begin(), m_GeometryHistory.end(), FileName);
  if (Iter != m_GeometryHistory.end()) {
    m_GeometryHistory.erase(Iter);
  }

  if (m_GeometryHistory.size() > 0) {
    m_GeometryHistory.push_back(m_GeometryHistory[m_GeometryHistory.size()-1]);
    for (unsigned int i = m_GeometryHistory.size()-1; i > 0; --i) {
      m_GeometryHistory[i] = m_GeometryHistory[i-1];
    }
    m_GeometryHistory[0] = FileName;
  } else {
    m_GeometryHistory.push_back(FileName);
  }

  while (GetNGeometryHistories() > m_MaxHistory) {
    m_GeometryHistory.pop_back();
  }
}


// MGUIData.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
