/*
 * MGUIETransceiver.h
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


#ifndef __MGUIETransceiver__
#define __MGUIETransceiver__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>

// Standard libs::

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"
#include "MTransceiverTcpIp.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIETransceiver : public MGUIElement
{
  // public interface:
 public:
  MGUIETransceiver(const TGWindow* Parent, MTransceiverTcpIp* Transceiver); 
  virtual ~MGUIETransceiver();

  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  bool IsMarked();

  MTransceiverTcpIp* GetTransceiver();
  
  using TGWindow::SetName;
  void SetName(MString Name);
  using TGWindow::GetName;
  MString GetName();

  void SetHostName(MString HostName);
  MString GetHostName();

  void SetPort(unsigned int m_Port);
  unsigned int GetPort();

  // private methods:
 private:
  bool OnConnect();
  bool OnDisconnect();


  // private members:
 private:
  MTransceiverTcpIp* m_Transceiver;

  TGLayoutHints* m_LabelFrameLayout;
  TGCompositeFrame* m_LabelFrame;

  TGLayoutHints* m_LabelLayout;
  TGCheckButton* m_Label;

  TGLayoutHints* m_ButtonFrameLayout;
  TGCompositeFrame* m_ButtonFrame;

  TGLayoutHints* m_ButtonLayout;
  TGTextButton* m_ConnectButton;
  TGTextButton* m_DisconnectButton;

  TGLayoutHints* m_EntryLayout;
  MGUIEEntry* m_Host;
  MGUIEEntry* m_Port;

  TGLayoutHints* m_StatusLayout;
  TGLabel* m_Status;

  enum Type { e_Id = 500, e_Connect, e_Disconnect, e_Host, e_Port };


#ifdef ___CLING___
 public:
  ClassDef(MGUIETransceiver, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
