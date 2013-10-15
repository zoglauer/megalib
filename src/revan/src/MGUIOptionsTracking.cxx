/*
 * MGUIOptionsTracking.cxx
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
// MGUIOptionsTracking
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsTracking.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MRawEventAnalyzer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsTracking)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTracking::MGUIOptionsTracking(const TGWindow* Parent, 
                                         const TGWindow* Main, 
                                         MSettingsEventReconstruction* Data,
                                         MDGeometryQuest* Geometry)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIOptionsTracking and bring it to the screen

  m_Data = Data;
  m_Geometry = Geometry;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTracking::~MGUIOptionsTracking()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsTracking::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Tracking");  

  if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoModifiedPearson) {
    AddSubTitle("Tracking options for Modified Pearson correlation:"); 
  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoPearson) {
    AddSubTitle("Tracking options for Pearson correlation:"); 
  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoRank) {
    AddSubTitle("Tracking options for Rank correlation:"); 
  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoChiSquare) {
    AddSubTitle("Tracking options for Chi-Square method:"); 
  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoGas) {
    AddSubTitle("Tracking options for gas detectors:"); 
  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoDirectional) {
    AddSubTitle("Tracking options for intrinsic electron tracking detectors - no options!"); 
  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoBayesian) {
    AddSubTitle("Tracking options for Bayesian electron tracking:"); 
  } else {
    AddSubTitle("You deselected electron tracking!");     
  }

  if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoModifiedPearson || 
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoPearson ||
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoRank ||
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoChiSquare ||
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoGas ||
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoBayesian) {
    m_CBList = new MGUIECBList(this, "Search for the following event types:");
    m_CBList->Add("Search for MIPS");              
    m_CBList->SetSelected(0, m_Data->GetSearchMIPs());
    m_CBList->Add("Search for pairs");  
    m_CBList->SetSelected(1, m_Data->GetSearchPairs());
    m_CBList->Add("Search for Compton tracks");          
    m_CBList->SetSelected(2, m_Data->GetSearchComptons());
//     if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoPearson) {
//       m_CBList->Add("Cheating: Assume Compton tracks go from top to bottom");
//       m_CBList->SetSelected(3, m_Data->GetAssumeTrackTopBottom());
//     }          
    m_CBList->Create();
    TGLayoutHints* CBListLayout = 
      new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 0);
    AddFrame(m_CBList, CBListLayout);

    
    TGLayoutHints* SubLabel = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 20, 5);
    TGLabel* ComptonLabel = new TGLabel(this, "Compton options:");
    AddFrame(ComptonLabel, SubLabel);
    
    TGLayoutHints* SubOptions = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 0, 5);
    m_MaxComptonJump = new MGUIEEntry(this,
                                      "Maximum layer jump for Comptons:",
                                      false,
                                      m_Data->GetMaxComptonJump(), true, 1);
    AddFrame(m_MaxComptonJump, SubOptions);

    m_NSequencesToKeep = new MGUIEEntry(this,
                                      "Maximum number of different track sequences to keep:",
                                      false,
                                      m_Data->GetNTrackSequencesToKeep(), true, 1);
    AddFrame(m_NSequencesToKeep, SubOptions);

    m_RejectPurelyAmbiguousSequences = new TGCheckButton(this,
                                                         "Reject purely ambiguous sequences",
                                                         e_RejectPurelyAmbiguousSequences);
    AddFrame(m_RejectPurelyAmbiguousSequences, SubOptions);
    if (m_Data->GetRejectPurelyAmbiguousTrackSequences() == true) {
      m_RejectPurelyAmbiguousSequences->SetState(kButtonDown);
    } else {
      m_RejectPurelyAmbiguousSequences->SetState(kButtonUp);
    }
    

    TGLabel* PairLabel = new TGLabel(this, "Pair options:");
    AddFrame(PairLabel, SubLabel);

    m_NLayersForVertexSearch = new MGUIEEntry(this,
                                      "Search for a pair vertex within this amount of layers (min 4):",
                                      false,
                                      m_Data->GetNLayersForVertexSearch(), true, 4);
    AddFrame(m_NLayersForVertexSearch, SubOptions);


    if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoBayesian) {
      TGLabel* GeneralLabel = new TGLabel(this, "General options:");
      AddFrame(GeneralLabel, SubLabel);

      m_FileSelector = 
        new MGUIEFileSelector(this, "File containing the Bayesian tracking data:", 
                              m_Data->GetBayesianElectronFileName());
      m_FileSelector->SetFileType("Response file", "*.rsp");
      AddFrame(m_FileSelector, SubOptions);      
    }

    
    TGLabel* LabelDetectors = new TGLabel(this, "Select the tracking detector(s):");
    TGLayoutHints* LabelDetectorsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 20, 0);
    AddFrame(LabelDetectors, LabelDetectorsLayout);

    m_DetectorList = new TGListBox(this, e_Detectors);
    m_DetectorList->SetMultipleSelections(true);
    TGLayoutHints* DetectorListLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 5, 0);
    AddFrame(m_DetectorList, DetectorListLayout);

    if (m_Geometry != 0) {
      for (unsigned int d = 0; d < m_Geometry->GetNDetectors(); ++d) {
        MString Name = m_Geometry->GetDetectorAt(d)->GetName() + " (" + m_Geometry->GetDetectorAt(d)->GetTypeName() + ")";
        m_DetectorList->AddEntry(Name, d);

        // Highlight those detectors which are already in the list:
        for (unsigned int e = 0; e < m_Data->GetNElectronTrackingDetectors(); ++e) {
          if (m_Data->GetElectronTrackingDetector(e) == m_Geometry->GetDetectorAt(d)->GetName()) {
            m_DetectorList->Select(d);
            break;
          }
        }
      }
    } else {
      merr<<"No geometry!"<<show; 
    }
    m_DetectorList->Resize(200, 100);
    
  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoDirectional) {
    // no options...
  }

  AddButtons();
  PositionWindow(GetDefaultWidth()+30, GetDefaultHeight()+30, false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsTracking::OnApply()
{
	// The Apply button has been pressed

  if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoModifiedPearson || 
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoPearson ||
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoRank ||
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoChiSquare ||
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoGas ||
      m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoBayesian) {
    m_Data->SetSearchMIPs(m_CBList->GetSelected(0));
    m_Data->SetSearchPairs(m_CBList->GetSelected(1));
    m_Data->SetSearchComptons(m_CBList->GetSelected(2));
    m_Data->SetMaxComptonJump(m_MaxComptonJump->GetAsInt());
    m_Data->SetNTrackSequencesToKeep(m_NSequencesToKeep->GetAsInt());
    m_Data->SetNLayersForVertexSearch(m_NLayersForVertexSearch->GetAsInt());  
    if (m_RejectPurelyAmbiguousSequences->GetState() == kButtonDown) {
      m_Data->SetRejectPurelyAmbiguousTrackSequences(true);
    } else {
      m_Data->SetRejectPurelyAmbiguousTrackSequences(false);
    }
    
    m_Data->RemoveAllElectronTrackingDetectors();
    for (int d = 0; d < m_DetectorList->GetNumberOfEntries(); ++d) {
      if (m_DetectorList->GetSelection(d) == true) {
        m_Data->AddElectronTrackingDetector(m_Geometry->GetDetectorAt(d)->GetName());
      }
    }
  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoDirectional) {
    // no options...
  }

   
//   if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoPearson || 
//       m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoRank) {
//     m_Data->SetAssumeTrackTopBottom(m_CBList->GetSelected(3));
//   } 
  
  if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoBayesian) {
    m_Data->SetBayesianElectronFileName(m_FileSelector->GetFileName());
  }

  
  
	return true;
}



// MGUIOptionsTracking: the end...
////////////////////////////////////////////////////////////////////////////////
