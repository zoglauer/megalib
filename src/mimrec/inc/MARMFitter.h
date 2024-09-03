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
  //! The default is 101
  void SetNumberOfBins(unsigned int NumberOfBins = 101);

  //! Set the maximum range of the ARM values
  //! The maximum value is 180
  //! The histogram will go from [-MaxARM, MaxARM]
  //! The default is 180
  void SetMaximumARMValue(double MaxARM = 180);

  //! Set the test position, and its coordinate system
  //! Do not call this function while adding events
  void SetTestPosition(MVector TestPosition, MCoordinateSystem CoordinateSystem);

  //! If this is set we try to optimize the binning
  //! Especially if we have too many bins we iterate until we have at least 50 counts in one bin - if possible
  void UseOptimizedBinning(bool DoIt = true) { m_OptimizeBinning = DoIt; }

  //! If this is set we use an unbinned-likelihood fit
  void UseBinnedFitting(bool DoIt = true) { m_UnbinnedFitting = !DoIt; }

  //! Get a list of all fit functions:
  vector<MARMFitFunctionID> GetListOfFitFunctions() const;

  //! Fit this function to the data
  //! One of: c_Gauss, c_Lorentz, c_GaussLorentz, c_GaussDoubleLorentz, c_AsymmetricGaussLorentzLorentz, etc.
  //! If not set,  default is c_AsymmetricGaussLorentzLorentz
  void SetFitFunction(MARMFitFunctionID ID) { m_ARMFitFunction = ID; }

  //! Fit this function to the data
  //! The ID corresponds to the position of the corresponding MARMFitFunctionID in the vector retuned by GetListOfFitFunctions()
  //! If it's not in there use c_AsymmetricGaussLorentzLorentz
  void SetFitFunction(unsigned int ID);

  // Interface to set the data

  //! Reset the internally stored ARM values
  void Reset();

  //! Add Event - only stores the actual ARM value - do not do any prior ARM selections
  void AddEvent(MComptonEvent* Event);
  //! Add an ARM value - do not do any prior ARM selections
  void AddARMValue(double ARMValue) { m_OriginalARMValues.push_back(ARMValue); }

  //! Load the ARM value from file
  bool LoadARMValues(MString FileName);
  //! Save the the ARM values to file
  bool SaveARMValues(MString FileName);

  // Interface to do the fits

  //! Performs N fits.
  //! Each fit samples randomly form the stored ARM values to determine the average width and error correctly
  bool Fit(unsigned int NumberOfFits = 1);

  //! Perform fits with all fit functions and report Baker-Cousin results
  bool FitAll();

  // Interface to retrieve the results

  //! Returns true if we have a successful fit and have the results
  bool WasFittingSuccessful() const { return m_FitSuccessful; }

  //! Return the bootstrapped FWHM'es
  vector<double> GetBootstrappedFWHMSamples() { return m_BootStrappedFWHMSamples; }

  //! Return the FWHM result or g_DoubleNotDefined if fit was not successful or not done yet
  double GetAverageFWHM() const;

  //! Return the FWHM uncertainty result or g_DoubleNotDefined if fit was not successful or not done yet
  double GetAverageFWHMUncertainty() const;

  //! Return the 50.0% containment for events with +- maximum ARM value or g_DoubleNotDefined if fit was not successful or not done yet
  double Get50p0PercentContainmentUsingARMSelection() const;
  //! Return the 68.3% containment for events with +- maximum ARM value or g_DoubleNotDefined if fit was not successful or not done yet
  double Get68p3PercentContainmentUsingARMSelection() const;
  //! Return the 95.5% containment for events with +- maximum ARM value or g_DoubleNotDefined if fit was not successful or not done yet
  double Get95p5PercentContainmentUsingARMSelection() const;
  //! Return the 99.7% containment for events with +- maximum ARM value or g_DoubleNotDefined if fit was not successful or not done yet
  double Get99p7PercentContainmentUsingARMSelection() const;

  //! Return the 50.0% containment using all data or g_DoubleNotDefined if fit was not successful or not done yet
  double Get50p0PercentContainmentUsingAllData() const;
  //! Return the 68.3% containment using all data or g_DoubleNotDefined if fit was not successful or not done yet
  double Get68p3PercentContainmentUsingAllData() const;
  //! Return the 95.5% containment using all data or g_DoubleNotDefined if fit was not successful or not done yet
  double Get95p5PercentContainmentUsingAllData() const;
  //! Return the 99.7% containment using all data or g_DoubleNotDefined if fit was not successful or not done yet
  double Get99p7PercentContainmentUsingAllData() const;

  //! Return a summary string
  MString ToString();

  //! Draw an ARM histogram
  //! This works like a root histogram, you would do:
  //! Canvas->cd();
  //! ARMFitter->Draw();
  //! Canvas->Update();
  void Draw();

  //! Return the function of the fit given the fit function ID
  MString GetARMFitFunction(MARMFitFunctionID ARMFitID);
  //! Return the name of the fit given the fit function ID
  MString GetARMFitFunctionName(MARMFitFunctionID ARMFitID);
  //! Return the number of parameters of the fit given the fit function ID
  unsigned int GetARMFitFunctionNumberOfParameters(MARMFitFunctionID ARMFitID);




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

  //! True if the binning has already been optimized
  bool m_IsBinningOptimized;

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

  //! The 50% containment radius within the ARM window (accurate to the next largest value in the data set)
  double m_Containment50PercentUsingARMSelection;
  //! The 1-sigmna containment radius within the ARM window (accurate to the next largest value in the data set)
  double m_Containment1SigmaUsingARMSelection;
  //! The 2-sigma containment radius within the ARM window (accurate to the next largest value in the data set)
  double m_Containment2SigmaUsingARMSelection;
  //! The 3-sigma containment radius within the ARM window (accurate to the next largest value in the data set)
  double m_Containment3SigmaUsingARMSelection;

  //! The 50% containment radius using all data (accurate to the next largest value in the data set)
  double m_Containment50PercentUsingAllData;
  //! The 1-sigmna containment radius using all data (accurate to the next largest value in the data set)
  double m_Containment1SigmaUsingAllData;
  //! The 2-sigma containment radius using all data (accurate to the next largest value in the data set)
  double m_Containment2SigmaUsingAllData;
  //! The 3-sigma containment radius using all data (accurate to the next largest value in the data set)
  double m_Containment3SigmaUsingAllData;

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
  double m_GuessWidth;
  //! Guess for the height for fitting
  double m_GuessHeight;
  //! Guess for the scale for fitting
  double m_GuessScale;
  //! Guess for the low-x maximum for fitting
  double m_GuessMaximumLow;
  //! Guess for the high-x maximum for fitting
  double m_GuessMaximumHigh;


#ifdef ___CLING___
 public:
  ClassDef(MARMFitter, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
