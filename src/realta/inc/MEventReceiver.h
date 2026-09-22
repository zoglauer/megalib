/*
 * MEventReceiver.h
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


#ifdef ___CLING___
 public:
  ClassDef(MEventReceiver, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
