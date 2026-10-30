/*
 * MGUIPathLength.h
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


#ifndef __MGUIPathLength__
#define __MGUIPathLength__


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


//! The GUI dialog shown the start and end position for by hand absorption calculations
class MGUIPathLength : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor - creates the GUI and waits until OK has been pressed
  MGUIPathLength(const TGWindow* Parent, const TGWindow* Main, 
                MSettingsGeomega* Data, bool& OkPressed);
  //! Default destructor
  virtual ~MGUIPathLength();


  // protected methods:
 protected:
  //! Create the display
  void Create();
  //! Action when OK has been pressed
  bool OnApply();

  //! Parse the string
  bool Parse(const MString& String, MVector& Pos);
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  // The return value
  bool& m_OkPressed;

  //! The settings 
  MSettingsGeomega* m_Settings;

  //! The start position in string format
  MGUIEEntry* m_StartString;
  //! The stop position in string format
  MGUIEEntry* m_StopString;


#ifdef ___CLING___
 public:
  ClassDef(MGUIPathLength, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
