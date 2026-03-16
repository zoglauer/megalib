/*
 * MLineFitter.cxx
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
#include "MLineFitter.h"

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
ClassImp(MLineFitter)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MLineFitter::MLineFitter()
{
  ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(250000);

  std::random_device RD;
  std::mt19937 m_MersenneTwister(RD());

  m_NumberOfBins = 100;
  m_MaximumEnergyValue = 10000;
  m_MinimumEnergyValue = 0;
  m_LineFitFunction = MLineFitFunctionID::c_Gauss;

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
MLineFitter::~MLineFitter()
{
}


////////////////////////////////////////////////////////////////////////////////


//!
void MLineFitter::SetNumberOfBins(unsigned int NumberOfBins)
{
  m_NumberOfBins = NumberOfBins;
  if (m_NumberOfBins < 10) {
    merr<<"You need at least 10 bins to be able to fit a line, and not "<<NumberOfBins<<". Setting it to 10."<<endl;
    m_NumberOfBins = 10;
  }
  m_IsBinningOptimized = false;
}


////////////////////////////////////////////////////////////////////////////////


//!
void MLineFitter::SetEnergyRange(double MinimumEnergyValue, double MaximumEnergyValue)
{
  m_MinimumEnergyValue = MinimumEnergyValue;
  m_MaximumEnergyValue = MaximumEnergyValue;
  if (m_MinimumEnergyValue <= 0) {
    merr<<"The minimum energy value must be non-negative and not "<<MinimumEnergyValue<<endl<<"Setting it to 0"<<endl;
    m_MinimumEnergyValue = 0;
  }
  if (m_MaximumEnergyValue <= m_MinimumEnergyValue) {
    merr<<"The maximum energy value must ne larger than the minimum value "<<MinimumEnergyValue<<endl<<"Setting it to minimum + 100 keV"<<endl;
    m_MaximumEnergyValue = m_MinimumEnergyValue + 100;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set the test position, and its coordinate system
void MLineFitter::SetTestPosition(MVector TestPosition, MCoordinateSystem CoordinateSystem)
{
  m_TestPosition = TestPosition;
  m_CoordinateSystem = CoordinateSystem;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit this function to the data
//! The ID corresponds to the position of the corresponding MLineFitFunctionID in the vector retuned by GetListOfFitFunctions()
//! If it's not in there use c_AsymmetricGaussLorentzLorentz
void MLineFitter::SetFitFunction(unsigned int ID)
{
  vector<MLineFitFunctionID> IDs = GetListOfFitFunctions();

  if (ID < IDs.size()) {
    m_LineFitFunction = IDs[ID];
  } else {
    merr<<"There is no function with ID n"<<ID<<". Choosing assymmetric gauss + Lorentz + Lorentz"<<endl;
    m_LineFitFunction = MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Reset the internally stored Line values
void MLineFitter::Reset()
{
  m_OriginalEnergyValues.clear();

  m_BootStrappedFWHMSamples.clear();
  m_BootStrappedFitParameters.clear();
  m_BootStrappedBakerCousins.clear();

  m_FitSuccessful = false;
  m_FinalFWHM = g_DoubleNotDefined;
  m_FinalFWHMUncertainty = g_DoubleNotDefined;
  m_FinalFitParameters.clear();
  m_FinalBakerCousins = g_DoubleNotDefined;
  m_FinalBakerCousinsUncertainty = g_DoubleNotDefined;

  m_CountsFWHMWindow = g_UnsignedIntNotDefined;
  m_CountsFWHMWindowMinimum = g_UnsignedIntNotDefined;
  m_CountsFWHMWindowMaximum = g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Add event - only stores the actual energy value
void MLineFitter::AddEvent(MPhysicalEvent* Event)
{
  m_OriginalEnergyValues.push_back(Event->GetEnergy());
}


////////////////////////////////////////////////////////////////////////////////


//! Optimize binning - checks if the binning is OK, otherwise check for a better one
bool MLineFitter::OptimizeBinning()
{
  if (m_IsBinningOptimized == true) return true;

  const unsigned int MinimumBinContent = 50;
  const unsigned int MinimumBins = 5;

  bool BinningOptimized = false;

  unsigned int NumberOfBins = m_NumberOfBins;

  while (BinningOptimized == false) {

    double BinWidth = 2*(m_MaximumEnergyValue - m_MinimumEnergyValue) / NumberOfBins;
    vector<unsigned int> BinnedData(NumberOfBins, 0);

    for (auto A: m_OriginalEnergyValues) {
      if (A >= m_MinimumEnergyValue && A <= m_MaximumEnergyValue) {
        unsigned int BinIndex = static_cast<unsigned int>((A + (m_MaximumEnergyValue - m_MinimumEnergyValue)) / BinWidth);

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

      mout<<"We do not have at least "<<MinimumBinContent<<" counts in any bin of the energy histogram, but only "<<Maximum<<endl;

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
MString MLineFitter::GetLineFitFunctionName(MLineFitFunctionID ID)
{
  if (ID == MLineFitFunctionID::c_Gauss) {
    return MString("Gauss");
  } else if (ID == MLineFitFunctionID::c_Lorentz) {
    return MString("Lorentz");
  } else if (ID == MLineFitFunctionID::c_GeneralizedNormal) {
    return MString("Generalized-Normal");
  } else if (ID == MLineFitFunctionID::c_GaussLorentz) {
    return MString("Gauss + Lorentz");
  } else if (ID == MLineFitFunctionID::c_GaussLorentzLorentz) {
    return MString("Gauss + Lorentz + Lorentz");
  } else if (ID == MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz) {
    return MString("Asymmetric-Gauss + Lorentz + Lorentz");
  } else if (ID == MLineFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz) {
    return MString("Asymmetric-Gauss + Gauss + Lorentz + Lorentz");
  } else if (ID == MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal) {
    return MString("Asymmetric-Generalized-Normal + Generalized-Normal");
  } else if (ID == MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal) {
    return MString("Asymmetric-Generalized-Normal + Generalized-Normal + Generalized-Normal");
  } else {
    new MExceptionObjectDoesNotExist("Unknown Line fit function ID");
    return MString("Unknown Line fit function ID");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the function of the fit given the fit function ID
MString MLineFitter::GetLineFitFunction(MLineFitFunctionID ID)
{
  if (ID == MLineFitFunctionID::c_Gauss) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)); }");
  } else if (ID == MLineFitFunctionID::c_Lorentz) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*(p[3]*p[3])/(p[3]*p[3] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MLineFitFunctionID::c_GeneralizedNormal) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[3], p[4])); }");
  } else if (ID == MLineFitFunctionID::c_GaussLorentz) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) + p[4]/(p[5]*p[5] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MLineFitFunctionID::c_GaussLorentzLorentz) {
    return MString("[&](double *x, double *p){ return p[0] + p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) + p[4]*(p[5]*p[5])/(p[5]*p[5] + (x[0]-p[1])*(x[0]-p[1])) + p[6]*(p[7]*p[7])/(p[7]*p[7] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz) {
    return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) : p[2]*exp(-0.5*pow((x[0]-p[1])/p[4], 2)) ) + p[5]*(p[6]*p[6])/(p[6]*p[6] + (x[0]-p[1])*(x[0]-p[1])) + p[7]*(p[8]*p[8])/(p[8]*p[8] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MLineFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz) {
    //return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) : p[2]*exp(-0.5*pow((x[0]-p[1])/p[4], 2)) )  +  p[5]*exp(-0.5*pow((x[0]-p[1])/p[6], 2))  +  p[7]*(p[8]*p[8])/(p[8]*p[8] + (x[0]-p[1])*(x[0]-p[1]))  +  p[9]*(p[10]*p[10])/(p[10]*p[10] + (x[0]-p[1])*(x[0]-p[1])); }");
    return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow((x[0]-p[1])/p[3], 2)) : p[2]*exp(-0.5*pow((x[0]-p[1])/p[4], 2)) )  +  p[5]*exp(-0.5*pow((x[0]-p[1])/p[6], 2))  +  p[7]*p[8]/(p[8]*p[8] + (x[0]-p[1])*(x[0]-p[1]))  +  p[9]*p[10]/(p[10]*p[10] + (x[0]-p[1])*(x[0]-p[1])); }");
  } else if (ID == MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal) {
    return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[3], p[4])) : p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[5], p[6])) )  +  p[7]*exp(-0.5*pow(fabs(x[0]-p[1])/p[8], p[9])) ; }");
  } else if (ID == MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal) {
    return MString("[&](double *x, double *p){ return p[0] + (x[0] - p[1] >= 0 ? p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[3], p[4])) : p[2]*exp(-0.5*pow(fabs(x[0]-p[1])/p[5], p[6])) )  +  p[7]*exp(-0.5*pow(fabs(x[0]-p[1])/p[8], p[9]))  +  p[10]*exp(-0.5*pow(fabs(x[0]-p[1])/p[11], p[12])) ; }");
  } else {
    new MExceptionObjectDoesNotExist("Unknown Line fit function ID");
    return MString("Unknown Line fit function ID");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of parameters of the fit given the fit function ID
unsigned int MLineFitter::GetLineFitFunctionNumberOfParameters(MLineFitFunctionID ID)
{
   if (ID == MLineFitFunctionID::c_Gauss) {
    return 4;
  } else if (ID == MLineFitFunctionID::c_Lorentz) {
    return 4;
  } else if (ID == MLineFitFunctionID::c_GeneralizedNormal) {
    return 5;
  } else if (ID == MLineFitFunctionID::c_GaussLorentz) {
    return 6;
  } else if (ID == MLineFitFunctionID::c_GaussLorentzLorentz) {
    return 8;
  } else if (ID == MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz) {
    return 9;
  } else if (ID == MLineFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz) {
    return 11;
  } else if (ID == MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal) {
    return 10;
  } else if (ID == MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal) {
    return 13;
  } else {
    new MExceptionObjectDoesNotExist("Unknown Line fit function ID");
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get a list of all fit functions:
vector<MLineFitFunctionID> MLineFitter::GetListOfFitFunctions() const
{
  vector<MLineFitFunctionID> List;
  List.push_back(MLineFitFunctionID::c_Gauss);
  List.push_back(MLineFitFunctionID::c_Lorentz);
  List.push_back(MLineFitFunctionID::c_GeneralizedNormal);
  List.push_back(MLineFitFunctionID::c_GaussLorentz);
  List.push_back(MLineFitFunctionID::c_GaussLorentzLorentz);
  List.push_back(MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz);
  List.push_back(MLineFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz);
  List.push_back(MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal);
  List.push_back(MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal);

  return List;
}


////////////////////////////////////////////////////////////////////////////////


//! Switch between the fit functions
void MLineFitter::SetupLineFit(MLineFitFunctionID LineFitID, ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  if (LineFitID == MLineFitFunctionID::c_Gauss) {
    SetupLineFitGauss(Fitter, FitFunction);
  } else if (LineFitID == MLineFitFunctionID::c_Lorentz) {
    SetupLineFitLorentz(Fitter, FitFunction);
  } else if (LineFitID == MLineFitFunctionID::c_GeneralizedNormal) {
    SetupLineFitGeneralizedNormal(Fitter, FitFunction);
  } else if (LineFitID == MLineFitFunctionID::c_GaussLorentz) {
    SetupLineFitGaussLorentz(Fitter, FitFunction);
  } else if (LineFitID == MLineFitFunctionID::c_GaussLorentzLorentz) {
    SetupLineFitGaussLorentzLorentz(Fitter, FitFunction);
  } else if (LineFitID == MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz) {
    SetupLineFitAsymmetricGaussLorentzLorentz(Fitter, FitFunction);
  } else if (LineFitID == MLineFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz) {
    SetupLineFitAsymmetricGaussGaussLorentzLorentz(Fitter, FitFunction);
  } else if (LineFitID == MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal) {
    SetupLineFitAsymmetricGeneralizedNormalGeneralizedNormal(Fitter, FitFunction);
  } else if (LineFitID == MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal) {
    SetupLineFitAsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal(Fitter, FitFunction);
  } else {
    new MExceptionObjectDoesNotExist("Unknown Line fit function ID");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Setup a Gauss fit function
void MLineFitter::SetupLineFitGauss(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_Gauss).Data(), GetLineFitFunction(MLineFitFunctionID::c_Gauss).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 4);
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
void MLineFitter::SetupLineFitLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_Lorentz).Data(), GetLineFitFunction(MLineFitFunctionID::c_Lorentz).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 4);
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
void MLineFitter::SetupLineFitGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_GeneralizedNormal).Data(), GetLineFitFunction(MLineFitFunctionID::c_GeneralizedNormal).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 5);
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
void MLineFitter::SetupLineFitGaussLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_GaussLorentz).Data(), GetLineFitFunction(MLineFitFunctionID::c_GaussLorentz).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 6);
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
void MLineFitter::SetupLineFitGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_GaussLorentzLorentz).Data(), GetLineFitFunction(MLineFitFunctionID::c_GaussLorentzLorentz).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 8);
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
void MLineFitter::SetupLineFitAsymmetricGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction  = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz).Data(), GetLineFitFunction(MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 9);
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
void MLineFitter::SetupLineFitAsymmetricGaussGaussLorentzLorentz(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction  = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz).Data(), GetLineFitFunction(MLineFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 11);
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
void MLineFitter::SetupLineFitAsymmetricGeneralizedNormalGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction  = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal).Data(), GetLineFitFunction(MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 10);
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
void MLineFitter::SetupLineFitAsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal(ROOT::Fit::Fitter& Fitter, TF1** FitFunction)
{
  *FitFunction  = new TF1(GetLineFitFunctionName(MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal).Data(), GetLineFitFunction(MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal).Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, 13);
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
bool MLineFitter::PerformFit(unsigned int FitID, vector<double>& EnergyValues)
{
  // Clean the data
  vector<double> CleanedData;
  CleanedData.reserve(EnergyValues.size());
  for (auto A: EnergyValues) {
    if (A >= m_MinimumEnergyValue && A <= m_MaximumEnergyValue) {
      CleanedData.push_back(A);
    }
  }

  // Set up for fitting
  ROOT::Fit::DataOptions DataOptions;
  DataOptions.fIntegral = true;

  ROOT::Fit::DataRange Range(m_MinimumEnergyValue, m_MaximumEnergyValue);

  TF1* FitFunction = nullptr;
  ROOT::Fit::Fitter Fitter; // need a new one every time
  SetupLineFit(m_LineFitFunction, Fitter, &FitFunction);

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
    TH1D* D = new TH1D("", "", m_NumberOfBins, m_MinimumEnergyValue, m_MaximumEnergyValue);
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
  double FWHM = MInterface::GetFWHM(FitFunction, m_MinimumEnergyValue, m_MaximumEnergyValue);

  {
    std::unique_lock<std::mutex> Lock(m_Mutex);

    m_BootStrappedFWHMSamples[FitID] = FWHM;

    m_BootStrappedFitParameters[FitID] = vector<double>();
    for (unsigned int i = 0; i < GetLineFitFunctionNumberOfParameters(m_LineFitFunction); ++i) {
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
bool MLineFitter::FitAll()
{
  unsigned int NFits = 500;

  MLineFitFunctionID Best = MLineFitFunctionID::c_Gauss;
  double BestBC = 0;

  cout<<"Fitting through all functions (except Lorentz) with "<<NFits<<" samples."<<endl;

  m_LineFitFunction = MLineFitFunctionID::c_Gauss;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  //m_LineFitFunction = MLineFitFunctionID::c_Lorentz;
  //FitMultiple(NFits);
  //if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  //cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_LineFitFunction = MLineFitFunctionID::c_GeneralizedNormal;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_LineFitFunction = MLineFitFunctionID::c_GaussLorentz;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_LineFitFunction = MLineFitFunctionID::c_GaussLorentzLorentz;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_LineFitFunction = MLineFitFunctionID::c_AsymmetricGaussLorentzLorentz;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_LineFitFunction = MLineFitFunctionID::c_AsymmetricGaussGaussLorentzLorentz;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_LineFitFunction = MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  m_LineFitFunction = MLineFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal;
  Fit(NFits);
  if (m_FinalBakerCousins < BestBC) { BestBC = m_FinalBakerCousins; Best = m_LineFitFunction; }
  cout<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  // Do the best again:
  m_LineFitFunction = Best;
  Fit(2*NFits);
  cout<<"Best: "<<GetLineFitFunctionName(m_LineFitFunction)<<" - BC: "<<MString(m_FinalBakerCousins, m_FinalBakerCousinsUncertainty)<<endl;

  return m_FitSuccessful;
}


////////////////////////////////////////////////////////////////////////////////


//! Bootstrap the energy values and return them as vector
vector<double> MLineFitter::BootstrapEnergyValues()
{
  std::uniform_int_distribution<> Distributor(0, m_OriginalEnergyValues.size() - 1);

  // Bootstrapping: sample with replacement
  vector<double> EnergyValues;
  EnergyValues.reserve(m_OriginalEnergyValues.size());
  for (size_t i = 0; i < m_OriginalEnergyValues.size(); ++i) {
    int D = Distributor(m_MersenneTwister);
    EnergyValues.push_back(m_OriginalEnergyValues[D]);
  }

  return EnergyValues;
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate all Line metrics from the bootstrapped results
void MLineFitter::CalculateBootStrappedMetrics()
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
    for (double& A: m_OriginalEnergyValues) {
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


//! Calculate all Line metrics from the distribution itself
void MLineFitter::CalculateLineMetrics()
{
  // Determine containment radii
}


////////////////////////////////////////////////////////////////////////////////


void MLineFitter::ParallelFitting(int FitID, std::condition_variable& CV)
{
  vector<double> EnergyValues;
  { // lock the mutex while we wait
    std::unique_lock<std::mutex> Lock(m_Mutex);
    CV.wait(Lock, [this]() { return m_NumberOfRunningThreads < m_MaximumNumberOfThreads; }); // the condition_variable releases the lock while waiting.
    ++m_NumberOfRunningThreads;
    //cout<<"Fit "<<FitID<<endl;
    EnergyValues = BootstrapEnergyValues();
  }

  PerformFit(FitID, EnergyValues);

  { // lock the mutex while we wait
    std::unique_lock<std::mutex> lock(m_Mutex);
    --m_NumberOfRunningThreads;
    CV.notify_all();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Performs N fits. N must be 1 or larger.
//! Each fit samples randomly from the stored energy values to determine the average width and error correctly
bool MLineFitter::Fit(unsigned int NumberOfFits)
{
  m_FitSuccessful = true;

  if (m_OptimizeBinning == true) {
    OptimizeBinning();
  }

  // Calculate the line metrics not depending on a fit
  CalculateLineMetrics();

  // Pre-calculate limits on fits

  // We need a histogram, to do it easily
  unsigned int NEvents = 0;
  TH1D* Hist = new TH1D("", "", m_NumberOfBins, m_MinimumEnergyValue, m_MaximumEnergyValue);
  for (double V: m_OriginalEnergyValues) {
    if (V >= m_MinimumEnergyValue && V <= m_MaximumEnergyValue) {
      Hist->Fill(V);
      NEvents++;
    }
  }
  if (NEvents == 0) {
    merr<<"No events for the fit!"<<endl;
    return false;
  }

  m_MinHeight = 0;
  if (m_UnbinnedFitting == false) {
    m_MaxHeight = 1.25 * Hist->GetMaximum();
  } else {
    m_MaxHeight = 0.5*NEvents;
  }
  m_GuessHeight = 0.5*m_MaxHeight;

  //cout<<"Height: min:"<<m_MinHeight<<" max:"<<m_MaxHeight<<" guess: "<<m_GuessHeight<<endl;


  double RMS = Hist->GetRMS();

  m_MinWidth = 0.25*Hist->GetRMS();
  m_MaxWidth = m_MaximumEnergyValue;
  m_GuessWidth = Hist->GetRMS();

  //cout<<"Width: min: "<<m_MinWidth<<" max: "<<m_MaxWidth<<" guess: "<<m_GuessWidth<<endl;


  double MaxX = Hist->GetBinCenter(Hist->GetMaximumBin());
  m_GuessMaximumLow = MaxX - 0.5*RMS;
  m_GuessMaximumHigh = MaxX + 0.5*RMS;

  //cout<<"Width: "<<m_GuessMaximumLow<<":"<<m_GuessMaximumHigh<<endl;


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
    PerformFit(0, m_OriginalEnergyValues);
  }

  if (NumberOfFits >= 1) {
    // Caluclate the results
    CalculateBootStrappedMetrics();
  }

  return m_FitSuccessful;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the the FWHM result
double MLineFitter::GetAverageFWHM() const
{
  return m_FitSuccessful ? m_FinalFWHM : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the FWHM uncertainty result
double MLineFitter::GetAverageFWHMUncertainty() const
{
  return m_FitSuccessful ? m_FinalFWHMUncertainty : g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the counts in the FWHM window or g_DoubleNotDefined if we don't have a FWHM
unsigned int MLineFitter::GetFWHMWindowCounts() const
{
  return m_FitSuccessful ? m_CountsFWHMWindow : g_UnsignedIntNotDefined;
}



////////////////////////////////////////////////////////////////////////////////


//! Get the counts in the FWHM minus uncertainty window or g_DoubleNotDefined if we don't have a FWHM uncertainty
unsigned int MLineFitter::GetFWHMWindowCountsMinimum() const
{
  return m_FitSuccessful ? m_CountsFWHMWindowMinimum : g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the counts in the FWHM plus uncertainty window or g_DoubleNotDefined if we don't have a FWHM uncertainty
unsigned int MLineFitter::GetFWHMWindowCountsMaximum() const
{
  return m_FitSuccessful ? m_CountsFWHMWindowMaximum : g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert the results into a string
MString MLineFitter::ToString()
{
  unsigned int SelectedEvents = 0;
  for (auto A: m_OriginalEnergyValues) {
    if (A >= m_MinimumEnergyValue && A <= m_MaximumEnergyValue) {
      SelectedEvents++;
    }
  }

  ostringstream out;
  out<<endl;
  out<<"Line fit results:"<<endl;
  out<<"================"<<endl;
  out<<endl;
  if (m_FitSuccessful == false) {
    out<<"Fit not yet performed or fit failed."<<endl;
    return out;
  }
  out<<"Events:"<<endl;
  out<<"  Total: "<<m_OriginalEnergyValues.size()<<endl;
  out<<"  In selection (+- "<<m_MaximumEnergyValue<<" deg): "<<SelectedEvents<<endl;
  out<<endl;
  out<<"Fit:"<<endl;
  if (m_BootStrappedFWHMSamples.size() > 0) {
    out<<"  Fit mode: "<<(m_UnbinnedFitting == true ? "Unbinned" : "Binned")<<" likelihood fit"<<endl;
    out<<"  Fit function: "<<GetLineFitFunctionName(m_LineFitFunction)<<endl;
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


  return out;
}


////////////////////////////////////////////////////////////////////////////////


//! Draw the Line histogram
void MLineFitter::Draw()
{
  if (m_OptimizeBinning == true) {
    OptimizeBinning();
  }


  TH1D* Histogram = new TH1D("LineFit", "Energy Resolution", m_NumberOfBins, m_MinimumEnergyValue, m_MaximumEnergyValue);
  Histogram->SetBit(kCanDelete);
  Histogram->SetDirectory(0);
  Histogram->SetXTitle("Energy [keV]");
  Histogram->SetYTitle("counts");
  Histogram->SetTitleOffset(1.3f, "Y");
  Histogram->SetStats(false);
  Histogram->SetLineColor(kBlack);
  Histogram->SetMinimum(0);

  for (double& EnergyValue: m_OriginalEnergyValues) {
    Histogram->Fill(EnergyValue);
  }
  Histogram->Draw("HIST"); // This defines all scales

  // Create a new fit since we need to scale the existing one,
  // since there the scaler height is a function of the bins (and we did an unbinned fit)
  unsigned int NumberOfParameters = GetLineFitFunctionNumberOfParameters(m_LineFitFunction);
  MString FitString = GetLineFitFunction(m_LineFitFunction);
  FitString.ReplaceAll("return", "return (");
  MString Addendum = ") * p[";
  Addendum += NumberOfParameters;
  Addendum += "]; }";
  FitString.ReplaceAll("; }", Addendum);

  TF1* ScaledFit = new TF1("ScaledFit", FitString.Data(), m_MinimumEnergyValue, m_MaximumEnergyValue, NumberOfParameters+1);
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
      Text += " keV";
    } else {
      Text += MString(m_FinalFWHM, m_FinalFWHMUncertainty, " keV", true);
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


//! Load the energy value from file
bool MLineFitter::LoadEnergyValues(MString FileName)
{
  m_OriginalEnergyValues.clear();

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
      m_OriginalEnergyValues.push_back(Value);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Save the the energy values to a file
bool MLineFitter::SaveEnergyValues(MString FileName)
{
  if (m_OriginalEnergyValues.size() > 0) {
    ofstream fout;
    fout.open(FileName);
    if (fout.is_open() == false) {
      merr<<"Unable to open "<<FileName<<endl;
      return false;
    }
    fout<<"# Energy data file"<<endl;
    fout<<endl;
    for (auto& A: m_OriginalEnergyValues) {
      fout<<"DP "<<A<<endl;
    }
    fout<<endl;
    fout.close();
  }

  return true;
}


// MLineFitter.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
