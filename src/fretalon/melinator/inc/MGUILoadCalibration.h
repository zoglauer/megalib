/*
 * MGUILoadCalibration.h
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


#ifndef __MGUILoadCalibration__
#define __MGUILoadCalibration__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMelinator.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"
#include "MIsotopeStore.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! GUI dialog showing the options for loading ans classifying the calibration file
class MGUILoadCalibration : public MGUIDialog
{
  // Public Interface:
 public:
  MGUILoadCalibration(const TGWindow* Parent, const TGWindow* Main, MSettingsMelinator* Settings);
  virtual ~MGUILoadCalibration();

  
  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! Add the isotopes to the combo box
  void AddIsotopes(TGComboBox* ComboBox, MString Select);


  // protected members:
 protected:


  // private members:
 private:
  TGWindow* m_Parent;

  MSettingsMelinator* m_Settings;

  vector<MGUIEFileSelector*> m_FileNames;
  vector<vector<TGComboBox*>> m_Isotopes;
  vector<MGUIEEntry*> m_GroupIDs;

  //! The isotope store from which we load the isotopes
  MIsotopeStore m_IsotopeStore;
  

#ifdef ___CLING___
 public:
  ClassDef(MGUILoadCalibration, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
