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
#include <iomanip>
#include <limits>
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


//! The base class for unit test
class MUnitTest
{
  // public interface:
 public:
  //! Default constructor
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
  
  //! Run the unit test
  virtual bool Run() = 0;
  
  //! Summarize the test run
  void Summarize();
  //! Run a child process with one argument and optionally redirect its output to a file
  static int RunChildProcess(const MString& Executable, const MString& Argument, const MString& OutputFileName = "");
  
  // protected methods:
 protected:
  //! Register a passed test
  void RegisterSuccess() { ++m_NumberOfPassedTests; }

  //! Return the unit test name
  const MString& GetName() const { return m_Name; }

  //! Disable the default MEGAlib output streams for expected noisy test paths
  void DisableDefaultStreams()
  {
    mout.Enable(false);
    mlog.Enable(false);
    mgui.Enable(false);
    __merr.Enable(false);
  }

  //! Re-enable the default MEGAlib output streams after a noisy test path
  void EnableDefaultStreams()
  {
    mout.Enable(true);
    mlog.Enable(true);
    mgui.Enable(true);
    __merr.Enable(true);
  }

  //! Register and report a failed test
  template <typename T> void RegisterFailure(MString Function, T Input, MString Description, MString Expected, MString Output)
  {
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



  // protected members:
 protected:


 // private members:
 private:
   //! Name of the unit test
   MString m_Name;
   //! Passed tests
   unsigned int m_NumberOfPassedTests;
   //! Failed tests
   unsigned int m_NumberOfFailedTests;


#ifdef ___CLING___
 public:
  ClassDef(MUnitTest, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
