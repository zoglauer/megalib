/*
 * MGUIEventSelection.cxx
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
// MGUIEventSelection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEventSelection.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TGMsgBox.h"
#include "TGTab.h"

// MEGAlib libs:
#include "MGUIEText.h"
#include "MCoordinateSystem.h"

#ifdef ___CLING___
ClassImp(MGUIEventSelection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEventSelection::MGUIEventSelection(const TGWindow* p, const TGWindow* main, 
                                       MSettingsEventSelections* Data, MDGeometryQuest* Geo)
  : MGUIDialog(p, main)
{
  // standard constructor

  m_Settings = Data;
  m_Geometry = Geo;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEventSelection::~MGUIEventSelection()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEventSelection::Create()
{
  // Create the main window

  int FieldSize = m_FontScaler*95;
  int LargeFieldSize = m_FontScaler*140;
  int LeftGap = m_FontScaler*65;
  int RightGap = m_FontScaler*65;
  int TopGap = m_FontScaler*20;

  // We start with a name and an icon...
  SetWindowName("Event selector");  
 
  // The subtitle
  SetSubTitleText("Please set all the cuts on the data for the analysis");

  CreateBasics();


  TGLayoutHints* MinMaxLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop | kLHintsLeft, LeftGap, RightGap, 10, 5);
  TGLayoutHints* MinMaxFirstLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop | kLHintsLeft, LeftGap, RightGap, TopGap, 5);
  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop | kLHintsLeft, LeftGap, RightGap, 10, 10);

  m_MainTab = new TGTab(m_MainPanel, 300, 300);
  TGLayoutHints* MainTabLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 10, 10, 0, 0);
  m_MainPanel->AddFrame(m_MainTab, MainTabLayout);

  TGCompositeFrame* EventTypeFrame = m_MainTab->AddTab("Type & ID");
  TGCompositeFrame* GeneralFrame = m_MainTab->AddTab("Time & Detector");
  TGCompositeFrame* EnergiesFrame = m_MainTab->AddTab("Energy");
  TGCompositeFrame* ComptonEAFrame = m_MainTab->AddTab("Scattering");
  TGCompositeFrame* ComptonDLFrame = m_MainTab->AddTab("Lengths");
  TGCompositeFrame* ComptonQFrame = m_MainTab->AddTab("Quality");
  TGCompositeFrame* ComptonEHCFrame = m_MainTab->AddTab("Earth");
  TGCompositeFrame* PairFrame = m_MainTab->AddTab("Pairs");
  TGCompositeFrame* SourceFrame = m_MainTab->AddTab("Origin");
  TGCompositeFrame* PointingFrame = m_MainTab->AddTab("Pointing");
  TGCompositeFrame* BeamFrame = m_MainTab->AddTab("Beam");
  TGCompositeFrame* SpecialFrame = 0;
  if (m_Settings->GetSpecialMode() == true) {
    SpecialFrame = m_MainTab->AddTab("Special");
  }

  m_MainTab->SetTab(m_Settings->GetEventSelectorTab());

  
  // Event type frame
  
  // Event and detector type frame:
  TGLabel* LabelEventSelection = new TGLabel(EventTypeFrame, new TGString("Choose from the following event types:"));
  TGLayoutHints* LabelEventSelectionLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, LeftGap, RightGap, TopGap, 0);
  EventTypeFrame->AddFrame(LabelEventSelection, LabelEventSelectionLayout);

  m_PhotoCB = new TGCheckButton(EventTypeFrame, "Single-site events (These will slow down imaging!)", c_Photo);
  TGLayoutHints* PhotoCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_PhotoCB, PhotoCBLayout);
  m_PhotoCB->SetState((m_Settings->GetEventTypePhoto() == 1) ?  kButtonDown : kButtonUp);

  m_ComptonCB = new TGCheckButton(EventTypeFrame, "Compton-scattering events:", c_Compton);
  TGLayoutHints* ComptonCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_ComptonCB, ComptonCBLayout);
  m_ComptonCB->Associate(this);
  m_ComptonCB->SetState((m_Settings->GetEventTypeCompton() == 1) ?  kButtonDown : kButtonUp);

  m_ComptonNotTrackedCB = new TGCheckButton(EventTypeFrame, "without recoil electron track", c_ComptonUntracked);
  TGLayoutHints* ComptonNotTrackedCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+30, RightGap, 2, 0);
  EventTypeFrame->AddFrame(m_ComptonNotTrackedCB, ComptonNotTrackedCBLayout);
  m_ComptonNotTrackedCB->SetState((m_Settings->GetEventTypeComptonNotTracked() == 1) ?  kButtonDown : kButtonUp);
  m_ComptonNotTrackedCB->Associate(this);
  if (m_ComptonCB->GetState() == kButtonUp) {
    m_ComptonNotTrackedCB->SetState(kButtonDisabled);
  }
  m_UseNotTracked = m_Settings->GetEventTypeComptonNotTracked();

  m_ComptonTrackedCB = new TGCheckButton(EventTypeFrame, "with recoil electron track", c_ComptonTracked);
  TGLayoutHints* ComptonTrackedCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+30, RightGap, 2, 0);
  EventTypeFrame->AddFrame(m_ComptonTrackedCB, ComptonTrackedCBLayout);
  m_ComptonTrackedCB->SetState((m_Settings->GetEventTypeComptonTracked() == 1) ?  kButtonDown : kButtonUp);
  m_ComptonTrackedCB->Associate(this);
  if (m_ComptonCB->GetState() == kButtonUp) {
    m_ComptonTrackedCB->SetState(kButtonDisabled);
  }
  m_UseTracked = m_Settings->GetEventTypeComptonTracked();
  
  m_PairCB = new TGCheckButton(EventTypeFrame, "Pair-creation events", c_Pair);
  TGLayoutHints* PairCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_PairCB, PairCBLayout);
  m_PairCB->SetState((m_Settings->GetEventTypePair() == 1) ?  kButtonDown : kButtonUp);
  
  m_PETCB = new TGCheckButton(EventTypeFrame, "Positron emission tomograhy events", c_PET);
  TGLayoutHints* PETCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_PETCB, PETCBLayout);
  m_PETCB->SetState((m_Settings->GetEventTypePET() == 1) ?  kButtonDown : kButtonUp);
  
  m_MultiCB = new TGCheckButton(EventTypeFrame, "Events consisting of multiple others", c_Multi);
  TGLayoutHints* MultiCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_MultiCB, MultiCBLayout);
  m_MultiCB->SetState((m_Settings->GetEventTypeMulti() == 1) ?  kButtonDown : kButtonUp);
  
  m_UnidentifiableCB = new TGCheckButton(EventTypeFrame, "Unidentifiable events", c_Unidentifiable);
  TGLayoutHints* UnidentifiableCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_UnidentifiableCB, UnidentifiableCBLayout);
  m_UnidentifiableCB->SetState((m_Settings->GetEventTypeUnidentifiable() == 1) ?  kButtonDown : kButtonUp);
  
  
  TGLabel* LabelSpecialEvents = new TGLabel(EventTypeFrame, new TGString("Choose, if events with these special flags should be used:"));
  EventTypeFrame->AddFrame(LabelSpecialEvents, LabelEventSelectionLayout);

  m_BadCB = new TGCheckButton(EventTypeFrame, "Use events where the \"Bad\" flag in set", c_Bad);
  TGLayoutHints* BadCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_BadCB, BadCBLayout);
  m_BadCB->SetState((m_Settings->GetFlaggedAsBad() == true) ?  kButtonDown : kButtonUp);

  m_DecayCB = new TGCheckButton(EventTypeFrame, "Use events where the \"Decay\" flag in set", c_Decay);
  TGLayoutHints* DecayCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_DecayCB, DecayCBLayout);
  m_DecayCB->SetState((m_Settings->GetEventTypeDecay() == 1) ?  kButtonDown : kButtonUp);


  // ID
  m_EventId = new MGUIEMinMaxEntry(EventTypeFrame,
                                   MString("Event ID window:"),
                                   false,
                                   MString("Minimum ID: "),
                                   MString("Maximum ID: "),
                                   m_Settings->GetEventIdRangeMin(), 
                                   m_Settings->GetEventIdRangeMax(), 
                                   true, 0l);
  m_EventId->SetEntryFieldSize(FieldSize);
  EventTypeFrame->AddFrame(m_EventId, MinMaxFirstLayout);



  // General Frame

  
  // GTI
  TGLabel* LabelTimes = new TGLabel(GeneralFrame, "Good time interval");
  TGLayoutHints* LabelTimesLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, LeftGap, RightGap, TopGap, 0);  
  GeneralFrame->AddFrame(LabelTimes, LabelTimesLayout);
    
  TGLayoutHints* GTIRBLayout = new TGLayoutHints(kLHintsLeft, LeftGap + 10*m_FontScaler, RightGap, 5*m_FontScaler, 2*m_FontScaler);

  m_GTIAllRB = new TGRadioButton(GeneralFrame, "Use all times", c_GTIAll);
  m_GTIAllRB->Associate(this);
  GeneralFrame->AddFrame(m_GTIAllRB, GTIRBLayout);
    
  m_GTIEntryRB = new TGRadioButton(GeneralFrame, "Set a single GTI directly", c_GTIEntry);
  m_GTIEntryRB->Associate(this);
  GeneralFrame->AddFrame(m_GTIEntryRB, GTIRBLayout);
  
  m_MinTimeEntry = new MGUIEEntry(GeneralFrame, "Minimum [sec]:", false, m_Settings->GetTimeRangeMin().GetLongIntsString());
  m_MinTimeEntry->SetEntryFieldSize(LargeFieldSize);
  TGLayoutHints* TimeEntryLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, LeftGap + 21*m_FontScaler, RightGap, 0, 2*m_FontScaler);
  GeneralFrame->AddFrame(m_MinTimeEntry, TimeEntryLayout);
  m_MaxTimeEntry = new MGUIEEntry(GeneralFrame, "Maximum [sec]:", false, m_Settings->GetTimeRangeMax().GetLongIntsString());
  m_MaxTimeEntry->SetEntryFieldSize(LargeFieldSize);
  GeneralFrame->AddFrame(m_MaxTimeEntry, TimeEntryLayout);
  
  m_GTIFileRB = new TGRadioButton(GeneralFrame, "Read the GTIs from file", c_GTIFile);
  m_GTIFileRB->Associate(this);
  GeneralFrame->AddFrame(m_GTIFileRB, GTIRBLayout);
  
  m_GTIFile = new MGUIEFileSelector(GeneralFrame, "", m_Settings->GetTimeFile());
  m_GTIFile->SetFileType("GTI file", "*.gti");
  GeneralFrame->AddFrame(m_GTIFile, TimeEntryLayout);

  if (m_Settings->GetTimeMode() == 1) {
    m_GTIAllRB->SetState(kButtonUp);
    m_GTIEntryRB->SetState(kButtonDown);
    m_GTIFileRB->SetState(kButtonUp);
    m_MinTimeEntry->SetEnabled(true);
    m_MaxTimeEntry->SetEnabled(true);
    m_GTIFile->SetEnabled(false);
  } else if (m_Settings->GetTimeMode() == 2) {
    m_GTIAllRB->SetState(kButtonUp);
    m_GTIEntryRB->SetState(kButtonUp);
    m_GTIFileRB->SetState(kButtonDown);
    m_MinTimeEntry->SetEnabled(false);
    m_MaxTimeEntry->SetEnabled(false);
    m_GTIFile->SetEnabled(true);
  } else {
    m_GTIAllRB->SetState(kButtonDown);
    m_GTIEntryRB->SetState(kButtonUp);
    m_GTIFileRB->SetState(kButtonUp);
    m_MinTimeEntry->SetEnabled(false);
    m_MaxTimeEntry->SetEnabled(false);
    m_GTIFile->SetEnabled(false);    
  }
  

  // Detectors
  TGLabel* LabelDetectors = 
    new TGLabel(GeneralFrame, new TGString("Select detectors in which the first interaction is *NOT* allowed to have happend:"));
  TGLayoutHints* LabelDetectorsLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, LeftGap, RightGap, 20, 0);
  GeneralFrame->AddFrame(LabelDetectors, LabelDetectorsLayout);

  m_DetectorList = new TGListBox(GeneralFrame, c_Detectors);
  m_DetectorList->SetMultipleSelections(true);
  TGLayoutHints* DetectorListLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, LeftGap, RightGap, 5, 0);
  GeneralFrame->AddFrame(m_DetectorList, DetectorListLayout);

  if (m_Geometry != 0) {
    for (unsigned int d = 0; d < m_Geometry->GetNDetectors(); ++d) {
      MString Name = m_Geometry->GetDetectorAt(d)->GetName() + " (" + 
        m_Geometry->GetDetectorAt(d)->GetTypeName() + ")";
      m_DetectorList->AddEntry(Name, d);

      // Highlight those detectors which are already in the list:
      for (unsigned int e = 0; e < m_Settings->GetNExcludedDetectors(); ++e) {
        if (m_Settings->GetExcludedDetectorAt(e) == m_Geometry->GetDetectorAt(d)->GetName()) {
          m_DetectorList->Select(d);
          break;
        }
      }
    }
  }
  m_DetectorList->Resize(m_FontScaler*200, m_FontScaler*100);


  
  
  // Energies Frame

  m_FirstTotalEnergy = new MGUIEMinMaxEntry(EnergiesFrame,
                                            MString("First energy window on total energy of incident gamma ray:"), 
                                            false,
                                            MString("Minimum energy [keV]: "),
                                            MString("Maximum energy [keV]: "),
                                            m_Settings->GetFirstEnergyRangeMin(), 
                                            m_Settings->GetFirstEnergyRangeMax(),
                                            true, 0.0);
  m_FirstTotalEnergy->SetEntryFieldSize(FieldSize);
  EnergiesFrame->AddFrame(m_FirstTotalEnergy, MinMaxFirstLayout);

  m_SecondTotalEnergy = new MGUIEMinMaxEntry(EnergiesFrame,
                                             MString("Second energy window on total energy of incident gamma ray:"), 
                                             false,
                                             MString("Minimum energy [keV]: "),
                                             MString("Maximum energy [keV]: "),
                                             m_Settings->GetSecondEnergyRangeMin(), 
                                             m_Settings->GetSecondEnergyRangeMax(), 
                                             true, 0.0);
  m_SecondTotalEnergy->SetEntryFieldSize(FieldSize);
  EnergiesFrame->AddFrame(m_SecondTotalEnergy, MinMaxLayout);

  m_ThirdTotalEnergy = new MGUIEMinMaxEntry(EnergiesFrame,
                                            MString("Third energy window on total energy of incident gamma ray:"), 
                                            false,
                                            MString("Minimum energy [keV]: "),
                                            MString("Maximum energy [keV]: "),
                                            m_Settings->GetThirdEnergyRangeMin(), 
                                            m_Settings->GetThirdEnergyRangeMax(), 
                                            true, 0.0);
  m_ThirdTotalEnergy->SetEntryFieldSize(FieldSize);
  EnergiesFrame->AddFrame(m_ThirdTotalEnergy, MinMaxLayout);

  m_FourthTotalEnergy = new MGUIEMinMaxEntry(EnergiesFrame,
                                             MString("Fourth energy window on total energy of incident gamma ray:"), 
                                             false,
                                             MString("Minimum energy [keV]: "),
                                             MString("Maximum energy [keV]: "),
                                             m_Settings->GetFourthEnergyRangeMin(), 
                                             m_Settings->GetFourthEnergyRangeMax(), 
                                             true, 0.0);
  m_FourthTotalEnergy->SetEntryFieldSize(FieldSize);
  EnergiesFrame->AddFrame(m_FourthTotalEnergy, MinMaxLayout);


  
  // Compton frame:

  m_ComptonAngle = new MGUIEMinMaxEntry(ComptonEAFrame,
                                        MString("Range of Compton scatter angles:"),
                                        false,
                                        MString("Minimum angle [deg]: "),
                                        MString("Maximum angle [deg]: "),
                                        m_Settings->GetComptonAngleRangeMin(), 
                                        m_Settings->GetComptonAngleRangeMax(), 
                                        true, 0.0, 180.0);
  m_ComptonAngle->SetEntryFieldSize(FieldSize);
  ComptonEAFrame->AddFrame(m_ComptonAngle, MinMaxFirstLayout);

  m_ThetaDeviationMax = 
    new MGUIEEntry(ComptonEAFrame,
                   MString("Maximum deviation of total scatter angles (energy vs. geo) [deg]:"),
                   false,
                   m_Settings->GetThetaDeviationMax(),
                   true, 0.0, 180.0);
  m_ThetaDeviationMax->SetEntryFieldSize(FieldSize);
  ComptonEAFrame->AddFrame(m_ThetaDeviationMax, SingleLayout);

  
  m_GammaEnergy = new MGUIEMinMaxEntry(ComptonEAFrame,
                                       MString("Energy range of scattered gamma ray:"),
                                       false,
                                       MString("Minimum energy [keV]: "),
                                       MString("Maximum energy [keV]: "),
                                       m_Settings->GetEnergyRangeGammaMin(), 
                                       m_Settings->GetEnergyRangeGammaMax(), 
                                       true, 0.0);
  m_GammaEnergy->SetEntryFieldSize(FieldSize);
  ComptonEAFrame->AddFrame(m_GammaEnergy, MinMaxFirstLayout);

  m_ElectronEnergy = new MGUIEMinMaxEntry(ComptonEAFrame,
                                          MString("Energy range of recoil electron:"),
                                          false,
                                          MString("Minimum energy [keV]: "),
                                          MString("Maximum energy [keV]: "),
                                          m_Settings->GetEnergyRangeElectronMin(), 
                                          m_Settings->GetEnergyRangeElectronMax(), 
                                          true, 0.0);
  m_ElectronEnergy->SetEntryFieldSize(FieldSize);
  ComptonEAFrame->AddFrame(m_ElectronEnergy, MinMaxLayout);

  
  

  // Compton 2 frame:

  m_FirstIADistance = new MGUIEMinMaxEntry(ComptonDLFrame,
                                           MString("Distance between the first interactions:"),
                                           false,
                                           MString("Minimum distance [cm]: "),
                                           MString("Maximum distance [cm]: "),
                                           m_Settings->GetFirstDistanceRangeMin(), 
                                           m_Settings->GetFirstDistanceRangeMax(), 
                                           true, 0.0);
  m_FirstIADistance->SetEntryFieldSize(FieldSize);
  ComptonDLFrame->AddFrame(m_FirstIADistance, MinMaxFirstLayout);

  m_IADistance = new MGUIEMinMaxEntry(ComptonDLFrame,
                                      MString("Min. distance between any two interactions:"),
                                       false,
                                      MString("Minimum distance [cm]: "),
                                      MString("Maximum distance [cm]: "),
                                      m_Settings->GetDistanceRangeMin(), 
                                      m_Settings->GetDistanceRangeMax(),
                                      true, 0.0);
  m_IADistance->SetEntryFieldSize(FieldSize);
  ComptonDLFrame->AddFrame(m_IADistance, MinMaxLayout);

  m_SequenceLength = new MGUIEMinMaxEntry(ComptonDLFrame,
                                          MString("Length of Compton Sequence:"),
                                          false,
                                          MString("Minimum: "),
                                          MString("Maximum: "),
                                          m_Settings->GetSequenceLengthRangeMin(), 
                                          m_Settings->GetSequenceLengthRangeMax(), 
                                          true, 1);
  m_SequenceLength->SetEntryFieldSize(FieldSize);
  ComptonDLFrame->AddFrame(m_SequenceLength, MinMaxLayout);

  m_TrackLength = new MGUIEMinMaxEntry(ComptonDLFrame,
                                       MString("Length of (first) electron track in layers:"),
                                       false,
                                       MString("Minimum: "),
                                       MString("Maximum: "),
                                       m_Settings->GetTrackLengthRangeMin(), 
                                       m_Settings->GetTrackLengthRangeMax(),
                                       true, 0);
  m_TrackLength->SetEntryFieldSize(FieldSize);
  ComptonDLFrame->AddFrame(m_TrackLength, MinMaxLayout);


  // Compton frame - quality factors:

  m_ComptonQualityFactor = new MGUIEMinMaxEntry(ComptonQFrame,
                                             MString("Compton quality factor:"),
                                             false,
                                             MString("Minimum: "),
                                             MString("Maximum: "),
                                             m_Settings->GetComptonQualityFactorRangeMin(), 
                                             m_Settings->GetComptonQualityFactorRangeMax());
  m_ComptonQualityFactor->SetEntryFieldSize(FieldSize);
  ComptonQFrame->AddFrame(m_ComptonQualityFactor, MinMaxFirstLayout);

  m_TrackQualityFactor = new MGUIEMinMaxEntry(ComptonQFrame,
                                             MString("Track quality factor:"),
                                             false,
                                             MString("Minimum: "),
                                             MString("Maximum: "),
                                             m_Settings->GetTrackQualityFactorRangeMin(), 
                                             m_Settings->GetTrackQualityFactorRangeMax(), 
                                             true, 0.0);
  m_TrackQualityFactor->SetEntryFieldSize(FieldSize);
  ComptonQFrame->AddFrame(m_TrackQualityFactor, MinMaxLayout);

  m_ClusteringQualityFactor = new MGUIEMinMaxEntry(ComptonQFrame,
                                                   MString("Clustering quality factor:"),
                                                   false,
                                                   MString("Minimum: "),
                                                   MString("Maximum: "),                                                     m_Settings->GetClusteringQualityFactorRangeMin(), 
                                                   m_Settings->GetClusteringQualityFactorRangeMax(), 
                                                   true, 0.0);
  m_ClusteringQualityFactor->SetEntryFieldSize(FieldSize);
  ComptonQFrame->AddFrame(m_ClusteringQualityFactor, MinMaxLayout);

  m_CoincidenceWindow = new MGUIEMinMaxEntry(ComptonQFrame,
                                             MString("Coincidence window:"),
                                             false,
                                             MString("Minimum (should be zero): "),
                                             MString("Maximum: "),
                                             m_Settings->GetCoincidenceWindowRangeMin(), 
                                             m_Settings->GetCoincidenceWindowRangeMax(), 
                                             true, 0.0);
  m_CoincidenceWindow->SetEntryFieldSize(FieldSize);
  ComptonQFrame->AddFrame(m_CoincidenceWindow, MinMaxLayout);




  // Compton frame - Earth horizon cut:

  TGLabel* LabelEHC = new TGLabel(ComptonEHCFrame, new TGString("Fixed Earth horizon cut (Earth in nadir):"));
  TGLayoutHints* LabelEHCLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, LeftGap, RightGap, TopGap, 0);
  ComptonEHCFrame->AddFrame(LabelEHC, LabelEHCLayout);

  TGLayoutHints* ComptonRBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);

  m_EHCRBNone = new TGRadioButton(ComptonEHCFrame, "None", c_EHCNone);
  ComptonEHCFrame->AddFrame(m_EHCRBNone, ComptonRBLayout);
  m_EHCRBNone->Associate(this);
  m_EHCRBNone->SetState((m_Settings->GetEHCType() == 0) ?  kButtonDown : kButtonUp);

  m_EHCRBIntersection = new TGRadioButton(ComptonEHCFrame, "Reject all events, whose full cone circle (ignoring possible arcs) intersects Earth", c_EHCIntersection);
  ComptonEHCFrame->AddFrame(m_EHCRBIntersection, ComptonRBLayout);
  m_EHCRBIntersection->Associate(this);
  m_EHCRBIntersection->SetState((m_Settings->GetEHCType() == 1) ?  kButtonDown : kButtonUp);

  m_EHCRBProbability = new TGRadioButton(ComptonEHCFrame, "Reject all events, whose probability of originating from Earth exceeds:", c_EHCProbability);
  ComptonEHCFrame->AddFrame(m_EHCRBProbability, ComptonRBLayout);
  m_EHCRBProbability->Associate(this);
  m_EHCRBProbability->SetState((m_Settings->GetEHCType() == 2) ?  kButtonDown : kButtonUp);

  TGLayoutHints* EHCSingleLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, LeftGap+30, RightGap, 5, 5);

  m_EHCProbability = 
    new MGUIEEntry(ComptonEHCFrame,
                   MString("Maximum Earth probability [0..1]:"),
                   false,
                   m_Settings->GetEHCProbability(),
                   true, 0.0, 1.0);
  m_EHCProbability->SetEntryFieldSize(FieldSize);
  ComptonEHCFrame->AddFrame(m_EHCProbability, EHCSingleLayout);

  m_EHCProbabilityFile = 
    new MGUIEFileSelector(ComptonEHCFrame,
                          MString("EHC Compton probability response file (*.compton.ehc.rsp):"),
                          m_Settings->GetEHCComptonProbabilityFileName());
  m_EHCProbabilityFile->SetFileType("EHC response file", "*.compton.ehc.rsp");
  ComptonEHCFrame->AddFrame(m_EHCProbabilityFile, EHCSingleLayout);

  m_EHCSelected = m_Settings->GetEHCType() + c_EHCNone;
  if (m_EHCSelected != c_EHCProbability) {
    m_EHCProbability->SetEnabled(false);
    m_EHCProbabilityFile->SetEnabled(false);
  }

  m_EHCAngle = 
    new MGUIEEntry(ComptonEHCFrame,
                   MString("Angle between direction to Earth's center and horizon (0 = nadir) [deg]:"),
                   false,
                   m_Settings->GetEHCAngle(),
                   true, 0.0, 180.0);
  m_EHCAngle->SetEntryFieldSize(FieldSize);
  ComptonEHCFrame->AddFrame(m_EHCAngle, SingleLayout);

  if (m_EHCSelected == c_EHCNone) {
    m_EHCAngle->SetEnabled(false);
  } else {
    m_EHCAngle->SetEnabled(true);
  }


  // Pair frame:

  m_OpeningAnglePair = new MGUIEMinMaxEntry(PairFrame,
                                            MString("Opening angle between electron and positron direction:"),
                                            false,
                                            MString("Minimum angle [deg]: "),
                                            MString("Maximum angle [deg]: "),
                                            m_Settings->GetOpeningAnglePairMin(), 
                                            m_Settings->GetOpeningAnglePairMax(),
                                            true, 0.0, 180.0);
  m_OpeningAnglePair->SetEntryFieldSize(FieldSize);
  PairFrame->AddFrame(m_OpeningAnglePair, MinMaxFirstLayout);

  m_InitialEnergyDepositPair = new MGUIEMinMaxEntry(PairFrame,
                                                    MString("Energy deposit in the first layer of interaction:"),
                                                    false,
                                                    MString("Minimum [keV]: "),
                                                    MString("Maximum [keV]: "),
                                                    m_Settings->GetInitialEnergyDepositPairMin(), 
                                                    m_Settings->GetInitialEnergyDepositPairMax(),
                                                    true, 0.0);
  m_InitialEnergyDepositPair->SetEntryFieldSize(FieldSize);
  PairFrame->AddFrame(m_InitialEnergyDepositPair, MinMaxLayout);



  // Source frame:
  
  
  m_UsePointSource = new TGCheckButton(SourceFrame, "Use selection on point source:", c_UsePointSource);
  SourceFrame->AddFrame(m_UsePointSource, MinMaxFirstLayout);
  m_UsePointSource->Associate(this);
  m_UsePointSource->SetState((m_Settings->GetSourceUsePointSource() == true) ?  kButtonDown : kButtonUp);

  TGLayoutHints* CoordinatesLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, LeftGap+20, RightGap, 10, 0);

  m_UseSphericPointSource = new TGRadioButton(SourceFrame, "Point source in spheric coordinates:", c_UseSphericPointSource);
  SourceFrame->AddFrame(m_UseSphericPointSource, CoordinatesLayout);
  m_UseSphericPointSource->Associate(this);

  TGLayoutHints* SourceLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, LeftGap+40, RightGap, 5, 0);

  m_SourceSpheric = new MGUIEEntryList(SourceFrame, "Location (Theta, Phi) [deg]", MGUIEEntryList::c_SingleLine);
  m_SourceSpheric->Add("", m_Settings->GetSourceTheta(), kTRUE, 0.0, 180.0);
  m_SourceSpheric->Add("", m_Settings->GetSourcePhi(), kTRUE, 0.0, 360.0);
  m_SourceSpheric->SetEntryFieldSize(FieldSize);
  m_SourceSpheric->Create();
  SourceFrame->AddFrame(m_SourceSpheric, SourceLayout);

  m_UseGalacticPointSource = new TGRadioButton(SourceFrame, "Point source in Galactic coordinates:", c_UseGalacticPointSource);
  SourceFrame->AddFrame(m_UseGalacticPointSource, CoordinatesLayout);
  m_UseGalacticPointSource->Associate(this);

  m_SourceGalactic = new MGUIEEntryList(SourceFrame, "Location (Long, Lat) [deg]", MGUIEEntryList::c_SingleLine);
  m_SourceGalactic->Add("", m_Settings->GetSourceLongitude(), kTRUE, 0.0, 360.0);
  m_SourceGalactic->Add("", m_Settings->GetSourceLatitude(), kTRUE, -90.0, 90.0);
  m_SourceGalactic->SetEntryFieldSize(FieldSize);
  m_SourceGalactic->Create();
  SourceFrame->AddFrame(m_SourceGalactic, SourceLayout);

  m_UseCartesianPointSource = new TGRadioButton(SourceFrame, "Point source in Cartesian coordinates:", c_UseCartesianPointSource);
  TGLayoutHints* UseCartesianPointSourceLayout = new TGLayoutHints(kLHintsLeft, LeftGap+20, RightGap, 10, 0);
  SourceFrame->AddFrame(m_UseCartesianPointSource, UseCartesianPointSourceLayout);
  m_UseCartesianPointSource->Associate(this);

  m_SourceCartesian = new MGUIEEntryList(SourceFrame, "Location (x, y, z) [cm]", MGUIEEntryList::c_SingleLine);
  m_SourceCartesian->Add("", m_Settings->GetSourceX(), true);
  m_SourceCartesian->Add("", m_Settings->GetSourceY(), true);
  m_SourceCartesian->Add("", m_Settings->GetSourceZ(), true);
  m_SourceCartesian->SetEntryFieldSize(FieldSize);
  m_SourceCartesian->Create();
  SourceFrame->AddFrame(m_SourceCartesian, SourceLayout);


  m_ARM = new MGUIEMinMaxEntry(SourceFrame,
                               MString("Radial window (= ARM for Compton):"),
                               false,
                               MString("Minimum [deg]: "),
                               MString("Maximum [deg]: "),
                               m_Settings->GetSourceARMMin(), 
                               m_Settings->GetSourceARMMax(), 
                               true, 0.0, 180.0);
  m_ARM->SetEntryFieldSize(FieldSize);
  SourceFrame->AddFrame(m_ARM, CoordinatesLayout);

  m_SPD = new MGUIEMinMaxEntry(SourceFrame,
                               MString("Transversal window (= SPD for tracked Comptons):"),
                               false,
                               MString("Minimum [deg]: "),
                               MString("Maximum [deg]: "),
                               m_Settings->GetSourceSPDMin(), 
                               m_Settings->GetSourceSPDMax(), 
                               true, 0.0, 180.0);
  m_SPD->SetEntryFieldSize(FieldSize);
  SourceFrame->AddFrame(m_SPD, CoordinatesLayout);

  if (m_Settings->GetSourceUsePointSource() == true) {
    if (m_Settings->GetSourceCoordinates() == MCoordinateSystem::c_Galactic) {
      m_CoordinatesSelected = c_UseGalacticPointSource;
    } else if (m_Settings->GetSourceCoordinates() == MCoordinateSystem::c_Spheric) {
      m_CoordinatesSelected = c_UseSphericPointSource;
    } else {
      m_CoordinatesSelected = c_UseCartesianPointSource;
    }           
    m_ARM->SetEnabled(true);
    m_SPD->SetEnabled(true);
    if (m_CoordinatesSelected == c_UseGalacticPointSource) {
      m_UseGalacticPointSource->SetState(kButtonDown);
      m_UseSphericPointSource->SetState(kButtonUp);
      m_UseCartesianPointSource->SetState(kButtonUp);
      m_SourceGalactic->SetEnabled(true);
      m_SourceSpheric->SetEnabled(false);
      m_SourceCartesian->SetEnabled(false);
    } else if (m_CoordinatesSelected == c_UseSphericPointSource) {
      m_UseGalacticPointSource->SetState(kButtonUp);
      m_UseSphericPointSource->SetState(kButtonDown);
      m_UseCartesianPointSource->SetState(kButtonUp);
      m_SourceGalactic->SetEnabled(false);
      m_SourceSpheric->SetEnabled(true);
      m_SourceCartesian->SetEnabled(false);
    } else {
      m_UseGalacticPointSource->SetState(kButtonUp);
      m_UseSphericPointSource->SetState(kButtonUp);
      m_UseCartesianPointSource->SetState(kButtonDown);
      m_SourceGalactic->SetEnabled(false);
      m_SourceSpheric->SetEnabled(false);
      m_SourceCartesian->SetEnabled(true);
    }
  } else {
    if (m_Settings->GetSourceCoordinates() == MCoordinateSystem::c_Galactic) {
      m_CoordinatesSelected = c_UseGalacticPointSource;
    } else if (m_Settings->GetSourceCoordinates() == MCoordinateSystem::c_Spheric) {
      m_CoordinatesSelected = c_UseSphericPointSource;
    } else {
      m_CoordinatesSelected = c_UseCartesianPointSource;
    }           
    m_UseGalacticPointSource->SetState(kButtonUp);
    m_SourceGalactic->SetEnabled(false);
    m_UseSphericPointSource->SetState(kButtonUp);
    m_SourceSpheric->SetEnabled(false);
    m_UseCartesianPointSource->SetState(kButtonUp);
    m_SourceCartesian->SetEnabled(false);
    m_ARM->SetEnabled(false);
    m_SPD->SetEnabled(false);
  }

  
  // Pointing frame
  
  TGLabel* PointingSelectionLabel = new TGLabel(PointingFrame, "Use only pointings which are within the following selection:");
  TGLayoutHints* PointingSelectionLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, LeftGap, RightGap, TopGap, 5);
  PointingFrame->AddFrame(PointingSelectionLabel, PointingSelectionLabelLayout);
  
  m_UsePointingSelectionNone = new TGRadioButton(PointingFrame, "Do NOT use a pointing selection", c_UsePointingSelectionNone);
  m_UsePointingSelectionNone->Associate(this);
  PointingFrame->AddFrame(m_UsePointingSelectionNone, CoordinatesLayout);

  m_UsePointingSelectionPointSource = new TGRadioButton(PointingFrame, "Disk around these coordinates:", c_UsePointingSelectionPointSource);
  m_UsePointingSelectionPointSource->Associate(this);
  PointingFrame->AddFrame(m_UsePointingSelectionPointSource, CoordinatesLayout);

  m_PointingPointSourceLocation = new MGUIEEntryList(PointingFrame, "Center of disk (Latitude, Longitude) [deg]", MGUIEEntryList::c_SingleLine);
  m_PointingPointSourceLocation->Add("", m_Settings->GetPointingPointSourceLatitude(), kTRUE, -90.0, 90.0);
  m_PointingPointSourceLocation->Add("", m_Settings->GetPointingPointSourceLongitude(), kTRUE, -180.0, 360.0);
  m_PointingPointSourceLocation->SetEntryFieldSize(FieldSize);
  m_PointingPointSourceLocation->Create();
  PointingFrame->AddFrame(m_PointingPointSourceLocation, SourceLayout);

  m_PointingPointSourceRadius = 
    new MGUIEEntry(PointingFrame,
                   MString("Radius of disk [deg]:"),
                   false,
                   m_Settings->GetPointingPointSourceRadius(),
                   true, 0.0, 180.0);
  m_PointingPointSourceRadius->SetEntryFieldSize(FieldSize);
  PointingFrame->AddFrame(m_PointingPointSourceRadius, SourceLayout);

  m_UsePointingSelectionBox = new TGRadioButton(PointingFrame, "Box with this center and extend:", c_UsePointingSelectionBox);
  m_UsePointingSelectionBox->Associate(this);
  PointingFrame->AddFrame(m_UsePointingSelectionBox, CoordinatesLayout);

  m_PointingBoxLocation = new MGUIEEntryList(PointingFrame, "Center of box (Latitude, Longitude) [deg]", MGUIEEntryList::c_SingleLine);
  m_PointingBoxLocation->Add("", m_Settings->GetPointingBoxLatitude(), kTRUE, -90.0, 90.0);
  m_PointingBoxLocation->Add("", m_Settings->GetPointingBoxLongitude(), kTRUE, -180.0, 360.0);
  m_PointingBoxLocation->SetEntryFieldSize(FieldSize);
  m_PointingBoxLocation->Create();
  PointingFrame->AddFrame(m_PointingBoxLocation, SourceLayout);

  m_PointingBoxExtentLatitude = 
    new MGUIEEntry(PointingFrame,
                   MString("Extent in latitude (half) [deg]:"),
                   false,
                   m_Settings->GetPointingBoxExtentLatitude(),
                   true, 0.0, 180.0);
  m_PointingBoxExtentLatitude->SetEntryFieldSize(FieldSize);
  PointingFrame->AddFrame(m_PointingBoxExtentLatitude, SourceLayout);

  m_PointingBoxExtentLongitude = 
    new MGUIEEntry(PointingFrame,
                   MString("Extent in longitude (half) [deg]:"),
                   false,
                   m_Settings->GetPointingBoxExtentLongitude(),
                   true, 0.0, 180.0);
  m_PointingBoxExtentLongitude->SetEntryFieldSize(FieldSize);
  PointingFrame->AddFrame(m_PointingBoxExtentLongitude, SourceLayout);
  
  if (m_Settings->GetPointingSelectionType() == 1) {
    m_UsePointingSelectionNone->SetState(kButtonUp);
    m_UsePointingSelectionPointSource->SetState(kButtonDown);
    m_UsePointingSelectionBox->SetState(kButtonUp);
    m_PointingPointSourceLocation->SetEnabled(true);
    m_PointingPointSourceRadius->SetEnabled(true);
    m_PointingBoxLocation->SetEnabled(false);
    m_PointingBoxExtentLatitude->SetEnabled(false);
    m_PointingBoxExtentLongitude->SetEnabled(false);
  } else if (m_Settings->GetPointingSelectionType() == 2) {
    m_UsePointingSelectionNone->SetState(kButtonUp);
    m_UsePointingSelectionPointSource->SetState(kButtonUp);
    m_UsePointingSelectionBox->SetState(kButtonDown);
    m_PointingPointSourceLocation->SetEnabled(false);
    m_PointingPointSourceRadius->SetEnabled(false);
    m_PointingBoxLocation->SetEnabled(true);
    m_PointingBoxExtentLatitude->SetEnabled(true);
    m_PointingBoxExtentLongitude->SetEnabled(true);
  } else {
    m_UsePointingSelectionNone->SetState(kButtonDown);
    m_UsePointingSelectionPointSource->SetState(kButtonUp);
    m_UsePointingSelectionBox->SetState(kButtonUp);
    m_PointingPointSourceLocation->SetEnabled(false);
    m_PointingPointSourceRadius->SetEnabled(false);
    m_PointingBoxLocation->SetEnabled(false);
    m_PointingBoxExtentLatitude->SetEnabled(false);
    m_PointingBoxExtentLongitude->SetEnabled(false);
  }
  

  // Beam frame
  
  
  m_UseBeam = new TGCheckButton(BeamFrame, "Use beam selection:", c_UseBeam);
  BeamFrame->AddFrame(m_UseBeam, MinMaxFirstLayout);
  m_UseBeam->Associate(this);
  m_UseBeam->SetState((m_Settings->GetBeamUse() == true) ?  kButtonDown : kButtonUp);

  TGLayoutHints* BeamLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, LeftGap+20, RightGap, 10, 0);

  m_BeamStart = new MGUIEEntryList(BeamFrame, "Start position (x, y, z) [cm]", MGUIEEntryList::c_SingleLine);
  m_BeamStart->Add("", m_Settings->GetBeamStartX());
  m_BeamStart->Add("", m_Settings->GetBeamStartY());
  m_BeamStart->Add("", m_Settings->GetBeamStartZ());
  m_BeamStart->SetEntryFieldSize(FieldSize);
  m_BeamStart->Create();
  BeamFrame->AddFrame(m_BeamStart, BeamLayout);

  m_BeamFocalSpot = new MGUIEEntryList(BeamFrame, "Focal spot on detector (x, y, z) [cm]", MGUIEEntryList::c_SingleLine);
  m_BeamFocalSpot->Add("", m_Settings->GetBeamFocalSpotX());
  m_BeamFocalSpot->Add("", m_Settings->GetBeamFocalSpotY());
  m_BeamFocalSpot->Add("", m_Settings->GetBeamFocalSpotZ());
  m_BeamFocalSpot->SetEntryFieldSize(FieldSize);
  m_BeamFocalSpot->Create();
  BeamFrame->AddFrame(m_BeamFocalSpot, BeamLayout);

  m_BeamRadius = 
    new MGUIEEntry(BeamFrame,
                   "Selected radius around beam center [cm]:",
                   false,
                   m_Settings->GetBeamRadius(),
                   true, 0.0);
  m_BeamRadius->SetEntryFieldSize(FieldSize);
  BeamFrame->AddFrame(m_BeamRadius, BeamLayout);

  m_BeamDepth = 
    new MGUIEEntry(BeamFrame,
                   "Selected maximum (fist) interaction depth [cm]:",
                   false,
                   m_Settings->GetBeamDepth(),
                   true, 0.0);
  m_BeamDepth->SetEntryFieldSize(FieldSize);
  BeamFrame->AddFrame(m_BeamDepth, BeamLayout);

  MString Description;
  Description += "This beam selection ONLY determines the volume within which the first interaction of a\nsequence must have occurred for an event to possibly be valid. This volume is a cylinder, with\na starting point \"focal spot\", a depth \"maximum first IA depth\", and radius. If the \"start position\"\ndiffers from the focal spot only in z, the cylinder's axis is parallel to z; if the start position's x\nand/or y differ as well, the cylinder is rotated around the x or y axes to lie parallel to the\nstart-focal spot line.";

  MGUIEText* BeamDescription = new MGUIEText(BeamFrame, Description, MGUIEText::c_Left);
  BeamFrame->AddFrame(BeamDescription, BeamLayout);


  if (m_UseBeam->GetState() == kButtonUp) {
    m_BeamStart->SetEnabled(false);
    m_BeamFocalSpot->SetEnabled(false);
    m_BeamRadius->SetEnabled(false);
    m_BeamDepth->SetEnabled(false);
  } else {
    m_BeamStart->SetEnabled(true);
    m_BeamFocalSpot->SetEnabled(true);
    m_BeamRadius->SetEnabled(true);
    m_BeamDepth->SetEnabled(true);
  }



  // Special frame:
  if (m_Settings->GetSpecialMode() == true) {
    m_TimeWalk = new MGUIEMinMaxEntry(SpecialFrame,
                                      MString("Time walk between D1 and D2 trigger (Only relevant for Mega):"),
                                      false,
                                      MString("Minimum [ns]: "),
                                      MString("Maximum [ns]: "),
                                      m_Settings->GetTimeWalkRangeMin(), 
                                      m_Settings->GetTimeWalkRangeMax());
    SpecialFrame->AddFrame(m_TimeWalk, MinMaxFirstLayout);
  }
  

  // Finally bring it to the screen
  FinalizeCreate(GetDefaultWidth(), GetDefaultHeight()+40, false);

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEventSelection::ProcessMessage(long Message, long Parameter1, 
                                        long Parameter2)
{
  // Process the messages for this application
  //cout<<"Messages: "<<Message<<", "<<Parameter1<<", "<<Parameter2<<endl;

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_RADIOBUTTON:
      if (Parameter1 == c_GTIAll) {
        if (m_GTIAllRB->GetState() == kButtonUp) {
          m_GTIAllRB->SetState(kButtonDown); 
        } else {
          m_GTIEntryRB->SetState(kButtonUp);
          m_GTIFileRB->SetState(kButtonUp);
          m_MinTimeEntry->SetEnabled(false);
          m_MaxTimeEntry->SetEnabled(false);
          m_GTIFile->SetEnabled(false);
        }
      } else if (Parameter1 == c_GTIEntry) {
        if (m_GTIEntryRB->GetState() == kButtonUp) {
          m_GTIEntryRB->SetState(kButtonDown); 
        } else {
          m_GTIAllRB->SetState(kButtonUp);
          m_GTIFileRB->SetState(kButtonUp);
          m_MinTimeEntry->SetEnabled(true);
          m_MaxTimeEntry->SetEnabled(true);
          m_GTIFile->SetEnabled(false);
        }
      } else if (Parameter1 == c_GTIFile) {
        if (m_GTIFileRB->GetState() == kButtonUp) {
          m_GTIFileRB->SetState(kButtonDown); 
        } else {
          m_GTIAllRB->SetState(kButtonUp);
          m_GTIEntryRB->SetState(kButtonUp);
          m_MinTimeEntry->SetEnabled(false);
          m_MaxTimeEntry->SetEnabled(false);
          m_GTIFile->SetEnabled(true);
        }
      } else if (Parameter1 >= c_EHCNone && Parameter1 <= c_EHCProbability) {
        m_EHCSelected = Parameter1;
        if (c_EHCNone == Parameter1) {
          m_EHCRBNone->SetState(kButtonDown);
        } else {
          m_EHCRBNone->SetState(kButtonUp);
        }
        if (c_EHCIntersection == Parameter1) {
          m_EHCRBIntersection->SetState(kButtonDown);
        } else {
          m_EHCRBIntersection->SetState(kButtonUp);
        }
        if (c_EHCProbability == Parameter1) {
          m_EHCRBProbability->SetState(kButtonDown);
        } else {
          m_EHCRBProbability->SetState(kButtonUp);
        }

        if (m_EHCSelected == c_EHCNone) {
          m_EHCAngle->SetEnabled(false);
        } else {
          m_EHCAngle->SetEnabled(true);
        }
        if (m_EHCSelected == c_EHCProbability) {
          m_EHCProbability->SetEnabled(true);
          m_EHCProbabilityFile->SetEnabled(true);
        } else {
          m_EHCProbability->SetEnabled(false);
          m_EHCProbabilityFile->SetEnabled(false);
        }
        
      } else if (Parameter1 == c_UsePointingSelectionNone) {
        if (m_UsePointingSelectionNone->GetState() == kButtonUp) {
          m_UsePointingSelectionNone->SetState(kButtonDown); 
        } else {
          m_UsePointingSelectionPointSource->SetState(kButtonUp); 
          m_UsePointingSelectionBox->SetState(kButtonUp); 
          m_PointingPointSourceLocation->SetEnabled(false);
          m_PointingPointSourceRadius->SetEnabled(false);
          m_PointingBoxLocation->SetEnabled(false);
          m_PointingBoxExtentLatitude->SetEnabled(false);
          m_PointingBoxExtentLongitude->SetEnabled(false);
        }
        
      } else if (Parameter1 == c_UsePointingSelectionPointSource) {
        if (m_UsePointingSelectionPointSource->GetState() == kButtonUp) {
          m_UsePointingSelectionPointSource->SetState(kButtonDown); 
        } else {
          m_UsePointingSelectionNone->SetState(kButtonUp); 
          m_UsePointingSelectionBox->SetState(kButtonUp); 
          m_PointingPointSourceLocation->SetEnabled(true);
          m_PointingPointSourceRadius->SetEnabled(true);
          m_PointingBoxLocation->SetEnabled(false);
          m_PointingBoxExtentLatitude->SetEnabled(false);
          m_PointingBoxExtentLongitude->SetEnabled(false);
        }
        
      } else if (Parameter1 == c_UsePointingSelectionBox) {
        if (m_UsePointingSelectionBox->GetState() == kButtonUp) {
          m_UsePointingSelectionBox->SetState(kButtonDown); 
        } else {
          m_UsePointingSelectionNone->SetState(kButtonUp); 
          m_UsePointingSelectionPointSource->SetState(kButtonUp); 
          m_PointingPointSourceLocation->SetEnabled(false);
          m_PointingPointSourceRadius->SetEnabled(false);
          m_PointingBoxLocation->SetEnabled(true);
          m_PointingBoxExtentLatitude->SetEnabled(true);
          m_PointingBoxExtentLongitude->SetEnabled(true);
        }
        
      } else if (Parameter1 >= c_UseGalacticPointSource && 
                 Parameter1 <= c_UseCartesianPointSource) {
        if (m_UsePointSource->GetState() == kButtonUp) {
          m_UsePointSource->SetState(kButtonUp);
          m_UseSphericPointSource->SetState(kButtonUp);
          m_UseCartesianPointSource->SetState(kButtonUp);
        } else {
          m_CoordinatesSelected = Parameter1;
          if (c_UseGalacticPointSource == Parameter1) {
            m_UseGalacticPointSource->SetState(kButtonDown);
            m_SourceGalactic->SetEnabled(true);
          } else {
            m_UseGalacticPointSource->SetState(kButtonUp);
            m_SourceGalactic->SetEnabled(false);
          }     
          if (c_UseSphericPointSource == Parameter1) {
            m_UseSphericPointSource->SetState(kButtonDown);
            m_SourceSpheric->SetEnabled(true);
          } else {
            m_UseSphericPointSource->SetState(kButtonUp);
            m_SourceSpheric->SetEnabled(false);
          }     
          if (c_UseCartesianPointSource == Parameter1) {
            m_UseCartesianPointSource->SetState(kButtonDown);
            m_SourceCartesian->SetEnabled(true);
          } else {
            m_UseCartesianPointSource->SetState(kButtonUp);
            m_SourceCartesian->SetEnabled(false);
          }     
        }
      }
      break;
    case kCM_CHECKBUTTON:
      switch (Parameter1) {
      case c_Compton:
        if (m_ComptonCB->GetState() == kButtonUp) {
          (m_ComptonNotTrackedCB->GetState() == kButtonDown) ? m_UseNotTracked = 1 : m_UseNotTracked = 0;
          m_ComptonNotTrackedCB->SetState(kButtonDisabled);

          (m_ComptonTrackedCB->GetState() == kButtonDown) ? m_UseTracked = 1 : m_UseTracked = 0;
          m_ComptonTrackedCB->SetState(kButtonDisabled);
        } else {
          if (m_UseNotTracked == 1) {
            m_ComptonNotTrackedCB->SetState(kButtonDown);
          } else {
            m_ComptonNotTrackedCB->SetState(kButtonUp);
          }
          if (m_UseTracked == 1) {
            m_ComptonTrackedCB->SetState(kButtonDown);
          } else {
            m_ComptonTrackedCB->SetState(kButtonUp);
          }
        } 
        break;

      case c_ComptonTracked:
        if (m_ComptonTrackedCB->GetState() == kButtonDown) {
          m_UseTracked = 1;
        } else if (m_ComptonTrackedCB->GetState() == kButtonUp) {
          m_UseTracked = 0;
        }
        break;

      case c_ComptonUntracked:
        if (m_ComptonNotTrackedCB->GetState() == kButtonDown) {
          m_UseNotTracked = 1;
        } else if (m_ComptonNotTrackedCB->GetState() == kButtonUp) {
          m_UseNotTracked = 0;
        }
        break;

       case c_UsePointSource:
         if (m_UsePointSource->GetState() == kButtonUp) {
           m_UseGalacticPointSource->SetState(kButtonUp);
           m_SourceGalactic->SetEnabled(false);
           m_UseSphericPointSource->SetState(kButtonUp);
           m_SourceSpheric->SetEnabled(false);
           m_UseCartesianPointSource->SetState(kButtonUp);
           m_SourceCartesian->SetEnabled(false);
           m_ARM->SetEnabled(false);
           m_SPD->SetEnabled(false);
         } else {
           m_ARM->SetEnabled(true);
           m_SPD->SetEnabled(true);
           if (m_CoordinatesSelected == c_UseSphericPointSource) {
             m_UseGalacticPointSource->SetState(kButtonDown);
             m_UseSphericPointSource->SetState(kButtonUp);
             m_UseCartesianPointSource->SetState(kButtonUp);
             m_SourceGalactic->SetEnabled(true);
             m_SourceSpheric->SetEnabled(false);
             m_SourceCartesian->SetEnabled(false);
           } else if (m_CoordinatesSelected == c_UseSphericPointSource) {
             m_UseGalacticPointSource->SetState(kButtonUp);
             m_UseSphericPointSource->SetState(kButtonDown);
             m_UseCartesianPointSource->SetState(kButtonUp);
             m_SourceGalactic->SetEnabled(false);
             m_SourceSpheric->SetEnabled(true);
             m_SourceCartesian->SetEnabled(false);
           } else {
             m_UseGalacticPointSource->SetState(kButtonUp);
             m_UseSphericPointSource->SetState(kButtonUp);
             m_UseCartesianPointSource->SetState(kButtonDown);
             m_SourceGalactic->SetEnabled(false);
             m_SourceSpheric->SetEnabled(false);
             m_SourceCartesian->SetEnabled(true);
           }
         }
         break;

      case c_UseBeam:
        if (m_UseBeam->GetState() == kButtonUp) {
          m_BeamStart->SetEnabled(false);
          m_BeamFocalSpot->SetEnabled(false);
          m_BeamRadius->SetEnabled(false);
          m_BeamDepth->SetEnabled(false);
        } else {
          m_BeamStart->SetEnabled(true);
          m_BeamFocalSpot->SetEnabled(true);
          m_BeamRadius->SetEnabled(true);
          m_BeamDepth->SetEnabled(true);
        }
        break;

     default:
        break; 
      }
      break;

    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
        Status = OnOk();
        break;

      case e_Cancel:
        Status = OnCancel();
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


bool MGUIEventSelection::OnApply()
{

  // Check the data:
  if (m_ComptonCB->GetState() == kButtonUp && 
      m_PairCB->GetState()  == kButtonUp && 
      m_PhotoCB->GetState() == kButtonUp && 
      m_PETCB->GetState() == kButtonUp && 
      m_MultiCB->GetState() == kButtonUp && 
      m_UnidentifiableCB->GetState() == kButtonUp) {
    new TGMsgBox(gClient->GetRoot(), this, "Type error", 
                 "You have to choose at least one of these event-types:"
                 " Compton / pair / photo / PET / multi / unidentifiable", kMBIconStop, kMBOk);
    return false;
  }
  if (m_ComptonCB->GetState() == kButtonDown && 
      m_ComptonNotTrackedCB->GetState() == kButtonUp &&
      m_ComptonTrackedCB->GetState() == kButtonUp) {
    new TGMsgBox(gClient->GetRoot(), this, "Type error", 
                       "You have to selected Compton-events. So you have also to select: \n"
                 "Tracked and/or not-tracked Compton-events", kMBIconStop, kMBOk);
    return false;
  }
  if (m_EventId->CheckRange(0l, numeric_limits<long>::max(), 
                            0l, numeric_limits<long>::max(), false) == false) return false;
  if (m_TrackLength->CheckRange(1, numeric_limits<int>::max(), 
                                1, numeric_limits<int>::max(), false) == false) return false;
  if (m_SequenceLength->CheckRange(2, numeric_limits<int>::max(), 
                                   2, numeric_limits<int>::max(), false) == false) return false;
  if (m_ClusteringQualityFactor->CheckRange(0.0, numeric_limits<double>::max(), 
                                            0.0, numeric_limits<double>::max(), true) == false) return false;
  //if (m_ComptonQualityFactor->CheckRange(0.0, numeric_limits<double>::max(), 
  //                                       0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_TrackQualityFactor->CheckRange(0.0, numeric_limits<double>::max(), 
                                       0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_CoincidenceWindow->CheckRange(0.0, numeric_limits<double>::max(), 
                                      0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_FirstTotalEnergy->CheckRange(0.0, numeric_limits<double>::max(), 
                                     0.0, numeric_limits<double>::max(), false) == false) return false;
  if (m_SecondTotalEnergy->CheckRange(0.0, numeric_limits<double>::max(), 
                                      0.0, numeric_limits<double>::max(), false) == false) return false;
  if (m_ThirdTotalEnergy->CheckRange(0.0, numeric_limits<double>::max(), 
                                      0.0, numeric_limits<double>::max(), false) == false) return false;
  if (m_FourthTotalEnergy->CheckRange(0.0, numeric_limits<double>::max(), 
                                      0.0, numeric_limits<double>::max(), false) == false) return false;
  if (m_GammaEnergy->CheckRange(0.0, numeric_limits<double>::max(), 
                                      0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_ElectronEnergy->CheckRange(0.0, numeric_limits<double>::max(), 
                                   0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_ComptonAngle->CheckRange(0.0, 180.0, 0.0, 180.0, true) == false) return false;
  if (m_FirstIADistance->CheckRange(0.0, numeric_limits<double>::max(), 
                                    0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_IADistance->CheckRange(0.0, numeric_limits<double>::max(), 
                               0.0, numeric_limits<double>::max(), true) == false) return false;
  double MinTime = m_MinTimeEntry->GetAsString().ToDouble();
  double MaxTime = m_MaxTimeEntry->GetAsString().ToDouble();
  if (MinTime >= MaxTime) {
    mgui<<"The minimum time is larger or equal the maximum time"<<error;     
    return false;
  }
  if (MinTime < 0) {
    mgui<<"The minimum time is smaller than zero!"<<error;     
    return false;    
  }
  if (m_Settings->GetSpecialMode() == true) {
    if (m_TimeWalk->CheckRange(-numeric_limits<double>::max(), numeric_limits<double>::max(), 
                               -numeric_limits<double>::max(), numeric_limits<double>::max(), true) == false) return false;
  }
  if (m_OpeningAnglePair->CheckRange(0.0, 180.0, 0.0, 180.0, true) == false) return false;
  if (m_InitialEnergyDepositPair->CheckRange(0.0, 100000.0, 0.0, 100000.0, true) == false) return false;
  
  
  // Forgot why this has to be the case - or if it is still valid...
  /*
  if ((m_FirstTotalEnergy->GetMinValue() < m_SecondTotalEnergy->GetMaxValue() && 
       m_FirstTotalEnergy->GetMaxValue() > m_SecondTotalEnergy->GetMinValue()) || 
      (m_SecondTotalEnergy->GetMinValue() < m_FirstTotalEnergy->GetMaxValue() && 
       m_SecondTotalEnergy->GetMaxValue() > m_FirstTotalEnergy->GetMinValue()) ||
      (m_ThirdTotalEnergy->GetMinValue() < m_ThirdTotalEnergy->GetMaxValue() && 
       m_ThirdTotalEnergy->GetMaxValue() > m_ThirdTotalEnergy->GetMinValue()) ||
      (m_FourthTotalEnergy->GetMinValue() < m_FourthTotalEnergy->GetMaxValue() && 
       m_FourthTotalEnergy->GetMaxValue() > m_FourthTotalEnergy->GetMinValue())) {
    new TGMsgBox(gClient->GetRoot(), this, "Type error", 
                 "Overlapping energy bands are not allowed!", kMBIconStop, kMBOk);
    return false;
  }
  */
  if (m_FirstTotalEnergy->GetMaxValue() == 0 && 
      m_SecondTotalEnergy->GetMaxValue() == 0 && 
      m_ThirdTotalEnergy->GetMaxValue() == 0 && 
      m_FourthTotalEnergy->GetMaxValue() == 0) {
    mgui<<"At least one total energy window must be open."<<error; 
    return false;
  }


  // And save it:
  if (m_Settings->GetEventTypeCompton() != ((m_ComptonCB->GetState() == kButtonDown) ? 1 : 0)) m_Settings->SetEventTypeCompton((m_ComptonCB->GetState() == kButtonDown) ? 1 : 0);
  if (m_Settings->GetEventTypeComptonNotTracked() != m_UseNotTracked) m_Settings->SetEventTypeComptonNotTracked(m_UseNotTracked);
  if (m_Settings->GetEventTypeComptonTracked() != m_UseTracked) m_Settings->SetEventTypeComptonTracked(m_UseTracked);
  if (m_Settings->GetEventTypePair() != ((m_PairCB->GetState() == kButtonDown) ? 1 : 0)) m_Settings->SetEventTypePair((m_PairCB->GetState() == kButtonDown) ? 1 : 0);
  if (m_Settings->GetEventTypePhoto() != ((m_PhotoCB->GetState() == kButtonDown) ? 1 : 0)) m_Settings->SetEventTypePhoto((m_PhotoCB->GetState() == kButtonDown) ? 1 : 0); 
  if (m_Settings->GetEventTypePET() != ((m_PETCB->GetState() == kButtonDown) ? 1 : 0)) m_Settings->SetEventTypePET((m_PETCB->GetState() == kButtonDown) ? 1 : 0); 
  if (m_Settings->GetEventTypeMulti() != ((m_MultiCB->GetState() == kButtonDown) ? 1 : 0)) m_Settings->SetEventTypeMulti((m_MultiCB->GetState() == kButtonDown) ? 1 : 0); 
  if (m_Settings->GetEventTypeUnidentifiable() != ((m_UnidentifiableCB->GetState() == kButtonDown) ? 1 : 0)) m_Settings->SetEventTypeUnidentifiable((m_UnidentifiableCB->GetState() == kButtonDown) ? 1 : 0);
  if (m_Settings->GetEventTypeDecay() != ((m_DecayCB->GetState() == kButtonDown) ? 1 : 0)) m_Settings->SetEventTypeDecay((m_DecayCB->GetState() == kButtonDown) ? 1 : 0);

  if (m_Settings->GetFlaggedAsBad() != ((m_BadCB->GetState() == kButtonDown) ? true : false)) m_Settings->SetFlaggedAsBad((m_BadCB->GetState() == kButtonDown) ? true : false);  

  if (m_EventId->IsModified() == true) {
    m_Settings->SetEventIdRangeMin(long(m_EventId->GetMinValueInt()));
    m_Settings->SetEventIdRangeMax(long(m_EventId->GetMaxValueInt()));
  }


  if (m_GTIEntryRB->GetState() == kButtonDown) {
    if (m_Settings->GetTimeMode() != 1) {
      m_Settings->SetTimeMode(1);
    }
  } else if (m_GTIFileRB->GetState() == kButtonDown) {
    if (m_Settings->GetTimeMode() != 2) {
      m_Settings->SetTimeMode(2);
    }
  } else {
    if (m_Settings->GetTimeMode() != 0) {
      m_Settings->SetTimeMode(0);
    }
  }
  
  if (m_MinTimeEntry->IsModified() == true) {
    m_Settings->SetTimeRangeMin(m_MinTimeEntry->GetAsString().ToDouble());
  }
  if (m_MaxTimeEntry->IsModified() == true) {
    m_Settings->SetTimeRangeMax(m_MaxTimeEntry->GetAsString().ToDouble());
  }  
  if (m_GTIFile->GetFileName() != m_Settings->GetTimeFile()) {
    m_Settings->SetTimeFile(m_GTIFile->GetFileName());
  }
  
  
  
  if (m_TrackLength->IsModified() == true) {
    m_Settings->SetTrackLengthRangeMin(m_TrackLength->GetMinValueInt());
    m_Settings->SetTrackLengthRangeMax(m_TrackLength->GetMaxValueInt());
  }

  if (m_SequenceLength->IsModified() == true) {
    m_Settings->SetSequenceLengthRangeMin(m_SequenceLength->GetMinValueInt());
    m_Settings->SetSequenceLengthRangeMax(m_SequenceLength->GetMaxValueInt());
  }

  if (m_ClusteringQualityFactor->IsModified() == true) {
    m_Settings->SetClusteringQualityFactorRangeMin(m_ClusteringQualityFactor->GetMinValue());
    m_Settings->SetClusteringQualityFactorRangeMax(m_ClusteringQualityFactor->GetMaxValue());
  }

  if (m_ComptonQualityFactor->IsModified() == true) {
    m_Settings->SetComptonQualityFactorRangeMin(m_ComptonQualityFactor->GetMinValue());
    m_Settings->SetComptonQualityFactorRangeMax(m_ComptonQualityFactor->GetMaxValue());
  }

  if (m_TrackQualityFactor->IsModified() == true) {
    m_Settings->SetTrackQualityFactorRangeMin(m_TrackQualityFactor->GetMinValue());
    m_Settings->SetTrackQualityFactorRangeMax(m_TrackQualityFactor->GetMaxValue());
  }

  if (m_CoincidenceWindow->IsModified() == true) {
    m_Settings->SetCoincidenceWindowRangeMin(m_CoincidenceWindow->GetMinValue());
    m_Settings->SetCoincidenceWindowRangeMax(m_CoincidenceWindow->GetMaxValue());
  }

  if (m_FirstTotalEnergy->IsModified() == true) {
    m_Settings->SetFirstEnergyRangeMin(m_FirstTotalEnergy->GetMinValue());
    m_Settings->SetFirstEnergyRangeMax(m_FirstTotalEnergy->GetMaxValue());
  }

  if (m_SecondTotalEnergy->IsModified() == true) {
    m_Settings->SetSecondEnergyRangeMin(m_SecondTotalEnergy->GetMinValue());
    m_Settings->SetSecondEnergyRangeMax(m_SecondTotalEnergy->GetMaxValue());
  }

  if (m_ThirdTotalEnergy->IsModified() == true) {
    m_Settings->SetThirdEnergyRangeMin(m_ThirdTotalEnergy->GetMinValue());
    m_Settings->SetThirdEnergyRangeMax(m_ThirdTotalEnergy->GetMaxValue());
  }

  if (m_FourthTotalEnergy->IsModified() == true) {
    m_Settings->SetFourthEnergyRangeMin(m_FourthTotalEnergy->GetMinValue());
    m_Settings->SetFourthEnergyRangeMax(m_FourthTotalEnergy->GetMaxValue());
  }

  if (m_GammaEnergy->IsModified() == true) {
    m_Settings->SetEnergyRangeGammaMin(m_GammaEnergy->GetMinValue());
    m_Settings->SetEnergyRangeGammaMax(m_GammaEnergy->GetMaxValue());
  }

  if (m_ElectronEnergy->IsModified() == true) {
    m_Settings->SetEnergyRangeElectronMin(m_ElectronEnergy->GetMinValue());
    m_Settings->SetEnergyRangeElectronMax(m_ElectronEnergy->GetMaxValue());
  }

  if (m_ComptonAngle->IsModified() == true) {
    m_Settings->SetComptonAngleRangeMin(m_ComptonAngle->GetMinValue());
    m_Settings->SetComptonAngleRangeMax(m_ComptonAngle->GetMaxValue());
  }

  if (m_EHCAngle->IsModified() == true) {
    m_Settings->SetEHCAngle(m_EHCAngle->GetAsDouble());
  }
  if (m_EHCSelected - c_EHCNone != m_Settings->GetEHCType()) {
    m_Settings->SetEHCType(m_EHCSelected - c_EHCNone);
  }
  if (m_EHCProbability->IsModified() == true) {
    m_Settings->SetEHCProbability(m_EHCProbability->GetAsDouble());
  }
  if (m_EHCProbabilityFile->GetFileName() != m_Settings->GetEHCComptonProbabilityFileName()) {
    m_Settings->SetEHCComptonProbabilityFileName(m_EHCProbabilityFile->GetFileName());
  }
  if (m_ThetaDeviationMax->IsModified() == true) {
    m_Settings->SetThetaDeviationMax(m_ThetaDeviationMax->GetAsDouble());
  }

  
  if (((m_UsePointSource->GetState() == kButtonDown) ? true : false) != m_Settings->GetSourceUsePointSource()) {
    m_Settings->SetSourceUsePointSource((m_UsePointSource->GetState() == kButtonDown) ? true : false);
  }
  if (m_CoordinatesSelected == c_UseGalacticPointSource) {
    if (m_Settings->GetSourceCoordinates() != MCoordinateSystem::c_Galactic) m_Settings->SetSourceCoordinates(MCoordinateSystem::c_Galactic);
  } else if (m_CoordinatesSelected == c_UseSphericPointSource) {
    if (m_Settings->GetSourceCoordinates() != MCoordinateSystem::c_Spheric) m_Settings->SetSourceCoordinates(MCoordinateSystem::c_Spheric);
  } else {
    if (m_Settings->GetSourceCoordinates() != MCoordinateSystem::c_Cartesian3D) m_Settings->SetSourceCoordinates(MCoordinateSystem::c_Cartesian3D);
  }

  if (m_SourceGalactic->IsModified() == true) {
    m_Settings->SetSourceLongitude(m_SourceGalactic->GetAsDouble(0));
    m_Settings->SetSourceLatitude(m_SourceGalactic->GetAsDouble(1));
  }

  if (m_SourceSpheric->IsModified() == true) {
    m_Settings->SetSourceTheta(m_SourceSpheric->GetAsDouble(0));
    m_Settings->SetSourcePhi(m_SourceSpheric->GetAsDouble(1));
  }

  if (m_SourceCartesian->IsModified() == true) {
    m_Settings->SetSourceX(m_SourceCartesian->GetAsDouble(0));
    m_Settings->SetSourceY(m_SourceCartesian->GetAsDouble(1));
    m_Settings->SetSourceZ(m_SourceCartesian->GetAsDouble(2));
  }

  if (m_ARM->IsModified() == true) {
    m_Settings->SetSourceARMMin(m_ARM->GetMinValue());
    m_Settings->SetSourceARMMax(m_ARM->GetMaxValue());
  }
  if (m_SPD->IsModified() == true) {
    m_Settings->SetSourceSPDMin(m_SPD->GetMinValue());
    m_Settings->SetSourceSPDMax(m_SPD->GetMaxValue());
  }

  if (m_UsePointingSelectionPointSource->GetState() == kButtonDown) {
    if (m_Settings->GetPointingSelectionType() != 1) {
      m_Settings->SetPointingSelectionType(1);
    }
  } else if (m_UsePointingSelectionBox->GetState() == kButtonDown) {
    if (m_Settings->GetPointingSelectionType() != 2) {
      m_Settings->SetPointingSelectionType(2);
    }
  } else {
    if (m_Settings->GetPointingSelectionType() != 0) {
      m_Settings->SetPointingSelectionType(0);
    }
  }
  
  if (m_PointingPointSourceLocation->IsModified() == true) {
    m_Settings->SetPointingPointSourceLatitude(m_PointingPointSourceLocation->GetAsDouble(0));
    m_Settings->SetPointingPointSourceLongitude(m_PointingPointSourceLocation->GetAsDouble(1));
  }
  if (m_PointingPointSourceRadius->IsModified() == true) {
    m_Settings->SetPointingPointSourceRadius(m_PointingPointSourceRadius->GetAsDouble());
  }
  if (m_PointingBoxLocation->IsModified() == true) {
    m_Settings->SetPointingBoxLatitude(m_PointingBoxLocation->GetAsDouble(0));
    m_Settings->SetPointingBoxLongitude(m_PointingBoxLocation->GetAsDouble(1));
  }
  if (m_PointingBoxExtentLatitude->IsModified() == true) {
    m_Settings->SetPointingBoxExtentLatitude(m_PointingBoxExtentLatitude->GetAsDouble());
  }
  if (m_PointingBoxExtentLongitude->IsModified() == true) {
    m_Settings->SetPointingBoxExtentLongitude(m_PointingBoxExtentLongitude->GetAsDouble());
  }
  
  
  
  if (m_Settings->GetBeamUse() != ((m_UseBeam->GetState() == kButtonDown) ? true : false)) {
    m_Settings->SetBeamUse((m_UseBeam->GetState() == kButtonDown) ? true : false);
  }

  if (m_BeamStart->IsModified() == true) {
    m_Settings->SetBeamStartX(m_BeamStart->GetAsDouble(0));
    m_Settings->SetBeamStartY(m_BeamStart->GetAsDouble(1));
    m_Settings->SetBeamStartZ(m_BeamStart->GetAsDouble(2));
  }

  if (m_BeamFocalSpot->IsModified() == true) {
    m_Settings->SetBeamFocalSpotX(m_BeamFocalSpot->GetAsDouble(0));
    m_Settings->SetBeamFocalSpotY(m_BeamFocalSpot->GetAsDouble(1));
    m_Settings->SetBeamFocalSpotZ(m_BeamFocalSpot->GetAsDouble(2));
  }

  if (m_BeamRadius->IsModified() == true) {
    m_Settings->SetBeamRadius(m_BeamRadius->GetAsDouble());
    m_Settings->SetBeamDepth(m_BeamDepth->GetAsDouble());
  }

  if (m_FirstIADistance->IsModified() == true) {
    m_Settings->SetFirstDistanceRangeMin(m_FirstIADistance->GetMinValue());
    m_Settings->SetFirstDistanceRangeMax(m_FirstIADistance->GetMaxValue());
  }

  if (m_IADistance->IsModified() == true) {
    m_Settings->SetDistanceRangeMin(m_IADistance->GetMinValue());
    m_Settings->SetDistanceRangeMax(m_IADistance->GetMaxValue());
  }

  if (m_Settings->GetSpecialMode() == true) {
    if (m_TimeWalk->IsModified() == true) {
      m_Settings->SetTimeWalkRangeMin(m_TimeWalk->GetMinValue());
      m_Settings->SetTimeWalkRangeMax(m_TimeWalk->GetMaxValue());
    }
  }

  if (m_OpeningAnglePair->IsModified() == true) {
    m_Settings->SetOpeningAnglePairMin(m_OpeningAnglePair->GetMinValue());
    m_Settings->SetOpeningAnglePairMax(m_OpeningAnglePair->GetMaxValue());
  }

  if (m_InitialEnergyDepositPair->IsModified() == true) {
    m_Settings->SetInitialEnergyDepositPairMin(m_InitialEnergyDepositPair->GetMinValue());
    m_Settings->SetInitialEnergyDepositPairMax(m_InitialEnergyDepositPair->GetMaxValue());
  }

  // Check if the detector have been modified:
  unsigned int NSelectedDetectors = 0;
  bool DetectorsModified = false;
  for (int d = 0; d < m_DetectorList->GetNumberOfEntries(); ++d) {
    if (m_DetectorList->GetSelection(d) == true) {
      ++NSelectedDetectors;
      bool Found = false;
      for (unsigned int e = 0; e < m_Settings->GetNExcludedDetectors(); ++e) {
        if (m_Settings->GetExcludedDetectorAt(e) == m_Geometry->GetDetectorAt(d)->GetName()) {
          Found = true;
          break;
        }
      }
      if (Found == false) { 
        DetectorsModified = true;
        break;
      }
    }
  }
  if (NSelectedDetectors != m_Settings->GetNExcludedDetectors()) {
    DetectorsModified = true;
  }

  if (DetectorsModified == true) {
    m_Settings->RemoveAllExcludedDetectors();
    for (int d = 0; d < m_DetectorList->GetNumberOfEntries(); ++d) {
      if (m_DetectorList->GetSelection(d) == true) {
        m_Settings->AddExcludedDetector(m_Geometry->GetDetectorAt(d)->GetName());
      }
    }
  }

  m_Settings->SetEventSelectorTab(m_MainTab->GetCurrent());

  dynamic_cast<MSettings*>(m_Settings)->Write();
  
  return true;
}


// MGUIEventSelection: the end...
////////////////////////////////////////////////////////////////////////////////
