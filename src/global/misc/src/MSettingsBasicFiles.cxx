/*
 * MSettingsBasicFiles.cxx
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
// MSettingsBasicFiles.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsBasicFiles.h"

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
ClassImp(MSettingsBasicFiles)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MSettingsBasicFiles::m_MaxHistory = 20;


////////////////////////////////////////////////////////////////////////////////


MSettingsBasicFiles::MSettingsBasicFiles() : MSettingsInterface()
{
  // default constructor

  m_CurrentFileName = "";
  m_GeometryFileName = "$(MEGALIB)/resource/examples/geomega/special/Dummy.geo.setup";

  for (unsigned int i = 0; i < m_MaxHistory; ++i) {
    m_FileHistory.push_back(g_StringNotDefined);
    m_GeometryHistory.push_back(g_StringNotDefined);
  }
}


////////////////////////////////////////////////////////////////////////////////


MSettingsBasicFiles::~MSettingsBasicFiles()
{
  // default destructor
}

////////////////////////////////////////////////////////////////////////////////


bool MSettingsBasicFiles::ReadXml(MXmlNode* Node)
{
  // Load the configuration-file

  MXmlNode* aNode = 0;

  if ((aNode = Node->GetNode("GeometryFileName")) != 0) {
    m_GeometryFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("DataFileName")) != 0) {
    m_CurrentFileName = aNode->GetValueAsString();
  }

  if ((aNode = Node->GetNode("DataFileHistory")) != 0) {
    m_FileHistory.clear();
    for (unsigned int n = 0; n < aNode->GetNNodes(); ++n) {
      if (aNode->GetNode(n)->GetName() == "DataFileHistoryItem") {
        if (MFile::Exists(aNode->GetNode(n)->GetValueAsString()) == true) {
          m_FileHistory.push_back(aNode->GetNode(n)->GetValueAsString());
        }
      }
    }
  }

  if ((aNode = Node->GetNode("GeometryFileHistory")) != 0) {
    m_GeometryHistory.clear();
    for (unsigned int n = 0; n < aNode->GetNNodes(); ++n) {
      if (aNode->GetNode(n)->GetName() == "GeometryFileHistoryItem") {
        if (MFile::Exists(aNode->GetNode(n)->GetValueAsString()) == true) {
          m_GeometryHistory.push_back(aNode->GetNode(n)->GetValueAsString());
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsBasicFiles::WriteXml(MXmlNode* Node)
{
  // Save the current configuration

  // Store the file names
  new MXmlNode(Node, "GeometryFileName", CleanPath(m_GeometryFileName));
  new MXmlNode(Node, "DataFileName", CleanPath(m_CurrentFileName));


  MXmlNode* DataFileHistory = new MXmlNode(Node, "DataFileHistory");
  for (unsigned int i = 0; i < m_FileHistory.size(); ++i) {
    if (m_FileHistory[i] == g_StringNotDefined) break;
    new MXmlNode(DataFileHistory, "DataFileHistoryItem", CleanPath(m_FileHistory[i]));
  }

  MXmlNode* GeometryFileHistory = new MXmlNode(Node, "GeometryFileHistory");
  for (unsigned int i = 0; i < m_GeometryHistory.size(); ++i) {
    if (m_GeometryHistory[i] == g_StringNotDefined) break;
    new MXmlNode(GeometryFileHistory, "GeometryFileHistoryItem", CleanPath(m_GeometryHistory[i]));
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MSettingsBasicFiles::GetCurrentFileName()
{
  return m_CurrentFileName;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsBasicFiles::SetCurrentFileName(MString FileName)
{ 
  MFile::ExpandFileName(FileName);
  if (MFile::Exists(FileName) == false) {
    return false;
  }
  m_CurrentFileName = FileName; 
  AddFileHistory(FileName);
  Modify(4);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsBasicFiles::SetGeometryFileName(MString FileName)
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


MString MSettingsBasicFiles::GetGeometryFileName()
{
  return m_GeometryFileName;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSettingsBasicFiles::GetNFileHistories()
{
  for (unsigned int i = 0; i < m_FileHistory.size(); ++i) {
    if (m_FileHistory[i] == g_StringNotDefined) {
      return i;
    }
  }

  return m_FileHistory.size();
}


////////////////////////////////////////////////////////////////////////////////


MString MSettingsBasicFiles::GetFileHistoryAt(unsigned int i)
{
  massert(i < GetNFileHistories());

  return m_FileHistory[i];
}


////////////////////////////////////////////////////////////////////////////////


void MSettingsBasicFiles::AddFileHistory(MString FileName)
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


unsigned int MSettingsBasicFiles::GetNGeometryHistories()
{
  for (unsigned int i = 0; i < m_GeometryHistory.size(); ++i) {
    if (m_GeometryHistory[i] == g_StringNotDefined) {
      return i;
    }
  }

  return m_GeometryHistory.size();
}


////////////////////////////////////////////////////////////////////////////////


MString MSettingsBasicFiles::GetGeometryHistoryAt(unsigned int i)
{
  massert(i < GetNGeometryHistories());

  return m_GeometryHistory[i];
}


////////////////////////////////////////////////////////////////////////////////


void MSettingsBasicFiles::AddGeometryHistory(MString FileName)
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


// MSettingsBasicFiles.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
