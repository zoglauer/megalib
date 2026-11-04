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


//! Define a scalar statistic of a sample vector for bootstrap resampling
//! Use a lambda capture for fixed extra parameters and return a finite estimate for each resample
//! Example: estimate the median ARM and its bootstrap standard error, with ARM values in degrees
//! \code{.cpp}
//!   vector<double> ARMs = { -0.7, 2.1, 0.3, -1.4 };
//!   MStatisticFunction Median = [](const vector<double>& V) { return MStatistics::GetMedian(V); };
//!   double Position = Median(ARMs);
//!   double Error = MStatistics::GetBootstrapError(ARMs, Median);
//! \endcode
typedef function<double(const vector<double>&)> MStatisticFunction;


//! Class representing sample statistics and numerical means of densities
//! Provide static operations and leave the caller's sample vectors unchanged
//! Require finite sample values and parameters except for the documented special fraction cases
//! Use sample units for locations, standard deviations, widths, and radii; fractions and z scores are dimensionless
//! Use MEGAlib-scale magnitudes; extreme double exponents can overflow or underflow intermediate calculations
//! For example, magnitudes above 1e150 or nonzero magnitudes below 1e-150 may give infinity, NaN, or zero
//! Several routines store sample counts and indices as unsigned int
//! Use peak estimators for one dominant component; multimodal samples need not select the tallest peak
//! Limit binned sample peak searches to an 81-bin window centered on the median
//! Require non-null one-dimensional histograms for histogram overloads and finite bin contents
//! Zero may be a valid result or the documented empty or undefined-result sentinel
class MStatistics
{
  // public interface:
 public:

  // Moments:

  //! Return the arithmetic mean of the sample values, or zero for an empty sample
  //! Calculate sum(x)/n with compensated differences from the first value and a corrected division
  //! Identical values retain exactly their original value as the mean
  static double GetMean(const vector<double>& Values);
  //! Return the sample standard deviation, or zero for fewer than two values
  //! Calculate sqrt(sum((x - mean)^2)/(n - 1)) using the compensated sample mean
  //! Apply the n - 1 correction to the variance before taking its square root
  static double GetStandardDeviation(const vector<double>& Values);
  //! Get the selected count, arithmetic mean, and RMS about Center
  //! Select values with abs(x - Center) < Limit; exclude both boundary values
  //! Return the mean in sample coordinates and RMS = sqrt(sum((x - Center)^2)/Number)
  //! Set Number, Mean, and RMS to zero if no values pass, including for a nonpositive Limit
  static void GetTruncatedMoments(const vector<double>& Values, double Center, double Limit, unsigned int& Number, double& Mean, double& RMS);
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
  static double GetMeanOfDensity(const function<double(double)>& Density, double Min, double Max, const vector<double>& Breakpoints = vector<double>());


  // Order statistics:

  //! Return the sample median, or zero for an empty sample
  //! Sort a copy and take the central value for odd n or the average of the central pair for even n
  static double GetMedian(vector<double> Values);
  //! Return an approximate standard error of the sample median
  //! For sorted values, use m = floor(n/2), h = max(1, floor(sqrt(n))), l = max(0, m - h), u = min(n - 1, m + h)
  //! Estimate the local density as f = (u - l)/(n*(x[u] - x[l])) and return 1/(2*f*sqrt(n))
  //! Use the smooth-density approximation near the median; tied values can give a zero local span
  //! Return zero for an empty sample or a zero local span, including a singleton
  static double GetMedianError(vector<double> Values);
  //! Return the linearly interpolated sample quantile at Fraction
  //! Interpolate between adjacent sorted values at the zero-based position Fraction*(n - 1)
  //! For finite Fraction <= 0 return the minimum; for finite Fraction >= 1 return the maximum
  //! Return zero for an empty sample or a non-finite Fraction, including either infinity
  static double GetQuantile(vector<double> Values, double Fraction);
  //! Return 1.4826 times the median absolute deviation from the sample median
  //! Use interpolated medians; the scale factor converts Gaussian median absolute deviation to sigma
  //! Return zero for an empty sample; repeated values can also give zero deviation
  static double GetRobustSigma(const vector<double>& Values);
  //! Return half the width of the narrowest interval containing the requested sample fraction
  //! Compare windows of ceil(Fraction*n) consecutive sorted values and allow the interval center to vary
  //! Return half the full range for Fraction >= 1, including positive infinity
  //! Return zero for an empty sample, Fraction <= 0, negative infinity, or NaN
  //! A window containing one value has zero width; sample positions are not interpolated
  //! Example: values 1, 2, 3, 4, 10 with Fraction = 0.8 select [1, 4] and return 1.5
  static double GetHalfWidthContaining(vector<double> Values, double Fraction);


