/*
 * UTERDataSetShared.h
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


#ifndef __UTERDataSetShared__
#define __UTERDataSetShared__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>
#include <sstream>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MDMaterial.h"
#include "MGeometryRevan.h"
#include "MGlobal.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MString.h"
#include "MVector.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Base class of the unit tests of the Compton sequence data sets (MERQualityDataSet, MERCSRDataSet):
//! cross sections for the telescope, sequences of hits, and the expected values of the Compton kinematics
class UTERDataSetBase : public UTRevanBase
{
public:
  //! Standard constructor with the name of the test
  UTERDataSetBase(const MString& Name) : UTRevanBase(Name) {}
  //! Default destructor
  virtual ~UTERDataSetBase() {}

protected:
  //! Write cross section tables with known values for the materials of the telescope and load them into the scanned geometry
  //! The total absorption coefficients are Vacuum: 0.0078125, Silicon: 0.5, CsI: 0.25, Germanium: 0.125, NE110: 0.0625 per cm up to 500 keV,
  //! and twice as much from 501 keV on. Photo, Compton, pair, and Rayleigh are 1/4, 1/2, 1/8, and 1/16 of the total one.
  //! Only powers of 2 are used, since the tables only have float precision
  bool LoadTestCrossSections(MGeometryRevan& Geometry)
  {
    if (PrepareTemporaryDirectory("cross-sections") == false) return false;

    vector<MString> Types = { "Total", "Photo", "Compton", "Pair", "Rayleigh" };
    vector<double> TypeFactors = { 1.0, 0.25, 0.5, 0.125, 0.0625 };
    vector<MString> Names = { "Vacuum", "Silicon", "CsI", "Germanium", "NE110" };
    vector<double> Coefficients = { 0.0078125, 0.5, 0.25, 0.125, 0.0625 };

    for (unsigned int m = 0; m < Names.size(); ++m) {
      MDMaterial* Material = Geometry.GetMaterial(Names[m]);
      if (Material == nullptr) return false;
      for (unsigned int t = 0; t < Types.size(); ++t) {
        double Value = Coefficients[m]*TypeFactors[t];
        ostringstream Table;
        Table<<"Type ResponseMatrixO1"<<endl;
        Table<<"Version 1"<<endl;
        Table<<endl;
        Table<<"NM Test table"<<endl;
        Table<<"CE false"<<endl;
        Table<<"HA "<<Material->GetHash()<<endl;
        Table<<endl;
        Table<<"R1 1.0 "<<Value<<endl;
        Table<<"R1 500.0 "<<Value<<endl;
        Table<<"R1 501.0 "<<2.0*Value<<endl;
        Table<<"R1 100000000.0 "<<2.0*Value<<endl;
        Table<<endl;
        MString FileName = GetTemporaryDirectoryName("cross-sections") + "/Xsection." + Types[t] + "." + Names[m] + ".rsp";
        if (WriteTextFile(FileName, Table.str().c_str()) == false) return false;
      }
      DisableDefaultStreams();
      bool Loaded = Material->LoadCrossSections();
      EnableDefaultStreams();
      if (Loaded == false || Material->AreCrossSectionsPresent() == false) return false;
    }

    return true;
  }

  //! Return new hits at the given positions (cm) with the given energies (keV), without volume sequence; the caller deletes them with DeleteSequence
  vector<MRESE*> CreateSequence(const vector<MVector>& Positions, const vector<double>& Energies)
  {
    vector<MRESE*> Sequence;
    for (unsigned int i = 0; i < Positions.size(); ++i) {
      Sequence.push_back(CreateHit(Positions[i], Energies[i], MDDetector::c_Strip2D));
    }

    return Sequence;
  }

  //! Delete the hits of a sequence
  void DeleteSequence(vector<MRESE*>& Sequence)
  {
    for (unsigned int i = 0; i < Sequence.size(); ++i) {
      delete Sequence[i];
    }
    Sequence.clear();
  }

  //! Return the tolerance for a value which was stored with single precision: the spacing of floats at this value
  double GetFloatTolerance(double Value) const
  {
    // The relative spacing of single precision numbers is 2^-23 = 1.2e-7, the rounding error is at most half of it
    return 1.2e-7*fabs(Value) + 1e-30;
  }

  //! Return the Compton absorption probabilities from far outside to the first interaction for directions on the Compton cone:
  //! the origin directions have the angle 180 degrees - Phi to the direction from the first to the second interaction, and Samples
  //! equally spaced azimuth angles around it, starting at the direction given by MVector::Orthogonal() of the scatter direction
  vector<double> GetConeProbabilities(MGeometryRevan& Geometry, const MVector& First, const MVector& Second, double Phi, double Energy, unsigned int Samples) const
  {
    vector<double> Probabilities;

    MVector Scattered = Second - First;
    // The flight direction of the incoming gamma ray has the angle Phi to the one of the scattered gamma ray
    MVector Incoming = Scattered;
    Incoming.RotateAroundVector(Scattered.Orthogonal(), Phi);

    for (unsigned int s = 0; s < Samples; ++s) {
      // The direction from the first interaction back to the origin of the gamma ray
      MVector Origin = -Incoming;
      Origin.RotateAroundVector(Scattered, s*c_TwoPi/Samples);
      Origin.Unitize();
      Probabilities.push_back(Geometry.GetComptonAbsorptionProbability(First + 1000000*Origin, First, Energy));
    }

    return Probabilities;
  }
};


#endif


// UTERDataSetShared.h: the end...
////////////////////////////////////////////////////////////////////////////////
