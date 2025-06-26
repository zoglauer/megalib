/*
 * MGUIReportCreation.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MGUIReportCreation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIReportCreation.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIReportCreation)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIReportCreation::MGUIReportCreation(const TGWindow* Parent, const TGWindow* Main, MSettingsMelinator* Settings, bool& OkPressed)
  : MGUIDialog(Parent, Main), m_OkPressed(OkPressed)
{
  // Construct an instance of MGUIReportCreation and bring it to the screen

  m_Settings = Settings;
  m_OkPressed = false;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIReportCreation::~MGUIReportCreation()
{
  // Delete an instance of MGUIReportCreation
}


////////////////////////////////////////////////////////////////////////////////


void MGUIReportCreation::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Calibration report");

  AddSubTitle("Choose the options for the calibration report:");

  TGLayoutHints* FirstLayout = new TGLayoutHints(kLHintsCenterX | kLHintsTop | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 0*m_FontScaler, 20*m_FontScaler);
  TGLayoutHints* DimensionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 0*m_FontScaler, 0*m_FontScaler);
  TGLayoutHints* BinLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40*m_FontScaler, 20*m_FontScaler, 0*m_FontScaler, 20*m_FontScaler);
  


  m_ADCDimension = new MGUIEMinMaxEntry(this,
                                        MString("ADC histogram:"),
                                        false,
                                        MString("Minimum [ADC units]: "),
                                        MString("Maximum [ADC units]: "),
                                        m_Settings->GetReportADCHistogramMinimum(),
                                        m_Settings->GetReportADCHistogramMaximum(),
                                        true,
                                        0.0);
  AddFrame(m_ADCDimension, DimensionLayout);

  m_ADCBins = new MGUIEEntry(this, "Number of Bins:", false, m_Settings->GetReportADCHistogramBins(), true, 1);
  AddFrame(m_ADCBins, BinLayout);


  m_EnergyDimension = new MGUIEMinMaxEntry(this,
                                        MString("Energy histogram:"),
                                        false,
                                        MString("Minimum [keV]: "),
                                        MString("Maximum [keV]: "),
                                        m_Settings->GetReportEnergyHistogramMinimum(),
                                        m_Settings->GetReportEnergyHistogramMaximum(),
                                        true,
                                        0.0);
  AddFrame(m_EnergyDimension, DimensionLayout);

  m_EnergyBins = new MGUIEEntry(this, "Number of Bins:", false, m_Settings->GetReportEnergyHistogramBins(), true, 1);
  AddFrame(m_EnergyBins, BinLayout);



  m_ThresholdDimension = new MGUIEMinMaxEntry(this,
                                        MString("Threshold histogram:"),
                                        false,
                                        MString("Minimum [keV]: "),
                                        MString("Maximum [keV]: "),
                                        m_Settings->GetReportThresholdHistogramMinimum(),
                                        m_Settings->GetReportThresholdHistogramMaximum(),
                                        true,
                                        0.0);
  AddFrame(m_ThresholdDimension, DimensionLayout);

  m_ThresholdBins = new MGUIEEntry(this, "Number of Bins:", false, m_Settings->GetReportThresholdHistogramBins(), true, 1);
  AddFrame(m_ThresholdBins, BinLayout);



  m_OverflowDimension = new MGUIEMinMaxEntry(this,
                                        MString("Overflow histogram:"),
                                        false,
                                        MString("Minimum [keV]: "),
                                        MString("Maximum [keV]: "),
                                        m_Settings->GetReportOverflowHistogramMinimum(),
                                        m_Settings->GetReportOverflowHistogramMaximum(),
                                        true,
                                        0.0);
  AddFrame(m_OverflowDimension, DimensionLayout);

  m_OverflowBins = new MGUIEEntry(this, "Number of Bins:", false, m_Settings->GetReportOverflowHistogramBins(), true, 1);
  AddFrame(m_OverflowBins, BinLayout);



  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), true);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);
 
  return;
}

////////////////////////////////////////////////////////////////////////////////


bool MGUIReportCreation::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    default:
      break;
    }
  default:
    break;
  }

  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


//! Action after the Apply or OK button has been pressed.
bool MGUIReportCreation::OnApply()
{
  if (m_ADCDimension->CheckRange(0.0, 100000.0, 0.0, 100000.0, true) == false) return false;
  if (m_ADCBins->IsInt(1, 100000) == false) return false;

  if (m_EnergyDimension->CheckRange(0.0, 100000.0, 0.0, 100000.0, true) == false) return false;
  if (m_EnergyBins->IsInt(1, 100000) == false) return false;

  if (m_ThresholdDimension->CheckRange(0.0, 100000.0, 0.0, 100000.0, true) == false) return false;
  if (m_ThresholdBins->IsInt(1, 100000) == false) return false;

  if (m_OverflowDimension->CheckRange(0.0, 100000.0, 0.0, 100000.0, true) == false) return false;
  if (m_OverflowBins->IsInt(1, 100000) == false) return false;

  m_Settings->SetReportADCHistogramMinimum(m_ADCDimension->GetMinValue());
  m_Settings->SetReportADCHistogramMaximum(m_ADCDimension->GetMaxValue());
  m_Settings->SetReportADCHistogramBins(m_ADCBins->GetAsInt());

  m_Settings->SetReportEnergyHistogramMinimum(m_EnergyDimension->GetMinValue());
  m_Settings->SetReportEnergyHistogramMaximum(m_EnergyDimension->GetMaxValue());
  m_Settings->SetReportEnergyHistogramBins(m_EnergyBins->GetAsInt());

  m_Settings->SetReportThresholdHistogramMinimum(m_ThresholdDimension->GetMinValue());
  m_Settings->SetReportThresholdHistogramMaximum(m_ThresholdDimension->GetMaxValue());
  m_Settings->SetReportThresholdHistogramBins(m_ThresholdBins->GetAsInt());

  m_Settings->SetReportOverflowHistogramMinimum(m_OverflowDimension->GetMinValue());
  m_Settings->SetReportOverflowHistogramMaximum(m_OverflowDimension->GetMaxValue());
  m_Settings->SetReportOverflowHistogramBins(m_OverflowBins->GetAsInt());

  m_OkPressed = true;

  return true;
}


// MGUIReportCreation: the end...
////////////////////////////////////////////////////////////////////////////////
