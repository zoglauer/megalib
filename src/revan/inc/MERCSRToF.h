/*
 * MERCSRToF.h
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


#ifndef __MERCSRToF__
#define __MERCSRToF__


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
#include "MREAMDriftChamberEnergy.h"

// Forward declarations:
class MRESE;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSRToF : public MERCSR
{
  // public interface:
 public:
  MERCSRToF();
  virtual ~MERCSRToF();

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:
  //MERCSRToF() {};
  //MERCSRToF(const MERCSRToF& ERCSR) {};

  virtual double ComputeQualityFactor(vector<MRESE*>& Interactions);

  // private methods:
 private:
  void CalculateTS(vector<MRESE*>& RESEs, double& TS);

  //! Upgrade energies and resolutions
  virtual void ModifyEventList();

  //! 
  void CorrectEnergiesAndTime(MREAMDriftChamberEnergy* Ream, 
                              vector<MRESE*>& Interactions);

  // protected members:
 protected:


  // private members:
 private:

#ifdef ___CLING___
 public:
  ClassDef(MERCSRToF, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
