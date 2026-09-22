/*
 * MAssert.cxx
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
