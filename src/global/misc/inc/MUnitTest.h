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


////////////////////////////////////////////////////////////////////////////////


//! The base class for unit testing
class MUnitTest
{
  // public interface:
 public:
  //! Construct a named unit test
  MUnitTest(const MString& Name);
  //! Default destuctor 
  virtual ~MUnitTest();
  
  //! Evaluate the success of this test run
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
    
    RegisterSuccess();
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

  //! Evaluate two floating-point values within a given tolerance
  template <typename T> bool EvaluateNear(MString Function, T Input, MString Description, double Output, double Truth, double Tolerance)
  {
    if (std::isfinite(Output) == false || std::isfinite(Truth) == false || fabs(Output - Truth) > Tolerance) {
      ostringstream ExpectedStream;
      ExpectedStream<<setprecision(numeric_limits<long double>::max_digits10)<<Truth<<" +/- "<<Tolerance;
      ostringstream OutputStream;
      OutputStream<<setprecision(numeric_limits<long double>::max_digits10)<<Output;
      RegisterFailure(Function, Input, Description, ExpectedStream.str(), OutputStream.str());
      return false;
    }

    RegisterSuccess();
    return true;
  }

  //! Evaluate two vectors within a given tolerance: the distance between them must not exceed Tolerance, all components must be finite
  bool EvaluateVectorNear(MString Function, MString Input, MString Description, const MVector& Output, const MVector& Truth, double Tolerance);

  //! Evaluate two rotation matrices within a given tolerance: each of the nine elements may differ by at most Tolerance, all elements must be finite
  bool EvaluateRotationNear(MString Function, MString Input, MString Description, const MRotation& Output, const MRotation& Truth, double Tolerance);

  //! Evaluate the expected size of a container or collection
  template <typename T> bool EvaluateSize(MString Function, T Input, MString Description, unsigned int Output, unsigned int Truth)
  {
    return Evaluate(Function, Input, Description, Output, Truth);
  }

