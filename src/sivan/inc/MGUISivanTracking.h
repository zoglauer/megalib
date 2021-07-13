/*
 * MGUISivanTracking.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUISivanTracking__
#define __MGUISivanTracking__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT libs
#include <TGFrame.h>
#include <TGLabel.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsSivan.h"


////////////////////////////////////////////////////////////////////////////////


class MGUISivanTracking : public MGUIDialog
{
  // Public Interface:
 public:
  MGUISivanTracking(const TGWindow* Parent, const TGWindow* Main, MSettingsSivan* Data);
  virtual ~MGUISivanTracking();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long msg, long param1, long);
  virtual bool OnApply();

  // private members:
 private:
  MSettingsSivan* m_GUIData;
  int m_Realism;

  // 
  TGRadioButton* m_RealRB[3];
  TGLayoutHints* m_RBLayout;


#ifdef ___CLING___
 public:
  ClassDef(MGUISivanTracking, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
