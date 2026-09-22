/*
 * MGUIExportSpectrum.h
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


#ifndef __MGUIExportSpectrum__
#define __MGUIExportSpectrum__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGButton.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsRevan.h"
#include "MGUIEEntry.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExportSpectrum : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIExportSpectrum(const TGWindow* Parent, const TGWindow* Main, 
                     MSettingsRevan* Data, bool& OKPressed);
  virtual ~MGUIExportSpectrum();


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
  //! The GUI data
  MSettingsRevan* m_Data;

  //! The number of bins
  MGUIEEntry* m_Bins;
  //! Logarithmic display
  TGCheckButton* m_Log;
  // The minimum and maximum energy value
  MGUIEMinMaxEntry* m_Range;
  //! The file selection dialog
  MGUIEFileSelector* m_File;

  //! True if OK button has been pressed
  bool& m_OKPressed;


#ifdef ___CLING___
 public:
  ClassDef(MGUIExportSpectrum, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
