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
#include "MProjection.h"

#ifdef ___CINT___
ClassImp(MGUIEventSelection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEventSelection::MGUIEventSelection(const TGWindow* p, const TGWindow* main, 
                                       MSettingsEventSelections* Data, MDGeometryQuest* Geo)
  : MGUIDialog(p, main)
{
  // standard constructor

  m_GUIData = Data;
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

  int FieldSize = 95;
  int LeftGap = 65;
  int RightGap = 65;

  // We start with a name and an icon...
  SetWindowName("Event selector");  
 
  // The subtitle
  SetSubTitleText("Please set all the cuts on the data for the analysis");

  CreateBasics();


  TGLayoutHints* MinMaxLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, LeftGap, RightGap, 10, 5);
  TGLayoutHints* MinMaxFirstLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, LeftGap, RightGap, 20, 5);
  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, LeftGap, RightGap, 10, 10);

  m_MainTab = new TGTab(m_MainPanel, 300, 300);
  TGLayoutHints* MainTabLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 10, 10, 0, 0);
  m_MainPanel->AddFrame(m_MainTab, MainTabLayout);

  TGCompositeFrame* EventTypeFrame = m_MainTab->AddTab("Event types");
  TGCompositeFrame* GeneralFrame = m_MainTab->AddTab("General");
  TGCompositeFrame* EnergiesFrame = m_MainTab->AddTab("Energies");
  TGCompositeFrame* ComptonEAFrame = m_MainTab->AddTab("C: Energies/Angles");
  TGCompositeFrame* ComptonDLFrame = m_MainTab->AddTab("C: Lengths");
  TGCompositeFrame* ComptonQFrame = m_MainTab->AddTab("C: Quality");
  TGCompositeFrame* ComptonEHCFrame = m_MainTab->AddTab("C: EHC");
  TGCompositeFrame* PairFrame = m_MainTab->AddTab("Pair: All");
  TGCompositeFrame* SourceFrame = m_MainTab->AddTab("Origins");
  TGCompositeFrame* BeamFrame = m_MainTab->AddTab("Beam");
  TGCompositeFrame* SpecialFrame = 0;
  if (m_GUIData->GetSpecialMode() == true) {
    SpecialFrame = m_MainTab->AddTab("Special");
  }

  m_MainTab->SetTab(m_GUIData->GetEventSelectorTab());

  // Event and detector type frame:
  TGLabel* LabelEventSelection = new TGLabel(EventTypeFrame, new TGString("Choose from the following event types:"));
  TGLayoutHints* LabelEventSelectionLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, LeftGap, RightGap, 20, 0);
  EventTypeFrame->AddFrame(LabelEventSelection, LabelEventSelectionLayout);

  m_ComptonCB = new TGCheckButton(EventTypeFrame, "Compton scattering events:", c_Compton);
  TGLayoutHints* ComptonCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_ComptonCB, ComptonCBLayout);
  m_ComptonCB->Associate(this);
  m_ComptonCB->SetState((m_GUIData->GetEventTypeCompton() == 1) ?  kButtonDown : kButtonUp);

  m_ComptonNotTrackedCB = new TGCheckButton(EventTypeFrame, "without recoil electron track", c_ComptonUntracked);
  TGLayoutHints* ComptonNotTrackedCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+30, RightGap, 2, 0);
  EventTypeFrame->AddFrame(m_ComptonNotTrackedCB, ComptonNotTrackedCBLayout);
  m_ComptonNotTrackedCB->SetState((m_GUIData->GetEventTypeComptonNotTracked() == 1) ?  kButtonDown : kButtonUp);
  m_ComptonNotTrackedCB->Associate(this);
  if (m_ComptonCB->GetState() == kButtonUp) {
    m_ComptonNotTrackedCB->SetState(kButtonDisabled);
  }
  m_UseNotTracked = m_GUIData->GetEventTypeComptonNotTracked();

  m_ComptonTrackedCB = new TGCheckButton(EventTypeFrame, "with recoil electron track", c_ComptonTracked);
  TGLayoutHints* ComptonTrackedCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+30, RightGap, 2, 0);
  EventTypeFrame->AddFrame(m_ComptonTrackedCB, ComptonTrackedCBLayout);
  m_ComptonTrackedCB->SetState((m_GUIData->GetEventTypeComptonTracked() == 1) ?  kButtonDown : kButtonUp);
  m_ComptonTrackedCB->Associate(this);
  if (m_ComptonCB->GetState() == kButtonUp) {
    m_ComptonTrackedCB->SetState(kButtonDisabled);
  }
  m_UseTracked = m_GUIData->GetEventTypeComptonTracked();

  m_PairCB = new TGCheckButton(EventTypeFrame, "Pair creation events", c_Pair);
  TGLayoutHints* PairCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_PairCB, PairCBLayout);
  m_PairCB->SetState((m_GUIData->GetEventTypePair() == 1) ?  kButtonDown : kButtonUp);

  m_PhotoCB = new TGCheckButton(EventTypeFrame, "Photo effect events", c_Photo);
  TGLayoutHints* PhotoCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_PhotoCB, PhotoCBLayout);
  m_PhotoCB->SetState((m_GUIData->GetEventTypePhoto() == 1) ?  kButtonDown : kButtonUp);

  m_UnidentifiableCB = new TGCheckButton(EventTypeFrame, "Unidentifiable effect events", c_Unidentifiable);
  TGLayoutHints* UnidentifiableCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);
  EventTypeFrame->AddFrame(m_UnidentifiableCB, UnidentifiableCBLayout);
  m_UnidentifiableCB->SetState((m_GUIData->GetEventTypeUnidentifiable() == 1) ?  kButtonDown : kButtonUp);

  m_DecayCB = new TGCheckButton(EventTypeFrame, "Use events where the \"Decay\" flag in set", c_Decay);
  TGLayoutHints* DecayCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 15, 5);
  EventTypeFrame->AddFrame(m_DecayCB, DecayCBLayout);
  m_DecayCB->SetState((m_GUIData->GetEventTypeDecay() == 1) ?  kButtonDown : kButtonUp);

  m_BadCB = new TGCheckButton(EventTypeFrame, "Use events where the \"Bad\" flag in set", c_Bad);
  TGLayoutHints* BadCBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 20);
  EventTypeFrame->AddFrame(m_BadCB, BadCBLayout);
  m_BadCB->SetState((m_GUIData->GetFlaggedAsBad() == true) ?  kButtonDown : kButtonUp);




  // General Frame

  m_EventId = new MGUIEMinMaxEntry(GeneralFrame,
                                   MString("Event ID window:"),
                                   false,
                                   MString("Minimum ID: "),
                                   MString("Maximum ID: "),
                                   double(m_GUIData->GetEventIdRangeMin()), 
                                   double(m_GUIData->GetEventIdRangeMax()), 
                                   true, 0.0);
  m_EventId->SetEntryFieldSize(FieldSize);
  GeneralFrame->AddFrame(m_EventId, MinMaxFirstLayout);

  m_Time = new MGUIEMinMaxEntry(GeneralFrame,
                                MString("Detection time window of the event:"),
                                false,
                                MString("Minimum [sec in system time]: "),
                                MString("Maximum [sec in system time]: "),
                                m_GUIData->GetTimeRangeMin(), 
                                m_GUIData->GetTimeRangeMax(), 
                                true, 0.0);
  m_Time->SetEntryFieldSize(FieldSize);
  GeneralFrame->AddFrame(m_Time, MinMaxLayout);


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
      for (unsigned int e = 0; e < m_GUIData->GetNExcludedDetectors(); ++e) {
        if (m_GUIData->GetExcludedDetectorAt(e) == m_Geometry->GetDetectorAt(d)->GetName()) {
          m_DetectorList->Select(d);
          break;
        }
      }
    }
  }
  m_DetectorList->Resize(200, 100);


  
  // Energies Frame

  m_FirstTotalEnergy = new MGUIEMinMaxEntry(EnergiesFrame,
                                            MString("First energy window on total energy of incident gamma-ray:"), 
                                            false,
                                            MString("Minimum energy [keV]: "),
                                            MString("Maximum energy [keV]: "),
                                            m_GUIData->GetFirstEnergyRangeMin(), 
                                            m_GUIData->GetFirstEnergyRangeMax(),
                                            true, 0.0);
  m_FirstTotalEnergy->SetEntryFieldSize(FieldSize);
  EnergiesFrame->AddFrame(m_FirstTotalEnergy, MinMaxFirstLayout);

  m_SecondTotalEnergy = new MGUIEMinMaxEntry(EnergiesFrame,
                                             MString("Second energy window on total energy of incident gamma-ray:"), 
                                             false,
                                             MString("Minimum energy [keV]: "),
                                             MString("Maximum energy [keV]: "),
                                             m_GUIData->GetSecondEnergyRangeMin(), 
                                             m_GUIData->GetSecondEnergyRangeMax(), 
                                             true, 0.0);
  m_SecondTotalEnergy->SetEntryFieldSize(FieldSize);
  EnergiesFrame->AddFrame(m_SecondTotalEnergy, MinMaxLayout);

  m_ThirdTotalEnergy = new MGUIEMinMaxEntry(EnergiesFrame,
                                            MString("Third energy window on total energy of incident gamma-ray:"), 
                                            false,
                                            MString("Minimum energy [keV]: "),
                                            MString("Maximum energy [keV]: "),
                                            m_GUIData->GetThirdEnergyRangeMin(), 
                                            m_GUIData->GetThirdEnergyRangeMax(), 
                                            true, 0.0);
  m_ThirdTotalEnergy->SetEntryFieldSize(FieldSize);
  EnergiesFrame->AddFrame(m_ThirdTotalEnergy, MinMaxLayout);

  m_FourthTotalEnergy = new MGUIEMinMaxEntry(EnergiesFrame,
                                             MString("Fourth energy window on total energy of incident gamma-ray:"), 
                                             false,
                                             MString("Minimum energy [keV]: "),
                                             MString("Maximum energy [keV]: "),
                                             m_GUIData->GetFourthEnergyRangeMin(), 
                                             m_GUIData->GetFourthEnergyRangeMax(), 
                                             true, 0.0);
  m_FourthTotalEnergy->SetEntryFieldSize(FieldSize);
  EnergiesFrame->AddFrame(m_FourthTotalEnergy, MinMaxLayout);


  // Compton frame:

  m_GammaEnergy = new MGUIEMinMaxEntry(ComptonEAFrame,
                                       MString("Energy range of scattered gamma-ray:"),
                                       false,
                                       MString("Minimum energy [keV]: "),
                                       MString("Maximum energy [keV]: "),
                                       m_GUIData->GetEnergyRangeGammaMin(), 
                                       m_GUIData->GetEnergyRangeGammaMax(), 
                                       true, 0.0);
  m_GammaEnergy->SetEntryFieldSize(FieldSize);
  ComptonEAFrame->AddFrame(m_GammaEnergy, MinMaxFirstLayout);

  m_ElectronEnergy = new MGUIEMinMaxEntry(ComptonEAFrame,
                                          MString("Energy range of recoil electron:"),
                                          false,
                                          MString("Minimum energy [keV]: "),
                                          MString("Maximum energy [keV]: "),
                                          m_GUIData->GetEnergyRangeElectronMin(), 
                                          m_GUIData->GetEnergyRangeElectronMax(), 
                                          true, 0.0);
  m_ElectronEnergy->SetEntryFieldSize(FieldSize);
  ComptonEAFrame->AddFrame(m_ElectronEnergy, MinMaxLayout);

  m_ComptonAngle = new MGUIEMinMaxEntry(ComptonEAFrame,
                                        MString("Range of Compton scatter angles:"),
                                        false,
                                        MString("Minimum angle [deg]: "),
                                        MString("Maximum angle [deg]: "),
                                        m_GUIData->GetComptonAngleRangeMin(), 
                                        m_GUIData->GetComptonAngleRangeMax(), 
                                        true, 0.0, 180.0);
  m_ComptonAngle->SetEntryFieldSize(FieldSize);
  ComptonEAFrame->AddFrame(m_ComptonAngle, MinMaxLayout);

  m_ThetaDeviationMax = 
    new MGUIEEntry(ComptonEAFrame,
                   MString("Maximum deviation of total scatter angles (energy vs. geo) [deg]:"),
                   false,
                   m_GUIData->GetThetaDeviationMax(),
                   true, 0.0, 180.0);
  m_ThetaDeviationMax->SetEntryFieldSize(FieldSize);
  ComptonEAFrame->AddFrame(m_ThetaDeviationMax, SingleLayout);


  // Compton 2 frame:

  m_FirstIADistance = new MGUIEMinMaxEntry(ComptonDLFrame,
                                           MString("Distance between the first interactions:"),
                                           false,
                                           MString("Minimum distance [cm]: "),
                                           MString("Maximum distance [cm]: "),
                                           m_GUIData->GetFirstDistanceRangeMin(), 
                                           m_GUIData->GetFirstDistanceRangeMax(), 
                                           true, 0.0);
  m_FirstIADistance->SetEntryFieldSize(FieldSize);
  ComptonDLFrame->AddFrame(m_FirstIADistance, MinMaxFirstLayout);

  m_IADistance = new MGUIEMinMaxEntry(ComptonDLFrame,
                                      MString("Min. distance between any two interactions:"),
                                       false,
                                      MString("Minimum distance [cm]: "),
                                      MString("Maximum distance [cm]: "),
                                      m_GUIData->GetDistanceRangeMin(), 
                                      m_GUIData->GetDistanceRangeMax(),
                                      true, 0.0);
  m_IADistance->SetEntryFieldSize(FieldSize);
  ComptonDLFrame->AddFrame(m_IADistance, MinMaxLayout);

  m_TrackLength = new MGUIEMinMaxEntry(ComptonDLFrame,
                                       MString("Length of (first) electron track in layers:"),
                                       false,
                                       MString("Minimum: "),
                                       MString("Maximum: "),
                                       m_GUIData->GetTrackLengthRangeMin(), 
                                       m_GUIData->GetTrackLengthRangeMax(),
                                       true, 0);
  m_TrackLength->SetEntryFieldSize(FieldSize);
  ComptonDLFrame->AddFrame(m_TrackLength, MinMaxLayout);

  m_SequenceLength = new MGUIEMinMaxEntry(ComptonDLFrame,
                                          MString("Length of Compton Sequence:"),
                                          false,
                                          MString("Minimum: "),
                                          MString("Maximum: "),
                                          m_GUIData->GetSequenceLengthRangeMin(), 
                                          m_GUIData->GetSequenceLengthRangeMax(), 
                                          true, 1);
  m_SequenceLength->SetEntryFieldSize(FieldSize);
  ComptonDLFrame->AddFrame(m_SequenceLength, MinMaxLayout);


  // Compton frame - quality factors:

  m_ClusteringQualityFactor = new MGUIEMinMaxEntry(ComptonQFrame,
                                                   MString("Clustering quality factor:"),
                                                   false,
                                                   MString("Minimum: "),
                                                   MString("Maximum: "),
                                                   m_GUIData->GetClusteringQualityFactorRangeMin(), 
                                                   m_GUIData->GetClusteringQualityFactorRangeMax(), 
                                                   true, 0.0);
  m_ClusteringQualityFactor->SetEntryFieldSize(FieldSize);
  ComptonQFrame->AddFrame(m_ClusteringQualityFactor, MinMaxFirstLayout);

  m_ComptonQualityFactor = new MGUIEMinMaxEntry(ComptonQFrame,
                                             MString("Compton quality factor:"),
                                             false,
                                             MString("Minimum: "),
                                             MString("Maximum: "),
                                             m_GUIData->GetComptonQualityFactorRangeMin(), 
                                             m_GUIData->GetComptonQualityFactorRangeMax(), 
                                             true, 0.0);
  m_ComptonQualityFactor->SetEntryFieldSize(FieldSize);
  ComptonQFrame->AddFrame(m_ComptonQualityFactor, MinMaxFirstLayout);

  m_TrackQualityFactor = new MGUIEMinMaxEntry(ComptonQFrame,
                                             MString("Track quality factor:"),
                                             false,
                                             MString("Minimum: "),
                                             MString("Maximum: "),
                                             m_GUIData->GetTrackQualityFactorRangeMin(), 
                                             m_GUIData->GetTrackQualityFactorRangeMax(), 
                                             true, 0.0);
  m_TrackQualityFactor->SetEntryFieldSize(FieldSize);
  ComptonQFrame->AddFrame(m_TrackQualityFactor, MinMaxLayout);




  // Compton frame - Earth horizon cut:

  TGLabel* LabelEHC = new TGLabel(ComptonEHCFrame, new TGString("Fixed Earth horizon cut (Earth in nadir):"));
  TGLayoutHints* LabelEHCLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, LeftGap, RightGap, 20, 0);
  ComptonEHCFrame->AddFrame(LabelEHC, LabelEHCLayout);

  TGLayoutHints* ComptonRBLayout = new TGLayoutHints(kLHintsLeft, LeftGap+10, RightGap, 5, 0);

  m_EHCRBNone = new TGRadioButton(ComptonEHCFrame, "None", c_EHCNone);
  ComptonEHCFrame->AddFrame(m_EHCRBNone, ComptonRBLayout);
  m_EHCRBNone->Associate(this);
  m_EHCRBNone->SetState((m_GUIData->GetEHCType() == 0) ?  kButtonDown : kButtonUp);

  m_EHCRBIntersection = new TGRadioButton(ComptonEHCFrame, "Reject all events, whose full cone circle (ignoring possible arcs) intersects Earth", c_EHCIntersection);
  ComptonEHCFrame->AddFrame(m_EHCRBIntersection, ComptonRBLayout);
  m_EHCRBIntersection->Associate(this);
  m_EHCRBIntersection->SetState((m_GUIData->GetEHCType() == 1) ?  kButtonDown : kButtonUp);

  m_EHCRBProbability = new TGRadioButton(ComptonEHCFrame, "Reject all events, whose probability of originating from Earth exceeds:", c_EHCProbability);
  ComptonEHCFrame->AddFrame(m_EHCRBProbability, ComptonRBLayout);
  m_EHCRBProbability->Associate(this);
  m_EHCRBProbability->SetState((m_GUIData->GetEHCType() == 2) ?  kButtonDown : kButtonUp);

  TGLayoutHints* EHCSingleLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, LeftGap+30, RightGap, 5, 5);

  m_EHCProbability = 
    new MGUIEEntry(ComptonEHCFrame,
                   MString("Maximum Earth probability [0..1]:"),
                   false,
                   m_GUIData->GetEHCProbability(),
                   true, 0.0, 1.0);
  m_EHCProbability->SetEntryFieldSize(FieldSize);
  ComptonEHCFrame->AddFrame(m_EHCProbability, EHCSingleLayout);

  m_EHCProbabilityFile = 
    new MGUIEFileSelector(ComptonEHCFrame,
                          MString("EHC Compton probability response file (*.compton.ehc.rsp):"),
                          m_GUIData->GetEHCComptonProbabilityFileName());
  m_EHCProbabilityFile->SetFileType("EHC response file", "*.compton.ehc.rsp");
  ComptonEHCFrame->AddFrame(m_EHCProbabilityFile, EHCSingleLayout);

  m_EHCSelected = m_GUIData->GetEHCType() + c_EHCNone;
  if (m_EHCSelected != c_EHCProbability) {
    m_EHCProbability->SetEnabled(false);
    m_EHCProbabilityFile->SetEnabled(false);
  }

  m_EHCAngle = 
    new MGUIEEntry(ComptonEHCFrame,
                   MString("Angle between direction to Earth's center and horizon (0 = nadir) [deg]:"),
                   false,
                   m_GUIData->GetEHCAngle(),
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
                                            m_GUIData->GetOpeningAnglePairMin(), 
                                            m_GUIData->GetOpeningAnglePairMax(),
                                            true, 0.0, 180.0);
  m_OpeningAnglePair->SetEntryFieldSize(FieldSize);
  PairFrame->AddFrame(m_OpeningAnglePair, MinMaxFirstLayout);

  m_InitialEnergyDepositPair = new MGUIEMinMaxEntry(PairFrame,
                                                    MString("Energy deposit in the first layer of interaction:"),
                                                    false,
                                                    MString("Minimum [keV]: "),
                                                    MString("Maximum [keV]: "),
                                                    m_GUIData->GetInitialEnergyDepositPairMin(), 
                                                    m_GUIData->GetInitialEnergyDepositPairMax(),
                                                    true, 0.0);
  m_InitialEnergyDepositPair->SetEntryFieldSize(FieldSize);
  PairFrame->AddFrame(m_InitialEnergyDepositPair, MinMaxLayout);



  // Source frame:
  m_UsePointSource = new TGCheckButton(SourceFrame, "Use selection on point source:", c_UsePointSource);
  SourceFrame->AddFrame(m_UsePointSource, MinMaxFirstLayout);
  m_UsePointSource->Associate(this);
  m_UsePointSource->SetState((m_GUIData->GetSourceUsePointSource() == true) ?  kButtonDown : kButtonUp);

  TGLayoutHints* CoordinatesLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, LeftGap+20, RightGap, 10, 0);

  m_UseSphericPointSource = new TGRadioButton(SourceFrame, "Point source in spheric coordinates:", c_UseSphericPointSource);
  SourceFrame->AddFrame(m_UseSphericPointSource, CoordinatesLayout);
  m_UseSphericPointSource->Associate(this);

  TGLayoutHints* SourceLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, LeftGap+40, RightGap, 5, 0);

  m_SourceSpheric = new MGUIEEntryList(SourceFrame, "Location (Theta, Phi) [deg]", MGUIEEntryList::c_SingleLine);
  m_SourceSpheric->Add("", m_GUIData->GetSourceTheta(), kTRUE, 0.0, 180.0);
  m_SourceSpheric->Add("", m_GUIData->GetSourcePhi(), kTRUE, 0.0, 360.0);
  m_SourceSpheric->SetEntryFieldSize(FieldSize);
  m_SourceSpheric->Create();
  SourceFrame->AddFrame(m_SourceSpheric, SourceLayout);

  m_UseGalacticPointSource = new TGRadioButton(SourceFrame, "Point source in Galactic coordinates:", c_UseGalacticPointSource);
  SourceFrame->AddFrame(m_UseGalacticPointSource, CoordinatesLayout);
  m_UseGalacticPointSource->Associate(this);

  m_SourceGalactic = new MGUIEEntryList(SourceFrame, "Location (Long, Lat) [deg]", MGUIEEntryList::c_SingleLine);
  m_SourceGalactic->Add("", m_GUIData->GetSourceLongitude(), kTRUE, 0.0, 360.0);
  m_SourceGalactic->Add("", m_GUIData->GetSourceLatitude(), kTRUE, -90.0, 90.0);
  m_SourceGalactic->SetEntryFieldSize(FieldSize);
  m_SourceGalactic->Create();
  SourceFrame->AddFrame(m_SourceGalactic, SourceLayout);

  m_UseCartesianPointSource = new TGRadioButton(SourceFrame, "Point source in Cartesian coordinates:", c_UseCartesianPointSource);
  TGLayoutHints* UseCartesianPointSourceLayout = new TGLayoutHints(kLHintsLeft, LeftGap+20, RightGap, 10, 0);
  SourceFrame->AddFrame(m_UseCartesianPointSource, UseCartesianPointSourceLayout);
  m_UseCartesianPointSource->Associate(this);

  m_SourceCartesian = new MGUIEEntryList(SourceFrame, "Location (x, y, z) [cm]", MGUIEEntryList::c_SingleLine);
  m_SourceCartesian->Add("", m_GUIData->GetSourceX(), kTRUE, 0.0);
  m_SourceCartesian->Add("", m_GUIData->GetSourceY(), kTRUE, 0.0);
  m_SourceCartesian->Add("", m_GUIData->GetSourceZ(), kTRUE, 0.0);
  m_SourceCartesian->SetEntryFieldSize(FieldSize);
  m_SourceCartesian->Create();
  SourceFrame->AddFrame(m_SourceCartesian, SourceLayout);


  m_ARM = new MGUIEMinMaxEntry(SourceFrame,
                               MString("Radial window (= ARM for Compton):"),
                               false,
                               MString("Minimum [deg]: "),
                               MString("Maximum [deg]: "),
                               m_GUIData->GetSourceARMMin(), 
                               m_GUIData->GetSourceARMMax(), 
                               true, 0.0, 180.0);
  m_ARM->SetEntryFieldSize(FieldSize);
  SourceFrame->AddFrame(m_ARM, CoordinatesLayout);

  m_SPD = new MGUIEMinMaxEntry(SourceFrame,
                               MString("Transversal window (= SPD for tracked Comptons):"),
                               false,
                               MString("Minimum [deg]: "),
                               MString("Maximum [deg]: "),
                               m_GUIData->GetSourceSPDMin(), 
                               m_GUIData->GetSourceSPDMax(), 
                               true, 0.0, 180.0);
  m_SPD->SetEntryFieldSize(FieldSize);
  SourceFrame->AddFrame(m_SPD, CoordinatesLayout);

  if (m_GUIData->GetSourceUsePointSource() == true) {
    if (m_GUIData->GetSourceCoordinates() == MProjection::c_Galactic) {
      m_CoordinatesSelected = c_UseGalacticPointSource;
    } else if (m_GUIData->GetSourceCoordinates() == MProjection::c_Spheric) {
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
    if (m_GUIData->GetSourceCoordinates() == MProjection::c_Galactic) {
      m_CoordinatesSelected = c_UseGalacticPointSource;
    } else if (m_GUIData->GetSourceCoordinates() == MProjection::c_Spheric) {
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



  // Beam frame
  m_UseBeam = new TGCheckButton(BeamFrame, "Use beam selection:", c_UseBeam);
  BeamFrame->AddFrame(m_UseBeam, MinMaxFirstLayout);
  m_UseBeam->Associate(this);
  m_UseBeam->SetState((m_GUIData->GetBeamUse() == true) ?  kButtonDown : kButtonUp);

  TGLayoutHints* BeamLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, LeftGap+20, RightGap, 10, 0);

  m_BeamStart = new MGUIEEntryList(BeamFrame, "Start position (x, y, z) [cm]", MGUIEEntryList::c_SingleLine);
  m_BeamStart->Add("", m_GUIData->GetBeamStartX());
  m_BeamStart->Add("", m_GUIData->GetBeamStartY());
  m_BeamStart->Add("", m_GUIData->GetBeamStartZ());
  m_BeamStart->SetEntryFieldSize(FieldSize);
  m_BeamStart->Create();
  BeamFrame->AddFrame(m_BeamStart, BeamLayout);

  m_BeamFocalSpot = new MGUIEEntryList(BeamFrame, "Focal spot on detector (x, y, z) [cm]", MGUIEEntryList::c_SingleLine);
  m_BeamFocalSpot->Add("", m_GUIData->GetBeamFocalSpotX());
  m_BeamFocalSpot->Add("", m_GUIData->GetBeamFocalSpotY());
  m_BeamFocalSpot->Add("", m_GUIData->GetBeamFocalSpotZ());
  m_BeamFocalSpot->SetEntryFieldSize(FieldSize);
  m_BeamFocalSpot->Create();
  BeamFrame->AddFrame(m_BeamFocalSpot, BeamLayout);

  m_BeamRadius = 
    new MGUIEEntry(BeamFrame,
                   "Selected radius around beam center [cm]:",
                   false,
                   m_GUIData->GetBeamRadius(),
                   true, 0.0);
  m_BeamRadius->SetEntryFieldSize(FieldSize);
  BeamFrame->AddFrame(m_BeamRadius, BeamLayout);

  m_BeamDepth = 
    new MGUIEEntry(BeamFrame,
                   "Selected maximum (fist) interaction depth [cm]:",
                   false,
                   m_GUIData->GetBeamDepth(),
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
  if (m_GUIData->GetSpecialMode() == true) {
    m_TimeWalk = new MGUIEMinMaxEntry(SpecialFrame,
                                      MString("Time walk between D1 and D2 trigger (Only relevant for Mega):"),
                                      false,
                                      MString("Minimum [ns]: "),
                                      MString("Maximum [ns]: "),
                                      m_GUIData->GetTimeWalkRangeMin(), 
                                      m_GUIData->GetTimeWalkRangeMax());
    SpecialFrame->AddFrame(m_TimeWalk, MinMaxFirstLayout);
  }
  

  // Finally bring it to the screen
  FinalizeCreate(GetDefaultWidth(), GetDefaultHeight()+10, false);

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
      if (Parameter1 >= c_EHCNone && Parameter1 <= c_EHCProbability) {
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
      m_UnidentifiableCB->GetState() == kButtonUp) {
    new TGMsgBox(gClient->GetRoot(), this, "Type error", 
                 "You have to choose one of these event-types:"
                 " Compton events and/or pair events and/or photo events", kMBIconStop, kMBOk);
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
  if (m_EventId->CheckRange(0.0, numeric_limits<double>::max(), 
                            0.0, double(numeric_limits<long>::max()), false) == false) return false;
  if (m_TrackLength->CheckRange(1, numeric_limits<int>::max(), 
                                1, numeric_limits<int>::max(), false) == false) return false;
  if (m_SequenceLength->CheckRange(2, numeric_limits<int>::max(), 
                                   2, numeric_limits<int>::max(), false) == false) return false;
  if (m_ClusteringQualityFactor->CheckRange(0.0, numeric_limits<double>::max(), 
                                            0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_ComptonQualityFactor->CheckRange(0.0, numeric_limits<double>::max(), 
                                         0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_TrackQualityFactor->CheckRange(0.0, numeric_limits<double>::max(), 
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
  if (m_Time->CheckRange(0.0, numeric_limits<double>::max(), 
                         0.0, numeric_limits<double>::max(), true) == false) return false;
  if (m_GUIData->GetSpecialMode() == true) {
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
  if (m_GUIData->GetEventTypeCompton() != ((m_ComptonCB->GetState() == kButtonDown) ? 1 : 0)) m_GUIData->SetEventTypeCompton((m_ComptonCB->GetState() == kButtonDown) ? 1 : 0);
  if (m_GUIData->GetEventTypeComptonNotTracked() != m_UseNotTracked) m_GUIData->SetEventTypeComptonNotTracked(m_UseNotTracked);
  if (m_GUIData->GetEventTypeComptonTracked() != m_UseTracked) m_GUIData->SetEventTypeComptonTracked(m_UseTracked);
  if (m_GUIData->GetEventTypePair() != ((m_PairCB->GetState() == kButtonDown) ? 1 : 0)) m_GUIData->SetEventTypePair((m_PairCB->GetState() == kButtonDown) ? 1 : 0);
  if (m_GUIData->GetEventTypePhoto() != ((m_PhotoCB->GetState() == kButtonDown) ? 1 : 0)) m_GUIData->SetEventTypePhoto((m_PhotoCB->GetState() == kButtonDown) ? 1 : 0); 
  if (m_GUIData->GetEventTypeUnidentifiable() != ((m_UnidentifiableCB->GetState() == kButtonDown) ? 1 : 0)) m_GUIData->SetEventTypeUnidentifiable((m_UnidentifiableCB->GetState() == kButtonDown) ? 1 : 0);
  if (m_GUIData->GetEventTypeDecay() != ((m_DecayCB->GetState() == kButtonDown) ? 1 : 0)) m_GUIData->SetEventTypeDecay((m_DecayCB->GetState() == kButtonDown) ? 1 : 0);

  if (m_GUIData->GetFlaggedAsBad() != ((m_BadCB->GetState() == kButtonDown) ? true : false)) m_GUIData->SetFlaggedAsBad((m_BadCB->GetState() == kButtonDown) ? true : false);  

  if (m_EventId->IsModified() == true) {
    m_GUIData->SetEventIdRangeMin(long(m_EventId->GetMinValueDouble()));
    m_GUIData->SetEventIdRangeMax(long(m_EventId->GetMaxValueDouble()));
  }

  if (m_TrackLength->IsModified() == true) {
    m_GUIData->SetTrackLengthRangeMin(m_TrackLength->GetMinValueInt());
    m_GUIData->SetTrackLengthRangeMax(m_TrackLength->GetMaxValueInt());
  }

  if (m_SequenceLength->IsModified() == true) {
    m_GUIData->SetSequenceLengthRangeMin(m_SequenceLength->GetMinValueInt());
    m_GUIData->SetSequenceLengthRangeMax(m_SequenceLength->GetMaxValueInt());
  }

  if (m_ClusteringQualityFactor->IsModified() == true) {
    m_GUIData->SetClusteringQualityFactorRangeMin(m_ClusteringQualityFactor->GetMinValue());
    m_GUIData->SetClusteringQualityFactorRangeMax(m_ClusteringQualityFactor->GetMaxValue());
  }

  if (m_ComptonQualityFactor->IsModified() == true) {
    m_GUIData->SetComptonQualityFactorRangeMin(m_ComptonQualityFactor->GetMinValue());
    m_GUIData->SetComptonQualityFactorRangeMax(m_ComptonQualityFactor->GetMaxValue());
  }

  if (m_TrackQualityFactor->IsModified() == true) {
    m_GUIData->SetTrackQualityFactorRangeMin(m_TrackQualityFactor->GetMinValue());
    m_GUIData->SetTrackQualityFactorRangeMax(m_TrackQualityFactor->GetMaxValue());
  }

  if (m_FirstTotalEnergy->IsModified() == true) {
    m_GUIData->SetFirstEnergyRangeMin(m_FirstTotalEnergy->GetMinValue());
    m_GUIData->SetFirstEnergyRangeMax(m_FirstTotalEnergy->GetMaxValue());
  }

  if (m_SecondTotalEnergy->IsModified() == true) {
    m_GUIData->SetSecondEnergyRangeMin(m_SecondTotalEnergy->GetMinValue());
    m_GUIData->SetSecondEnergyRangeMax(m_SecondTotalEnergy->GetMaxValue());
  }

  if (m_ThirdTotalEnergy->IsModified() == true) {
    m_GUIData->SetThirdEnergyRangeMin(m_ThirdTotalEnergy->GetMinValue());
    m_GUIData->SetThirdEnergyRangeMax(m_ThirdTotalEnergy->GetMaxValue());
  }

  if (m_FourthTotalEnergy->IsModified() == true) {
    m_GUIData->SetFourthEnergyRangeMin(m_FourthTotalEnergy->GetMinValue());
    m_GUIData->SetFourthEnergyRangeMax(m_FourthTotalEnergy->GetMaxValue());
  }

  if (m_GammaEnergy->IsModified() == true) {
    m_GUIData->SetEnergyRangeGammaMin(m_GammaEnergy->GetMinValue());
    m_GUIData->SetEnergyRangeGammaMax(m_GammaEnergy->GetMaxValue());
  }

  if (m_ElectronEnergy->IsModified() == true) {
    m_GUIData->SetEnergyRangeElectronMin(m_ElectronEnergy->GetMinValue());
    m_GUIData->SetEnergyRangeElectronMax(m_ElectronEnergy->GetMaxValue());
  }

  if (m_ComptonAngle->IsModified() == true) {
    m_GUIData->SetComptonAngleRangeMin(m_ComptonAngle->GetMinValue());
    m_GUIData->SetComptonAngleRangeMax(m_ComptonAngle->GetMaxValue());
  }

  if (m_EHCAngle->IsModified() == true) {
    m_GUIData->SetEHCAngle(m_EHCAngle->GetAsDouble());
  }
  if (m_EHCSelected - c_EHCNone != m_GUIData->GetEHCType()) {
    m_GUIData->SetEHCType(m_EHCSelected - c_EHCNone);
  }
  if (m_EHCProbability->IsModified() == true) {
    m_GUIData->SetEHCProbability(m_EHCProbability->GetAsDouble());
  }
  if (m_EHCProbabilityFile->GetFileName() != m_GUIData->GetEHCComptonProbabilityFileName()) {
    m_GUIData->SetEHCComptonProbabilityFileName(m_EHCProbabilityFile->GetFileName());
  }
  if (m_ThetaDeviationMax->IsModified() == true) {
    m_GUIData->SetThetaDeviationMax(m_ThetaDeviationMax->GetAsDouble());
  }

  if (((m_UsePointSource->GetState() == kButtonDown) ? true : false) != m_GUIData->GetSourceUsePointSource()) {
    m_GUIData->SetSourceUsePointSource((m_UsePointSource->GetState() == kButtonDown) ? true : false);
  }
  if (m_CoordinatesSelected == c_UseGalacticPointSource) {
    if (m_GUIData->GetSourceCoordinates() != MProjection::c_Galactic) m_GUIData->SetSourceCoordinates(MProjection::c_Galactic);
  } else if (m_CoordinatesSelected == c_UseSphericPointSource) {
    if (m_GUIData->GetSourceCoordinates() != MProjection::c_Spheric) m_GUIData->SetSourceCoordinates(MProjection::c_Spheric);
  } else {
    if (m_GUIData->GetSourceCoordinates() != MProjection::c_Cartesian3D) m_GUIData->SetSourceCoordinates(MProjection::c_Cartesian3D);
  }

  if (m_SourceGalactic->IsModified() == true) {
    m_GUIData->SetSourceLongitude(m_SourceSpheric->GetAsDouble(0));
    m_GUIData->SetSourceLatitude(m_SourceSpheric->GetAsDouble(1));
  }

  if (m_SourceSpheric->IsModified() == true) {
    m_GUIData->SetSourceTheta(m_SourceSpheric->GetAsDouble(0));
    m_GUIData->SetSourcePhi(m_SourceSpheric->GetAsDouble(1));
  }

  if (m_SourceCartesian->IsModified() == true) {
    m_GUIData->SetSourceX(m_SourceCartesian->GetAsDouble(0));
    m_GUIData->SetSourceY(m_SourceCartesian->GetAsDouble(1));
    m_GUIData->SetSourceZ(m_SourceCartesian->GetAsDouble(2));
  }

  if (m_ARM->IsModified() == true) {
    m_GUIData->SetSourceARMMin(m_ARM->GetMinValue());
    m_GUIData->SetSourceARMMax(m_ARM->GetMaxValue());
  }
  if (m_SPD->IsModified() == true) {
    m_GUIData->SetSourceSPDMin(m_SPD->GetMinValue());
    m_GUIData->SetSourceSPDMax(m_SPD->GetMaxValue());
  }

  if (m_GUIData->GetBeamUse() != ((m_UseBeam->GetState() == kButtonDown) ? true : false)) {
    m_GUIData->SetBeamUse((m_UseBeam->GetState() == kButtonDown) ? true : false);
  }

  if (m_BeamStart->IsModified() == true) {
    m_GUIData->SetBeamStartX(m_BeamStart->GetAsDouble(0));
    m_GUIData->SetBeamStartY(m_BeamStart->GetAsDouble(1));
    m_GUIData->SetBeamStartZ(m_BeamStart->GetAsDouble(2));
  }

  if (m_BeamFocalSpot->IsModified() == true) {
    m_GUIData->SetBeamFocalSpotX(m_BeamFocalSpot->GetAsDouble(0));
    m_GUIData->SetBeamFocalSpotY(m_BeamFocalSpot->GetAsDouble(1));
    m_GUIData->SetBeamFocalSpotZ(m_BeamFocalSpot->GetAsDouble(2));
  }

  if (m_BeamRadius->IsModified() == true) {
    m_GUIData->SetBeamRadius(m_BeamRadius->GetAsDouble());
    m_GUIData->SetBeamDepth(m_BeamDepth->GetAsDouble());
  }

  if (m_FirstIADistance->IsModified() == true) {
    m_GUIData->SetFirstDistanceRangeMin(m_FirstIADistance->GetMinValue());
    m_GUIData->SetFirstDistanceRangeMax(m_FirstIADistance->GetMaxValue());
  }

  if (m_IADistance->IsModified() == true) {
    m_GUIData->SetDistanceRangeMin(m_IADistance->GetMinValue());
    m_GUIData->SetDistanceRangeMax(m_IADistance->GetMaxValue());
  }

  if (m_Time->IsModified() == true) {
    m_GUIData->SetTimeRangeMin(m_Time->GetMinValue());
    m_GUIData->SetTimeRangeMax(m_Time->GetMaxValue());
  }

  if (m_GUIData->GetSpecialMode() == true) {
    if (m_TimeWalk->IsModified() == true) {
      m_GUIData->SetTimeWalkRangeMin(m_TimeWalk->GetMinValue());
      m_GUIData->SetTimeWalkRangeMax(m_TimeWalk->GetMaxValue());
    }
  }

  if (m_OpeningAnglePair->IsModified() == true) {
    m_GUIData->SetOpeningAnglePairMin(m_OpeningAnglePair->GetMinValue());
    m_GUIData->SetOpeningAnglePairMax(m_OpeningAnglePair->GetMaxValue());
  }

  if (m_InitialEnergyDepositPair->IsModified() == true) {
    m_GUIData->SetInitialEnergyDepositPairMin(m_InitialEnergyDepositPair->GetMinValue());
    m_GUIData->SetInitialEnergyDepositPairMax(m_InitialEnergyDepositPair->GetMaxValue());
  }

  // Check if the detector have been modified:
  unsigned int NSelectedDetectors = 0;
  bool DetectorsModified = false;
  for (int d = 0; d < m_DetectorList->GetNumberOfEntries(); ++d) {
    if (m_DetectorList->GetSelection(d) == true) {
      ++NSelectedDetectors;
      bool Found = false;
      for (unsigned int e = 0; e < m_GUIData->GetNExcludedDetectors(); ++e) {
        if (m_GUIData->GetExcludedDetectorAt(e) == m_Geometry->GetDetectorAt(d)->GetName()) {
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
  if (NSelectedDetectors != m_GUIData->GetNExcludedDetectors()) {
    DetectorsModified = true;
  }

  if (DetectorsModified == true) {
    m_GUIData->RemoveAllExcludedDetectors();
    for (int d = 0; d < m_DetectorList->GetNumberOfEntries(); ++d) {
      if (m_DetectorList->GetSelection(d) == true) {
        m_GUIData->AddExcludedDetector(m_Geometry->GetDetectorAt(d)->GetName());
      }
    }
  }

  m_GUIData->SetEventSelectorTab(m_MainTab->GetCurrent());

	return true;
}


// MGUIEventSelection: the end...
////////////////////////////////////////////////////////////////////////////////
