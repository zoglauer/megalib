/*
 * MCalibrationFitGaussian.cxx
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
#include "MCalibrationFitGaussian.h"

// Standard libs:

// ROOT libs:
#include "TMath.h"
#include "TFitResult.h"
#include "TVirtualFitter.h"
#include "TH1.h"
#include "TF1.h"
#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"
#include "Math/MinimizerOptions.h"
#include "Fit/BinData.h"
#include "Fit/UnBinData.h"
#include "HFitInterface.h"
#include "Fit/Fitter.h"

// MEGAlib libs:
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrationFitGaussian)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationFitGaussian::MCalibrationFitGaussian() : MCalibrationFit()
{
  m_GaussianMean = g_DoubleNotDefined;
  m_GaussianSigma = g_DoubleNotDefined;
  m_GaussianHeight = g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Copy constructor
MCalibrationFitGaussian::MCalibrationFitGaussian(const MCalibrationFitGaussian& Fit)
{
  // We don't care about double initilization, so:
  m_Fit = 0;
  *this = Fit;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationFitGaussian::~MCalibrationFitGaussian()
{
}


////////////////////////////////////////////////////////////////////////////////


//! The assignment operator
MCalibrationFitGaussian& MCalibrationFitGaussian::operator= (const MCalibrationFitGaussian& Fit)
{
  MCalibrationFit::operator=(Fit);

  m_GaussianMean = Fit.m_GaussianMean;
  m_GaussianSigma = Fit.m_GaussianSigma;
  m_GaussianHeight = Fit.m_GaussianHeight;
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this fit - the returned element must be deleted!
MCalibrationFitGaussian* MCalibrationFitGaussian::Clone() const
{
  return new MCalibrationFitGaussian(*this);
}


////////////////////////////////////////////////////////////////////////////////


//! The function for ROOT fitting
double MCalibrationFitGaussian::DoEvalPar(double X, const double* P) const
{
  double Return = 0.0;
  Return += BackgroundFit(X, P);
  Return += EnergyLossFit(X, P);
  
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int Ps = GetBackgroundFitParameters();
    if (P[Ps+3] != 0) {
      double Arg = ((X - P[Ps+2])/P[Ps+3]);
      //Return += P[Ps+4]/sqrt(2*TMath::Pi())/P[Ps+3] * TMath::Exp(-0.5*Arg*Arg); // Integral is 1
      Return += P[Ps+4] * TMath::Exp(-0.5*Arg*Arg); // Height is 1
    }
  } else {
    int Ps = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    if (P[Ps+1] != 0) {
      double Arg = ((X - P[Ps])/P[Ps+1]);
      //Return += P[Ps+2]/sqrt(2*TMath::Pi())/P[Ps+1] * TMath::Exp(-0.5*Arg*Arg); // Integral is 1
      Return += P[Ps+2] * TMath::Exp(-0.5*Arg*Arg); // Height is 1
    }
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Number of parameters of the fit
unsigned int MCalibrationFitGaussian::NPar() const
{
  unsigned int Parameters = GetBackgroundFitParameters();
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    Parameters += 5;
  } else {
    Parameters += GetEnergyLossFitParameters() + 3;
  }
  return Parameters;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the given histogram in the given range
bool MCalibrationFitGaussian::Fit(TH1D& Histogram, double Min, double Max)
{
  // Clean up the old fit:
  TThread::Lock();
  delete m_Fit;
  TThread::UnLock();
  m_Fit = 0;
  m_IsFitUpToDate = false;
  m_AverageDeviation = 1;
  
  int Parameters = NPar();
  
  // Prepare the array for ROOT
  m_ROOTParameters.resize(Parameters, 0);
  
  
  ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(20000);

  ROOT::Fit::DataOptions Options; 
  ROOT::Fit::DataRange Range; 
  Range.SetRange(Min, Max);
  ROOT::Fit::BinData TheData(Options, Range); 
  ROOT::Fit::FillData(TheData, &Histogram);

  ROOT::Fit::Fitter TheFitter; 
  TheFitter.Config().SetMinimizer("Minuit2");
  if (g_Verbosity >= c_Info) TheFitter.Config().MinimizerOptions().SetPrintLevel(1);
  
  TheFitter.SetFunction(*this);
  
  SetFitParameters(TheFitter, Histogram, Min, Max);
  
  /*
  bool ReturnCode = TheFitter.Fit(TheData);
  cout<<"Return code: "<<<<endl;
  if (ReturnCode == true) {
    if (TheFitter.CalculateHessErrors() == false) {
      if (TheFitter.CalculateMinosErrors() == false) {
        cout<<"Unable to calculate either Minos or Hess error!"<<endl;
        ReturnCode = false;
      }
    }
  }
  */
  
  if (TheFitter.Fit(TheData) == false) {
    cout<<"The fit is not perfectly OK..."<<endl;
  }
  
  const ROOT::Fit::FitResult& TheFitResult = TheFitter.Result(); 
  bool ReturnCode = true;
  if (TheFitResult.IsEmpty()) {
    ReturnCode = false;
  }
  
  if (ReturnCode == true) {
  
    m_IsFitUpToDate = true;
    
    // Copy the key data
    if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
      m_GaussianMean = TheFitResult.Parameter(GetBackgroundFitParameters()+2);
      m_GaussianSigma = TheFitResult.Parameter(GetBackgroundFitParameters()+3);
      m_GaussianHeight = TheFitResult.Parameter(GetBackgroundFitParameters()+4);
    } else {
      m_GaussianMean = TheFitResult.Parameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+0);
      m_GaussianSigma = TheFitResult.Parameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+1);
      m_GaussianHeight = TheFitResult.Parameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+2);
    }
    
    // Create a TF1 object for drawing
    TThread::Lock();
    m_Fit = new TF1("", this, Min, Max, Parameters);
    TThread::UnLock();
    
    m_Fit->SetChisquare(TheFitResult.Chi2());
    m_Fit->SetNDF(TheFitResult.Ndf());
    m_Fit->SetNumberFitPoints(TheData.Size());

    m_Fit->SetParameters( &(TheFitResult.Parameters().front()) ); 
    if (int(TheFitResult.Errors().size()) >= m_Fit->GetNpar()) { 
      m_Fit->SetParErrors( &(TheFitResult.Errors().front()) );
    }

    // Calculate the average deviation for evaluation
    for (int b = 1; b <= Histogram.GetNbinsX(); ++b) {
      if (Histogram.GetBinCenter(b) < Min || Histogram.GetBinCenter(b) > Max) continue;
      double FitValue = m_Fit->Eval(Histogram.GetBinCenter(b));
      if (FitValue != 0) {
        m_AverageDeviation += (Histogram.GetBinContent(b) - FitValue)/FitValue;
      }
    }
    m_AverageDeviation = fabs(m_AverageDeviation/Histogram.GetNbinsX());
  }
    
  return ReturnCode;
}


