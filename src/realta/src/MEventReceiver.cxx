/*
 * MEventReceiver.cxx                                   v0.1  01/01/2001
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
// MEventReceiver
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEventReceiver.h"

// Standard libs:
#include <iostream>
#include <TSystem.h>

// ROOT libs:
#include <TServerSocket.h>
#include <TSocket.h>
#include <TMonitor.h>
#include <TMessage.h>
#include <TH1.h>

// MEGAlib libs:
#include "MHitEvent.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MEventReceiver)
#endif


////////////////////////////////////////////////////////////////////////////////


MEventReceiver::MEventReceiver()
{
  //

  m_NReceivedEvents = 0;
  m_NReceivedBytes = 0;
  m_NSentBytes = 0;

  m_Port = 9090;

  m_Socket = 0;
  m_ServerSocket = 0;
  m_Connected = kFALSE;
  m_Message = 0;

  Connect();
}


////////////////////////////////////////////////////////////////////////////////


MEventReceiver::~MEventReceiver()
{
  // default destructor

  delete m_Socket;
  //delete m_ServerSocket;
}


////////////////////////////////////////////////////////////////////////////////


void MEventReceiver::SetPort(UInt_t Port)
{
  // Set a new port for the network connection and start the server with 
  // the new port ONLY when the server was already running

  m_Port = Port;

  if (m_ServerSocket != 0) {
    m_ServerSocket->Close();
    delete m_ServerSocket;
    m_ServerSocket = new TServerSocket(m_Port, kTRUE);
    m_ServerSocket->SetOption(kNoBlock,1);
  }
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MEventReceiver::Connect()
{
  //

  if (m_ServerSocket == 0) {
    m_ServerSocket = new TServerSocket(m_Port, kTRUE);
    m_ServerSocket->SetOption(kNoBlock,1);
  }

  m_Socket = m_ServerSocket->Accept();
  if ((Int_t) m_Socket > 0) {
    m_Connected = kTRUE;
    //cout<<"Connected"<<endl;
  } else {
    m_Connected = kFALSE;
    cout<<"Not connected! Total received events: "<<m_NReceivedEvents<<endl;
    gSystem->Sleep(500);
  }
  
  return m_Connected;
}


////////////////////////////////////////////////////////////////////////////////


MHitEvent* MEventReceiver::ReceiveOneEvent()
{
  // Receive one event

  if (IsConnected() == kFALSE) {
    if (Connect() == kFALSE) {
      return 0;
    }
  }

  MHitEvent* Event = 0;

  // Receive a new message:
  TMessage *Message;
  m_Socket->Recv(Message);
  m_NReceivedBytes = m_Socket->GetBytesRecv();

  if (Message == 0) {
    cout<<"Connection lost"<<endl;
    m_Connected = 0;
    m_Message = 0;
    return 0;
  }

  // Now do not evaluate this message, but the last one,
  // In case the connection broke during the last message we do not have a 
  // message now

  // Case: last time was an error
  if (m_Message == 0) {
    m_Message = Message;
    return 0;
  }

  // Case: the new event is ok and the last was ok
  if (m_Message->What() == kMESS_OBJECT) {
    Event = (MHitEvent *) m_Message->ReadObject(m_Message->GetClass());
    m_NReceivedEvents++;
    Int_t a = 0;
  } else {
    cout<<"Unexpected message: "<<(int) m_Message->What()<<endl;
  }

  // Now copy the message:
  delete m_Message;
  m_Message = Message;


  return Event;
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MEventReceiver::IsConnected()
{
  //

  return m_Connected;
}


// MEventReceiver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
