/*
 * MFitFunctions.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFitFunctions__
#define __MFitFunctions__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

double Lorentz2(double* x, double* par);
double Lorentz(double* x, double* par);
double LorentzARM(double* x, double* par);
double Gauss(double* x, double* par);
double Gauss2(double* x, double* par);
double GaussLandau(double* x, double* par);
double AsymGaus(double* x, double* par);
double LorentzLogNormalARM(double* x, double* par);
double LorentzAsymGausArm(double* x, double* par);
double DoubleLorentzAsymGausArm(double* x, double* par);
double ArcTanLorentzGausArm(double* x, double* par);
double GaussSPD(double* x, double* par);
double LorentzGaussSPD(double* x, double* par);
double TrippleGaussSPD(double* x, double* par);
double DoubleGauss(double* x, double* par);
double TrippleGauss(double* x, double* par);
double QuintupleGauss(double* x, double* par);
double LandauGauss(double* x, double* par);
double PolarizationModulation(double* x, double* par);
double SpikyPolarizationModulation(double* x, double* par);


#endif


////////////////////////////////////////////////////////////////////////////////
