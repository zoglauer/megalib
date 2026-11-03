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


//! Return the mean of the values (zero if empty)
//! mean = (1/n) * sum x_i
double MStatistics::GetMean(const vector<double>& Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  double Sum = 0.0;
  for (double Value: Values) {
    Sum += Value;
  }
  return Sum/Values.size();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the sample standard deviation (zero for fewer than two values)
//! Divide the sum of squared deviations from the mean by n - 1, then take the square root
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


//! Get the count, mean, and RMS of values with |value - Center| < Limit
//! Measure the RMS from Center; set all outputs to zero if no values are selected
void MStatistics::GetTruncatedMoments(const vector<double>& Values, double Center, double Limit, unsigned int& Number, double& Mean, double& RMS)
{
  double Sum = 0.0;
  double SumOfSquares = 0.0;
  Number = 0;
  for (double Value: Values) {
    if (fabs(Value - Center) < Limit) {
      Sum += Value;
      SumOfSquares += (Value - Center)*(Value - Center);
      ++Number;
    }
  }
  Mean = 0.0;
  RMS = 0.0;
  if (Number > 0) {
    Mean = Sum/Number;
    RMS = sqrt(SumOfSquares/Number);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the median of the values (zero if empty)
//! Use the middle sorted value for an odd count, or the mean of the two middle values for an even count
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


//! Return the estimated standard error of the median (zero if empty or the local spread is zero)
//! Estimate the density f near the median using up to sqrt(n) values on each side
//! Calculate the error as 1 / (2 * f * sqrt(n))
double MStatistics::GetMedianError(vector<double> Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }

  sort(Values.begin(), Values.end());
  const unsigned int Size = Values.size();

  // Estimate the density near the median using up to sqrt(N) values on each side:
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

  const double Density = static_cast<double>(Upper - Lower)/(Size*Width);
  return 1.0/(2.0*Density*sqrt(static_cast<double>(Size)));
}


////////////////////////////////////////////////////////////////////////////////


//! Return the value at the requested fraction of the sorted data (zero if empty)
//! Interpolate between neighboring values at position Fraction * (n - 1)
//! Fractions at or below zero give the minimum; fractions at or above one give the maximum
//! Non-finite fractions give zero
double MStatistics::GetQuantile(vector<double> Values, double Fraction)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  sort(Values.begin(), Values.end());
  // Reject non-finite fractions, then clamp finite fractions to the range [0, 1]
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


//! Return a width estimate resistant to outliers (zero if empty)
//! Multiply the median distance from the median by 1.4826 to match the sigma of a Gaussian
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


//! Return half the width of the shortest interval containing the requested fraction of values (zero if empty)
//! Compare windows of ceil(Fraction * n) consecutive sorted values and choose the narrowest
//! A fraction of one or more, including positive infinity, includes all values
//! A fraction of zero or less, negative infinity, or NaN gives zero
//! Example: for 1, 2, 3, 4, 10 and Fraction = 0.8, the shortest interval is [1, 4], giving 1.5
double MStatistics::GetHalfWidthContaining(vector<double> Values, double Fraction)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  sort(Values.begin(), Values.end());
  // Include all values for fractions >= 1; return zero for other invalid fractions
  if (Fraction >= 1.0) {
    return 0.5*(Values.back() - Values.front());
  }
  if (isfinite(Fraction) == false || Fraction <= 0.0) {
    return 0.0;
  }
  const unsigned int Needed = max(1U, static_cast<unsigned int>(ceil(Fraction*Values.size())));
  double Shortest = Values.back() - Values.front();
  for (unsigned int i = 0; i + Needed - 1 < Values.size(); ++i) {
    Shortest = min(Shortest, Values[i + Needed - 1] - Values[i]);
  }
  return 0.5*Shortest;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the peak position using the half-sample mode (Bickel 2002), without binning (zero if empty)
//! Repeatedly select the shortest interval containing half the remaining sorted values
//! Finish with the mean of two values, or the closest pair of three; use the middle value if both pairs tie
//! For equally short intervals, choose the middle interval
//! This method resists outliers and asymmetric tails, but its uncertainty decreases only as n^(-1/3)
double MStatistics::GetPeakPosition(vector<double> Values)
{
  if (Values.empty() == true) {
    return 0.0;
  }
  sort(Values.begin(), Values.end());

  // Keep the shortest half of [Begin, End) until at most three values remain
  unsigned int Begin = 0;
  unsigned int End = Values.size();
  while (End - Begin > 3) {
    const unsigned int Half = (End - Begin + 1)/2;
    double ShortestLength = Values[End - 1] - Values[Begin];
    for (unsigned int i = Begin; i + Half <= End; ++i) {
      ShortestLength = min(ShortestLength, Values[i + Half - 1] - Values[i]);
    }
    // Reuse the same subtraction to find exact ties for the shortest interval
    vector<unsigned int> Starts;
    for (unsigned int i = Begin; i + Half <= End; ++i) {
      if (Values[i + Half - 1] - Values[i] == ShortestLength) {
        Starts.push_back(i);
      }
    }
    Begin = Starts[Starts.size()/2];
    End = Begin + Half;
  }

  // Average the closest pair; use the middle value if both gaps are equal
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


//! Return the peak position from a smoothed histogram centered on the median (zero if no peak is found)
//! Use c_PeakHistogramBins bins of width BinWidth and smoothing weights (1/4, 1/2, 1/4)
//! Fit a parabola through the highest interior bin and its neighbors to refine the position
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
  // Convert the fitted peak position from bin coordinates to data coordinates
  const double Vertex = Max + GetParabolaVertex(Smoothed[Max - 1], Smoothed[Max], Smoothed[Max + 1]);
  return Center + (Vertex - 0.5*(c_PeakHistogramBins - 1))*BinWidth;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the histogram peak position (zero if the highest bin has no positive content)
//! Fit a parabola through the highest bin and its neighbors, assuming equal bin widths
//! Use the bin center alone if the highest bin is the first or last bin
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


//! Return the peak width at Fraction times the maximum of a smoothed histogram
//! Use c_PeakHistogramBins bins of width BinWidth centered on the median, with smoothing weights (1/4, 1/2, 1/4)
//! Interpolate the level crossings on both sides of the highest interior bin
//! Return zero if the peak or either crossing is missing, or Fraction is outside the open interval (0, 1)
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
    if (Smoothed[b - 1] < Level) {
      Left = (b - 1) + (Level - Smoothed[b - 1])/(Smoothed[b] - Smoothed[b - 1]);
      LeftFound = true;
      break;
    }
  }
  double Right = 0.0;
  bool RightFound = false;
  for (unsigned int b = Max; b < NBins - 2; ++b) {
    if (Smoothed[b + 1] < Level) {
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


//! Return the histogram peak width at Fraction times the highest bin content
//! Interpolate the level crossings on both sides of the highest bin
//! Return zero if either crossing is missing, the peak has no positive content, or Fraction is outside (0, 1)
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
    if (Histogram->GetBinContent(b - 1) < Level) {
      const double Low = Histogram->GetBinContent(b - 1), High = Histogram->GetBinContent(b);
      Left = Histogram->GetXaxis()->GetBinCenter(b - 1) + (Level - Low)/(High - Low)*(Histogram->GetXaxis()->GetBinCenter(b) - Histogram->GetXaxis()->GetBinCenter(b - 1));
      LeftFound = true;
      break;
    }
  }
  double Right = 0.0;
  bool RightFound = false;
  for (unsigned int b = Peak; b < NBins; ++b) {
    if (Histogram->GetBinContent(b + 1) < Level) {
      const double Low = Histogram->GetBinContent(b + 1), High = Histogram->GetBinContent(b);
      Right = Histogram->GetXaxis()->GetBinCenter(b + 1) - (Level - Low)/(High - Low)*(Histogram->GetXaxis()->GetBinCenter(b + 1) - Histogram->GetXaxis()->GetBinCenter(b));
      RightFound = true;
      break;
    }
  }
  // Require both level crossings to measure the peak width
  if (LeftFound == false || RightFound == false) {
    return 0.0;
  }
  return Right - Left;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the full width at half maximum from a smoothed histogram centered on the median
//! Use the same binning and crossing search as the width at a fraction of the maximum, with Fraction = 0.5
//! Return zero if the peak or either crossing is missing
double MStatistics::GetFullWidthAtHalfMaximum(const vector<double>& Values, double BinWidth)
{
  return GetFullWidthAtFractionOfMaximum(Values, BinWidth, 0.5);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the histogram peak width at half its maximum (zero if the width cannot be determined)
double MStatistics::GetFullWidthAtHalfMaximum(const TH1* Histogram)
{
  return GetFullWidthAtFractionOfMaximum(Histogram, 0.5);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the radius around the half-sample peak containing the requested fraction of values (zero if empty)
//! Take the requested quantile of the absolute distances from the peak
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


//! Return the fraction of values with an absolute distance from the half-sample peak less than Distance (zero if empty)
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


//! Return the bootstrap uncertainty of the supplied statistic (zero for fewer than two values)
//! Draw samples of the original size with replacement and calculate the statistic for each sample
//! Return the sample standard deviation of the results, using at least two resamples
//! Use a fixed random seed for repeatable results with the same inputs and statistic
double MStatistics::GetBootstrapError(const vector<double>& Values, const MStatisticFunction& Statistic, unsigned int NResamples)
{
  if (Values.size() < 2) {
    return 0.0;
  }
  // The standard deviation needs two resamples
  NResamples = max(2U, NResamples);
  // Use a fixed seed to make resampling repeatable
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
  double Mean = 0.0;
  for (double R: Results) {
    Mean += R;
  }
  Mean /= Results.size();
  double Sum = 0.0;
  for (double R: Results) {
    Sum += (R - Mean)*(R - Mean);
  }
  return sqrt(Sum/(Results.size() - 1));
}


////////////////////////////////////////////////////////////////////////////////


//! Return the absolute difference between CountA/CountB and ExpectedRatio in units of the estimated ratio error
//! Estimate the error for independent Poisson counts as (CountA/CountB) * sqrt(1/CountA + 1/CountB)
//! Return 1e9 if either count is zero or negative
double MStatistics::GetRatioSigma(double CountA, double CountB, double ExpectedRatio)
{
  // Var(A/B) ~ (A/B)^2 * (1/A + 1/B)
  if (CountA <= 0 || CountB <= 0) {
    const double NoSignificance = 1e9; // sigma: the ratio is undefined
    return NoSignificance;
  }
  return fabs(CountA/CountB - ExpectedRatio)/(CountA/CountB*sqrt(1.0/CountA + 1.0/CountB));
}


////////////////////////////////////////////////////////////////////////////////


//! Return the absolute two-proportion z score comparing the efficiencies K1/N1 and K2/N2
//! Use the pooled success rate p = (K1 + K2) / (N1 + N2) to estimate the error
//! Return zero for equal efficiencies, or 1e9 for nonpositive trial counts or an undefined pooled error
double MStatistics::GetProportionSigma(double K1, double N1, double K2, double N2)
{
  const double NoSignificance = 1e9; // sigma: the proportions are undefined
  if (N1 <= 0 || N2 <= 0) {
    return NoSignificance;
  }
  // Return zero for equal efficiencies, including all-success and all-failure samples
  const double Difference = fabs(K1/N1 - K2/N2);
  if (Difference == 0.0) {
    return 0.0;
  }
  const double Proportion = (K1 + K2)/(N1 + N2);
  if (Proportion <= 0.0 || Proportion >= 1.0) {
    return NoSignificance;
  }
  return Difference/sqrt(Proportion*(1.0 - Proportion)*(1.0/N1 + 1.0/N2));
}


//! Build a histogram centered on the median and smooth it with weights (1/4, 1/2, 1/4)
//! Return false for fewer than 20 values or a nonpositive bin width
bool MStatistics::MakeSmoothedHistogram(const vector<double>& Values, double BinWidth, double& Center, vector<double>& Smoothed)
{
  // Require enough values for a binned peak estimate
  const unsigned int MinimumValues = 20;
  if (Values.size() < MinimumValues || BinWidth <= 0.0) {
    return false;
  }
  Center = GetQuantile(Values, 0.5);
  // Center the histogram window on the median to focus on the peak
  const unsigned int NBins = c_PeakHistogramBins;
  vector<double> Content(NBins, 0.0);
  for (double V: Values) {
    // Bin only values inside the window; the range check also rejects NaN
    const double Position = (V - Center)/BinWidth + 0.5*NBins;
    if (Position >= 0.0 && Position < NBins) {
      Content[static_cast<unsigned int>(Position)] += 1.0;
    }
  }
  Smoothed.assign(NBins, 0.0);
  for (unsigned int b = 1; b < NBins - 1; ++b) {
    Smoothed[b] = 0.25*Content[b - 1] + 0.5*Content[b] + 0.25*Content[b + 1];
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the parabola peak offset from the middle bin, using the left, middle, and right bin contents
//! Measure the offset in bins and return zero if the parabola does not open downward
double MStatistics::GetParabolaVertex(double Left, double Middle, double Right)
{
  // Fit a parabola at bin offsets -1, 0, and 1; its peak is at -b/(2a)
  const double Curvature = Left - 2.0*Middle + Right;
  if (Curvature >= 0.0) {
    return 0.0;
  }
  return 0.5*(Left - Right)/Curvature;
}


// MStatistics.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
