/*
 * MGUIReconstructedSpectrum.cxx
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
// MGUIReconstructedSpectrum
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIReconstructedSpectrum.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIReconstructedSpectrum)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIReconstructedSpectrum::MGUIReconstructedSpectrum(const TGWindow* Parent, 
                                                     const TGWindow* Main, 
                                                     MSettingsRevan* Data, 
                                                     bool& OKPressed)
  : MGUIDialog(Parent, Main), m_OKPressed(OKPressed)
{
  // Construct an instance of MGUIExportSpectrum and bring it to the screen

  m_Data = Data;
  m_OKPressed = false;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}



////////////////////////////////////////////////////////////////////////////////


MGUIReconstructedSpectrum::~MGUIReconstructedSpectrum()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIReconstructedSpectrum::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Generate spectra");  
  AddSubTitle("Options for generating and displaying various spectra"); 

  int Left = 20;
  int Right = 20;
  int TopLarge = 20;
  int TopSmall = 10;
  int Bottom = 0;
  
  TGLayoutHints* StandardLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, Left, Right, TopLarge, Bottom);

  m_BeforeAfter = new MGUIECBList(this, "Select if the spectrum should be done before and/or after event reconstruction:");
  m_BeforeAfter->Add("Before", m_Data->GetSpectrumBefore());          
  m_BeforeAfter->Add("After", m_Data->GetSpectrumAfter());          
  m_BeforeAfter->Create();
  m_BeforeAfter->Associate(this);
  AddFrame(m_BeforeAfter, StandardLayout);

  m_Detectors = new MGUIECBList(this, "Select how the hits should be histogrammed. Attention: If you choose to look at the spectrum *after* event reconstruction then you have events spanning multiple detectors or detector types. Those will be sorted into the histogram where the start of the event is determined!");
  m_Detectors->Add("Into one spectrum for the whole setup", m_Data->GetSpectrumSortByInstrument());          
  m_Detectors->Add("Into one spectrum per detector type (e.g. combine all strip detectors)", m_Data->GetSpectrumSortByDetectorType());          
  m_Detectors->Add("Into one spectrum per named detector (if two detectors have the same name they are combined)", m_Data->GetSpectrumSortByNamedDetector());          
  m_Detectors->Add("Into one spectrum per detector (one spectrum for each positioned detector)", m_Data->GetSpectrumSortByDetector());          
  m_Detectors->Create();
  m_Detectors->Associate(this);
  AddFrame(m_Detectors, StandardLayout);

  m_Combinations = new MGUIECBList(this, "Select if the energy of the hits of the events should be combined or added individually to the histogram. Attention: If you select not to combine the hits *after* event reconstruction, then you add the individual high-level hits such as combined clusters, tracks, etc. but not the individual hits.");
  m_Combinations->Add("Combine hit energies", m_Data->GetSpectrumCombine());
  m_Combinations->Create();
  m_Combinations->Associate(this);
  AddFrame(m_Combinations, StandardLayout);
  
  m_Output = new MGUIECBList(this, "Select where the output should go:");
  m_Output->Add("To screen", m_Data->GetSpectrumOutputToScreen());          
  m_Output->Add("To file (will be written in the same directory as the input file)", m_Data->GetSpectrumOutputToFile());          
  m_Output->Create();
  m_Output->Associate(this);
  AddFrame(m_Output, StandardLayout);

  
  TGLabel* BinningLabel = new TGLabel(this, "Choose the binning and energy range:");
  AddFrame(BinningLabel, StandardLayout);

  TGHorizontalFrame* Frame = new TGHorizontalFrame(this);
  TGLayoutHints* TightLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 2*Left, 2*Right, TopSmall, 0);
  AddFrame(Frame, TightLayout);

  TGLayoutHints* InnerLeftLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 0, 0);
  TGLayoutHints* InnerRightLayout = new TGLayoutHints(kLHintsTop, 32, 0, 0, 0);  
  
  m_Bins = new MGUIEEntry(Frame, "Bins: ", false, m_Data->GetSpectrumBins(), true, 1);
  Frame->AddFrame(m_Bins, InnerLeftLayout);
  
  m_Log = new TGCheckButton(Frame, "Logarithmic");
  m_Log->SetState((m_Data->GetSpectrumLog() == true) ?  kButtonDown : kButtonUp);
  Frame->AddFrame(m_Log, InnerRightLayout);

  m_Range = new MGUIEMinMaxEntry(this, "Range [keV]: ", false, m_Data->GetSpectrumMin(), m_Data->GetSpectrumMax(), true, 0.0);
  AddFrame(m_Range, TightLayout);

  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.  m_Data->SetSpectrumBefore(m_BeforeAfter->IsSelected(0));

  MapSubwindows();
  MapWindow();  
  Layout();

  fClient->WaitFor(this);
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIReconstructedSpectrum::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this window
  
  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}

////////////////////////////////////////////////////////////////////////////////


bool MGUIReconstructedSpectrum::OnApply()
{
  // The Apply button has been pressed

  if (m_BeforeAfter->IsSelected(0) == false && m_BeforeAfter->IsSelected(1) == false) {
    mgui<<"You have choose at least either the data before or after reconstrcution!"<<error;
    return false;    
  }
  
  if (m_Detectors->IsSelected(0) == false && m_Detectors->IsSelected(1) == false && m_Detectors->IsSelected(2) == false && m_Detectors->IsSelected(3) == false) {
    mgui<<"You have choose at least one sorting option!"<<error;
    return false;    
  }
  
  if (m_Output->IsSelected(0) == false && m_Output->IsSelected(1) == false) {
    mgui<<"You have choose at least one output option!"<<error;
    return false;    
  }
  
  if (m_Log->GetState() == kButtonDown && m_Range->GetMinValue() == 0) {
    mgui<<"You have to use a minimum range larger than 0 for logarithmic binning!"<<error;
    return false;
  }
  
  m_Data->SetSpectrumBefore(m_BeforeAfter->IsSelected(0));
  m_Data->SetSpectrumAfter(m_BeforeAfter->IsSelected(1));
  
  m_Data->SetSpectrumSortByInstrument(m_Detectors->IsSelected(0));
  m_Data->SetSpectrumSortByDetectorType(m_Detectors->IsSelected(1));
  m_Data->SetSpectrumSortByNamedDetector(m_Detectors->IsSelected(2));
  m_Data->SetSpectrumSortByDetector(m_Detectors->IsSelected(3));
  
  m_Data->SetSpectrumCombine(m_Combinations->IsSelected(0));

  m_Data->SetSpectrumOutputToScreen(m_Output->IsSelected(0));
  m_Data->SetSpectrumOutputToFile(m_Output->IsSelected(1));

  m_Data->SetSpectrumBins(m_Bins->GetAsInt());
  m_Data->SetSpectrumLog((m_Log->GetState() == kButtonDown) ? true : false);
  m_Data->SetSpectrumMin(m_Range->GetMinValue());
  m_Data->SetSpectrumMax(m_Range->GetMaxValue());

  m_OKPressed = true;

  return true;
}


// MGUIReconstructedSpectrum: the end...
////////////////////////////////////////////////////////////////////////////////
