/*
 * UTBinnerSpherical.cxx
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


// Standard libs:
#include <cmath>
#include <memory>
#include <sstream>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MBinnerFISBEL.h"
#include "MBinnerHEALPix.h"
#include "MBinnerSpherical.h"
#include "MGlobal.h"
#include "MUnitTest.h"


//! Unit test class for the interface of MBinnerSpherical, tested through all its implementations
class UTBinnerSpherical : public MUnitTest
{
public:
  UTBinnerSpherical() : MUnitTest("UTBinnerSpherical") {}
  virtual ~UTBinnerSpherical() {}

  virtual bool Run();

private:
  //! Test the contract which holds for every spherical binner
  bool TestContract(const MBinnerSpherical& Binner, const MString& Name);
};


////////////////////////////////////////////////////////////////////////////////


bool UTBinnerSpherical::TestContract(const MBinnerSpherical& Binner, const MString& Name)
{
  bool Passed = true;

  const unsigned int NBins = Binner.GetNBins();
  Passed = EvaluateTrue("GetNBins()", Name, "The binner has at least one bin", NBins >= 1) && Passed;

  // Centers
  vector<MVector> Centers = Binner.GetAllBinCenters();
  Passed = EvaluateSize("GetAllBinCenters()", Name, "There is one center vector per bin", Centers.size(), NBins) && Passed;
  unsigned int WrongBins = 0;
  unsigned int NotUnit = 0;
  unsigned int WrongSize = 0;
  for (unsigned int b = 0; b < NBins; ++b) {
    vector<double> Center = Binner.GetBinCenters(b);
    if (Center.size() != 2) {
      ++WrongSize;
      continue;
    }
    if (Binner.FindBin(Center[0], Center[1]) != b) {
      ++WrongBins;
    }
    if (b < Centers.size() && fabs(Centers[b].Mag() - 1.0) > 1e-9) {
      ++NotUnit;
    }
  }
  Passed = Evaluate("GetBinCenters()", Name, "Every bin center has two coordinates (number of others)", WrongSize, 0U) && Passed;
  Passed = Evaluate("FindBin()", Name, "Every bin center is found in its own bin (number of wrong bins)", WrongBins, 0U) && Passed;
  Passed = Evaluate("GetAllBinCenters()", Name, "All center vectors have length one (number of others)", NotUnit, 0U) && Passed;

  // Every direction is in a valid bin, including the poles, the phi limits, and negative or large phi
  unsigned int OutOfRange = 0;
  const vector<double> Thetas = { 0.0, 1e-9, 0.3, 0.5*c_Pi, 2.5, c_Pi - 1e-9, c_Pi };
  const vector<double> Phis = { -7.0, -1.0, 0.0, 1e-9, 1.0, c_Pi, 2*c_Pi - 1e-9, 2*c_Pi, 2*c_Pi + 1.0, 20.0 };
  for (double Theta: Thetas) {
    for (double Phi: Phis) {
      if (Binner.FindBin(Theta, Phi) >= NBins) {
        ++OutOfRange;
      }
    }
  }
  Passed = Evaluate("FindBin()", Name, "Every direction is in a valid bin (number of invalid)", OutOfRange, 0U) && Passed;

  // The limits are in degrees: theta 0..180, phi spans 360 degrees
  vector<double> Minima = Binner.GetMinima();
  vector<double> Maxima = Binner.GetMaxima();
  Passed = EvaluateSize("GetMinima()", Name, "There are two minima", Minima.size(), 2U) && Passed;
  Passed = EvaluateSize("GetMaxima()", Name, "There are two maxima", Maxima.size(), 2U) && Passed;
  if (Minima.size() == 2 && Maxima.size() == 2) {
    Passed = EvaluateNear("GetMinima()", Name + " theta", "The minimum of theta is 0 degrees", Minima[0], 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetMaxima()", Name + " theta", "The maximum of theta is 180 degrees", Maxima[0], 180.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetMaxima()-GetMinima()", Name + " phi", "The range of phi is 360 degrees", Maxima[1] - Minima[1], 360.0, 1e-9) && Passed;
  }

  // Drawing axes: phi and theta edges, increasing, with theta ending at pi and phi spanning 2 pi
  vector<vector<double>> Edges = Binner.GetDrawingAxisBinEdges();
  Passed = EvaluateSize("GetDrawingAxisBinEdges()", Name, "There are two drawing axes", Edges.size(), 2U) && Passed;
  if (Edges.size() == 2) {
    Passed = EvaluateTrue("GetDrawingAxisBinEdges()", Name + " phi", "The phi drawing axis has at least two edges", Edges[0].size() >= 2) && Passed;
    Passed = EvaluateTrue("GetDrawingAxisBinEdges()", Name + " theta", "The theta drawing axis has at least two edges", Edges[1].size() >= 2) && Passed;
  }
  if (Edges.size() == 2 && Edges[0].size() >= 2 && Edges[1].size() >= 2) {
    bool Increasing = true;
    for (unsigned int a = 0; a < 2; ++a) {
      for (unsigned int i = 1; i < Edges[a].size(); ++i) {
        if (Edges[a][i] <= Edges[a][i-1]) {
          Increasing = false;
        }
      }
    }
    Passed = EvaluateTrue("GetDrawingAxisBinEdges()", Name, "The edges of both drawing axes are strictly increasing", Increasing) && Passed;
    Passed = EvaluateNear("GetDrawingAxisBinEdges()", Name + " theta start", "The theta axis starts at 0", Edges[1].front(), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetDrawingAxisBinEdges()", Name + " theta stop", "The theta axis ends at pi", Edges[1].back(), c_Pi, 1e-9) && Passed;
    Passed = EvaluateNear("GetDrawingAxisBinEdges()", Name + " phi range", "The phi axis spans 2 pi", Edges[0].back() - Edges[0].front(), 2*c_Pi, 1e-9) && Passed;
  }

  // Writing
  ostringstream Out;
  Binner.Write("Axis", Out);
  MString Text = Out.str();
  Passed = EvaluateTrue("Write()", Name, "The written axis starts with the axis name block", Text.BeginsWith("# Axis name\nAN \"Axis\"\n# Axis type\nAT 2D ")) && Passed;
  Passed = EvaluateTrue("Write()", Name, "The written axis ends with the axis data line", Text.Contains("\n# Axis data\nAD ") && Text.EndsWith("\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTBinnerSpherical::Run()
{
  bool Passed = true;

  // FISBEL and HEALPix through the common interface (pointer to the base class)
  vector<unique_ptr<MBinnerSpherical>> Binners;
  vector<MString> Names;

  const vector<unsigned int> FISBELBins = { 1, 2, 4, 12, 100 };
  for (unsigned int b: FISBELBins) {
    unique_ptr<MBinnerFISBEL> Binner(new MBinnerFISBEL());
    Binner->Create(b);
    Binners.push_back(std::move(Binner));
    Names.push_back(MString("FISBEL ") + b + " bins");
  }
  {
    unique_ptr<MBinnerFISBEL> Binner(new MBinnerFISBEL());
    Binner->Create(12, 30.0*c_Rad);
    Binners.push_back(std::move(Binner));
    Names.push_back("FISBEL 12 bins, shift 30 degrees");
  }
  for (int Order = -1; Order <= 3; ++Order) {
    Binners.push_back(unique_ptr<MBinnerSpherical>(new MBinnerHEALPix(Order)));
    Names.push_back(MString("HEALPix order ") + Order);
  }

  for (unsigned int i = 0; i < Binners.size(); ++i) {
    Passed = TestContract(*Binners[i], Names[i]) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinnerSpherical Test;
  return Test.Run() == true ? 0 : 1;
}


// UTBinnerSpherical.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
