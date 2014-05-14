/*
 * MCalibrateLines.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrateLines__
#define __MCalibrateLines__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TCanvas.h"

// MEGAlib libs:
#include "MIsotope.h"
#include "MCalibrate.h"
#include "MCalibrationSpectrum.h"
#include "MReadOutDataGroup.h"
#include "MCalibrationSpectralPoint.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A class to calibrate the lines in a spectrum
class MCalibrateLines : public MCalibrate
{
  // public interface:
 public:
  //! Default constructor
  MCalibrateLines();
  //! Default destuctor 
  virtual ~MCalibrateLines();
  
  //! Set the peak parametrization method
  void SetPeakParametrizationMethod(unsigned int Method) { m_PeakParametrizationMethod = Method; }

  //! Add a read-out data group and the associated isotopes
  void AddReadOutDataGroup(const MReadOutDataGroup& ROG, const vector<MIsotope>& Isotopes);
  
  //! Perform the calibration
  virtual bool Calibrate();
  
  //! Get/compile the calibration result
  virtual MCalibrationSpectrum GetCalibration();

  
  //! ID for finding the peak parameters via Bayesian blocks
  static const unsigned int c_PeakParametrizationMethodBayesianBlockPeak = 0;
  //! ID for finding the peak parameters via smoothing
  static const unsigned int c_PeakParametrizationMethodSmoothedPeak = 1;
  //! ID for finding the peak parameters via fitting
  static const unsigned int c_PeakParametrizationMethodFittedPeak = 2;
  
  
  // protected methods:
 protected:
  //! Step 1: Find the peaks in this read-out data group
  bool FindPeaks(unsigned int ROGID);
  
  //! Step 2: Fit the peaks in this read-out data group
  bool FitPeaks(unsigned int ROGID);

  //! Step 3: Assign energies
  bool AssignEnergies();
  
  //! Step 4: Find best fit to all data points
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The read-out data groups to be calibrated
  vector<MReadOutDataGroup> m_ROGs;
  //! The identified spectral points for each read-out data group
  vector<vector<MCalibrationSpectralPoint> > m_SpectralPoints;
  //! The isotopes assicoated with the read-out data groups
  vector<vector<MIsotope> > m_Isotopes;

  //! The peak parametrization method
  unsigned int m_PeakParametrizationMethod;
  
  //! Store the calibration results
  MCalibrationSpectrum m_Results;

#ifdef ___CINT___
 public:
  ClassDef(MCalibrateLines, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
