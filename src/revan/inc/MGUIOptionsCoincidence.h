/*
 * MGUIOptionsCoincidence.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsCoincidence__
#define __MGUIOptionsCoincidence__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsCoincidence : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsCoincidence(const TGWindow* Parent, const TGWindow* Main, 
                        MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsCoincidence();


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

  MGUIEEntry* m_Window;


#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsCoincidence, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
