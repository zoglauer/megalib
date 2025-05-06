/*
 * MGUIReportCreation.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIReportCreation__
#define __MGUIReportCreation__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMelinator.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"
#include "MGUIEMinMaxEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! GUI dialog showing the options for loading ans classifying the calibration file
class MGUIReportCreation : public MGUIDialog
{
  // Public Interface:
 public:
  //! Default constructor
  MGUIReportCreation(const TGWindow* Parent, const TGWindow* Main, MSettingsMelinator* Settings, bool& OKPressed);
  //! Default destructor
  virtual ~MGUIReportCreation();

  
  // protected methods:
 protected:
  //! Create the UI
  virtual void Create();
  //! Apply changes
  virtual bool OnApply();
  //! Process all messages
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);



  // protected members:
 protected:


  // private members:
 private:
  //! The return message that OK was pressed
  bool& m_OkPressed;

  //! The settings file
  MSettingsMelinator* m_Settings;

  //! The report file name
  MGUIEFileSelector* m_FileName;

  //! ADC histogram: display min & max
  MGUIEMinMaxEntry* m_ADCDimension;
  //! ADC histogram: number of bins
  MGUIEEntry* m_ADCBins;

  //! Energy histogram: display min & max
  MGUIEMinMaxEntry* m_EnergyDimension;
  //! Energy histogram: number of bins
  MGUIEEntry* m_EnergyBins;

  //! Threshold histogram: display min & max
  MGUIEMinMaxEntry* m_ThresholdDimension;
  //! Threshold histogram: number of bins
  MGUIEEntry* m_ThresholdBins;

  //! Overflow histogram: display min & max
  MGUIEMinMaxEntry* m_OverflowDimension;
  //! Overflow histogram: number of bins
  MGUIEEntry* m_OverflowBins;


#ifdef ___CLING___
 public:
  ClassDef(MGUIReportCreation, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
