/*
 * MCalibrationFit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrationFit__
#define __MCalibrationFit__


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
class MCalibrationFit
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationFit();
  //! Copy constructor
  MCalibrationFit(const MCalibrationFit& CalibrationFit);
  //! Default destuctor 
  virtual ~MCalibrationFit();

  //! The assignment operator
  virtual MCalibrationFit& operator= (const MCalibrationFit& CalibrationFit);

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationFit* Clone() const;
  
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { return 0; }

  //! Return true, if the fit is up-to-date
  bool IsFitUpToDate() const { return m_IsFitUpToDate; }
  
  //! Fit the given histogram in the given range
  virtual bool Fit(TH1D& Histogram, double Min, double Max) { return false; }
  
  //! Get the peak
  virtual double GetPeak() const { return 0.0; }
  
  //! Mimic ROOT Draw functionality
  void Draw(MString Options = "");
  
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
  //! Return the number of fit parameters in the background fit
  int GetBackgroundFitParameters();
  //! Return the number of fit parameters in the energy loss fit
  int GetEnergyLossFitParameters();
  
  //! The function for fitting the background
  double BackgroundFit(double* X, double* P);
  //! The function for fitting the energy loss
  double EnergyLossFit(double* X, double* P);

  //! Set all fit parameters
  virtual void SetFitParameters(TH1D& Hist, double Min, double Max);
  
  // private methods:
 private:



  // protected members:
 protected:
  //! The actual fit
  TF1* m_Fit;
  //! True if the fit has been performed and no parameters have been chnaged
  bool m_IsFitUpToDate;

  //! The used background model. One of c_BackgroundModelXXX
  unsigned int m_BackgroundModel;

  //! The used energy loss model. One of c_EnergyLossModelXXX
  unsigned int m_EnergyLossModel;
  
  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MCalibrationFit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