////////////////////////////////////////////////////////////////////////////////


//! Set all fit parameters
void MCalibrationFitGaussian::SetFitParameters(ROOT::Fit::Fitter& Fitter, TH1D& Hist, double Min, double Max)
{
  MCalibrationFit::SetFitParameters(Fitter, Hist, Min, Max);
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int BPM = GetBackgroundFitParameters();
    Fitter.Config().ParSettings(2+BPM).SetName("Mean (energy loss and Gaussian)");
    if (m_GaussianMean != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(2+BPM).SetValue(m_GaussianMean);
    } else {
      Fitter.Config().ParSettings(2+BPM).SetValue(0.5*(Min+Max));
    }
    Fitter.Config().ParSettings(2+BPM).SetLimits(Min, Max);

    Fitter.Config().ParSettings(3+BPM).SetName("Sigma (energy loss and Gaussian)");
    if (m_GaussianSigma != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(3+BPM).SetValue(m_GaussianSigma);
      Fitter.Config().ParSettings(3+BPM).SetLimits(0.3*m_GaussianSigma, 2*m_GaussianSigma);
    } else {
      Fitter.Config().ParSettings(3+BPM).SetValue(5*Hist.GetBinWidth(1));
      Fitter.Config().ParSettings(3+BPM).SetLimits(Hist.GetBinWidth(1), 0.5*(Max-Min));
    }

    Fitter.Config().ParSettings(4+BPM).SetName("Amplitude (Gaussian)");
    Fitter.Config().ParSettings(4+BPM).SetValue(Hist.GetMaximum());
    Fitter.Config().ParSettings(4+BPM).SetLimits(0, 3*Hist.GetMaximum());
  } else {
    int BPM = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    
    Fitter.Config().ParSettings(0+BPM).SetName("Mean (Gaussian)");
    if (m_GaussianMean != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(0+BPM).SetValue(m_GaussianMean);
    } else {
      Fitter.Config().ParSettings(0+BPM).SetValue(0.5*(Min+Max));
    }
    Fitter.Config().ParSettings(0+BPM).SetLimits(Min, Max);
    
    Fitter.Config().ParSettings(1+BPM).SetName("Sigma (Gaussian)");
    if (m_GaussianSigma != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(1+BPM).SetValue(m_GaussianSigma);
      Fitter.Config().ParSettings(1+BPM).SetLimits(0.3*m_GaussianSigma, 2*m_GaussianSigma);
    } else {
      Fitter.Config().ParSettings(1+BPM).SetValue(5*Hist.GetBinWidth(1));
      Fitter.Config().ParSettings(1+BPM).SetLimits(Hist.GetBinWidth(1), 0.5*(Max-Min));
    }
    
    Fitter.Config().ParSettings(2+BPM).SetName("Amplitude (Gaussian)");
    if (m_GaussianHeight != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(2+BPM).SetValue(m_GaussianHeight);
    } else {
      Fitter.Config().ParSettings(2+BPM).SetValue(Hist.GetMaximum());
    }
    Fitter.Config().ParSettings(2+BPM).SetLimits(0, 3*Hist.GetMaximum());
  }
}


// MCalibrationFitGaussian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
