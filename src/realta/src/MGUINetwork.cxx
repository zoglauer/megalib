/*
 * MGUINetwork.cxx
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
// MGUINetwork
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUINetwork.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGUIEEntry.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUINetwork)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUINetwork::MGUINetwork(const TGWindow* Parent, const TGWindow* Main, MSettingsRealta* Settings)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUINetwork and bring it to the screen

  m_Settings = Settings;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUINetwork::~MGUINetwork()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUINetwork::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Network connection");  

  AddSubTitle("Set the TCP/IP connection options"); 

  // The Host field
  TGLayoutHints* EntryLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 2, 2);
  m_Host = new MGUIEEntry(this, "Host name:", false, m_Settings->GetHostName());
  m_Host->SetEntryFieldSize(200);
  AddFrame(m_Host, EntryLayout);

  // The Port field
  m_Port = new MGUIEEntry(this, "Port:", false, m_Settings->GetPort());
  AddFrame(m_Port, EntryLayout);

  // The autostart option:
  TGLayoutHints* OptionsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 20, 2);
  m_ConnectOnStart = new TGCheckButton(this, "Connect at program start");
  m_ConnectOnStart->Associate(this);
  if (m_Settings->GetConnectOnStart() == true) {
    m_ConnectOnStart->SetState(kButtonDown);
  } else {
    m_ConnectOnStart->SetState(kButtonUp);    
  }  
  AddFrame(m_ConnectOnStart, OptionsLayout);

  m_TransceiverMode = new MGUIERBList(this, "Transceiver mode:", false);
  m_TransceiverMode->Add("ASCII Text embedded in a ROOT TMessage");
  m_TransceiverMode->Add("Raw event list (event or event list must be terminated with an \"EN\")");
  m_TransceiverMode->SetSelected(m_Settings->GetTransceiverMode());
  m_TransceiverMode->Create();
  AddFrame(m_TransceiverMode, OptionsLayout);


  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth()+30, GetDefaultHeight()+20);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINetwork::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application
  
  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINetwork::OnApply()
{
  // Set all data 
  
  m_Settings->SetHostName(m_Host->GetAsString());
  m_Settings->SetPort(m_Port->GetAsInt());

  if (m_ConnectOnStart->GetState() == kButtonDown) {
    m_Settings->SetConnectOnStart(true);
  } else {
    m_Settings->SetConnectOnStart(false);    
  }
  m_Settings->SetTransceiverMode(m_TransceiverMode->GetSelected());
  
  return true;
}


// MGUINetwork: the end...
////////////////////////////////////////////////////////////////////////////////
