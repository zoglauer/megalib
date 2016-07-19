/*
 * MGUISpectralAnalyzer.cxx
 *
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Michelle Galloway & Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MGUISpectralAnalyzer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUISpectralAnalyzer.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TGMsgBox.h"
#include "TGTab.h"

// MEGAlib libs:
#include "MGUIEText.h"

#ifdef ___CINT___
ClassImp(MGUISpectralAnalyzer)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUISpectralAnalyzer::MGUISpectralAnalyzer(const TGWindow* p, const TGWindow* main, 
                                           MSettingsSpectralOptions* Data, bool* OkPressed)
  : MGUIDialog(p, main), m_OkPressed(OkPressed)
{
  // standard constructor - waiting

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUISpectralAnalyzer::~MGUISpectralAnalyzer()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUISpectralAnalyzer::Create()
{
  // Create the main window

  int FieldSize = 95;
  int LeftGap = 40;
  int RightGap = 40;

  // We start with a name and an icon...
  SetWindowName("Spectral Analyzer");  
 
  // The subtitle
  SetSubTitleText("Please set all the options for the spectral analyzer");

  CreateBasics();

//  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, LeftGap, RightGap, 5, 5);
  TGLayoutHints* SingleFirstLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, LeftGap, RightGap, 20, 5);
  TGLayoutHints* SingleLastLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, LeftGap, RightGap, 5, 20);

  m_MainTab = new TGTab(m_MainPanel, 300, 300);
  TGLayoutHints* MainTabLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 10, 10, 0, 0);
  m_MainPanel->AddFrame(m_MainTab, MainTabLayout);

  TGCompositeFrame* PeakFinderFrame = m_MainTab->AddTab("Peak Finder");
//  TGCompositeFrame* PeakSelectorFrame = m_MainTab->AddTab("Peak selector");
  TGCompositeFrame* IsotopeIDFrame = m_MainTab->AddTab("Isotope ID");
  

  // Peak finder Frame  
/*  m_HistBinsSpectralyzer =
        new MGUIEEntry(PeakFinderFrame,
                   "Bins (default: 400):",
                   false,
                   m_GUIData->GetSpectralHistBinsSpectralyzer(),
                   true, 10, 10000);
  m_HistBinsSpectralyzer->SetEntryFieldSize(FieldSize);
  PeakFinderFrame->AddFrame(m_HistBinsSpectralyzer, SingleFirstLayout);
*/
  m_SignaltoNoiseRatio = 
    new MGUIEEntry(PeakFinderFrame,
                   "Sigma above background (default: 3):",
                   false,
                   m_GUIData->GetSpectralSignaltoNoiseRatio(),
                   true, 1, 100);
  m_SignaltoNoiseRatio->SetEntryFieldSize(FieldSize);
  PeakFinderFrame->AddFrame(m_SignaltoNoiseRatio, SingleFirstLayout);

  m_PoissonLimit = 
    new MGUIEEntry(PeakFinderFrame,
                   "Gaussian Limit on Integrated Counts (default: 20) [keV]:",
                   false,
                   m_GUIData->GetSpectralPoissonLimit(),
                   true, 1, 1000);
  m_PoissonLimit->SetEntryFieldSize(FieldSize);
  PeakFinderFrame->AddFrame(m_PoissonLimit, SingleLastLayout);



  
  // Peak selector frame
/*  m_DeconvolutionLimit = 
    new MGUIEEntry(PeakSelectorFrame,
                   "Deconvolve peaks with energy resolution above x*sigma (default: 3):",
                   false,
                   m_GUIData->GetSpectralDeconvolutionLimit(),
                   true, 1.0, 10.0);
  m_DeconvolutionLimit->SetEntryFieldSize(FieldSize);
  PeakSelectorFrame->AddFrame(m_DeconvolutionLimit, SingleLayout);
*/  

  
  // Isotope ID frame
  m_IsotopeFile = 
    new MGUIEFileSelector(IsotopeIDFrame,
                    MString("File containing isotope list for identification:"),
                    m_GUIData->GetSpectralIsotopeFileName());
  m_IsotopeFile->SetFileType("Isotope ID file", "*.isotopes");
  IsotopeIDFrame->AddFrame(m_IsotopeFile, SingleFirstLayout);

  m_EnergyWindow = 
    new MGUIEEntry(IsotopeIDFrame,
                    "Energy match [sigma from peak]:",
                    false,
                    m_GUIData->GetSpectralEnergyRange(), 
                    true, 0.0, 5.0);
  m_EnergyWindow->SetEntryFieldSize(FieldSize);
  IsotopeIDFrame->AddFrame(m_EnergyWindow, SingleLastLayout);


  // Finally bring it to the screen
  FinalizeCreate(GetDefaultWidth(), GetDefaultHeight()+10, false);

  if (m_OkPressed != 0) {
    fClient->WaitFor(this);
  }
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUISpectralAnalyzer::ProcessMessage(long Message, long Parameter1, 
                                        long Parameter2)
{
  // Process the messages for this application
  //cout<<"Messages: "<<Message<<", "<<Parameter1<<", "<<Parameter2<<endl;

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_RADIOBUTTON:
       break;
    case kCM_CHECKBUTTON:
      break;

    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
        Status = OnOk();
        break;

      case e_Cancel:
        Status = OnCancel();
        if (m_OkPressed != 0) {
          *m_OkPressed = false;
        }
        break;  
        
      default:
        break;
      }
      
    case kCM_MENUSELECT:
      break;
      
    case kCM_MENU:
      switch (Parameter1) {
      default:
        break;
      }
    default:
      break;
    }
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUISpectralAnalyzer::OnApply()
{

  // Check the data:
 


  // And save it:
/*  if (m_HistBinsSpectralyzer->IsModified() == true) {
    m_GUIData->SetSpectralHistBinsSpectralyzer(m_HistBinsSpectralyzer->GetAsInt());
  }*/
  if (m_SignaltoNoiseRatio->IsModified() == true) {
    m_GUIData->SetSpectralSignaltoNoiseRatio(m_SignaltoNoiseRatio->GetAsInt());
  }
  if (m_PoissonLimit->IsModified() == true) {
    m_GUIData->SetSpectralPoissonLimit(m_PoissonLimit->GetAsInt());
  }

/*  if (m_DeconvolutionLimit->IsModified() == true) {
    m_GUIData->SetSpectralDeconvolutionLimit(m_DeconvolutionLimit->GetAsDouble());
  }
*/
  if (m_IsotopeFile->GetFileName() != m_GUIData->GetSpectralIsotopeFileName()) {
    m_GUIData->SetSpectralIsotopeFileName(m_IsotopeFile->GetFileName());
  }
  if (m_EnergyWindow->IsModified() == true) {
    m_GUIData->SetSpectralEnergyRange(m_EnergyWindow->GetAsDouble());
  }


  if (m_OkPressed != 0) {
    *m_OkPressed = true;
  }
  
  return true;
}


// MGUISpectralAnalyzer: the end...
////////////////////////////////////////////////////////////////////////////////
