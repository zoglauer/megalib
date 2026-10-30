/*
 * MGUIOptionsLoaderRoa.cxx
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


// Include the header:
#include "MGUIOptionsLoaderRoa.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleLoaderRoa.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsLoaderRoa)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderRoa::MGUIOptionsLoaderRoa(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderRoa::~MGUIOptionsLoaderRoa()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsLoaderRoa::Create()
{
  PreCreate();

  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select an roa file:",
    dynamic_cast<MModuleLoaderRoa*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Roa file", "*.roa");
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsLoaderRoa::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsLoaderRoa::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleLoaderRoa*>(m_Module)->SetFileName(m_FileSelector->GetFileName());
  
  return true;
}


// MGUIOptionsLoaderRoa: the end...
////////////////////////////////////////////////////////////////////////////////
