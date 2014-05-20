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
double MCalibrationFitGaussLandau::operator() (double* X, double* P)
{ 
  double Return = MCalibrationFitGaussian::operator()(X, P);

  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int Ps = GetBackgroundFitParameters();
    if (4*P[Ps+5] <  P[Ps+3] || P[Ps+5] >  4*P[Ps+3]) return 0.001;
    Return += P[Ps+6]*TMath::Landau(-X[0] + P[Ps+2], 0, P[Ps+5]);
  } else {
    int Ps = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    if (4*P[Ps+1] <  P[Ps+3] || P[Ps+1] >  4*P[Ps+3]) return 0.001;
    Return += P[Ps+4]*TMath::Landau(-X[0] + P[Ps], 0, P[Ps+3]);
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the given histogram in the given range
bool MCalibrationFitGaussLandau::Fit(TH1D& Histogram, double Min, double Max)
{
  int Parameters = GetBackgroundFitParameters();
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    Parameters += 7;
  } else {
    Parameters += GetEnergyLossFitParameters() + 5;
  }
  
  delete m_Fit;
  m_Fit = new TF1("", this, Min, Max, Parameters);
  
  SetFitParameters(Histogram, Min, Max);
  
  TFitResultPtr FitResult = Histogram.Fit(m_Fit, "RN S");
  
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters()+2);
  } else {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+0);
  }
  
  m_IsFitUpToDate = true;
  
  m_Fit->Print();
  
  return FitResult->IsValid();
}


////////////////////////////////////////////////////////////////////////////////


//! Set all fit parameters
void MCalibrationFitGaussLandau::SetFitParameters(TH1D& Hist, double Min, double Max)
{
  if (m_Fit == 0) return;
  
  MCalibrationFitGaussian::SetFitParameters(Hist, Min, Max);
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int BPM = GetBackgroundFitParameters();
    m_Fit->SetParName(2+BPM, "Mean (energy loss, Gaussian & Landau)");
    m_Fit->SetParName(5+BPM, "Sigma (Landau)");
    if (m_LandauSigma != g_DoubleNotDefined) {
      m_Fit->SetParameter(5+BPM, m_LandauSigma);
    } else {
      m_Fit->SetParameter(5+BPM, 5*Hist.GetBinWidth(1));
    }
    m_Fit->SetParLimits(5+BPM, Hist.GetBinWidth(1), 0.5*(Max-Min));

    m_Fit->SetParName(6+BPM, "Amplitude (Landau)");
    if (m_LandauHeight != g_DoubleNotDefined) {
      m_Fit->SetParameter(6+BPM, m_LandauHeight);
    } else {
      m_Fit->SetParameter(6+BPM, Hist.GetMaximum());
    }
    m_Fit->SetParLimits(6+BPM, 0, 10*Hist.GetMaximum());

    
  } else {
    int BPM = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    
    m_Fit->SetParName(0+BPM, "Mean (Gaussian + Landau)");
    
    m_Fit->SetParName(3+BPM, "Sigma (Landau)");
    if (m_LandauSigma != g_DoubleNotDefined) {
      m_Fit->SetParameter(3+BPM, m_LandauSigma);
    } else {
      m_Fit->SetParameter(3+BPM, 5*Hist.GetBinWidth(1));
    }
    m_Fit->SetParLimits(3+BPM, Hist.GetBinWidth(1), 0.5*(Max-Min));
    
    m_Fit->SetParName(4+BPM, "Amplitude (Landau)");
    if (m_LandauHeight != g_DoubleNotDefined) {
      m_Fit->SetParameter(4+BPM, m_LandauHeight);
    } else {
      m_Fit->SetParameter(4+BPM, Hist.GetMaximum());
    }
    m_Fit->SetParLimits(4+BPM, 0, 10*Hist.GetMaximum());
  }
}


////////////////////////////////////////////////////////////////////////////////



//! Get the FWHM
double MCalibrationFitGaussLandau::GetFWHM() const
{
  cout<<"Gauss-Landau FWHM never tested!"<<endl;
  
  // A Gauss-Landau lambda without background and energy loss 
  auto GL = [&](double x) {
    double Return = 0.0;
    if (m_GaussianSigma != 0) {
      double Arg = ((x - m_GaussianMean)/m_GaussianSigma);
      Return += m_GaussianHeight/sqrt(2*TMath::Pi())/m_GaussianSigma * TMath::Exp(-0.5*Arg*Arg);
    }
    Return += m_LandauHeight*TMath::Landau(-x + m_GaussianMean, 0, m_LandauSigma);
    return Return;
  };
  
  double HalfPeak = 2*GL(m_GaussianMean);
  
  // Find the low half value:
  double Min = -1.0; // We have energies... I hope
  while (GL(Min) > HalfPeak) {
    Min *= 2;
    if (std::isinf(Min) == true) {
      merr<<"Cannot find a low value below 50% of peak"<<error;
      return -1;
    }
  }
  double Max = m_GaussianMean;
  
  double Epsilon = Max - Min;
  while (Epsilon > 0.001) {
    if (GL(0.5*(Min+Max)) > HalfPeak) {
      Max = 0.5*(Min+Max);
    } else {
      Min = 0.5*(Min+Max);
    }
    Epsilon = Max - Min;
  }
  double Left = 0.5*(Min+Max);
  
  // Find the high half value:
  Min = m_GaussianMean;
  Max = 10*m_GaussianMean; // We have energies... I hope
  while (GL(Max) > HalfPeak) {
    Max *= 2;
    if (std::isinf(Max) == true) {
      merr<<"Cannot find a high value below 50% of peak"<<error;
      return -1;
    }
  }
  
  Epsilon = Max - Min;
  while (Epsilon > 0.001) {
    if (GL(0.5*(Min+Max)) > HalfPeak) {
      Max = 0.5*(Min+Max);
    } else {
      Min = 0.5*(Min+Max);
    }
    Epsilon = Max - Min;
  }
  double Right = 0.5*(Min+Max);
  
  return Right-Left;
}


// MCalibrationFitGaussLandau.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