  // Peaks:

  //! Return the unbinned half-sample mode (Bickel 2002), or zero for an empty sample
  //! Repeatedly retain the shortest sorted window containing ceil(remaining count/2) values
  //! For exactly tied shortest windows, choose the middle start, or the upper middle start for an even tie count
  //! Return the value for a singleton or the average for two remaining values
  //! For three remaining values, average the closest pair; return the middle value if the gaps tie
  static double GetPeakPosition(vector<double> Values);
  //! Return the fitted peak of a smoothed sample histogram centered on the median
  //! Use 81 bins of width BinWidth and smoothing weights (1/4, 1/2, 1/4)
  //! Ignore values outside the half-open window [median - 40.5*BinWidth, median + 40.5*BinWidth)
  //! Fit a parabola through the first highest interior bin and its two neighbors
  //! Return zero for fewer than 20 values, a nonpositive BinWidth, or no positive smoothed content
  static double GetPeakPosition(const vector<double>& Values, double BinWidth);
  //! Return the fitted location of the histogram bin selected by ROOT as the maximum
  //! Require a non-null one-dimensional histogram with equal bin widths for the parabolic interpolation
  //! Read the adjacent regular bins when the maximum is interior, including neighbors outside an active axis range
  //! Use the selected bin center when either regular neighbor is unavailable or the parabola is not concave
  //! Return zero if the selected maximum has nonpositive content
  static double GetPeakPosition(const TH1* Histogram);
  //! Return the width at Fraction times the highest interior bin of the smoothed sample histogram
  //! Use the median-centered 81-bin window and smoothing weights (1/4, 1/2, 1/4)
  //! Interpolate the nearest left and right crossings between interior bin centers
  //! Treat a bin exactly at the level as a crossing at its center, including the nearest bin of a level plateau
  //! Return zero for fewer than 20 values, a nonpositive BinWidth, or Fraction outside (0, 1)
  //! Return zero for a nonpositive peak or if either crossing is missing inside the window
  static double GetFullWidthAtFractionOfMaximum(const vector<double>& Values, double BinWidth, double Fraction);
  //! Return the histogram width at Fraction times the bin selected by ROOT as the maximum
  //! Require a non-null one-dimensional histogram; use its stored contents directly, also for variable-width bins
  //! Search from the selected peak toward the first and last regular bins, including outside an active axis range
  //! Interpolate the nearest crossings between bin centers; an exact level value crosses at its own center
  //! Stop at the closest bin of a plateau at the requested level
  //! Return zero for Fraction outside (0, 1), a nonpositive peak, or either missing crossing
  static double GetFullWidthAtFractionOfMaximum(const TH1* Histogram, double Fraction);
  //! Return the full width at half maximum of the smoothed sample histogram
  //! Use the sample fractional-height width with Fraction = 0.5 and the same window and input requirements
  //! Return zero if the peak or either crossing cannot be measured
  static double GetFullWidthAtHalfMaximum(const vector<double>& Values, double BinWidth);
  //! Return the full width at half maximum of a non-null one-dimensional histogram
  //! Use the histogram fractional-height width with Fraction = 0.5, including exact-level crossings
  //! Return zero for a nonpositive peak or either missing crossing
  static double GetFullWidthAtHalfMaximum(const TH1* Histogram);


