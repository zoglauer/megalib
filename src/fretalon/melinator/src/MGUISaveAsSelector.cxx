/*
 * MGUISaveAsSelector.cxx
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
// MGUISaveAsSelector
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUISaveAsSelector.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUISaveAsSelector)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUISaveAsSelector::MGUISaveAsSelector(const TGWindow* Parent, const TGWindow* Main, MSettingsMelinator* Settings, bool& OkPressed)
  : MGUIDialog(Parent, Main), m_OkPressed(OkPressed)
{
  // Construct an instance of MGUISaveAsSelector and bring it to the screen

  m_Settings = Settings;
  m_OkPressed = false;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUISaveAsSelector::~MGUISaveAsSelector()
{
  // Delete an instance of MGUISaveAsSelector
}


////////////////////////////////////////////////////////////////////////////////


void MGUISaveAsSelector::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("File name selector");

  AddSubTitle("Choose the file prefix name used for the ecal, fits, and report file:");

  TGLayoutHints* FirstLayout = new TGLayoutHints(kLHintsCenterX | kLHintsTop | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 0*m_FontScaler, 20*m_FontScaler);

  m_FileName = new MGUIEFileSelector(this, "Choose the ecal file name as template", m_Settings->GetSaveAsFileName());
  m_FileName->SetFileType("Energy calibration file", "*.ecal");
  //FileName->ChangeOptions(kFixedWidth);
  //FileName->SetWidth(m_FontScaler*500);
  AddFrame(m_FileName, FirstLayout);
  
  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), true);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);
 
  return;
}

////////////////////////////////////////////////////////////////////////////////


bool MGUISaveAsSelector::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    default:
      break;
    }
  default:
    break;
  }

  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


//! Action after the Apply or OK button has been pressed.
bool MGUISaveAsSelector::OnApply()
{
  m_Settings->SetSaveAsFileName(m_FileName->GetFileName());

  m_OkPressed = true;

  return true;
}


// MGUISaveAsSelector: the end...
////////////////////////////////////////////////////////////////////////////////
