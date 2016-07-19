/*
 * MGUIOptionsGeneral.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsGeneral__
#define __MGUIOptionsGeneral__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIECBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsGeneral : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsGeneral(const TGWindow* Parent, const TGWindow* Main, 
                     MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsGeneral();


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

  TGLayoutHints* m_MinMaxLayout;
  MGUIEMinMaxEntry* m_TotalEnergy;
  MGUIEMinMaxEntry* m_LeverArm;
  MGUIEMinMaxEntry* m_EventId;

  MGUIECBList* m_Options;

#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsGeneral, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
