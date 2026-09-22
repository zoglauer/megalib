/*
 * MGUISignificance.h
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


#ifndef __MGUISignificance__
#define __MGUISignificance__


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
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUISignificance : public MGUIDialog
{
  // Public Interface:
 public:
  MGUISignificance(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data, bool& OkPressed);
  virtual ~MGUISignificance();

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
  MSettingsImaging* m_GUIData;

  TGLayoutHints* m_SelectorLayout;
  MGUIEEntry* m_Radius;
  MGUIEEntry* m_Distance;

  bool& m_OkPressed;

#ifdef ___CLING___
 public:
  ClassDef(MGUISignificance, 0) // Dialogbox: Select the Significance-parameters
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
