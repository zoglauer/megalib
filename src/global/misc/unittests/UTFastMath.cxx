/*
 * UTFastMath.cxx
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
#include <cmath>
using namespace std;

// ROOT:
#include "TObject.h"

// MEGAlib:
#include "MFastMath.h"
#include "MUnitTest.h"


//! Unit test class for MFastMath
class UTFastMath : public MUnitTest
{
public:
  UTFastMath() : MUnitTest("UTFastMath") {}
  virtual ~UTFastMath() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTFastMath::Run()
{
  bool Passed = true;

  // Documented maximum errors of the approximations:
  const double AtanAccuracy = 0.00150887;         // rad, absolute
  const double AcosAccuracy = 1e-4;               // absolute, 0.01%
  const double InvsqrtRelativeAccuracy = 5e-6;    // relative, 0.0005%
  const double SinCosAccuracy = 2.5e-9;           // absolute, Abramowitz and Stegun 4.3.97/4.3.99: 2e-9 plus coefficient rounding
  const double FloatPi = static_cast<double>(static_cast<float>(c_Pi)); // Clamp value at -1 is a float

  Passed = EvaluateNear("atan()", "zero", "MFastMath::atan returns the representative zero angle", MFastMath::atan(0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("atan()", "unity", "MFastMath::atan approximates the representative unit-angle well", MFastMath::atan(1.0), std::atan(1.0), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan()", "large negative", "MFastMath::atan approximates representative large negative arguments well", MFastMath::atan(-5.0), std::atan(-5.0), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan()", "interior 0.25", "MFastMath::atan approximates a representative interior positive argument", MFastMath::atan(0.25), std::atan(0.25), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan()", "interior -0.75", "MFastMath::atan approximates a representative interior negative argument", MFastMath::atan(-0.75), std::atan(-0.75), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan()", "interior 2.5", "MFastMath::atan approximates a representative interior argument above one", MFastMath::atan(2.5), std::atan(2.5), AtanAccuracy) && Passed;

  // Largest error at |x| = 0.477: 1.5088e-3 (scan against the standard library)
  Passed = EvaluateNear("atan()", "largest error positive", "MFastMath::atan stays within its documented accuracy at the point of the largest error", MFastMath::atan(0.477), std::atan(0.477), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan()", "largest error negative", "MFastMath::atan stays within its documented accuracy at the mirrored point of the largest error", MFastMath::atan(-0.477), std::atan(-0.477), AtanAccuracy) && Passed;

  Passed = EvaluateNear("atan2()", "quadrant I", "MFastMath::atan2 returns the representative first-quadrant angle", MFastMath::atan2(1.0, 1.0), std::atan2(1.0, 1.0), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan2()", "quadrant II", "MFastMath::atan2 returns the representative second-quadrant angle", MFastMath::atan2(1.0, -1.0), std::atan2(1.0, -1.0), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan2()", "axis", "MFastMath::atan2 returns the representative positive y-axis angle", MFastMath::atan2(1.0, 0.0), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("atan2()", "origin", "MFastMath::atan2 returns zero at the representative origin", MFastMath::atan2(0.0, 0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("atan2()", "interior quadrant I", "MFastMath::atan2 approximates a representative interior first-quadrant angle", MFastMath::atan2(0.7, 1.9), std::atan2(0.7, 1.9), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan2()", "interior quadrant III", "MFastMath::atan2 approximates a representative interior third-quadrant angle", MFastMath::atan2(-0.8, -1.4), std::atan2(-0.8, -1.4), AtanAccuracy) && Passed;
  Passed = EvaluateNear("atan2()", "interior quadrant IV", "MFastMath::atan2 approximates a representative interior fourth-quadrant angle", MFastMath::atan2(-2.2, 0.9), std::atan2(-2.2, 0.9), AtanAccuracy) && Passed;

  Passed = EvaluateNear("acos()", "mid-range", "MFastMath::acos approximates the representative mid-range value well", MFastMath::acos(0.5F), std::acos(0.5F), AcosAccuracy) && Passed;
  Passed = EvaluateNear("acos()", "upper clamp", "MFastMath::acos clamps the representative upper boundary", MFastMath::acos(1.0F), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("acos()", "lower clamp", "MFastMath::acos clamps the representative lower boundary", MFastMath::acos(-1.0F), FloatPi, 1e-12) && Passed;
  Passed = EvaluateNear("acos()", "upper overflow clamp", "MFastMath::acos clamps representative values above one to zero", MFastMath::acos(1.1F), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("acos()", "lower overflow clamp", "MFastMath::acos clamps representative values below minus one to pi", MFastMath::acos(-1.1F), FloatPi, 1e-12) && Passed;
  Passed = EvaluateNear("acos()", "interior 0.2", "MFastMath::acos approximates a representative interior positive value", MFastMath::acos(0.2F), std::acos(0.2F), AcosAccuracy) && Passed;
  Passed = EvaluateNear("acos()", "interior -0.3", "MFastMath::acos approximates a representative interior negative value", MFastMath::acos(-0.3F), std::acos(-0.3F), AcosAccuracy) && Passed;
  Passed = EvaluateNear("acos()", "interior 0.9", "MFastMath::acos approximates a representative value near the upper interior range", MFastMath::acos(0.9F), std::acos(0.9F), AcosAccuracy) && Passed;

  Passed = EvaluateNear("invsqrt()", "representative value", "MFastMath::invsqrt approximates the representative inverse square root well", MFastMath::invsqrt(4.0F), 0.5, InvsqrtRelativeAccuracy*0.5) && Passed;
  Passed = EvaluateNear("invsqrt()", "unity", "MFastMath::invsqrt approximates the representative unity case well", MFastMath::invsqrt(1.0F), 1.0, InvsqrtRelativeAccuracy*1.0) && Passed;
  Passed = EvaluateNear("invsqrt()", "interior 2.5", "MFastMath::invsqrt approximates a representative interior positive value", MFastMath::invsqrt(2.5F), 1.0/std::sqrt(2.5), InvsqrtRelativeAccuracy*(1.0/std::sqrt(2.5))) && Passed;
  Passed = EvaluateNear("invsqrt()", "interior 10", "MFastMath::invsqrt approximates a representative larger positive value", MFastMath::invsqrt(10.0F), 1.0/std::sqrt(10.0), InvsqrtRelativeAccuracy*(1.0/std::sqrt(10.0))) && Passed;
  Passed = EvaluateNear("invsqrt()", "interior 0.25", "MFastMath::invsqrt approximates a representative fractional positive value", MFastMath::invsqrt(0.25F), 2.0, InvsqrtRelativeAccuracy*2.0) && Passed;

  Passed = EvaluateNear("cos()", "zero", "MFastMath::cos returns the representative cosine at zero", MFastMath::cos(0.0), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("cos()", "pi", "MFastMath::cos approximates the representative cosine at pi", MFastMath::cos(c_Pi), -1.0, 1e-12) && Passed;
  Passed = EvaluateNear("cos()", "periodic", "MFastMath::cos handles the representative periodic wrap", MFastMath::cos(2.0*c_Pi), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("cos()", "interior pi/6", "MFastMath::cos approximates a representative first-quadrant angle", MFastMath::cos(c_Pi/6.0), std::cos(c_Pi/6.0), SinCosAccuracy) && Passed;
  Passed = EvaluateNear("cos()", "interior 2.1", "MFastMath::cos approximates a representative interior second-quadrant angle", MFastMath::cos(2.1), std::cos(2.1), SinCosAccuracy) && Passed;
  Passed = EvaluateNear("cos()", "interior 5.4", "MFastMath::cos approximates a representative interior fourth-quadrant angle", MFastMath::cos(5.4), std::cos(5.4), SinCosAccuracy) && Passed;

  Passed = EvaluateNear("cos()", "half pi", "MFastMath::cos approximates the cosine at pi/2", MFastMath::cos(c_Pi/2.0), 0.0, SinCosAccuracy) && Passed;
  Passed = EvaluateNear("cos()", "negative", "MFastMath::cos wraps a negative angle", MFastMath::cos(-0.5), std::cos(-0.5), SinCosAccuracy) && Passed;
  Passed = EvaluateNear("cos()", "above 2 pi", "MFastMath::cos wraps an angle above 2*pi", MFastMath::cos(7.0), std::cos(7.0), SinCosAccuracy) && Passed;
  Passed = EvaluateNear("cos()", "near half pi", "MFastMath::cos approximates the cosine just below pi/2", MFastMath::cos(1.5), std::cos(1.5), SinCosAccuracy) && Passed;

  Passed = EvaluateNear("sin()", "zero", "MFastMath::sin returns the representative sine at zero", MFastMath::sin(0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("sin()", "half pi", "MFastMath::sin approximates the representative sine at pi/2", MFastMath::sin(c_Pi/2.0), 1.0, SinCosAccuracy) && Passed;
  Passed = EvaluateNear("sin()", "periodic", "MFastMath::sin handles the representative periodic wrap", MFastMath::sin(2.0*c_Pi), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("sin()", "interior pi/6", "MFastMath::sin approximates a representative first-quadrant angle", MFastMath::sin(c_Pi/6.0), std::sin(c_Pi/6.0), SinCosAccuracy) && Passed;
  Passed = EvaluateNear("sin()", "interior 2.1", "MFastMath::sin approximates a representative interior second-quadrant angle", MFastMath::sin(2.1), std::sin(2.1), SinCosAccuracy) && Passed;
  Passed = EvaluateNear("sin()", "interior 5.4", "MFastMath::sin approximates a representative interior fourth-quadrant angle", MFastMath::sin(5.4), std::sin(5.4), SinCosAccuracy) && Passed;

  Passed = EvaluateNear("sin()", "negative", "MFastMath::sin wraps a negative angle", MFastMath::sin(-1.2), std::sin(-1.2), SinCosAccuracy) && Passed;
  Passed = EvaluateNear("sin()", "above 2 pi", "MFastMath::sin wraps an angle above 2*pi", MFastMath::sin(7.0), std::sin(7.0), SinCosAccuracy) && Passed;
  Passed = EvaluateNear("sin()", "pi", "MFastMath::sin returns zero at pi", MFastMath::sin(c_Pi), 0.0, 1e-12) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFastMath Test;
  return Test.Run() == true ? 0 : 1;
}
