/*
 * MGUIExpo.cxx
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


// Include the header:
#include "MGUIExpo.h"

// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:
#include <KeySymbols.h>
#include <TSystem.h>
#include <TString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"

// Nuclearizer:
#include "MModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIExpo)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpo::MGUIExpo(MModule* Module) : TGCompositeFrame(gClient->GetRoot(), 320, 240)
{
  // standard constructor

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_Module = Module;
  m_TabTitle = "No title";
  m_NeedsUpdate = false;
  
  m_IsCreated = false;
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpo::~MGUIExpo()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpo::CloseWindow()
{
  // When the x is pressed, this function is called.

  DeleteWindow();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpo::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this window

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpo::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpo::WriteXmlConfiguration(MXmlNode* ModuleNode) 
{
  //! Add XML nodes to the tree of the module 

  return true;
}


// MGUIExpo: the end...
////////////////////////////////////////////////////////////////////////////////
