/*
 * MGUIMGeant.cxx
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
// MGUIMGeant
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIMGeant.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIMGeant)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIMGeant::MGUIMGeant(const TGWindow* Parent, const TGWindow* Main, 
                       MSettingsGeomega* Data, bool& OkPressed)
  : MGUIDialog(Parent, Main), m_OkPressed(OkPressed)
{
  // standard constructor

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_OkPressed = false;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIMGeant::~MGUIMGeant()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMGeant::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("MGeant/MGGPOD");  

  AddSubTitle("Options for the conversion of the geometry to MGeant/MGGPOD"); 


  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 2, 0);  
  TGLabel* FileNameOptionsLabel = new TGLabel(this, "Choose the file name options:");
  AddFrame(FileNameOptionsLabel, LabelLayout);

  TGLayoutHints* RadioButtonLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 2, 0);
  m_StandardName = 
    new TGRadioButton(this, "Use default mgeant/mggpod file names (setup.geo, materials.may, etc.)", c_StandardName);
  m_SpecialName = 
    new TGRadioButton(this, "Use a special a base name. Do not add any suffix like geo, med or mat.", c_SpecialName);

  AddFrame(m_StandardName, RadioButtonLayout);
  AddFrame(m_SpecialName, RadioButtonLayout);

  TGLayoutHints* FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 60, 20, 2, 20);
  m_FileSelector = new MGUIEFileSelector(this, "", 
                                        m_GUIData->GetMGeantFileName());
  AddFrame(m_FileSelector, FileSelectorLayout);

  m_MEGAlibExtensionOptions = new MGUIECBList(this, "Special MGGPOD/MEGAlib-extension options:");
  m_MEGAlibExtensionOptions->Add("Store complete interaction information (all IAs)");          
  m_MEGAlibExtensionOptions->Add("Store vetoed events");              
  m_MEGAlibExtensionOptions->SetSelected(0, m_GUIData->GetStoreIAs());
  m_MEGAlibExtensionOptions->SetSelected(1, m_GUIData->GetStoreVetoes());
  m_MEGAlibExtensionOptions->Create();
  m_MEGAlibExtensionOptions->Associate(this);
  AddFrame(m_MEGAlibExtensionOptions, LabelLayout);

  AddOKCancelButtons();

  if (m_GUIData->GetMGeantOutputMode() == 0) {
    m_StandardName->SetState(kButtonDown);
    m_FileSelector->SetEnabled(false);
  } else if (m_GUIData->GetMGeantOutputMode() == 1) {
    m_SpecialName->SetState(kButtonDown);
  }

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMGeant::ProcessMessage(long Message, long Parameter1, 
                                              long Parameter2)
{
  // Process the messages for this window

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
        Status = OnOk();
        break;
        
      case e_Cancel:
        Status = OnCancel();
        break;

      case e_Apply:
        Status = OnApply();
        break;
        
      default:
        break;
      }
      break;
    case kCM_RADIOBUTTON:
      if (Parameter1 == c_StandardName) {
        m_StandardName->SetState(kButtonDown);
        m_SpecialName->SetState(kButtonUp);
        m_FileSelector->SetEnabled(false);
      } else if (Parameter1 == c_SpecialName) {
        m_StandardName->SetState(kButtonUp);
        m_SpecialName->SetState(kButtonDown);
        m_FileSelector->SetEnabled(true);
      }
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


bool MGUIMGeant::OnApply()
{
  // The Apply button has been pressed
  
  if (m_StandardName->GetState() == kButtonDown) {
    m_GUIData->SetMGeantOutputMode(0);
  } else if (m_SpecialName->GetState() == kButtonDown) {
    m_GUIData->SetMGeantOutputMode(1);
  }

  m_GUIData->SetMGeantFileName(m_FileSelector->GetFileName());
  m_GUIData->SetStoreIAs(m_MEGAlibExtensionOptions->GetSelected(0));
  m_GUIData->SetStoreVetoes(m_MEGAlibExtensionOptions->GetSelected(1));
  m_OkPressed = true;

  return true;
}


// MGUIMGeant: the end...
////////////////////////////////////////////////////////////////////////////////
