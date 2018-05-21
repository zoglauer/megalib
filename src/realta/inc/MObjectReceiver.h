/*
 * TObjectReceiver.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __TObjectReceiver__
#define __TObjectReceiver__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class TSocket;
class TMonitor;
class TServerSocket;
class TThread;
class TList;

////////////////////////////////////////////////////////////////////////////////


void* StartReceiverThread(void* ObjectReceiver);


////////////////////////////////////////////////////////////////////////////////


class TObjectReceiver : public TObject
{
  // public interface:
 public:
  TObjectReceiver(UInt_t Port = 9090, Bool_t MultiThreaded = kFALSE);
  ~TObjectReceiver();

  void SetPort(UInt_t Port);

  TObject* ReceiveOneObject();

  void ReceiveLoop();

  Bool_t Connect();
  void Disconnect();
  Bool_t IsConnected();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TServerSocket *m_ServerSocket;
  TSocket *m_Socket;

  Bool_t m_Connected;
  Bool_t m_ReceivingEnabled;

  UInt_t m_NSentBytes;
  UInt_t m_NReceivedBytes;
  UInt_t m_NReceivedEvents;

  UInt_t m_Port;

  Bool_t m_MultiThreaded;
  TThread *m_ReceiverThread;

  TList *m_ObjectsList;
  UInt_t m_NStoredObjects;


#ifdef ___CLING___
 public:
  ClassDef(TObjectReceiver, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
