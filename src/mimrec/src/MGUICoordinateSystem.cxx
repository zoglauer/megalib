/*
 * MGUICoordinateSystem.cxx
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
// MGUICoordinateSystem
//
// Dialog-box where the coordinate system can be selected
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUICoordinateSystem.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MProjection.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUICoordinateSystem)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUICoordinateSystem::MGUICoordinateSystem(const TGWindow* Parent, 
                                           const TGWindow* Main, 
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


MGUICoordinateSystem::~MGUICoordinateSystem()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUICoordinateSystem::Create()
{
   // Create the main window

  // We start with a name and an icon...
  SetWindowName("Coordinate system selection");  
  SetSubTitleText("Please select the coordinate system");

  CreateBasics();

  // ... add the radiobutton selection ...
  TGLayoutHints* SystemLayout = 
    new TGLayoutHints(kLHintsExpandX | kLHintsTop, 50, 20, 0, 20);
  
  m_System = new MGUIERBList(m_MainPanel, "Coordinate-System:", false);
  m_System->Add("Galactic");
  m_System->Add("Spherical");
  m_System->Add("Cartesian 2D");
  m_System->Add("Cartesian 3D");
  
  if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    m_System->SetSelected(0);
  } else if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    m_System->SetSelected(1);
  } else if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D) {
    m_System->SetSelected(2);
  } else if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    m_System->SetSelected(3);
  } else {
    m_System->SetSelected(0);
  }
  m_System->Create();
  
  m_MainPanel->AddFrame(m_System, SystemLayout);


  FinalizeCreate();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUICoordinateSystem::OnApply()
{
  // The Apply button has been pressed

  if (m_System->GetSelected() == 0) {
    if (m_GUIData->GetCoordinateSystem() != MCoordinateSystem::c_Galactic) m_GUIData->SetCoordinateSystem(MCoordinateSystem::c_Galactic);
  } else if (m_System->GetSelected() == 1) {
    if (m_GUIData->GetCoordinateSystem() != MCoordinateSystem::c_Spheric) m_GUIData->SetCoordinateSystem(MCoordinateSystem::c_Spheric);
  } else if (m_System->GetSelected() == 2) {
    if (m_GUIData->GetCoordinateSystem() != MCoordinateSystem::c_Cartesian2D) m_GUIData->SetCoordinateSystem(MCoordinateSystem::c_Cartesian2D);
  } else if (m_System->GetSelected() == 3) {
    if (m_GUIData->GetCoordinateSystem() != MCoordinateSystem::c_Cartesian3D) m_GUIData->SetCoordinateSystem(MCoordinateSystem::c_Cartesian3D);
  } 

  return true;
}


// MGUICoordinateSystem: the end...
////////////////////////////////////////////////////////////////////////////////
