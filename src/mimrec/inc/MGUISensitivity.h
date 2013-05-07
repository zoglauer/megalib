/*
 * MGUISensitivity.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUISensitivity__
#define __MGUISensitivity__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUISensitivity : public MGUIDialog
{
  // Public Interface:
 public:
  MGUISensitivity(const TGWindow* Parent, const TGWindow* Main, 
                  MSettingsImaging* Data);
  virtual ~MGUISensitivity();

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
  MSettingsImaging* m_GUIData;

  TGLayoutHints* m_FileSelectorLayout;
  MGUIEFileSelector* m_FileSelector;


#ifdef ___CINT___
 public:
  ClassDef(MGUISensitivity, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
