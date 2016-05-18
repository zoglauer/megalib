/*
 * MGUIETransceiver.cxx
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
// MGUIETransceiver
//
//
// This class is an elementary GUI-widget:
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIETransceiver.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIETransceiver)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIETransceiver::MGUIETransceiver(const TGWindow *Parent, MTransceiverTcpIp* Transceiver)
  : MGUIElement(Parent, kRaisedFrame | kSunkenFrame | kVerticalFrame)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label

  m_Transceiver = Transceiver;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIETransceiver::~MGUIETransceiver()
{
  // Destruct this instance of MGUIETransceiver

  if (MustCleanup() == kNoCleanup) {
    delete m_Label;
    delete m_LabelLayout;
    
    delete m_ConnectButton;
    delete m_DisconnectButton;
    delete m_ButtonLayout;
    
    delete m_ButtonFrame;
    delete m_ButtonFrameLayout;
    
    delete m_LabelFrame;
    delete m_LabelFrameLayout;

    delete m_Host;
    delete m_Port;
    delete m_EntryLayout;
    
    //m_Transceiver->GetStatusStream().Disconnect(m_Status);
    delete m_Status;
    delete m_StatusLayout;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIETransceiver::Create()
{
  // Create the label and the input-field.

  // Label:
  m_LabelFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  m_LabelFrame = new TGCompositeFrame(this, 100, 20, kHorizontalFrame);
  AddFrame(m_LabelFrame, m_LabelFrameLayout);

  m_Label = new TGCheckButton(m_LabelFrame, m_Transceiver->GetName(), e_Id);
  m_LabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 0, 0, 5, 0);
  m_Label->Associate(this);
  m_LabelFrame->AddFrame(m_Label, m_LabelLayout);

  // Buttons:
  m_ButtonFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsCenterY, 0, 0, 0, 0);
  m_ButtonFrame = new TGCompositeFrame(m_LabelFrame, 205, 20, kHorizontalFrame | kFixedWidth);
  m_LabelFrame->AddFrame(m_ButtonFrame, m_ButtonFrameLayout);

  m_ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsCenterY, 5, 0, 0, 0);
  m_ConnectButton = new TGTextButton(m_ButtonFrame, "Connect", e_Connect);
  m_ConnectButton->Associate(this);
  m_ButtonFrame->AddFrame(m_ConnectButton, m_ButtonLayout);
  m_DisconnectButton = new TGTextButton(m_ButtonFrame, "Disconnect", e_Disconnect); 
  m_DisconnectButton->Associate(this);
  m_ButtonFrame->AddFrame(m_DisconnectButton, m_ButtonLayout);

  m_EntryLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 0, 5, 0);

  // The Host field
  m_Host = new MGUIEEntry(this, "Host name:", false, m_Transceiver->GetHost());
  m_Host->SetEntryFieldSize(200);
  m_Host->Associate(this, e_Host);
  AddFrame(m_Host, m_EntryLayout);

  // The Port field
  m_Port  = new MGUIEEntry(this, "Port:", false, m_Transceiver->GetPort(), true, 0l);
  m_Port->SetEntryFieldSize(200);
  m_Port->Associate(this, e_Port);
  AddFrame(m_Port, m_EntryLayout);

  m_StatusLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 0, 5, 0);
  m_Status = new TGLabel(this, "Not connected");
  AddFrame(m_Status, m_StatusLayout);
  //m_Transceiver->GetStatusStream().Connect(m_Status);
  //m_Transceiver->UpdateStatus();

  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIETransceiver::ProcessMessage(long Message, long Parameter1, 
                                  long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:

  switch (GET_MSG(Message)) {
  case kC_ENTRY:
    switch (GET_SUBMSG(Message)) {
    case kET_CHANGED:
      switch (Parameter1) {
      case e_Host:
        m_Transceiver->SetHost(m_Host->GetAsString());
        break;
      case e_Port:
        m_Transceiver->SetPort(m_Port->GetAsInt());
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;

  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_CHECKBUTTON:
      break;
    case kCM_BUTTON:
    case kCM_MENU:
      switch (Parameter1) {
      case e_Connect:
        OnConnect();
        break;

      case e_Disconnect:
        OnDisconnect();
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }

  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIETransceiver::IsMarked()
{
  return (m_Label->GetState() == kButtonDown) ? true : false;
}


////////////////////////////////////////////////////////////////////////////////


MTransceiverTcpIp* MGUIETransceiver::GetTransceiver()
{
  return m_Transceiver;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIETransceiver::OnConnect()
{
  m_Transceiver->SetPort(m_Port->GetAsInt());
  m_Transceiver->SetHost(m_Host->GetAsString());
  m_Transceiver->Connect();

  m_Host->SetEnabled(false);
  m_Port->SetEnabled(false);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIETransceiver::OnDisconnect()
{
  m_Transceiver->Disconnect();

  m_Host->SetEnabled(true);
  m_Port->SetEnabled(true);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIETransceiver::SetName(MString Name)
{
  m_Transceiver->SetName(Name);
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIETransceiver::GetName()
{
  return m_Transceiver->GetName();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIETransceiver::SetHostName(MString HostName)
{
  m_Transceiver->SetHost(HostName);
  m_Host->SetValue(HostName);
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIETransceiver::GetHostName()
{
  return m_Host->GetAsString();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIETransceiver::SetPort(unsigned int Port)
{
  m_Transceiver->SetPort(Port);
  m_Port->SetValue((long) Port);
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MGUIETransceiver::GetPort()
{
  return (unsigned int) m_Port->GetAsInt();
}



// MGUIETransceiver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
