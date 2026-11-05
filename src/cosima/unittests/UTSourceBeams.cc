/*
 * UTSourceBeams.cc
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
#include "MCSource.hh"
#include "UTSDHitShared.h"
#include "UTSourceShared.h"

// MEGAlib:
#include "MUnitTest.h"

// Standard lib:
#include <algorithm>
#include <cmath>
#include <sstream>
#include <functional>
#include <string>
#include <utility>
#include <vector>
using namespace std;


//! Unit test class for the generation of start positions and directions of all beam types of MCSource:
//! Every beam is sampled with a fixed seed and compared with the independently derived geometry and distributions
class UTSourceBeams : public UTSourceTestBase
{
public:
  //! Default constructor
  UTSourceBeams() : UTSourceTestBase("UTSourceBeams") {}
  //! Default destructor
  virtual ~UTSourceBeams() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One generated start position and momentum direction
  struct Sample {
    //! The start position
    G4ThreeVector m_Position;
    //! The momentum direction
    G4ThreeVector m_Direction;
  };

  //! The number of samples
  static const unsigned int c_Samples = 20000;
  //! The radius of the start sphere
  static constexpr double c_Radius = 30*cm;

  //! Set the position parameters of a source from a vector
  static bool SetPositionFrom(MCSource& S, const vector<double>& P)
  {
    vector<double> Q = P;
    Q.resize(14, MCSource::c_Invalid);
    return S.SetPosition(Q[0], Q[1], Q[2], Q[3], Q[4], Q[5], Q[6], Q[7], Q[8], Q[9], Q[10], Q[11], Q[12], Q[13]);
  }

  //! Configure a complete source: beam, position parameters, flux, start area (in this order, since other orders abort)
  bool Configure(MCSource& S, int Coordinates, int Beam, const vector<double>& P, int Area = MCSource::c_StartAreaSphere, const MString& Volume = "", const MString& File = "")
  {
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(511*keV) && OK;
    OK = S.SetBeamType(Coordinates, Beam) && OK;
    if (Volume != "") S.SetVolume(Volume);
    OK = SetPositionFrom(S, P) && OK;
    if (File != "") OK = S.SetPosition(File) && OK;
    OK = S.SetFlux(1.0/(cm*cm*second)) && OK;
    OK = S.SetStartAreaType(Area) && OK;
    if (Area == MCSource::c_StartAreaTube) {
      OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20*cm, 40*cm) && OK;
    } else {
      OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, c_Radius) && OK;
    }
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    return OK;
  }

  //! Generate N positions and directions
  vector<Sample> Generate(MCSource& S, unsigned int N, bool* AllOK = nullptr)
  {
    vector<Sample> Samples;
    bool OK = true;
    SilenceOutput();
    for (unsigned int i = 0; i < N; ++i) {
      if (S.GeneratePosition(m_Gun) == false) {
        OK = false;
        break;
      }
      Samples.push_back({ GetGunPosition(), GetGunDirection() });
    }
    RestoreOutput();
    if (AllOK != nullptr) *AllOK = OK;
    return Samples;
  }

  //! Check a KS test and return the result
  bool KS(const MString& Test, const MString& Name, const MString& Description, const vector<double>& Values, const function<double(double)>& Density, double A, double B)
  {
    const double D = KSDistance(Values, Density, A, B);
    return EvaluateNear(Test, Name + ", KS " + D, Description, D, 0.0, KSCritical(Values.size()));
  }

  //! The azimuth in [0, 2pi)
  static double Azimuth(const G4ThreeVector& V)
  {
    double Phi = atan2(V.y(), V.x());
    return (Phi < 0) ? Phi + 2*c_Pi : Phi;
  }

  //! The area of the start tube for the zenith angle Theta
  static double TubeArea(double Theta) { return 4*(20*cm)*((40*cm)*sin(Theta) + (20*cm)*fabs(cos(Theta))); }

  bool TestFarFieldPoint();
  bool TestFarFieldAreas();
  bool TestFarFieldGaussians();
  bool TestFarFieldFile();
  bool TestFarFieldStartArea();
  bool TestNearFieldShapes();
  bool TestNearFieldBeams();
  bool TestNearFieldIlluminated();
  bool TestNearFieldVolume();
  bool TestFanBeam();
  bool TestNearFieldMaps();
  bool TestReverseDirection();
  bool TestDiffractionPoint();
  bool TestNormalizedEnergyBeamFlux();
  bool TestRadialProfile();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("InitializeEnvironment()", "beams", "The Geant4 environment can be created", InitializeEnvironment()) && Passed;

  Passed = TestFarFieldPoint() && Passed;
  Passed = TestFarFieldAreas() && Passed;
  Passed = TestFarFieldGaussians() && Passed;
  Passed = TestFarFieldFile() && Passed;
  Passed = TestFarFieldStartArea() && Passed;
  Passed = TestNearFieldShapes() && Passed;
  Passed = TestNearFieldBeams() && Passed;
  Passed = TestFanBeam() && Passed;
  Passed = TestNearFieldMaps() && Passed;
  Passed = TestReverseDirection() && Passed;
  Passed = TestDiffractionPoint() && Passed;
  Passed = TestNormalizedEnergyBeamFlux() && Passed;
  Passed = TestNearFieldIlluminated() && Passed;
  Passed = TestNearFieldVolume() && Passed;
  Passed = TestRadialProfile() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestFarFieldPoint()
{
  bool Passed = true;
  RandomGuard Guard(11);

  MCSource S("Far point");
  Passed = EvaluateTrue("Configure()", "far field point", "The source can be configured", Configure(S, MCSource::c_FarField, MCSource::c_FarFieldPoint, { 30*deg, 45*deg })) && Passed;
  bool OK = false;
  vector<Sample> Samples = Generate(S, 2000, &OK);
  Passed = EvaluateTrue("GeneratePosition()", "far field point", "The generation succeeds", OK) && Passed;

  const G4ThreeVector Source(sin(30*deg)*cos(45*deg), sin(30*deg)*sin(45*deg), cos(30*deg));
  double MaxDirection = 0.0, MaxPlane = 0.0, MaxOutside = 0.0;
  vector<double> Radii;
  for (const Sample& P: Samples) {
    // The particle flies from the source direction towards the center of the start sphere:
    MaxDirection = max(MaxDirection, (P.m_Direction + Source).mag());
    // The position is on the plane perpendicular to the direction, at distance R from the center, and on a disk with radius R around the axis:
    const double Along = P.m_Position.dot(Source);
    MaxPlane = max(MaxPlane, fabs(Along - c_Radius));
    const double Perpendicular = (P.m_Position - Along*Source).mag();
    MaxOutside = max(MaxOutside, Perpendicular/c_Radius);
    Radii.push_back(Perpendicular*Perpendicular/(c_Radius*c_Radius));
  }
  Passed = EvaluateNear("GeneratePosition()", "far field point, direction", "The direction points from the source direction to the center", MaxDirection, 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "far field point, plane", "The start positions are on the plane at the distance of the start sphere radius", MaxPlane/cm, 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "far field point, disk", "The start positions are inside the disk of the start sphere (shown: the excess over the limit)", max(MaxOutside - (1.0), 0.0), 0.0, 1e-9) && Passed;
  Passed = KS("GeneratePosition()", "far field point, disk radius", "The start positions are uniformly distributed on the disk (r^2 uniform)", Radii, [](double) { return 1.0; }, 0.0, 1.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestFarFieldAreas()
{
  bool Passed = true;

  const double Theta1 = 20*deg;
  const double Theta2 = 70*deg;
  const double Phi1 = 30*deg;
  const double Phi2 = 200*deg;

  // Area source, sphere and tube:
  for (int Area: { MCSource::c_StartAreaSphere, MCSource::c_StartAreaTube }) {
    RandomGuard Guard(21 + Area);
    const MString Name = (Area == MCSource::c_StartAreaSphere) ? "area, sphere" : "area, tube";
    MCSource S("Far area");
    Passed = EvaluateTrue("Configure()", Name, "The source can be configured", Configure(S, MCSource::c_FarField, MCSource::c_FarFieldArea, { Theta1, Theta2, Phi1, Phi2 }, Area)) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", OK) && Passed;
    vector<double> Thetas, Phis;
    for (const Sample& P: Samples) {
      const G4ThreeVector Source = -P.m_Direction;
      Thetas.push_back(Source.theta());
      Phis.push_back(Azimuth(Source));
    }
    if (Area == MCSource::c_StartAreaSphere) {
      Passed = KS("GeneratePosition()", Name + ", theta", "The zenith angles are isotropically distributed in the range", Thetas, [](double T) { return sin(T); }, Theta1, Theta2) && Passed;
    } else {
      Passed = KS("GeneratePosition()", Name + ", theta", "The zenith angles are distributed as sin(theta) times the projected area of the tube", Thetas, [](double T) { return sin(T)*TubeArea(T); }, Theta1, Theta2) && Passed;
    }
    Passed = KS("GeneratePosition()", Name + ", phi", "The azimuth angles are uniformly distributed in the range", Phis, [](double) { return 1.0; }, Phi1, Phi2) && Passed;
  }

  // Isotropic:
  for (int Area: { MCSource::c_StartAreaSphere, MCSource::c_StartAreaTube }) {
    RandomGuard Guard(31 + Area);
    const MString Name = (Area == MCSource::c_StartAreaSphere) ? "isotropic, sphere" : "isotropic, tube";
    MCSource S("Far isotropic");
    Passed = EvaluateTrue("Configure()", Name, "The source can be configured", Configure(S, MCSource::c_FarField, MCSource::c_FarFieldIsotropic, { }, Area)) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", OK) && Passed;
    vector<double> Thetas, Phis;
    for (const Sample& P: Samples) {
      const G4ThreeVector Source = -P.m_Direction;
      Thetas.push_back(Source.theta());
      Phis.push_back(Azimuth(Source));
    }
    if (Area == MCSource::c_StartAreaSphere) {
      Passed = KS("GeneratePosition()", Name + ", theta", "The zenith angles are isotropically distributed", Thetas, [](double T) { return sin(T); }, 0.0, c_Pi) && Passed;
    } else {
      Passed = KS("GeneratePosition()", Name + ", theta", "The zenith angles are distributed as sin(theta) times the projected area of the tube", Thetas, [](double T) { return sin(T)*TubeArea(T); }, 0.0, c_Pi) && Passed;
    }
    Passed = KS("GeneratePosition()", Name + ", phi", "The azimuth angles are uniformly distributed over the full circle", Phis, [](double) { return 1.0; }, 0.0, 2*c_Pi) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestFarFieldGaussians()
{
  bool Passed = true;

  const double Theta0 = 60*deg;
  const double Phi0 = 40*deg;
  const G4ThreeVector Center(sin(Theta0)*cos(Phi0), sin(Theta0)*sin(Phi0), cos(Theta0));

  // Gaussian:
  {
    RandomGuard Guard(41);
    const double Sigma = 30*deg;
    MCSource S("Far gaussian");
    Passed = EvaluateTrue("Configure()", "gaussian", "The source can be configured", Configure(S, MCSource::c_FarField, MCSource::c_FarFieldGaussian, { Theta0, Phi0, Sigma })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "gaussian", "The generation succeeds", OK) && Passed;
    vector<double> Angles;
    G4ThreeVector Mean;
    for (const Sample& P: Samples) {
      const G4ThreeVector Source = -P.m_Direction;
      Angles.push_back(Source.angle(Center));
      Mean += Source;
    }
    Mean /= Samples.size();
    // The mean direction is tilted towards the center, the perpendicular components vanish. For 20000 samples each of the two perpendicular
    // components of the mean has a standard deviation of ~0.4/sqrt(20000) = 0.003, the length (Rayleigh) is below 0.01 with a probability of 99.8%:
    Passed = EvaluateNear("GeneratePosition()", "gaussian, symmetry", "The distribution is symmetric around the center", (Mean - Mean.dot(Center)*Center).mag(), 0.0, 0.01) && Passed;
    // The density in the angle from the center is sin(psi)*exp(-psi^2/(2 sigma^2)), the sampling goes to five sigma:
    Passed = KS("GeneratePosition()", "gaussian, angle", "The angles from the center follow sin(psi)*exp(-psi^2/2sigma^2)", Angles, [Sigma](double A) { return sin(A)*exp(-A*A/(2*Sigma*Sigma)); }, 0.0, 5*Sigma) && Passed;
  }

  // Disk:
  {
    RandomGuard Guard(42);
    const double Radius = 25*deg;
    MCSource S("Far disk");
    Passed = EvaluateTrue("Configure()", "disk", "The source can be configured", Configure(S, MCSource::c_FarField, MCSource::c_FarFieldDisk, { Theta0, Phi0, Radius })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "disk", "The generation succeeds", OK) && Passed;
    vector<double> Angles;
    G4ThreeVector Mean;
    for (const Sample& P: Samples) {
      const G4ThreeVector Source = -P.m_Direction;
      Angles.push_back(Source.angle(Center));
      Mean += Source;
    }
    Mean /= Samples.size();
    Passed = EvaluateNear("GeneratePosition()", "disk, symmetry", "The distribution is symmetric around the center", (Mean - Mean.dot(Center)*Center).mag(), 0.0, 0.01) && Passed;
    Passed = KS("GeneratePosition()", "disk, angle", "The angles from the center are uniformly distributed on the cap (sin(psi))", Angles, [](double A) { return sin(A); }, 0.0, Radius) && Passed;
  }

  // Assymetric Gaussian: sigma phi = 8 deg, sigma theta = 3 deg, rotation 0 and 90 deg
  {
    const double SigmaPhi = 8*deg;
    const double SigmaTheta = 3*deg;
    const G4ThreeVector EastTangent = G4ThreeVector(-sin(Phi0), cos(Phi0), 0.0);
    const G4ThreeVector SouthTangent = G4ThreeVector(cos(Theta0)*cos(Phi0), cos(Theta0)*sin(Phi0), -sin(Theta0));
    vector<double> StdPhi, StdTheta;
    for (double Rotation: { 0.0, 90*deg }) {
      RandomGuard Guard(43 + (unsigned int) (Rotation/deg));
      MCSource S("Far assymetric gaussian");
      Passed = EvaluateTrue("Configure()", "assymetric gaussian", "The source can be configured", Configure(S, MCSource::c_FarField, MCSource::c_FarFieldAssymetricGaussian, { Theta0, Phi0, SigmaPhi, SigmaTheta, Rotation })) && Passed;
      bool OK = false;
      vector<Sample> Samples = Generate(S, c_Samples, &OK);
      Passed = EvaluateTrue("GeneratePosition()", "assymetric gaussian", "The generation succeeds", OK) && Passed;
      vector<double> East, South;
      G4ThreeVector Mean;
      for (const Sample& P: Samples) {
        const G4ThreeVector Source = -P.m_Direction;
        East.push_back(Source.dot(EastTangent));
        South.push_back(Source.dot(SouthTangent));
        Mean += Source;
      }
      Mean /= Samples.size();
      // The mean has an uncertainty of sigma/sqrt(N) = 8 deg/141 = 0.06 deg, 0.3 deg is five standard deviations
      Passed = EvaluateNear("GeneratePosition()", MString("assymetric gaussian, rotation ") + Rotation/deg + ", center", "The mean direction is the center", Mean.angle(Center)/deg, 0.0, 0.3) && Passed;
      StdPhi.push_back(Statistics(East).m_Sigma);
      StdTheta.push_back(Statistics(South).m_Sigma);
    }
    // The standard deviation of the sample standard deviation is sigma/sqrt(2N) = 0.5% of sigma: 0.04 deg (8 deg) and 0.015 deg (3 deg);
    // the tolerances are five standard deviations plus 0.3% of sigma for the projection of the angles onto the tangent plane
    // Without rotation the width along phi (east) is sigma phi, along theta (south) sigma theta - with 90 deg rotation they are exchanged:
    Passed = EvaluateNear("GeneratePosition()", "assymetric gaussian, no rotation, phi width", "The width in phi is sigma phi", StdPhi[0]/deg, SigmaPhi/deg, 0.25) && Passed;
    Passed = EvaluateNear("GeneratePosition()", "assymetric gaussian, no rotation, theta width", "The width in theta is sigma theta", StdTheta[0]/deg, SigmaTheta/deg, 0.1) && Passed;
    Passed = EvaluateNear("GeneratePosition()", "assymetric gaussian, rotated, phi width", "After a rotation by 90 deg the width along phi is sigma theta", StdPhi[1]/deg, SigmaTheta/deg, 0.1) && Passed;
    Passed = EvaluateNear("GeneratePosition()", "assymetric gaussian, rotated, theta width", "After a rotation by 90 deg the width along theta is sigma phi", StdTheta[1]/deg, SigmaPhi/deg, 0.25) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestFarFieldFile()
{
  bool Passed = true;
  RandomGuard Guard(51);

  const MString File = GetTemporaryFileName("Zenith.dat");
  Passed = EvaluateTrue("WriteTextFile()", "zenith", "The zenith fixture can be written", WriteTextFile(File, "DP 0 1.0\nDP 45 0.5\nDP 90 0.1\nEN\n")) && Passed;

  MCSource S("Zenith file");
  SilenceOutput();
  bool OK = S.SetParticleType(MCSource::c_Gamma);
  OK = S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldFileZenithDependent) && OK;
  OK = S.SetPosition(File) && OK;
  OK = S.SetFlux(1.0) && OK;
  OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
  OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, c_Radius) && OK;
  OK = S.GenerateParticleDefinition() && OK;
  RestoreOutput();
  Passed = EvaluateTrue("SetPosition()", "zenith file", "The source can be configured", OK) && Passed;

  bool Generated = false;
  vector<Sample> Samples = Generate(S, c_Samples, &Generated);
  Passed = EvaluateTrue("GeneratePosition()", "zenith file", "The generation succeeds", Generated) && Passed;
  vector<double> Thetas, Phis;
  for (const Sample& P: Samples) {
    Thetas.push_back((-P.m_Direction).theta());
    Phis.push_back(Azimuth(-P.m_Direction));
  }
  auto Density = [](double T) {
    const double D = T/deg;
    const double F = (D <= 45) ? 1.0 - 0.5*D/45 : 0.5 - 0.4*(D - 45)/45;
    return F*sin(T);
  };
  Passed = KS("GeneratePosition()", "zenith file, theta", "The zenith angles follow the file times sin(theta)", Thetas, Density, 0.0, 90*deg) && Passed;
  Passed = KS("GeneratePosition()", "zenith file, phi", "The azimuth angles are uniform", Phis, [](double) { return 1.0; }, 0.0, 2*c_Pi) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestFarFieldStartArea()
{
  bool Passed = true;
  RandomGuard Guard(61);

  // Start area tube: the start positions are on a rectangle perpendicular to the direction at the distance of the bounding sphere
  const double R = 20*cm;
  const double H = 40*cm;
  const double Theta = 50*deg;
  const double Phi = 120*deg;
  MCSource S("Tube");
  Passed = EvaluateTrue("Configure()", "tube", "The source can be configured", Configure(S, MCSource::c_FarField, MCSource::c_FarFieldPoint, { Theta, Phi }, MCSource::c_StartAreaTube)) && Passed;
  bool OK = false;
  vector<Sample> Samples = Generate(S, 5000, &OK);
  Passed = EvaluateTrue("GeneratePosition()", "tube", "The generation succeeds", OK) && Passed;

  const G4ThreeVector Source(sin(Theta)*cos(Phi), sin(Theta)*sin(Phi), cos(Theta));
  // Basis of the rectangle: e1 lies in the plane of the axis and the direction, e2 is perpendicular to both:
  const G4ThreeVector E2(-sin(Phi), cos(Phi), 0.0);
  const G4ThreeVector E1 = E2.cross(Source);
  const double HalfX = H*sin(Theta) + R*fabs(cos(Theta));
  double MaxPlane = 0.0, MaxX = 0.0, MaxY = 0.0;
  vector<double> Xs, Ys;
  for (const Sample& P: Samples) {
    MaxPlane = max(MaxPlane, fabs(P.m_Position.dot(Source) - sqrt(R*R + H*H)));
    const double X = fabs(P.m_Position.dot(E1));
    const double Y = fabs(P.m_Position.dot(E2));
    MaxX = max(MaxX, X);
    MaxY = max(MaxY, Y);
    Xs.push_back(P.m_Position.dot(E1));
    Ys.push_back(P.m_Position.dot(E2));
  }
  Passed = EvaluateNear("GeneratePosition()", "tube, plane", "The start positions are on the plane at the distance of the bounding sphere", MaxPlane/cm, 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "tube, rectangle, x", "The start positions are inside the projected rectangle along the long side (shown: the excess over the limit)", max(MaxX - HalfX, 0.0), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "tube, rectangle, y", "The start positions are inside the projected rectangle along the short side (shown: the excess over the limit)", max(MaxY - R, 0.0), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "tube, coverage, x", "The start positions cover the whole long side of the rectangle (shown: the shortfall)", max(0.98*HalfX - MaxX, 0.0), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "tube, coverage, y", "The start positions cover the whole short side of the rectangle (shown: the shortfall)", max(0.98*R - MaxY, 0.0), 0.0, 0.0) && Passed;
  Passed = KS("GeneratePosition()", "tube, x", "The start positions are uniform along the long side of the rectangle", Xs, [](double) { return 1.0; }, -HalfX, HalfX) && Passed;
  Passed = KS("GeneratePosition()", "tube, y", "The start positions are uniform along the short side of the rectangle", Ys, [](double) { return 1.0; }, -R, R) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestNearFieldShapes()
{
  bool Passed = true;

  const G4ThreeVector Center(1*cm, 2*cm, 3*cm);

  // Point:
  {
    RandomGuard Guard(71);
    MCSource S("Point");
    Passed = EvaluateTrue("Configure()", "point", "The source can be configured", Configure(S, MCSource::c_NearField, MCSource::c_NearFieldPoint, { 1*cm, 2*cm, 3*cm })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "point", "The generation succeeds", OK) && Passed;
    double MaxDistance = 0.0;
    vector<double> CosThetas, Phis;
    for (const Sample& P: Samples) {
      MaxDistance = max(MaxDistance, (P.m_Position - Center).mag());
      CosThetas.push_back(P.m_Direction.cosTheta());
      Phis.push_back(Azimuth(P.m_Direction));
    }
    Passed = EvaluateNear("GeneratePosition()", "point, position", "All particles start at the point", MaxDistance/cm, 0.0, 1e-9) && Passed;
    Passed = KS("GeneratePosition()", "point, cos(theta)", "The directions are isotropic (cos theta uniform)", CosThetas, [](double) { return 1.0; }, -1.0, 1.0) && Passed;
    Passed = KS("GeneratePosition()", "point, phi", "The azimuth angles are uniform", Phis, [](double) { return 1.0; }, 0.0, 2*c_Pi) && Passed;
  }

  // Line:
  {
    RandomGuard Guard(72);
    const G4ThreeVector A(-10*cm, 5*cm, 0*cm), B(20*cm, -15*cm, 30*cm);
    MCSource S("Line");
    Passed = EvaluateTrue("Configure()", "line", "The source can be configured", Configure(S, MCSource::c_NearField, MCSource::c_NearFieldLine, { A.x(), A.y(), A.z(), B.x(), B.y(), B.z() })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "line", "The generation succeeds", OK) && Passed;
    double MaxOff = 0.0;
    vector<double> Fractions;
    const G4ThreeVector AB = B - A;
    for (const Sample& P: Samples) {
      const double T = (P.m_Position - A).dot(AB)/AB.mag2();
      MaxOff = max(MaxOff, (P.m_Position - (A + T*AB)).mag());
      Fractions.push_back(T);
    }
    Passed = EvaluateNear("GeneratePosition()", "line, on line", "All positions are on the line", MaxOff/cm, 0.0, 1e-6) && Passed;
    Passed = KS("GeneratePosition()", "line, position", "The positions are uniformly distributed along the line", Fractions, [](double) { return 1.0; }, 0.0, 1.0) && Passed;
  }

  // Box:
  {
    RandomGuard Guard(73);
    MCSource S("Box");
    Passed = EvaluateTrue("Configure()", "box", "The source can be configured", Configure(S, MCSource::c_NearField, MCSource::c_NearFieldBox, { -10*cm, -20*cm, -5*cm, 10*cm, 20*cm, 15*cm })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "box", "The generation succeeds", OK) && Passed;
    vector<double> X, Y, Z;
    for (const Sample& P: Samples) {
      X.push_back(P.m_Position.x()/cm);
      Y.push_back(P.m_Position.y()/cm);
      Z.push_back(P.m_Position.z()/cm);
    }
    Passed = KS("GeneratePosition()", "box, x", "The x positions are uniform in the box", X, [](double) { return 1.0; }, -10.0, 10.0) && Passed;
    Passed = KS("GeneratePosition()", "box, y", "The y positions are uniform in the box", Y, [](double) { return 1.0; }, -20.0, 20.0) && Passed;
    Passed = KS("GeneratePosition()", "box, z", "The z positions are uniform in the box", Z, [](double) { return 1.0; }, -5.0, 15.0) && Passed;
  }

  // Ellipsoid:
  {
    RandomGuard Guard(74);
    const double A = 10*cm, B = 20*cm, C = 5*cm;
    MCSource S("Sphere");
    Passed = EvaluateTrue("Configure()", "sphere", "The source can be configured", Configure(S, MCSource::c_NearField, MCSource::c_NearFieldSphere, { 1*cm, 2*cm, 3*cm, A, B, C })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "sphere", "The generation succeeds", OK) && Passed;
    vector<double> Volume;
    double Max = 0.0;
    for (const Sample& P: Samples) {
      const G4ThreeVector D = P.m_Position - Center;
      const double U = D.x()*D.x()/(A*A) + D.y()*D.y()/(B*B) + D.z()*D.z()/(C*C);
      Max = max(Max, U);
      Volume.push_back(pow(U, 1.5));
    }
    Passed = EvaluateNear("GeneratePosition()", "sphere, inside", "All positions are inside the ellipsoid around the given center (shown: the excess over the limit)", max(Max - (1.0), 0.0), 0.0, 1e-9) && Passed;
    Passed = KS("GeneratePosition()", "sphere, volume", "The positions are uniformly distributed in the ellipsoid (u^1.5 uniform)", Volume, [](double) { return 1.0; }, 0.0, 1.0) && Passed;
  }

  // Disk:
  {
    RandomGuard Guard(75);
    const G4ThreeVector N = G4ThreeVector(1.0, 1.0, 1.0).unit();
    const double Rin = 2*cm, Rout = 8*cm, Height = 4*cm, Phi1 = 20*deg, Phi2 = 200*deg;
    MCSource S("Disk");
    Passed = EvaluateTrue("Configure()", "disk", "The source can be configured", Configure(S, MCSource::c_NearField, MCSource::c_NearFieldDisk, { 1*cm, 2*cm, 3*cm, 1.0, 1.0, 1.0, Rin, Rout, Height, Phi1, Phi2 })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "disk", "The generation succeeds", OK) && Passed;
    vector<double> Radii2, Heights;
    double MaxH = 0.0;
    for (const Sample& P: Samples) {
      const G4ThreeVector D = P.m_Position - Center;
      const double Along = D.dot(N);
      MaxH = max(MaxH, fabs(Along));
      Heights.push_back(Along/cm);
      Radii2.push_back((D - Along*N).mag2()/(cm*cm));
    }
    Passed = EvaluateNear("GeneratePosition()", "disk, height", "The positions are within half the height along the normal (shown: the excess over the limit)", max(MaxH - (Height/2), 0.0), 0.0, 1e-6) && Passed;
    Passed = KS("GeneratePosition()", "disk, height", "The positions are uniform along the normal", Heights, [](double) { return 1.0; }, -2.0, 2.0) && Passed;
    Passed = KS("GeneratePosition()", "disk, radius", "The positions are uniform in the ring area (r^2 uniform)", Radii2, [](double) { return 1.0; }, 4.0, 64.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestNearFieldBeams()
{
  bool Passed = true;

  const G4ThreeVector Start(1*cm, 2*cm, 3*cm);

  // Homogeneous beam:
  {
    RandomGuard Guard(81);
    const G4ThreeVector N = G4ThreeVector(0.3, -0.5, 0.8).unit();
    const double Radius = 2*cm;
    MCSource S("Beam");
    Passed = EvaluateTrue("Configure()", "beam", "The source can be configured", Configure(S, MCSource::c_NearField, MCSource::c_NearFieldBeam, { 1*cm, 2*cm, 3*cm, 0.3, -0.5, 0.8, Radius })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "beam", "The generation succeeds", OK) && Passed;
    double MaxDirection = 0.0, MaxAlong = 0.0;
    vector<double> Radii2;
    for (const Sample& P: Samples) {
      MaxDirection = max(MaxDirection, (P.m_Direction.unit() - N).mag());
      const G4ThreeVector D = P.m_Position - Start;
      MaxAlong = max(MaxAlong, fabs(D.dot(N)));
      Radii2.push_back((D - D.dot(N)*N).mag2()/(Radius*Radius));
    }
    Passed = EvaluateNear("GeneratePosition()", "beam, direction", "All particles fly in the beam direction", MaxDirection, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GeneratePosition()", "beam, plane", "The start positions are on the plane perpendicular to the beam through the start", MaxAlong/cm, 0.0, 1e-6) && Passed;
    Passed = KS("GeneratePosition()", "beam, radius", "The positions are uniform in the beam area (r^2 uniform)", Radii2, [](double) { return 1.0; }, 0.0, 1.0) && Passed;
  }

  // Cone beams:
  for (int Gauss = 0; Gauss <= 1; ++Gauss) {
    RandomGuard Guard(82 + Gauss);
    const G4ThreeVector N = G4ThreeVector(0.3, -0.5, 0.8).unit();
    const double Opening = 25*deg;
    const double Sigma = 10*deg;
    MCSource S("Cone");
    vector<double> P = { 1*cm, 2*cm, 3*cm, 0.3, -0.5, 0.8, Opening };
    if (Gauss == 1) P.push_back(Sigma);
    const MString Name = (Gauss == 1) ? "gaussian cone beam" : "cone beam";
    Passed = EvaluateTrue("Configure()", Name, "The source can be configured", Configure(S, MCSource::c_NearField, (Gauss == 1) ? MCSource::c_NearFieldConeBeamGauss : MCSource::c_NearFieldConeBeam, P)) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", OK) && Passed;
    vector<double> Angles, Phis;
    double MaxPosition = 0.0;
    // Azimuth around the axis:
    const G4ThreeVector U = N.orthogonal().unit();
    const G4ThreeVector V = N.cross(U);
    for (const Sample& Q: Samples) {
      MaxPosition = max(MaxPosition, (Q.m_Position - Start).mag());
      Angles.push_back(Q.m_Direction.angle(N));
      double Phi = atan2(Q.m_Direction.dot(V), Q.m_Direction.dot(U));
      Phis.push_back(Phi < 0 ? Phi + 2*c_Pi : Phi);
    }
    Passed = EvaluateNear("GeneratePosition()", Name + ", position", "All particles start at the given position", MaxPosition/cm, 0.0, 1e-9) && Passed;
    if (Gauss == 1) {
      Passed = KS("GeneratePosition()", Name + ", angle", "The angles from the axis follow sin(psi)*exp(-psi^2/2sigma^2)", Angles, [Sigma](double A) { return sin(A)*exp(-A*A/(2*Sigma*Sigma)); }, 0.0, Opening) && Passed;
    } else {
      Passed = KS("GeneratePosition()", Name + ", angle", "The angles from the axis are isotropic within the cone (sin(psi))", Angles, [](double A) { return sin(A); }, 0.0, Opening) && Passed;
    }
    Passed = KS("GeneratePosition()", Name + ", azimuth", "The azimuth around the axis is uniform", Phis, [](double) { return 1.0; }, 0.0, 2*c_Pi) && Passed;
  }

  // Restricted point: the cone towards the start sphere, all rays must hit the sphere, the whole sphere is illuminated:
  {
    RandomGuard Guard(84);
    const G4ThreeVector Point(0.0, 0.0, 60*cm);
    MCSource S("Restricted point");
    Passed = EvaluateTrue("Configure()", "restricted point", "The source can be configured", Configure(S, MCSource::c_NearField, MCSource::c_NearFieldRestrictedPoint, { Point.x(), Point.y(), Point.z() })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "restricted point", "The generation succeeds", OK) && Passed;
    const double Cone = asin(c_Radius/Point.mag());
    const G4ThreeVector Axis = -Point.unit();
    double MaxDistance = 0.0;
    vector<double> Angles;
    for (const Sample& Q: Samples) {
      // Distance of the ray from the origin:
      const G4ThreeVector D = Q.m_Direction.unit();
      MaxDistance = max(MaxDistance, (Point - Point.dot(D)*D).mag());
      Angles.push_back(D.angle(Axis));
    }
    Passed = EvaluateNear("GeneratePosition()", "restricted point, hits", "All rays hit the start sphere (shown: the excess over the limit)", max(MaxDistance - c_Radius, 0.0), 0.0, c_Radius*1e-6) && Passed;
    Passed = KS("GeneratePosition()", "restricted point, angle", "The rays are isotropic inside the cone which covers the start sphere", Angles, [](double A) { return sin(A); }, 0.0, Cone) && Passed;
  }

  // Restricted line:
  {
    RandomGuard Guard(85);
    const G4ThreeVector A(50*cm, 0.0, -10*cm), B(50*cm, 0.0, 10*cm);
    MCSource S("Restricted line");
    Passed = EvaluateTrue("Configure()", "restricted line", "The source can be configured", Configure(S, MCSource::c_NearField, MCSource::c_NearFieldRestrictedLine, { A.x(), A.y(), A.z(), B.x(), B.y(), B.z() })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", "restricted line", "The generation succeeds", OK) && Passed;
    double MaxOff = 0.0;
    vector<double> Angles;
    const double Cone = asin(c_Radius/(50*cm));
    for (const Sample& Q: Samples) {
      const G4ThreeVector D = Q.m_Direction.unit();
      Angles.push_back(D.angle(-Q.m_Position));
      const double T = (Q.m_Position - A).dot(B - A)/(B - A).mag2();
      MaxOff = max(MaxOff, (Q.m_Position - (A + T*(B - A))).mag());
    }
    Passed = EvaluateNear("GeneratePosition()", "restricted line, on line", "All positions are on the line", MaxOff/cm, 0.0, 1e-6) && Passed;
    // The cone is the one under which the start sphere is seen from the closest point of the line (the sphere center is the origin):
    Passed = EvaluateNear("GeneratePosition()", "restricted line, cone", "All directions are inside the cone around the direction to the start sphere center (shown: the excess over the limit)", max(*max_element(Angles.begin(), Angles.end()) - Cone, 0.0), 0.0, Cone*1e-6) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestFanBeam()
{
  bool Passed = true;

  // The fan beam spans the plane of two directions: the directions are uniform in the angle between them, the start positions are uniform in a disk of the
  // beam width around the start, perpendicular to the direction (width zero: a point beam)
  struct Case {
    const char* m_Name;
    G4ThreeVector m_First;
    G4ThreeVector m_Second;
    double m_Width;
  };
  const vector<Case> Cases = {
    { "axes, width 0.5 cm", G4ThreeVector(1.0, 0.0, 0.0), G4ThreeVector(0.0, 1.0, 0.0), 0.5*cm },
    { "skew directions, width 0.5 cm", G4ThreeVector(1.0, 2.0, 2.0), G4ThreeVector(2.0, -1.0, 2.0), 0.5*cm },
    { "skew directions, width 2 cm", G4ThreeVector(-3.0, 1.0, 0.5), G4ThreeVector(0.2, 0.4, -5.0), 2.0*cm },
    { "axes, width zero", G4ThreeVector(1.0, 0.0, 0.0), G4ThreeVector(0.0, 1.0, 0.0), 0.0 }
  };

  unsigned int Index = 0;
  for (const Case& C: Cases) {
    RandomGuard Guard(91 + Index++);
    const MString Name = MString("fan beam, ") + C.m_Name;
    const double Width = C.m_Width;
    MCSource S("Fan");
    Passed = EvaluateTrue("Configure()", Name, "The source can be configured",
                          Configure(S, MCSource::c_NearField, MCSource::c_NearFieldFanBeam, { 1*cm, 2*cm, 3*cm, C.m_First.x(), C.m_First.y(), C.m_First.z(), C.m_Second.x(), C.m_Second.y(), C.m_Second.z(), Width })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", OK) && Passed;

    const G4ThreeVector Start(1*cm, 2*cm, 3*cm);
    const G4ThreeVector First = C.m_First.unit();
    const G4ThreeVector Normal = C.m_First.cross(C.m_Second).unit();
    const double Opening = C.m_First.angle(C.m_Second);
    vector<double> Angles, Radii2;
    double MaxPlane = 0.0, MaxRadius = 0.0, MaxAlong = 0.0;
    for (const Sample& P: Samples) {
      const G4ThreeVector D = P.m_Direction.unit();
      Angles.push_back(D.angle(First));
      MaxPlane = max(MaxPlane, fabs(D.dot(Normal)));
      const G4ThreeVector Offset = P.m_Position - Start;
      MaxAlong = max(MaxAlong, fabs(Offset.dot(D)));
      MaxRadius = max(MaxRadius, Offset.mag());
      if (Width > 0) Radii2.push_back(Offset.mag2()/(Width*Width));
    }
    Passed = EvaluateNear("GeneratePosition()", Name + ", plane", "All directions are in the plane of the two given directions", MaxPlane, 0.0, 1e-9) && Passed;
    Passed = KS("GeneratePosition()", Name + ", angle", "The direction angles are uniform between the two given directions", Angles, [](double) { return 1.0; }, 0.0, Opening) && Passed;
    Passed = EvaluateNear("GeneratePosition()", Name + ", perpendicular", "The offsets of the start positions are perpendicular to the direction of the photon", MaxAlong/cm, 0.0, 1e-9) && Passed;
    if (Width > 0) {
      Passed = EvaluateNear("GeneratePosition()", Name + ", width", "The start positions are within the beam width around the start (shown: the excess over the limit)", max(MaxRadius - Width, 0.0), 0.0, Width*1e-6) && Passed;
      Passed = EvaluateNear("GeneratePosition()", Name + ", used", "The beam width is used (positions are not all at the start point) (shown: the shortfall)", max(0.9*Width - MaxRadius, 0.0), 0.0, 0.0) && Passed;
      Passed = KS("GeneratePosition()", Name + ", radius", "The start positions are uniform in the disk (the squared radius is uniform)", Radii2, [](double) { return 1.0; }, 0.0, 1.0) && Passed;
    } else {
      Passed = EvaluateNear("GeneratePosition()", Name + ", point", "A beam of width zero starts at the given position", MaxRadius/cm, 0.0, 1e-9) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestNearFieldMaps()
{
  bool Passed = true;

  // The 2D profile beam and the flat map take the start position in the plane perpendicular to the beam direction from a 2D map (x and y in cm), rotated by an
  // angle around the beam axis. A bilinear pyramid with its peak at (x, y) = (3, -1) cm on [2, 4] x [-2, 0] cm has independent triangular marginals in x and y;
  // its supports differ in x and y, thus swapped axes, a wrong sign of the rotation or a wrong plane are visible
  const MString Map = GetTemporaryFileName("NearFieldMap.dat");
  Passed = EvaluateTrue("WriteTextFile()", "map", "The map fixture can be written", WriteTextFile(Map, "IP LIN\nXA 2 3 4\nYA -2 -1 0\nAP 1 1 1.0\nEN\n")) && Passed;

  const G4ThreeVector Center(1*cm, 2*cm, 3*cm);
  unsigned int Index = 0;
  for (int Beam: { MCSource::c_NearFieldBeam2DProfile, MCSource::c_NearFieldFlatMap }) {
    for (const G4ThreeVector& Normal: { G4ThreeVector(0.0, 0.0, 1.0), G4ThreeVector(0.3, -0.5, 0.8) }) {
      for (double Rotation: { 0.0, 90*deg, 35*deg }) {
        RandomGuard Guard(1000 + Index++);
        const MString Name = MString(Beam == MCSource::c_NearFieldFlatMap ? "flat map" : "2D profile") + ", normal (" + Normal.x() + ", " + Normal.y() + ", " + Normal.z() + "), rotation " + Rotation/deg + " deg";
        MCSource S("Map beam");
        Passed = EvaluateTrue("Configure()", Name, "The source can be configured",
                              Configure(S, MCSource::c_NearField, Beam, { Center.x(), Center.y(), Center.z(), Normal.x(), Normal.y(), Normal.z(), Rotation }, MCSource::c_StartAreaSphere, "", Map)) && Passed;
        bool OK = false;
        vector<Sample> Samples = Generate(S, c_Samples, &OK);
        Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", OK) && Passed;

        // From the position back to the map coordinates: undo the rotation into the beam frame (z axis along the normal), then the rotation around the beam axis
        const G4ThreeVector N = Normal.unit();
        vector<double> X, Y, CosTheta, Phi;
        double MaxPlane = 0.0, MaxDirection = 0.0;
        for (const Sample& P: Samples) {
          G4ThreeVector Local = P.m_Position - Center;
          MaxPlane = max(MaxPlane, fabs(Local.dot(N)));
          Local.rotateZ(-N.phi());
          Local.rotateY(-N.theta());
          X.push_back((Local.x()*cos(Rotation) + Local.y()*sin(Rotation))/cm);
          Y.push_back((-Local.x()*sin(Rotation) + Local.y()*cos(Rotation))/cm);
          const G4ThreeVector D = P.m_Direction.unit();
          MaxDirection = max(MaxDirection, (D - N).mag());
          CosTheta.push_back(D.z());
          Phi.push_back(Azimuth(D));
        }
        Passed = EvaluateNear("GeneratePosition()", Name + ", plane", "The start positions are in the plane perpendicular to the beam direction through the center", MaxPlane/cm, 0.0, 1e-9) && Passed;
        Passed = KS("GeneratePosition()", Name + ", x", "The x coordinate follows the triangular marginal of the map on [2, 4] cm", X, [](double x) { return 1.0 - fabs(x - 3.0); }, 2.0, 4.0) && Passed;
        Passed = KS("GeneratePosition()", Name + ", y", "The y coordinate follows the triangular marginal of the map on [-2, 0] cm", Y, [](double y) { return 1.0 - fabs(y + 1.0); }, -2.0, 0.0) && Passed;
        if (Beam == MCSource::c_NearFieldBeam2DProfile) {
          Passed = EvaluateNear("GeneratePosition()", Name + ", direction", "All particles fly in the beam direction", MaxDirection, 0.0, 1e-9) && Passed;
        } else {
          // The flat map emits isotropically
          Passed = KS("GeneratePosition()", Name + ", cos theta", "The direction is isotropic: cos(theta) is uniform", CosTheta, [](double) { return 1.0; }, -1.0, 1.0) && Passed;
          Passed = KS("GeneratePosition()", Name + ", phi", "The direction is isotropic: the azimuth is uniform", Phi, [](double) { return 1.0; }, 0.0, 2*c_Pi) && Passed;
        }
      }
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestReverseDirection()
{
  bool Passed = true;
  RandomGuard Guard(1100);

  // A successor with the beam ReverseDirectionToPredecessor starts where its predecessor started and flies in the opposite direction.
  // The run does the hand-over as in MCRun::GenerateParticles(): after the predecessor, SetPosition(position, -direction) is called on the successor
  MCSource Predecessor("Predecessor");
  Passed = EvaluateTrue("Configure()", "predecessor", "The predecessor can be configured", Configure(Predecessor, MCSource::c_NearField, MCSource::c_NearFieldPoint, { 1*cm, 2*cm, 3*cm })) && Passed;
  MCSource Successor("Successor");
  Passed = EvaluateTrue("Configure()", "successor", "The successor can be configured", Configure(Successor, MCSource::c_NearField, MCSource::c_NearFieldReverseDirectionToPredecessor, { 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 })) && Passed;

  double MaxPosition = 0.0, MaxReverse = 0.0, MaxNorm = 0.0;
  unsigned int Generated = 0;
  for (unsigned int i = 0; i < 1000; ++i) {
    SilenceOutput();
    bool First = Predecessor.GeneratePosition(m_Gun);
    const G4ThreeVector Position = GetGunPosition();
    const G4ThreeVector Direction = GetGunDirection();
    bool Handover = Successor.SetPosition(Position.x(), Position.y(), Position.z(), -Direction.x(), -Direction.y(), -Direction.z());
    bool Second = Successor.GeneratePosition(m_Gun);
    RestoreOutput();
    if (First == false || Handover == false || Second == false) break;
    ++Generated;
    MaxPosition = max(MaxPosition, (GetGunPosition() - Position).mag());
    MaxReverse = max(MaxReverse, (GetGunDirection().unit() + Direction.unit()).mag());
    MaxNorm = max(MaxNorm, fabs(GetGunDirection().mag() - 1.0));
  }
  Passed = Evaluate("GeneratePosition()", "reverse direction", "All hand-overs and generations succeed", Generated, 1000U) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "reverse direction, position", "The successor starts at the position of the predecessor", MaxPosition/cm, 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "reverse direction, direction", "The successor flies in the opposite direction of the predecessor", MaxReverse, 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "reverse direction, unit", "The direction of the gun is a unit vector", MaxNorm, 0.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestDiffractionPoint()
{
  bool Passed = true;

  // The diffraction point source starts at a point and emits into the directions given by a 2D map over theta (x, degree) and phi (y, degree) in the beam frame
  // (z axis along the normal, rotated by an angle around it). The map is separable, f(theta, phi) = g(theta)*h(phi) with piecewise linear g and h; bilinear
  // interpolation of the node products is exactly g*h, thus the marginals are known: phi follows h, cos(theta) follows g(acos(cos theta)) (uniform solid angle)
  const vector<double> ThetaNodes = { 0.0, 60.0, 120.0, 180.0 };
  const vector<double> ThetaValues = { 0.2, 1.0, 0.4, 0.8 };
  const vector<double> PhiNodes = { 0.0, 90.0, 180.0, 270.0, 360.0 };
  const vector<double> PhiValues = { 1.0, 3.0, 0.5, 2.0, 1.0 };
  auto Piecewise = [](const vector<double>& Nodes, const vector<double>& Values, double X) {
    for (unsigned int i = 0; i + 1 < Nodes.size(); ++i) {
      if (X >= Nodes[i] && X <= Nodes[i+1]) return Values[i] + (Values[i+1] - Values[i])*(X - Nodes[i])/(Nodes[i+1] - Nodes[i]);
    }
    return 0.0;
  };

  MString Content = "IP LIN\nXA";
  for (double T: ThetaNodes) Content += MString(" ") + T;
  Content += "\nYA";
  for (double P: PhiNodes) Content += MString(" ") + P;
  Content += "\n";
  for (unsigned int t = 0; t < ThetaNodes.size(); ++t) {
    for (unsigned int p = 0; p < PhiNodes.size(); ++p) {
      Content += MString("AP ") + t + " " + p + " " + ThetaValues[t]*PhiValues[p] + "\n";
    }
  }
  Content += "EN\n";
  const MString Map = GetTemporaryFileName("Diffraction.dat");
  Passed = EvaluateTrue("WriteTextFile()", "diffraction map", "The map fixture can be written", WriteTextFile(Map, Content)) && Passed;

  const G4ThreeVector Center(1*cm, 2*cm, 3*cm);
  unsigned int Index = 0;
  for (const G4ThreeVector& Normal: { G4ThreeVector(0.0, 0.0, 1.0), G4ThreeVector(0.3, -0.5, 0.8) }) {
    for (double Rotation: { 0.0, 90*deg, 35*deg }) {
      RandomGuard Guard(1200 + Index++);
      const MString Name = MString("diffraction point, normal (") + Normal.x() + ", " + Normal.y() + ", " + Normal.z() + "), rotation " + Rotation/deg + " deg";
      MCSource S("Diffraction");
      Passed = EvaluateTrue("Configure()", Name, "The source can be configured",
                            Configure(S, MCSource::c_NearField, MCSource::c_NearFieldDiffractionPoint, { Center.x(), Center.y(), Center.z(), Normal.x(), Normal.y(), Normal.z(), Rotation },
                                      MCSource::c_StartAreaSphere, "", Map)) && Passed;
      bool OK = false;
      vector<Sample> Samples = Generate(S, c_Samples, &OK);
      Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", OK) && Passed;

      // Back into the beam frame: undo the rotation into the normal frame, then the rotation around the beam axis
      const G4ThreeVector N = Normal.unit();
      vector<double> CosTheta, Phi;
      double MaxPosition = 0.0, MaxNorm = 0.0;
      for (const Sample& P: Samples) {
        MaxPosition = max(MaxPosition, (P.m_Position - Center).mag());
        G4ThreeVector D = P.m_Direction;
        MaxNorm = max(MaxNorm, fabs(D.mag() - 1.0));
        D.rotateZ(-N.phi());
        D.rotateY(-N.theta());
        CosTheta.push_back(D.z());
        double Angle = Azimuth(D) - Rotation;
        while (Angle < 0) Angle += 2*c_Pi;
        while (Angle >= 2*c_Pi) Angle -= 2*c_Pi;
        Phi.push_back(Angle);
      }
      Passed = EvaluateNear("GeneratePosition()", Name + ", position", "All particles start at the given point", MaxPosition/cm, 0.0, 1e-9) && Passed;
      Passed = EvaluateNear("GeneratePosition()", Name + ", unit", "The directions are unit vectors", MaxNorm, 0.0, 1e-9) && Passed;
      Passed = KS("GeneratePosition()", Name + ", cos theta", "cos(theta) follows the theta profile of the map", CosTheta,
                  [&](double C) { return Piecewise(ThetaNodes, ThetaValues, acos(max(-1.0, min(1.0, C)))/deg); }, -1.0, 1.0) && Passed;
      Passed = KS("GeneratePosition()", Name + ", phi", "The azimuth follows the phi profile of the map, rotated by the rotation angle", Phi,
                  [&](double P) { return Piecewise(PhiNodes, PhiValues, P/deg); }, 0.0, 2*c_Pi) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestNormalizedEnergyBeamFlux()
{
  bool Passed = true;

  // The normalized energy beam flux function gives the flux as a function of phi (degree), theta (degree) and energy (keV): beam and spectrum are drawn together.
  // The function is separable, f = g(phi)*h(theta)*e(E), with piecewise linear factors whose kinks are on the grid nodes (phi every 10 deg, theta every 5 deg,
  // E every 50 keV), thus the trilinear interpolation is exactly g*h*e and the marginals are known: phi follows g, cos(theta) follows h(acos(cos theta)) (uniform
  // solid angle), and the energy follows e - for any subset of the other variables
  const vector<double> PhiNodes = { 0.0, 120.0, 240.0, 360.0 };
  const vector<double> PhiValues = { 1.0, 3.0, 0.5, 1.0 };
  const vector<double> ThetaNodes = { 0.0, 60.0, 120.0, 180.0 };
  const vector<double> ThetaValues = { 0.2, 1.0, 0.4, 0.8 };
  const vector<double> EnergyNodes = { 100.0, 400.0, 700.0, 1000.0 };
  const vector<double> EnergyValues = { 1.0, 0.3, 2.0, 0.5 };
  auto Piecewise = [](const vector<double>& Nodes, const vector<double>& Values, double X) {
    for (unsigned int i = 0; i + 1 < Nodes.size(); ++i) {
      if (X >= Nodes[i] && X <= Nodes[i+1]) return Values[i] + (Values[i+1] - Values[i])*(X - Nodes[i])/(Nodes[i+1] - Nodes[i]);
    }
    return 0.0;
  };

  vector<double> Phis, Thetas, Energies;
  for (double P = 0.0; P <= 360.0; P += 10.0) Phis.push_back(P);
  for (double T = 0.0; T <= 180.0; T += 5.0) Thetas.push_back(T);
  for (double E = 100.0; E <= 1000.0; E += 50.0) Energies.push_back(E);
  ostringstream Content;
  auto Axis = [&Content](const char* Keyword, const vector<double>& Values) {
    Content<<Keyword;
    for (double V: Values) Content<<" "<<V;
    Content<<"\n";
  };
  Content<<"IP LIN\n";
  Axis("PA", Phis);
  Axis("TA", Thetas);
  Axis("EA", Energies);
  for (unsigned int p = 0; p < Phis.size(); ++p) {
    for (unsigned int t = 0; t < Thetas.size(); ++t) {
      for (unsigned int e = 0; e < Energies.size(); ++e) {
        Content<<"AP "<<p<<" "<<t<<" "<<e<<" "<<Piecewise(PhiNodes, PhiValues, Phis[p])*Piecewise(ThetaNodes, ThetaValues, Thetas[t])*Piecewise(EnergyNodes, EnergyValues, Energies[e])*1e-6<<"\n";
      }
    }
  }
  Content<<"EN\n";
  const MString File = GetTemporaryFileName("NormalizedEnergyBeamFlux.dat");
  Passed = EvaluateTrue("WriteTextFile()", "normalized function", "The function fixture can be written", WriteTextFile(File, Content.str().c_str())) && Passed;

  RandomGuard Guard(1300);
  MCSource S("Normalized energy beam flux");
  SilenceOutput();
  bool OK = S.SetParticleType(MCSource::c_Gamma);
  OK = S.SetSpectralType(MCSource::c_NormalizedEnergyBeamFluxFunction) && OK;
  OK = S.SetEnergy() && OK;
  OK = S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction) && OK;
  OK = S.SetPosition() && OK;
  OK = S.SetNormalizedEnergyBeamFluxFunction(File) && OK;
  OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
  OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, c_Radius) && OK;
  OK = S.GenerateParticleDefinition() && OK;
  RestoreOutput();
  Passed = EvaluateTrue("Configure()", "normalized function", "The source can be configured", OK) && Passed;

  vector<double> Phi, CosTheta, Energy, EnergyForward, EnergyBackward;
  bool AllGenerated = true;
  SilenceOutput();
  for (unsigned int i = 0; i < c_Samples; ++i) {
    AllGenerated = S.GenerateParticles(m_Gun) && AllGenerated;
    // The start direction of the source in the sky is opposite to the flight direction of the particle
    const G4ThreeVector Source = -GetGunDirection().unit();
    Phi.push_back(Azimuth(Source));
    CosTheta.push_back(Source.z());
    const double E = GetGunEnergy()/keV;
    Energy.push_back(E);
    if (acos(Source.z())/deg < 60.0) EnergyForward.push_back(E); else EnergyBackward.push_back(E);
  }
  RestoreOutput();
  Passed = EvaluateTrue("GenerateParticles()", "normalized function", "All particles are generated", AllGenerated) && Passed;

  auto EnergyDensity = [&](double E) { return Piecewise(EnergyNodes, EnergyValues, E); };
  Passed = KS("GenerateParticles()", "normalized function, phi", "The azimuth follows the phi profile of the function", Phi,
              [&](double P) { return Piecewise(PhiNodes, PhiValues, P/deg); }, 0.0, 2*c_Pi) && Passed;
  Passed = KS("GenerateParticles()", "normalized function, cos theta", "cos(theta) follows the theta profile of the function", CosTheta,
              [&](double C) { return Piecewise(ThetaNodes, ThetaValues, acos(max(-1.0, min(1.0, C)))/deg); }, -1.0, 1.0) && Passed;
  Passed = KS("GenerateParticles()", "normalized function, energy", "The energy (in keV) follows the energy profile of the function", Energy, EnergyDensity, 100.0, 1000.0) && Passed;
  Passed = KS("GenerateParticles()", "normalized function, energy, theta below 60 deg", "The energy distribution does not depend on theta (small theta)", EnergyForward, EnergyDensity, 100.0, 1000.0) && Passed;
  Passed = KS("GenerateParticles()", "normalized function, energy, theta above 60 deg", "The energy distribution does not depend on theta (large theta)", EnergyBackward, EnergyDensity, 100.0, 1000.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestNearFieldIlluminated()
{
  bool Passed = true;

  const double Half = 5*cm;
  const double DiskTheta = 30*deg, DiskPhi = 70*deg;
  const double BeamTheta = 40*deg, BeamPhi = 60*deg;
  const G4ThreeVector Normal(sin(DiskTheta)*cos(DiskPhi), sin(DiskTheta)*sin(DiskPhi), cos(DiskTheta));
  const G4ThreeVector BeamDirection = -G4ThreeVector(sin(BeamTheta)*cos(BeamPhi), sin(BeamTheta)*sin(BeamPhi), cos(BeamTheta));
  // Basis on the disk:
  // (the first axis is the local x axis rotated first around y by theta, then around z by phi)
  const G4ThreeVector U(cos(DiskTheta)*cos(DiskPhi), cos(DiskTheta)*sin(DiskPhi), -sin(DiskTheta));
  const G4ThreeVector V(-sin(DiskPhi), cos(DiskPhi), 0.0);

  for (int Square = 0; Square <= 1; ++Square) {
    RandomGuard Guard(95 + Square);
    const MString Name = (Square == 1) ? "illuminated square" : "illuminated disk";
    MCSource S("Illuminated");
    Passed = EvaluateTrue("Configure()", Name, "The source can be configured", Configure(S, MCSource::c_NearField, (Square == 1) ? MCSource::c_NearFieldIlluminatedSquare : MCSource::c_NearFieldIlluminatedDisk,
                          { 0.0, 0.0, 0.0, Half, DiskTheta, DiskPhi, BeamTheta, BeamPhi })) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", OK) && Passed;

    double MaxSphere = 0.0, MaxDirection = 0.0, MaxOutside = 0.0;
    vector<double> X, Y, Radii2;
    for (const Sample& P: Samples) {
      MaxSphere = max(MaxSphere, fabs(P.m_Position.mag() - c_Radius));
      MaxDirection = max(MaxDirection, (P.m_Direction.unit() - BeamDirection).mag());
      // Back-project the ray onto the plane of the disk:
      const double T = -P.m_Position.dot(Normal)/P.m_Direction.unit().dot(Normal);
      const G4ThreeVector Q = P.m_Position + T*P.m_Direction.unit();
      X.push_back(Q.dot(U)/cm);
      Y.push_back(Q.dot(V)/cm);
      Radii2.push_back(Q.mag2()/(Half*Half));
      MaxOutside = max(MaxOutside, (Square == 1) ? max(fabs(Q.dot(U)), fabs(Q.dot(V)))/Half : Q.mag()/Half);
    }
    Passed = EvaluateNear("GeneratePosition()", Name + ", sphere", "The start positions are on the start sphere", MaxSphere/cm, 0.0, 1e-6) && Passed;
    Passed = EvaluateNear("GeneratePosition()", Name + ", direction", "All particles fly in the beam direction", MaxDirection, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GeneratePosition()", Name + ", illuminated", "All rays hit the illuminated area (shown: the excess over the limit)", max(MaxOutside - (1.0), 0.0), 0.0, 1e-6) && Passed;
    if (Square == 1) {
      Passed = KS("GeneratePosition()", Name + ", x", "The rays are uniform over the square (x)", X, [](double) { return 1.0; }, -Half/cm, Half/cm) && Passed;
      Passed = KS("GeneratePosition()", Name + ", y", "The rays are uniform over the square (y)", Y, [](double) { return 1.0; }, -Half/cm, Half/cm) && Passed;
    } else {
      Passed = KS("GeneratePosition()", Name + ", radius", "The rays are uniform over the disk (r^2 uniform)", Radii2, [](double) { return 1.0; }, 0.0, 1.0) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestNearFieldVolume()
{
  bool Passed = true;

  for (int Beam: { MCSource::c_NearFieldVolume, MCSource::c_NearFieldActivation }) {
    RandomGuard Guard(97 + Beam);
    const MString Name = (Beam == MCSource::c_NearFieldVolume) ? "volume" : "activation";
    MCSource S("Volume");
    Passed = EvaluateTrue("Configure()", Name, "The source can be configured", Configure(S, MCSource::c_NearField, Beam, { }, MCSource::c_StartAreaSphere, "Cube")) && Passed;
    bool OK = false;
    vector<Sample> Samples = Generate(S, c_Samples, &OK);
    Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", OK) && Passed;
    vector<double> X, Y, Z, CosThetas;
    double Max = 0.0;
    for (const Sample& P: Samples) {
      X.push_back(P.m_Position.x()/cm);
      Y.push_back(P.m_Position.y()/cm);
      Z.push_back(P.m_Position.z()/cm);
      Max = max(Max, max(fabs(P.m_Position.x()), max(fabs(P.m_Position.y()), fabs(P.m_Position.z()))));
      CosThetas.push_back(P.m_Direction.cosTheta());
    }
    Passed = EvaluateNear("GeneratePosition()", Name + ", inside", "All positions are inside the volume (shown: the excess over the limit)", max(Max - (10*cm), 0.0), 0.0, 1e-6) && Passed;
    Passed = KS("GeneratePosition()", Name + ", x", "The x positions are uniform in the volume", X, [](double) { return 1.0; }, -10.0, 10.0) && Passed;
    Passed = KS("GeneratePosition()", Name + ", y", "The y positions are uniform in the volume", Y, [](double) { return 1.0; }, -10.0, 10.0) && Passed;
    Passed = KS("GeneratePosition()", Name + ", z", "The z positions are uniform in the volume", Z, [](double) { return 1.0; }, -10.0, 10.0) && Passed;
    Passed = KS("GeneratePosition()", Name + ", cos(theta)", "The directions are isotropic", CosThetas, [](double) { return 1.0; }, -1.0, 1.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBeams::TestRadialProfile()
{
  bool Passed = true;

  // Radial profiles f(r) between 0 and 2 cm (r in cm); the radius is distributed as r*f(r)
  // KNOWN DEFECT (MFunction::GetRandomInterpolate, outside of MCSource): the "rising" profile does not follow r*f(r) and the
  // "falling" profile always returns radius 0 - the KS checks for these two profiles fail until MFunction is fixed
  struct Profile {
    const char* m_Name;
    const char* m_File;
    function<double(double)> m_F;
  };
  const vector<Profile> Profiles = {
    { "flat", "DP 0 1.0\nDP 2 1.0\nEN\n", [](double) { return 1.0; } },
    { "rising", "DP 0 0.0\nDP 2 1.0\nEN\n", [](double R) { return R/2.0; } },
    { "falling", "DP 0 1.0\nDP 2 0.2\nEN\n", [](double R) { return 1.0 - 0.4*R; } }
  };

  unsigned int Index = 0;
  for (const Profile& P: Profiles) {
    RandomGuard Guard(101 + Index++);
    const MString Name = MString("radial profile, ") + P.m_Name;
    const MString File = GetTemporaryFileName(MString("Radial") + P.m_Name + ".dat");
    Passed = EvaluateTrue("WriteTextFile()", Name, "The profile fixture can be written", WriteTextFile(File, P.m_File)) && Passed;

    MCSource S("Radial profile");
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldBeam1DProfile) && OK;
    OK = S.SetPosition(0.0, 0.0, 0.0, 0.0, 0.0, 1.0) && OK;
    OK = S.SetPosition(File) && OK;
    OK = S.SetFlux(1.0) && OK;
    OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
    OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, c_Radius) && OK;
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    Passed = EvaluateTrue("SetPosition()", Name, "The source can be configured", OK) && Passed;

    bool Generated = false;
    vector<Sample> Samples = Generate(S, c_Samples, &Generated);
    Passed = EvaluateTrue("GeneratePosition()", Name, "The generation succeeds", Generated) && Passed;
    vector<double> Radii;
    double MaxZ = 0.0;
    for (const Sample& Q: Samples) {
      Radii.push_back(sqrt(Q.m_Position.x()*Q.m_Position.x() + Q.m_Position.y()*Q.m_Position.y())/cm);
      MaxZ = max(MaxZ, fabs(Q.m_Position.z()));
    }
    Passed = EvaluateNear("GeneratePosition()", Name + ", plane", "The start positions are in the plane of the beam", MaxZ/cm, 0.0, 1e-9) && Passed;
    const function<double(double)> F = P.m_F;
    Passed = KS("GeneratePosition()", Name + ", radius", "The radii follow r times the profile", Radii, [F](double R) { return R*F(R); }, 0.0, 2.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSourceBeams Test;
  return Test.Run() == true ? 0 : 1;
}
