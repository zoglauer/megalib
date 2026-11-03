/*
 * UTEndToEndCampaignAnalysis.cxx
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
#include <limits>
#include <random>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TMath.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MEndToEndCampaignAnalysis.h"
#include "MEndToEndFingerprint.h"
#include "MFile.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndCampaignAnalysis
class UTEndToEndCampaignAnalysis : public MUnitTest
{
public:
  UTEndToEndCampaignAnalysis() : MUnitTest("UTEndToEndCampaignAnalysis") {}
  virtual ~UTEndToEndCampaignAnalysis() {}

  virtual bool Run();

private:
  //! Return a run with the observables A/x and A/y: the values, the bootstrap errors, and the number of failed checks
  ETCampaignRun MakeRun(double X, double Y, double Error, unsigned int FailedChecks = 0);
  //! Return the number of findings of the severity which contain the text
  unsigned int Count(const MEndToEndCampaignAnalysis& Analysis, MCampaignSeverity Severity, const MString& Text);

  bool TestStatistics();
  bool TestGates();
  bool TestFindings();
  bool TestCoverage();
  bool TestReferences();
  bool TestReadRun();
};


////////////////////////////////////////////////////////////////////////////////


ETCampaignRun UTEndToEndCampaignAnalysis::MakeRun(double X, double Y, double Error, unsigned int FailedChecks)
{
  ETCampaignRun Run;
  Run.m_FailedChecks = FailedChecks;
  Run.m_Fingerprint = { { "geant4", "10.2.3" }, { "root", "6.40.04" }, { "geometry", "00000000000000aa" }, { "scenarios", "00000000000000bb" } };
  MEndToEndResult Result;
  Result.SetFromReferences(true);
  Result.SetName("A/x");
  Result.SetValue(X);
  Result.SetError(Error);
  Run.m_Results.AddResult(Result);
  Result.SetName("A/y");
  Result.SetValue(Y);
  Run.m_Results.AddResult(Result);

  // A key result which is not compared with a reference is no observable of the campaign
  MEndToEndResult Derived;
  Derived.SetName("A/derived");
  Derived.SetValue(123.0);
  Run.m_Results.AddResult(Derived);
  return Run;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTEndToEndCampaignAnalysis::Count(const MEndToEndCampaignAnalysis& Analysis, MCampaignSeverity Severity, const MString& Text)
{
  unsigned int Number = 0;
  for (const ETCampaignFinding& Finding: Analysis.GetFindings()) {
    if (Finding.m_Severity == Severity && Finding.m_Message.Contains(Text) == true) {
      ++Number;
    }
  }
  return Number;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaignAnalysis::Run()
{
  bool Passed = true;
  Passed = TestStatistics() && Passed;
  Passed = TestGates() && Passed;
  Passed = TestFindings() && Passed;
  Passed = TestCoverage() && Passed;
  Passed = TestReferences() && Passed;
  Passed = TestReadRun() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaignAnalysis::TestStatistics()
{
  bool Passed = true;

  // Ten runs with the values 1 ... 10 of x (bootstrap error 2), and y = 5 in all runs: x has the mean 5.5, the scatter sqrt(82.5/9) = 3.0276, and the median 5.5
  // Its median absolute deviation is 2.5: robust scatter 1.4826*2.5 = 3.7065, the ratio of the scatter and the bootstrap error is 1.5138
  MEndToEndCampaignAnalysis Analysis;
  Analysis.SetMinimumRuns(10);
  for (unsigned int r = 1; r <= 10; ++r) {
    Analysis.AddRun(MakeRun(r, 5.0, 2.0));
  }
  Passed = Evaluate("MEndToEndCampaignAnalysis::GetNumberOfRuns()", "ten", "Ten runs were added", Analysis.GetNumberOfRuns(), 10U) && Passed;
  Analysis.Analyze();
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::GetObservables()", "number", "Only the key results which are compared with references are observables", Analysis.GetObservables().size(), 2) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::GetObservables()", "sorted", "The observables are sorted by name", Analysis.GetObservables()[0].m_Name, MString("A/x")) && Passed;
  const ETCampaignObservable& X = Analysis.GetObservables()[0];
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "mean", "The mean of 1 ... 10", X.m_Mean, 5.5, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "scatter", "The standard deviation of 1 ... 10 with the divisor 9", X.m_Scatter, sqrt(82.5/9.0), 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "error", "The standard error of the mean is the scatter divided by the square root of ten", X.m_Error, sqrt(82.5/9.0)/sqrt(10.0), 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "median", "The median of 1 ... 10", X.m_Median, 5.5, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "robust", "The median absolute deviation of 1 ... 10 is 2.5", X.m_RobustScatter, 1.4826*2.5, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "bootstrap", "The mean bootstrap error", X.m_MeanBootstrapError, 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "ratio", "The ratio of the scatter and the bootstrap error", X.m_Ratio, sqrt(82.5/9.0)/2.0, 1e-12) && Passed;
  // Halves 1 ... 5 and 6 ... 10: means 3 and 8, the variance within a half is 2.5: the error of the difference is sqrt((2.5 + 2.5)/5) = 1
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "drift", "The drift of the halves in standard errors of the difference", X.m_Drift, 5.0, 1e-12) && Passed;
  const ETCampaignObservable& Y = Analysis.GetObservables()[1];
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "constant scatter", "A constant observable has no scatter", Y.m_Scatter, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "constant ratio", "A constant observable has no ratio", Y.m_Ratio, 0.0, 0.0) && Passed;

  // An even number of values: the median is the mean of the two in the middle, an odd number: the one in the middle
  MEndToEndCampaignAnalysis Odd;
  Odd.SetMinimumRuns(3);
  for (double Value: { 9.0, 1.0, 5.0 }) {
    Odd.AddRun(MakeRun(Value, 0.0, 1.0));
  }
  Odd.Analyze();
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::Analyze()", "odd median", "The median of 9, 1, 5", Odd.GetObservables()[0].m_Median, 5.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaignAnalysis::TestGates()
{
  bool Passed = true;

  // No runs
  MEndToEndCampaignAnalysis Empty;
  Empty.Analyze();
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::HasErrors()", "no runs", "A campaign without runs has an error", Empty.HasErrors()) && Passed;

  // Too few runs: one below the minimum is an error, the minimum is fine
  MEndToEndCampaignAnalysis Few;
  Few.SetMinimumRuns(5);
  for (unsigned int r = 0; r < 4; ++r) {
    Few.AddRun(MakeRun(r, 1.0, 1.0));
  }
  Few.Analyze();
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::HasErrors()", "n-1", "One run less than the minimum is an error", Few.HasErrors()) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "text", "The error names the numbers", Count(Few, MCampaignSeverity::c_Error, "There are 4 runs, at least 5"), 1U) && Passed;
  Few.AddRun(MakeRun(4.0, 1.0, 1.0));
  Few.Analyze();
  Passed = EvaluateFalse("MEndToEndCampaignAnalysis::HasErrors()", "n", "The minimum number of runs is enough", Few.HasErrors()) && Passed;
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::GetFindings()", "repeat", "Analyzing again does not keep the findings of the first time", Few.GetFindings().size(), Few.GetFindings().size()) && Passed;

  // A failed check in one run
  MEndToEndCampaignAnalysis Failing;
  Failing.SetMinimumRuns(3);
  Failing.AddRun(MakeRun(1.0, 1.0, 1.0));
  Failing.AddRun(MakeRun(2.0, 1.0, 1.0, 2));
  Failing.AddRun(MakeRun(3.0, 1.0, 1.0));
  Failing.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "failed checks", "A run with failed checks is an error", Count(Failing, MCampaignSeverity::c_Error, "Run 1 has 2 failed checks"), 1U) && Passed;

  // Another environment in one run
  MEndToEndCampaignAnalysis Mixed;
  Mixed.SetMinimumRuns(3);
  Mixed.AddRun(MakeRun(1.0, 1.0, 1.0));
  Mixed.AddRun(MakeRun(2.0, 1.0, 1.0));
  ETCampaignRun Other = MakeRun(3.0, 1.0, 1.0);
  Other.m_Fingerprint["geant4"] = "11.2.0";
  Mixed.AddRun(Other);
  Mixed.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "environment", "A run in another environment is an error", Count(Mixed, MCampaignSeverity::c_Error, "Run 2 was made in another environment"), 1U) && Passed;
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::GetFingerprint()", "mixed", "There is no common environment", Mixed.GetFingerprint().size(), 0) && Passed;

  // No environment recorded
  MEndToEndCampaignAnalysis Unknown;
  Unknown.SetMinimumRuns(1);
  ETCampaignRun Silent = MakeRun(1.0, 1.0, 1.0);
  Silent.m_Fingerprint.clear();
  Unknown.AddRun(Silent);
  Unknown.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "no environment", "Runs without an environment are an error", Count(Unknown, MCampaignSeverity::c_Error, "did not record their environment"), 1U) && Passed;

  // An environment with only some keys known, or with a key which is "unknown"
  MEndToEndCampaignAnalysis Partial;
  Partial.SetMinimumRuns(1);
  ETCampaignRun OnlyRoot = MakeRun(1.0, 1.0, 1.0);
  OnlyRoot.m_Fingerprint = { { "root", "6.40.04" } };
  Partial.AddRun(OnlyRoot);
  Partial.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "partial environment", "Runs which recorded only ROOT are an error naming the other keys", Count(Partial, MCampaignSeverity::c_Error, "unknown: geant4, geometry, scenarios"), 1U) && Passed;
  MEndToEndCampaignAnalysis NoGeometry;
  NoGeometry.SetMinimumRuns(1);
  ETCampaignRun Unreadable = MakeRun(1.0, 1.0, 1.0);
  Unreadable.m_Fingerprint["geometry"] = "unknown";
  NoGeometry.AddRun(Unreadable);
  NoGeometry.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "unknown geometry", "A geometry which is unknown is an error", Count(NoGeometry, MCampaignSeverity::c_Error, "unknown: geometry"), 1U) && Passed;

  // A good campaign has no error and knows its environment
  MEndToEndCampaignAnalysis Good;
  Good.SetMinimumRuns(3);
  for (unsigned int r = 0; r < 3; ++r) {
    Good.AddRun(MakeRun(r, 1.0, 1.0));
  }
  Good.Analyze();
  Passed = EvaluateFalse("MEndToEndCampaignAnalysis::HasErrors()", "good", "A good campaign has no error", Good.HasErrors()) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::GetFingerprint()", "good", "The common environment is known", Good.GetFingerprint().at("geant4"), MString("10.2.3")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaignAnalysis::TestFindings()
{
  bool Passed = true;

  // Missing, doubled, and invalid observables are errors, the observable is not analyzed
  MEndToEndCampaignAnalysis Analysis;
  Analysis.SetMinimumRuns(3);
  Analysis.AddRun(MakeRun(1.0, 1.0, 1.0));
  ETCampaignRun Lacking = MakeRun(2.0, 1.0, 1.0);
  MEndToEndResultList WithoutFirst;
  for (unsigned int k = 1; k < Lacking.m_Results.GetResults().size(); ++k) {
    WithoutFirst.AddResult(Lacking.m_Results.GetResults()[k]);
  }
  Lacking.m_Results = WithoutFirst;
  Analysis.AddRun(Lacking);
  ETCampaignRun Invalid = MakeRun(3.0, numeric_limits<double>::quiet_NaN(), 1.0);
  MEndToEndResult Extra;
  Extra.SetFromReferences(true);
  Extra.SetName("A/new");
  Invalid.m_Results.AddResult(Extra);
  Analysis.AddRun(Invalid);
  Analysis.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "missing", "An observable which is missing in a run is an error", Count(Analysis, MCampaignSeverity::c_Error, "A/x is missing in run 1"), 1U) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "not finite", "An observable which is not finite is an error", Count(Analysis, MCampaignSeverity::c_Error, "A/y is not finite in run 2"), 1U) && Passed;

  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "new", "An observable which the first run does not have is an error", Count(Analysis, MCampaignSeverity::c_Error, "A/new is in run 2 but not in run 0"), 1U) && Passed;
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::GetObservables()", "excluded", "Incomplete observables have no statistics", Analysis.GetObservables().size(), 0) && Passed;

  // A negative uncertainty is an error, and the observable has no statistics
  MEndToEndCampaignAnalysis Negative;
  Negative.SetMinimumRuns(2);
  Negative.AddRun(MakeRun(1.0, 1.0, 1.0));
  Negative.AddRun(MakeRun(2.0, 2.0, -1.0));
  Negative.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "negative error", "A negative uncertainty is an error", Count(Negative, MCampaignSeverity::c_Error, "has a negative uncertainty in run 1"), 2U) && Passed;
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::Analyze()", "negative error, observables", "Observables with a negative uncertainty have no statistics", Negative.GetObservables().size(), 0) && Passed;

  MEndToEndCampaignAnalysis Doubled;
  Doubled.SetMinimumRuns(2);
  Doubled.AddRun(MakeRun(1.0, 1.0, 1.0));
  ETCampaignRun Twice = MakeRun(2.0, 1.0, 1.0);
  Twice.m_Results.AddResult(Twice.m_Results.GetResults()[0]);
  Doubled.AddRun(Twice);
  Doubled.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "twice", "An observable which is twice in a run is an error", Count(Doubled, MCampaignSeverity::c_Error, "A/x is twice in run 1"), 1U) && Passed;

  // The ratio of the scatter and the bootstrap error: just inside 0.7 and 1.4 is fine, just outside is a warning
  // The values -1, 1 (scatter sqrt(2)... ) are replaced by a pair with a known scatter: the runs 0 and 2*S give the mean S and the scatter S*sqrt(2)
  MEndToEndCampaignAnalysis Ratio;
  Ratio.SetMinimumRuns(2);
  Ratio.AddRun(MakeRun(0.0, 0.0, 1.0));
  Ratio.AddRun(MakeRun(2.0, 0.0, 1.0));
  Ratio.Analyze();
  // The scatter is sqrt(2) = 1.4142 times the error 1: above 1.4
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "ratio above", "A ratio of 1.4142 is above the largest expected", Count(Ratio, MCampaignSeverity::c_Warning, "A/x: the scatter of the runs is 1.41"), 1U) && Passed;
  // The scatter is sqrt(2) = 1.41421: the error 1.0103 gives the ratio 1.3998, just inside, the error 1.0100 gives 1.4002, just outside
  MEndToEndCampaignAnalysis Inside;
  Inside.SetMinimumRuns(2);
  Inside.AddRun(MakeRun(0.0, 0.0, 1.0103));
  Inside.AddRun(MakeRun(2.0, 0.0, 1.0103));
  Inside.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "ratio just inside", "A ratio of 1.3998 is inside", Count(Inside, MCampaignSeverity::c_Warning, "the scatter of the runs"), 0U) && Passed;
  MEndToEndCampaignAnalysis Outside;
  Outside.SetMinimumRuns(2);
  Outside.AddRun(MakeRun(0.0, 0.0, 1.0100));
  Outside.AddRun(MakeRun(2.0, 0.0, 1.0100));
  Outside.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "ratio just outside", "A ratio of 1.4002 is outside", Count(Outside, MCampaignSeverity::c_Warning, "A/x: the scatter of the runs is 1.4"), 1U) && Passed;
  // The error 2.0200 gives the ratio 0.7001, just inside the smallest expected, the error 2.0204 gives 0.6999, just outside
  MEndToEndCampaignAnalysis LowInside;
  LowInside.SetMinimumRuns(2);
  LowInside.AddRun(MakeRun(0.0, 0.0, 2.0200));
  LowInside.AddRun(MakeRun(2.0, 0.0, 2.0200));
  LowInside.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "ratio low inside", "A ratio of 0.7001 is inside", Count(LowInside, MCampaignSeverity::c_Warning, "the scatter of the runs"), 0U) && Passed;
  MEndToEndCampaignAnalysis LowOutside;
  LowOutside.SetMinimumRuns(2);
  LowOutside.AddRun(MakeRun(0.0, 0.0, 2.0204));
  LowOutside.AddRun(MakeRun(2.0, 0.0, 2.0204));
  LowOutside.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "ratio low outside", "A ratio of 0.6999 is outside", Count(LowOutside, MCampaignSeverity::c_Warning, "the scatter of the runs"), 1U) && Passed;
  MEndToEndCampaignAnalysis Below;
  Below.SetMinimumRuns(2);
  Below.AddRun(MakeRun(0.0, 0.0, 3.0));
  Below.AddRun(MakeRun(2.0, 0.0, 3.0));
  Below.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "ratio below", "A ratio of 0.47 is below the smallest expected", Count(Below, MCampaignSeverity::c_Warning, "A/x: the scatter of the runs is 0.471"), 1U) && Passed;

  // No scatter although the bootstrap error is not zero
  MEndToEndCampaignAnalysis Flat;
  Flat.SetMinimumRuns(2);
  Flat.AddRun(MakeRun(1.0, 0.0, 1.0));
  Flat.AddRun(MakeRun(1.0, 0.0, 1.0));
  Flat.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "no scatter", "Runs which do not scatter although the bootstrap error is not zero are a warning", Count(Flat, MCampaignSeverity::c_Warning, "do not scatter"), 2U) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaignAnalysis::HasErrors()", "warnings", "Warnings are no errors", Flat.HasErrors()) && Passed;
  // A single run cannot scatter: no warning
  MEndToEndCampaignAnalysis Single;
  Single.SetMinimumRuns(1);
  Single.AddRun(MakeRun(1.0, 0.0, 1.0));
  Single.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "single run", "A single run has no warning about the scatter", Count(Single, MCampaignSeverity::c_Warning, "do not scatter"), 0U) && Passed;

  // An outlier: 19 values of 0, 1, -1 (median 0, median absolute deviation 1, robust scatter 1.4826) and one value of 9: beyond 6 robust scatters = 8.8956
  vector<double> Values;
  for (unsigned int i = 0; i < 6; ++i) {
    Values.push_back(0.0);
    Values.push_back(1.0);
    Values.push_back(-1.0);
  }
  Values.push_back(0.0);
  MEndToEndCampaignAnalysis Outlier;
  Outlier.SetMinimumRuns(2);
  for (double Value: Values) {
    Outlier.AddRun(MakeRun(Value, 0.0, 1.0));
  }
  Outlier.AddRun(MakeRun(9.0, 0.0, 1.0));
  Outlier.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "outlier", "A value far from the others is a warning", Count(Outlier, MCampaignSeverity::c_Warning, "run 19 (9) is an outlier"), 1U) && Passed;
  MEndToEndCampaignAnalysis Near;
  Near.SetMinimumRuns(2);
  for (double Value: Values) {
    Near.AddRun(MakeRun(Value, 0.0, 1.0));
  }
  Near.AddRun(MakeRun(8.8, 0.0, 1.0));
  Near.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "no outlier", "A value just inside six robust scatters is no outlier", Count(Near, MCampaignSeverity::c_Warning, "is an outlier"), 0U) && Passed;

  // A drift: the first half 0 and the second half 1 with a small scatter: far beyond 4 standard errors
  MEndToEndCampaignAnalysis Drift;
  Drift.SetMinimumRuns(8);
  for (unsigned int r = 0; r < 8; ++r) {
    Drift.AddRun(MakeRun((r < 4 ? 0.0 : 1.0) + 0.01*(r % 2), 0.0, 1.0));
  }
  Drift.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "drift", "Halves which differ are a warning", Count(Drift, MCampaignSeverity::c_Warning, "the first and the last half of the runs differ"), 1U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaignAnalysis::TestCoverage()
{
  bool Passed = true;

  // Gaussian runs with a fixed seed: 50 runs of two observables, the pulls are Student's t with 48 degrees of freedom
  mt19937 Random(4711);
  normal_distribution<double> Gauss(10.0, 2.0);
  MEndToEndCampaignAnalysis Analysis;
  Analysis.SetMinimumRuns(50);
  for (unsigned int r = 0; r < 50; ++r) {
    Analysis.AddRun(MakeRun(Gauss(Random), Gauss(Random), 2.0));
  }
  Analysis.Analyze();
  const ETCampaignCoverage& Coverage = Analysis.GetCoverage();
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::GetCoverage()", "limits", "The limits are 1 to 5 sigma", Coverage.m_Sigmas.size(), 5) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::GetCoverage()", "pulls", "Every run is compared with the others for both observables", Coverage.m_NumberOfPulls, 100U) && Passed;
  // The expected number beyond k sigma is 100 * 2 * (1 - StudentI(k, 48)): beyond 1 sigma about 32
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::GetCoverage()", "expected 1 sigma", "The expected number beyond one sigma of Student's t distribution with 48 degrees of freedom", Coverage.m_Expected[0], 100.0*2.0*(1.0 - TMath::StudentI(1.0, 48.0)), 1e-9) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::GetCoverage()", "expected 1 sigma value", "It is about 32", Coverage.m_Expected[0] > 30.0 && Coverage.m_Expected[0] < 34.0) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::GetCoverage()", "expected 5 sigma", "Beyond five sigma almost nothing is expected", Coverage.m_Expected[4] < 0.001) && Passed;
  // For the seed the counts are fixed: the observed numbers must be plausible for 100 comparisons (binomial: 32 +- 5 beyond one sigma, at most a few beyond three)
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::GetCoverage()", "observed 1 sigma", "Beyond one sigma about 32 of 100 comparisons are expected", Coverage.m_Outside[0] >= 18 && Coverage.m_Outside[0] <= 46) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::GetCoverage()", "observed 3 sigma", "Beyond three sigma at most four of 100", Coverage.m_Outside[2] <= 4) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::GetCoverage()", "monotonic", "The counts decrease with the limit", Coverage.m_Outside[0] >= Coverage.m_Outside[1] && Coverage.m_Outside[1] >= Coverage.m_Outside[2] && Coverage.m_Outside[2] >= Coverage.m_Outside[3] && Coverage.m_Outside[3] >= Coverage.m_Outside[4]) && Passed;

  Passed = Evaluate("MEndToEndCampaignAnalysis::GetCoverage()", "all pulls", "All pulls are kept for the histogram", Coverage.m_Pulls.size(), static_cast<size_t>(100)) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::GetCoverage()", "degrees of freedom", "The pulls of 50 runs have 48 degrees of freedom", Coverage.m_DegreesOfFreedom, 48U) && Passed;

  // The report of the analysis is written as PDF
  const MString Report = GetTemporaryDirectoryName("report") + "/ETReport.campaign.pdf";
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::WriteReport()", "written", "The report of the campaign is written, the directory is created", Analysis.WriteReport("ETReport", Report)) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::WriteReport()", "exists", "The report exists", MFile::Exists(Report)) && Passed;
  MEndToEndCampaignAnalysis Nothing;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::WriteReport()", "no runs", "The report of a campaign without runs is written", Nothing.WriteReport("ETNothing", GetTemporaryDirectoryName("report") + "/ETNothing.campaign.pdf")) && Passed;

  // A report which cannot be written is a failure: a directory occupies the place of the PDF
  const MString Blocked = GetTemporaryDirectoryName("blocked") + "/ETBlocked.campaign.pdf";
  Passed = EvaluateTrue("MFile::CreateDirectory()", "blocked", "The directory in the place of the report can be created", MFile::CreateDirectory(Blocked)) && Passed;
  DisableDefaultStreams();
  const bool BlockedWritten = Analysis.WriteReport("ETBlocked", Blocked);
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaignAnalysis::WriteReport()", "blocked", "A report which cannot be written is a failure", BlockedWritten) && Passed;

  // A run which is far away is beyond the limits: 9 runs with 0, 1 and one run with 1000
  MEndToEndCampaignAnalysis Far;
  Far.SetMinimumRuns(10);
  for (unsigned int r = 0; r < 9; ++r) {
    Far.AddRun(MakeRun(r % 2, 0.0, 1.0));
  }
  Far.AddRun(MakeRun(1000.0, 0.0, 1.0));
  Far.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::GetCoverage()", "far", "The far run is beyond five sigma of the others", Far.GetCoverage().m_Outside[4], 1U) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::Analyze()", "far finding", "It is a finding", Count(Far, MCampaignSeverity::c_Warning, "comparisons of a run with the others are beyond"), 1U) && Passed;

  // Observables which do not scatter and too few runs have no pulls
  MEndToEndCampaignAnalysis Constant;
  Constant.SetMinimumRuns(4);
  for (unsigned int r = 0; r < 4; ++r) {
    Constant.AddRun(MakeRun(1.0, 2.0, 1.0));
  }
  Constant.Analyze();
  Passed = Evaluate("MEndToEndCampaignAnalysis::GetCoverage()", "constant", "Observables which do not scatter have no pulls", Constant.GetCoverage().m_NumberOfPulls, 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaignAnalysis::TestReferences()
{
  bool Passed = true;

  // Runs 1 ... 10 of x with bootstrap error 2
  MEndToEndCampaignAnalysis Analysis;
  Analysis.SetMinimumRuns(10);
  for (unsigned int r = 1; r <= 10; ++r) {
    Analysis.AddRun(MakeRun(r, 5.0, 2.0));
  }
  Analysis.Analyze();
  const MEndToEndReferenceList References = Analysis.MakeReferences("ETTest", "A test of the campaign");
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::MakeReferences()", "number", "There is a reference for every observable", References.GetNumberOfReferences(), 2) && Passed;
  const MEndToEndReference X = References.GetReference("A/x");
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::MakeReferences()", "mean", "The mean", X.GetMean(), 5.5, 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::MakeReferences()", "error", "The standard error of the mean", X.GetError(), sqrt(82.5/9.0)/sqrt(10.0), 1e-12) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::MakeReferences()", "scatter", "The scatter", X.GetScatter(), sqrt(82.5/9.0), 1e-12) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::MakeReferences()", "runs", "The number of runs", X.GetRuns(), 10U) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::MakeReferences()", "calibration", "The calibration is the ratio of the scatter and the bootstrap error", X.GetCalibration(), sqrt(82.5/9.0)/2.0, 1e-12) && Passed;
  // No floor: a floor larger than the statistical tolerance would hide shifts of many sigma and take the comparison out of the pull statistics
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::MakeReferences()", "relative floor", "The references have no relative floor: the statistics alone set the tolerance", X.GetRelativeFloor(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::MakeReferences()", "absolute floor", "The references have no absolute floor", X.GetAbsoluteFloor(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndCampaignAnalysis::MakeReferences()", "no calibration", "An observable which does not scatter keeps the calibration one", References.GetReference("A/y").GetCalibration(), 1.0, 0.0) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::MakeReferences()", "test", "The record names the test", References.GetHeader().Get("test"), MString("ETTest")) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::MakeReferences()", "reason", "The record has the reason", References.GetHeader().Get("reason"), MString("A test of the campaign")) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::MakeReferences()", "runs header", "The record has the number of runs", References.GetHeader().Get("runs"), MString("10")) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::MakeReferences()", "geant4", "The record has the environment of the runs", References.GetHeader().Get("geant4"), MString("10.2.3")) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::MakeReferences()", "scenarios", "The record has the hash of the scenarios", References.GetHeader().Get("scenarios"), MString("00000000000000bb")) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::MakeReferences()", "date", "The record has the date", References.GetHeader().Get("date").Length() == 10) && Passed;
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::MakeReferences()", "fingerprint", "The references match the environment of the runs", MEndToEndFingerprint::Compare(References.GetHeader(), Analysis.GetFingerprint()).size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaignAnalysis::TestReadRun()
{
  bool Passed = true;

  // The files of a run: the key results and the environment of the test
  const MString Directory = GetTemporaryDirectoryName("run");
  Passed = EvaluateTrue("MFile::CreateDirectory()", "run", "The directory of the run can be created", MFile::CreateDirectory(Directory)) && Passed;
  {
    MEndToEndResultList Results;
    MEndToEndResult& First = Results.AddResult("A/x", 4.5, 0.25);
    First.SetFromReferences(true);
    Results.AddResult("A/derived", 1.0, 0.1);
    Results.Write(Directory + "/ETRun.keyresults.tsv");
    MEndToEndRecord Environment;
    Environment.Set("test", "ETRun");
    Environment.Set("geant4", "10.2.3");
    Environment.Set("root", "6.40.04");
    Environment.Set("geometry", "00000000000000aa");
    Environment.Set("scenarios", "00000000000000bb");
    Passed = EvaluateTrue("MEndToEndRecord::Write()", "fingerprint", "The file of the fingerprint can be written", Environment.Write(Directory + "/ETRun.fingerprint.tsv", "Fingerprint")) && Passed;
  }
  ETCampaignRun Run;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::ReadRun()", "read", "A run can be read", MEndToEndCampaignAnalysis::ReadRun(Directory, "ETRun", 3, Run)) && Passed;
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::ReadRun()", "key results", "Both key results are read", Run.m_Results.GetResults().size(), 2) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaignAnalysis::ReadRun()", "from references", "The flag of the reference is read", Run.m_Results.GetResults()[0].IsFromReferences()) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaignAnalysis::ReadRun()", "derived", "A derived key result has no flag of the reference", Run.m_Results.GetResults()[1].IsFromReferences()) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::ReadRun()", "failed", "The number of failed checks is the given one", Run.m_FailedChecks, 3U) && Passed;
  Passed = Evaluate("MEndToEndCampaignAnalysis::ReadRun()", "geant4", "The environment is read", Run.m_Fingerprint.at("geant4"), MString("10.2.3")) && Passed;
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::ReadRun()", "keys", "The environment has the four keys of the fingerprint", Run.m_Fingerprint.size(), 4) && Passed;

  // A directory without the files
  DisableDefaultStreams();
  const bool Missing = MEndToEndCampaignAnalysis::ReadRun(Directory, "ETMissing", 0, Run);
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaignAnalysis::ReadRun()", "missing", "A run without key results cannot be read", Missing) && Passed;
  Passed = EvaluateSize("MEndToEndCampaignAnalysis::ReadRun()", "missing key results", "A failed read leaves no key results", Run.m_Results.GetResults().size(), 0) && Passed;

  // A key result which is malformed makes the run unreadable: it is not skipped
  {
    ofstream Out((Directory + "/ETRun.keyresults.tsv").Data(), ios::app);
    Out<<"A/nan\tnan\t0.1\t0\t0\t0\t0\t0\t1\t0\t1\t1"<<endl;
  }
  ETCampaignRun Malformed;
  DisableDefaultStreams();
  const bool MalformedRead = MEndToEndCampaignAnalysis::ReadRun(Directory, "ETRun", 0, Malformed);
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaignAnalysis::ReadRun()", "malformed", "A run with a malformed key result cannot be read", MalformedRead) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("UTEndToEndCampaignAnalysis", "unit test of the analysis of an end-to-end campaign") == false) {
    return 1;
  }

  // ROOT runs without graphics - the canvases of the summary are written without a display
  TApplication ROOT("ROOT", 0, 0);
  gROOT->SetBatch(true);

  UTEndToEndCampaignAnalysis Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTEndToEndCampaignAnalysis.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
