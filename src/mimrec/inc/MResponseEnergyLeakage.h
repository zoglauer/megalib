/*
 * MResponseEnergyLeakage.h
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


#ifndef __MResponseEnergyLeakage__
#define __MResponseEnergyLeakage__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponse.h"

// Forward declarations:
class MComptonEvent;

////////////////////////////////////////////////////////////////////////////////


class MResponseEnergyLeakage : public MResponse
{
  // public interface:
 public:
  MResponseEnergyLeakage(const double Electron, const double Gamma);
  virtual ~MResponseEnergyLeakage();

  void SetGaussians(const double Electron, const double Gamma);

  virtual double GetComptonResponse(const double t);
  virtual double GetComptonResponse(const double t, const double l);
  virtual double GetComptonMaximum();
  virtual double GetComptonIntegral(const double Radius) const;

  virtual double GetComptonTransversalMax();
  virtual double GetComptonTransversalMin();

  bool AnalyzeEvent(MPhysicalEvent* Event);

  double GetKleinNishina(const double Phi) const;

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  double m_FitElectron;         // longitudinal sigma of the cone-arc
  double m_FitGamma;          // transversal sigma of the cone

  double m_tmin;
  double m_tmax;

  double m_Theta;

  MComptonEvent *m_Compton;


#ifdef ___CLING___
 public:
  ClassDef(MResponseEnergyLeakage, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
