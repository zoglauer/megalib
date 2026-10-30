/*
 * MGUIEComboBox.h
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


#ifndef __MGUIEComboBox__
#define __MGUIEComboBox__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>
#include <TGComboBox.h>

// Standard libs::
#include "vector"
using std::vector;

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEComboBox : public MGUIElement
{
  // public interface:
 public:
  MGUIEComboBox(const TGWindow* Parent, MString Label, bool Emphasize = false, unsigned int Id = 1); 
  virtual ~MGUIEComboBox();

  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  void SetComboBoxSize(int Size = 100);

  void Add(MString Entry, bool IsSelected = false);
  void Remove(MString Entry);
  MString GetSelected();

  // private methods:
 private:
  void AddAll();

  // private members:
 private:
  unsigned int m_Id;

  MString m_Label;

  int m_Selected;
  int m_ComboBoxSize;
  vector<MString> m_Names;


  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;

  TGComboBox* m_ComboBox;
  TGLayoutHints* m_ComboBoxLayout;
  

  enum Type { e_Nothing };


#ifdef ___CLING___
 public:
  ClassDef(MGUIEComboBox, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
