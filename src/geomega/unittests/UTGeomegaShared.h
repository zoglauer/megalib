/*
 * UTGeomegaShared.h
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


#ifndef __UTGeomegaShared__
#define __UTGeomegaShared__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <csignal>
#include <cstdlib>
#include <functional>
#include <iostream>
using namespace std;

// POSIX libs: process control has no equivalent in the C++ standard library
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

// ROOT libs:
#include "TEnv.h"

// MEGAlib libs:
#include "MDGeometry.h"
#include "MString.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Base class of the geomega unit tests which read geometries written by the test
class UTGeomegaBase : public MUnitTest
{
public:
  //! Standard constructor with the name of the test
  UTGeomegaBase(const MString& Name) : MUnitTest(Name) {}
  //! Default destructor
  virtual ~UTGeomegaBase() {}

protected:
  //! Return the head of a geometry: name, version, surrounding sphere, materials, and a world volume of half length 100 cm
  MString GetGeometryHead() const
  {
    MString Head;
    Head += "Name UnitTest\n";
    Head += "Version 1.0\n";
    Head += "SurroundingSphere 50.0 0.0 0.0 0.0 50.0\n";
    Head += "Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo\n";
    Head += "Volume World\n";
    Head += "World.Material Vacuum\n";
    Head += "World.Shape BRIK 100.0 100.0 100.0\n";
    Head += "World.Visibility 0\n";
    Head += "World.Mother 0\n";
    return Head;
  }

  //! Write the content into a private file and scan it into the geometry - no cross section creation
  //! Attention: The streams are silenced during the scan, so do not rely on them to see why it failed
  bool ScanGeometry(MDGeometry& Geometry, const MString& Content, const MString& Name = "Fixture.geo.setup")
  {
    const MString FileName = GetTemporaryFileName(Name);
    if (WriteTextFile(FileName, Content) == false) {
      return false;
    }
    DisableDefaultStreams();
    bool Result = Geometry.ScanSetupFile(FileName, false, false, false);
    EnableDefaultStreams();
    return Result;
  }

  //! Return the vector rotated actively around x, then y, then z (angles in degrees), the reference of the rotation tests: it does not use MRotation
  MVector RotateActive(const MVector& V, double X, double Y, double Z) const
  {
    double A = X*c_Rad;
    double B = Y*c_Rad;
    double C = Z*c_Rad;

    // Around x:
    double X1 = V.X();
    double Y1 = cos(A)*V.Y() - sin(A)*V.Z();
    double Z1 = sin(A)*V.Y() + cos(A)*V.Z();

    // Around y:
    double X2 = cos(B)*X1 + sin(B)*Z1;
    double Y2 = Y1;
    double Z2 = -sin(B)*X1 + cos(B)*Z1;

    // Around z:
    return MVector(cos(C)*X2 - sin(C)*Y2, sin(C)*X2 + cos(C)*Y2, Z2);
  }
};


#endif


// UTGeomegaShared.h: the end...
////////////////////////////////////////////////////////////////////////////////
