/*
 * MGUINormalizers.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUINormalizers__
#define __MGUINormalizers__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGFrame.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUINormalizers : public MGUIDialog
{
  // Public Interface:
 public:
  MGUINormalizers(const TGWindow* Parent, const TGWindow* Main, 
               MSettingsImaging* Settings);
  virtual ~MGUINormalizers();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  virtual bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsImaging* m_Settings;

  TGCheckButton* m_UseNearFieldNormalizers;
  MGUIEFileSelector* m_DetectionEfficiency;
  MGUIEFileSelector* m_ParameterEfficiency;

  enum ButtonIDs { c_UseNearFieldNormalizers = 140 };


#ifdef ___CLING___
 public:
  ClassDef(MGUINormalizers, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
