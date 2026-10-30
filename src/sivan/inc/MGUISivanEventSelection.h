/*
 * MGUISivanEventSelection.h
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


#ifndef __MGUISivanEventSelection__
#define __MGUISivanEventSelection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsSivan.h"
#include "MGUIESlider.h"
#include "MGUIERBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUISivanEventSelection : public MGUIDialog
{
  // Public Interface:
 public:
  MGUISivanEventSelection(const TGWindow* Parent, const TGWindow* Main, 
                          MSettingsSivan* Data);
  virtual ~MGUISivanEventSelection();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsSivan* m_GUIData;



#ifdef ___CLING___
 public:
  ClassDef(MGUISivanEventSelection, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
