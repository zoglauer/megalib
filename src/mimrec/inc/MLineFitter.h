/*
 * MLineFitter.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MLineFitter__
#define __MLineFitter__


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
enum class MLineFitFunctionID {
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


//! A class to fit a single nuclear line
class MLineFitter
{
  // public interface:
 public:
  //! Default constructor
  MLineFitter();
  //! Default destuctor 
  virtual ~MLineFitter();

  // Interface to set up the fitter

  //! Set the number of bins for the line fit hisogram
  //! The default is 100
  void SetNumberOfBins(unsigned int NumberOfBins = 100);

  //! Set the minimum & maximum range of the fit energy values
  void SetEnergyRange(double MinimumEnergy, double MaximumEnergy);

  //! Set the test position, and its coordinate system
  //! Do not call this function while adding events
  void SetTestPosition(MVector TestPosition, MCoordinateSystem CoordinateSystem);

  //! If this is set we try to optimize the binning
  //! Especially if we have too many bins we iterate until we have at least 50 counts in one bin - if possible
  void UseOptimizedBinning(bool DoIt = true) { m_OptimizeBinning = DoIt; }

  //! If this is set we use an unbinned-likelihood fit
  void UseBinnedFitting(bool DoIt = true) { m_UnbinnedFitting = !DoIt; }

  //! Get a list of all fit functions:
  vector<MLineFitFunctionID> GetListOfFitFunctions() const;

  //! Fit this function to the data
  //! One of: c_Gauss, c_Lorentz, c_GaussLorentz, c_GaussDoubleLorentz, c_AsymmetricGaussLorentzLorentz, etc.
  //! If not set,  default is c_AsymmetricGaussLorentzLorentz
  void SetFitFunction(MLineFitFunctionID ID) { m_LineFitFunction = ID; }

  //! Fit this function to the data
  //! The ID corresponds to the position of the corresponding MLineFitFunctionID in the vector retuned by GetListOfFitFunctions()
  //! If it's not in there use c_AsymmetricGaussLorentzLorentz
  void SetFitFunction(unsigned int ID);

  // Interface to set the data

  //! Reset the internally stored Line values
  void Reset();

  //! Add an event - only stores the actual energy value - do not do any prior energy selections
  void AddEvent(MPhysicalEvent* Event);
  //! Add an energy value - do not do any prior Line selections
  void AddEnergyValue(double EnergyValue) { m_OriginalEnergyValues.push_back(EnergyValue); }

  //! Load the energy values from file
  bool LoadEnergyValues(MString FileName);
  //! Save the the energy values to file
  bool SaveEnergyValues(MString FileName);

  // Interface to do the fits

  //! Performs N fits.
  //! Each fit samples randomly form the stored Line values to determine the average width and error correctly
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

  //! Get the counts in the FWHM window or g_DoubleNotDefined if we don't have a FWHM
  unsigned int GetFWHMWindowCounts() const;
  //! Get the counts in the FWHM minus uncertainty window or g_DoubleNotDefined if we don't have a FWHM uncertainty
  unsigned int GetFWHMWindowCountsMinimum() const;
  //! Get the counts in the FWHM plus uncertainty window or g_DoubleNotDefined if we don't have a FWHM uncertainty
  unsigned int GetFWHMWindowCountsMaximum() const;


  //! Return a summary string
  MString ToString();

  //! Draw an Line histogram
  //! This works like a root histogram, you would do:
  //! Canvas->cd();
  //! LineFitter->Draw();
  //! Canvas->Update();
  void Draw();

  //! Return the function of the fit given the fit function ID
  MString GetLineFitFunction(MLineFitFunctionID LineFitID);
  //! Return the name of the fit given the fit function ID
  MString GetLineFitFunctionName(MLineFitFunctionID LineFitID);
  //! Return the number of parameters of the fit given the fit function ID
  unsigned int GetLineFitFunctionNumberOfParameters(MLineFitFunctionID LineFitID);




  // protected methods:
 protected:
  //! Optimize binning - check sif the binning is OK, otherwise check for a better one
  bool OptimizeBinning();
  //! Perform a single fit
  bool PerformFit(unsigned int FitID, vector<double>& LineValues);
  //! Thread entry for parallel fitting
  void ParallelFitting(int id, condition_variable& CV);
  //! Bootstrap the energy values and return them as vector
  vector<double> BootstrapEnergyValues();
  //! Calculate the average metrics from boot strapping (FWHM & uncertainty)
  void CalculateBootStrappedMetrics();
  //! Calculate the metrics of the line (containment etc.)
  void CalculateLineMetrics();

  //! Setup the fit function - LineFitID is one of the members of the MLineFitFunctions enum
  void SetupLineFit(MLineFitFunctionID LineFitID, ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a Gauss-function
  void SetupLineFitGauss(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a Gauss-Lorentz-function
  void SetupLineFitLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a GeneralizedNormal-function
  void SetupLineFitGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a Gauss-Lorentz-function
  void SetupLineFitGaussLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup a Gauss-Double-Lorentz-function
  void SetupLineFitGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup an Assymetric-Gauss-Double-Lorentz-function
  void SetupLineFitAsymmetricGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup an Assymetric-Gauss-Double-Lorentz-function
  void SetupLineFitAsymmetricGaussGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup an Assymetric-Gauss-Double-Lorentz-function
  void SetupLineFitAsymmetricGeneralizedNormalGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);
  //! Setup an Assymetric-Gauss-Double-Lorentz-function
  void SetupLineFitAsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The number of bins
  unsigned int m_NumberOfBins;
  //! The maximum energy value to fit
  double m_MinimumEnergyValue;
  //! The minimum energy value to fit
  double m_MaximumEnergyValue;
  //! The test position
  MVector m_TestPosition;
  //! The coordinate system of the test position
  MCoordinateSystem m_CoordinateSystem;
  //! The function to use for fitting
  MLineFitFunctionID m_LineFitFunction;

  //! Perform binning optimization
  bool m_OptimizeBinning;
  //! Use unbinned fitting
  bool m_UnbinnedFitting;

  //! Random device
  std::mt19937 m_MersenneTwister;

  //! Store for the energy values
  vector<double> m_OriginalEnergyValues;

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

  //! The final (average) FWHM of the Line
  double m_FinalFWHM;

  //! The final FWHM uncertainty
  double m_FinalFWHMUncertainty;

  //! The final (average) baker-Cousins likelihood ratio
  double m_FinalBakerCousins;

  //! The final baker-Cousins likelihood ratio uncertainty
  double m_FinalBakerCousinsUncertainty;

  //! The final fit parameters
  vector<double> m_FinalFitParameters;

  //! The counts in the FWHM window or g_UnsignedIntNotDefined if we don't have a FWHM
  unsigned int m_CountsFWHMWindow;
  //! The counts in the FWHM minus uncertainty window or g_UnsignedIntNotDefined if we don't have a FWHM uncertainty
  unsigned int m_CountsFWHMWindowMinimum;
  //! The counts in the FWHM plus uncertainty window or g_UnsignedIntNotDefined if we don't have a FWHM uncertainty
  unsigned int m_CountsFWHMWindowMaximum;

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
  ClassDef(MLineFitter, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
