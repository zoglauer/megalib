/*
 * MGUIOptionsEventClustering.cxx
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
// MGUIOptionsEventClustering
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsEventClustering.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MRawEventAnalyzer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsEventClustering)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventClustering::MGUIOptionsEventClustering(const TGWindow* Parent, const TGWindow* Main, MSettingsEventReconstruction* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIOptionsEventClustering and bring it to the screen

  m_Data = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventClustering::~MGUIOptionsEventClustering()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEventClustering::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Options for clustering neighboring hits");  

  //TGLayoutHints* EntryLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 10, 0);

  if (m_Data->GetEventClusteringAlgorithm() == MRawEventAnalyzer::c_EventClusteringAlgoTMVA) {
    
    AddSubTitle("Options for TMVA-based event clustering"); 
    
    TGLayoutHints* TMVAFileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 2);
    m_TMVAFileSelector = 
    new MGUIEFileSelector(this, "File containing the TMVA data (\".tmva\"):", 
                          m_Data->GetEventClusteringTMVAFileName());
    m_TMVAFileSelector->SetFileType("TMVA steering file", "*.tmva");
    AddFrame(m_TMVAFileSelector, TMVAFileSelectorLayout);      
    
    m_TMVAMethods = new MGUIERBList(this, "Choose the TMVA method (if it is not in the tmva file, you will get an error message later):");
    vector<MERCSRTMVAMethod> Methods = m_Data->GetEventClusteringTMVAMethods().GetAllMethods();
    for (unsigned int m = 0; m < Methods.size(); ++m) {
      m_TMVAMethods->Add(m_Data->GetEventClusteringTMVAMethods().GetFullString(Methods[m]));
      m_TMVAMethodsMap[m] = Methods[m];
    }
    
    vector<MERCSRTMVAMethod> M = m_Data->GetEventClusteringTMVAMethods().GetUsedMethods();
    if (M.size() > 0) {
      for (auto I = m_TMVAMethodsMap.begin(); I != m_TMVAMethodsMap.end(); ++I) {
        if (I->second == M[0]) {
          m_TMVAMethods->SetSelected(I->first);
        }
      }
    } else {
      m_TMVAMethods->SetSelected(0);
    }
    m_TMVAMethods->Create();
    TGLayoutHints* TMVAMethodsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 20, 2);
    AddFrame(m_TMVAMethods, TMVAMethodsLayout); 
  } 
  
  else {
    AddSubTitle("No event clustering will be performed.");     
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


bool MGUIOptionsEventClustering::OnApply()
{
  // The Apply button has been pressed

  if (m_Data->GetEventClusteringAlgorithm() == MRawEventAnalyzer::c_EventClusteringAlgoTMVA) {
    m_Data->SetEventClusteringTMVAFileName(m_TMVAFileSelector->GetFileName());
    MERCSRTMVAMethods M;
    M.AddUsedMethod(m_TMVAMethodsMap[m_TMVAMethods->GetSelected()]);
    m_Data->SetEventClusteringTMVAMethods(M);
  }

  return true;
}


// MGUIOptionsEventClustering: the end...
////////////////////////////////////////////////////////////////////////////////
