/*
 * MGUIOptionsEventType.cxx
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
// MGUIOptionsEventType
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsEventType.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MRawEventAnalyzer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsEventType)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventType::MGUIOptionsEventType(const TGWindow* Parent,
                                         const TGWindow* Main,
                                         MSettingsEventReconstruction* Data,
                                         MDGeometryQuest* Geometry)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIOptionsEventType and bring it to the screen
  m_Data = Data;
  m_Geometry = Geometry;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventType::~MGUIOptionsEventType()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEventType::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Event Type Identification");

  if (m_Data->GetEventTypeAlgorithm() == MRawEventAnalyzer::c_EventTypeDefault) {
    // Which event types to search
    AddSubTitle("Options for default algorithm");
    m_CBList = new MGUIECBList(this, "Search for the following event types:");
    m_CBList->Add("Search for single-site");
    m_CBList->SetSelected(0, m_Data->GetSearchPhoto());
    m_CBList->Add("Search for pairs");
    m_CBList->SetSelected(1, m_Data->GetSearchPairs());
    m_CBList->Add("Search for MIPS");
    m_CBList->SetSelected(2, m_Data->GetSearchMIPs());
    m_CBList->Add("Search for Compton tracks");
    m_CBList->SetSelected(3, m_Data->GetSearchComptons());
    m_CBList->Create();
    TGLayoutHints* CBListLayout =
      new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 0);
    AddFrame(m_CBList, CBListLayout);

    // Number of layers for vertex
    TGLayoutHints* SubLabel = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 20, 5);
    TGLabel* PairLabel = new TGLabel(this, "Pair options:");
    AddFrame(PairLabel, SubLabel);

    TGLayoutHints* SubOptions = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 0, 5);
    m_NLayersForVertexSearch = new MGUIEEntry(this,
                                      "Search for a pair vertex within this amount of layers (min 4):",
                                      false,
                                      m_Data->GetNLayersForVertexSearch(), true, 4);
    AddFrame(m_NLayersForVertexSearch, SubOptions);

    // Select tracking detectors
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

  } else if (m_Data->GetEventTypeAlgorithm() == MRawEventAnalyzer::c_EventTypeExternal) {
    AddSubTitle("Options for external input");

    TGLayoutHints* SubOptions = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 0, 5);

    m_FileSelector =
      new MGUIEFileSelector(this, "File containing the event type data:",
                            m_Data->GetEventTypeFileName());
    m_FileSelector->SetFileType("file to be implemented", "*.etp");
    AddFrame(m_FileSelector, SubOptions);
  } else {
    AddSubTitle("Unknown Event Type identification algorithm!");
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


bool MGUIOptionsEventType::OnApply()
{
	// The Apply button has been pressed

  if (m_Data->GetEventTypeAlgorithm() == MRawEventAnalyzer::c_EventTypeDefault) {
    m_Data->SetSearchPhoto(m_CBList->GetSelected(0));
    m_Data->SetSearchPairs(m_CBList->GetSelected(1));
    m_Data->SetSearchMIPs(m_CBList->GetSelected(2));
    m_Data->SetSearchComptons(m_CBList->GetSelected(3));
    m_Data->SetNLayersForVertexSearch(m_NLayersForVertexSearch->GetAsInt());

    m_Data->RemoveAllElectronTrackingDetectors();
    for (int d = 0; d < m_DetectorList->GetNumberOfEntries(); ++d) {
      if (m_DetectorList->GetSelection(d) == true) {
        m_Data->AddElectronTrackingDetector(m_Geometry->GetDetectorAt(d)->GetName());
      }
    }
  } else if (m_Data->GetEventTypeAlgorithm() == MRawEventAnalyzer::c_EventTypeExternal) {
    m_Data->SetEventTypeFileName(m_FileSelector->GetFileName());
  }

	return true;
}



// MGUIOptionsEventType: the end...
////////////////////////////////////////////////////////////////////////////////
