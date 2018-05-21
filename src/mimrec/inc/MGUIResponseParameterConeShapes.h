/*
 * MGUIResponseParameterConeShapes.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIResponseParameterConeShapes__
#define __MGUIResponseParameterConeShapes__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIResponseParameterConeShapes : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIResponseParameterConeShapes(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Settings);
  virtual ~MGUIResponseParameterConeShapes();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();


  // protected members:
 protected:


  // private members:
 private:
  //! The UI data
  MSettingsImaging* m_Settings;

  //! A file selector for the ARM response
  MGUIEFileSelector* m_ARMFile;



#ifdef ___CLING___
 public:
  ClassDef(MGUIResponseParameterConeShapes, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
