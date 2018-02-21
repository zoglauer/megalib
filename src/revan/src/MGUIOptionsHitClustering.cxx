/*
 * MGUIOptionsHitClustering.cxx
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
// MGUIOptionsHitClustering
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsHitClustering.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MRawEventAnalyzer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsHitClustering)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsHitClustering::MGUIOptionsHitClustering(const TGWindow* Parent, 
                                             const TGWindow* Main, 
                                             MSettingsEventReconstruction* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIOptionsHitClustering and bring it to the screen

  m_Data = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsHitClustering::~MGUIOptionsHitClustering()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsHitClustering::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Options for clustering neighboring hits");  

  TGLayoutHints* EntryLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 10, 0);

  if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoDistance) {
    AddSubTitle("Options for clustering by distance:"); 
    
    m_MinDistance = new MGUIEEntryList(this, "Minimum distance:");
    m_MinDistance->Add("Double-sided strip detector (ID=1) [cm]", 
                       m_Data->GetStandardClusterizerMinDistanceD1(), true, 0.0);
    m_MinDistance->Add("Calorimeter (ID=2) [cm]", 
                       m_Data->GetStandardClusterizerMinDistanceD2(), true, 0.0);
    m_MinDistance->Add("Double-sided strip detector with depth resolution (ID=3) [cm]", 
                       m_Data->GetStandardClusterizerMinDistanceD3(), true, 0.0);
    m_MinDistance->Add("Scintillator (ID=4) [cm]", 
                       m_Data->GetStandardClusterizerMinDistanceD4(), true, 0.0);
    m_MinDistance->Add("Drift chamber (ID=5) [cm]", 
                       m_Data->GetStandardClusterizerMinDistanceD5(), true, 0.0);
    m_MinDistance->Add("Double-sided strip detector with depth and directional resolution (ID=6) [cm]       ", 
                       m_Data->GetStandardClusterizerMinDistanceD6(), true, 0.0);
    m_MinDistance->Add("Anger camera (ID=7) [cm]", 
                       m_Data->GetStandardClusterizerMinDistanceD7(), true, 0.0);
    m_MinDistance->Add("Voxel detector (ID=8) [cm]", 
                       m_Data->GetStandardClusterizerMinDistanceD8(), true, 0.0);
    m_MinDistance->Create();
    AddFrame(m_MinDistance, EntryLayout);


    TGLayoutHints* ReferenceLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 20, 0);

    m_ReferencePoint = new TGCheckButton(this, "Use center of cluster (instead of each sub hit individually) as reference point for distance calculations", e_ReferencePoint);
    if (m_Data->GetStandardClusterizerCenterIsReference() == true) {
      m_ReferencePoint->SetState(kButtonDown);
    } else {
      m_ReferencePoint->SetState(kButtonUp);
    }
    AddFrame(m_ReferencePoint, ReferenceLayout);

  } else if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoAdjacent) {
    AddSubTitle("Options for adjacent voxel clustering:"); 
    m_AdjacentLevel = new TGComboBox(this);
    m_AdjacentLevel->AddEntry("4 neighboring voxels", 1);
    m_AdjacentLevel->AddEntry("8 neighboring voxels", 2);
    m_AdjacentLevel->AddEntry("12 neighboring voxels", 4);
    m_AdjacentLevel->AddEntry("20 neighboring voxels", 5);
    m_AdjacentLevel->AddEntry("24 neighboring voxels", 8);
    m_AdjacentLevel->SetHeight(m_FontScaler*20);
    if (m_Data->GetAdjacentLevel() == 1 || 
        m_Data->GetAdjacentLevel() == 2 ||
        m_Data->GetAdjacentLevel() == 4 ||
        m_Data->GetAdjacentLevel() == 5 ||
        m_Data->GetAdjacentLevel() == 8) {
      m_AdjacentLevel->Select(m_Data->GetAdjacentLevel());
    } else {
      m_AdjacentLevel->Select(1);     
    }
    AddFrame(m_AdjacentLevel, EntryLayout);
    m_AdjacentSigma = 
      new MGUIEEntry(this,
                     "Position sigma for non voxelized dimension (i.e. depth) in sigmas (-1: ignore)    ",
                     false,
                     m_Data->GetAdjacentSigma(), true, -10.0);
    AddFrame(m_AdjacentSigma, EntryLayout);
  } else if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoPDF) {
    AddSubTitle("Options for clustering using a probability densitiy function:");
    
    m_PDFClusterizerBaseFileName = new MGUIEFileSelector(this, "File containing the data (XXX.dualseparable.yes.rsp):", 
                                           m_Data->GetPDFClusterizerBaseFileName());
    m_PDFClusterizerBaseFileName->SetFileType("response file", "*.dualseparable.yes.rsp");
    AddFrame(m_PDFClusterizerBaseFileName, EntryLayout);      
  } else {
    AddSubTitle("You deselected clustering!");     
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


bool MGUIOptionsHitClustering::OnApply()
{
  // The Apply button has been pressed

  if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoDistance) {
    m_Data->SetStandardClusterizerMinDistanceD1(m_MinDistance->GetAsDouble(0));
    m_Data->SetStandardClusterizerMinDistanceD2(m_MinDistance->GetAsDouble(1));
    m_Data->SetStandardClusterizerMinDistanceD3(m_MinDistance->GetAsDouble(2));
    m_Data->SetStandardClusterizerMinDistanceD4(m_MinDistance->GetAsDouble(3));
    m_Data->SetStandardClusterizerMinDistanceD5(m_MinDistance->GetAsDouble(4));
    m_Data->SetStandardClusterizerMinDistanceD6(m_MinDistance->GetAsDouble(5));
    m_Data->SetStandardClusterizerMinDistanceD7(m_MinDistance->GetAsDouble(6));
    m_Data->SetStandardClusterizerMinDistanceD8(m_MinDistance->GetAsDouble(7));
    if (m_ReferencePoint->GetState() == kButtonDown) {
      m_Data->SetStandardClusterizerCenterIsReference(true);
    } else {
      m_Data->SetStandardClusterizerCenterIsReference(false);
    }
  } else if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoAdjacent) {
    m_Data->SetAdjacentLevel(m_AdjacentLevel->GetSelected());
    m_Data->SetAdjacentSigma(m_AdjacentSigma->GetAsDouble());
  } else if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoPDF) {
    m_Data->SetPDFClusterizerBaseFileName(m_PDFClusterizerBaseFileName->GetFileName());
  }

  return true;
}


// MGUIOptionsHitClustering: the end...
////////////////////////////////////////////////////////////////////////////////
