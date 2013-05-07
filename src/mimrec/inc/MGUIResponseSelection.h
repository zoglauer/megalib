/*
 * MGUIResponseSelection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIResponseSelection__
#define __MGUIResponseSelection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIERBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIResponseSelection : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIResponseSelection(const TGWindow* Parent, const TGWindow* Main, 
                        MSettingsImaging* Data = 0);
  virtual ~MGUIResponseSelection();

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

  TGLayoutHints* m_ResponseChoiceLayout;
  MGUIERBList* m_ResponseChoice;


#ifdef ___CINT___
 public:
  ClassDef(MGUIResponseSelection, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
