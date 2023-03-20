/*
 * MModuleEnergyCalibration.cxx
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
// MModuleEnergyCalibration
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleEnergyCalibration.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MReadOutElementPixel2D.h"
#include "MReadOutDataADCValue.h"
#include "MGUIOptionsFileChooser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleEnergyCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleEnergyCalibration::MModuleEnergyCalibration() : MModule(), MModuleInterfaceFileName()
{
  // Construct an instance of MModuleEnergyCalibration
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Converter for Sim to Roa events";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagEnergyCalibration";

  // Set all modules, which have to be done before this module
  // None
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventLoader);
  AddModuleType(MAssembly::c_EventLoaderSimulation);
  AddModuleType(MAssembly::c_DetectorEffectsEngine);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // True if this module is associated with an options GUI
  m_HasOptionsGUI = true;
  
  // This is a special start module which can generate its own events
  m_IsStartModule = true;
  
  // Allow the use of multiple threads and but not instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = true;
}


////////////////////////////////////////////////////////////////////////////////


MModuleEnergyCalibration::~MModuleEnergyCalibration()
{
  // Delete this instance of MModuleEnergyCalibration
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibration::Initialize()
{
  // Initialize the module 

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibration::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to the next level:
  // Here: Energy calibration.

  for (unsigned int i = 0; i < Event->GetNumberOfReadOuts(); ++i) {
    MReadOut& RO = Event->GetReadOut(i);
    unsigned int ADUs = RO.GetReadOutData().GetADCValue();
  }

  // Set the analysis progress of the event:
  Event->SetAnalysisProgress(MAssembly::c_EnergyCalibration);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibration::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();

  m_SimFile.Close();
}


///////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibration::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsFileChooser* Options = new MGUIOptionsFileChooser(this);
  Options->SetDescription("Choose a sim file:");
  Options->SetFileTypes({ "*.sim", "*.sim.gz" }, { "sim files", "sim files - gzipped" });
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibration::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleEnergyCalibration::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  
  return Node;
}


// MModuleEnergyCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
