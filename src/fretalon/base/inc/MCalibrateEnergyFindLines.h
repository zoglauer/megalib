/*
 * MCalibrateEnergyFindLines.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
  
  //! Set the selected temperature window
  void SetTemperatureWindow(double Min, double Max) { m_TemperatureMin = Min; m_TemperatureMax = Max; }
  
  //! Set the prior
  void SetBayesianBlockPrior(unsigned int Prior) { m_Prior = Prior; }

  //! Set the number of bins which are excluded from peak finding at low energies
  //! to avoid noise peaks
  void SetNumberOfExcludedBinsAtLowEnergies(unsigned int Bins) { m_ExcludeFirstNumberOfBins = Bins; }

  //! Set the minimum number of counts in accepted peaks
  void SetMinimumNumberOfPeakCounts(unsigned int Counts) { m_MinimumPeakCounts = Counts; }

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
  
  //! Step 3: Do a final check of all peaks
  bool CheckPeaks();
  
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

  
  //! The selected temperature window minimum
  double m_TemperatureMin;
  //! The selected temperature window maximum
  double m_TemperatureMax;
  
  // User adjustable flags

  //! The prior for the Bayesian block binning
  unsigned int m_Prior;
  //! Ignore peaks found in the first X number of bins
  unsigned int m_ExcludeFirstNumberOfBins;
  //! Minimum number of counts in accepted peak
  unsigned int m_MinimumPeakCounts;


#ifdef ___CLING___
 public:
  ClassDef(MCalibrateEnergyFindLines, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
