/*
 * MGUIERAlgorithm.cxx
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
// MGUIERAlgorithm
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIERAlgorithm.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MRawEventAnalyzer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIERAlgorithm)
#endif


////////////////////////////////////////////////////////////////////////////////



MGUIERAlgorithm::MGUIERAlgorithm(const TGWindow* Parent, const TGWindow* Main, MSettingsEventReconstruction* Data) : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIERAlgorithm and bring it to the screen
  m_Data = Data;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIERAlgorithm::~MGUIERAlgorithm()
{
  // Delete an instance of MGUIERAlgorithm

  delete m_CoincidenceList;
  delete m_EventClusteringList;
  delete m_HitClusteringList;
  delete m_TrackingList;
  delete m_EventTypeList;
  delete m_CSRList;
//  delete m_PairList;
  //delete m_DecayList;
  delete m_ListLayout;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIERAlgorithm::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("ER algorithms");
  //AddSubTitle("Choose the reconstruction algorithms");

  m_ListLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 10, 10);

  // Coincidence search
  m_CoincidenceList = new MGUIERBList(this, "Coincidence search", true);
  m_CoincidenceList->Add("No coincidence or coincident hits already merged in simulation or hardware (DEFAULT)");
  m_CoincidenceList->Add("Coincidence window");
  m_CoincidenceList->SetSelected(m_Data->GetCoincidenceAlgorithm());
  m_CoincidenceList->Create();
  AddFrame(m_CoincidenceList, m_ListLayout);
  
  // Event clustering
  m_EventClusteringList = new MGUIERBList(this, "Find multiple, coincident gamma rays in one event", true);
  m_EventClusteringList->Add("No event clustering (DEFAULT - unless you have the very rare case of pile up)");
  m_EventClusteringList->Add("Event clustering using using interaction distances (alpha)");
  m_EventClusteringList->Add("Event clustering using TMVA machine learning (alpha)");
  if (m_Data->GetEventClusteringAlgorithm() == MRawEventAnalyzer::c_EventClusteringAlgoNone) {
    m_EventClusteringList->SetSelected(0);
  } else if (m_Data->GetEventClusteringAlgorithm() == MRawEventAnalyzer::c_EventClusteringAlgoTMVA) {
    m_EventClusteringList->SetSelected(2);
  } else if (m_Data->GetEventClusteringAlgorithm() == MRawEventAnalyzer::c_EventClusteringAlgoDistance) {
    m_EventClusteringList->SetSelected(1);
  } else {
    m_EventClusteringList->SetSelected(0);
  }
  m_EventClusteringList->Create();
  AddFrame(m_EventClusteringList, m_ListLayout);
  
  // Hit clustering
  m_HitClusteringList = new MGUIERBList(this, "Clustering of neighboring hits", true);
  m_HitClusteringList->Add("No clustering");
  m_HitClusteringList->Add("Clustering by Distance");
  m_HitClusteringList->Add("Clustering of adjacent voxels");
  m_HitClusteringList->Add("Clustering using probability density function");
  m_HitClusteringList->SetSelected(m_Data->GetHitClusteringAlgorithm());
  m_HitClusteringList->Create();
  AddFrame(m_HitClusteringList, m_ListLayout);
  
  // Event identification
  m_EventTypeList = new MGUIERBList(this, "Event type identification", true);
  m_EventTypeList->Add("Default algorithm"); // 0
  m_EventTypeList->Add("External from file (not implemented)"); // 1
  m_EventTypeList->SetSelected(m_Data->GetEventTypeAlgorithm());
  m_EventTypeList->Create();
  AddFrame(m_EventTypeList, m_ListLayout);

  // Electron tracking
  m_TrackingList = new MGUIERBList(this, "Electron tracking", true);
  m_TrackingList->Add("No electron tracking"); // 0
  m_TrackingList->Add("Modified Pearson correlation (Figure-of-merit approach)"); // 1
  m_TrackingList->Add("Pearson correlation (Correlation test)"); // 2
  m_TrackingList->Add("Spearman-Rank correlation (Correlation test)"); // 3
  m_TrackingList->Add("Chi-Square approach (Figure-of-merit approach)"); // 4
  m_TrackingList->Add("Tracking in gas detector (Figure-of-merit approach)"); // 5
  m_TrackingList->Add("Tracking in direction sensitive strip detector (Figure-of-merit approach)"); // 6
  m_TrackingList->Add("Bayesian electron tracking (Bayesian model selection)"); // 7
  m_TrackingList->Add("Kalman Filter 2D (work in progress)"); // 8
  m_TrackingList->Add("Kalman Filter 3D (work in progress)"); // 9
  m_TrackingList->Add("First two layers (to be implemented)"); // 10
  m_TrackingList->SetSelected(m_Data->GetTrackingAlgorithm());
//  if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoNone) {
//    m_TrackingList->SetSelected(0);
//  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoModifiedPearson) {
//    m_TrackingList->SetSelected(1);
//  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoPearson) {
//    m_TrackingList->SetSelected(2);
//  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoRank) {
//    m_TrackingList->SetSelected(3);
//  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoChiSquare) {
//    m_TrackingList->SetSelected(4);
//  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoGas) {
//    m_TrackingList->SetSelected(5);
//  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoDirectional) {
//    m_TrackingList->SetSelected(6);
//  } else if (m_Data->GetTrackingAlgorithm() == MRawEventAnalyzer::c_TrackingAlgoBayesian) {
//    m_TrackingList->SetSelected(7);
//  } else {
//    m_TrackingList->SetSelected(0);
//  }
  m_TrackingList->Create();
  AddFrame(m_TrackingList, m_ListLayout);
/*
  m_PairList = new MGUIERBList(this, "Pair reconstruction", true);
  m_PairList->Add("Default algorithm");  // 0
  //m_PairList->Add("Kalman Filter 3D (work in progress)"); // 1
  //m_PairList->Add("Kalman Filter 2D (work in progress)"); // 2
  m_PairList->SetSelected(m_Data->GetPairAlgorithm());
  m_PairList->Create();
  AddFrame(m_PairList, m_ListLayout);
*/
  m_CSRList = new MGUIERBList(this, "Compton tracking", true);
  m_CSRList->Add("No Compton tracking");
  m_CSRList->Add("Classic Compton Sequence Reconstruction without Energy Recovery (Chi-square approach via angles)");
  m_CSRList->Add("Classic Compton Sequence Reconstruction with Energy Recovery (Chi-square approach via energies)");
  m_CSRList->Add("Classic Compton Sequence Reconstruction with Time of Flight (Chi-square approach via angles and time)");
  m_CSRList->Add("Bayesian Compton-scatter pattern identification (Bayesian model selection)");
  m_CSRList->Add("TMVA-based Compton-scatter pattern identification (MLP, DNN, Boosted decision trees, etc.)");
  if (m_Data->GetSpecialMode() == true) {
    m_CSRList->Add("UNDER CONSTRUCTION: Classic Compton Sequence Reconstruction with Time of Flight and Energy Recovery");
  }
  m_CSRList->SetSelected(m_Data->GetCSRAlgorithm());
  m_CSRList->Create();
  AddFrame(m_CSRList, m_ListLayout);

  /*
  m_DecayList = new MGUIERBList(this, "Decay detection", true);
  m_DecayList->Add("No Decay detection");
  m_DecayList->Add("Decay detection for Comptons after Compton tracking");
  m_DecayList->SetSelected(m_Data->GetDecayAlgorithm());
  m_DecayList->Create();
  AddFrame(m_DecayList, m_ListLayout);
  */

  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight()-11, false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();

  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIERAlgorithm::OnApply()
{
  // The Apply button has been pressed

  m_Data->SetCoincidenceAlgorithm(m_CoincidenceList->GetSelected());
  
  if (m_EventClusteringList->GetSelected() == 0) {
    m_Data->SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoNone);
  } else if (m_EventClusteringList->GetSelected() == 1) {
    m_Data->SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoDistance);
  } else if (m_EventClusteringList->GetSelected() == 2) {
    m_Data->SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoTMVA);
  }
    
  m_Data->SetHitClusteringAlgorithm(m_HitClusteringList->GetSelected());
  
  if (m_TrackingList->GetSelected() == 0) {
    m_Data->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
  } else if (m_TrackingList->GetSelected() == 1) {
    m_Data->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoModifiedPearson);
  } else if (m_TrackingList->GetSelected() == 2) {
    m_Data->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoPearson);
  } else if (m_TrackingList->GetSelected() == 3) {
    m_Data->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoRank);
  } else if (m_TrackingList->GetSelected() == 4) {
    m_Data->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoChiSquare);
  } else if (m_TrackingList->GetSelected() == 5) {
    m_Data->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoGas);
  } else if (m_TrackingList->GetSelected() == 6) {
    m_Data->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoDirectional);
  } else if (m_TrackingList->GetSelected() == 7) {
    m_Data->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoBayesian);
  }
  //m_Data->SetPairAlgorithm(m_PairList->GetSelected());
  m_Data->SetCSRAlgorithm(m_CSRList->GetSelected());
  
  //m_Data->SetDecayAlgorithm(m_DecayList->GetSelected());
  
  m_Data->SetDecayAlgorithm(0);

  return true;
}


// MGUIERAlgorithm: the end...
////////////////////////////////////////////////////////////////////////////////
