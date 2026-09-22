/*
 * MGUIPQSelection.h
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


#ifndef __MGUIPQSelection__
#define __MGUIPQSelection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGListBox.h>
#include <TGTextEntry.h>
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMimrec.h"
#include "MGUIMultiInput.h"
#include "MPointSource.h"


////////////////////////////////////////////////////////////////////////////////


class MGUIPQSelection : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIPQSelection(const TGWindow* Parent, const TGWindow* Main, 
                  MSettingsMimrec* Data);
  virtual ~MGUIPQSelection();


  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  virtual bool OnApply();
  virtual bool OnCancel();

  void AddPointSource();
  void DeletePointSource();
  void ModifyPointSource();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsMimrec* m_GUIData;                // all the data of the GUI
  int m_NEntries;                   // number of pointsources in the list

  //
  TGVerticalFrame* m_LabelFrame;
  TGLayoutHints* m_LabelFrameLayout;

  TGLabel* m_Label1;
  TGLayoutHints* m_Label1Layout;
  TGLabel* m_Label2;
  TGLayoutHints* m_Label2Layout;

  //
  TGHorizontalFrame* m_ListBoxFrame;
  TGLayoutHints* m_ListBoxFrameLayout;

  TGListBox* m_ListBoxLong;
  TGLayoutHints* m_ListBoxLayout;

  //
  TGHorizontalFrame* m_AddDeleteFrame;
  TGLayoutHints* m_AddDeleteFrameLayout;

  TGTextButton* m_AddButton;
  TGLayoutHints* m_AddButtonLayout;

  TGTextButton* m_ModifyButton;
  TGLayoutHints* m_ModifyButtonLayout;

  TGTextButton* m_DeleteButton;
  TGLayoutHints* m_DeleteButtonLayout;

  TGCheckButton* m_UsePQsCB;
  TGLayoutHints* m_UsePQsCBLayout;

  //
  TGHorizontalFrame* m_ButtonFrame;
  TGLayoutHints* m_ButtonFrameLayout;

  TGTextButton* m_CancelButton;
  TGLayoutHints* m_CancelButtonLayout;

  TGTextButton* m_OKButton;
  TGLayoutHints* m_OKButtonLayout;



#ifdef ___CLING___
 public:
  ClassDef(MGUIPQSelection, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
