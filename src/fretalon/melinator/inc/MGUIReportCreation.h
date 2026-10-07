/*
 * MGUIReportCreation.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
