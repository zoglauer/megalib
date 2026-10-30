/*
 * MERCSREnergyRecovery.h
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


#ifndef __MERCSREnergyRecovery__
#define __MERCSREnergyRecovery__


////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
using namespace std;

// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MERCSR.h"
#include "MRawEventIncarnations.h"
#include "MComptonEvent.h"
#include "MVector.h"


// Forward declarations:
class MRESE;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSREnergyRecovery : public MERCSR
{
  // public interface:
 public:
  MERCSREnergyRecovery();
  virtual ~MERCSREnergyRecovery();

  virtual MString ToString(bool CoreOnly = false) const;

  static const int c_TestStatisticsChiSquareWithoutErrors;
  static const int c_TestStatisticsChiSquareWithErrors;
  static const int c_TestStatisticsChiSquareProbability;

  static const int c_TestStatisticsFirst;
  static const int c_TestStatisticsLast;

  // protected methods:
 protected:
  //MERCSREnergyRecovery() {};
  //MERCSREnergyRecovery(const MERCSREnergyRecovery& ERCSR) {};

  virtual double ComputeQualityFactor(vector<MRESE*>& Interactions);

  /// Returns the escaped energy of the initial photons 
  virtual double GetEscapedEnergy(vector<MRESE*>& RESEs);


  // private methods:
 private:
  /// The actual calculation of the quality factor is done here
  void CalculateQF(vector<MRESE*>& RESEs, double& QF, double& Eavg);

  /// Split tracks
  virtual void ModifyEventList();

  // protected members:
 protected:
  int m_TestStatisticsType;
  int m_MaxSearchSequence;

  // private members:
 private:

#ifdef ___CLING___
 public:
  ClassDef(MERCSREnergyRecovery, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
