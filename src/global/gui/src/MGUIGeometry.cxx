/*
 * MGUIGeometry.cxx
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
