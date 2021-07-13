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
#include "MResponseType.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Default UI to select the response type
class MGUIResponseSelection : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIResponseSelection(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data = 0);
  //! Default destructor
  virtual ~MGUIResponseSelection();

  // protected methods:
 protected:
  //! Create the UI
  virtual void Create();
  //! Apply the set data 
  virtual bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The settings store
  MSettingsImaging* m_Settings;
  //! A radiobutton list of the different response choices
  MGUIERBList* m_ResponseChoice;


#ifdef ___CLING___
 public:
  ClassDef(MGUIResponseSelection, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
