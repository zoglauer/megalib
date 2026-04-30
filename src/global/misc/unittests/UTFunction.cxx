/*
 * UTFunction.cxx
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
#include "MFunction.h"
#include "MResponseMatrixO1.h"
#include "MUnitTest.h"


//! Unit test class for MFunction
class UTFunction : public MUnitTest
{
public:
  UTFunction() : MUnitTest("UTFunction") {}
  virtual ~UTFunction() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTFunction::Run()
{
  bool Passed = true;

  system("mkdir -p /tmp/UTFunction");

  MFunction Default;
  Passed = Evaluate("MFunction()", "construction", "A representative MFunction instance can be constructed", true, true) && Passed;
  Passed = EvaluateNear("GetSize()", "default constructor", "The default representative function starts with one fallback data point", Default.GetSize(), 0.0, 1e-12) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetXMin()", "default constructor", "GetXMin throws for a representative empty function", [&]() { Default.GetXMin(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetXMax()", "default constructor", "GetXMax throws for a representative empty function", [&]() { Default.GetXMax(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("Evaluate()", "default constructor", "Evaluate throws for a representative empty function", [&]() { Default.Evaluate(1.0); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("Integrate()", "default constructor", "Integrate throws for a representative empty function", [&]() { Default.Integrate(); }) && Passed;

  {
    MFunction LinLin;
    Passed = Evaluate("Set()", "vector linlin", "MFunction accepts representative vector data with linear interpolation",
                      LinLin.Set(vector<double>{1.0, 2.0, 4.0}, vector<double>{3.0, 5.0, 9.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    Passed = Evaluate("GetNDataPoints()", "vector linlin", "The representative linear function stores the expected number of data points", LinLin.GetNDataPoints(), 3U) && Passed;
    Passed = EvaluateNear("GetSize()", "vector linlin", "GetSize returns the representative number of stored data points", LinLin.GetSize(), 3.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetDataPointX()", "vector linlin", "The representative linear function stores the first x value", LinLin.GetDataPointX(0), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetDataPointY()", "vector linlin", "The representative linear function stores the last y value", LinLin.GetDataPointY(2), 9.0, 1e-12) && Passed;
    Passed = EvaluateNear("Evaluate()", "vector linlin interior", "Linear interpolation evaluates a representative interior point exactly", LinLin.Evaluate(3.0), 7.0, 1e-12) && Passed;
    Passed = EvaluateNear("Evaluate()", "vector linlin extrapolation low", "Linear interpolation extrapolates a representative lower-x point consistently", LinLin.Evaluate(0.0), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("Evaluate()", "vector linlin extrapolation high", "Linear interpolation extrapolates a representative higher-x point consistently", LinLin.Evaluate(5.0), 11.0, 1e-12) && Passed;
    Passed = EvaluateNear("Eval()", "deprecated representative call", "Eval forwards to Evaluate for a representative interior point", LinLin.Eval(2.5), 6.0, 1e-12) && Passed;
    Passed = EvaluateNear("Integrate()", "vector linlin full", "Integrate returns the exact representative full linear integral", LinLin.Integrate(), 18.0, 1e-10) && Passed;
    Passed = EvaluateNear("Integrate(XMin, XMax)", "vector linlin partial", "Integrate over a representative interior sub-range is exact for the linear case", LinLin.Integrate(1.5, 3.5), 12.0, 1e-10) && Passed;
    Passed = EvaluateNear("GetXMin()", "vector linlin", "GetXMin returns the representative minimum x value", LinLin.GetXMin(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetXMax()", "vector linlin", "GetXMax returns the representative maximum x value", LinLin.GetXMax(), 4.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetYMin()", "vector linlin", "GetYMin returns the representative minimum y value", LinLin.GetYMin(), 3.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetYMax()", "vector linlin", "GetYMax returns the representative maximum y value", LinLin.GetYMax(), 9.0, 1e-12) && Passed;

    LinLin.ScaleX(0.5);
    Passed = EvaluateNear("ScaleX()", "vector linlin", "ScaleX rescales a representative linear function x-axis", LinLin.Evaluate(1.5), 7.0, 1e-12) && Passed;
    Passed = EvaluateNear("ScaleX()", "vector linlin range", "ScaleX updates the representative maximum x value", LinLin.GetXMax(), 2.0, 1e-12) && Passed;

    LinLin.ScaleY(2.0);
    Passed = EvaluateNear("ScaleY()", "vector linlin", "ScaleY rescales a representative linear function y content", LinLin.Evaluate(1.5), 14.0, 1e-12) && Passed;
    Passed = EvaluateNear("ScaleY()", "vector linlin range", "ScaleY updates the representative maximum y value", LinLin.GetYMax(), 18.0, 1e-12) && Passed;
  }

  {
    MFunction InvalidVectors;
    Passed = EvaluateException<MExceptionTestFailed>("Set()", "mismatched vector sizes", "Set rejects representative mismatched x and y vector sizes", [&]() {
      InvalidVectors.Set(vector<double>{1.0, 2.0}, vector<double>{3.0}, MFunction::c_InterpolationLinLin);
    }) && Passed;
    Passed = EvaluateException<MExceptionTestFailed>("Set()", "non-increasing vector<double> x", "Set rejects representative non-increasing x values for vector<double> input", [&]() {
      InvalidVectors.Set(vector<double>{2.0, 1.0}, vector<double>{3.0, 4.0}, MFunction::c_InterpolationLinLin);
    }) && Passed;
    Passed = EvaluateException<MExceptionTestFailed>("Set()", "mismatched long double vector sizes", "Set rejects representative mismatched x and y vector sizes for vector<long double> input", [&]() {
      InvalidVectors.Set(vector<long double>{1.0L, 2.0L}, vector<long double>{3.0L}, MFunction::c_InterpolationLinLin);
    }) && Passed;
    Passed = EvaluateException<MExceptionTestFailed>("Set()", "non-increasing vector<long double> x", "Set rejects representative non-increasing x values for vector<long double> input", [&]() {
      InvalidVectors.Set(vector<long double>{2.0L, 1.0L}, vector<long double>{3.0L, 4.0L}, MFunction::c_InterpolationLinLin);
    }) && Passed;
  }

  {
    MFunction LinLog;
    Passed = Evaluate("Set()", "vector linlog", "MFunction accepts representative vector data with lin-log interpolation",
                      LinLog.Set(vector<double>{1.0, 3.0}, vector<double>{exp(1.0), exp(5.0)}, MFunction::c_InterpolationLinLog), true) && Passed;
    Passed = EvaluateNear("Evaluate()", "vector linlog interior", "Lin-log interpolation evaluates a representative interior point exactly", LinLog.Evaluate(2.0), exp(3.0), 1e-10) && Passed;
    Passed = EvaluateNear("Integrate()", "vector linlog full", "Lin-log integration matches the representative analytic result", LinLog.Integrate(), 0.5*(exp(5.0) - exp(1.0)), 1e-9) && Passed;
  }

  {
    MFunction LogLin;
    Passed = Evaluate("Set()", "vector loglin", "MFunction accepts representative vector data with log-lin interpolation",
                      LogLin.Set(vector<double>{1.0, exp(2.0)}, vector<double>{1.0, 5.0}, MFunction::c_InterpolationLogLin), true) && Passed;
    Passed = EvaluateNear("Evaluate()", "vector loglin interior", "Log-lin interpolation evaluates a representative interior point exactly", LogLin.Evaluate(exp(1.0)), 3.0, 1e-10) && Passed;
    Passed = EvaluateNear("Integrate()", "vector loglin full", "Log-lin integration matches the representative analytic result", LogLin.Integrate(), 3.0*exp(2.0) + 1.0, 1e-8) && Passed;
  }

  {
    MFunction LogLog;
    Passed = Evaluate("Set()", "vector loglog", "MFunction accepts representative vector data with log-log interpolation",
                      LogLog.Set(vector<double>{1.0, exp(1.0)}, vector<double>{1.0, exp(2.0)}, MFunction::c_InterpolationLogLog), true) && Passed;
    Passed = EvaluateNear("Evaluate()", "vector loglog interior", "Log-log interpolation evaluates a representative interior point exactly", LogLog.Evaluate(sqrt(exp(1.0))), exp(1.0), 1e-10) && Passed;
    Passed = EvaluateNear("Integrate()", "vector loglog full", "Log-log integration matches the representative analytic result", LogLog.Integrate(), (exp(3.0) - 1.0)/3.0, 1e-8) && Passed;
  }

  {
    MFunction Discrete;
    Passed = Evaluate("Set()", "vector none", "MFunction accepts representative vector data without interpolation",
                      Discrete.Set(vector<double>{0.0, 10.0, 30.0}, vector<double>{1.0, 4.0, 9.0}, MFunction::c_InterpolationNone), true) && Passed;
    Passed = EvaluateNear("Evaluate()", "vector none low half-bin", "No interpolation returns the representative lower bin value inside a half-bin", Discrete.Evaluate(14.0), 4.0, 1e-12) && Passed;
    Passed = EvaluateNear("Evaluate()", "vector none upper half-bin", "No interpolation returns the representative upper bin value beyond the half-bin boundary", Discrete.Evaluate(21.0), 9.0, 1e-12) && Passed;
    Passed = EvaluateNear("Integrate(XMin, XMax)", "vector none first bin edge case", "No interpolation integrates a representative range starting at the first x value without accessing invalid lower bins", Discrete.Integrate(0.0, 10.0), 25.0, 1e-10) && Passed;
    Passed = EvaluateNear("Integrate(XMin, XMax)", "vector none multi-bin edge case", "No interpolation integrates a representative multi-bin range using each interior segment exactly once", Discrete.Integrate(0.0, 30.0), 155.0, 1e-10) && Passed;
  }

  {
    MFunction Added;
    Passed = Evaluate("Add()", "first point", "Add accepts the first representative data point", Added.Add(3.0, 7.0), true) && Passed;
    Passed = Evaluate("Add()", "prepend representative point", "Add inserts a representative lower x value at the front", Added.Add(1.0, 3.0), true) && Passed;
    Passed = Evaluate("Add()", "interior representative point", "Add inserts a representative interior x value in order", Added.Add(2.0, 5.0), true) && Passed;
    DisableDefaultStreams();
    bool AddedDuplicate = Added.Add(2.0, 6.0);
    EnableDefaultStreams();
    Passed = Evaluate("Add()", "duplicate representative point", "Add replaces the representative duplicate x value", AddedDuplicate, true) && Passed;
    Passed = Evaluate("GetNDataPoints()", "Add()", "Add keeps the representative duplicate replacement at the same number of data points", Added.GetNDataPoints(), 3U) && Passed;
    Passed = EvaluateNear("GetDataPointX()", "Add()", "Add keeps representative x values ordered after insertion", Added.GetDataPointX(1), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetDataPointY()", "Add()", "Add replaces the representative duplicate y value", Added.GetDataPointY(1), 6.0, 1e-12) && Passed;
  }

  {
    MResponseMatrixO1 Response("RepresentativeResponse", vector<float>{0.0f, 2.0f, 4.0f});
    Response.SetBinContent(0, 10.0f);
    Response.SetBinContent(1, 20.0f);
    MFunction FromResponse;
    Passed = Evaluate("Set()", "response matrix", "MFunction accepts a representative 1D response matrix", FromResponse.Set(Response), true) && Passed;
    Passed = Evaluate("GetNDataPoints()", "response matrix", "The representative response matrix creates one point per bin center", FromResponse.GetNDataPoints(), 2U) && Passed;
    Passed = EvaluateNear("GetDataPointX()", "response matrix", "The representative first response-matrix bin center is stored", FromResponse.GetDataPointX(0), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetDataPointY()", "response matrix", "The representative second response-matrix content is stored", FromResponse.GetDataPointY(1), 20.0, 1e-12) && Passed;
  }

  {
    MFunction Original;
    Passed = Evaluate("Set()", "copy representative source", "MFunction accepts representative source data for copy checks",
                      Original.Set(vector<double>{1.0, 2.0, 4.0}, vector<double>{3.0, 5.0, 9.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    MFunction Copied(Original);
    Passed = EvaluateNear("MFunction(const MFunction&)", "representative copy constructor", "The representative copy constructor preserves interpolation behavior", Copied.Evaluate(3.0), 7.0, 1e-12) && Passed;
    MFunction Assigned;
    Assigned = Original;
    Passed = EvaluateNear("operator=()", "representative assignment", "The representative assignment operator preserves interpolation behavior", Assigned.Evaluate(3.0), 7.0, 1e-12) && Passed;
  }

  {
    MFunction Constant;
    Passed = Evaluate("Set()", "constant representative function", "MFunction accepts a representative constant function for FindX and random tests",
                      Constant.Set(vector<double>{0.0, 10.0}, vector<double>{2.0, 2.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    Passed = EvaluateNear("FindX()", "non-cyclic representative integral", "FindX advances a representative non-cyclic constant function by the requested area", Constant.FindX(1.0, 6.0, false), 4.0, 1e-10) && Passed;
    Passed = EvaluateNear("FindX()", "cyclic representative integral", "FindX continues a representative cyclic search in the original x frame after wrapping internally", Constant.FindX(9.0, 6.0, true), 12.0, 1e-10) && Passed;

    gRandom->SetSeed(17);
    long double RandomA = Constant.GetRandom();
    long double RandomB = Constant.GetRandom();
    long double RandomC = Constant.GetRandom();
    Passed = EvaluateNear("GetRandom()", "representative draw A", "GetRandom returns the representative first seeded golden value", RandomA, 2.9466508319050074, 1e-6) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative draw B", "GetRandom returns the representative second seeded golden value", RandomB, 1.8177782750795287, 1e-6) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative draw C", "GetRandom returns the representative third seeded golden value", RandomC, 5.3058675560737306, 1e-6) && Passed;

    gRandom->SetSeed(23);
    long double RandomTimesXA = Constant.GetRandomTimesX();
    long double RandomTimesXB = Constant.GetRandomTimesX();
    long double RandomTimesXC = Constant.GetRandomTimesX();
    Passed = EvaluateNear("GetRandomTimesX()", "representative draw A", "GetRandomTimesX returns the representative first seeded golden value", RandomTimesXA, 7.1923381582481518, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandomTimesX()", "representative draw B", "GetRandomTimesX returns the representative second seeded golden value", RandomTimesXB, 8.1798413440764889, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandomTimesX()", "representative draw C", "GetRandomTimesX returns the representative third seeded golden value", RandomTimesXC, 9.731204644267359, 1e-4) && Passed;
  }

  {
    MFunction ConstantOffset;
    Passed = Evaluate("Set()", "offset constant representative function", "MFunction accepts a representative non-zero-offset constant function for random range checks",
                      ConstantOffset.Set(vector<double>{5.0, 9.0}, vector<double>{2.0, 2.0}, MFunction::c_InterpolationConstant), true) && Passed;
    gRandom->SetSeed(71);
    long double RandomOffset = ConstantOffset.GetRandom();
    Passed = EvaluateTrue("GetRandom()", "offset constant representative draw", "GetRandom returns a representative draw inside the non-zero-offset constant x-range", RandomOffset >= 5.0 && RandomOffset <= 9.0) && Passed;
  }

  {
    MFunction CyclicOffset;
    Passed = Evaluate("Set()", "cyclic non-zero-offset representative function", "MFunction accepts a representative non-zero-offset constant function for cyclic FindX checks",
                      CyclicOffset.Set(vector<double>{100.0, 110.0}, vector<double>{2.0, 2.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    Passed = EvaluateNear("FindX()", "cyclic non-zero-offset representative integral", "FindX wraps a representative cyclic search to the actual minimum x value instead of zero", CyclicOffset.FindX(109.0, 6.0, true), 112.0, 1e-10) && Passed;
  }

  {
    MFunction DynamicRangeZeroFront;
    Passed = Evaluate("Set()", "zero-front dynamic range representative function", "MFunction accepts a representative function starting at zero without dividing by zero in the dynamic-range check",
                      DynamicRangeZeroFront.Set(vector<double>{0.0, 100000000.0, 200000000.0}, vector<double>{1.0, 2.0, 3.0}, MFunction::c_InterpolationLinLin), true) && Passed;
  }

  {
    MFunction CachedRandom;
    Passed = Evaluate("Set()", "cached-random representative function", "MFunction accepts representative source data for cumulative-cache invalidation checks",
                      CachedRandom.Set(vector<double>{0.0, 1.0}, vector<double>{1.0, 1.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    gRandom->SetSeed(101);
    long double BeforeAdd = CachedRandom.GetRandom();
    Passed = Evaluate("Add()", "cached-random representative new point", "Add accepts a representative new point after the cumulative cache has been built", CachedRandom.Add(2.0, 1.0), true) && Passed;
    gRandom->SetSeed(101);
    long double AfterAdd = CachedRandom.GetRandom();
    Passed = EvaluateTrue("Add()", "cached-random representative invalidation", "Add invalidates the representative cumulative cache so the same seeded draw changes after the domain changes", fabsl(AfterAdd - BeforeAdd) > 1e-8) && Passed;
    Passed = EvaluateTrue("GetRandom()", "cached-random representative updated range", "GetRandom uses the representative updated range after Add", AfterAdd >= 0.0 && AfterAdd <= 2.0) && Passed;
  }

  {
    MFunction Saved;
    Passed = Evaluate("Set()", "save round-trip representative source", "MFunction accepts representative source data for a file round-trip",
                      Saved.Set(vector<double>{1.0, 2.5, 5.0}, vector<double>{3.0, 6.0, 11.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    MString FileName = "/tmp/UTFunction/roundtrip.dat";
    Passed = Evaluate("Save()", "representative round-trip file", "Save writes a representative function file", Saved.Save(FileName, "DP"), true) && Passed;

    MFunction ReadBack;
    Passed = Evaluate("Set()", "representative round-trip file", "Set reads back the representative function file", ReadBack.Set(FileName, "DP"), true) && Passed;
    Passed = Evaluate("GetNDataPoints()", "representative round-trip file", "Set preserves the representative data point count after a round-trip", ReadBack.GetNDataPoints(), 3U) && Passed;
    Passed = EvaluateNear("Evaluate()", "representative round-trip interior", "Set preserves representative linear interpolation after a file round-trip", ReadBack.Evaluate(3.0), 7.0, 1e-12) && Passed;
  }

  {
    MString DefaultInterpolationFile = "/tmp/UTFunction/default_interpolation.dat";
    ofstream Out(DefaultInterpolationFile.Data());
    Out<<"DP 2 5\n";
    Out<<"DP 4 9\n";
    Out.close();

    MFunction DefaultInterpolation;
    DisableDefaultStreams();
    bool DefaultInterpolationOk = DefaultInterpolation.Set(DefaultInterpolationFile, "DP");
    EnableDefaultStreams();
    Passed = Evaluate("Set()", "file without IP", "Set falls back to representative linear interpolation when no IP keyword is present", DefaultInterpolationOk, true) && Passed;
    Passed = EvaluateNear("Evaluate()", "file without IP interior", "The representative fallback interpolation behaves linearly at an interior point", DefaultInterpolation.Evaluate(3.0), 7.0, 1e-12) && Passed;
  }

  {
    MFunction Clamped;
    Passed = Evaluate("Set()", "representative clamped source", "MFunction accepts representative source data for integration edge checks",
                      Clamped.Set(vector<double>{1.0, 2.0, 4.0}, vector<double>{3.0, 5.0, 9.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    DisableDefaultStreams();
    double LowClamp = Clamped.Integrate(0.0, 3.5);
    double HighClamp = Clamped.Integrate(1.5, 10.0);
    double InvalidRange = Clamped.Integrate(3.5, 1.5);
    EnableDefaultStreams();
    Passed = EvaluateNear("Integrate(XMin, XMax)", "representative low clamp", "Integrate clamps a representative lower out-of-range limit to the function minimum", LowClamp, 13.75, 1e-10) && Passed;
    Passed = EvaluateNear("Integrate(XMin, XMax)", "representative high clamp", "Integrate clamps a representative upper out-of-range limit to the function maximum", HighClamp, 16.25, 1e-10) && Passed;
    Passed = EvaluateNear("Integrate(XMin, XMax)", "representative invalid range", "Integrate returns the representative sentinel value for XMin >= XMax", InvalidRange, 0.0, 1e-12) && Passed;
  }

  {
    MString BadTokenFile = "/tmp/UTFunction/bad_token.dat";
    ofstream Out(BadTokenFile.Data());
    Out<<"IP LinLin\n";
    Out<<"DP 1\n";
    Out.close();
    MFunction BadToken;
    DisableDefaultStreams();
    bool BadTokenOk = BadToken.Set(BadTokenFile, "DP");
    EnableDefaultStreams();
    Passed = Evaluate("Set()", "bad DP token count", "Set rejects a representative file with the wrong DP token count", BadTokenOk, false) && Passed;
  }

  {
    MString UnknownIPFile = "/tmp/UTFunction/unknown_ip.dat";
    ofstream Out(UnknownIPFile.Data());
    Out<<"IP Cubic\n";
    Out<<"DP 1 2\n";
    Out<<"DP 2 3\n";
    Out.close();
    MFunction UnknownIP;
    DisableDefaultStreams();
    bool UnknownIPOk = UnknownIP.Set(UnknownIPFile, "DP");
    EnableDefaultStreams();
    Passed = Evaluate("Set()", "unknown IP", "Set rejects a representative file with an unknown interpolation keyword", UnknownIPOk, false) && Passed;
  }

  {
    MString NonIncreasingFile = "/tmp/UTFunction/non_increasing.dat";
    ofstream Out(NonIncreasingFile.Data());
    Out<<"IP LinLin\n";
    Out<<"DP 2 2\n";
    Out<<"DP 1 3\n";
    Out.close();
    MFunction NonIncreasing;
    DisableDefaultStreams();
    bool NonIncreasingOk = NonIncreasing.Set(NonIncreasingFile, "DP");
    EnableDefaultStreams();
    Passed = Evaluate("Set()", "non-increasing x", "Set rejects a representative file with non-increasing x values", NonIncreasingOk, false) && Passed;
  }

  {
    MString LogFallbackFile = "/tmp/UTFunction/log_fallback.dat";
    ofstream Out(LogFallbackFile.Data());
    Out<<"IP LogLog\n";
    Out<<"DP -1 2\n";
    Out<<"DP 2 4\n";
    Out.close();
    MFunction LogFallback;
    DisableDefaultStreams();
    bool LogFallbackOk = LogFallback.Set(LogFallbackFile, "DP");
    EnableDefaultStreams();
    Passed = Evaluate("Set()", "log interpolation fallback", "Set accepts a representative log-log file with invalid x support and falls back to linear interpolation", LogFallbackOk, true) && Passed;
    Passed = EvaluateNear("Evaluate()", "log interpolation fallback interior", "The representative invalid log-log file behaves as linear interpolation after fallback", LogFallback.Evaluate(1.0), 3.3333333333333335, 1e-12) && Passed;
  }

  {
    MFunction Plotted;
    Passed = Evaluate("Set()", "representative plot source", "MFunction accepts representative source data for plot checks",
                      Plotted.Set(vector<double>{1.0, 2.0, 4.0}, vector<double>{3.0, 5.0, 9.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    Bool_t WasBatch = gROOT->IsBatch();
    int Before = gROOT->GetListOfCanvases()->GetSize();
    gROOT->SetBatch(true);
    Plotted.Plot();
    gROOT->SetBatch(WasBatch);
    Passed = EvaluateTrue("Plot()", "representative plotted function", "Plot creates a representative diagnostics canvas", gROOT->GetListOfCanvases()->GetSize() == Before + 1) && Passed;
    TCanvas* Canvas = dynamic_cast<TCanvas*>(gROOT->GetListOfCanvases()->Last());
    Passed = EvaluateTrue("Plot()", "representative plotted canvas", "Plot leaves a representative diagnostics canvas accessible through ROOT", Canvas != 0) && Passed;
    if (Canvas != 0) Canvas->Close();
  }

  {
    MFunction SaveFailure;
    Passed = Evaluate("Set()", "representative save-failure source", "MFunction accepts representative source data for save failure checks",
                      SaveFailure.Set(vector<double>{1.0, 2.0}, vector<double>{3.0, 5.0}, MFunction::c_InterpolationLinLin), true) && Passed;
    DisableDefaultStreams();
    bool SaveFailureOk = SaveFailure.Save("/tmp/UTFunction_missing_directory/out.dat");
    EnableDefaultStreams();
    Passed = Evaluate("Save()", "unwritable representative target", "Save returns false for a representative unwritable target path", SaveFailureOk, false) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFunction Test;
  return Test.Run() == true ? 0 : 1;
}
