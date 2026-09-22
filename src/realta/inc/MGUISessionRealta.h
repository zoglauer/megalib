/*
 * MGUISessionRealta.h
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


#ifndef __MGUISessionRealta__
#define __MGUISessionRealta__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TGMenu.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUISession.h"
#include "MSettingsRealta.h"
#include "MInterfaceRealta.h"
#include "MSystem.h"
#include "MGUIControlCenter.h"

// Forward declarations:




////////////////////////////////////////////////////////////////////////////////


enum RealtaIdentifier 
{
  M_REALTA_NETWORK = 500,
  M_REALTA_STORAGE = 501
};


////////////////////////////////////////////////////////////////////////////////


class MGUISessionRealta : public MGUISession
{
  // public Session:
 public:
  MGUISessionRealta(MGUIMainFrame *Parent);
  ~MGUISessionRealta();

  virtual Bool_t ProcessMessage(Long_t msg, Long_t param1, Long_t);

  virtual void CreateMenuBar();
  virtual Char_t** GetFileTypes();
  virtual void CreateCenterFrame();
  virtual void Launch();
  virtual void SetFileName(MString FileName);

  virtual void Activate();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  MSettingsRealta *m_GUIData;
  MInterfaceRealta *m_Realta;

  MGUIControlCenter *m_ControlCenter;

  // private members:
 private:
  TGPopupMenu *m_MenuAnalysis;

  TGLayoutHints *m_PictureLayout;
  TGPictureButton *m_PictureButton;



#ifdef ___CLING___
 public:
  ClassDef(MGUISessionRealta, 0) // Realta main GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
