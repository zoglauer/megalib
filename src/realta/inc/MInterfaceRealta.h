/*
 * MInterfaceRealta.h
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


#ifndef __MInterfaceRealta__
#define __MInterfaceRealta__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MInterface.h"
#include "MSettingsRealta.h"
#include "MSettingsMimrec.h"
#include "MSettingsRevan.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MInterfaceRealta : public MInterface
{
  // public interface:
 public:
  MInterfaceRealta(MSettingsRealta *GUIData);
  ~MInterfaceRealta();

  void AnalyzeEvents();



  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsRealta *m_GUIData; // All the information of the GUI

  MSettingsMimrec *m_GUIDataImaging;
  MSettingsRevan *m_GUIDataRevan;


#ifdef ___CLING___
 public:
  ClassDef(MInterfaceRealta, 0) // interface to the Realta-part of MEGAlib
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
