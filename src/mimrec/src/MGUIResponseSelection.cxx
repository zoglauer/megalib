/*
 * MGUIResponseSelection.cxx
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
// MGUIResponseSelection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIResponseSelection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIResponseSelection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIResponseSelection::MGUIResponseSelection(const TGWindow* Parent, 
                                             const TGWindow* Main, 
                                             MSettingsImaging* Settings)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIResponseSelection and bring it to the screen

  m_Settings = Settings;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIResponseSelection::~MGUIResponseSelection()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIResponseSelection::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Response type selection");  

  AddSubTitle("Select the type of response you want to use"); 

  TGLayoutHints* ResponseChoiceLayout = new TGLayoutHints(kLHintsCenterX | kLHintsTop | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 0*m_FontScaler, 20*m_FontScaler);
  
  m_ResponseChoice = new MGUIERBList(this, "Response types:");
  m_ResponseChoice->Add("Standard one parameter linear gaussian for photo-peak events (List-mode)");
  m_ResponseChoice->Add("Standard one parameter linear gaussian calculated by uncertainties (List-mode)");
  m_ResponseChoice->Add("Standard one parameter linear gaussian with limited energy measurement (List-mode)");
  m_ResponseChoice->Add("Cone shapes determined by simulations -- handles only not-tracked Compton events (List-mode)");
  m_ResponseChoice->Add("Precalculated response matrices (List-mode)");
  if (m_Settings->GetResponseType() == MResponseType::GaussByUncertainties) {
    m_ResponseChoice->SetSelected(1);
  } else if (m_Settings->GetResponseType() == MResponseType::GaussByEnergyLeakage) {
    m_ResponseChoice->SetSelected(2);
  } else if (m_Settings->GetResponseType() == MResponseType::ConeShapes) {
    m_ResponseChoice->SetSelected(3);
  } else if (m_Settings->GetResponseType() == MResponseType::PRM) {
    m_ResponseChoice->SetSelected(4);
  } else {
    m_ResponseChoice->SetSelected(0);
  }
  m_ResponseChoice->Create();
  
  AddFrame(m_ResponseChoice, ResponseChoiceLayout);

  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIResponseSelection::OnApply()
{
  // The Apply button has been pressed
  
  MResponseType Type;
  if (m_ResponseChoice->GetSelected() == 1) {
    Type = MResponseType::GaussByUncertainties;
  } else if (m_ResponseChoice->GetSelected() == 2) {
    Type = MResponseType::GaussByEnergyLeakage;
  } else if (m_ResponseChoice->GetSelected() == 3) {
    Type = MResponseType::ConeShapes;
  } else if (m_ResponseChoice->GetSelected() == 4) {
    Type = MResponseType::PRM;
  } else {
    Type = MResponseType::Gauss1D;
  }
  
  if (Type != m_Settings->GetResponseType()) {
    m_Settings->SetResponseType(Type);
  }
 
  return true;
}


// MGUIResponseSelection: the end...
////////////////////////////////////////////////////////////////////////////////
