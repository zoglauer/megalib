/*
 * MGUIMemory.h
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


#ifndef __MGUIMemory__
#define __MGUIMemory__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIESlider.h"
#include "MGUIERBList.h"
#include "MGUIEEntryList.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIMemory : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIMemory(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data);
  //! default destructor
  virtual ~MGUIMemory();

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

  //! GUI element for the maximum amount of used RAM
  MGUIEEntryList* m_MaxRAM;
  //! GUI element to select the number of bytes in the response
  MGUIERBList* m_Bytes;
  //! GUI element to select the maths
  MGUIERBList* m_Maths;
  //! GUI element to select the fast file parsing
  MGUIERBList* m_Parsing;
  //! GUI element to select the number of threads
  MGUIEEntryList* m_Threads;


#ifdef ___CLING___
 public:
  ClassDef(MGUIMemory, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
