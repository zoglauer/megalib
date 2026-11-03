/*
 * MStatistics.h
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

#ifndef __MStatistics__
#define __MStatistics__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <functional>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:

// Forward declarations:
class TH1;


////////////////////////////////////////////////////////////////////////////////


//! A function that summarizes a list of values as one number, such as a mean, median, or width
//! Use a lambda to fix any extra arguments before passing the function to the bootstrap
//! Example: estimate the angular resolution and its uncertainty from ARM values in degrees
//!   vector<double> ARMs = { -0.7, 2.1, 0.3, -1.4, ... }; // One ARM value per Compton event
//!   MStatisticFunction Fwhm = [](const vector<double>& Values) { return MStatistics::GetFullWidthAtHalfMaximum(Values, 0.5); };
//!   double Width = Fwhm(ARMs); // FWHM using 0.5 degree bins
//!   double Error = MStatistics::GetBootstrapError(ARMs, Fwhm); // Uncertainty from resampling
typedef function<double(const vector<double>&)> MStatisticFunction;


//! Class representing statistical tools for lists of values
//! All functions are static and require finite input values
//! Peak functions assume a single peak and select the highest peak if several are present
//! Equally high peaks may give an arbitrary result without a warning
class MStatistics
{
  // public interface:
 public:

  // Moments:

  //! Return the mean of the values (zero if empty)
  //! mean = (1/n) * sum x_i
  static double GetMean(const vector<double>& Values);
  //! Return the sample standard deviation (zero for fewer than two values)
  //! Divide the sum of squared deviations from the mean by n - 1, then take the square root
  static double GetStandardDeviation(const vector<double>& Values);
  //! Get the count, mean, and RMS of values with |value - Center| < Limit
  //! Measure the RMS from Center; set all outputs to zero if no values are selected
  static void GetTruncatedMoments(const vector<double>& Values, double Center, double Limit, unsigned int& Number, double& Mean, double& RMS);


  // Order statistics:

  //! Return the median of the values (zero if empty)
  //! Use the middle sorted value for an odd count, or the mean of the two middle values for an even count
  static double GetMedian(vector<double> Values);
  //! Return the estimated standard error of the median (zero if empty or the local spread is zero)
  //! Estimate the density f near the median using up to sqrt(n) values on each side
  //! Calculate the error as 1 / (2 * f * sqrt(n))
  static double GetMedianError(vector<double> Values);
  //! Return the value at the requested fraction of the sorted data (zero if empty)
  //! Interpolate between neighboring values at position Fraction * (n - 1)
  //! Fractions at or below zero give the minimum; fractions at or above one give the maximum
  //! Non-finite fractions give zero
  static double GetQuantile(vector<double> Values, double Fraction);
  //! Return a width estimate resistant to outliers (zero if empty)
  //! Multiply the median distance from the median by 1.4826 to match the sigma of a Gaussian
  static double GetRobustSigma(const vector<double>& Values);
  //! Return half the width of the shortest interval containing the requested fraction of values (zero if empty)
  //! Compare windows of ceil(Fraction * n) consecutive sorted values and choose the narrowest
  //! A fraction of one or more, including positive infinity, includes all values
  //! A fraction of zero or less, negative infinity, or NaN gives zero
  //! Example: for 1, 2, 3, 4, 10 and Fraction = 0.8, the shortest interval is [1, 4], giving 1.5
  static double GetHalfWidthContaining(vector<double> Values, double Fraction);


  // Peaks:

  //! Return the peak position using the half-sample mode (Bickel 2002), without binning (zero if empty)
  //! Repeatedly select the shortest interval containing half the remaining sorted values
  //! Finish with the mean of two values, or the closest pair of three; use the middle value if both pairs tie
  //! For equally short intervals, choose the middle interval
  //! This method resists outliers and asymmetric tails, but its uncertainty decreases only as n^(-1/3)
  static double GetPeakPosition(vector<double> Values);
  //! Return the peak position from a smoothed histogram centered on the median (zero if no peak is found)
  //! Use c_PeakHistogramBins bins of width BinWidth and smoothing weights (1/4, 1/2, 1/4)
  //! Fit a parabola through the highest interior bin and its neighbors to refine the position
  static double GetPeakPosition(const vector<double>& Values, double BinWidth);
  //! Return the histogram peak position (zero if the highest bin has no positive content)
  //! Fit a parabola through the highest bin and its neighbors, assuming equal bin widths
  //! Use the bin center alone if the highest bin is the first or last bin
  static double GetPeakPosition(const TH1* Histogram);
  //! Return the peak width at Fraction times the maximum of a smoothed histogram
  //! Use c_PeakHistogramBins bins of width BinWidth centered on the median, with smoothing weights (1/4, 1/2, 1/4)
  //! Interpolate the level crossings on both sides of the highest interior bin
  //! Return zero if the peak or either crossing is missing, or Fraction is outside the open interval (0, 1)
  static double GetFullWidthAtFractionOfMaximum(const vector<double>& Values, double BinWidth, double Fraction);
  //! Return the histogram peak width at Fraction times the highest bin content
  //! Interpolate the level crossings on both sides of the highest bin
  //! Return zero if either crossing is missing, the peak has no positive content, or Fraction is outside (0, 1)
  static double GetFullWidthAtFractionOfMaximum(const TH1* Histogram, double Fraction);
  //! Return the full width at half maximum from a smoothed histogram centered on the median
  //! Use the same binning and crossing search as the width at a fraction of the maximum, with Fraction = 0.5
  //! Return zero if the peak or either crossing is missing
  static double GetFullWidthAtHalfMaximum(const vector<double>& Values, double BinWidth);
  //! Return the histogram peak width at half its maximum (zero if the width cannot be determined)
  static double GetFullWidthAtHalfMaximum(const TH1* Histogram);


  //! Return the radius around the half-sample peak containing the requested fraction of values (zero if empty)
  //! Take the requested quantile of the absolute distances from the peak
  static double GetContainmentAroundPeak(const vector<double>& Values, double Fraction);
  //! Return the fraction of values with an absolute distance from the half-sample peak less than Distance (zero if empty)
  static double GetFractionAroundPeak(const vector<double>& Values, double Distance);


  // Resampling:

  //! Return the bootstrap uncertainty of the supplied statistic (zero for fewer than two values)
  //! Draw samples of the original size with replacement and calculate the statistic for each sample
  //! Return the sample standard deviation of the results, using at least two resamples
  //! Use a fixed random seed for repeatable results with the same inputs and statistic
  static double GetBootstrapError(const vector<double>& Values, const MStatisticFunction& Statistic, unsigned int NResamples = 100);


  // Significances:

  //! Return the absolute difference between CountA/CountB and ExpectedRatio in units of the estimated ratio error
  //! Estimate the error for independent Poisson counts as (CountA/CountB) * sqrt(1/CountA + 1/CountB)
  //! Return 1e9 if either count is zero or negative
  static double GetRatioSigma(double CountA, double CountB, double ExpectedRatio);
  //! Return the absolute two-proportion z score comparing the efficiencies K1/N1 and K2/N2
  //! Use the pooled success rate p = (K1 + K2) / (N1 + N2) to estimate the error
  //! Return zero for equal efficiencies, or 1e9 for nonpositive trial counts or an undefined pooled error
  static double GetProportionSigma(double K1, double N1, double K2, double N2);


  // private methods:
 private:
  //! Build a histogram centered on the median and smooth it with weights (1/4, 1/2, 1/4)
  //! Return false for fewer than 20 values or a nonpositive bin width
  static bool MakeSmoothedHistogram(const vector<double>& Values, double BinWidth, double& Center, vector<double>& Smoothed);
  //! Return the parabola peak offset from the middle bin, using the left, middle, and right bin contents
  //! Measure the offset in bins and return zero if the parabola does not open downward
  static double GetParabolaVertex(double Left, double Middle, double Right);

  // private members:
 private:
  //! The odd number of histogram bins, so that the middle bin is centered on the median
  //! The binned peak and width searches are limited to this window
  static constexpr unsigned int c_PeakHistogramBins = 81;

  // public members:
 public:
  //! The scale factor relating the median absolute deviation to Gaussian sigma
  static constexpr double c_MedianAbsoluteDeviationFactor = 1.4826;
};

#endif


////////////////////////////////////////////////////////////////////////////////
