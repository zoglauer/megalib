/*
 * MFPDataPoint.h
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


#ifndef __MFPDataPoint__
#define __MFPDataPoint__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>

// ROOT libs
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


class MFPDataPoint : public TObject
{
  // Public Interface:
 public:
  MFPDataPoint();
  MFPDataPoint(double Psi, double Eta, double Phi, double Theta);
  ~MFPDataPoint();


  double GetPsi() { return m_Psi; }
  void SetPsi(double Psi) { m_Psi = Psi; }

  double GetEta() { return m_Eta; }
  void SetEta(double Eta) { m_Eta = Eta; }

  double GetPhi() { return m_Phi; }
  void SetPhi(double Phi) { m_Phi = Phi; }

  double GetTheta() { return m_Theta; }
  void SetTheta(double Theta) { m_Theta = Theta; }


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  double m_Psi;
  double m_Eta;
  double m_Phi;
  double m_Theta;


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MFPDataPoint, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
