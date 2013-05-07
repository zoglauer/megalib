/*
 * MGUIAccumulation.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIAccumulation__
#define __MGUIAccumulation__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIEFileSelector.h"
#include "MSettingsRealta.h"

// Forward declarations:
class MGUIEEntry;

////////////////////////////////////////////////////////////////////////////////


class MGUIAccumulation : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIAccumulation(const TGWindow* p, const TGWindow* main, MSettingsRealta* Settings);
  virtual ~MGUIAccumulation();


  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();
  virtual bool ProcessMessage(Long_t Message, Long_t Parameter1, Long_t Parameter2);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsRealta* m_Settings;

  //! The accumulation time
  MGUIEEntry* m_AccumulationTime;
  
  //! The number bins in the count rate histogram
  MGUIEEntry* m_BinsCountRate;
  //! The number bins in the spectrum
  MGUIEEntry* m_BinsSpectrum;  
  
  //! Choose a file to save the data to:
  MGUIEFileSelector* m_AccumulationFileName;
  //! True if we want to add the date and time to the filename
  TGCheckButton* m_AddDateAndTime;

#ifdef ___CINT___
 public:
  ClassDef(MGUIAccumulation, 0) // GUI window: ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
