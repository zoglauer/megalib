/*
 * MGUIOptionsCSR.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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


#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsCSR, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
