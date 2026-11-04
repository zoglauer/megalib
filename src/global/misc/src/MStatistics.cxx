/*
 * MStatistics.cxx
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

// Include the header:
#include "MStatistics.h"

// Standard libs:
#include <algorithm>
#include <cmath>
#include <random>
using namespace std;

// ROOT libs:
#include "TAxis.h"
#include "TH1.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


//! Return the arithmetic mean of the sample values, or zero for an empty sample
//! Calculate sum(x)/n with compensated differences from the first value and a corrected division
//! Identical values retain exactly their original value as the mean
double MStatistics::GetMean(const vector<double>& Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  // Accumulate differences from the first sample value
  const double Shift = Values[0];
  double Sum = 0.0;
  double Compensation = 0.0;
  double Residual = 0.0;
  for (double Value: Values) {
    // Recover each subtraction residual with TwoSum
    const double Term = Value - Shift;
    const double Virtual = Term - Value;
    Residual += (Value - (Term - Virtual)) + (-Shift - Virtual);
    // Add the term with Neumaier compensation
    const double NewSum = Sum + Term;
    if (fabs(Sum) >= fabs(Term)) {
      Compensation += (Sum - NewSum) + Term;
    } else {
      Compensation += (Term - NewSum) + Sum;
    }
    Sum = NewSum;
  }
  // Recover the quotient remainder with a fused multiply-add
  const double Count = static_cast<double>(Values.size());
  const double Quotient = Sum/Count;
  const double Remainder = fma(-Quotient, Count, Sum);
  return (Shift + Quotient) + (Remainder + Compensation + Residual)/Count;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the numerical mean integral(x*f(x))/integral(f(x)) over [Min, Max]
//! Require a callable Density on a nonempty range, with finite non-negative values at sampled positions
//! The density need not be normalized; return zero for Max <= Min or a nonpositive estimated mass
//! Supply increasing breakpoints at jumps, kinks, or the boundaries of narrow features
//! Discard entries outside (Min, Max), duplicates, and entries below the last accepted breakpoint
//! Use composite Simpson integration with 20000 intervals if no breakpoint is accepted, otherwise 200 per piece
//! Use log(x) coordinates on pieces whose lower bound is positive, and linear coordinates on other pieces
//! Evaluate piece endpoints just inside the piece to sample the density on the appropriate side of a jump
//! The fixed grids provide no error estimate; features must be resolved by the grid or the supplied breakpoints
//! Propagate exceptions from Density; skip all callback evaluations when Max <= Min
//! Can throw: std::bad_function_call for an empty Density on a nonempty range
double MStatistics::GetMeanOfDensity(const function<double(double)>& Density, double Min, double Max, const vector<double>& Breakpoints)
{
  // Set the even Simpson interval counts
  const unsigned int IntervalsOfSinglePiece = 20000;
  const unsigned int IntervalsPerPiece = 200;
  // Fractional endpoint inset in the integration coordinate
  const double EndPointInset = 1e-9;

  // Reject empty or reversed ranges
  if (Max <= Min) {
    return 0.0;
  }

  // Keep increasing interior breakpoints
  vector<double> Edges = { Min };
  for (double Breakpoint: Breakpoints) {
    if (Breakpoint > Edges.back() && Breakpoint < Max) {
      Edges.push_back(Breakpoint);
    }
  }
  Edges.push_back(Max);

  // Use the dense grid for an unsplit range
  unsigned int Intervals = IntervalsPerPiece;
  if (Edges.size() == 2) {
    Intervals = IntervalsOfSinglePiece;
  }

  // Anchor the first moment at the point nearest zero
  double Origin = 0.0;
  if (Min > 0) {
    Origin = Min;
  } else if (Max < 0) {
    Origin = Max;
  }
  double IntegralOfDensity = 0.0;
  double IntegralOfFirstMoment = 0.0;
  for (unsigned int p = 0; p + 1 < Edges.size(); ++p) {
    const double Low = Edges[p];
    const double High = Edges[p+1];

    // Use logarithmic coordinates on positive pieces
    bool LogarithmicGrid = false;
    if (Low > 0) {
      LogarithmicGrid = true;
    }

    // Resolve narrow logarithmic spans with log1p
    double Span = High - Low;
    bool LogarithmByRelativeWidth = false;
    if (LogarithmicGrid == true) {
      const double RelativeWidth = (High - Low)/Low;
      if (std::isfinite(RelativeWidth) == true) {
        Span = log1p(RelativeWidth);
        LogarithmByRelativeWidth = true;
      } else {
        // Avoid an overflowing ratio of bounds
        Span = log(High) - log(Low);
      }
    }
    const double Step = Span/Intervals;

    for (unsigned int i = 0; i <= Intervals; ++i) {
      // Apply Simpson endpoint and alternating interior weights
      double Weight = 2.0;
      if (i == 0 || i == Intervals) {
        Weight = 1.0;
      } else if (i % 2 == 1) {
        Weight = 4.0;
      }
      // Move boundary samples inside this piece
      const double Fraction = min(max(static_cast<double>(i)/Intervals, EndPointInset), 1.0 - EndPointInset);

      // Map the integration coordinate back to x
      double X = Low + Fraction*Span;
      if (LogarithmicGrid == true) {
        if (LogarithmByRelativeWidth == true) {
          X = Low*exp(Fraction*Span);
        } else {
          X = exp(log(Low) + Fraction*Span);
        }
      }

      // Include dx/d(log x) = x on logarithmic grids
      double Value = Density(X);
      if (LogarithmicGrid == true) {
        Value *= X;
      }
      // Accumulate mass and the shifted first moment
      IntegralOfDensity += Weight*Value*Step/3.0;
      IntegralOfFirstMoment += Weight*Value*(X - Origin)*Step/3.0;
    }
  }

  if (IntegralOfDensity <= 0) {
    return 0.0;
  }

  return Origin + IntegralOfFirstMoment/IntegralOfDensity;
}

////////////////////////////////////////////////////////////////////////////////


//! Return the sample standard deviation, or zero for fewer than two values
//! Calculate sqrt(sum((x - mean)^2)/(n - 1)) using the compensated sample mean
//! Apply the n - 1 correction to the variance before taking its square root
double MStatistics::GetStandardDeviation(const vector<double>& Values)
{
  if (Values.size() < 2) {
    return 0.0;
  }
  const double Mean = GetMean(Values);
  double Sum = 0.0;
  for (double Value: Values) {
    Sum += (Value - Mean)*(Value - Mean);
  }
  return sqrt(Sum/(Values.size() - 1));
}


////////////////////////////////////////////////////////////////////////////////


//! Get the selected count, arithmetic mean, and RMS about Center
//! Select values with abs(x - Center) < Limit; exclude both boundary values
//! Return the mean in sample coordinates and RMS = sqrt(sum((x - Center)^2)/Number)
//! Set Number, Mean, and RMS to zero if no values pass, including for a nonpositive Limit
void MStatistics::GetTruncatedMoments(const vector<double>& Values, double Center, double Limit, unsigned int& Number, double& Mean, double& RMS)
{
  vector<double> Selected;
  double SumOfSquares = 0.0;
  for (double Value: Values) {
    if (fabs(Value - Center) < Limit) {
      Selected.push_back(Value);
      SumOfSquares += (Value - Center)*(Value - Center);
    }
  }
  Number = static_cast<unsigned int>(Selected.size());
  Mean = 0.0;
  RMS = 0.0;
  if (Number > 0) {
    Mean = GetMean(Selected);
    RMS = sqrt(SumOfSquares/Number);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the sample median, or zero for an empty sample
//! Sort a copy and take the central value for odd n or the average of the central pair for even n
double MStatistics::GetMedian(vector<double> Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  sort(Values.begin(), Values.end());
  const unsigned int Middle = Values.size()/2;
  if (Values.size() % 2 == 1) {
    return Values[Middle];
  }
  return 0.5*(Values[Middle - 1] + Values[Middle]);
}


////////////////////////////////////////////////////////////////////////////////


//! Return an approximate standard error of the sample median
//! For sorted values, use m = floor(n/2), h = max(1, floor(sqrt(n))), l = max(0, m - h), u = min(n - 1, m + h)
//! Estimate the local density as f = (u - l)/(n*(x[u] - x[l])) and return 1/(2*f*sqrt(n))
//! Use the smooth-density approximation near the median; tied values can give a zero local span
//! Return zero for an empty sample or a zero local span, including a singleton
double MStatistics::GetMedianError(vector<double> Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }

  sort(Values.begin(), Values.end());
  const unsigned int Size = Values.size();

  // Choose neighboring order statistics around the median
  const unsigned int Middle = Size/2;
  const unsigned int Half = max(1U, static_cast<unsigned int>(sqrt(static_cast<double>(Size))));
  unsigned int Lower = 0;
  if (Middle >= Half) {
    Lower = Middle - Half;
  }
  const unsigned int Upper = min(Size - 1, Middle + Half);
  const double Width = Values[Upper] - Values[Lower];
  if (Width <= 0.0) {
    return 0.0;
  }

  // Estimate the local density from the selected span
  const double Density = static_cast<double>(Upper - Lower)/(Size*Width);
  return 1.0/(2.0*Density*sqrt(static_cast<double>(Size)));
}


////////////////////////////////////////////////////////////////////////////////


//! Return the linearly interpolated sample quantile at Fraction
//! Interpolate between adjacent sorted values at the zero-based position Fraction*(n - 1)
//! For finite Fraction <= 0 return the minimum; for finite Fraction >= 1 return the maximum
//! Return zero for an empty sample or a non-finite Fraction, including either infinity
double MStatistics::GetQuantile(vector<double> Values, double Fraction)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  sort(Values.begin(), Values.end());
  // Handle non-finite fractions before clamping finite fractions
  if (isfinite(Fraction) == false) {
    return 0.0;
  }
  if (Fraction <= 0.0) {
    return Values.front();
  }
  if (Fraction >= 1.0) {
    return Values.back();
  }
  const double Position = Fraction*(Values.size() - 1);
  const unsigned int Lower = static_cast<unsigned int>(floor(Position));
  const unsigned int Upper = min(Lower + 1, static_cast<unsigned int>(Values.size()) - 1);
  return Values[Lower] + (Position - Lower)*(Values[Upper] - Values[Lower]);
}


////////////////////////////////////////////////////////////////////////////////


//! Return 1.4826 times the median absolute deviation from the sample median
//! Use interpolated medians; the scale factor converts Gaussian median absolute deviation to sigma
//! Return zero for an empty sample; repeated values can also give zero deviation
double MStatistics::GetRobustSigma(const vector<double>& Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  const double Center = GetQuantile(Values, 0.5);
  vector<double> Deviations;
  Deviations.reserve(Values.size());
  for (double V: Values) {
    Deviations.push_back(fabs(V - Center));
  }
  return c_MedianAbsoluteDeviationFactor*GetQuantile(Deviations, 0.5);
}


////////////////////////////////////////////////////////////////////////////////


//! Return half the width of the narrowest interval containing the requested sample fraction
//! Compare windows of ceil(Fraction*n) consecutive sorted values and allow the interval center to vary
//! Return half the full range for Fraction >= 1, including positive infinity
//! Return zero for an empty sample, Fraction <= 0, negative infinity, or NaN
//! A window containing one value has zero width; sample positions are not interpolated
//! Example: values 1, 2, 3, 4, 10 with Fraction = 0.8 select [1, 4] and return 1.5
double MStatistics::GetHalfWidthContaining(vector<double> Values, double Fraction)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  sort(Values.begin(), Values.end());
  // Include the full range for fractions at or above one
  if (Fraction >= 1.0) {
    return 0.5*(Values.back() - Values.front());
  }
  if (isfinite(Fraction) == false || Fraction <= 0.0) {
    return 0.0;
  }
  // Compare fixed-count windows in sorted order
  const unsigned int Needed = max(1U, static_cast<unsigned int>(ceil(Fraction*Values.size())));
  double Shortest = Values.back() - Values.front();
  for (unsigned int i = 0; i + Needed - 1 < Values.size(); ++i) {
    Shortest = min(Shortest, Values[i + Needed - 1] - Values[i]);
  }
  return 0.5*Shortest;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the unbinned half-sample mode (Bickel 2002), or zero for an empty sample
//! Repeatedly retain the shortest sorted window containing ceil(remaining count/2) values
//! For exactly tied shortest windows, choose the middle start, or the upper middle start for an even tie count
//! Return the value for a singleton or the average for two remaining values
//! For three remaining values, average the closest pair; return the middle value if the gaps tie
double MStatistics::GetPeakPosition(vector<double> Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  sort(Values.begin(), Values.end());

  // Retain the shortest half until at most three samples remain
  unsigned int Begin = 0;
  unsigned int End = Values.size();
  while (End - Begin > 3) {
    const unsigned int Half = (End - Begin + 1)/2;
    double ShortestLength = Values[End - 1] - Values[Begin];
    for (unsigned int i = Begin; i + Half <= End; ++i) {
      ShortestLength = min(ShortestLength, Values[i + Half - 1] - Values[i]);
    }
    // Collect starts with exactly the same minimal span
    vector<unsigned int> Starts;
    for (unsigned int i = Begin; i + Half <= End; ++i) {
      if (Values[i + Half - 1] - Values[i] == ShortestLength) {
        Starts.push_back(i);
      }
    }
    // Choose the upper middle start for an even tie count
    Begin = Starts[Starts.size()/2];
    End = Begin + Half;
  }

  // Resolve the final three samples by their closest pair
  if (End - Begin == 3) {
    const double LeftGap = Values[Begin + 1] - Values[Begin];
    const double RightGap = Values[Begin + 2] - Values[Begin + 1];
    if (LeftGap < RightGap) {
      return 0.5*(Values[Begin] + Values[Begin + 1]);
    }
    if (RightGap < LeftGap) {
      return 0.5*(Values[Begin + 1] + Values[Begin + 2]);
    }
    return Values[Begin + 1];
  }
  return 0.5*(Values[Begin] + Values[End - 1]);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fitted peak of a smoothed sample histogram centered on the median
//! Use 81 bins of width BinWidth and smoothing weights (1/4, 1/2, 1/4)
//! Ignore values outside the half-open window [median - 40.5*BinWidth, median + 40.5*BinWidth)
//! Fit a parabola through the first highest interior bin and its two neighbors
//! Return zero for fewer than 20 values, a nonpositive BinWidth, or no positive smoothed content
double MStatistics::GetPeakPosition(const vector<double>& Values, double BinWidth)
{
  double Center = 0.0;
  vector<double> Smoothed;
  if (MakeSmoothedHistogram(Values, BinWidth, Center, Smoothed) == false) {
    return 0.0;
  }
  unsigned int Max = 1;
  for (unsigned int b = 1; b < c_PeakHistogramBins - 1; ++b) {
    if (Smoothed[b] > Smoothed[Max]) {
      Max = b;
    }
  }
  if (Smoothed[Max] <= 0.0) {
    return 0.0;
  }
  // Express the fitted vertex in sample units
  const double Vertex = Max + GetParabolaVertex(Smoothed[Max - 1], Smoothed[Max], Smoothed[Max + 1]);
  return Center + (Vertex - 0.5*(c_PeakHistogramBins - 1))*BinWidth;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fitted location of the histogram bin selected by ROOT as the maximum
//! Require a non-null one-dimensional histogram with equal bin widths for the parabolic interpolation
//! Read the adjacent regular bins when the maximum is interior, including neighbors outside an active axis range
//! Use the selected bin center when either regular neighbor is unavailable or the parabola is not concave
//! Return zero if the selected maximum has nonpositive content
double MStatistics::GetPeakPosition(const TH1* Histogram)
{
  const unsigned int NBins = static_cast<unsigned int>(Histogram->GetNbinsX());
  const unsigned int Peak = static_cast<unsigned int>(const_cast<TH1*>(Histogram)->GetMaximumBin());
  if (Histogram->GetBinContent(Peak) <= 0.0) {
    return 0.0;
  }
  double Vertex = 0.0;
  if (Peak > 1 && Peak < NBins) {
    Vertex = GetParabolaVertex(Histogram->GetBinContent(Peak - 1), Histogram->GetBinContent(Peak), Histogram->GetBinContent(Peak + 1));
  }
  return Histogram->GetXaxis()->GetBinCenter(Peak) + Vertex*Histogram->GetXaxis()->GetBinWidth(Peak);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the width at Fraction times the highest interior bin of the smoothed sample histogram
//! Use the median-centered 81-bin window and smoothing weights (1/4, 1/2, 1/4)
//! Interpolate the nearest left and right crossings between interior bin centers
//! Treat a bin exactly at the level as a crossing at its center, including the nearest bin of a level plateau
//! Return zero for fewer than 20 values, a nonpositive BinWidth, or Fraction outside (0, 1)
//! Return zero for a nonpositive peak or if either crossing is missing inside the window
double MStatistics::GetFullWidthAtFractionOfMaximum(const vector<double>& Values, double BinWidth, double Fraction)
{
  if (Fraction <= 0.0 || Fraction >= 1.0) {
    return 0.0;
  }
  double Center = 0.0;
  vector<double> Smoothed;
  if (MakeSmoothedHistogram(Values, BinWidth, Center, Smoothed) == false) {
    return 0.0;
  }
  const unsigned int NBins = c_PeakHistogramBins;
  unsigned int Max = 1;
  for (unsigned int b = 1; b < NBins - 1; ++b) {
    if (Smoothed[b] > Smoothed[Max]) {
      Max = b;
    }
  }
  const double Level = Fraction*Smoothed[Max];
  if (Level <= 0.0) {
    return 0.0;
  }
  double Left = 0.0;
  bool LeftFound = false;
  for (unsigned int b = Max; b > 1; --b) {
    if (Smoothed[b - 1] <= Level) {
      Left = (b - 1) + (Level - Smoothed[b - 1])/(Smoothed[b] - Smoothed[b - 1]);
      LeftFound = true;
      break;
    }
  }
  double Right = 0.0;
  bool RightFound = false;
  for (unsigned int b = Max; b < NBins - 2; ++b) {
    if (Smoothed[b + 1] <= Level) {
      Right = b + (Smoothed[b] - Level)/(Smoothed[b] - Smoothed[b + 1]);
      RightFound = true;
      break;
    }
  }
  if (LeftFound == false || RightFound == false) {
    return 0.0;
  }
  return (Right - Left)*BinWidth;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the histogram width at Fraction times the bin selected by ROOT as the maximum
//! Require a non-null one-dimensional histogram; use its stored contents directly, also for variable-width bins
//! Search from the selected peak toward the first and last regular bins, including outside an active axis range
//! Interpolate the nearest crossings between bin centers; an exact level value crosses at its own center
//! Stop at the closest bin of a plateau at the requested level
//! Return zero for Fraction outside (0, 1), a nonpositive peak, or either missing crossing
double MStatistics::GetFullWidthAtFractionOfMaximum(const TH1* Histogram, double Fraction)
{
  if (Fraction <= 0.0 || Fraction >= 1.0) {
    return 0.0;
  }
  const unsigned int NBins = static_cast<unsigned int>(Histogram->GetNbinsX());
  const unsigned int Peak = static_cast<unsigned int>(const_cast<TH1*>(Histogram)->GetMaximumBin());
  const double Level = Fraction*Histogram->GetBinContent(Peak);
  if (Level <= 0) {
    return 0.0;
  }
  double Left = 0.0;
  bool LeftFound = false;
  for (unsigned int b = Peak; b > 1; --b) {
    if (Histogram->GetBinContent(b - 1) <= Level) {
      const double Low = Histogram->GetBinContent(b - 1), High = Histogram->GetBinContent(b);
      Left = Histogram->GetXaxis()->GetBinCenter(b - 1) + (Level - Low)/(High - Low)*(Histogram->GetXaxis()->GetBinCenter(b) - Histogram->GetXaxis()->GetBinCenter(b - 1));
      LeftFound = true;
      break;
    }
  }
  double Right = 0.0;
  bool RightFound = false;
  for (unsigned int b = Peak; b < NBins; ++b) {
    if (Histogram->GetBinContent(b + 1) <= Level) {
      const double Low = Histogram->GetBinContent(b + 1), High = Histogram->GetBinContent(b);
      Right = Histogram->GetXaxis()->GetBinCenter(b + 1) - (Level - Low)/(High - Low)*(Histogram->GetXaxis()->GetBinCenter(b + 1) - Histogram->GetXaxis()->GetBinCenter(b));
      RightFound = true;
      break;
    }
  }
  // Require a crossing on each side
  if (LeftFound == false || RightFound == false) {
    return 0.0;
  }
  return Right - Left;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the full width at half maximum of the smoothed sample histogram
//! Use the sample fractional-height width with Fraction = 0.5 and the same window and input requirements
//! Return zero if the peak or either crossing cannot be measured
double MStatistics::GetFullWidthAtHalfMaximum(const vector<double>& Values, double BinWidth)
{
  return GetFullWidthAtFractionOfMaximum(Values, BinWidth, 0.5);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the full width at half maximum of a non-null one-dimensional histogram
//! Use the histogram fractional-height width with Fraction = 0.5, including exact-level crossings
//! Return zero for a nonpositive peak or either missing crossing
double MStatistics::GetFullWidthAtHalfMaximum(const TH1* Histogram)
{
  return GetFullWidthAtFractionOfMaximum(Histogram, 0.5);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the interpolated Fraction quantile of absolute distances from the unbinned half-sample mode
//! Center the containment interval on the estimated peak and return its radius in sample units
//! For finite Fraction <= 0 return the smallest radius; for finite Fraction >= 1 return the largest
//! Return zero for an empty sample or a non-finite Fraction
double MStatistics::GetContainmentAroundPeak(const vector<double>& Values, double Fraction)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  const double Peak = GetPeakPosition(Values);
  vector<double> Distances;
  Distances.reserve(Values.size());
  for (double Value: Values) {
    Distances.push_back(fabs(Value - Peak));
  }
  return GetQuantile(Distances, Fraction);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fraction of samples strictly closer than Distance to the unbinned half-sample mode
//! Exclude samples exactly at Distance and return a fraction between zero and one
//! Return zero for an empty sample or a nonpositive Distance
double MStatistics::GetFractionAroundPeak(const vector<double>& Values, double Distance)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  const double Peak = GetPeakPosition(Values);
  unsigned int Inside = 0;
  for (double Value: Values) {
    if (fabs(Value - Peak) < Distance) {
      ++Inside;
    }
  }
  return static_cast<double>(Inside)/Values.size();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the bootstrap standard error of the supplied scalar statistic
//! Draw NResamples samples of the original size with replacement and evaluate Statistic on each (default 100)
//! Return the sample standard deviation of those estimates; raise NResamples below two to two
//! Return zero for fewer than two input values without invoking Statistic
//! Require a callable Statistic with finite results for every resample, including samples with repeated values
//! Use a local mt19937 seeded with 12345; repeatability requires a deterministic callback and the same sampling implementation
//! Propagate exceptions from Statistic
//! Can throw: std::bad_function_call for an empty Statistic when at least two input values are supplied
double MStatistics::GetBootstrapError(const vector<double>& Values, const MStatisticFunction& Statistic, unsigned int NResamples)
{
  if (Values.size() < 2) {
    return 0.0;
  }
  // Require at least two bootstrap estimates
  NResamples = max(2U, NResamples);
  // Seed a local generator for repeatable resampling
  const unsigned int Seed = 12345;
  mt19937 Generator(Seed);
  uniform_int_distribution<unsigned int> Pick(0, static_cast<unsigned int>(Values.size()) - 1);
  vector<double> Results;
  Results.reserve(NResamples);
  vector<double> Resample(Values.size());
  for (unsigned int r = 0; r < NResamples; ++r) {
    for (unsigned int i = 0; i < Values.size(); ++i) {
      Resample[i] = Values[Pick(Generator)];
    }
    Results.push_back(Statistic(Resample));
  }
  return GetStandardDeviation(Results);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the absolute deviation of CountA/CountB from ExpectedRatio in estimated standard-error units
//! For independent positive Poisson counts, use error = (CountA/CountB)*sqrt(1/CountA + 1/CountB)
//! Interpret the propagated-error score as a large-count approximation
//! Return the invalid-input sentinel 1e9 if either count is nonpositive
double MStatistics::GetRatioSigma(double CountA, double CountB, double ExpectedRatio)
{
  // Reject counts with an undefined ratio error
  if (CountA <= 0 || CountB <= 0) {
    const double NoSignificance = 1e9; // Undefined-result sentinel
    return NoSignificance;
  }
  return fabs(CountA/CountB - ExpectedRatio)/(CountA/CountB*sqrt(1.0/CountA + 1.0/CountB));
}


////////////////////////////////////////////////////////////////////////////////


//! Return the absolute pooled two-proportion z score for independent binomial samples K1/N1 and K2/N2
//! Pool the counts: p = (K1 + K2)/(N1 + N2), q = ((N1 - K1) + (N2 - K2))/(N1 + N2)
//! Use the equal-efficiency hypothesis and a normal approximation
//! Calculate abs(K1/N1 - K2/N2)/sqrt(p*q*(1/N1 + 1/N2))
//! Form both outcome counts directly and subtract the rates of the less common pooled outcome
//! Return zero for equal valid efficiencies, including two all-success or two all-failure samples
//! Return the sentinel 1e9 for nonpositive trials, successes outside [0, trials], or an undefined pooled error
double MStatistics::GetProportionSigma(double K1, double N1, double K2, double N2)
{
  const double NoSignificance = 1e9; // Undefined-result sentinel
  if (N1 <= 0 || N2 <= 0) {
    return NoSignificance;
  }
  // Validate each sample independently
  if (K1 < 0 || K1 > N1 || K2 < 0 || K2 > N2) {
    return NoSignificance;
  }
  // Form both pooled outcome counts directly
  const double Trials = N1 + N2;
  const double Successes = K1 + K2;
  const double Failures = (N1 - K1) + (N2 - K2);
  // Subtract rates of the less common pooled outcome
  double Difference = fabs(K1/N1 - K2/N2);
  if (Successes > Failures) {
    Difference = fabs((N1 - K1)/N1 - (N2 - K2)/N2);
  }
  // Handle equal rates before degenerate pooled samples
  if (Difference == 0.0) {
    return 0.0;
  }
  if (Successes <= 0.0 || Failures <= 0.0) {
    return NoSignificance;
  }
  return Difference/sqrt((Successes/Trials)*(Failures/Trials)*(1.0/N1 + 1.0/N2));
}


//! Fill Center with the sample median and Smoothed with the 81-bin smoothed sample histogram
//! Use the half-open window [Center - 40.5*BinWidth, Center + 40.5*BinWidth) and discard values outside it
//! Convolve interior bins with weights (1/4, 1/2, 1/4); leave the first and last smoothed bins at zero
//! Return false for fewer than 20 values or a nonpositive BinWidth, leaving both outputs unchanged
bool MStatistics::MakeSmoothedHistogram(const vector<double>& Values, double BinWidth, double& Center, vector<double>& Smoothed)
{
  // Require enough samples and a positive bin width
  const unsigned int MinimumValues = 20;
  if (Values.size() < MinimumValues || BinWidth <= 0.0) {
    return false;
  }
  Center = GetQuantile(Values, 0.5);
  // Place the central bin at the sample median
  const unsigned int NBins = c_PeakHistogramBins;
  vector<double> Content(NBins, 0.0);
  for (double V: Values) {
    // Fill only the half-open histogram window
    const double Position = (V - Center)/BinWidth + 0.5*NBins;
    if (Position >= 0.0 && Position < NBins) {
      Content[static_cast<unsigned int>(Position)] += 1.0;
    }
  }
  // Convolve interior bins with (1, 2, 1)/4
  Smoothed.assign(NBins, 0.0);
  for (unsigned int b = 1; b < NBins - 1; ++b) {
    Smoothed[b] = 0.25*Content[b - 1] + 0.5*Content[b] + 0.25*Content[b + 1];
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the vertex offset of the parabola through Left, Middle, Right at positions -1, 0, 1
//! Measure the offset in bin units and return zero for flat or upward curvature
double MStatistics::GetParabolaVertex(double Left, double Middle, double Right)
{
  // Use vertex -b/(2*a) for offsets -1, 0, and 1
  const double Curvature = Left - 2.0*Middle + Right;
  if (Curvature >= 0.0) {
    return 0.0;
  }
  return 0.5*(Left - Right)/Curvature;
}


// MStatistics.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
