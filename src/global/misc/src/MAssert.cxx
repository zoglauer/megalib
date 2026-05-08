/*
 * MAssert.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Include the header:
#include "MAssert.h"

// Standard libs:
#include <cstdlib>

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

using namespace std;


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MAssert)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MAssert::MAssert()
{
}

////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MAssert::~MAssert()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Print a pretty "Assertion failed" message
//! e.g. Assertion failed in file MFruits.cxx at line 135:
//!          Condition "Apple == Pear" not fulfilled!   Aborting!
[[noreturn]] void MAssert::AssertionFailed(const char* Assertion, const char* File, unsigned int Line, const char* Function)
{
  const char* SafeAssertion = (Assertion != nullptr) ? Assertion : "[unknown condition]";
  const char* SafeFile = (File != nullptr) ? File : "[unknown file name]";
  const char* SafeFunction = Function;

  if (SafeFunction != nullptr) {
    __merr<<"Assertion failed in file "<<SafeFile<<" in function "<<SafeFunction<<" at line "<<Line<<":"<<endl;
  } else {
    __merr<<"Assertion failed in file "<<SafeFile<<" at line "<<Line<<":"<<endl;
  }
  __merr<<"    Condition \""<<SafeAssertion<<"\" not fulfilled!   Aborting!"<<endl;
  
  abort();
}


// MAssert.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
