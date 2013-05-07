/*
 * MGUIResponseParameterPRM.cxx
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
// MGUIResponseParameterPRM
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIResponseParameterPRM.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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

  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 20, 20, 5, 0);

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
