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
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MUnitTest::~MUnitTest()
{
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::Evaluate(vector<double> Output, vector<double> Truth, MString NameOfTest)
{
  if (Output != Truth) {
    cout<<endl;
    cout<<"FAILED: "<<NameOfTest<<endl;
    cout<<"        Expected: ";
    for (auto v: Truth) cout<<v<<" "; 
    cout<<endl;
    cout<<"        Received: ";
    for (auto v: Output) cout<<v<<" "; 
    cout<<endl;
    
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::Evaluate(vector<bool> Output, vector<bool> Truth, MString NameOfTest)
{
  if (Output != Truth) {
    cout<<endl;
    cout<<"FAILED: "<<NameOfTest<<endl;
    cout<<"        Expected: ";
    for (auto v: Truth) cout<<v<<" "; 
    cout<<endl;
    cout<<"        Received: ";
    for (auto v: Output) cout<<v<<" "; 
    cout<<endl;
    
    return false;
  }
  
  return true;
}


// MUnitTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
