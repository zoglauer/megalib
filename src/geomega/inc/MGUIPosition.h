/*
 * MGUIPosition.h
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


#ifndef __MGUIPosition__
#define __MGUIPosition__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsGeomega.h"
#include "MGUIEEntry.h"
#include "MGUIEEntryList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIPosition : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIPosition(const TGWindow* Parent, const TGWindow* Main, 
               MSettingsGeomega* Data, bool& OkPressed);
  virtual ~MGUIPosition();


  // protected methods:
 protected:
  void Create();
  bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsGeomega* m_Data;

  bool& m_OkPressed;

  TGLayoutHints* m_PositionLayout;
  MGUIEEntryList* m_Position;
  MGUIEEntry* m_PositionString;


#ifdef ___CLING___
 public:
  ClassDef(MGUIPosition, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
