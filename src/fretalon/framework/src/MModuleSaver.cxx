/*
 * MModuleSaver.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer
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


#ifdef ___CINT___
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
  
  m_NAllowedWorkerThreads = 1;
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
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": The file must be either a evta or roa file, and not: "<<m_FileName<<endl;
    return false;    
  }
  
  m_Out.open(m_FileName);
  if (m_Out.is_open() == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open file: "<<m_FileName<<endl;
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
    //m_Out<<"UF TBD TBD"<<endl;
    m_Out<<endl;
  } else {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
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
    Event->StreamRoa(m_Out);
  } else {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
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
