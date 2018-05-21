/*
 * MCalibrateEnergyFindLines.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrateEnergyFindLines__
#define __MCalibrateEnergyFindLines__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TCanvas.h"

// MEGAlib libs:
#include "MCalibrateEnergy.h"
#include "MIsotope.h"
#include "MCalibrate.h"
#include "MCalibrationSpectrum.h"
#include "MReadOutDataGroup.h"
#include "MCalibrationSpectralPoint.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A class to find the lines in a spectrum
class MCalibrateEnergyFindLines : public MCalibrateEnergy
{
  // public interface:
 public:
  //! Default constructor
  MCalibrateEnergyFindLines();
  //! Default destuctor 
  virtual ~MCalibrateEnergyFindLines();
  
  //! Set the peak parametrization method
  void SetPeakParametrizationMethod(unsigned int Method) { m_PeakParametrizationMethod = Method; }  
  //! Set the peak parametrization options for the fitting method
  void SetPeakParametrizationMethodFittedPeakOptions(unsigned int BackgroundModel, 
                                                     unsigned int EnergyLossModel, 
                                                     unsigned int PeakShapeModel) {
    m_PeakParametrizationMethodFittedPeakBackgroundModel = BackgroundModel; 
    m_PeakParametrizationMethodFittedPeakEnergyLossModel = EnergyLossModel; 
    m_PeakParametrizationMethodFittedPeakPeakShapeModel = PeakShapeModel; }
  
  //! Perform the calibration
  virtual bool Calibrate();
  
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
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The peak parametrization method
  unsigned int m_PeakParametrizationMethod;
  //! The background model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakBackgroundModel; 
  //! The energy loss model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakEnergyLossModel; 
  //! The peak shape model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakPeakShapeModel;

  
#ifdef ___CLING___
 public:
  ClassDef(MCalibrateEnergyFindLines, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
