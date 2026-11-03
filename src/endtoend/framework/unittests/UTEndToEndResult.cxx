/*
 * UTEndToEndResult.cxx
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
#include <cmath>
#include <limits>
using namespace std;

// MEGAlib libs:
#include "MEndToEndResult.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndResult
class UTEndToEndResult : public MUnitTest
{
public:
  UTEndToEndResult() : MUnitTest("UTEndToEndResult") {}
  virtual ~UTEndToEndResult() {}

  virtual bool Run();

private:
  bool TestValues();
  bool TestFromLine();
  bool TestToLine();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResult::Run()
{
  bool Passed = true;
  Passed = TestValues() && Passed;
  Passed = TestFromLine() && Passed;
  Passed = TestToLine() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResult::TestValues()
{
  bool Passed = true;

  // A new result has no reference and passes
  MEndToEndResult Result("A/b");
  Passed = Evaluate("MEndToEndResult::GetName()", "name", "The name of the constructor", Result.GetName(), MString("A/b")) && Passed;
  Passed = EvaluateNear("MEndToEndResult::GetValue()", "value", "The value is zero", Result.GetValue(), 0.0, 0.0) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::HasReference()", "reference", "There is no reference", Result.HasReference()) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::IsValid()", "valid", "A new result is valid", Result.IsValid()) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::IsStatistical()", "statistical", "A new result is statistical", Result.IsStatistical()) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::IsPassed()", "passed", "A new result passes", Result.IsPassed()) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::IsFromReferences()", "from references", "A new result is not from references", Result.IsFromReferences()) && Passed;

  Result.SetName("C/d");
  Result.SetValue(1.5);
  Result.SetError(0.25);
  Result.SetHasReference(true);
  Result.SetReference(1.25);
  Result.SetReferenceError(0.5);
  Result.SetPull(0.4);
  Result.SetTolerance(2.5);
  Result.SetValid(false);
  Result.SetStatistical(false);
  Result.SetPassed(false);
  Result.SetFromReferences(true);
  Passed = Evaluate("MEndToEndResult::SetName()", "name", "The name", Result.GetName(), MString("C/d")) && Passed;
  Passed = EvaluateNear("MEndToEndResult::SetValue()", "value", "The value", Result.GetValue(), 1.5, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::SetError()", "error", "The error", Result.GetError(), 0.25, 0.0) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::SetHasReference()", "has reference", "There is a reference", Result.HasReference()) && Passed;
  Passed = EvaluateNear("MEndToEndResult::SetReference()", "reference", "The reference", Result.GetReference(), 1.25, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::SetReferenceError()", "reference error", "The error of the reference", Result.GetReferenceError(), 0.5, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::SetPull()", "pull", "The pull", Result.GetPull(), 0.4, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::SetTolerance()", "tolerance", "The tolerance", Result.GetTolerance(), 2.5, 0.0) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::SetValid()", "valid", "Not valid", Result.IsValid()) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::SetStatistical()", "statistical", "Not statistical", Result.IsStatistical()) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::SetPassed()", "passed", "Not passed", Result.IsPassed()) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::SetFromReferences()", "from references", "From references", Result.IsFromReferences()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResult::TestFromLine()
{
  bool Passed = true;

  MEndToEndResult Result("Old");
  Result.SetValue(7.0);
  Passed = EvaluateTrue("MEndToEndResult::FromLine()", "twelve", "A line with twelve columns is a result", Result.FromLine("A/b\t1.5\t0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1")) && Passed;
  Passed = Evaluate("MEndToEndResult::FromLine()", "name", "The name", Result.GetName(), MString("A/b")) && Passed;
  Passed = EvaluateNear("MEndToEndResult::FromLine()", "value", "The value", Result.GetValue(), 1.5, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::FromLine()", "error", "The error", Result.GetError(), 0.25, 0.0) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::FromLine()", "has reference", "There is a reference", Result.HasReference()) && Passed;
  Passed = EvaluateNear("MEndToEndResult::FromLine()", "reference", "The reference", Result.GetReference(), 1.25, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::FromLine()", "reference error", "The error of the reference", Result.GetReferenceError(), 0.5, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::FromLine()", "pull", "The pull", Result.GetPull(), 0.4, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::FromLine()", "tolerance", "The tolerance", Result.GetTolerance(), 2.5, 0.0) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::FromLine()", "valid", "Valid", Result.IsValid()) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::FromLine()", "statistical", "Not statistical", Result.IsStatistical()) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::FromLine()", "passed", "Passed", Result.IsPassed()) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::FromLine()", "from references", "From references", Result.IsFromReferences()) && Passed;

  // Another number of columns is an error and leaves the result as it is
  MEndToEndResult Kept("Kept");
  Kept.SetValue(7.0);
  DisableDefaultStreams();
  const bool Eleven = Kept.FromLine("A/b\t1.5\t0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1");
  const bool Thirteen = Kept.FromLine("A/b\t1.5\t0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1\t1");
  const bool Empty = Kept.FromLine("");
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndResult::FromLine()", "eleven", "Eleven columns are no result", Eleven) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::FromLine()", "thirteen", "Thirteen columns are no result", Thirteen) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::FromLine()", "empty", "An empty line is no result", Empty) && Passed;
  Passed = Evaluate("MEndToEndResult::FromLine()", "kept name", "A failed line leaves the name", Kept.GetName(), MString("Kept")) && Passed;
  Passed = EvaluateNear("MEndToEndResult::FromLine()", "kept value", "A failed line leaves the value", Kept.GetValue(), 7.0, 0.0) && Passed;

  // A number which is not complete or not in its range, or a flag which is not 0 or 1, is no result either
  const vector<MString> Bad = {
    "A/b\tgarbage\t0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1", "A/b\t1.5x\t0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1", "A/b\t\t0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1",
    "A/b\t1.5\t-0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1", "A/b\t1.5\t0.25\t1\t1.25\t-0.5\t0.4\t2.5\t1\t0\t1\t1", "A/b\t1.5\t0.25\t1\t1.25\t0.5\t0.4\t-2.5\t1\t0\t1\t1",
    "A/b\t1.5\t0.25\t2\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1", "A/b\t1.5\t0.25\ttrue\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1", "A/b\t1.5\t0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t-1",
    "A/b\t1.5\t0.25\t1\t1.25\t0.5\tx\t2.5\t1\t0\t1\t1", "A/b\t1.5\t0.25\t1\t1e999\t0.5\t0.4\t2.5\t1\t0\t1\t1" };
  for (const MString& Line: Bad) {
    MEndToEndResult Unchanged("Kept");
    Unchanged.SetValue(7.0);
    DisableDefaultStreams();
    const bool IsResult = Unchanged.FromLine(Line);
    EnableDefaultStreams();
    Passed = EvaluateFalse("MEndToEndResult::FromLine()", Line, "A line with a bad number or flag is no result", IsResult) && Passed;
    Passed = EvaluateNear("MEndToEndResult::FromLine()", MString("kept ") + Line, "A line which is no result leaves the value", Unchanged.GetValue(), 7.0, 0.0) && Passed;
  }

  // Values which are not finite are no result: a test does not record them
  for (const MString& Line: vector<MString>({ "A/b\tnan\t0.25\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1", "A/b\t1.5\tinf\t1\t1.25\t0.5\t0.4\t2.5\t1\t0\t1\t1", "A/b\t1.5\t0.25\t1\t1.25\t0.5\tnan\t2.5\t1\t0\t1\t1", "A/b\t1.5\t0.25\t1\t1.25\t0.5\t0.4\t-inf\t1\t0\t1\t1" })) {
    MEndToEndResult Unchanged("Kept");
    DisableDefaultStreams();
    const bool IsResult = Unchanged.FromLine(Line);
    EnableDefaultStreams();
    Passed = EvaluateFalse("MEndToEndResult::FromLine()", Line, "A line with a value which is not finite is no result", IsResult) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResult::TestToLine()
{
  bool Passed = true;

  // Values which need all 17 digits survive the round trip, a tab in the name is replaced by a blank
  MEndToEndResult Result("Line100/with\ttab");
  Result.SetValue(1.0/3.0);
  Result.SetError(1.0e-9/3.0);
  Result.SetHasReference(true);
  Result.SetReference(12345.678901234567);
  Result.SetReferenceError(2.5e-4);
  Result.SetPull(-0.1);
  Result.SetTolerance(7.0/3.0);
  Result.SetValid(true);
  Result.SetStatistical(false);
  Result.SetPassed(false);
  Result.SetFromReferences(true);

  MEndToEndResult Read;
  Passed = EvaluateTrue("MEndToEndResult::ToLine()", "round trip", "The line is a result", Read.FromLine(Result.ToLine())) && Passed;
  Passed = Evaluate("MEndToEndResult::ToLine()", "name", "The tab in the name is a blank", Read.GetName(), MString("Line100/with tab")) && Passed;
  Passed = EvaluateNear("MEndToEndResult::ToLine()", "value", "The value is exact", Read.GetValue(), 1.0/3.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::ToLine()", "error", "The error is exact", Read.GetError(), 1.0e-9/3.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::ToLine()", "reference", "The reference is exact", Read.GetReference(), 12345.678901234567, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::ToLine()", "reference error", "The error of the reference is exact", Read.GetReferenceError(), 2.5e-4, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::ToLine()", "pull", "The pull is exact", Read.GetPull(), -0.1, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResult::ToLine()", "tolerance", "The tolerance is exact", Read.GetTolerance(), 7.0/3.0, 0.0) && Passed;
  Passed = EvaluateTrue("MEndToEndResult::ToLine()", "flags", "The flags", Read.HasReference() && Read.IsValid() && Read.IsFromReferences()) && Passed;
  Passed = EvaluateFalse("MEndToEndResult::ToLine()", "flags false", "The flags which are false", Read.IsStatistical() || Read.IsPassed()) && Passed;

  // A NaN pull is written, but it is no result when it is read
  Result.SetPull(numeric_limits<double>::quiet_NaN());
  MEndToEndResult NaNRead;
  DisableDefaultStreams();
  const bool NaNIsResult = NaNRead.FromLine(Result.ToLine());
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndResult::ToLine()", "nan line", "The line with a NaN is no result", NaNIsResult) && Passed;

  Passed = EvaluateTrue("MEndToEndResult::GetColumnNames()", "names", "The names of the columns start with the name and end with the origin", MEndToEndResult::GetColumnNames().BeginsWith("Name, ") && MEndToEndResult::GetColumnNames().EndsWith("From references")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTEndToEndResult Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTEndToEndResult.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
