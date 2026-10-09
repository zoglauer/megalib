/*
 * UTBinnerFISBEL.cxx
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


// ROOT:
#include <TCanvas.h>
#include <TROOT.h>

// MEGAlib:
#include "MBinnerFISBEL.h"
#include "MVector.h"
#include "MExceptions.h"
#include "MUnitTest.h"


//! Unit test class for MBinnerFISBEL
class UTBinnerFISBEL : public MUnitTest
{
public:
  UTBinnerFISBEL() : MUnitTest("UTBinnerFISBEL") {}
  virtual ~UTBinnerFISBEL() {}

  virtual bool Run();

private:
  //! Return the current number of ROOT canvases
  static int GetCanvasCount();
  //! Delete canvases until the requested count is reached
  static void CleanupCanvases(int TargetCount);
};


////////////////////////////////////////////////////////////////////////////////


bool UTBinnerFISBEL::Run()
{
  bool Passed = true;

  MBinnerFISBEL Default;
  Passed = Evaluate("GetNBins()", "default constructor", "The default FISBEL binner starts with zero representative bins", Default.GetNBins(), 0U) && Passed;

  MBinnerFISBEL Single(1);
  Passed = Evaluate("GetNBins()", "single bin", "The one-bin FISBEL binner stores the representative number of bins", Single.GetNBins(), 1U) && Passed;
  Passed = EvaluateNear("GetLongitudeShift()", "single bin", "The one-bin FISBEL binner starts with zero representative longitude shift", Single.GetLongitudeShift(), 0.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetLongitudeBins()", "single bin", "The one-bin FISBEL binner stores one representative longitude collar", Single.GetLongitudeBins().size(), 1UL) && Passed;
  Passed = Evaluate("GetLongitudeBins()", "single bin", "The one-bin FISBEL binner stores one representative longitude bin in its collar", Single.GetLongitudeBins()[0], 1U) && Passed;
  Passed = EvaluateSize("GetLatitudeBinEdges()", "single bin", "The one-bin FISBEL binner stores two representative latitude edges", Single.GetLatitudeBinEdges().size(), 2UL) && Passed;
  Passed = EvaluateNear("GetLatitudeBinEdges()", "first edge", "The one-bin FISBEL binner stores the representative first latitude edge in radians", Single.GetLatitudeBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetLatitudeBinEdges()", "last edge", "The one-bin FISBEL binner stores the representative last latitude edge in radians", Single.GetLatitudeBinEdges()[1], c_Pi, 1e-12) && Passed;
  Passed = Evaluate("FindBin()", "north pole", "The one-bin FISBEL binner maps the representative north pole into its only bin", Single.FindBin(0.0, 0.0), 0U) && Passed;
  Passed = Evaluate("FindBin()", "wrapped longitude", "The one-bin FISBEL binner maps wrapped representative longitudes into its only bin", Single.FindBin(0.5*c_Pi, 5.0*c_Pi), 0U) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "single bin theta", "The one-bin FISBEL binner returns the representative theta center in radians", Single.GetBinCenters(0)[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "single bin phi", "The one-bin FISBEL binner returns the representative phi center in radians", Single.GetBinCenters(0)[1], 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetAllBinCenters()", "single bin count", "The one-bin FISBEL binner returns one representative vector center", Single.GetAllBinCenters().size(), 1UL) && Passed;
  Passed = EvaluateNear("GetAllBinCenters()", "single bin z", "The one-bin FISBEL binner returns the representative north-pole vector", Single.GetAllBinCenters()[0].Z(), 1.0, 1e-12) && Passed;
  Passed = Evaluate("GetDrawingAxisBinEdges()", "single bin axes", "The one-bin FISBEL binner returns two representative drawing-axis vectors", Single.GetDrawingAxisBinEdges().size(), 2UL) && Passed;
  vector<vector<double>> SingleAxes = Single.GetDrawingAxisBinEdges();
  if (SingleAxes.size() == 2) {
    Passed = EvaluateSize("GetDrawingAxisBinEdges()", "single bin longitude", "The longitude axis of the one-bin FISBEL binner has the two edges 0 and 2*pi", SingleAxes[0].size(), 2UL) && Passed;
    if (SingleAxes[0].size() == 2) {
      Passed = EvaluateNear("GetDrawingAxisBinEdges()", "single bin longitude 0", "The first longitude edge of the one-bin FISBEL binner is 0", SingleAxes[0][0], 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("GetDrawingAxisBinEdges()", "single bin longitude 1", "The last longitude edge of the one-bin FISBEL binner is 2*pi", SingleAxes[0][1], 2.0*c_Pi, 1e-12) && Passed;
    }
    Passed = EvaluateSize("GetDrawingAxisBinEdges()", "single bin latitude", "The latitude axis of the one-bin FISBEL binner has the two edges 0 and pi", SingleAxes[1].size(), 2UL) && Passed;
  }
  Passed = EvaluateException<MExceptionParameterOutOfRange>("FindBin()", "theta underflow", "FindBin rejects representative theta underflow", [&](){ Single.FindBin(-0.1, 0.0); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBinCenters()", "out of bounds", "GetBinCenters rejects representative out-of-bounds bin requests", [&](){ Single.GetBinCenters(1); }) && Passed;

  MBinnerFISBEL Shifted;
  Shifted.Create(1, 30.0*c_Rad);
  Passed = EvaluateNear("Create()", "shifted longitude", "Create stores the representative longitude shift in radians", Shifted.GetLongitudeShift(), 30.0*c_Rad, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "shifted single bin phi", "A shifted one-bin FISBEL binner returns the representative shifted phi center", Shifted.GetBinCenters(0)[1], 30.0*c_Rad, 1e-12) && Passed;

  MBinnerFISBEL FourBins;
  FourBins.Create(4);
  Passed = Evaluate("GetNBins()", "four bins", "Create stores the representative four-bin count", FourBins.GetNBins(), 4U) && Passed;
  Passed = Evaluate("GetAllBinCenters()", "four bins", "The four-bin FISBEL binner returns one representative vector per bin", FourBins.GetAllBinCenters().size(), 4UL) && Passed;
  // Expected: area per bin 4*pi/4 = pi, square length sqrt(pi), collars int(pi/sqrt(pi) - 1 + 0.5) + 2 = 3 with 1, 2, 1 bins, edges 0, acos(1 - 2/4) = pi/3, pi - pi/3, pi
  vector<unsigned int> FourBinsLongitudeBins = FourBins.GetLongitudeBins();
  Passed = EvaluateSize("GetLongitudeBins()", "four bins", "The four-bin FISBEL binner has three collars", FourBinsLongitudeBins.size(), 3UL) && Passed;
  if (FourBinsLongitudeBins.size() == 3) {
    Passed = Evaluate("GetLongitudeBins()", "four bins north", "The four-bin FISBEL binner has one bin in the northern cap", FourBinsLongitudeBins[0], 1U) && Passed;
    Passed = Evaluate("GetLongitudeBins()", "four bins equator", "The four-bin FISBEL binner has two bins in the equatorial collar", FourBinsLongitudeBins[1], 2U) && Passed;
    Passed = Evaluate("GetLongitudeBins()", "four bins south", "The four-bin FISBEL binner has one bin in the southern cap", FourBinsLongitudeBins[2], 1U) && Passed;
  }
  vector<double> FourBinsLatitudeEdges = FourBins.GetLatitudeBinEdges();
  Passed = EvaluateSize("GetLatitudeBinEdges()", "four bins", "The four-bin FISBEL binner has four latitude edges", FourBinsLatitudeEdges.size(), 4UL) && Passed;
  if (FourBinsLatitudeEdges.size() == 4) {
    Passed = EvaluateNear("GetLatitudeBinEdges()", "four bins edge 0", "The first latitude edge of the four-bin FISBEL binner is the north pole", FourBinsLatitudeEdges[0], 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetLatitudeBinEdges()", "four bins edge 1", "The second latitude edge of the four-bin FISBEL binner is acos(0.5) = pi/3", FourBinsLatitudeEdges[1], c_Pi/3.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetLatitudeBinEdges()", "four bins edge 2", "The third latitude edge of the four-bin FISBEL binner is 2*pi/3", FourBinsLatitudeEdges[2], 2.0*c_Pi/3.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetLatitudeBinEdges()", "four bins edge 3", "The last latitude edge of the four-bin FISBEL binner is the south pole", FourBinsLatitudeEdges[3], c_Pi, 1e-12) && Passed;
  }

  // Expected centers: poles, equatorial bins at theta = pi/2 with phi = (k + 0.5)*2*pi/2
  const double FourBinsTheta[4] = { 0.0, 0.5*c_Pi, 0.5*c_Pi, c_Pi };
  const double FourBinsPhi[4] = { 0.0, 0.5*c_Pi, 1.5*c_Pi, 0.0 };
  for (unsigned int b = 0; b < 4; ++b) {
    vector<double> Center = FourBins.GetBinCenters(b);
    Passed = EvaluateSize("GetBinCenters()", "four bins center " + to_string(b), "GetBinCenters returns theta and phi", Center.size(), 2UL) && Passed;
    if (Center.size() == 2) {
      Passed = EvaluateNear("GetBinCenters()", "four bins theta " + to_string(b), "The theta center of the four-bin FISBEL binner is the expected latitude center", Center[0], FourBinsTheta[b], 1e-12) && Passed;
      Passed = EvaluateNear("GetBinCenters()", "four bins phi " + to_string(b), "The phi center of the four-bin FISBEL binner is the expected longitude center", Center[1], FourBinsPhi[b], 1e-12) && Passed;
    }
  }

  // Expected vectors: (sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta))
  vector<MVector> FourBinsVectors = FourBins.GetAllBinCenters();
  if (FourBinsVectors.size() == 4) {
    Passed = EvaluateVectorNear("GetAllBinCenters()", "four bins north", "The northern cap center is the +z axis", FourBinsVectors[0], MVector(0.0, 0.0, 1.0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("GetAllBinCenters()", "four bins east", "The first equatorial center is the +y axis", FourBinsVectors[1], MVector(0.0, 1.0, 0.0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("GetAllBinCenters()", "four bins west", "The second equatorial center is the -y axis", FourBinsVectors[2], MVector(0.0, -1.0, 0.0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("GetAllBinCenters()", "four bins south", "The southern cap center is the -z axis", FourBinsVectors[3], MVector(0.0, 0.0, -1.0), 1e-12) && Passed;
  }

  // Check the drawing axes - longitude edges 0, pi, 2*pi, and the latitude edges:
  vector<vector<double>> FourBinsAxes = FourBins.GetDrawingAxisBinEdges();
  Passed = EvaluateSize("GetDrawingAxisBinEdges()", "four bins", "The drawing axes consist of a longitude and a latitude axis", FourBinsAxes.size(), 2UL) && Passed;
  if (FourBinsAxes.size() == 2) {
    Passed = EvaluateSize("GetDrawingAxisBinEdges()", "four bins longitude", "The longitude axis of the four-bin FISBEL binner has three edges", FourBinsAxes[0].size(), 3UL) && Passed;
    if (FourBinsAxes[0].size() == 3) {
      Passed = EvaluateNear("GetDrawingAxisBinEdges()", "four bins longitude 0", "The first longitude edge is the longitude shift", FourBinsAxes[0][0], 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("GetDrawingAxisBinEdges()", "four bins longitude 1", "The second longitude edge splits the equatorial collar at pi", FourBinsAxes[0][1], c_Pi, 1e-12) && Passed;
      Passed = EvaluateNear("GetDrawingAxisBinEdges()", "four bins longitude 2", "The last longitude edge is the longitude shift plus 2*pi", FourBinsAxes[0][2], 2.0*c_Pi, 1e-12) && Passed;
    }
    Passed = EvaluateSize("GetDrawingAxisBinEdges()", "four bins latitude", "The latitude axis of the four-bin FISBEL binner has four edges", FourBinsAxes[1].size(), 4UL) && Passed;
  }

  // Check the bin search at the poles, the collars, and both sides of the longitude split at phi = pi:
  Passed = Evaluate("FindBin()", "four bins north pole", "The north pole is in bin 0", FourBins.FindBin(0.0, 1.0), 0U) && Passed;
  Passed = Evaluate("FindBin()", "four bins south pole", "The south pole is in the last bin", FourBins.FindBin(c_Pi, 1.0), 3U) && Passed;
  Passed = Evaluate("FindBin()", "four bins north cap", "A direction just inside the northern cap edge pi/3 is in bin 0", FourBins.FindBin(c_Pi/3.0 - 1e-6, 4.0), 0U) && Passed;
  Passed = Evaluate("FindBin()", "four bins first equatorial bin below", "A direction just outside the northern cap edge with phi below pi is in bin 1", FourBins.FindBin(c_Pi/3.0 + 1e-6, 1.0), 1U) && Passed;
  Passed = Evaluate("FindBin()", "four bins second equatorial bin above", "A direction just outside the northern cap edge with phi above pi is in bin 2", FourBins.FindBin(c_Pi/3.0 + 1e-6, 4.0), 2U) && Passed;
  Passed = Evaluate("FindBin()", "four bins south cap", "A direction just inside the southern cap edge 2*pi/3 is in bin 3", FourBins.FindBin(2.0*c_Pi/3.0 + 1e-6, 4.0), 3U) && Passed;
  Passed = Evaluate("FindBin()", "four bins last equatorial", "A direction just outside the southern cap edge is in the equatorial collar", FourBins.FindBin(2.0*c_Pi/3.0 - 1e-6, 4.0), 2U) && Passed;
  Passed = Evaluate("FindBin()", "four bins phi inside", "Phi just below pi is in the first equatorial bin", FourBins.FindBin(0.5*c_Pi, c_Pi - 1e-9), 1U) && Passed;
  Passed = Evaluate("FindBin()", "four bins phi outside", "Phi just above pi is in the second equatorial bin", FourBins.FindBin(0.5*c_Pi, c_Pi + 1e-9), 2U) && Passed;
  Passed = Evaluate("FindBin()", "four bins phi wrap up", "Phi 0.25*pi + 2*pi is wrapped into the first equatorial bin", FourBins.FindBin(0.5*c_Pi, 2.25*c_Pi), 1U) && Passed;
  Passed = Evaluate("FindBin()", "four bins phi wrap down", "Phi -0.75*pi is wrapped to 1.25*pi in the second equatorial bin", FourBins.FindBin(0.5*c_Pi, -0.75*c_Pi), 2U) && Passed;
  Passed = Evaluate("FindBin()", "four bins phi 2 pi", "Phi 2*pi is wrapped to 0 in the first equatorial bin", FourBins.FindBin(0.5*c_Pi, 2.0*c_Pi), 1U) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("FindBin()", "theta overflow", "FindBin rejects theta above pi", [&](){ FourBins.FindBin(c_Pi + 0.1, 0.0); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBinCenters()", "four bins out of bounds", "GetBinCenters rejects the bin one past the last bin", [&](){ FourBins.GetBinCenters(4); }) && Passed;

  // Check the invariants for several bin numbers - collar bins sum to N, symmetric latitude edges, area 4*pi/N per bin, centers found in their bin:
  for (unsigned int NumberOfBins : { 2U, 3U, 10U, 41U, 100U, 1000U }) {
    const MString Input = "N = " + to_string(NumberOfBins);
    MBinnerFISBEL Invariant(NumberOfBins);
    vector<unsigned int> Collars = Invariant.GetLongitudeBins();
    vector<double> Edges = Invariant.GetLatitudeBinEdges();
    Passed = EvaluateSize("GetLatitudeBinEdges()", Input, "There is one more latitude edge than collars", Edges.size(), Collars.size() + 1) && Passed;
    if (Edges.size() != Collars.size() + 1) continue;

    unsigned int Sum = 0;
    double MaximumAreaDeviation = 0.0;
    double MaximumAsymmetry = 0.0;
    for (unsigned int c = 0; c < Collars.size(); ++c) {
      Sum += Collars[c];
      double Area = 2.0*c_Pi*(cos(Edges[c]) - cos(Edges[c+1]))/Collars[c];
      MaximumAreaDeviation = GetMaximum(MaximumAreaDeviation, fabs(Area/(4.0*c_Pi/NumberOfBins) - 1.0));
    }
    for (unsigned int e = 0; e < Edges.size(); ++e) {
      MaximumAsymmetry = GetMaximum(MaximumAsymmetry, fabs(Edges[e] + Edges[Edges.size() - 1 - e] - c_Pi));
    }
    Passed = Evaluate("Create()", Input, "The bins of all collars add up to the number of bins", Sum, NumberOfBins) && Passed;
    Passed = EvaluateNear("Create()", Input, "All bins have the same area (maximum relative deviation)", MaximumAreaDeviation, 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Create()", Input, "The latitude edges are symmetric around the equator (maximum deviation)", MaximumAsymmetry, 0.0, 1e-12) && Passed;

    unsigned int WrongBins = 0;
    for (unsigned int b = 0; b < NumberOfBins; ++b) {
      vector<double> Center = Invariant.GetBinCenters(b);
      if (Invariant.FindBin(Center[0], Center[1]) != b) ++WrongBins;
    }
    Passed = Evaluate("FindBin()", Input, "FindBin maps every bin center into its own bin (number of wrong bins)", WrongBins, 0U) && Passed;
  }

  vector<unsigned int> LongitudeBins = FourBins.GetLongitudeBins();
  vector<double> LatitudeEdges = FourBins.GetLatitudeBinEdges();
  MBinnerFISBEL Copied;
  Copied.Set(LongitudeBins, LatitudeEdges, FourBins.GetNBins(), FourBins.GetLongitudeShift());
  Passed = Evaluate("operator==()", "copied binning", "Set reproduces the representative FISBEL binning exactly", Copied == FourBins, true) && Passed;
  Passed = Evaluate("operator!=", "different longitude shift", "FISBEL binners with a different representative longitude shift compare unequal", Shifted != Single, true) && Passed;

  MBinnerFISBEL ShiftedCopied;
  ShiftedCopied.Set(LongitudeBins, LatitudeEdges, FourBins.GetNBins(), 30.0*c_Rad);
  Passed = EvaluateNear("Set()", "shifted longitude", "Set stores the representative longitude shift exactly", ShiftedCopied.GetLongitudeShift(), 30.0*c_Rad, 1e-12) && Passed;

  {
    bool WasBatch = gROOT->IsBatch();
    gROOT->SetBatch(true);
    int BeforeCanvases = GetCanvasCount();
    DisableDefaultStreams();
    Single.View(vector<double>{1.0});
    EnableDefaultStreams();
    Passed = Evaluate("View()", "representative display", "View creates a representative ROOT canvas", GetCanvasCount(), BeforeCanvases + 1) && Passed;
    CleanupCanvases(BeforeCanvases);
    gROOT->SetBatch(WasBatch);
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTBinnerFISBEL::GetCanvasCount()
{
  return gROOT != 0 && gROOT->GetListOfCanvases() != 0 ? gROOT->GetListOfCanvases()->GetSize() : 0;
}


////////////////////////////////////////////////////////////////////////////////


void UTBinnerFISBEL::CleanupCanvases(int TargetCount)
{
  while (gROOT != 0 && gROOT->GetListOfCanvases() != 0 && gROOT->GetListOfCanvases()->GetSize() > TargetCount) {
    TCanvas* Canvas = dynamic_cast<TCanvas*>(gROOT->GetListOfCanvases()->Last());
    if (Canvas == 0) {
      break;
    }
    delete Canvas;
  }
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinnerFISBEL Test;
  return Test.Run() == true ? 0 : 1;
}
