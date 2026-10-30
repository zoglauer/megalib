/*
 * MGUIOptionsTracking.h
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


#ifndef __MGUIOptionsTracking__
#define __MGUIOptionsTracking__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGListBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MDGeometryQuest.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIECBList.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsTracking : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsTracking(const TGWindow* Parent, const TGWindow* Main, 
                      MSettingsEventReconstruction* Data,
                      MDGeometryQuest* Geometry);
  virtual ~MGUIOptionsTracking();


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
  MDGeometryQuest* m_Geometry;
  
  MGUIECBList* m_CBList;
  MGUIEEntry* m_MaxComptonJump;
  MGUIEEntry* m_NSequencesToKeep;
  TGCheckButton* m_RejectPurelyAmbiguousSequences;
  MGUIEFileSelector* m_FileSelector;
  MGUIEEntry* m_NLayersForVertexSearch;

  TGListBox* m_DetectorList;
  
  enum ButtonCodes { e_DoTracking = 100, e_RejectPurelyAmbiguousSequences, e_Detectors };

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsTracking, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
