/*
 * UTResponseMatrixO2.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// MEGAlib:
#include "MResponseMatrixO2.h"
#include "MStreams.h"
#include "UTResponseMatrixOrderShared.h"


//! Unit test class for MResponseMatrixO2
class UTResponseMatrixO2 : public MUnitTest
{
public:
  UTResponseMatrixO2() : MUnitTest("UTResponseMatrixO2") {}
  virtual ~UTResponseMatrixO2() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTResponseMatrixO2::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareResponseMatrixTempDirectory()", "setup", "The temporary response-matrix directory can be created", PrepareResponseMatrixTempDirectory()) && Passed;

  vector<float> A1{0.0f, 1.0f, 2.0f};
  vector<float> A2{0.0f, 1.0f, 2.0f};
  vector<float> ADifferent{0.0f, 1.0f, 3.0f};
  vector<float> EmptyAxis;
  vector<float> NonIncreasingAxis{0.0f, 1.0f, 1.0f};

  Passed = EvaluateTrue("SetAxis()", "empty first axis", "An empty first axis triggers the documented assertion path", RunAssertingUnitTest("bin/UTResponseMatrixO2", "--assert-empty-first-axis")) && Passed;
  Passed = EvaluateTrue("SetAxis()", "non-increasing second axis", "A non-increasing second axis triggers the documented assertion path", RunAssertingUnitTest("bin/UTResponseMatrixO2", "--assert-nonincreasing-second-axis")) && Passed;

  MResponseMatrixO2 Default;
  Passed = Evaluate("GetOrder()", "default constructor", "The default constructor sets the matrix order", Default.GetOrder(), 2U) && Passed;
  Passed = Evaluate("GetNBins()", "default constructor", "The default constructor starts with zero bins", Default.GetNBins(), 0UL) && Passed;
  Default.SetAxis(A1, A2);
  Default.SetAxisNames("X1", "X2");
  Passed = Evaluate("GetNBins()", "set axis", "Setting representative axes creates the expected number of bins", Default.GetNBins(), 4UL) && Passed;
  Passed = Evaluate("GetAxisName()", "first axis", "The first axis name is stored correctly", Default.GetAxisName(1), MString("X1")) && Passed;
  Passed = Evaluate("GetAxisName()", "last axis", "The last axis name is stored correctly", Default.GetAxisName(2), MString("X2")) && Passed;
  Passed = EvaluateNear("GetAxis()", "first axis last edge", "GetAxis returns the expected edge vector for the first axis", Default.GetAxis(1).back(), 2.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisContent()", "first axis first edge", "The first axis edge content is correct", Default.GetAxisContent(0, 1), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisContent()", "last axis last edge", "The last axis edge content is correct", Default.GetAxisContent(2, 2), 2.0, 1e-6) && Passed;
  Passed = Evaluate("GetAxisBins()", "first axis", "The first axis has the expected number of bins", Default.GetAxisBins(1), 2U) && Passed;
  Passed = Evaluate("GetAxisBins()", "last axis", "The last axis has the expected number of bins", Default.GetAxisBins(2), 2U) && Passed;
  Passed = EvaluateNear("GetAxisBinCenter()", "first axis", "The first bin center is correct", Default.GetAxisBinCenter(0, 1), 0.5, 1e-6) && Passed;
  Passed = Evaluate("GetAxisBin()", "first axis", "The representative coordinate maps to the first bin", Default.GetAxisBin(0.5f, 1), 0U) && Passed;
  Passed = EvaluateNear("GetAxisMinimum()", "first axis", "The first axis minimum is correct", Default.GetAxisMinimum(1), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisMaximum()", "last axis", "The last axis maximum is correct", Default.GetAxisMaximum(2), 2.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisLowEdge()", "last axis", "The last axis low edge is correct", Default.GetAxisLowEdge(0, 2), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetAxisHighEdge()", "last axis", "The last axis high edge is correct", Default.GetAxisHighEdge(0, 2), 1.0, 1e-6) && Passed;

  MResponseMatrixO2 DifferentAxis("Different", A1, ADifferent);
  MResponseMatrixO2 Matrix("Representative", A1, A2);
  Matrix.SetAxisNames("X1", "X2");
  Passed = Evaluate("operator==()", "same axes", "Matrices with identical axes compare equal", Matrix == Default, true) && Passed;
  Passed = Evaluate("operator==()", "different axes", "Matrices with different axes compare unequal", Matrix == DifferentAxis, false) && Passed;
  Matrix.SetBinContent(0, 0, 5.0f);
  Passed = EvaluateNear("GetBinContent()", "set bin content", "SetBinContent stores the representative first-bin value", Matrix.GetBinContent(0, 0), 5.0, 1e-6) && Passed;
  Matrix.Add(0.25f, 0.25f, 1.0f);
  Passed = EvaluateNear("Add()", "representative add", "Add accumulates into the representative first bin", Matrix.GetBinContent(0, 0), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetBinArea()", "first bin", "The representative first bin has the expected volume", Matrix.GetBinArea(0, 0), 1.0, 1e-6) && Passed;
  Passed = EvaluateNear("Get()", "representative lookup", "Get returns the representative first-bin content", Matrix.Get(0.25f, 0.25f), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetInterpolated()", "bin center", "Interpolation at the representative first-bin center returns the stored content", Matrix.GetInterpolated(0.5f, 0.5f, false), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetMaximum()", "representative content", "GetMaximum returns the representative maximum", Matrix.GetMaximum(), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetMinimum()", "representative content", "GetMinimum returns the representative minimum", Matrix.GetMinimum(), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetSum()", "representative content", "GetSum returns the total representative content", Matrix.GetSum(), 6.0, 1e-6) && Passed;

  MResponseMatrixO1 Lower(A1);
  Lower.SetAxisNames("X1");
  Lower.SetBinContent(0, 3.0f);
  MResponseMatrixO2 WithSlice("WithSlice", A1, A2);
  WithSlice.SetAxisNames("X1", "X2");
  WithSlice.SetMatrix(0, Lower);
  Passed = EvaluateNear("SetMatrix()", "first slice", "SetMatrix installs the representative lower-order slice", WithSlice.GetBinContent(0, 0), 3.0, 1e-6) && Passed;

  MResponseMatrixO2 Other("Other", A1, A2);
  Other.SetBinContent(0, 0, 2.0f);
  MResponseMatrixO2 Combined = Matrix;
  Combined += Other;
  Passed = EvaluateNear("operator+=()", "matrix addition", "Matrix addition accumulates the representative first bin", Combined.GetBinContent(0, 0), 8.0, 1e-6) && Passed;
  Combined -= Other;
  Passed = EvaluateNear("operator-=()", "matrix subtraction", "Matrix subtraction restores the representative first bin", Combined.GetBinContent(0, 0), 6.0, 1e-6) && Passed;
  MResponseMatrixO2 Divided = Matrix;
  Divided /= Other;
  Passed = EvaluateNear("operator/=()", "matrix division", "Matrix division divides the representative first bin", Divided.GetBinContent(0, 0), 3.0, 1e-6) && Passed;
  MResponseMatrixO2 Scalar("Scalar", A1, A2);
  Scalar += 1.0f;
  Passed = EvaluateNear("operator+=(float)", "scalar addition", "Scalar addition affects every bin", Scalar.GetSum(), 4.0, 1e-6) && Passed;
  Scalar *= 2.0f;
  Passed = EvaluateNear("operator*=(float)", "scalar multiplication", "Scalar multiplication rescales every bin", Scalar.GetSum(), 8.0, 1e-6) && Passed;

  MResponseMatrixO1 Sum1 = Matrix.GetSumMatrixO1(1);
  Passed = EvaluateNear("GetSumMatrixO1()", "representative projection", "GetSumMatrixO1 preserves the representative first-bin content", Sum1.GetBinContent(0), 6.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetSumMatrixO1()", "representative projection empty bin", "GetSumMatrixO1 preserves the representative empty second bin", Sum1.GetBinContent(1), 0.0, 1e-6) && Passed;
  MResponseMatrixO2 Sum2 = Matrix.GetSumMatrixO2(1, 2);
  Passed = EvaluateNear("GetSumMatrixO2()", "identity projection", "The full-order sum projection preserves the representative first-bin content", Sum2.GetBinContent(0, 0), 6.0, 1e-6) && Passed;
  MResponseMatrixO1 Slice = Matrix.GetSliceInterpolated(0.5f, 2);
  Passed = EvaluateNear("GetSliceInterpolated()", "representative slice", "The representative order-1 slice preserves the first-bin content", Slice.GetBinContent(0), 6.0, 1e-6) && Passed;

  MString FileName = "/tmp/UTResponseMatrix/UTResponseMatrixO2.rsp";
  Passed = Evaluate("Write()", "stream round trip", "Writing the representative matrix in stream mode succeeds", Matrix.Write(FileName, true), true) && Passed;
  MResponseMatrixO2 ReadBack;
  Passed = Evaluate("Read()", "stream round trip", "Reading the representative matrix written in stream mode succeeds", ReadBack.Read(FileName), true) && Passed;
  Passed = EvaluateNear("GetBinContent()", "stream round trip", "The representative first-bin content survives a stream round trip", ReadBack.GetBinContent(0, 0), 6.0, 1e-6) && Passed;

  mout.Enable(false);
  __merr.Enable(false);
  TH1* Histogram = Matrix.GetHistogram(MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, false);
  __merr.Enable(true);
  mout.Enable(true);
  Passed = EvaluateTrue("GetHistogram()", "representative histogram", "GetHistogram returns a representative ROOT histogram", Histogram != nullptr) && Passed;
  if (Histogram != nullptr) {
    Passed = EvaluateTrue("GetHistogram()", "representative histogram dimensionality", "GetHistogram returns a two-dimensional histogram when both axes are selected for display", dynamic_cast<TH2*>(Histogram) != nullptr) && Passed;
    Passed = EvaluateNear("GetHistogram()", "representative histogram content", "GetHistogram preserves the representative first-bin content", Histogram->GetBinContent(1, 1), 6.0, 1e-6) && Passed;
    delete Histogram;
  }

  {
    bool WasBatch = gROOT->IsBatch();
    gROOT->SetBatch(true);
    int BeforeCanvases = GetCanvasCount();
    mout.Enable(false);
    __merr.Enable(false);
    Matrix.Show(MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, false);
    __merr.Enable(true);
    mout.Enable(true);
    Passed = Evaluate("Show()", "representative display", "Show creates a ROOT canvas for the representative histogram", GetCanvasCount(), BeforeCanvases + 1) && Passed;
    CleanupCanvases(BeforeCanvases);
    gROOT->SetBatch(WasBatch);
  }

  __merr.Enable(false);
  Passed = Evaluate("Write()", "non-stream mode", "Writing the representative matrix in non-stream mode is rejected explicitly", Matrix.Write("/tmp/UTResponseMatrix/UTResponseMatrixO2_text.rsp", false), false) && Passed;
  MFile File;
  Passed = Evaluate("Open()", "non-stream read setup", "The representative non-stream response-matrix file can be created", File.Open("/tmp/UTResponseMatrix/UTResponseMatrixO2_text_read.rsp", MFile::c_Write), true) && Passed;
  File.Write("# Response Matrix 2\nVersion 1\n\nNM Representative\n\nOD 2\n\nTS 0\n\nSA 0\n\nSM \n\nType ResponseMatrixO2\nR2 0 0 6\n");
  File.Close();
  MResponseMatrixO2 ReadBackText;
  Passed = Evaluate("Read()", "non-stream mode", "Reading the representative matrix in non-stream mode is rejected explicitly", ReadBackText.Read("/tmp/UTResponseMatrix/UTResponseMatrixO2_text_read.rsp"), false) && Passed;
  __merr.Enable(true);

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (argc == 2) {
    vector<float> A2{0.0f, 1.0f, 2.0f};
    vector<float> EmptyAxis;
    vector<float> NonIncreasingAxis{0.0f, 1.0f, 1.0f};
    MResponseMatrixO2 Matrix;
    MString Argument = argv[1];
    if (Argument == "--assert-empty-first-axis") {
      Matrix.SetAxis(EmptyAxis, A2);
      return 0;
    }
    if (Argument == "--assert-nonincreasing-second-axis") {
      Matrix.SetAxis(A2, NonIncreasingAxis);
      return 0;
    }
  }
  UTResponseMatrixO2 Test;
  return Test.Run() == true ? 0 : 1;
}
