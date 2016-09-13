/*
 * MGUIExposure.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExposure__
#define __MGUIExposure__


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


class MGUIExposure : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIExposure(const TGWindow* Parent, const TGWindow* Main, 
                  MSettingsImaging* Data);
  virtual ~MGUIExposure();

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
  ClassDef(MGUIExposure, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
