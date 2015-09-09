/*
 * MGUIReconstructedSpectrum.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIReconstructedSpectrum__
#define __MGUIReconstructedSpectrum__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsRevan.h"
#include "MGUIECBList.h"
#include "MGUIERBList.h"
#include "MGUIEEntry.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEFileSelector.h"


////////////////////////////////////////////////////////////////////////////////


//! A UI to select the options for the reconstructed (or not) spectrum
class MGUIReconstructedSpectrum : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIReconstructedSpectrum(const TGWindow* Parent, const TGWindow* Main, 
                            MSettingsRevan* Data, bool& OKPressed);
  //! Default destructor
  virtual ~MGUIReconstructedSpectrum();

  // protected methods:
 protected:
  //! Create the UI
  virtual void Create();
  //! This happens when the OK/Apply button is pressed
  virtual bool OnApply();

  //! Process the UI messages
  virtual bool ProcessMessage(long msg, long param1, long param2);
  
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The settings data
  MSettingsRevan* m_Data;
  //! The checkbutton list for the before/after spectrum
  MGUIECBList* m_BeforeAfter;
  //! The checkbutton list of the included detectors
  MGUIECBList* m_Detectors;
  //! A radiobutton list for determined if the hits should be combined or not
  MGUIECBList* m_Combinations;
  //! The checkbutton list of the output location
  MGUIECBList* m_Output;
  
  //! The number of bins
  MGUIEEntry* m_Bins;
  //! Logarithmic display
  TGCheckButton* m_Log;
  //! The minimum and maximum energy value
  MGUIEMinMaxEntry* m_Range;


  //! True if OK button has been pressed
  bool& m_OKPressed;  


#ifdef ___CINT___
 public:
  ClassDef(MGUIReconstructedSpectrum, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
