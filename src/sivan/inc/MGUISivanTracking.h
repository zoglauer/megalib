/*
 * MGUISivanTracking.h
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


#ifndef __MGUISivanTracking__
#define __MGUISivanTracking__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT libs
#include <TGFrame.h>
#include <TGLabel.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsSivan.h"


////////////////////////////////////////////////////////////////////////////////


class MGUISivanTracking : public MGUIDialog
{
  // Public Interface:
 public:
  MGUISivanTracking(const TGWindow* Parent, const TGWindow* Main, MSettingsSivan* Data);
  virtual ~MGUISivanTracking();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long msg, long param1, long);
  virtual bool OnApply();

  // private members:
 private:
  MSettingsSivan* m_GUIData;
  int m_Realism;

  // 
  TGRadioButton* m_RealRB[3];
  TGLayoutHints* m_RBLayout;


#ifdef ___CLING___
 public:
  ClassDef(MGUISivanTracking, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
