/*
 * MGUICoordinateSystem.h
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


#ifndef __MGUICoordinateSystem__
#define __MGUICoordinateSystem__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGButton.h>
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIERBList.h"
#include "MSettingsImaging.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUICoordinateSystem : public MGUIDialog
{
  // Public Interface:
 public:
  MGUICoordinateSystem(const TGWindow* Parent, const TGWindow* Main, 
                       MSettingsImaging* Data);
  virtual ~MGUICoordinateSystem();

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

  MGUIERBList* m_System;


#ifdef ___CLING___
 public:
  ClassDef(MGUICoordinateSystem, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
