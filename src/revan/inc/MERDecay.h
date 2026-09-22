/*
 * MERDecay.h
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


#ifndef __MERDecay__
#define __MERDecay__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventIncarnations.h"

// Forward declarations:
class MRERawEvent;

////////////////////////////////////////////////////////////////////////////////


class MERDecay : public MERConstruction
{
  // public interface:
 public:
  MERDecay();
  virtual ~MERDecay();

  virtual bool SetParameters(MString FileName, 
                             const vector<double>& Energy, 
                             const vector<double>& EnergyError);
  virtual bool Analyze(MRawEventIncarnations* List);

  virtual bool PreAnalysis();
  virtual bool PostAnalysis();

  virtual MString ToString(bool CoreOnly = false) const;

  static const int c_AllCombinations;
  static const int c_AllButOneCombination;
  static const int c_ComptonSequenceEnd;


  // protected methods:
 protected:
  //MERDecay() {};
  //MERDecay(const MERDecay& ERDecay) {};

  bool IsDecay(double Energy, double EnergyError);
  bool CheckCombinations(double Energy, double EnergyError, 
                         int Start, MRERawEvent* RE, int Level);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  vector<int> m_Occupation;
  vector<double> m_Energy;
  vector<double> m_EnergyErrorSigma;

  int m_Mode;

#ifdef ___CLING___
 public:
  ClassDef(MERDecay, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
