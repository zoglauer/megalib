/*
 * UTResponseMatrixAxisSpheric.cxx
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

// MEGAlib:
#include "MExceptions.h"
#include "MResponseMatrixAxisSpheric.h"
#include "MResponseMatrixON.h"
#include "MGlobal.h"
#include "MUnitTest.h"


//! Unit test class for MResponseMatrixAxisSpheric
class UTResponseMatrixAxisSpheric : public MUnitTest
{
public:
  UTResponseMatrixAxisSpheric() : MUnitTest("UTResponseMatrixAxisSpheric") {}
  virtual ~UTResponseMatrixAxisSpheric() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTResponseMatrixAxisSpheric::Run()
{
  bool Passed = true;

  MResponseMatrixAxisSpheric Default("Theta", "Phi");
  Passed = Evaluate("GetDimension()", "default constructor", "The spherical axis has two dimensions", Default.GetDimension(), 2U) && Passed;
  Passed = Evaluate("GetNumberOfBins()", "default constructor", "The default spherical axis starts with one FISBEL bin", Default.GetNumberOfBins(), 0UL) && Passed;
  Passed = Evaluate("Has1DBinEdges()", "default constructor", "The spherical axis reports that it has no one-dimensional bin edges", Default.Has1DBinEdges(), false) && Passed;
  Passed = Evaluate("Get1DBinEdges()", "default constructor", "The spherical axis returns an empty representative one-dimensional bin-edge vector", Default.Get1DBinEdges().size(), 0UL) && Passed;
  Passed = EvaluateSize("GetNames()", "default constructor", "The spherical axis stores two representative axis names", Default.GetNames().size(), 2UL) && Passed;
  Passed = Evaluate("GetNames()", "default constructor theta", "The spherical axis stores the representative theta name", Default.GetNames()[0], MString("Theta")) && Passed;
  Passed = Evaluate("GetNames()", "default constructor phi", "The spherical axis stores the representative phi name", Default.GetNames()[1], MString("Phi")) && Passed;
  Passed = Evaluate("GetAllBinCenters()", "default constructor", "The default spherical axis keeps the representative bin-center count consistent with the reported number of bins", Default.GetAllBinCenters().size(), Default.GetNumberOfBins()) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetArea()", "default constructor", "A spherical axis with zero reported bins rejects representative area queries", [&](){ Default.GetArea(0); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBinCenters()", "default constructor", "A spherical axis with zero reported bins rejects representative bin-center queries", [&](){ Default.GetBinCenters(0); }) && Passed;
  Passed = Evaluate("InRange()", "representative theta lower bound", "The spherical axis accepts the representative lower theta boundary", Default.InRange(0.0, 0.0), true) && Passed;
  Passed = Evaluate("InRange()", "representative theta upper bound", "The spherical axis accepts the representative upper theta boundary", Default.InRange(180.0, 0.0), true) && Passed;
  Passed = Evaluate("InRange()", "theta underflow", "The spherical axis rejects representative theta underflow", Default.InRange(-1.0, 0.0), false) && Passed;
  Passed = Evaluate("InRange()", "theta overflow", "The spherical axis rejects representative theta overflow", Default.InRange(181.0, 0.0), false) && Passed;
  Passed = Evaluate("InRange()", "phi ignored", "The spherical axis ignores the representative phi value when checking range", Default.InRange(90.0, 720.0), true) && Passed;

  Default.SetFISBELByNumberOfBins(1);
  Passed = Evaluate("GetNumberOfBins()", "SetFISBEL(1)", "SetFISBEL stores the representative number of spherical bins", Default.GetNumberOfBins(), 1UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "north pole", "The single representative spherical bin contains the north pole", Default.GetAxisBin(0.0, 0.0), 0UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "south pole", "The single representative spherical bin contains the south pole", Default.GetAxisBin(180.0, 270.0), 0UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "phi wrapping", "The single representative spherical bin accepts wrapped phi values", Default.GetAxisBin(45.0, 720.0), 0UL) && Passed;
  Passed = EvaluateNear("GetArea()", "single bin", "GetArea returns the representative full-sphere area in square degrees", Default.GetArea(0), 4.0*c_Pi*c_Deg*c_Deg, 1e-9) && Passed;
  Passed = Evaluate("GetDrawingAxisBinEdges()", "single bin dimension", "GetDrawingAxisBinEdges returns representative theta/phi drawing axes", Default.GetDrawingAxisBinEdges().size(), 2UL) && Passed;
  Passed = EvaluateSize("GetDrawingAxisBinEdges()", "single bin theta edge count", "GetDrawingAxisBinEdges returns the representative theta drawing-edge count", Default.GetDrawingAxisBinEdges()[0].size(), 2UL) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "single bin theta first edge", "GetDrawingAxisBinEdges returns the representative first theta edge in degrees", Default.GetDrawingAxisBinEdges()[0][0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "single bin theta second edge", "GetDrawingAxisBinEdges returns the representative second theta edge in degrees", Default.GetDrawingAxisBinEdges()[0][1], 180.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetDrawingAxisBinEdges()", "single bin phi edge count", "GetDrawingAxisBinEdges returns the representative phi drawing-edge count", Default.GetDrawingAxisBinEdges()[1].size(), 2UL) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "single bin phi first edge", "GetDrawingAxisBinEdges returns the representative first phi edge in degrees", Default.GetDrawingAxisBinEdges()[1][0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "single bin phi second edge", "GetDrawingAxisBinEdges returns the representative second phi edge in degrees", Default.GetDrawingAxisBinEdges()[1][1], 360.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetMinima()", "single bin", "GetMinima returns the representative theta/phi minima", Default.GetMinima().size(), 2UL) && Passed;
  Passed = EvaluateNear("GetMinima()", "single bin theta", "GetMinima returns the representative theta minimum", Default.GetMinima()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinima()", "single bin phi", "GetMinima returns the representative phi minimum", Default.GetMinima()[1], 0.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetMaxima()", "single bin", "GetMaxima returns the representative theta/phi maxima", Default.GetMaxima().size(), 2UL) && Passed;
  Passed = EvaluateNear("GetMaxima()", "single bin theta", "GetMaxima returns the representative theta maximum", Default.GetMaxima()[0], 180.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxima()", "single bin phi", "GetMaxima returns the representative phi maximum", Default.GetMaxima()[1], 360.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "single bin theta", "GetBinCenters returns the representative theta center in degrees", Default.GetBinCenters(0)[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "single bin phi", "GetBinCenters returns the representative phi center in degrees", Default.GetBinCenters(0)[1], 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetAllBinCenters()", "single bin count", "GetAllBinCenters returns one representative unit vector per spherical bin", Default.GetAllBinCenters().size(), 1UL) && Passed;
  Passed = EvaluateNear("GetAllBinCenters()", "single bin x", "GetAllBinCenters returns the representative north-pole unit vector x component", Default.GetAllBinCenters()[0].X(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAllBinCenters()", "single bin y", "GetAllBinCenters returns the representative north-pole unit vector y component", Default.GetAllBinCenters()[0].Y(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAllBinCenters()", "single bin z", "GetAllBinCenters returns the representative north-pole unit vector z component", Default.GetAllBinCenters()[0].Z(), 1.0, 1e-12) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("GetAxisBin()", "theta underflow", "GetAxisBin rejects representative theta underflow", [&](){ Default.GetAxisBin(-1.0, 0.0); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetArea()", "out of bounds", "GetArea throws on representative out-of-bounds spherical-bin access", [&](){ Default.GetArea(1); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBinCenters()", "out of bounds", "GetBinCenters throws on representative out-of-bounds spherical-bin access", [&](){ Default.GetBinCenters(1); }) && Passed;

  MResponseMatrixAxisSpheric Shifted("Theta", "Phi");
  Shifted.SetFISBELByNumberOfBins(1, 30.0);
  Passed = EvaluateNear("GetMinima()", "longitude shift theta", "GetMinima keeps the representative theta minimum under a longitude shift", Shifted.GetMinima()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinima()", "longitude shift phi", "GetMinima reflects the representative longitude shift in degrees", Shifted.GetMinima()[1], 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxima()", "longitude shift theta", "GetMaxima keeps the representative theta maximum under a longitude shift", Shifted.GetMaxima()[0], 180.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxima()", "longitude shift phi", "GetMaxima reflects the representative longitude shift in degrees", Shifted.GetMaxima()[1], 390.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "longitude shift phi first edge", "GetDrawingAxisBinEdges reflects the representative shifted first phi edge in degrees", Shifted.GetDrawingAxisBinEdges()[1][0], 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "longitude shift phi second edge", "GetDrawingAxisBinEdges reflects the representative shifted second phi edge in degrees", Shifted.GetDrawingAxisBinEdges()[1][1], 390.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "longitude shift phi", "GetBinCenters reflects the representative shifted phi center in degrees", Shifted.GetBinCenters(0)[1], 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAllBinCenters()", "longitude shift z", "GetAllBinCenters keeps the representative shifted north-pole vector along z", Shifted.GetAllBinCenters()[0].Z(), 1.0, 1e-12) && Passed;

  MResponseMatrixAxisSpheric FourBins("Theta", "Phi");
  FourBins.SetFISBELByNumberOfBins(4);
  Passed = Evaluate("GetNumberOfBins()", "SetFISBEL(4)", "SetFISBEL stores the representative multi-bin spherical bin count", FourBins.GetNumberOfBins(), 4UL) && Passed;
  Passed = Evaluate("GetAllBinCenters()", "SetFISBEL(4)", "GetAllBinCenters returns one representative vector per spherical bin", FourBins.GetAllBinCenters().size(), 4UL) && Passed;
  // FISBEL(4): bin area pi, 3 collars with 1, 2, 1 bins, theta edges 0, 60, 120, 180 deg, middle collar split at phi 180 deg
  Passed = EvaluateSize("GetDrawingAxisBinEdges()", "SetFISBEL(4) theta count", "GetDrawingAxisBinEdges returns four theta edges for three collars", FourBins.GetDrawingAxisBinEdges()[0].size(), 4UL) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetFISBEL(4) theta edge 0", "GetDrawingAxisBinEdges returns the theta edge 0 of the four-bin axis in degrees", FourBins.GetDrawingAxisBinEdges()[0][0], 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetFISBEL(4) theta edge 1", "GetDrawingAxisBinEdges returns the theta edge 1 of the four-bin axis in degrees", FourBins.GetDrawingAxisBinEdges()[0][1], 60.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetFISBEL(4) theta edge 2", "GetDrawingAxisBinEdges returns the theta edge 2 of the four-bin axis in degrees", FourBins.GetDrawingAxisBinEdges()[0][2], 120.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetFISBEL(4) theta edge 3", "GetDrawingAxisBinEdges returns the theta edge 3 of the four-bin axis in degrees", FourBins.GetDrawingAxisBinEdges()[0][3], 180.0, 1e-9) && Passed;
  Passed = EvaluateSize("GetDrawingAxisBinEdges()", "SetFISBEL(4) phi count", "GetDrawingAxisBinEdges returns three phi edges", FourBins.GetDrawingAxisBinEdges()[1].size(), 3UL) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetFISBEL(4) phi edge 0", "GetDrawingAxisBinEdges returns the phi edge 0 of the four-bin axis in degrees", FourBins.GetDrawingAxisBinEdges()[1][0], 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetFISBEL(4) phi edge 1", "GetDrawingAxisBinEdges returns the phi edge 1 of the four-bin axis in degrees", FourBins.GetDrawingAxisBinEdges()[1][1], 180.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetFISBEL(4) phi edge 2", "GetDrawingAxisBinEdges returns the phi edge 2 of the four-bin axis in degrees", FourBins.GetDrawingAxisBinEdges()[1][2], 360.0, 1e-9) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetFISBEL(4) north pole", "GetAxisBin maps the representative north pole into the first spherical bin", FourBins.GetAxisBin(0.0, 0.0), 0UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetFISBEL(4) equator", "GetAxisBin maps an equatorial direction at phi 45 degrees into the first bin of the middle collar", FourBins.GetAxisBin(90.0, 45.0), 1UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetFISBEL(4) equator second", "GetAxisBin maps an equatorial direction at phi 225 degrees into the second bin of the middle collar", FourBins.GetAxisBin(90.0, 225.0), 2UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetFISBEL(4) south", "GetAxisBin maps a direction at theta 150 degrees into the last bin", FourBins.GetAxisBin(150.0, 10.0), 3UL) && Passed;

  MResponseMatrixAxisSpheric EqualA("ThetaA", "PhiA");
  EqualA.SetFISBELByNumberOfBins(4, 15.0);
  MResponseMatrixAxisSpheric EqualB("ThetaB", "PhiB");
  EqualB.SetFISBELByNumberOfBins(4, 15.0);
  Passed = Evaluate("operator==()", "same physical axis", "Spherical axes with the same representative physical binning compare equal", EqualA == EqualB, true) && Passed;
  EqualB.SetFISBELByNumberOfBins(4, 0.0);
  Passed = Evaluate("operator!=", "different longitude shift", "Spherical axes with a different representative longitude shift compare unequal", EqualA != EqualB, true) && Passed;

  MResponseMatrixAxisSpheric* Clone = EqualA.Clone();
  Passed = EvaluateTrue("Clone()", "representative clone", "Clone returns a representative heap-allocated spherical axis", Clone != 0) && Passed;
  if (Clone != 0) {
    Passed = Evaluate("operator==()", "representative clone", "A spherical clone compares equal to the representative source axis", *Clone == EqualA, true) && Passed;
    delete Clone;
  }

  ostringstream Stream;
  Shifted.Write(Stream);
  Passed = Evaluate("Write()", "representative text", "Write serializes the representative spherical axis deterministically", MString(Stream.str()), MString("# Axis name\nAN \"Theta\" \"Phi\"\n# Axis type\nAT 2D FISBEL\n# Axis data\nAD 1 30\n")) && Passed;

  ostringstream FourBinStream;
  FourBins.Write(FourBinStream);
  Passed = Evaluate("Write()", "multi-bin text", "Write serializes the representative multi-bin spherical axis deterministically", MString(FourBinStream.str()), MString("# Axis name\nAN \"Theta\" \"Phi\"\n# Axis type\nAT 2D FISBEL\n# Axis data\nAD 4 0\n")) && Passed;

  // HEALPix axis
  {
    MResponseMatrixAxisSpheric Sky("Theta", "Phi");
    Sky.SetHEALPixByOrder(0);
    Passed = Evaluate("GetNumberOfBins()", "HEALPix order 0", "A HEALPix axis of order 0 has 12 bins", Sky.GetNumberOfBins(), 12UL) && Passed;
    // Base pixels of order 0: pixel 4 is the first equatorial pixel (theta = 90 degrees, phi = 0 degrees)
    Passed = Evaluate("GetAxisBin()", "HEALPix equator", "A direction at theta = 90 degrees, phi = 5 degrees is in the first equatorial pixel", Sky.GetAxisBin(90.0, 5.0), 4UL) && Passed;
    Passed = Evaluate("GetAxisBin()", "HEALPix north", "A direction at theta = 5 degrees, phi = 5 degrees is in the first north-cap pixel", Sky.GetAxisBin(5.0, 5.0), 0UL) && Passed;
    // The center of the first pixel is at theta = acos(2/3) = 48.19 degrees, phi = 45 degrees
    Passed = EvaluateNear("GetBinCenters()", "HEALPix pixel 0 theta", "The center of the first pixel is at theta = acos(2/3) in degrees", Sky.GetBinCenters(0)[0], acos(2.0/3.0)/c_Rad, 1e-7) && Passed;
    Passed = EvaluateNear("GetBinCenters()", "HEALPix pixel 0 phi", "The center of the first pixel is at phi = 45 degrees", Sky.GetBinCenters(0)[1], 45.0, 1e-7) && Passed;
    Passed = Evaluate("GetAllBinCenters()", "HEALPix", "There is one center vector per pixel", Sky.GetAllBinCenters().size(), 12UL) && Passed;
    // 4 pi steradian in square degrees / 12 pixels
    Passed = EvaluateNear("GetArea()", "HEALPix", "All pixels have the same area: 4 pi (in square degrees) / 12", Sky.GetArea(3), 4.0*c_Pi*c_Deg*c_Deg/12.0, 1e-6) && Passed;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetArea()", "HEALPix out of bounds", "GetArea throws on a pixel which does not exist", [&](){ Sky.GetArea(12); }) && Passed;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBinCenters()", "HEALPix out of bounds", "GetBinCenters throws on a pixel which does not exist", [&](){ Sky.GetBinCenters(12); }) && Passed;
    Passed = EvaluateNear("GetMinima()", "HEALPix theta", "The minimum of theta is 0 degrees", Sky.GetMinima()[0], 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMinima()", "HEALPix phi", "The minimum of phi is 0 degrees", Sky.GetMinima()[1], 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMaxima()", "HEALPix theta", "The maximum of theta is 180 degrees", Sky.GetMaxima()[0], 180.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMaxima()", "HEALPix phi", "The maximum of phi is 360 degrees", Sky.GetMaxima()[1], 360.0, 1e-12) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("GetAxisBin()", "HEALPix theta underflow", "GetAxisBin rejects a negative theta (as for the FISBEL axis)", [&](){ Sky.GetAxisBin(-1.0, 0.0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("GetAxisBin()", "HEALPix theta overflow", "GetAxisBin rejects a theta above 180 degrees (as for the FISBEL axis)", [&](){ Sky.GetAxisBin(181.0, 0.0); }) && Passed;

    ostringstream Out;
    Sky.Write(Out);
    Passed = Evaluate("Write()", "HEALPix", "A HEALPix axis is written as 2D HEALPix with the order and the ring scheme", MString(Out.str()), MString("# Axis name\nAN \"Theta\" \"Phi\"\n# Axis type\nAT 2D HEALPix\n# Axis data\nAD 0 RING\n")) && Passed;

    MResponseMatrixAxisSpheric* CloneOfSky = Sky.Clone();
    Passed = Evaluate("Clone()", "HEALPix bins", "A clone of a HEALPix axis has the same number of bins", CloneOfSky->GetNumberOfBins(), 12UL) && Passed;
    Passed = Evaluate("Clone()", "HEALPix bin", "A clone of a HEALPix axis finds the same bin", CloneOfSky->GetAxisBin(90.0, 5.0), 4UL) && Passed;
    delete CloneOfSky;

    // Changing the type: FISBEL axis to HEALPix axis
    MResponseMatrixAxisSpheric Changing("Theta", "Phi");
    Changing.SetFISBELByNumberOfBins(4);
    Changing.SetHEALPixByOrder(1);
    Passed = Evaluate("SetHEALPixByOrder()", "after FISBEL", "A FISBEL axis becomes a HEALPix axis of order 1 with 48 bins", Changing.GetNumberOfBins(), 48UL) && Passed;
  }

  // The number of bins is rounded down to the nearest order: 12 * 4^order
  {
    const vector<unsigned long> Requested = { 12, 47, 48, 191, 192, 200, 800 };
    const vector<unsigned long> Expected  = { 12, 12, 48,  48, 192, 192, 768 };
    for (unsigned int i = 0; i < Requested.size(); ++i) {
      MResponseMatrixAxisSpheric Sky("Theta", "Phi");
      Sky.SetHEALPixByNumberOfBins(Requested[i]);
      Passed = Evaluate("SetHEALPixByNumberOfBins()", Requested[i], "The requested number of bins is rounded down to 12 * 4^order", Sky.GetNumberOfBins(), Expected[i]) && Passed;
    }
  }

  // The pixel sizes of the documentation (in degrees) give the documented orders
  {
    const vector<double> PixelSize = { 58.632, 29.316, 14.658, 7.329, 3.664 };
    const vector<unsigned long> Expected = { 12, 48, 192, 768, 3072 };
    for (unsigned int i = 0; i < PixelSize.size(); ++i) {
      MResponseMatrixAxisSpheric Sky("Theta", "Phi");
      Sky.SetHEALPixByPixelSize(PixelSize[i]);
      Passed = Evaluate("SetHEALPixByPixelSize()", PixelSize[i], "The pixel size of the documentation gives the documented order", Sky.GetNumberOfBins(), Expected[i]) && Passed;
    }
  }

  // FISBEL by pixel size: 4 pi / (pixel size)^2 bins, at least one
  {
    MResponseMatrixAxisSpheric Sky("Theta", "Phi");
    // 4 pi sr = 41252.96 square degrees, / 90^2 = 5.09 bins -> 5
    Sky.SetFISBELByPixelSize(90.0);
    Passed = Evaluate("SetFISBELByPixelSize()", "90", "A pixel size of 90 degrees gives 5 bins", Sky.GetNumberOfBins(), 5UL) && Passed;
    Sky.SetFISBELByPixelSize(1000.0);
    Passed = Evaluate("SetFISBELByPixelSize()", "1000", "A pixel size larger than the sky gives one bin", Sky.GetNumberOfBins(), 1UL) && Passed;
  }

  // Invalid sizes and numbers are rejected instead of giving a silent single-bin axis
  {
    MResponseMatrixAxisSpheric Sky("Theta", "Phi");
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetFISBELByPixelSize()", "0", "A pixel size of zero is rejected", [&](){ Sky.SetFISBELByPixelSize(0.0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetHEALPixByPixelSize()", "0", "A pixel size of zero is rejected", [&](){ Sky.SetHEALPixByPixelSize(0.0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetHEALPixByNumberOfBins()", "0", "Zero bins are rejected", [&](){ Sky.SetHEALPixByNumberOfBins(0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetFISBELByPixelSize()", "-1", "A negative pixel size is rejected", [&](){ Sky.SetFISBELByPixelSize(-1.0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetHEALPixByPixelSize()", "-1", "A negative pixel size is rejected", [&](){ Sky.SetHEALPixByPixelSize(-1.0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetFISBELByPixelSize()", "-0.001", "A tiny negative pixel size is rejected", [&](){ Sky.SetFISBELByPixelSize(-0.001); }) && Passed;
  }

  // A rejected setter leaves an already configured axis unchanged
  {
    MResponseMatrixAxisSpheric Sky("Theta", "Phi");
    Sky.SetHEALPixByOrder(2);
    MResponseMatrixAxisSpheric Reference("Theta", "Phi");
    Reference.SetHEALPixByOrder(2);
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetFISBELByPixelSize()", "rejected", "A rejected pixel size", [&](){ Sky.SetFISBELByPixelSize(0.0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetHEALPixByPixelSize()", "rejected", "A rejected pixel size", [&](){ Sky.SetHEALPixByPixelSize(-2.0); }) && Passed;
    Passed = EvaluateException<MExceptionParameterOutOfRange>("SetHEALPixByNumberOfBins()", "rejected", "Rejected zero bins", [&](){ Sky.SetHEALPixByNumberOfBins(0); }) && Passed;
    Passed = EvaluateTrue("Set*()", "rejected", "The axis is unchanged after rejected setters (same binning)", Sky == Reference) && Passed;
    Passed = Evaluate("GetNumberOfBins()", "rejected", "The number of bins is unchanged after rejected setters", Sky.GetNumberOfBins(), 192UL) && Passed;
  }

  // Axes of different binning are different, also if they have the same number of bins
  {
    MResponseMatrixAxisSpheric FISBEL12("Theta", "Phi");
    FISBEL12.SetFISBELByNumberOfBins(12);
    MResponseMatrixAxisSpheric FISBEL12Again("Theta", "Phi");
    FISBEL12Again.SetFISBELByNumberOfBins(12);
    MResponseMatrixAxisSpheric FISBEL48("Theta", "Phi");
    FISBEL48.SetFISBELByNumberOfBins(48);
    MResponseMatrixAxisSpheric HEALPix0("Theta", "Phi");
    HEALPix0.SetHEALPixByOrder(0);
    MResponseMatrixAxisSpheric HEALPix0Again("Theta", "Phi");
    HEALPix0Again.SetHEALPixByOrder(0);
    MResponseMatrixAxisSpheric HEALPix1("Theta", "Phi");
    HEALPix1.SetHEALPixByOrder(1);

    // Compare through the base class, as MResponseMatrixON does
    const MResponseMatrixAxis& A = FISBEL12;
    Passed = EvaluateTrue("operator==()", "FISBEL 12 = FISBEL 12", "Two FISBEL axes with 12 bins are equal", A == FISBEL12Again) && Passed;
    Passed = EvaluateFalse("operator==()", "FISBEL 12 = FISBEL 48", "FISBEL axes with 12 and 48 bins are different", A == FISBEL48) && Passed;
    Passed = EvaluateFalse("operator==()", "FISBEL 12 = HEALPix 0", "A FISBEL axis and a HEALPix axis with 12 bins each are different", A == HEALPix0) && Passed;
    const MResponseMatrixAxis& B = HEALPix0;
    Passed = EvaluateTrue("operator==()", "HEALPix 0 = HEALPix 0", "Two HEALPix axes of order 0 are equal", B == HEALPix0Again) && Passed;
    Passed = EvaluateFalse("operator==()", "HEALPix 0 = HEALPix 1", "HEALPix axes of order 0 and 1 are different", B == HEALPix1) && Passed;
  }

  // ON matrices with different spherical axes are not identical
  {
    MResponseMatrixAxisSpheric FISBEL12("#nu", "#lambda");
    FISBEL12.SetFISBELByNumberOfBins(12);
    MResponseMatrixAxisSpheric HEALPix0("#nu", "#lambda");
    HEALPix0.SetHEALPixByOrder(0);
    MResponseMatrixON FISBELMatrix("FISBEL");
    FISBELMatrix.AddAxis(FISBEL12);
    MResponseMatrixON HEALPixMatrix("HEALPix");
    HEALPixMatrix.AddAxis(HEALPix0);
    Passed = EvaluateFalse("operator==()", "ON different spherical axes", "Response matrices with a FISBEL and a HEALPix axis of 12 bins each are different", FISBELMatrix == HEALPixMatrix) && Passed;
    Passed = EvaluateException<MExceptionObjectsNotIdentical>("operator+=()", "ON different spherical axes", "Adding response matrices with different spherical axes throws", [&](){ FISBELMatrix += HEALPixMatrix; }) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTResponseMatrixAxisSpheric Test;
  return Test.Run() == true ? 0 : 1;
}
