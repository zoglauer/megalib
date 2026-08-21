/*
 * MCalibrationFitGaussian.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrationFitGaussian__
#define __MCalibrationFitGaussian__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MCalibrationFit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A Gaussian fit to a line profile
class MCalibrationFitGaussian : public MCalibrationFit
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationFitGaussian();
  //! Copy constructor
  MCalibrationFitGaussian(const MCalibrationFitGaussian& Fit);
  //! Default destuctor 
  virtual ~MCalibrationFitGaussian();

  //! The assignment operator
  MCalibrationFitGaussian& operator= (const MCalibrationFitGaussian& Fit);

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationFitGaussian* Clone() const;

  // Interface for ROOT fitting
  
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const;
  //! The number of parameters for ROOT fitting
  virtual unsigned int NPar() const;
  
  //! Fit the given histogram in the given range
  virtual bool Fit(TH1D& Histogram, double Min, double Max);

  //! Set the mean of the Gaussian distribution
  void SetGaussianMean(double Mean) { m_GaussianMean = Mean; }
  //! Set the height of the Gaussian distribution
  void SetGaussianHeight(double Height) { m_GaussianHeight = Height; }
  //! Set the sigma of the Gaussian distribution
  void SetGaussianSigma(double Sigma) { m_GaussianSigma = Sigma; }
  
  //! Get the peak
  virtual double GetPeak() const { return m_GaussianMean; }
  //! Get the FWHM
  virtual double GetFWHM() const { return 2.35482*m_GaussianSigma; }

  //! Return the data as parsable string
  virtual MString ToParsableString(const MString& Mode, bool WithDescriptor = false);

  // protected methods:
 protected:
  //! Set all fit parameters
  virtual void SetFitParameters(ROOT::Fit::Fitter& Fitter, TH1D& Hist, double Min, double Max);

  // private methods:
 private:



  // protected members:
 protected:
  //! Gaussian mean
  double m_GaussianMean;
  //! Gaussian height
  double m_GaussianHeight;
  //! Gaussian Sigma
  double m_GaussianSigma;


  // private members:
 private:
   


#ifdef ___CLING___
 public:
  ClassDef(MCalibrationFitGaussian, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
