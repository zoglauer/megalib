/*
 * MGUIOptionsGeneral.h
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


#ifndef __MGUIOptionsGeneral__
#define __MGUIOptionsGeneral__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIECBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsGeneral : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsGeneral(const TGWindow* Parent, const TGWindow* Main, 
                     MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsGeneral();


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
  MSettingsEventReconstruction* m_Data;

  TGLayoutHints* m_MinMaxLayout;
  MGUIEMinMaxEntry* m_TotalEnergy;
  MGUIEMinMaxEntry* m_LeverArm;
  MGUIEMinMaxEntry* m_EventId;

  MGUIECBList* m_Options;

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsGeneral, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
