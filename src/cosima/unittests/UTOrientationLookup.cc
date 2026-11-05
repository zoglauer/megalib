/*
 * UTOrientationLookup.cc
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

// MEGAlib:
#include "MExceptions.h"
#include "MStreams.h"
#include "MString.h"
#include "MTokenizer.h"
#include "MUnitTest.h"

// Geant4:
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <cmath>
#include <string>
using namespace std;


//! Class exposing the protected functions of MCOrientation to the unit test
class MCOrientationAccess : public MCOrientation
{
public:
  using MCOrientation::Read;
  using MCOrientation::InRange;
  using MCOrientation::FindClosestIndex;
};


//! Unit test for the lookup of the orientation entry closest in time in MCOrientation (backport of the tests of UTOrientation of develop-cosi-dc6)
class UTOrientationLookup : public MUnitTest
{
public:
  //! Default constructor
  UTOrientationLookup() : MUnitTest("UTOrientationLookup") {}
  //! Default destructor
  virtual ~UTOrientationLookup() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the closest index search
  bool TestFindClosestIndex();
  //! Test the closest entry between the samples of a looping file
  bool TestLoopBetweenSamples();
  //! Test the closest entry of a slew sampled in equal steps, as in the end-to-end test ETCosimaToMimrecOrientation
  bool TestSlewSteps();

  //! Parse a text without output
  bool ParseText(MCOrientation& Orientation, const MString& Text);
  //! Write a file and parse it as orientation file
  bool ParseFile(MCOrientation& Orientation, const MString& Name, const MString& Content, const MString& Mode, const MString& System = "Galactic");
};


////////////////////////////////////////////////////////////////////////////////


bool UTOrientationLookup::Run()
{
  bool Passed = true;

  Passed = TestFindClosestIndex() && Passed;
  Passed = TestLoopBetweenSamples() && Passed;
  Passed = TestSlewSteps() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTOrientationLookup::ParseText(MCOrientation& Orientation, const MString& Text)
{
  MTokenizer Tokenizer;
  Tokenizer.Analyse(Text, false);

  DisableDefaultStreams();
  bool Result = Orientation.Parse(Tokenizer);
  EnableDefaultStreams();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


bool UTOrientationLookup::ParseFile(MCOrientation& Orientation, const MString& Name, const MString& Content, const MString& Mode, const MString& System)
{
  // The file is written below the private temporary root, which is the working directory of the test, and the parser gets the relative name:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  if (WriteTextFile(GetTemporaryFileName(Name), Content) == false) return false;

  return ParseText(Orientation, MString("Run Orientation ") + System + " File " + Mode + " " + Name);
}


////////////////////////////////////////////////////////////////////////////////


bool UTOrientationLookup::TestFindClosestIndex()
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


bool UTOrientationLookup::TestLoopBetweenSamples()
{
  bool Passed = true;

  // Times between the samples of a looping file (10, 20, 30 s; period 20 s): the closest entry is used, a tie goes to the earlier entry,
  // times beyond the last entry or before the first entry are wrapped into the period
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

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTOrientationLookup::TestSlewSteps()
{
  bool Passed = true;

  // The slew of ETCosimaToMimrecOrientation: the z-axis moves along the equator in steps of 0.45 deg every 0.5 s,
  // a time between two entries has to use the closer entry, i.e. the entry round(Time/Step)
  const double Step = 0.5;
  const unsigned int NumberOfSteps = 30;
  MString Content;
  for (unsigned int i = 0; i <= NumberOfSteps; ++i) {
    Content += MString("OG ") + Step*i + " 90 0 0 " + 0.45*i + "\n";
  }
  MCOrientationAccess O;
  Passed = EvaluateTrue("Parse()", "slew", "The slew file can be parsed", ParseFile(O, "Slew.ori", Content, "NoLoop")) && Passed;

  const double Times[] = { 6.0019, 6.2, 6.24, 6.26, 6.49, 0.01, 0.49, 14.51, 14.99 };
  for (double Time: Times) {
    const unsigned int Expected = static_cast<unsigned int>(round(Time/Step));
    Passed = Evaluate("FindClosestIndex()", MString("t = ") + Time + " s", "The entry closest in time is used: round(t/step)", O.FindClosestIndex(Time*s), Expected) && Passed;
  }
  // Exactly in the middle the earlier entry is used:
  Passed = Evaluate("FindClosestIndex()", "t = 6.25 s", "In the middle of two entries the earlier entry is used", O.FindClosestIndex(6.25*s), 12U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTOrientationLookup Test;
  return Test.Run() == true ? 0 : 1;
}
