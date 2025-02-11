/*
 * MARMFitter.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MARMFitter.h"

// Standard libs:
#include <cmath>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <fstream>

// ROOT libs:
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TVirtualFitter.h"
#include "TFitResult.h"
#include "Math/MinimizerOptions.h"
#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"
#include "Fit/BinData.h"
#include "Fit/UnBinData.h"
#include "HFitInterface.h"
#include "Fit/Fitter.h"
#include "TLatex.h"
#include "TStyle.h"

// MEGAlib libs:
#include "MExceptions.h"
#include "MComptonEvent.h"
#include "MFitFunctions.h"
#include "MInterface.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MARMFitter)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MARMFitter::MARMFitter()
{
  ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(250000);

  std::random_device RD; // <-- should use MEGAlib global
  std::mt19937 m_MersenneTwister(RD());

  m_NumberOfBins = 101;
  m_MaxARMValue = 180;
  m_ARMFitFunction = MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz;

  m_OptimizeBinning = false;
  m_IsBinningOptimized = false;

  m_UnbinnedFitting = true;

  m_MinHeight = 0;
  m_MaxHeight = 1E20;
  m_MinWidth = 0.05;
  m_MaxWidth = 180;
  m_MinScale = 0.1;
  m_MaxScale = 1000;

  m_GuessWidth = 2;
  m_GuessHeight = 2;
  m_GuessScale = 2;

  m_MaximumNumberOfThreads = 2*std::thread::hardware_concurrency();
  if (m_MaximumNumberOfThreads < 1) m_MaximumNumberOfThreads = 1;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MARMFitter::~MARMFitter()
{
}


////////////////////////////////////////////////////////////////////////////////


//!
void MARMFitter::SetNumberOfBins(unsigned int NumberOfBins)
{
  m_NumberOfBins = NumberOfBins;
  if (m_NumberOfBins < 10) {
    merr<<"You need at least 10 bins to be able to fit an ARM, and not "<<NumberOfBins<<". Setting it to 10."<<endl;
    m_NumberOfBins = 10;
  }
  m_IsBinningOptimized = false;
}


////////////////////////////////////////////////////////////////////////////////



//!
void MARMFitter::SetMaximumARMValue(double MaxARMValue)
{
  m_MaxARMValue = MaxARMValue;
  if (m_MaxARMValue <= 0 || m_MaxARMValue > 180) {
    merr<<"The maximum ARM value must be within (0,180] and not "<<MaxARMValue<<endl<<"Setting it to 180"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set the test position, and its coordinate system
void MARMFitter::SetTestPosition(MVector TestPosition, MCoordinateSystem CoordinateSystem)
{
  m_TestPosition = TestPosition;
  m_CoordinateSystem = CoordinateSystem;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit this function to the data
//! The ID corresponds to the position of the corresponding MARMFitFunctionID in the vector retuned by GetListOfFitFunctions()
//! If it's not in there use c_AsymmetricGaussLorentzLorentz
void MARMFitter::SetFitFunction(unsigned int ID)
{
  vector<MARMFitFunctionID> IDs = GetListOfFitFunctions();

  if (ID < IDs.size()) {
    m_ARMFitFunction = IDs[ID];
  } else {
    merr<<"There is no function with ID n"<<ID<<". Choosing assymmetric gauss + Lorentz + Lorentz"<<endl;
    m_ARMFitFunction = MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Reset the internally stored ARM values
void MARMFitter::Reset()
{
  m_OriginalARMValues.clear();

  m_BootStrappedFWHMSamples.clear();
  m_BootStrappedFitParameters.clear();
  m_BootStrappedBakerCousins.clear();

  m_FitSuccessful = false;
  m_FinalFWHM = g_DoubleNotDefined;
  m_FinalFWHMUncertainty = g_DoubleNotDefined;
  m_FinalFitParameters.clear();
  m_FinalBakerCousins = g_DoubleNotDefined;
  m_FinalBakerCousinsUncertainty = g_DoubleNotDefined;

  m_Containment50PercentUsingARMSelection = g_DoubleNotDefined;
  m_Containment1SigmaUsingARMSelection = g_DoubleNotDefined;
  m_Containment2SigmaUsingARMSelection = g_DoubleNotDefined;
  m_Containment3SigmaUsingARMSelection = g_DoubleNotDefined;

  m_Containment50PercentUsingAllData = g_DoubleNotDefined;
  m_Containment1SigmaUsingAllData = g_DoubleNotDefined;
  m_Containment2SigmaUsingAllData = g_DoubleNotDefined;
  m_Containment3SigmaUsingAllData = g_DoubleNotDefined;

  m_CountsFWHMWindow = g_UnsignedIntNotDefined;
  m_CountsFWHMWindowMinimum = g_UnsignedIntNotDefined;
  m_CountsFWHMWindowMaximum = g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Add event - only stores the actual ARM value
void MARMFitter::AddEvent(MComptonEvent* ComptonEvent)
{
  m_OriginalARMValues.push_back(ComptonEvent->GetARMGamma(m_TestPosition, m_CoordinateSystem));
}


////////////////////////////////////////////////////////////////////////////////


//! Optimize binning - check sif the binning is OK, otherwise check for a better one
bool MARMFitter::OptimizeBinning()
{
  if (m_IsBinningOptimized == true) return true;

  const unsigned int MinimumBinContent = 50;
  const unsigned int MinimumBins = 5;

  bool BinningOptimized = false;

  unsigned int NumberOfBins = m_NumberOfBins;

  while (BinningOptimized == false) {

    double BinWidth = 2*m_MaxARMValue / NumberOfBins;
    vector<unsigned int> BinnedData(NumberOfBins, 0);

    for (auto A: m_OriginalARMValues) {
      if (A >= -m_MaxARMValue && A <= m_MaxARMValue) {
        unsigned int BinIndex = static_cast<unsigned int>((A + m_MaxARMValue) / BinWidth);

        // Sanity check: Do we have a correct bin
        if (BinIndex >= 0 && BinIndex < NumberOfBins) {
          BinnedData[BinIndex]++;
        } else {
          new MExceptionIndexOutOfBounds(0, NumberOfBins, BinIndex);
          return false;
        }
      }
    }

    // We need at least 1 bin with more than >>MinimumBinContent<< counts in it:
    auto MaximumIterator = std::max_element(BinnedData.begin(), BinnedData.end());

    if (MaximumIterator == BinnedData.end()) {
      new MExceptionObjectDoesNotExist("Unable to find maximum of array.");
      return false;
    }
    unsigned int Maximum = *MaximumIterator;

    if (Maximum < MinimumBinContent) {
      if (NumberOfBins == MinimumBins) {
        mout<<"Unable to find a good number of bins. Using "<<MinimumBins<<endl;
        m_NumberOfBins = MinimumBins;
        return false;
      }

      mout<<"We do not have at least "<<MinimumBinContent<<" counts in any bin of the ARM histogram, but only "<<Maximum<<endl;

      // Make sure we alsways have a reduction
      unsigned int Reduction = NumberOfBins - static_cast<unsigned int>(static_cast<double>(NumberOfBins) * Maximum / MinimumBinContent);
      if (Reduction == 0) Reduction = 1;
      NumberOfBins -= Reduction;
      // Make sure we have always an odd number
      if (NumberOfBins % 2 == 0) NumberOfBins -= 1;
      // Make sure we do not go below our limit
      if (NumberOfBins < MinimumBins) NumberOfBins = MinimumBins;

      mout<<"Reducing the bin number to "<<NumberOfBins<<" and checking again"<<endl;

    } else {
      BinningOptimized = true;
    }
  }

  m_NumberOfBins = NumberOfBins;
  m_IsBinningOptimized = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the name of the fit given the fit function ID
MString MARMFitter::GetARMFitFunctionName(MARMFitFunctionID ID)
{
  if (ID == MARMFitFunctionID::c_Gauss) {
    return MString("Gauss");
  } else if (ID == MARMFitFunctionID::c_Lorentz) {
    return MString("Lorentz");
  } else if (ID == MARMFitFunctionID::c_GeneralizedNormal) {
    return MString("Generalized-Normal");
  } else if (ID == MARMFitFunctionID::c_GaussLorentz) {
    return MString("Gauss + Lorentz");
  } else if (ID == MARMFitFunctionID::c_GaussLorentzLorentz) {
    return MString("Gauss + Lorentz + Lorentz");
  } else if (ID == MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz) {
    return MString("Asymmetric-Gauss + Lorentz + Lorentz");
  } else if (ID == MARMFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz) {
    return MString("Asymmetric-Gauss + Gauss + Lorentz + Lorentz");
  } else if (ID == MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal) {
    return MString("Asymmetric-Generalized-Normal + Generalized-Normal");
  } else if (ID == MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal) {
    return MString("Asymmetric-Generalized-Normal + Generalized-Normal + Generalized-Normal");
  } else {
    new MExceptionObjectDoesNotExist("Unknown ARM fit function ID");
    return MString("Unknown ARM fit function ID");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the function of the fit given the fit function ID
MString MARMFitter::GetARMFitFunction(MARMFitFunctionID ID)
{
  if (ID == MARMFitFunctionID::c_Gauss) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)); }");
  } else if (ID == MARMFitFunctionID::c_Lorentz) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*(p[3]*p[3])/(p[3]*p[3] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MARMFitFunctionID::c_GeneralizedNormal) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[3], p[4])); }");
  } else if (ID == MARMFitFunctionID::c_GaussLorentz) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) + p[4]/(p[5]*p[5] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MARMFitFunctionID::c_GaussLorentzLorentz) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) + p[4]*(p[5]*p[5])/(p[5]*p[5] + (x[0]-p[1])*(x[0]-p[1])) + p[6]*(p[7]*p[7])/(p[7]*p[7] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz) {
    return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) : p[2]*exp(-0.5*pow((x[0]-p[1])/p[4], 2)) ) + p[5]*(p[6]*p[6])/(p[6]*p[6] + (x[0]-p[1])*(x[0]-p[1])) + p[7]*(p[8]*p[8])/(p[8]*p[8] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MARMFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz) {
    //return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) : p[2]*exp(-0.5*pow((x[0]-p[1])/p[4], 2)) )  +  p[5]*exp(-0.5*pow((x[0]-p[1])/p[6], 2))  +  p[7]*(p[8]*p[8])/(p[8]*p[8] + (x[0]-p[1])*(x[0]-p[1]))  +  p[9]*(p[10]*p[10])/(p[10]*p[10] + (x[0]-p[1])*(x[0]-p[1])); }");
    return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) : p[2]*exp(-0.5*pow((x[0]-p[1])/p[4], 2)) )  +  p[5]*exp(-0.5*pow((x[0]-p[1])/p[6], 2))  +  p[7]*p[8]/(p[8]*p[8] + (x[0]-p[1])*(x[0]-p[1]))  +  p[9]*p[10]/(p[10]*p[10] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal) {
    return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[3], p[4])) : p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[5], p[6])) )  +  p[7]*exp(-0.5*pow(fabs(x[0]-p[1])/p[8], p[9])) ; }");
  } else if (ID == MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal) {
    return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[3], p[4])) : p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[5], p[6])) )  +  p[7]*exp(-0.5*pow(fabs(x[0]-p[1])/p[8], p[9]))  +  p[10]*exp(-0.5*pow(fabs(x[0]-p[1])/p[11], p[12])) ; }");
  } else {
    new MExceptionObjectDoesNotExist("Unknown ARM fit function ID");
    return MString("Unknown ARM fit function ID");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of parameters of the fit given the fit function ID
unsigned int MARMFitter::GetARMFitFunctionNumberOfParameters(MARMFitFunctionID ID)
{
   if (ID == MARMFitFunctionID::c_Gauss) {
    return 4;
  } else if (ID == MARMFitFunctionID::c_Lorentz) {
    return 4;
  } else if (ID == MARMFitFunctionID::c_GeneralizedNormal) {
    return 5;
  } else if (ID == MARMFitFunctionID::c_GaussLorentz) {
    return 6;
  } else if (ID == MARMFitFunctionID::c_GaussLorentzLorentz) {
    return 8;
  } else if (ID == MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz) {
    return 9;
  } else if (ID == MARMFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz) {
    return 11;
  } else if (ID == MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal) {
    return 10;
  } else if (ID == MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal) {
    return 13;
  } else {
    new MExceptionObjectDoesNotExist("Unknown ARM fit function ID");
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get a list of all fit functions:
vector<MARMFitFunctionID> MARMFitter::GetListOfFitFunctions() const
{
  vector<MARMFitFunctionID> List;
  List.push_back(MARMFitFunctionID::c_Gauss);
  List.push_back(MARMFitFunctionID::c_Lorentz);
  List.push_back(MARMFitFunctionID::c_GeneralizedNormal);
  List.push_back(MARMFitFunctionID::c_GaussLorentz);
  List.push_back(MARMFitFunctionID::c_GaussLorentzLorentz);
  List.push_back(MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz);
  List.push_back(MARMFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz);
  List.push_back(MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal);
  List.push_back(MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal);

  return List;
}


////////////////////////////////////////////////////////////////////////////////


//! Switch between the fit functions
void MARMFitter::SetupARMFit(MARMFitFunctionID ARMFitID, ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  if (ARMFitID == MARMFitFunctionID::c_Gauss) {
    SetupARMFitGauss(Fitter, FitFunction);
  } else if (ARMFitID == MARMFitFunctionID::c_Lorentz) {
    SetupARMFitLorentz(Fitter, FitFunction);
  } else if (ARMFitID == MARMFitFunctionID::c_GeneralizedNormal) {
    SetupARMFitGeneralizedNormal(Fitter, FitFunction);
  } else if (ARMFitID == MARMFitFunctionID::c_GaussLorentz) {
    SetupARMFitGaussLorentz(Fitter, FitFunction);
  } else if (ARMFitID == MARMFitFunctionID::c_GaussLorentzLorentz) {
    SetupARMFitGaussLorentzLorentz(Fitter, FitFunction);
  } else if (ARMFitID == MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz) {
    SetupARMFitAsymmetricGaussLorentzLorentz(Fitter, FitFunction);
  } else if (ARMFitID == MARMFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz) {
    SetupARMFitAsymmetricGaussGaussLorentzLorentz(Fitter, FitFunction);
  } else if (ARMFitID == MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal) {
    SetupARMFitAsymmetricGeneralizedNormalGeneralizedNormal(Fitter, FitFunction);
  } else if (ARMFitID == MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal) {
    SetupARMFitAsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal(Fitter, FitFunction);
  } else {
    new MExceptionObjectDoesNotExist("Unknown ARM fit function ID");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Setup a Gauss fit function
void MARMFitter::SetupARMFitGauss(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_Gauss).Data(), GetARMFitFunction(MARMFitFunctionID::c_Gauss).Data(), -m_MaxARMValue, m_MaxARMValue, 4);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1, 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, m_GuessWidth };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());

  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("Gauss - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("Gauss - sigma");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().SetUpdateAfterFit();
}


////////////////////////////////////////////////////////////////////////////////


//! Setup a Lorentz fit function
//! Return: The fitter
void MARMFitter::SetupARMFitLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_Lorentz).Data(), GetARMFitFunction(MARMFitFunctionID::c_Lorentz).Data(), -m_MaxARMValue, m_MaxARMValue, 4);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1, 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, m_GuessWidth };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());

  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("Lorentz - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("Lorentz - width");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().SetUpdateAfterFit();
}


////////////////////////////////////////////////////////////////////////////////


//! Setup a generalzied normal distribution fit function
void MARMFitter::SetupARMFitGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_GeneralizedNormal).Data(), GetARMFitFunction(MARMFitFunctionID::c_GeneralizedNormal).Data(), -m_MaxARMValue, m_MaxARMValue, 5);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1, 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, m_GuessWidth, m_GuessScale };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());

  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("Generalized normal - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("Generalized normal - sigma");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(4).SetName("Generalized normal - scale");
  Fitter.Config().ParSettings(4).SetLimits(m_MinScale, m_MaxScale);

  Fitter.Config().SetUpdateAfterFit();
}


////////////////////////////////////////////////////////////////////////////////


//! Setup an Gauss-Lorentz fit function
void MARMFitter::SetupARMFitGaussLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_GaussLorentz).Data(), GetARMFitFunction(MARMFitFunctionID::c_GaussLorentz).Data(), -m_MaxARMValue, m_MaxARMValue, 6);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1, 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, m_GuessWidth, m_GuessHeight, m_GuessWidth };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());

  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("Gauss - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("Gauss - sigma");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().ParSettings(4).SetName("Lorentz - height");
  Fitter.Config().ParSettings(4).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(5).SetName("Lorentz - width");
  Fitter.Config().ParSettings(5).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().SetUpdateAfterFit();
}


////////////////////////////////////////////////////////////////////////////////


//! Setup an Gauss-Double-Lorentz fit function
void MARMFitter::SetupARMFitGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_GaussLorentzLorentz).Data(), GetARMFitFunction(MARMFitFunctionID::c_GaussLorentzLorentz).Data(), -m_MaxARMValue, m_MaxARMValue, 8);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1 , 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, m_GuessWidth, m_GuessHeight, m_GuessWidth, m_GuessHeight, m_GuessWidth };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());

  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("Gauss - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("Gauss - sigma");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().ParSettings(4).SetName("Lorentz #1 - height");
  Fitter.Config().ParSettings(4).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(5).SetName("Lorentz #1 - width");
  Fitter.Config().ParSettings(5).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().ParSettings(6).SetName("Lorentz #2 - height");
  Fitter.Config().ParSettings(6).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(7).SetName("Lorentz #2 - width");
  Fitter.Config().ParSettings(7).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().SetUpdateAfterFit();
}


////////////////////////////////////////////////////////////////////////////////


//! Setup an Asymmetric-Gauss-Double-Lorentz fit function
void MARMFitter::SetupARMFitAsymmetricGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction  = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz).Data(), GetARMFitFunction(MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz).Data(), -m_MaxARMValue, m_MaxARMValue, 9);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1, 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, 0.9*m_GuessWidth, 1.1*m_GuessWidth, 0.9*m_GuessHeight, 0.5*m_GuessWidth, 1.1*m_GuessHeight, 2*m_GuessWidth };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());


  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("Gauss - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("Gauss - sigma left");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(4).SetName("Gauss - sigma right");
  Fitter.Config().ParSettings(4).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().ParSettings(5).SetName("Lorentz #1 - height");
  Fitter.Config().ParSettings(5).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(6).SetName("Lorentz #1 - width");
  Fitter.Config().ParSettings(6).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().ParSettings(7).SetName("Lorentz #2 - height");
  Fitter.Config().ParSettings(7).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(8).SetName("Lorentz #2 - width");
  Fitter.Config().ParSettings(8).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().SetUpdateAfterFit();
}


////////////////////////////////////////////////////////////////////////////////


//! Setup an Asymmetric-Gauss-Double-Lorentz fit function
void MARMFitter::SetupARMFitAsymmetricGaussGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction  = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz).Data(), GetARMFitFunction(MARMFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz).Data(), -m_MaxARMValue, m_MaxARMValue, 11);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1, 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, m_GuessWidth, m_GuessWidth, m_GuessHeight, m_GuessWidth, m_GuessHeight, m_GuessWidth, m_GuessHeight, m_GuessWidth };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());


  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("Gauss #1 - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("Gauss #1 - sigma left");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(4).SetName("Gauss #1 - sigma right");
  Fitter.Config().ParSettings(4).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().ParSettings(5).SetName("Gauss #2 - height");
  Fitter.Config().ParSettings(5).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(6).SetName("Gauss #2 - sigma");
  Fitter.Config().ParSettings(6).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().ParSettings(7).SetName("Lorentz #1 - height");
  Fitter.Config().ParSettings(7).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(8).SetName("Lorentz #1 - width");
  Fitter.Config().ParSettings(8).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().ParSettings(9).SetName("Lorentz #2 - height");
  Fitter.Config().ParSettings(9).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(10).SetName("Lorentz #2 - width");
  Fitter.Config().ParSettings(10).SetLimits(m_MinWidth, m_MaxWidth);

  Fitter.Config().SetUpdateAfterFit();
}


////////////////////////////////////////////////////////////////////////////////


//! Setup an Asymmetric-GeneralizedNormal+GeneralizedNormal fit function
void MARMFitter::SetupARMFitAsymmetricGeneralizedNormalGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction  = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal).Data(), GetARMFitFunction(MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal).Data(), -m_MaxARMValue, m_MaxARMValue, 10);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1, 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, m_GuessWidth, m_GuessScale, m_GuessWidth, m_GuessHeight, m_GuessScale, m_GuessHeight, m_GuessWidth, m_GuessScale };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());

  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("GeneralizedNormal #1 - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("GeneralizedNormal #1 - sigma left");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(4).SetName("GeneralizedNormal #1 - scale left");
  Fitter.Config().ParSettings(4).SetLimits(m_MinScale, m_MaxScale);
  Fitter.Config().ParSettings(5).SetName("GeneralizedNormal #1 - sigma right");
  Fitter.Config().ParSettings(5).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(6).SetName("GeneralizedNormal #1 - scale right");
  Fitter.Config().ParSettings(6).SetLimits(m_MinScale, m_MaxScale);

  Fitter.Config().ParSettings(7).SetName("GeneralizedNormal #2 - height");
  Fitter.Config().ParSettings(7).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(8).SetName("GeneralizedNormal #2 - sigma");
  Fitter.Config().ParSettings(8).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(9).SetName("GeneralizedNormal #2 - scale");
  Fitter.Config().ParSettings(9).SetLimits(m_MinScale, m_MaxScale);

  Fitter.Config().SetUpdateAfterFit();
}



////////////////////////////////////////////////////////////////////////////////


//! Setup an Asymmetric-GeneralizedNormal+GeneralizedNormal+GeneralizedNormal fit function
void MARMFitter::SetupARMFitAsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction  = new TF1(GetARMFitFunctionName(MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal).Data(), GetARMFitFunction(MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal).Data(), -m_MaxARMValue, m_MaxARMValue, 13);
  ROOT::Math::WrappedMultiTF1 WrappedFitFunction(**FitFunction, 1);

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) Fitter.Config().MinimizerOptions().SetPrintLevel(1);
  Fitter.SetFunction(WrappedFitFunction, false);

  vector<double> Parameters = { 1, 0.5*(m_GuessMaximumLow+m_GuessMaximumHigh), m_GuessHeight, m_GuessWidth, m_GuessScale, m_GuessWidth, m_GuessScale, m_GuessHeight, m_GuessWidth, m_GuessScale, m_GuessHeight, m_GuessWidth, m_GuessScale };
  Fitter.Config().SetParamsSettings(Parameters.size(), Parameters.data());

  Fitter.Config().ParSettings(0).SetName("Offset");
  Fitter.Config().ParSettings(0).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(1).SetName("Shift");
  Fitter.Config().ParSettings(1).SetLimits(m_GuessMaximumLow, m_GuessMaximumHigh);

  Fitter.Config().ParSettings(2).SetName("GeneralizedNormal #1 - height");
  Fitter.Config().ParSettings(2).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(3).SetName("GeneralizedNormal #1 - sigma left");
  Fitter.Config().ParSettings(3).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(4).SetName("GeneralizedNormal #1 - scale left");
  Fitter.Config().ParSettings(4).SetLimits(m_MinScale, m_MaxScale);
  Fitter.Config().ParSettings(5).SetName("GeneralizedNormal #1 - sigma right");
  Fitter.Config().ParSettings(5).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(6).SetName("GeneralizedNormal #1 - scale right");
  Fitter.Config().ParSettings(6).SetLimits(m_MinScale, m_MaxScale);

  Fitter.Config().ParSettings(7).SetName("GeneralizedNormal #2 - height");
  Fitter.Config().ParSettings(7).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(8).SetName("GeneralizedNormal #2 - sigma");
  Fitter.Config().ParSettings(8).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(9).SetName("GeneralizedNormal #2 - scale");
  Fitter.Config().ParSettings(9).SetLimits(m_MinScale, m_MaxScale);

  Fitter.Config().ParSettings(10).SetName("GeneralizedNormal #3 - height");
  Fitter.Config().ParSettings(10).SetLimits(m_MinHeight, m_MaxHeight);
  Fitter.Config().ParSettings(11).SetName("GeneralizedNormal #3 - sigma");
  Fitter.Config().ParSettings(11).SetLimits(m_MinWidth, m_MaxWidth);
  Fitter.Config().ParSettings(12).SetName("GeneralizedNormal #3 - scale");
  Fitter.Config().ParSettings(12).SetLimits(m_MinScale, m_MaxScale);

  Fitter.Config().SetUpdateAfterFit();
}


////////////////////////////////////////////////////////////////////////////////


//! Perform a series of fits to find the best match
bool MARMFitter::PerformFit(unsigned int FitID, vector<double>& ARMValues)
{
  // Clean the data
  vector<double> CleanedData;
  CleanedData.reserve(ARMValues.size());
  for (auto A: ARMValues) {
    if (A >= -m_MaxARMValue && A <= m_MaxARMValue) {
      CleanedData.push_back(A);
    }
  }

  // Set up for fitting
  ROOT::Fit::DataOptions DataOptions;
  DataOptions.fIntegral = true;

  ROOT::Fit::DataRange Range(-m_MaxARMValue, m_MaxARMValue);

  TF1* FitFunction = nullptr;
  ROOT::Fit::Fitter Fitter; // need a new one every time
  SetupARMFit(m_ARMFitFunction, Fitter, &FitFunction);

  // Do the fitting
  if (m_UnbinnedFitting == true) {
    //cout<<"Unbinned fit"<<endl;
    ROOT::Fit::UnBinData UnbinnedData(CleanedData.size(), CleanedData.data(), Range);
    Fitter.LikelihoodFit(UnbinnedData, true);
  } else {
    //cout<<"Binned fit"<<endl;
    DataOptions.fIntegral = true;
    DataOptions.fUseRange =true;
    ROOT::Fit::BinData BinnedData(DataOptions, Range);
    // Easiest option to fill the binned data set is via a histogram:
    TH1D* D = new TH1D("", "", m_NumberOfBins, -m_MaxARMValue, m_MaxARMValue);
    for (auto& V: CleanedData) D->Fill(V);
    for (int b = 1; b <= D->GetNbinsX(); ++b) D->SetBinError(b, sqrt(D->GetBinContent(b)));
    ROOT::Fit::FillData(BinnedData, D);
    delete D;
    Fitter.LikelihoodFit(BinnedData, true);
    //Fitter.LeastSquareFit(BinnedData);
  }

  // Retrieve results
  TFitResult Result = Fitter.Result();

  FitFunction->SetParameters(Result.GetParams());

  // Calculate FWHM ...
  double FWHM = MInterface::GetFWHM(FitFunction, -m_MaxARMValue, m_MaxARMValue);

  {
    std::unique_lock<std::mutex> Lock(m_Mutex);

    m_BootStrappedFWHMSamples[FitID] = FWHM;

    m_BootStrappedFitParameters[FitID] = vector<double>();
    for (unsigned int i = 0; i < GetARMFitFunctionNumberOfParameters(m_ARMFitFunction); ++i) {
      m_BootStrappedFitParameters[FitID].push_back(Result.Parameter(i));
    }

    m_BootStrappedBakerCousins[FitID] = Result.MinFcnValue();

    Result.Print();
  }

  delete FitFunction;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perform fits with all fit functions and report Baker-Cousin likelihood ratios
bool MARMFitter::FitAll()
{
  unsigned int NFits = 500;

  MARMFitFunctionID Best = MARMFitFunctionID::c_Gauss;
  double BestBC = 0;

  cout<<"Fitting through all functions (except Lorentz) with "<<NFits<<" samples."<<endl;

  m_ARMFitFunction = MARMFitFunctionID::c_Gauss;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  //m_ARMFitFunction = MARMFitFunctionID::c_Lorentz;
  //FitMultiple(NFits);
  //if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  //cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_ARMFitFunction = MARMFitFunctionID::c_GeneralizedNormal;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_ARMFitFunction = MARMFitFunctionID::c_GaussLorentz;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_ARMFitFunction = MARMFitFunctionID::c_GaussLorentzLorentz;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_ARMFitFunction = MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_ARMFitFunction = MARMFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_ARMFitFunction = MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_ARMFitFunction = MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_ARMFitFunction; }
  cout<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  // Do the best again:
  m_ARMFitFunction = Best;
  Fit(2*NFits);
  cout<<"Best: "<<GetARMFitFunctionName(m_ARMFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  return m_FitSuccessful;
}


////////////////////////////////////////////////////////////////////////////////


//! Bootstrap the ARM values and return them as vector
vector<double> MARMFitter::BootstrapARMValues()
{
  std::uniform_int_distribution<> Distributor(0, m_OriginalARMValues.size() - 1);

  // Bootstrapping: sample with replacement
  vector<double> ARMValues;
  ARMValues.reserve(m_OriginalARMValues.size());
  for (size_t i = 0; i < m_OriginalARMValues.size(); ++i) {
    int D = Distributor(m_MersenneTwister);
    ARMValues.push_back(m_OriginalARMValues[D]);
  }

  return ARMValues;
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate all ARM metrics from the bootstrapped results
void MARMFitter::CalculateBootStrappedMetrics()
{
  m_FinalFWHM = g_DoubleNotDefined;
  m_FinalFWHMUncertainty = g_DoubleNotDefined;
  m_FinalFitParameters.clear();
  m_FinalBakerCousins = g_DoubleNotDefined;
  m_FinalBakerCousinsUncertainty = g_DoubleNotDefined;

  if (m_BootStrappedFWHMSamples.size() > 0) {
    // Calculate the average
    double SumFWHM = std::accumulate(m_BootStrappedFWHMSamples.begin(), m_BootStrappedFWHMSamples.end(), 0.0);
    m_FinalFWHM = SumFWHM / m_BootStrappedFWHMSamples.size();

    // Calculate the standard deviation
    double SquaredSumFWHM = std::accumulate(m_BootStrappedFWHMSamples.begin(), m_BootStrappedFWHMSamples.end(), 0.0,
        [FWHM = m_FinalFWHM](double acc, double x) {
            return acc + (x - FWHM) * (x - FWHM);
        });
    m_FinalFWHMUncertainty = std::sqrt(SquaredSumFWHM / m_BootStrappedFWHMSamples.size());

    // Find the bin with the FWHM closest to the average to choose the best fit parameters
    unsigned int ClosestBin = 0;
    //double ClosestValue = m_BootStrappedFWHMSamples[0];
    double MinDifference = std::abs(m_BootStrappedFWHMSamples[0] - m_FinalFWHM);

    for (unsigned int b = 1; b < m_BootStrappedFWHMSamples.size(); ++b) {
      double Difference = std::abs(m_BootStrappedFWHMSamples[b] - m_FinalFWHM);
      if (Difference < MinDifference) {
        MinDifference = Difference;
        //ClosestValue = m_BootStrappedFWHMSamples[b];
        ClosestBin = b;
      }
    }
    m_FinalFitParameters = m_BootStrappedFitParameters[ClosestBin];

    // The average Baker-Cousins value
    double SumBakerCousins = std::accumulate(m_BootStrappedBakerCousins.begin(), m_BootStrappedBakerCousins.end(), 0.0);
    m_FinalBakerCousins = SumBakerCousins / m_BootStrappedBakerCousins.size();

    // Calculate its standard deviation
    double SquaredSumBakerCousins = std::accumulate(m_BootStrappedBakerCousins.begin(), m_BootStrappedBakerCousins.end(), 0.0,
        [BC = m_FinalBakerCousins](double acc, double x) {
            return acc + (x - BC) * (x - BC);
        });
    m_FinalBakerCousinsUncertainty = std::sqrt(SquaredSumBakerCousins / m_BootStrappedBakerCousins.size());


    m_CountsFWHMWindow = g_UnsignedIntNotDefined;
    m_CountsFWHMWindowMinimum = g_UnsignedIntNotDefined;
    m_CountsFWHMWindowMaximum = g_UnsignedIntNotDefined;

    vector<double> SortedAbs;
    for (double& A: m_OriginalARMValues) {
      SortedAbs.push_back(fabs(A));
    }
    std::sort(SortedAbs.begin(), SortedAbs.end());

    for (unsigned int b = 0; b < SortedAbs.size(); ++b) {
      if (m_CountsFWHMWindowMinimum == g_UnsignedIntNotDefined && SortedAbs[b] >= m_FinalFWHM - m_FinalFWHMUncertainty) {
        m_CountsFWHMWindowMinimum = b;
      }
      if (m_CountsFWHMWindow == g_UnsignedIntNotDefined && SortedAbs[b] >= m_FinalFWHM) {
        m_CountsFWHMWindow = b;
      }
      if (m_CountsFWHMWindowMaximum == g_UnsignedIntNotDefined && SortedAbs[b] >= m_FinalFWHM + m_FinalFWHMUncertainty) {
        m_CountsFWHMWindowMaximum = b;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate all ARM metrics from the distribution itself
void MARMFitter::CalculateARMMetrics()
{
  // Determine containment radii

  double Sigma0 = 0.5;
  double Sigma1 = 0.6826;
  double Sigma2 = 0.9546;
  double Sigma3 = 0.9973;

  m_Containment50PercentUsingARMSelection = g_DoubleNotDefined;
  m_Containment1SigmaUsingARMSelection = g_DoubleNotDefined;
  m_Containment2SigmaUsingARMSelection = g_DoubleNotDefined;
  m_Containment3SigmaUsingARMSelection = g_DoubleNotDefined;

  vector<double> SortedAbsSelected;
  for (double& A: m_OriginalARMValues) {
    if (A >= -m_MaxARMValue && A <= m_MaxARMValue) {
      SortedAbsSelected.push_back(fabs(A));
    }
  }
  std::sort(SortedAbsSelected.begin(), SortedAbsSelected.end());

  double Total = SortedAbsSelected.size();
  for (unsigned int b = 0; b < SortedAbsSelected.size(); ++b) {
    if (m_Containment50PercentUsingARMSelection == g_DoubleNotDefined && b >= Sigma0*Total) {
      m_Containment50PercentUsingARMSelection = SortedAbsSelected[b];
    }
    if (m_Containment1SigmaUsingARMSelection == g_DoubleNotDefined && b >= Sigma1*Total) {
      m_Containment1SigmaUsingARMSelection = SortedAbsSelected[b];
    }
    if (m_Containment2SigmaUsingARMSelection == g_DoubleNotDefined && b >= Sigma2*Total) {
      m_Containment2SigmaUsingARMSelection = SortedAbsSelected[b];
    }
    if (m_Containment3SigmaUsingARMSelection == g_DoubleNotDefined && b >= Sigma3*Total) {
      m_Containment3SigmaUsingARMSelection = SortedAbsSelected[b];
    }
  }


  m_Containment50PercentUsingAllData = g_DoubleNotDefined;
  m_Containment1SigmaUsingAllData = g_DoubleNotDefined;
  m_Containment2SigmaUsingAllData = g_DoubleNotDefined;
  m_Containment3SigmaUsingAllData = g_DoubleNotDefined;

  vector<double> SortedAbsAll;
  for (double& A: m_OriginalARMValues) {
    SortedAbsAll.push_back(fabs(A));
  }
  std::sort(SortedAbsAll.begin(), SortedAbsAll.end());

  Total = SortedAbsAll.size();
  for (unsigned int b = 0; b < SortedAbsAll.size(); ++b) {
    if (m_Containment50PercentUsingAllData == g_DoubleNotDefined && b >= Sigma0*Total) {
      m_Containment50PercentUsingAllData = SortedAbsAll[b];
    }
    if (m_Containment1SigmaUsingAllData == g_DoubleNotDefined && b >= Sigma1*Total) {
      m_Containment1SigmaUsingAllData = SortedAbsAll[b];
    }
    if (m_Containment2SigmaUsingAllData == g_DoubleNotDefined && b >= Sigma2*Total) {
      m_Containment2SigmaUsingAllData = SortedAbsAll[b];
    }
    if (m_Containment3SigmaUsingAllData == g_DoubleNotDefined && b >= Sigma3*Total) {
      m_Containment3SigmaUsingAllData = SortedAbsAll[b];
    }
  }

}


////////////////////////////////////////////////////////////////////////////////


void MARMFitter::ParallelFitting(int FitID, std::condition_variable& CV)
{
  { // lock the mutex while we wait
    std::unique_lock<std::mutex> Lock(m_Mutex);
    CV.wait(Lock, [this]() { return m_NumberOfRunningThreads < m_MaximumNumberOfThreads; }); // the condition_variable releases the lock while waiting.
    ++m_NumberOfRunningThreads;
    //cout<<"Fit "<<FitID<<endl;
  }

  vector<double> ARMValues = BootstrapARMValues();
  PerformFit(FitID, ARMValues);

  { // lock the mutex while we wait
    std::unique_lock<std::mutex> lock(m_Mutex);
    --m_NumberOfRunningThreads;
    CV.notify_all();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Performs N fits. N must be 1 or larger.
//! Each fit samples randomly from the stored ARM values to determine the average width and error correctly
bool MARMFitter::Fit(unsigned int NumberOfFits)
{
  m_FitSuccessful = true;

  if (m_OptimizeBinning == true) {
    OptimizeBinning();
  }

  // Calculate the ARM metrics not depending on a fit
  CalculateARMMetrics();

  // Pre-calculate limits on fits

  // We need a histogram, to do it easily
  unsigned int NEvents = 0;
  TH1D* Hist = new TH1D("", "", m_NumberOfBins, -m_MaxARMValue, m_MaxARMValue);
  for (double V: m_OriginalARMValues) {
    if (V >= -15 && V <= 15) {
      Hist->Fill(V);
      NEvents++;
    }
  }
  m_MinHeight = 0;
  if (m_UnbinnedFitting == false) {
    m_MaxHeight = 1.25 * Hist->GetMaximum();
  } else {
    m_MaxHeight = 0.5*NEvents;
  }
  m_GuessHeight = 0.5*m_MaxHeight;

  m_MinWidth = 0.25*Hist->GetRMS();
  m_MaxWidth = m_MaxARMValue;
  m_GuessWidth = Hist->GetRMS();

  double MaxX = Hist->GetBinCenter(Hist->GetMaximumBin());
  m_GuessMaximumLow = MaxX - 0.5*Hist->GetRMS();
  m_GuessMaximumHigh = MaxX + 0.5*Hist->GetRMS();


  // Right size the result storage
  m_BootStrappedFWHMSamples.clear();
  m_BootStrappedFWHMSamples.resize(NumberOfFits);

  m_BootStrappedFitParameters.clear();
  m_BootStrappedFitParameters.resize(NumberOfFits);

  m_BootStrappedBakerCousins.clear();
  m_BootStrappedBakerCousins.resize(NumberOfFits);

  if (NumberOfFits > 1) {
    // Start the threads
    m_NumberOfRunningThreads = 0;
    vector<thread> Threads;
    condition_variable CV;

    for (unsigned int i = 0; i < NumberOfFits; ++i) {
      Threads.emplace_back([this, &CV, i]() { this->ParallelFitting(i, CV); } );
    }

    // Join all threads with the main thread
    for (thread& T: Threads) {
      T.join();
    }
  } else if (NumberOfFits == 1) {
    PerformFit(0, m_OriginalARMValues);
  }

  if (NumberOfFits >= 1) {
    // Caluclate the results
    CalculateBootStrappedMetrics();
  }

  return m_FitSuccessful;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the the FWHM result
double MARMFitter::GetAverageFWHM() const
{
  return m_FitSuccessful ? m_FinalFWHM : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the FWHM uncertainty result
double MARMFitter::GetAverageFWHMUncertainty() const
{
  return m_FitSuccessful ? m_FinalFWHMUncertainty : g_DoubleNotDefined;
}



////////////////////////////////////////////////////////////////////////////////


//! Return the 50.0% containment for events with +- maximum ARM value
double MARMFitter::Get50p0PercentContainmentUsingARMSelection() const
{
  return m_FitSuccessful ? m_Containment50PercentUsingARMSelection : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the 68.3% containment for events with +- maximum ARM value
double MARMFitter::Get68p3PercentContainmentUsingARMSelection() const
{
  return m_FitSuccessful ? m_Containment1SigmaUsingARMSelection : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the 95.5% containment for events with +- maximum ARM value
double MARMFitter::Get95p5PercentContainmentUsingARMSelection() const
{
  return m_FitSuccessful ? m_Containment2SigmaUsingARMSelection : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the 99.7% containment for events with +- maximum ARM value
double MARMFitter::Get99p7PercentContainmentUsingARMSelection() const
{
  return m_FitSuccessful ? m_Containment3SigmaUsingARMSelection : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the 50.0% containment for all events
double MARMFitter::Get50p0PercentContainmentUsingAllData() const
{
  return m_FitSuccessful ? m_Containment50PercentUsingAllData : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the 68.3% containment for all events
double MARMFitter::Get68p3PercentContainmentUsingAllData() const
{
  return m_FitSuccessful ? m_Containment1SigmaUsingAllData : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the 95.5% containment for all events
double MARMFitter::Get95p5PercentContainmentUsingAllData() const
{
  return m_FitSuccessful ? m_Containment2SigmaUsingAllData : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the 99.7% containment for all events
double MARMFitter::Get99p7PercentContainmentUsingAllData() const
{
  return m_FitSuccessful ? m_Containment3SigmaUsingAllData : g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the counts in the FWHM window or g_DoubleNotDefined if we don't have a FWHM
unsigned int MARMFitter::GetFWHMWindowCounts() const
{
  return m_FitSuccessful ? m_CountsFWHMWindow : g_UnsignedIntNotDefined;
}



////////////////////////////////////////////////////////////////////////////////


//! Get the counts in the FWHM minus uncertainty window or g_DoubleNotDefined if we don't have a FWHM uncertainty
unsigned int MARMFitter::GetFWHMWindowCountsMinimum() const
{
  return m_FitSuccessful ? m_CountsFWHMWindowMinimum : g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the counts in the FWHM plus uncertainty window or g_DoubleNotDefined if we don't have a FWHM uncertainty
unsigned int MARMFitter::GetFWHMWindowCountsMaximum() const
{
  return m_FitSuccessful ? m_CountsFWHMWindowMaximum : g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert the results into a string
MString MARMFitter::ToString()
{
  unsigned int SelectedEvents = 0;
  for (auto A: m_OriginalARMValues) {
    if (A >= -m_MaxARMValue && A <= m_MaxARMValue) {
      SelectedEvents++;
    }
  }

  ostringstream out;
  out<<endl;
  out<<"ARM fit results:"<<endl;
  out<<"================"<<endl;
  out<<endl;
  if (m_FitSuccessful == false) {
    out<<"Fit not yet performed or fit failed."<<endl;
    return out;
  }
  out<<"Events:"<<endl;
  out<<"  Total: "<<m_OriginalARMValues.size()<<endl;
  out<<"  In selection (+- "<<m_MaxARMValue<<" deg): "<<SelectedEvents<<endl;
  out<<endl;
  out<<"Fit:"<<endl;
  if (m_BootStrappedFWHMSamples.size() > 0) {
    out<<"  Fit mode: "<<(m_UnbinnedFitting == true ? "Unbinned" : "Binned")<<" likelihood fit"<<endl;
    out<<"  Fit function: "<<GetARMFitFunctionName(m_ARMFitFunction)<<endl;
    if (m_BootStrappedFWHMSamples.size() > 1) {
      out<<"  Average FWHM after "<<m_BootStrappedFWHMSamples.size()<<" bootstraps: "<<MString(m_FinalFWHM, m_FinalFWHMUncertainty, "degree")<<endl;
      out<<"  Counts in +-FWHM window: "<<m_CountsFWHMWindow<<" (-"<<m_CountsFWHMWindow-m_CountsFWHMWindowMinimum<<", +"<<m_CountsFWHMWindowMaximum-m_CountsFWHMWindow<<")"<<endl;
    } else {
      out<<"  FWHM after 1 fit: "<<setprecision(3)<<m_FinalFWHM<<" degree"<<endl;
      out<<"  Counts in +-FWHM window: "<<m_CountsFWHMWindow<<endl;
    }
  } else {
    out<<"  No fits performed"<<endl;
  }
  out<<endl;
  out<<"Containment:"<<endl;
  out<<"  using +- "<<m_MaxARMValue<<" deg ARM selection:"<<endl;
  out<<fixed<<setprecision(2);
  out<<"    50.0 %:\t"<<m_Containment50PercentUsingARMSelection<<" deg"<<endl;
  out<<"    68.3 %:\t"<<m_Containment1SigmaUsingARMSelection<<" deg"<<endl;
  out<<"    95.5 %:\t"<<m_Containment2SigmaUsingARMSelection<<" deg"<<endl;
  out<<"    99.7 %:\t"<<m_Containment3SigmaUsingARMSelection<<" deg"<<endl;
  out<<"  using all data:"<<endl;
  out<<"    50.0 %:\t"<<m_Containment50PercentUsingAllData<<" deg"<<endl;
  out<<"    68.3 %:\t"<<m_Containment1SigmaUsingAllData<<" deg"<<endl;
  out<<"    95.5 %:\t"<<m_Containment2SigmaUsingAllData<<" deg"<<endl;
  out<<"    99.7 %:\t"<<m_Containment3SigmaUsingAllData<<" deg"<<endl;
  out<<endl;

  return out;
}


////////////////////////////////////////////////////////////////////////////////


//! Draw the ARM histogram
void MARMFitter::Draw()
{
  if (m_OptimizeBinning == true) {
    OptimizeBinning();
  }


  TH1D* Histogram = new TH1D("ARMComptonCone", "Angular Resolution Measure", m_NumberOfBins, -m_MaxARMValue, m_MaxARMValue);
  Histogram->SetBit(kCanDelete);
  Histogram->SetDirectory(0);
  Histogram->SetXTitle("ARM [#circ]");
  Histogram->SetYTitle("counts");
  Histogram->SetTitleOffset(1.3f, "Y");
  Histogram->SetStats(false);
  Histogram->SetLineColor(kBlack);
  Histogram->SetMinimum(0);

  for (double& ARMValue: m_OriginalARMValues) {
    Histogram->Fill(ARMValue);
  }
  Histogram->Draw("HIST"); // This defines all scales

  // Create a new fit since we need to scale the existing one,
  // since there the scaler height is a function of the bins (and we did an unbinned fit)
  unsigned int NumberOfParameters = GetARMFitFunctionNumberOfParameters(m_ARMFitFunction);
  MString FitString = GetARMFitFunction(m_ARMFitFunction);
  FitString.ReplaceAll("return", "return (");
  MString Addendum = ") * p[";
  Addendum += NumberOfParameters;
  Addendum += "]; }";
  FitString.ReplaceAll("; }", Addendum);

  TF1* ScaledFit = new TF1("ScaledFit", FitString.Data(), -m_MaxARMValue, m_MaxARMValue, NumberOfParameters+1);
  for (unsigned int f = 0; f < m_BootStrappedFitParameters.size(); ++f) {
    //cout<<"Fit "<<f<<endl;
    ScaledFit->SetName(MString("ScaledFit")+f);
    for (unsigned int i = 0; i < NumberOfParameters; ++i) {
      ScaledFit->FixParameter(i, m_BootStrappedFitParameters[f][i]);
    }

    Histogram->Fit(ScaledFit, "QR");
    double Alpha = 1.0 / (0.1 * m_BootStrappedFitParameters.size());
    if (Alpha > 1.0) Alpha = 1.0;
    if (Alpha < 0.005) Alpha = 0.005;
    ScaledFit->SetLineColorAlpha(kAzure-2, Alpha);
    ScaledFit->SetLineWidth(4);
    ScaledFit->DrawCopy("SAME");
  }
  delete ScaledFit;

  Histogram->Draw("HIST SAME"); // Overdraws the fits

  if (m_BootStrappedFWHMSamples.size() > 0) {
    MString Text("FWHM of fit function: ");
    if (m_BootStrappedFWHMSamples.size() == 1) {
      Text += MString(m_FinalFWHM, 3u);
      Text += "#circ";
    } else {
      Text += MString(m_FinalFWHM, m_FinalFWHMUncertainty, "#circ", true);
    }

    TLatex* Latex = new TLatex();
    Latex->SetTextFont(gStyle->GetLabelFont());
    Latex->SetTextSize(gStyle->GetLabelSize());
    Latex->SetTextAlign(21);
    Latex->DrawLatexNDC(0.5, 0.02, Text.Data());
  }

  // No delete Histogram - Histogramn deletion managed by ROOT

  return;
}


////////////////////////////////////////////////////////////////////////////////


//! Load the ARM value from file
bool MARMFitter::LoadARMValues(MString FileName)
{
  m_OriginalARMValues.clear();

  ifstream fin;
  fin.open(FileName);
  if (fin.is_open() == false) {
    merr<<"Unable to open "<<FileName<<endl;
    return false;
  }

  string Line;
  string Prefix;
  double Value;
  while (getline(fin, Line)) {
    if (Line.size() < 4 || Line.substr(0, 2) != "DP") continue;
    istringstream iss(Line);
    // Read the prefix ("DP") and the value
    if (iss>>Prefix>>Value) {
      m_OriginalARMValues.push_back(Value);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Save the the ARM values to a file
bool MARMFitter::SaveARMValues(MString FileName)
{
  if (m_OriginalARMValues.size() > 0) {
    ofstream fout;
    fout.open(FileName);
    if (fout.is_open() == false) {
      merr<<"Unable to open "<<FileName<<endl;
      return false;
    }
    fout<<"# ARM data file"<<endl;
    fout<<endl;
    for (auto& A: m_OriginalARMValues) {
      fout<<"DP "<<A<<endl;
    }
    fout<<endl;
    fout.close();
  }

  return true;
}


// MARMFitter.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
