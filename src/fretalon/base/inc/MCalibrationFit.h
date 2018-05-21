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
#include "TF1.h"
#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"
#include "Fit/BinData.h"
#include "Fit/UnBinData.h"
#include "HFitInterface.h"
#include "Fit/Fitter.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class TH1D;


////////////////////////////////////////////////////////////////////////////////


//! Class representing a fit to a line
class MCalibrationFit : public ROOT::Math::IParamFunction
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
  
  // Interface for ROOT fitting
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { return 0; }
  void SetParameters(const double *p) { m_ROOTParameters.clear(); for (unsigned int i = 0; i < NPar(); ++i) m_ROOTParameters.push_back(p[i]);  }
  const double* Parameters() const { return &m_ROOTParameters[0]; }
  virtual unsigned int NPar() const { return GetBackgroundFitParameters() + GetEnergyLossFitParameters(); }

  //! Return true, if the fit is up-to-date
  bool IsFitUpToDate() const { return m_IsFitUpToDate; }
  
  //! Fit the given histogram in the given range
  virtual bool Fit(TH1D& Histogram, double Min, double Max) { return false; }
  
  //! Get the peak
  virtual double GetPeak() const { return 0.0; }
  //! Get the FWHM
  virtual double GetFWHM() const { return -1.0; }
  
  //! Get a value at the given x
  virtual double Evaluate(double X) const;
  
  //! Get the reduced chi square
  double GetReducedChisquare() const { return IsFitUpToDate() ? m_Fit->GetChisquare()/m_Fit->GetNDF() : 0; }
  
  //! Get the average deviation between fit and histogram in percent
  double GetAverageDeviation() const { return m_AverageDeviation; }
  
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
  //! Energy loss model: Gaussian-convolved delta function
  static const unsigned int c_EnergyLossModelGaussianConvolvedDeltaFunctionWithExponentialDecay = 2;
  
  // This is a bit unfortunate here, but a list of all available peak fitting algos helps a lot
  //! Peak shape: None
  static const unsigned int c_PeakShapeModelNone = 0;
  //! Peak shape: Gaussian
  static const unsigned int c_PeakShapeModelGaussian = 1;
  //! Peak shape: Gauss-Landau
  static const unsigned int c_PeakShapeModelGaussLandau = 2;
  
  
  // protected methods:
 protected:
  //! Return the number of fit parameters in the background fit
  int GetBackgroundFitParameters() const;
  //! Return the number of fit parameters in the energy loss fit
  int GetEnergyLossFitParameters() const;
  
  //! The function for fitting the background
  double BackgroundFit(double X, const double* P) const;
  //! The function for fitting the energy loss
  double EnergyLossFit(double X, const double* P) const;
  
  //! Set all fit parameters
  virtual void SetFitParameters(ROOT::Fit::Fitter& Fitter, TH1D& Hist, double Min, double Max);
  
  // private methods:
 private:



  // protected members:
 protected:
  //! The actual fit
  TF1* m_Fit;
  //! True if the fit has been performed and no parameters have been chnaged
  bool m_IsFitUpToDate;

  //! The average percentual deviation between histogram and fit -- good values are around 1
  double m_AverageDeviation;
  
  //! The used background model. One of c_BackgroundModelXXX
  unsigned int m_BackgroundModel;
  //! The used energy loss model. One of c_EnergyLossModelXXX
  unsigned int m_EnergyLossModel;
  
  //! The parameters for the ROOT fitting
  vector<double> m_ROOTParameters;

  // private members:
 private:
  

#ifdef ___CLING___
 public:
  ClassDef(MCalibrationFit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
