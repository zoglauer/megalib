/*
 * MGUISignificance.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUISignificance__
#define __MGUISignificance__


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
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUISignificance : public MGUIDialog
{
  // Public Interface:
 public:
  MGUISignificance(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data, bool& OkPressed);
  virtual ~MGUISignificance();

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

  TGLayoutHints* m_SelectorLayout;
  MGUIEEntry* m_Radius;
  MGUIEEntry* m_Distance;

  bool& m_OkPressed;

#ifdef ___CLING___
 public:
  ClassDef(MGUISignificance, 0) // Dialogbox: Select the Significance-parameters
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
