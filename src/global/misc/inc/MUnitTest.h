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
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MUnitTest.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The base class for unit test
class MUnitTest
{
  // public interface:
 public:
  //! Default constructor
  MUnitTest();
  //! Default destuctor 
  virtual ~MUnitTest();
  
  //! Evaluate the success of this test run
  template <typename T1, typename T2> bool Evaluate(MString Function, T1 Input, MString Description, T2 Output, T2 Truth)
  {
    if (Output != Truth) {
      cout<<endl;
      cout<<"FAILED: "<<Function<<"  <-- "<<Input<<endl;
      cout<<"   Description: "<<Description<<endl;
      cout<<"   Expected:    "<<Truth<<endl;
      cout<<"   Output:      "<<Output<<endl;
      cout<<endl;
      
      ++m_NumberOfFailedTests;
      return false;
    }
    
    ++ m_NumberOfPassedTests;
    return true;
  }
  
  //! Run the unit test
  virtual bool Run() = 0;
  
  //! Summarize the test run
  void Summarize();
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
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
