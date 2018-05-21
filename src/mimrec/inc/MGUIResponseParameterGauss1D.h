/*
 * MGUIResponseParameterGauss1D.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIResponseParameterGauss1D__
#define __MGUIResponseParameterGauss1D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntryList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIResponseParameterGauss1D : public MGUIDialog
{
  // Public Interface:
 public:
  //! DEfault constructor
  MGUIResponseParameterGauss1D(const TGWindow* Parent, const TGWindow* Main, 
                               MSettingsImaging* Data);
  //! Default destructor
  virtual ~MGUIResponseParameterGauss1D();

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

  //! Gaussian response parameters
  MGUIEEntryList* m_Gaussians;
  //! GUI for the Gaussian cut-off
  MGUIEEntryList* m_CutOff;
  //! Check button for the absorption on/off mode
  TGCheckButton* m_Absorptions;

  static const int c_AbsorptionId = 50;


#ifdef ___CLING___
 public:
  ClassDef(MGUIResponseParameterGauss1D, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
