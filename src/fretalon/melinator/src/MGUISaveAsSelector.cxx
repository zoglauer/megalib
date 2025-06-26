/*
 * MGUISaveAsSelector.cxx
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
  TGLayoutHints* DimensionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 0*m_FontScaler, 0*m_FontScaler);
  TGLayoutHints* BinLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40*m_FontScaler, 20*m_FontScaler, 0*m_FontScaler, 20*m_FontScaler);


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
