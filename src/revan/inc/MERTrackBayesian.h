/*
 * MERTrackBayesian.h
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


#ifndef __MERTrackBayesian__
#define __MERTrackBayesian__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventIncarnations.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////


class MERTrackBayesian : public MERTrack
{
  // public interface:
 public:
  MERTrackBayesian();
  virtual ~MERTrackBayesian();

  //! Set all special parameters - this function should not rely on a previous 
  //! call to SetParameters()
  virtual bool SetSpecialParameters(MString FileName);

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:
  //MERTrackBayesian() {};
  //MERTrackBayesian(const MERTrackBayesian& ERTrack) {};

  virtual bool EvaluateTracks(MRERawEvent* ER);

  virtual void SortByTrackQualityFactor(MRawEventIncarnations* List);

  // private methods:
 private:
  double CalculateAngleIn(MRESE* Start, MRESE* Central);
  double CalculateAngleOutPhi(MRESE* Start, MRESE* Central, MRESE* Stop);
  double CalculateAngleOutTheta(MRESE* Start, MRESE* Central, MRESE* Stop);



  // protected members:
 protected:

  // private members:
 private:
  MString m_FileName;

  MResponseMatrixO1 m_GoodBad;

  MResponseMatrixO3 m_GoodStart;
  MResponseMatrixO5 m_GoodCentral;
  MResponseMatrixO2 m_GoodStop;
  MResponseMatrixO3 m_GoodDual;
  float m_SumGoodStart; 
  float m_SumGoodCentral; 
  float m_SumGoodStop; 
  float m_SumGoodDual; 

  MResponseMatrixO3 m_BadStart;
  MResponseMatrixO5 m_BadCentral;
  MResponseMatrixO2 m_BadStop;
  MResponseMatrixO3 m_BadDual;
  float m_SumBadStart;
  float m_SumBadCentral;
  float m_SumBadStop;
  float m_SumBadDual;


#ifdef ___CLING___
 public:
  ClassDef(MERTrackBayesian, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
