/*
 * TestDriverCalibration.h
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

#ifndef __TestDriverCalibration__
#define __TestDriverCalibration__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Class representing the program of the test driver which measures the speed of this machine: a fixed chain cosima -> revan -> mimrec, run twice
//! * The ratio to the time on the reference machine goes to ~/.testdrive.cfg, the test driver adds the machine ID (CPU, cores, MEGAlib version) and starts this program only if needed
class TestDriverCalibration : public MEndToEndTest
{
public:
  //! Default constructor
  TestDriverCalibration() : MEndToEndTest("TestDriverCalibration") {}
  //! Default destructor
  virtual ~TestDriverCalibration() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Run the fixed chain cosima -> revan -> mimrec in a scenario of the given name, return the time in seconds, or a negative value if one of the programs failed
  double RunChain(const MString& Name);
};

#endif


////////////////////////////////////////////////////////////////////////////////
