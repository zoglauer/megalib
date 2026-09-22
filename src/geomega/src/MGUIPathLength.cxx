/*
 * MGUIPathLength.cxx
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
// MGUIPathLength
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIPathLength.h"

// Standard libs:
#include <iostream>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIPathLength)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIPathLength::MGUIPathLength(const TGWindow* Parent, const TGWindow* Main, 
                               MSettingsGeomega* Settings, bool& OkPressed)
  : MGUIDialog(Parent, Main), m_OkPressed(OkPressed)
{
  // Construct an instance of MGUIPathLength and bring it to the screen

  m_Settings = Settings;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_OkPressed = false;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIPathLength::~MGUIPathLength()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPathLength::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Volume information");  
  AddSubTitle("Please enter a position in cm"); 

  TGLayoutHints* PositionLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 2, 2);

  m_StartString = new MGUIEEntry(this, "Start position as string \"(x, y, z)\" or x; y; z [cm]:", false, m_Settings->GetPathLengthStart().ToString());
  m_StartString->SetEntryFieldSize(m_FontScaler*275);
  AddFrame(m_StartString, PositionLayout);

  m_StopString = new MGUIEEntry(this, "Stop position as string \"(x, y, z)\" or x; y; z [cm]:", false, m_Settings->GetPathLengthStop().ToString());
  m_StopString->SetEntryFieldSize(m_FontScaler*275);
  AddFrame(m_StopString, PositionLayout);

  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight());

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPathLength::Parse(const MString& String, MVector& Start)
{
  //! Parse the string

  double x, y, z;
  if (sscanf(String.Data(), "(%lf, %lf, %lf)", &x, &y, &z) == 3) {
    Start.SetXYZ(x, y, z);
    return true;
  }
  if (sscanf(String.Data(), "%lf, %lf, %lf", &x, &y, &z) == 3) {
    Start.SetXYZ(x, y, z);
    return true;
  }
  if (sscanf(String.Data(), "%lf; %lf; %lf", &x, &y, &z) == 3) {
    Start.SetXYZ(x, y, z);
    return true;
  }
  if (sscanf(String.Data(), "%lf %lf %lf", &x, &y, &z) == 3) {
    Start.SetXYZ(x, y, z);
    return true;
  }
  
  return false;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MGUIPathLength::OnApply()
{
  // The Apply button has been pressed

  MVector Start;
  if (Parse(m_StartString->GetAsString(), Start) == false) {
    mgui<<"Incorrectly formated START string"<<error;
    return false;
  }
  MVector Stop;
  if (Parse(m_StopString->GetAsString(), Stop) == false) {
    mgui<<"Incorrectly formated STOP string"<<error;
    return false;
  }
  
  m_Settings->SetPathLengthStart(Start);
  m_Settings->SetPathLengthStop(Stop);

  m_OkPressed = true; 

  return true;
}


// MGUIPathLength: the end...
////////////////////////////////////////////////////////////////////////////////
