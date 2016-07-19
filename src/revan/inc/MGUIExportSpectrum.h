/*
 * MGUIExportSpectrum.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExportSpectrum__
#define __MGUIExportSpectrum__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGButton.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsRevan.h"
#include "MGUIEEntry.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExportSpectrum : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIExportSpectrum(const TGWindow* Parent, const TGWindow* Main, 
                     MSettingsRevan* Data, bool& OKPressed);
  virtual ~MGUIExportSpectrum();


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
  //! The GUI data
  MSettingsRevan* m_Data;

  //! The number of bins
  MGUIEEntry* m_Bins;
  //! Logarithmic display
  TGCheckButton* m_Log;
  // The minimum and maximum energy value
  MGUIEMinMaxEntry* m_Range;
  //! The file selection dialog
  MGUIEFileSelector* m_File;

  //! True if OK button has been pressed
  bool& m_OKPressed;


#ifdef ___CINT___
 public:
  ClassDef(MGUIExportSpectrum, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
