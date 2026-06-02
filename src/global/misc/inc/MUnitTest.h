/*
 * MUnitTest.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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


////////////////////////////////////////////////////////////////////////////////


//! The base class for unit testing
class MUnitTest
{
  // public interface:
 public:
  //! Construct a named unit test
  //! For the name, use only ASCII letters, digits, '_' and '-' so the name can also serve as the temporary-directory basename, otherwise temporary files use c_FallbackTemporaryBaseName
  MUnitTest(const MString& Name);
  //! Default destuctor 
  virtual ~MUnitTest();
  
  //! Evaluate the success of this test run
  template <typename T1, typename T2> bool Evaluate(MString Function, T1 Input, MString Description, T2 Output, T2 Truth)
  {
    if (Output != Truth) {
      ostringstream ExpectedStream;
      ExpectedStream << setprecision(numeric_limits<long double>::max_digits10) << Truth;
      ostringstream OutputStream;
      OutputStream << setprecision(numeric_limits<long double>::max_digits10) << Output;
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
      ExpectedStream << setprecision(numeric_limits<long double>::max_digits10) << Truth << " +/- " << Tolerance;
      ostringstream OutputStream;
      OutputStream << setprecision(numeric_limits<long double>::max_digits10) << Output;
      RegisterFailure(Function, Input, Description, ExpectedStream.str(), OutputStream.str());
      return false;
    }

    RegisterSuccess();
    return true;
  }

  //! Evaluate the expected size of a container or collection
  template <typename T> bool EvaluateSize(MString Function, T Input, MString Description, size_t Output, size_t Truth)
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
      ExpectedStream << "exception of type " << typeid(TException).name();
      ostringstream OutputStream;
      OutputStream << "std::exception: " << Exception.what();
      RegisterFailure(Function, Input, Description, ExpectedStream.str(), OutputStream.str());
      return false;
    } catch (...) {
      ostringstream ExpectedStream;
      ExpectedStream << "exception of type " << typeid(TException).name();
      RegisterFailure(Function, Input, Description, ExpectedStream.str(), "unknown exception");
      return false;
    }

    ostringstream ExpectedStream;
    ExpectedStream << "exception of type " << typeid(TException).name();
    RegisterFailure(Function, Input, Description, ExpectedStream.str(), "no exception");
    return false;
  }
  
  //! Evaluate that two files are identical line by line; stops at and reports the first differing line
  bool EvaluateFilesIdentical(MString Function, MString Input, MString Description, const MString& GeneratedFile, const MString& ReferenceFile);

  //! Run the unit test
  virtual bool Run() = 0;

  //! Summarize the test run
  void Summarize();

  //! Fallback basename used when a unit-test name cannot safely name a temporary directory
  static const MString c_FallbackTemporaryBaseName;

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

  //! Remove and recreate a process-local temporary directory for this test
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
  //! Create this test's randomized private temporary root if necessary
  bool CreateTemporaryRootDirectory() const;

  //! Return the randomized private temporary root for this test
  MString GetTemporaryRootDirectory() const;

  //! Return true only for a plain child file or directory name without path components; see IsValidTemporaryBaseName() for the root-basename rule
  bool IsValidTemporaryPathName(const MString& Name, bool AllowEmpty = false) const;

  //! Return true only if Name is suitable as the unit-test-derived temporary directory basename; see IsValidTemporaryPathName() for child names
  bool IsValidTemporaryBaseName(const MString& Name) const;

  //! Return true only if Path resolves inside this test's randomized temporary root
  bool IsSafeTemporaryPath(const MString& Path, bool AllowRoot) const;

  //! Return true if Child is Parent or is contained below Parent
  bool IsPathContained(const std::filesystem::path& Parent, const std::filesystem::path& Child) const;

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
   //! Serialize lazy temporary-root creation and guarded filesystem operations
   mutable recursive_mutex m_TemporaryPathMutex;


#ifdef ___CLING___
 public:
  ClassDef(MUnitTest, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
