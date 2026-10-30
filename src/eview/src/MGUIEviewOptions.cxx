/*
 * MGUIEviewOptions.cxx
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
// MGUIEviewOptions
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEviewOptions.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEviewOptions)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEviewOptions::MGUIEviewOptions(const TGWindow* Parent, const TGWindow* Main, 
                                   MGUIData* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIEviewOptions and bring it to the screen

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEviewOptions::~MGUIEviewOptions()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewOptions::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Title");  

  AddSubTitle("Subtitle"); 

  // Add here ...


  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight());

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEviewOptions::OnApply()
{
  return true;
}


// MGUIEviewOptions: the end...
////////////////////////////////////////////////////////////////////////////////
