/*
 * MGUIEConnection.h
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


#ifndef __MGUIEConnection__
#define __MGUIEConnection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>

// Standard libs::
#include <vector>
using std::vector;

// MEGAlib libs:
#include "MGlobal.h"
#include "MConnection.h"
#include "MGUIElement.h"
#include "MGUIEComboBox.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEConnection : public MGUIElement
{
  // public interface:
 public:
  MGUIEConnection(const TGWindow* Parent, MConnection* Connection); 
  virtual ~MGUIEConnection();

  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  void AddTransceiverName(MString Name);
  void RemoveTransceiverName(MString Name);

  bool IsMarked();
  MConnection* GetConnection();

  // private methods:
 private:
  bool OnConnect();
  bool OnDisconnect();


  // private members:
 private:
  MConnection* m_Connection;
  vector<MString> m_TransceiverNames;

  TGLayoutHints* m_LabelLayout;
  TGCheckButton* m_Label;

  TGLayoutHints* m_ComboLayout;
  MGUIEComboBox* m_Type;
  MGUIEComboBox* m_Start;
  MGUIEComboBox* m_Stop;

  enum Type { e_Id = 500 };


#ifdef ___CLING___
 public:
  ClassDef(MGUIEConnection, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
