/*
 * MCalibrationSpectralFit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrationSpectralFit__
#define __MCalibrationSpectralFit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TH1.h"
#include "TF1.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a fit to a line
class MCalibrationSpectralFit
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationSpectralFit();
  //! Copy constructor
  MCalibrationSpectralFit(const MCalibrationSpectralFit& CalibrationSpectralFit);
  //! Default destuctor 
  virtual ~MCalibrationSpectralFit();

  //! The assignment operator
  virtual MCalibrationSpectralFit& operator= (const MCalibrationSpectralFit& CalibrationSpectralFit);

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationSpectralFit* Clone() const;
  
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { return 0; }

  //! Return true, if the fit is up-to-date
  bool IsFitUpToDate() const { return m_IsFitUpToDate; }
  
  //! Fit the given histogram in the given range
  virtual bool Fit(TH1D& Histogram, double Min, double Max) { return false; }
  
  //! Get the peak
  virtual double GetPeak() const { return 0.0; }
  
  //! Mimic ROOT Draw functionality
  void Draw(MString Options);
  
  //! Set the background model
  //! If it doesn't exist don't use any
  void SetBackgroundModel(unsigned int BackgroundModel); 
  
  //! Background model: No background model
  static const unsigned int c_BackgroundModelNone = 0;
  //! Background model: Flat fit
  static const unsigned int c_BackgroundModelFlat = 1;
  //! Background model: Linear fit
  static const unsigned int c_BackgroundModelLinear = 2;
  
  
  //! Set the energy loss model
  //! If it doesn't exist don't use any
  void SetEnergyLossModel(unsigned int EnergyLossModel); 

  //! Energy loss model: None
  static const unsigned int c_EnergyLossModelNone = 0;
  //! Energy loss model: Gaussian-convolved delta function
  static const unsigned int c_EnergyLossModelGaussianConvolvedDeltaFunction = 1;
  
  // protected methods:
 protected:
  
  // private methods:
 private:



  // protected members:
 protected:
  //! All used fit functions
  vector<void(*)()> m_FitFunctions; 
  //! Number of parameters of the used fit functions
  vector<unsigned int> m_NumberOfParameters
  
  //! The data points
  vector<double> m_ADCValues;
  //! The data points
  vector<double> m_EnergyValues;
  
  
  
  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MCalibrationSpectralFit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
