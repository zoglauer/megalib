/*
 * MARMFitter.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MARMFitter__
#define __MARMFitter__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <random>
#include <mutex>
#include <condition_variable>
using namespace std;

// ROOT libs:
#include "TH1.h"
#include "Fit/Fitter.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MCoordinateSystem.h"
#include "MComptonEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! List of the available fit functions
enum class MARMFitFunctionID {
  c_Gauss,
  c_Lorentz,
  c_GeneralizedNormal,
  c_GaussLorentz,
  c_GaussLorentzLorentz,
  c_AsymmetricGaussLorentzLorentz,
  c_AsymmetricGaussGaussLorentzLorentz,
  c_AsymmetricGeneralizedNormalGeneralizedNormal,
  c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal
};


////////////////////////////////////////////////////////////////////////////////


//! A class to fit ARM's
class MARMFitter
{
  // public interface:
 public:
  //! Default constructor
  MARMFitter();
  //! Default destuctor 
  virtual ~MARMFitter();

  // Interface to set up the fitter

  //! Set the number of bins for the ARM fit hisogram
  //! The deafult is 101
  void SetNumberOfBins(unsigned int NumberOfBins = 101);

  //! Set the maximum range of the ARM values
  //! The maximum value is 180
  //! The histogram will go from [-MaxARM, MaxARM]
  //! The defaulkt is 180
  void SetMaxARM(double MaxARM = 180);

  //! Set the test position, and its coordinate system
  //! Do not call this function while adding events
  void SetTestPosition(MVector TestPosition, MCoordinateSystem);

  //! If this is set we try to optimize the binning
  //! Especially if we have too many bins we iterate until we have at least 50 counts in one bin - if possible
  void UseOptimizedBinning(bool DoIt = true) { m_OptimizeBinning = DoIt; }

  //! If this is set we use an unbinned-likelihood fit
  void UseUnbinnedFitting(bool DoIt = true) { m_UnbinnedFitting = DoIt; }

  //! Fit this function to the data
  //! One of: c_Gauss, c_Lorentz, c_GaussLorentz, c_GaussDoubleLorentz, c_AsymmetricGaussDoubleLorentz
  //! If not set,  default is c_AsymmetricGaussDoubleLorentz
  void SetFitFunction(MARMFitFunctionID ID) { m_ARMFitFunction = ID; }

  // Interface to set the data

  //! Reset the internally stored ARM values
  void Reset();

  //! Add Event - only stores the actual ARM value
  void AddEvent(MComptonEvent* Event);
  //! Add an ARM value
  void AddARMValue(double ARMValue) { m_OriginalARMValues.push_back(ARMValue); }

  // Interface to do the fits

  //! Perform a single fit with the chosen fit function
  bool FitOnce();

  //! Perform fits with all fit functions and report Baker-Cousin results
  bool FitAll();

  //! Performs N fits.
  //! Each fit samples randomly form the stored ARM values to determine the average width and error correctly
  bool FitMultiple(unsigned int NumberOfFits = 100);


  // Interface to retrieve the results

  //! Returns true if we have a successful fit and have the results
  bool WasFittingSuccessful() const { return m_FitSuccessful; }

  //! Return the bootstrapped FWHM'es
  vector<double> GetBootstrappedFWHMSamples() { return m_BootStrappedFWHMSamples; }

  //! Return the FWHM result
  double GetFWHM() const;

  //! Return the FWHM uncertainty result
  double GetFWHMUncertainty() const;

  //! Return a summary string
  MString ToString();

  //! Draw an ARM histogram
  void Draw();



  // protected methods:
 protected:
  //! Optimize binning - check sif the binning is OK, otherwise check for a better one
  bool OptimizeBinning();
  //! Perform a single fit
  bool PerformFit(unsigned int FitID, vector<double>& ARMValues);
  //! Thread entry for parallel fitting
  void ParallelFitting(int id, condition_variable& CV);
  //! Bootstrap the ARM values and return them as vector
  vector<double> BootstrapARMValues();
  //! Calculate the average metrics from boot strapping (FWHM & uncertainty)
  void CalculateBootStrappedMetrics();
  //! Calculate the metrics of the ARM (containment etc.)
  void CalculateARMMetrics();

  //! Setup the fit function - ARMFitID is one of the members of the MARMFitFunctions enum
  void SetupARMFit(MARMFitFunctionID ARMFitID, ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a Gauss-function
  void SetupARMFitGauss(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a Gauss-Lorentz-function
  void SetupARMFitLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a GeneralizedNormal-function
  void SetupARMFitGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a Gauss-Lorentz-function
  void SetupARMFitGaussLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a Gauss-Double-Lorentz-function
  void SetupARMFitGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup an Assymetric-Gauss-Double-Lorentz-function
  void SetupARMFitAsymmetricGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup an Assymetric-Gauss-Double-Lorentz-function
  void SetupARMFitAsymmetricGaussGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup an Assymetric-Gauss-Double-Lorentz-function
  void SetupARMFitAsymmetricGeneralizedNormalGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup an Assymetric-Gauss-Double-Lorentz-function
  void SetupARMFitAsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);

  //! Return the function of the fit given the fit function ID
  MString GetARMFitFunction(MARMFitFunctionID ARMFitID);
  //! Return the name of the fit given the fit function ID
  MString GetARMFitFunctionName(MARMFitFunctionID ARMFitID);
  //! Return the number of parameters of the fit given the fit function ID
  unsigned int GetARMFitFunctionNumberOfParameters(MARMFitFunctionID ARMFitID);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The number of bins
  unsigned int m_NumberOfBins;
  //! The maximum ARM value
  double m_MaxARMValue;
  //! The test position
  MVector m_TestPosition;
  //! The coordinate system of the test position
  MCoordinateSystem m_CoordinateSystem;
  //! The function to use for fitting
  MARMFitFunctionID m_ARMFitFunction;

  //! Perform binning optimization
  bool m_OptimizeBinning;
  //! Use unbinned fitting
  bool m_UnbinnedFitting;

  //! Random device
  std::mt19937 m_MersenneTwister;

  //! Store for the ARM values
  vector<double> m_OriginalARMValues;

  //! The boot-strapped FWHM samples
  vector<double> m_BootStrappedFWHMSamples;
  //! The boot-strapped fit parameters:
  vector<vector<double>> m_BootStrappedFitParameters;
  //! The boot-strapped Baker-Cousins-Likelihood ratio values:
  vector<double> m_BootStrappedBakerCousins;

  //! The maximum number of allowed threads
  unsigned int m_MaximumNumberOfThreads;
  //! The number of currently running threads
  unsigned int m_NumberOfRunningThreads;
  //! The global mutex
  std::mutex m_Mutex;

  //! Check is we have a final results
  bool m_FitSuccessful;

  //! The final (average) FWHM of the ARM
  double m_FinalFWHM;

  //! The final FWHM uncertainty
  double m_FinalFWHMUncertainty;

  //! The final (average) baker-Cousins likelihood ratio
  double m_FinalBakerCousins;

  //! The final baker-Cousins likelihood ratio uncertainty
  double m_FinalBakerCousinsUncertainty;

  //! The final fit parameters
  vector<double> m_FinalFitParameters;

  //! The 50% containment radius (accurate to the next largest bin in the histogram)
  double m_Containment50Percent;
  //! The 1-sigmna containment radius (accurate to the next largest bin in the histogra)
  double m_Containment1Sigma;
  //! The 2-sigma containment radius (accurate to the next largest bin in the histogra)
  double m_Containment2Sigma;
  //! The 3-sigma containment radius (accurate to the next largest bin in the histogra)
  double m_Containment3Sigma;

  //! Minimum height for fitting
  double m_MinHeight;
  //! Maximum height for fitting
  double m_MaxHeight;
  //! Minimum width for fitting
  double m_MinWidth;
  //! Maximum width for fitting
  double m_MaxWidth;
  //! Minimum scale of generalized normal function for fitting
  double m_MinScale;
  //! Maximum scale of generalized normal functiofor fitting
  double m_MaxScale;

  //! Guess for the widths for fitting
  double m_WidthGuess;
  //! Guess for the height for fitting
  double m_HeightGuess;
  //! Guess for the scale for fitting
  double m_ScaleGuess;


#ifdef ___CLING___
 public:
  ClassDef(MARMFitter, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
