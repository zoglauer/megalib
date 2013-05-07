/*
 * MEventReceiver.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEventReceiver__
#define __MEventReceiver__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MHitEvent;
class TSocket;
class TMonitor;
class TServerSocket;
class TMessage;


////////////////////////////////////////////////////////////////////////////////


class MEventReceiver : public TObject
{
  // public interface:
 public:
  MEventReceiver();
  ~MEventReceiver();

  void SetPort(UInt_t Port);

  MHitEvent* ReceiveOneEvent();

  Bool_t Connect();
  Bool_t IsConnected();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:

  // public members
 public:
  enum Status { c_Online, kOffline };

  // private members:
 private:
  TServerSocket *m_ServerSocket;
  TSocket *m_Socket;
  TMessage *m_Message;

  Bool_t m_Connected;

  UInt_t m_NSentBytes;
  UInt_t m_NReceivedBytes;
  UInt_t m_NReceivedEvents;

  UInt_t m_Port;


#ifdef ___CINT___
 public:
  ClassDef(MEventReceiver, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
