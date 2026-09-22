/*
 * MGUIResponseParameterPRM.h
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


#ifndef __MGUIResponseParameterPRM__
#define __MGUIResponseParameterPRM__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIResponseParameterPRM : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIResponseParameterPRM(const TGWindow* Parent, const TGWindow* Main, 
                           MSettingsImaging* Data);
  virtual ~MGUIResponseParameterPRM();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();


  // protected members:
 protected:


  // private members:
 private:
  TGWindow* m_Parent;

  MSettingsImaging* m_GUIData;

  MGUIEFileSelector* m_ComptonLongitudinalFile;
  MGUIEFileSelector* m_ComptonTransversalFile;



#ifdef ___CLING___
 public:
  ClassDef(MGUIResponseParameterPRM, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
