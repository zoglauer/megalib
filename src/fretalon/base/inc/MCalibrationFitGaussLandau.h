/*
 * MCalibrationFitGaussLandau.h
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


#ifndef __MCalibrationFitGaussLandau__
#define __MCalibrationFitGaussLandau__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MCalibrationFitGaussian.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A Gauss-Landau fit to a line profile - the mean is identical to the Gaussianone 
class MCalibrationFitGaussLandau : public MCalibrationFitGaussian
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationFitGaussLandau();
  //! Copy constructor
  MCalibrationFitGaussLandau(const MCalibrationFitGaussLandau& Fit);
  //! Default destuctor 
  virtual ~MCalibrationFitGaussLandau();

  //! The assignment operator
  MCalibrationFitGaussLandau& operator= (const MCalibrationFitGaussLandau& Fit);

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationFitGaussLandau* Clone() const;

  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const;
  //! The number of parameters for ROOT fitting
  virtual unsigned int NPar() const;
  
  //! Fit the given histogram in the given range
  virtual bool Fit(TH1D& Histogram, double Min, double Max);

  //! Set the height of the Landau distribution
  void SetLandauHeight(double Height) { m_LandauHeight = Height; }
  //! Set the sigma of the Landau distribution
  void SetLandauSigma(double Sigma) { m_LandauSigma = Sigma; }

  //! Get the FWHM
  virtual double GetFWHM() const;

  // protected methods:
 protected:
  //! Set all fit parameters
  virtual void SetFitParameters(ROOT::Fit::Fitter& Fitter, TH1D& Hist, double Min, double Max);

  // private methods:
 private:



  // protected members:
 protected:
  //! Landau height
  double m_LandauHeight;
  //! Landau Sigma
  double m_LandauSigma;


  // private members:
 private:
   


#ifdef ___CLING___
 public:
  ClassDef(MCalibrationFitGaussLandau, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
