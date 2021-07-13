/*
 * MGUIOptionsFileChooser.cxx
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
#include "MGUIOptionsFileChooser.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleInterfaceFileName.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsFileChooser)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsFileChooser::MGUIOptionsFileChooser(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsFileChooser::~MGUIOptionsFileChooser()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsFileChooser::Create()
{
  PreCreate();

  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, m_Description, dynamic_cast<MModuleInterfaceFileName*>(m_Module)->GetFileName());
  for (unsigned int i = 0; i < min(m_FileTypes.size(), m_FileTypeDescriptions.size()); ++i) {
    m_FileSelector->SetFileType(m_FileTypes[i], m_FileTypeDescriptions[i]);
  }
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsFileChooser::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsFileChooser::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleInterfaceFileName*>(m_Module)->SetFileName(m_FileSelector->GetFileName());
  
  return true;
}


// MGUIOptionsFileChooser: the end...
////////////////////////////////////////////////////////////////////////////////
