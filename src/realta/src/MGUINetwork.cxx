/*
 * MGUINetwork.cxx
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


#ifdef ___CINT___
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
