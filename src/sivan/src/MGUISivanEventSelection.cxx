/*
 * MGUISivanEventSelection.cxx
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
// MGUISivanEventSelection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUISivanEventSelection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUISivanEventSelection)
#endif



////////////////////////////////////////////////////////////////////////////////


MGUISivanEventSelection::MGUISivanEventSelection(const TGWindow* Parent, 
                                                 const TGWindow* Main, 
                                                 MSettingsSivan* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUISivanEventSelection::~MGUISivanEventSelection()
{
  // standard destructor

}


////////////////////////////////////////////////////////////////////////////////


void MGUISivanEventSelection::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Event selection");  

  AddSubTitle("What events can Sivan use?"); 


  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight());

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUISivanEventSelection::OnApply()
{
  return true;
}


// MGUISivanEventSelection: the end...
////////////////////////////////////////////////////////////////////////////////
