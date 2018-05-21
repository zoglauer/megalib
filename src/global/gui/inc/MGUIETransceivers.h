/*
 * MGUIETransceivers.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIETransceivers__
#define __MGUIETransceivers__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>
#include <TGCanvas.h>

// Standard libs::
#include <vector>
using std::vector;

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"
#include "MTransceiverTcpIp.h"
#include "MGUIETransceiver.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIETransceivers : public MGUIElement
{
  // public interface:
 public:
  MGUIETransceivers(const TGWindow *Parent, MString Label, bool Emphasize); 
  virtual ~MGUIETransceivers();

  void Create();
  void Associate(TGCompositeFrame* w);

  void Add(MTransceiverTcpIp* Transceiver);
  void Remove(MTransceiverTcpIp* Transceiver);

  vector<MTransceiverTcpIp*> GetMarked();

  // private methods:
 private:
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // private members:
 private:
  TGCompositeFrame* m_MessageWindow;

  MString m_Label;

  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;

  TGLayoutHints* m_TransceiversLayout;
  TGCanvas* m_Transceivers;
  TGCompositeFrame* m_Container;

  TGLayoutHints* m_TransceiverLayout;
  vector<MGUIETransceiver*> m_TransceiverList;

  TGLayoutHints* m_ButtonFrameLayout;
  TGCompositeFrame* m_ButtonFrame;

  TGLayoutHints* m_ButtonLayout;
  TGTextButton* m_AddButton;
  TGTextButton* m_RemoveButton;

  enum ButtonIDs { e_Add = 200, e_Remove };


#ifdef ___CLING___
 public:
  ClassDef(MGUIETransceivers, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
