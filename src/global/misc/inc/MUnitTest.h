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
  
  //!
  bool Evaluate(vector<double> Output, vector<double> Truth, MString NameOfTest); 
  
  //!
  bool Evaluate(vector<bool> Output, vector<bool> Truth, MString NameOfTest); 
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MUnitTest, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