  //! Return the interpolated Fraction quantile of absolute distances from the unbinned half-sample mode
  //! Center the containment interval on the estimated peak and return its radius in sample units
  //! For finite Fraction <= 0 return the smallest radius; for finite Fraction >= 1 return the largest
  //! Return zero for an empty sample or a non-finite Fraction
  static double GetContainmentAroundPeak(const vector<double>& Values, double Fraction);
  //! Return the fraction of samples strictly closer than Distance to the unbinned half-sample mode
  //! Exclude samples exactly at Distance and return a fraction between zero and one
  //! Return zero for an empty sample or a nonpositive Distance
  static double GetFractionAroundPeak(const vector<double>& Values, double Distance);


  // Resampling:

  //! Return the bootstrap standard error of the supplied scalar statistic
  //! Draw NResamples samples of the original size with replacement and evaluate Statistic on each (default 100)
  //! Return the sample standard deviation of those estimates; raise NResamples below two to two
  //! Return zero for fewer than two input values without invoking Statistic
  //! Require a callable Statistic with finite results for every resample, including samples with repeated values
  //! Use a local mt19937 seeded with 12345; repeatability requires a deterministic callback and the same sampling implementation
  //! Propagate exceptions from Statistic
  //! Can throw: std::bad_function_call for an empty Statistic when at least two input values are supplied
  static double GetBootstrapError(const vector<double>& Values, const MStatisticFunction& Statistic, unsigned int NResamples = 100);


  // Significances:

  //! Return the absolute deviation of CountA/CountB from ExpectedRatio in estimated standard-error units
  //! For independent positive Poisson counts, use error = (CountA/CountB)*sqrt(1/CountA + 1/CountB)
  //! Interpret the propagated-error score as a large-count approximation
  //! Return the invalid-input sentinel 1e9 if either count is nonpositive
  static double GetRatioSigma(double CountA, double CountB, double ExpectedRatio);
  //! Return the absolute pooled two-proportion z score for independent binomial samples K1/N1 and K2/N2
  //! Pool the counts: p = (K1 + K2)/(N1 + N2), q = ((N1 - K1) + (N2 - K2))/(N1 + N2)
  //! Use the equal-efficiency hypothesis and a normal approximation
  //! Calculate abs(K1/N1 - K2/N2)/sqrt(p*q*(1/N1 + 1/N2))
  //! Form both outcome counts directly and subtract the rates of the less common pooled outcome
  //! Return zero for equal valid efficiencies, including two all-success or two all-failure samples
  //! Return the sentinel 1e9 for nonpositive trials, successes outside [0, trials], or an undefined pooled error
  static double GetProportionSigma(double K1, double N1, double K2, double N2);


  // private methods:
 private:
  //! Fill Center with the sample median and Smoothed with the 81-bin smoothed sample histogram
  //! Use the half-open window [Center - 40.5*BinWidth, Center + 40.5*BinWidth) and discard values outside it
  //! Convolve interior bins with weights (1/4, 1/2, 1/4); leave the first and last smoothed bins at zero
  //! Return false for fewer than 20 values or a nonpositive BinWidth, leaving both outputs unchanged
  static bool MakeSmoothedHistogram(const vector<double>& Values, double BinWidth, double& Center, vector<double>& Smoothed);
  //! Return the vertex offset of the parabola through Left, Middle, Right at positions -1, 0, 1
  //! Measure the offset in bin units and return zero for flat or upward curvature
  static double GetParabolaVertex(double Left, double Middle, double Right);

  // private members:
 private:
  //! Number of bins in the median-centered sample histogram, including the two boundary bins
  //! An odd count places the median at the central bin center
  static constexpr unsigned int c_PeakHistogramBins = 81;

  // public members:
 public:
  //! Gaussian sigma divided by the median absolute deviation, rounded to 1.4826
  static constexpr double c_MedianAbsoluteDeviationFactor = 1.4826;
};

#endif


////////////////////////////////////////////////////////////////////////////////
