/*
 * MGUIERAlgorithm.h
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


#ifndef __MGUIERAlgorithm__
#define __MGUIERAlgorithm__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIERBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIERAlgorithm : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIERAlgorithm(const TGWindow* Parent, const TGWindow* Main, 
                  MSettingsEventReconstruction* Data);
  virtual ~MGUIERAlgorithm();

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

  TGLayoutHints* m_ListLayout;
  MGUIERBList* m_CoincidenceList;
  MGUIERBList* m_EventClusteringList;
  MGUIERBList* m_HitClusteringList;
  MGUIERBList* m_TrackingList;
  MGUIERBList* m_CSRList;
  //MGUIERBList* m_DecayList;
 


#ifdef ___CLING___
 public:
  ClassDef(MGUIERAlgorithm, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
