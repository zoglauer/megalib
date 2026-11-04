/*
 * UTStatistics.cxx
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


// MEGAlib:
#include "MGlobal.h"
#include "MStatistics.h"
#include "MUnitTest.h"

// Standard libs:
#include <cmath>
#include <limits>
#include <vector>
using namespace std;

// ROOT libs:
#include "TH1D.h"


//! Unit test class for MStatistics
class UTStatistics : public MUnitTest
{
public:
  UTStatistics() : MUnitTest("UTStatistics") {}
  virtual ~UTStatistics() {}

  virtual bool Run();

private:
  bool TestMoments();
  bool TestDensityMean();
  bool TestOrderStatistics();
  bool TestPeaks();
  bool TestBootstrap();
  bool TestSignificances();
  bool TestGuards();
  bool TestNonGaussian();
};


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::Run()
{
  bool Passed = true;

  Passed = TestMoments() && Passed;
  Passed = TestDensityMean() && Passed;
  Passed = TestOrderStatistics() && Passed;
  Passed = TestPeaks() && Passed;
  Passed = TestBootstrap() && Passed;
  Passed = TestSignificances() && Passed;
  Passed = TestGuards() && Passed;
  Passed = TestNonGaussian() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::TestMoments()
{
  bool Passed = true;

  // 1 ... 5: mean 3, squared deviations 4, 1, 0, 1, 4 = 10, standard deviation sqrt(10/4) with the divisor n - 1
  const vector<double> Five = { 4.0, 1.0, 5.0, 2.0, 3.0 };
  Passed = EvaluateNear("GetMean()", "five", "The mean of 1 ... 5", MStatistics::GetMean(Five), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMean()", "negative", "The mean of -3, -1, 2, 4 is 0.5", MStatistics::GetMean(vector<double>{-3.0, -1.0, 2.0, 4.0}), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetMean()", "single", "The mean of a single value is the value", MStatistics::GetMean(vector<double>{7.0}), 7.0, 0.0) && Passed;
  Passed = EvaluateNear("GetMean()", "empty", "The mean of no values is zero", MStatistics::GetMean(vector<double>()), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetStandardDeviation()", "five", "The standard deviation of 1 ... 5 is sqrt(10/4)", MStatistics::GetStandardDeviation(Five), sqrt(2.5), 1e-12) && Passed;
  // 2, 4, 4, 4, 5, 5, 7, 9: mean 5, squared deviations 9, 1, 1, 1, 0, 0, 4, 16 = 32, sqrt(32/7)
  Passed = EvaluateNear("GetStandardDeviation()", "eight", "The standard deviation of eight values with the divisor n - 1", MStatistics::GetStandardDeviation(vector<double>{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0}), sqrt(32.0/7.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetStandardDeviation()", "identical", "The standard deviation of identical values is zero", MStatistics::GetStandardDeviation(vector<double>{3.0, 3.0, 3.0}), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetStandardDeviation()", "single", "A single value has no standard deviation", MStatistics::GetStandardDeviation(vector<double>{3.0}), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetStandardDeviation()", "empty", "No values have no standard deviation", MStatistics::GetStandardDeviation(vector<double>()), 0.0, 0.0) && Passed;

  // -3, -1, 0, 2, 5 about the center 0 with the limit 3: -1, 0, 2 are inside (the limit excludes -3), mean 1/3, RMS about zero sqrt((1 + 0 + 4)/3)
  const vector<double> Spread = { -3.0, -1.0, 0.0, 2.0, 5.0 };
  unsigned int Number = 99;
  double Mean = 99.0;
  double RMS = 99.0;
  MStatistics::GetTruncatedMoments(Spread, 0.0, 3.0, Number, Mean, RMS);
  Passed = Evaluate("GetTruncatedMoments()", "limit 3 number", "Three values are inside the limit, the value at the limit is outside", Number, 3U) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "limit 3 mean", "The mean of -1, 0, 2", Mean, 1.0/3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "limit 3 RMS", "The RMS about zero of -1, 0, 2", RMS, sqrt(5.0/3.0), 1e-12) && Passed;
  // Just above the limit -3 is inside: four values, mean -0.5, RMS sqrt((9 + 1 + 0 + 4)/4)
  MStatistics::GetTruncatedMoments(Spread, 0.0, 3.0 + 1e-9, Number, Mean, RMS);
  Passed = Evaluate("GetTruncatedMoments()", "just above limit number", "Four values are inside a limit just above 3", Number, 4U) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "just above limit mean", "The mean of -3, -1, 0, 2", Mean, -0.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "just above limit RMS", "The RMS about zero of -3, -1, 0, 2", RMS, sqrt(3.5), 1e-12) && Passed;
  // Nothing inside: the number is zero and the moments are zero
  MStatistics::GetTruncatedMoments(Spread, 0.0, 0.0, Number, Mean, RMS);
  Passed = Evaluate("GetTruncatedMoments()", "nothing inside number", "No value is inside the limit zero", Number, 0U) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "nothing inside mean", "The mean is zero if nothing is inside", Mean, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "nothing inside RMS", "The RMS is zero if nothing is inside", RMS, 0.0, 0.0) && Passed;
  MStatistics::GetTruncatedMoments(vector<double>(), 0.0, 3.0, Number, Mean, RMS);
  Passed = Evaluate("GetTruncatedMoments()", "empty", "No values: the number is zero", Number, 0U) && Passed;
  // About the center 2 with the limit 3: 0 and 2 are inside (-1 and 5 are at the limit), the mean is their mean 1, the RMS about 2 is sqrt((4 + 0)/2)
  MStatistics::GetTruncatedMoments(Spread, 2.0, 3.0, Number, Mean, RMS);
  Passed = Evaluate("GetTruncatedMoments()", "center 2 number", "Two values are closer to 2 than 3", Number, 2U) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "center 2 mean", "The mean of 0 and 2 is their mean, not relative to the center", Mean, 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "center 2 RMS", "The RMS of 0 and 2 about the center 2", RMS, sqrt(2.0), 1e-12) && Passed;
  // Just above the limit -1 and 5 are inside: mean (-1 + 0 + 2 + 5)/4, RMS about 2 sqrt((9 + 4 + 0 + 9)/4)
  MStatistics::GetTruncatedMoments(Spread, 2.0, 3.0 + 1e-9, Number, Mean, RMS);
  Passed = Evaluate("GetTruncatedMoments()", "center 2 just above limit number", "Four values are closer to 2 than a limit just above 3", Number, 4U) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "center 2 just above limit mean", "The mean of -1, 0, 2, 5", Mean, 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "center 2 just above limit RMS", "The RMS of -1, 0, 2, 5 about the center 2", RMS, sqrt(5.5), 1e-12) && Passed;


  // Rounding: the mean of 1e16, 1, -1e16 is 1/3 (a plain sum gives 0), ten identical values have exactly their value as mean and no spread
  Passed = EvaluateNear("GetMean()", "cancellation", "The mean of 1e16, 1, -1e16 is 1/3", MStatistics::GetMean(vector<double>{1e16, 1.0, -1e16}), 1.0/3.0, 1e-15) && Passed;
  // In every order: the quotient of the shifted sum must not round at the magnitude of the shift (1e15 + (-2e15 + 1)/3 gave 0.375)
  const vector<vector<double>> Orders = { { 1e15, 1.0, -1e15 }, { 1e15, -1e15, 1.0 }, { 1.0, 1e15, -1e15 }, { 1.0, -1e15, 1e15 }, { -1e15, 1e15, 1.0 }, { -1e15, 1.0, 1e15 } };
  for (unsigned int o = 0; o < Orders.size(); ++o) {
    Passed = EvaluateNear("GetMean()", MString("cancellation, order ") + o, "The mean of 1e15, 1, -1e15 is 1/3 in every order of the values", MStatistics::GetMean(Orders[o]), 1.0/3.0, 1e-15) && Passed;
  }
  const vector<double> Identical(10, 1e15 + 0.1);
  Passed = EvaluateNear("GetMean()", "identical large values", "The mean of ten identical values is exactly the value", MStatistics::GetMean(Identical), Identical[0], 0.0) && Passed;
  Passed = EvaluateNear("GetStandardDeviation()", "identical large values", "Ten identical values have exactly no spread", MStatistics::GetStandardDeviation(Identical), 0.0, 0.0) && Passed;
  MStatistics::GetTruncatedMoments(Identical, Identical[0], 1.0, Number, Mean, RMS);
  Passed = EvaluateNear("GetTruncatedMoments()", "identical large values mean", "The truncated mean of ten identical values is exactly the value", Mean, Identical[0], 0.0) && Passed;
  Passed = EvaluateNear("GetTruncatedMoments()", "identical large values RMS", "The RMS about the value itself is zero", RMS, 0.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::TestDensityMean()
{
  bool Passed = true;

  // Linear grid (Min = 0): the mean of the uniform density on [0, 2] is 1, of f(x) = x on [0, 1] it is 2/3, of exp(-x/2) on [0, 200] it is 2
  Passed = EvaluateNear("GetMeanOfDensity()", "uniform", "The mean of a uniform density on [0, 2] is 1", MStatistics::GetMeanOfDensity([](double) { return 1.0; }, 0.0, 2.0), 1.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetMeanOfDensity()", "linear", "The mean of f(x) = x on [0, 1] is 2/3", MStatistics::GetMeanOfDensity([](double X) { return X; }, 0.0, 1.0), 2.0/3.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetMeanOfDensity()", "exponential", "The mean of exp(-x/2) on [0, 200] is 2", MStatistics::GetMeanOfDensity([](double X) { return exp(-X/2.0); }, 0.0, 200.0), 2.0, 1e-6) && Passed;
  // A normalization of the density does not matter
  Passed = EvaluateNear("GetMeanOfDensity()", "not normalized", "The mean does not depend on the normalization of the density", MStatistics::GetMeanOfDensity([](double X) { return 1000.0*X; }, 0.0, 1.0), 2.0/3.0, 1e-9) && Passed;
  // Negative values are fine on the linear grid: a uniform density on [-1, 1] has the mean 0
  Passed = EvaluateNear("GetMeanOfDensity()", "negative range", "The mean of a uniform density on [-1, 1] is 0", MStatistics::GetMeanOfDensity([](double) { return 1.0; }, -1.0, 1.0), 0.0, 1e-9) && Passed;

  // Logarithmic grid (Min > 0): the mean of x^-2 on [1, 100] is ln(100)/(1 - 1/100)
  Passed = EvaluateNear("GetMeanOfDensity()", "power law", "The mean of x^-2 on [1, 100] is ln(100)/(1 - 1/100)", MStatistics::GetMeanOfDensity([](double X) { return 1.0/(X*X); }, 1.0, 100.0), log(100.0)/0.99, 1e-6) && Passed;
  // Over four decades the logarithmic grid still resolves the density: x^-1.5 on [1, 10000] has the mean (sqrt(10000) - 1)/(1 - 1/sqrt(10000)) = 100
  Passed = EvaluateNear("GetMeanOfDensity()", "decades", "The mean of x^-1.5 on [1, 10000] is 100", MStatistics::GetMeanOfDensity([](double X) { return pow(X, -1.5); }, 1.0, 10000.0), 100.0, 1e-6) && Passed;

  // Breakpoints: a jump from 1 to 0 at x = 1 on [0, 2] gives the mean 0.5 when the jump is a breakpoint; a kink (the triangle 1 - |x - 1|) gives 1
  Passed = EvaluateNear("GetMeanOfDensity()", "jump with breakpoint", "A density with a jump at a breakpoint is integrated exactly on both sides", MStatistics::GetMeanOfDensity([](double X) { return (X < 1.0) ? 1.0 : 0.0; }, 0.0, 2.0, vector<double>{1.0}), 0.5, 1e-6) && Passed;
  Passed = EvaluateNear("GetMeanOfDensity()", "kink with breakpoint", "A density with a kink at a breakpoint", MStatistics::GetMeanOfDensity([](double X) { return 1.0 - fabs(X - 1.0); }, 0.0, 2.0, vector<double>{1.0}), 1.0, 1e-9) && Passed;
  // Breakpoints outside the range or out of order are ignored
  Passed = EvaluateNear("GetMeanOfDensity()", "breakpoints outside", "Breakpoints outside the range and out of order are ignored", MStatistics::GetMeanOfDensity([](double X) { return (X < 1.0) ? 1.0 : 0.0; }, 0.0, 2.0, vector<double>{5.0, -1.0, 1.0, 0.5}), 0.5, 1e-6) && Passed;
  // Ignored breakpoints must not reduce the accuracy: exp(-x) on [0, 2000] with a breakpoint outside the range is integrated with the full number of intervals,
  // i.e. gives exactly the result without a breakpoint list (200 intervals would give 0.0018 instead of 1)
  Passed = Evaluate("GetMeanOfDensity()", "only ignored breakpoints", "A breakpoint list with only ignored entries gives the same result as no list", MStatistics::GetMeanOfDensity([](double X) { return exp(-X); }, 0.0, 2000.0, vector<double>{-1.0}), MStatistics::GetMeanOfDensity([](double X) { return exp(-X); }, 0.0, 2000.0)) && Passed;
  // The logarithmic grid must not form the ratio Max/Min, which overflows for a tiny minimum: a uniform density on [1e-310, 1] has the mean 0.5
  Passed = EvaluateNear("GetMeanOfDensity()", "tiny minimum", "A uniform density on [1e-310, 1] has the mean 0.5 (no overflow of the ratio of the bounds)", MStatistics::GetMeanOfDensity([](double) { return 1.0; }, 1e-310, 1.0), 0.5, 1e-5) && Passed;
  // A narrow range far from zero: log(Max) - log(Min) would round to zero, the mean of a uniform density on [1e8, 1e8 + 1e-7] is 1e8 + 5e-8
  Passed = EvaluateNear("GetMeanOfDensity()", "narrow range", "A uniform density on [1e8, 1e8 + 1e-7] has the mean 1e8 + 5e-8 (no loss of the width in the logarithms)", MStatistics::GetMeanOfDensity([](double) { return 1.0; }, 1e8, 1e8 + 1e-7), 1e8 + 5e-8, 1e-7) && Passed;
  // A narrow negative range: the mirror image, the mean of a uniform density on [-1e8 - 1e-7, -1e8] is -1e8 - 5e-8
  Passed = EvaluateNear("GetMeanOfDensity()", "narrow negative range", "A uniform density on [-1e8 - 1e-7, -1e8] has the mean -1e8 - 5e-8", MStatistics::GetMeanOfDensity([](double) { return 1.0; }, -1e8 - 1e-7, -1e8), -1e8 - 5e-8, 1e-7) && Passed;
  // A density close to zero in a range with a far bound: the first moment must not be taken relative to that bound (1 on [0, 1] over [-1e16, 1] has the mean 0.5)
  Passed = EvaluateNear("GetMeanOfDensity()", "far lower bound", "A density which is 1 on [0, 1] and 0 below, over [-1e16, 1], has the mean 0.5", MStatistics::GetMeanOfDensity([](double X) { return (X >= 0.0) ? 1.0 : 0.0; }, -1e16, 1.0, vector<double>{0.0}), 0.5, 1e-6) && Passed;
  Passed = EvaluateNear("GetMeanOfDensity()", "far upper bound", "A density which is 1 on [-1, 0] and 0 above, over [-1, 1e16], has the mean -0.5", MStatistics::GetMeanOfDensity([](double X) { return (X <= 0.0) ? 1.0 : 0.0; }, -1.0, 1e16, vector<double>{0.0}), -0.5, 1e-6) && Passed;

  // Degenerate input: a zero density and an empty range give zero
  Passed = EvaluateNear("GetMeanOfDensity()", "zero density", "A density which is zero everywhere has the mean zero", MStatistics::GetMeanOfDensity([](double) { return 0.0; }, 0.0, 2.0), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetMeanOfDensity()", "empty range", "An empty range gives the mean zero", MStatistics::GetMeanOfDensity([](double) { return 1.0; }, 1.0, 1.0), 0.0, 0.0) && Passed;


  // A reversed or empty range gives zero, also when a logarithm could not be taken
  Passed = EvaluateNear("GetMeanOfDensity()", "reversed range", "Max below Min gives zero", MStatistics::GetMeanOfDensity([](double) { return 1.0; }, 1.0, -1.0), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetMeanOfDensity()", "reversed range to zero", "Max zero below a positive Min gives zero", MStatistics::GetMeanOfDensity([](double) { return 1.0; }, 1.0, 0.0), 0.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::TestOrderStatistics()
{
  bool Passed = true;

  // Test the median of the absolute values and its standard error 1/(2 f sqrt(N)) with f from the sqrt(N) values on each side of the median:
  // Values (j + 0.5)/10000 for j = 0 ... 4999, each twice - median 0.25 (mean of j = 2499 and 2500)
  // Expected error: 1/(2*2*sqrt(10000)) = 0.0025 with density 2 on [0, 0.5]
  vector<double> Uniform;
  for (int i = 0; i < 5000; ++i) {
    Uniform.push_back((i + 0.5)/10000.0);
    Uniform.push_back((i + 0.5)/10000.0);
  }
  Passed = EvaluateNear("GetMedian()", "uniform", "The median of a uniform distribution", MStatistics::GetMedian(Uniform), 0.25, 1e-12) && Passed;
  Passed = EvaluateNear("GetMedianError()", "uniform", "The error of a uniform distribution follows its density", MStatistics::GetMedianError(Uniform), 0.0025, 1e-9) && Passed;

  // {1, 2, 3} - median 2, f = 2/6 = 1/3, error 1/(2*(1/3)*sqrt(3)) = sqrt(3)/2
  Passed = EvaluateNear("GetMedian()", "three", "The median of an odd number of values", MStatistics::GetMedian(vector<double>{3.0, 1.0, 2.0}), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMedianError()", "three", "The error of three values", MStatistics::GetMedianError(vector<double>{3.0, 1.0, 2.0}), sqrt(3.0)/2.0, 1e-12) && Passed;

  // {1, 2, 3, 4} - median (2 + 3)/2, f = 3/(4*3) = 1/4, error 1/(2*(1/4)*2) = 1
  Passed = EvaluateNear("GetMedian()", "four", "The median of an even number of values is the mean of the two in the middle", MStatistics::GetMedian(vector<double>{4.0, 1.0, 3.0, 2.0}), 2.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetMedianError()", "four", "The error of four values", MStatistics::GetMedianError(vector<double>{4.0, 1.0, 3.0, 2.0}), 1.0, 1e-12) && Passed;

  // {-3, -1, 2, 4} - median (-1 + 2)/2
  Passed = EvaluateNear("GetMedian()", "negative", "The median of values with a sign", MStatistics::GetMedian(vector<double>{-1.0, 2.0, -3.0, 4.0}), 0.5, 1e-12) && Passed;

  // Identical values have no width - error 0; no values - median 0 and error 0
  Passed = EvaluateNear("GetMedian()", "identical", "The median of identical values", MStatistics::GetMedian(vector<double>{5.0, 5.0, 5.0, 5.0}), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMedianError()", "identical", "The error of identical values is zero", MStatistics::GetMedianError(vector<double>{5.0, 5.0, 5.0, 5.0}), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetMedian()", "single", "The median of a single value is the value", MStatistics::GetMedian(vector<double>{7.0}), 7.0, 0.0) && Passed;
  Passed = EvaluateNear("GetMedian()", "empty", "The median of no values is zero", MStatistics::GetMedian(vector<double>()), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetMedianError()", "empty", "The error of no values is zero", MStatistics::GetMedianError(vector<double>()), 0.0, 0.0) && Passed;

  const vector<double> Five = { 4.0, 1.0, 5.0, 2.0, 3.0 };
  Passed = EvaluateNear("GetQuantile()", "median", "The 0.5 quantile of 1 ... 5", MStatistics::GetQuantile(Five, 0.5), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetQuantile()", "minimum", "The 0 quantile is the minimum", MStatistics::GetQuantile(Five, 0.0), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetQuantile()", "maximum", "The 1 quantile is the maximum", MStatistics::GetQuantile(Five, 1.0), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetQuantile()", "interpolation", "The 0.125 quantile of 1 ... 5 lies at position 0.5: 1.5", MStatistics::GetQuantile(Five, 0.125), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetQuantile()", "single", "The quantile of a single value", MStatistics::GetQuantile(vector<double>{7.0}, 0.3), 7.0, 0.0) && Passed;
  Passed = EvaluateNear("GetQuantile()", "empty", "The quantile of no values is zero", MStatistics::GetQuantile(vector<double>(), 0.5), 0.0, 0.0) && Passed;

  // 1 ... 5: median 3, absolute deviations 2, 1, 0, 1, 2 with the median 1
  Passed = EvaluateNear("GetRobustSigma()", "five", "1.4826 times the median absolute deviation of 1 ... 5", MStatistics::GetRobustSigma(Five), 1.4826, 1e-12) && Passed;
  Passed = EvaluateNear("GetRobustSigma()", "factor", "The factor of the median absolute deviation is the one of a Gaussian", MStatistics::c_MedianAbsoluteDeviationFactor, 1.4826, 0.0) && Passed;
  Passed = EvaluateNear("GetRobustSigma()", "identical", "The robust sigma of identical values is zero", MStatistics::GetRobustSigma(vector<double>{2.0, 2.0, 2.0}), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetRobustSigma()", "empty", "The robust sigma of no values is zero", MStatistics::GetRobustSigma(vector<double>()), 0.0, 0.0) && Passed;

  // 1, 2, 3, 4, 10: 80% are four values: the shortest interval is 1 ... 4 (width 3), 100% need 1 ... 10 (width 9)
  const vector<double> Outlier = { 10.0, 1.0, 3.0, 2.0, 4.0 };
  Passed = EvaluateNear("GetHalfWidthContaining()", "80%", "The shortest interval with four of five values is 1 ... 4", MStatistics::GetHalfWidthContaining(Outlier, 0.8), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetHalfWidthContaining()", "just above 80%", "More than four of five values need the whole range", MStatistics::GetHalfWidthContaining(Outlier, 0.81), 4.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetHalfWidthContaining()", "100%", "All values need the whole range", MStatistics::GetHalfWidthContaining(Outlier, 1.0), 4.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetHalfWidthContaining()", "small", "A tiny fraction needs one value: width zero", MStatistics::GetHalfWidthContaining(Outlier, 0.01), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetHalfWidthContaining()", "empty", "No values: zero", MStatistics::GetHalfWidthContaining(vector<double>(), 0.5), 0.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::TestPeaks()
{
  bool Passed = true;

  // A triangle: 100 - 10|k| values at k*0.5, k = -10 ... 10 (1000 values, median 0). The smoothing with a triangle of three bins
  // lowers only the apex (100 -> 95), the half maximum 47.5 is reached at |k| = 5.25: FWHM = 10.5 bins = 5.25
  vector<double> Triangle;
  for (int k = -10; k <= 10; ++k) {
    for (int n = 0; n < 100 - 10*abs(k); ++n) {
      Triangle.push_back(0.5*k);
    }
  }
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "triangle", "The FWHM of a triangle after the smoothing: 10.5 bins of 0.5", MStatistics::GetFullWidthAtHalfMaximum(Triangle, 0.5), 5.25, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "few", "Fewer than 20 values: no peak", MStatistics::GetFullWidthAtHalfMaximum(vector<double>(19, 1.0), 0.5), 0.0, 0.0) && Passed;
  // 20 identical values are one bin of 20: the smoothing makes 5, 10, 5 (half maximum 5), the bins next to them are the first below 5:
  // the crossings are at the far edges of the neighbors of the peak bin: 2 bins of 0.5
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "twenty", "20 identical values are a peak of two bins", MStatistics::GetFullWidthAtHalfMaximum(vector<double>(20, 1.0), 0.5), 1.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "bin width", "A bin width of zero: no peak", MStatistics::GetFullWidthAtHalfMaximum(Triangle, 0.0), 0.0, 0.0) && Passed;

  // The same triangle at other fractions of its smoothed apex 95: the ramp 100 - 10|k| crosses f*95 at |k| = (100 - 95 f)/10, the width is 2|k| bins of 0.5
  // f = 0.9: |k| = 1.45, width 1.45; f = 0.2: |k| = 8.1, width 8.1
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "90%", "The width of the triangle at 90% of its maximum", MStatistics::GetFullWidthAtFractionOfMaximum(Triangle, 0.5, 0.9), 1.45, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "20%", "The width of the triangle at 20% of its maximum", MStatistics::GetFullWidthAtFractionOfMaximum(Triangle, 0.5, 0.2), 8.1, 1e-9) && Passed;
  // f = 0.1: the level 9.5 lies between |k| = 9 (10) and |k| = 10, where the smoothing of the base makes 2.5 instead of 0: 9 + 0.5/7.5 on each side
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "10%", "The width of the triangle at 10% of its maximum, with the smoothed base", MStatistics::GetFullWidthAtFractionOfMaximum(Triangle, 0.5, 0.1), 2.0*(9.0 + 0.5/7.5)*0.5, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "half", "The width at the fraction 0.5 is the FWHM", MStatistics::GetFullWidthAtFractionOfMaximum(Triangle, 0.5, 0.5), MStatistics::GetFullWidthAtHalfMaximum(Triangle, 0.5), 0.0) && Passed;
  // f = 0.95: the level 90.25 lies between the apex 95 and its neighbors 90: 0.95 bins on each side; f = 0.05: the level 4.75 between |k| = 9 (10) and the smoothed base 2.5
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "95%", "The width of the triangle at 95% of its maximum, within the apex bins", MStatistics::GetFullWidthAtFractionOfMaximum(Triangle, 0.5, 0.95), 0.95, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "5%", "The width of the triangle at 5% of its maximum, with the smoothed base", MStatistics::GetFullWidthAtFractionOfMaximum(Triangle, 0.5, 0.05), 2.0*(9.0 + 5.25/7.5)*0.5, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "zero", "The fraction zero gives no width", MStatistics::GetFullWidthAtFractionOfMaximum(Triangle, 0.5, 0.0), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "one", "The fraction one gives no width", MStatistics::GetFullWidthAtFractionOfMaximum(Triangle, 0.5, 1.0), 0.0, 0.0) && Passed;

  // The peak of the values: the middle bin of 0.5 is centered on the median 0, so the values k*0.5 sit at the centers of their bins,
  // the smoothed contents around the apex are symmetric (90, 95, 90): the parabola does not move it, the position is 0
  Passed = EvaluateNear("GetPeakPosition()", "triangle", "The peak of the symmetric triangle is its apex", MStatistics::GetPeakPosition(Triangle, 0.5), 0.0, 1e-9) && Passed;
  vector<double> Shifted;
  for (double V: Triangle) {
    Shifted.push_back(V + 1.75);
  }
  Passed = EvaluateNear("GetPeakPosition()", "shifted", "The peak moves with the values", MStatistics::GetPeakPosition(Shifted, 0.5), 1.75, 1e-9) && Passed;
  // Five fewer values at k = 1 (95 instead of 100 - 10): the smoothed contents at k = -1, 0, 1 are 90, 96.25, 92.5, the median stays 0,
  // the vertex of the parabola is at (90 - 92.5)/(2*(90 - 192.5 + 92.5)) = 0.125 bins: 0.125*0.5
  vector<double> Leaning;
  for (int k = -10; k <= 10; ++k) {
    int Count = 100 - 10*abs(k);
    if (k == 1) {
      Count = 95;
    }
    for (int n = 0; n < Count; ++n) {
      Leaning.push_back(0.5*k);
    }
  }
  Passed = EvaluateNear("GetPeakPosition()", "leaning", "The parabola through the apex and its neighbors moves the peak towards the heavier side", MStatistics::GetPeakPosition(Leaning, 0.5), 0.0625, 1e-9) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "few", "Fewer than 20 values: no peak", MStatistics::GetPeakPosition(vector<double>(19, 1.0), 0.5), 0.0, 0.0) && Passed;

  // Around the peak 0 of the triangle, the distances |k*0.5| sorted have 100 zeros, 180 at 0.5, 160 at 1.0, 140 at 1.5, 120 at 2.0 (700 so far), 100 at 2.5
  // (800), 80 at 3.0 (880), 60 at 3.5 (940): the 68% quantile is at the position 679.32 (2.0), the 90% at 899.1 (3.5); 580 of 1000 are closer than 2.0,
  // 700 closer than 2.5. The shifted triangle has its peak at 1.75 and the same numbers about it (about zero they would differ)
  Passed = EvaluateNear("GetContainmentAroundPeak()", "68%", "The 68% containment of the triangle about its peak", MStatistics::GetContainmentAroundPeak(Triangle, 0.68), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetContainmentAroundPeak()", "90%", "The 90% containment of the triangle about its peak", MStatistics::GetContainmentAroundPeak(Triangle, 0.90), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetContainmentAroundPeak()", "shifted", "The containment is about the peak, not about zero", MStatistics::GetContainmentAroundPeak(Shifted, 0.68), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetContainmentAroundPeak()", "empty", "No values: zero", MStatistics::GetContainmentAroundPeak(vector<double>(), 0.68), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetFractionAroundPeak()", "2.0", "The fraction of the triangle closer than 2.0 to its peak (the values at 2.0 are not closer)", MStatistics::GetFractionAroundPeak(Triangle, 2.0), 0.58, 1e-12) && Passed;
  Passed = EvaluateNear("GetFractionAroundPeak()", "2.5", "The fraction of the triangle closer than 2.5 to its peak", MStatistics::GetFractionAroundPeak(Triangle, 2.5), 0.70, 1e-12) && Passed;
  Passed = EvaluateNear("GetFractionAroundPeak()", "shifted", "The fraction is about the peak, not about zero", MStatistics::GetFractionAroundPeak(Shifted, 2.0), 0.58, 1e-12) && Passed;
  Passed = EvaluateNear("GetFractionAroundPeak()", "empty", "No values: zero", MStatistics::GetFractionAroundPeak(vector<double>(), 2.0), 0.0, 0.0) && Passed;

  // Unbinned: the half-sample mode. The 100 values at 0 of the triangle are the middle of the equally short halves at every step: exactly 0 (and 1.75 shifted),
  // the results of the three triangles are confirmed with an independent implementation of the half-sample mode
  Passed = EvaluateNear("GetPeakPosition()", "unbinned triangle", "The half-sample mode of the triangle is its apex", MStatistics::GetPeakPosition(Triangle), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "unbinned shifted", "The half-sample mode moves with the values", MStatistics::GetPeakPosition(Shifted), 1.75, 1e-12) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "unbinned leaning", "Five fewer values at 0.5 do not move the mode away from the 100 values at 0", MStatistics::GetPeakPosition(Leaning), 0.0, 0.0) && Passed;
  // 0, 1, 2, 2.5, 3, 10, 20: the shortest 4 of 7 are 1 ... 3, the halves 2, 2.5 and 2.5, 3 are equally short (0.5): the second (middle) of the two, 2.75
  Passed = EvaluateNear("GetPeakPosition()", "unbinned halves", "The shortest halves of 0, 1, 2, 2.5, 3, 10, 20 end at 2.5 and 3", MStatistics::GetPeakPosition(vector<double>{20.0, 2.5, 0.0, 10.0, 3.0, 1.0, 2.0}), 2.75, 1e-12) && Passed;
  // Three values: the closer pair; 1, 2, 4 gives 1.5, 1, 3, 4 gives 3.5, 1, 2, 3 the middle 2
  Passed = EvaluateNear("GetPeakPosition()", "unbinned three left", "Of three values the closer pair on the left", MStatistics::GetPeakPosition(vector<double>{4.0, 1.0, 2.0}), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "unbinned three right", "Of three values the closer pair on the right", MStatistics::GetPeakPosition(vector<double>{1.0, 4.0, 3.0}), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "unbinned three equal", "Of three equally spaced values the middle one", MStatistics::GetPeakPosition(vector<double>{3.0, 1.0, 2.0}), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "unbinned two", "Two values: their mean", MStatistics::GetPeakPosition(vector<double>{1.0, 4.0}), 2.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "unbinned single", "A single value is the mode", MStatistics::GetPeakPosition(vector<double>{7.0}), 7.0, 0.0) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "unbinned empty", "No values: zero", MStatistics::GetPeakPosition(vector<double>()), 0.0, 0.0) && Passed;
  // An outlier far away does not move the mode of 1 ... 5: the halves 1 ... 3, 2 ... 4, 3 ... 5 are equally short, the middle one 2, 3, 4 is equally spaced: 3
  Passed = EvaluateNear("GetPeakPosition()", "unbinned outlier", "An outlier does not move the mode: the middle of the equally short halves of 1 ... 5, 1000 is 2, 3, 4", MStatistics::GetPeakPosition(vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 1000.0}), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "bin width", "A bin width of zero: no peak", MStatistics::GetPeakPosition(Triangle, 0.0), 0.0, 0.0) && Passed;

  // Use a triangle at x = 5 with the base from 2 to 8 and bin width 0.1 - highest bin centers 4.95 and 5.05 at 9.8333, half maximum 4.91667
  // Expected: interpolation between bin centers 3.45 (4.8333) and 3.55 (5.1667) crosses at 3.475, symmetrically at 6.525, width 3.05
  TH1D Fine("UTTriangle", "UTTriangle", 100, 0.0, 10.0);
  Fine.SetDirectory(nullptr);
  for (int b = 1; b <= 100; ++b) {
    const double CentroidX = Fine.GetXaxis()->GetBinCenter(b);
    const double Height = 10.0*(1.0 - fabs(CentroidX - 5.0)/3.0);
    if (Height > 0.0) {
      Fine.SetBinContent(b, Height);
    }
  }
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "fine histogram", "The width of a triangle at half its maximum of the bin centers", MStatistics::GetFullWidthAtHalfMaximum(&Fine), 3.05, 1e-9) && Passed;
  // At the fraction f of the highest bin 9.8333 the ramp 10*(1 - |x - 5|/3) is crossed at |x - 5| = 3*(1 - 0.98333 f): f = 0.9: 0.345, width 0.69; f = 0.2: 2.41, width 4.82
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "fine 90%", "The width of the histogram at 90% of its maximum", MStatistics::GetFullWidthAtFractionOfMaximum(&Fine, 0.9), 0.69, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "fine 20%", "The width of the histogram at 20% of its maximum", MStatistics::GetFullWidthAtFractionOfMaximum(&Fine, 0.2), 4.82, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "fine half", "The width at the fraction 0.5 is the FWHM", MStatistics::GetFullWidthAtFractionOfMaximum(&Fine, 0.5), MStatistics::GetFullWidthAtHalfMaximum(&Fine), 0.0) && Passed;
  // f = 0.95: |x - 5| = 3*(1 - 0.98333*0.95) = 0.1975, width 0.395; f = 0.05: 2.8525, width 5.705
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "fine 95%", "The width of the histogram at 95% of its maximum", MStatistics::GetFullWidthAtFractionOfMaximum(&Fine, 0.95), 0.395, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "fine 5%", "The width of the histogram at 5% of its maximum", MStatistics::GetFullWidthAtFractionOfMaximum(&Fine, 0.05), 5.705, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "fine zero", "The fraction zero gives no width", MStatistics::GetFullWidthAtFractionOfMaximum(&Fine, 0.0), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "fine one", "The fraction one gives no width", MStatistics::GetFullWidthAtFractionOfMaximum(&Fine, 1.0), 0.0, 0.0) && Passed;
  // The fine triangle has two highest bins (4.95 and 5.05, both 9.8333): ROOT takes the first, the parabola through 9.5, 9.8333, 9.8333 has its
  // vertex at (9.5 - 9.8333)/(2*(9.5 - 19.6667 + 9.8333)) = 0.5 bins: 4.95 + 0.05 = 5
  Passed = EvaluateNear("GetPeakPosition()", "fine histogram", "The peak of the fine triangle is at its apex between the two highest bins", MStatistics::GetPeakPosition(&Fine), 5.0, 1e-9) && Passed;

  // A histogram of 11 bins of width 1 with the contents 0, 2, 4, 6, 8, 10, 8, 6, 4, 2, 0: the half maximum 5 is crossed at 3.0 and 8.0
  TH1D Peak("FwhmPeak", "FwhmPeak", 11, 0.0, 11.0);
  Peak.SetDirectory(nullptr);
  const vector<double> PeakContents = { 0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 8.0, 6.0, 4.0, 2.0, 0.0 };
  for (unsigned int b = 0; b < PeakContents.size(); ++b) {
    Peak.SetBinContent(b + 1, PeakContents[b]);
  }
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "histogram", "The FWHM of a triangular histogram from the interpolated crossings", MStatistics::GetFullWidthAtHalfMaximum(&Peak), 5.0, 1e-9) && Passed;
  // Without a crossing the width is not known: a flat histogram never falls below the half maximum, a peak in the first bin has no left crossing
  TH1D Flat("FwhmFlat", "FwhmFlat", 10, 0.0, 10.0);
  Flat.SetDirectory(nullptr);
  for (int b = 1; b <= 10; ++b) {
    Flat.SetBinContent(b, 5.0);
  }
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "flat", "A flat histogram has no crossings: the width is zero, not the width of the histogram", MStatistics::GetFullWidthAtHalfMaximum(&Flat), 0.0, 0.0) && Passed;
  TH1D Edge("FwhmEdge", "FwhmEdge", 10, 0.0, 10.0);
  Edge.SetDirectory(nullptr);
  for (int b = 1; b <= 10; ++b) {
    Edge.SetBinContent(b, 10.0 - b);
  }
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "edge", "A peak at the edge of the histogram has no left crossing: the width is zero", MStatistics::GetFullWidthAtHalfMaximum(&Edge), 0.0, 0.0) && Passed;
  TH1D Empty("FwhmEmpty", "FwhmEmpty", 10, 0.0, 10.0);
  Empty.SetDirectory(nullptr);
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "empty histogram", "An empty histogram has no peak: the width is zero", MStatistics::GetFullWidthAtHalfMaximum(&Empty), 0.0, 0.0) && Passed;

  // The peak of a histogram: symmetric neighbors keep the center of the highest bin (5.5), a heavier left neighbor moves it:
  // contents 6, 10, 8 around the bin 5 to 6 give the vertex (6 - 8)/(2*(6 - 20 + 8)) = 1/6 bins: 5.5 + 1/6
  Passed = EvaluateNear("GetPeakPosition()", "histogram", "The peak of the symmetric histogram is the center of its highest bin", MStatistics::GetPeakPosition(&Peak), 5.5, 1e-9) && Passed;
  TH1D Lean("PeakLean", "PeakLean", 11, 0.0, 11.0);
  Lean.SetDirectory(nullptr);
  Lean.SetBinContent(5, 6.0);
  Lean.SetBinContent(6, 10.0);
  Lean.SetBinContent(7, 8.0);
  Passed = EvaluateNear("GetPeakPosition()", "leaning histogram", "The parabola moves the peak towards the heavier neighbor", MStatistics::GetPeakPosition(&Lean), 5.5 + 1.0/6.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "edge", "A peak in the first bin has no parabola: the center of the bin", MStatistics::GetPeakPosition(&Edge), 0.5, 1e-9) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "empty", "An empty histogram has no peak", MStatistics::GetPeakPosition(&Empty), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "flat", "A flat histogram has no parabola: the center of the first highest bin", MStatistics::GetPeakPosition(&Flat), 0.5, 1e-9) && Passed;


  // A bin content exactly at the half maximum is the crossing at that bin center: 5, 10, 5 in bins of width 1 has the width 2
  TH1D LevelAtBoundary("LevelAtBoundary", "", 3, 0.0, 3.0);
  LevelAtBoundary.SetBinContent(1, 5.0);
  LevelAtBoundary.SetBinContent(2, 10.0);
  LevelAtBoundary.SetBinContent(3, 5.0);
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "level at the boundary bins", "Bin contents exactly at the half maximum in the first and last bin are the crossings", MStatistics::GetFullWidthAtHalfMaximum(&LevelAtBoundary), 2.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::TestBootstrap()
{
  bool Passed = true;

  // Alternating 0 and 1 (400 values): standard deviation 0.5, the standard error of the mean is 0.5/sqrt(400) = 0.025,
  // the standard deviation of the 1000 resamples has an uncertainty of 0.025/sqrt(2*999) = 0.00056
  vector<double> Alternating;
  for (int i = 0; i < 200; ++i) {
    Alternating.push_back(0.0);
    Alternating.push_back(1.0);
  }
  const MStatisticFunction Mean = [](const vector<double>& V) { return MStatistics::GetMean(V); };
  Passed = EvaluateNear("GetBootstrapError()", "mean", "The bootstrap error of a mean is the standard error of the mean (five sigma of the estimate)", MStatistics::GetBootstrapError(Alternating, Mean, 1000), 0.025, 0.0028) && Passed;
  Passed = EvaluateNear("GetBootstrapError()", "repeat", "The same data give the same uncertainty", MStatistics::GetBootstrapError(Alternating, Mean, 50) - MStatistics::GetBootstrapError(Alternating, Mean, 50), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetBootstrapError()", "constant", "A constant has no uncertainty", MStatistics::GetBootstrapError(vector<double>(50, 3.0), Mean, 50), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetBootstrapError()", "single", "A single value has no uncertainty", MStatistics::GetBootstrapError(vector<double>{1.0}, Mean, 50), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetBootstrapError()", "empty", "No values: no uncertainty", MStatistics::GetBootstrapError(vector<double>(), Mean, 50), 0.0, 0.0) && Passed;
  const double OneResample = MStatistics::GetBootstrapError(Alternating, Mean, 1);
  Passed = EvaluateTrue("GetBootstrapError()", "one resample", "One resample is raised to two: the uncertainty is finite", isfinite(OneResample) == true) && Passed;
  Passed = EvaluateTrue("GetBootstrapError()", "zero resamples", "Zero resamples are raised to two: the uncertainty is finite", isfinite(MStatistics::GetBootstrapError(Alternating, Mean, 0)) == true) && Passed;

  // The statistic decides what is resampled: the bootstrap error of the median of 1 ... 5 differs from the one of the mean
  const vector<double> Five = { 4.0, 1.0, 5.0, 2.0, 3.0 };
  const MStatisticFunction Median = [](const vector<double>& V) { return MStatistics::GetMedian(V); };
  Passed = EvaluateTrue("GetBootstrapError()", "statistic", "The uncertainty depends on the statistic", MStatistics::GetBootstrapError(Five, Median, 200) != MStatistics::GetBootstrapError(Five, Mean, 200)) && Passed;


  // A statistic which always gives the same large value has exactly no uncertainty
  Passed = EvaluateNear("GetBootstrapError()", "constant statistic", "A constant statistic has exactly no bootstrap uncertainty", MStatistics::GetBootstrapError(vector<double>{1.0, 2.0, 3.0}, [](const vector<double>&) { return 1e15 + 0.1; }, 10), 0.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::TestSignificances()
{
  bool Passed = true;

  // 100 and 50 with the expected ratio 2: no deviation; with the expected ratio 1: |2 - 1|/(2*sqrt(1/100 + 1/50)) = 1/(2*sqrt(0.03))
  Passed = EvaluateNear("GetRatioSigma()", "expected", "A ratio which is the expected one has no significance", MStatistics::GetRatioSigma(100.0, 50.0, 2.0), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetRatioSigma()", "one", "The significance of the ratio 2 against the expected 1", MStatistics::GetRatioSigma(100.0, 50.0, 1.0), 1.0/(2.0*sqrt(0.03)), 1e-12) && Passed;
  Passed = EvaluateNear("GetRatioSigma()", "symmetric", "The significance does not depend on the sign of the deviation", MStatistics::GetRatioSigma(100.0, 50.0, 3.0), 1.0/(2.0*sqrt(0.03)), 1e-12) && Passed;
  // Without counts the ratio is undefined: the significance is huge
  Passed = EvaluateTrue("GetRatioSigma()", "no A", "Without counts in A the significance is huge", MStatistics::GetRatioSigma(0.0, 50.0, 1.0) >= 1e9) && Passed;
  Passed = EvaluateTrue("GetRatioSigma()", "no B", "Without counts in B the significance is huge", MStatistics::GetRatioSigma(100.0, 0.0, 1.0) >= 1e9) && Passed;

  // 30 of 100 against 20 of 100: pooled proportion 0.25, |0.3 - 0.2|/sqrt(0.25*0.75*(1/100 + 1/100))
  Passed = EvaluateNear("GetProportionSigma()", "different", "The z value of 30% against 20% of 100 each", MStatistics::GetProportionSigma(30.0, 100.0, 20.0, 100.0), 0.1/sqrt(0.25*0.75*0.02), 1e-12) && Passed;
  Passed = EvaluateNear("GetProportionSigma()", "symmetric", "The z value does not depend on the order", MStatistics::GetProportionSigma(20.0, 100.0, 30.0, 100.0), 0.1/sqrt(0.25*0.75*0.02), 1e-12) && Passed;
  Passed = EvaluateNear("GetProportionSigma()", "equal", "Equal proportions have no significance", MStatistics::GetProportionSigma(15.0, 50.0, 30.0, 100.0), 0.0, 0.0) && Passed;
  // 30 of 100 against 20 of 400: pooled proportion 0.1, |0.3 - 0.05|/sqrt(0.1*0.9*(1/100 + 1/400))
  Passed = EvaluateNear("GetProportionSigma()", "sizes", "The z value with different sample sizes", MStatistics::GetProportionSigma(30.0, 100.0, 20.0, 400.0), 0.25/sqrt(0.09*0.0125), 1e-12) && Passed;


  // Successes outside [0, trials] are invalid for each sample on its own, also if the pooled rate looks fine
  Passed = EvaluateNear("GetProportionSigma()", "more successes than trials, first", "20 of 10 is invalid", MStatistics::GetProportionSigma(20, 10, 0, 100), 1e9, 0.0) && Passed;
  Passed = EvaluateNear("GetProportionSigma()", "negative successes, first", "-1 of 10 is invalid", MStatistics::GetProportionSigma(-1, 10, 20, 100), 1e9, 0.0) && Passed;
  Passed = EvaluateNear("GetProportionSigma()", "more successes than trials, both", "20 of 10 and 40 of 20 are invalid, not equal rates", MStatistics::GetProportionSigma(20, 10, 40, 20), 1e9, 0.0) && Passed;
  Passed = EvaluateNear("GetProportionSigma()", "more successes than trials, second", "20 of 10 is invalid as second sample too", MStatistics::GetProportionSigma(0, 100, 20, 10), 1e9, 0.0) && Passed;
  // A pooled rate which rounds to one is still defined if there is a failure: 1e16 of 1e16 against 0 of 1 has p(1 - p) = 1e-16, the z score is 1e8
  Passed = EvaluateNear("GetProportionSigma()", "pooled rate close to one", "A single failure among 1e16 + 1 trials keeps the pooled error defined: 1e8 sigma", MStatistics::GetProportionSigma(1e16, 1e16, 0, 1), 1e8, 1e-3) && Passed;
  // Two efficiencies close to one: their difference comes from the failure rates (2/1e16 against 2/1.2e16), the successes would cancel to zero
  Passed = EvaluateNear("GetProportionSigma()", "efficiencies close to one", "Two failures among 1e16 and among 1.2e16 trials differ by 0.1826 sigma, not zero", MStatistics::GetProportionSigma(1e16 - 2, 1e16, 1.2e16 - 2, 1.2e16), 0.182574185835055, 1e-9) && Passed;
  Passed = EvaluateNear("GetProportionSigma()", "relabeled", "Relabeling successes as failures gives the same score", MStatistics::GetProportionSigma(2, 1e16, 2, 1.2e16), MStatistics::GetProportionSigma(1e16 - 2, 1e16, 1.2e16 - 2, 1.2e16), 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::TestGuards()
{
  bool Passed = true;

  // Fractions outside 0 to 1: the quantile is clamped, the containment needs all or one value
  const vector<double> Five = { 4.0, 1.0, 5.0, 2.0, 3.0 };
  const double NaN = numeric_limits<double>::quiet_NaN();
  Passed = EvaluateNear("GetQuantile()", "above one", "A fraction above one is the maximum", MStatistics::GetQuantile(Five, 1.5), 5.0, 0.0) && Passed;
  Passed = EvaluateNear("GetQuantile()", "below zero", "A fraction below zero is the minimum", MStatistics::GetQuantile(Five, -0.5), 1.0, 0.0) && Passed;
  Passed = EvaluateNear("GetQuantile()", "not a number", "A fraction which is not a number gives zero", MStatistics::GetQuantile(Five, NaN), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetHalfWidthContaining()", "above one", "A fraction above one needs all values: half the range", MStatistics::GetHalfWidthContaining(Five, 1.5), 2.0, 0.0) && Passed;
  Passed = EvaluateNear("GetHalfWidthContaining()", "below zero", "A fraction below zero needs one value: zero", MStatistics::GetHalfWidthContaining(Five, -0.5), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetHalfWidthContaining()", "not a number", "A fraction which is not a number gives zero", MStatistics::GetHalfWidthContaining(Five, NaN), 0.0, 0.0) && Passed;

  // Values far outside the window of the binned peak and infinite values are left out, the peak of the others is found
  vector<double> Triangle;
  for (int k = -10; k <= 10; ++k) {
    for (int n = 0; n < 100 - 10*abs(k); ++n) {
      Triangle.push_back(0.5*k);
    }
  }
  vector<double> Polluted = Triangle;
  Polluted.push_back(1e300);
  Polluted.push_back(-1e300);
  Polluted.push_back(numeric_limits<double>::infinity());
  Polluted.push_back(-numeric_limits<double>::infinity());
  Passed = EvaluateNear("GetPeakPosition()", "far values", "Values far outside the window do not change the binned peak", MStatistics::GetPeakPosition(Polluted, 0.5), 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "far values", "Values far outside the window do not change the width", MStatistics::GetFullWidthAtHalfMaximum(Polluted, 0.5), 5.25, 1e-9) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "far values unbinned", "Values far away do not change the half-sample mode", MStatistics::GetPeakPosition(Polluted), 0.0, 0.0) && Passed;

  // A histogram with variable bin widths (edges 0, 1, 2, 4, 8 with the contents 0, 10, 6, 2): the half maximum 5 is crossed at 1.0 (halfway between the
  // centers 0.5 and 1.5 of the bins with 0 and 10) and at 3.75 (three quarters of the way from the center 3 of the bin with 6 to the center 6 of the bin with 2)
  const vector<double> Edges = { 0.0, 1.0, 2.0, 4.0, 8.0 };
  TH1D Variable("Variable", "Variable", 4, Edges.data());
  Variable.SetDirectory(nullptr);
  Variable.SetBinContent(2, 10.0);
  Variable.SetBinContent(3, 6.0);
  Variable.SetBinContent(4, 2.0);
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "variable bins", "The crossings are interpolated between the bin centers, also for variable bin widths", MStatistics::GetFullWidthAtHalfMaximum(&Variable), 2.75, 1e-9) && Passed;

  // Proportions which are undefined or equal
  Passed = EvaluateNear("GetProportionSigma()", "none", "No successes on both sides: equal proportions, no significance", MStatistics::GetProportionSigma(0.0, 100.0, 0.0, 50.0), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetProportionSigma()", "all", "All successes on both sides: equal proportions, no significance", MStatistics::GetProportionSigma(100.0, 100.0, 50.0, 50.0), 0.0, 0.0) && Passed;
  Passed = EvaluateTrue("GetProportionSigma()", "no trials", "Without trials the significance is huge", MStatistics::GetProportionSigma(1.0, 0.0, 1.0, 10.0) >= 1e9) && Passed;
  Passed = EvaluateTrue("GetProportionSigma()", "more successes than trials", "More successes than trials are undefined: huge", MStatistics::GetProportionSigma(20.0, 10.0, 30.0, 10.0) >= 1e9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTStatistics::TestNonGaussian()
{
  bool Passed = true;

  // A Cauchy (Lorentzian) distribution with the scale 1: x = tan(pi (u - 1/2)) at u = (i + 1/2)/n for n = 1025, a quantile grid without sampling noise.
  // The mean and the standard deviation are meaningless (the extreme values are +-652), the order statistics are not: the middle value is 0 exactly,
  // the median absolute deviation is the value of |x| in the middle of the 1024 others, which are pairs tan(pi j/n): the pair j = 256
  const unsigned int NCauchy = 1025;
  vector<double> Cauchy;
  for (unsigned int i = 0; i < NCauchy; ++i) {
    Cauchy.push_back(tan(c_Pi*((i + 0.5)/NCauchy - 0.5)));
  }
  Passed = EvaluateNear("GetMedian()", "Cauchy", "The median of the Cauchy grid is its middle value 0", MStatistics::GetMedian(Cauchy), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetRobustSigma()", "Cauchy", "The robust sigma of the Cauchy grid is 1.4826 times the median absolute deviation tan(256 pi/1025)", MStatistics::GetRobustSigma(Cauchy), 1.4826*tan(256.0*c_Pi/NCauchy), 1e-9) && Passed;
  Passed = EvaluateTrue("GetStandardDeviation()", "Cauchy", "The standard deviation of the Cauchy grid is dominated by its extreme values: far above the robust sigma", MStatistics::GetStandardDeviation(Cauchy) > 10.0) && Passed;
  // The half-sample mode: every halving of 1025 = 2^10 + 1 symmetric values has a unique central shortest half, the last three are equally spaced: 0 exactly
  Passed = EvaluateNear("GetPeakPosition()", "Cauchy unbinned", "The half-sample mode of the symmetric Cauchy grid is its center", MStatistics::GetPeakPosition(Cauchy), 0.0, 1e-12) && Passed;
  // Binned in 0.1: the middle bin is centered on the median 0 and the symmetric values fill the bins symmetrically, the parabola stays at 0;
  // the analytic FWHM of the Lorentzian is 2 (half maximum at +-1), the binning and the smoothing resolve it to about a bin width
  Passed = EvaluateNear("GetPeakPosition()", "Cauchy binned", "The binned peak of the symmetric Cauchy grid is exactly at its middle bin", MStatistics::GetPeakPosition(Cauchy, 0.1), 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "Cauchy", "The FWHM of the Lorentzian is 2 within a bin width", MStatistics::GetFullWidthAtHalfMaximum(Cauchy, 0.1), 2.0, 0.1) && Passed;
  // The 68% containment of the Cauchy: the shortest interval with 0.68*1025 = 697 values is central: half of x_(i+696) - x_(i) with i = 164, by the grid
  const unsigned int Needed68 = static_cast<unsigned int>(ceil(0.68*NCauchy));
  const unsigned int Start68 = (NCauchy - Needed68)/2;
  Passed = EvaluateNear("GetHalfWidthContaining()", "Cauchy 68%", "The 68% containment of the Cauchy grid is the central window of 697 values", MStatistics::GetHalfWidthContaining(Cauchy, 0.68), 0.5*(tan(c_Pi*((Start68 + Needed68 - 1 + 0.5)/NCauchy - 0.5)) - tan(c_Pi*((Start68 + 0.5)/NCauchy - 0.5))), 1e-9) && Passed;

  // An exponential distribution: x = -ln(1 - u) at u = (i + 1/2)/n for n = 1001, the mode is at the edge 0, the median is the middle value ln 2 exactly
  const unsigned int NExponential = 1001;
  vector<double> Exponential;
  for (unsigned int i = 0; i < NExponential; ++i) {
    Exponential.push_back(-log(1.0 - (i + 0.5)/NExponential));
  }
  Passed = EvaluateNear("GetMedian()", "exponential", "The median of the exponential grid is ln 2", MStatistics::GetMedian(Exponential), log(2.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetQuantile()", "exponential quartile", "The first quartile is the value at the position 250 of the grid", MStatistics::GetQuantile(Exponential, 0.25), -log(1.0 - 250.5/NExponential), 1e-12) && Passed;
  // The shortest half: the 501 smallest values, from the first one
  Passed = EvaluateNear("GetHalfWidthContaining()", "exponential 50%", "The shortest half of the exponential starts at its smallest value", MStatistics::GetHalfWidthContaining(Exponential, 0.5), 0.5*(Exponential[500] - Exponential[0]), 1e-12) && Passed;
  // The standard error of the median is 1/(2 f sqrt(n)) with the density f = 1/2 at ln 2: 1/sqrt(n). The density from the sqrt(n) = 31 values on each
  // side is the average over u = 0.469 ... 0.531, which is 1/2 to a part in a thousand; the bootstrap with 1000 resamples estimates it to about 10%
  const double MedianErrorExpected = 1.0/sqrt(static_cast<double>(NExponential));
  Passed = EvaluateNear("GetMedianError()", "exponential", "The error of the median of the exponential is 1/sqrt(n) within one percent", MStatistics::GetMedianError(Exponential), MedianErrorExpected, 0.01*MedianErrorExpected) && Passed;
  const MStatisticFunction Median = [](const vector<double>& V) { return MStatistics::GetMedian(V); };
  Passed = EvaluateNear("GetBootstrapError()", "exponential median", "The bootstrap error of the median of the exponential is 1/sqrt(n) within 15 percent", MStatistics::GetBootstrapError(Exponential, Median, 1000), MedianErrorExpected, 0.15*MedianErrorExpected) && Passed;
  // The peak is at the edge: the half-sample mode nests into the smallest values (the first ones are 0.0005, 0.0015, ...). The binned peak with bins of
  // 0.05 around the median ln 2 has the edge inside a bin: this first bin is only partly filled, the smoothing moves the maximum to the second full
  // bin and the parabola further in: a peak at the edge of the data is found within three bins
  Passed = EvaluateNear("GetPeakPosition()", "exponential unbinned", "The half-sample mode of the exponential is at its edge within a few grid steps", MStatistics::GetPeakPosition(Exponential), 0.0, 0.005) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "exponential binned", "The binned peak of the exponential is at its edge within three bins", MStatistics::GetPeakPosition(Exponential, 0.05), 0.0, 0.15) && Passed;

  // A skewed triangular density on 0 ... 3 with the mode at 1 (rising from 0 to 1, falling from 1 to 3): x = sqrt(3u) for u <= 1/3, else 3 - sqrt(6 (1 - u)),
  // on the grid u = (i + 1/2)/n with n = 2049: the median is the middle value 3 - sqrt(3), the quantile grid nests the halves onto the kink at 1
  const unsigned int NSkewed = 2049;
  vector<double> Skewed;
  for (unsigned int i = 0; i < NSkewed; ++i) {
    const double U = (i + 0.5)/NSkewed;
    if (U <= 1.0/3.0) {
      Skewed.push_back(sqrt(3.0*U));
    } else {
      Skewed.push_back(3.0 - sqrt(6.0*(1.0 - U)));
    }
  }
  Passed = EvaluateNear("GetMedian()", "skewed", "The median of the skewed triangle is 3 - sqrt(3)", MStatistics::GetMedian(Skewed), 3.0 - sqrt(3.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "skewed unbinned", "The half-sample mode of the skewed triangle is its kink at 1", MStatistics::GetPeakPosition(Skewed), 1.0, 1e-3) && Passed;
  // Binned in 0.05, the smoothing and the parabola straddle the kink and the flatter side (slope -1/2 against +1) pulls the vertex to the bin edge
  // above it: within a bin, where the unbinned mode is exact
  Passed = EvaluateNear("GetPeakPosition()", "skewed binned", "The binned peak of the skewed triangle is its kink within a bin of 0.05", MStatistics::GetPeakPosition(Skewed, 0.05), 1.0, 0.05) && Passed;
  // The half maximum of the density is at 0.5 on the rising and at 2 on the falling side: FWHM 1.5. The smoothing lowers the kinked apex by about
  // 2 percent, which widens it to about 1.53: expected within a bin width
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "skewed", "The FWHM of the skewed triangle is 1.5 within a bin width (the smoothing of the kink)", MStatistics::GetFullWidthAtHalfMaximum(Skewed, 0.05), 1.5, 0.05) && Passed;
  Passed = EvaluateTrue("GetMean()", "skewed", "The mean of the skewed triangle (4/3) is above its mode", MStatistics::GetMean(Skewed) > 1.3) && Passed;

  // Two peaks: a triangle of height 100 at -5 and one of height 60 at +5 (steps of 0.5), 1000 and 360 values: the higher one is the peak, its widths
  // are not changed by the second one (the valley between them is empty), the median -4 lies in the first one (the 680th and 681st values)
  vector<double> Bimodal;
  for (int k = -10; k <= 10; ++k) {
    for (int n = 0; n < 100 - 10*abs(k); ++n) {
      Bimodal.push_back(-5.0 + 0.5*k);
    }
  }
  for (int k = -5; k <= 5; ++k) {
    for (int n = 0; n < 60 - 10*abs(k); ++n) {
      Bimodal.push_back(5.0 + 0.5*k);
    }
  }
  Passed = EvaluateNear("GetMedian()", "bimodal", "The median of the two peaks is in the higher one", MStatistics::GetMedian(Bimodal), -4.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "bimodal unbinned", "The half-sample mode is the higher peak", MStatistics::GetPeakPosition(Bimodal), -5.0, 0.0) && Passed;
  // The middle bin is centered on the median -4, so the values sit at the centers of their bins: the apex of the higher peak exactly
  Passed = EvaluateNear("GetPeakPosition()", "bimodal binned", "The binned peak is the apex of the higher peak", MStatistics::GetPeakPosition(Bimodal, 0.5), -5.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "bimodal", "The FWHM is the one of the higher peak alone", MStatistics::GetFullWidthAtHalfMaximum(Bimodal, 0.5), 5.25, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtFractionOfMaximum()", "bimodal 20%", "The width at 20% is the one of the higher peak alone, the crossing is before the valley", MStatistics::GetFullWidthAtFractionOfMaximum(Bimodal, 0.5, 0.2), 8.1, 1e-9) && Passed;
  // The shortest window with 680 values is k = -4 ... 4 of the higher peak (700 values): 4.0 wide
  Passed = EvaluateNear("GetHalfWidthContaining()", "bimodal 50%", "The shortest half is inside the higher peak", MStatistics::GetHalfWidthContaining(Bimodal, 0.5), 2.0, 1e-12) && Passed;

  // Ten outliers at a million among 1 ... 100: the median moves to 55.5, the deviations |k - 55.5| are pairs 0.5, 0.5, 1.5, 1.5, ... for k = 11 ... 100, the
  // 55th and 56th of 110 are the pair 27.5: robust sigma 1.4826*27.5. The half-sample mode: the halves of 55, 28, 14, 7, 4 consecutive integers are
  // all equally short, the middle ones are 24 ... 78, 38 ... 65, 45 ... 58, 49 ... 55, 51 ... 54, and of these 52 and 53
  vector<double> Outliers;
  for (unsigned int k = 1; k <= 100; ++k) {
    Outliers.push_back(k);
  }
  for (unsigned int o = 0; o < 10; ++o) {
    Outliers.push_back(1e6);
  }
  Passed = EvaluateNear("GetMedian()", "outliers", "Ten outliers among a hundred move the median by five", MStatistics::GetMedian(Outliers), 55.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetRobustSigma()", "outliers", "The robust sigma ignores the outliers: 1.4826 times the median absolute deviation 27.5", MStatistics::GetRobustSigma(Outliers), 1.4826*27.5, 1e-9) && Passed;
  Passed = EvaluateTrue("GetStandardDeviation()", "outliers", "The standard deviation is ruined by the outliers: far above the robust sigma", MStatistics::GetStandardDeviation(Outliers) > 1e5) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "outliers unbinned", "The half-sample mode of 1 ... 100 with outliers is the middle of the equally short halves", MStatistics::GetPeakPosition(Outliers), 52.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetHalfWidthContaining()", "outliers 50%", "The shortest 55 of 110 are 55 consecutive integers", MStatistics::GetHalfWidthContaining(Outliers, 0.5), 27.0, 1e-12) && Passed;

  // A flat distribution: 1000 values (i + 1/2)/1000 in bins of 0.05 with the middle bin centered on the median 1/2: 25 in the first and the last bin
  // (0 ... 0.025 and 0.975 ... 1), 50 in the 19 between. The smoothing gives 25 at the two end bins and 6.25 outside: the half maximum 25 is crossed
  // at the far edges of the end bins, the width is the whole range 1.0. A flat top has no peak: the parabola through 43.75, 50, 50 puts the binned
  // peak at the far edge of the first full bin, 0.125
  vector<double> Flat;
  for (unsigned int i = 0; i < 1000; ++i) {
    Flat.push_back((i + 0.5)/1000.0);
  }
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "flat", "The FWHM of a flat distribution is its range", MStatistics::GetFullWidthAtHalfMaximum(Flat, 0.05), 1.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "flat binned", "A flat top has no peak: the parabola ends at the edge of the first full bin", MStatistics::GetPeakPosition(Flat, 0.05), 0.125, 1e-9) && Passed;
  // Unbinned, all halves are equally short up to the rounding of the values: the result is somewhere in the range
  const double FlatMode = MStatistics::GetPeakPosition(Flat);
  Passed = EvaluateTrue("GetPeakPosition()", "flat unbinned", "A flat distribution has no mode: the half-sample mode is somewhere in its range", FlatMode > 0.0 && FlatMode < 1.0) && Passed;

  // Six equal peaks: the density 1 + cos(x) on -6 pi ... 6 pi (the inverse of its distribution function (x + sin x + 6 pi)/(12 pi) by Newton's method).
  // Which of the equal maxima is found depends on the rounding of the values: the peak functions only promise a maximum, not a particular one,
  // and never the median 0 by itself. The FWHM of a lobe is pi (1 + cos x = 1 at +-pi/2)
  const unsigned int NCosine = 12000;
  vector<double> Cosine;
  for (unsigned int i = 0; i < NCosine; ++i) {
    const double U = (i + 0.5)/NCosine;
    double X = -6.0*c_Pi + U*12.0*c_Pi;
    for (unsigned int Iteration = 0; Iteration < 60; ++Iteration) {
      X -= ((X + sin(X) + 6.0*c_Pi)/(12.0*c_Pi) - U)/((1.0 + cos(X))/(12.0*c_Pi) + 1e-12);
    }
    Cosine.push_back(X);
  }
  const double UnbinnedCosinePeak = MStatistics::GetPeakPosition(Cosine);
  const double BinnedCosinePeak = MStatistics::GetPeakPosition(Cosine, 0.5);
  Passed = EvaluateNear("GetPeakPosition()", "cosine unbinned", "The half-sample mode of six equal peaks is at one of them (a multiple of 2 pi)", UnbinnedCosinePeak/(2.0*c_Pi) - round(UnbinnedCosinePeak/(2.0*c_Pi)), 0.0, 0.01/(2.0*c_Pi)) && Passed;
  Passed = EvaluateNear("GetPeakPosition()", "cosine binned", "The binned peak of six equal peaks is at one of them (a multiple of 2 pi)", BinnedCosinePeak/(2.0*c_Pi) - round(BinnedCosinePeak/(2.0*c_Pi)), 0.0, 0.01/(2.0*c_Pi)) && Passed;
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "cosine", "The FWHM of a lobe of 1 + cos x is pi within a bin of 0.5", MStatistics::GetFullWidthAtHalfMaximum(Cosine, 0.5), c_Pi, 0.5) && Passed;
  // A histogram of 1 + cos(x) with a bin centered on 0: that maximum is exactly 2 and is the one which is found, the half maximum 1 is at +-pi/2
  TH1D CosineHistogram("Cosine", "Cosine", 1201, -6.0*c_Pi, 6.0*c_Pi);
  CosineHistogram.SetDirectory(nullptr);
  for (int b = 1; b <= 1201; ++b) {
    CosineHistogram.SetBinContent(b, 1.0 + cos(CosineHistogram.GetBinCenter(b)));
  }
  Passed = EvaluateNear("GetPeakPosition()", "cosine histogram", "Of the equal maxima of the cosine histogram the exact one at the bin center 0 is found", MStatistics::GetPeakPosition(&CosineHistogram), 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetFullWidthAtHalfMaximum()", "cosine histogram", "The FWHM of the lobe at 0 is pi within a hundredth of a bin", MStatistics::GetFullWidthAtHalfMaximum(&CosineHistogram), c_Pi, 1e-2*12.0*c_Pi/1201.0) && Passed;

  // Identical values in the middle: the median has no width there, its error is zero
  Passed = EvaluateNear("GetMedianError()", "ties", "The error of a median in a cluster of identical values is zero", MStatistics::GetMedianError(vector<double>{1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 3.0}), 0.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTStatistics Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTStatistics.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
