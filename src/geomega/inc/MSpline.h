/*
 * MSpline.h
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


#ifndef __MSpline__
#define __MSpline__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TVector2.h>
#include <TObjArray.h>
#include <TSpline.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSpline
{
  // public interface:
 public:
  MSpline(int Mode = 0);
  MSpline(const MSpline& S);
  virtual ~MSpline();

  void AddDataPoint(double x, double y);

  int GetNDataPoints();
  double GetDataPointXValueAt(int i);
  double GetDataPointYValueAt(int i);

  double Get(double x);

  MString ToString();
  

  // protected methods:
 protected:
  TVector2* GetDataPointAt(int i);
  void SetDataPointAt(TVector2* V, int i);


  // private methods:
 private:


  // public members:
 public:
  enum Mode { Interpolation = 0, Spline };

  // protected members:
 protected:


  // private members:
 private:
  TObjArray *m_DataPoints;
  int m_NDataPoints;
  int m_Mode;

#ifdef ___CLING___
 public:
  ClassDef(MSpline, 0) // fit some data-points with a spline-function
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
