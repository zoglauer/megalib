/*
 * MGUIResponseParameterGaussianByUncertainties.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIResponseParameterGaussianByUncertainties__
#define __MGUIResponseParameterGaussianByUncertainties__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIResponseParameterGaussianByUncertainties : public MGUIDialog
{
  // Public Interface:
 public:
  //! DEfault constructor
  MGUIResponseParameterGaussianByUncertainties(const TGWindow* Parent, const TGWindow* Main, 
                                               MSettingsImaging* Data);
  //! Default destructor
  virtual ~MGUIResponseParameterGaussianByUncertainties();

  // protected methods:
 protected:
  //! Create the GUI
  virtual void Create();
  //! Handle Apply/OK button
  virtual bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! All the GUI data
  MSettingsImaging* m_GUIData;

  //! Increase the 1-sigma Gaussian width with this amount of degrees
  MGUIEEntry* m_Increase;


#ifdef ___CINT___
 public:
  ClassDef(MGUIResponseParameterGaussianByUncertainties, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
