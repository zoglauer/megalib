/*
 * UTEndToEndReference.cxx
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
using namespace std;

// MEGAlib libs:
#include "MEndToEndReference.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndReference
class UTEndToEndReference : public MUnitTest
{
public:
  UTEndToEndReference() : MUnitTest("UTEndToEndReference") {}
  virtual ~UTEndToEndReference() {}

  virtual bool Run();

private:
  bool TestValues();
  bool TestFromLine();
  bool TestToLine();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReference::Run()
{
  bool Passed = true;
  Passed = TestValues() && Passed;
  Passed = TestFromLine() && Passed;
  Passed = TestToLine() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReference::TestValues()
{
  bool Passed = true;

  // A reference without values: no name, nothing measured, the calibration of the bootstrap error is one
  MEndToEndReference Empty;
  Passed = Evaluate("MEndToEndReference::GetName()", "default", "A default reference has no name", Empty.GetName(), MString("")) && Passed;
  Passed = EvaluateNear("MEndToEndReference::GetMean()", "default", "The default mean is zero", Empty.GetMean(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::GetError()", "default", "The default error is zero", Empty.GetError(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::GetScatter()", "default", "The default scatter is zero", Empty.GetScatter(), 0.0, 0.0) && Passed;
  Passed = Evaluate("MEndToEndReference::GetRuns()", "default", "The default number of runs is zero", Empty.GetRuns(), 0U) && Passed;
  Passed = EvaluateNear("MEndToEndReference::GetCalibration()", "default", "The default calibration is one", Empty.GetCalibration(), 1.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::GetRelativeFloor()", "default", "The default relative floor is zero", Empty.GetRelativeFloor(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::GetAbsoluteFloor()", "default", "The default absolute floor is zero", Empty.GetAbsoluteFloor(), 0.0, 0.0) && Passed;

  // The standard constructor gives the name, the setters set the values
  MEndToEndReference Reference("S/O");
  Passed = Evaluate("MEndToEndReference::GetName()", "constructor", "The standard constructor gives the name", Reference.GetName(), MString("S/O")) && Passed;
  Reference.SetName("T/P");
  Reference.SetMean(1.5);
  Reference.SetError(0.25);
  Reference.SetScatter(0.875);
  Reference.SetRuns(12);
  Reference.SetCalibration(1.37);
  Reference.SetRelativeFloor(0.005);
  Reference.SetAbsoluteFloor(0.2);
  Passed = Evaluate("MEndToEndReference::SetName()", "set", "The name", Reference.GetName(), MString("T/P")) && Passed;
  Passed = EvaluateNear("MEndToEndReference::SetMean()", "set", "The mean", Reference.GetMean(), 1.5, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::SetError()", "set", "The error", Reference.GetError(), 0.25, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::SetScatter()", "set", "The scatter", Reference.GetScatter(), 0.875, 0.0) && Passed;
  Passed = Evaluate("MEndToEndReference::SetRuns()", "set", "The number of runs", Reference.GetRuns(), 12U) && Passed;
  Passed = EvaluateNear("MEndToEndReference::SetCalibration()", "set", "The calibration", Reference.GetCalibration(), 1.37, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::SetRelativeFloor()", "set", "The relative floor", Reference.GetRelativeFloor(), 0.005, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::SetAbsoluteFloor()", "set", "The absolute floor", Reference.GetAbsoluteFloor(), 0.2, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReference::TestFromLine()
{
  bool Passed = true;

  // A line with all eight columns, the name has spaces and slashes
  MEndToEndReference Reference;
  Passed = EvaluateTrue("MEndToEndReference::FromLine()", "eight columns", "A line with eight columns can be read", Reference.FromLine("Line100/simulated events area / cm2\t22.5\t0.125\t0.875\t50\t1.37\t0.005\t0.25")) && Passed;
  Passed = Evaluate("MEndToEndReference::FromLine()", "name", "The name is the first column", Reference.GetName(), MString("Line100/simulated events area / cm2")) && Passed;
  Passed = EvaluateNear("MEndToEndReference::FromLine()", "mean", "The mean", Reference.GetMean(), 22.5, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::FromLine()", "error", "The error", Reference.GetError(), 0.125, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::FromLine()", "scatter", "The scatter", Reference.GetScatter(), 0.875, 0.0) && Passed;
  Passed = Evaluate("MEndToEndReference::FromLine()", "runs", "The number of runs", Reference.GetRuns(), 50U) && Passed;
  Passed = EvaluateNear("MEndToEndReference::FromLine()", "calibration", "The calibration", Reference.GetCalibration(), 1.37, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::FromLine()", "relative floor", "The relative floor", Reference.GetRelativeFloor(), 0.005, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::FromLine()", "absolute floor", "The absolute floor", Reference.GetAbsoluteFloor(), 0.25, 0.0) && Passed;

  // A line with five columns has the calibration one and no floors, also if the reference had other values before
  Passed = EvaluateTrue("MEndToEndReference::FromLine()", "five columns", "A line with five columns can be read", Reference.FromLine("A/b\t1.5\t0.25\t0.875\t12")) && Passed;
  Passed = Evaluate("MEndToEndReference::FromLine()", "five name", "The name of the new line replaces the old one", Reference.GetName(), MString("A/b")) && Passed;
  Passed = EvaluateNear("MEndToEndReference::FromLine()", "five calibration", "Without the column the calibration is one", Reference.GetCalibration(), 1.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::FromLine()", "five floors", "Without the columns there are no floors", Reference.GetRelativeFloor() + Reference.GetAbsoluteFloor(), 0.0, 0.0) && Passed;

  // Another number of columns, or a number which is not complete or not in its range, is no reference: the reference stays as it is
  const vector<MString> Wrong = { "", "A/b", "A/b\t1.5", "A/b\t1.5\t0.25\t0.875", "A/b\t1.5\t0.25\t0.875\t12\t1.37", "A/b\t1.5\t0.25\t0.875\t12\t1.37\t0.005", "A/b\t1.5\t0.25\t0.875\t12\t1.37\t0.005\t0.25\t7",
    // Numbers which are not complete or not in their range
    "A/b\tgarbage\t0.25\t0.875\t12", "A/b\t1.5x\t0.25\t0.875\t12", "A/b\t\t0.25\t0.875\t12", "A/b\tnan\t0.25\t0.875\t12", "A/b\t1e999\t0.25\t0.875\t12",
    "A/b\t1.5\t-0.25\t0.875\t12", "A/b\t1.5\tnan\t0.875\t12", "A/b\t1.5\t0.25\t-0.875\t12",
    "A/b\t1.5\t0.25\t0.875\t0", "A/b\t1.5\t0.25\t0.875\t-3", "A/b\t1.5\t0.25\t0.875\t1.5", "A/b\t1.5\t0.25\t0.875\t12x", "A/b\t1.5\t0.25\t0.875\t99999999999",
    "A/b\t1.5\t0.25\t0.875\t12\t0\t0.005\t0.25", "A/b\t1.5\t0.25\t0.875\t12\t1.37\t-0.005\t0.25", "A/b\t1.5\t0.25\t0.875\t12\t1.37\t0.005\toops", "A/b\t1.5\t0.25\t0.875\t12\t1.37\t0.005\tinf" };
  for (const MString& Line: Wrong) {
    MEndToEndReference Kept("Kept");
    Kept.SetMean(3.0);
    DisableDefaultStreams();
    const bool IsReference = Kept.FromLine(Line);
    EnableDefaultStreams();
    Passed = EvaluateFalse("MEndToEndReference::FromLine()", Line, "A line with the wrong number of columns or with a bad number is no reference", IsReference) && Passed;
    Passed = Evaluate("MEndToEndReference::FromLine()", MString("kept name ") + Line, "A line which is no reference leaves the name as it is", Kept.GetName(), MString("Kept")) && Passed;
    Passed = EvaluateNear("MEndToEndReference::FromLine()", MString("kept mean ") + Line, "A line which is no reference leaves the mean as it is", Kept.GetMean(), 3.0, 0.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReference::TestToLine()
{
  bool Passed = true;

  // The exact line: all eight columns, tab separated
  MEndToEndReference Reference("A/b");
  Reference.SetMean(1.5);
  Reference.SetError(0.25);
  Reference.SetScatter(0.875);
  Reference.SetRuns(12);
  Passed = Evaluate("MEndToEndReference::ToLine()", "line", "The line has the name and all values, the default calibration is one", Reference.ToLine(), MString("A/b\t1.5\t0.25\t0.875\t12\t1\t0\t0")) && Passed;
  Passed = EvaluateSize("MEndToEndReference::ToLine()", "columns", "The line has eight columns", Reference.ToLine().Tokenize("\t").size(), 8) && Passed;

  // A tab in the name would make a column: it is replaced by a space
  Reference.SetName("with\ttab");
  Passed = Evaluate("MEndToEndReference::ToLine()", "tab", "A tab in the name is a space", Reference.ToLine().Tokenize("\t")[0], MString("with tab")) && Passed;
  Passed = EvaluateSize("MEndToEndReference::ToLine()", "tab columns", "A tab in the name does not add a column", Reference.ToLine().Tokenize("\t").size(), 8) && Passed;

  // The values are written with 17 digits: reading the line gives the same reference
  MEndToEndReference Third("Line100/third");
  Third.SetMean(1.0/3.0);
  Third.SetError(1.0e-9/3.0);
  Third.SetScatter(12345.678901234567);
  Third.SetRuns(50);
  Third.SetCalibration(1.37);
  Third.SetRelativeFloor(0.005);
  Third.SetAbsoluteFloor(2.5e-4);
  MEndToEndReference Read;
  Passed = EvaluateTrue("MEndToEndReference::ToLine()", "round trip read", "The line of a reference can be read", Read.FromLine(Third.ToLine())) && Passed;
  Passed = Evaluate("MEndToEndReference::ToLine()", "round trip name", "The name survives", Read.GetName(), Third.GetName()) && Passed;
  Passed = EvaluateNear("MEndToEndReference::ToLine()", "round trip mean", "The mean is exact", Read.GetMean(), Third.GetMean(), 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::ToLine()", "round trip error", "The error is exact", Read.GetError(), Third.GetError(), 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::ToLine()", "round trip scatter", "The scatter is exact", Read.GetScatter(), Third.GetScatter(), 0.0) && Passed;
  Passed = Evaluate("MEndToEndReference::ToLine()", "round trip runs", "The number of runs is exact", Read.GetRuns(), Third.GetRuns()) && Passed;
  Passed = EvaluateNear("MEndToEndReference::ToLine()", "round trip calibration", "The calibration is exact", Read.GetCalibration(), Third.GetCalibration(), 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::ToLine()", "round trip relative floor", "The relative floor is exact", Read.GetRelativeFloor(), Third.GetRelativeFloor(), 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReference::ToLine()", "round trip absolute floor", "The absolute floor is exact", Read.GetAbsoluteFloor(), Third.GetAbsoluteFloor(), 0.0) && Passed;

  // The names of the columns
  Passed = Evaluate("MEndToEndReference::GetColumnNames()", "names", "The names of the columns of the line", MEndToEndReference::GetColumnNames(), MString("Name, Mean, Error (of the mean), Scatter (of the runs), Runs, Calibration of the bootstrap error, Relative floor, Absolute floor")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTEndToEndReference Test;
  if (Test.Run() == false) {
    return 1;
  }
  return 0;
}
