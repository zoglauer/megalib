/*
 * UTBinnerHEALPix.cxx
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
#include <sstream>
using namespace std;

// MEGAlib libs:
#include "MBinnerFISBEL.h"
#include "MBinnerHEALPix.h"
#include "MExceptions.h"
#include "MGlobal.h"
#include "MUnitTest.h"


//! Unit test class for MBinnerHEALPix
class UTBinnerHEALPix : public MUnitTest
{
public:
  UTBinnerHEALPix() : MUnitTest("UTBinnerHEALPix") {}
  virtual ~UTBinnerHEALPix() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTBinnerHEALPix::Run()
{
  bool Passed = true;

  // The number of bins is 12 * 4^Order, NSIDE = 2^Order
  {
    const vector<int> Orders = { 0, 1, 2, 3, 5 };
    const vector<unsigned int> NBins = { 12U, 48U, 192U, 768U, 12288U };
    const vector<unsigned int> NSides = { 1U, 2U, 4U, 8U, 32U };
    for (unsigned int o = 0; o < Orders.size(); ++o) {
      MBinnerHEALPix Binner(Orders[o]);
      Passed = Evaluate("GetOrder()", Orders[o], "The order is the one given to the constructor", Binner.GetOrder(), Orders[o]) && Passed;
      Passed = Evaluate("GetNBins()", Orders[o], "The number of bins is 12 * 4^order", Binner.GetNBins(), NBins[o]) && Passed;
      Passed = Evaluate("GetNSide()", Orders[o], "NSIDE is 2^order", Binner.GetNSide(), NSides[o]) && Passed;
    }
  }

  // A negative order is a single bin covering the whole sky
  {
    MBinnerHEALPix Single(-1);
    Passed = Evaluate("GetNBins()", "order -1", "A negative order has one bin", Single.GetNBins(), 1U) && Passed;
    Passed = Evaluate("GetOrder()", "order -1", "A negative order is stored as -1", Single.GetOrder(), -1) && Passed;
    Passed = Evaluate("FindBin()", "order -1 north pole", "Every direction is in bin 0 (north pole)", Single.FindBin(0.0, 0.0), 0U) && Passed;
    Passed = Evaluate("FindBin()", "order -1 south pole", "Every direction is in bin 0 (south pole)", Single.FindBin(c_Pi, 1.0), 0U) && Passed;
    Passed = Evaluate("FindBin()", "order -1 equator", "Every direction is in bin 0 (equator)", Single.FindBin(0.5*c_Pi, 4.0), 0U) && Passed;
    Passed = EvaluateSize("GetBinCenters()", "order -1", "The single bin has a center with two coordinates", Single.GetBinCenters(0).size(), 2U) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "order -1 theta", "The center of the single bin is the north pole (theta)", Single.GetBinCenters(0)[0], 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "order -1 phi", "The center of the single bin is the north pole (phi)", Single.GetBinCenters(0)[1], 0.0, 1e-12) && Passed;
    Passed = EvaluateSize("GetAllBinCenters()", "order -1", "The single bin has one center vector", Single.GetAllBinCenters().size(), 1U) && Passed;
    vector<vector<double>> Edges = Single.GetDrawingAxisBinEdges();
    Passed = EvaluateSize("GetDrawingAxisBinEdges()", "order -1", "The drawing axes of the single bin are phi and theta", Edges.size(), 2U) && Passed;
    if (Edges.size() == 2) {
      Passed = EvaluateSize("GetDrawingAxisBinEdges()", "order -1 phi", "The single bin has two phi edges", Edges[0].size(), 2U) && Passed;
      if (Edges[0].size() == 2) {
        Passed = EvaluateNear("GetDrawingAxisBinEdges()", "order -1 phi stop", "The single bin ends at phi = 2 pi", Edges[0].back(), 2*c_Pi, 1e-12) && Passed;
      }
      Passed = EvaluateSize("GetDrawingAxisBinEdges()", "order -1 theta", "The single bin has two theta edges", Edges[1].size(), 2U) && Passed;
      if (Edges[1].size() == 2) {
        Passed = EvaluateNear("GetDrawingAxisBinEdges()", "order -1 theta stop", "The single bin ends at theta = pi", Edges[1].back(), c_Pi, 1e-12) && Passed;
      }
    }
  }

  // Order 0 (NSIDE = 1) has the 12 base pixels, which are known:
  // - pixels 0..3: north polar cap, center at theta = acos(2/3), phi = 45, 135, 225, 315 degrees
  // - pixels 4..7: equatorial belt, center at theta = 90 degrees, phi = 0, 90, 180, 270 degrees
  // - pixels 8..11: south polar cap, center at theta = 180 - acos(2/3) degrees, phi = 45, 135, 225, 315 degrees
  {
    MBinnerHEALPix Base(0);
    const double CapTheta = acos(2.0/3.0);
    Passed = EvaluateNear("GetBinCenters()", "base pixel 0 theta", "The first north-cap pixel is at theta = acos(2/3)", Base.GetBinCenters(0)[0], CapTheta, 1e-9) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "base pixel 0 phi", "The first north-cap pixel is at phi = 45 degrees", Base.GetBinCenters(0)[1], 0.25*c_Pi, 1e-9) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "base pixel 3 phi", "The fourth north-cap pixel is at phi = 315 degrees", Base.GetBinCenters(3)[1], 1.75*c_Pi, 1e-9) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "base pixel 4 theta", "The first equatorial pixel is at theta = 90 degrees", Base.GetBinCenters(4)[0], 0.5*c_Pi, 1e-9) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "base pixel 4 phi", "The first equatorial pixel is at phi = 0", Base.GetBinCenters(4)[1], 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "base pixel 5 phi", "The second equatorial pixel is at phi = 90 degrees", Base.GetBinCenters(5)[1], 0.5*c_Pi, 1e-9) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "base pixel 8 theta", "The first south-cap pixel is at theta = pi - acos(2/3)", Base.GetBinCenters(8)[0], c_Pi - CapTheta, 1e-9) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "base pixel 8 phi", "The first south-cap pixel is at phi = 45 degrees", Base.GetBinCenters(8)[1], 0.25*c_Pi, 1e-9) && Passed;

    Passed = Evaluate("FindBin()", "north cap 1", "A direction at theta = 0.1, phi = 0.1 is in the first north-cap pixel", Base.FindBin(0.1, 0.1), 0U) && Passed;
    Passed = Evaluate("FindBin()", "north cap 2", "A direction at theta = 0.1, phi = 90 degrees + 0.1 is in the second north-cap pixel", Base.FindBin(0.1, 0.5*c_Pi + 0.1), 1U) && Passed;
    Passed = Evaluate("FindBin()", "equator 1", "A direction at the equator, phi = 0.1 is in the first equatorial pixel", Base.FindBin(0.5*c_Pi, 0.1), 4U) && Passed;
    Passed = Evaluate("FindBin()", "equator 2", "A direction at the equator, phi = 90 degrees + 0.1 is in the second equatorial pixel", Base.FindBin(0.5*c_Pi, 0.5*c_Pi + 0.1), 5U) && Passed;
    Passed = Evaluate("FindBin()", "south cap 1", "A direction at theta = pi - 0.1, phi = 0.1 is in the first south-cap pixel", Base.FindBin(c_Pi - 0.1, 0.1), 8U) && Passed;
    Passed = Evaluate("FindBin()", "phi wrap", "Phi is periodic: phi = 2 pi + 0.1 is in the same pixel as phi = 0.1", Base.FindBin(0.5*c_Pi, 2*c_Pi + 0.1), 4U) && Passed;
  }

  // Properties of all pixels for several orders
  for (unsigned int Order = 0; Order <= 3; ++Order) {
    MBinnerHEALPix Binner(static_cast<int>(Order));
    const unsigned int NBins = Binner.GetNBins();
    const MString Name = MString("order ") + Order;

    vector<MVector> Centers = Binner.GetAllBinCenters();
    Passed = EvaluateSize("GetAllBinCenters()", Name, "There is one center vector per bin", Centers.size(), NBins) && Passed;

    unsigned int WrongBins = 0;
    unsigned int WrongCenters = 0;
    unsigned int NotUnit = 0;
    MVector Sum(0.0, 0.0, 0.0);
    for (unsigned int b = 0; b < NBins; ++b) {
      vector<double> Center = Binner.GetBinCenters(b);
      if (Center.size() != 2) {
        ++WrongCenters;
        continue;
      }
      if (Binner.FindBin(Center[0], Center[1]) != b) {
        ++WrongBins;
      }
      if (b < Centers.size()) {
        if (fabs(Centers[b].Mag() - 1.0) > 1e-9) {
          ++NotUnit;
        }
        Sum += Centers[b];
      }
    }
    Passed = Evaluate("GetBinCenters()", Name, "Every bin center has two coordinates (number of others)", WrongCenters, 0U) && Passed;
    Passed = Evaluate("FindBin()", Name, "Every bin center is found in its own bin (number of wrong bins)", WrongBins, 0U) && Passed;
    Passed = Evaluate("GetAllBinCenters()", Name, "All center vectors have length one (number of others)", NotUnit, 0U) && Passed;
    // The pixels are symmetric around the origin: the sum of all centers is zero
    Passed = EvaluateNear("GetAllBinCenters()", Name + " sum x", "The center vectors sum up to zero (x)", Sum.X(), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetAllBinCenters()", Name + " sum y", "The center vectors sum up to zero (y)", Sum.Y(), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetAllBinCenters()", Name + " sum z", "The center vectors sum up to zero (z)", Sum.Z(), 0.0, 1e-9) && Passed;

    // All pixels have the same area 4 pi / NBins: a regular grid in cos(theta) and phi has to give counts
    // which are close to the same for all pixels
    const unsigned int NZ = 200;
    const unsigned int NPhi = 400;
    vector<unsigned int> Counts(NBins, 0);
    unsigned int OutOfRange = 0;
    for (unsigned int z = 0; z < NZ; ++z) {
      double CosTheta = -1.0 + (z + 0.5) * 2.0 / NZ;
      for (unsigned int p = 0; p < NPhi; ++p) {
        double Phi = (p + 0.5) * 2*c_Pi / NPhi;
        const unsigned int Bin = Binner.FindBin(acos(CosTheta), Phi);
        if (Bin < NBins) {
          ++Counts[Bin];
        } else {
          ++OutOfRange;
        }
      }
    }
    unsigned int MinCount = Counts[0];
    unsigned int MaxCount = Counts[0];
    for (unsigned int b = 0; b < NBins; ++b) {
      if (Counts[b] < MinCount) {
        MinCount = Counts[b];
      }
      if (Counts[b] > MaxCount) {
        MaxCount = Counts[b];
      }
    }
    // Expected: NZ*NPhi/NBins = 80000/NBins samples per pixel (order 3: 104), the discretization of the grid gives a spread
    const double Mean = static_cast<double>(NZ*NPhi) / NBins;
    Passed = Evaluate("FindBin()", Name + " range", "All samples of the uniform grid are in a valid bin (number of others)", OutOfRange, 0U) && Passed;
    Passed = EvaluateTrue("FindBin()", Name + " equal area", "All pixels get a similar number of samples of a uniform grid on the sphere (within 30%)", MinCount >= 0.7*Mean && MaxCount <= 1.3*Mean) && Passed;
  }

  // Limits
  {
    MBinnerHEALPix Binner(2);
    Passed = EvaluateSize("GetMinima()", "order 2", "There are two minima", Binner.GetMinima().size(), 2U) && Passed;
    Passed = EvaluateNear("GetMinima()", "theta", "The minimum of theta is 0 degrees", Binner.GetMinima()[0], 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMinima()", "phi", "The minimum of phi is 0 degrees", Binner.GetMinima()[1], 0.0, 1e-12) && Passed;
    Passed = EvaluateSize("GetMaxima()", "order 2", "There are two maxima", Binner.GetMaxima().size(), 2U) && Passed;
    Passed = EvaluateNear("GetMaxima()", "theta", "The maximum of theta is 180 degrees", Binner.GetMaxima()[0], 180.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMaxima()", "phi", "The maximum of phi is 360 degrees", Binner.GetMaxima()[1], 360.0, 1e-12) && Passed;
  }

  // Drawing edges: 0.5 degree resolution (or finer for finer pixels), covering the whole sphere
  {
    MBinnerHEALPix Binner(0);
    vector<vector<double>> Edges = Binner.GetDrawingAxisBinEdges();
    Passed = EvaluateSize("GetDrawingAxisBinEdges()", "order 0", "There are two drawing axes: phi and theta", Edges.size(), 2U) && Passed;
    if (Edges.size() == 2) {
      // 360 degrees / 0.5 degrees = 720 phi bins = 721 edges, 180 / 0.5 = 360 theta bins = 361 edges
      Passed = EvaluateSize("GetDrawingAxisBinEdges()", "order 0 phi", "The phi axis has 720 bins of 0.5 degrees (721 edges)", Edges[0].size(), 721U) && Passed;
      Passed = EvaluateSize("GetDrawingAxisBinEdges()", "order 0 theta", "The theta axis has 360 bins of 0.5 degrees (361 edges)", Edges[1].size(), 361U) && Passed;
      if (Edges[0].size() == 721 && Edges[1].size() == 361) {
        Passed = EvaluateNear("GetDrawingAxisBinEdges()", "order 0 phi start", "The phi axis starts at 0", Edges[0].front(), 0.0, 1e-12) && Passed;
        Passed = EvaluateNear("GetDrawingAxisBinEdges()", "order 0 phi stop", "The phi axis ends at 2 pi", Edges[0].back(), 2*c_Pi, 1e-12) && Passed;
        Passed = EvaluateNear("GetDrawingAxisBinEdges()", "order 0 theta start", "The theta axis starts at 0", Edges[1].front(), 0.0, 1e-12) && Passed;
        Passed = EvaluateNear("GetDrawingAxisBinEdges()", "order 0 theta stop", "The theta axis ends at pi", Edges[1].back(), c_Pi, 1e-12) && Passed;
      }
    }
  }

  // Writing
  {
    MBinnerHEALPix Binner(2);
    ostringstream Out;
    Binner.Write("Sky", Out);
    Passed = Evaluate("Write()", "order 2", "The axis is written as a 2D HEALPix axis in the RING scheme", MString(Out.str()), MString("# Axis name\nAN \"Sky\"\n# Axis type\nAT 2D HEALPix\n# Axis data\nAD 2 RING\n")) && Passed;
  }

  // Equality
  {
    MBinnerHEALPix Order2(2);
    MBinnerHEALPix Order2Again(2);
    MBinnerHEALPix Order3(3);
    MBinnerHEALPix One(-1);
    MBinnerHEALPix MoreNegative(-5);
    Passed = EvaluateTrue("operator==", "same order", "Binners of the same order are equal", Order2 == Order2Again) && Passed;
    Passed = EvaluateFalse("operator!=", "same order", "Binners of the same order are not unequal", Order2 != Order2Again) && Passed;
    Passed = EvaluateFalse("operator==", "different order", "Binners of different orders are not equal", Order2 == Order3) && Passed;
    Passed = EvaluateTrue("operator!=", "different order", "Binners of different orders are unequal", Order2 != Order3) && Passed;
    Passed = EvaluateTrue("operator==", "negative orders", "All negative orders mean one bin and are equal", One == MoreNegative) && Passed;
    MBinnerFISBEL Fisbel(12);
    Passed = EvaluateFalse("operator==", "other type", "A HEALPix binner is never equal to a FISBEL binner, even with the same number of bins", MBinnerHEALPix(0) == Fisbel) && Passed;
  }

  // Invalid arguments
  {
    MBinnerHEALPix Binner(1);
    Passed = EvaluateException<MExceptionParameterOutOfRange>("FindBin()", "theta below 0", "A negative theta is rejected (as for the FISBEL binner)", [&](){ Binner.FindBin(-0.1, 0.0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("FindBin()", "theta above pi", "A theta above pi is rejected (as for the FISBEL binner)", [&](){ Binner.FindBin(c_Pi + 0.1, 0.0); }) && Passed;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBinCenters()", "bin too large", "A bin number which is not smaller than the number of bins is rejected", [&](){ Binner.GetBinCenters(Binner.GetNBins()); }) && Passed;
    // The HEALPix library (Healpix_Base) uses 32-bit pixel numbers and supports orders up to 13 (12 * 4^13 = 805306368 pixels)
    Passed = EvaluateNear("GetNBins()", "order 13", "The finest supported order 13 has 12 * 4^13 bins", MBinnerHEALPix(13).GetNBins(), 805306368.0, 0.5) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("MBinnerHEALPix()", "order 14", "An order which the HEALPix library does not support is rejected with a MEGAlib exception", [&](){ MBinnerHEALPix TooFine(14); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("MBinnerHEALPix()", "order 15", "An order whose number of bins does not fit into 32 bits is rejected with a MEGAlib exception", [&](){ MBinnerHEALPix TooFine(15); }) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinnerHEALPix Test;
  return Test.Run() == true ? 0 : 1;
}


// UTBinnerHEALPix.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
