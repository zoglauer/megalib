/*
 * MGUIResponseSelection.cxx
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
// MGUIResponseSelection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIResponseSelection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIResponseSelection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIResponseSelection::MGUIResponseSelection(const TGWindow* Parent, 
                                             const TGWindow* Main, 
                                             MSettingsImaging* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIResponseSelection and bring it to the screen

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIResponseSelection::~MGUIResponseSelection()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIResponseSelection::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Response type selection");  

  AddSubTitle("Select the type of response you want to use"); 

  m_ResponseChoiceLayout = new TGLayoutHints(kLHintsCenterX | kLHintsTop | kLHintsExpandX, 20, 20, 0, 20);
  
  m_ResponseChoice = new MGUIERBList(this, "Response types:");
  m_ResponseChoice->Add("Standard one parameter linear gaussian for photo-peak events (List-mode)");
  m_ResponseChoice->Add("Standard one parameter linear gaussian calculated by uncertainties (List-mode)");
  m_ResponseChoice->Add("Standard one parameter linear gaussian with limited energy measurement (List-mode)");
  m_ResponseChoice->Add("Precalculated response matrices (List-mode)");
  m_ResponseChoice->SetSelected(m_GUIData->GetResponseType());
  m_ResponseChoice->Create();
  
  AddFrame(m_ResponseChoice, m_ResponseChoiceLayout);
    

  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIResponseSelection::OnApply()
{
  // The Apply button has been pressed

  if (m_ResponseChoice->GetSelected() != m_GUIData->GetResponseType()) {
    m_GUIData->SetResponseType(m_ResponseChoice->GetSelected());
  }
 
  return true;
}


// MGUIResponseSelection: the end...
////////////////////////////////////////////////////////////////////////////////
