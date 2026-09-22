/*
 * MGUIResponseParameterConeShapes.cxx
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
// MGUIResponseParameterConeShapes
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIResponseParameterConeShapes.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIResponseParameterConeShapes)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIResponseParameterConeShapes::MGUIResponseParameterConeShapes(const TGWindow* Parent, 
                                                                 const TGWindow* Main, 
                                                                 MSettingsImaging* Settings)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIResponseParameterConeShapes and bring it to the screen

  m_Settings = Settings;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIResponseParameterConeShapes::~MGUIResponseParameterConeShapes()
{
  // Delete an instance of MGUIResponseParameterConeShapes
}


////////////////////////////////////////////////////////////////////////////////


void MGUIResponseParameterConeShapes::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Response: Simulated cone shapes");  

  AddSubTitle("Choose the file containing the cone shapes."); 

  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 5*m_FontScaler, 0);

  m_ARMFile = new MGUIEFileSelector(this, "Cone-shapes file name", m_Settings->GetImagingResponseConeShapesFileName());
  m_ARMFile->SetFileType("Response", "*.imagingarm.photopeak.rsp");
  m_ARMFile->SetFileType("Response (gzip'ed)", "*.imagingarm.photopeak.rsp.gz");
  AddFrame(m_ARMFile, SingleLayout);

  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIResponseParameterConeShapes::OnApply()
{
  if (m_ARMFile->GetFileName() != m_Settings->GetImagingResponseConeShapesFileName()) {
    m_Settings->SetImagingResponseConeShapesFileName(m_ARMFile->GetFileName());
  }

  return true;
}


// MGUIResponseParameterConeShapes: the end...
////////////////////////////////////////////////////////////////////////////////
