/*
 * UTFixtureShared.h
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

#ifndef __UTFixtureShared__
#define __UTFixtureShared__


// The fixture files below $MEGALIB which the cosima unit tests need (geometries, data files)


// MEGAlib:
#include "MFile.h"
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Standard lib:
#include <cstdlib>


//! Return true if the fixture file (a path relative to $MEGALIB) exists, so the tests which need it can run
//! If $MEGALIB is not set, the tests which need the file are skipped with a notice and Passed stays as it is: a local build must not fail without the data
//! If $MEGALIB is set but the file is missing, the precondition is a failed check which is counted and reported
inline bool RequireFixture(MUnitTest& Test, bool& Passed, const MString& RelativePath)
{
  const char* Root = getenv("MEGALIB");
  if (Root == nullptr) {
    mout<<"MEGALIB is not set: the tests which need "<<RelativePath<<" are skipped"<<endl;
    return false;
  }
  const bool Exists = MFile::Exists(MString(Root) + "/" + RelativePath);
  Passed = Test.EvaluateTrue("Fixture", RelativePath, "The fixture file exists below $MEGALIB", Exists) && Passed;
  return Exists;
}


#endif


////////////////////////////////////////////////////////////////////////////////
