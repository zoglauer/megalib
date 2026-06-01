/*
 * UTFunction2D.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include <TCanvas.h>
#include <TROOT.h>
#include <TRandom.h>

// MEGAlib:
#include "MExceptions.h"
#include "MFile.h"
#define private public
#include "MFunction2D.h"
#undef private
#include "MUnitTest.h"


//! Unit test class for MFunction2D
class UTFunction2D : public MUnitTest
{
public:
  UTFunction2D() : MUnitTest("UTFunction2D") {}
  virtual ~UTFunction2D() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTFunction2D::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "temporary directory", "The temporary directory for MFunction2D fixtures can be created", PrepareTemporaryDirectory()) && Passed;

  MFunction2D Default;
  Passed = Evaluate("MFunction2D()", "construction", "A representative MFunction2D instance can be constructed", true, true) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetXMin()", "default constructor", "GetXMin throws for a representative empty 2D function", [&]() { Default.GetXMin(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetXMax()", "default constructor", "GetXMax throws for a representative empty 2D function", [&]() { Default.GetXMax(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetYMin()", "default constructor", "GetYMin throws for a representative empty 2D function", [&]() { Default.GetYMin(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetYMax()", "default constructor", "GetYMax throws for a representative empty 2D function", [&]() { Default.GetYMax(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetZMin()", "default constructor", "GetZMin throws for a representative empty 2D function", [&]() { Default.GetZMin(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetZMax()", "default constructor", "GetZMax throws for a representative empty 2D function", [&]() { Default.GetZMax(); }) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("Evaluate()", "default constant function", "Evaluate throws for a representative empty constant 2D function", [&]() {
    MFunction2D EmptyConstant;
    EmptyConstant.m_InterpolationType = MFunction2D::c_InterpolationConstant;
    EmptyConstant.Evaluate(0.0, 0.0);
  }) && Passed;

  MString LinearFile = GetTemporaryFileName("linear.fun");
  Passed = EvaluateTrue("WriteTextFile()", "linear file", "The representative linear 2D file can be written",
                        WriteTextFile(LinearFile, "IP LIN\nXA 1 2 4\nYA -1 1 2\nGR 0 0 2 6\nGR 1 6 8 12\nGR 2 9 11 15\n")) && Passed;

  MFunction2D Linear;
  Passed = Evaluate("Set()", "representative linear file", "MFunction2D reads a representative linear file successfully", Linear.Set(LinearFile, "AP"), true) && Passed;
  MFunction2D Copied(Linear);
  Passed = EvaluateNear("MFunction2D(const MFunction2D&)", "representative 2D copy constructor", "The representative 2D copy constructor preserves interpolation behavior", Copied.Evaluate(2.5, 0.5), 7.5, 1e-12) && Passed;
  MFunction2D Assigned;
  Assigned = Linear;
  Passed = EvaluateNear("operator=()", "representative 2D assignment", "The representative 2D assignment operator preserves interpolation behavior", Assigned.Evaluate(2.5, 0.5), 7.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetXMin()", "representative linear file", "GetXMin returns the representative minimum x value", Linear.GetXMin(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetXMax()", "representative linear file", "GetXMax returns the representative maximum x value", Linear.GetXMax(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetYMin()", "representative linear file", "GetYMin returns the representative minimum y value", Linear.GetYMin(), -1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetYMax()", "representative linear file", "GetYMax returns the representative maximum y value", Linear.GetYMax(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZMin()", "representative linear file", "GetZMin returns the representative minimum z value", Linear.GetZMin(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZMax()", "representative linear file", "GetZMax returns the representative maximum z value", Linear.GetZMax(), 15.0, 1e-12) && Passed;
  Passed = EvaluateNear("Evaluate()", "representative linear interior", "Linear interpolation evaluates a representative interior point of an affine plane exactly", Linear.Evaluate(2.5, 0.5), 7.5, 1e-12) && Passed;
  Passed = EvaluateNear("Evaluate()", "representative linear extrapolation", "Linear interpolation extrapolates a representative point consistently", Linear.Evaluate(0.5, -0.5), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("Eval()", "representative linear deprecated call", "Eval forwards to Evaluate for a representative 2D interior point", Linear.Eval(3.0, 1.25), 10.75, 1e-12) && Passed;

  Linear.ScaleX(2.0);
  Linear.ScaleY(0.5);
  Linear.ScaleZ(2.0);
  Passed = EvaluateNear("ScaleX()/ScaleY()/ScaleZ()", "representative affine plane", "Scaling a representative 2D affine plane updates axes and values consistently", Linear.Evaluate(5.0, 0.25), 15.0, 1e-12) && Passed;
  Passed = EvaluateNear("ScaleX()", "representative affine plane range", "ScaleX updates the representative maximum x value", Linear.GetXMax(), 8.0, 1e-12) && Passed;
  Passed = EvaluateNear("ScaleY()", "representative affine plane range", "ScaleY updates the representative maximum y value", Linear.GetYMax(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("ScaleZ()", "representative affine plane range", "ScaleZ updates the representative maximum z value", Linear.GetZMax(), 30.0, 1e-12) && Passed;

  MString NoneFile = GetTemporaryFileName("none.fun");
  Passed = EvaluateTrue("WriteTextFile()", "none file", "The representative no-interpolation 2D file can be written",
                        WriteTextFile(NoneFile, "IP NONE\nXA 0 10 30\nYA 0 20 50\nGR 0 0 1 2\nGR 1 10 11 12\nGR 2 20 21 22\n")) && Passed;

  MFunction2D None;
  Passed = Evaluate("Set()", "representative none file", "MFunction2D reads a representative no-interpolation file successfully", None.Set(NoneFile, "AP"), true) && Passed;
  Passed = EvaluateNear("Evaluate()", "representative none interior", "No interpolation selects the representative nearest x/y sample in the interior", None.Evaluate(24.0, 11.0), 12.0, 1e-12) && Passed;
  Passed = EvaluateNear("Evaluate()", "representative none low edge", "No interpolation selects the representative lower-edge sample consistently", None.Evaluate(-2.0, 3.0), 0.0, 1e-12) && Passed;

  {
    gRandom->SetSeed(41);
    double X1 = 0.0;
    double Y1 = 0.0;
    double X2 = 0.0;
    double Y2 = 0.0;
    double X3 = 0.0;
    double Y3 = 0.0;
    None.GetRandom(X1, Y1);
    None.GetRandom(X2, Y2);
    None.GetRandom(X3, Y3);
    Passed = EvaluateNear("GetRandom()", "representative draw 1 x", "GetRandom returns the representative first seeded golden x value", X1, 7.5277065185175756, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative draw 1 y", "GetRandom returns the representative first seeded golden y value", Y1, 34.028579722378756, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative draw 2 x", "GetRandom returns the representative second seeded golden x value", X2, 9.6292030625674019, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative draw 2 y", "GetRandom returns the representative second seeded golden y value", Y2, 33.840824853547192, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative draw 3 x", "GetRandom returns the representative third seeded golden x value", X3, 1.3040826282232657, 1e-4) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative draw 3 y", "GetRandom returns the representative third seeded golden y value", Y3, 30.028429401520969, 1e-4) && Passed;
  }

  MString InvalidXBFile = GetTemporaryFileName("invalid_xb.fun");
  Passed = EvaluateTrue("WriteTextFile()", "invalid XB file", "The representative invalid-XB 2D file can be written",
                        WriteTextFile(InvalidXBFile, "IP LIN\nXB 1 4 1\nYA 0 1\nGR 0 1\nGR 1 2\n")) && Passed;

  MFunction2D InvalidXB;
  DisableDefaultStreams();
  Passed = Evaluate("Set()", "invalid XB", "MFunction2D rejects a representative XB definition with too few bin centers", InvalidXB.Set(InvalidXBFile, "AP"), false) && Passed;
  EnableDefaultStreams();

  {
    MString InvalidAPXIndexFile = GetTemporaryFileName("invalid_ap_x_index.fun");
    Passed = EvaluateTrue("WriteTextFile()", "invalid AP x-index file", "The representative invalid AP x-index file can be written",
                          WriteTextFile(InvalidAPXIndexFile, "IP LIN\nXA 0 1\nYA 0 1\nAP 2 0 1\n")) && Passed;
    MFunction2D InvalidAPXIndex;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "invalid AP x index equal to size", "MFunction2D rejects a representative AP x index equal to the x-axis size", InvalidAPXIndex.Set(InvalidAPXIndexFile, "AP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MString InvalidAPYIndexFile = GetTemporaryFileName("invalid_ap_y_index.fun");
    Passed = EvaluateTrue("WriteTextFile()", "invalid AP y-index file", "The representative invalid AP y-index file can be written",
                          WriteTextFile(InvalidAPYIndexFile, "IP LIN\nXA 0 1\nYA 0 1\nAP 0 2 1\n")) && Passed;
    MFunction2D InvalidAPYIndex;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "invalid AP y index equal to size", "MFunction2D rejects a representative AP y index equal to the y-axis size", InvalidAPYIndex.Set(InvalidAPYIndexFile, "AP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MString InvalidGRYIndexFile = GetTemporaryFileName("invalid_gr_y_index.fun");
    Passed = EvaluateTrue("WriteTextFile()", "invalid GR y-index file", "The representative invalid GR y-index file can be written",
                          WriteTextFile(InvalidGRYIndexFile, "IP LIN\nXA 0 1\nYA 0 1\nGR 2 1 2\n")) && Passed;
    MFunction2D InvalidGRYIndex;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "invalid GR y index equal to size", "MFunction2D rejects a representative GR y index equal to the y-axis size", InvalidGRYIndex.Set(InvalidGRYIndexFile, "AP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MString UnknownIPFile = GetTemporaryFileName("unknown_ip.fun");
    Passed = EvaluateTrue("WriteTextFile()", "unknown interpolation file", "The representative unknown-interpolation 2D file can be written",
                          WriteTextFile(UnknownIPFile, "IP SPLINE\nXA 0 1\nYA 0 1\nGR 0 1 2\nGR 1 3 4\n")) && Passed;
    MFunction2D UnknownIP;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "unknown 2D IP", "MFunction2D rejects a representative file with an unknown interpolation keyword", UnknownIP.Set(UnknownIPFile, "AP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MString NonIncreasingYFile = GetTemporaryFileName("non_increasing_y.fun");
    Passed = EvaluateTrue("WriteTextFile()", "non-increasing y file", "The representative non-increasing-y 2D file can be written",
                          WriteTextFile(NonIncreasingYFile, "IP LIN\nXA 0 1\nYA 2 1\nGR 0 1 2\nGR 1 3 4\n")) && Passed;
    MFunction2D NonIncreasingY;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "non-increasing y", "MFunction2D rejects a representative file with non-increasing y values", NonIncreasingY.Set(NonIncreasingYFile, "AP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    Bool_t WasBatch = gROOT->IsBatch();
    int Before = gROOT->GetListOfCanvases()->GetSize();
    gROOT->SetBatch(true);
    Linear.Plot();
    gROOT->SetBatch(WasBatch);
    Passed = EvaluateTrue("Plot()", "representative 2D plot", "Plot creates a representative 2D diagnostics canvas", gROOT->GetListOfCanvases()->GetSize() == Before + 1) && Passed;
    TCanvas* Canvas = dynamic_cast<TCanvas*>(gROOT->GetListOfCanvases()->Last());
    Passed = EvaluateTrue("Plot()", "representative 2D plotted canvas", "Plot leaves a representative 2D diagnostics canvas accessible through ROOT", Canvas != 0) && Passed;
    if (Canvas != 0) Canvas->Close();
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFunction2D Test;
  return Test.Run() == true ? 0 : 1;
}
