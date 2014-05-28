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
  //! Set the peak parametrization options for the fitting method
  void SetPeakParametrizationMethodFittedPeakOptions(unsigned int BackgroundModel, 
                                                     unsigned int EnergyLossModel, 
                                                     unsigned int PeakShapeModel) {
    m_PeakParametrizationMethodFittedPeakBackgroundModel = BackgroundModel; 
    m_PeakParametrizationMethodFittedPeakEnergyLossModel = EnergyLossModel; 
    m_PeakParametrizationMethodFittedPeakPeakShapeModel = PeakShapeModel; }

  //! Set the calibration model determination method 
  void SetCalibrationModelDeterminationMethod(unsigned int Method) { m_CalibrationModelDeterminationMethod = Method; }
  //! Set the fitting model options for the calibration model determination method
  void SetCalibrationModelDeterminationMethodFittingOptions(unsigned int Model) { 
    m_CalibrationModelDeterminationMethodFittingModel = Model; }

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
    
  //! ID for a step-wise interpolated calibration model
  static const unsigned int c_CalibrationModelStepWise = 0;
  //! ID for a fitted interpolated calibration model
  static const unsigned int c_CalibrationModelFit = 1;
  //! ID for a fitted interpolated calibration model chosen as the best fit from a selection
  static const unsigned int c_CalibrationModelBestFit = 2;

  
  // protected methods:
 protected:
  //! Step 1: Find the peaks in this read-out data group
  bool FindPeaks(unsigned int ROGID);
  
  //! Step 2: Fit the peaks in this read-out data group
  bool FitPeaks(unsigned int ROGID);

  //! Step 3: Assign energies
  bool AssignEnergies();
  
  //! Step 4: Find best fit to all data points
  bool DetermineModels();
  
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
  //! The background model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakBackgroundModel; 
  //! The energy loss model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakEnergyLossModel; 
  //! The peak shape model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakPeakShapeModel;
 
  //! The calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethod;
  //! Fitting model of the calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethodFittingModel;
  
  
  //! Store the calibration results
  MCalibrationSpectrum m_Results;

#ifdef ___CINT___
 public:
  ClassDef(MCalibrateLines, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
