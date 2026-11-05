/*
 * MUnitTest.h
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


#ifndef __MUnitTest__
#define __MUnitTest__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <mutex>
#include <sstream>
#include <typeinfo>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MStreams.h"

// Forward declarations:
class MVector;
class MRotation;

//! Mark one check as a known failure inside a derived member function
#define ExpectedToFail(...) (BeginExpectedFailureMode(), __VA_ARGS__, EndExpectedFailureMode())


////////////////////////////////////////////////////////////////////////////////


//! Class representing a base for unit tests
//!
//! A test program normally creates one instance of a class derived from MUnitTest
//! Its private test directory is created below the configured log directory
//! This directory becomes HOME and the working directory for the test program
//! It is also the temporary root: the top directory for the test's temporary files and subdirectories
//! The filesystem helpers restrict file access and removal to this directory
//! Remove the directory when the instance is destroyed after a passing test; keep it after a failed test
//!
//! Wrap one check with ExpectedToFail in a derived member function to mark a known failure
//! Report a failing wrapped check as KNOWN FAILURE and a passing one as UNEXPECTED PASS
//! Count an unexpected pass as a failed test
//!
//! UTUnitTest and UTEndToEndTest also create helper test instances in the same process
//! Each helper has its own private test directory, but HOME and the working directory are shared by the process
//! They remain pointed at the first instance's directory, even while helper instances are created and destroyed
class MUnitTest
{
  // public interface:
 public:
  //! Standard constructor
  MUnitTest(const MString& Name);
  //! Default destructor
  virtual ~MUnitTest();

  //! Run the unit test
  virtual bool Run() = 0;

  //! Summarize the test run
  void Summarize();
  //! Read the last passed, failed, and known failure counts from a summary, return false if missing or invalid
  //! The known failures are zero if the summary has no such line (it is printed only if there are any)
  static bool ParseSummary(const MString& Output, unsigned int& Passed, unsigned int& Failed, unsigned int& KnownFailures);

  //! Evaluate if the output equals the expected value
  template <typename T1, typename T2> bool Evaluate(MString Function, T1 Input, MString Description, T2 Output, T2 Truth)
  {
    if (Output != Truth) {
      ostringstream ExpectedStream;
      ExpectedStream<<setprecision(numeric_limits<long double>::max_digits10)<<Truth;
      ostringstream OutputStream;
      OutputStream<<setprecision(numeric_limits<long double>::max_digits10)<<Output;
      RegisterFailure(Function, Input, Description, ExpectedStream.str(), OutputStream.str());
      return false;
    }

    RegisterSuccess(Function, Input, Description);
    return true;
  }

  //! Evaluate a boolean expression expected to be true
  template <typename T> bool EvaluateTrue(MString Function, T Input, MString Description, bool Output)
  {
    return Evaluate(Function, Input, Description, Output, true);
  }

  //! Evaluate a boolean expression expected to be false
  template <typename T> bool EvaluateFalse(MString Function, T Input, MString Description, bool Output)
  {
    return Evaluate(Function, Input, Description, Output, false);
  }

  //! Evaluate the expected size of a container or collection
  template <typename T> bool EvaluateSize(MString Function, T Input, MString Description, unsigned long Output, unsigned long Truth)
  {
    return Evaluate(Function, Input, Description, Output, Truth);
  }

  //! Evaluate if two finite floating-point values differ by at most Tolerance
  template <typename T> bool EvaluateNear(MString Function, T Input, MString Description, double Output, double Truth, double Tolerance)
  {
    if (std::isfinite(Output) == false || std::isfinite(Truth) == false || std::isfinite(Tolerance) == false || fabs(Output - Truth) > Tolerance) {
      ostringstream ExpectedStream;
      ExpectedStream<<setprecision(numeric_limits<long double>::max_digits10)<<Truth<<" +/- "<<Tolerance;
      ostringstream OutputStream;
      OutputStream<<setprecision(numeric_limits<long double>::max_digits10)<<Output;
      RegisterFailure(Function, Input, Description, ExpectedStream.str(), OutputStream.str());
      return false;
    }

    RegisterSuccess(Function, Input, Description);
    return true;
  }

  //! Evaluate if two finite vectors have a distance of at most Tolerance
  bool EvaluateVectorNear(MString Function, MString Input, MString Description, const MVector& Output, const MVector& Truth, double Tolerance);

  //! Evaluate if each pair of finite rotation matrix elements differs by at most Tolerance
  bool EvaluateRotationNear(MString Function, MString Input, MString Description, const MRotation& Output, const MRotation& Truth, double Tolerance);

  //! Evaluate if a callable throws the expected exception type
  template <typename TException, typename TInput, typename TCallable> bool EvaluateException(MString Function, TInput Input, MString Description, TCallable Callable)
  {
    try {
      Callable();
    } catch (const TException&) {
      RegisterSuccess(Function, Input, Description);
      return true;
    } catch (const std::exception& Exception) {
      ostringstream ExpectedStream;
      ExpectedStream<<"exception of type "<<typeid(TException).name();
      ostringstream OutputStream;
      OutputStream<<"std::exception: "<<Exception.what();
      RegisterFailure(Function, Input, Description, ExpectedStream.str(), OutputStream.str());
      return false;
    } catch (...) {
      ostringstream ExpectedStream;
      ExpectedStream<<"exception of type "<<typeid(TException).name();
      RegisterFailure(Function, Input, Description, ExpectedStream.str(), "unknown exception");
      return false;
    }

    ostringstream ExpectedStream;
    ExpectedStream<<"exception of type "<<typeid(TException).name();
    RegisterFailure(Function, Input, Description, ExpectedStream.str(), "no exception");
    return false;
  }

  //! Evaluate if two text files have identical lines, ignoring LF/CRLF endings and a missing final newline
  bool EvaluateFilesIdentical(MString Function, MString Input, MString Description, const MString& TestFile, const MString& ReferenceFile);

  //! Evaluate if text files have matching tokens on each line, ignoring whitespace between tokens
  //! Allow floating-point differences up to MaximumLastDigitDifference times the smaller last-printed-digit unit
  //! Accept numerical differences only for zero or finite normal doubles; other tokens must match exactly
  //! Report the first unequal line or a different line count
  bool EvaluateFilesNumericallyEquivalent(MString Function, MString Input, MString Description,
                                          const MString& TestFile, const MString& ReferenceFile,
                                          unsigned int MaximumLastDigitDifference = 2);

  //! Remove source context lines from error messages
  MString RemoveErrorMessageContext(const MString& Text) const;

  //! Return the larger of two values or NaN if one of them is NaN
  double GetMaximum(double First, double Second) const;

  
  
  // protected methods:
 protected:
  //! Return the unit test name
  const MString& GetName() const { return m_Name; }

  //! Count a passed check, or report and count an unexpected pass as a failed test
  template <typename T> void RegisterSuccess(MString Function, T Input, MString Description) {
    if (m_IsExpectedFailureModeActive == true) {
      ++m_NumberOfExpectedFailureModeChecks;
      m_HasUnexpectedPass = true;
      mout<<endl;
      mout<<"UNEXPECTED PASS: "<<Function<<"  <-- "<<Input<<endl;
      mout<<"   Description: "<<Description<<endl;
      mout<<"   The check passes now: remove ExpectedToFail()"<<endl;
      mout<<endl;
      ++m_NumberOfFailedTests;
    } else {
      ++m_NumberOfPassedTests;
    }
  }

  //! Report a failed check and count it as a failed test or a known failure
  template <typename T> void RegisterFailure(MString Function, T Input, MString Description, MString Expected, MString Output) {
    mout<<endl;
    if (m_IsExpectedFailureModeActive == true) {
      mout<<"KNOWN FAILURE: "<<Function<<"  <-- "<<Input<<endl;
    } else {
      mout<<"FAILED: "<<Function<<"  <-- "<<Input<<endl;
    }
    mout<<"   Description: "<<Description<<endl;
    mout<<"   Expected:    "<<Expected<<endl;
    mout<<"   Output:      "<<Output<<endl;
    mout<<endl;

    if (m_IsExpectedFailureModeActive == true) {
      ++m_NumberOfExpectedFailureModeChecks;
      ++m_NumberOfKnownFailures;
    } else {
      ++m_NumberOfFailedTests;
    }
  }

  //! Begin tracking the check wrapped by ExpectedToFail
  void BeginExpectedFailureMode();
  //! End tracking and return true if exactly one wrapped check failed
  bool EndExpectedFailureMode();

  //! Disable the default MEGAlib streams around expected noisy calls
  void DisableDefaultStreams() {
    mout.Enable(false);
    mlog.Enable(false);
    mgui.Enable(false);
    __merr.Enable(false);
  }

  //! Enable the default MEGAlib streams before evaluating checks
  void EnableDefaultStreams() {
    mout.Enable(true);
    mlog.Enable(true);
    mgui.Enable(true);
    __merr.Enable(true);
  }

  //! Return a file path below the private temporary root without creating the file
  MString GetTemporaryFileName(const MString& Name) const;

  //! Return a directory path below the private temporary root (empty name: root itself)
  MString GetTemporaryDirectoryName(const MString& Name = "") const;

  //! Remove and recreate a temporary directory (empty name: clear the root contents)
  bool PrepareTemporaryDirectory(const MString& Name = "") const;

  //! Remove a file inside the private temporary root
  bool RemoveTemporaryFile(const MString& FileName) const;

  //! Remove a directory and its contents inside the private temporary root (empty path: root itself)
  bool RemoveTemporaryDirectory(const MString& DirectoryName = "") const;

  //! Write complete text content to a test fixture file
  bool WriteTextFile(const MString& FileName, const MString& Content) const;

  //! Read complete text content from a test fixture file, return an empty string on failure
  MString ReadTextFile(const MString& FileName) const;

  
  
  // private methods:
 private:
  //! Create a private temporary root directory and redirect HOME and the working directory there
  bool RedirectHomeDirectory();

  //! Create this object's private test directory below the configured log directory, or reuse it if it still exists
  //! Use the test basename and a random suffix, and check that the directory stays inside the log directory
  //! Hold a directory lock against test driver cleanup; leave HOME and the working directory unchanged
  //! Return false if directory creation or path validation fails
  bool CreateTemporaryRootDirectory() const;
  //! Return the randomized private temporary root
  MString GetTemporaryRootDirectory() const;
  //! Release the lock on the temporary root
  void ReleaseTemporaryRootLock() const;
  //! Build a directory basename from ASCII letters, digits, '_' and '-' (fallback: "UnitTest")
  MString CreateTemporaryDirectoryBaseName(const MString& Name) const;

  //! Read the log directory from the testing settings file
  void LoadLogDirectory();
  //! Return the log directory name
  MString GetLogDirectory() const { return m_LogDirectory; }

  //! Return true for a plain file or directory name (empty name: allowed if AllowEmpty is true)
  bool IsValidTemporaryPathName(const MString& Name, bool AllowEmpty = false) const;

  //! Return true if the path resolves inside the private temporary root (AllowRoot: accept the root itself)
  bool IsSafeTemporaryPath(const MString& Path, bool AllowRoot) const;

  //! Return true if Child equals Parent or lies below it
  bool IsPathContained(const std::filesystem::path& Parent, const std::filesystem::path& Child) const;

  //! Return true if two lines have matching tokens, allowing differences in the last printed digits
  bool LinesMatchNumerically(const MString& TestLine, const MString& ReferenceLine,
                             unsigned int MaximumLastDigitDifference) const;

  //! Read the count of a summary line after its label: digits only, up to the end, a blank, or the comma of the one-line form
  //! Return false if there is no count or it is larger than an unsigned int
  static bool ParseSummaryCount(const string& Text, string::size_type Position, unsigned int& Count);

                             
                             
  // protected members:
 protected:


 // private members:
 private:
   //! Name of the unit test
   MString m_Name;
   //! Basename of the private temporary root
   MString m_TemporaryBaseName;

   //! Number of passed tests
   unsigned int m_NumberOfPassedTests;
   //! Number of failed tests
   unsigned int m_NumberOfFailedTests;
   //! Number of known failures marked with ExpectedToFail
   unsigned int m_NumberOfKnownFailures;

   //! Flag indicating if a wrapped check is being tracked
   bool m_IsExpectedFailureModeActive;
   //! Number of nested expected-failure modes to ignore
   unsigned int m_NumberOfNestedExpectedFailureModes;
   //! Number of checks evaluated in the active expected-failure mode
   unsigned int m_NumberOfExpectedFailureModeChecks;
   //! Flag indicating if a wrapped check passed unexpectedly
   bool m_HasUnexpectedPass;

   //! Path of the randomized private temporary root
   mutable MString m_TemporaryRootDirectory;
   //! File descriptor holding the temporary root lock against test driver cleanup (-1: none)
   mutable int m_TemporaryRootLock;
   //! The log directory
   MString m_LogDirectory;
   //! Mutex protecting temporary root creation and filesystem operations
   mutable recursive_mutex m_TemporaryPathMutex;


#ifdef ___CLING___
 public:
  ClassDef(MUnitTest, 1)
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////
