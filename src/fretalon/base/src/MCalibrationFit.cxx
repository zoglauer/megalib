/*
 * MCalibrationFit.cxx
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
#include "MCalibrationFit.h"

// Standard libs:

// ROOT libs:
#include "TMath.h"
#include "TH1.h"
#include "TF1.h"

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MCalibrationFit)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationFit::MCalibrationFit() : m_Fit(0), m_IsFitUpToDate(false), m_AverageDeviation(1), m_BackgroundModel(0), m_EnergyLossModel(0), m_PeakShapeModel(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Copy constructor
MCalibrationFit::MCalibrationFit(const MCalibrationFit& Fit)
{
  // We don't care about double initilization, so:
  m_Fit = 0;
  *this = Fit;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationFit::~MCalibrationFit()
{
  delete m_Fit;
}


////////////////////////////////////////////////////////////////////////////////


//! The assignment operator
MCalibrationFit& MCalibrationFit::operator=(const MCalibrationFit& CalibrationFit)
{
  delete m_Fit;
  if (CalibrationFit.m_Fit != 0) {
    m_Fit = new TF1(*CalibrationFit.m_Fit);
    for (int i = 0; i < CalibrationFit.m_Fit->GetNpar(); ++i) {
      m_Fit->SetParameter(i, CalibrationFit.m_Fit->GetParameter(i));
    }
  } else {
    m_Fit = 0;
  }
    
  m_IsFitUpToDate = CalibrationFit.m_IsFitUpToDate;
  m_AverageDeviation = CalibrationFit.m_AverageDeviation;
  m_BackgroundModel = CalibrationFit.m_BackgroundModel;
  m_EnergyLossModel = CalibrationFit.m_EnergyLossModel;
  m_PeakShapeModel = CalibrationFit.m_PeakShapeModel;
  
  m_ROOTParameters = CalibrationFit.m_ROOTParameters;
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this fit - the returned element must be deleted!
MCalibrationFit* MCalibrationFit::Clone() const
{
  return new MCalibrationFit(*this);
}


////////////////////////////////////////////////////////////////////////////////


//! Set the background model
//! If it doesn't exist don't use any
void MCalibrationFit::SetBackgroundModel(unsigned int BackgroundModel)
{
  if (BackgroundModel == c_BackgroundModelNone ||
      BackgroundModel == c_BackgroundModelFlat ||
      BackgroundModel == c_BackgroundModelLinear) {
    m_BackgroundModel = BackgroundModel;  
  } else {
    merr<<"Unknown background model ID: "<<BackgroundModel<<". Using no background model"<<endl; 
    m_BackgroundModel = c_BackgroundModelNone;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set the energy loss model
//! If it doesn't exist don't use any
void MCalibrationFit::SetEnergyLossModel(unsigned int EnergyLossModel)
{
  if (EnergyLossModel == c_EnergyLossModelNone ||
      EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction ||
      EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunctionWithExponentialDecay) {
    m_EnergyLossModel = EnergyLossModel;  
  } else {
    merr<<"Unknown energy loss model ID: "<<EnergyLossModel<<". Using no energy loss model"<<endl; 
    m_EnergyLossModel = c_EnergyLossModelNone;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of fit parameters in the background fit
int MCalibrationFit::GetBackgroundFitParameters() const
{
  if (m_BackgroundModel == c_BackgroundModelNone) {
    return 0;
  } else if (m_BackgroundModel == c_BackgroundModelFlat) {
    return 1;
  } else if (m_BackgroundModel == c_BackgroundModelLinear) {
    return 2;
  }
  
  merr<<"Unknown background model ID: "<<m_BackgroundModel<<". Using no background model"<<endl; 
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of fit parameters in the energy loss fit
int MCalibrationFit::GetEnergyLossFitParameters() const
{
  if (m_EnergyLossModel == c_EnergyLossModelNone) {
    return 0;
  } else if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    return 1;
  } else if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunctionWithExponentialDecay) {
    return 2;
  }
  
  merr<<"Unknown energy loss model ID: "<<m_EnergyLossModel<<". Using no energy loss model"<<endl; 
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! The function for fitting the background
double MCalibrationFit::BackgroundFit(double X, const double* P) const
{
  if (m_BackgroundModel == c_BackgroundModelNone) {
    return 0;
  } else if (m_BackgroundModel == c_BackgroundModelFlat) {
    return P[0];
  } else if (m_BackgroundModel == c_BackgroundModelLinear) {
    return P[0] + P[1]*X;
  }
  
  merr<<"Unknown background model ID: "<<m_BackgroundModel<<". Using no background model"<<endl; 
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! The function for fitting the energy loss
double MCalibrationFit::EnergyLossFit(double X, const double* P) const
{
  int BPM = GetBackgroundFitParameters();
  
  if (m_EnergyLossModel == c_EnergyLossModelNone) {
    return 0;
  } else if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    // From Mark's thesis, page 80
    // P[0]: Lower level
    // P[1]: Upper level
    // P[2]: Mean of Gaussian
    // P[3]: Sigma of Gaussian
    double Return = 0.5*(P[0+BPM] + P[1+BPM]);
    if (P[3+BPM] != 0) {
      Return += 0.5*(P[1+BPM] - P[0+BPM])*TMath::Erf((X-P[2+BPM])/sqrt(2.0)/P[3+BPM]);
    }
    return Return;
  } else if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunctionWithExponentialDecay) {
    // From Mark's thesis, page 104
    // P[0]: Scaler
    // P[1]: Decay constant
    // P[2]: Mean of Gaussian
    // P[3]: Sigma of Gaussian
    //double Return = 0.5*(P[0+BPM] + P[1+BPM]);
    //if (P[3+BPM] != 0) {
    //  Return += 0.5*(P[1+BPM] - P[0+BPM])*TMath::Erf((X-P[2+BPM])/sqrt(2.0)/P[3+BPM]);
    //}
    double Return = 0;
    if (P[1+BPM] != 0 && P[3+BPM] != 0) {
     
      // Scaler
      Return += P[0+BPM];
      // Exponential
      // The exponential can get infinite
      // But large numbers are anyway a horrble bad fit
      // Therefore is the exp would be larger 1E20 we replace it by a function which goes slower to infinite...
      double Arg = (X-P[2+BPM])/P[1+BPM] + P[3+BPM]*P[3+BPM]/2/P[1+BPM]/P[1+BPM];
      if (Arg > 46.05170185988) { // = ln(1E20)
        Return *= 1E20 * Arg / 46.05170185988; 
      } else {
        Return *= exp(Arg);
      }
      // The Gauss-deconvolved delta function part
      Return *= (1 - TMath::Erf((X-P[2+BPM]) / sqrt(2.0) / P[3+BPM] + P[3+BPM] / sqrt(2) / P[1+BPM])); 
    }
    
    if (!isfinite(Return)) {
      Return = 1E20; // Just something too large 
    }
    
    return Return;
  }
  
  merr<<"Unknown energy loss model ID: "<<m_EnergyLossModel<<". Using no energy loss model"<<endl; 
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Set all fit parameters
void MCalibrationFit::SetFitParameters(ROOT::Fit::Fitter& Fitter, TH1D& Hist, double Min, double Max)
{
  // Determine the maximum in the range!
  double MaxValue = 0.0;
  int bmin = Hist.GetXaxis()->FindBin(Min);
  int bmax = Hist.GetXaxis()->FindBin(Max);
  for (int b = bmin; b <= bmax; ++b) {
    if (Hist.GetBinContent(b) > MaxValue) MaxValue = Hist.GetBinContent(b); 
  }

  
  if (m_BackgroundModel == c_BackgroundModelNone) {
  } else if (m_BackgroundModel == c_BackgroundModelFlat) {
    Fitter.Config().ParSettings(0).SetName("Offset flat background");
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(0).SetLimits(0, 2*MaxValue);
  } else if (m_BackgroundModel == c_BackgroundModelLinear) {
    Fitter.Config().ParSettings(0).SetName("Offset linear background");
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetName("Gradient linear background");
    Fitter.Config().ParSettings(1).SetValue(0);
  }
  
  int BPM = GetBackgroundFitParameters();
  
  if (m_EnergyLossModel == c_EnergyLossModelNone) {
  } else if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunction) {
    Fitter.Config().ParSettings(0+BPM).SetName("Left offset energy loss");
    Fitter.Config().ParSettings(0+BPM).SetValue(0.1*MaxValue);
    Fitter.Config().ParSettings(0+BPM).SetLimits(0, 2*MaxValue);
    Fitter.Config().ParSettings(1+BPM).SetName("Right offset energy loss");
    Fitter.Config().ParSettings(1+BPM).SetValue(0.1*MaxValue);
    Fitter.Config().ParSettings(1+BPM).SetLimits(0, 2*MaxValue);
    Fitter.Config().ParSettings(2+BPM).SetName("Mean energy loss");
    Fitter.Config().ParSettings(2+BPM).SetValue(0.5*(Min+Max));
    Fitter.Config().ParSettings(2+BPM).SetLimits(Min, Max);
    Fitter.Config().ParSettings(3+BPM).SetName("Sigma energy loss");
    Fitter.Config().ParSettings(3+BPM).SetValue(5*Hist.GetBinWidth(1));
    Fitter.Config().ParSettings(3+BPM).SetLimits(Hist.GetBinWidth(1), Max-Min);
  } else if (m_EnergyLossModel == c_EnergyLossModelGaussianConvolvedDeltaFunctionWithExponentialDecay) {
    Fitter.Config().ParSettings(0+BPM).SetName("Scaler");
    Fitter.Config().ParSettings(0+BPM).SetValue(0*MaxValue);
    Fitter.Config().ParSettings(0+BPM).SetLimits(0, 0.33*MaxValue);
    Fitter.Config().ParSettings(1+BPM).SetName("Decay");
    Fitter.Config().ParSettings(1+BPM).SetValue(30);
    Fitter.Config().ParSettings(1+BPM).SetLimits(1, 5*(Min+Max)); // Do not go too small
    Fitter.Config().ParSettings(2+BPM).SetName("Mean energy loss");
    Fitter.Config().ParSettings(2+BPM).SetValue(0.5*(Min+Max));
    Fitter.Config().ParSettings(2+BPM).SetLimits(Min, Max);
    Fitter.Config().ParSettings(3+BPM).SetName("Sigma energy loss");
    Fitter.Config().ParSettings(3+BPM).SetValue(5*Hist.GetBinWidth(1));
    Fitter.Config().ParSettings(3+BPM).SetLimits(Hist.GetBinWidth(1), 0.3*(Max-Min));
  }
  
  for (int b = 1; b <= Hist.GetNbinsX(); ++b) {
    double Error = sqrt(Hist.GetBinContent(b));
    if (Error == 0) Error = 1;
    Hist.SetBinError(b, Error);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get a value at the given x
double MCalibrationFit::Evaluate(double X) const
{
  if (m_Fit != 0 && m_IsFitUpToDate == true) {
    return m_Fit->Eval(X);
  }
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Mimic ROOT Draw functionality
void MCalibrationFit::Draw(MString Options)
{
  if (m_Fit != 0 && m_IsFitUpToDate == true) {
    m_Fit->DrawCopy(Options);
  } else {
    merr<<"Fit cannot be drawn since it is not up to date"<<endl; 
  }
}


// MCalibrationFit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
