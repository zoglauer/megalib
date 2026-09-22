/*
 * MGUISaveAsSelector.h
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


#ifndef __MGUISaveAsSelector__
#define __MGUISaveAsSelector__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMelinator.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"
#include "MGUIEMinMaxEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! GUI dialog to select the default file name for ecal, fits, and report
class MGUISaveAsSelector : public MGUIDialog
{
  // Public Interface:
 public:
  //! Default constructor
  MGUISaveAsSelector(const TGWindow* Parent, const TGWindow* Main, MSettingsMelinator* Settings, bool& OKPressed);
  //! Default destructor
  virtual ~MGUISaveAsSelector();

  
  // protected methods:
 protected:
  //! Create the UI
  virtual void Create();
  //! Apply changes
  virtual bool OnApply();
  //! Process all messages
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);



  // protected members:
 protected:


  // private members:
 private:
  //! The return message that OK was pressed
  bool& m_OkPressed;

  //! The settings file
  MSettingsMelinator* m_Settings;

  //! The report file name
  MGUIEFileSelector* m_FileName;


#ifdef ___CLING___
 public:
  ClassDef(MGUISaveAsSelector, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
