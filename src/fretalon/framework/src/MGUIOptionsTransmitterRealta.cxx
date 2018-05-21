/*
 * MGUIOptionsTransmitterRealta.cxx
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
#include "MGUIOptionsTransmitterRealta.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleTransmitterRealta.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsTransmitterRealta)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTransmitterRealta::MGUIOptionsTransmitterRealta(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTransmitterRealta::~MGUIOptionsTransmitterRealta()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsTransmitterRealta::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_HostName = new MGUIEEntry(m_OptionsFrame, "Host name/IP: ", false,
                              dynamic_cast<MModuleTransmitterRealta*>(m_Module)->GetHostName());
  m_OptionsFrame->AddFrame(m_HostName, LabelLayout);

  m_HostPort = new MGUIEEntry(m_OptionsFrame, "Host listening port: ", false,
                              dynamic_cast<MModuleTransmitterRealta*>(m_Module)->GetHostPort());
  m_OptionsFrame->AddFrame(m_HostPort, LabelLayout);


  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsTransmitterRealta::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

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
  
  if (Status == false) {
    return false;
  }

  // Call also base class
  return MGUIOptions::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsTransmitterRealta::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleTransmitterRealta*>(m_Module)->SetHostName(m_HostName->GetAsString());
  dynamic_cast<MModuleTransmitterRealta*>(m_Module)->SetHostPort(m_HostPort->GetAsInt());
  
  return true;
}


// MGUIOptionsTransmitterRealta: the end...
////////////////////////////////////////////////////////////////////////////////
