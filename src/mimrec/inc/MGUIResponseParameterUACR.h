/*
 * MGUIResponseParameterUACR.h
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIResponseParameterUACR__
#define __MGUIResponseParameterUACR__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIDataMimrec.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIResponseParameterUACR : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIResponseParameterUACR(const TGWindow* Parent, const TGWindow* Main, 
                            MGUIDataMimrec* Data);
  virtual ~MGUIResponseParameterUACR();

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
  MGUIDataMimrec* m_GUIData;

  TGLayoutHints* m_SelectorLayout;

  MGUIEEntry* m_ComptonLong;
  MGUIEEntry* m_ComptonTrans;
  MGUIEEntry* m_Pair;

  TGLayoutHints* m_AbsorptionsLayout;
  TGCheckButton* m_Absorptions;

  TGLayoutHints*m_FileSelectorLayout;
  MGUIEFileSelector* m_FileSelector;

  static const int c_AbsorptionId = 50;


#ifdef ___CINT___
 public:
  ClassDef(MGUIResponseParameterUACR, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
