/*
 * MGUIETransceiver.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  
  void SetName(MString Name);
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


#ifdef ___CINT___
 public:
  ClassDef(MGUIETransceiver, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
