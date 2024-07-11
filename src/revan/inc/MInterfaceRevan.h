/*
 * MInterfaceRevan.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInterfaceRevan__
#define __MInterfaceRevan__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MInterface.h"
#include "MSettingsRevan.h"
#include "MRawEventAnalyzer.h"

// Forward declarations:
class MGeometryRevan;
class MGUIRevanMain;

////////////////////////////////////////////////////////////////////////////////


class MInterfaceRevan : public MInterface
{
  // public interface:
 public:
  MInterfaceRevan();
  virtual ~MInterfaceRevan();

  //! Each interface must be able to parse a command line - 
  //! this function is called by main()
  virtual bool ParseCommandLine(int argc, char** argv);

  //! Set the geometry file name and initialize the geometry
  //! Returns true if successful
  virtual bool SetGeometry(MString FileName, bool UpdateGui = true);
  //! Returns the geometry or zero if there is none
  virtual MDGeometryQuest* GetGeometry() { return dynamic_cast<MDGeometryQuest*>(m_Geometry); }

  //! Load the configuration file
  virtual bool LoadConfiguration(MString FileName);
  //! Save the configuration file
  virtual bool SaveConfiguration(MString FileName);

  void SetGuiData(MRawEventAnalyzer& REA);
  bool IsInitialized();

  void AnalyzeEvents();

  //! Generate a spectra based on a wide set of parameters
  void GenerateSpectra();
  
  //! Export the spectrum 
  void ExportSpectrum();

  //! The spectral analyzer
  void SpectralAnalyzer();

  void InitializeEventView();
  void ViewEvent();
  void InitialEnergySpectrum();
  void EnergyDistribution();
  void EnergyPerCentralTrackElement();
  //! Dump some initial event statistics, i.e. average hits, average energy deposit, average channels
  void InitialEventStatistics();

  void HitStatistics();
  //! Show either the spacial distribution of energy or of the hits
  void SpatialDistribution(bool UseEnergy);
  //! For each detector with a depth recognition (defined z-axis), show a depth profile
  void DepthProfileByDetector();
  
  void StartDistribution();
  //void ShowCSRTestStatistics();
  void EnergyPerDetector();
  void NumberOfClusters();
  void DetectorTypeClusterDistribution(bool Before = true);
  void DetectorTypeHitDistribution(bool Before = true);

  void Coincidence();

  void TriggerStatistics();

  void FindBeamPath();
  void FindPolarization();

  void Test();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MGUIRevanMain* m_Gui;
  MSettingsRevan* m_Data; // All the information of the GUI

  MRawEventAnalyzer* m_Analyzer;
  MGeometryRevan* m_Geometry;

  //! True if we perform a test run
  bool m_TestRun;

  //! Default output file name
  MString m_OutputFilenName;

#ifdef ___CLING___
 public:
  ClassDef(MInterfaceRevan, 0) // interface to the Revan-part of MEGAlib
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
