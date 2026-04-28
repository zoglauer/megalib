/*
 * UTResponseMatrixAxis.cxx
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
#include "MResponseMatrixAxis.h"
#include "MUnitTest.h"


//! Unit test class for MResponseMatrixAxis
class UTResponseMatrixAxis : public MUnitTest
{
public:
  UTResponseMatrixAxis() : MUnitTest("UTResponseMatrixAxis") {}
  virtual ~UTResponseMatrixAxis() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTResponseMatrixAxis::Run()
{
  bool Passed = true;

  MResponseMatrixAxis Default("Energy");
  Passed = Evaluate("GetDimension()", "default constructor", "The default axis has one dimension", Default.GetDimension(), 1U) && Passed;
  Passed = Evaluate("GetNumberOfBins()", "default constructor", "The default axis starts with zero bins", Default.GetNumberOfBins(), 0UL) && Passed;
  Passed = Evaluate("IsLogarithmic()", "default constructor", "The default axis is not logarithmic", Default.IsLogarithmic(), false) && Passed;
  Passed = Evaluate("GetNames()", "default constructor", "The default axis stores the representative axis name", Default.GetNames()[0], MString("Energy")) && Passed;
  Passed = Evaluate("Has1DBinEdges()", "default constructor", "The default axis reports one-dimensional bin edges", Default.Has1DBinEdges(), true) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("GetMinima()", "default constructor", "An axis without bin edges cannot report minima", [&](){ Default.GetMinima(); }) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("GetMaxima()", "default constructor", "An axis without bin edges cannot report maxima", [&](){ Default.GetMaxima(); }) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("GetAxisBin()", "default constructor", "An axis without bin edges cannot determine bins", [&](){ Default.GetAxisBin(0.0); }) && Passed;
  Passed = Evaluate("InRange()", "default constructor", "An axis without bin edges reports values out of range", Default.InRange(0.0), false) && Passed;

  vector<double> BinEdges{0.0, 1.0, 3.0};
  Default.SetBinEdges(BinEdges);
  Passed = Evaluate("GetNumberOfBins()", "SetBinEdges()", "SetBinEdges stores the expected number of bins", Default.GetNumberOfBins(), 2UL) && Passed;
  Passed = EvaluateSize("Get1DBinEdges()", "SetBinEdges()", "SetBinEdges stores the expected representative number of bin edges", Default.Get1DBinEdges().size(), 3UL) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetBinEdges() first edge", "SetBinEdges stores the representative first bin edge", Default.Get1DBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetBinEdges() second edge", "SetBinEdges stores the representative middle bin edge", Default.Get1DBinEdges()[1], 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetBinEdges() third edge", "SetBinEdges stores the representative last bin edge", Default.Get1DBinEdges()[2], 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetAxisBin()", "first bin interior", "GetAxisBin maps a representative interior value to the first bin", Default.GetAxisBin(0.5), 0UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "upper edge", "GetAxisBin maps the exact upper edge to the last bin", Default.GetAxisBin(3.0), 1UL) && Passed;
  Passed = EvaluateNear("GetArea()", "first bin", "GetArea returns the representative first-bin width", Default.GetArea(0), 1.0, 1e-12) && Passed;
  Passed = Evaluate("InRange()", "lower boundary", "InRange accepts the representative lower boundary", Default.InRange(0.0), true) && Passed;
  Passed = Evaluate("InRange()", "upper boundary", "InRange accepts the representative upper boundary", Default.InRange(3.0), true) && Passed;
  Passed = Evaluate("InRange()", "underflow", "InRange rejects values below the representative lower boundary", Default.InRange(-0.1), false) && Passed;
  Passed = Evaluate("GetDrawingAxisBinEdges()", "SetBinEdges()", "GetDrawingAxisBinEdges returns a single representative drawing axis", Default.GetDrawingAxisBinEdges().size(), 1UL) && Passed;
  Passed = EvaluateSize("GetDrawingAxisBinEdges()", "SetBinEdges() edge count", "GetDrawingAxisBinEdges returns the representative number of drawing edges", Default.GetDrawingAxisBinEdges()[0].size(), 3UL) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetBinEdges() first edge", "GetDrawingAxisBinEdges returns the representative first drawing edge", Default.GetDrawingAxisBinEdges()[0][0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetBinEdges() middle edge", "GetDrawingAxisBinEdges returns the representative middle drawing edge", Default.GetDrawingAxisBinEdges()[0][1], 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDrawingAxisBinEdges()", "SetBinEdges() last edge", "GetDrawingAxisBinEdges returns the representative last drawing edge", Default.GetDrawingAxisBinEdges()[0][2], 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinima()", "SetBinEdges()", "GetMinima returns the representative minimum edge", Default.GetMinima()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxima()", "SetBinEdges()", "GetMaxima returns the representative maximum edge", Default.GetMaxima()[0], 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "first bin", "GetBinCenters returns the representative first-bin center", Default.GetBinCenters(0)[0], 0.5, 1e-12) && Passed;
  Passed = EvaluateException<MExceptionValueOutOfBounds>("GetAxisBin()", "underflow", "GetAxisBin throws on representative underflow", [&](){ Default.GetAxisBin(-0.1); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetArea()", "out of bounds", "GetArea throws on representative out-of-bounds access", [&](){ Default.GetArea(2); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBinCenters()", "out of bounds", "GetBinCenters throws on representative out-of-bounds access", [&](){ Default.GetBinCenters(2); }) && Passed;

  MResponseMatrixAxis SingleEdge("SingleEdge");
  SingleEdge.SetBinEdges(vector<double>{42.0});
  Passed = Evaluate("GetNumberOfBins()", "SetBinEdges() single edge", "SetBinEdges with one representative edge produces zero bins", SingleEdge.GetNumberOfBins(), 0UL) && Passed;
  Passed = EvaluateNear("GetMinima()", "SetBinEdges() single edge", "GetMinima returns the representative lone edge", SingleEdge.GetMinima()[0], 42.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxima()", "SetBinEdges() single edge", "GetMaxima returns the representative lone edge", SingleEdge.GetMaxima()[0], 42.0, 1e-12) && Passed;
  Passed = Evaluate("InRange()", "SetBinEdges() single edge", "InRange reports values out of range when no representative bins exist", SingleEdge.InRange(42.0), false) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("GetAxisBin()", "SetBinEdges() single edge", "GetAxisBin rejects a representative axis with fewer than two bin edges", [&](){ SingleEdge.GetAxisBin(42.0); }) && Passed;

  MResponseMatrixAxis Linear("Linear");
  Linear.SetLinear(2, 0.0, 4.0);
  Passed = EvaluateSize("Get1DBinEdges()", "SetLinear()", "SetLinear creates the expected representative number of linear bin edges", Linear.Get1DBinEdges().size(), 3UL) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLinear() first edge", "SetLinear creates the representative first linear bin edge", Linear.Get1DBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLinear() middle edge", "SetLinear creates the representative middle linear bin edge", Linear.Get1DBinEdges()[1], 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLinear() last edge", "SetLinear creates the representative last linear bin edge", Linear.Get1DBinEdges()[2], 4.0, 1e-12) && Passed;
  Passed = Evaluate("IsLogarithmic()", "SetLinear()", "SetLinear keeps the representative axis non-logarithmic", Linear.IsLogarithmic(), false) && Passed;

  MResponseMatrixAxis LinearWithFlows("LinearFlows");
  LinearWithFlows.SetLinear(2, 0.0, 4.0, -1.0, 5.0);
  Passed = EvaluateSize("Get1DBinEdges()", "SetLinear() with flows", "SetLinear stores the expected representative number of bin edges with flows", LinearWithFlows.Get1DBinEdges().size(), 5UL) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLinear() with flows underflow edge", "SetLinear stores the representative underflow edge", LinearWithFlows.Get1DBinEdges()[0], -1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLinear() with flows overflow edge", "SetLinear stores the representative overflow edge", LinearWithFlows.Get1DBinEdges()[4], 5.0, 1e-12) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetLinear() with flows underflow bin", "GetAxisBin uses the representative underflow bin when configured", LinearWithFlows.GetAxisBin(-0.5), 0UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetLinear() with flows overflow bin", "GetAxisBin uses the representative overflow bin when configured", LinearWithFlows.GetAxisBin(4.5), 3UL) && Passed;

  MResponseMatrixAxis Logarithmic("Time");
  Logarithmic.SetLogarithmic(2, 1.0, 100.0);
  Passed = Evaluate("IsLogarithmic()", "SetLogarithmic()", "SetLogarithmic marks the representative axis as logarithmic", Logarithmic.IsLogarithmic(), true) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLogarithmic() first edge", "SetLogarithmic keeps the representative first logarithmic edge", Logarithmic.Get1DBinEdges()[0], 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLogarithmic() middle edge", "SetLogarithmic creates the expected representative middle logarithmic edge", Logarithmic.Get1DBinEdges()[1], 10.0, 1e-10) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLogarithmic() last edge", "SetLogarithmic keeps the representative last logarithmic edge", Logarithmic.Get1DBinEdges()[2], 100.0, 1e-10) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetLogarithmic() representative interior", "GetAxisBin maps a representative logarithmic interior value to the first bin", Logarithmic.GetAxisBin(5.0), 0UL) && Passed;

  MResponseMatrixAxis LogarithmicWithFlows("TimeFlows");
  LogarithmicWithFlows.SetLogarithmic(2, 1.0, 100.0, 0.5, 200.0);
  Passed = EvaluateSize("Get1DBinEdges()", "SetLogarithmic() with flows", "SetLogarithmic stores the expected representative number of edges with flows", LogarithmicWithFlows.Get1DBinEdges().size(), 5UL) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLogarithmic() with flows underflow edge", "SetLogarithmic stores the representative underflow edge", LogarithmicWithFlows.Get1DBinEdges()[0], 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("Get1DBinEdges()", "SetLogarithmic() with flows overflow edge", "SetLogarithmic stores the representative overflow edge", LogarithmicWithFlows.Get1DBinEdges()[4], 200.0, 1e-12) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetLogarithmic() with flows underflow bin", "GetAxisBin uses the representative logarithmic underflow bin when configured", LogarithmicWithFlows.GetAxisBin(0.75), 0UL) && Passed;
  Passed = Evaluate("GetAxisBin()", "SetLogarithmic() with flows overflow bin", "GetAxisBin uses the representative logarithmic overflow bin when configured", LogarithmicWithFlows.GetAxisBin(150.0), 3UL) && Passed;

  Logarithmic.SetLinear(2, 0.0, 2.0);
  Passed = Evaluate("IsLogarithmic()", "SetLinear() after logarithmic", "SetLinear resets the representative logarithmic flag after a logarithmic configuration", Logarithmic.IsLogarithmic(), false) && Passed;

  Passed = EvaluateException<MExceptionTestFailed>("SetLinear()", "minimum >= maximum", "SetLinear rejects a representative invalid minimum/maximum ordering", [&](){ MResponseMatrixAxis Invalid; Invalid.SetLinear(2, 4.0, 4.0); }) && Passed;
  Passed = EvaluateException<MExceptionTestFailed>("SetLinear()", "zero bins", "SetLinear rejects a representative zero-bin configuration", [&](){ MResponseMatrixAxis Invalid; Invalid.SetLinear(0, 0.0, 4.0); }) && Passed;
  Passed = EvaluateException<MExceptionTestFailed>("SetLinear()", "underflow >= minimum", "SetLinear rejects a representative invalid underflow edge", [&](){ MResponseMatrixAxis Invalid; Invalid.SetLinear(2, 0.0, 4.0, 0.0); }) && Passed;
  Passed = EvaluateException<MExceptionTestFailed>("SetLinear()", "overflow <= maximum", "SetLinear rejects a representative invalid overflow edge", [&](){ MResponseMatrixAxis Invalid; Invalid.SetLinear(2, 0.0, 4.0, g_DoubleNotDefined, 4.0); }) && Passed;
  Passed = EvaluateException<MExceptionTestFailed>("SetLogarithmic()", "minimum <= 0", "SetLogarithmic rejects a representative non-positive minimum", [&](){ MResponseMatrixAxis Invalid; Invalid.SetLogarithmic(2, 0.0, 4.0); }) && Passed;
  Passed = EvaluateException<MExceptionTestFailed>("SetLogarithmic()", "zero bins", "SetLogarithmic rejects a representative zero-bin configuration", [&](){ MResponseMatrixAxis Invalid; Invalid.SetLogarithmic(0, 1.0, 4.0); }) && Passed;
  Passed = EvaluateException<MExceptionTestFailed>("SetLogarithmic()", "minimum >= maximum", "SetLogarithmic rejects a representative invalid minimum/maximum ordering", [&](){ MResponseMatrixAxis Invalid; Invalid.SetLogarithmic(2, 4.0, 4.0); }) && Passed;

  MResponseMatrixAxis LogarithmicThenCustom("Custom");
  LogarithmicThenCustom.SetLogarithmic(2, 1.0, 100.0);
  LogarithmicThenCustom.SetBinEdges(vector<double>{0.0, 1.0, 3.0});
  Passed = Evaluate("IsLogarithmic()", "SetBinEdges() after logarithmic", "SetBinEdges resets the representative logarithmic flag after replacing the axis definition", LogarithmicThenCustom.IsLogarithmic(), false) && Passed;

  MResponseMatrixAxis CloneSource("Clone");
  CloneSource.SetLinear(2, 0.0, 4.0);
  MResponseMatrixAxis* Clone = CloneSource.Clone();
  Passed = EvaluateTrue("Clone()", "representative clone", "Clone returns a representative heap-allocated axis", Clone != 0) && Passed;
  if (Clone != 0) {
    Passed = Evaluate("operator==()", "representative clone", "A clone compares equal to the representative source axis", *Clone == CloneSource, true) && Passed;
    delete Clone;
  }

  MResponseMatrixAxis EqualA("A");
  EqualA.SetLinear(2, 0.0, 4.0);
  MResponseMatrixAxis EqualB("B");
  EqualB.SetLinear(2, 0.0, 4.0);
  Passed = Evaluate("operator==()", "same physical axis", "Axes with the same representative physical binning compare equal", EqualA == EqualB, true) && Passed;
  EqualB.SetLinear(2, 0.0, 6.0);
  Passed = Evaluate("operator!=", "different physical axis", "Axes with different representative physical binning compare unequal", EqualA != EqualB, true) && Passed;

  ostringstream Stream;
  EqualA.Write(Stream);
  Passed = Evaluate("Write()", "representative text", "Write serializes the representative axis deterministically", MString(Stream.str()), MString("# Axis name\nAN \"A\"\n# Axis type\nAT 1D BinEdges\n# Axis data\nAD 0 2 4 \n")) && Passed;

  ostringstream FlowStream;
  LogarithmicWithFlows.Write(FlowStream);
  Passed = Evaluate("Write()", "logarithmic with flows", "Write serializes the representative logarithmic axis with flows deterministically", MString(FlowStream.str()), MString("# Axis name\nAN \"TimeFlows\"\n# Axis type\nAT 1D BinEdges\n# Axis data\nAD 0.5 1 10 100 200 \n")) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTResponseMatrixAxis Test;
  return Test.Run() == true ? 0 : 1;
}
