/*
 * MGUISensitivity.cxx
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
// MGUISensitivity
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUISensitivity.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUISensitivity)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUISensitivity::MGUISensitivity(const TGWindow* Parent, const TGWindow* Main, 
                                 MSettingsImaging* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUISensitivity::~MGUISensitivity()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUISensitivity::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Sensitivity matrix");  

  AddSubTitle("Select the file, which contains the sensitivity (efficiency) data"); 

  m_FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 2);
  m_FileSelector = new MGUIEFileSelector(this, "Choose a file (assume constant sensitivity, if the box is empty):", 
                                        m_GUIData->GetSensitivityFile());

  AddFrame(m_FileSelector, m_FileSelectorLayout);


  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUISensitivity::OnApply()
{
  // The Apply button has been pressed

  m_GUIData->SetSensitivityFile(m_FileSelector->GetFileName());
  if (m_FileSelector->GetFileName().CompareTo(MString("")) != 0) {
    m_GUIData->SetUseSensitivityFile(true);
  } else {
    m_GUIData->SetUseSensitivityFile(false);
  }
  
  return true;
}


// MGUISensitivity: the end...
////////////////////////////////////////////////////////////////////////////////
