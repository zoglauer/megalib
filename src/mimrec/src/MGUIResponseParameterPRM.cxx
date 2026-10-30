/*
 * MGUIResponseParameterPRM.cxx
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
// MGUIResponseParameterPRM
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIResponseParameterPRM.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIResponseParameterPRM)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIResponseParameterPRM::MGUIResponseParameterPRM(const TGWindow* Parent, 
                                                   const TGWindow* Main, 
                                                   MSettingsImaging* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIResponseParameterPRM and bring it to the screen

  m_Parent = (TGWindow *) Parent;
  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIResponseParameterPRM::~MGUIResponseParameterPRM()
{
  // Delete an instance of MGUIResponseParameterPRM
}


////////////////////////////////////////////////////////////////////////////////


void MGUIResponseParameterPRM::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Response: Precalculated response matrices");  

  AddSubTitle("Choose the files for the response matrix."); 

  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 5*m_FontScaler, 0);

  m_ComptonTransversalFile = 
    new MGUIEFileSelector(this,
                          "Compton response perpendicular to the cone (width of the cone)",
                          m_GUIData->GetImagingResponseComptonTransversalFileName());
  m_ComptonTransversalFile->SetFileType("Response", "*.compton.trans.rsp");
  AddFrame(m_ComptonTransversalFile, SingleLayout);

  m_ComptonLongitudinalFile = 
    new MGUIEFileSelector(this,
                          "Compton response along the cone (length of the arc)",
                          m_GUIData->GetImagingResponseComptonLongitudinalFileName());
  m_ComptonLongitudinalFile->SetFileType("Response", "*.compton.long.rsp");
  AddFrame(m_ComptonLongitudinalFile, SingleLayout);


  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIResponseParameterPRM::OnApply()
{
  if (m_ComptonLongitudinalFile->GetFileName() != m_GUIData->GetImagingResponseComptonLongitudinalFileName()) {
    m_GUIData->SetImagingResponseComptonLongitudinalFileName(m_ComptonLongitudinalFile->GetFileName());
  }

  if (m_ComptonTransversalFile->GetFileName() != m_GUIData->GetImagingResponseComptonTransversalFileName()) {
    m_GUIData->SetImagingResponseComptonTransversalFileName(m_ComptonTransversalFile->GetFileName());
  }

  return true;
}


// MGUIResponseParameterPRM: the end...
////////////////////////////////////////////////////////////////////////////////
