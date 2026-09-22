/*
 * MGUIOptionsCoincidence.cxx
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


#ifdef ___CLING___
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
