/*
 * UTMath.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <fcntl.h>
#include <unistd.h>
using namespace std;

// MEGAlib:
#include "MMath.h"
#include "MUnitTest.h"


class UTMathSupport
{
public:
  static int SilenceStdout()
  {
    int SavedStdout = dup(STDOUT_FILENO);
    int DevNull = open("/dev/null", O_WRONLY);
    if (DevNull >= 0) {
      dup2(DevNull, STDOUT_FILENO);
      close(DevNull);
    }
    return SavedStdout;
  }

  static void RestoreStdout(int SavedStdout)
  {
    if (SavedStdout >= 0) {
      dup2(SavedStdout, STDOUT_FILENO);
      close(SavedStdout);
    }
  }
};


//! Unit test class for MMath
class UTMath : public MUnitTest
{
public:
  UTMath() : MUnitTest("UTMath") {}
  virtual ~UTMath() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTMath::Run()
{
  bool Passed = true;

  MMath Math;
  Passed = Evaluate("MMath()", "construction", "A representative MMath instance can be constructed", true, true) && Passed;

  double Theta = 30.0;
  double Phi = 45.0;
  MMath::GalacticToSpheric(Theta, Phi);
  Passed = EvaluateNear("GalacticToSpheric()", "representative values theta", "GalacticToSpheric converts representative latitude to spherical theta", Theta, 60.0, 1e-12) && Passed;
  Passed = EvaluateNear("GalacticToSpheric()", "representative values phi", "GalacticToSpheric converts representative longitude to spherical phi", Phi, -45.0, 1e-12) && Passed;
  MMath::SphericToGalactic(Theta, Phi);
  Passed = EvaluateNear("SphericToGalactic()", "round trip theta", "SphericToGalactic reverses the representative theta transformation", Theta, 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("SphericToGalactic()", "round trip phi", "SphericToGalactic reverses the representative phi transformation", Phi, 45.0, 1e-12) && Passed;

  double X = 90.0;
  double Y = 0.0;
  double Z = 2.0;
  MMath::SphericToCartesean(X, Y, Z);
  Passed = EvaluateNear("SphericToCartesean()", "representative values x", "SphericToCartesean converts representative spherical coordinates to Cartesian x", X, 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("SphericToCartesean()", "representative values y", "SphericToCartesean converts representative spherical coordinates to Cartesian y", Y, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("SphericToCartesean()", "representative values z", "SphericToCartesean converts representative spherical coordinates to Cartesian z", Z, 0.0, 1e-12) && Passed;

  double ThetaBack = X;
  double PhiBack = Y;
  double RadiusBack = Z;
  MMath::CarteseanToSpheric(ThetaBack, PhiBack, RadiusBack);
  Passed = EvaluateNear("CarteseanToSpheric()", "representative round trip theta", "CarteseanToSpheric reconstructs the representative theta from Cartesian input", ThetaBack, 90.0, 1e-12) && Passed;
  Passed = EvaluateNear("CarteseanToSpheric()", "representative round trip phi", "CarteseanToSpheric reconstructs the representative phi from Cartesian input", PhiBack, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("CarteseanToSpheric()", "representative round trip radius", "CarteseanToSpheric reconstructs the representative radius from Cartesian input", RadiusBack, 2.0, 1e-12) && Passed;

  {
    double InteriorTheta = -20.0;
    double InteriorPhi = 135.0;
    MMath::GalacticToSpheric(InteriorTheta, InteriorPhi);
    Passed = EvaluateNear("GalacticToSpheric()", "interior case 1 theta", "GalacticToSpheric converts a representative interior latitude", InteriorTheta, 110.0, 1e-12) && Passed;
    Passed = EvaluateNear("GalacticToSpheric()", "interior case 1 phi", "GalacticToSpheric converts a representative interior longitude", InteriorPhi, 45.0, 1e-12) && Passed;
    MMath::SphericToGalactic(InteriorTheta, InteriorPhi);
    Passed = EvaluateNear("SphericToGalactic()", "interior case 1 theta", "SphericToGalactic reverses a representative interior theta", InteriorTheta, -20.0, 1e-12) && Passed;
    Passed = EvaluateNear("SphericToGalactic()", "interior case 1 phi", "SphericToGalactic reverses a representative interior phi", InteriorPhi, 135.0, 1e-12) && Passed;
  }

  {
    double InteriorTheta = 72.5;
    double InteriorPhi = -170.0;
    MMath::GalacticToSpheric(InteriorTheta, InteriorPhi);
    Passed = EvaluateNear("GalacticToSpheric()", "interior case 2 theta", "GalacticToSpheric converts a representative high-latitude interior value", InteriorTheta, 17.5, 1e-12) && Passed;
    Passed = EvaluateNear("GalacticToSpheric()", "interior case 2 phi", "GalacticToSpheric converts a representative wrapped longitude", InteriorPhi, -260.0, 1e-12) && Passed;
    MMath::SphericToGalactic(InteriorTheta, InteriorPhi);
    Passed = EvaluateNear("SphericToGalactic()", "interior case 2 theta", "SphericToGalactic reverses a representative high-latitude interior theta", InteriorTheta, 72.5, 1e-12) && Passed;
    Passed = EvaluateNear("SphericToGalactic()", "interior case 2 phi", "SphericToGalactic reverses a representative wrapped interior phi", InteriorPhi, -170.0, 1e-12) && Passed;
  }

  {
    double InteriorTheta = 35.0;
    double InteriorPhi = 120.0;
    double InteriorRadius = 3.5;
    MMath::SphericToCartesean(InteriorTheta, InteriorPhi, InteriorRadius);
    Passed = EvaluateNear("SphericToCartesean()", "interior case 1 x", "SphericToCartesean converts a representative interior theta/phi pair to Cartesian x", InteriorTheta, 0.9850497869294838, 1e-12) && Passed;
    Passed = EvaluateNear("SphericToCartesean()", "interior case 1 y", "SphericToCartesean converts a representative interior theta/phi pair to Cartesian y", InteriorPhi, 1.7064425506458136, 1e-12) && Passed;
    Passed = EvaluateNear("SphericToCartesean()", "interior case 1 z", "SphericToCartesean converts a representative interior theta/phi pair to Cartesian z", InteriorRadius, 2.8670321550114714, 1e-12) && Passed;

    MMath::CarteseanToSpheric(InteriorTheta, InteriorPhi, InteriorRadius);
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 1 theta", "CarteseanToSpheric reconstructs a representative interior theta", InteriorTheta, 35.0, 1e-12) && Passed;
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 1 phi", "CarteseanToSpheric reconstructs a representative interior phi", InteriorPhi, 120.0, 1e-12) && Passed;
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 1 radius", "CarteseanToSpheric reconstructs a representative interior radius", InteriorRadius, 3.5, 1e-12) && Passed;
  }

  {
    double InteriorTheta = 123.0;
    double InteriorPhi = -40.0;
    double InteriorRadius = 1.7;
    MMath::SphericToCartesean(InteriorTheta, InteriorPhi, InteriorRadius);
    Passed = EvaluateNear("SphericToCartesean()", "interior case 2 x", "SphericToCartesean converts a representative lower-hemisphere interior theta/phi pair to Cartesian x", InteriorTheta, 1.1091126948997413, 1e-12) && Passed;
    Passed = EvaluateNear("SphericToCartesean()", "interior case 2 y", "SphericToCartesean converts a representative lower-hemisphere interior theta/phi pair to Cartesian y", InteriorPhi, -0.9307087844854911, 1e-12) && Passed;
    Passed = EvaluateNear("SphericToCartesean()", "interior case 2 z", "SphericToCartesean converts a representative lower-hemisphere interior theta/phi pair to Cartesian z", InteriorRadius, -0.9257770557617784, 1e-12) && Passed;

    MMath::CarteseanToSpheric(InteriorTheta, InteriorPhi, InteriorRadius);
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 2 theta", "CarteseanToSpheric reconstructs a representative lower-hemisphere interior theta", InteriorTheta, 123.0, 1e-12) && Passed;
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 2 phi", "CarteseanToSpheric reconstructs a representative lower-hemisphere interior phi", InteriorPhi, -40.0, 1e-12) && Passed;
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 2 radius", "CarteseanToSpheric reconstructs a representative lower-hemisphere interior radius", InteriorRadius, 1.7, 1e-12) && Passed;
  }

  {
    double InteriorTheta = 58.0;
    double InteriorPhi = 210.0;
    double InteriorRadius = 4.2;
    MMath::SphericToCartesean(InteriorTheta, InteriorPhi, InteriorRadius);
    Passed = EvaluateNear("SphericToCartesean()", "interior case 3 x", "SphericToCartesean converts a representative third-quadrant interior theta/phi pair to Cartesian x", InteriorTheta, -3.085977923426456, 1e-12) && Passed;
    Passed = EvaluateNear("SphericToCartesean()", "interior case 3 y", "SphericToCartesean converts a representative third-quadrant interior theta/phi pair to Cartesian y", InteriorPhi, -1.7816889301744826, 1e-12) && Passed;
    Passed = EvaluateNear("SphericToCartesean()", "interior case 3 z", "SphericToCartesean converts a representative third-quadrant interior theta/phi pair to Cartesian z", InteriorRadius, 2.225197889879566, 1e-12) && Passed;

    MMath::CarteseanToSpheric(InteriorTheta, InteriorPhi, InteriorRadius);
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 3 theta", "CarteseanToSpheric reconstructs a representative third-quadrant interior theta", InteriorTheta, 58.0, 1e-12) && Passed;
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 3 phi", "CarteseanToSpheric reconstructs a representative third-quadrant interior phi", InteriorPhi, -150.0, 1e-12) && Passed;
    Passed = EvaluateNear("CarteseanToSpheric()", "interior case 3 radius", "CarteseanToSpheric reconstructs a representative third-quadrant interior radius", InteriorRadius, 4.2, 1e-12) && Passed;
  }

  Passed = EvaluateNear("Gauss()", "center", "Gauss returns the normalized representative Gaussian peak value", Math.Gauss(0.0), 1.0/c_Sqrt2Pi, 1e-12) && Passed;
  Passed = Evaluate("Gauss()", "zero sigma", "Gauss returns the representative sentinel for zero sigma", Math.Gauss(1.0, 0.0, 0.0), 1.0e30) && Passed;
  Passed = EvaluateNear("Gauss()", "interior offset standard", "Gauss returns the representative standard Gaussian value away from the center", Math.Gauss(1.25), 0.18264908538902191, 1e-12) && Passed;
  Passed = EvaluateNear("Gauss()", "interior shifted", "Gauss returns the representative shifted Gaussian value", Math.Gauss(2.5, 1.5, 0.75), 0.21868009956799153, 1e-12) && Passed;
  Passed = EvaluateNear("Gauss()", "interior narrow", "Gauss returns the representative narrow Gaussian interior value", Math.Gauss(-0.4, -1.0, 0.5), 0.38837210996642596, 1e-12) && Passed;

  Passed = Evaluate("InRange()", "finite", "InRange accepts representative finite values", Math.InRange(42.0), true) && Passed;
  Passed = Evaluate("InRange()", "infinite", "InRange rejects representative infinite values", Math.InRange(numeric_limits<double>::infinity()), false) && Passed;
  Passed = Evaluate("InRange()", "negative finite interior", "InRange accepts a representative negative interior finite value", Math.InRange(-13.75), true) && Passed;
  Passed = Evaluate("InRange()", "fractional finite interior", "InRange accepts a representative fractional interior finite value", Math.InRange(0.125), true) && Passed;
  Passed = Evaluate("InRange()", "nan", "InRange rejects representative nan values", Math.InRange(numeric_limits<double>::quiet_NaN()), false) && Passed;

  Passed = EvaluateNear("AngleBetweenTwoVectors()", "orthogonal", "AngleBetweenTwoVectors returns the representative right angle", Math.AngleBetweenTwoVectors(1.0, 0.0, 0.0, 0.0, 1.0, 0.0), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("AngleBetweenTwoVectors()", "zero vector", "AngleBetweenTwoVectors returns zero for representative zero-length input", Math.AngleBetweenTwoVectors(0.0, 0.0, 0.0, 1.0, 0.0, 0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("AngleBetweenTwoVectors()", "interior acute", "AngleBetweenTwoVectors returns the representative acute interior angle", Math.AngleBetweenTwoVectors(1.0, 2.0, 3.0, 4.0, 0.5, -1.0), 1.3436261524769253, 1e-12) && Passed;
  Passed = EvaluateNear("AngleBetweenTwoVectors()", "interior obtuse", "AngleBetweenTwoVectors returns the representative obtuse interior angle", Math.AngleBetweenTwoVectors(-2.0, 1.0, 0.5, 1.5, -3.0, 2.0), 2.1125800146854643, 1e-12) && Passed;
  Passed = EvaluateNear("AngleBetweenTwoVectors()", "interior nearly parallel", "AngleBetweenTwoVectors returns the representative small interior angle between nearly parallel vectors", Math.AngleBetweenTwoVectors(1.0, 1.0, 0.0, 2.0, 1.8, 0.1), 0.180683842366634, 1e-12) && Passed;

  vector<bool> HighOutlier = Math.ModifiedThomsonTauTest(vector<double>{1.0, 1.1, 1.2, 10.0});
  Passed = Evaluate("ModifiedThomsonTauTest()", "high outlier size", "ModifiedThomsonTauTest returns one representative flag per value", HighOutlier.size(), 4UL) && Passed;
  Passed = Evaluate("ModifiedThomsonTauTest()", "high outlier detection", "ModifiedThomsonTauTest flags the representative high outlier", bool(HighOutlier[3]), true) && Passed;

  vector<bool> LowOutlier = Math.ModifiedThomsonTauTest(vector<double>{-10.0, 1.0, 1.1, 1.2});
  Passed = Evaluate("ModifiedThomsonTauTest()", "low outlier detection", "ModifiedThomsonTauTest flags the representative low outlier", bool(LowOutlier[0]), true) && Passed;

  vector<bool> KnownOutliers(4, false);
  KnownOutliers[3] = true;
  vector<bool> Preserved = Math.ModifiedThomsonTauTest(vector<double>{1.0, 1.1, 1.2, 10.0}, 0.05, KnownOutliers);
  Passed = Evaluate("ModifiedThomsonTauTest()", "known outliers", "ModifiedThomsonTauTest preserves representative known outliers", bool(Preserved[3]), true) && Passed;

  {
    int SavedStdout = UTMathSupport::SilenceStdout();
    vector<bool> TooFew = Math.ModifiedThomsonTauTest(vector<double>{1.0, 2.0});
    UTMathSupport::RestoreStdout(SavedStdout);
    Passed = Evaluate("ModifiedThomsonTauTest()", "too few values", "ModifiedThomsonTauTest returns representative default flags for too few values", TooFew.size(), 2UL) && Passed;
    Passed = Evaluate("ModifiedThomsonTauTest()", "too few values flags", "ModifiedThomsonTauTest leaves representative too-few-value flags unset", bool(TooFew[0]), false) && Passed;
  }

  {
    int SavedStdout = UTMathSupport::SilenceStdout();
    vector<bool> InvalidAlpha = Math.ModifiedThomsonTauTest(vector<double>{1.0, 2.0, 3.0}, 0.0);
    UTMathSupport::RestoreStdout(SavedStdout);
    Passed = Evaluate("ModifiedThomsonTauTest()", "invalid alpha", "ModifiedThomsonTauTest returns representative default flags for invalid alpha", InvalidAlpha.size(), 3UL) && Passed;
    Passed = Evaluate("ModifiedThomsonTauTest()", "invalid alpha flags", "ModifiedThomsonTauTest leaves representative invalid-alpha flags unset", bool(InvalidAlpha[1]), false) && Passed;
  }

  {
    int SavedStdout = UTMathSupport::SilenceStdout();
    vector<bool> InvalidKnown = Math.ModifiedThomsonTauTest(vector<double>{1.0, 2.0, 3.0}, 0.05, vector<bool>{false});
    UTMathSupport::RestoreStdout(SavedStdout);
    Passed = Evaluate("ModifiedThomsonTauTest()", "mismatched outlier mask", "ModifiedThomsonTauTest returns the representative mismatched outlier mask unchanged", InvalidKnown.size(), 1UL) && Passed;
  }

  {
    int SavedStdout = UTMathSupport::SilenceStdout();
    vector<bool> AllKnown = Math.ModifiedThomsonTauTest(vector<double>{1.0, 2.0, 3.0}, 0.05, vector<bool>{true, true, true});
    UTMathSupport::RestoreStdout(SavedStdout);
    Passed = Evaluate("ModifiedThomsonTauTest()", "all known outliers", "ModifiedThomsonTauTest preserves representative all-known-outlier masks", AllKnown.size(), 3UL) && Passed;
    Passed = Evaluate("ModifiedThomsonTauTest()", "all known outliers first flag", "ModifiedThomsonTauTest leaves representative all-known-outlier entries set", bool(AllKnown[0]), true) && Passed;
  }

  {
    int SavedStdout = UTMathSupport::SilenceStdout();
    vector<bool> NanInput = Math.ModifiedThomsonTauTest(vector<double>{1.0, numeric_limits<double>::quiet_NaN(), 3.0});
    UTMathSupport::RestoreStdout(SavedStdout);
    Passed = Evaluate("ModifiedThomsonTauTest()", "nan input", "ModifiedThomsonTauTest returns representative default flags for nan input", NanInput.size(), 3UL) && Passed;
    Passed = Evaluate("ModifiedThomsonTauTest()", "nan input flags", "ModifiedThomsonTauTest leaves representative nan-input flags unset", bool(NanInput[1]), false) && Passed;
  }

  {
    int SavedStdout = UTMathSupport::SilenceStdout();
    vector<bool> InfInput = Math.ModifiedThomsonTauTest(vector<double>{1.0, numeric_limits<double>::infinity(), 3.0});
    UTMathSupport::RestoreStdout(SavedStdout);
    Passed = Evaluate("ModifiedThomsonTauTest()", "inf input", "ModifiedThomsonTauTest returns representative default flags for infinite input", InfInput.size(), 3UL) && Passed;
    Passed = Evaluate("ModifiedThomsonTauTest()", "inf input flags", "ModifiedThomsonTauTest leaves representative infinite-input flags unset", bool(InfInput[1]), false) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTMath Test;
  return Test.Run() == true ? 0 : 1;
}
