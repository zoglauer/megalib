/*
 * MGUIOptionsCoincidence.cxx
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
// MGUIOptionsCoincidence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsCoincidence.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MRawEventAnalyzer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsCoincidence)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsCoincidence::MGUIOptionsCoincidence(const TGWindow* Parent, 
                                               const TGWindow* Main, 
                                               MSettingsEventReconstruction* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIOptionsCoincidence and bring it to the screen

  m_Data = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsCoincidence::~MGUIOptionsCoincidence()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsCoincidence::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Options for clustering");  

  if (m_Data->GetCoincidenceAlgorithm() == MRawEventAnalyzer::c_CoincidenceAlgoWindow) {
    AddSubTitle("Options for coincidence search using a time window:"); 
    
    m_Window = new MGUIEEntry(this, "Time window [s]:", false, m_Data->GetCoincidenceWindow(), true, 0.0);
    TGLayoutHints* EntryLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 10, 0);
    AddFrame(m_Window, EntryLayout);
  } else {
    AddSubTitle("You deselected coincidence search!");     
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


bool MGUIOptionsCoincidence::OnApply()
{
  // The Apply button has been pressed

  if (m_Data->GetCoincidenceAlgorithm() == MRawEventAnalyzer::c_CoincidenceAlgoWindow) {
    m_Data->SetCoincidenceWindow(m_Window->GetAsDouble());
  }

  return true;
}


// MGUIOptionsCoincidence: the end...
////////////////////////////////////////////////////////////////////////////////
