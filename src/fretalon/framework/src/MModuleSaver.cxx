/*
 * MModuleSaver.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


////////////////////////////////////////////////////////////////////////////////
//
// MModuleSaver
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleSaver.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGUIOptionsFileChooser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleSaver)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleSaver::MModuleSaver() : MModule(), MModuleInterfaceFileName()
{
  // Construct an instance of MNCTModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Save events (roa or evta format)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagEventSaver";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventSaver);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // Set if this module has an options GUI
  m_HasOptionsGUI = true;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleSaver::~MModuleSaver()
{
  // Delete this instance of MNCTModuleTemplate
  m_Out.close();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaver::Initialize()
{
  // Initialize the module
  
  if (m_FileName.EndsWith("evta") == false && m_FileName.EndsWith("roa") == false) {
    if (g_Verbosity >= c_Error) mout<<m_XmlTag<<": The file must be either a evta or roa file, and not: "<<m_FileName<<endl;
    return false;    
  }
  
  m_Out.open(m_FileName);
  if (m_Out.is_open() == false) {
    if (g_Verbosity >= c_Error) mout<<m_XmlTag<<": Unable to open file: "<<m_FileName<<endl;
    return false;
  }
  
  if (m_FileName.EndsWith("evta")) {
    m_Mode = c_EvtaFile;
    m_Out<<endl;
    m_Out<<"Version 21"<<endl;
    m_Out<<"Type EVTA"<<endl;
    m_Out<<endl;
  } else if (m_FileName.EndsWith("roa")) {
    m_Mode = c_RoaFile;
    m_Out<<endl;
    m_Out<<"TYPE ROA"<<endl;
    m_Out<<endl;

    // Known read-out units form the header, new ones are declared where they first occur
    for (unsigned int u = 0; u < m_RoaFileFormat.GetNumberOfReadOutUnits(); ++u) {
      m_Out<<m_RoaFileFormat.GetUFLine(u)<<endl;
    }
    if (m_RoaFileFormat.GetNumberOfReadOutUnits() > 0) m_Out<<endl;
  } else {
    if (g_Verbosity >= c_Error) mout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaver::Finalize()
{
  // Initialize the module 

  MModule::Finalize();
  
  m_Out<<"EN"<<endl;
  m_Out.close();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaver::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Write the event to disk

  if (m_Mode == c_EvtaFile) {
    Event->StreamEvta(m_Out);  
  } else if (m_Mode == c_RoaFile) {
    // Declare the read-out units first used in this event
    const unsigned int Before = m_RoaFileFormat.GetNumberOfReadOutUnits();
    for (unsigned int r = 0; r < Event->GetNumberOfReadOuts(); ++r) {
      const MReadOut& RO = Event->GetReadOut(r);
      const MString ReadOutElementType = RO.GetReadOutElement().GetType();
      const MString ReadOutDataType = RO.GetReadOutData().GetCombinedType();
      if (m_RoaFileFormat.FindByTypes(ReadOutElementType, ReadOutDataType) == g_UnsignedIntNotDefined) {
        m_RoaFileFormat.AddReadOutUnit(ReadOutElementType, ReadOutDataType);
      }
    }
    for (unsigned int u = Before; u < m_RoaFileFormat.GetNumberOfReadOutUnits(); ++u) {
      m_Out<<m_RoaFileFormat.GetUFLine(u)<<endl;
    }
    if (Event->StreamRoa(m_Out, m_RoaFileFormat) == false) {
      if (g_Verbosity >= c_Error) mout<<m_XmlTag<<": Event "<<Event->GetID()<<" has read-outs not defined in the roa file format: "<<m_FileName<<endl;
      m_IsOK = false;
      return false;
    }
  } else {
    if (g_Verbosity >= c_Error) mout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }
  
  Event->SetAnalysisProgress(MAssembly::c_EventSaver);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaver::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsFileChooser* Options = new MGUIOptionsFileChooser(this);
  Options->SetDescription("Choose either an roa or an evta file:");
  Options->SetFileTypes({ "*.roa", "*.evta" }, { "roa files", "evta files" });
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaver::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
  MXmlNode* ModeNode = Node->GetNode("Mode");
  if (ModeNode != 0) {
    m_Mode = ModeNode->GetValueAsUnsignedInt();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleSaver::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  new MXmlNode(Node, "Mode", m_Mode);

  return Node;
}


// MModuleSaver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
