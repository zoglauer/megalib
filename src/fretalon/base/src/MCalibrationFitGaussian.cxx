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
#include "TH1.h"
#include "TF1.h"
#include "Math/MinimizerOptions.h"

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
double MCalibrationFitGaussian::operator() (double* X, double* P)
{
  double Return = 0.0;
  Return += BackgroundFit(X, P);
  Return += EnergyLossFit(X, P);
  

  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int Ps = GetBackgroundFitParameters();
    if (P[Ps+3] != 0) {
      double Arg = ((X[0] - P[Ps+2])/P[Ps+3]);
      //Return += P[Ps+4]/sqrt(2*TMath::Pi())/P[Ps+3] * TMath::Exp(-0.5*Arg*Arg); // Integral is 1
      Return += P[Ps+4] * TMath::Exp(-0.5*Arg*Arg); // Height is 1
    }
  } else {
    int Ps = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    if (P[Ps+1] != 0) {
      double Arg = ((X[0] - P[Ps])/P[Ps+1]);
      //Return += P[Ps+2]/sqrt(2*TMath::Pi())/P[Ps+1] * TMath::Exp(-0.5*Arg*Arg); // Integral is 1
      Return += P[Ps+2] * TMath::Exp(-0.5*Arg*Arg); // Height is 1
    }
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the given histogram in the given range
bool MCalibrationFitGaussian::Fit(TH1D& Histogram, double Min, double Max)
{
  int Parameters = GetBackgroundFitParameters();
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    Parameters += 5;
  } else {
    Parameters += GetEnergyLossFitParameters() + 3;
  }
  
  delete m_Fit;
  ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(20000);
  m_Fit = new TF1("", this, Min, Max, Parameters);
  
  SetFitParameters(Histogram, Min, Max);
  
  MString Options = "RNI S"; 
  if (g_Verbosity < c_Chatty) Options += " Q";
  TFitResultPtr FitResult = Histogram.Fit(m_Fit, Options);
  
  //cout<<"Fit quality: "<<m_Fit->GetChisquare()<<" - "<<m_Fit->GetNDF()<<endl;
  
  m_IsFitUpToDate = true;
  
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters()+2);
    m_GaussianSigma = m_Fit->GetParameter(GetBackgroundFitParameters()+3);
    m_GaussianHeight = m_Fit->GetParameter(GetBackgroundFitParameters()+4);
  } else {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+0);
    m_GaussianSigma = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+1);
    m_GaussianHeight = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+2);
  }
    
  m_AverageDeviation = 0;
  for (int b = 1; b <= Histogram.GetNbinsX(); ++b) {
    if (Histogram.GetBinCenter(b) < Min || Histogram.GetBinCenter(b) > Max) continue;
    double FitValue = m_Fit->Eval(Histogram.GetBinCenter(b));
    if (FitValue != 0) {
      m_AverageDeviation += (Histogram.GetBinContent(b) - FitValue)/FitValue;
    }
  }
  m_AverageDeviation = fabs(m_AverageDeviation/Histogram.GetNbinsX());
    
    
  return FitResult->IsValid();
}


////////////////////////////////////////////////////////////////////////////////


//! Set all fit parameters
void MCalibrationFitGaussian::SetFitParameters(TH1D& Hist, double Min, double Max)
{
  if (m_Fit == 0) return;
  
  MCalibrationFit::SetFitParameters(Hist, Min, Max);
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int BPM = GetBackgroundFitParameters();
    m_Fit->SetParName(2+BPM, "Mean (energy loss and Gaussian)");
    if (m_GaussianMean != g_DoubleNotDefined) {
      m_Fit->SetParameter(2+BPM, m_GaussianMean);
    } else {
      m_Fit->SetParameter(2+BPM, 0.5*(Min+Max));
    }
    m_Fit->SetParLimits(2+BPM, Min, Max);

    m_Fit->SetParName(3+BPM, "Sigma (energy loss and Gaussian)");
    if (m_GaussianSigma != g_DoubleNotDefined) {
      m_Fit->SetParameter(3+BPM, m_GaussianSigma);
      m_Fit->SetParLimits(3+BPM, 0.3*m_GaussianSigma, 2*m_GaussianSigma);
    } else {
      m_Fit->SetParameter(3+BPM, 5*Hist.GetBinWidth(1));
      m_Fit->SetParLimits(3+BPM, Hist.GetBinWidth(1), 0.5*(Max-Min));
    }

    m_Fit->SetParName(4+BPM, "Amplitude (Gaussian)");
    m_Fit->SetParameter(4+BPM, Hist.GetMaximum());
    m_Fit->SetParLimits(4+BPM, 0, 3*Hist.GetMaximum());
  } else {
    int BPM = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    
    m_Fit->SetParName(0+BPM, "Mean (Gaussian)");
    if (m_GaussianMean != g_DoubleNotDefined) {
      m_Fit->SetParameter(0+BPM, m_GaussianMean);
    } else {
      m_Fit->SetParameter(0+BPM, 0.5*(Min+Max));
    }
    m_Fit->SetParLimits(0+BPM, Min, Max);
    
    m_Fit->SetParName(1+BPM, "Sigma (Gaussian)");
    if (m_GaussianSigma != g_DoubleNotDefined) {
      m_Fit->SetParameter(1+BPM, m_GaussianSigma);
      m_Fit->SetParLimits(1+BPM, 0.3*m_GaussianSigma, 2*m_GaussianSigma);
    } else {
      m_Fit->SetParameter(1+BPM, 5*Hist.GetBinWidth(1));
      m_Fit->SetParLimits(1+BPM, Hist.GetBinWidth(1), 0.5*(Max-Min));
    }
    
    m_Fit->SetParName(2+BPM, "Amplitude (Gaussian)");
    if (m_GaussianHeight != g_DoubleNotDefined) {
      m_Fit->SetParameter(2+BPM, m_GaussianHeight);
    } else {
      m_Fit->SetParameter(2+BPM, Hist.GetMaximum());
    }
    m_Fit->SetParLimits(2+BPM, 0, 3*Hist.GetMaximum());
  }
}


// MCalibrationFitGaussian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
