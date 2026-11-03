/*
 * MEndToEndResult.h
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


#ifndef __MEndToEndResult__
#define __MEndToEndResult__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
using namespace std;

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A key result of an end-to-end test: a measured value and, if there is one, its reference and the outcome of the comparison
//! With the line which stores it in the file of the key results
class MEndToEndResult
{
  // public interface:
 public:
  //! Default constructor
  MEndToEndResult() = default;
  //! Standard constructor giving the name of the observable: scenario and observable
  explicit MEndToEndResult(const MString& Name) : m_Name(Name) {}
  //! Default destructor
  ~MEndToEndResult() = default;

  //! Set the the name: scenario and observable
  void SetName(const MString& Name) { m_Name = Name; }
  //! Return the the name: scenario and observable
  const MString& GetName() const { return m_Name; }

  //! Set the The measured value
  void SetValue(double Value) { m_Value = Value; }
  //! Return the The measured value
  double GetValue() const { return m_Value; }

  //! Set the The uncertainty of the measured value
  void SetError(double Error) { m_Error = Error; }
  //! Return the The uncertainty of the measured value
  double GetError() const { return m_Error; }

  //! Set the True if there is a reference
  void SetHasReference(bool HasReference) { m_HasReference = HasReference; }
  //! Return the True if there is a reference
  bool HasReference() const { return m_HasReference; }

  //! Set the The reference value
  void SetReference(double Reference) { m_Reference = Reference; }
  //! Return the The reference value
  double GetReference() const { return m_Reference; }

  //! Set the The uncertainty of the reference
  void SetReferenceError(double ReferenceError) { m_ReferenceError = ReferenceError; }
  //! Return the The uncertainty of the reference
  double GetReferenceError() const { return m_ReferenceError; }

  //! Set the The score of the comparison: the difference to the reference in combined standard errors for a statistical comparison, in units of the tolerance divided by the pull limit otherwise (zero without a reference, NaN if invalid)
  void SetPull(double Pull) { m_Pull = Pull; }
  //! Return the The score of the comparison: the difference to the reference in combined standard errors for a statistical comparison, in units of the tolerance divided by the pull limit otherwise (zero without a reference, NaN if invalid)
  double GetPull() const { return m_Pull; }

  //! Set the The largest difference to the reference which the test accepts
  void SetTolerance(double Tolerance) { m_Tolerance = Tolerance; }
  //! Return the The largest difference to the reference which the test accepts
  double GetTolerance() const { return m_Tolerance; }

  //! Set the True if the values and their uncertainties are finite (a comparison which is not valid fails)
  void SetValid(bool Valid) { m_IsValid = Valid; }
  //! Return the True if the values and their uncertainties are finite (a comparison which is not valid fails)
  bool IsValid() const { return m_IsValid; }

  //! Set the True if the score is the difference in standard errors, which follows a Gaussian for statistical fluctuations alone: only these scores are part of the statistics of the pulls
  void SetStatistical(bool Statistical) { m_IsStatistical = Statistical; }
  //! Return the True if the score is the difference in standard errors, which follows a Gaussian for statistical fluctuations alone: only these scores are part of the statistics of the pulls
  bool IsStatistical() const { return m_IsStatistical; }

  //! Set the True if the value is inside the tolerance of the reference: the outcome of the check of the test
  void SetPassed(bool Passed) { m_Passed = Passed; }
  //! Return the True if the value is inside the tolerance of the reference: the outcome of the check of the test
  bool IsPassed() const { return m_Passed; }

  //! Set the True if the observable is compared with a reference of a campaign, false for an expectation which the test derives itself
  void SetFromReferences(bool FromReferences) { m_FromReferences = FromReferences; }
  //! Return the True if the observable is compared with a reference of a campaign, false for an expectation which the test derives itself
  bool IsFromReferences() const { return m_FromReferences; }

  //! Set the result from a line of the file (twelve tab separated columns), return false and leave the result as it is if the line has another number of columns
  bool FromLine(const MString& Line);
  //! Return the line of the file: all columns with 17 digits, a tab in the name is replaced by a space
  MString ToLine() const;

  //! Return the names of the columns of the line
  static MString GetColumnNames();

  // private members:
 private:
  //! The name: scenario and observable
  MString m_Name;
  //! The measured value
  double m_Value = 0.0;
  //! The uncertainty of the measured value
  double m_Error = 0.0;
  //! True if there is a reference
  bool m_HasReference = false;
  //! The reference value
  double m_Reference = 0.0;
  //! The uncertainty of the reference
  double m_ReferenceError = 0.0;
  //! The score of the comparison: the difference to the reference in combined standard errors for a statistical comparison, in units of the tolerance divided by the pull limit otherwise (zero without a reference, NaN if invalid)
  double m_Pull = 0.0;
  //! The largest difference to the reference which the test accepts
  double m_Tolerance = 0.0;
  //! True if the values and their uncertainties are finite (a comparison which is not valid fails)
  bool m_IsValid = true;
  //! True if the score is the difference in standard errors, which follows a Gaussian for statistical fluctuations alone: only these scores are part of the statistics of the pulls
  bool m_IsStatistical = true;
  //! True if the value is inside the tolerance of the reference: the outcome of the check of the test
  bool m_Passed = true;
  //! True if the observable is compared with a reference of a campaign, false for an expectation which the test derives itself
  bool m_FromReferences = false;
};

#endif


////////////////////////////////////////////////////////////////////////////////
