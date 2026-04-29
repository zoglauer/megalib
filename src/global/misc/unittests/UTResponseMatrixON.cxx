/*
 * UTResponseMatrixON.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <sys/wait.h>
using namespace std;

// ROOT:
#include <TCanvas.h>
#include <TROOT.h>

// MEGAlib:
#include "MExceptions.h"
#include "MResponseMatrixAxisSpheric.h"
#include "MResponseMatrixON.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for MResponseMatrixON
class UTResponseMatrixON : public MUnitTest
{
public:
  UTResponseMatrixON() : MUnitTest("UTResponseMatrixON") {}
  virtual ~UTResponseMatrixON() {}

  virtual bool Run();

  //! Return the current number of ROOT canvases
  static int GetCanvasCount();
  //! Delete canvases until the requested count is reached
  static void CleanupCanvases(int TargetCount);
  //! Run a child process and require success
  static bool RunChildExpectingSuccess(const MString& Argument);
};


////////////////////////////////////////////////////////////////////////////////


bool UTResponseMatrixON::Run()
{
  bool Passed = true;

  system("mkdir -p /tmp/UTResponseMatrixON");

  Passed = EvaluateTrue("CopyConstructor()", "representative process", "Copy constructing and destroying a representative MResponseMatrixON succeeds", RunChildExpectingSuccess("--copy-constructor")) && Passed;
  Passed = EvaluateTrue("AssignmentOperator()", "representative process", "Assigning and destroying a representative MResponseMatrixON succeeds", RunChildExpectingSuccess("--assignment-operator")) && Passed;
  Passed = EvaluateTrue("CopyConstructor()", "clear independence process", "A representative copy remains usable after the original is cleared", RunChildExpectingSuccess("--copy-after-clear")) && Passed;
  Passed = EvaluateTrue("AssignmentOperator()", "clear independence process", "A representative assigned matrix remains usable after the original is cleared", RunChildExpectingSuccess("--assignment-after-clear")) && Passed;

  MResponseMatrixON Default;
  Passed = Evaluate("GetOrder()", "default constructor", "The default ON matrix starts with order zero", Default.GetOrder(), 0U) && Passed;
  Passed = Evaluate("GetNBins()", "default constructor", "The default ON matrix starts with zero bins", Default.GetNBins(), 0UL) && Passed;
  Passed = Evaluate("GetNumberOfAxes()", "default constructor", "The default ON matrix starts with zero axes", Default.GetNumberOfAxes(), 0U) && Passed;
  Passed = Evaluate("GetNumberOfSparseBins()", "default constructor", "The default ON matrix starts with zero sparse bins", Default.GetNumberOfSparseBins(), 0UL) && Passed;
  Passed = Evaluate("IsSparse()", "default constructor", "The default ON matrix starts in non-sparse mode", Default.IsSparse(), false) && Passed;

  MResponseMatrixON SparseDefault(true);
  Passed = Evaluate("IsSparse()", "sparse constructor", "The sparse ON constructor stores the representative sparse flag", SparseDefault.IsSparse(), true) && Passed;

  MResponseMatrixON CopySource("CopySource");
  CopySource.SetHash(42UL);
  CopySource.SetSimulatedEvents(17);
  CopySource.SetFarFieldStartArea(3.5);
  CopySource.SetSpectrum("Mono", vector<double>{511.0});
  CopySource.AddAxisLinear("Energy", 2, 0.0, 2.0);
  MResponseMatrixON CopyConstructed(CopySource);
  Passed = Evaluate("CopyConstructor()", "base metadata name", "The copy constructor preserves the representative matrix name", CopyConstructed.GetName(), MString("CopySource")) && Passed;
  Passed = Evaluate("CopyConstructor()", "base metadata hash", "The copy constructor preserves the representative matrix hash", CopyConstructed.GetHash(), 42UL) && Passed;
  Passed = Evaluate("CopyConstructor()", "base metadata simulated events", "The copy constructor preserves the representative simulated-event count", CopyConstructed.GetSimulatedEvents(), 17L) && Passed;
  Passed = EvaluateNear("CopyConstructor()", "base metadata area", "The copy constructor preserves the representative far-field area", CopyConstructed.GetFarFieldStartArea(), 3.5, 1e-12) && Passed;
  Passed = Evaluate("CopyConstructor()", "base metadata spectrum", "The copy constructor preserves the representative spectral type", CopyConstructed.GetSpectralType(), MString("Mono")) && Passed;
  Passed = Evaluate("CopyConstructor()", "base metadata spectrum parameters", "The copy constructor preserves the representative spectral parameters", CopyConstructed.GetSpectralParameters().size(), 1UL) && Passed;

  MResponseMatrixON AssignmentSource("AssignmentSource");
  AssignmentSource.SetHash(84UL);
  AssignmentSource.SetSimulatedEvents(23);
  AssignmentSource.SetFarFieldStartArea(7.5);
  AssignmentSource.SetSpectrum("Linear", vector<double>{1.0, 2.0});
  AssignmentSource.AddAxisLinear("Energy", 2, 0.0, 2.0);
  MResponseMatrixON Assigned("Assigned");
  Assigned.AddAxisLinear("Other", 2, 0.0, 2.0);
  Assigned = AssignmentSource;
  Passed = Evaluate("AssignmentOperator()", "base metadata name", "The assignment operator preserves the representative matrix name", Assigned.GetName(), MString("AssignmentSource")) && Passed;
  Passed = Evaluate("AssignmentOperator()", "base metadata hash", "The assignment operator preserves the representative matrix hash", Assigned.GetHash(), 84UL) && Passed;
  Passed = Evaluate("AssignmentOperator()", "base metadata simulated events", "The assignment operator preserves the representative simulated-event count", Assigned.GetSimulatedEvents(), 23L) && Passed;
  Passed = EvaluateNear("AssignmentOperator()", "base metadata area", "The assignment operator preserves the representative far-field area", Assigned.GetFarFieldStartArea(), 7.5, 1e-12) && Passed;
  Passed = Evaluate("AssignmentOperator()", "base metadata spectrum", "The assignment operator preserves the representative spectral type", Assigned.GetSpectralType(), MString("Linear")) && Passed;
  Passed = Evaluate("AssignmentOperator()", "base metadata spectrum parameters", "The assignment operator preserves the representative spectral parameters", Assigned.GetSpectralParameters().size(), 2UL) && Passed;

  MResponseMatrixON Resettable("Resettable");
  Resettable.AddAxisLinear("Energy", 2, 0.0, 2.0);
  Resettable.Set(0UL, 3.0f);
  Resettable.Clear();
  Passed = Evaluate("Clear()", "representative state", "Clear resets the representative ON matrix order", Resettable.GetOrder(), 0U) && Passed;
  Passed = Evaluate("Clear()", "representative state axes", "Clear removes the representative ON matrix axes", Resettable.GetNumberOfAxes(), 0U) && Passed;
  Passed = Evaluate("Clear()", "representative state bins", "Clear resets the representative ON matrix bin count", Resettable.GetNBins(), 0UL) && Passed;

  Default.AddAxisLinear("Energy", 2, 0.0, 2.0);
  Passed = Evaluate("GetOrder()", "AddAxisLinear()", "AddAxisLinear increases the representative order by one", Default.GetOrder(), 1U) && Passed;
  Passed = Evaluate("GetNBins()", "AddAxisLinear()", "AddAxisLinear creates the expected representative number of bins", Default.GetNBins(), 2UL) && Passed;
  Passed = Evaluate("GetNumberOfAxes()", "AddAxisLinear()", "AddAxisLinear creates one representative axis", Default.GetNumberOfAxes(), 1U) && Passed;
  Passed = Evaluate("GetAxisNames()", "AddAxisLinear()", "AddAxisLinear stores the representative axis name", Default.GetAxisNames(0)[0], MString("Energy")) && Passed;
  Passed = EvaluateNear("GetAxis()", "AddAxisLinear()", "AddAxis returns the representative first axis minimum", Default.GetAxis(0).GetMinima()[0], 0.0, 1e-12) && Passed;

  Default.AddAxisLogarithmic("Time", 2, 1.0, 100.0);
  Passed = Evaluate("GetOrder()", "AddAxisLogarithmic()", "AddAxisLogarithmic increases the representative order by one", Default.GetOrder(), 2U) && Passed;
  Passed = Evaluate("GetNBins()", "AddAxisLogarithmic()", "AddAxisLogarithmic multiplies the representative number of bins", Default.GetNBins(), 4UL) && Passed;
  Passed = Evaluate("GetNumberOfAxes()", "AddAxisLogarithmic()", "AddAxisLogarithmic creates a second representative axis", Default.GetNumberOfAxes(), 2U) && Passed;

  MResponseMatrixAxisSpheric Sky("#nu", "#lambda");
  Sky.SetFISBEL(1, 15.0);
  MResponseMatrixON MultiDimensional;
  MultiDimensional.AddAxis(Sky);
  Passed = Evaluate("AddAxis()", "spherical axis", "Adding a representative spherical axis increases the order by its dimension", MultiDimensional.GetOrder(), 2U) && Passed;
  Passed = Evaluate("GetNumberOfAxes()", "spherical axis", "Adding a representative spherical axis creates one axis object", MultiDimensional.GetNumberOfAxes(), 1U) && Passed;
  Passed = Evaluate("GetAxisNames()", "spherical axis", "Adding a representative spherical axis preserves both axis names", MultiDimensional.GetAxisNames(0).size(), 2UL) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetAxisNames()", "out of bounds", "GetAxisNames rejects representative out-of-bounds axis access", [&](){ MultiDimensional.GetAxisNames(1); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetAxis()", "out of bounds", "GetAxis rejects representative out-of-bounds axis access", [&](){ MultiDimensional.GetAxis(1); }) && Passed;

  MResponseMatrixON Matrix("RepresentativeON");
  Matrix.AddAxisLinear("X", 2, 0.0, 2.0);
  Matrix.AddAxisLinear("Y", 2, 0.0, 2.0);
  Matrix.Set(3UL, 6.0f);
  Passed = EvaluateNear("Set(unsigned long)", "representative flat bin", "Set stores representative ON matrix content by flat-bin index", Matrix.Get(3UL), 6.0, 1e-6) && Passed;
  Matrix.Set(3UL, 0.0f);
  Passed = Evaluate("InRange()", "axis values in range", "InRange accepts representative axis values inside all ranges", Matrix.InRange(vector<double>{0.5, 1.5}), true) && Passed;
  {
    DisableDefaultStreams();
    Passed = Evaluate("InRange()", "axis values out of range", "InRange rejects representative axis values outside the configured ranges", Matrix.InRange(vector<double>{-0.5, 1.5}), false) && Passed;
    EnableDefaultStreams();
  }
  Passed = Evaluate("InRange()", "axis bins in range", "InRange accepts representative axis bins inside all ranges", Matrix.InRange(vector<unsigned long>{1, 0}), true) && Passed;
  Passed = Evaluate("InRange()", "axis bins out of range", "InRange rejects representative axis bins outside the configured ranges", Matrix.InRange(vector<unsigned long>{2, 0}), false) && Passed;
  Passed = Evaluate("FindBin()", "representative axis bins", "FindBin maps representative axis bins to the expected flat bin", Matrix.FindBin(vector<unsigned long>{1, 0}), 1UL) && Passed;
  Passed = Evaluate("FindBins()", "representative flat bin", "FindBins inverts the representative flat-bin mapping", Matrix.FindBins(3)[0], 1UL) && Passed;
  Passed = Evaluate("FindBins()", "representative flat bin second axis", "FindBins inverts the representative flat-bin mapping for the second axis", Matrix.FindBins(3)[1], 1UL) && Passed;
  Passed = Evaluate("FindBin()", "representative axis values", "FindBin maps representative axis values to the expected flat bin", Matrix.FindBin(vector<double>{0.5, 1.5}), 2UL) && Passed;
  Passed = Evaluate("FindBins()", "representative axis values", "FindBins maps representative axis values to the expected axis bins", Matrix.FindBins(vector<double>{0.5, 1.5})[1], 1UL) && Passed;

  Matrix.Set(vector<unsigned long>{0, 1}, 5.0f);
  Passed = EvaluateNear("Set()", "representative axis bins", "Set stores the representative bin content by axis bins", Matrix.Get(vector<unsigned long>{0, 1}), 5.0, 1e-6) && Passed;
  Matrix.Set(vector<double>{1.5, 0.5}, 7.0f);
  Passed = EvaluateNear("Set()", "representative axis values", "Set stores the representative bin content by axis values", Matrix.Get(vector<unsigned long>{1, 0}), 7.0, 1e-6) && Passed;
  Matrix.Add(vector<unsigned long>{1, 0}, 2.0f);
  Passed = EvaluateNear("Add()", "representative axis bins", "Add accumulates representative content by axis bins", Matrix.Get(vector<unsigned long>{1, 0}), 9.0, 1e-6) && Passed;
  Matrix.Add(vector<double>{0.5, 1.5}, 3.0f);
  Passed = EvaluateNear("Add()", "representative axis values", "Add accumulates representative content by axis values", Matrix.Get(vector<unsigned long>{0, 1}), 8.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetArea()", "representative values", "GetArea returns the representative Cartesian bin area", Matrix.GetArea(vector<double>{0.5, 1.5}), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaximum()", "representative values", "GetMaximum returns the representative maximum bin content", Matrix.GetMaximum(), 9.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetMinimum()", "representative values", "GetMinimum returns the representative minimum bin content", Matrix.GetMinimum(), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetSum()", "representative values", "GetSum returns the representative total matrix content", Matrix.GetSum(), 17.0, 1e-6) && Passed;
  {
    DisableDefaultStreams();
    Passed = EvaluateNear("GetInterpolated()", "representative values", "GetInterpolated falls back to the representative containing-bin value", Matrix.GetInterpolated(vector<double>{0.5, 1.5}), 8.0, 1e-6) && Passed;
    EnableDefaultStreams();
  }

  MResponseMatrixON BulkMatrix = Matrix;
  Matrix.Add(0UL, 1.0f);
  Passed = EvaluateNear("Add(unsigned long)", "representative flat bin", "Add accumulates representative content by flat bin", Matrix.Get(0UL), 1.0, 1e-6) && Passed;
  BulkMatrix.Add(vector<unsigned long>{2UL, 3UL}, vector<float>{4.0f, 5.0f});
  Passed = EvaluateNear("Add(vector<unsigned long>, vector<float>)", "representative bulk add", "Bulk Add accumulates representative content for multiple bins", BulkMatrix.Get(2UL), 12.0, 1e-6) && Passed;

  MResponseMatrixON Scalar = Matrix;
  Scalar += 1.0f;
  Passed = EvaluateNear("operator+=(float)", "representative scalar", "Adding a representative scalar affects every ON matrix bin", Scalar.Get(3UL), 1.0, 1e-6) && Passed;
  Scalar -= 1.0f;
  Passed = EvaluateNear("operator-=(float)", "representative scalar", "Subtracting a representative scalar restores the original ON matrix bin", Scalar.Get(3UL), 0.0, 1e-6) && Passed;
  Scalar *= 2.0f;
  Passed = EvaluateNear("operator*=(float)", "representative scalar", "Multiplying by a representative scalar rescales every ON matrix bin", Scalar.Get(2UL), 16.0, 1e-6) && Passed;
  Scalar /= 2.0f;
  Passed = EvaluateNear("operator/=(float)", "representative scalar", "Dividing by a representative scalar restores the original ON matrix bin", Scalar.Get(2UL), 8.0, 1e-6) && Passed;

  MResponseMatrixON Other("Other");
  Other.AddAxisLinear("X", 2, 0.0, 2.0);
  Other.AddAxisLinear("Y", 2, 0.0, 2.0);
  Other.Set(vector<unsigned long>{0, 1}, 1.0f);
  Other.Set(vector<unsigned long>{1, 0}, 2.0f);
  Passed = Evaluate("operator==()", "same axes", "Matrices with the same representative axes compare equal", Matrix == Other, true) && Passed;
  MResponseMatrixON MatrixPlus = Matrix;
  MatrixPlus += Other;
  Passed = EvaluateNear("operator+=(matrix)", "representative matrix", "Matrix addition accumulates representative ON matrix content", MatrixPlus.Get(vector<unsigned long>{0, 1}), 9.0, 1e-6) && Passed;
  MResponseMatrixON MatrixMinus = MatrixPlus;
  MatrixMinus -= Other;
  Passed = EvaluateNear("operator-=(matrix)", "representative matrix", "Matrix subtraction restores representative ON matrix content", MatrixMinus.Get(vector<unsigned long>{0, 1}), 8.0, 1e-6) && Passed;
  MResponseMatrixON MatrixDivide = Matrix;
  MatrixDivide /= Other;
  Passed = EvaluateNear("operator/=(matrix)", "representative matrix", "Matrix division divides representative nonzero ON matrix content", MatrixDivide.Get(vector<unsigned long>{0, 1}), 8.0, 1e-6) && Passed;

  MResponseMatrixON Sparse(true);
  Sparse.AddAxisLinear("X", 2, 0.0, 2.0);
  Sparse.AddAxisLinear("Y", 2, 0.0, 2.0);
  Sparse.Set(vector<unsigned long>{1, 0}, 3.0f);
  Passed = Evaluate("GetNumberOfSparseBins()", "representative sparse set", "Setting representative sparse content creates one sparse bin", Sparse.GetNumberOfSparseBins(), 1UL) && Passed;
  Passed = EvaluateNear("GetSparse()", "representative sparse set", "GetSparse returns the representative sparse content", Sparse.GetSparse(0), 3.0, 1e-6) && Passed;
  Passed = Evaluate("FindBinsSparse()", "representative sparse set", "FindBinsSparse maps the representative sparse bin back to the axis bins", Sparse.FindBinsSparse(0)[0], 1UL) && Passed;
  Sparse.AddSparse(0, 2.0f);
  Passed = EvaluateNear("AddSparse()", "representative sparse add", "AddSparse accumulates representative sparse content", Sparse.GetSparse(0), 5.0, 1e-6) && Passed;
  Sparse.SetSparse(0, 4.0f);
  Passed = EvaluateNear("SetSparse()", "representative sparse set", "SetSparse overwrites representative sparse content", Sparse.GetSparse(0), 4.0, 1e-6) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetSparse()", "out of bounds", "GetSparse rejects representative out-of-bounds sparse-bin access", [&](){ Sparse.GetSparse(1); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("SetSparse()", "out of bounds", "SetSparse rejects representative out-of-bounds sparse-bin access", [&](){ Sparse.SetSparse(1, 1.0f); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("AddSparse()", "out of bounds", "AddSparse rejects representative out-of-bounds sparse-bin access", [&](){ Sparse.AddSparse(1, 1.0f); }) && Passed;
  Sparse.SwitchToNonSparse();
  Passed = Evaluate("IsSparse()", "SwitchToNonSparse()", "SwitchToNonSparse converts the representative sparse matrix to non-sparse mode", Sparse.IsSparse(), false) && Passed;
  Passed = EvaluateNear("SwitchToNonSparse()", "representative content", "SwitchToNonSparse preserves representative sparse content", Sparse.Get(vector<unsigned long>{1, 0}), 4.0, 1e-6) && Passed;
  Sparse.SwitchToSparse();
  Passed = Evaluate("IsSparse()", "SwitchToSparse()", "SwitchToSparse converts the representative matrix back to sparse mode", Sparse.IsSparse(), true) && Passed;
  Passed = Evaluate("GetNumberOfSparseBins()", "SwitchToSparse()", "SwitchToSparse preserves the representative number of sparse bins", Sparse.GetNumberOfSparseBins(), 1UL) && Passed;

  MResponseMatrixON CollapseSource("Collapse");
  CollapseSource.AddAxisLinear("X", 2, 0.0, 2.0);
  CollapseSource.AddAxisLinear("Y", 2, 0.0, 2.0);
  CollapseSource.Set(vector<unsigned long>{0, 0}, 1.0f);
  CollapseSource.Set(vector<unsigned long>{1, 0}, 2.0f);
  CollapseSource.Set(vector<unsigned long>{0, 1}, 3.0f);
  CollapseSource.Set(vector<unsigned long>{1, 1}, 4.0f);
  MResponseMatrixON Collapsed = CollapseSource.Collapse(vector<bool>{false, true});
  Passed = Evaluate("Collapse()", "representative axes", "Collapse keeps the expected representative number of axes", Collapsed.GetNumberOfAxes(), 1U) && Passed;
  Passed = EvaluateNear("Collapse()", "representative content first bin", "Collapse sums the representative collapsed content for the first bin", Collapsed.Get(vector<unsigned long>{0}), 4.0, 1e-6) && Passed;
  Passed = EvaluateNear("Collapse()", "representative content second bin", "Collapse sums the representative collapsed content for the second bin", Collapsed.Get(vector<unsigned long>{1}), 6.0, 1e-6) && Passed;

  MString SparseFile = "/tmp/UTResponseMatrixON/representative_sparse.rsp";
  {
    DisableDefaultStreams();
    Passed = Evaluate("Write()", "representative sparse round trip", "Writing the representative sparse ON matrix succeeds", CollapseSource.Write(SparseFile, false), true) && Passed;
    MResponseMatrixON SparseReadBack;
    Passed = Evaluate("Read()", "representative sparse round trip", "Reading the representative sparse ON matrix succeeds", SparseReadBack.Read(SparseFile), true) && Passed;
    EnableDefaultStreams();
    Passed = EvaluateNear("Read()", "representative sparse round trip content", "The representative sparse ON matrix content survives a round trip", SparseReadBack.Get(vector<unsigned long>{1, 1}), 4.0, 1e-6) && Passed;
  }

  MResponseMatrixON StreamMatrix("Stream");
  StreamMatrix.AddAxisLinear("X", 2, 0.0, 2.0);
  StreamMatrix.AddAxisLinear("Y", 2, 0.0, 2.0);
  StreamMatrix.Set(vector<unsigned long>{0, 0}, 2.0f);
  MString StreamFile = "/tmp/UTResponseMatrixON/representative_stream.rsp";
  {
    DisableDefaultStreams();
    Passed = Evaluate("Write()", "representative stream round trip", "Writing the representative stream ON matrix succeeds", StreamMatrix.Write(StreamFile, true), true) && Passed;
    MResponseMatrixON StreamReadBack;
    Passed = Evaluate("Read()", "representative stream round trip", "Reading the representative stream ON matrix succeeds", StreamReadBack.Read(StreamFile), true) && Passed;
    EnableDefaultStreams();
    Passed = EvaluateNear("Read()", "representative stream round trip content", "The representative stream ON matrix content survives a round trip", StreamReadBack.Get(vector<unsigned long>{0, 0}), 2.0, 1e-6) && Passed;
  }

  MResponseMatrixON ReusedRead("ReusedRead");
  {
    DisableDefaultStreams();
    Passed = Evaluate("Read()", "reused read first file", "Reading the first representative ON file into the same object succeeds", ReusedRead.Read(StreamFile), true) && Passed;
    Passed = Evaluate("Read()", "reused read second file", "Reading a second representative ON file into the same object succeeds", ReusedRead.Read(SparseFile), true) && Passed;
    EnableDefaultStreams();
  }
  Passed = Evaluate("GetNumberOfAxes()", "reused read second file", "Reading a second ON file into the same object replaces the representative axes", ReusedRead.GetNumberOfAxes(), 2U) && Passed;
  Passed = EvaluateNear("Read()", "reused read second file content", "Reading a second ON file into the same object replaces the representative bin content", ReusedRead.Get(vector<unsigned long>{1, 1}), 4.0, 1e-6) && Passed;

  Passed = EvaluateTrue("GetStatistics()", "representative text", "GetStatistics emits representative axis and content information", CollapseSource.GetStatistics().Contains("Number of axes:           2")) && Passed;

  MResponseMatrixON SmoothSource("Smooth");
  SmoothSource.AddAxisLinear("X", 2, 0.0, 2.0);
  SmoothSource.AddAxisLinear("Y", 2, 0.0, 2.0);
  SmoothSource.Set(vector<unsigned long>{0, 0}, 2.0f);
  SmoothSource.Smooth(0);
  Passed = EvaluateNear("Smooth()", "zero times", "Smooth with representative zero iterations leaves the ON matrix unchanged", SmoothSource.Get(vector<unsigned long>{0, 0}), 2.0, 1e-6) && Passed;

  {
    bool WasBatch = gROOT->IsBatch();
    gROOT->SetBatch(true);
    int BeforeCanvases = GetCanvasCount();
    DisableDefaultStreams();
    CollapseSource.ShowSlice(vector<float>{MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY}, true, "Representative slice");
    EnableDefaultStreams();
    Passed = Evaluate("ShowSlice()", "representative display", "ShowSlice creates a representative ROOT canvas", GetCanvasCount(), BeforeCanvases + 1) && Passed;
    CleanupCanvases(BeforeCanvases);
    gROOT->SetBatch(WasBatch);
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTResponseMatrixON::GetCanvasCount()
{
  return gROOT != 0 && gROOT->GetListOfCanvases() != 0 ? gROOT->GetListOfCanvases()->GetSize() : 0;
}


////////////////////////////////////////////////////////////////////////////////


void UTResponseMatrixON::CleanupCanvases(int TargetCount)
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


bool UTResponseMatrixON::RunChildExpectingSuccess(const MString& Argument)
{
  int Status = RunChildProcess("bin/UTResponseMatrixON", Argument, "/dev/null");
  return WIFEXITED(Status) && WEXITSTATUS(Status) == 0;
}


int main(int argc, char** argv)
{
  if (argc == 2) {
    MString Argument = argv[1];
    if (Argument == "--copy-constructor") {
      MResponseMatrixON Original("Original");
      Original.AddAxisLinear("X", 2, 0.0, 2.0);
      Original.AddAxisLinear("Y", 2, 0.0, 2.0);
      Original.Set(vector<unsigned long>{1, 1}, 3.0f);
      MResponseMatrixON Copy(Original);
      return Copy.Get(vector<unsigned long>{1, 1}) == 3.0f ? 0 : 1;
    }
    if (Argument == "--assignment-operator") {
      MResponseMatrixON Original("Original");
      Original.AddAxisLinear("X", 2, 0.0, 2.0);
      Original.AddAxisLinear("Y", 2, 0.0, 2.0);
      Original.Set(vector<unsigned long>{0, 1}, 4.0f);
      MResponseMatrixON Assigned;
      Assigned = Original;
      return Assigned.Get(vector<unsigned long>{0, 1}) == 4.0f ? 0 : 1;
    }
    if (Argument == "--copy-after-clear") {
      MResponseMatrixON Original("Original");
      Original.AddAxisLinear("X", 2, 0.0, 2.0);
      MResponseMatrixON Copy(Original);
      Original.Clear();
      return Copy.GetAxisNames(0)[0] == "X" ? 0 : 1;
    }
    if (Argument == "--assignment-after-clear") {
      MResponseMatrixON Original("Original");
      Original.AddAxisLinear("X", 2, 0.0, 2.0);
      MResponseMatrixON Assigned;
      Assigned = Original;
      Original.Clear();
      return Assigned.GetAxisNames(0)[0] == "X" ? 0 : 1;
    }
  }

  UTResponseMatrixON Test;
  return Test.Run() == true ? 0 : 1;
}
