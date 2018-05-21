/*
 * TObjectReceiver.cxx
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
// TObjectReceiver
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MObjectReceiver.h"

// Standard libs:
#include <iostream>
// ROOT libs:
#include <TServerSocket.h>
#include <TSocket.h>
#include <TMonitor.h>
#include <TMessage.h>
#include <TH1.h>
#include <TSystem.h>
#include <TThread.h>
#include <TList.h>

// MEGAlib libs:
#include "MEventData.h"
#include "MHouseData.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(TObjectReceiver)
#endif


////////////////////////////////////////////////////////////////////////////////


void* StartReceiverThread(void* ObjectReceiver)
{
  ((TObjectReceiver *) ObjectReceiver)->ReceiveLoop();
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


TObjectReceiver::TObjectReceiver(UInt_t Port, Bool_t MultiThreaded)
{
  //

  m_NReceivedEvents = 0;
  m_NReceivedBytes = 0;
  m_NSentBytes = 0;

  m_Port = Port;

  m_Socket = 0;
  m_ServerSocket = 0;
  m_Connected = kFALSE;

  m_MultiThreaded = MultiThreaded;
  m_ReceivingEnabled = kFALSE;

  m_ObjectsList = new TList();
  m_NStoredObjects = 0;

  if (m_MultiThreaded == kFALSE) {
    Connect();
  } else {
    m_ReceiverThread = 
      new TThread("ReceiverThread", 
                  (void(*) (void *)) &StartReceiverThread, 
                  (void*) this);
    m_ReceiverThread->SetPriority(TThread::kLowPriority);
    m_ReceiverThread->Run();
  
    while (m_ReceiverThread->GetState() != TThread::kRunningState) {
      gSystem->ProcessEvents();
    }
    cout<<"Receiver thread running!"<<endl;    
  }
}


////////////////////////////////////////////////////////////////////////////////


TObjectReceiver::~TObjectReceiver()
{
  // default destructor

  delete m_Socket;
  //delete m_ServerSocket;
}


////////////////////////////////////////////////////////////////////////////////


void TObjectReceiver::SetPort(UInt_t Port)
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


Bool_t TObjectReceiver::Connect()
{
  //

  if (m_MultiThreaded == kTRUE) {
    return kFALSE;
  }

  if ((Int_t) m_Socket > 0) {
    m_Socket->Close();
    delete m_Socket;
  }


  if (m_ServerSocket == 0) {
    m_ServerSocket = new TServerSocket(m_Port, kTRUE);
    m_ServerSocket->SetOption(kNoBlock,1);
  }

  m_Socket = m_ServerSocket->Accept();
  if ((Int_t) m_Socket > 0) {
  m_Socket->SetOption(kRecvBuffer, 1000000);
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


void TObjectReceiver::Disconnect()
{
  //

  if (m_MultiThreaded == kTRUE) {
    return;
  }

  if (IsConnected() == kTRUE) {
    m_Socket->Close();
    delete m_Socket;
    m_Socket = 0;
    m_ServerSocket->Close();
    delete m_ServerSocket;
    m_ServerSocket = 0;

    m_Connected = kFALSE;
  }
}


////////////////////////////////////////////////////////////////////////////////


TObject* TObjectReceiver::ReceiveOneObject()
{
  // Receive one event

  TObject* Object;

  // Multiple threads case:
  if (m_MultiThreaded == kTRUE) {
    
    if (m_ReceivingEnabled == kFALSE) {
      m_ReceivingEnabled = kTRUE;
    }

    if (m_NStoredObjects == 0) {
      return 0;
    }

    m_NStoredObjects--;
    Object = m_ObjectsList->First();
    m_ObjectsList->Remove(m_ObjectsList->First());
    return Object;
  } 
  // Single thread case:
  else {

    if (IsConnected() == kFALSE) {
      if (Connect() == kFALSE) {
        return 0;
      }
    }

    TMessage *Message;
    Int_t BytesToRead;
    m_Socket->GetOption(kBytesToRead, BytesToRead);
    if (BytesToRead > 0) {
      m_Socket->Recv(Message);
    } else {
      return 0;
    }
    m_NReceivedBytes = m_Socket->GetBytesRecv();
    
    if (Message == 0) {
      cout<<"Connection lost"<<endl;
      m_Connected = 0;
      return 0;
    }

    if (Message->What() == kMESS_OBJECT) {
      Object = Message->ReadObject(Message->GetClass());
    } else {
      cout<<"Unexpected message: "<<(int) Message->What()<<endl;
    }

    delete Message;
  
    return Object;
  }
}


////////////////////////////////////////////////////////////////////////////////


void TObjectReceiver::ReceiveLoop()
{
  // This is the loop where the receiver thread waits/sleeps for events in the queue. 
  // Do NOT call the method explicitly, its automatically called when 
  // calling the constructor with MultiThreaded == kTRUE

  Bool_t Connected = kFALSE;

  //TObject *Object;
  TMessage *Message;
  TServerSocket *ServerSocket = 0;
  TSocket *Socket = 0;

  while (1) {

    // Test id receiving is allowed:
    if (m_ReceivingEnabled == kFALSE) {
      gSystem->Sleep(500);
      continue;
    }

    // Test if connection has been established ..

    if (Connected == kFALSE) {
      // Try to reconnect or sleep otherwise:
      TThread::Lock();
      if (ServerSocket == 0) {
        ServerSocket = new TServerSocket(m_Port, kTRUE);
        ServerSocket->SetOption(kNoBlock,1);
      }
      TThread::UnLock();

      Socket = ServerSocket->Accept();
      if ((Int_t) Socket > 0) {
        //Socket->SetOption(kNoBlock, 0);
        cout<<"Connected!"<<endl;
        Connected = kTRUE;
      } else {
        Connected = kFALSE;
        cout<<"Not connected! Total received events: "<<m_NReceivedEvents<<endl;
        gSystem->Sleep(500);
        continue;
      }
    }

    // Add the object to the list:
    TThread::Lock();
    Socket->Recv(Message);
    TThread::UnLock();
    //m_NReceivedBytes = m_Socket->GetBytesRecv();

    if (Message == 0) {
      cout<<"Connection lost"<<endl;
      Connected = kFALSE;
      continue;
    }

    TThread::Lock();
    if (Message->What() == kMESS_OBJECT) {
      m_ObjectsList->AddLast(Message->ReadObject(Message->GetClass()));
      m_NStoredObjects++;
    } else {
      cout<<"Unexpected message: "<<(int) Message->What()<<endl;
    }
    delete Message;
    TThread::UnLock();

  } // and loop again
}


////////////////////////////////////////////////////////////////////////////////


Bool_t TObjectReceiver::IsConnected()
{
  //

  return m_Connected;
}


// TObjectReceiver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
