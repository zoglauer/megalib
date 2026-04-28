/*
 * UTResponseMatrixAxisSpheric.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <sstream>
using namespace std;

// MEGAlib:
#include "MExceptions.h"
#include "MResponseMatrixAxisSpheric.h"
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

  Default.SetFISBEL(1);
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
  Shifted.SetFISBEL(1, 30.0);
  Passed = EvaluateNear("GetMinima()", "longitude shift theta", "GetMinima keeps the representative theta minimum under a longitude shift", Shifted.GetMinima()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinima()", "longitude shift phi", "GetMinima reflects the representative longitude shift in degrees", Shifted.GetMinima()[1], 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxima()", "longitude shift theta", "GetMaxima keeps the representative theta maximum under a longitude shift", Shifted.GetMaxima()[0], 180.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxima()", "longitude shift phi", "GetMaxima reflects the representative longitude shift in degrees", Shifted.GetMaxima()[1], 390.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "longitude shift phi first edge", "GetDrawingAxisBinEdges reflects the representative shifted first phi edge in degrees", Shifted.GetDrawingAxisBinEdges()[1][0], 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "longitude shift phi second edge", "GetDrawingAxisBinEdges reflects the representative shifted second phi edge in degrees", Shifted.GetDrawingAxisBinEdges()[1][1], 390.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "longitude shift phi", "GetBinCenters reflects the representative shifted phi center in degrees", Shifted.GetBinCenters(0)[1], 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAllBinCenters()", "longitude shift z", "GetAllBinCenters keeps the representative shifted north-pole vector along z", Shifted.GetAllBinCenters()[0].Z(), 1.0, 1e-12) && Passed;

  MResponseMatrixAxisSpheric FourBins("Theta", "Phi");
  FourBins.SetFISBEL(4);
  Passed = Evaluate("GetNumberOfBins()", "SetFISBEL(4)", "SetFISBEL stores the representative multi-bin spherical bin count", FourBins.GetNumberOfBins(), 4UL) && Passed;
  Passed = Evaluate("GetAllBinCenters()", "SetFISBEL(4)", "GetAllBinCenters returns one representative vector per spherical bin", FourBins.GetAllBinCenters().size(), 4UL) && Passed;
  Passed = Evaluate("GetDrawingAxisBinEdges()", "SetFISBEL(4) theta count", "GetDrawingAxisBinEdges returns a representative nontrivial theta edge set", FourBins.GetDrawingAxisBinEdges()[0].size() > 2, true) && Passed;
  Passed = Evaluate("GetDrawingAxisBinEdges()", "SetFISBEL(4) phi count", "GetDrawingAxisBinEdges returns a representative nontrivial phi edge set", FourBins.GetDrawingAxisBinEdges()[1].size() > 2, true) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetFISBEL(4) north pole", "GetAxisBin maps the representative north pole into the first spherical bin", FourBins.GetAxisBin(0.0, 0.0), 0UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetFISBEL(4) equator", "GetAxisBin maps a representative equatorial direction into a valid spherical bin", FourBins.GetAxisBin(90.0, 45.0) < FourBins.GetNumberOfBins(), true) && Passed;

  MResponseMatrixAxisSpheric EqualA("ThetaA", "PhiA");
  EqualA.SetFISBEL(4, 15.0);
  MResponseMatrixAxisSpheric EqualB("ThetaB", "PhiB");
  EqualB.SetFISBEL(4, 15.0);
  Passed = Evaluate("operator==()", "same physical axis", "Spherical axes with the same representative physical binning compare equal", EqualA == EqualB, true) && Passed;
  EqualB.SetFISBEL(4, 0.0);
  Passed = Evaluate("operator!=", "different longitude shift", "Spherical axes with a different representative longitude shift compare unequal", EqualA != EqualB, true) && Passed;

  MResponseMatrixAxisSpheric* Clone = EqualA.Clone();
  Passed = EvaluateTrue("Clone()", "representative clone", "Clone returns a representative heap-allocated spherical axis", Clone != 0) && Passed;
  if (Clone != 0) {
    Passed = Evaluate("operator==()", "representative clone", "A spherical clone compares equal to the representative source axis", *Clone == EqualA, true) && Passed;
    delete Clone;
  }

  ostringstream Stream;
  Shifted.Write(Stream);
  Passed = Evaluate("Write()", "representative text", "Write serializes the representative spherical axis deterministically", MString(Stream.str()), MString("# Axis name\nAN \"Theta\" \"Phi\"\n# Axis type\nAT 2D FISBEL\n# Axis data\nAD 1  30\n")) && Passed;

  ostringstream FourBinStream;
  FourBins.Write(FourBinStream);
  Passed = Evaluate("Write()", "multi-bin text", "Write serializes the representative multi-bin spherical axis deterministically", MString(FourBinStream.str()), MString("# Axis name\nAN \"Theta\" \"Phi\"\n# Axis type\nAT 2D FISBEL\n# Axis data\nAD 4  0\n")) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTResponseMatrixAxisSpheric Test;
  return Test.Run() == true ? 0 : 1;
}
