/*
 * UTEndToEndResultList.cxx
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
#include <fstream>
#include <deque>
#include <limits>
using namespace std;

// MEGAlib libs:
#include "MEndToEndResultList.h"
#include "MFile.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndResultList
class UTEndToEndResultList : public MUnitTest
{
public:
  UTEndToEndResultList() : MUnitTest("UTEndToEndResultList") {}
  virtual ~UTEndToEndResultList() {}

  virtual bool Run();

private:
  bool TestPull();
  bool TestPullStatistics();
  bool TestComparisons();
  bool TestFile();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResultList::Run()
{
  bool Passed = true;
  Passed = TestPull() && Passed;
  Passed = TestPullStatistics() && Passed;
  Passed = TestComparisons() && Passed;
  Passed = TestFile() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResultList::TestPull()
{
  bool Passed = true;

  ETMeasuredValue Measured, Expected;
  Measured.m_Value = 12.0;
  Measured.m_Error = 3.0;
  Expected.m_Value = 8.0;
  Expected.m_Error = 4.0;
  // (12 - 8)/sqrt(9 + 16) = 0.8
  Passed = EvaluateNear("MEndToEndResultList::GetPull()", "difference", "The pull is the difference in combined standard errors", MEndToEndResultList::GetPull(Measured, Expected), 0.8, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetPull()", "sign", "The pull is negative below the expected value", MEndToEndResultList::GetPull(Expected, Measured), -0.8, 1e-12) && Passed;
  Measured.m_Error = 0.0;
  Expected.m_Error = 0.0;
  Passed = EvaluateNear("MEndToEndResultList::GetPull()", "equal without errors", "Equal values without uncertainties have the pull zero", MEndToEndResultList::GetPull(Measured, Measured), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetPull()", "different without errors", "Different values without uncertainties have the pull ten", MEndToEndResultList::GetPull(Measured, Expected), 10.0, 0.0) && Passed;

  // A value or an uncertainty which is not finite gives no pull
  Measured.m_Error = numeric_limits<double>::infinity();
  Passed = EvaluateTrue("MEndToEndResultList::GetPull()", "infinite", "An infinite uncertainty gives a NaN pull", isnan(MEndToEndResultList::GetPull(Measured, Expected))) && Passed;
  Measured.m_Error = 1.0;
  Measured.m_Value = numeric_limits<double>::quiet_NaN();
  Passed = EvaluateTrue("MEndToEndResultList::GetPull()", "NaN value", "A NaN value gives a NaN pull", isnan(MEndToEndResultList::GetPull(Measured, Expected))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResultList::TestPullStatistics()
{
  bool Passed = true;

  MEndToEndResultList Results;
  Passed = Evaluate("MEndToEndResultList::GetPullStatistics()", "empty", "Without comparisons there are no pulls", Results.GetPullStatistics().m_NumberOfPulls, 0U) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetPullStatistics()", "empty fraction", "The fraction without pulls is zero", Results.GetPullStatistics().GetFraction(1).m_Value, 0.0, 0.0) && Passed;

  // A key result without reference and four comparisons with the pulls 0.5, 1.5, 2.5, 3.5 (error 1 against the expected value 0)
  ETMeasuredValue Zero;
  Results.AddResult("no reference", 1.0, 0.1);
  for (double Pull: { 0.5, 1.5, 2.5, 3.5 }) {
    ETMeasuredValue Value;
    Value.m_Value = Pull;
    Value.m_Error = 1.0;
    Results.AddComparison("Pull", Value, Zero, MEndToEndResultList::c_PullLimit*Value.m_Error, true);
  }
  const ETPullStatistics Statistics = Results.GetPullStatistics();
  Passed = Evaluate("MEndToEndResultList::GetPullStatistics()", "number", "Only the comparisons count", Statistics.m_NumberOfPulls, 4U) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetPullStatistics()", "inside 1", "One of four pulls is inside one sigma", Statistics.GetFraction(1).m_Value, 0.25, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetPullStatistics()", "inside 2", "Two of four pulls are inside two sigma", Statistics.GetFraction(2).m_Value, 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetPullStatistics()", "inside 3", "Three of four pulls are inside three sigma", Statistics.GetFraction(3).m_Value, 0.75, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetPullStatistics()", "invalid sigma", "Three sigma is the maximum: four gives zero", Statistics.GetFraction(4).m_Value, 0.0, 0.0) && Passed;

  // Comparisons with a limit, without uncertainties, and invalid comparisons are no Gaussian pulls: they do not change the statistics
  ETMeasuredValue Distance, NoDistance, NotANumber;
  Distance.m_Value = 0.1;
  NotANumber.m_Error = numeric_limits<double>::quiet_NaN();
  Results.AddComparison("Limit", Distance, NoDistance, 0.5, false);
  Results.AddComparison("Invalid", NotANumber, NoDistance, MEndToEndResultList::c_PullLimit, true);
  Results.AddComparison("Degenerate", NoDistance, NoDistance, 0.0, true);
  Passed = Evaluate("MEndToEndResultList::GetPullStatistics()", "only statistical", "Comparisons with a limit, without uncertainties, and invalid comparisons are not counted", Results.GetPullStatistics().m_NumberOfPulls, 4U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResultList::TestComparisons()
{
  bool Passed = true;

  // The comparison is inside the tolerance up to the limit, the expected value is 0 with no error
  MEndToEndResultList Results;
  ETMeasuredValue Zero, AtLimit, AboveLimit;
  AtLimit.m_Value = MEndToEndResultList::c_PullLimit;
  AtLimit.m_Error = 1.0;
  AboveLimit.m_Value = MEndToEndResultList::c_PullLimit + 0.1;
  AboveLimit.m_Error = 1.0;
  Passed = EvaluateTrue("MEndToEndResultList::AddComparison()", "at the limit", "A difference of the tolerance is inside", Results.AddComparison("At", AtLimit, Zero, MEndToEndResultList::c_PullLimit, true).IsPassed()) && Passed;
  Passed = EvaluateFalse("MEndToEndResultList::AddComparison()", "above the limit", "A difference above the tolerance is outside", Results.AddComparison("Above", AboveLimit, Zero, MEndToEndResultList::c_PullLimit, true).IsPassed()) && Passed;
  Passed = EvaluateTrue("MEndToEndResultList::AddComparison()", "reference", "The comparison has a reference", Results.GetResults().back().HasReference()) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::AddComparison()", "pull", "The pull of a statistical comparison is the difference in standard errors", Results.GetResults().back().GetPull(), MEndToEndResultList::c_PullLimit + 0.1, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::AddComparison()", "tolerance", "The tolerance is stored", Results.GetResults().back().GetTolerance(), MEndToEndResultList::c_PullLimit, 0.0) && Passed;
  Passed = EvaluateTrue("MEndToEndResultList::AddComparison()", "valid", "The comparison with finite values is valid", Results.GetResults().back().IsValid()) && Passed;

  // A comparison with a limit scores in units of the tolerance divided by the pull limit: a difference of 0.3 with the tolerance 0.5 scores 0.3*5/0.5 = 3
  ETMeasuredValue Distance, NoDistance;
  Distance.m_Value = 0.3;
  const MEndToEndResult& Limited = Results.AddComparison("Limited", Distance, NoDistance, 0.5, false);
  Passed = EvaluateNear("MEndToEndResultList::AddComparison()", "score", "A comparison with a limit scores in units of the tolerance divided by the pull limit", Limited.GetPull(), 3.0, 1e-12) && Passed;
  Passed = EvaluateFalse("MEndToEndResultList::AddComparison()", "not statistical", "A comparison with a limit is not statistical", Limited.IsStatistical()) && Passed;
  Passed = EvaluateTrue("MEndToEndResultList::AddComparison()", "limited inside", "The difference is inside the limit", Limited.IsPassed()) && Passed;
  // The tolerance of a statistical comparison is the pull limit times the combined uncertainty: 5*sqrt(0.3^2 + 0.4^2) = 2.5, the default of the drawing, which scores a 0.5 sigma pull as 0.5 and not as 10
  ETMeasuredValue Small, Reference, BadValue;
  BadValue.m_Error = numeric_limits<double>::quiet_NaN();
  Small.m_Value = 1.25;
  Small.m_Error = 0.3;
  Reference.m_Value = 1.0;
  Reference.m_Error = 0.4;
  Passed = EvaluateNear("MEndToEndResultList::GetStatisticalTolerance()", "value", "The tolerance is the pull limit times the combined uncertainty", MEndToEndResultList::GetStatisticalTolerance(Small, Reference), MEndToEndResultList::c_PullLimit*0.5, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetScore()", "default tolerance", "A pull of 0.5 sigma scores 0.5 with the tolerance of a statistical comparison", MEndToEndResultList::GetScore(Small, Reference, MEndToEndResultList::GetStatisticalTolerance(Small, Reference), true), 0.5, 1e-12) && Passed;
  Passed = EvaluateTrue("MEndToEndResultList::GetStatisticalTolerance()", "NaN", "A value which is not valid has no tolerance", isnan(MEndToEndResultList::GetStatisticalTolerance(BadValue, Reference))) && Passed;
  Passed = EvaluateTrue("MEndToEndResultList::IsStatisticalComparison()", "default", "The comparison with the tolerance of a statistical comparison is statistical", MEndToEndResultList::IsStatisticalComparison(Small, Reference, MEndToEndResultList::GetStatisticalTolerance(Small, Reference), true)) && Passed;

  // The pull 5 with the tolerance 7 passes: it is no statistical pull, the score is 5*5/7 and the plot shows it inside the limit
  ETMeasuredValue Five;
  Five.m_Value = 5.0;
  Five.m_Error = 1.0;
  const MEndToEndResult& Allowed = Results.AddComparison("Allowed", Five, Zero, 7.0, true);
  Passed = EvaluateTrue("MEndToEndResultList::AddComparison()", "wide tolerance passes", "A pull of five passes with the tolerance seven", Allowed.IsPassed()) && Passed;
  Passed = EvaluateFalse("MEndToEndResultList::AddComparison()", "wide tolerance not statistical", "A comparison with a tolerance which is not the pull limit is not statistical", Allowed.IsStatistical()) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::AddComparison()", "wide tolerance score", "The score is inside the pull limit", Allowed.GetPull(), 5.0*MEndToEndResultList::c_PullLimit/7.0, 1e-12) && Passed;
  // Both uncertainties are zero: the pull would be zero or ten
  const MEndToEndResult& Degenerate = Results.AddComparison("Degenerate", Distance, NoDistance, 0.5, true);
  Passed = EvaluateFalse("MEndToEndResultList::AddComparison()", "no uncertainties", "A comparison without uncertainties is not statistical", Degenerate.IsStatistical()) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetScore()", "zero tolerance, equal", "Equal values without tolerance score zero", MEndToEndResultList::GetScore(NoDistance, NoDistance, 0.0, false), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndResultList::GetScore()", "zero tolerance, different", "Different values without tolerance score ten", MEndToEndResultList::GetScore(Distance, NoDistance, 0.0, false), 10.0, 0.0) && Passed;

  // Values which are not valid fail: a NaN or an infinite uncertainty must not hide a difference
  ETMeasuredValue NotANumber, Infinite, Negative;
  NotANumber.m_Value = 100.0;
  NotANumber.m_Error = numeric_limits<double>::quiet_NaN();
  Infinite.m_Value = 100.0;
  Infinite.m_Error = numeric_limits<double>::infinity();
  Negative.m_Value = 0.0;
  Negative.m_Error = -1.0;
  Passed = EvaluateFalse("ETMeasuredValue::IsValid()", "NaN error", "A NaN uncertainty is not valid", NotANumber.IsValid()) && Passed;
  Passed = EvaluateFalse("ETMeasuredValue::IsValid()", "infinite error", "An infinite uncertainty is not valid", Infinite.IsValid()) && Passed;
  Passed = EvaluateFalse("ETMeasuredValue::IsValid()", "negative error", "A negative uncertainty is not valid", Negative.IsValid()) && Passed;
  Passed = EvaluateTrue("ETMeasuredValue::IsValid()", "valid", "A finite value with a finite uncertainty is valid", AtLimit.IsValid()) && Passed;
  Passed = EvaluateTrue("MEndToEndResultList::GetPull()", "NaN error", "The pull with a NaN uncertainty is NaN", isnan(MEndToEndResultList::GetPull(NotANumber, Zero))) && Passed;
  Passed = EvaluateTrue("MEndToEndResultList::GetPull()", "infinite error", "The pull with an infinite uncertainty is NaN", isnan(MEndToEndResultList::GetPull(Infinite, Zero))) && Passed;
  const MEndToEndResult& Invalid = Results.AddComparison("Invalid", NotANumber, Zero, 5.0, true);
  Passed = EvaluateFalse("MEndToEndResultList::AddComparison()", "NaN error passes", "A comparison with a NaN uncertainty fails", Invalid.IsPassed()) && Passed;
  Passed = EvaluateFalse("MEndToEndResultList::AddComparison()", "NaN error valid", "A comparison with a NaN uncertainty is marked invalid", Invalid.IsValid()) && Passed;
  Passed = EvaluateFalse("MEndToEndResultList::AddComparison()", "infinite tolerance", "A comparison with an infinite tolerance fails", Results.AddComparison("Unlimited", Zero, Zero, numeric_limits<double>::infinity(), false).IsPassed()) && Passed;

  // A key result which was measured before is kept as it is
  MEndToEndResult Before;
  Before.SetName("Before");
  Before.SetPull(2.5);
  Before.SetHasReference(true);
  Passed = EvaluateNear("MEndToEndResultList::AddResult()", "kept", "A key result which was measured before keeps its pull", Results.AddResult(Before).GetPull(), 2.5, 0.0) && Passed;
  Passed = EvaluateSize("MEndToEndResultList::GetResults()", "number", "All key results are kept", Results.GetResults().size(), 8) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndResultList::TestFile()
{
  bool Passed = true;

  // The key results are stored as text and read back with all their values: the values are written with 17 digits
  const MString Directory = GetTemporaryDirectoryName("keyresults");
  Passed = EvaluateTrue("MFile::CreateDirectory()", "keyresults", "The directory for the key results can be created", MFile::CreateDirectory(Directory)) && Passed;
  {
    MEndToEndResultList Results;
    ETMeasuredValue Measured, Expected;
    Measured.m_Value = 10.2;
    Measured.m_Error = 0.3;
    Expected.m_Value = 10.0;
    Expected.m_Error = 0.5;
    Results.AddComparison("S/O", Measured, Expected, MEndToEndResultList::c_PullLimit*sqrt(0.3*0.3 + 0.5*0.5), true).SetFromReferences(true);
    Results.AddResult("Free", 1.0, 0.1);
    Passed = EvaluateTrue("MEndToEndResultList::Write()", "written", "The key results can be written", Results.Write(Directory + "/Probe.keyresults.tsv")) && Passed;
  }
  Passed = EvaluateTrue("MEndToEndResultList::Write()", "key results", "The key results are stored as text", MFile::Exists(Directory + "/Probe.keyresults.tsv")) && Passed;
  MEndToEndResultList Read;
  Passed = EvaluateTrue("MEndToEndResultList::Read()", "read", "The key results can be read back", Read.Read(Directory + "/Probe.keyresults.tsv")) && Passed;
  // A line which is not a key result makes the file unusable: the other rows are not used
  {
    MString Content;
    MFile::ReadTextFile(Directory + "/Probe.keyresults.tsv", Content);
    for (const MString& Malformed: vector<MString>({ "B/nan\tnan\t0.1\t0\t0\t0\t0\t0\t1\t0\t1\t1", "garbage", "B/short\t1.0\t0.1" })) {
      ofstream Out((Directory + "/Bad.keyresults.tsv").Data());
      Out<<Content<<Malformed<<endl;
      Out.close();
      MEndToEndResultList Bad;
      DisableDefaultStreams();
      const bool BadRead = Bad.Read(Directory + "/Bad.keyresults.tsv");
      EnableDefaultStreams();
      Passed = EvaluateFalse("MEndToEndResultList::Read()", Malformed, "A file with a line which is not a key result cannot be read", BadRead) && Passed;
      Passed = EvaluateTrue("MEndToEndResultList::Read()", MString("empty ") + Malformed, "Such a file leaves no key results", Bad.IsEmpty()) && Passed;
    }
  }
  const deque<MEndToEndResult>& Stored = Read.GetResults();
  Passed = EvaluateSize("MEndToEndResultList::Read()", "number", "Both key results are read back", Stored.size(), 2) && Passed;
  if (Stored.size() == 2) {
    Passed = Evaluate("MEndToEndResultList::Read()", "name", "The name of the comparison", Stored[0].GetName(), MString("S/O")) && Passed;
    Passed = EvaluateNear("MEndToEndResultList::Read()", "value", "The measured value", Stored[0].GetValue(), 10.2, 0.0) && Passed;
    Passed = EvaluateNear("MEndToEndResultList::Read()", "error", "The measured uncertainty", Stored[0].GetError(), 0.3, 0.0) && Passed;
    Passed = EvaluateTrue("MEndToEndResultList::Read()", "has reference", "The comparison has a reference", Stored[0].HasReference()) && Passed;
    Passed = EvaluateNear("MEndToEndResultList::Read()", "reference", "The reference value", Stored[0].GetReference(), 10.0, 0.0) && Passed;
    Passed = EvaluateNear("MEndToEndResultList::Read()", "reference error", "The reference uncertainty", Stored[0].GetReferenceError(), 0.5, 0.0) && Passed;
    // (10.2 - 10)/sqrt(0.3^2 + 0.5^2)
    Passed = EvaluateNear("MEndToEndResultList::Read()", "pull", "The pull", Stored[0].GetPull(), 0.2/sqrt(0.3*0.3 + 0.5*0.5), 1e-12) && Passed;
    Passed = EvaluateTrue("MEndToEndResultList::Read()", "passed", "The comparison is inside the tolerance", Stored[0].IsPassed()) && Passed;
    Passed = EvaluateNear("MEndToEndResultList::Read()", "tolerance", "The tolerance", Stored[0].GetTolerance(), MEndToEndResultList::c_PullLimit*sqrt(0.3*0.3 + 0.5*0.5), 1e-12) && Passed;
    Passed = EvaluateTrue("MEndToEndResultList::Read()", "valid", "The comparison is valid", Stored[0].IsValid()) && Passed;
    Passed = EvaluateTrue("MEndToEndResultList::Read()", "from references", "The flag of the reference is read", Stored[0].IsFromReferences()) && Passed;
    Passed = EvaluateFalse("MEndToEndResultList::Read()", "free from references", "A key result without the flag has none", Stored[1].IsFromReferences()) && Passed;
    Passed = EvaluateTrue("MEndToEndResultList::Read()", "statistical", "The comparison is statistical", Stored[0].IsStatistical()) && Passed;
    Passed = EvaluateFalse("MEndToEndResultList::Read()", "free has no reference", "The key result without reference has none", Stored[1].HasReference()) && Passed;
  }

  // A file which cannot be read leaves no key results, reading replaces the key results
  Passed = EvaluateFalse("MEndToEndResultList::Read()", "missing file", "A missing file cannot be read", Read.Read(Directory + "/Missing.tsv")) && Passed;
  Passed = EvaluateTrue("MEndToEndResultList::Read()", "missing file empty", "A failed read leaves no key results", Read.IsEmpty()) && Passed;
  DisableDefaultStreams();
  const bool NotWritten = Read.Write(Directory + "/NoDirectory/Probe.keyresults.tsv");
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndResultList::Write()", "no directory", "A file in a directory which does not exist cannot be written", NotWritten) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTEndToEndResultList Test;
  if (Test.Run() == false) {
    return 1;
  }
  return 0;
}
