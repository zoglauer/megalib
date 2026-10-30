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


// Dame classes:
#include "MAssert.h"

// External classes:
#include <iostream>
#include <cstdlib>
using namespace std;

// Namespaces:
//using namespace Zogy;

/******************************************************************************/


/******************************************************************************
 * Print a pretty "Assertion failed" message
 * e.g. Assertion failed in file ZDObst.cpp at line 135:
 *          Condition "Apfel == Birne" not fulfilled!   Aborting!
 */
void /*Zogy::*/AssertionFailed(const char* assertion, const char* file,
                           unsigned int line, const char* function)
{
  if (function != 0) {
    cerr<<"Assertion failed in file "<<file<<" in function "<<function
        <<" at line "<<line<<":"<<endl;
  } else {
    cerr<<"Assertion failed in file "<<file<<" at line "<<line<<":"<<endl;
  }
  cerr<<"    Condition \""<<assertion<<"\" not fulfilled!   Aborting!"<<endl;
  abort();
}

/*
 * MAssert end...
 ******************************************************************************/

