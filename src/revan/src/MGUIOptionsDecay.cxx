/*
 * MGUIOptionsDecay.cxx
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
// MGUIOptionsDecay
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsDecay.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsDecay)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDecay::MGUIOptionsDecay(const TGWindow* Parent, const TGWindow* Main, 
                                   MSettingsEventReconstruction* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIOptionsDecay and bring it to the screen

  m_Data = Data;

  m_FileSelector = 0;
  m_FileSelectorLayout = 0;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDecay::~MGUIOptionsDecay()
{
  // Delete an instance of MGUIOptionsDecay

  delete m_FileSelector;
  delete m_FileSelectorLayout;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsDecay::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Options for decay detection");  
  
  if (m_Data->GetDecayAlgorithm() == 1) {
    AddSubTitle("Options for decay detection"); 
    m_FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 2);
    m_FileSelector = 
      new MGUIEFileSelector(this, "File containing decay lines:", 
                            m_Data->GetDecayFileName());
    m_FileSelector->SetFileType("Decay file", "*.decay");
    AddFrame(m_FileSelector, m_FileSelectorLayout);
  } else {
    AddSubTitle("You deselected decay detection!");     
  }
    
  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsDecay::OnApply()
{
	// The Apply button has been pressed

  if (m_Data->GetDecayAlgorithm() == 1) {
    m_Data->SetDecayFileName(m_FileSelector->GetFileName());
  }

	return true;
}


// MGUIOptionsDecay: the end...
////////////////////////////////////////////////////////////////////////////////
