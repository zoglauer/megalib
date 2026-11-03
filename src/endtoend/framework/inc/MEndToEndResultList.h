/*
 * MEndToEndResultList.h
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

#ifndef __MEndToEndResultList__
#define __MEndToEndResultList__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <array>
#include <cmath>
#include <deque>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MEndToEndResult.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A measured value with its uncertainty
class ETMeasuredValue
{
public:
  //! The value
  double m_Value = 0.0;
  //! The uncertainty (standard error)
  double m_Error = 0.0;

  //! Return true if the value and the uncertainty are finite and the uncertainty is not negative
  bool IsValid() const
  {
    if (isfinite(m_Value) == false || isfinite(m_Error) == false || m_Error < 0.0) {
      return false;
    }
    return true;
  }
};


//! The distribution of the pulls of all comparisons: are they distributed as expected for statistical fluctuations?
class ETPullStatistics
{
public:
  //! The fractions inside one, two, and three sigma which are expected for Gaussian pulls
  static constexpr array<double, 3> c_GaussianFractions = { 0.6827, 0.9545, 0.9973 };

  //! The number of pulls
  unsigned int m_NumberOfPulls = 0;
  //! The number of pulls with |pull| <= 1, 2, 3
  vector<unsigned int> m_Inside = { 0, 0, 0 };

  //! Return the fraction of the pulls inside the given number of sigma (1, 2, or 3), the uncertainty is zero
  ETMeasuredValue GetFraction(unsigned int Sigma) const
  {
    ETMeasuredValue Fraction;
    if (Sigma >= 1 && Sigma <= 3 && m_NumberOfPulls > 0) {
      Fraction.m_Value = static_cast<double>(m_Inside[Sigma - 1])/m_NumberOfPulls;
    }
    return Fraction;
  }
};


////////////////////////////////////////////////////////////////////////////////


//! The list of the key results of an end-to-end test: measured values, comparisons with their expectations, and their scores
//! * The test fills them, the report shows them, and the test driver and the reference campaign read them from the file which the test writes
//! * The file is tab separated text, one line per key result (MEndToEndResult::ToLine()), the first line names the columns
class MEndToEndResultList
{
  // public interface:
 public:
  //! The comparisons are inside the tolerance if the pull is not larger than this number of combined standard errors
  static constexpr double c_PullLimit = 5.0;

  //! Default constructor
  MEndToEndResultList() = default;
  //! Default destructor
  ~MEndToEndResultList() = default;

  //! Add a key result: name, value, uncertainty, return it
  MEndToEndResult& AddResult(const MString& Name, double Value, double Error);
  //! Add a key result which was measured before (e.g. read from a file), return it
  MEndToEndResult& AddResult(const MEndToEndResult& Result);
  //! Add a comparison to the key results with the tolerance (the largest accepted difference) and the kind of score (statistical: in standard errors, otherwise in units of the tolerance divided by the pull limit), return it
  MEndToEndResult& AddComparison(const MString& Name, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Return the key results
  const deque<MEndToEndResult>& GetResults() const { return m_Results; }
  //! Return the key result which was added last for changing it (there has to be one)
  MEndToEndResult& GetLastResult() { return m_Results.back(); }
  //! Return true if there are no key results
  bool IsEmpty() const { return m_Results.empty(); }
  //! Return the statistics of the pulls of all valid statistical comparisons (comparisons without uncertainties and with a tolerance which is not the pull limit times the uncertainty are scored against their tolerance)
  ETPullStatistics GetPullStatistics() const;

  //! Replace the key results by the ones in the file which Write() made, return false if the file cannot be read or has a line which is not a key result
  bool Read(const MString& FileName);
  //! Write the key results as tab separated text (17 digits), return false if the file cannot be written
  bool Write(const MString& FileName) const;

  //! Return the difference of a measured and an expected value in combined standard errors (10 if they differ without uncertainties, NaN if a value is not valid)
  static double GetPull(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected);
  //! Return the tolerance of a statistical comparison: the pull limit times the combined uncertainty, NaN if a value is not valid
  static double GetStatisticalTolerance(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected);
  //! Return true if the comparison can be a statistical pull: the uncertainties are not zero and the tolerance is the pull limit times the combined uncertainty, if it was requested
  static bool IsStatisticalComparison(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Return the score of a comparison: the pull if it is statistical, otherwise the difference in units of the tolerance divided by the pull limit, NaN if a value is not valid
  static double GetScore(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);

  // private members:
 private:
  //! The key results (a deque: the references which are returned stay valid)
  deque<MEndToEndResult> m_Results;
};

#endif


////////////////////////////////////////////////////////////////////////////////
