/*
 * MEventSelector.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEventSelector__
#define __MEventSelector__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MCoordinateSystem.h"
#include "MString.h"
#include "MTime.h"
#include "MGTI.h"
#include "MPhysicalEvent.h"
#include "MEarthHorizon.h"
#include "MDGeometryQuest.h"
#include "MSettingsEventSelections.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MEventSelector
{
  // public interface:
 public:
  //! Empty default constructor - all selections are initialized with default values
  MEventSelector();
  //! Copy constructor
  MEventSelector(const MEventSelector& EventSelector);
  //! Default (empty) destructor
  virtual ~MEventSelector();

  //! Default assignment constructor
  const MEventSelector& operator=(const MEventSelector& EventSelector);

  //! Reset the STATISTICS data only
  void Reset();

  //! Retrieve the relevant setting from the settings class
  void SetSettings(MSettingsEventSelections* S);

  
  //! Check if the event fullfills all event selection criteria
  //! This is the detailed version which is output capable and stores 
  //! which criteria trigger on the event (i.e. fills the statistics)
  bool IsQualifiedEvent(MPhysicalEvent* P, bool DumpOutput = false);
  //! The same as IsQualifiedEvent, it only immdiately return false if one clause is wrong
  //! In addition the test sequence is more optimally arranged
  //! This version does not fill the event selection statistics!
  bool IsQualifiedEventFast(MPhysicalEvent* P);
  //! Check if the direction is within the ARM window. 
  //! Direction is relative to the source position of this selector
  bool IsDirectionWithinARMWindow(MVector Direction);

  MString ToString();

  // Global:
  void SetGeometry(MDGeometryQuest* Geometry);
  void ApplyGeometry(MEventSelector& E) { E.SetGeometry(m_Geometry); }
  void SetFirstTotalEnergy(double Min = 0, double Max = 1e+20);
  void ApplyFirstTotalEnergy(MEventSelector& E) { E.SetFirstTotalEnergy(m_FirstTotalEnergyMin, m_FirstTotalEnergyMax); }
  void SetSecondTotalEnergy(double Min = 0, double Max = 1e+20);
  void ApplySecondTotalEnergy(MEventSelector& E) { E.SetSecondTotalEnergy(m_SecondTotalEnergyMin, m_SecondTotalEnergyMax); }
  void SetThirdTotalEnergy(double Min = 0, double Max = 1e+20);
  void ApplyThirdTotalEnergy(MEventSelector& E) { E.SetThirdTotalEnergy(m_ThirdTotalEnergyMin, m_ThirdTotalEnergyMax); }
  void SetFourthTotalEnergy(double Min = 0, double Max = 1e+20);
  void ApplyFourthTotalEnergy(MEventSelector& E) { E.SetFourthTotalEnergy(m_FourthTotalEnergyMin, m_FourthTotalEnergyMax); }
  void SetTimeWalk(double Min, double Max);
  void ApplyTimeWalk(MEventSelector& E) { E.SetTime(m_TimeWalkMin, m_TimeWalkMax); }
  void SetExcludedDetectors(vector<MString> ExcludedDetectors);
  void ApplyExcludedDetectors(MEventSelector& E) { E.SetExcludedDetectors(m_ExcludedDetectors); }

  void SetTimeUseFile(bool TimeUseFile);
  void ApplyTimeUseFile(MEventSelector& E) { E.SetTimeUseFile(m_TimeUseFile); }
  void SetTime(const MTime& Min, const MTime& Max);
  void ApplyTime(MEventSelector& E) { E.SetTime(m_TimeMin, m_TimeMax); }
  void SetTimeFile(MString TimeFile);
  void ApplyTimeFile(MEventSelector& E) { E.SetTimeFile(m_TimeFile); }
  
  
  void SetSourceWindow(bool Use) { m_UseSource = false; }
  void SetSourceWindow(bool Use, MVector SourcePosition, MCoordinateSystem CS);
  void ApplySourceWindow(MEventSelector& E) { E.SetSourceWindow(m_UseSource, m_SourcePosition, m_SourceCoordinateSystem); }
  void SetSourceARM(double ARMMin = 0.0, double ARMMax = 180.0);
  void ApplySourceARM(MEventSelector& E) { E.SetSourceARM(m_ARMMin, m_ARMMax); }
  void SetSourceSPD(double SPDMin = 0.0, double SPDMax = 180.0);
  void ApplySourceSPD(MEventSelector& E) { E.SetSourceSPD(m_SPDMin, m_SPDMax); }

  void SetBeam(bool Use, MVector BeamStart, MVector BeamFocalSpot);
  void ApplyBeam(MEventSelector& E) { E.SetBeam(m_UseBeam, m_BeamStart, m_BeamFocalSpot); }
  void SetBeamRadius(double Radius);  
  void ApplyBeamRadius(MEventSelector& E) { E.SetBeamRadius(m_BeamRadius); }
  void SetBeamDepth(double Depth);  
  void ApplyBeamDepth(MEventSelector& E) { E.SetBeamDepth(m_BeamDepth); }

  // Compton specific:
  void UseComptons(bool Comptons = true);
  void ApplyUseComptons(MEventSelector& E) { E.UseComptons(m_UseComptons); }
  void UseTrackedComptons(bool Comptons = true);
  void ApplyUseTrackedComptons(MEventSelector& E) { E.UseTrackedComptons(m_UseTrackedComptons); }
  void UseNotTrackedComptons(bool Comptons = true);
  void ApplyUseNotTrackedComptons(MEventSelector& E) { E.UseNotTrackedComptons(m_UseNotTrackedComptons); }

  void SetElectronEnergy(double Min = 0, double Max = 1e+20);
  void ApplyElectronEnergy(MEventSelector& E) { E.SetElectronEnergy(m_ElectronEnergyMin, m_ElectronEnergyMax); }
  void SetGammaEnergy(double Min = 0, double Max = 1e+20);
  void ApplyGammaEnergy(MEventSelector& E) { E.SetGammaEnergy(m_GammaEnergyMin, m_GammaEnergyMax); }
  void SetComptonAngle(double Min = 0, double Max = 180);
  void ApplyComptonAngle(MEventSelector& E) { E.SetComptonAngle(m_ComptonAngleMin, m_ComptonAngleMax); }
  void SetFirstDistance(double Min = 0, double Max = 1e+20);
  void ApplyFirstDistance(MEventSelector& E) { E.SetFirstDistance(m_FirstLeverArmMin, m_FirstLeverArmMax); }
  void SetDistance(double Min = 0, double Max = 1e+20);
  void ApplyDistance(MEventSelector& E) { E.SetDistance(m_LeverArmMin, m_LeverArmMax); }
  void SetSequenceLength(int Min = 2, int Max = 1000);
  void ApplySequenceLength(MEventSelector& E) { E.SetSequenceLength(m_SequenceLengthMin, m_SequenceLengthMax); }
  void SetTrackLength(int Min = 2, int Max = 1000);
  void ApplyTrackLength(MEventSelector& E) { E.SetTrackLength(m_TrackLengthMin, m_TrackLengthMax); }
  void SetEarthHorizonCut(const MEarthHorizon& EH);
  void ApplyEarthHorizonCut(MEventSelector& E) { E.SetEarthHorizonCut(m_EarthHorizon); }
  void SetEarthHorizonCutAngle(double Angle = 90);
  void SetThetaDeviationMax(double Distance = 180);
  void ApplyThetaDeviationMax(MEventSelector& E) { E.SetThetaDeviationMax(m_ThetaDeviationMax); }
  void SetClusteringQualityFactor(double Min = 0, double Max = 1);
  void ApplyClusteringQualityFactor(MEventSelector& E) { E.SetClusteringQualityFactor(m_ClusteringQualityFactorMin, m_ClusteringQualityFactorMax); }
  void SetComptonQualityFactor(double Min = 0, double Max = 1);
  void ApplyComptonQualityFactor(MEventSelector& E) { E.SetComptonQualityFactor(m_ComptonQualityFactorMin, m_ComptonQualityFactorMax); }
  void SetTrackQualityFactor(double Min = 0, double Max = 1);
  void ApplyTrackQualityFactor(MEventSelector& E) { E.SetTrackQualityFactor(m_TrackQualityFactorMin, m_TrackQualityFactorMax); }
  void SetCoincidenceWindow(double Min = 0, double Max = 1);
  void ApplyCoincidenceWindow(MEventSelector& E) { E.SetCoincidenceWindow(m_CoincidenceWindowMin, m_CoincidenceWindowMax); }
  void SetEventId(int Min = 2, int Max = 10);
  void ApplyEventId(MEventSelector& E) { E.SetEventId(m_EventIdMin, m_EventIdMax); }


  // Pair specific:
  void UsePairs(bool Pairs = true);
  void ApplyUsePairs(MEventSelector& E) { E.UsePairs(m_UsePairs); }
  void SetOpeningAnglePair(double Min = 0, double Max = 1e+20);
  void ApplyOpeningAnglePair(MEventSelector& E) { E.SetOpeningAnglePair(m_OpeningAnglePairMin, m_OpeningAnglePairMax); }
  void SetInitialEnergyDepositPair(double Min = 0, double Max = 1e+20);
  void ApplyInitialEnergyDepositPair(MEventSelector& E) { E.SetInitialEnergyDepositPair(m_InitialEnergyDepositPairMin, m_InitialEnergyDepositPairMax); }
  void SetPairQualityFactor(double Min = 0, double Max = 1);
  void ApplyPairQualityFactor(MEventSelector& E) { E.SetPairQualityFactor(m_PairQualityFactorMin, m_PairQualityFactorMax); }

  // Photo specific:
  void UsePhotos(bool Photos = true);
  void ApplyUsePhotos(MEventSelector& E) { E.UsePhotos(m_UsePhotos); }

  // Unidentifiable specific:
  void UseUnidentifiables(bool Unidentifiables = true);
  void ApplyUseUnidentifiables(MEventSelector& E) { E.UseUnidentifiables(m_UseUnidentifiables); }

  // All:
  void UseDecays(bool Decays = true);
  void ApplyUseDecays(MEventSelector& E) { E.UseDecays(m_UseDecays); }

  void UseFlaggedAsBad(bool FlaggedAsBad = true);
  void ApplyFlaggedAsBad(MEventSelector& E) { E.UseFlaggedAsBad(m_UseFlaggedAsBad); }


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MDGeometryQuest* m_Geometry;

  double m_FirstTotalEnergyMin;
  double m_FirstTotalEnergyMax;
  double m_SecondTotalEnergyMin;
  double m_SecondTotalEnergyMax;
  double m_ThirdTotalEnergyMin;
  double m_ThirdTotalEnergyMax;
  double m_FourthTotalEnergyMin;
  double m_FourthTotalEnergyMax;

  bool m_TimeUseFile;
  MTime m_TimeMin;
  MTime m_TimeMax;
  MString m_TimeFile;
  MGTI m_TimeGTI;
  
  double m_TimeWalkMin;
  double m_TimeWalkMax;

  double m_ElectronEnergyMin;
  double m_ElectronEnergyMax;
  double m_GammaEnergyMin;
  double m_GammaEnergyMax;
  double m_ComptonAngleMin;
  double m_ComptonAngleMax;
  double m_LeverArmMin;
  double m_LeverArmMax;
  double m_FirstLeverArmMin;
  double m_FirstLeverArmMax;
  unsigned int m_SequenceLengthMin;
  unsigned int m_SequenceLengthMax;
  unsigned int m_TrackLengthMin;
  unsigned int m_TrackLengthMax;
  double m_ClusteringQualityFactorMin;
  double m_ClusteringQualityFactorMax;
  double m_ComptonQualityFactorMin;
  double m_ComptonQualityFactorMax;
  double m_TrackQualityFactorMin;
  double m_TrackQualityFactorMax;
  double m_CoincidenceWindowMin;
  double m_CoincidenceWindowMax;
  unsigned int m_EventIdMin;
  unsigned int m_EventIdMax;
  double m_ThetaDeviationMax;
  MEarthHorizon m_EarthHorizon;

  double m_InitialEnergyDepositPairMin;
  double m_InitialEnergyDepositPairMax;
  double m_OpeningAnglePairMin;
  double m_OpeningAnglePairMax;
  double m_PairQualityFactorMin;
  double m_PairQualityFactorMax;
  
  bool m_UseSource;
  MVector m_SourcePosition;
  MCoordinateSystem m_SourceCoordinateSystem;
  double m_ARMMin;
  double m_ARMMax;
  double m_SPDMin;
  double m_SPDMax;

  bool m_UseBeam;
  MVector m_BeamStart;
  MVector m_BeamFocalSpot;
  double m_BeamRadius;
  double m_BeamDepth;

  bool m_UsePhotos;
  bool m_UsePairs;
  bool m_UseComptons;
  bool m_UseTrackedComptons;
  bool m_UseNotTrackedComptons;
  bool m_UseUnidentifiables;

  bool m_UseDecays;
  bool m_UseFlaggedAsBad;

  vector<MString> m_ExcludedDetectors;

  int m_NAnalyzed;
  int m_NAccepted;
  int m_NRejectedIsGood;
  int m_NRejectedStartDetector;
  int m_NRejectedTotalEnergy;
  int m_NRejectedTime;
  int m_NRejectedTimeWalk;
  int m_NRejectedElectronEnergy;
  int m_NRejectedGammaEnergy;
  int m_NRejectedComptonAngle;
  int m_NRejectedFirstLeverArm;
  int m_NRejectedLeverArm;
  int m_NRejectedEventId;
  int m_NRejectedTrackLength;
  int m_NRejectedSequenceLength;
  int m_NRejectedClusteringQualityFactor;
  int m_NRejectedComptonQualityFactor;
  int m_NRejectedTrackQualityFactor;
  int m_NRejectedCoincidenceWindow;
  int m_NRejectedEarthHorizonCut;
  int m_NRejectedThetaDeviationMax;
  int m_NRejectedUsePhotos;
  int m_NRejectedUsePairs;
  int m_NRejectedUseComptons;
  int m_NRejectedUseMuons;
  int m_NRejectedUseDecays;
  int m_NRejectedUseUnidentifiables;
  int m_NRejectedUseFlaggedAsBad;
  int m_NRejectedUseTrackedComptons;
  int m_NRejectedUseNotTrackedComptons;
  int m_NRejectedOpeningAnglePair;
  int m_NRejectedInitialEnergyDepositPair;
  int m_NRejectedPairQualityFactor;
  int m_NRejectedARM;
  int m_NRejectedSPD;
  int m_NRejectedBeam;

  friend ostream& operator<<(ostream& os, MEventSelector& S);

#ifdef ___CINT___
 public:
  ClassDef(MEventSelector, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////

// Global scope:
ostream& operator<<(ostream& os, MEventSelector& S);


////////////////////////////////////////////////////////////////////////////////
