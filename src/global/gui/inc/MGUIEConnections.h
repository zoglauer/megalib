/*
 * MGUIEConnections.h
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


#ifndef __MGUIEConnections__
#define __MGUIEConnections__


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
#include "MConnection.h"
#include "MGUIElement.h"
#include "MGUIEConnection.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEConnections : public MGUIElement
{
  // public interface:
 public:
  MGUIEConnections(const TGWindow* Parent, MString Label, bool Emphasize); 
  virtual ~MGUIEConnections();

  void Create();
  void Associate(TGCompositeFrame* w);

  void Add(MConnection* Connection);
  void Remove(MConnection* Connection);

  void AddTransceiverName(MString Name);
  void RemoveTransceiverName(MString Name);

  vector<MConnection*> GetMarked();

  // private methods:
 private:
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // private members:
 private:
  TGCompositeFrame* m_MessageWindow;

  MString m_Label;
  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;

  TGLayoutHints* m_ConnectionsLayout;
  TGCanvas* m_Connections;
  TGCompositeFrame* m_Container;

  TGLayoutHints* m_ConnectionLayout;
  vector<MGUIEConnection*> m_ConnectionList;
  vector<MString> m_TransceiverNames;

  TGLayoutHints* m_ButtonFrameLayout;
  TGCompositeFrame* m_ButtonFrame;

  TGLayoutHints* m_ButtonLayout;
  TGTextButton* m_AddButton;
  TGTextButton* m_RemoveButton;

  enum ButtonIDs { e_Add = 300, e_Remove };


#ifdef ___CLING___
 public:
  ClassDef(MGUIEConnections, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
