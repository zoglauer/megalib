/*
 * MModuleTemplate.cxx
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
// MModuleTemplate
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleTemplate.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MModule.h"
#include "MGUIOptionsTemplate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleTemplate)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleTemplate::MModuleTemplate() : MModule()
{
  // Construct an instance of MModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Template";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagTemplate";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_DetectorEffectsEngine);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_ChargeSharingCorrection);
  AddPreceedingModuleType(MAssembly::c_DepthCorrection);
  AddPreceedingModuleType(MAssembly::c_StripPairing);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_DetectorEffectsEngine);
  AddModuleType(MAssembly::c_EnergyCalibration);
  AddModuleType(MAssembly::c_ChargeSharingCorrection);
  AddModuleType(MAssembly::c_DepthCorrection);
  AddModuleType(MAssembly::c_StripPairing);
  AddModuleType(MAssembly::c_EventReconstruction);

  // Set all modules, which can follow this module

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleTemplate::~MModuleTemplate()
{
  // Delete this instance of MModuleTemplate
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTemplate::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTemplate::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleTemplate::Finalize()
{
  // Initialize the module 

  MModule::Finalize();
  
  // Your code here
}


////////////////////////////////////////////////////////////////////////////////


void MModuleTemplate::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsTemplate* Options = new MGUIOptionsTemplate(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTemplate::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  /*
  MXmlNode* SomeTagNode = Node->GetNode("SomeTag");
  if (SomeTagNode != 0) {
    m_SomeTagValue = SomeTagNode->GetValue();
  }
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleTemplate::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MModuleTemplate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
