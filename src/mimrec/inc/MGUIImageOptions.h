/*
 * MGUIImageOptions.h
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


#ifndef __MGUIImageOptions__
#define __MGUIImageOptions__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGFrame.h"
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIESlider.h"
#include "MGUIERBList.h"
#include "MGUIEEntryList.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIImageOptions : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIImageOptions(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data);
  //! default destructor
  virtual ~MGUIImageOptions();

  // protected methods:
 protected:
  //! Create the GUI
  virtual void Create();
  //! Apply the data
  virtual bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The GUI data
  MSettingsImaging* m_GUIData;

  //! GUI element for the palette
  TGComboBox* m_Palette;
  //! GUI element for the drawing mode
  TGComboBox* m_DrawMode;
  //! GUI element for the source catalog
  MGUIEFileSelector* m_SourceCatalog;


#ifdef ___CLING___
 public:
  ClassDef(MGUIImageOptions, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
