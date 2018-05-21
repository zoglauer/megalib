/*
 * MModuleLoaderRoa.cxx
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
// MModuleLoaderRoa
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderRoa.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MReadOutElementDoubleStrip.h"
#include "MGUIOptionsFileChooser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderRoa)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderRoa::MModuleLoaderRoa() : MModule(), MModuleInterfaceFileName()
{
  // Construct an instance of MModuleLoaderRoa
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Loader for Roa files";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagLoaderRoa";

  // Set all modules, which have to be done before this module
  // None
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventLoader);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // True if this module is associated with an options GUI
  m_HasOptionsGUI = true;
  
  // This is a special start module which can generate its own events
  m_IsStartModule = true;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderRoa::~MModuleLoaderRoa()
{
  // Delete this instance of MModuleLoaderRoa
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderRoa::Initialize()
{
  // Initialize the module 
  
  m_RoaFile.Open(m_FileName);
  if (m_RoaFile.IsOpen() == false) return false;
    
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderRoa::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.
    
  if (ReadNextEvent(Event) == false) {
    cout<<"MModuleLoaderRoa: No more events!"<<endl;
    m_IsFinished = true;
    return false;
  }
  
  Event->SetAnalysisProgress(MAssembly::c_EventLoader);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderRoa::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();

  m_RoaFile.Close();  
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderRoa::ReadNextEvent(MReadOutAssembly* Event)
{
  // Return next single event from file... or 0 if there are no more.
  
  Event->Clear();

  MReadOutSequence ROS;
  m_RoaFile.ReadNext(ROS);

  if (ROS.GetNumberOfReadOuts() == 0) {
    cout<<m_Name<<": No more read-outs available in File"<<endl;
    return false;
  }
  
  Event->SetID(ROS.GetID());
  Event->SetTime(ROS.GetTime());
  
  for (unsigned int r = 0; r < ROS.GetNumberOfReadOuts(); ++r) {
    MReadOut RO = ROS.GetReadOut(r);
    
    Event->AddReadOut(RO);
  }
  
  return true;
}


///////////////////////////////////////////////////////////////////////////////


void MModuleLoaderRoa::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsFileChooser* Options = new MGUIOptionsFileChooser(this);
  Options->SetDescription("Choose an roa file:");
  Options->SetFileTypes({ "*.roa" }, { "roa files" });
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderRoa::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleLoaderRoa::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  
  return Node;
}


// MModuleLoaderRoa.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
