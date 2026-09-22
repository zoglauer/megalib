/*
 * MGUIResponseParameterGaussianByUncertainties.h
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


#ifndef __MGUIResponseParameterGaussianByUncertainties__
#define __MGUIResponseParameterGaussianByUncertainties__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIResponseParameterGaussianByUncertainties : public MGUIDialog
{
  // Public Interface:
 public:
  //! DEfault constructor
  MGUIResponseParameterGaussianByUncertainties(const TGWindow* Parent, const TGWindow* Main, 
                                               MSettingsImaging* Data);
  //! Default destructor
  virtual ~MGUIResponseParameterGaussianByUncertainties();

  // protected methods:
 protected:
  //! Create the GUI
  virtual void Create();
  //! Handle Apply/OK button
  virtual bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! All the GUI data
  MSettingsImaging* m_GUIData;

  //! Increase the 1-sigma Gaussian width with this amount of degrees
  MGUIEEntry* m_Increase;


#ifdef ___CLING___
 public:
  ClassDef(MGUIResponseParameterGaussianByUncertainties, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
