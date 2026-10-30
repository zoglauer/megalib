/*
 * MGUIOptionsEventClustering.h
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


#ifndef __MGUIOptionsEventClustering__
#define __MGUIOptionsEventClustering__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIEEntryList.h"
#include "MGUIEEntry.h"
#include "MGUIERBList.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsEventClustering : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsEventClustering(const TGWindow* Parent, const TGWindow* Main, 
                        MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsEventClustering();


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

  MGUIEEntry* m_DistanceCutOff;
  
  MGUIEFileSelector* m_TMVAFileSelector;
  MGUIERBList* m_TMVAMethods;
  //! Map button IDs to TMVA methods
  map<int, MERCSRTMVAMethod> m_TMVAMethodsMap;
  
  
  enum ButtonIDs { e_ReferencePoint = 200};


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsEventClustering, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
