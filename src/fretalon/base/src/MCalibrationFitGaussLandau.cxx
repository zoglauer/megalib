/*
 * MCalibrationFitGaussLandau.cxx
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
#include "MCalibrationFitGaussLandau.h"

// Standard libs:
#include <cmath>
using namespace std;

// ROOT libs:
#include "TMath.h"
#include "TFitResult.h"
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
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrationFitGaussLandau)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationFitGaussLandau::MCalibrationFitGaussLandau() : MCalibrationFitGaussian()
{
  m_LandauSigma = g_DoubleNotDefined;
  m_LandauHeight = g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Copy constructor
MCalibrationFitGaussLandau::MCalibrationFitGaussLandau(const MCalibrationFitGaussLandau& Fit)
{
  // We don't care about double initilization, so:
  m_Fit = 0;
  *this = Fit;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationFitGaussLandau::~MCalibrationFitGaussLandau()
{
}


////////////////////////////////////////////////////////////////////////////////


//! The assignment operator
MCalibrationFitGaussLandau& MCalibrationFitGaussLandau::operator= (const MCalibrationFitGaussLandau& Fit)
{
  MCalibrationFitGaussian::operator=(Fit);

  m_LandauSigma = Fit.m_LandauSigma;
  m_LandauHeight = Fit.m_LandauHeight;
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this fit - the returned element must be deleted!
MCalibrationFitGaussLandau* MCalibrationFitGaussLandau::Clone() const
{
  return new MCalibrationFitGaussLandau(*this);
}


////////////////////////////////////////////////////////////////////////////////


//! The function for ROOT fitting
double MCalibrationFitGaussLandau::DoEvalPar(double X, const double* P) const
{ 
  // We do have (not case c_EnergyLossModelGaussianConvolvedDeltaFunction):
  // 0 .. Ps-1: Background + Energy loss
  // Ps+1: Mean landau & gauss
  // Ps+1: sigma gauss
  // Ps+2: amplitude gauss
  // Ps+3: sigma landau
  // Ps+4: amplitude landau
  
  double Return = MCalibrationFitGaussian::DoEvalPar(X, P);

  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int Ps = GetBackgroundFitParameters();
    //if (4*P[Ps+5] <  P[Ps+3] || P[Ps+5] >  4*P[Ps+3]) return 0.001;
    //Return += P[Ps+6]*TMath::Landau(-X, -(P[Ps+2] - 0.22278298 * P[Ps+5]), P[Ps+5]);
    Return += P[Ps+6]*TMath::Landau(-X, -(P[Ps+2] - 0.22278298 * P[Ps+3]), P[Ps+3], true);
  } else {
    int Ps = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    //if (4*P[Ps+1] <  P[Ps+3] || P[Ps+1] >  4*P[Ps+3]) return 0.001;
    //Return += P[Ps+4]*TMath::Landau(-X, -(P[Ps] - 0.22278298*P[Ps+3]), P[Ps+3]);
    Return += P[Ps+4]*TMath::Landau(-X, -(P[Ps] - 0.22278298*P[Ps+1]), P[Ps+1], true);
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Number of parameters of the fit
unsigned int MCalibrationFitGaussLandau::NPar() const
{
  unsigned int Parameters = GetBackgroundFitParameters();
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    Parameters += 7;
  } else {
    Parameters += GetEnergyLossFitParameters() + 5;
  }
  return Parameters;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the given histogram in the given range
bool MCalibrationFitGaussLandau::Fit(TH1D& Histogram, double Min, double Max)
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
  
  TheFitter.SetFunction(*this);
  
  SetFitParameters(TheFitter, Histogram, Min, Max);

  /*  
  bool ReturnCode = TheFitter.Fit(TheData);
  if (ReturnCode == true) {
    if (TheFitter.CalculateHessErrors() == false) {
      if (TheFitter.CalculateMinosErrors() == false) {
        cout<<"Unable to calculate either Minos or Hess error!"<<endl;
        ReturnCode = false;
      }
    }
  }
  
  const ROOT::Fit::FitResult& TheFitResult = TheFitter.Result(); 
  if (TheFitResult.IsEmpty()) ReturnCode = false;
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
    } else {
      m_GaussianMean = TheFitResult.Parameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+0);
      m_GaussianSigma = TheFitResult.Parameter(GetBackgroundFitParameters()+3);
    }
    
    if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
      m_GaussianMean = TheFitResult.Parameter(GetBackgroundFitParameters()+2);
      m_GaussianSigma = TheFitResult.Parameter(GetBackgroundFitParameters()+3);
      m_GaussianHeight = TheFitResult.Parameter(GetBackgroundFitParameters()+4);
      m_LandauSigma = m_GaussianSigma;
      m_LandauHeight = TheFitResult.Parameter(GetBackgroundFitParameters()+6);
    } else {
      m_GaussianMean = TheFitResult.Parameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+0);
      m_GaussianSigma = TheFitResult.Parameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+1);
      m_GaussianHeight = TheFitResult.Parameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+2);
      m_LandauSigma = m_GaussianSigma;
      m_LandauHeight = TheFitResult.Parameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+4);
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
    m_AverageDeviation = 0;
    for (int b = 1; b <= Histogram.GetNbinsX(); ++b) {
      if (Histogram.GetBinCenter(b) < Min || Histogram.GetBinCenter(b) > Max) continue;
      double FitValue = m_Fit->Eval(Histogram.GetBinCenter(b));
      if (FitValue != 0) {
        //cout<<Histogram.GetBinContent(b)<<":"<<FitValue<<endl;
        m_AverageDeviation += (Histogram.GetBinContent(b) - FitValue)/FitValue;
      }
    }
    m_AverageDeviation = fabs(m_AverageDeviation/Histogram.GetNbinsX());
  }
  
  return ReturnCode;
}


////////////////////////////////////////////////////////////////////////////////


//! Set all fit parameters
void MCalibrationFitGaussLandau::SetFitParameters(ROOT::Fit::Fitter& Fitter, TH1D& Hist, double Min, double Max)
{
  MCalibrationFitGaussian::SetFitParameters(Fitter, Hist, Min, Max);
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int BPM = GetBackgroundFitParameters();
    Fitter.Config().ParSettings(2+BPM).SetName("Mean (energy loss, Gaussian & Landau)");
    Fitter.Config().ParSettings(5+BPM).SetName("Sigma (Landau)");
    if (m_LandauSigma != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(5+BPM).SetValue(m_LandauSigma);
    } else {
      Fitter.Config().ParSettings(5+BPM).SetValue(5*Hist.GetBinWidth(1));
    }
    Fitter.Config().ParSettings(5+BPM).SetLimits(Hist.GetBinWidth(1), 0.5*(Max-Min));

    Fitter.Config().ParSettings(6+BPM).SetName("Amplitude (Landau)");
    if (m_LandauHeight != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(6+BPM).SetValue(m_LandauHeight);
    } else {
      Fitter.Config().ParSettings(6+BPM).SetValue(0.25*Hist.GetMaximum());
    }
    Fitter.Config().ParSettings(6+BPM).SetLimits(0, 5*Hist.GetMaximum());

    
  } else {
    int BPM = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    
    Fitter.Config().ParSettings(0+BPM).SetName("Mean (Gaussian + Landau)");
    
    Fitter.Config().ParSettings(3+BPM).SetName("Sigma (Landau)");
    if (m_LandauSigma != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(3+BPM).SetValue(m_LandauSigma);
    } else {
      Fitter.Config().ParSettings(3+BPM).SetValue(5*Hist.GetBinWidth(1));
    }
    Fitter.Config().ParSettings(3+BPM).SetLimits(Hist.GetBinWidth(1), 0.5*(Max-Min));
    
    Fitter.Config().ParSettings(4+BPM).SetName("Amplitude (Landau)");
    if (m_LandauHeight != g_DoubleNotDefined) {
      Fitter.Config().ParSettings(4+BPM).SetValue(m_LandauHeight);
    } else {
      Fitter.Config().ParSettings(4+BPM).SetValue(0.25*Hist.GetMaximum());
    }
    Fitter.Config().ParSettings(4+BPM).SetLimits(0, 3*Hist.GetMaximum());
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get the FWHM
double MCalibrationFitGaussLandau::GetFWHM() const
{ 
  if (m_IsFitUpToDate == false) {
    if (g_Verbosity >= c_Error) cout<<"Error: The FWHM can only be determined if we have an up-to-date fit!"<<endl;
    return 1000000;
  }
  
  // A Gauss-Landau lambda without background and energy loss 
  auto GL = [&](double x) {
    double Return = 0.0;
    if (m_GaussianSigma != 0) {
      double Arg = ((x - m_GaussianMean)/m_GaussianSigma);
      //Return += m_GaussianHeight/sqrt(2*TMath::Pi())/m_GaussianSigma * TMath::Exp(-0.5*Arg*Arg);
      Return += m_GaussianHeight * TMath::Exp(-0.5*Arg*Arg);
    }
    Return += m_LandauHeight*TMath::Landau(-x, -(m_GaussianMean - 0.22278298*m_GaussianSigma), m_LandauSigma, true);
    return Return;
  };
  
  double HalfPeak = 0.5*GL(m_GaussianMean);
  //cout<<"Peak: "<<GL(m_GaussianMean)<<" at "<<m_GaussianMean<<" adu"<<endl;
  
  // Find the low half value:
  
  // (a) Just in case the first guess for half is not enough go even farther left
  double Min = -1.0; // We have energies... I hope
  while (GL(Min) > HalfPeak) {
    Min *= 2;
    if (std::isinf(Min) == true) {
      merr<<"Cannot find a low value below 50% of peak"<<error;
      return -1;
    }
  }
  double Max = m_GaussianMean;
  
  // (b) Test half point and choose a new max or min, depending where the half height is 
  double Epsilon = GL(Max) - GL(Min);
  while (Epsilon > 0.001) {
    if (GL(0.5*(Min+Max)) > HalfPeak) {
      Max = 0.5*(Min+Max);
    } else {
      Min = 0.5*(Min+Max);
    }
    Epsilon = GL(Max) - GL(Min);
  }
  double Left = 0.5*(Min+Max);
  //cout<<"left: "<<GL(Left)<<" at "<<Left<<" adu"<<endl;

  // Find the high half value:
    
    
  // (a) Just in case the first guess for half is not enough go even farther right
  Min = m_GaussianMean;
  Max = 10*m_GaussianMean; // We have energies... I hope
  while (GL(Max) > HalfPeak) {
    Max *= 2;
    if (std::isinf(Max) == true) {
      merr<<"Cannot find a high value below 50% of peak"<<error;
      return -1;
    }
  }
  
  // (b) Test half point and choose a new max or min, depending where the half height is 
  Epsilon = GL(Min) - GL(Max);
  while (Epsilon > 0.001) {
    if (GL(0.5*(Min+Max)) > HalfPeak) {
      Min = 0.5*(Min+Max);
    } else {
      Max = 0.5*(Min+Max);
    }
    Epsilon = GL(Min) - GL(Max);
  }
  double Right = 0.5*(Min+Max);
  //cout<<"Right: "<<GL(Right)<<" at "<<Right<<" adu"<<endl;
  
  //cout<<"Gaus-Landau FWHM: "<<Right-Left<<endl;
  
  return Right-Left;
}


// MCalibrationFitGaussLandau.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
