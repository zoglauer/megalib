/*
 * MGUIPolarization.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIPolarization__
#define __MGUIPolarization__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMimrec.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIPolarization : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIPolarization(const TGWindow* Parent, const TGWindow* Main, 
                   MSettingsMimrec* Data, bool& OkPressed);
  virtual ~MGUIPolarization();


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
  MSettingsMimrec* m_Data;
  bool& m_OkPressed;

  MGUIEFileSelector* m_PolarizationFile;
  MGUIEFileSelector* m_BackgroundFile; 
  
  MGUIEEntry* m_Theta;
  MGUIEEntry* m_Phi;
  MGUIEEntry* m_Cut;
  MGUIEEntry* m_Bins;


#ifdef ___CINT___
 public:
  ClassDef(MGUIPolarization, 0) // GUI window for unknown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
