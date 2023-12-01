/*
 * MGUIEventSelection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEventSelection__
#define __MGUIEventSelection__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextEntry.h>
#include <TGTextBuffer.h>
#include <TObjArray.h>
#include <TGTab.h>
#include <TGListBox.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventSelections.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEEntry.h"
#include "MGUIEEntryList.h"
#include "MGUIEFileSelector.h"
#include "MDGeometryQuest.h"


////////////////////////////////////////////////////////////////////////////////


class MGUIEventSelection : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIEventSelection(const TGWindow* p, const TGWindow* main,
                     MSettingsEventSelections* Data, MDGeometryQuest* Geometry);
  virtual ~MGUIEventSelection();


  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  virtual bool OnApply();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsEventSelections* m_Settings;
  MDGeometryQuest* m_Geometry;

  int m_UseTracked;
  int m_UseNotTracked;

  TGTab* m_MainTab;

  TGCheckButton* m_ComptonCB;
  TGCheckButton* m_ComptonNotTrackedCB;
  TGCheckButton* m_ComptonTrackedCB;
  TGCheckButton* m_PairCB;
  TGCheckButton* m_PhotoCB;
  TGCheckButton* m_PETCB;
  TGCheckButton* m_MultiCB;
  TGCheckButton* m_UnidentifiableCB;
  TGCheckButton* m_DecayCB;

  TGCheckButton* m_BadCB;

  MGUIEMinMaxEntry* m_EventId;

  TGRadioButton* m_GTIAllRB;
  TGRadioButton* m_GTIEntryRB;
  MGUIEEntry* m_MinTimeEntry;
  MGUIEEntry* m_MaxTimeEntry;
  TGRadioButton* m_GTIFileRB;
  MGUIEFileSelector* m_GTIFile;
  
  TGListBox* m_FirstIADetectorList;
  TGListBox* m_SecondIADetectorList;
  
  MGUIEMinMaxEntry* m_TrackLength;
  MGUIEMinMaxEntry* m_SequenceLength;
  MGUIEMinMaxEntry* m_ClusteringQualityFactor;
  MGUIEMinMaxEntry* m_ComptonQualityFactor;
  MGUIEMinMaxEntry* m_TrackQualityFactor;
  MGUIEMinMaxEntry* m_CoincidenceWindow;
  MGUIEMinMaxEntry* m_FirstTotalEnergy;
  MGUIEMinMaxEntry* m_SecondTotalEnergy;
  MGUIEMinMaxEntry* m_ThirdTotalEnergy;
  MGUIEMinMaxEntry* m_FourthTotalEnergy;
  MGUIEMinMaxEntry* m_GammaEnergy;
  MGUIEMinMaxEntry* m_ElectronEnergy;
  MGUIEMinMaxEntry* m_ComptonAngle;
  MGUIEMinMaxEntry* m_FirstIADistance;
  MGUIEMinMaxEntry* m_IADistance;

  MGUIEMinMaxEntry* m_TimeWalk;
  MGUIEMinMaxEntry* m_OpeningAnglePair;
  MGUIEMinMaxEntry* m_InitialEnergyDepositPair;
  MGUIEMinMaxEntry* m_QualityFactorPair;

  MGUIEEntry* m_ThetaDeviationMax;

  TGRadioButton* m_EHCRBNone;
  TGRadioButton* m_EHCRBIntersection;
  TGRadioButton* m_EHCRBProbability;
  MGUIEFileSelector* m_EHCProbabilityFile;
  MGUIEEntry* m_EHCProbability;
  MGUIEEntry* m_EHCAngle;
  int m_EHCSelected;

  // Source frame
  TGCheckButton* m_UsePointSource;
  TGRadioButton* m_UseGalacticPointSource;
  MGUIEEntryList* m_SourceGalactic;
  TGRadioButton* m_UseSphericPointSource;
  MGUIEEntryList* m_SourceSpheric;
  TGRadioButton* m_UseCartesianPointSource;
  MGUIEEntryList* m_SourceCartesian;
  MGUIEMinMaxEntry* m_ARM;
  MGUIEMinMaxEntry* m_SPD;
  int m_CoordinatesSelected;


  // Pointing frame
  TGRadioButton* m_UsePointingSelectionNone;
  TGRadioButton* m_UsePointingSelectionPointSource;
  MGUIEEntryList* m_PointingPointSourceLocation;
  MGUIEEntry* m_PointingPointSourceRadius;
  TGRadioButton* m_UsePointingSelectionBox;
  MGUIEEntryList* m_PointingBoxLocation;
  MGUIEEntry* m_PointingBoxExtentLatitude;
  MGUIEEntry* m_PointingBoxExtentLongitude;
  
  
  // Beam/Box frame
  TGCheckButton* m_UseBeam;
  MGUIEEntryList* m_BeamStart;
  MGUIEEntryList* m_BeamFocalSpot;
  MGUIEEntry* m_BeamRadius;
  MGUIEEntry* m_BeamDepth;
  
  TGCheckButton* m_UseBox;
  MGUIEEntryList* m_BoxFirstIAMin;
  MGUIEEntryList* m_BoxFirstIAMax;
  MGUIEEntryList* m_BoxSecondIAMin;
  MGUIEEntryList* m_BoxSecondIAMax;



  enum ButtonIDs { c_Compton = 140,
                   c_ComptonUntracked,
                   c_ComptonTracked, 
                   c_Pair, 
                   c_Photo, 
                   c_PET, 
                   c_Multi, 
                   c_Unidentifiable,
                   c_Decay,
                   c_Bad,
                   c_GTIAll,
                   c_GTIEntry,
                   c_GTIFile,
                   c_Detectors,
                   c_EHCNone,
                   c_EHCIntersection,
                   c_EHCProbability,
                   c_UsePointSource,
                   c_UseGalacticPointSource,
                   c_UseSphericPointSource,
                   c_UseCartesianPointSource,
                   c_UsePointingSelectionNone,
                   c_UsePointingSelectionPointSource,
                   c_UsePointingSelectionBox,
                   c_UseBeam,
                   c_UseBox };


#ifdef ___CLING___
 public:
  ClassDef(MGUIEventSelection, 0) // gui window for basic event selections
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
