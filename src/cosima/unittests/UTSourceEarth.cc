/*
 * UTSourceEarth.cc
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
#include "MCRun.hh"
#include "MCSource.hh"
#include "UTSDHitShared.h"
#include "UTSourceShared.h"

// MEGAlib:
#include "MUnitTest.h"

// Standard lib:
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>
using namespace std;


//! Unit test class for the Earth occultation and the Galactic polarization of MCSource in a Galactic sky
//! with an orientation file which contains the Earth position (altitude 500 km, latitude 10 deg, longitude 200 deg)
class UTSourceEarth : public UTSourceTestBase
{
public:
  //! Default constructor
  UTSourceEarth() : UTSourceTestBase("UTSourceEarth") {}
  //! Default destructor
  virtual ~UTSourceEarth() {}

  //! Run all tests
  virtual bool Run();

private:
  //! The number of samples
  static const unsigned int c_Samples = 20000;
  //! The altitude of the orbit
  static constexpr double c_Altitude = 500.0;

  //! Configure a far field isotropic source
  bool ConfigureFarField(MCSource& S, int Beam, const vector<double>& P, int Occultation, double Theta, bool Inverse)
  {
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(511*keV) && OK;
    OK = S.SetBeamType(MCSource::c_FarField, Beam) && OK;
    if (P.size() > 0) OK = S.SetPosition(P[0], P.size() > 1 ? P[1] : MCSource::c_Invalid) && OK; else OK = S.SetPosition() && OK;
    OK = S.SetFlux(1.0/(cm*cm*second)) && OK;
    OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
    OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 30*cm) && OK;
    if (Occultation == 1) OK = S.SetEarthOccultation(Theta, Inverse) && OK;
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    return OK;
  }

  //! The Earth zenith in Galactic Cartesian coordinates for the orientation file
  static G4ThreeVector EarthZenith() { return G4ThreeVector(cos(10*deg)*cos(200*deg), cos(10*deg)*sin(200*deg), sin(10*deg)); }

  //! The limb angle for the altitude
  static double LimbAngle() { return (c_Pi - asin(6378.0/(6378.0 + c_Altitude)))/deg; }

  bool TestOccultationFraction();
  bool TestOccultationBeam();
  bool TestGalacticPolarization();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSourceEarth::Run()
{
  bool Passed = true;

  // The orientation file: the detector z axis points to (lat 20, lon 40), x to (lat 0, lon 130) - perpendicular
  // The parser gets the file name relative to the private temporary root, which is the working directory of the test:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  PrepareTemporaryDirectory("Sky");
  const MString SkyFile = GetTemporaryDirectoryName("Sky") + "/Sky.ori";
  Passed = EvaluateTrue("WriteTextFile()", "sky", "The orientation file can be written",
                        WriteTextFile(SkyFile, "OG 0 0 130 20 40 500 10 200\nOG 1000000 0 130 20 40 500 10 200\n")) && Passed;
  Passed = EvaluateTrue("InitializeEnvironment()", "earth", "The Geant4 environment with a Galactic sky can be created",
                        InitializeEnvironment("Dummy.OrientationSky Galactic File Loop Sky/Sky.ori\n")) && Passed;
  if (m_EnvironmentOK == false) {
    Summarize();
    return false;
  }

  Passed = TestOccultationFraction() && Passed;
  Passed = TestOccultationBeam() && Passed;
  Passed = TestGalacticPolarization() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceEarth::TestOccultationFraction()
{
  bool Passed = true;

  const MCOrientation& Sky = MCRunManager::GetMCRunManager()->GetCurrentRun().GetSkyOrientationReference();
  Passed = EvaluateTrue("GetSkyOrientationReference()", "sky", "The sky is oriented and Galactic", Sky.IsOriented() && Sky.GetCoordinateSystem() == MCOrientationCoordinateSystem::c_Galactic) && Passed;
  double Alt = 0.0, Lat = 0.0, Lon = 0.0;
  Passed = EvaluateTrue("GetEarthCoordinate()", "sky", "The Earth coordinates are available", Sky.GetEarthCoordinate(0.0, Alt, Lat, Lon)) && Passed;
  Passed = EvaluateNear("GetEarthCoordinate()", "altitude", "The altitude is the one of the file", Alt/km, c_Altitude, 1e-9) && Passed;
  Passed = EvaluateNear("GetEarthCoordinate()", "latitude", "The latitude is the one of the file", Lat/deg, 10.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEarthCoordinate()", "longitude", "The longitude is the one of the file", Lon/deg, 200.0, 1e-9) && Passed;

  // Isotropic far field source with the automatic limb angle, with a fixed angle, and with the inverted cut:
  struct Case {
    double m_Theta;
    bool m_Inverse;
    double m_Fraction;
    const char* m_Name;
  };
  const double Limb = LimbAngle();
  const vector<Case> Cases = {
    { 0.0, false, (1.0 - cos(Limb*deg))/2.0, "automatic limb angle" },
    { 0.0, true, (1.0 + cos(Limb*deg))/2.0, "automatic limb angle, inverted" },
    { 60.0, false, (1.0 - cos(60*deg))/2.0, "60 deg" },
    { 60.0, true, (1.0 + cos(60*deg))/2.0, "60 deg, inverted" }
  };
  unsigned int Index = 0;
  for (const Case& C: Cases) {
    RandomGuard Guard(200 + Index++);
    MCSource S("Occultation");
    Passed = EvaluateTrue("SetEarthOccultation()", C.m_Name, "The source can be configured", ConfigureFarField(S, MCSource::c_FarFieldIsotropic, { }, 1, C.m_Theta, C.m_Inverse)) && Passed;
    unsigned int Kept = 0;
    bool AllGenerated = true;
    for (unsigned int i = 0; i < c_Samples; ++i) {
      SilenceOutput();
      AllGenerated = S.GenerateParticles(m_Gun) && AllGenerated;
      RestoreOutput();
      if (GetGunEnergy() > 0.0) ++Kept;
    }
    Passed = EvaluateTrue("GenerateParticles()", C.m_Name, "All particles are generated", AllGenerated) && Passed;
    // The kept fraction is the fraction of the sky which is not blocked (accept: angle to the Earth zenith <= maximum angle)
    const double Sigma = sqrt(c_Samples*C.m_Fraction*(1.0 - C.m_Fraction));
    Passed = EvaluateNear("GenerateParticles()", C.m_Name + MString(", fraction"), "The fraction of kept particles is the solid angle fraction of the sky which is not occulted", double(Kept), C.m_Fraction*c_Samples, 5*Sigma) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceEarth::TestOccultationBeam()
{
  bool Passed = true;

  const MCOrientation& Sky = MCRunManager::GetMCRunManager()->GetCurrentRun().GetSkyOrientationReference();
  const double Limb = LimbAngle();

  for (int Inverse = 0; Inverse <= 1; ++Inverse) {
    RandomGuard Guard(300 + Inverse);
    const MString Name = Inverse == 1 ? "beam, inverted" : "beam";
    MCSource S("Occultation beam");
    // The parameter file sets the beam and the Earth occultation:
    Passed = EvaluateTrue("ConfigureFarField()", Name, "The source can be configured", ConfigureFarField(S, MCSource::c_FarFieldEarthOccultation, { }, 1, 0.0, Inverse == 1)) && Passed;

    bool OK = false;
    vector<double> CosAngles;
    unsigned int Absorbed = 0;
    SilenceOutput();
    OK = true;
    for (unsigned int i = 0; i < c_Samples; ++i) {
      OK = S.GenerateParticles(m_Gun) && OK;
      if (GetGunEnergy() == 0.0) ++Absorbed;
      // The direction of the source in the Galactic frame as used by the code (local to Galactic, z flipped):
      G4ThreeVector Dir = -GetGunDirection();
      Sky.OrientDirection(S.GetNextEmission(), Dir);
      Dir[2] *= -1;
      CosAngles.push_back(cos(EarthZenith().angle(Dir)));
    }
    RestoreOutput();
    Passed = EvaluateTrue("GenerateParticles()", Name, "All particles are generated", OK) && Passed;
    // The beam only generates directions which are not occulted: nothing is removed afterwards:
    Passed = Evaluate("GenerateParticles()", Name + ", absorbed", "The beam and the Earth occultation of the particle agree: no particle is removed", Absorbed, 0U) && Passed;
    if (Inverse == 0) {
      Passed = EvaluateTrue("GenerateParticles()", Name + ", range", "All directions are inside the allowed cone around the Earth zenith",
                            *min_element(CosAngles.begin(), CosAngles.end()) >= cos(Limb*deg) - 1e-9) && Passed;
      Passed = EvaluateTrue("GenerateParticles()", Name + ", isotropic", "The directions are isotropic in the allowed cone",
                            KSDistance(CosAngles, [](double) { return 1.0; }, cos(Limb*deg), 1.0) < KSCritical(CosAngles.size())) && Passed;
    } else {
      Passed = EvaluateTrue("GenerateParticles()", Name + ", range", "All directions are inside the blocked cone around the Earth nadir",
                            *max_element(CosAngles.begin(), CosAngles.end()) <= cos(Limb*deg) + 1e-9) && Passed;
      Passed = EvaluateTrue("GenerateParticles()", Name + ", isotropic", "The directions are isotropic in the blocked cone",
                            KSDistance(CosAngles, [](double) { return 1.0; }, -1.0, cos(Limb*deg)) < KSCritical(CosAngles.size())) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceEarth::TestGalacticPolarization()
{
  bool Passed = true;
  const MCOrientation& Sky = MCRunManager::GetMCRunManager()->GetCurrentRun().GetSkyOrientationReference();

  // The celestial north pole as in the code (Galactic coordinates l = 122.93 deg, b = 27.13 deg, z flipped due to the left-handed system)
  G4ThreeVector North(cos(27.13*deg)*cos(122.93*deg), cos(27.13*deg)*sin(122.93*deg), -1*sin(27.13*deg));
  Sky.OrientDirectionInvers(0.0, North);

  for (double Angle: { 0.0, 90*deg, 35*deg }) {
    RandomGuard Guard(400);
    MCSource S("Galactic polarization");
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(511*keV) && OK;
    OK = S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldBeam) && OK;
    OK = S.SetPosition(0.0, 0.0, 0.0, 0.3, -0.5, 0.8, 1*cm) && OK;
    OK = S.SetFlux(1.0) && OK;
    OK = S.SetPolarizationType(MCSource::c_PolarizationGalactic) && OK;
    S.SetPolarizationDegree(1.0);
    S.SetPolarization(Angle);
    OK = S.GenerateParticleDefinition() && OK;
    bool Generated = S.GeneratePosition(m_Gun) && S.GeneratePolarization(m_Gun);
    RestoreOutput();
    const MString Name = MString("angle ") + Angle/deg;
    Passed = EvaluateTrue("GeneratePolarization()", Name, "The source can be configured and generated", OK && Generated) && Passed;

    const G4ThreeVector D = GetGunDirection().unit();
    const G4ThreeVector P = GetGunPolarization();
    Passed = EvaluateNear("GeneratePolarization()", Name + ", norm", "The polarization is a unit vector", P.mag(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GeneratePolarization()", Name + ", orthogonal", "The polarization is orthogonal to the direction", P.dot(D), 0.0, 1e-9) && Passed;

    // The polarization is fixed relative to the projection of the north pole on the plane perpendicular to the direction:
    const G4ThreeVector NorthPerpendicular = (North - North.dot(D)*D).unit();
    const G4ThreeVector East = D.cross(NorthPerpendicular);
    // The polarization is the north direction rotated by the angle towards east. The vector P and -P are the same linear polarization (modulo 180 deg), but
    // a reversed angle (rotation towards west) is not: it would give |cos(2*angle)| instead of 1 for the overlap with the expected direction
    const G4ThreeVector Expected = cos(Angle)*NorthPerpendicular + sin(Angle)*East;
    Passed = EvaluateNear("GeneratePolarization()", Name + ", angle", "The polarization is the projected celestial north rotated by the polarization angle towards east (modulo 180 deg)", fabs(P.dot(Expected)), 1.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSourceEarth Test;
  return Test.Run() == true ? 0 : 1;
}
