/*
 * MGUIPenalty.h
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


#ifndef __MGUIPenalty__
#define __MGUIPenalty__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>

// ROOT libs

#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextBuffer.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"


////////////////////////////////////////////////////////////////////////////////


class MGUIPenalty : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIPenalty(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging *Data);
  virtual ~MGUIPenalty();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsImaging* m_GUIData;

  TGVerticalFrame* m_LabelFrame;
  TGLayoutHints* m_LabelFrameLayout;

  TGLabel* m_Label1;
  TGLayoutHints* m_Label1Layout;
  TGLabel* m_Label2;
  TGLayoutHints* m_Label2Layout;

  TGRadioButton* m_RB[3];
  TGLayoutHints* m_RBLayout;


  TGHorizontalFrame* m_PenaltyAlphaFrame;
  TGLayoutHints* m_PenaltyAlphaFrameLayout;

  TGLabel* m_LabelPenaltyAlpha;
  TGLayoutHints* m_LabelPenaltyAlphaLayout;

  TGLayoutHints* m_PenaltyAlphaLayout;
  TGTextBuffer* m_PenaltyAlphaBuffer;
  TGTextEntry* m_PenaltyAlphaInput;


  TGHorizontalFrame* m_ButtonFrame;
  TGLayoutHints* m_ButtonFrameLayout;

  TGTextButton* m_CancelButton;
  TGLayoutHints* m_CancelButtonLayout;

  TGTextButton* m_OKButton;
  TGLayoutHints* m_OKButtonLayout;



#ifdef ___CLING___
 public:
  ClassDef(MGUIPenalty, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
