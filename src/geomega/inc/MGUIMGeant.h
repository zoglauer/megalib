/*
 * MGUIMGeant.h
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


#ifndef __MGUIMGeant__
#define __MGUIMGeant__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsGeomega.h"
#include "MGUIEFileSelector.h"
#include "MGUIECBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIMGeant : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIMGeant(const TGWindow* Parent, const TGWindow* Main, 
             MSettingsGeomega* Data, bool& OkPressed);
  virtual ~MGUIMGeant();


  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  virtual bool OnApply();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsGeomega* m_GUIData;

  TGRadioButton* m_StandardName;
  TGRadioButton* m_SpecialName;

  MGUIEFileSelector* m_FileSelector;

  MGUIECBList* m_MEGAlibExtensionOptions;

  bool& m_OkPressed;

  enum ButtonIDs {
    c_Min = 100, 
    c_StandardName,
    c_SpecialName,
    c_Max }; 


#ifdef ___CLING___
 public:
  ClassDef(MGUIMGeant, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
