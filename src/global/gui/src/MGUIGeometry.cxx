/*
 * MGUIGeometry.cxx
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
// MGUIGeometry
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIGeometry.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIEFileSelector.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIGeometry)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIGeometry::MGUIGeometry(const TGWindow* Parent, const TGWindow* Main, const MString& FileName)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIGeometry and bring it to the screen

  m_GeometryFileName = FileName;
  if (m_GeometryFileName == g_StringNotDefined) m_GeometryFileName = "";

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIGeometry::MGUIGeometry(const TGWindow* Parent, const TGWindow* Main, 
                           MSettingsBasicFiles* Settings)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIGeometry and bring it to the screen

  if (Settings != 0) m_GeometryFileName = Settings->GetGeometryFileName();
  if (m_GeometryFileName == g_StringNotDefined) m_GeometryFileName = "";

  m_OkPressed = false;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIGeometry::~MGUIGeometry()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIGeometry::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Geometry");  

  AddSubTitle("Select the geometry for the analysis"); 

  // Add here ...
  
  TGLayoutHints* FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 2);
  m_FileSelector = new MGUIEFileSelector(this, "Currently selected geometry file (*.geo.setup):", 
                                         m_GeometryFileName);
  m_FileSelector->SetFileType("Geometry setup file", "*.geo.setup");
  m_FileSelector->SetFileType("Geometry file", "*.geo");
  AddFrame(m_FileSelector, FileSelectorLayout);

  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight());

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIGeometry::OnOk()
{
  // The Apply button has been pressed

  return OnApply();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIGeometry::OnApply()
{
  // The Apply button has been pressed

  m_OkPressed = true;
  m_GeometryFileName = m_FileSelector->GetFileName();
  
  UnmapWindow();
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIGeometry::OnCancel()
{
  // The Apply button has been pressed

  m_OkPressed = false;
  UnmapWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIGeometry::CloseWindow()
{
  // Call OnCanel for controlled good-bye

  OnCancel();
}



// MGUIGeometry: the end...
////////////////////////////////////////////////////////////////////////////////
