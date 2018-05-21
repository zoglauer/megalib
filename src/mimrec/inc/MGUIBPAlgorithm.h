/*
 * MDummy.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIBPAlgorithm__
#define __MGUIBPAlgorithm__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGButton.h>
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIERBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIBPAlgorithm : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIBPAlgorithm(const TGWindow* Parent, const TGWindow* Main, 
                  MSettingsImaging* Data);
  virtual ~MGUIBPAlgorithm();

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
  MSettingsImaging* m_Data;

  MGUIERBList* m_BPList;


#ifdef ___CLING___
 public:
  ClassDef(MGUIBPAlgorithm, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
