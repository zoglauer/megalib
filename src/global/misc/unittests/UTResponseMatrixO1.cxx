/*
 * UTResponseMatrixO1.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// MEGAlib:
#include "MResponseMatrixO1.h"
#include "UTResponseMatrixOrderShared.h"

// ROOT:
#include <TCanvas.h>
#include <TH1.h>
#include <TGraph.h>
#include <TROOT.h>


//! Unit test class for MResponseMatrixO1
class UTResponseMatrixO1 : public MUnitTest
{
public:
  UTResponseMatrixO1() : MUnitTest("UTResponseMatrixO1") {}
  virtual ~UTResponseMatrixO1() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTResponseMatrixO1::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareResponseMatrixTempDirectory()", "setup", "The temporary response-matrix directory can be created", PrepareResponseMatrixTempDirectory()) && Passed;

  vector<float> Axis{0.0f, 1.0f, 2.0f};
  vector<float> DifferentAxis{0.0f, 1.0f, 3.0f};
  vector<float> EmptyAxis;
  vector<float> NonIncreasingAxis{0.0f, 1.0f, 1.0f};

  Passed = EvaluateTrue("SetAxis()", "empty axis", "An empty axis triggers the documented assertion path", RunAssertingUnitTest("bin/UTResponseMatrixO1", "--assert-empty-axis")) && Passed;
  Passed = EvaluateTrue("SetAxis()", "non-increasing axis", "A non-increasing axis triggers the documented assertion path", RunAssertingUnitTest("bin/UTResponseMatrixO1", "--assert-nonincreasing-axis")) && Passed;

  MResponseMatrixO1 Default;
  Passed = Evaluate("GetOrder()", "default constructor", "The default constructor sets the matrix order", Default.GetOrder(), 1U) && Passed;
  Passed = Evaluate("GetNBins()", "default constructor", "The default constructor starts with zero bins", Default.GetNBins(), 0UL) && Passed;
  Passed = Evaluate("GetAxisBins()", "default constructor", "The default constructor starts with zero axis bins", Default.GetAxisBins(), 0U) && Passed;

  Default.SetAxis(Axis);
  Default.SetAxisNames("X1");
  Passed = Evaluate("GetNBins()", "set axis", "Setting the representative axis creates the expected number of bins", Default.GetNBins(), 2UL) && Passed;
  Passed = Evaluate("GetAxisName()", "set axis names", "The representative axis name is stored correctly", Default.GetAxisName(1), MString("X1")) && Passed;
  Passed = EvaluateNear("GetAxis()", "last edge", "GetAxis returns the expected edge vector", Default.GetAxis().back(), 2.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisContent()", "first edge", "The first edge value is correct", Default.GetAxisContent(0), 0.0, 1e-6) && Passed;
  Passed = Evaluate("GetAxisBins()", "representative axis", "The representative axis has the expected number of bins", Default.GetAxisBins(), 2U) && Passed;
  Passed = EvaluateNear("GetAxisBinCenter()", "first bin", "The first bin center is correct", Default.GetAxisBinCenter(0), 0.5, 1e-6) && Passed;
  Passed = Evaluate("GetAxisBin()", "representative coordinate", "A representative coordinate maps to the first bin", Default.GetAxisBin(0.5f), 0U) && Passed;
  Passed = EvaluateNear("GetAxisMinimum()", "representative axis", "The representative axis minimum is correct", Default.GetAxisMinimum(), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisMaximum()", "representative axis", "The representative axis maximum is correct", Default.GetAxisMaximum(), 2.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisLowEdge()", "first bin", "The first bin low edge is correct", Default.GetAxisLowEdge(0), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisHighEdge()", "first bin", "The first bin high edge is correct", Default.GetAxisHighEdge(0), 1.0, 1e-6) && Passed;

  MResponseMatrixO1 Matrix("Representative", Axis);
  Matrix.SetAxisNames("X1");
  MResponseMatrixO1 Different("Different", DifferentAxis);
  Passed = Evaluate("operator==()", "same axis", "Matrices with identical axes compare equal", Matrix == Default, true) && Passed;
  Passed = Evaluate("operator==()", "different axis", "Matrices with different axes compare unequal", Matrix == Different, false) && Passed;

  Matrix.SetBinContent(0, 5.0f);
  Passed = EvaluateNear("GetBinContent()", "set bin content", "SetBinContent stores the representative first-bin value", Matrix.GetBinContent(0), 5.0, 1e-6) && Passed;
  Matrix.Add(0.25f, 1.0f);
  Passed = EvaluateNear("Add()", "representative add", "Add accumulates into the representative first bin", Matrix.GetBinContent(0), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetBinArea()", "first bin", "The representative first bin has the expected area", Matrix.GetBinArea(0), 1.0, 1e-6) && Passed;
  Passed = EvaluateNear("Get()", "representative lookup", "Get returns the representative first-bin content", Matrix.Get(0.25f), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetInterpolated()", "bin center", "Interpolation at the representative first-bin center returns the stored content", Matrix.GetInterpolated(0.5f, false), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetMaximum()", "representative content", "GetMaximum returns the representative maximum", Matrix.GetMaximum(), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetMinimum()", "representative content", "GetMinimum returns the representative minimum", Matrix.GetMinimum(), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetSum()", "representative content", "GetSum returns the total representative content", Matrix.GetSum(), 6.0, 1e-6) && Passed;

  MResponseMatrixO1 SetMatrix(Axis);
  SetMatrix.SetAxisNames("X1");
  SetMatrix.Set(0.5f, 7.0f);
  Passed = EvaluateNear("Set()", "existing edge insertion behavior", "Set on a representative non-edge coordinate creates the expected new first-bin value", SetMatrix.Get(0.5f), 7.0, 1e-6) && Passed;

  MResponseMatrixO1 ExistingEdgeMatrix(Axis);
  ExistingEdgeMatrix.SetAxisNames("X1");
  ExistingEdgeMatrix.Set(0.0f, 9.0f);
  Passed = EvaluateNear("Set()", "existing edge axis stability", "Set on an existing representative axis edge preserves the axis edge value", ExistingEdgeMatrix.GetAxisContent(0), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("Set()", "existing edge bin content", "Set on an existing representative axis edge updates the first bin content", ExistingEdgeMatrix.GetBinContent(0), 9.0, 1e-6) && Passed;

  MResponseMatrixO1 Other(Axis);
  Other.SetBinContent(0, 2.0f);
  MResponseMatrixO1 Combined = Matrix;
  Combined += Other;
  Passed = EvaluateNear("operator+=()", "matrix addition", "Matrix addition accumulates the representative first bin", Combined.GetBinContent(0), 8.0, 1e-6) && Passed;
  Combined -= Other;
  Passed = EvaluateNear("operator-=()", "matrix subtraction", "Matrix subtraction restores the representative first bin", Combined.GetBinContent(0), 6.0, 1e-6) && Passed;
  MResponseMatrixO1 Divided = Matrix;
  Divided /= Other;
  Passed = EvaluateNear("operator/=()", "matrix division", "Matrix division divides the representative first bin", Divided.GetBinContent(0), 3.0, 1e-6) && Passed;
  MResponseMatrixO1 Scalar(Axis);
  Scalar += 1.0f;
  Passed = EvaluateNear("operator+=(float)", "scalar addition", "Scalar addition affects every bin", Scalar.GetSum(), 2.0, 1e-6) && Passed;
  Scalar *= 2.0f;
  Passed = EvaluateNear("operator*=(float)", "scalar multiplication", "Scalar multiplication rescales every bin", Scalar.GetSum(), 4.0, 1e-6) && Passed;

  MResponseMatrixO1 Sum1 = Matrix.GetSumMatrixO1(1);
  Passed = EvaluateNear("GetSumMatrixO1()", "identity projection", "The order-1 sum projection preserves the representative first-bin content", Sum1.GetBinContent(0), 6.0, 1e-6) && Passed;

  TH1* Histogram = Matrix.GetHistogram(false);
  Passed = EvaluateTrue("GetHistogram()", "representative histogram", "GetHistogram returns a representative ROOT histogram", Histogram != nullptr) && Passed;
  if (Histogram != nullptr) {
    Passed = EvaluateNear("GetHistogram()", "representative histogram content", "The representative histogram stores the first-bin content", Histogram->GetBinContent(1), 6.0, 1e-6) && Passed;
    delete Histogram;
  }

  MResponseMatrixO1 NormalizedHistogramMatrix({0.0f, 2.0f, 5.0f});
  NormalizedHistogramMatrix.SetBinContent(0, 8.0f);
  TH1* NormalizedHistogram = NormalizedHistogramMatrix.GetHistogram(true);
  Passed = EvaluateTrue("GetHistogram()", "normalized histogram", "GetHistogram returns a representative normalized histogram", NormalizedHistogram != nullptr) && Passed;
  if (NormalizedHistogram != nullptr) {
    Passed = EvaluateNear("GetHistogram()", "normalized first bin", "GetHistogram normalizes by the representative bin width", NormalizedHistogram->GetBinContent(1), 4.0, 1e-6) && Passed;
    delete NormalizedHistogram;
  }

  {
    bool WasBatch = gROOT->IsBatch();
    gROOT->SetBatch(true);
    int BeforeCanvases = GetCanvasCount();
    Matrix.Show(false);
    Passed = Evaluate("Show()", "representative display", "Show creates a ROOT canvas for the representative histogram", GetCanvasCount(), BeforeCanvases + 1) && Passed;
    CleanupCanvases(BeforeCanvases);
    gROOT->SetBatch(WasBatch);
  }

  TGraph* Graph = Matrix.GenerateGraph();
  Passed = EvaluateTrue("GenerateGraph()", "representative graph", "GenerateGraph returns a representative ROOT graph", Graph != nullptr) && Passed;
  if (Graph != nullptr) {
    double X = 0.0;
    double Y = 0.0;
    Passed = Evaluate("GenerateGraph()", "point count", "The representative graph contains one point per bin", Graph->GetN(), 2) && Passed;
    Graph->GetPoint(0, X, Y);
    Passed = EvaluateNear("GenerateGraph()", "first point x", "The representative graph uses the first bin center as x coordinate", X, 0.5, 1e-6) && Passed;
    Passed = EvaluateNear("GenerateGraph()", "first point y", "The representative graph uses the first bin content as y coordinate", Y, 6.0, 1e-6) && Passed;
    delete Graph;
  }

  MResponseMatrixO1 SmoothMatrix({0.0f, 1.0f, 2.0f, 3.0f});
  SmoothMatrix.SetBinContent(0, 0.0f);
  SmoothMatrix.SetBinContent(1, 10.0f);
  SmoothMatrix.SetBinContent(2, 0.0f);
  double ExpectedSmooth[3] = {0.0, 10.0, 0.0};
  TH1::SmoothArray(3, ExpectedSmooth, 1);
  SmoothMatrix.Smooth(1);
  Passed = EvaluateNear("Smooth()", "first bin", "Smoothing reproduces the ROOT reference smoothing for the first representative bin", SmoothMatrix.GetBinContent(0), ExpectedSmooth[0], 1e-6) && Passed;
  Passed = EvaluateNear("Smooth()", "second bin", "Smoothing reproduces the ROOT reference smoothing for the second representative bin", SmoothMatrix.GetBinContent(1), ExpectedSmooth[1], 1e-6) && Passed;
  Passed = EvaluateNear("Smooth()", "third bin", "Smoothing reproduces the ROOT reference smoothing for the third representative bin", SmoothMatrix.GetBinContent(2), ExpectedSmooth[2], 1e-6) && Passed;

  MString FileName = "/tmp/UTResponseMatrix/UTResponseMatrixO1.rsp";
  MString TextFileName = "/tmp/UTResponseMatrix/UTResponseMatrixO1_text.rsp";
  Passed = Evaluate("Write()", "text round trip", "Writing the representative matrix in text mode succeeds", Matrix.Write(TextFileName, false), true) && Passed;
  MResponseMatrixO1 ReadBackText;
  Passed = Evaluate("Read()", "text round trip", "Reading the representative matrix written in text mode succeeds", ReadBackText.Read(TextFileName), true) && Passed;
  Passed = EvaluateNear("GetBinContent()", "text round trip", "The representative first-bin content survives a text round trip", ReadBackText.GetBinContent(0), 6.0, 1e-6) && Passed;
  Passed = Evaluate("Write()", "stream round trip", "Writing the representative matrix in stream mode succeeds", Matrix.Write(FileName, true), true) && Passed;
  MResponseMatrixO1 ReadBack;
  Passed = Evaluate("Read()", "stream round trip", "Reading the representative matrix written in stream mode succeeds", ReadBack.Read(FileName), true) && Passed;
  Passed = EvaluateNear("GetBinContent()", "stream round trip", "The representative first-bin content survives a stream round trip", ReadBack.GetBinContent(0), 6.0, 1e-6) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (argc == 2) {
    vector<float> EmptyAxis;
    vector<float> NonIncreasingAxis{0.0f, 1.0f, 1.0f};
    MResponseMatrixO1 Matrix;
    MString Argument = argv[1];
    if (Argument == "--assert-empty-axis") {
      Matrix.SetAxis(EmptyAxis);
      return 0;
    }
    if (Argument == "--assert-nonincreasing-axis") {
      Matrix.SetAxis(NonIncreasingAxis);
      return 0;
    }
  }
  UTResponseMatrixO1 Test;
  return Test.Run() == true ? 0 : 1;
}
