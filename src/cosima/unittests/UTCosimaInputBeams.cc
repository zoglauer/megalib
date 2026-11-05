/*
 * UTCosimaInputBeams.cc
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
#include "MCParameterFile.hh"
#include "MCSource.hh"
#include "UTParameterFileShared.h"

// Standard lib:
#include <cmath>
#include <utility>
#include <vector>
using namespace std;


//! Unit test class for the beam keywords of the cosima source files: every keyword and alias is parsed into
//! the expected beam type, coordinate system, and parameters
class UTCosimaInputBeams : public UTParameterFileTest
{
public:
  //! Default constructor
  UTCosimaInputBeams() : UTParameterFileTest("UTCosimaInputBeams") {}
  //! Default destructor
  virtual ~UTCosimaInputBeams() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One beam: the name for the output, the Beam keyword line, the expected beam type and coordinate system,
  //! the expected position parameters (angles in rad, lengths in the Geant4 length unit), and the expected volume
  struct BeamCase {
    MString m_Name;
    MString m_Line;
    int m_Beam;
    int m_Coordinates;
    vector<double> m_Position;
    MString m_Volume;
  };

  //! Return a complete source file with the beam line, the spectrum which fits the beam, and the predecessor a reverse direction beam needs
  MString SourceWithBeam(const BeamCase& C) const;
  //! Evaluate that the beam line is parsed into the expected beam
  bool EvaluateBeam(const BeamCase& C);

  //! Test all beam keywords and aliases which need no example data file
  bool TestBeams();
  //! Test the beam keywords which need an example data file
  bool TestFileBeams();
  //! Test that the removed k-space diffraction beam is rejected
  bool TestRemovedBeams();
  //! Test the deprecated numeric Position keyword
  bool TestDeprecatedPosition();
};


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputBeams::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareFiles()", "private directory", "The private directory with the geometry fixture can be created", PrepareFiles()) && Passed;
  if (Passed == false) {
    Summarize();
    return Passed;
  }

  Passed = TestBeams() && Passed;
  Passed = TestFileBeams() && Passed;
  Passed = TestRemovedBeams() && Passed;
  Passed = TestDeprecatedPosition() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTCosimaInputBeams::SourceWithBeam(const BeamCase& C) const
{
  MString Lines = MString("S.ParticleType 1\nS.Beam ") + C.m_Line + "\n";
  if (C.m_Beam == MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction) {
    // The normalized energy beam flux function brings its own spectrum and flux
    Lines += "S.Spectrum NormalizedEnergyBeamFluxFunction\n";
  } else {
    Lines += "S.Spectrum Mono 511\nS.Flux 1\n";
  }
  if (C.m_Beam == MCSource::c_NearFieldReverseDirectionToPredecessor) {
    // A source with a reverse direction beam must be the successor of another source
    Lines += "R.Source P\nP.ParticleType 1\nP.Beam PointSource 0 0 0\nP.Spectrum Mono 511\nP.Flux 1\nP.Successor S\nS.IsSuccessor true\n";
  }
  return OneSource(Lines);
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputBeams::EvaluateBeam(const BeamCase& C)
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", C.m_Name, "The beam line is accepted by the parser", Open(P, SourceWithBeam(C))) && Passed;
  MCSource* S = GetS(P);
  Passed = EvaluateTrue("GetSource()", C.m_Name, "The source S exists after parsing", S != nullptr) && Passed;
  if (S == nullptr) {
    return Passed;
  }

  Passed = Evaluate("GetBeamType()", C.m_Name, "The beam type is the one of the keyword", S->GetBeamType(), C.m_Beam) && Passed;
  Passed = Evaluate("GetCoordinateSystem()", C.m_Name, "The coordinate system is the one of the keyword", S->GetCoordinateSystem(), C.m_Coordinates) && Passed;
  for (unsigned int p = 0; p < C.m_Position.size(); ++p) {
    // The parameters are read from text and multiplied by their unit: exact to the rounding of a double
    const double Expected = C.m_Position[p];
    Passed = EvaluateNear("GetPositionParameter()", C.m_Name + " parameter " + (p+1), "The position parameter is stored in its unit", S->GetPositionParameter(p+1), Expected, 1e-12*max(1.0, fabs(Expected))) && Passed;
  }
  if (C.m_Volume != "") {
    Passed = Evaluate("GetVolume()", C.m_Name, "The volume name is stored", S->GetVolume(), C.m_Volume) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputBeams::TestBeams()
{
  bool Passed = true;

  // A tiny valid 2D grid (theta and phi in degree) for the diffraction beam reader: MFunction2D reads XA, YA and AP lines
  const MString DiffractionFile = WriteSource("Diffraction.dat", "IP LIN\nXA 0 1\nYA 0 1\nAP 0 0 1\nAP 0 1 1\nAP 1 0 1\nAP 1 1 1\n");

  const int FF = MCSource::c_FarField;
  const int NF = MCSource::c_NearField;
  const vector<double> None;
  const vector<double> Angles = { 12*deg, 34*deg };
  const vector<double> Area = { 10*deg, 80*deg, 20*deg, 160*deg };
  const vector<double> Disk = { 12*deg, 34*deg, 5*deg };
  const vector<double> Asymmetric = { 12*deg, 34*deg, 5*deg, 7*deg, 9*deg };
  const vector<double> Point = { 1*cm, 2*cm, 3*cm };
  const vector<double> Line = { 1*cm, 2*cm, 3*cm, 4*cm, 5*cm, 6*cm };
  const vector<double> Beam = { 1*cm, 2*cm, 3*cm, 0, 0, 1, 4*cm };
  const vector<double> Profile = { 1*cm, 2*cm, 3*cm, 0, 0, 1, 0 };
  const vector<double> Cone = { 1*cm, 2*cm, 3*cm, 0, 0, 1, 10*deg };
  const vector<double> GaussianCone = { 1*cm, 2*cm, 3*cm, 0, 0, 1, 10*deg, 2*deg };
  const vector<double> Fan = { 1*cm, 2*cm, 3*cm, 1, 0, 0, 0, 1, 0, 4*cm };
  // The diffraction point source takes the rotation angle (deg) before the normal vector: position, angle, normal, file
  const vector<double> Diffraction = { 1*cm, 2*cm, 3*cm, 0, 0, 1, 30*deg };

  const vector<BeamCase> Cases = {
    { "FarFieldPointSource", "FarFieldPointSource 12 34", MCSource::c_FarFieldPoint, FF, Angles, "" },
    { "FarFieldPoint alias", "FarFieldPoint 12 34", MCSource::c_FarFieldPoint, FF, Angles, "" },
    { "FFPS alias", "FFPS 12 34", MCSource::c_FarFieldPoint, FF, Angles, "" },
    { "FarFieldAreaSource", "FarFieldAreaSource 10 80 20 160", MCSource::c_FarFieldArea, FF, Area, "" },
    { "FarFieldArea alias", "FarFieldArea 10 80 20 160", MCSource::c_FarFieldArea, FF, Area, "" },
    { "FFAS alias", "FFAS 10 80 20 160", MCSource::c_FarFieldArea, FF, Area, "" },
    { "FarFieldIsotropic", "FarFieldIsotropic", MCSource::c_FarFieldIsotropic, FF, None, "" },
    { "Isotropic alias", "Isotropic", MCSource::c_FarFieldIsotropic, FF, None, "" },
    { "FarFieldDisk", "FarFieldDisk 12 34 5", MCSource::c_FarFieldDisk, FF, Disk, "" },
    { "FFD alias", "FFD 12 34 5", MCSource::c_FarFieldDisk, FF, Disk, "" },
    { "FarFieldGaussian", "FarFieldGaussian 12 34 5", MCSource::c_FarFieldGaussian, FF, Disk, "" },
    { "FFG alias", "FFG 12 34 5", MCSource::c_FarFieldGaussian, FF, Disk, "" },
    { "FarFieldAssymetricGaussian", "FarFieldAssymetricGaussian 12 34 5 7 9", MCSource::c_FarFieldAssymetricGaussian, FF, Asymmetric, "" },
    { "FarFieldAssymGauss alias", "FarFieldAssymGauss 12 34 5 7 9", MCSource::c_FarFieldAssymetricGaussian, FF, Asymmetric, "" },
    { "FFAG alias", "FFAG 12 34 5 7 9", MCSource::c_FarFieldAssymetricGaussian, FF, Asymmetric, "" },
    { "FarFieldEarthOccultation", "FarFieldEarthOccultation false", MCSource::c_FarFieldEarthOccultation, FF, None, "" },
    { "PointSource", "PointSource 1 2 3", MCSource::c_NearFieldPoint, NF, Point, "" },
    { "Point alias", "Point 1 2 3", MCSource::c_NearFieldPoint, NF, Point, "" },
    { "PS alias", "PS 1 2 3", MCSource::c_NearFieldPoint, NF, Point, "" },
    { "RestrictedPointSource", "RestrictedPointSource 0 0 0", MCSource::c_NearFieldRestrictedPoint, NF, None, "" },
    { "RestrictedPoint alias", "RestrictedPoint 0 0 0", MCSource::c_NearFieldRestrictedPoint, NF, None, "" },
    { "PointSourceRestricted alias", "PointSourceRestricted 0 0 0", MCSource::c_NearFieldRestrictedPoint, NF, None, "" },
    { "RPS alias", "RPS 0 0 0", MCSource::c_NearFieldRestrictedPoint, NF, None, "" },
    { "PSR alias", "PSR 0 0 0", MCSource::c_NearFieldRestrictedPoint, NF, None, "" },
    { "LineSource", "LineSource 1 2 3 4 5 6", MCSource::c_NearFieldLine, NF, Line, "" },
    { "Line alias", "Line 1 2 3 4 5 6", MCSource::c_NearFieldLine, NF, Line, "" },
    { "LS alias", "LS 1 2 3 4 5 6", MCSource::c_NearFieldLine, NF, Line, "" },
    { "RestrictedLineSource", "RestrictedLineSource 10 0 0 10 10 0", MCSource::c_NearFieldRestrictedLine, NF, None, "" },
    { "RestrictedLine alias", "RestrictedLine 10 0 0 10 10 0", MCSource::c_NearFieldRestrictedLine, NF, None, "" },
    { "RLS alias", "RLS 10 0 0 10 10 0", MCSource::c_NearFieldRestrictedLine, NF, None, "" },
    { "BoxSource", "BoxSource 1 2 3 4 5 6", MCSource::c_NearFieldBox, NF, Line, "" },
    { "Box alias", "Box 1 2 3 4 5 6", MCSource::c_NearFieldBox, NF, Line, "" },
    { "BS alias", "BS 1 2 3 4 5 6", MCSource::c_NearFieldBox, NF, Line, "" },
    { "SphereSource", "SphereSource 1 2 3 4 5 6", MCSource::c_NearFieldSphere, NF, Line, "" },
    { "Sphere alias", "Sphere 1 2 3 4 5 6", MCSource::c_NearFieldSphere, NF, Line, "" },
    { "SS alias", "SS 1 2 3 4 5 6", MCSource::c_NearFieldSphere, NF, Line, "" },
    { "DiscSource", "DiscSource 0 0 30 1 0 1 9 10 1 0 360", MCSource::c_NearFieldDisk, NF, None, "" },
    { "Disc alias", "Disc 0 0 30 1 0 1 9 10 1 0 360", MCSource::c_NearFieldDisk, NF, None, "" },
    { "DiskSource alias", "DiskSource 0 0 30 1 0 1 9 10 1 0 360", MCSource::c_NearFieldDisk, NF, None, "" },
    { "Disk alias", "Disk 0 0 30 1 0 1 9 10 1 0 360", MCSource::c_NearFieldDisk, NF, None, "" },
    { "DS alias", "DS 0 0 30 1 0 1 9 10 1 0 360", MCSource::c_NearFieldDisk, NF, None, "" },
    { "HomogeneousBeam", "HomogeneousBeam 1 2 3 0 0 1 4", MCSource::c_NearFieldBeam, NF, Beam, "" },
    { "HB alias", "HB 1 2 3 0 0 1 4", MCSource::c_NearFieldBeam, NF, Beam, "" },
    { "DiffractionPointSource", MString("DiffractionPointSource 1 2 3 30 0 0 1 ") + DiffractionFile, MCSource::c_NearFieldDiffractionPoint, NF, Diffraction, "" },
    { "DiffractionPoint alias", MString("DiffractionPoint 1 2 3 30 0 0 1 ") + DiffractionFile, MCSource::c_NearFieldDiffractionPoint, NF, Diffraction, "" },
    { "DPS alias", MString("DPS 1 2 3 30 0 0 1 ") + DiffractionFile, MCSource::c_NearFieldDiffractionPoint, NF, Diffraction, "" },
    { "ConeBeam", "ConeBeam 1 2 3 0 0 1 10", MCSource::c_NearFieldConeBeam, NF, Cone, "" },
    { "CB alias", "CB 1 2 3 0 0 1 10", MCSource::c_NearFieldConeBeam, NF, Cone, "" },
    { "GaussianConeBeam", "GaussianConeBeam 1 2 3 0 0 1 10 2", MCSource::c_NearFieldConeBeamGauss, NF, GaussianCone, "" },
    { "GCB alias", "GCB 1 2 3 0 0 1 10 2", MCSource::c_NearFieldConeBeamGauss, NF, GaussianCone, "" },
    { "FanBeam", "FanBeam 1 2 3 1 0 0 0 1 0 4", MCSource::c_NearFieldFanBeam, NF, Fan, "" },
    { "FB alias", "FB 1 2 3 1 0 0 0 1 0 4", MCSource::c_NearFieldFanBeam, NF, Fan, "" },
    { "IlluminatedDisk", "IlluminatedDisk 1 2 3 4 10 20 30 40", MCSource::c_NearFieldIlluminatedDisk, NF, None, "" },
    { "ID alias", "ID 1 2 3 4 10 20 30 40", MCSource::c_NearFieldIlluminatedDisk, NF, None, "" },
    { "IlluminatedBox", "IlluminatedBox 1 2 3 4 10 20 30 40", MCSource::c_NearFieldIlluminatedSquare, NF, None, "" },
    { "IB alias", "IB 1 2 3 4 10 20 30 40", MCSource::c_NearFieldIlluminatedSquare, NF, None, "" },
    { "Activation", "Activation", MCSource::c_Invalid, MCSource::c_Invalid, None, "" },
    { "ACT alias", "ACT", MCSource::c_Invalid, MCSource::c_Invalid, None, "" },
    { "ReverseDirectionToPredecessor", "ReverseDirectionToPredecessor", MCSource::c_NearFieldReverseDirectionToPredecessor, NF, None, "" },
    { "Volume", "Volume TestVolume", MCSource::c_NearFieldVolume, NF, None, "TestVolume" },
    { "VOL alias", "VOL TestVolume", MCSource::c_NearFieldVolume, NF, None, "TestVolume" }
  };

  for (const BeamCase& C: Cases) {
    Passed = EvaluateBeam(C) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputBeams::TestFileBeams()
{
  bool Passed = true;

  const MString Data = GetExampleDirectory();
  if (Data.IsEmpty() == true) {
    mout<<"MEGALIB is not set: the beams with example data files are not tested"<<endl;
    return Passed;
  }

  const int FF = MCSource::c_FarField;
  const int NF = MCSource::c_NearField;
  const vector<double> None;
  const vector<double> Profile = { 1*cm, 2*cm, 3*cm, 0, 0, 1, 0 };

  const vector<BeamCase> Cases = {
    { "FarFieldFileZenithDependent", "FarFieldFileZenithDependent " + Data + "/AlbedoPhotonBeam.dat", MCSource::c_FarFieldFileZenithDependent, FF, None, "" },
    { "FFFZD alias", "FFFZD " + Data + "/AlbedoPhotonBeam.dat", MCSource::c_FarFieldFileZenithDependent, FF, None, "" },
    { "FileSphereZenithDep alias", "FileSphereZenithDep " + Data + "/AlbedoPhotonBeam.dat", MCSource::c_FarFieldFileZenithDependent, FF, None, "" },
    { "FarFieldNormalizedEnergyBeamFluxFunction", "FarFieldNormalizedEnergyBeamFluxFunction " + Data + "/3DSpherical.dat", MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction, FF, None, "" },
    { "RadialProfileBeam", "RadialProfileBeam 1 2 3 0 0 1 " + Data + "/BeamProfile1D.dat", MCSource::c_NearFieldBeam1DProfile, NF, None, "" },
    { "RPB alias", "RPB 1 2 3 0 0 1 " + Data + "/BeamProfile1D.dat", MCSource::c_NearFieldBeam1DProfile, NF, None, "" },
    { "MapProfileBeam", "MapProfileBeam 1 2 3 0 0 1 " + Data + "/BeamProfile2D.dat", MCSource::c_NearFieldBeam2DProfile, NF, Profile, "" },
    { "ProfiledBeam alias", "ProfiledBeam 1 2 3 0 0 1 " + Data + "/BeamProfile2D.dat", MCSource::c_NearFieldBeam2DProfile, NF, Profile, "" },
    { "MPB alias", "MPB 1 2 3 0 0 1 " + Data + "/BeamProfile2D.dat", MCSource::c_NearFieldBeam2DProfile, NF, Profile, "" },
    { "PB alias", "PB 1 2 3 0 0 1 " + Data + "/BeamProfile2D.dat", MCSource::c_NearFieldBeam2DProfile, NF, Profile, "" },
    { "FlatMap", "FlatMap 1 2 3 0 0 1 0 " + Data + "/FlatMap.dat", MCSource::c_NearFieldFlatMap, NF, None, "" }
  };

  for (const BeamCase& C: Cases) {
    Passed = EvaluateBeam(C) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputBeams::TestRemovedBeams()
{
  bool Passed = true;

  // The k-space diffraction beam was never implemented and has been removed: its keywords are unknown beam types
  const MString DiffractionFile = WriteSource("Diffraction.dat", "IP LIN\nXA 0 1\nYA 0 1\nAP 0 0 1\nAP 0 1 1\nAP 1 0 1\nAP 1 1 1\n");
  for (const char* Keyword: { "DiffractionPointSourceKSpace", "DiffractionPointKSpace", "DPSK" }) {
    const MString Lines = MString("S.ParticleType 1\nS.Beam ") + Keyword + " 1 2 3 30 0 0 1 " + DiffractionFile + "\nS.Spectrum Mono 511\nS.Flux 1\n";
    Passed = ExpectRejected("Open()", Keyword, "The removed k-space diffraction beam is rejected", OneSource(Lines)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputBeams::TestDeprecatedPosition()
{
  bool Passed = true;

  const MString Data = GetExampleDirectory();

  // The deprecated numeric Position keyword: coordinate system, beam type, and the parameters of the beam
  const vector<pair<MString, pair<int, int>>> Cases = {
    { "Position 1 1 12 34", { MCSource::c_FarFieldPoint, MCSource::c_FarField } },
    { "Position 1 2 10 80 20 160", { MCSource::c_FarFieldArea, MCSource::c_FarField } },
    { "Position 2 1 1 2 3", { MCSource::c_NearFieldPoint, MCSource::c_NearField } },
    { "Position 2 2 1 2 3 4 5 6", { MCSource::c_NearFieldLine, MCSource::c_NearField } },
    { "Position 2 3 1 2 3 4 5 6", { MCSource::c_NearFieldBox, MCSource::c_NearField } },
    { "Position 2 4 1 2 3 4 5 6", { MCSource::c_NearFieldSphere, MCSource::c_NearField } },
    { "Position 2 6 0 0 0", { MCSource::c_NearFieldRestrictedPoint, MCSource::c_NearField } },
    { "Position 2 5 1 2 3 0 0 1 4", { MCSource::c_NearFieldBeam, MCSource::c_NearField } },
    { "Position 2 8 1 2 3 0 0 1 " + Data + "/BeamProfile2D.dat", { MCSource::c_NearFieldBeam2DProfile, MCSource::c_NearField } },
    { "Position 2 9 1 2 3 0 0 1 10", { MCSource::c_NearFieldConeBeam, MCSource::c_NearField } },
    { "Position 2 10 1 2 3 0 0 1 10 2", { MCSource::c_NearFieldConeBeamGauss, MCSource::c_NearField } }
  };

  for (const auto& C: Cases) {
    if (C.first.Contains("BeamProfile2D") == true && Data.IsEmpty() == true) {
      mout<<"MEGALIB is not set: the deprecated position with an example data file is not tested"<<endl;
      continue;
    }
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", C.first, "The deprecated numeric position is accepted", Open(P, OneSource("S.ParticleType 1\nS.Spectrum Mono 511\nS.Flux 1\nS." + C.first + "\n"))) && Passed;
    MCSource* S = GetS(P);
    Passed = EvaluateTrue("GetSource()", C.first, "The source S exists after parsing", S != nullptr) && Passed;
    if (S == nullptr) {
      continue;
    }
    Passed = Evaluate("GetBeamType()", C.first, "The beam type is the one of the numeric code", S->GetBeamType(), C.second.first) && Passed;
    Passed = Evaluate("GetCoordinateSystem()", C.first, "The coordinate system is the one of the numeric code", S->GetCoordinateSystem(), C.second.second) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCosimaInputBeams Test;
  return Test.Run() == true ? 0 : 1;
}


/*
 * UTCosimaInputBeams.cc: the end...
 ******************************************************************************/
