/*
 * UTFunction3D.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:
#include <TCanvas.h>
#include <TROOT.h>
#include <TRandom.h>

// MEGAlib:
#include "MExceptions.h"
#include "MFunction3D.h"
#include "MUnitTest.h"


//! Unit test class for MFunction3D
class UTFunction3D : public MUnitTest
{
public:
  UTFunction3D() : MUnitTest("UTFunction3D") {}
  virtual ~UTFunction3D() {}

  virtual bool Run();

private:
  vector<double> CreateAffineValues(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) const;
};


////////////////////////////////////////////////////////////////////////////////


vector<double> UTFunction3D::CreateAffineValues(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) const
{
  vector<double> Values;
  for (unsigned int z = 0; z < Z.size(); ++z) {
    for (unsigned int y = 0; y < Y.size(); ++y) {
      for (unsigned int x = 0; x < X.size(); ++x) {
        Values.push_back(1.0 + 2.0*X[x] - 3.0*Y[y] + 4.0*Z[z]);
      }
    }
  }
  return Values;
}


////////////////////////////////////////////////////////////////////////////////


bool UTFunction3D::Run()
{
  bool Passed = true;

  system("mkdir -p /tmp/UTFunction3D");

  MFunction3D Default;
  Passed = Evaluate("MFunction3D()", "construction", "A representative MFunction3D instance can be constructed", true, true) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetXMin()", "default constructor", "GetXMin throws for a representative empty 3D function", [&]() { Default.GetXMin(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetXMax()", "default constructor", "GetXMax throws for a representative empty 3D function", [&]() { Default.GetXMax(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetYMin()", "default constructor", "GetYMin throws for a representative empty 3D function", [&]() { Default.GetYMin(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetYMax()", "default constructor", "GetYMax throws for a representative empty 3D function", [&]() { Default.GetYMax(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetZMin()", "default constructor", "GetZMin throws for a representative empty 3D function", [&]() { Default.GetZMin(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetZMax()", "default constructor", "GetZMax throws for a representative empty 3D function", [&]() { Default.GetZMax(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetVMin()", "default constructor", "GetVMin throws for a representative empty 3D function", [&]() { Default.GetVMin(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetVMax()", "default constructor", "GetVMax throws for a representative empty 3D function", [&]() { Default.GetVMax(); }) && Passed;

  const vector<double> X{0.0, 1.0, 3.0};
  const vector<double> Y{2.0, 4.0, 5.0};
  const vector<double> Z{-1.0, 1.0, 2.0};
  const vector<double> Values = CreateAffineValues(X, Y, Z);

  {
    MFunction3D Invalid;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "too few x bins", "MFunction3D rejects representative vector input with too few x bins", Invalid.Set(vector<double>{0.0}, Y, Z, Values), false) && Passed;
    Passed = Evaluate("Set()", "too few y bins", "MFunction3D rejects representative vector input with too few y bins", Invalid.Set(X, vector<double>{2.0}, Z, Values), false) && Passed;
    Passed = Evaluate("Set()", "too few z bins", "MFunction3D rejects representative vector input with too few z bins", Invalid.Set(X, Y, vector<double>{-1.0}, Values), false) && Passed;
    Passed = Evaluate("Set()", "non-increasing x vectors", "MFunction3D rejects representative vector input with non-increasing x values", Invalid.Set(vector<double>{0.0, 0.0, 3.0}, Y, Z, Values), false) && Passed;
    Passed = Evaluate("Set()", "non-increasing y vectors", "MFunction3D rejects representative vector input with non-increasing y values", Invalid.Set(X, vector<double>{2.0, 2.0, 5.0}, Z, Values), false) && Passed;
    Passed = Evaluate("Set()", "non-increasing z vectors", "MFunction3D rejects representative vector input with non-increasing z values", Invalid.Set(X, Y, vector<double>{-1.0, -1.0, 2.0}, Values), false) && Passed;
    Passed = Evaluate("Set()", "wrong value count", "MFunction3D rejects representative vector input with the wrong number of values", Invalid.Set(X, Y, Z, vector<double>(Values.size()-1, 1.0)), false) && Passed;
    EnableDefaultStreams();
  }

  MFunction3D Affine;
  Passed = Evaluate("Set()", "representative 3D vectors", "MFunction3D accepts representative vector data", Affine.Set(X, Y, Z, Values), true) && Passed;
  MFunction3D Copied(Affine);
  Passed = EvaluateNear("MFunction3D(const MFunction3D&)", "representative 3D copy constructor", "The representative 3D copy constructor preserves interpolation behavior", Copied.Evaluate(2.0, 4.5, 1.5), -2.5, 1e-12) && Passed;
  MFunction3D Assigned;
  Assigned = Affine;
  Passed = EvaluateNear("operator=()", "representative 3D assignment", "The representative 3D assignment operator preserves interpolation behavior", Assigned.Evaluate(2.0, 4.5, 1.5), -2.5, 1e-12) && Passed;
  Passed = EvaluateNear("Evaluate()", "representative 3D interior 1", "Trilinear interpolation evaluates a representative interior affine field exactly", Affine.Evaluate(0.5, 3.0, 0.0), -7.0, 1e-12) && Passed;
  Passed = EvaluateNear("Evaluate()", "representative 3D interior 2", "Trilinear interpolation evaluates another representative interior affine field exactly", Affine.Evaluate(2.0, 4.5, 1.5), -2.5, 1e-12) && Passed;
  Passed = EvaluateNear("Eval()", "representative 3D deprecated call", "Eval forwards to Evaluate for a representative 3D interior point", Affine.Eval(1.5, 3.5, 0.5), -4.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetXMin()", "representative 3D vectors", "GetXMin returns the representative minimum x value", Affine.GetXMin(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetXMax()", "representative 3D vectors", "GetXMax returns the representative maximum x value", Affine.GetXMax(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetYMin()", "representative 3D vectors", "GetYMin returns the representative minimum y value", Affine.GetYMin(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetYMax()", "representative 3D vectors", "GetYMax returns the representative maximum y value", Affine.GetYMax(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZMin()", "representative 3D vectors", "GetZMin returns the representative minimum z value", Affine.GetZMin(), -1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZMax()", "representative 3D vectors", "GetZMax returns the representative maximum z value", Affine.GetZMax(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetVMin()", "representative 3D vectors", "GetVMin returns the representative minimum value", Affine.GetVMin(), -18.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetVMax()", "representative 3D vectors", "GetVMax returns the representative maximum value", Affine.GetVMax(), 9.0, 1e-12) && Passed;
  Passed = Evaluate("GetXAxis()", "representative 3D vectors", "GetXAxis returns the representative x-axis size", Affine.GetXAxis().size(), 3UL) && Passed;
  Passed = Evaluate("GetYAxis()", "representative 3D vectors", "GetYAxis returns the representative y-axis size", Affine.GetYAxis().size(), 3UL) && Passed;
  Passed = Evaluate("GetZAxis()", "representative 3D vectors", "GetZAxis returns the representative z-axis size", Affine.GetZAxis().size(), 3UL) && Passed;
  Passed = Evaluate("GetValues()", "representative 3D vectors", "GetValues returns the representative value count", Affine.GetValues().size(), Values.size()) && Passed;

  {
    const vector<double> NX{0.0, 1.0, 2.5, 4.0};
    const vector<double> NY{0.0, 1.0};
    const vector<double> NZ{0.0, 1.0};
    vector<double> NonAffineValues;
    for (unsigned int z = 0; z < NZ.size(); ++z) {
      for (unsigned int y = 0; y < NY.size(); ++y) {
        NonAffineValues.push_back(0.0);
        NonAffineValues.push_back(10.0);
        NonAffineValues.push_back(20.0);
        NonAffineValues.push_back(80.0);
      }
    }

    MFunction3D NonEquidistant;
    Passed = Evaluate("Set()", "representative non-equidistant vectors", "MFunction3D accepts representative non-equidistant vector data", NonEquidistant.Set(NX, NY, NZ, NonAffineValues), true) && Passed;
    Passed = EvaluateNear("Evaluate()", "representative non-equidistant left bin", "Evaluate uses the representative left-edge x bin for non-equidistant interpolation", NonEquidistant.Evaluate(0.5, 0.5, 0.5), 5.0, 1e-12) && Passed;
  }

  {
    const vector<double> NX{0.0, 1.0, 2.5, 4.0};
    const vector<double> NY{0.0, 1.0};
    const vector<double> NZ{0.0, 1.0};
    vector<double> NonAffineValues;
    for (unsigned int z = 0; z < NZ.size(); ++z) {
      for (unsigned int y = 0; y < NY.size(); ++y) {
        NonAffineValues.push_back(0.0);
        NonAffineValues.push_back(10.0);
        NonAffineValues.push_back(20.0);
        NonAffineValues.push_back(80.0);
      }
    }

    MFunction3D Reused;
    Passed = Evaluate("Set()", "representative reused equidistant source", "MFunction3D accepts representative equidistant source data before vector reuse", Reused.Set(vector<double>{0.0, 1.0, 2.0}, vector<double>{0.0, 1.0}, vector<double>{0.0, 1.0}, CreateAffineValues(vector<double>{0.0, 1.0, 2.0}, vector<double>{0.0, 1.0}, vector<double>{0.0, 1.0})), true) && Passed;
    Passed = Evaluate("Set()", "representative reused non-equidistant vectors", "MFunction3D resets representative axis-distance caches when vector Set reuses an object", Reused.Set(NX, NY, NZ, NonAffineValues), true) && Passed;
    Passed = EvaluateNear("Evaluate()", "representative reused non-equidistant left bin", "Evaluate uses the representative updated non-equidistant binning after vector Set reuse", Reused.Evaluate(0.5, 0.5, 0.5), 5.0, 1e-12) && Passed;
  }

  {
    MFunction3D Scaled;
    Passed = Evaluate("Set()", "scaled representative 3D vectors", "MFunction3D accepts representative vector data for scaling checks", Scaled.Set(X, Y, Z, Values), true) && Passed;
    Scaled.ScaleX(2.0);
    Scaled.ScaleY(0.5);
    Scaled.ScaleZ(3.0);
    Scaled.ScaleV(2.0);
    Passed = EvaluateNear("ScaleX()/ScaleY()/ScaleZ()/ScaleV()", "representative 3D affine field", "Scaling axes and values keeps a representative transformed interior point consistent", Scaled.Evaluate(1.0, 1.5, 0.0), -14.0, 1e-10) && Passed;
    Passed = EvaluateNear("GetVMax()", "representative 3D scaled range", "ScaleV updates the representative maximum value", Scaled.GetVMax(), 18.0, 1e-12) && Passed;
  }

  {
    const vector<double> NX{0.0, 1.0, 2.0};
    const vector<double> NY{0.0, 1.0};
    const vector<double> NZ{0.0, 1.0};
    vector<double> NonAffineValues;
    for (unsigned int z = 0; z < NZ.size(); ++z) {
      for (unsigned int y = 0; y < NY.size(); ++y) {
        NonAffineValues.push_back(0.0);
        NonAffineValues.push_back(10.0);
        NonAffineValues.push_back(80.0);
      }
    }

    MFunction3D ScaledCache;
    Passed = Evaluate("Set()", "representative cached-distance source", "MFunction3D accepts representative equidistant source data for cache-refresh checks", ScaledCache.Set(NX, NY, NZ, NonAffineValues), true) && Passed;
    ScaledCache.ScaleX(2.0);
    Passed = EvaluateNear("ScaleX()", "representative refreshed x distance", "ScaleX refreshes the representative x-axis distance cache before interpolation", ScaledCache.Evaluate(1.0, 0.5, 0.5), 5.0, 1e-12) && Passed;
    ScaledCache.RescaleX(10.0, 14.0);
    Passed = EvaluateNear("RescaleX()", "representative refreshed x distance after rescale", "RescaleX refreshes the representative x-axis distance cache before interpolation", ScaledCache.Evaluate(11.0, 0.5, 0.5), 5.0, 1e-12) && Passed;
  }

  {
    MFunction3D Rescaled;
    Passed = Evaluate("Set()", "rescaled representative 3D vectors", "MFunction3D accepts representative vector data for rescaling checks", Rescaled.Set(X, Y, Z, Values), true) && Passed;
    Rescaled.RescaleX(10.0, 16.0);
    Rescaled.RescaleY(-2.0, 1.0);
    Rescaled.RescaleZ(4.0, 10.0);
    Passed = EvaluateNear("RescaleX()/RescaleY()/RescaleZ()", "representative 3D affine field", "Rescaling axes keeps a representative transformed interior point consistent", Rescaled.Evaluate(11.0, -1.0, 6.0), -7.0, 1e-10) && Passed;
    Passed = EvaluateNear("GetXMin()", "representative 3D rescaled range", "RescaleX updates the representative minimum x value", Rescaled.GetXMin(), 10.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetYMax()", "representative 3D rescaled range", "RescaleY updates the representative maximum y value", Rescaled.GetYMax(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetZMax()", "representative 3D rescaled range", "RescaleZ updates the representative maximum z value", Rescaled.GetZMax(), 10.0, 1e-12) && Passed;
  }

  {
    MFunction3D Inverted;
    vector<double> SimpleValues{0, 1, 2, 3, 4, 5, 6, 7};
    Passed = Evaluate("Set()", "invert representative 3D vectors", "MFunction3D accepts representative 2x2x2 data for inversion checks",
                      Inverted.Set(vector<double>{0.0, 1.0}, vector<double>{0.0, 1.0}, vector<double>{0.0, 1.0}, SimpleValues), true) && Passed;
    Inverted.InvertX();
    Passed = EvaluateNear("InvertX()", "representative 2x2x2 values", "InvertX mirrors representative values along the x dimension", Inverted.GetValues()[0], 1.0, 1e-12) && Passed;
    Inverted.InvertY();
    Passed = EvaluateNear("InvertY()", "representative 2x2x2 values", "InvertY mirrors representative values along the y dimension", Inverted.GetValues()[0], 3.0, 1e-12) && Passed;
    Inverted.InvertZ();
    Passed = EvaluateNear("InvertZ()", "representative 2x2x2 values", "InvertZ mirrors representative values along the z dimension", Inverted.GetValues()[0], 7.0, 1e-12) && Passed;
  }

  {
    gRandom->SetSeed(51);
    double XR = 0.0;
    double YR = 0.0;
    double ZR = 0.0;
    Affine.GetRandom(XR, YR, ZR);
    Passed = EvaluateNear("GetRandom()", "representative 3D draw x", "GetRandom returns the representative seeded golden x value", XR, 2.2639156126804718, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative 3D draw y", "GetRandom returns the representative seeded golden y value", YR, 3.2432868623669222, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative 3D draw z", "GetRandom returns the representative seeded golden z value", ZR, 1.1861309729513574, 1e-4) && Passed;
  }

  {
    MFunction3D RoundTripSource;
    const vector<double> Axis{0.0, 1.0, 2.0};
    Passed = Evaluate("Set()", "representative 3D round-trip source", "MFunction3D accepts representative zero-based axes for a file round-trip",
                      RoundTripSource.Set(Axis, Axis, Axis, CreateAffineValues(Axis, Axis, Axis)), true) && Passed;
    MString FileName = "/tmp/UTFunction3D/roundtrip.fun";
    Passed = Evaluate("Save()", "representative 3D round-trip file", "Save writes a representative 3D function file", RoundTripSource.Save(FileName, "DP"), true) && Passed;

    MFunction3D RoundTripRead;
    Passed = Evaluate("Set()", "representative 3D round-trip file", "Set reads back a representative 3D function file", RoundTripRead.Set(FileName, "DP"), true) && Passed;
    Passed = EvaluateNear("Evaluate()", "representative 3D round-trip interior", "Set preserves representative trilinear interpolation after a zero-based file round-trip", RoundTripRead.Evaluate(0.5, 1.25, 1.5), 4.25, 1e-10) && Passed;
  }

  {
    MFunction3D InvalidScale;
    Passed = Evaluate("Set()", "invalid scale representative source", "MFunction3D accepts representative source data for invalid ScaleZ checks", InvalidScale.Set(X, Y, Z, Values), true) && Passed;
    DisableDefaultStreams();
    InvalidScale.ScaleX(0.0);
    EnableDefaultStreams();
    Passed = EvaluateNear("ScaleX()", "invalid non-positive scaler", "ScaleX leaves the representative x-axis unchanged for a non-positive scaler", InvalidScale.GetXMax(), 3.0, 1e-12) && Passed;
    DisableDefaultStreams();
    InvalidScale.ScaleY(-1.0);
    EnableDefaultStreams();
    Passed = EvaluateNear("ScaleY()", "invalid non-positive scaler", "ScaleY leaves the representative y-axis unchanged for a non-positive scaler", InvalidScale.GetYMax(), 5.0, 1e-12) && Passed;
    DisableDefaultStreams();
    InvalidScale.ScaleZ(0.0);
    EnableDefaultStreams();
    Passed = EvaluateNear("ScaleZ()", "invalid non-positive scaler", "ScaleZ leaves the representative z-axis unchanged for a non-positive scaler", InvalidScale.GetZMax(), 2.0, 1e-12) && Passed;
  }

  {
    MString UnknownIPFile = "/tmp/UTFunction3D/unknown_ip.fun";
    ofstream Out(UnknownIPFile.Data());
    Out<<"IP SPLINE\n";
    Out<<"XA 0 1\n";
    Out<<"YA 0 1\n";
    Out<<"ZA 0 1\n";
    Out<<"DP 0 0 0 1\n";
    Out<<"DP 1 0 0 2\n";
    Out<<"DP 0 1 0 3\n";
    Out<<"DP 1 1 0 4\n";
    Out<<"DP 0 0 1 5\n";
    Out<<"DP 1 0 1 6\n";
    Out<<"DP 0 1 1 7\n";
    Out<<"DP 1 1 1 8\n";
    Out.close();
    MFunction3D UnknownIP;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "unknown 3D IP", "MFunction3D rejects a representative file with an unknown interpolation keyword", UnknownIP.Set(UnknownIPFile, "DP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MString NonIncreasingZFile = "/tmp/UTFunction3D/non_increasing_z.fun";
    ofstream Out(NonIncreasingZFile.Data());
    Out<<"IP LIN\n";
    Out<<"XA 0 1\n";
    Out<<"YA 0 1\n";
    Out<<"ZA 2 1\n";
    Out<<"DP 0 0 2 1\n";
    Out<<"DP 1 0 2 2\n";
    Out<<"DP 0 1 2 3\n";
    Out<<"DP 1 1 2 4\n";
    Out<<"DP 0 0 1 5\n";
    Out<<"DP 1 0 1 6\n";
    Out<<"DP 0 1 1 7\n";
    Out<<"DP 1 1 1 8\n";
    Out.close();
    MFunction3D NonIncreasingZ;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "non-increasing z", "MFunction3D rejects a representative file with non-increasing z values", NonIncreasingZ.Set(NonIncreasingZFile, "DP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MString CompleteFile = "/tmp/UTFunction3D/complete.fun";
    ofstream OutComplete(CompleteFile.Data());
    OutComplete<<"IP LIN\n";
    OutComplete<<"XA 0 1\n";
    OutComplete<<"YA 0 1\n";
    OutComplete<<"ZA 0 1\n";
    OutComplete<<"DP 0 0 0 1\n";
    OutComplete<<"DP 1 0 0 2\n";
    OutComplete<<"DP 0 1 0 3\n";
    OutComplete<<"DP 1 1 0 4\n";
    OutComplete<<"DP 0 0 1 5\n";
    OutComplete<<"DP 1 0 1 6\n";
    OutComplete<<"DP 0 1 1 7\n";
    OutComplete<<"DP 1 1 1 8\n";
    OutComplete.close();

    MString MissingZAFile = "/tmp/UTFunction3D/missing_za.fun";
    ofstream OutMissing(MissingZAFile.Data());
    OutMissing<<"IP LIN\n";
    OutMissing<<"XA 0 1\n";
    OutMissing<<"YA 0 1\n";
    OutMissing<<"DP 0 0 0 1\n";
    OutMissing<<"DP 1 0 0 2\n";
    OutMissing<<"DP 0 1 0 3\n";
    OutMissing<<"DP 1 1 0 4\n";
    OutMissing.close();

    MFunction3D ReRead;
    Passed = Evaluate("Set()", "representative complete file before reread", "MFunction3D accepts a representative complete file before reread failure checks", ReRead.Set(CompleteFile, "DP"), true) && Passed;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "representative missing ZA on reread", "MFunction3D rejects a representative reread file that is missing the z-axis instead of reusing stale state", ReRead.Set(MissingZAFile, "DP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    DisableDefaultStreams();
    Passed = Evaluate("Save()", "unwritable representative 3D target", "Save returns false for a representative unwritable 3D target path", Affine.Save("/tmp/UTFunction3D_missing_directory/out.fun"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    Bool_t WasBatch = gROOT->IsBatch();
    int Before = gROOT->GetListOfCanvases()->GetSize();
    gROOT->SetBatch(true);
    Affine.Plot();
    gROOT->SetBatch(WasBatch);
    Passed = EvaluateTrue("Plot()", "representative 3D plot", "Plot creates a representative 3D diagnostics canvas", gROOT->GetListOfCanvases()->GetSize() == Before + 1) && Passed;
    TCanvas* Canvas = dynamic_cast<TCanvas*>(gROOT->GetListOfCanvases()->Last());
    Passed = EvaluateTrue("Plot()", "representative 3D plotted canvas", "Plot leaves a representative 3D diagnostics canvas accessible through ROOT", Canvas != 0) && Passed;
    if (Canvas != 0) Canvas->Close();
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFunction3D Test;
  return Test.Run() == true ? 0 : 1;
}
