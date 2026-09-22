/*
 * MGUIETransceivers.h
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
