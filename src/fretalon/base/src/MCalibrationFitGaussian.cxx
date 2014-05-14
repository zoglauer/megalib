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
    if (P[Ps+2] != 0) {
      double Arg = ((X[0] - P[Ps+2])/P[Ps+3]);
      Return += P[Ps+4]/sqrt(2*TMath::Pi())/P[Ps+3] * TMath::Exp(-0.5*Arg*Arg);
    }
  } else {
    int Ps = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    if (P[Ps+1] != 0) {
      double Arg = ((X[0] - P[Ps])/P[Ps+1]);
      Return += P[Ps+2]/sqrt(2*TMath::Pi())/P[Ps+1] * TMath::Exp(-0.5*Arg*Arg);
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
  m_Fit = new TF1("", this, Min, Max, Parameters);
  
  SetFitParameters(Histogram, Min, Max);
  
  TFitResultPtr FitResult = Histogram.Fit(m_Fit, "RN S");
  
  m_IsFitUpToDate = true;
  
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters()+2);
  } else {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+0);
  }
    
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
    } else {
      m_Fit->SetParameter(3+BPM, 5*Hist.GetBinWidth(1));
    }
    m_Fit->SetParLimits(3+BPM, Hist.GetBinWidth(1), 0.5*(Max-Min));

    m_Fit->SetParName(4+BPM, "Amplitude (Gaussian)");
    m_Fit->SetParameter(4+BPM, Hist.GetMaximum());
    m_Fit->SetParLimits(4+BPM, 0, 10*Hist.GetMaximum());
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
    } else {
      m_Fit->SetParameter(1+BPM, 5*Hist.GetBinWidth(1));
    }
    m_Fit->SetParLimits(1+BPM, Hist.GetBinWidth(1), 0.5*(Max-Min));
    
    m_Fit->SetParName(2+BPM, "Amplitude (Gaussian)");
    if (m_GaussianHeight != g_DoubleNotDefined) {
      m_Fit->SetParameter(2+BPM, m_GaussianHeight);
    } else {
      m_Fit->SetParameter(2+BPM, Hist.GetMaximum());
    }
    m_Fit->SetParLimits(2+BPM, 0, 10*Hist.GetMaximum());
  }
}


// MCalibrationFitGaussian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
