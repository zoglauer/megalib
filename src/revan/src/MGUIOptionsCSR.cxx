/*
 * MGUIOptionsCSR.cxx
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
// MGUIOptionsCSR
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsCSR.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TGMsgBox.h"

// MEGAlib libs:
#include "MGUIEFileSelector.h"
#include "MRawEventAnalyzer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsCSR)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsCSR::MGUIOptionsCSR(const TGWindow* Parent, const TGWindow* Main, 
                               MSettingsEventReconstruction* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIOptionsCSR and bring it to the screen

  m_Data = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
  m_Options = 0;
  m_OptionsUndecided = 0;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsCSR::~MGUIOptionsCSR()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsCSR::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Compton Sequence Reconstruction");  

  if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoM) {
    AddSubTitle("Options for classic Compton sequence reconstruction"); 

    m_Options = new MGUIECBList(this, "General options:");
    m_Options->Add("Reject events which are limited to one detector type");          
    m_Options->Add("Only look at combinations which start in 2D strip detectors (tracker) or in drift chambers");              
    m_Options->Add("Use two-site events without track");
    m_Options->SetSelected(0, m_Data->GetRejectOneDetectorTypeOnlyEvents());
    m_Options->SetSelected(1, m_Data->GetGuaranteeStartD1());
    m_Options->SetSelected(2, m_Data->GetUseComptelTypeEvents());
    m_Options->Create();
    m_Options->Associate(this);
    TGLayoutHints* OptionsLayout = 
      new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 0, 10);
    AddFrame(m_Options, OptionsLayout);

    m_OptionsUndecided = new MGUIERBList(this, "Handling of two-site events without track:");
    m_OptionsUndecided->Add("Reject if ambiguous");              
    m_OptionsUndecided->Add("First hit has to be in 2D strip detectors (tracker) or in drift chambers - otherwise reject");              
    m_OptionsUndecided->Add("First hit has larger Klein-Nishina probability");
    m_OptionsUndecided->Add("First hit has larger Klein-Nishina times photo absorption probability");
    m_OptionsUndecided->Add("First hit has larger energy deposit");
    m_OptionsUndecided->SetSelected(m_Data->GetClassicUndecidedHandling());
    m_OptionsUndecided->Create();
    TGLayoutHints* OptionsUndecidedLayout = 
      new TGLayoutHints(kLHintsLeft | kLHintsTop, 60, 20, 0, 20);
    AddFrame(m_OptionsUndecided, OptionsUndecidedLayout);
    
    
    TGLayoutHints* EntryLayout = 
      new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 20);
    m_Threshold = new MGUIEMinMaxEntry(this,
                                       "Allowed range for Compton quality factor:",
                                       false,
                                       m_Data->GetCSRThresholdMin(), 
                                       m_Data->GetCSRThresholdMax(), true, 0.0);
    AddFrame(m_Threshold, EntryLayout);
    
    m_MaxNSingleHits = new MGUIEEntry(this,
                                      "Maximum number of interactions",
                                      false,
                                      m_Data->GetCSRMaxNHits(), true, 3);
    AddFrame(m_MaxNSingleHits, EntryLayout);
    

//     m_FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 2);
//     m_FileSelector = 
//       new MGUIEFileSelector(this, "All Comptons need to originate from objects in this file (Ignore if empty):", 
//                             m_Data->GetOriginObjectsFileName());
//     m_FileSelector->SetFileType("Geometry file", "*.geo.setup");
//     AddFrame(m_FileSelector, m_FileSelectorLayout);
  } else if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoME || 
             m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoMToF || 
             m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoMToFAndE) {
    if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoME) {
      AddSubTitle("Options for Compton sequence reconstruction with energy recovery"); 
    } else if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoMToF) {
      AddSubTitle("Options for Compton sequence reconstruction with time-of-flight"); 
    } else if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoMToFAndE) {
      AddSubTitle("Options for Compton sequence reconstruction with time-of-flight and energy recovery"); 
    } 

    m_Options = new MGUIECBList(this, "General options:");
    m_Options->Add("Only use events which start in 2D strip or drift chamber detectors");  
    m_Options->SetSelected(0, m_Data->GetGuaranteeStartD1());
    m_Options->Create();
    
    TGLayoutHints* OptionsLayout = 
      new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 0, 20);
    AddFrame(m_Options, OptionsLayout);

    TGLayoutHints* EntryLayout = 
      new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 20);
    m_Threshold = new MGUIEMinMaxEntry(this,
                                       "Allowed range for Compton quality factor:",
                                       false,
                                       m_Data->GetCSRThresholdMin(), 
                                       m_Data->GetCSRThresholdMax(), true, 0.0);
    AddFrame(m_Threshold, EntryLayout);
    
    m_MaxNSingleHits = new MGUIEEntry(this,
                                      "Maximum number of interactions",
                                      false,
                                      m_Data->GetCSRMaxNHits(), true, 3);
    AddFrame(m_MaxNSingleHits, EntryLayout);
    
  } else if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoBayesian) {
    AddSubTitle("Options for Bayesian Compton tracking"); 
    m_BayesianFileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 2);
    m_BayesianFileSelector = 
      new MGUIEFileSelector(this, "File containing the Bayesian data (\".mc.goodbad.rsp\"):", 
                            m_Data->GetBayesianComptonFileName());
    m_BayesianFileSelector->SetFileType("Response file", "*.rsp");
    AddFrame(m_BayesianFileSelector, m_BayesianFileSelectorLayout);      
    
    TGLayoutHints* EntryLayout = 
      new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 20, 20);
    m_MaxNSingleHits = new MGUIEEntry(this,
                                      "Maximum number of interactions:",
                                      false,
                                      m_Data->GetCSRMaxNHits(), true, 3);
    AddFrame(m_MaxNSingleHits, EntryLayout);

  } else {
    AddSubTitle("You deselected Compton tracking"); 
  }

  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsCSR::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this window

  if (m_Options != 0 && m_OptionsUndecided != 0) {
    if (m_Options->IsSelected(2) == true && m_OptionsUndecided->IsEnabled() == false) {
      m_OptionsUndecided->SetEnabled(true);
    } else if (m_Options->IsSelected(2) == false && m_OptionsUndecided->IsEnabled() == true) {
      m_OptionsUndecided->SetEnabled(false);    
    }
  }
  
  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}

////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsCSR::OnApply()
{
  // The Apply button has been pressed
  if (m_MaxNSingleHits->GetAsInt() > 8) {
    int Return = 0;
    MString Text = "A maximum number of interactions beyond 8 is rather unreasonable and consumes large amounts of memory!\n";
    Text += "Do you really want to proceed?";
    new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(),
                 "Warning", Text, kMBIconExclamation, kMBYes | kMBNo, &Return);
    if (Return == kMBNo) {
      return false;
    }
  }

  if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoM) {
    if (m_Threshold->CheckRange(0.0, numeric_limits<double>::max(), 
                                0.0, numeric_limits<double>::max(), true) == false) {
      return false;
    }

    
    m_Data->SetRejectOneDetectorTypeOnlyEvents(m_Options->GetSelected(0));
    m_Data->SetGuaranteeStartD1(m_Options->GetSelected(1));
    m_Data->SetUseComptelTypeEvents(m_Options->GetSelected(2));

    m_Data->SetClassicUndecidedHandling(m_OptionsUndecided->GetSelected());
    
    m_Data->SetCSRThresholdMin(m_Threshold->GetMinValue());
    m_Data->SetCSRThresholdMax(m_Threshold->GetMaxValue());
    m_Data->SetCSRMaxNHits(m_MaxNSingleHits->GetAsInt());

    //m_Data->SetOriginObjectsFileName(m_FileSelector->GetFileName());
  } else if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoME || 
             m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoMToFAndE ||
             m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoFoMToF) {
    if (m_Threshold->CheckRange(0.0, numeric_limits<double>::max(), 
                                0.0, numeric_limits<double>::max(), true) == false) {
      return false;
    }
    
    m_Data->SetGuaranteeStartD1(m_Options->GetSelected(0));
    
    m_Data->SetCSRThresholdMin(m_Threshold->GetMinValue());
    m_Data->SetCSRThresholdMax(m_Threshold->GetMaxValue());
    m_Data->SetCSRMaxNHits(m_MaxNSingleHits->GetAsInt());
  } else if (m_Data->GetCSRAlgorithm() == MRawEventAnalyzer::c_CSRAlgoBayesian) {
    m_Data->SetBayesianComptonFileName(m_BayesianFileSelector->GetFileName());
    m_Data->SetCSRMaxNHits(m_MaxNSingleHits->GetAsInt());
  }
  return true;
}


// MGUIOptionsCSR: the end...
////////////////////////////////////////////////////////////////////////////////
