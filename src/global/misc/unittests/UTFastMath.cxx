/*
 * UTFastMath.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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

  MFastMath Math;
  Passed = Evaluate("MFastMath()", "construction", "A representative MFastMath instance can be constructed", true, true) && Passed;

  Passed = EvaluateNear("atan()", "zero", "MFastMath::atan returns the representative zero angle", MFastMath::atan(0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("atan()", "unity", "MFastMath::atan approximates the representative unit-angle well", MFastMath::atan(1.0), std::atan(1.0), 2e-3) && Passed;
  Passed = EvaluateNear("atan()", "large negative", "MFastMath::atan approximates representative large negative arguments well", MFastMath::atan(-5.0), std::atan(-5.0), 2e-3) && Passed;
  Passed = EvaluateNear("atan()", "interior 0.25", "MFastMath::atan approximates a representative interior positive argument", MFastMath::atan(0.25), std::atan(0.25), 2e-3) && Passed;
  Passed = EvaluateNear("atan()", "interior -0.75", "MFastMath::atan approximates a representative interior negative argument", MFastMath::atan(-0.75), std::atan(-0.75), 2e-3) && Passed;
  Passed = EvaluateNear("atan()", "interior 2.5", "MFastMath::atan approximates a representative interior argument above one", MFastMath::atan(2.5), std::atan(2.5), 2e-3) && Passed;

  Passed = EvaluateNear("atan2()", "quadrant I", "MFastMath::atan2 returns the representative first-quadrant angle", MFastMath::atan2(1.0, 1.0), std::atan2(1.0, 1.0), 2e-3) && Passed;
  Passed = EvaluateNear("atan2()", "quadrant II", "MFastMath::atan2 returns the representative second-quadrant angle", MFastMath::atan2(1.0, -1.0), std::atan2(1.0, -1.0), 2e-3) && Passed;
  Passed = EvaluateNear("atan2()", "axis", "MFastMath::atan2 returns the representative positive y-axis angle", MFastMath::atan2(1.0, 0.0), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("atan2()", "origin", "MFastMath::atan2 returns zero at the representative origin", MFastMath::atan2(0.0, 0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("atan2()", "interior quadrant I", "MFastMath::atan2 approximates a representative interior first-quadrant angle", MFastMath::atan2(0.7, 1.9), std::atan2(0.7, 1.9), 2e-3) && Passed;
  Passed = EvaluateNear("atan2()", "interior quadrant III", "MFastMath::atan2 approximates a representative interior third-quadrant angle", MFastMath::atan2(-0.8, -1.4), std::atan2(-0.8, -1.4), 2e-3) && Passed;
  Passed = EvaluateNear("atan2()", "interior quadrant IV", "MFastMath::atan2 approximates a representative interior fourth-quadrant angle", MFastMath::atan2(-2.2, 0.9), std::atan2(-2.2, 0.9), 2e-3) && Passed;

  Passed = EvaluateNear("acos()", "mid-range", "MFastMath::acos approximates the representative mid-range value well", MFastMath::acos(0.5F), std::acos(0.5F), 2e-4) && Passed;
  Passed = EvaluateNear("acos()", "upper clamp", "MFastMath::acos clamps the representative upper boundary", MFastMath::acos(1.0F), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("acos()", "lower clamp", "MFastMath::acos clamps the representative lower boundary", MFastMath::acos(-1.0F), c_Pi, 1e-6) && Passed;
  Passed = EvaluateNear("acos()", "upper overflow clamp", "MFastMath::acos clamps representative values above one to zero", MFastMath::acos(1.1F), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("acos()", "lower overflow clamp", "MFastMath::acos clamps representative values below minus one to pi", MFastMath::acos(-1.1F), c_Pi, 1e-6) && Passed;
  Passed = EvaluateNear("acos()", "interior 0.2", "MFastMath::acos approximates a representative interior positive value", MFastMath::acos(0.2F), std::acos(0.2F), 3e-4) && Passed;
  Passed = EvaluateNear("acos()", "interior -0.3", "MFastMath::acos approximates a representative interior negative value", MFastMath::acos(-0.3F), std::acos(-0.3F), 3e-4) && Passed;
  Passed = EvaluateNear("acos()", "interior 0.9", "MFastMath::acos approximates a representative value near the upper interior range", MFastMath::acos(0.9F), std::acos(0.9F), 3e-4) && Passed;

  Passed = EvaluateNear("invsqrt()", "representative value", "MFastMath::invsqrt approximates the representative inverse square root well", MFastMath::invsqrt(4.0F), 0.5, 5e-5) && Passed;
  Passed = EvaluateNear("invsqrt()", "unity", "MFastMath::invsqrt approximates the representative unity case well", MFastMath::invsqrt(1.0F), 1.0, 5e-5) && Passed;
  Passed = EvaluateNear("invsqrt()", "interior 2.5", "MFastMath::invsqrt approximates a representative interior positive value", MFastMath::invsqrt(2.5F), 1.0/std::sqrt(2.5), 5e-5) && Passed;
  Passed = EvaluateNear("invsqrt()", "interior 10", "MFastMath::invsqrt approximates a representative larger positive value", MFastMath::invsqrt(10.0F), 1.0/std::sqrt(10.0), 5e-5) && Passed;
  Passed = EvaluateNear("invsqrt()", "interior 0.25", "MFastMath::invsqrt approximates a representative fractional positive value", MFastMath::invsqrt(0.25F), 2.0, 5e-5) && Passed;

  Passed = EvaluateNear("cos()", "zero", "MFastMath::cos returns the representative cosine at zero", MFastMath::cos(0.0), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("cos()", "pi", "MFastMath::cos approximates the representative cosine at pi", MFastMath::cos(c_Pi), -1.0, 3e-5) && Passed;
  Passed = EvaluateNear("cos()", "periodic", "MFastMath::cos handles the representative periodic wrap", MFastMath::cos(2.0*c_Pi), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("cos()", "interior pi/6", "MFastMath::cos approximates a representative first-quadrant angle", MFastMath::cos(c_Pi/6.0), std::cos(c_Pi/6.0), 3e-5) && Passed;
  Passed = EvaluateNear("cos()", "interior 2.1", "MFastMath::cos approximates a representative interior second-quadrant angle", MFastMath::cos(2.1), std::cos(2.1), 3e-5) && Passed;
  Passed = EvaluateNear("cos()", "interior 5.4", "MFastMath::cos approximates a representative interior fourth-quadrant angle", MFastMath::cos(5.4), std::cos(5.4), 3e-5) && Passed;

  Passed = EvaluateNear("sin()", "zero", "MFastMath::sin returns the representative sine at zero", MFastMath::sin(0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("sin()", "half pi", "MFastMath::sin approximates the representative sine at pi/2", MFastMath::sin(c_Pi/2.0), 1.0, 3e-5) && Passed;
  Passed = EvaluateNear("sin()", "periodic", "MFastMath::sin handles the representative periodic wrap", MFastMath::sin(2.0*c_Pi), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("sin()", "interior pi/6", "MFastMath::sin approximates a representative first-quadrant angle", MFastMath::sin(c_Pi/6.0), std::sin(c_Pi/6.0), 3e-5) && Passed;
  Passed = EvaluateNear("sin()", "interior 2.1", "MFastMath::sin approximates a representative interior second-quadrant angle", MFastMath::sin(2.1), std::sin(2.1), 3e-5) && Passed;
  Passed = EvaluateNear("sin()", "interior 5.4", "MFastMath::sin approximates a representative interior fourth-quadrant angle", MFastMath::sin(5.4), std::sin(5.4), 3e-5) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFastMath Test;
  return Test.Run() == true ? 0 : 1;
}
