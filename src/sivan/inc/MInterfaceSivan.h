/*
 * MInterfaceSivan.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInterfaceSivan__
#define __MInterfaceSivan__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MInterface.h"
#include "MSettingsSivan.h"
#include "MDGeometryQuest.h"

// Forward declarations:
class MGUISivanMain;


////////////////////////////////////////////////////////////////////////////////


class MInterfaceSivan : public MInterface
{
  // Public Interface:
 public:
  MInterfaceSivan();
  virtual ~MInterfaceSivan();

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

  void AnalyzeSimEventsOrig();
  void AnalyzeSimEvents(bool UseIdealEvent = false);        

  void ComptonEnergyEscape();
  void IncidenceAngle();
  void IncidenceEnergy();
  void IncidenceVsMeasuredEnergy();
  void StartLocations();
  void InitialComptonScatterAngle();
  void EnergyOfSecondaries();
  void AverageNumberOfHits();
  void LocationOfFirstDetectedInteraction();
  void LocationsOfAllDetectedInteractions();
  void EnergyLossByMaterial();
  void MissingInteractionsStatistics();
  void CompleteAbsorptionRatio();
  void DopplerArm();
  void NInteractions();
  void InteractionDetectorSequence();
  void ChargeSharing();

  void ViewHits();
  void EnergyLoss();
  void CreateResponse();
  void TrackLengthVersusEnergy();
  void AverageEnergyLossInFirstLayer();
  // void InputEnergy();
  void EnergyPerVoxel();
  // void MultipleComptons();
  void DopplerBroadening();
  void SumDeposits();
  void PitchAnalysis();
  void RetrieveCosima();
  void ChanceCoincidences();
  void Moliere();
  void EnergySpectrum();
  void HitsPerEnergy();
  void InteractionsPerVoxel();
  
  //! Shows stacked spectra, one per isotope generating the enrgy deposits
  void EnergyPerNucleus();
  //! Shows a 2D histogram of the produced isotopes
  void IsotopeGeneration();
  
  //! Shows a table of initial interaction vs detector type
  void InitialInteraction();
  
  void SecondaryGenerationPattern();

  void TriggerPatternEfficiency();
  void BeamMonitorEfficiency();

  void TestOnly();

  // protected methods:
 protected:
  void InitializeSimEventLoader();

  // Find the minimum and maximum start energy in the sims file
  void FindMinimumAndMaximumStartEnergy(double& Min, double& Max, unsigned int NEventsToCheck = 1000); 
   
  double ComptonAngle(double E1, double E2);

  // private methods:
 private:


  // protected members:
 protected:
  MSettingsSivan* m_Data;                   // All the information of the GUI
  MDGeometryQuest* m_Geometry;
 


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MInterfaceSivan, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
