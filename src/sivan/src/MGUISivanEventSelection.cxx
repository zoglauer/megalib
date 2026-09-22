/*
 * MGUISivanEventSelection.cxx
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
