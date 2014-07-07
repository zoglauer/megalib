/*
 * MFitFunctions.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MFitFunctions
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFitFunctions.h"

// Standard libs:

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFitFunctions)
#endif


////////////////////////////////////////////////////////////////////////////////


double Lorentz2(double* x, double* par)
{
  double fitval = par[0]/(par[1]*par[1]+x[0]*x[0]);

  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


double Gauss(double* x, double* par)
{
  // 0: Scaler
  // 1: Mean
  // 2: Sigma
  
  double arg = 0;
  if (par[2] != 0) arg = (x[0] - par[1])/par[2];
  
  return par[0]*exp(-0.5*arg*arg);
}


////////////////////////////////////////////////////////////////////////////////


double Gauss2(double *x, double *par)
{
  // par[0] is Offset
  // par[1] is Gaus-height
  // par[2] is Gaus-mean
  // par[3] is Gaus-sigma

  double fitval = par[0];
  double arg = 0;

  if (par[3] != 0) arg = (x[0] - par[2])/par[3];
  fitval += par[1]*TMath::Exp(-0.5*arg*arg);

  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


double GaussLandau(double *x, double *par)
{
  // par[0] is Offset
  // par[1] is Landau-height
  // par[2] is Landau-mean
  // par[3] is Landau-sigma
  // par[4] is Gaus-height
  // par[5] is Gaus-mean
  // par[6] is Gaus-sigma

  double fitval = par[0];
  double arg = 0;

  if (par[6] != 0) arg = ((x[0] - par[5])/par[6]);
  fitval += par[4]*TMath::Exp(-0.5*arg*arg);

  fitval += par[1]*TMath::Landau(-x[0] + par[2], 0, par[3]);

  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


double Lorentz(double* x, double* par)
{
  double fitval = par[0]/(par[1]*par[1]+(x[0]-par[2])*(x[0]-par[2]));

  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


double LorentzARM(double* x, double* par)
{
  if ((par[1]*par[1]+(x[0]-par[2])*(x[0]-par[2])) == 0) return 0.0;
  return par[0]/(par[1]*par[1]+(x[0]-par[2])*(x[0]-par[2]))+par[3];
}


////////////////////////////////////////////////////////////////////////////////


double AsymGaus(double* x, double* par)
{
  // Parameters:
  // 0: Offset
  // 1: Height
  // 2: Mean
  // 3: Sigma of left gaus
  // 4: Sigma of right gaus

  double Ret = par[0];

  double arg = 0;
  if (x[0] - par[2] >= 0) {
    if (par[3] != 0) {
      arg = (x[0] - par[2])/par[3];
      Ret += par[1]*exp(-0.5*arg*arg);
    }
  } else {
    if (par[4] != 0) {
      arg = (x[0] - par[2])/par[4];
      Ret += par[1]*exp(-0.5*arg*arg);
    }
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


double LorentzLogNormalARM(double* x, double* par)
{
  if (par[5] == 0 || par[6] == 0) return 0.0;

  if ((x[0]-par[7])/par[5] > 0) {
    return par[0]/(par[1]*par[1]+(x[0]-par[2])*(x[0]-par[2]))+par[3] + 
      par[4]*exp(-0.5*pow(log((x[0]-par[7])/par[5])/par[6],2));
  } else {
    return par[0]/(par[1]*par[1]+(x[0]-par[2])*(x[0]-par[2]))+par[3];
  }
}


////////////////////////////////////////////////////////////////////////////////


double LorentzAsymGausArm(double* x, double* par)
{
  double Ret = par[0]/(par[1]*par[1]+(x[0]-par[2])*(x[0]-par[2]))+par[3];

  double arg = 0;
  if (x[0] - par[5] >= 0) {
    if (par[6] != 0) {
      arg = (x[0] - par[5])/par[6];
      Ret += par[4]*exp(-0.5*arg*arg);
    }
  } else {
    if (par[7] != 0) {
      arg = (x[0] - par[5])/par[7];
      Ret += par[4]*exp(-0.5*arg*arg);
    }
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


double DoubleLorentzAsymGausArm(double* x, double* par)
{
  double Ret = par[0];
  //Ret += fabs(par[3])/(par[2]*par[2]+(x[0]-par[1])*(x[0]-par[1]));
  //Ret += fabs(par[5])/(par[4]*par[4]+(x[0]-par[1])*(x[0]-par[1]));

  Ret += fabs(par[3]) * (par[2]*par[2])/(par[2]*par[2]+(x[0]-par[1])*(x[0]-par[1]));
  Ret += fabs(par[5]) * (par[4]*par[4])/(par[4]*par[4]+(x[0]-par[1])*(x[0]-par[1]));

  double arg = 0;
  if (x[0] - par[1] >= 0) {
    if (par[7] != 0) {
      arg = (x[0] - par[1])/par[7];
      Ret += fabs(par[6])*exp(-0.5*arg*arg);
    }
  } else {
    if (par[8] != 0) {
      arg = (x[0] - par[1])/par[8];
      Ret += fabs(par[6])*exp(-0.5*arg*arg);
    }
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


double ArcTanLorentzGausArm(double* x, double* par)
{
  // 0: Offset
  // 1: Tan height
  // 2: Tan width
  // 3: Gaus-height
  // 4: Gaus-mean
  // 5: Gaus-sigma
  // 6: Lorentz-height
  // 7: Lorentz-mean
  // 8: Lorentz-sigma

  double arg = 0;
  double Ret = par[0];

  // Tan:
  Ret += par[1]*atan(par[2]*x[0]);

  // Gaus:
  if (par[5] != 0) arg = (x[0] - par[4])/par[5];
  Ret += par[3]*TMath::Exp(-0.5*arg*arg);
                                                
  // Lorentz:
  Ret += fabs(par[6])/(par[8]*par[8]+(x[0]-par[7])*(x[0]-par[7]));
  
  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


double GaussSPD(double* x, double* par)
{
  double arg = 0;
  if (par[2] != 0) arg = (x[0] - par[1])/par[2];
  
  return par[0]*TMath::Exp(-0.5*arg*arg) + par[3];
}


////////////////////////////////////////////////////////////////////////////////


double LorentzGaussSPD(double* x, double* par)
{
  double arg = 0;
  if (par[1] != 0) arg = x[0]/par[1];

  if ((par[4]*par[4]+x[0]*x[0]) == 0) return 0.0;
  
  return par[0] + par[2]*TMath::Exp(-0.5*arg*arg) + 
    par[3]/(par[4]*par[4]+x[0]*x[0]);
}


////////////////////////////////////////////////////////////////////////////////


double TrippleGaussSPD(double* x, double* par)
{
  double val = par[0];
  double arg = 0;
  if (par[1] != 0) arg = x[0]/par[1];
  val += par[2]*TMath::Exp(-0.5*arg*arg);
  if (par[3] != 0) arg = x[0]/par[3];
  val += par[4]*TMath::Exp(-0.5*arg*arg);
  if (par[5] != 0) arg = x[0]/par[5];
  val += par[6]*TMath::Exp(-0.5*arg*arg);

  return val;
}


////////////////////////////////////////////////////////////////////////////////


double DoubleGauss(double* x, double* par)
{
//   L->SetParameters(1, 1, 1, 1, 1, 1, 1);
//   L->SetParLimits(1, 0, 1E10);
//   L->SetParLimits(2, 0, 1E10);
//   L->SetParLimits(3, 0, 1E10);
//   L->SetParLimits(4, 0, 1E10);
//   L->SetParLimits(5, 0, 1E10);
//   L->SetParLimits(6, 0, 1E10);
//   L->SetParNames("Offset", "Sigma1", "Mean1", "Height1", 
//                  "Sigma2", "Mean2", "Height2");

  double val = par[0];
  double arg = 0;
  if (par[1] != 0) arg = (x[0]-par[2])/par[1];
  val += par[3]*TMath::Exp(-0.5*arg*arg);
  arg = 0;
  if (par[4] != 0) arg = (x[0]-par[5])/par[4];
  val += par[6]*TMath::Exp(-0.5*arg*arg);

  return val;
}


////////////////////////////////////////////////////////////////////////////////


double TrippleGauss(double* x, double* par)
{
//   L->SetParameters(1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
//   L->SetParLimits(1, 0, 1E10);
//   L->SetParLimits(2, 0, 1E10);
//   L->SetParLimits(3, 0, 1E10);
//   L->SetParLimits(4, 0, 1E10);
//   L->SetParLimits(5, 0, 1E10);
//   L->SetParLimits(6, 0, 1E10);
//   L->SetParNames("Offset", "Sigma1", "Mean1", "Height1", 
//                  "Sigma2", "Mean2", "Height2",
//                  "Sigma3", "Mean3", "Height3");

  double val = par[0];
  double arg = 0;
  if (par[1] != 0) arg = (x[0]-par[2])/par[1];
  val += par[3]*TMath::Exp(-0.5*arg*arg);
  arg = 0;
  if (par[4] != 0) arg = (x[0]-par[5])/par[4];
  val += par[6]*TMath::Exp(-0.5*arg*arg);
  arg = 0;
  if (par[7] != 0) arg = (x[0]-par[8])/par[7];
  val += par[9]*TMath::Exp(-0.5*arg*arg);

  return val;
}


////////////////////////////////////////////////////////////////////////////////


double QuintupleGauss(double* x, double* par)
{
//   L->SetParameters(1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
//   L->SetParLimits(1, 0, 1E10);
//   L->SetParLimits(2, 0, 1E10);
//   L->SetParLimits(3, 0, 1E10);
//   L->SetParLimits(4, 0, 1E10);
//   L->SetParLimits(5, 0, 1E10);
//   L->SetParLimits(6, 0, 1E10);
//   L->SetParNames("Offset", "Sigma1", "Mean1", "Height1", 
//                  "Sigma2", "Mean2", "Height2",
//                  "Sigma3", "Mean3", "Height3");

  double val = par[0];
  double arg = 0;
  if (par[1] != 0) arg = (x[0]-par[2])/par[1];
  val += par[3]*TMath::Exp(-0.5*arg*arg);
  arg = 0;
  if (par[4] != 0) arg = (x[0]-par[5])/par[4];
  val += par[6]*TMath::Exp(-0.5*arg*arg);
  arg = 0;
  if (par[7] != 0) arg = (x[0]-par[8])/par[7];
  val += par[9]*TMath::Exp(-0.5*arg*arg);
  arg = 0;
  if (par[10] != 0) arg = (x[0]-par[11])/par[10];
  val += par[12]*TMath::Exp(-0.5*arg*arg);
  arg = 0;
  if (par[13] != 0) arg = (x[0]-par[14])/par[13];
  val += par[15]*TMath::Exp(-0.5*arg*arg);

  return val;
}


////////////////////////////////////////////////////////////////////////////////


double LandauGauss(double* x, double* par)
{
//   L->SetParameters(600, 600, 100, 1, 1, 1);
//   L->SetParLimits(1, 0, 1E10);
//   L->SetParLimits(2, 0, 1E10);
//   L->SetParLimits(3, 0, 1E10);
//   L->SetParLimits(4, 0, 1E10);
//   L->SetParLimits(5, 0, 1E10);
//   L->SetParLimits(6, 0, 1E10);
//   L->SetParNames("Offset", "MVP", "Sigma1", 
//                  "Sigma2", "Mean2", "Height2");
//   Hist->Fit("LandauGauss", "R");  double val = par[0];


  double val = par[0];
  double arg = 0;
  val += TMath::Landau(x[0], par[1], par[2]);
  if (par[3] != 0) arg = (x[0]-par[4])/par[3];
  val += par[5]*TMath::Exp(-0.5*arg*arg);

  return val;
}


////////////////////////////////////////////////////////////////////////////////


double PolarizationModulation(double* x, double* par)
{
  return par[0] + par[1]*cos(2*(x[0]-par[2] + 90.0)*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


double SpikyPolarizationModulation(double* x, double* par)
{
  double Mod = par[0] - par[1]*cos(2*x[0]*c_Rad+par[2]*c_Rad);

  int Angle;
  int width, height;
  for (width = 3, height = 12, Angle = -180; Angle <= 180; Angle += 45, width++, height++) {
    if (fabs(x[0]-Angle) < par[width]) {
      Mod += par[height];
    }
  }

  return Mod;
}


// MFitFunctions.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
