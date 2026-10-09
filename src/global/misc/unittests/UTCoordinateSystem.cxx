/*
 * UTCoordinateSystem.cxx
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


// Standard libs:
#include <sstream>
using namespace std;

// MEGAlib:
#include "MCoordinateSystem.h"
#include "MUnitTest.h"


//! Unit test class for MCoordinateSystem
class UTCoordinateSystem : public MUnitTest
{
public:
  UTCoordinateSystem() : MUnitTest("UTCoordinateSystem") {}
  virtual ~UTCoordinateSystem() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTCoordinateSystem::Run()
{
  bool Passed = true;

  {
    ostringstream Out;
    Out << MCoordinateSystem::c_Unknown;
    Passed = Evaluate("operator<<()", "unknown", "The unknown coordinate system prints the representative fallback label", MString(Out.str().c_str()), MString("unknown")) && Passed;
  }

  {
    ostringstream Out;
    Out << MCoordinateSystem::c_Galactic;
    Passed = Evaluate("operator<<()", "galactic", "The galactic coordinate system prints the representative galactic label", MString(Out.str().c_str()), MString("Galactic")) && Passed;
  }

  {
    ostringstream Out;
    Out << MCoordinateSystem::c_Spheric;
    Passed = Evaluate("operator<<()", "spheric", "The spherical coordinate system prints the representative spherical label", MString(Out.str().c_str()), MString("spheric")) && Passed;
  }

  {
    ostringstream Out;
    Out << MCoordinateSystem::c_Cartesian2D;
    Passed = Evaluate("operator<<()", "cartesian 2d", "The 2D cartesian coordinate system prints the representative 2D label", MString(Out.str().c_str()), MString("Cartesian 2D")) && Passed;
  }

  {
    ostringstream Out;
    Out << MCoordinateSystem::c_Cartesian3D;
    Passed = Evaluate("operator<<()", "cartesian 3d", "The 3D cartesian coordinate system prints the representative 3D label", MString(Out.str().c_str()), MString("Cartesian 3D")) && Passed;
  }

  {
    ostringstream Out;
    Out << static_cast<MCoordinateSystem>(99);
    Passed = Evaluate("operator<<()", "unknown enumerator", "An unrecognized coordinate system enumerator prints the representative fallback label", MString(Out.str().c_str()), MString("unknown")) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCoordinateSystem Test;
  return Test.Run() == true ? 0 : 1;
}
