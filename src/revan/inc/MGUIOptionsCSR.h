/*
 * MGUIOptionsCSR.h
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


#ifndef __MGUIOptionsCSR__
#define __MGUIOptionsCSR__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MInterface.h"
#include "MGUIECBList.h"
#include "MGUIERBList.h"
#include "MGUIEEntry.h"
#include "MGUIEEntryList.h"
#include "MGUIEMinMaxEntry.h"

// Forward declarations:
class MGUIEFileSelector;


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsCSR : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsCSR(const TGWindow* Parent, const TGWindow* Main, 
                 MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsCSR();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();

  virtual bool ProcessMessage(long msg, long param1, long param2);
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsEventReconstruction* m_Data;

  MGUIECBList* m_Options;
  MGUIERBList* m_OptionsUndecided;
  MGUIEMinMaxEntry* m_Threshold;
  MGUIEEntry* m_MaxNSingleHits;

  TGLayoutHints* m_BayesianFileSelectorLayout;
  MGUIEFileSelector* m_BayesianFileSelector;

  MGUIEFileSelector* m_TMVAFileSelector;
  MGUIERBList* m_TMVAMethods;
  //! Map button IDs to TMVA methods
  map<int, MERCSRTMVAMethod> m_TMVAMethodsMap;
  
#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsCSR, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
