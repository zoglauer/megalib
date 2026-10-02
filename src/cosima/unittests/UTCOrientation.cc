/*
 * UTCOrientation.cc
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


// Cosima:
#include "MCOrientation.hh"
#include "UTCSDHitShared.h"

// MEGAlib:
#include "MExceptions.h"
#include "MStreams.h"
#include "MString.h"
#include "MTokenizer.h"
#include "MUnitTest.h"

// Geant4:
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

// Standard lib:
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
using namespace std;

// POSIX - the symbol lookup checks if a declared function is really defined in the library:


//! Class exposing the protected functions of MCOrientation to the unit test
class MCOrientationAccess : public MCOrientation
{
public:
  using MCOrientation::Read;
  using MCOrientation::InRange;
  using MCOrientation::FindClosestIndex;
  using MCOrientation::CalculateRotation;
};


//! Unit test class for MCOrientation
class UTCOrientation : public MUnitTest
{
public:
  //! Default constructor
  UTCOrientation() : MUnitTest("UTCOrientation") {}
  //! Default destructor
  virtual ~UTCOrientation() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default constructor and the getters
  bool TestDefaultConstruction();
  //! Test all error paths of Parse
  bool TestParseErrors();
  //! Test the local fixed orientation
  bool TestLocalFixed();
  //! Test the galactic fixed orientation with all token variants
  bool TestGalacticFixed();
  //! Test the range checks of the latitudes and thetas
  bool TestLatitudeRanges();
  //! Test reading galactic orientations from file without looping
  bool TestFileNoLoop();
  //! Test reading galactic orientations from file with looping
  bool TestFileLoop();
  //! Test the closest index search
  bool TestFindClosestIndex();
  //! Test the local orientations (OL) from file
  bool TestLocalFile();
  //! Test the Earth coordinates
  bool TestEarthCoordinates();
  //! Test the error paths of the file reading
  bool TestFileErrors();
  //! Test the start and stop time
  bool TestStartStopTime();
  //! Test that the orientation can be reused and copied
  bool TestReuseAndCopy();
  //! Test the stream operator and the declared functions
  bool TestStream();
  //! Test that orientations from slightly non-perpendicular axes are still rigid rotations
  bool TestNonPerpendicularAxes();
  //! Test the calculation of the rotation from the axis angles
  bool TestCalculateRotation();
  //! Test skewed frames with both axes tilted away from the coordinate planes (Galactic and local)
  bool TestTiltedSkewedFrames();
  //! Check one tilted and skewed frame: corrected angles, rotation, and forward and inverse transformations
  bool CheckTiltedSkewedFrame(bool Galactic);

  //! Parse a text without output
  bool ParseText(MCOrientation& Orientation, const MString& Text);
  //! Evaluate a Geant4 vector within the tolerance, forwards to the MVector version of MUnitTest
  using MUnitTest::EvaluateVectorNear;
  bool EvaluateVectorNear(const MString& Function, const MString& Input, const MString& Description, const G4ThreeVector& Output, const G4ThreeVector& Truth, double Tolerance);
  //! Write a file and parse it as orientation file
  bool ParseFile(MCOrientation& Orientation, const MString& Name, const MString& Content, const MString& Mode, const MString& System = "Galactic");
};


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestParseErrors() && Passed;
  Passed = TestLocalFixed() && Passed;
  Passed = TestGalacticFixed() && Passed;
  Passed = TestLatitudeRanges() && Passed;
  Passed = TestFileNoLoop() && Passed;
  Passed = TestFileLoop() && Passed;
  Passed = TestFindClosestIndex() && Passed;
  Passed = TestLocalFile() && Passed;
  Passed = TestEarthCoordinates() && Passed;
  Passed = TestFileErrors() && Passed;
  Passed = TestStartStopTime() && Passed;
  Passed = TestReuseAndCopy() && Passed;
  Passed = TestStream() && Passed;
  Passed = TestNonPerpendicularAxes() && Passed;
  Passed = TestCalculateRotation() && Passed;
  Passed = TestTiltedSkewedFrames() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::ParseText(MCOrientation& Orientation, const MString& Text)
{
  MTokenizer Tokenizer;
  Tokenizer.Analyse(Text, false);

  DisableDefaultStreams();
  bool Result = Orientation.Parse(Tokenizer);
  EnableDefaultStreams();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::EvaluateVectorNear(const MString& Function, const MString& Input, const MString& Description, const G4ThreeVector& Output, const G4ThreeVector& Truth, double Tolerance)
{
  return MUnitTest::EvaluateVectorNear(Function, Input, Description, MVector(Output.x(), Output.y(), Output.z()), MVector(Truth.x(), Truth.y(), Truth.z()), Tolerance);
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::ParseFile(MCOrientation& Orientation, const MString& Name, const MString& Content, const MString& Mode, const MString& System)
{
  const MString FileName = GetTemporaryFileName(Name);
  if (WriteTextFile(FileName, Content) == false) return false;

  return ParseText(Orientation, MString("Run Orientation ") + System + " File " + Mode + " " + FileName);
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestDefaultConstruction()
{
  bool Passed = true;

  MCOrientation O;
  Passed = EvaluateFalse("MCOrientation()", "default", "A new orientation is not oriented", O.IsOriented()) && Passed;
  Passed = EvaluateTrue("MCOrientation()", "default", "A new orientation is looping", O.IsLooping()) && Passed;
  Passed = Evaluate("MCOrientation()", "default", "A new orientation is in local coordinates", (int) O.GetCoordinateSystem(), (int) MCOrientationCoordinateSystem::c_Local) && Passed;
  Passed = EvaluateNear("GetStartTime()", "default", "A new orientation has no start time", O.GetStartTime(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetStopTime()", "default", "A new orientation has no stop time", O.GetStopTime(), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestParseErrors()
{
  bool Passed = true;

  MCOrientation O;
  Passed = EvaluateFalse("Parse()", "3 tokens", "At least 4 tokens are required", ParseText(O, "Run Orientation Local")) && Passed;
  Passed = EvaluateFalse("Parse()", "empty", "An empty text is rejected", ParseText(O, "")) && Passed;
  Passed = EvaluateFalse("Parse()", "unknown system", "An unknown coordinate system is rejected", ParseText(O, "Run Orientation Equatorial Fixed")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong case", "The coordinate system is case sensitive", ParseText(O, "Run Orientation local Fixed")) && Passed;
  Passed = EvaluateFalse("Parse()", "unknown mode", "An unknown mode is rejected", ParseText(O, "Run Orientation Local Rotating")) && Passed;
  Passed = EvaluateFalse("Parse()", "local fixed 5 tokens", "Local Fixed with additional tokens is rejected", ParseText(O, "Run Orientation Local Fixed 10")) && Passed;
  Passed = EvaluateFalse("Parse()", "local fixed 6 tokens", "Local Fixed with two additional tokens is rejected", ParseText(O, "Run Orientation Local Fixed 10 20")) && Passed;
  Passed = EvaluateFalse("Parse()", "galactic fixed 5 tokens", "Galactic Fixed with 5 tokens is rejected", ParseText(O, "Run Orientation Galactic Fixed 10")) && Passed;
  Passed = EvaluateFalse("Parse()", "galactic fixed 7 tokens", "Galactic Fixed with 7 tokens is rejected", ParseText(O, "Run Orientation Galactic Fixed 0 0 0")) && Passed;
  Passed = EvaluateFalse("Parse()", "galactic fixed 9 tokens", "Galactic Fixed with 9 tokens is rejected", ParseText(O, "Run Orientation Galactic Fixed 0 0 0 90 0")) && Passed;
  Passed = EvaluateFalse("Parse()", "file 5 tokens", "File without file name is rejected", ParseText(O, "Run Orientation Galactic File Loop")) && Passed;
  Passed = EvaluateFalse("Parse()", "file 7 tokens", "File with an additional token is rejected", ParseText(O, "Run Orientation Galactic File Loop a.ori b.ori")) && Passed;
  Passed = EvaluateFalse("Parse()", "file unknown loop keyword", "The 5th token must be Loop or NoLoop", ParseText(O, "Run Orientation Galactic File Repeat a.ori")) && Passed;
  Passed = EvaluateFalse("Parse()", "file missing", "A missing file is rejected", ParseText(O, MString("Run Orientation Galactic File Loop ") + GetTemporaryFileName("DoesNotExist.ori"))) && Passed;

  // Axes which are not at a right angle:
  Passed = EvaluateFalse("Parse()", "axes at 45 deg", "Galactic Fixed with axes not at a right angle is rejected", ParseText(O, "Run Orientation Galactic Fixed 0 0 0 45")) && Passed;
  Passed = EvaluateFalse("Parse()", "identical axes", "Galactic Fixed with identical axes is rejected", ParseText(O, "Run Orientation Galactic Fixed 0 30 0 30")) && Passed;

  // Every rejected text leaves an orientation without content:
  Passed = EvaluateFalse("Parse()", "after rejection", "A rejected text leaves the orientation not oriented", O.IsOriented()) && Passed;

  // A rejected text must not leave a state which makes the orientation functions crash (times without rotations):
  MCOrientationAccess Rejected;
  Passed = EvaluateFalse("Parse()", "rejected text", "The text is rejected", ParseText(Rejected, "Run Orientation Galactic Fixed 0 0 0 45")) && Passed;
  bool RunsClean = UTCSDHitShared::RunsCleanlyInChild([&]() {
    G4ThreeVector D(1, 0, 0);
    Rejected.OrientDirection(0.0, D);
    return true;
  });
  Passed = EvaluateTrue("OrientDirection()", "after rejected text", "Orienting a direction after a rejected text does not crash", RunsClean) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestLocalFixed()
{
  bool Passed = true;

  MCOrientation O;
  Passed = EvaluateTrue("Parse()", "Local Fixed", "Local Fixed is accepted", ParseText(O, "Run Orientation Local Fixed")) && Passed;
  Passed = EvaluateFalse("IsOriented()", "Local Fixed", "Local Fixed is the standard, thus not oriented", O.IsOriented()) && Passed;
  Passed = EvaluateTrue("IsLooping()", "Local Fixed", "Local Fixed is looping", O.IsLooping()) && Passed;
  Passed = Evaluate("GetCoordinateSystem()", "Local Fixed", "The coordinate system is local", (int) O.GetCoordinateSystem(), (int) MCOrientationCoordinateSystem::c_Local) && Passed;

  double XT = 0, XP = 0, ZT = 0, ZP = 0;
  Passed = EvaluateTrue("GetOrientation()", "Local Fixed", "The orientation can be retrieved at time zero", O.GetOrientation(0.0, XT, XP, ZT, ZP)) && Passed;
  Passed = EvaluateNear("GetOrientation()", "Local Fixed", "The x-axis theta is 90 deg", XT/deg, 90.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetOrientation()", "Local Fixed", "The x-axis phi is 0 deg", XP/deg, 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetOrientation()", "Local Fixed", "The z-axis theta is 0 deg", ZT/deg, 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetOrientation()", "Local Fixed", "The z-axis phi is 0 deg", ZP/deg, 0.0, 1e-9) && Passed;
  Passed = EvaluateTrue("GetOrientation()", "Local Fixed, late", "A fixed orientation is valid at any time", O.GetOrientation(1.0e6*s, XT, XP, ZT, ZP)) && Passed;

  // The local fixed orientation is the identity:
  const G4ThreeVector Direction(0.3, -0.5, 0.8);
  G4ThreeVector D = Direction;
  Passed = EvaluateTrue("OrientDirection()", "Local Fixed", "A direction can be oriented", O.OrientDirection(5*s, D)) && Passed;
  Passed = EvaluateVectorNear("OrientDirection()", "Local Fixed", "The local fixed orientation does not change a direction", D, Direction, 1e-9) && Passed;
  D = Direction;
  Passed = EvaluateTrue("OrientDirectionInvers()", "Local Fixed", "A direction can be oriented inversely", O.OrientDirectionInvers(5*s, D)) && Passed;
  Passed = EvaluateVectorNear("OrientDirectionInvers()", "Local Fixed", "The inverse local fixed orientation does not change a direction", D, Direction, 1e-9) && Passed;

  G4ThreeVector P(1*cm, 2*cm, 3*cm);
  D = Direction;
  Passed = EvaluateTrue("OrientPositionAndDirection()", "Local Fixed", "Position and direction can be oriented", O.OrientPositionAndDirection(5*s, P, D)) && Passed;
  Passed = EvaluateVectorNear("OrientPositionAndDirection()", "Local Fixed", "The position is not changed", P/cm, G4ThreeVector(1, 2, 3), 1e-9) && Passed;
  Passed = EvaluateVectorNear("OrientPositionAndDirection()", "Local Fixed", "The direction is not changed", D, Direction, 1e-9) && Passed;
  Passed = EvaluateTrue("OrientPositionAndDirectionInvers()", "Local Fixed", "Position and direction can be oriented inversely", O.OrientPositionAndDirectionInvers(5*s, P, D)) && Passed;
  Passed = EvaluateVectorNear("OrientPositionAndDirectionInvers()", "Local Fixed", "The position is not changed", P/cm, G4ThreeVector(1, 2, 3), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestGalacticFixed()
{
  bool Passed = true;

  // Default - 4 tokens: the pointing axis is the Galactic center (lat 0, long 0), the x-axis is at longitude 90 deg:
  {
    MCOrientation O;
    Passed = EvaluateTrue("Parse()", "Galactic Fixed", "Galactic Fixed is accepted", ParseText(O, "Run Orientation Galactic Fixed")) && Passed;
    Passed = EvaluateTrue("IsOriented()", "Galactic Fixed", "Galactic Fixed is oriented", O.IsOriented()) && Passed;
    Passed = EvaluateTrue("IsLooping()", "Galactic Fixed", "Galactic Fixed is looping", O.IsLooping()) && Passed;
    Passed = Evaluate("GetCoordinateSystem()", "Galactic Fixed", "The coordinate system is Galactic", (int) O.GetCoordinateSystem(), (int) MCOrientationCoordinateSystem::c_Galactic) && Passed;

    double XT = 0, XP = 0, ZT = 0, ZP = 0;
    Passed = EvaluateTrue("GetOrientation()", "Galactic Fixed", "The orientation can be retrieved", O.GetOrientation(0.0, XT, XP, ZT, ZP)) && Passed;
    Passed = EvaluateNear("GetOrientation()", "Galactic Fixed", "The x-axis latitude is 0 deg", XT/deg, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetOrientation()", "Galactic Fixed", "The x-axis longitude is 90 deg", XP/deg, 90.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetOrientation()", "Galactic Fixed", "The z-axis latitude is 0 deg", ZT/deg, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetOrientation()", "Galactic Fixed", "The z-axis longitude is 0 deg", ZP/deg, 0.0, 1e-9) && Passed;

    // The columns of the rotation are the x, y (left-handed: -Z cross X), and z axis:
    G4ThreeVector D(1, 0, 0);
    O.OrientDirection(0.0, D);
    Passed = EvaluateVectorNear("OrientDirection()", "x axis", "The local x-axis points to longitude 90 deg", D, G4ThreeVector(0, 1, 0), 1e-9) && Passed;
    D.set(0, 1, 0);
    O.OrientDirection(0.0, D);
    Passed = EvaluateVectorNear("OrientDirection()", "y axis", "The local y-axis is minus z cross x in the left-handed Galactic system", D, G4ThreeVector(0, 0, -1), 1e-9) && Passed;
    D.set(0, 0, 1);
    O.OrientDirection(0.0, D);
    Passed = EvaluateVectorNear("OrientDirection()", "z axis", "The local z-axis points to the Galactic center", D, G4ThreeVector(1, 0, 0), 1e-9) && Passed;
  }

  // 6 tokens: the pointing axis (z) is given, x is computed as an orthogonal axis:
  {
    MCOrientation O;
    Passed = EvaluateTrue("Parse()", "Galactic Fixed 30 60", "Galactic Fixed with the z-axis is accepted", ParseText(O, "Run Orientation Galactic Fixed 30 60")) && Passed;
    double XT = 0, XP = 0, ZT = 0, ZP = 0;
    Passed = EvaluateTrue("GetOrientation()", "Galactic Fixed 30 60", "The orientation can be retrieved", O.GetOrientation(0.0, XT, XP, ZT, ZP)) && Passed;
    Passed = EvaluateNear("GetOrientation()", "Galactic Fixed 30 60", "The z-axis latitude is the given one", ZT/deg, 30.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetOrientation()", "Galactic Fixed 30 60", "The z-axis longitude is the given one", ZP/deg, 60.0, 1e-9) && Passed;
    Passed = EvaluateTrue("GetOrientation()", "Galactic Fixed 30 60", "The computed x-axis latitude is within [-90, 90] deg", XT/deg >= -90.0 - 1e-9 && XT/deg <= 90.0 + 1e-9) && Passed;
    Passed = EvaluateTrue("GetOrientation()", "Galactic Fixed 30 60", "The computed x-axis longitude is within [0, 360) deg", XP/deg >= 0.0 && XP/deg < 360.0) && Passed;

    // The local z-axis points to the given position: theta = 90 deg + latitude
    G4ThreeVector D(0, 0, 1);
    O.OrientDirection(0.0, D);
    const double Theta = 90*deg + 30*deg;
    const double Phi = 60*deg;
    Passed = EvaluateVectorNear("OrientDirection()", "z axis", "The local z-axis is rotated to the given position", D, G4ThreeVector(sin(Theta)*cos(Phi), sin(Theta)*sin(Phi), cos(Theta)), 1e-9) && Passed;

    // The computed x-axis is orthogonal to the z-axis:
    G4ThreeVector X(1, 0, 0);
    O.OrientDirection(0.0, X);
    Passed = EvaluateNear("OrientDirection()", "x axis", "The rotated x-axis is orthogonal to the rotated z-axis", X.dot(D), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("OrientDirection()", "x axis", "The rotated x-axis has unit length", X.mag(), 1.0, 1e-9) && Passed;
  }

  // 8 tokens: x and z axis given (x: lat 0 long 0, z: lat 0 long 90):
  {
    MCOrientation O;
    Passed = EvaluateTrue("Parse()", "Galactic Fixed 0 0 0 90", "Galactic Fixed with both axes is accepted", ParseText(O, "Run Orientation Galactic Fixed 0 0 0 90")) && Passed;
    G4ThreeVector D(1, 0, 0);
    O.OrientDirection(0.0, D);
    Passed = EvaluateVectorNear("OrientDirection()", "x axis", "The x-axis points to longitude 0", D, G4ThreeVector(1, 0, 0), 1e-9) && Passed;
    D.set(0, 1, 0);
    O.OrientDirection(0.0, D);
    Passed = EvaluateVectorNear("OrientDirection()", "y axis", "The y-axis is minus z cross x", D, G4ThreeVector(0, 0, 1), 1e-9) && Passed;
    D.set(0, 0, 1);
    O.OrientDirection(0.0, D);
    Passed = EvaluateVectorNear("OrientDirection()", "z axis", "The z-axis points to longitude 90", D, G4ThreeVector(0, 1, 0), 1e-9) && Passed;

    // Inverse and round trip with a generic direction:
    const G4ThreeVector Direction(0.3, -0.5, 0.8);
    G4ThreeVector R = Direction;
    O.OrientDirection(0.0, R);
    Passed = EvaluateNear("OrientDirection()", "generic", "A rotation does not change the length of a direction", R.mag(), Direction.mag(), 1e-12) && Passed;
    Passed = EvaluateVectorNear("OrientDirection()", "generic", "The generic direction is rotated as the columns of the rotation", R, G4ThreeVector(0.3, 0.8, -0.5), 1e-9) && Passed;
    O.OrientDirectionInvers(0.0, R);
    Passed = EvaluateVectorNear("OrientDirectionInvers()", "round trip", "The inverse rotation restores the direction", R, Direction, 1e-9) && Passed;

    G4ThreeVector P(1*cm, 2*cm, 3*cm);
    G4ThreeVector Dir = Direction;
    O.OrientPositionAndDirection(0.0, P, Dir);
    Passed = EvaluateVectorNear("OrientPositionAndDirection()", "generic", "A fixed orientation has no translation, the position is rotated like a direction", P/cm, G4ThreeVector(1, 3, 2), 1e-9) && Passed;
    Passed = EvaluateVectorNear("OrientPositionAndDirection()", "generic", "The direction is rotated", Dir, G4ThreeVector(0.3, 0.8, -0.5), 1e-9) && Passed;
    O.OrientPositionAndDirectionInvers(0.0, P, Dir);
    Passed = EvaluateVectorNear("OrientPositionAndDirectionInvers()", "round trip", "The inverse restores the position", P/cm, G4ThreeVector(1, 2, 3), 1e-9) && Passed;
    Passed = EvaluateVectorNear("OrientPositionAndDirectionInvers()", "round trip", "The inverse restores the direction", Dir, Direction, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestLatitudeRanges()
{
  bool Passed = true;

  // Latitudes have to be within [-90, 90] deg - the messages of the code say so. The axes of the following texts are at a right angle:
  MCOrientation O;
  Passed = EvaluateFalse("Parse()", "x latitude 100 deg", "A latitude of 100 deg for the x-axis is rejected", ParseText(O, "Run Orientation Galactic Fixed 100 0 0 90")) && Passed;
  Passed = EvaluateFalse("Parse()", "x latitude -100 deg", "A latitude of -100 deg for the x-axis is rejected", ParseText(O, "Run Orientation Galactic Fixed -100 0 0 90")) && Passed;
  Passed = EvaluateFalse("Parse()", "z latitude 100 deg", "A latitude of 100 deg for the z-axis is rejected", ParseText(O, "Run Orientation Galactic Fixed 0 0 100 90")) && Passed;
  Passed = EvaluateFalse("Parse()", "z latitude -100 deg", "A latitude of -100 deg for the z-axis is rejected", ParseText(O, "Run Orientation Galactic Fixed 0 0 -100 90")) && Passed;
  Passed = EvaluateFalse("Parse()", "6 tokens, latitude 100 deg", "A latitude of 100 deg for the given z-axis is rejected", ParseText(O, "Run Orientation Galactic Fixed 100 0")) && Passed;

  // The limits are valid:
  Passed = EvaluateTrue("Parse()", "z latitude 90 deg", "A latitude of 90 deg is valid", ParseText(O, "Run Orientation Galactic Fixed 90 0")) && Passed;
  Passed = EvaluateTrue("Parse()", "z latitude -90 deg", "A latitude of -90 deg is valid", ParseText(O, "Run Orientation Galactic Fixed -90 0")) && Passed;

  // The same in files - Galactic (OG): latitude in [-90, 90] deg, local (OL): theta in [0, 180] deg
  MCOrientation FileOrientation;
  Passed = EvaluateFalse("Parse()", "OG latitude 100 deg", "A latitude of 100 deg in an OG line is rejected", ParseFile(FileOrientation, "OGLat100.ori", "OG 0 100 0 0 90\n", "Loop")) && Passed;
  Passed = EvaluateFalse("Parse()", "OG latitude -100 deg", "A latitude of -100 deg in an OG line is rejected", ParseFile(FileOrientation, "OGLatM100.ori", "OG 0 -100 0 0 90\n", "Loop")) && Passed;
  Passed = EvaluateFalse("Parse()", "OG z latitude 100 deg", "A latitude of 100 deg for the z-axis in an OG line is rejected", ParseFile(FileOrientation, "OGZLat100.ori", "OG 0 0 0 100 90\n", "Loop")) && Passed;
  Passed = EvaluateFalse("Parse()", "OL theta 190 deg", "A theta of 190 deg in an OL line is rejected", ParseFile(FileOrientation, "OLTheta190.ori", "OL 0 0 0 0 190 0 90 90\n", "Loop", "Local")) && Passed;
  Passed = EvaluateFalse("Parse()", "OL theta -10 deg", "A theta of -10 deg in an OL line is rejected", ParseFile(FileOrientation, "OLThetaM10.ori", "OL 0 0 0 0 -10 0 90 90\n", "Loop", "Local")) && Passed;
  Passed = EvaluateFalse("Parse()", "OL z theta 190 deg", "A theta of 190 deg for the z-axis in an OL line is rejected", ParseFile(FileOrientation, "OLZTheta190.ori", "OL 0 0 0 0 90 90 190 0\n", "Loop", "Local")) && Passed;

  // The line type has to match the selected coordinate system (the angle conventions differ):
  Passed = EvaluateFalse("Parse()", "OG line in Local file", "An OG line in a file read as Local orientation is rejected", ParseFile(FileOrientation, "OGInLocal.ori", "OG 0 0 0 0 90\n", "Loop", "Local")) && Passed;
  Passed = EvaluateFalse("Parse()", "OL line in Galactic file", "An OL line in a file read as Galactic orientation is rejected", ParseFile(FileOrientation, "OLInGalactic.ori", "OL 0 0 0 0 90 0 0 0\n", "Loop", "Galactic")) && Passed;
  Passed = EvaluateTrue("Parse()", "OG line in Galactic file", "An OG line in a file read as Galactic orientation is accepted", ParseFile(FileOrientation, "OGInGalactic.ori", "OG 0 0 0 0 90\n", "Loop", "Galactic")) && Passed;
  Passed = EvaluateTrue("Parse()", "OL line in Local file", "An OL line in a file read as Local orientation is accepted", ParseFile(FileOrientation, "OLInLocal.ori", "OL 0 0 0 0 90 0 0 0\n", "Loop", "Local")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestFileNoLoop()
{
  bool Passed = true;

  // Three entries with the x-axis and z-axis at right angles and the pointing (z) axis in the equatorial plane:
  const MString Content =
    "# Orientation test file\n"
    "OG 0 0 0 0 90\n"
    "OG 10 0 90 0 180\n"
    "OG 20 0 180 0 270\n";

  MCOrientationAccess O;
  Passed = EvaluateTrue("Parse()", "file no loop", "A galactic orientation file without looping can be parsed", ParseFile(O, "NoLoop.ori", Content, "NoLoop")) && Passed;
  Passed = EvaluateTrue("IsOriented()", "file no loop", "A file gives an orientation", O.IsOriented()) && Passed;
  Passed = EvaluateFalse("IsLooping()", "file no loop", "NoLoop is not looping", O.IsLooping()) && Passed;
  Passed = Evaluate("GetCoordinateSystem()", "file no loop", "The coordinate system is Galactic", (int) O.GetCoordinateSystem(), (int) MCOrientationCoordinateSystem::c_Galactic) && Passed;
  Passed = EvaluateNear("GetStartTime()", "file no loop", "The start time is the first time", O.GetStartTime()/s, 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetStopTime()", "file no loop", "The stop time is the last time", O.GetStopTime()/s, 20.0, 1e-9) && Passed;

  // Exact times return the entries:
  const double ExpectedXPhi[3] = { 0.0, 90.0, 180.0 };
  const double ExpectedZPhi[3] = { 90.0, 180.0, 270.0 };
  for (unsigned int i = 0; i < 3; ++i) {
    double XT = -1, XP = -1, ZT = -1, ZP = -1;
    MString Input = MString("t = ") + (10*i) + " s";
    Passed = EvaluateTrue("GetOrientation()", Input, "The orientation at an exact time is available", O.GetOrientation(10*i*s, XT, XP, ZT, ZP)) && Passed;
    Passed = EvaluateNear("GetOrientation()", Input, "The x-axis latitude is stored", XT/deg, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetOrientation()", Input, "The x-axis longitude is stored in rad", XP/deg, ExpectedXPhi[i], 1e-9) && Passed;
    Passed = EvaluateNear("GetOrientation()", Input, "The z-axis latitude is stored", ZT/deg, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetOrientation()", Input, "The z-axis longitude is stored in rad", ZP/deg, ExpectedZPhi[i], 1e-9) && Passed;
  }

  // The x-axis (1, 0, 0) of the local system is rotated into the x-axis of the entry:
  const G4ThreeVector ExpectedX[3] = { G4ThreeVector(1, 0, 0), G4ThreeVector(0, 1, 0), G4ThreeVector(-1, 0, 0) };
  for (unsigned int i = 0; i < 3; ++i) {
    G4ThreeVector D(1, 0, 0);
    Passed = EvaluateTrue("OrientDirection()", MString("t = ") + (10*i) + " s", "A direction can be oriented at an exact time", O.OrientDirection(10*i*s, D)) && Passed;
    Passed = EvaluateVectorNear("OrientDirection()", MString("t = ") + (10*i) + " s", "The local x-axis is rotated into the x-axis of the entry", D, ExpectedX[i], 1e-9) && Passed;
    Passed = EvaluateTrue("OrientDirectionInvers()", MString("t = ") + (10*i) + " s", "A direction can be oriented inversely at an exact time", O.OrientDirectionInvers(10*i*s, D)) && Passed;
    Passed = EvaluateVectorNear("OrientDirectionInvers()", MString("t = ") + (10*i) + " s", "The inverse restores the direction", D, G4ThreeVector(1, 0, 0), 1e-9) && Passed;
  }

  // Times outside of the range are rejected and do not change the arguments:
  double XT = -1, XP = -1, ZT = -1, ZP = -1;
  G4ThreeVector D(0.1, 0.2, 0.3);
  G4ThreeVector P(1*cm, 2*cm, 3*cm);
  Passed = EvaluateFalse("GetOrientation()", "t = -1 s", "Before the start no orientation is available", O.GetOrientation(-1*s, XT, XP, ZT, ZP)) && Passed;
  Passed = EvaluateFalse("GetOrientation()", "t = 21 s", "After the end no orientation is available", O.GetOrientation(21*s, XT, XP, ZT, ZP)) && Passed;
  Passed = EvaluateNear("GetOrientation()", "t = 21 s", "A rejected time does not change the output", XT, -1.0, 1e-12) && Passed;
  DisableDefaultStreams();
  bool Dir1 = O.OrientDirection(-1*s, D);
  bool Dir2 = O.OrientDirectionInvers(21*s, D);
  bool Pos1 = O.OrientPositionAndDirection(21*s, P, D);
  bool Pos2 = O.OrientPositionAndDirectionInvers(-1*s, P, D);
  EnableDefaultStreams();
  Passed = EvaluateFalse("OrientDirection()", "t = -1 s", "Before the start a direction cannot be oriented", Dir1) && Passed;
  Passed = EvaluateFalse("OrientDirectionInvers()", "t = 21 s", "After the end a direction cannot be oriented inversely", Dir2) && Passed;
  Passed = EvaluateFalse("OrientPositionAndDirection()", "t = 21 s", "After the end a position cannot be oriented", Pos1) && Passed;
  Passed = EvaluateFalse("OrientPositionAndDirectionInvers()", "t = -1 s", "Before the start a position cannot be oriented inversely", Pos2) && Passed;
  Passed = EvaluateVectorNear("OrientDirection()", "rejected times", "Rejected times do not change the direction", D, G4ThreeVector(0.1, 0.2, 0.3), 1e-9) && Passed;
  Passed = EvaluateVectorNear("OrientPositionAndDirection()", "rejected times", "Rejected times do not change the position", P/cm, G4ThreeVector(1, 2, 3), 1e-9) && Passed;
  Passed = EvaluateFalse("InRange()", "t = -1 s", "InRange is false before the start", O.InRange(-1*s)) && Passed;
  Passed = EvaluateTrue("InRange()", "t = 0", "InRange is true at the start", O.InRange(0.0)) && Passed;
  Passed = EvaluateTrue("InRange()", "t = 20 s", "InRange is true at the end", O.InRange(20*s)) && Passed;
  Passed = EvaluateFalse("InRange()", "t = 20.001 s", "InRange is false after the end", O.InRange(20.001*s)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestFileLoop()
{
  bool Passed = true;

  // First time zero - the periodic time is the time itself:
  {
    const MString Content =
      "OG 0 0 0 0 90\n"
      "OG 10 0 90 0 180\n"
      "OG 20 0 180 0 270\n";
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", "loop from 0", "A looping file can be parsed", ParseFile(O, "LoopFrom0.ori", Content, "Loop")) && Passed;
    Passed = EvaluateTrue("IsLooping()", "loop from 0", "Loop is looping", O.IsLooping()) && Passed;
    Passed = EvaluateNear("GetStartTime()", "loop from 0", "A looping orientation has no start time", O.GetStartTime(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetStopTime()", "loop from 0", "A looping orientation has no stop time", O.GetStopTime(), 0.0, 1e-12) && Passed;
    Passed = EvaluateTrue("InRange()", "loop from 0", "Any time is in range for a looping orientation", O.InRange(1.0e6*s)) && Passed;

    // Period 20 s:
    const double Times[5] = { 10, 30, 40, 50, 0 };
    const double ExpectedXPhi[5] = { 90, 90, 0, 90, 0 };
    for (unsigned int t = 0; t < 5; ++t) {
      double XT = 0, XP = 0, ZT = 0, ZP = 0;
      MString Input = MString("t = ") + Times[t] + " s";
      Passed = EvaluateTrue("GetOrientation()", Input, "A looping orientation is available at any time", O.GetOrientation(Times[t]*s, XT, XP, ZT, ZP)) && Passed;
      Passed = EvaluateNear("GetOrientation()", Input, "The looping orientation repeats with the period 20 s", XP/deg, ExpectedXPhi[t], 1e-9) && Passed;
    }
  }

  // First time not zero - the times in the file are absolute times, the period is (last - first) = 20 s:
  {
    const MString Content =
      "OG 10 0 0 0 90\n"
      "OG 20 0 90 0 180\n"
      "OG 30 0 180 0 270\n";
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", "loop from 10 s", "A looping file starting at 10 s can be parsed", ParseFile(O, "LoopFrom10.ori", Content, "Loop")) && Passed;

    // Exact entry times inside the first period:
    const double Times[6] = { 10, 20, 50, 60, 70, 90 };
    // Entry index at each time in a periodic sense: 10 -> e0, 20 -> e1, 50 -> (50-10) mod 20 = 0 -> e0, 60 -> 10 -> 20 s -> e1, 70 -> 0 -> e0, 90 -> 0 -> e0
    const double ExpectedXPhi[6] = { 0, 90, 0, 90, 0, 0 };
    for (unsigned int t = 0; t < 6; ++t) {
      double XT = 0, XP = 0, ZT = 0, ZP = 0;
      MString Input = MString("t = ") + Times[t] + " s";
      Passed = EvaluateTrue("GetOrientation()", Input, "A looping orientation is available at any time", O.GetOrientation(Times[t]*s, XT, XP, ZT, ZP)) && Passed;
      Passed = EvaluateNear("GetOrientation()", Input, "The looping orientation with a first time not zero returns the entry of that time", XP/deg, ExpectedXPhi[t], 1e-9) && Passed;
    }
  }

  // Times between the samples of a looping file (10, 20, 30 s; period 20 s): the closest entry is used, a tie goes to the earlier entry,
  // times beyond the last entry or before the first entry are wrapped into the period
  {
    const MString Content =
      "OG 10 0 0 0 90\n"
      "OG 20 0 90 0 180\n"
      "OG 30 0 180 0 270\n";
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", "loop between samples", "A looping file can be parsed", ParseFile(O, "LoopBetween.ori", Content, "Loop")) && Passed;

    struct Case { double Time; double ExpectedXPhi; const char* Description; };
    const Case Cases[] = {
      { 14, 0,   "14 s is closer to the entry at 10 s" },
      { 16, 90,  "16 s is closer to the entry at 20 s" },
      { 15, 0,   "15 s is exactly in the middle of 10 s and 20 s, the earlier entry is used" },
      { 25, 90,  "25 s is exactly in the middle of 20 s and 30 s, the earlier entry is used" },
      { 29, 180, "29 s is closer to the entry at 30 s" },
      { 30, 0,   "30 s is the end of the period and thus the entry at 10 s" },
      { 31, 0,   "31 s wraps to 11 s, which is closest to the entry at 10 s" },
      { 49, 180, "49 s wraps to 29 s, which is closest to the entry at 30 s" },
      { 50, 0,   "50 s wraps to 10 s" },
      { 5,  90,  "5 s wraps to 25 s, in the middle of 20 s and 30 s, the earlier entry is used" },
      { -1, 90,  "-1 s wraps to 19 s, which is closest to the entry at 20 s" }
    };
    for (const Case& C : Cases) {
      double XT = 0, XP = 0, ZT = 0, ZP = 0;
      MString Input = MString("t = ") + C.Time + " s";
      Passed = EvaluateTrue("GetOrientation()", Input, "A looping orientation is available at any time", O.GetOrientation(C.Time*s, XT, XP, ZT, ZP)) && Passed;
      Passed = EvaluateNear("GetOrientation()", Input, C.Description, XP/deg, C.ExpectedXPhi, 1e-9) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestFindClosestIndex()
{
  bool Passed = true;

  const MString Content =
    "OG 0 0 0 0 90\n"
    "OG 10 0 90 0 180\n"
    "OG 20 0 180 0 270\n";

  // No entries:
  {
    MCOrientationAccess Empty;
    Passed = EvaluateFalse("InRange()", "no entries", "Without entries no time is in range", Empty.InRange(0.0)) && Passed;
    Passed = EvaluateException<MExceptionEmptyArray>("FindClosestIndex()", "no entries, looping", "Without entries the search throws", [&]() { Empty.FindClosestIndex(0.0); }) && Passed;
    double XT, XP, ZT, ZP;
    Passed = EvaluateFalse("GetOrientation()", "no entries", "Without entries no orientation is available", Empty.GetOrientation(0.0, XT, XP, ZT, ZP)) && Passed;
  }

  // Not looping:
  {
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", "no loop", "The file can be parsed", ParseFile(O, "ClosestNoLoop.ori", Content, "NoLoop")) && Passed;
    Passed = Evaluate("FindClosestIndex()", "t = 0", "The first entry is found at its time", O.FindClosestIndex(0.0), 0U) && Passed;
    Passed = Evaluate("FindClosestIndex()", "t = 10 s", "The second entry is found at its time", O.FindClosestIndex(10*s), 1U) && Passed;
    Passed = Evaluate("FindClosestIndex()", "t = 20 s", "The last entry is found at its time", O.FindClosestIndex(20*s), 2U) && Passed;
    Passed = Evaluate("FindClosestIndex()", "t = 6 s", "6 s is closer to 10 s than to 0 s", O.FindClosestIndex(6*s), 1U) && Passed;
    Passed = Evaluate("FindClosestIndex()", "t = 16 s", "16 s is closer to 20 s than to 10 s", O.FindClosestIndex(16*s), 2U) && Passed;
    Passed = Evaluate("FindClosestIndex()", "t = 4 s", "4 s is closer to 0 s than to 10 s", O.FindClosestIndex(4*s), 0U) && Passed;
    Passed = Evaluate("FindClosestIndex()", "t = 14 s", "14 s is closer to 10 s than to 20 s", O.FindClosestIndex(14*s), 1U) && Passed;
    Passed = Evaluate("FindClosestIndex()", "t = 1 s", "1 s is closer to 0 s than to 10 s", O.FindClosestIndex(1*s), 0U) && Passed;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("FindClosestIndex()", "t = -1 s", "A time before the start throws", [&]() { O.FindClosestIndex(-1*s); }) && Passed;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("FindClosestIndex()", "t = 21 s", "A time after the end throws", [&]() { O.FindClosestIndex(21*s); }) && Passed;

    // The public interface returns the closest entry:
    double XT = 0, XP = 0, ZT = 0, ZP = 0;
    O.GetOrientation(4*s, XT, XP, ZT, ZP);
    Passed = EvaluateNear("GetOrientation()", "t = 4 s", "The orientation at 4 s is the one of the entry at 0 s", XP/deg, 0.0, 1e-9) && Passed;
    O.GetOrientation(14*s, XT, XP, ZT, ZP);
    Passed = EvaluateNear("GetOrientation()", "t = 14 s", "The orientation at 14 s is the one of the entry at 10 s", XP/deg, 90.0, 1e-9) && Passed;
  }

  // Looping with a single entry:
  {
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", "Galactic Fixed", "Galactic Fixed can be parsed", ParseText(O, "Run Orientation Galactic Fixed")) && Passed;
    Passed = Evaluate("FindClosestIndex()", "single entry", "A single entry is always found", O.FindClosestIndex(12345*s), 0U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestLocalFile()
{
  bool Passed = true;

  // Local orientation with identity axes (x along +x: theta 90 phi 0, z along +z: theta 0) and a translation:
  // By design (used in production to get correct images) the y-axis of an OL entry is -(z cross x), i.e. the frame is mirrored in y
  {
    MCOrientationAccess O;
    const MString FileName = GetTemporaryFileName("OLIdentity.ori");
    Passed = EvaluateTrue("WriteTextFile()", "OL identity", "The fixture can be written", WriteTextFile(FileName, "OL 0 10 20 30 90 0 0 0\n")) && Passed;
    Passed = EvaluateTrue("Parse()", "OL identity", "A local orientation file can be parsed", ParseText(O, MString("Run Orientation Local File Loop ") + FileName)) && Passed;
    Passed = EvaluateTrue("IsOriented()", "OL identity", "A file gives an orientation", O.IsOriented()) && Passed;

    G4ThreeVector P(1*cm, 2*cm, 3*cm);
    G4ThreeVector D(0, 1, 0);
    Passed = EvaluateTrue("OrientPositionAndDirection()", "OL identity", "Position and direction can be oriented", O.OrientPositionAndDirection(0.0, P, D)) && Passed;
    Passed = EvaluateVectorNear("OrientPositionAndDirection()", "OL identity", "Identity axes translate the position by (10, 20, 30) cm and mirror y", P/cm, G4ThreeVector(11, 18, 33), 1e-9) && Passed;
    Passed = EvaluateVectorNear("OrientPositionAndDirection()", "OL identity", "Identity axes mirror the y component of the direction", D, G4ThreeVector(0, -1, 0), 1e-9) && Passed;

    // The inverse restores the input:
    Passed = EvaluateTrue("OrientPositionAndDirectionInvers()", "OL identity", "Position and direction can be oriented inversely", O.OrientPositionAndDirectionInvers(0.0, P, D)) && Passed;
    Passed = EvaluateVectorNear("OrientPositionAndDirectionInvers()", "OL identity", "The inverse restores the position", P/cm, G4ThreeVector(1, 2, 3), 1e-9) && Passed;
    Passed = EvaluateVectorNear("OrientPositionAndDirectionInvers()", "OL identity", "The inverse restores the direction", D, G4ThreeVector(0, 1, 0), 1e-9) && Passed;
  }

  // Local orientation which maps the x-axis to +y (x: theta 90 phi 90, z: theta 0):
  // By design the y-axis is -(z cross x), i.e. the oriented frame is mirrored (triple product -1)
  {
    MCOrientationAccess O;
    const MString FileName = GetTemporaryFileName("OLRotated.ori");
    Passed = EvaluateTrue("WriteTextFile()", "OL rotated", "The fixture can be written", WriteTextFile(FileName, "OL 0 0 0 0 90 90 0 0\n")) && Passed;
    Passed = EvaluateTrue("Parse()", "OL rotated", "A local orientation file can be parsed", ParseText(O, MString("Run Orientation Local File Loop ") + FileName)) && Passed;

    G4ThreeVector X(1, 0, 0);
    G4ThreeVector Y(0, 1, 0);
    G4ThreeVector Z(0, 0, 1);
    O.OrientDirection(0.0, X);
    O.OrientDirection(0.0, Y);
    O.OrientDirection(0.0, Z);
    Passed = EvaluateVectorNear("OrientDirection()", "OL rotated x", "The x-axis is rotated to +y", X, G4ThreeVector(0, 1, 0), 1e-9) && Passed;
    Passed = EvaluateVectorNear("OrientDirection()", "OL rotated z", "The z-axis stays", Z, G4ThreeVector(0, 0, 1), 1e-9) && Passed;
    Passed = EvaluateVectorNear("OrientDirection()", "OL rotated y", "The y-axis is mapped to +x (mirrored frame)", Y, G4ThreeVector(1, 0, 0), 1e-9) && Passed;
    Passed = EvaluateNear("OrientDirection()", "OL rotated", "The oriented axes have the opposite handedness of the local system (triple product -1)", X.dot(Y.cross(Z)), -1.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestEarthCoordinates()
{
  bool Passed = true;

  // A file which has Earth coordinates only for some of the entries is ambiguous and rejected:
  {
    MCOrientationAccess Mixed;
    Passed = EvaluateFalse("Parse()", "mixed Earth data", "A file with Earth coordinates for only some entries is rejected",
                           ParseFile(Mixed, "EarthMixed.ori", "OG 0 0 0 0 90 500 20 30\nOG 10 0 90 0 180\n", "NoLoop")) && Passed;
    Passed = EvaluateFalse("Parse()", "mixed Earth data, first without", "A file with Earth coordinates only for the later entries is rejected",
                           ParseFile(Mixed, "EarthMixed2.ori", "OG 0 0 0 0 90\nOG 10 0 90 0 180 510 21 31\n", "NoLoop")) && Passed;
  }

  // Entries with 9 tokens contain altitude, latitude and longitude of the spacecraft:
  {
    const MString Content =
      "OG 0 0 0 0 90 500 20 30\n"
      "OG 10 0 90 0 180 510 21 31\n";
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", "Earth data", "A file with Earth coordinates can be parsed", ParseFile(O, "Earth.ori", Content, "NoLoop")) && Passed;

    double Alt = 0, Lat = 0, Long = 0;
    Passed = EvaluateTrue("GetEarthCoordinate()", "t = 0", "The Earth coordinates are available", O.GetEarthCoordinate(0.0, Alt, Lat, Long)) && Passed;
    Passed = EvaluateNear("GetEarthCoordinate()", "t = 0", "The altitude is in km", Alt/km, 500.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEarthCoordinate()", "t = 0", "The latitude is in deg", Lat/deg, 20.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEarthCoordinate()", "t = 0", "The longitude is in deg", Long/deg, 30.0, 1e-9) && Passed;
    Passed = EvaluateTrue("GetEarthCoordinate()", "t = 10 s", "The Earth coordinates of the second entry are available", O.GetEarthCoordinate(10*s, Alt, Lat, Long)) && Passed;
    Passed = EvaluateNear("GetEarthCoordinate()", "t = 10 s", "The altitude of the second entry is in km", Alt/km, 510.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEarthCoordinate()", "t = 10 s", "The latitude of the second entry is in deg", Lat/deg, 21.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEarthCoordinate()", "t = 10 s", "The longitude of the second entry is in deg", Long/deg, 31.0, 1e-9) && Passed;
    Passed = EvaluateFalse("GetEarthCoordinate()", "t = 11 s", "Outside of the range no Earth coordinates are available", O.GetEarthCoordinate(11*s, Alt, Lat, Long)) && Passed;

    // The pointing of the same file is unaffected:
    double XT, XP, ZT, ZP;
    Passed = EvaluateTrue("GetOrientation()", "Earth data", "A file with Earth coordinates also has an orientation", O.GetOrientation(10*s, XT, XP, ZT, ZP)) && Passed;
    Passed = EvaluateNear("GetOrientation()", "Earth data", "The pointing of the second entry is stored", XP/deg, 90.0, 1e-9) && Passed;
  }

  // Entries with only 6 tokens have no Earth coordinates - the function must say so and not read empty arrays (MCSource asks for them unconditionally):
  {
    const MString Content =
      "OG 0 0 0 0 90\n"
      "OG 10 0 90 0 180\n";
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", "no Earth data", "A file without Earth coordinates can be parsed", ParseFile(O, "NoEarth.ori", Content, "NoLoop")) && Passed;
    UTCSDHitShared::ChildOutcome Outcome = UTCSDHitShared::RunInChild([&]() {
      double Alt = 0, Lat = 0, Long = 0;
      // Correct: no Earth coordinates available
      return O.GetEarthCoordinate(0.0, Alt, Lat, Long) == false;
    });
    Passed = EvaluateTrue("GetEarthCoordinate()", "no Earth data", "Without Earth coordinates in the file the function returns false (and does not crash or return garbage)", Outcome == UTCSDHitShared::ChildOutcome::c_ReturnedTrue) && Passed;
  }

  // Fixed orientations have no Earth coordinates either:
  {
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", "Galactic Fixed", "Galactic Fixed can be parsed", ParseText(O, "Run Orientation Galactic Fixed")) && Passed;
    UTCSDHitShared::ChildOutcome Outcome = UTCSDHitShared::RunInChild([&]() {
      double Alt = 0, Lat = 0, Long = 0;
      return O.GetEarthCoordinate(0.0, Alt, Lat, Long) == false;
    });
    Passed = EvaluateTrue("GetEarthCoordinate()", "Galactic Fixed", "A fixed orientation has no Earth coordinates, the function returns false (and does not crash or return garbage)", Outcome == UTCSDHitShared::ChildOutcome::c_ReturnedTrue) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestFileErrors()
{
  bool Passed = true;

  MCOrientation O;

  // Number of tokens:
  Passed = EvaluateFalse("Parse()", "OG 7 tokens", "An OG line with 7 tokens is rejected", ParseFile(O, "OG7.ori", "OG 0 0 0 0 90 500\n", "Loop")) && Passed;
  Passed = EvaluateFalse("Parse()", "OG 10 tokens", "An OG line with 10 tokens is rejected", ParseFile(O, "OG10.ori", "OG 0 0 0 0 90 500 20 30 1\n", "Loop")) && Passed;
  Passed = EvaluateFalse("Parse()", "OL 8 tokens", "An OL line with 8 tokens is rejected", ParseFile(O, "OL8.ori", "OL 0 0 0 0 90 0 0\n", "Loop", "Local")) && Passed;
  Passed = EvaluateFalse("Parse()", "OL 10 tokens", "An OL line with 10 tokens is rejected", ParseFile(O, "OL10.ori", "OL 0 0 0 0 90 0 0 0 1\n", "Loop", "Local")) && Passed;

  // Axes not at a right angle:
  Passed = EvaluateFalse("Parse()", "OG axes at 45 deg", "An OG line with axes not at a right angle is rejected", ParseFile(O, "OGAngle.ori", "OG 0 0 0 0 45\n", "Loop")) && Passed;
  Passed = EvaluateFalse("Parse()", "OL axes at 45 deg", "An OL line with axes not at a right angle is rejected", ParseFile(O, "OLAngle.ori", "OL 0 0 0 0 90 0 45 0\n", "Loop", "Local")) && Passed;

  // Times have to increase:
  Passed = EvaluateFalse("Parse()", "decreasing times", "Decreasing times are rejected", ParseFile(O, "Decreasing.ori", "OG 10 0 0 0 90\nOG 5 0 90 0 180\n", "NoLoop")) && Passed;
  Passed = EvaluateFalse("Parse()", "equal times", "Equal times are rejected", ParseFile(O, "Equal.ori", "OG 10 0 0 0 90\nOG 10 0 90 0 180\n", "NoLoop")) && Passed;
  Passed = EvaluateTrue("Parse()", "increasing times", "Increasing times are accepted", ParseFile(O, "Increasing.ori", "OG 5 0 0 0 90\nOG 10 0 90 0 180\n", "NoLoop")) && Passed;

  // Empty file and lines which are not orientations:
  Passed = EvaluateTrue("Parse()", "empty file", "An empty file can be parsed", ParseFile(O, "Empty.ori", "\n", "Loop")) && Passed;
  Passed = EvaluateFalse("IsOriented()", "empty file", "An empty file gives no orientation", O.IsOriented()) && Passed;
  Passed = EvaluateTrue("Parse()", "other lines", "A file with lines which are no orientations can be parsed", ParseFile(O, "Other.ori", "XX 1 2 3\n# comment\nOG 0 0 0 0 90\n", "Loop")) && Passed;
  Passed = EvaluateTrue("IsOriented()", "other lines", "The orientation line is used, the others are ignored", O.IsOriented()) && Passed;

  // A failed parse must not leave a partial orientation behind - the first entry is fine, the second one is not:
  {
    MCOrientation Failed;
    Passed = EvaluateFalse("Parse()", "second entry invalid", "A file with an invalid second entry is rejected", ParseFile(Failed, "Partial.ori", "OG 0 0 0 0 90\nOG 10 0 0 0 45\n", "NoLoop")) && Passed;
    Passed = EvaluateFalse("IsOriented()", "second entry invalid", "A rejected file gives no orientation", Failed.IsOriented()) && Passed;
    double XT = 0, XP = 0, ZT = 0, ZP = 0;
    Passed = EvaluateFalse("GetOrientation()", "second entry invalid", "A rejected file leaves no orientation data behind", Failed.GetOrientation(5*s, XT, XP, ZT, ZP)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestStartStopTime()
{
  bool Passed = true;

  MCOrientation O;
  Passed = EvaluateTrue("Parse()", "no loop from 10 s", "The file can be parsed", ParseFile(O, "StartStop.ori", "OG 10 0 0 0 90\nOG 20 0 90 0 180\nOG 30 0 180 0 270\n", "NoLoop")) && Passed;
  Passed = EvaluateNear("GetStartTime()", "no loop from 10 s", "The start time is the first time", O.GetStartTime()/s, 10.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetStopTime()", "no loop from 10 s", "The stop time is the last time", O.GetStopTime()/s, 30.0, 1e-9) && Passed;

  Passed = EvaluateTrue("Parse()", "loop from 10 s", "The looping file can be parsed", ParseFile(O, "StartStopLoop.ori", "OG 10 0 0 0 90\nOG 20 0 90 0 180\nOG 30 0 180 0 270\n", "Loop")) && Passed;
  Passed = EvaluateNear("GetStartTime()", "loop from 10 s", "A looping orientation has no start time", O.GetStartTime(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetStopTime()", "loop from 10 s", "A looping orientation has no stop time", O.GetStopTime(), 0.0, 1e-12) && Passed;

  Passed = EvaluateTrue("Parse()", "Galactic Fixed", "Galactic Fixed can be parsed", ParseText(O, "Run Orientation Galactic Fixed")) && Passed;
  Passed = EvaluateNear("GetStartTime()", "Galactic Fixed", "A fixed orientation has no start time", O.GetStartTime(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetStopTime()", "Galactic Fixed", "A fixed orientation has no stop time", O.GetStopTime(), 0.0, 1e-12) && Passed;

  Passed = EvaluateTrue("Parse()", "Local Fixed", "Local Fixed can be parsed", ParseText(O, "Run Orientation Local Fixed")) && Passed;
  Passed = EvaluateNear("GetStartTime()", "Local Fixed", "A not oriented Local Fixed has no start time", O.GetStartTime(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetStopTime()", "Local Fixed", "A not oriented Local Fixed has no stop time", O.GetStopTime(), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestReuseAndCopy()
{
  bool Passed = true;

  // The same object is reused - every parse starts from scratch:
  MCOrientationAccess O;
  Passed = EvaluateTrue("Parse()", "file", "A file can be parsed", ParseFile(O, "Reuse.ori", "OG 10 0 0 0 90\nOG 20 0 90 0 180\n", "NoLoop")) && Passed;
  Passed = EvaluateFalse("IsLooping()", "file", "The file is not looping", O.IsLooping()) && Passed;
  Passed = EvaluateNear("GetStopTime()", "file", "The stop time is the one of the file", O.GetStopTime()/s, 20.0, 1e-9) && Passed;

  Passed = EvaluateTrue("Parse()", "Local Fixed", "The same object can parse Local Fixed afterwards", ParseText(O, "Run Orientation Local Fixed")) && Passed;
  Passed = EvaluateTrue("IsLooping()", "Local Fixed after file", "Local Fixed is looping again", O.IsLooping()) && Passed;
  Passed = EvaluateFalse("IsOriented()", "Local Fixed after file", "Local Fixed is not oriented again", O.IsOriented()) && Passed;
  Passed = EvaluateNear("GetStopTime()", "Local Fixed after file", "The stop time of the previous file is forgotten", O.GetStopTime(), 0.0, 1e-12) && Passed;
  double XT, XP, ZT, ZP;
  Passed = EvaluateTrue("GetOrientation()", "Local Fixed after file", "At 15 s only the fixed orientation remains", O.GetOrientation(15*s, XT, XP, ZT, ZP)) && Passed;
  Passed = EvaluateNear("GetOrientation()", "Local Fixed after file", "The fixed orientation replaces the file entries", XT/deg, 90.0, 1e-9) && Passed;

  // A failed parse after a successful one:
  Passed = EvaluateTrue("Parse()", "Galactic Fixed", "Galactic Fixed can be parsed", ParseText(O, "Run Orientation Galactic Fixed")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid after valid", "An invalid text is rejected", ParseText(O, "Run Orientation Galactic Fixed 0 0 0 45")) && Passed;
  Passed = EvaluateFalse("IsOriented()", "invalid after valid", "A rejected text does not keep the previous orientation", O.IsOriented()) && Passed;

  // Clear:
  Passed = EvaluateTrue("Parse()", "Galactic Fixed", "Galactic Fixed can be parsed", ParseText(O, "Run Orientation Galactic Fixed")) && Passed;
  O.Clear();
  Passed = EvaluateFalse("Clear()", "after Galactic Fixed", "Clear removes the orientation", O.IsOriented()) && Passed;
  Passed = EvaluateTrue("Clear()", "after Galactic Fixed", "Clear restores looping", O.IsLooping()) && Passed;
  Passed = Evaluate("Clear()", "after Galactic Fixed", "Clear restores the local coordinate system", (int) O.GetCoordinateSystem(), (int) MCOrientationCoordinateSystem::c_Local) && Passed;
  Passed = EvaluateFalse("GetOrientation()", "after Clear", "Clear removes all entries", O.GetOrientation(0.0, XT, XP, ZT, ZP)) && Passed;

  // The run keeps its orientations by value:
  MCOrientation Original;
  Passed = EvaluateTrue("Parse()", "original", "The original can be parsed", ParseFile(Original, "Copy.ori", "OG 10 0 0 0 90\nOG 20 0 90 0 180\n", "NoLoop")) && Passed;
  MCOrientation Copy(Original);
  Passed = EvaluateTrue("MCOrientation(const MCOrientation&)", "copy", "The copy is oriented", Copy.IsOriented()) && Passed;
  Passed = EvaluateFalse("MCOrientation(const MCOrientation&)", "copy", "The copy is not looping", Copy.IsLooping()) && Passed;
  Passed = EvaluateNear("MCOrientation(const MCOrientation&)", "copy", "The copy has the same stop time", Copy.GetStopTime()/s, 20.0, 1e-9) && Passed;
  G4ThreeVector D(1, 0, 0);
  Passed = EvaluateTrue("MCOrientation(const MCOrientation&)", "copy", "The copy can orient", Copy.OrientDirection(20*s, D)) && Passed;
  Passed = EvaluateVectorNear("MCOrientation(const MCOrientation&)", "copy", "The copy orients like the original", D, G4ThreeVector(0, 1, 0), 1e-9) && Passed;
  Original.Clear();
  Passed = EvaluateTrue("MCOrientation(const MCOrientation&)", "independence", "Clearing the original does not change the copy", Copy.IsOriented()) && Passed;
  MCOrientation Assigned;
  Assigned = Copy;
  Passed = EvaluateTrue("operator=", "assignment", "The assigned orientation is oriented", Assigned.IsOriented()) && Passed;
  Passed = EvaluateNear("operator=", "assignment", "The assigned orientation has the same stop time", Assigned.GetStopTime()/s, 20.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestNonPerpendicularAxes()
{
  bool Passed = true;

  // The x and z axes are accepted up to 0.001 rad from a right angle. The resulting transformation has to stay a rigid rotation:
  // lengths and angles are preserved and the inverse restores the input.
  auto CheckRigid = [&](const MString& Name, MCOrientation& O, const G4ThreeVector& Offset, bool Galactic) {
    const double Tolerance = 1e-9;
    const G4ThreeVector Directions[] = { G4ThreeVector(1, 0, 0), G4ThreeVector(0, 1, 0), G4ThreeVector(0, 0, 1),
                                         G4ThreeVector(1, 1, 1).unit(), G4ThreeVector(0.3, -0.5, 0.8).unit() };
    for (const G4ThreeVector& Direction : Directions) {
      MString Input = Name + ", direction (" + Direction.x() + ", " + Direction.y() + ", " + Direction.z() + ")";

      G4ThreeVector Position = Offset;
      G4ThreeVector Oriented = Direction;
      Passed = EvaluateTrue("OrientPositionAndDirection()", Input, "A direction can be oriented", O.OrientPositionAndDirection(0.0, Position, Oriented)) && Passed;
      Passed = EvaluateNear("OrientPositionAndDirection()", Input, "A rotation keeps the length of a unit direction", Oriented.mag(), 1.0, Tolerance) && Passed;

      G4ThreeVector Restored = Oriented;
      G4ThreeVector RestoredPosition = Position;
      Passed = EvaluateTrue("OrientPositionAndDirectionInvers()", Input, "A direction can be oriented inversely", O.OrientPositionAndDirectionInvers(0.0, RestoredPosition, Restored)) && Passed;
      Passed = EvaluateNear("OrientPositionAndDirectionInvers()", Input, "The inverse restores the direction", (Restored - Direction).mag(), 0.0, Tolerance) && Passed;
      Passed = EvaluateNear("OrientPositionAndDirectionInvers()", Input, "The inverse restores the position", (RestoredPosition - Offset).mag()/cm, 0.0, Tolerance) && Passed;
    }

    // Perpendicular axes stay perpendicular:
    G4ThreeVector X(1, 0, 0);
    G4ThreeVector Y(0, 1, 0);
    G4ThreeVector Z(0, 0, 1);
    O.OrientDirection(0.0, X);
    O.OrientDirection(0.0, Y);
    O.OrientDirection(0.0, Z);
    Passed = EvaluateNear("OrientDirection()", Name, "A rotation keeps x and y perpendicular", X.dot(Y), 0.0, Tolerance) && Passed;
    Passed = EvaluateNear("OrientDirection()", Name, "A rotation keeps x and z perpendicular", X.dot(Z), 0.0, Tolerance) && Passed;
    Passed = EvaluateNear("OrientDirection()", Name, "A rotation keeps y and z perpendicular", Y.dot(Z), 0.0, Tolerance) && Passed;

    // The reported pointing (it is written into the events) has to be the one of the transformation: X and Z are the oriented x- and z-axis
    double XTheta = 0, XPhi = 0, ZTheta = 0, ZPhi = 0;
    Passed = EvaluateTrue("GetOrientation()", Name, "The orientation is available", O.GetOrientation(0.0, XTheta, XPhi, ZTheta, ZPhi)) && Passed;
    const double Shift = Galactic ? c_Pi/2 : 0.0;
    G4ThreeVector ReportedX(sin(Shift + XTheta)*cos(XPhi), sin(Shift + XTheta)*sin(XPhi), cos(Shift + XTheta));
    G4ThreeVector ReportedZ(sin(Shift + ZTheta)*cos(ZPhi), sin(Shift + ZTheta)*sin(ZPhi), cos(Shift + ZTheta));
    Passed = EvaluateNear("GetOrientation()", Name, "The reported x-axis is the transformed x-axis", (ReportedX - X).mag(), 0.0, Tolerance) && Passed;
    Passed = EvaluateNear("GetOrientation()", Name, "The reported z-axis is the transformed z-axis", (ReportedZ - Z).mag(), 0.0, Tolerance) && Passed;
  };

  // Galactic Fixed: x at latitude 0 longitude 0, z at latitude 0 and longitude 89.95 deg / 90.05 deg, i.e. 0.05 deg from a right angle:
  for (const char* Longitude : { "89.95", "90.05" }) {
    MCOrientation O;
    MString Text = MString("Run Orientation Galactic Fixed 0 0 0 ") + Longitude;
    Passed = EvaluateTrue("Parse()", Text, "Axes 0.05 deg from a right angle are accepted", ParseText(O, Text)) && Passed;
    CheckRigid(Text, O, G4ThreeVector(0, 0, 0), true);
  }

  // Galactic file (OG):
  for (const char* Longitude : { "89.95", "90.05" }) {
    MCOrientation O;
    MString Text = MString("OG line, z longitude ") + Longitude;
    Passed = EvaluateTrue("Parse()", Text, "Axes 0.05 deg from a right angle are accepted", ParseFile(O, MString("OGSkew") + Longitude + ".ori", MString("OG 0 0 0 0 ") + Longitude + "\n", "Loop", "Galactic")) && Passed;
    CheckRigid(Text, O, G4ThreeVector(0, 0, 0), true);
  }

  // Local file (OL): x along +x (theta 90, phi 0), z at theta 0.05 deg and phi 0 / 180 deg, i.e. 0.05 deg from a right angle on either side, with a translation:
  for (const char* ZPhi : { "0", "180" }) {
    MCOrientation O;
    MString Text = MString("OL line, z theta 0.05 phi ") + ZPhi;
    Passed = EvaluateTrue("Parse()", Text, "Axes 0.05 deg from a right angle are accepted", ParseFile(O, MString("OLSkew") + ZPhi + ".ori", MString("OL 0 10 20 30 90 0 0.05 ") + ZPhi + "\n", "Loop", "Local")) && Passed;
    CheckRigid(Text, O, G4ThreeVector(1*cm, 2*cm, 3*cm), false);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestTiltedSkewedFrames()
{
  bool Passed = true;

  Passed = CheckTiltedSkewedFrame(true) && Passed;
  Passed = CheckTiltedSkewedFrame(false) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::CheckTiltedSkewedFrame(bool Galactic)
{
  bool Passed = true;

  // A frame in which neither axis lies in a coordinate plane and the axes are 0.04 deg (within the accepted 0.001 rad) from a right angle:
  //   z is a generic direction, x0 is perpendicular to z (rolled by 55 deg around z), and the input x is x0 tilted by 0.04 deg towards z:
  //   x_in = cos(d) x0 + sin(d) z.
  // The projection of x_in perpendicular to z is x0 exactly (the part along z is removed), thus the expected corrected x-axis is x0,
  // the expected rotation has the columns x0, y = -(z cross x0) (the frame is mirrored), and z.
  const MString Name = Galactic ? "tilted Galactic frame" : "tilted local frame";
  const double Offset = Galactic ? c_Pi/2 : 0.0;           // Galactic angles are latitudes, local angles are theta angles
  const double ZFirst = Galactic ? -25*deg : 35*deg;       // z: latitude / theta
  const double ZSecond = Galactic ? 130*deg : 70*deg;      // z: longitude / phi
  const double Tolerance = 1e-12;

  auto ToVector = [Offset](double First, double Second) {
    return MVector(sin(Offset + First)*cos(Second), sin(Offset + First)*sin(Second), cos(Offset + First));
  };
  auto ToAngles = [Offset](const MVector& V, double& First, double& Second) {
    First = acos(V.Z()/V.Mag()) - Offset;
    Second = atan2(V.Y(), V.X());
  };
  auto ToG4 = [](const MVector& V) { return G4ThreeVector(V.X(), V.Y(), V.Z()); };

  const MVector Z = ToVector(ZFirst, ZSecond);
  const MVector U = Z.Cross(MVector(0.0, 0.0, 1.0)).Unit();
  const MVector V = Z.Cross(U);
  const double Roll = 55*deg;
  const double Skew = 0.04*deg;
  const MVector X0 = U*cos(Roll) + V*sin(Roll);
  const MVector XIn = X0*cos(Skew) + Z*sin(Skew);
  MVector Y = Z.Cross(X0);
  Y *= -1;

  double XInFirst = 0, XInSecond = 0, X0First = 0, X0Second = 0;
  ToAngles(XIn, XInFirst, XInSecond);
  ToAngles(X0, X0First, X0Second);
  const MRotation Expected(X0.X(), Y.X(), Z.X(),
                           X0.Y(), Y.Y(), Z.Y(),
                           X0.Z(), Y.Z(), Z.Z());

  // The axes are really skewed and the frame is generic:
  Passed = EvaluateNear("CheckTiltedSkewedFrame()", Name, "The input axes are 0.04 deg from a right angle", XIn.Angle(Z)/deg, 89.96, 1e-9) && Passed;
  Passed = EvaluateTrue("CheckTiltedSkewedFrame()", Name, "No axis lies in a coordinate plane", fabs(Z.X()) > 0.1 && fabs(Z.Y()) > 0.1 && fabs(Z.Z()) > 0.1 && fabs(X0.X()) > 0.1 && fabs(X0.Y()) > 0.1 && fabs(X0.Z()) > 0.1) && Passed;

  // The calculation of the rotation: corrected x angles, kept z angles, and the rotation
  {
    MCOrientationAccess O;
    Passed = EvaluateTrue("Parse()", Name, "The orientation can be parsed", ParseText(O, Galactic ? "Run Orientation Galactic Fixed" : "Run Orientation Local Fixed")) && Passed;
    double XFirst = XInFirst, XSecond = XInSecond, ZFirstAngle = ZFirst, ZSecondAngle = ZSecond;
    MRotation Rotation;
    Passed = EvaluateTrue("CalculateRotation()", Name, "The skewed axes are accepted", O.CalculateRotation(XFirst, XSecond, ZFirstAngle, ZSecondAngle, Rotation)) && Passed;
    Passed = EvaluateRotationNear("CalculateRotation()", Name, "The rotation has the columns x0, -(z cross x0), z", Rotation, Expected, Tolerance) && Passed;
    Passed = EvaluateNear("CalculateRotation()", Name, "The corrected x angle (latitude or theta)", XFirst, X0First, Tolerance) && Passed;
    Passed = EvaluateNear("CalculateRotation()", Name, "The corrected x angle (longitude or phi)", XSecond, X0Second, Tolerance) && Passed;
    Passed = EvaluateNear("CalculateRotation()", Name, "The z angle (latitude or theta) is kept exactly", ZFirstAngle, ZFirst, 0.0) && Passed;
    Passed = EvaluateNear("CalculateRotation()", Name, "The z angle (longitude or phi) is kept exactly", ZSecondAngle, ZSecond, 0.0) && Passed;
  }

  // The same frame read from a file: reported pointing, forward and inverse transformation
  const MVector Translation = Galactic ? MVector(0.0, 0.0, 0.0) : MVector(10.0, -20.0, 30.0);
  ostringstream Content;
  Content<<setprecision(17);
  if (Galactic) {
    Content<<"OG 0 "<<XInFirst/deg<<" "<<XInSecond/deg<<" "<<ZFirst/deg<<" "<<ZSecond/deg<<"\n";
  } else {
    Content<<"OL 0 "<<Translation.X()<<" "<<Translation.Y()<<" "<<Translation.Z()<<" "<<XInFirst/deg<<" "<<XInSecond/deg<<" "<<ZFirst/deg<<" "<<ZSecond/deg<<"\n";
  }
  MCOrientation O;
  Passed = EvaluateTrue("Parse()", Name, "The file with the skewed axes can be parsed", ParseFile(O, Galactic ? "TiltedGalactic.ori" : "TiltedLocal.ori", MString(Content.str().c_str()), "Loop", Galactic ? "Galactic" : "Local")) && Passed;

  double XFirst = 0, XSecond = 0, ZFirstReported = 0, ZSecondReported = 0;
  Passed = EvaluateTrue("GetOrientation()", Name, "The orientation is available", O.GetOrientation(0.0, XFirst, XSecond, ZFirstReported, ZSecondReported)) && Passed;
  Passed = EvaluateNear("GetOrientation()", Name, "The reported x angle (latitude or theta) is the corrected one", XFirst, X0First, Tolerance) && Passed;
  Passed = EvaluateNear("GetOrientation()", Name, "The reported x angle (longitude or phi) is the corrected one", XSecond, X0Second, Tolerance) && Passed;
  Passed = EvaluateNear("GetOrientation()", Name, "The reported z angle (latitude or theta)", ZFirstReported, ZFirst, Tolerance) && Passed;
  Passed = EvaluateNear("GetOrientation()", Name, "The reported z angle (longitude or phi)", ZSecondReported, ZSecond, Tolerance) && Passed;

  // The columns of the rotation are the images of the unit vectors, a rotation is applied as R*v (+ translation)
  auto Apply = [&](const MVector& Vector) { return X0*Vector.X() + Y*Vector.Y() + Z*Vector.Z(); };
  const MVector Direction = MVector(0.3, -0.5, 0.8).Unit();
  const MVector Position(1.5, -2.0, 0.7);
  const MVector ExpectedDirection = Apply(Direction);
  const MVector ExpectedPosition = Apply(Position) + Translation;

  G4ThreeVector OrientedDirection = ToG4(Direction);
  Passed = EvaluateTrue("OrientDirection()", Name, "A direction can be oriented", O.OrientDirection(0.0, OrientedDirection)) && Passed;
  Passed = EvaluateVectorNear("OrientDirection()", Name, "The direction is rotated by x0, -(z cross x0), z", OrientedDirection, ToG4(ExpectedDirection), Tolerance) && Passed;
  Passed = EvaluateNear("OrientDirection()", Name, "The length of the direction is preserved", OrientedDirection.mag(), 1.0, Tolerance) && Passed;
  Passed = EvaluateTrue("OrientDirectionInvers()", Name, "A direction can be oriented inversely", O.OrientDirectionInvers(0.0, OrientedDirection)) && Passed;
  Passed = EvaluateVectorNear("OrientDirectionInvers()", Name, "The inverse restores the direction", OrientedDirection, ToG4(Direction), Tolerance) && Passed;

  G4ThreeVector OrientedPosition = ToG4(Position)*cm;
  G4ThreeVector OrientedPositionDirection = ToG4(Direction);
  Passed = EvaluateTrue("OrientPositionAndDirection()", Name, "A position and a direction can be oriented", O.OrientPositionAndDirection(0.0, OrientedPosition, OrientedPositionDirection)) && Passed;
  Passed = EvaluateVectorNear("OrientPositionAndDirection()", Name, "The position is rotated and translated", OrientedPosition/cm, ToG4(ExpectedPosition), Tolerance) && Passed;
  Passed = EvaluateVectorNear("OrientPositionAndDirection()", Name, "The direction is rotated", OrientedPositionDirection, ToG4(ExpectedDirection), Tolerance) && Passed;
  Passed = EvaluateNear("OrientPositionAndDirection()", Name, "The distance of the position from the translation is preserved", ((OrientedPosition/cm) - ToG4(Translation)).mag(), Position.Mag(), Tolerance) && Passed;
  Passed = EvaluateTrue("OrientPositionAndDirectionInvers()", Name, "A position and a direction can be oriented inversely", O.OrientPositionAndDirectionInvers(0.0, OrientedPosition, OrientedPositionDirection)) && Passed;
  Passed = EvaluateVectorNear("OrientPositionAndDirectionInvers()", Name, "The inverse restores the position", OrientedPosition/cm, ToG4(Position), Tolerance) && Passed;
  Passed = EvaluateVectorNear("OrientPositionAndDirectionInvers()", Name, "The inverse restores the direction", OrientedPositionDirection, ToG4(Direction), Tolerance) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestCalculateRotation()
{
  bool Passed = true;

  // The coordinate system of the orientation selects the meaning of the angles (Galactic: latitudes, local: theta angles)
  MCOrientationAccess Galactic;
  Passed = EvaluateTrue("Parse()", "Galactic Fixed", "A Galactic orientation can be parsed", ParseText(Galactic, "Run Orientation Galactic Fixed")) && Passed;
  MCOrientationAccess Local;
  Passed = EvaluateTrue("Parse()", "Local Fixed", "A local orientation can be parsed", ParseText(Local, "Run Orientation Local Fixed")) && Passed;
  const double Tolerance = 1e-14;

  // Local axes, x = (1, 0, 0) (theta 90 deg, phi 0), z = (0, 0, 1) (theta 0): the frame is always mirrored, y = -(z cross x) = (0, -1, 0)
  {
    double XTheta = 90*deg, XPhi = 0.0, ZTheta = 0.0, ZPhi = 0.0;
    MRotation Rotation;
    Passed = EvaluateTrue("CalculateRotation()", "local, exact axes", "Exact axes are accepted", Local.CalculateRotation(XTheta, XPhi, ZTheta, ZPhi, Rotation)) && Passed;
    Passed = EvaluateRotationNear("CalculateRotation()", "local, exact axes", "The rotation is the identity with y mirrored", Rotation, MRotation(1, 0, 0, 0, -1, 0, 0, 0, 1), Tolerance) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "local, exact axes", "The angle of the exact x-axis is not touched (theta)", XTheta, 90*deg, 0.0) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "local, exact axes", "The angle of the exact x-axis is not touched (phi)", XPhi, 0.0, 0.0) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "local, exact axes", "The angle of the exact z-axis is not touched (theta)", ZTheta, 0.0, 0.0) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "local, exact axes", "The angle of the exact z-axis is not touched (phi)", ZPhi, 0.0, 0.0) && Passed;
  }

  // Galactic axes (the angles are latitudes): x at latitude 0 and longitude 0 is (1, 0, 0), z at latitude 0 and longitude 90 deg is (0, 1, 0),
  // y = -(z cross x) = -(0, 0, -1) = (0, 0, 1), the columns of the rotation are x, y, z
  {
    double XLat = 0.0, XLong = 0.0, ZLat = 0.0, ZLong = 90*deg;
    MRotation Rotation;
    Passed = EvaluateTrue("CalculateRotation()", "Galactic, exact axes", "Exact Galactic axes are accepted", Galactic.CalculateRotation(XLat, XLong, ZLat, ZLong, Rotation)) && Passed;
    Passed = EvaluateRotationNear("CalculateRotation()", "Galactic, exact axes", "The columns are x = (1, 0, 0), y = (0, 0, 1), z = (0, 1, 0)", Rotation, MRotation(1, 0, 0, 0, 0, 1, 0, 1, 0), Tolerance) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "Galactic, exact axes", "The exact z longitude is not touched", ZLong, 90*deg, 0.0) && Passed;
  }

  // Galactic axes 0.05 deg from a right angle: z = (cos p, sin p, 0) with p = 89.95 deg, x = (1, 0, 0).
  // Closed form: x' = x - (x.z) z, normalized = (sin p, -cos p, 0), i.e. latitude 0 and longitude p - 90 deg = -0.05 deg; z is kept; y' = -(z cross x') = (0, 0, 1)
  {
    const double P = 89.95*deg;
    double XLat = 0.0, XLong = 0.0, ZLat = 0.0, ZLong = P;
    MRotation Rotation;
    Passed = EvaluateTrue("CalculateRotation()", "Galactic, 0.05 deg skew", "Axes 0.05 deg from a right angle are accepted", Galactic.CalculateRotation(XLat, XLong, ZLat, ZLong, Rotation)) && Passed;
    Passed = EvaluateRotationNear("CalculateRotation()", "Galactic, 0.05 deg skew", "x is projected perpendicular to z, z is kept", Rotation, MRotation(sin(P), 0, cos(P), -cos(P), 0, sin(P), 0, 1, 0), Tolerance) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "Galactic, 0.05 deg skew", "The corrected x latitude", XLat, 0.0, Tolerance) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "Galactic, 0.05 deg skew", "The corrected x longitude is -0.05 deg", XLong, P - 90*deg, Tolerance) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "Galactic, 0.05 deg skew", "The z latitude is kept exactly", ZLat, 0.0, 0.0) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "Galactic, 0.05 deg skew", "The z longitude is kept exactly", ZLong, P, 0.0) && Passed;
  }

  // The longitude stays in the range of the input: x at 350 deg, z at 79.95 deg -> the corrected x is at 79.95 - 90 = -10.05 deg = 349.95 deg, not -10.05 deg
  {
    double XLat = 0.0, XLong = 350*deg, ZLat = 0.0, ZLong = 79.95*deg;
    MRotation Rotation;
    Passed = EvaluateTrue("CalculateRotation()", "Galactic, longitude range", "Axes 0.05 deg from a right angle are accepted", Galactic.CalculateRotation(XLat, XLong, ZLat, ZLong, Rotation)) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "Galactic, longitude range", "The corrected x longitude stays in the range of the input (349.95 deg)", XLong, 349.95*deg, Tolerance) && Passed;
  }

  // An axis at the pole: its longitude is arbitrary and must not be changed if the axis was not corrected
  {
    double XLat = 0.0, XLong = 40*deg, ZLat = c_Pi/2, ZLong = 123*deg;
    MRotation Rotation;
    Passed = EvaluateTrue("CalculateRotation()", "Galactic, z at the pole", "A z-axis at the pole is accepted", Galactic.CalculateRotation(XLat, XLong, ZLat, ZLong, Rotation)) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "Galactic, z at the pole", "The arbitrary longitude of the z-axis at the pole is kept", ZLong, 123*deg, 0.0) && Passed;
    Passed = EvaluateNear("CalculateRotation()", "Galactic, z at the pole", "The longitude of the exact x-axis is kept", XLong, 40*deg, 0.0) && Passed;
  }

  // Invalid axes are rejected and nothing is changed (the rotation and all angles stay as they were):
  {
    const MRotation Marker(2, 0, 0, 0, 2, 0, 0, 0, 2);
    struct Case { const char* Name; bool IsGalactic; double XTheta; double XPhi; double ZTheta; double ZPhi; };
    const Case Cases[] = {
      { "local, 45 deg apart", false, 90*deg, 0.0, 45*deg, 0.0 },
      { "local, parallel", false, 90*deg, 0.0, 90*deg, 0.0 },
      { "Galactic, 0.07 deg from a right angle", true, 0.0, 0.0, 0.0, 89.93*deg }
    };
    for (const Case& C : Cases) {
      double XTheta = C.XTheta, XPhi = C.XPhi, ZTheta = C.ZTheta, ZPhi = C.ZPhi;
      MRotation Rotation = Marker;
      MString Input = C.Name;
      MCOrientationAccess& O = C.IsGalactic ? Galactic : Local;
      DisableDefaultStreams();
      const bool Result = O.CalculateRotation(XTheta, XPhi, ZTheta, ZPhi, Rotation);
      EnableDefaultStreams();
      Passed = EvaluateFalse("CalculateRotation()", Input, "Axes which are not at a right angle (within 0.001 rad) are rejected", Result) && Passed;
      Passed = EvaluateRotationNear("CalculateRotation()", Input, "A rejection leaves the rotation unchanged", Rotation, Marker, 0.0) && Passed;
      Passed = EvaluateNear("CalculateRotation()", Input, "A rejection leaves the x angle (theta) unchanged", XTheta, C.XTheta, 0.0) && Passed;
      Passed = EvaluateNear("CalculateRotation()", Input, "A rejection leaves the x angle (phi) unchanged", XPhi, C.XPhi, 0.0) && Passed;
      Passed = EvaluateNear("CalculateRotation()", Input, "A rejection leaves the z angle (theta) unchanged", ZTheta, C.ZTheta, 0.0) && Passed;
      Passed = EvaluateNear("CalculateRotation()", Input, "A rejection leaves the z angle (phi) unchanged", ZPhi, C.ZPhi, 0.0) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCOrientation::TestStream()
{
  bool Passed = true;

  ostringstream Out;
  Out<<MCOrientationCoordinateSystem::c_Local<<" "<<MCOrientationCoordinateSystem::c_Galactic;
  Passed = Evaluate("operator<<", "coordinate systems", "The coordinate systems are streamed as their integer values", MString(Out.str().c_str()), MString("0 1")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCOrientation Test;
  return Test.Run() == true ? 0 : 1;
}
