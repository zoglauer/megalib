/*
 * MGUIMultiInput.h
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


#ifndef __MGUIMultiInput__
#define __MGUIMultiInput__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextEntry.h>
#include <TObjArray.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIData.h"



////////////////////////////////////////////////////////////////////////////////


class MGUIMultiInput : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIMultiInput(const TGWindow* Parent, const TGWindow* Main, 
                 MString Title, MString SubTitle, int nEntries,
                 MString* Label, MString* Input);
  ~MGUIMultiInput();


  // protected methods:
 protected:
  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Title;      // Title of the window
  MString m_SubTitle;   // Description of the purpose of this dialog 
  
  int m_NEntries;     // number textboxes

  MString* m_Label;     // labels of the textboxes
  MString* m_Input;     // pointer to the default and returned text

  //
  TGVerticalFrame* m_LabelFrame;
  TGLayoutHints* m_LabelFrameLayout;

  TGLayoutHints* m_SubTitleLabelLayout;
  TObjArray* m_SubTitleLabel;

  //
  TGHorizontalFrame* m_AddFrame;
  TGLayoutHints* m_AddFrameLayout;
  
  TGVerticalFrame* m_AddLabelsFrame;
  TGLayoutHints* m_AddLabelsFrameLayout;

  TObjArray* m_NameLabel;
  TGLayoutHints* m_NameLabelLayout;

  TGVerticalFrame* m_AddEntryFrame;
  TGLayoutHints* m_AddEntryFrameLayout;

  TObjArray* m_NameEntry;
  TGLayoutHints* m_NameEntryLayout;

  //
  TGHorizontalFrame* m_ButtonFrame;
  TGLayoutHints* m_ButtonFrameLayout;

  TGTextButton* m_CancelButton;
  TGLayoutHints* m_CancelButtonLayout;

  TGTextButton* m_OKButton;
  TGLayoutHints* m_OKButtonLayout;



#ifdef ___CLING___
 public:
  ClassDef(MGUIMultiInput, 0) // multiple input dialog box
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
