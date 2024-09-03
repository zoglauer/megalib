/*
 * MGUIARMExtended.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIARMExtended__
#define __MGUIARMExtended__


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

//! The position selection dialog box called by all interfaces which allow
//! this selection
//! It has 3 modes: ARM Gamma, ARM Electron, Spectrum
//! This window will wait for being unmapped and not be deleted on close
class MGUIARMExtended : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIARMExtended(const TGWindow* Parent, const TGWindow* Main, MSettingsMimrec* Data, bool& OkPressed);
  //! Default destructor
  virtual ~MGUIARMExtended();

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
  //! The user settings data
  MSettingsMimrec* m_Settings;


  //! The return message that OK was pressed
  bool& m_OkPressed;

  //! Check button indicating if we do want to use a test position in spectral mode
  TGCheckButton* m_UseTestPosition;
  const int m_UseTestPositionID = 140;

  //! Check button indicating if we do want to use a logarithmic binning in spectral mode
  TGCheckButton* m_UseLog;
  const int m_UseLogID = 141;
  
  MGUIEEntry* m_ThetaIsX;
  MGUIEEntry* m_PhiIsY;
  MGUIEEntry* m_RadiusIsZ;
  MGUIEEntry* m_Distance;
  MGUIEEntry* m_Bins;

  
#ifdef ___CLING___
 public:
  ClassDef(MGUIARMExtended, 0) // Dialogbox: Select the ARM-parameters
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
