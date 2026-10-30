/*
 * MGUIResponseSelection.h
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


#ifndef __MGUIResponseSelection__
#define __MGUIResponseSelection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIERBList.h"
#include "MResponseType.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Default UI to select the response type
class MGUIResponseSelection : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIResponseSelection(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data = 0);
  //! Default destructor
  virtual ~MGUIResponseSelection();

  // protected methods:
 protected:
  //! Create the UI
  virtual void Create();
  //! Apply the set data 
  virtual bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The settings store
  MSettingsImaging* m_Settings;
  //! A radiobutton list of the different response choices
  MGUIERBList* m_ResponseChoice;


#ifdef ___CLING___
 public:
  ClassDef(MGUIResponseSelection, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
