/*
 * UTCommon.cc
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


// Geant4 - MCCommon.hh does not include the units it uses itself:
#include "G4SystemOfUnits.hh"
#include "CLHEP/Units/PhysicalConstants.h"

// Cosima:
#include "MCCommon.hh"

// MEGAlib:
#include "MUnitTest.h"


//! Unit test class for the constants in MCCommon.hh
class UTCommon : public MUnitTest
{
public:
  //! Default constructor
  UTCommon() : MUnitTest("UTCommon") {}
  //! Default destructor
  virtual ~UTCommon() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the values of the constants against the Geant4 values
  bool TestValues();
  //! Test that the constants which are meant to be reciprocals are consistent
  bool TestReciprocals();
  //! Test that the constants can be used in expressions without parentheses
  bool TestMacroExpansion();
};


////////////////////////////////////////////////////////////////////////////////


bool UTCommon::Run()
{
  bool Passed = true;

  Passed = TestValues() && Passed;
  Passed = TestReciprocals() && Passed;
  Passed = TestMacroExpansion() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCommon::TestValues()
{
  bool Passed = true;

  // The speed of light is given with three digits:
  Passed = EvaluateNear("k_SpeedOfLight", "value", "The speed of light agrees with the Geant4 value within 1%", k_SpeedOfLight, CLHEP::c_light, 1e-2*CLHEP::c_light) && Passed;
  Passed = EvaluateNear("k_inv_SpeedOfLight", "value", "The inverse speed of light agrees with the Geant4 value within 1e-4", k_inv_SpeedOfLight, 1.0/CLHEP::c_light, 1e-4/CLHEP::c_light) && Passed;
  Passed = EvaluateNear("k_RestMassElectron", "value", "The electron rest mass agrees with the Geant4 value within 1e-5", k_RestMassElectron, CLHEP::electron_mass_c2, 1e-5*CLHEP::electron_mass_c2) && Passed;
  Passed = EvaluateNear("k_E0", "value", "The energy E0 is the electron rest mass", k_E0, k_RestMassElectron, 1e-12) && Passed;
  Passed = EvaluateNear("k_RestMassElectron", "keV", "The electron rest mass is about 511 keV", k_RestMassElectron/keV, 511.0, 0.01) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCommon::TestReciprocals()
{
  bool Passed = true;

  // k_inv_SpeedOfLight is named the inverse of k_SpeedOfLight, thus their product must be one:
  const double Product = (k_SpeedOfLight)*(k_inv_SpeedOfLight);
  // Known failure: k_SpeedOfLight is 2.99E+08 m/s but k_inv_SpeedOfLight is 1/2.99792E+08 m/s, their product is 0.9974
  Passed = ExpectedToFail(EvaluateNear("k_SpeedOfLight*k_inv_SpeedOfLight", "product", "The two speed of light constants are reciprocals of each other within 1e-3", Product, 1.0, 1e-3)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCommon::TestMacroExpansion()
{
  bool Passed = true;

  // The macros are not parenthesized, thus a division expands to a division by the number only and a multiplication by the units:
  const double Time = meter / k_SpeedOfLight;
  const double TimeTruth = meter / (k_SpeedOfLight);
  // Known failure: the macro k_SpeedOfLight is not parenthesized, "meter / k_SpeedOfLight" divides by 2.99E+08 only and multiplies by meter/second
  Passed = ExpectedToFail(EvaluateNear("meter / k_SpeedOfLight", "time of flight", "The time of flight over 1 m is 3.3 ns", Time/ns, TimeTruth/ns, 1e-9)) && Passed;

  const double Inverse = 1.0 / k_SpeedOfLight;
  const double InverseTruth = 1.0 / (k_SpeedOfLight);
  // Known failure: the macro k_SpeedOfLight is not parenthesized, "1.0 / k_SpeedOfLight" divides by 2.99E+08 only and multiplies by meter/second
  Passed = ExpectedToFail(EvaluateNear("1.0 / k_SpeedOfLight", "inverse", "The inverse of the speed of light is the reciprocal of its value", Inverse, InverseTruth, 1e-9*InverseTruth)) && Passed;

  // A product is fine:
  const double Length = k_SpeedOfLight * (1*ns);
  Passed = EvaluateNear("k_SpeedOfLight * (1 ns)", "distance", "Light travels about 30 cm in 1 ns", Length/cm, 29.9, 0.1) && Passed;
  const double Flight = 100*cm * k_inv_SpeedOfLight;
  Passed = EvaluateNear("100 cm * k_inv_SpeedOfLight", "time", "Light needs about 3.3 ns for 1 m", Flight/ns, 3.3357, 0.001) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCommon Test;
  return Test.Run() == true ? 0 : 1;
}
