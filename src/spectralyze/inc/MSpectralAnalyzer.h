/*
 * MSpectralAnalyzer.h
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSpectralAnalyzer__
#define __MSpectralAnalyzer__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <stdio.h>
#include <iostream>
#include <iomanip>

using namespace std;

// ROOT libs:
#include "TH1.h"
#include "TMath.h"
#include "TMinuit.h"
#include "TSpectrum.h"
#include "TPaveLabel.h"
#include "TPaveText.h"
#include "TFrame.h"
#include "TPad.h"
#include "TPolyMarker.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MMath.h"
#include "MPeak.h"
#include "MQualifiedIsotope.h"
#include "MFitFunctions.h"
#include "MDGeometryQuest.h"
#include "MInterface.h"
#include "MSettingsSpectralOptions.h"
// #include "MEventSelector.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSpectralAnalyzer
{
  // public interface:
 public:
  //! Deafult constructor
  MSpectralAnalyzer();
  //! Default destructor
  virtual ~MSpectralAnalyzer();

  // Housekeeping
  
  //! Resets all automatically filled data sets, e.g. the initial and intermediate histograms, the results, etc.
  bool Reset();

  
  // Interface to fill the initial histogram
  
  //! Set to batch mode to avoid plotting canvases
  void SetBatch(bool IsBatch) { m_IsBatch = IsBatch; }
  
  //! Set the geometry 
  void SetGeometry(MDGeometryQuest* Geo) { m_Geometry = Geo; }
  
  //! Set the initial histogram using a TH1D Spectrum 
  bool SetSpectrum(TH1D* Spectrum);
  //! Set the initial spectrum gradually by filling it with individual energies
  bool SetSpectrum(int NBins, double EnergyMin, double EnergyMax, int BinningMode = 2);

  //! Fills an existing (!) spectrum with the events from the given file
  // bool FillSpectrum(MString FileName, MEventSelector EventSelector);
  //! Adds the given energy to an existing (!) spectrum
  bool FillSpectrum(double Energy);
  //! Adds the given energies to an existing (!) spectrum
  bool FillSpectrum(vector<double> Energy);
  //! Resets the spectrum
  void ResetSpectrum();
  
  
  // Interface to peak finding
  //! number of bins in histogram
  //  void SetHistBinsSpectralyzer(double HistBinsSpectralyzer) {m_HistBinsSpectralyzer = HistBinsSpectralyzer; }
  //! signal to noise ratio limit
  void SetSignaltoNoiseRatio(int SignaltoNoiseRatio) { m_SignaltoNoiseRatio = SignaltoNoiseRatio; }
  //! Poisson limit on integrated peak counts
  void SetPoissonLimit(int PoissonLimit) {m_PoissonLimit = PoissonLimit; }

  //! Deconvolve peaks above specified sigma value
  //  void SetDeconvolutionLimit(double DeconvolutionLimit) {m_DeconvolutionLimit = DeconvolutionLimit; }
  
  // Interface to isotope identification  
  //! Name of candidate isotope
  bool SetIsotopeFileName(MString IsotopeFileName);
  //! Range for candidate isotopes (+/- found peak value)
  void SetEnergyRange(double EnergyRange) { m_EnergyRange = EnergyRange; }

  
  // Interface to run the analysis

  //! Do all the analysis and find the peaks & isotopes
  bool FindIsotopes();
  
  
  // Interface to results
  
  //! Return the number of found peaks
  unsigned int GetNPeaks() const { return m_Peaks.size(); }
  //! Return the list of peaks
  vector<MPeak*> GetPeaks() { return m_Peaks; }
  
  //! Return the number of found isotopes
  unsigned int GetNIsotopes() const { return m_Isotopes.size(); }
  //! Return a COPY of the list of isotopes
  vector<MQualifiedIsotope> GetIsotopes();
  

  
  // protected methods:
 protected:
  //MSpectralAnalyzer() {};
  //MSpectralAnalyzer(const MSpectralAnalyzer& SpectralAnalyzer) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The geometry
  MDGeometryQuest* m_Geometry;

  //! The initial spectrum
  TH1D* m_InitialSpectrum;

  //! Are we in batch mode
  bool m_IsBatch;
  
  // Peak finder options:
  
  //! SNR
  int m_SignaltoNoiseRatio;
  //! Poisson Limit
  int m_PoissonLimit;
  
  //! deconvolve peaks woth sigma > deconvolution limit
  //  double m_DeconvolutionLimit;
  
  //! Short explanation
  MString m_IsotopeFileName;
  //! # (in sigma) away from found peak for acceptable isotope energy match
  double m_EnergyRange;
  
  // Results:
  
  //! the list of found peaks
  vector<MPeak*> m_Peaks;
  
  //! the list of found isotopes
  vector<MQualifiedIsotope*> m_Isotopes;
  
  //! the list of found isotopes
  vector<MQualifiedIsotope*> m_ComparisonIsotopes;


#ifdef ___CLING___
 public:
  ClassDef(MSpectralAnalyzer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
