/*
 * MGUISivanEventSelection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUISivanEventSelection__
#define __MGUISivanEventSelection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsSivan.h"
#include "MGUIESlider.h"
#include "MGUIERBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUISivanEventSelection : public MGUIDialog
{
  // Public Interface:
 public:
  MGUISivanEventSelection(const TGWindow* Parent, const TGWindow* Main, 
                          MSettingsSivan* Data);
  virtual ~MGUISivanEventSelection();

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
  MSettingsSivan* m_GUIData;



#ifdef ___CLING___
 public:
  ClassDef(MGUISivanEventSelection, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
