/*
 * MGUIGeometry.h
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


#ifndef __MGUIGeometry__
#define __MGUIGeometry__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsBasicFiles.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Display the default geometry selector dialog box in all MEGAlib GUIs
class MGUIGeometry : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor given a file name
  MGUIGeometry(const TGWindow* Parent, const TGWindow* Main, const MString& FileName);
  //! Standard constructor given a basic settings file containing the latest geometry file name
  MGUIGeometry(const TGWindow* Parent, const TGWindow* Main, MSettingsBasicFiles* Data);
  //! Default destructor
  virtual ~MGUIGeometry();

  //! Return true if OK has been pressed
  bool OkPressed() { return m_OkPressed; }
  //! Return the name of the selected file
  MString GetGeometryFileName() { return m_FileSelector->GetFileName(); }
  //! Called when x is pressed - Overwrite for graceful exit 
  virtual void CloseWindow();

  // protected methods:
 protected:
  //! Create the GUI
  virtual void Create();
  //! Handle a pressed OK button
  virtual bool OnOk();
  //! Handle a pressed Apply button
  virtual bool OnApply();
  //! Handle a pressed Cancel button
  virtual bool OnCancel();


  // protected members:
 protected:


  // private members:
 private:
  //! The file selector
  MGUIEFileSelector* m_FileSelector;

  //! True if, the OK Button was pressed
  bool m_OkPressed;

  //! The geometry file name
  MString m_GeometryFileName;

#ifdef ___CLING___
 public:
  ClassDef(MGUIGeometry, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
