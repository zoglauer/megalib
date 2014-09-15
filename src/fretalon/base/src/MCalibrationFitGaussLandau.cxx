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
  // We do have (not case c_EnergyLossModelGaussianConvolvedDeltaFunction):
  // 0 .. Ps-1: Background + Energy loss
  // Ps+1: Mean landau & gauss
  // Ps+1: sigma gauss
  // Ps+2: amplitude gauss
  // Ps+3: sigma landau
  // Ps+4: amplitude landau
  
  double Return = MCalibrationFitGaussian::operator()(X, P);

  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    int Ps = GetBackgroundFitParameters();
    //if (4*P[Ps+5] <  P[Ps+3] || P[Ps+5] >  4*P[Ps+3]) return 0.001;
    //Return += P[Ps+6]*TMath::Landau(-X[0], -(P[Ps+2] - 0.22278298 * P[Ps+5]), P[Ps+5]);
    Return += P[Ps+6]*TMath::Landau(-X[0], -(P[Ps+2] - 0.22278298 * P[Ps+3]), P[Ps+3], true);
  } else {
    int Ps = GetBackgroundFitParameters() + GetEnergyLossFitParameters();
    //if (4*P[Ps+1] <  P[Ps+3] || P[Ps+1] >  4*P[Ps+3]) return 0.001;
    //Return += P[Ps+4]*TMath::Landau(-X[0], -(P[Ps] - 0.22278298*P[Ps+3]), P[Ps+3]);
    Return += P[Ps+4]*TMath::Landau(-X[0], -(P[Ps] - 0.22278298*P[Ps+1]), P[Ps+1], true);
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
  
  MString Options = "RNIM S"; 
  if (g_Verbosity < c_Chatty) Options += " Q";
  TFitResultPtr FitResult = Histogram.Fit(m_Fit, Options);
  
  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters()+2);
  } else {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+0);
    m_GaussianSigma = m_Fit->GetParameter(GetBackgroundFitParameters()+3);
  }

  if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters()+2);
    m_GaussianSigma = m_Fit->GetParameter(GetBackgroundFitParameters()+3);
    m_GaussianHeight = m_Fit->GetParameter(GetBackgroundFitParameters()+4);
    m_LandauSigma = m_GaussianSigma;
    m_LandauHeight = m_Fit->GetParameter(GetBackgroundFitParameters()+6);
  } else {
    m_GaussianMean = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+0);
    m_GaussianSigma = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+1);
    m_GaussianHeight = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+2);
    m_LandauSigma = m_GaussianSigma;
    m_LandauHeight = m_Fit->GetParameter(GetBackgroundFitParameters() + GetEnergyLossFitParameters()+4);
  }

  m_IsFitUpToDate = true;
    
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
      m_Fit->SetParameter(6+BPM, 0.25*Hist.GetMaximum());
    }
    m_Fit->SetParLimits(6+BPM, 0, 5*Hist.GetMaximum());

    
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
      m_Fit->SetParameter(4+BPM, 0.25*Hist.GetMaximum());
    }
    m_Fit->SetParLimits(4+BPM, 0, 3*Hist.GetMaximum());
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get the FWHM
double MCalibrationFitGaussLandau::GetFWHM() const
{  
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
