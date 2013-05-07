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
  //! Interrupt the default image reconstrcution
  void InterruptReconstruction();
  //! Create a significance map
  void SignificanceMap();
  
	void ShowEventSelections();
	void ShowEventSelectionsStepwise();
	void ExtractEvents();

  void ThetaOriginDistribution();

  void ARMGamma();
  void ARMGammaVsCompton();
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
  void AngularResolutionPairs();
  void AngularResolutionVsQualityFactorPair();

  void EnergySpectra();
  
  //! The spectral analyzer
  void SpectralAnalyzer();
  
  //! Performs a standard analysis
  void StandardAnalysis(double Energy, MVector Position);
  
  void InitialEnergyDeposit();
  void EnergyDistributionD2();
  void TimeDistribution();
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

  // Show the pointing (z-axis) of the instrument in galactic coordinates
  void PointingInGalacticCoordinates();

  // Show the horizon zenith in spherical detector coordinates
  void HorizonInSphericalDetectorCoordinates();
  
  void SelectIds();

  // Not used:
  void InteractionDepth();

  // protected methods:
 protected:
  bool InitializeEventloader(MString File = "");

private:

  // private methods:
 private:


  // protected members:
 protected:
  MSettingsMimrec* m_Data;                   // All the information of the GUI

  double* m_Sensitivities;             // Sensitivity image
  double** m_SingleBackprojection;      // Single backprojection
  double* m_FirstBackprojection;       // First backprojection
  double* m_Image;                     // Image after ML-iterations

  int m_NExecutedIterations;          // Number of performed iterations
	int m_NEvents;

	bool m_ThreadAActive;

	int m_ThreadCounter;
	int m_EventCounter;

  MFileEventsTra *m_EventFile;
  MEventSelector *m_Selector;

  // private members:
 private:
  bool m_Interrupt;              // true: the precomputation has been interrupted

  MImager *m_Imager;

  MMath m_Maths;

	MImage *m_IImage;


#ifdef ___CINT___
 public:
  ClassDef(MInterfaceMimrec, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
