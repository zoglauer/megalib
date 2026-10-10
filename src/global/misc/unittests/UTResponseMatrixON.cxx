/*
 * UTResponseMatrixON.cxx
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
#include <vector>

// POSIX libs:
#include <sys/wait.h>
using namespace std;

// ROOT:
#include <TCanvas.h>
#include <TROOT.h>

// MEGAlib:
#include "MExceptions.h"
#include "MFile.h"
#include "MResponseMatrixAxisSpheric.h"
#include "MResponseMatrixON.h"
#include "MStreams.h"
#include "MSystem.h"
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

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "temporary directory", "The temporary directory for MResponseMatrixON fixtures can be created", PrepareTemporaryDirectory()) && Passed;

  Passed = EvaluateTrue("CopyConstructor()", "representative process", "Copy constructing and destroying a representative MResponseMatrixON succeeds", RunChildExpectingSuccess("--copy-constructor")) && Passed;
  Passed = EvaluateTrue("AssignmentOperator()", "representative process", "Assigning and destroying a representative MResponseMatrixON succeeds", RunChildExpectingSuccess("--assignment-operator")) && Passed;
  Passed = EvaluateTrue("CopyConstructor()", "clear independence process", "A representative copy remains usable after the original is cleared", RunChildExpectingSuccess("--copy-after-clear")) && Passed;
  Passed = EvaluateTrue("AssignmentOperator()", "clear independence process", "A representative assigned matrix remains usable after the original is cleared", RunChildExpectingSuccess("--assignment-after-clear")) && Passed;

  MResponseMatrixON Default;
  Passed = Evaluate("GetOrder()", "default constructor", "The default ON matrix starts with order zero", Default.GetOrder(), 0U) && Passed;
  Passed = Evaluate("GetNBins()", "default constructor", "The default ON matrix starts with zero bins", Default.GetNBins(), 0UL) && Passed;
  Passed = Evaluate("GetNumberOfAxes()", "default constructor", "The default ON matrix starts with zero axes", Default.GetNumberOfAxes(), 0U) && Passed;
  Passed = Evaluate("IsSparse()", "default constructor", "The default ON matrix starts in non-sparse mode", Default.IsSparse(), false) && Passed;

  MResponseMatrixON SparseDefault(true);
  Passed = Evaluate("IsSparse()", "sparse constructor", "The sparse ON constructor stores the representative sparse flag", SparseDefault.IsSparse(), true) && Passed;

  MResponseMatrixON CopySource("CopySource");
  CopySource.SetHash(42UL);
  CopySource.SetSimulatedEvents(17);
  CopySource.SetFarFieldStartArea(3.5);
  CopySource.SetSpectralType("Mono");
  CopySource.AddAxisLinear("Energy", 2, 0.0, 2.0);
  MResponseMatrixON CopyConstructed(CopySource);
  Passed = Evaluate("CopyConstructor()", "base metadata name", "The copy constructor preserves the representative matrix name", CopyConstructed.GetName(), MString("CopySource")) && Passed;
  Passed = Evaluate("CopyConstructor()", "base metadata hash", "The copy constructor preserves the representative matrix hash", CopyConstructed.GetHash(), 42UL) && Passed;
  Passed = Evaluate("CopyConstructor()", "base metadata simulated events", "The copy constructor preserves the representative simulated-event count", CopyConstructed.GetSimulatedEvents(), 17L) && Passed;
  Passed = EvaluateNear("CopyConstructor()", "base metadata area", "The copy constructor preserves the representative far-field area", CopyConstructed.GetFarFieldStartArea(), 3.5, 1e-12) && Passed;
  Passed = Evaluate("CopyConstructor()", "base metadata spectrum", "The copy constructor preserves the representative spectral type", CopyConstructed.GetSpectralType(), MString("Mono")) && Passed;

  MResponseMatrixON AssignmentSource("AssignmentSource");
  AssignmentSource.SetHash(84UL);
  AssignmentSource.SetSimulatedEvents(23);
  AssignmentSource.SetFarFieldStartArea(7.5);
  AssignmentSource.SetSpectralType("Linear");
  AssignmentSource.AddAxisLinear("Energy", 2, 0.0, 2.0);
  MResponseMatrixON Assigned("Assigned");
  Assigned.AddAxisLinear("Other", 2, 0.0, 2.0);
  Assigned = AssignmentSource;
  Passed = Evaluate("AssignmentOperator()", "base metadata name", "The assignment operator preserves the representative matrix name", Assigned.GetName(), MString("AssignmentSource")) && Passed;
  Passed = Evaluate("AssignmentOperator()", "base metadata hash", "The assignment operator preserves the representative matrix hash", Assigned.GetHash(), 84UL) && Passed;
  Passed = Evaluate("AssignmentOperator()", "base metadata simulated events", "The assignment operator preserves the representative simulated-event count", Assigned.GetSimulatedEvents(), 23L) && Passed;
  Passed = EvaluateNear("AssignmentOperator()", "base metadata area", "The assignment operator preserves the representative far-field area", Assigned.GetFarFieldStartArea(), 7.5, 1e-12) && Passed;
  Passed = Evaluate("AssignmentOperator()", "base metadata spectrum", "The assignment operator preserves the representative spectral type", Assigned.GetSpectralType(), MString("Linear")) && Passed;

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
  Sky.SetFISBELByNumberOfBins(1, 15.0);
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
  Passed = EvaluateNear("Set(unsigned long)", "representative flat bin", "Set stores representative ON matrix content by flat-bin index", Matrix.Get(3UL), 6.0, 1e-12) && Passed;
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
  Passed = EvaluateNear("Set()", "representative axis bins", "Set stores the representative bin content by axis bins", Matrix.Get(vector<unsigned long>{0, 1}), 5.0, 1e-12) && Passed;
  Matrix.Set(vector<double>{1.5, 0.5}, 7.0f);
  Passed = EvaluateNear("Set()", "representative axis values", "Set stores the representative bin content by axis values", Matrix.Get(vector<unsigned long>{1, 0}), 7.0, 1e-12) && Passed;
  Matrix.Add(vector<unsigned long>{1, 0}, 2.0f);
  Passed = EvaluateNear("Add()", "representative axis bins", "Add accumulates representative content by axis bins", Matrix.Get(vector<unsigned long>{1, 0}), 9.0, 1e-12) && Passed;
  Matrix.Add(vector<double>{0.5, 1.5}, 3.0f);
  Passed = EvaluateNear("Add()", "representative axis values", "Add accumulates representative content by axis values", Matrix.Get(vector<unsigned long>{0, 1}), 8.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetArea()", "representative values", "GetArea returns the representative Cartesian bin area", Matrix.GetArea(vector<double>{0.5, 1.5}), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaximum()", "representative values", "GetMaximum returns the representative maximum bin content", Matrix.GetMaximum(), 9.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinimum()", "representative values", "GetMinimum returns the representative minimum bin content", Matrix.GetMinimum(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetSum()", "representative values", "GetSum returns the representative total matrix content", Matrix.GetSum(), 17.0, 1e-12) && Passed;
  {
    DisableDefaultStreams();
    Passed = EvaluateNear("GetInterpolated()", "representative values", "GetInterpolated falls back to the representative containing-bin value", Matrix.GetInterpolated(vector<double>{0.5, 1.5}), 8.0, 1e-12) && Passed;
    EnableDefaultStreams();
  }

  MResponseMatrixON BulkMatrix = Matrix;
  Matrix.Add(0UL, 1.0f);
  Passed = EvaluateNear("Add(unsigned long)", "representative flat bin", "Add accumulates representative content by flat bin", Matrix.Get(0UL), 1.0, 1e-12) && Passed;
  BulkMatrix.Add(vector<unsigned long>{2UL, 3UL}, vector<float>{4.0f, 5.0f});
  Passed = EvaluateNear("Add(vector<unsigned long>, vector<float>)", "representative bulk add", "Bulk Add accumulates representative content for multiple bins", BulkMatrix.Get(2UL), 12.0, 1e-12) && Passed;

  MResponseMatrixON Scalar = Matrix;
  Scalar += 1.0f;
  Passed = EvaluateNear("operator+=(float)", "representative scalar", "Adding a representative scalar affects every ON matrix bin", Scalar.Get(3UL), 1.0, 1e-12) && Passed;
  Scalar -= 1.0f;
  Passed = EvaluateNear("operator-=(float)", "representative scalar", "Subtracting a representative scalar restores the original ON matrix bin", Scalar.Get(3UL), 0.0, 1e-12) && Passed;
  Scalar *= 2.0f;
  Passed = EvaluateNear("operator*=(float)", "representative scalar", "Multiplying by a representative scalar rescales every ON matrix bin", Scalar.Get(2UL), 16.0, 1e-12) && Passed;
  Scalar /= 2.0f;
  Passed = EvaluateNear("operator/=(float)", "representative scalar", "Dividing by a representative scalar restores the original ON matrix bin", Scalar.Get(2UL), 8.0, 1e-12) && Passed;

  MResponseMatrixON Other("Other");
  Other.AddAxisLinear("X", 2, 0.0, 2.0);
  Other.AddAxisLinear("Y", 2, 0.0, 2.0);
  Other.Set(vector<unsigned long>{0, 1}, 1.0f);
  Other.Set(vector<unsigned long>{1, 0}, 2.0f);
  Passed = Evaluate("operator==()", "same axes", "Matrices with the same representative axes compare equal", Matrix == Other, true) && Passed;
  MResponseMatrixON MatrixPlus = Matrix;
  MatrixPlus += Other;
  Passed = EvaluateNear("operator+=(matrix)", "representative matrix", "Matrix addition accumulates representative ON matrix content", MatrixPlus.Get(vector<unsigned long>{0, 1}), 9.0, 1e-12) && Passed;
  MResponseMatrixON MatrixMinus = MatrixPlus;
  MatrixMinus -= Other;
  Passed = EvaluateNear("operator-=(matrix)", "representative matrix", "Matrix subtraction restores representative ON matrix content", MatrixMinus.Get(vector<unsigned long>{0, 1}), 8.0, 1e-12) && Passed;
  MResponseMatrixON MatrixDivide = Matrix;
  MatrixDivide /= Other;
  Passed = EvaluateNear("operator/=(matrix)", "representative matrix", "Matrix division divides representative nonzero ON matrix content", MatrixDivide.Get(vector<unsigned long>{0, 1}), 8.0, 1e-12) && Passed;

  MResponseMatrixON Sparse(true);
  Sparse.AddAxisLinear("X", 2, 0.0, 2.0);
  Sparse.AddAxisLinear("Y", 2, 0.0, 2.0);
  Sparse.Set(vector<unsigned long>{1, 0}, 3.0f);
  Passed = EvaluateNear("Get()", "representative sparse set", "Get returns the representative sparse content", Sparse.Get(vector<unsigned long>{1, 0}), 3.0, 1e-12) && Passed;
  Sparse.Add(vector<unsigned long>{1, 0}, 2.0f);
  Passed = EvaluateNear("Add()", "representative sparse add", "Add accumulates representative sparse content", Sparse.Get(vector<unsigned long>{1, 0}), 5.0, 1e-12) && Passed;
  Sparse.Set(vector<unsigned long>{1, 0}, 4.0f);
  Passed = EvaluateNear("Set()", "representative sparse overwrite", "Set overwrites representative sparse content", Sparse.Get(vector<unsigned long>{1, 0}), 4.0, 1e-12) && Passed;
  Sparse.SwitchToNonSparse();
  Passed = Evaluate("IsSparse()", "SwitchToNonSparse()", "SwitchToNonSparse converts the representative sparse matrix to non-sparse mode", Sparse.IsSparse(), false) && Passed;
  Passed = EvaluateNear("SwitchToNonSparse()", "representative content", "SwitchToNonSparse preserves representative sparse content", Sparse.Get(vector<unsigned long>{1, 0}), 4.0, 1e-12) && Passed;
  Sparse.SwitchToSparse();
  Passed = Evaluate("IsSparse()", "SwitchToSparse()", "SwitchToSparse converts the representative matrix back to sparse mode", Sparse.IsSparse(), true) && Passed;

  MResponseMatrixON CollapseSource("Collapse");
  CollapseSource.AddAxisLinear("X", 2, 0.0, 2.0);
  CollapseSource.AddAxisLinear("Y", 2, 0.0, 2.0);
  CollapseSource.Set(vector<unsigned long>{0, 0}, 1.0f);
  CollapseSource.Set(vector<unsigned long>{1, 0}, 2.0f);
  CollapseSource.Set(vector<unsigned long>{0, 1}, 3.0f);
  CollapseSource.Set(vector<unsigned long>{1, 1}, 4.0f);
  MResponseMatrixON Collapsed = CollapseSource.Collapse(vector<bool>{false, true});
  Passed = Evaluate("Collapse()", "representative axes", "Collapse keeps the expected representative number of axes", Collapsed.GetNumberOfAxes(), 1U) && Passed;
  Passed = EvaluateNear("Collapse()", "representative content first bin", "Collapse sums the representative collapsed content for the first bin", Collapsed.Get(vector<unsigned long>{0}), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Collapse()", "representative content second bin", "Collapse sums the representative collapsed content for the second bin", Collapsed.Get(vector<unsigned long>{1}), 6.0, 1e-12) && Passed;

  MString SparseFile = GetTemporaryFileName("representative_sparse.rsp");
  {
    DisableDefaultStreams();
    Passed = Evaluate("Write()", "representative sparse round trip", "Writing the representative sparse ON matrix succeeds", CollapseSource.Write(SparseFile, false), true) && Passed;
    MResponseMatrixON SparseReadBack;
    Passed = Evaluate("Read()", "representative sparse round trip", "Reading the representative sparse ON matrix succeeds", SparseReadBack.Read(SparseFile), true) && Passed;
    EnableDefaultStreams();
    Passed = EvaluateNear("Read()", "representative sparse round trip content", "The representative sparse ON matrix content survives a round trip", SparseReadBack.Get(vector<unsigned long>{1, 1}), 4.0, 1e-12) && Passed;
  }

  MResponseMatrixON StreamMatrix("Stream");
  StreamMatrix.AddAxisLinear("X", 2, 0.0, 2.0);
  StreamMatrix.AddAxisLinear("Y", 2, 0.0, 2.0);
  StreamMatrix.Set(vector<unsigned long>{0, 0}, 2.0f);
  MString StreamFile = GetTemporaryFileName("representative_stream.rsp");
  {
    DisableDefaultStreams();
    Passed = Evaluate("Write()", "representative stream round trip", "Writing the representative stream ON matrix succeeds", StreamMatrix.Write(StreamFile, true), true) && Passed;
    MResponseMatrixON StreamReadBack;
    Passed = Evaluate("Read()", "representative stream round trip", "Reading the representative stream ON matrix succeeds", StreamReadBack.Read(StreamFile), true) && Passed;
    EnableDefaultStreams();
    Passed = EvaluateNear("Read()", "representative stream round trip content", "The representative stream ON matrix content survives a round trip", StreamReadBack.Get(vector<unsigned long>{0, 0}), 2.0, 1e-12) && Passed;
  }

  MResponseMatrixON ReusedRead("ReusedRead");
  {
    DisableDefaultStreams();
    Passed = Evaluate("Read()", "reused read first file", "Reading the first representative ON file into the same object succeeds", ReusedRead.Read(StreamFile), true) && Passed;
    Passed = Evaluate("Read()", "reused read second file", "Reading a second representative ON file into the same object succeeds", ReusedRead.Read(SparseFile), true) && Passed;
    EnableDefaultStreams();
  }
  Passed = Evaluate("GetNumberOfAxes()", "reused read second file", "Reading a second ON file into the same object replaces the representative axes", ReusedRead.GetNumberOfAxes(), 2U) && Passed;
  Passed = EvaluateNear("Read()", "reused read second file content", "Reading a second ON file into the same object replaces the representative bin content", ReusedRead.Get(vector<unsigned long>{1, 1}), 4.0, 1e-12) && Passed;

  // Expected: 4 non-zero bins (sparseness 0 %), maximum 4, minimum 1, sum 10, average 10/4 = 2.5
  MString ExpectedStatistics("\nStatistics for response matrix \"Collapse\":\n\nNumber of axes:           2\nNumber of dimensions:     2\nNumber of bins:           4\nNumber of non-zero bins:  4\nSparseness:               0 %\nMaximum:                  4\nMinimum:                  1\nSum:                      10\nAverage value:            2.5\n\nAxes:\n  x0:  X (from 0 to 2 with 2 bins)\n  x1:  Y (from 0 to 2 with 2 bins)\n");
  Passed = Evaluate("GetStatistics()", "representative text", "GetStatistics emits exactly the representative counts, extrema, sum, average, and axis descriptions", CollapseSource.GetStatistics(), ExpectedStatistics) && Passed;

  MResponseMatrixON SmoothSource("Smooth");
  SmoothSource.AddAxisLinear("X", 2, 0.0, 2.0);
  SmoothSource.AddAxisLinear("Y", 2, 0.0, 2.0);
  SmoothSource.Set(vector<unsigned long>{0, 0}, 2.0f);
  SmoothSource.Smooth(0);
  Passed = EvaluateNear("Smooth()", "zero times", "Smooth with representative zero iterations leaves the ON matrix unchanged", SmoothSource.Get(vector<unsigned long>{0, 0}), 2.0, 1e-12) && Passed;

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

  // Sparse matrix: reading with one thread and with several threads gives the same content
  {
    MResponseMatrixON SparseSource("SparseSource", true);
    SparseSource.AddAxisLinear("X", 3, 0.0, 3.0);
    SparseSource.AddAxisLinear("Y", 3, 0.0, 3.0);
    SparseSource.Set(vector<unsigned long>{0, 1}, 2.5f);
    SparseSource.Set(vector<unsigned long>{2, 2}, 4.0f);
    SparseSource.Set(vector<unsigned long>{1, 0}, -1.5f);
    MString SparseThreadFile = GetTemporaryFileName("sparse_threads.rsp");
    MResponseMatrixON SingleThread;
    MResponseMatrixON MultiThread;
    DisableDefaultStreams();
    const bool WrittenSparse = SparseSource.Write(SparseThreadFile, false);
    const bool ReadSingle = SingleThread.Read(SparseThreadFile, false);
    const bool ReadMulti = MultiThread.Read(SparseThreadFile, true);
    EnableDefaultStreams();
    Passed = Evaluate("Write()", "sparse threads", "Writing the sparse ON matrix succeeds", WrittenSparse, true) && Passed;
    Passed = Evaluate("Read(single)", "sparse threads", "Reading the sparse ON matrix with one thread succeeds", ReadSingle, true) && Passed;
    Passed = Evaluate("Read(multi)", "sparse threads", "Reading the sparse ON matrix with several threads succeeds", ReadMulti, true) && Passed;
    Passed = Evaluate("IsSparse()", "sparse threads single", "The matrix read with one thread is sparse", SingleThread.IsSparse(), true) && Passed;
    Passed = Evaluate("IsSparse()", "sparse threads multi", "The matrix read with several threads is sparse", MultiThread.IsSparse(), true) && Passed;
    Passed = EvaluateNear("GetSum()", "sparse threads single", "The sum is 2.5 + 4 - 1.5 (one thread)", SingleThread.GetSum(), 5.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetSum()", "sparse threads multi", "The sum is 2.5 + 4 - 1.5 (several threads)", MultiThread.GetSum(), 5.0, 1e-12) && Passed;
    Passed = EvaluateNear("Get()", "sparse threads single 0,1", "The content of bin (0,1) survives (one thread)", SingleThread.Get(vector<unsigned long>{0, 1}), 2.5, 1e-12) && Passed;
    Passed = EvaluateNear("Get()", "sparse threads multi 0,1", "The content of bin (0,1) survives (several threads)", MultiThread.Get(vector<unsigned long>{0, 1}), 2.5, 1e-12) && Passed;
    Passed = EvaluateNear("Get()", "sparse threads single 1,0", "The negative content of bin (1,0) survives (one thread)", SingleThread.Get(vector<unsigned long>{1, 0}), -1.5, 1e-12) && Passed;
    Passed = EvaluateNear("Get()", "sparse threads multi 1,0", "The negative content of bin (1,0) survives (several threads)", MultiThread.Get(vector<unsigned long>{1, 0}), -1.5, 1e-12) && Passed;
    Passed = EvaluateNear("Get()", "sparse threads multi 2,2", "The content of bin (2,2) survives (several threads)", MultiThread.Get(vector<unsigned long>{2, 2}), 4.0, 1e-12) && Passed;
    Passed = EvaluateNear("Get()", "sparse threads multi empty bin", "A bin which was never set is zero (several threads)", MultiThread.Get(vector<unsigned long>{1, 1}), 0.0, 1e-12) && Passed;
  }

  // Sparse matrix with only negative values: the implicit zeros are the maximum
  {
    MResponseMatrixON Negative("Negative", true);
    Negative.AddAxisLinear("X", 2, 0.0, 2.0);
    Negative.AddAxisLinear("Y", 2, 0.0, 2.0);
    Negative.Set(vector<unsigned long>{0, 0}, -3.0f);
    Negative.Set(vector<unsigned long>{1, 1}, -1.0f);
    Passed = EvaluateNear("GetMaximum()", "sparse negative", "The maximum of a sparse matrix with negative values and empty bins is zero", Negative.GetMaximum(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMinimum()", "sparse negative", "The minimum of a sparse matrix with negative values is the most negative value", Negative.GetMinimum(), -3.0, 1e-12) && Passed;
    Passed = EvaluateTrue("GetStatistics()", "sparse negative maximum", "The statistics report the same maximum as GetMaximum() (zero)", Negative.GetStatistics().Contains("\nMaximum:                  0\n")) && Passed;
    Passed = EvaluateTrue("GetStatistics()", "sparse negative minimum", "The statistics report the same minimum as GetMinimum() (-3)", Negative.GetStatistics().Contains("\nMinimum:                  -3\n")) && Passed;
  }

  // Empty sparse matrix: all extrema are zero and it can be written and read back
  {
    MResponseMatrixON EmptySparse("EmptySparse", true);
    EmptySparse.AddAxisLinear("X", 2, 0.0, 2.0);
    EmptySparse.AddAxisLinear("Y", 2, 0.0, 2.0);
    Passed = EvaluateNear("GetMaximum()", "sparse empty", "The maximum of an empty sparse matrix is zero", EmptySparse.GetMaximum(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetMinimum()", "sparse empty", "The minimum of an empty sparse matrix is zero", EmptySparse.GetMinimum(), 0.0, 1e-12) && Passed;
    Passed = EvaluateTrue("GetStatistics()", "sparse empty maximum", "The statistics report a maximum of zero for an empty sparse matrix", EmptySparse.GetStatistics().Contains("\nMaximum:                  0\n")) && Passed;
    MString EmptyFile = GetTemporaryFileName("sparse_empty.rsp");
    MResponseMatrixON EmptySingle;
    MResponseMatrixON EmptyMulti;
    DisableDefaultStreams();
    const bool WrittenEmpty = EmptySparse.Write(EmptyFile, false);
    const bool ReadEmptySingle = EmptySingle.Read(EmptyFile, false);
    const bool ReadEmptyMulti = EmptyMulti.Read(EmptyFile, true);
    EnableDefaultStreams();
    Passed = Evaluate("Write()", "sparse empty", "Writing the empty sparse ON matrix succeeds", WrittenEmpty, true) && Passed;
    Passed = Evaluate("Read(single)", "sparse empty", "Reading the empty sparse ON matrix with one thread succeeds", ReadEmptySingle, true) && Passed;
    Passed = Evaluate("Read(multi)", "sparse empty", "Reading the empty sparse ON matrix with several threads succeeds", ReadEmptyMulti, true) && Passed;
    Passed = EvaluateNear("GetSum()", "sparse empty multi", "The sum of the empty sparse matrix read with several threads is zero", EmptyMulti.GetSum(), 0.0, 1e-12) && Passed;
  }

  // Sparse file without any data line, read with several threads
  {
    MString NoDataFile = GetTemporaryFileName("sparse_no_data.rsp");
    Passed = EvaluateTrue("WriteTextFile()", "sparse no data file", "The sparse ON file without data lines can be written",
                          WriteTextFile(NoDataFile, "Version 1\nNM NoData\nOD 1\nTS 0\nSA 0\nCE false\n\nAN \"X\"\nAT 1D BinEdges\nAD 0 1 2\n\nType ResponseMatrixONSparse\n\n")) && Passed;
    MResponseMatrixON NoDataSingle;
    MResponseMatrixON NoDataMulti;
    DisableDefaultStreams();
    const bool ReadNoDataSingle = NoDataSingle.Read(NoDataFile, false);
    const bool ReadNoDataMulti = NoDataMulti.Read(NoDataFile, true);
    EnableDefaultStreams();
    Passed = Evaluate("Read(single)", "sparse no data", "Reading a sparse ON file without data lines with one thread succeeds", ReadNoDataSingle, true) && Passed;
    Passed = Evaluate("Read(multi)", "sparse no data", "Reading a sparse ON file without data lines with several threads succeeds", ReadNoDataMulti, true) && Passed;
    Passed = EvaluateNear("GetSum()", "sparse no data multi", "A sparse ON file without data lines has the sum zero (several threads)", NoDataMulti.GetSum(), 0.0, 1e-12) && Passed;

    // The same file ending directly after the Type line
    MString NoLineFile = GetTemporaryFileName("sparse_no_line.rsp");
    Passed = EvaluateTrue("WriteTextFile()", "sparse no line file", "The sparse ON file ending after the type line can be written",
                          WriteTextFile(NoLineFile, "Version 1\nNM NoLine\nOD 1\nTS 0\nSA 0\nCE false\n\nAN \"X\"\nAT 1D BinEdges\nAD 0 1 2\n\nType ResponseMatrixONSparse\n")) && Passed;
    MResponseMatrixON NoLineMulti;
    DisableDefaultStreams();
    const bool ReadNoLineMulti = NoLineMulti.Read(NoLineFile, true);
    EnableDefaultStreams();
    Passed = Evaluate("Read(multi)", "sparse no line", "Reading a sparse ON file which ends after the type line with several threads succeeds", ReadNoLineMulti, true) && Passed;
  }

  // A sparse matrix which is too big for the dense mode stays sparse and keeps its content
  {
    // 3 axes of 100000 bins: 1e15 bins, 4 PB as dense floats
    MResponseMatrixON TooBig("TooBig", true);
    TooBig.AddAxisLinear("X", 100000, 0.0, 1.0);
    TooBig.AddAxisLinear("Y", 100000, 0.0, 1.0);
    TooBig.AddAxisLinear("Z", 100000, 0.0, 1.0);
    TooBig.Set(vector<unsigned long>{1, 2, 3}, 5.0f);
    DisableDefaultStreams();
    const bool Switched = TooBig.SwitchToNonSparse();
    EnableDefaultStreams();
    Passed = Evaluate("SwitchToNonSparse()", "too big", "SwitchToNonSparse returns false if the dense matrix does not fit into memory", Switched, false) && Passed;
    Passed = Evaluate("IsSparse()", "too big", "A matrix which could not be switched stays sparse", TooBig.IsSparse(), true) && Passed;
    Passed = EvaluateNear("Get()", "too big", "A matrix which could not be switched keeps its content", TooBig.Get(vector<unsigned long>{1, 2, 3}), 5.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetSum()", "too big", "A matrix which could not be switched keeps its sum", TooBig.GetSum(), 5.0, 1e-12) && Passed;
    // Scalar arithmetic needs the dense matrix: it reports the failure with an exception and leaves the matrix as it was
    bool AddThrew = false;
    try {
      TooBig += 1.0f;
    } catch (const MExceptionArbitrary&) {
      AddThrew = true;
    }
    bool SubtractThrew = false;
    try {
      TooBig -= 1.0f;
    } catch (const MExceptionArbitrary&) {
      SubtractThrew = true;
    }
    Passed = Evaluate("operator+=(float)", "too big", "Adding a value to a matrix which cannot become dense throws", AddThrew, true) && Passed;
    Passed = Evaluate("operator-=(float)", "too big", "Subtracting a value from a matrix which cannot become dense throws", SubtractThrew, true) && Passed;
    Passed = Evaluate("IsSparse()", "too big after arithmetic", "A matrix which could not become dense stays sparse after the failed arithmetic", TooBig.IsSparse(), true) && Passed;
    Passed = EvaluateNear("GetSum()", "too big after arithmetic", "A matrix which could not become dense keeps its content after the failed arithmetic", TooBig.GetSum(), 5.0, 1e-12) && Passed;
    Passed = Evaluate("SwitchToNonSparse()", "already dense", "SwitchToNonSparse returns true if the matrix is already dense", MResponseMatrixON("Dense").SwitchToNonSparse(), true) && Passed;
  }

  // Files written before the keyword MS existed carry the flag whether the matrix is sparse in an "SP true/false" line
  {
    MString OldFile = GetTemporaryFileName("sparse_old_format.rsp");
    Passed = EvaluateTrue("WriteTextFile()", "old format file", "The ON file in the old format with SP true can be written",
                          WriteTextFile(OldFile, "Version 1\nNM OldFormat\nOD 1\nTS 5\nSA 0\nCE false\nSP true\n\nAN \"X\"\nAT 1D BinEdges\nAD 0 1 2\n\nType ResponseMatrixONSparse\n\nRD 1 3.5\n")) && Passed;
    MResponseMatrixON OldFormat;
    DisableDefaultStreams();
    const bool ReadOldFormat = OldFormat.Read(OldFile);
    EnableDefaultStreams();
    Passed = Evaluate("Read()", "old format", "Reading an ON file in the old format succeeds", ReadOldFormat, true) && Passed;
    Passed = EvaluateNear("Get()", "old format content", "The content of an ON file in the old format is read", OldFormat.Get(vector<unsigned long>{1}), 3.5, 1e-12) && Passed;
    Passed = Evaluate("GetSpectralType()", "old format", "The sparse flag SP true of the old format is not a spectral type", OldFormat.GetSpectralType(), MString("")) && Passed;
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
  int Status = MSystem::RunProcess("bin/UTResponseMatrixON", Argument, "/dev/null");
  if (Status < 0) return false;
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
