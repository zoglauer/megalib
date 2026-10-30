/*
 * MGUIPrelude.h
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


#ifndef __MGUIPrelude__
#define __MGUIPrelude__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGTextView.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIPrelude : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIPrelude(MString Title, MString SubTitle, MString Text, MString OKButton, MString CancelButton = "");
  virtual ~MGUIPrelude();

  //! Close the window
  virtual void CloseWindow();

  bool IsOKed();
  void Create();


  // protected methods:
 protected:
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  bool OnOK();
  bool OnCancel();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Title;
  MString m_SubTitle;
  MString m_Text;
  MString m_OKButtonText;
  MString m_CancelButtonText;
   
  bool m_IsOKed;

  TGTextView* m_TextView;
  TGLayoutHints* m_TextLayout;

  TGHorizontalFrame* m_ButtonFrame;
  TGLayoutHints* m_ButtonFrameLayout;

  TGTextButton* m_OKButton;
  TGTextButton* m_CancelButton;
  TGLayoutHints* m_ButtonLayout;



#ifdef ___CLING___
 public:
  ClassDef(MGUIPrelude, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
