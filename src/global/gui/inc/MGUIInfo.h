/*
 * MGUIInfo.h
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


#ifndef __MGUIInfo__
#define __MGUIInfo__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include "TObject.h"
#include <TApplication.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextBuffer.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"


////////////////////////////////////////////////////////////////////////////////


class MGUIInfo : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIInfo(const TGWindow* Parent, const TGWindow* Main);
  ~MGUIInfo();

  bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  void CloseWindow();

  // protected methods:
 protected:
  void Create();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TGVerticalFrame* m_InfoFrame;        // Frame for the subtitle
  TGLayoutHints* m_InfoFrameLayout;    // Layout for this frame
  TGLayoutHints* m_InfoTextLayout;     // Layout of the subtitles
  TObjArray* m_InfoText;               // Array of the subtitle rows


  TGHorizontalFrame* m_ButtonFrame;
  TGLayoutHints* m_ButtonFrameLayout;

  TGTextButton* m_OKButton;
  TGLayoutHints* m_OKButtonLayout;



#ifdef ___CLING___
 public:
  ClassDef(MGUIInfo, 0) // Dialogbox: Select the backprojection algorithm
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
