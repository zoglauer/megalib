/*
 * MGUIOptionsGeneral.cxx
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
// MGUIOptionsGeneral
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsGeneral.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsGeneral)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsGeneral::MGUIOptionsGeneral(const TGWindow* Parent, 
                                       const TGWindow* Main, 
                                       MSettingsEventReconstruction* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIOptionsGeneral and bring it to the screen

  m_Data = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsGeneral::~MGUIOptionsGeneral()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsGeneral::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("General options");  
  AddSubTitle("General options and event selections\nfor the event reconstruction"); 

  m_MinMaxLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 10, 0);

  m_TotalEnergy = new MGUIEMinMaxEntry(this,
                                       "Total deposited energy [keV]:",
                                       false,
                                       m_Data->GetTotalEnergyMin(), 
                                       m_Data->GetTotalEnergyMax(), 
                                       true, 0.0);
  AddFrame(m_TotalEnergy, m_MinMaxLayout);

  m_LeverArm = new MGUIEMinMaxEntry(this,
                                    "Lever arm [cm]:",
                                    false,
                                    m_Data->GetLeverArmMin(), 
                                    m_Data->GetLeverArmMax(), 
                                    true, 0.0);
  AddFrame(m_LeverArm, m_MinMaxLayout);

  m_EventId = new MGUIEMinMaxEntry(this,
                                   "Event ID (-1 for unlimited)",
                                   false,
                                   m_Data->GetEventIdMin(), 
                                   m_Data->GetEventIdMax(), 
                                   true, -1l);
  AddFrame(m_EventId, m_MinMaxLayout);
  
  m_Options = new MGUIECBList(this, "Additional options:");
  m_Options->Add("Do not analyze events previously flagged as bad (\"BD\" keyword)");          
  m_Options->SetSelected(0, m_Data->GetRejectAllBadEvents());
  m_Options->Create();
  m_Options->Associate(this);
  TGLayoutHints* OptionsLayout = 
    new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 20, 10);
  AddFrame(m_Options, OptionsLayout);
  

  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsGeneral::OnApply()
{
  // The Apply button has been pressed

  if (m_TotalEnergy->CheckRange(0.0, numeric_limits<double>::max(), 
                                0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_LeverArm->CheckRange(0.0, numeric_limits<double>::max(), 
                             0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_EventId->CheckRange(-1l, numeric_limits<long>::max(), 
                            -1l, numeric_limits<long>::max(), false) == false) return false;

  m_Data->SetTotalEnergyMin(m_TotalEnergy->GetMinValue());
  m_Data->SetTotalEnergyMax(m_TotalEnergy->GetMaxValue());

  m_Data->SetLeverArmMin(m_LeverArm->GetMinValue());
  m_Data->SetLeverArmMax(m_LeverArm->GetMaxValue());

  m_Data->SetEventIdMin(m_EventId->GetMinValueInt());
  m_Data->SetEventIdMax(m_EventId->GetMaxValueInt());

  m_Data->SetRejectAllBadEvents(m_Options->GetSelected(0));

  return true;
}


// MGUIOptionsGeneral: the end...
////////////////////////////////////////////////////////////////////////////////
