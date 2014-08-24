/*
 * MGUIAccumulation.cxx
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
// MGUIAccumulation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIAccumulation.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
  TGCheckButton *m_ConnectOnStart;

#include "MGUIEEntry.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIAccumulation)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIAccumulation::MGUIAccumulation(const TGWindow* Parent, const TGWindow* Main, MSettingsRealta* Settings)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIAccumulation and bring it to the screen

  m_Settings = Settings;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIAccumulation::~MGUIAccumulation()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAccumulation::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Accumulation");  

  AddSubTitle("Set all event accumulation, etc. options"); 

  TGLayoutHints* ThreadsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 5, 20);
  m_Threads = new MGUIECBList(this, "Choose if you want to run the following threads:");
  m_Threads->Add("Do coincidence search", m_Settings->GetDoCoincidence() ? 1 : 0);
  m_Threads->Add("Do isotope identification", m_Settings->GetDoIdentification() ? 1 : 0);
  m_Threads->Create();
  AddFrame(m_Threads, ThreadsLayout);
  
  TGLayoutHints* AccumulationTimeLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 5, 20);
  m_AccumulationTime  = new MGUIEEntry(this, "Accumulation time [sec]:", false, m_Settings->GetAccumulationTime());
  AddFrame(m_AccumulationTime, AccumulationTimeLayout);

  TGLayoutHints* BinsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 5, 5);
  m_BinsCountRate = new MGUIEEntry(this, "Number of bins in count rate histogram:", false, m_Settings->GetBinsCountRate());
  AddFrame(m_BinsCountRate, BinsLayout);

  m_BinsSpectrum = new MGUIEEntry(this, "Number of bins in spectrum:", false, m_Settings->GetBinsSpectrum());
  AddFrame(m_BinsSpectrum, BinsLayout);

  m_RangeSpectrum = new MGUIEMinMaxEntry(this,
                                          MString("Energy window in main spectrum::"), 
                                          false,
                                          "Minimum energy [keV]: ",
                                          "Maximum energy [keV]: ",
                                          m_Settings->GetMinimumSpectrum(), 
                                          m_Settings->GetMaximumSpectrum(),
                                          true, 0.0);
  //m_EnergySpectrum->SetEntryFieldSize(FieldSize);
  AddFrame(m_RangeSpectrum, BinsLayout);


  TGLayoutHints* AccumulationFileNameLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 20, 5);
  m_AccumulationFileName = new MGUIEFileSelector(this, "Save the events after they are received (don't save if empty)", 
                                     m_Settings->GetAccumulationFileName());
  m_AccumulationFileName->SetFileType("Measured event file", "*.evta");
  m_AccumulationFileName->SetFileType("Simulated event file", "*.sim");
  AddFrame(m_AccumulationFileName, AccumulationFileNameLayout);

  TGLayoutHints* DateAndTimeLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 2);
  m_AddDateAndTime = new TGCheckButton(this, "Add date and time to the file name to prevent overwrites");
  m_AddDateAndTime->Associate(this);
  if (m_Settings->GetAccumulationFileNameAddDateAndTime() == true) {
    m_AddDateAndTime->SetState(kButtonDown);
  } else {
    m_AddDateAndTime->SetState(kButtonUp);    
  }  
  AddFrame(m_AddDateAndTime, DateAndTimeLayout);

  
  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth()+30, GetDefaultHeight()+20);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIAccumulation::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application
  
  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIAccumulation::OnApply()
{
  // Set all data 
 
  m_Settings->SetAccumulationTime(m_AccumulationTime->GetAsDouble());
  m_Settings->SetAccumulationFileName(m_AccumulationFileName->GetFileName());
  m_Settings->SetBinsCountRate(m_BinsCountRate->GetAsInt());
  m_Settings->SetBinsSpectrum(m_BinsSpectrum->GetAsInt());
  m_Settings->SetMinimumSpectrum(m_RangeSpectrum->GetMinValue());
  m_Settings->SetMaximumSpectrum(m_RangeSpectrum->GetMaxValue());
  
  if (m_AddDateAndTime->GetState() == kButtonDown) {
    m_Settings->SetAccumulationFileNameAddDateAndTime(true);
  } else {
    m_Settings->SetAccumulationFileNameAddDateAndTime(false);    
  }
  
  m_Settings->SetDoCoincidence(m_Threads->IsSelected(0));
  m_Settings->SetDoIdentification(m_Threads->IsSelected(1));

  UnmapWindow();
  
  return true;
}


// MGUIAccumulation: the end...
////////////////////////////////////////////////////////////////////////////////
