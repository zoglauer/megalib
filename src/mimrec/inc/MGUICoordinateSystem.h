/*
 * MGUICoordinateSystem.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUICoordinateSystem__
#define __MGUICoordinateSystem__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGButton.h>
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIERBList.h"
#include "MSettingsImaging.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUICoordinateSystem : public MGUIDialog
{
  // Public Interface:
 public:
  MGUICoordinateSystem(const TGWindow* Parent, const TGWindow* Main, 
                       MSettingsImaging* Data);
  virtual ~MGUICoordinateSystem();

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

  MGUIERBList* m_System;


#ifdef ___CINT___
 public:
  ClassDef(MGUICoordinateSystem, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
