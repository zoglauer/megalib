/*
 * MUnitTest.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MUnitTest.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MUnitTest)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MUnitTest::MUnitTest()
{
  m_NumberOfPassedTests = 0;
  m_NumberOfFailedTests = 0;
  
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MUnitTest::~MUnitTest()
{
}


////////////////////////////////////////////////////////////////////////////////


void MUnitTest::Summarize()
{
  cout<<"Passed tests: "<<m_NumberOfPassedTests<<endl;
  cout<<"Failed tests: "<<m_NumberOfFailedTests<<endl;
}


// MUnitTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
