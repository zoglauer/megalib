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


#ifdef ___CLING___
//ClassImp(MFitFunctions)
#endif


////////////////////////////////////////////////////////////////////////////////


double Linear(double* x, double* par)
{
  // Parameters:
  // 0: Offset
  // 1: Gradient
  
  double fitval = par[0] + par[1]*x[0];
  
  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


double Lorentz2(double* x, double* par)
{
  // Parameters:
  // 0: Height
  // 1: Sigma

  double fitval = par[0]/(par[1]*par[1]+x[0]*x[0]);
  
  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


double Gauss(double* x, double* par)
{
  // Parameters:
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
  // Parameters:
  // 0: Offset
  // 1: Gaussian height
  // 2: Gaussian mean
  // 3: Gaussian sigma

  double fitval = par[0];
  double arg = 0;

  if (par[3] != 0) arg = (x[0] - par[2])/par[3];
  fitval += par[1]*TMath::Exp(-0.5*arg*arg);

  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


double GaussLandau(double *x, double *par)
{
  // Parameters:
  // 0: Offset
  // 1: Landau height
  // 2: Landau mean
  // 3: Landau sigma
  // 4: Gaussian height
  // 5: Gaussian mean
  // 6: Gaussian sigma

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
  // Parameters:
  // 0: Height
  // 1: Sigma
  // 2: Mean

  double fitval = par[0]/(par[1]*par[1]+(x[0]-par[2])*(x[0]-par[2]));

  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


double LorentzARM(double* x, double* par)
{
  // Parameters:
  // 0: Height
  // 1: Sigma
  // 2: Mean
  // 3: Offset

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
  // 3: Sigma of right gaus
  // 4: Sigma of left gaus

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
  // Parameters:
  // 0: Lorentz height
  // 1: Lorentz sigma
  // 2: Lorentz mean
  // 3: Offset
  // 4: Log-normal height
  // 5: Log-normal scale
  // 6: Log-normal sigma
  // 7: Log-normal shift

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
  // Parameters:
  // 0: Lorentz height
  // 1: Lorentz sigma
  // 2: Lorentz mean
  // 3: Offset
  // 4: Gaussian height
  // 5: Gaussian mean
  // 6: Sigma of right gaus
  // 7: Sigma of left gaus

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
  // Parameters:
  // 0: Offset
  // 1: Common mean
  // 2: Sigma of Lorentz component 1
  // 3: Height of Lorentz component 1
  // 4: Sigma of Lorentz component 2
  // 5: Height of Lorentz component 2
  // 6: Height of asymmetric Gaussian component
  // 7: Sigma of right gaus
  // 8: Sigma of left gaus

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
  // Parameters:
  // 0: Offset
  // 1: Arctan height
  // 2: Arctan width
  // 3: Gaussian height
  // 4: Gaussian mean
  // 5: Gaussian sigma
  // 6: Lorentz height
  // 7: Lorentz mean
  // 8: Lorentz sigma

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
  // Parameters:
  // 0: Height
  // 1: Mean
  // 2: Sigma
  // 3: Offset

  double arg = 0;
  if (par[2] != 0) arg = (x[0] - par[1])/par[2];
  
  return par[0]*TMath::Exp(-0.5*arg*arg) + par[3];
}


////////////////////////////////////////////////////////////////////////////////


double LorentzGaussSPD(double* x, double* par)
{
  // Parameters:
  // 0: Offset
  // 1: Gaussian sigma
  // 2: Gaussian height
  // 3: Lorentz height
  // 4: Lorentz sigma

  double arg = 0;
  if (par[1] != 0) arg = x[0]/par[1];

  if ((par[4]*par[4]+x[0]*x[0]) == 0) return 0.0;
  
  return par[0] + par[2]*TMath::Exp(-0.5*arg*arg) + 
    par[3]/(par[4]*par[4]+x[0]*x[0]);
}


////////////////////////////////////////////////////////////////////////////////


double TrippleGaussSPD(double* x, double* par)
{
  // Parameters:
  // 0: Offset
  // 1: Sigma of Gaussian 1
  // 2: Height of Gaussian 1
  // 3: Sigma of Gaussian 2
  // 4: Height of Gaussian 2
  // 5: Sigma of Gaussian 3
  // 6: Height of Gaussian 3

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
  // Parameters:
  // 0: Offset
  // 1: Sigma of Gaussian 1
  // 2: Mean of Gaussian 1
  // 3: Height of Gaussian 1
  // 4: Sigma of Gaussian 2
  // 5: Mean of Gaussian 2
  // 6: Height of Gaussian 2

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
  // Parameters:
  // 0: Offset
  // 1: Sigma of Gaussian 1
  // 2: Mean of Gaussian 1
  // 3: Height of Gaussian 1
  // 4: Sigma of Gaussian 2
  // 5: Mean of Gaussian 2
  // 6: Height of Gaussian 2
  // 7: Sigma of Gaussian 3
  // 8: Mean of Gaussian 3
  // 9: Height of Gaussian 3

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
  // Parameters:
  // 0: Offset
  // 1: Sigma of Gaussian 1
  // 2: Mean of Gaussian 1
  // 3: Height of Gaussian 1
  // 4: Sigma of Gaussian 2
  // 5: Mean of Gaussian 2
  // 6: Height of Gaussian 2
  // 7: Sigma of Gaussian 3
  // 8: Mean of Gaussian 3
  // 9: Height of Gaussian 3
  // 10: Sigma of Gaussian 4
  // 11: Mean of Gaussian 4
  // 12: Height of Gaussian 4
  // 13: Sigma of Gaussian 5
  // 14: Mean of Gaussian 5
  // 15: Height of Gaussian 5

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
  // Parameters:
  // 0: Offset
  // 1: Landau most probable value
  // 2: Landau sigma
  // 3: Gaussian sigma
  // 4: Gaussian mean
  // 5: Gaussian height

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
  // Parameters:
  // 0: Offset
  // 1: Modulation amplitude
  // 2: Modulation phase in degree

  return par[0] + par[1]*cos(2*(x[0]-par[2] + 90.0)*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


double SpikyPolarizationModulation(double* x, double* par)
{
  // Parameters:
  // 0: Offset
  // 1: Modulation amplitude
  // 2: Modulation phase in degree
  // 3..11: Spike half-widths for angles -180, -135, -90, -45, 0, 45, 90, 135, 180
  // 12..20: Spike heights for angles -180, -135, -90, -45, 0, 45, 90, 135, 180

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
