/*
 * MGUIOptionsDecay.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsDecay__
#define __MGUIOptionsDecay__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIEEntryList.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsDecay : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsDecay(const TGWindow* Parent, const TGWindow* Main, 
                   MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsDecay();


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

  TGLayoutHints* m_FileSelectorLayout;
  MGUIEFileSelector* m_FileSelector;

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsDecay, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
