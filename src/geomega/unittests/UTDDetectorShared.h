/*
 * UTDDetectorShared.h
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


#ifndef __UTDDetectorShared__
#define __UTDDetectorShared__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <algorithm>
#include <cmath>
#include <csignal>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

// POSIX libs: fork, waitpid, and alarm for the crash checks have no C++ equivalent
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TMath.h"
#include "TROOT.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MDACS.h"
#include "MDAngerCamera.h"
#include "MDCalorimeter.h"
#include "MDDetector.h"
#include "MDDriftChamber.h"
#include "MDGeometry.h"
#include "MDGeometryQuest.h"
#include "MDGridPoint.h"
#include "MDGuardRing.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDStrip3DDirectional.h"
#include "MDVoxel3D.h"
#include "MStatistics.h"
#include "MFunction.h"
#include "MGlobal.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Base class of the unit tests of the detector classes with statistics helpers and the checks of the noise chain common to all detectors
//! Attention: All statistical tolerances are five standard errors derived from the number of samples - the seeds are fixed
class UTDDetectorBase : public UTGeomegaBase
{
public:
  //! Standard constructor with the name of the test
  UTDDetectorBase(const MString& Name) : UTGeomegaBase(Name) {}
  //! Default destructor
  virtual ~UTDDetectorBase() {}

protected:
  //! Check that a clone is independent of its source: changing a clone does not change the source or a clone of the clone, and a clone stays complete after its source is gone
  template <class DetectorType> bool CheckCloneIndependence(const MString& Case, DetectorType* Source)
  {
    bool Passed = true;
    // Attention: Some detectors hide this function of the base class with a version with arguments
    MDDetector* Base = Source;
    const double Threshold = Base->GetTriggerThreshold();
    unique_ptr<MDDetector> First(Source->Clone());
    unique_ptr<MDDetector> Second(First->Clone());
    DetectorType* SecondCopy = dynamic_cast<DetectorType*>(Second.get());
    if (EvaluateTrue("Clone()", Case, "The clone of a clone has the type of its source", SecondCopy != nullptr) == false) {
      return false;
    }
    const MString Text = SecondCopy->GetGeomega();

    First->SetTriggerThreshold(Threshold + 1.0);
    Passed = EvaluateNear("Clone()", Case + ", changed clone", "A change of the clone does not change its source", Base->GetTriggerThreshold(), Threshold, 1e-12) && Passed;
    Passed = EvaluateNear("Clone()", Case + ", changed clone", "A change of the clone does not change a clone of it", Second->GetTriggerThreshold(), Threshold, 1e-12) && Passed;

    // The guard rings are owned by the geometry, not by the detector: a clone has to have its own, and the ones of the clones are deleted here
    MDGuardRing* FirstRing = First->GetGuardRing();
    MDGuardRing* SecondRing = Second->GetGuardRing();
    if (Base->GetGuardRing() != nullptr) {
      MDDetector* SourceRing = Base->GetGuardRing();
      if (EvaluateTrue("Clone()", Case + ", guard ring", "Each clone has its own guard ring", FirstRing != nullptr && SecondRing != nullptr && FirstRing != SecondRing && FirstRing != Base->GetGuardRing() && SecondRing != Base->GetGuardRing()) == false) {
        return false;
      }
      MDDetector* FirstRingBase = FirstRing;
      MDDetector* SecondRingBase = SecondRing;
      const double RingThreshold = SourceRing->GetTriggerThreshold();
      FirstRingBase->SetTriggerThreshold(RingThreshold + 1.0);
      Passed = EvaluateNear("Clone()", Case + ", guard ring", "A change of the guard ring of a clone does not change the one of the source", SourceRing->GetTriggerThreshold(), RingThreshold, 1e-12) && Passed;
      Passed = EvaluateNear("Clone()", Case + ", guard ring", "A change of the guard ring of a clone does not change the one of another clone", SecondRingBase->GetTriggerThreshold(), RingThreshold, 1e-12) && Passed;
      delete FirstRing;
      Passed = EvaluateNear("Clone()", Case + ", guard ring", "A guard ring stays valid after the one of another clone is gone", SecondRingBase->GetTriggerThreshold(), RingThreshold, 1e-12) && Passed;
      delete SecondRing;
      SecondRing = nullptr;
    }

    First.reset();
    Passed = Evaluate("Clone()", Case + ", source gone", "A clone keeps all its data after its source is gone", SecondCopy->GetGeomega(), Text) && Passed;
    return Passed;
  }

  //! Check a direct call of CopyDataToNamedDetectors: a named detector inherits the data of its base, its own values stay. Configure sets data of the type, which Check reads from a named detector
  template <class DetectorType> bool CheckCopyDataToNamedDetectors(const MString& Case, const function<void(DetectorType&)>& Configure = nullptr, const function<bool(DetectorType&)>& Check = nullptr)
  {
    bool Passed = true;
    DetectorType Base("Base");
    DetectorType Named("Named");
    DetectorType Own("Own");
    MDDetector& BaseDetector = Base;
    MDDetector& NamedDetector = Named;
    MDDetector& OwnDetector = Own;
    Passed = EvaluateTrue("AddNamedDetector()", Case, "The first named detector is accepted", BaseDetector.AddNamedDetector(&Named)) && Passed;
    Passed = EvaluateTrue("AddNamedDetector()", Case, "The second named detector is accepted", BaseDetector.AddNamedDetector(&Own)) && Passed;
    BaseDetector.SetFailureRate(0.25);
    BaseDetector.SetOverflow(1234.0);
    OwnDetector.SetOverflow(4321.0);
    if (Configure) {
      Configure(Base);
    }
    Passed = EvaluateTrue("CopyDataToNamedDetectors()", Case, "The call is successful", Base.CopyDataToNamedDetectors()) && Passed;
    Passed = EvaluateNear("CopyDataToNamedDetectors()", Case + ", failure rate", "The value is inherited", NamedDetector.GetFailureRate(), 0.25, 1e-12) && Passed;
    Passed = EvaluateNear("CopyDataToNamedDetectors()", Case + ", overflow", "The value is inherited", NamedDetector.GetOverflow(), 1234.0, 1e-12) && Passed;
    Passed = EvaluateNear("CopyDataToNamedDetectors()", Case + ", own overflow", "The own value of a named detector stays", OwnDetector.GetOverflow(), 4321.0, 1e-12) && Passed;
    if (Check) {
      Passed = EvaluateTrue("CopyDataToNamedDetectors()", Case + ", data of the type", "The data of the detector type is inherited", Check(Named)) && Passed;
    }
    return Passed;
  }

  //! Check that the scanned geometry has the detector with its type, and update Passed: a geometry which lost it fails, and does not skip the checks of the detector
  bool CheckDetector(bool& Passed, const MString& Case, MDGeometry& Geometry, const MString& Name, int Type)
  {
    MDDetector* Detector = Geometry.GetDetector(Name);
    if (EvaluateTrue("GetDetector()", Case, MString("The geometry has the detector ") + Name, Detector != nullptr) == false) {
      Passed = false;
      return false;
    }
    if (Evaluate("GetType()", Case, MString("The detector ") + Name + " has its type", Detector->GetType(), Type) == false) {
      Passed = false;
      return false;
    }
    return true;
  }

  //! Number of samples of the statistical checks
  static constexpr unsigned int c_Samples = 100000;

  //! Return a box volume in the world volume
  MString GetBoxVolume(const MString& Name, double X, double Y, double Z, double PositionZ = 0.0, const MString& Mother = "World") const
  {
    ostringstream Out;
    Out<<"Volume "<<Name<<endl;
    Out<<Name<<".Material Silicon"<<endl;
    Out<<Name<<".Shape BRIK "<<X<<" "<<Y<<" "<<Z<<endl;
    Out<<Name<<".Position 0.0 0.0 "<<PositionZ<<endl;
    Out<<Name<<".Mother "<<Mother<<endl;
    return Out.str().c_str();
  }

  //! Scan the fixture consisting of the geometry head, the body and the extra keywords
  //! Attention: Some validations write to cout directly
  bool Scan(MDGeometry& Geometry, const MString& Body, const MString& Extra = "")
  {
    bool Result = false;
    BeginCaptureOutput();
    Result = ScanGeometry(Geometry, GetGeometryHead() + Body + Extra);
    EndCaptureOutput();
    return Result;
  }

  //! Return true if the fixture can be scanned
  bool IsScanOK(const MString& Body, const MString& Extra = "")
  {
    MDGeometry Geometry;
    return Scan(Geometry, Body, Extra);
  }

  //! Return the median of the absolute distances to the center
  double GetMedianDistance(const vector<double>& Values, double Center) const
  {
    vector<double> Distances;
    Distances.reserve(Values.size());
    for (double V: Values) {
      Distances.push_back(fabs(V - Center));
    }
    return MStatistics::GetMedian(Distances);
  }

  //! Return the fraction of values above the threshold
  double GetFractionAbove(const vector<double>& Values, double Threshold) const
  {
    unsigned int Above = 0;
    for (double V: Values) {
      if (V > Threshold) {
        ++Above;
      }
    }
    return static_cast<double>(Above)/Values.size();
  }

  //! Return the cumulative standard normal distribution
  double GetPhi(double X) const
  {
    return 0.5*erfc(-X/sqrt(2.0));
  }

  //! Return the tolerance of a mean: 5 standard errors
  double GetMeanTolerance(double Sigma, unsigned int N = c_Samples) const
  {
    return 5.0*Sigma/sqrt(static_cast<double>(N));
  }

  //! Return the tolerance of a standard deviation of a Gaussian: 5 standard errors
  double GetRMSTolerance(double Sigma, unsigned int N = c_Samples) const
  {
    return 5.0*Sigma/sqrt(2.0*N);
  }

  //! Return the tolerance of a fraction: 5 binomial standard errors
  double GetFractionTolerance(double Probability, unsigned int N = c_Samples) const
  {
    return 5.0*sqrt(Probability*(1.0 - Probability)/N);
  }

  //! Return the noised energies of a number of hits
  vector<double> SampleEnergy(MDDetector* Detector, MDVolume* Volume, const MVector& Position, double Energy, unsigned int N = c_Samples)
  {
    MString Flags;
    vector<double> Values;
    Values.reserve(N);
    DisableDefaultStreams();
    for (unsigned int i = 0; i < N; ++i) {
      MVector P = Position;
      double E = Energy;
      double T = 0.0;
      Detector->Noise(P, E, T, Flags, Volume);
      Values.push_back(E);
    }
    EnableDefaultStreams();
    return Values;
  }

  //! Return the noised times of a number of hits
  vector<double> SampleTime(MDDetector* Detector, MDVolume* Volume, const MVector& Position, double Energy, double Time, unsigned int N = c_Samples)
  {
    MString Flags;
    vector<double> Values;
    Values.reserve(N);
    DisableDefaultStreams();
    for (unsigned int i = 0; i < N; ++i) {
      MVector P = Position;
      double E = Energy;
      double T = Time;
      Detector->Noise(P, E, T, Flags, Volume);
      Values.push_back(T);
    }
    EnableDefaultStreams();
    return Values;
  }

  //! Return the noised position components of a number of hits
  vector<double> SamplePosition(MDDetector* Detector, MDVolume* Volume, const MVector& Position, double Energy, unsigned int Axis, unsigned int N = c_Samples)
  {
    MString Flags;
    vector<double> Values;
    Values.reserve(N);
    DisableDefaultStreams();
    for (unsigned int i = 0; i < N; ++i) {
      MVector P = Position;
      double E = Energy;
      double T = 0.0;
      Detector->Noise(P, E, T, Flags, Volume);
      Values.push_back(P[Axis]);
    }
    EnableDefaultStreams();
    return Values;
  }

  //! Return one noised energy
  double NoiseOnce(MDDetector* Detector, MDVolume* Volume, const MVector& Position, double Energy)
  {
    MString Flags;
    MVector P = Position;
    double E = Energy;
    double T = 0.0;
    DisableDefaultStreams();
    Detector->Noise(P, E, T, Flags, Volume);
    EnableDefaultStreams();
    return E;
  }

  //! Return the flags after one noising, which started with the given flags
  MString NoiseFlagsOnce(MDDetector* Detector, MDVolume* Volume, const MVector& Position, double Energy, const MString& StartFlags = "")
  {
    MString Flags = StartFlags;
    MVector P = Position;
    double E = Energy;
    double T = 0.0;
    DisableDefaultStreams();
    Detector->Noise(P, E, T, Flags, Volume);
    EnableDefaultStreams();
    return Flags;
  }

  //! Determine mean and standard deviation of the truncated Gauss-Landau distribution by numeric integration
  void GetGaussLandauMoments(double Peak1, double Width1, double Peak2, double Width2, double Ratio, double& Mean, double& RMS) const
  {
    double Minimum = Peak1 - 10.0*Width2 - 10.0*Width1;
    if (Minimum < 0.0) {
      Minimum = 0.0;
    }
    double Maximum = Peak1 + 4.0*Width2 + 4.0*Width1;
    unsigned int Steps = 200000;
    double Step = (Maximum - Minimum)/Steps;
    double Sum0 = 0.0;
    double Sum1 = 0.0;
    double Sum2 = 0.0;
    for (unsigned int s = 0; s < Steps; ++s) {
      double E = Minimum + (s + 0.5)*Step;
      double Arg = (E - Peak1)/Width1;
      double Density = Ratio*exp(-0.5*Arg*Arg) + (1.0 - Ratio)*TMath::Landau(Peak2 - E, 0.0, Width2);
      Sum0 += Density;
      Sum1 += Density*E;
      Sum2 += Density*E*E;
    }
    Mean = Sum1/Sum0;
    RMS = sqrt(Sum2/Sum0 - Mean*Mean);
  }

  //! Return the mean and the standard deviation of a truncated Gaussian
  void GetTruncatedGaussian(double Mean, double Sigma, double Low, double High, double& TruncatedMean, double& TruncatedRMS) const
  {
    // Textbook formula of the truncated normal distribution
    double A = (Low - Mean)/Sigma;
    double B = (High - Mean)/Sigma;
    double PhiA = exp(-0.5*A*A)/sqrt(2.0*c_Pi);
    double PhiB = exp(-0.5*B*B)/sqrt(2.0*c_Pi);
    double Z = GetPhi(B) - GetPhi(A);
    TruncatedMean = Mean + Sigma*(PhiA - PhiB)/Z;
    TruncatedRMS = Sigma*sqrt(1.0 + (A*PhiA - B*PhiB)/Z - (PhiA - PhiB)*(PhiA - PhiB)/(Z*Z));
  }

  //! Check the mean and the width of a noised energy against the configured Gauss table
  bool CheckGauss(const MString& Function, const MString& Input, const vector<double>& Values, double Mean, double Sigma)
  {
    bool Passed = true;
    Passed = EvaluateNear(Function, Input, "The mean agrees with the configured peak", MStatistics::GetMean(Values), Mean, GetMeanTolerance(Sigma)) && Passed;
    Passed = EvaluateNear(Function, Input, "The width agrees with the configured width", MStatistics::GetStandardDeviation(Values), Sigma, GetRMSTolerance(Sigma)) && Passed;
    return Passed;
  }

  //! Test the noise chain common to all detectors: the body must define the detector D with the sensitive volume Box containing the position
  //! SetsFlags is false for the detectors which are known not to flag failures and overflows (GEO-D-075)
  bool TestCommonNoiseChain(const MString& Body, const MVector& Position, bool SetsFlags = true)
  {
    bool Passed = true;
    Passed = TestNoiseGauss(Body, Position) && Passed;
    Passed = TestNoiseLorentz(Body, Position) && Passed;
    Passed = TestNoiseGaussLandau(Body, Position) && Passed;
    Passed = TestNoiseIdealAndNone(Body, Position) && Passed;
    Passed = TestNoiseOverflow(Body, Position) && Passed;
    Passed = TestNoiseThreshold(Body, Position) && Passed;
    Passed = TestNoiseTriggerThreshold(Body, Position) && Passed;
    Passed = TestNoiseFailureRate(Body, Position) && Passed;
    Passed = TestNoiseTimeResolution(Body, Position) && Passed;
    Passed = TestNoiseCalibration(Body, Position) && Passed;
    Passed = TestNoiseDeactivated(Body, Position) && Passed;
    Passed = TestNoiseFlags(Body, Position, SetsFlags) && Passed;
    Passed = TestNoiseEngineVersion(Body, Position) && Passed;
    return Passed;
  }

private:
  //! Test Gauss tables: at points, between points, outside points, with a shifted peak
  bool TestNoiseGauss(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1001);

    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "Gauss table", "The fixture with a Gauss table is accepted", Scan(Geometry, Body, "D.EnergyResolution Gauss 100 100 2\nD.EnergyResolution Gauss 500 500 6\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    MDDetector* D = Geometry.GetDetector("D");
    MDVolume* V = Geometry.GetVolume("Box");

    // Linear interpolation of width by hand: 2 + (E - 100)/100
    Passed = CheckGauss("Noise()", "Gauss, at first table point 100 keV", SampleEnergy(D, V, Position, 100.0), 100.0, 2.0) && Passed;
    Passed = CheckGauss("Noise()", "Gauss, at last table point 500 keV", SampleEnergy(D, V, Position, 500.0), 500.0, 6.0) && Passed;
    Passed = CheckGauss("Noise()", "Gauss, between the points at 300 keV", SampleEnergy(D, V, Position, 300.0), 300.0, 4.0) && Passed;
    Passed = CheckGauss("Noise()", "Gauss, above the table at 900 keV", SampleEnergy(D, V, Position, 900.0), 900.0, 10.0) && Passed;
    Passed = CheckGauss("Noise()", "Gauss, below the table at 50 keV", SampleEnergy(D, V, Position, 50.0), 50.0, 1.5) && Passed;

    Passed = EvaluateNear("GetEnergyResolution()", "Gauss, 300 keV", "The average resolution is the interpolated width", D->GetEnergyResolution(300.0, Position), 4.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionPeak1()", "Gauss, 300 keV", "The peak is interpolated", D->GetEnergyResolutionPeak1(300.0, Position), 300.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionWidth1()", "Gauss, 900 keV", "The width is extrapolated", D->GetEnergyResolutionWidth1(900.0, Position), 10.0, 1e-9) && Passed;

    // Peak shift: peak = 90 + 0.925*(E - 100)
    MDGeometry GeometryShift;
    Passed = EvaluateTrue("ScanSetupFile()", "shifted Gauss table", "The fixture with a shifted Gauss table is accepted", Scan(GeometryShift, Body, "D.EnergyResolution Gauss 100 90 2\nD.EnergyResolution Gauss 500 460 2\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    D = GeometryShift.GetDetector("D");
    V = GeometryShift.GetVolume("Box");
    Passed = CheckGauss("Noise()", "shifted Gauss at 100 keV", SampleEnergy(D, V, Position, 100.0), 90.0, 2.0) && Passed;
    Passed = CheckGauss("Noise()", "shifted Gauss at 300 keV", SampleEnergy(D, V, Position, 300.0), 275.0, 2.0) && Passed;

    // Width extrapolates below zero and is clamped to zero: width = 5 - 0.04*(E - 100) is -7 at 400 keV
    MDGeometry GeometryClamp;
    Passed = EvaluateTrue("ScanSetupFile()", "falling Gauss table", "The fixture with a falling width is accepted", Scan(GeometryClamp, Body, "D.EnergyResolution Gauss 100 100 5\nD.EnergyResolution Gauss 200 200 1\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    D = GeometryClamp.GetDetector("D");
    V = GeometryClamp.GetVolume("Box");
    Passed = EvaluateNear("Noise()", "width extrapolated below zero", "A negative extrapolated width is clamped to zero, so the energy is exact", NoiseOnce(D, V, Position, 400.0), 400.0, 1e-9) && Passed;

    // Noised energies below the zero noise threshold become zero: the mean of max(0, N(0.0, sigma)) is sigma/sqrt(2 pi)
    MDGeometry GeometryLow;
    Passed = EvaluateTrue("ScanSetupFile()", "Gauss table for low energies", "The fixture with a constant width is accepted", Scan(GeometryLow, Body, "D.EnergyResolution Gauss 0 0 5\nD.EnergyResolution Gauss 100 100 5\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    D = GeometryLow.GetDetector("D");
    V = GeometryLow.GetVolume("Box");
    vector<double> Low = SampleEnergy(D, V, Position, 0.0);
    Passed = EvaluateNear("Noise()", "0 keV with 5 keV width", "Half of the hits are noised to exactly zero", 1.0 - GetFractionAbove(Low, 0.0), 0.5, GetFractionTolerance(0.5)) && Passed;
    Passed = EvaluateNear("Noise()", "0 keV with 5 keV width", "The mean is the mean of the positive half of a Gaussian", MStatistics::GetMean(Low), 5.0/sqrt(2.0*c_Pi), GetMeanTolerance(5.0)) && Passed;

    return Passed;
  }

  //! Test a Lorentz table
  bool TestNoiseLorentz(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1002);

    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "Lorentz table", "The fixture with a Lorentz table is accepted", Scan(Geometry, Body, "D.EnergyResolution Lorentz 100 100 2\nD.EnergyResolution Lorentz 500 500 6\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    MDDetector* D = Geometry.GetDetector("D");
    MDVolume* V = Geometry.GetVolume("Box");

    // Truncated Lorentz with half width g = 2.35/2 sigma, sigma = 4 at 300 keV, cut where the height is 0.0025:
    // C/g = sqrt((1 - 0.0025)/0.0025); median: g tan(atan(C/g)/2); variance: g^2 ((C/g)/atan(C/g) - 1)
    double Gamma = 2.35/2.0*4.0;
    double CutOverGamma = sqrt((1.0 - 0.0025)/0.0025);
    double Median = Gamma*tan(0.5*atan(CutOverGamma));
    double RMS = Gamma*sqrt(CutOverGamma/atan(CutOverGamma) - 1.0);

    // Tolerances: the kurtosis of about 12 increases the error of the standard deviation by sqrt((12 - 1)/2) = 2.3 relative to a Gaussian,
    // the density at the median increases the error of the median to 1.45 g/sqrt(N)
    vector<double> Values = SampleEnergy(D, V, Position, 300.0);
    Passed = EvaluateNear("Noise()", "Lorentz at 300 keV", "The mean is the interpolated peak", MStatistics::GetMean(Values), 300.0, GetMeanTolerance(RMS)) && Passed;
    Passed = EvaluateNear("Noise()", "Lorentz at 300 keV", "The standard deviation is the one of the truncated Lorentzian", MStatistics::GetStandardDeviation(Values), RMS, 2.5*GetRMSTolerance(RMS)) && Passed;
    Passed = EvaluateNear("Noise()", "Lorentz at 300 keV", "The median distance is the one of the truncated Lorentzian", GetMedianDistance(Values, 300.0), Median, 1.5*GetMeanTolerance(Gamma)) && Passed;
    double Maximum = *max_element(Values.begin(), Values.end());
    Passed = EvaluateTrue("Noise()", "Lorentz at 300 keV", "No energy is beyond the cut-off", Maximum <= 300.0 + CutOverGamma*Gamma + 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolution()", "Lorentz, 300 keV", "The average resolution is the interpolated width", D->GetEnergyResolution(300.0, Position), 4.0, 1e-9) && Passed;

    return Passed;
  }

  //! Test a Gauss-Landau table
  bool TestNoiseGaussLandau(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1003);

    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "Gauss-Landau table", "The fixture with a Gauss-Landau table is accepted", Scan(Geometry, Body, "D.EnergyResolution GaussLandau 100 100 2 100 3 0.7\nD.EnergyResolution GaussLandau 900 900 2 900 3 0.7\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    MDDetector* D = Geometry.GetDetector("D");
    MDVolume* V = Geometry.GetVolume("Box");

    // Numeric integration of the distribution defined by the configured table
    double Mean = 0.0;
    double RMS = 0.0;
    GetGaussLandauMoments(500.0, 2.0, 500.0, 3.0, 0.7, Mean, RMS);

    // Tolerance: The Landau tail increases the kurtosis, the error of the standard deviation is about 2 times larger than for a Gaussian
    vector<double> Values = SampleEnergy(D, V, Position, 500.0);
    Passed = EvaluateNear("Noise()", "Gauss-Landau at 500 keV", "The mean agrees with the numeric integration of the distribution", MStatistics::GetMean(Values), Mean, GetMeanTolerance(RMS)) && Passed;
    Passed = EvaluateNear("Noise()", "Gauss-Landau at 500 keV", "The width agrees with the numeric integration of the distribution", MStatistics::GetStandardDeviation(Values), RMS, 2.0*GetRMSTolerance(RMS)) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionRatio()", "Gauss-Landau, 500 keV", "The ratio is interpolated", D->GetEnergyResolutionRatio(500.0, Position), 0.7, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionPeak2()", "Gauss-Landau, 500 keV", "The second peak is interpolated", D->GetEnergyResolutionPeak2(500.0, Position), 500.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionWidth2()", "Gauss-Landau, 500 keV", "The second width is interpolated", D->GetEnergyResolutionWidth2(500.0, Position), 3.0, 1e-9) && Passed;

    return Passed;
  }

  //! Test the types without randomization
  bool TestNoiseIdealAndNone(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1004);

    MDGeometry Ideal;
    Passed = EvaluateTrue("ScanSetupFile()", "ideal", "The fixture with an ideal energy resolution is accepted", Scan(Ideal, Body, "D.EnergyResolution Ideal\n")) && Passed;
    MDGeometry None;
    Passed = EvaluateTrue("ScanSetupFile()", "none", "The fixture with no energy resolution is accepted", Scan(None, Body, "D.EnergyResolution None\n")) && Passed;
    if (Passed == false) {
      return false;
    }

    Passed = EvaluateNear("Noise()", "ideal, 123.4 keV", "The energy stays unchanged", NoiseOnce(Ideal.GetDetector("D"), Ideal.GetVolume("Box"), Position, 123.4), 123.4, 1e-12) && Passed;
    Passed = EvaluateNear("GetEnergyResolution()", "ideal", "The ideal type has resolution zero", Ideal.GetDetector("D")->GetEnergyResolution(123.4, Position), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "none, 123.4 keV", "The type None measures no energy", NoiseOnce(None.GetDetector("D"), None.GetVolume("Box"), Position, 123.4), 0.0, 1e-12) && Passed;

    return Passed;
  }

  //! Test the overflow: checks just inside and outside, and the sigma
  bool TestNoiseOverflow(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1005);

    MDGeometry Sharp;
    Passed = EvaluateTrue("ScanSetupFile()", "overflow", "The fixture with a sharp overflow is accepted", Scan(Sharp, Body, "D.Overflow 200\n")) && Passed;
    MDGeometry Soft;
    Passed = EvaluateTrue("ScanSetupFile()", "overflow with sigma", "The fixture with a soft overflow is accepted", Scan(Soft, Body, "D.Overflow 200 5\n")) && Passed;
    if (Passed == false) {
      return false;
    }

    MDDetector* D = Sharp.GetDetector("D");
    MDVolume* V = Sharp.GetVolume("Box");
    Passed = EvaluateNear("GetOverflow()", "200 keV", "The overflow is stored", D->GetOverflow(), 200.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetOverflowSigma()", "no sigma", "The sigma of the overflow is zero by default", D->GetOverflowSigma(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "just below the overflow", "The energy stays unchanged", NoiseOnce(D, V, Position, 199.999), 199.999, 1e-9) && Passed;
    Passed = EvaluateNear("Noise()", "just above the overflow", "The energy is the overflow value", NoiseOnce(D, V, Position, 200.001), 200.0, 1e-9) && Passed;
    Passed = EvaluateNear("Noise()", "far above the overflow", "The energy is the overflow value", NoiseOnce(D, V, Position, 1E6), 200.0, 1e-9) && Passed;

    D = Soft.GetDetector("D");
    V = Soft.GetVolume("Box");
    Passed = EvaluateNear("GetOverflowSigma()", "sigma 5", "The sigma of the overflow is stored", D->GetOverflowSigma(), 5.0, 1e-12) && Passed;
    Passed = CheckGauss("Noise()", "far above a soft overflow", SampleEnergy(D, V, Position, 1000.0), 200.0, 5.0) && Passed;
    // E = 200 keV: min(200, N(200, 5)) has the mean 200 - sigma/sqrt(2 pi) and half of the hits are below 200 keV
    vector<double> Values = SampleEnergy(D, V, Position, 200.0);
    Passed = EvaluateNear("Noise()", "at a soft overflow", "The mean is reduced by sigma/sqrt(2 pi)", MStatistics::GetMean(Values), 200.0 - 5.0/sqrt(2.0*c_Pi), GetMeanTolerance(5.0)) && Passed;
    Passed = EvaluateNear("Noise()", "at a soft overflow", "Half of the hits are clipped", 1.0 - GetFractionAbove(Values, 200.0 - 1e-9), 0.5, GetFractionTolerance(0.5)) && Passed;

    return Passed;
  }

  //! Test the noise threshold: checks just inside and outside, and the sigma
  bool TestNoiseThreshold(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1006);

    MDGeometry Sharp;
    Passed = EvaluateTrue("ScanSetupFile()", "noise threshold", "The fixture with a sharp noise threshold is accepted", Scan(Sharp, Body, "D.NoiseThreshold 50\n")) && Passed;
    MDGeometry Soft;
    Passed = EvaluateTrue("ScanSetupFile()", "noise threshold with sigma", "The fixture with a soft noise threshold is accepted", Scan(Soft, Body, "D.NoiseThreshold 50 4\n")) && Passed;
    MDGeometry WithResolution;
    Passed = EvaluateTrue("ScanSetupFile()", "noise threshold after the energy resolution", "The fixture with a threshold and a resolution is accepted", Scan(WithResolution, Body, "D.NoiseThreshold 50\nD.EnergyResolution Gauss 0 0 5\nD.EnergyResolution Gauss 100 100 5\n")) && Passed;
    MDGeometry Equal;
    Passed = EvaluateTrue("ScanSetupFile()", "noise threshold equals trigger threshold", "The fixture with one threshold is accepted", Scan(Equal, Body, "D.NoiseThresholdEqualsTriggerThreshold true\nD.TriggerThreshold 30\n")) && Passed;
    if (Passed == false) {
      return false;
    }

    MDDetector* D = Sharp.GetDetector("D");
    MDVolume* V = Sharp.GetVolume("Box");
    Passed = EvaluateNear("Noise()", "just below the noise threshold", "The hit is removed", NoiseOnce(D, V, Position, 49.999), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "just above the noise threshold", "The hit stays", NoiseOnce(D, V, Position, 50.001), 50.001, 1e-9) && Passed;
    Passed = EvaluateNear("GetNoiseThresholdSigma()", "no sigma", "The default sigma is zero", D->GetNoiseThresholdSigma(), 0.0, 1e-12) && Passed;

    D = Soft.GetDetector("D");
    V = Soft.GetVolume("Box");
    // One sigma above the threshold survives with Phi(1), one sigma below with Phi(-1)
    Passed = EvaluateNear("Noise()", "one sigma above the soft threshold", "The surviving fraction is Phi(1)", GetFractionAbove(SampleEnergy(D, V, Position, 54.0), 0.0), GetPhi(1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;
    Passed = EvaluateNear("Noise()", "one sigma below the soft threshold", "The surviving fraction is Phi(-1)", GetFractionAbove(SampleEnergy(D, V, Position, 46.0), 0.0), GetPhi(-1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;

    // The threshold works on the noised energy: half of 50 keV hits survive, with a truncated Gaussian mean of 50 + sigma sqrt(2/pi)
    D = WithResolution.GetDetector("D");
    V = WithResolution.GetVolume("Box");
    vector<double> Values = SampleEnergy(D, V, Position, 50.0);
    Passed = EvaluateNear("Noise()", "threshold at the peak", "Half of the hits survive", GetFractionAbove(Values, 0.0), 0.5, GetFractionTolerance(0.5)) && Passed;
    Passed = EvaluateNear("Noise()", "threshold at the peak", "The surviving and removed hits average to half of the truncated mean", MStatistics::GetMean(Values), 0.5*(50.0 + 5.0*sqrt(2.0/c_Pi)), GetMeanTolerance(30.0)) && Passed;

    // The trigger threshold replaces the noise threshold:
    D = Equal.GetDetector("D");
    V = Equal.GetVolume("Box");
    Passed = EvaluateTrue("GetNoiseThresholdEqualsTriggerThreshold()", "flag set", "The flag is stored", D->GetNoiseThresholdEqualsTriggerThreshold()) && Passed;
    Passed = EvaluateNear("Noise()", "just below the combined threshold", "The hit is removed", NoiseOnce(D, V, Position, 29.999), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "just above the combined threshold", "The hit stays", NoiseOnce(D, V, Position, 30.001), 30.001, 1e-9) && Passed;

    return Passed;
  }

  //! Test the trigger threshold: checks just inside and outside, and the sigma
  bool TestNoiseTriggerThreshold(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1007);

    MDGeometry Sharp;
    Passed = EvaluateTrue("ScanSetupFile()", "trigger threshold", "The fixture with a sharp trigger threshold is accepted", Scan(Sharp, Body, "D.TriggerThreshold 30\n")) && Passed;
    MDGeometry Soft;
    Passed = EvaluateTrue("ScanSetupFile()", "trigger threshold with sigma", "The fixture with a soft trigger threshold is accepted", Scan(Soft, Body, "D.TriggerThreshold 30 3\n")) && Passed;
    MDGeometry Equal;
    Passed = EvaluateTrue("ScanSetupFile()", "noise threshold equals trigger threshold", "The fixture with one threshold is accepted", Scan(Equal, Body, "D.NoiseThresholdEqualsTriggerThreshold true\nD.TriggerThreshold 30\n")) && Passed;
    if (Passed == false) {
      return false;
    }

    MDDetector* D = Sharp.GetDetector("D");
    MDGridPoint Point = D->GetGridPoint(Position);
    Passed = EvaluateFalse("IsAboveTriggerThreshold()", "just below", "A hit just below the threshold does not trigger", D->IsAboveTriggerThreshold(29.999, Point)) && Passed;
    Passed = EvaluateTrue("IsAboveTriggerThreshold()", "just above", "A hit just above the threshold triggers", D->IsAboveTriggerThreshold(30.001, Point)) && Passed;

    D = Soft.GetDetector("D");
    Point = D->GetGridPoint(Position);
    double Above = 0.0;
    double Below = 0.0;
    for (unsigned int i = 0; i < c_Samples; ++i) {
      if (D->IsAboveTriggerThreshold(33.0, Point) == true) {
        Above += 1.0;
      }
      if (D->IsAboveTriggerThreshold(27.0, Point) == true) {
        Below += 1.0;
      }
    }
    Passed = EvaluateNear("IsAboveTriggerThreshold()", "one sigma above", "The triggered fraction is Phi(1)", Above/c_Samples, GetPhi(1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;
    Passed = EvaluateNear("IsAboveTriggerThreshold()", "one sigma below", "The triggered fraction is Phi(-1)", Below/c_Samples, GetPhi(-1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;

    // The noise threshold was already applied in this mode, thus every hit passed to the trigger triggers
    D = Equal.GetDetector("D");
    Point = D->GetGridPoint(Position);
    Passed = EvaluateTrue("IsAboveTriggerThreshold()", "noise threshold equals trigger threshold", "Every remaining hit triggers", D->IsAboveTriggerThreshold(1.0, Point)) && Passed;

    return Passed;
  }

  //! Test the failure rate
  bool TestNoiseFailureRate(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1008);

    MDGeometry Quarter;
    Passed = EvaluateTrue("ScanSetupFile()", "failure rate 0.25", "The fixture with a failure rate is accepted", Scan(Quarter, Body, "D.FailureRate 0.25\n")) && Passed;
    MDGeometry All;
    Passed = EvaluateTrue("ScanSetupFile()", "failure rate 1", "The fixture with a failure rate of one is accepted", Scan(All, Body, "D.FailureRate 1.0\n")) && Passed;
    MDGeometry Zero;
    Passed = EvaluateTrue("ScanSetupFile()", "failure rate 0", "The fixture with a failure rate of zero is accepted", Scan(Zero, Body, "D.FailureRate 0.0\n")) && Passed;
    if (Passed == false) {
      return false;
    }

    Passed = EvaluateNear("GetFailureRate()", "0.25", "The failure rate is stored", Quarter.GetDetector("D")->GetFailureRate(), 0.25, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "failure rate 0.25", "Three quarters of the hits survive", GetFractionAbove(SampleEnergy(Quarter.GetDetector("D"), Quarter.GetVolume("Box"), Position, 100.0), 0.0), 0.75, GetFractionTolerance(0.75)) && Passed;
    Passed = EvaluateNear("Noise()", "failure rate 1", "No hit survives", GetFractionAbove(SampleEnergy(All.GetDetector("D"), All.GetVolume("Box"), Position, 100.0), 0.0), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "failure rate 0", "All hits survive", GetFractionAbove(SampleEnergy(Zero.GetDetector("D"), Zero.GetVolume("Box"), Position, 100.0), 0.0), 1.0, 1e-12) && Passed;

    return Passed;
  }

  //! Test the time resolution
  bool TestNoiseTimeResolution(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1009);

    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "time resolution table", "The fixture with a time resolution is accepted", Scan(Geometry, Body, "D.TimeResolution 100 2\nD.TimeResolution 500 6\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    MDDetector* D = Geometry.GetDetector("D");
    MDVolume* V = Geometry.GetVolume("Box");

    Passed = EvaluateTrue("HasTimeResolution()", "table", "A time resolution exists", D->HasTimeResolution()) && Passed;
    Passed = EvaluateNear("GetTimeResolution()", "300 keV", "The time resolution is interpolated", D->GetTimeResolution(300.0), 4.0, 1e-9) && Passed;
    Passed = CheckGauss("Noise()", "time at 100 keV", SampleTime(D, V, Position, 100.0, 1000.0), 1000.0, 2.0) && Passed;
    Passed = CheckGauss("Noise()", "time at 300 keV", SampleTime(D, V, Position, 300.0, 1000.0), 1000.0, 4.0) && Passed;
    Passed = CheckGauss("Noise()", "time at 900 keV above the table", SampleTime(D, V, Position, 900.0, 1000.0), 1000.0, 10.0) && Passed;

    return Passed;
  }

  //! Test that the calibration is the last step of the chain
  bool TestNoiseCalibration(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1010);

    const MString FileName = GetTemporaryFileName("Calibration.dat");
    Passed = EvaluateTrue("WriteTextFile()", "calibration", "The calibration file can be created", WriteTextFile(FileName, "IP LinLin\nDP 0 0\nDP 1000 2000\nEN\n")) && Passed;

    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "calibration", "The fixture with a calibration is accepted", Scan(Geometry, Body, MString("D.NoiseThreshold 50\nD.EnergyCalibration ") + FileName + "\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    MDDetector* D = Geometry.GetDetector("D");
    MDVolume* V = Geometry.GetVolume("Box");

    // Calibration by hand: E' = 2 E, the threshold is in front of the calibration
    Passed = EvaluateNear("Noise()", "calibration above the threshold", "The energy is calibrated", NoiseOnce(D, V, Position, 60.0), 120.0, 1e-9) && Passed;
    Passed = EvaluateNear("Noise()", "calibration below the threshold", "The threshold is applied in front of the calibration", NoiseOnce(D, V, Position, 40.0), 0.0, 1e-12) && Passed;

    return Passed;
  }

  //! Test that nothing happens if the noising is deactivated
  bool TestNoiseDeactivated(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    MString Flags;
    gRandom->SetSeed(1011);

    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "deactivation", "The fixture with noise settings is accepted", Scan(Geometry, Body, "D.EnergyResolution Gauss 100 100 2\nD.EnergyResolution Gauss 500 500 6\nD.NoiseThreshold 50\nD.FailureRate 1.0\nD.TimeResolution 100 2\n")) && Passed;
    if (Passed == false) {
      return false;
    }
    MDDetector* D = Geometry.GetDetector("D");
    D->ActivateNoising(false);

    MVector P = Position;
    double E = 10.0;
    double T = 77.0;
    D->Noise(P, E, T, Flags, Geometry.GetVolume("Box"));
    Passed = EvaluateNear("ActivateNoising()", "false, energy", "The energy stays unchanged", E, 10.0, 1e-12) && Passed;
    Passed = EvaluateNear("ActivateNoising()", "false, time", "The time stays unchanged", T, 77.0, 1e-12) && Passed;
    Passed = EvaluateVectorNear("ActivateNoising()", "false, position", "The position stays unchanged", P, Position, 1e-12) && Passed;

    return Passed;
  }

  //! Test the flags which the noising sets for the detector effects engine
  bool TestNoiseFlags(const MString& Body, const MVector& Position, bool SetsFlags)
  {
    bool Passed = true;
    gRandom->SetSeed(1012);

    MDGeometry Overflow;
    Passed = EvaluateTrue("ScanSetupFile()", "flags, overflow", "The fixture with an overflow is accepted", Scan(Overflow, Body, "D.Overflow 200\nD.FailureRate 0.0\n")) && Passed;
    MDGeometry Failure;
    Passed = EvaluateTrue("ScanSetupFile()", "flags, failure rate 1", "The fixture with a failure rate of one is accepted", Scan(Failure, Body, "D.FailureRate 1.0\n")) && Passed;
    if (Passed == false) {
      return false;
    }

    MDDetector* D = Overflow.GetDetector("D");
    MDVolume* V = Overflow.GetVolume("Box");
    MString Normal = NoiseFlagsOnce(D, V, Position, 100.0);
    Passed = EvaluateFalse("Noise()", "flags, 100 keV with an overflow of 200 keV", "There is no overflow flag", Normal.Contains("OVERFLOW")) && Passed;
    Passed = EvaluateFalse("Noise()", "flags, failure rate 0", "There is no failure flag", Normal.Contains("FAILURE")) && Passed;

    MString AboveOverflow = NoiseFlagsOnce(D, V, Position, 1000.0);
    MString Kept = NoiseFlagsOnce(D, V, Position, 1000.0, " EARLIER");
    MString Failed = NoiseFlagsOnce(Failure.GetDetector("D"), Failure.GetVolume("Box"), Position, 100.0);
    if (SetsFlags == true) {
      Passed = EvaluateTrue("Noise()", "flags, 1000 keV with an overflow of 200 keV", "The overflow is flagged", AboveOverflow.Contains(" OVERFLOW")) && Passed;
      Passed = EvaluateFalse("Noise()", "flags, 1000 keV with an overflow of 200 keV", "There is no failure flag", AboveOverflow.Contains("FAILURE")) && Passed;
      Passed = EvaluateTrue("Noise()", "flags, earlier flag", "The new flag is added to the flags which are already there", Kept.BeginsWith(" EARLIER") && Kept.Contains(" OVERFLOW")) && Passed;
      Passed = EvaluateTrue("Noise()", "flags, failure rate 1", "The failure is flagged", Failed == " FAILURE") && Passed;
    } else {
      // GEO-D-075: This detector does not flag failures and overflows as the other detectors do
      Passed = ExpectedToFail(EvaluateTrue("Noise()", "flags, 1000 keV with an overflow of 200 keV", "The overflow is flagged", AboveOverflow.Contains(" OVERFLOW"))) && Passed;
      Passed = ExpectedToFail(EvaluateTrue("Noise()", "flags, failure rate 1", "The failure is flagged", Failed == " FAILURE")) && Passed;
    }

    // Nothing is flagged if the noising is deactivated:
    Failure.GetDetector("D")->ActivateNoising(false);
    Passed = EvaluateTrue("Noise()", "flags, noising deactivated", "The flags stay empty", NoiseFlagsOnce(Failure.GetDetector("D"), Failure.GetVolume("Box"), Position, 100.0).IsEmpty()) && Passed;

    return Passed;
  }

  //! Test the differences of the noising between the detector effects engines 1 and 2: the new engine applies the noise threshold itself
  bool TestNoiseEngineVersion(const MString& Body, const MVector& Position)
  {
    bool Passed = true;
    gRandom->SetSeed(1013);

    const MString Engine = "DetectorEffectsEngine 2\n";
    MDGeometry Old;
    Passed = EvaluateTrue("ScanSetupFile()", "engine 1, thresholds", "The fixture is accepted", Scan(Old, Body, "D.NoiseThreshold 50\nD.TriggerThreshold 80\n")) && Passed;
    MDGeometry New;
    Passed = EvaluateTrue("ScanSetupFile()", "engine 2, thresholds", "The fixture is accepted", Scan(New, Body, Engine + "D.NoiseThreshold 50\nD.TriggerThreshold 80\n")) && Passed;
    MDGeometry Soft;
    Passed = EvaluateTrue("ScanSetupFile()", "engine 2, noise threshold with sigma", "The fixture is accepted", Scan(Soft, Body, Engine + "D.NoiseThreshold 50 4\nD.TriggerThreshold 80\n")) && Passed;
    MDGeometry OldEqual;
    Passed = EvaluateTrue("ScanSetupFile()", "engine 1, noise threshold equals trigger threshold", "The fixture is accepted", Scan(OldEqual, Body, "D.NoiseThresholdEqualsTriggerThreshold true\nD.TriggerThreshold 30\n")) && Passed;
    MDGeometry NewEqual;
    Passed = EvaluateTrue("ScanSetupFile()", "engine 2, noise threshold equals trigger threshold", "The fixture is accepted", Scan(NewEqual, Body, Engine + "D.NoiseThresholdEqualsTriggerThreshold true\nD.TriggerThreshold 30\n")) && Passed;
    MDGeometry Gauss;
    Passed = EvaluateTrue("ScanSetupFile()", "engine 2, Gauss table", "The fixture is accepted", Scan(Gauss, Body, Engine + "D.EnergyResolution Gauss 100 100 2\nD.EnergyResolution Gauss 500 500 6\n")) && Passed;
    if (Passed == false) {
      return false;
    }

    // The version of the geometry arrives at the detector:
    MDDetector* D = Old.GetDetector("D");
    MDVolume* V = Old.GetVolume("Box");
    MDGridPoint Point = D->GetGridPoint(Position);
    Passed = Evaluate("GetDetectorEffectsEngineVersion()", "no keyword", "The detector uses the engine 1", D->GetDetectorEffectsEngineVersion(), 1U) && Passed;
    Passed = EvaluateNear("Noise()", "engine 1, just below the noise threshold", "The hit is removed", NoiseOnce(D, V, Position, 49.999), 0.0, 1e-12) && Passed;
    Passed = EvaluateFalse("IsAboveNoiseThreshold()", "engine 1, just below", "A hit just below the noise threshold is not above it", D->IsAboveNoiseThreshold(49.999, Point)) && Passed;
    Passed = EvaluateTrue("IsAboveNoiseThreshold()", "engine 1, just above", "A hit just above the noise threshold is above it", D->IsAboveNoiseThreshold(50.001, Point)) && Passed;

    D = New.GetDetector("D");
    V = New.GetVolume("Box");
    Point = D->GetGridPoint(Position);
    Passed = Evaluate("GetDetectorEffectsEngineVersion()", "DetectorEffectsEngine 2", "The detector uses the engine 2", D->GetDetectorEffectsEngineVersion(), 2U) && Passed;
    Passed = EvaluateNear("Noise()", "engine 2, just below the noise threshold", "The hit stays: the engine applies the noise threshold", NoiseOnce(D, V, Position, 49.999), 49.999, 1e-9) && Passed;
    Passed = EvaluateNear("Noise()", "engine 2, just above the noise threshold", "The hit stays", NoiseOnce(D, V, Position, 50.001), 50.001, 1e-9) && Passed;
    Passed = EvaluateFalse("IsAboveNoiseThreshold()", "engine 2, just below", "A hit just below the noise threshold is not above it", D->IsAboveNoiseThreshold(49.999, Point)) && Passed;
    Passed = EvaluateTrue("IsAboveNoiseThreshold()", "engine 2, just above", "A hit just above the noise threshold is above it", D->IsAboveNoiseThreshold(50.001, Point)) && Passed;
    Passed = EvaluateFalse("IsAboveTriggerThreshold()", "engine 2, between the thresholds", "A hit between noise and trigger threshold does not trigger", D->IsAboveTriggerThreshold(79.999, Point)) && Passed;
    Passed = EvaluateTrue("IsAboveTriggerThreshold()", "engine 2, just above", "A hit just above the trigger threshold triggers", D->IsAboveTriggerThreshold(80.001, Point)) && Passed;

    // One sigma above the soft threshold is above it with Phi(1), one sigma below with Phi(-1)
    D = Soft.GetDetector("D");
    Point = D->GetGridPoint(Position);
    double Above = 0.0;
    double Below = 0.0;
    for (unsigned int i = 0; i < c_Samples; ++i) {
      if (D->IsAboveNoiseThreshold(54.0, Point) == true) {
        Above += 1.0;
      }
      if (D->IsAboveNoiseThreshold(46.0, Point) == true) {
        Below += 1.0;
      }
    }
    Passed = EvaluateNear("IsAboveNoiseThreshold()", "one sigma above", "The fraction above is Phi(1)", Above/c_Samples, GetPhi(1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;
    Passed = EvaluateNear("IsAboveNoiseThreshold()", "one sigma below", "The fraction above is Phi(-1)", Below/c_Samples, GetPhi(-1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;

    // One common threshold: the old engine applies it in the noising and each remaining hit triggers, the new engine keeps the hit and applies it as trigger threshold
    D = OldEqual.GetDetector("D");
    V = OldEqual.GetVolume("Box");
    Point = D->GetGridPoint(Position);
    Passed = EvaluateNear("Noise()", "engine 1, just below the common threshold", "The hit is removed", NoiseOnce(D, V, Position, 29.999), 0.0, 1e-12) && Passed;
    Passed = EvaluateTrue("IsAboveTriggerThreshold()", "engine 1, common threshold", "Every remaining hit triggers", D->IsAboveTriggerThreshold(1.0, Point)) && Passed;

    D = NewEqual.GetDetector("D");
    V = NewEqual.GetVolume("Box");
    Point = D->GetGridPoint(Position);
    Passed = EvaluateNear("Noise()", "engine 2, just below the common threshold", "The hit stays", NoiseOnce(D, V, Position, 29.999), 29.999, 1e-9) && Passed;
    Passed = EvaluateFalse("IsAboveTriggerThreshold()", "engine 2, just below the common threshold", "The hit does not trigger", D->IsAboveTriggerThreshold(29.999, Point)) && Passed;
    Passed = EvaluateTrue("IsAboveTriggerThreshold()", "engine 2, just above the common threshold", "The hit triggers", D->IsAboveTriggerThreshold(30.001, Point)) && Passed;

    // The sigmas above the noise level are the energy in units of the interpolated width: 4 keV at 300 keV
    D = Gauss.GetDetector("D");
    Point = D->GetGridPoint(Position);
    Passed = EvaluateNear("SigmasAboveNoiseLevel()", "300 keV with a width of 4 keV", "The energy is 75 sigma above the noise level", D->SigmasAboveNoiseLevel(300.0, Point), 75.0, 1e-9) && Passed;
    Passed = EvaluateNear("SigmasAboveNoiseLevel()", "0 keV", "No energy is zero sigma above the noise level", D->SigmasAboveNoiseLevel(0.0, Point), 0.0, 1e-12) && Passed;

    // Without a Gauss table there is no noise level:
    D = New.GetDetector("D");
    Point = D->GetGridPoint(Position);
    DisableDefaultStreams();
    double Sigmas = D->SigmasAboveNoiseLevel(300.0, Point);
    EnableDefaultStreams();
    Passed = EvaluateNear("SigmasAboveNoiseLevel()", "no Gauss table", "The result is zero", Sigmas, 0.0, 1e-12) && Passed;

    // A channel which is blocked from triggering never triggers, but its hits are still above the noise threshold
    // Attention: Only channels with the z index 0 can be blocked
    D = New.GetDetector("D");
    Point = D->GetGridPoint(Position);
    Point = MDGridPoint(Point.GetXGrid(), Point.GetYGrid(), 0, Point.GetType(), Point.GetPosition());
    DisableDefaultStreams();
    bool IsBlocked = D->BlockTriggerChannel(Point.GetXGrid(), Point.GetYGrid());
    EnableDefaultStreams();
    Passed = EvaluateTrue("BlockTriggerChannel()", "channel of the hit", "The channel can be blocked", IsBlocked) && Passed;
    Passed = EvaluateFalse("IsAboveTriggerThreshold()", "engine 2, blocked channel", "A hit in a blocked channel does not trigger", D->IsAboveTriggerThreshold(1000.0, Point)) && Passed;
    // Defect GEO-D-077: IsAboveNoiseThreshold() has the check for blocked trigger channels of IsAboveTriggerThreshold(), so the new engine never reads out a blocked channel
    Passed = ExpectedToFail(EvaluateTrue("IsAboveNoiseThreshold()", "engine 2, blocked channel", "A hit in a channel which is blocked from triggering is still above the noise threshold", D->IsAboveNoiseThreshold(1000.0, Point))) && Passed;

    return Passed;
  }
};


#endif


// UTDDetectorShared.h: the end...
////////////////////////////////////////////////////////////////////////////////
