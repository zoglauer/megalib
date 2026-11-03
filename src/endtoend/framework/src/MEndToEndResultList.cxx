/*
 * MEndToEndResultList.cxx
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
#include "MEndToEndResultList.h"

// Standard libs:
#include <cmath>
#include <fstream>
#include <limits>
using namespace std;

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Add a key result: name, value, uncertainty, return it
MEndToEndResult& MEndToEndResultList::AddResult(const MString& Name, double Value, double Error)
{
  MEndToEndResult Result(Name);
  Result.SetValue(Value);
  Result.SetError(Error);
  m_Results.push_back(Result);
  return m_Results.back();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a key result which was measured before (e.g. read from a file), return it
MEndToEndResult& MEndToEndResultList::AddResult(const MEndToEndResult& Result)
{
  m_Results.push_back(Result);
  return m_Results.back();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a comparison of a measured value with an expected value to the key results, the tolerance is the largest accepted difference, return it
MEndToEndResult& MEndToEndResultList::AddComparison(const MString& Name, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  MEndToEndResult& Result = AddResult(Name, Measured.m_Value, Measured.m_Error);
  Result.SetHasReference(true);
  Result.SetReference(Expected.m_Value);
  Result.SetReferenceError(Expected.m_Error);
  Result.SetTolerance(Tolerance);
  Result.SetStatistical(IsStatisticalComparison(Measured, Expected, Tolerance, IsStatistical));
  Result.SetPull(GetScore(Measured, Expected, Tolerance, IsStatistical));

  // A comparison with a value which is not valid fails: the check of the test overwrites the outcome
  Result.SetValid(false);
  if (Measured.IsValid() == true && Expected.IsValid() == true && isfinite(Tolerance) == true) {
    Result.SetValid(true);
  }
  Result.SetPassed(false);
  if (Result.IsValid() == true && fabs(Measured.m_Value - Expected.m_Value) <= Tolerance) {
    Result.SetPassed(true);
  }
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the statistics of the pulls of all key results which have a reference
ETPullStatistics MEndToEndResultList::GetPullStatistics() const
{
  // Only valid statistical comparisons are Gaussian pulls
  ETPullStatistics Statistics;
  for (const MEndToEndResult& Result: m_Results) {
    if (Result.HasReference() == true && Result.IsValid() == true && Result.IsStatistical() == true) {
      ++Statistics.m_NumberOfPulls;
      for (unsigned int s = 0; s < 3; ++s) {
        if (fabs(Result.GetPull()) <= s + 1.0) {
          ++Statistics.m_Inside[s];
        }
      }
    }
  }
  return Statistics;
}


////////////////////////////////////////////////////////////////////////////////


//! Replace the key results by the ones in the file which Write() made, return false if the file cannot be read or has a line which is not a key result
bool MEndToEndResultList::Read(const MString& FileName)
{
  m_Results.clear();

  MString Content;
  if (MFile::ReadTextFile(FileName, Content) == false) {
    return false;
  }

  // One key result per line: a line which is not one makes the whole file unusable, a campaign must not miss an observable
  for (const MString& Line: Content.Tokenize("\n")) {
    if (Line.BeginsWith("#") == true) {
      continue;
    }
    MEndToEndResult Result;
    if (Result.FromLine(Line) == false) {
      m_Results.clear();
      return false;
    }
    m_Results.push_back(Result);
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the difference of a measured and an expected value in combined standard errors (10 if they differ without uncertainties, NaN if a value is not valid)
double MEndToEndResultList::GetPull(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected)
{
  // An infinite or NaN uncertainty must not hide a difference
  if (Measured.IsValid() == false || Expected.IsValid() == false) {
    return numeric_limits<double>::quiet_NaN();
  }
  const double Sigma = sqrt(Measured.m_Error*Measured.m_Error + Expected.m_Error*Expected.m_Error);
  if (Sigma > 0.0) {
    return (Measured.m_Value - Expected.m_Value)/Sigma;
  }
  if (Measured.m_Value == Expected.m_Value) {
    return 0.0;
  }
  return 10.0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the tolerance of a statistical comparison: the pull limit times the combined uncertainty, NaN if a value is not valid
double MEndToEndResultList::GetStatisticalTolerance(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected)
{
  if (Measured.IsValid() == false || Expected.IsValid() == false) {
    return numeric_limits<double>::quiet_NaN();
  }
  return c_PullLimit*sqrt(Measured.m_Error*Measured.m_Error + Expected.m_Error*Expected.m_Error);
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the comparison can be a statistical pull: the uncertainties are not zero and the tolerance is the pull limit times the combined uncertainty, if it was requested
bool MEndToEndResultList::IsStatisticalComparison(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  if (IsStatistical == false || Measured.IsValid() == false || Expected.IsValid() == false) {
    return false;
  }

  // Without uncertainties the pull is zero or ten, whatever the difference
  const double Sigma = sqrt(Measured.m_Error*Measured.m_Error + Expected.m_Error*Expected.m_Error);
  if (Sigma <= 0.0) {
    return false;
  }

  // A tolerance which is not the pull limit (a systematic allowance, another number of sigma) is not seen by the pull
  const double RelativeMatch = 1e-6;
  if (fabs(Tolerance - c_PullLimit*Sigma) > RelativeMatch*Tolerance) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the score of a comparison: the pull if it is statistical, otherwise the difference in units of the tolerance divided by the pull limit, NaN if a value is not valid
double MEndToEndResultList::GetScore(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  if (Measured.IsValid() == false || Expected.IsValid() == false || isfinite(Tolerance) == false) {
    return numeric_limits<double>::quiet_NaN();
  }
  if (IsStatisticalComparison(Measured, Expected, Tolerance, IsStatistical) == true) {
    return GetPull(Measured, Expected);
  }

  // The tolerance is the limit: the score is the pull limit at the limit like the pull
  const double Difference = Measured.m_Value - Expected.m_Value;
  if (Tolerance > 0.0) {
    return Difference*c_PullLimit/Tolerance;
  }
  if (Difference == 0.0) {
    return 0.0;
  }
  return 10.0;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the key results as tab separated text (17 digits), return false if the file cannot be written
bool MEndToEndResultList::Write(const MString& FileName) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) {
    merr<<"Unable to write the key results: "<<FileName<<show;
    return false;
  }
  Out<<"# columns: "<<MEndToEndResult::GetColumnNames()<<endl;
  for (const MEndToEndResult& Result: m_Results) {
    Out<<Result.ToLine()<<endl;
  }
  return Out.good();
}


// MEndToEndResultList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
