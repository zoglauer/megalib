/*
 * MGUIExposure.h
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


#ifndef __MGUIExposure__
#define __MGUIExposure__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExposure : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIExposure(const TGWindow* Parent, const TGWindow* Main, 
               MSettingsImaging* Settings);
  virtual ~MGUIExposure();

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
  MSettingsImaging* m_Settings;

  TGLayoutHints* m_FileSelectorLayout;
  MGUIEFileSelector* m_FileSelector;


#ifdef ___CLING___
 public:
  ClassDef(MGUIExposure, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
