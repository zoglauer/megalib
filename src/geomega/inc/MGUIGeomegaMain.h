/*
 * MGUIGeomegaMain.h
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


#ifndef __MGUIGeomegaMain__
#define __MGUIGeomegaMain__


////////////////////////////////////////////////////////////////////////////////


// Standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include <TGMenu.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGFileDialog.h>
#include <TGIcon.h>
#include <TGPicture.h>

// MEGAlib libs
#include "MGlobal.h"
#include "MGUIMain.h"
#include "MSettingsGeomega.h"
#include "MGUIEFlatToolBar.h"
#include "MGUIEStatusBar.h"

class MInterfaceGeomega;


////////////////////////////////////////////////////////////////////////////////


class MGUIGeomegaMain : public MGUIMain
{
  // Public Session:
 public:
  MGUIGeomegaMain(MInterfaceGeomega* Interface, MSettingsGeomega* Data);
  virtual ~MGUIGeomegaMain();

  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // protected members:
 protected:
  virtual void Open();
  virtual void Launch();
  virtual void About();
  void Position();
  void PathLengths();

  virtual bool LoadGeometry(MString FileName = "") { return false; }

  // This adds the geometry section to the file menu --- geomega overwrites this to do nothing
  virtual void AddGeometryToFileMenu() { }

  // private members:
 private:
  MInterfaceGeomega* m_Interface;
  MSettingsGeomega* m_Data;

  // Additional menu items:
  TGPopupMenu* m_MenuAnalysis;

  // - Title
  const TGPicture* m_TitlePicture;
  TGLayoutHints* m_TitleIconLayout;
  TGIcon* m_TitleIcon;

  enum ButtonIDs { c_Scan = c_LastMainButtonID, 
                   c_View, 
                   c_Raytrace, 
                   c_Start,
                   c_Intersect, 
                   c_Masses, 
                   c_Resolutions, 
                   c_Dump, 
                   c_WriteG3, 
                   c_WriteG4, 
                   c_WriteMG, 
                   c_Position,
                   c_PathLengths,
                   c_Test }; 


#ifdef ___CLING___
 public:
  ClassDef(MGUIGeomegaMain, 0) // main window of the Geomega GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
