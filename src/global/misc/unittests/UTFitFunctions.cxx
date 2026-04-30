/*
 * UTFitFunctions.cxx
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

// ROOT libs:
#include <TMath.h>

// MEGAlib:
#include "MFitFunctions.h"
#include "MUnitTest.h"


//! Unit test class for MFitFunctions
class UTFitFunctions : public MUnitTest
{
public:
  UTFitFunctions() : MUnitTest("UTFitFunctions") {}
  virtual ~UTFitFunctions() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTFitFunctions::Run()
{
  bool Passed = true;

  {
    double X[1] = {1.5};
    double P[2] = {2.0, -0.5};
    Passed = EvaluateNear("Linear()", "representative interior", "Linear evaluates a representative interior point exactly", Linear(X, P), 1.25, 1e-12) && Passed;
  }

  {
    double X[1] = {3.0};
    double P[2] = {18.0, 2.0};
    Passed = EvaluateNear("Lorentz2()", "representative interior", "Lorentz2 evaluates a representative interior point exactly", Lorentz2(X, P), 18.0/13.0, 1e-12) && Passed;
  }

  {
    double X[1] = {3.5};
    double P[3] = {4.0, 1.5, 2.0};
    Passed = EvaluateNear("Lorentz()", "representative interior", "Lorentz evaluates a representative interior point exactly", Lorentz(X, P), 4.0/(1.5*1.5 + 1.5*1.5), 1e-12) && Passed;
  }

  {
    double X[1] = {2.0};
    double P[4] = {9.0, 0.0, 2.0, 5.0};
    Passed = EvaluateNear("LorentzARM()", "zero denominator", "LorentzARM returns zero for the representative guarded zero-denominator case", LorentzARM(X, P), 0.0, 1e-12) && Passed;
  }

  {
    double X[1] = {1.5};
    double P[3] = {10.0, 1.0, 0.5};
    Passed = EvaluateNear("Gauss()", "representative interior", "Gauss evaluates a representative interior point exactly", Gauss(X, P), 10.0*exp(-0.5), 1e-12) && Passed;
  }

  {
    double X[1] = {2.0};
    double P[3] = {7.0, 3.0, 0.0};
    Passed = EvaluateNear("Gauss()", "zero sigma", "Gauss falls back to the representative scaler when sigma is zero", Gauss(X, P), 7.0, 1e-12) && Passed;
  }

  {
    double X[1] = {1.5};
    double P[4] = {2.0, 5.0, 1.0, 0.5};
    Passed = EvaluateNear("Gauss2()", "representative interior", "Gauss2 evaluates a representative offset-plus-gaussian point exactly", Gauss2(X, P), 2.0 + 5.0*exp(-0.5), 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[7] = {1.0, 2.0, 0.5, 0.3, 3.0, 1.5, 0.4};
    double Expected = 1.0 + 3.0*exp(-0.5*pow((1.0 - 1.5)/0.4, 2)) + 2.0*TMath::Landau(-1.0 + 0.5, 0, 0.3);
    Passed = EvaluateNear("GaussLandau()", "representative interior", "GaussLandau evaluates a representative mixed Landau-Gaussian point exactly", GaussLandau(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[5] = {1.0, 4.0, 0.0, 0.5, 2.0};
    Passed = EvaluateNear("AsymGaus()", "representative right side", "AsymGaus uses the representative right-side sigma branch", AsymGaus(X, P), 1.0 + 4.0*exp(-0.5*pow((1.0 - 0.0)/0.5, 2)), 1e-12) && Passed;
  }

  {
    double X[1] = {-1.0};
    double P[5] = {1.0, 4.0, 0.0, 0.5, 2.0};
    Passed = EvaluateNear("AsymGaus()", "representative left side", "AsymGaus uses the representative left-side sigma branch", AsymGaus(X, P), 1.0 + 4.0*exp(-0.5*pow((-1.0 - 0.0)/2.0, 2)), 1e-12) && Passed;
  }

  {
    double X[1] = {5.0};
    double P[8] = {3.0, 2.0, 4.0, 1.0, 6.0, 2.0, 0.5, 1.0};
    double Expected = 3.0/(2.0*2.0 + (5.0 - 4.0)*(5.0 - 4.0)) + 1.0 + 6.0*exp(-0.5*pow(log((5.0 - 1.0)/2.0)/0.5, 2));
    Passed = EvaluateNear("LorentzLogNormalARM()", "representative positive logarithmic tail", "LorentzLogNormalARM evaluates the representative positive logarithmic tail branch", LorentzLogNormalARM(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {0.5};
    double P[8] = {3.0, 2.0, 4.0, 1.0, 6.0, 2.0, 0.5, 1.0};
    double Expected = 3.0/(2.0*2.0 + (0.5 - 4.0)*(0.5 - 4.0)) + 1.0;
    Passed = EvaluateNear("LorentzLogNormalARM()", "representative non-positive logarithmic tail", "LorentzLogNormalARM suppresses the representative logarithmic tail when the argument is non-positive", LorentzLogNormalARM(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[8] = {4.0, 2.0, 0.5, 1.0, 3.0, 0.0, 0.25, 1.5};
    double Expected = 4.0/(2.0*2.0 + (1.0 - 0.5)*(1.0 - 0.5)) + 1.0 + 3.0*exp(-0.5*pow((1.0 - 0.0)/0.25, 2));
    Passed = EvaluateNear("LorentzAsymGausArm()", "representative right side", "LorentzAsymGausArm evaluates the representative right-side asymmetric Gaussian branch", LorentzAsymGausArm(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {-1.0};
    double P[8] = {4.0, 2.0, 0.5, 1.0, 3.0, 0.0, 0.25, 1.5};
    double Expected = 4.0/(2.0*2.0 + (-1.0 - 0.5)*(-1.0 - 0.5)) + 1.0 + 3.0*exp(-0.5*pow((-1.0 - 0.0)/1.5, 2));
    Passed = EvaluateNear("LorentzAsymGausArm()", "representative left side", "LorentzAsymGausArm evaluates the representative left-side asymmetric Gaussian branch", LorentzAsymGausArm(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[9] = {1.0, 0.0, 2.0, -3.0, 4.0, -5.0, -6.0, 0.5, 1.5};
    double Expected = 1.0 + fabs(-3.0)*(2.0*2.0)/(2.0*2.0 + 1.0*1.0) + fabs(-5.0)*(4.0*4.0)/(4.0*4.0 + 1.0*1.0) + fabs(-6.0)*exp(-0.5*pow((1.0 - 0.0)/0.5, 2));
    Passed = EvaluateNear("DoubleLorentzAsymGausArm()", "representative right side", "DoubleLorentzAsymGausArm evaluates the representative right-side mixed peak exactly", DoubleLorentzAsymGausArm(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {-1.0};
    double P[9] = {1.0, 0.0, 2.0, -3.0, 4.0, -5.0, -6.0, 0.5, 1.5};
    double Expected = 1.0 + fabs(-3.0)*(2.0*2.0)/(2.0*2.0 + 1.0*1.0) + fabs(-5.0)*(4.0*4.0)/(4.0*4.0 + 1.0*1.0) + fabs(-6.0)*exp(-0.5*pow((-1.0 - 0.0)/1.5, 2));
    Passed = EvaluateNear("DoubleLorentzAsymGausArm()", "representative left side", "DoubleLorentzAsymGausArm evaluates the representative left-side mixed peak exactly", DoubleLorentzAsymGausArm(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {0.25};
    double P[9] = {1.0, 2.0, 3.0, 4.0, -1.0, 0.5, -6.0, 0.0, 2.0};
    double Expected = 1.0 + 2.0*atan(3.0*0.25) + 4.0*exp(-0.5*pow((0.25 + 1.0)/0.5, 2)) + fabs(-6.0)/(2.0*2.0 + (0.25 - 0.0)*(0.25 - 0.0));
    Passed = EvaluateNear("ArcTanLorentzGausArm()", "representative interior", "ArcTanLorentzGausArm evaluates a representative mixed arctan-Lorentz-Gaussian point exactly", ArcTanLorentzGausArm(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[4] = {2.0, 0.0, 0.0, 3.0};
    Passed = EvaluateNear("GaussSPD()", "zero sigma", "GaussSPD falls back to the representative offset plus height when sigma is zero", GaussSPD(X, P), 5.0, 1e-12) && Passed;
  }

  {
    double X[1] = {0.0};
    double P[5] = {1.0, 2.0, 3.0, 4.0, 0.0};
    Passed = EvaluateNear("LorentzGaussSPD()", "zero Lorentz denominator", "LorentzGaussSPD returns zero for the representative guarded zero-denominator case", LorentzGaussSPD(X, P), 0.0, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[7] = {1.0, 1.0, 2.0, 2.0, 3.0, 4.0, 5.0};
    double Expected = 1.0 + 2.0*exp(-0.5*pow(1.0/1.0, 2)) + 3.0*exp(-0.5*pow(1.0/2.0, 2)) + 5.0*exp(-0.5*pow(1.0/4.0, 2));
    Passed = EvaluateNear("TrippleGaussSPD()", "representative interior", "TrippleGaussSPD evaluates a representative three-component SPD fit exactly", TrippleGaussSPD(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[7] = {1.0, 2.0, 0.0, 3.0, 4.0, 2.0, 5.0};
    double Expected = 1.0 + 3.0*exp(-0.5*pow((1.0 - 0.0)/2.0, 2)) + 5.0*exp(-0.5*pow((1.0 - 2.0)/4.0, 2));
    Passed = EvaluateNear("DoubleGauss()", "representative interior", "DoubleGauss evaluates a representative two-Gaussian mixture exactly", DoubleGauss(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[10] = {1.0, 2.0, 0.0, 3.0, 4.0, 2.0, 5.0, 6.0, -1.0, 7.0};
    double Expected = 1.0 + 3.0*exp(-0.5*pow((1.0 - 0.0)/2.0, 2)) + 5.0*exp(-0.5*pow((1.0 - 2.0)/4.0, 2)) + 7.0*exp(-0.5*pow((1.0 + 1.0)/6.0, 2));
    Passed = EvaluateNear("TrippleGauss()", "representative interior", "TrippleGauss evaluates a representative three-Gaussian mixture exactly", TrippleGauss(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[16] = {1.0, 2.0, 0.0, 3.0, 4.0, 2.0, 5.0, 6.0, -1.0, 7.0, 8.0, 3.0, 9.0, 10.0, 4.0, 11.0};
    double Expected = 1.0 + 3.0*exp(-0.5*pow((1.0 - 0.0)/2.0, 2)) + 5.0*exp(-0.5*pow((1.0 - 2.0)/4.0, 2)) + 7.0*exp(-0.5*pow((1.0 + 1.0)/6.0, 2)) + 9.0*exp(-0.5*pow((1.0 - 3.0)/8.0, 2)) + 11.0*exp(-0.5*pow((1.0 - 4.0)/10.0, 2));
    Passed = EvaluateNear("QuintupleGauss()", "representative interior", "QuintupleGauss evaluates a representative five-Gaussian mixture exactly", QuintupleGauss(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {1.0};
    double P[6] = {1.0, 0.5, 0.3, 0.4, 1.5, 2.0};
    double Expected = 1.0 + TMath::Landau(1.0, 0.5, 0.3) + 2.0*exp(-0.5*pow((1.0 - 1.5)/0.4, 2));
    Passed = EvaluateNear("LandauGauss()", "representative interior", "LandauGauss evaluates a representative mixed Landau-Gaussian point exactly", LandauGauss(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {30.0};
    double P[3] = {5.0, 2.0, 15.0};
    double Expected = 5.0 + 2.0*cos(2*(30.0 - 15.0 + 90.0)*c_Rad);
    Passed = EvaluateNear("PolarizationModulation()", "representative interior", "PolarizationModulation evaluates a representative interior angle exactly", PolarizationModulation(X, P), Expected, 1e-12) && Passed;
  }

  {
    double X[1] = {0.5};
    double P[21] = {
      10.0, 3.0, 20.0,
      1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      5.0, 0.0, 0.0, 0.0, 7.0, 0.0, 0.0, 0.0, 0.0
    };
    double Expected = 10.0 - 3.0*cos(2*0.5*c_Rad + 20.0*c_Rad) + 7.0;
    Passed = EvaluateNear("SpikyPolarizationModulation()", "representative interior spike", "SpikyPolarizationModulation adds the representative matching spike height", SpikyPolarizationModulation(X, P), Expected, 1e-12) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFitFunctions Test;
  return Test.Run() == true ? 0 : 1;
}
