/*
 * MInterfaceMimrec.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInterfaceMimrec__
#define __MInterfaceMimrec__


////////////////////////////////////////////////////////////////////////////////


// standard libs

// ROOT libs
#include <TSystem.h>
#include <TTime.h>
#include <TArrayD.h>
#include <TPaveLabel.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h> 

// MEGAlib libs:
#include "MGlobal.h"
#include "MInterface.h"
#include "MSettingsMimrec.h"
#include "MSettingsSpectralOptions.h"
#include "MImager.h"
#include "MSystem.h"
#include "MGUIProgressBar.h"
#include "MMath.h"
#include "MImage.h"
#include "MDGeometryQuest.h"

// Forward declarations:
class MGUIMimrecMain;


////////////////////////////////////////////////////////////////////////////////


class MInterfaceMimrec : public MInterface
{
  // Public Interface:
 public:
  MInterfaceMimrec();
  virtual ~MInterfaceMimrec();

  //! Each interface must be able to parse a command line - 
  //! this function is called by main()
  virtual bool ParseCommandLine(int argc, char** argv);

  //! Set the geometry file name and initialize the geometry
  //! Returns true if successful
  virtual bool SetGeometry(MString FileName, bool UpdateGui = true);

  //! Load the configuration file
  virtual bool LoadConfiguration(MString FileName);
  //! Save the configuration file
  virtual bool SaveConfiguration(MString FileName);

  //! Return the test position for ARM calculations in Cartesian coordinates
  MVector GetTestPosition();

  //! Return the minimum energy from the energy windows
  double GetTotalEnergyMin();
  //! Return the maximum energy from the energy windows
  double GetTotalEnergyMax();

  //! Perform the default image reconstruction
  void Reconstruct(bool Animate = false);
  //! Create a significance map
  void SignificanceMap();
  
  void ShowEventSelections();
  void ShowEventSelectionsStepwise();
  void ExtractEvents();

  void ThetaOriginDistribution();

  //! The ARM gamma distribution calculated in unbinned mode
  void ARMGamma();
  //! The classic ARM gamma calculation
  void ARMGammaClassic();
  //! Distribution of ARM values of the gamma-ray vs. the Compton scatter angle
  void ARMGammaVsComptonScatterAngle();
  //! 2D Histogram of Phi Calculated via kinemetics vs. Phi calculated via geometry
  void PhiKinVsPhiGeo();
  void ARMGammaVsDistance();
  void ARMGammaVsComptonProbability();
  void ARMGammaVsClusteringProbability();
  void SPDVsTrackQualityFactor();
  void SPDVsTotalScatterAngleDeviation();
  void ComptonProbabilityWithARMSelection();
  void EnergyVsComptonProbability();
  void ComptonSequenceLengthVsComptonProbability();
  void ClusteringQualityFactor();
  void ComptonQualityFactor();
  void TrackQualityFactor();
  void SPDElectron();
  void SPDElectronVsCompton();
  void ARMElectron();
  void DualARM();
  void ARMResponseComparison();
  void AngularResolutionPairs();
  void AngularResolutionVsQualityFactorPair();
  //! The resolution measure for PET events
  void ResolutionMeasurePET();

  void EnergySpectra();
  
  //! The spectral analyzer
  void SpectralAnalyzer();
  
  //! Performs a standard analysis
  void StandardAnalysis(double Energy, MVector Position);
  
  void InitialEnergyDeposit();
  void EnergyDistributionD2();
  //! Show the light curve, i.e. distribution
  void LightCurve();
  void CoincidenceWindowDistribution();
  void TimeWalkDistribution();
  void TimeWalkArmDistribution();
  void LocationOfFirstIA();
  void AzimuthalElectronScatterAngle();
  void AzimuthalComptonScatterAngle();
  void Polarization();
  void DOM();
  void TestOnly();
  void ScatterAnglesDistribution();
  void DistanceDistribution();
  void EarthCenterDistance();
  void EnergyDistributionElectronPhoton();
  void OpeningAnglePair();
  void SequenceLengths();
  void LocationOfInitialInteraction();
  void DirectionScatteredGammaRay();

  // Show the pointing (z-axis) of the instrument in galactic coordinates
  void PointingInGalacticCoordinates();

  // Create an orientstion file for cosima from an existing observation
  void CreateCosimaOrientationFile();

  // Show the horizon zenith in spherical detector coordinates
  void HorizonInSphericalDetectorCoordinates();
  
  //! Create an exposure map
  void CreateExposureMap();
  
  void SelectIds();

  // Not used:
  void InteractionDepth();

  // protected methods:
 protected:
  //! Intialize the event loader
  bool InitializeEventLoader(MString File = "");
  //! Get the next event
  MPhysicalEvent* GetNextEvent(bool Checks = false);
  //! Finalize the event loader
  void FinalizeEventLoader();

private:

  // private methods:
 private:


  // protected members:
 protected:
  //! All the settings from the UI
  MSettingsMimrec* m_Settings;
  
  //! The default used event file
  MFileEventsTra* m_EventFile;
  //! The default used event selector
  MEventSelector* m_Selector;

  //! The image reconstructor
  MImager* m_Imager;

  //! In automatic mode, save the canvas to this file
  MString m_OutputFileName;

  // private members:
 private:

#ifdef ___CLING___
 public:
  ClassDef(MInterfaceMimrec, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
