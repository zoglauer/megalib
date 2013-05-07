/*
 * MGUIARM.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIARM__
#define __MGUIARM__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TApplication.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextBuffer.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMimrec.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


enum MARMTYPE
{
  MARMTYPE_GAMMA = 1,
  MARMTYPE_ELECTRON = 2,
  MARMTYPE_SPD = 3,
  MARMTYPE_ENERGY = 4
};


////////////////////////////////////////////////////////////////////////////////


class MGUIARM : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIARM(const TGWindow* Parent, const TGWindow* Main, MSettingsMimrec* Data, MARMTYPE ID, bool& OkPressed);
  virtual ~MGUIARM();

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
  MSettingsMimrec* m_GUIData;
  MARMTYPE m_ID;

  TGLayoutHints* m_SelectorLayout;
  MGUIEEntry* m_ThetaIsX;
  MGUIEEntry* m_PhiIsY;
  MGUIEEntry* m_RadiusIsZ;
  MGUIEEntry* m_Distance;
  MGUIEEntry* m_Bins;

  bool& m_OkPressed;

#ifdef ___CINT___
 public:
  ClassDef(MGUIARM, 0) // Dialogbox: Select the ARM-parameters
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
