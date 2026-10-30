/*
 * MGUIAnimation.h
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


#ifndef __MGUIAnimation__
#define __MGUIAnimation__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGFrame.h"
#include "TGNumberEntry.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIAnimation : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIAnimation(const TGWindow* Parent, const TGWindow* Main, 
                MSettingsImaging* Data, bool& OkPressed);
  virtual ~MGUIAnimation();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  virtual bool OnApply();

  void EnableRadioButton(long Parameter1);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsImaging* m_GUIData;

  TGRadioButton* m_Iterations;
  TGRadioButton* m_Backprojections;
  TGNumberEntry* m_SnapshotTime;
  MGUIEFileSelector* m_FileSelector;

  bool& m_OkPressed;
  
  enum ButtenIds { c_Iterations = 140,
                   c_Backprojections };


#ifdef ___CLING___
 public:
  ClassDef(MGUIAnimation, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
