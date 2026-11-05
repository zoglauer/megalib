/*
 * UTParameterFileBeams.cc
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
#include "UTParameterFileShared.h"

// Standard lib:
#include <cmath>
using namespace std;


//! Unit test class for the beam keywords of MCParameterFile
class UTParameterFileBeams : public UTParameterFileTest
{
public:
  //! Default constructor
  UTParameterFileBeams() : UTParameterFileTest("UTParameterFileBeams") {}
  //! Default destructor
  virtual ~UTParameterFileBeams() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One beam: the keyword line, coordinate system, beam type, and the expected position parameters (angles in rad, lengths in the Geant4 length unit)
  struct BeamCase {
    MString Line;
    int System;
    int Type;
    vector<double> Parameters;
  };

  //! Test all beams which need no data file
  bool TestBeams();
  //! Test the beams which need a data file
  bool TestFileBeams();
  //! Test the far field boundaries
  bool TestFarFieldRanges();
  //! Test that wrong numbers of tokens and invalid values are rejected
  bool TestBeamErrors();
  //! Test special beams: volume, activation, reverse direction
  bool TestSpecialBeams();
  //! Test the deprecated Position keyword
  bool TestDeprecatedPosition();
};


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileBeams::Run()
{
  bool Passed = true;

  if (PrepareFiles() == false) {
    Passed = EvaluateTrue("PrepareFiles()", "private directory", "The private directory with the geometry fixture can be created", false) && Passed;
    Summarize();
    return Passed;
  }

  Passed = TestBeams() && Passed;
  Passed = TestFileBeams() && Passed;
  Passed = TestFarFieldRanges() && Passed;
  Passed = TestBeamErrors() && Passed;
  Passed = TestSpecialBeams() && Passed;
  Passed = TestDeprecatedPosition() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileBeams::TestBeams()
{
  bool Passed = true;

  const int FF = MCSource::c_FarField;
  const int NF = MCSource::c_NearField;
  vector<BeamCase> Cases = {
    { "FarFieldPointSource 30 60", FF, MCSource::c_FarFieldPoint, { 30*deg, 60*deg } },
    { "FarFieldPoint 30 60", FF, MCSource::c_FarFieldPoint, { 30*deg, 60*deg } },
    { "ffps 30 60", FF, MCSource::c_FarFieldPoint, { 30*deg, 60*deg } },
    { "FARFIELDPOINTSOURCE 30 60", FF, MCSource::c_FarFieldPoint, { 30*deg, 60*deg } },
    { "FarFieldAreaSource 10 80 20 340", FF, MCSource::c_FarFieldArea, { 10*deg, 80*deg, 20*deg, 340*deg } },
    { "farfieldarea 10 80 20 340", FF, MCSource::c_FarFieldArea, { 10*deg, 80*deg, 20*deg, 340*deg } },
    { "ffas 10 80 20 340", FF, MCSource::c_FarFieldArea, { 10*deg, 80*deg, 20*deg, 340*deg } },
    { "FarFieldIsotropic", FF, MCSource::c_FarFieldIsotropic, {} },
    { "isotropic", FF, MCSource::c_FarFieldIsotropic, {} },
    { "FarFieldDisk 30 60 5", FF, MCSource::c_FarFieldDisk, { 30*deg, 60*deg, 5*deg } },
    { "ffd 30 60 5", FF, MCSource::c_FarFieldDisk, { 30*deg, 60*deg, 5*deg } },
    { "FarFieldGaussian 30 60 5", FF, MCSource::c_FarFieldGaussian, { 30*deg, 60*deg, 5*deg } },
    { "ffg 30 60 5", FF, MCSource::c_FarFieldGaussian, { 30*deg, 60*deg, 5*deg } },
    { "FarFieldAssymetricGaussian 30 60 5 7 20", FF, MCSource::c_FarFieldAssymetricGaussian, { 30*deg, 60*deg, 5*deg, 7*deg, 20*deg } },
    { "farfieldassymgauss 30 60 5 7 20", FF, MCSource::c_FarFieldAssymetricGaussian, { 30*deg, 60*deg, 5*deg, 7*deg, 20*deg } },
    { "ffag 30 60 5 7 20", FF, MCSource::c_FarFieldAssymetricGaussian, { 30*deg, 60*deg, 5*deg, 7*deg, 20*deg } },
    { "PointSource 1 2 3", NF, MCSource::c_NearFieldPoint, { 1*cm, 2*cm, 3*cm } },
    { "point 1 2 3", NF, MCSource::c_NearFieldPoint, { 1*cm, 2*cm, 3*cm } },
    { "ps 1 2 3", NF, MCSource::c_NearFieldPoint, { 1*cm, 2*cm, 3*cm } },
    { "RestrictedPointSource 1 2 3", NF, MCSource::c_NearFieldRestrictedPoint, { 1*cm, 2*cm, 3*cm } },
    { "psr 1 2 3", NF, MCSource::c_NearFieldRestrictedPoint, { 1*cm, 2*cm, 3*cm } },
    { "LineSource 0 0 0 1 2 3", NF, MCSource::c_NearFieldLine, { 0, 0, 0, 1*cm, 2*cm, 3*cm } },
    { "line 0 0 0 1 2 3", NF, MCSource::c_NearFieldLine, { 0, 0, 0, 1*cm, 2*cm, 3*cm } },
    { "BoxSource 0 0 0 1 2 3", NF, MCSource::c_NearFieldBox, { 0, 0, 0, 1*cm, 2*cm, 3*cm } },
    { "box 0 0 0 1 2 3", NF, MCSource::c_NearFieldBox, { 0, 0, 0, 1*cm, 2*cm, 3*cm } },
    { "SphereSource 0 0 0 1 2 3", NF, MCSource::c_NearFieldSphere, { 0, 0, 0, 1*cm, 2*cm, 3*cm } },
    { "ss 0 0 0 1 2 3", NF, MCSource::c_NearFieldSphere, { 0, 0, 0, 1*cm, 2*cm, 3*cm } },
    { "HomogeneousBeam 0 0 0 0 0 1 1.5", NF, MCSource::c_NearFieldBeam, { 0, 0, 0, 0, 0, 1, 1.5*cm } },
    { "hb 0 0 0 0 0 1 1.5", NF, MCSource::c_NearFieldBeam, { 0, 0, 0, 0, 0, 1, 1.5*cm } },
    { "ConeBeam 0 0 0 0 0 1 5", NF, MCSource::c_NearFieldConeBeam, { 0, 0, 0, 0, 0, 1, 5*deg } },
    { "cb 0 0 0 0 0 1 5", NF, MCSource::c_NearFieldConeBeam, { 0, 0, 0, 0, 0, 1, 5*deg } },
    { "GaussianConeBeam 0 0 0 0 0 1 5 2", NF, MCSource::c_NearFieldConeBeamGauss, { 0, 0, 0, 0, 0, 1, 5*deg, 2*deg } },
    { "gcb 0 0 0 0 0 1 5 2", NF, MCSource::c_NearFieldConeBeamGauss, { 0, 0, 0, 0, 0, 1, 5*deg, 2*deg } },
    { "IlluminatedDisk 0 0 0 10 10 15 10 195", NF, MCSource::c_NearFieldIlluminatedDisk, { 0, 0, 0, 10*cm, 10*deg, 15*deg, 10*deg, 195*deg } },
    { "id 0 0 0 10 10 15 10 195", NF, MCSource::c_NearFieldIlluminatedDisk, { 0, 0, 0, 10*cm, 10*deg, 15*deg, 10*deg, 195*deg } },
    { "IlluminatedBox 0 0 0 10 10 15 10 195", NF, MCSource::c_NearFieldIlluminatedSquare, { 0, 0, 0, 10*cm, 10*deg, 15*deg, 10*deg, 195*deg } },
    { "ib 0 0 0 10 10 15 10 195", NF, MCSource::c_NearFieldIlluminatedSquare, { 0, 0, 0, 10*cm, 10*deg, 15*deg, 10*deg, 195*deg } }
  };

  for (const BeamCase& C: Cases) {
    UTParameterFileAccess P;
    bool Opened = Open(P, OneSource(MString("S.Beam ") + C.Line + "\n"));
    Passed = EvaluateTrue("Open()", C.Line, "The beam is accepted", Opened) && Passed;
    MCSource* S = GetS(P);
    if (S == nullptr) continue;
    Passed = Evaluate("Beam", C.Line, "The coordinate system is set", S->GetCoordinateSystem(), C.System) && Passed;
    Passed = Evaluate("Beam", C.Line, "The beam type is set", S->GetBeamType(), C.Type) && Passed;
    for (unsigned int p = 0; p < C.Parameters.size(); ++p) {
      Passed = EvaluateNear("Beam", C.Line + " parameter " + (p+1), "The parameter is stored in its unit", S->GetPositionParameter(p+1), C.Parameters[p], 1e-9*max(1.0, fabs(C.Parameters[p]))) && Passed;
    }
  }

  // Disk source:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "DiscSource", "The disc source is accepted", Open(P, OneSource("S.Beam DiscSource 0 0 0 0 0 1 0 5 1 0 360\n"))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileBeams::TestFileBeams()
{
  bool Passed = true;

  const MString Directory = GetExampleDirectory();
  if (Directory.IsEmpty() == true) {
    return Passed;
  }

  struct Entry { MString Line; int System; int Type; };
  vector<Entry> Entries = {
    { "FarFieldFileZenithDependent " + Directory + "/AlbedoPhotonBeam.dat", MCSource::c_FarField, MCSource::c_FarFieldFileZenithDependent },
    { "fffzd " + Directory + "/AlbedoPhotonBeam.dat", MCSource::c_FarField, MCSource::c_FarFieldFileZenithDependent },
    { "RadialProfileBeam 0 0 0 0 0 1 " + Directory + "/BeamProfile1D.dat", MCSource::c_NearField, MCSource::c_NearFieldBeam1DProfile },
    { "rpb 0 0 0 0 0 1 " + Directory + "/BeamProfile1D.dat", MCSource::c_NearField, MCSource::c_NearFieldBeam1DProfile },
    { "MapProfileBeam 0 0 0 0 0 1 " + Directory + "/BeamProfile2D.dat", MCSource::c_NearField, MCSource::c_NearFieldBeam2DProfile },
    { "pb 0 0 0 0 0 1 " + Directory + "/BeamProfile2D.dat", MCSource::c_NearField, MCSource::c_NearFieldBeam2DProfile },
    { "MapProfileBeam 0 0 0 30 0 0 1 " + Directory + "/BeamProfile2D.dat", MCSource::c_NearField, MCSource::c_NearFieldBeam2DProfile },
    { "FlatMap 0 0 0 30 0 0 1 " + Directory + "/FlatMap.dat", MCSource::c_NearField, MCSource::c_NearFieldFlatMap }
  };
  for (const Entry& E: Entries) {
    UTParameterFileAccess P;
    bool Opened = Open(P, OneSource(MString("S.Beam ") + E.Line + "\n"));
    Passed = EvaluateTrue("Open()", E.Line, "The beam with a data file is accepted", Opened) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Beam", E.Line, "The coordinate system is set", GetS(P)->GetCoordinateSystem(), E.System) && Passed;
      Passed = Evaluate("Beam", E.Line, "The beam type is set", GetS(P)->GetBeamType(), E.Type) && Passed;
    }
  }

  // The normalized energy beam flux function needs its own spectrum and no flux:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "NormalizedEnergyBeamFluxFunction", "The normalized energy beam flux function is accepted",
                          Open(P, OneSource("S.ParticleType 1\nS.Beam FarFieldNormalizedEnergyBeamFluxFunction " + Directory + "/3DSpherical.dat\nS.Spectrum NormalizedEnergyBeamFluxFunction\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Beam", "NormalizedEnergyBeamFluxFunction", "The beam type is set", GetS(P)->GetBeamType(), MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction) && Passed;
    }
    Passed = ExpectRejected("Open()", "NormalizedEnergyBeamFluxFunction with flux", "A flux for a normalized beam is rejected",
                            OneSource("S.ParticleType 1\nS.Beam FarFieldNormalizedEnergyBeamFluxFunction " + Directory + "/3DSpherical.dat\nS.Spectrum NormalizedEnergyBeamFluxFunction\nS.Flux 1\n")) && Passed;
  }

  // Missing files:
  vector<MString> Missing = { "FarFieldFileZenithDependent ", "RadialProfileBeam 0 0 0 0 0 1 ", "MapProfileBeam 0 0 0 0 0 1 ", "FlatMap 0 0 0 30 0 0 1 ", "FarFieldNormalizedEnergyBeamFluxFunction " };
  for (const MString& M: Missing) {
    Passed = ExpectRejected("Open()", M + "missing", "A beam with a missing data file is rejected", OneSource("S.Beam " + M + GetDirectory() + "/Missing.dat\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileBeams::TestFarFieldRanges()
{
  bool Passed = true;

  // Theta in [0, 180], phi any angle:
  Passed = ExpectAccepted("Open()", "theta 0", "Theta of 0 degrees is accepted", OneSource("S.Beam FarFieldPointSource 0 0\n")) && Passed;
  Passed = ExpectAccepted("Open()", "theta 180", "Theta of 180 degrees is accepted", OneSource("S.Beam FarFieldPointSource 180 0\n")) && Passed;
  Passed = ExpectAccepted("Open()", "theta 90 phi 360", "Phi of 360 degrees is accepted", OneSource("S.Beam FarFieldPointSource 90 360\n")) && Passed;
  Passed = ExpectRejected("Open()", "theta 181", "Theta above 180 degrees is rejected", OneSource("S.Beam FarFieldPointSource 181 0\n")) && Passed;
  Passed = ExpectRejected("Open()", "theta -1", "A negative theta is rejected", OneSource("S.Beam FarFieldPointSource -1 0\n")) && Passed;
  Passed = ExpectRejected("Open()", "theta min above max", "An area with minimum theta above maximum theta is rejected", OneSource("S.Beam FarFieldAreaSource 80 10 0 360\n")) && Passed;
  Passed = ExpectRejected("Open()", "disk negative radius", "A disk with a negative radius is rejected", OneSource("S.Beam FarFieldDisk 30 60 -5\n")) && Passed;
  Passed = ExpectRejected("Open()", "gaussian negative sigma", "A Gaussian with a negative sigma is rejected", OneSource("S.Beam FarFieldGaussian 30 60 -5\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileBeams::TestBeamErrors()
{
  bool Passed = true;

  vector<MString> Bad = {
    "", "Unknown", "Unknown 1 2 3",
    "FarFieldPointSource", "FarFieldPointSource 1", "FarFieldPointSource 1 2 3", "FarFieldPointSource a b",
    "FarFieldAreaSource 1 2 3", "FarFieldAreaSource 1 2 3 4 5",
    "FarFieldIsotropic 1", "FarFieldDisk 1 2", "FarFieldDisk 1 2 3 4",
    "FarFieldGaussian 1 2", "FarFieldAssymetricGaussian 1 2 3 4", "FarFieldFileZenithDependent", "FarFieldNormalizedEnergyBeamFluxFunction",
    "PointSource", "PointSource 1 2", "PointSource 1 2 3 4", "PointSource a b c",
    "RestrictedPointSource 1 2", "LineSource 1 2 3", "LineSource 1 2 3 4 5 6 7", "BoxSource 1 2 3 4 5", "BoxSource 0 0 0 0 0 0",
    "SphereSource 1 2 3", "SphereSource 0 0 0 0 1 1", "DiscSource 1 2 3",
    "HomogeneousBeam 0 0 0 0 0 1", "HomogeneousBeam 0 0 0 0 0 0 1", "HomogeneousBeam 0 0 0 0 0 1 -1",
    "ConeBeam 0 0 0 0 0 1", "ConeBeam 0 0 0 0 0 0 5", "ConeBeam 0 0 0 0 0 1 -5",
    "GaussianConeBeam 0 0 0 0 0 1 5", "GaussianConeBeam 0 0 0 0 0 1 5 -2",
    "IlluminatedDisk 0 0 0 10 10 15 10", "IlluminatedBox 0 0 0 10 10",
    "RadialProfileBeam 0 0 0 0 0 1", "MapProfileBeam 0 0 0 0 0 1", "FlatMap 0 0 0 0 0 1", "Volume", "Volume A B"
  };
  for (const MString& B: Bad) {
    if (B == "FarFieldPointSource a b" || B == "PointSource a b c") {
      // Known failure: the parser converts the coordinates with atof() without checking that the tokens are numbers, so "a" becomes 0 and the beam is accepted
      Passed = ExpectedToFail(ExpectRejected("Open()", MString("Beam ") + B, "The invalid beam is rejected", OneSource(MString("S.Beam ") + B + "\n"))) && Passed;
    } else {
      Passed = ExpectRejected("Open()", MString("Beam ") + B, "The invalid beam is rejected", OneSource(MString("S.Beam ") + B + "\n")) && Passed;
    }
  }
  Passed = ExpectRejected("Open()", "Beam without type", "The beam keyword needs a type", OneSource("S.Beam\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileBeams::TestSpecialBeams()
{
  bool Passed = true;

  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "Volume", "A volume beam is accepted", Open(P, OneSource("S.Beam Volume BigSphere\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Beam", "Volume", "The beam type is volume", GetS(P)->GetBeamType(), MCSource::c_NearFieldVolume) && Passed;
      Passed = Evaluate("Beam", "Volume", "The volume name is stored", GetS(P)->GetVolume(), MString("BigSphere")) && Passed;
    }
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "vol", "The short keyword is accepted", Open(P, OneSource("S.Beam vol Sphere1\n"))) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "ReverseDirectionToPredecessor", "The reverse direction beam after a predecessor is accepted",
                          Open(P, Header() + "Run R\nR.Source A\nR.Source B\nA.Beam PointSource 0 0 0\nA.Successor B\nB.IsSuccessor true\nB.Beam ReverseDirectionToPredecessor\n")) && Passed;
    if (P.GetSource("B") != nullptr) {
      Passed = Evaluate("Beam", "ReverseDirectionToPredecessor", "The beam type is set", P.GetSource("B")->GetBeamType(), MCSource::c_NearFieldReverseDirectionToPredecessor) && Passed;
    }
  }
  Passed = ExpectRejected("Open()", "ReverseDirectionToPredecessor without predecessor", "The reverse direction beam needs a predecessor", OneSource("S.Beam ReverseDirectionToPredecessor\n")) && Passed;
  // Far field Earth occultation beam:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "FarFieldEarthOccultation", "The Earth occultation beam is accepted", Open(P, OneSource("S.Beam FarFieldEarthOccultation false 65\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Beam", "FarFieldEarthOccultation", "The beam type is set", GetS(P)->GetBeamType(), MCSource::c_FarFieldEarthOccultation) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileBeams::TestDeprecatedPosition()
{
  bool Passed = true;

  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "Position far field point", "The deprecated far field point position is accepted",
                          Open(P, OneSource(MString("S.Position ") + MCSource::c_FarField + " 1 30 60\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Position", "far field point", "The beam type is far field point", GetS(P)->GetBeamType(), MCSource::c_FarFieldPoint) && Passed;
      Passed = EvaluateNear("Position", "far field point", "Theta is given in degrees", GetS(P)->GetPositionParameter(1), 30*deg, 1e-9) && Passed;
      Passed = EvaluateNear("Position", "far field point", "Phi is given in degrees", GetS(P)->GetPositionParameter(2), 60*deg, 1e-9) && Passed;
    }
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "Position near field point", "The deprecated near field point position is accepted",
                          Open(P, OneSource(MString("S.Position ") + MCSource::c_NearField + " 1 1 2 3\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Position", "near field point", "The beam type is near field point", GetS(P)->GetBeamType(), MCSource::c_NearFieldPoint) && Passed;
      Passed = EvaluateNear("Position", "near field point", "The z coordinate is given in cm", GetS(P)->GetPositionParameter(3), 3*cm, 1e-9) && Passed;
    }
  }
  Passed = ExpectRejected("Open()", "Position too short", "A position without parameters is rejected", OneSource("S.Position 1\n")) && Passed;
  Passed = ExpectRejected("Open()", "Position unknown combination", "An unknown combination of coordinate system and beam type is rejected", OneSource(MString("S.Position ") + MCSource::c_FarField + " 99 1 2\n")) && Passed;
  Passed = ExpectRejected("Open()", "Position far field wrong count", "A far field point with three angles is rejected", OneSource(MString("S.Position ") + MCSource::c_FarField + " 1 30 60 90\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTParameterFileBeams Test;
  return Test.Run() == true ? 0 : 1;
}