  //! Evaluate if a callable throws the expected exception type
  template <typename TException, typename TInput, typename TCallable> bool EvaluateException(MString Function, TInput Input, MString Description, TCallable Callable)
  {
    try {
      Callable();
    } catch (const TException&) {
      RegisterSuccess();
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
  
  //! Evaluate that two text files contain exactly the same lines
  //! All characters within each line, including whitespace, must match; line-ending characters are consumed while reading
  //! The files must contain the same number of lines; the first unequal line is reported
  bool EvaluateFilesIdentical(MString Function, MString Input, MString Description, const MString& TestFile, const MString& ReferenceFile);

  //! Evaluate that two text files have the same lines and whitespace-separated tokens
  //! Floating-point tokens may differ by MaximumLastDigitDifference times the smaller last-printed-digit unit; all other tokens must match exactly
  //! Tolerance matching requires both differing tokens to represent zero or finite normal doubles; subnormal and out-of-range values do not match numerically
  //! For example, with MaximumLastDigitDifference = 2, 0.518236 and 0.518237 match because their difference of 0.000001 is less than 2 * 0.000001
  //! Likewise, 1.234e-4 and 1.235e-4 match because their difference of 0.001e-4 is less than 2 * 0.001e-4
  //! Differences in whitespace between tokens are ignored; the first unequal line is reported
  bool EvaluateFilesNumericallyEquivalent(MString Function, MString Input, MString Description,
                                          const MString& TestFile, const MString& ReferenceFile,
                                          unsigned int MaximumLastDigitDifference = 2);

  //! Run the unit test
  virtual bool Run() = 0;

  //! Summarize the test run
  void Summarize();
  //! Read the numbers of passed and failed tests from the output of Summarize (the last "Passed tests: N" followed by "Failed tests: M"), return false if there are none
  static bool ParseSummary(const MString& Output, unsigned int& Passed, unsigned int& Failed);

  // protected methods:
 protected:
  //! Register a passed test
  void RegisterSuccess() { ++m_NumberOfPassedTests; }

  //! Return the unit test name
  const MString& GetName() const { return m_Name; }

  //! Disable the default MEGAlib output streams for expected noisy test paths
  void DisableDefaultStreams() {
    mout.Enable(false);
    mlog.Enable(false);
    mgui.Enable(false);
    __merr.Enable(false);
  }

  //! Re-enable the default MEGAlib output streams after a noisy test path
  void EnableDefaultStreams() {
    mout.Enable(true);
    mlog.Enable(true);
    mgui.Enable(true);
    __merr.Enable(true);
  }

  //! Write complete text content to a test fixture file
  bool WriteTextFile(const MString& FileName, const MString& Content) const;

  //! Read complete text content from a test fixture file; returns an empty string on failure
  MString ReadTextFile(const MString& FileName) const;

  //! Return a process-local temporary file name for this test; the file is not created
  MString GetTemporaryFileName(const MString& Name) const;

  //! Return a process-local temporary directory name for this test; the directory is not created
  MString GetTemporaryDirectoryName(const MString& Name = "") const;

  //! Remove and recreate a process-local temporary directory for this test, the root itself (no name) is only cleared
  bool PrepareTemporaryDirectory(const MString& Name = "") const;

  //! Remove a temporary file only if it is inside this test's randomized temporary root
  bool RemoveTemporaryFile(const MString& FileName) const;

  //! Recursively remove a temporary directory only if it is inside this test's randomized temporary root; an empty name removes the root itself
  bool RemoveTemporaryDirectory(const MString& DirectoryName = "") const;

  //! Register and report a failed test
  template <typename T> void RegisterFailure(MString Function, T Input, MString Description, MString Expected, MString Output) {
    mout<<endl;
    mout<<"FAILED: "<<Function<<"  <-- "<<Input<<endl;
    mout<<"   Description: "<<Description<<endl;
    mout<<"   Expected:    "<<Expected<<endl;
    mout<<"   Output:      "<<Output<<endl;
    mout<<endl;

    ++m_NumberOfFailedTests;
  }

  // private methods:
 private:
  //! Release the lock on the temporary root
  void ReleaseTemporaryRootLock() const;
  //! Create this test's randomized private temporary root if necessary
  bool CreateTemporaryRootDirectory() const;

  //! Return the randomized private temporary root for this test
  MString GetTemporaryRootDirectory() const;
  //! The log directory name 
  MString GetLogDirectory() const { return m_LogDirectory; }
  //! Read the log directory from the testing settings file
  void LoadLogDirectory();

  //! Return true only for a plain child file or directory name without path components; see IsValidTemporaryBaseName() for the root-basename rule
  bool IsValidTemporaryPathName(const MString& Name, bool AllowEmpty = false) const;

  //! Return the basename of the temporary directory: the characters of the name which are ASCII letters, digits, '_' and '-', or "UnitTest" if there are none
  MString CreateTemporaryDirectoryBaseName(const MString& Name) const;

  //! Return true only if Path resolves inside this test's randomized temporary root
  bool IsSafeTemporaryPath(const MString& Path, bool AllowRoot) const;

  //! Return true if Child is Parent or is contained below Parent
  bool IsPathContained(const std::filesystem::path& Parent, const std::filesystem::path& Child) const;

  //! Return true if two lines contain the same number of whitespace-separated tokens and each token matches
  //! Floating-point tokens use MString::AreNumbersNumericallyMatching() for comparison
  //! All other tokens must be identical
  //! For example, with MaximumLastDigitDifference = 2, "CT 0.518237 0.481763" matches "CT 0.518236 0.481764", but a changed "CT" token does not
  bool LinesMatchNumerically(const MString& TestLine, const MString& ReferenceLine,
                             unsigned int MaximumLastDigitDifference) const;

  // protected members:
 protected:


 // private members:
 private:
   //! Name of the unit test
   MString m_Name;
   //! Safe basename used for this test's randomized temporary root
   MString m_TemporaryBaseName;
   //! Passed tests
   unsigned int m_NumberOfPassedTests;
   //! Failed tests
   unsigned int m_NumberOfFailedTests;
   //! Randomized private temporary root, created lazily
   mutable MString m_TemporaryRootDirectory;
   //! The open temporary root with a lock on it as long as the root exists: the test driver does not remove the root of a running test (-1: none)
   mutable int m_TemporaryRootLock;
   //! The log directory
   MString m_LogDirectory;
   //! Serialize lazy temporary-root creation and guarded filesystem operations
   mutable recursive_mutex m_TemporaryPathMutex;


#ifdef ___CLING___
 public:
  ClassDef(MUnitTest, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
