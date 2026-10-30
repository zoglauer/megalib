/*
 * MGUIARM.h
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


#ifndef __MGUIARM__
#define __MGUIARM__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TApplication.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextBuffer.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMimrec.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


enum class MGUIARMModes
{
  m_ARMGamma = 1,
  m_ARMElectron = 2,
  m_Spectrum = 3,
  m_PET = 4
};


////////////////////////////////////////////////////////////////////////////////

//! The position selection dialog box called by all interfaces which allow
//! this selection
//! It has 3 modes: ARM Gamma, ARM Electron, Spectrum
//! This window will wait for being unmapped and not be deleted on close
class MGUIARM : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIARM(const TGWindow* Parent, const TGWindow* Main, MSettingsMimrec* Data, MGUIARMModes m_Mode, bool& OkPressed);
  //! Default destructor
  virtual ~MGUIARM();

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
  //! The user settings data
  MSettingsMimrec* m_Settings;
  //! The sub type of the UI
  MGUIARMModes m_Mode;

  //! The return message that OK was pressed
  bool& m_OkPressed;

  //! Check button indicating if we do want to use a test position in spectral mode
  TGCheckButton* m_UseTestPosition;
  const int m_UseTestPositionID = 140;

  //! Check button indicating if we do want to use a logarithmic binning in spectral mode
  TGCheckButton* m_UseLog;
  const int m_UseLogID = 141;
  
  MGUIEEntry* m_ThetaIsX;
  MGUIEEntry* m_PhiIsY;
  MGUIEEntry* m_RadiusIsZ;
  MGUIEEntry* m_Distance;
  MGUIEEntry* m_Bins;

  
#ifdef ___CLING___
 public:
  ClassDef(MGUIARM, 0) // Dialogbox: Select the ARM-parameters
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
