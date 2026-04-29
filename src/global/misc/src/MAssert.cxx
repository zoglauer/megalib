/******************************************************************************
 *                                                                            *
 * MAssert.cpp    Version 1.0                                                *
 *                                                                            *
 * Copyright (C) by Andreas Zoglauer.                               *
 * All rights reserved.                                                       *
 *                                                                            *
 * Please see the file Licence.txt for further copyright information.         *
 *                                                                            *
 ******************************************************************************/

// Dame classes:
#include "MAssert.h"
#include "MStreams.h"

// External classes:
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
    __merr<<"Assertion failed in file "<<file<<" in function "<<function
          <<" at line "<<line<<":"<<endl;
  } else {
    __merr<<"Assertion failed in file "<<file<<" at line "<<line<<":"<<endl;
  }
  __merr<<"    Condition \""<<assertion<<"\" not fulfilled!   Aborting!"<<endl;
  abort();
}

/*
 * MAssert end...
 ******************************************************************************/
