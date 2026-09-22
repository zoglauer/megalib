/*
 * MFitFunctions.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

double Linear(double* x, double* par);
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
