/*
 * UTERCSRBayesian.cxx
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
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MERCSRBayesian.h"
#include "MRETrack.h"
#include "UTERCSRShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected functions of MERCSRBayesian
class UTERCSRBayesianAccess : public MERCSRBayesian
{
public:
  //! Default constructor
  UTERCSRBayesianAccess() : MERCSRBayesian() {}
  //! Default destructor
  virtual ~UTERCSRBayesianAccess() {}

  using MERCSRBayesian::VerifyEntries;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the Bayesian Compton sequence reconstruction MERCSRBayesian
class UTERCSRBayesian : public UTERCSRBase
{
public:
  //! Default constructor
  UTERCSRBayesian() : UTERCSRBase("UTERCSRBayesian") {}
  //! Default destructor
  virtual ~UTERCSRBayesian() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the scatter angles of the photon by energy and by geometry
  bool TestPhotonAngles();
  //! Test the angles of the electron track
  bool TestElectronAngles();
  //! Test the lever arm and the material
  bool TestLeverArmAndMaterial();
  //! Test the correction of empty bins
  bool TestVerifyEntries();
  //! Test setting the parameters and the text of the options
  bool TestSetParametersAndToString();

  //! The test telescope
  MGeometryRevan m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSRBayesian::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The test telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  if (m_Geometry.IsScanned() == true) {
    Passed = TestPhotonAngles() && Passed;
    Passed = TestElectronAngles() && Passed;
    Passed = TestLeverArmAndMaterial() && Passed;
    Passed = TestSetParametersAndToString() && Passed;
  }
  Passed = TestVerifyEntries() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scatter angles of the photon by energy and by geometry
bool UTERCSRBayesian::TestPhotonAngles()
{
  bool Passed = true;

  // A photon of 500 keV at the central hit, which deposits 100 keV there
  vector<MVector> Positions = GetThreeSitePositions();
  MREHit* Start = CreateHit(Positions[0], 150.0, MDDetector::c_Strip2D, &m_Geometry);
  MREHit* Central = CreateHit(Positions[1], 100.0, MDDetector::c_Strip2D, &m_Geometry);
  MREHit* Stop = CreateHit(Positions[2], 400.0, MDDetector::c_Calorimeter, &m_Geometry);

  double CosPhiE = MComptonEvent::ComputeCosPhiViaEeEg(100.0, 400.0);
  double CosPhiG = GetCosScatterAngle(Positions[0], Positions[1], Positions[2]);

  Passed = EvaluateNear("CalculateCosPhiE()", "100 keV of 500 keV", "The cosine is 1 - E0/Eg + E0/Ei", MERCSRBayesian::CalculateCosPhiE(Central, 500.0), CosPhiE, 1E-14) && Passed;
  Passed = EvaluateNear("CalculatePhiEInDegree()", "100 keV of 500 keV", "The angle is the arc cosine in degree", MERCSRBayesian::CalculatePhiEInDegree(Central, 500.0), acos(CosPhiE)*c_Deg, 1E-11) && Passed;
  Passed = EvaluateNear("CalculateCosPhiG()", "three sites", "The cosine is the one between the flight directions", MERCSRBayesian::CalculateCosPhiG(Start, Central, Stop), CosPhiG, 1E-14) && Passed;
  Passed = EvaluateNear("CalculatePhiGInDegree()", "three sites", "The angle is the one between the flight directions in degree", MERCSRBayesian::CalculatePhiGInDegree(Start, Central, Stop), acos(CosPhiG)*c_Deg, 1E-11) && Passed;
  Passed = EvaluateNear("CalculateDPhiInDegree()", "three sites", "The value is the difference of the angles by energy and by geometry in degree", MERCSRBayesian::CalculateDPhiInDegree(Start, Central, Stop, 500.0), fabs(acos(CosPhiG) - acos(CosPhiE))*c_Deg, 1E-11) && Passed;
  // Defect REV-G-4: The difference of the cosines uses an electron rest mass of 511.004 keV, and not the 510.999 keV of MEGAlib and of CalculateCosPhiE
  Passed = ExpectedToFail(EvaluateNear("CalculateDCosPhi()", "three sites", "The value is the difference of the cosines by energy and by geometry", MERCSRBayesian::CalculateDCosPhi(Start, Central, Stop, 500.0), CosPhiE - CosPhiG, 1E-13)) && Passed;

  // A right angle: 90 degree by geometry
  MREHit* Right = CreateHit(MVector(4.0, 0.0, 3.0), 400.0, MDDetector::c_Strip2D, &m_Geometry);
  Passed = EvaluateNear("CalculateCosPhiG()", "right angle", "The cosine is zero", MERCSRBayesian::CalculateCosPhiG(Start, Central, Right), 0.0, 1E-15) && Passed;
  Passed = EvaluateNear("CalculatePhiGInDegree()", "right angle", "The angle is 90 degree", MERCSRBayesian::CalculatePhiGInDegree(Start, Central, Right), 90.0, 1E-12) && Passed;

  // Energies without a scattered photon are marked with the values 2 and 180.5 outside of the ranges
  Passed = EvaluateNear("CalculateCosPhiE()", "all energy in the hit", "The value is 2", MERCSRBayesian::CalculateCosPhiE(Central, 100.0), 2.0, 0.0) && Passed;
  Passed = EvaluateNear("CalculateCosPhiE()", "no energy", "The value is 2", MERCSRBayesian::CalculateCosPhiE(Central, 0.0), 2.0, 0.0) && Passed;
  Passed = EvaluateNear("CalculatePhiEInDegree()", "all energy in the hit", "The value is 180.5", MERCSRBayesian::CalculatePhiEInDegree(Central, 100.0), 180.5, 0.0) && Passed;
  Passed = EvaluateNear("CalculatePhiEInDegree()", "no energy", "The value is 180.5", MERCSRBayesian::CalculatePhiEInDegree(Central, 0.0), 180.5, 0.0) && Passed;
  Passed = EvaluateNear("CalculateDCosPhi()", "all energy in the hit", "The value is 2", MERCSRBayesian::CalculateDCosPhi(Start, Central, Stop, 100.0), 2.0, 0.0) && Passed;
  Passed = EvaluateNear("CalculateDCosPhi()", "no energy", "The value is 2", MERCSRBayesian::CalculateDCosPhi(Start, Central, Stop, 0.0), 2.0, 0.0) && Passed;
  Passed = EvaluateNear("CalculateDPhiInDegree()", "all energy in the hit", "The value is 180.5", MERCSRBayesian::CalculateDPhiInDegree(Start, Central, Stop, 100.0), 180.5, 0.0) && Passed;
  Passed = EvaluateNear("CalculateDPhiInDegree()", "no energy", "The value is 180.5", MERCSRBayesian::CalculateDPhiInDegree(Start, Central, Stop, 0.0), 180.5, 0.0) && Passed;

  // The cosine is not limited to the physical range: 100 keV of 120 keV: 1 - 511/20 + 511/120 = -20.3
  Passed = EvaluateNear("CalculateCosPhiE()", "100 keV of 120 keV", "The cosine is below -1", MERCSRBayesian::CalculateCosPhiE(Central, 120.0), MComptonEvent::ComputeCosPhiViaEeEg(100.0, 20.0), 1E-12) && Passed;

  delete Start;
  delete Central;
  delete Stop;
  delete Right;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the angles of the electron track
bool UTERCSRBayesian::TestElectronAngles()
{
  bool Passed = true;

  // A track of 200 keV in the tracker, which points to -z, and a hit in the calorimeter; the photon had 700 keV
  MRETrack* Track = new MRETrack();
  MREHit* TrackStart = CreateHit(MVector(0.0, 0.0, 3.0), 120.0, MDDetector::c_Strip2D, &m_Geometry);
  Track->AddRESE(TrackStart);
  Track->AddRESE(CreateHit(MVector(0.0, 0.0, 2.0), 80.0, MDDetector::c_Strip2D, &m_Geometry));
  Track->SetStartPoint(TrackStart);
  Track->SetFixedDirection(MVector(0.0, 0.0, -1.0));
  MREHit* Central = CreateHit(MVector(1.0, 1.0, -8.0), 500.0, MDDetector::c_Calorimeter, &m_Geometry);

  Passed = EvaluateNear("GetEnergy()", "track", "The track has 200 keV", Track->GetEnergy(), 200.0, 1E-12) && Passed;

  // The angle between the electron and the scattered photon by energies: cos alpha = Ee*(Eg - E0)/(Eg*sqrt(Ee*(Ee + 2*E0))) with Ee = 200 keV and Eg = 500 keV
  double CosAlphaE = 200.0*(500.0 - c_E0)/(500.0*sqrt(200.0*(200.0 + 2*c_E0)));
  // By geometry: between the direction of the track and the line from the track to the hit
  MVector Line = Central->GetPosition() - Track->GetPosition();
  double CosAlphaG = -Line.Z()/sqrt(Line.X()*Line.X() + Line.Y()*Line.Y() + Line.Z()*Line.Z());

  Passed = EvaluateNear("CalculateCosAlphaE()", "200 keV of 700 keV", "The cosine follows from the energies", MERCSRBayesian::CalculateCosAlphaE(Track, Central, 700.0), CosAlphaE, 1E-14) && Passed;
  Passed = EvaluateNear("CalculateAlphaEInDegree()", "200 keV of 700 keV", "The angle is the arc cosine in degree", MERCSRBayesian::CalculateAlphaEInDegree(Track, Central, 700.0), acos(CosAlphaE)*c_Deg, 1E-11) && Passed;
  Passed = EvaluateNear("CalculateCosAlphaG()", "track to -z", "The cosine is the one between the track and the line to the hit", MERCSRBayesian::CalculateCosAlphaG(Track, Central, 700.0), CosAlphaG, 1E-14) && Passed;
  Passed = EvaluateNear("CalculateAlphaGInDegree()", "track to -z", "The angle is the one between the track and the line to the hit in degree", MERCSRBayesian::CalculateAlphaGInDegree(Track, Central, 700.0), acos(CosAlphaG)*c_Deg, 1E-11) && Passed;
  Passed = EvaluateNear("CalculateDCosAlpha()", "200 keV of 700 keV", "The value is the difference of the cosines by energy and by geometry", MERCSRBayesian::CalculateDCosAlpha(Track, Central, 700.0), CosAlphaE - CosAlphaG, 1E-14) && Passed;
  Passed = EvaluateNear("CalculateDAlphaInDegree()", "200 keV of 700 keV", "The value is the difference of the angles by energy and by geometry in degree", MERCSRBayesian::CalculateDAlphaInDegree(Track, Central, 700.0), fabs(acos(CosAlphaE) - acos(CosAlphaG))*c_Deg, 1E-11) && Passed;

  // Energies for which the cosine is not possible: 200 keV of 205 keV: 200*(5 - 511)/(5*sqrt(200*1222)) = -40.9
  Passed = EvaluateNear("CalculateAlphaEInDegree()", "200 keV of 205 keV", "The value is 180.5", MERCSRBayesian::CalculateAlphaEInDegree(Track, Central, 205.0), 180.5, 0.0) && Passed;
  Passed = EvaluateNear("CalculateDAlphaInDegree()", "200 keV of 205 keV", "The value is 180.5", MERCSRBayesian::CalculateDAlphaInDegree(Track, Central, 205.0), 180.5, 0.0) && Passed;

  delete Track;
  delete Central;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the lever arm and the material
bool UTERCSRBayesian::TestLeverArmAndMaterial()
{
  bool Passed = true;

  // Distances 3 cm and 5 cm
  Passed = EvaluateNear("CalculateMinLeverArm()", "3 cm and 5 cm", "The shorter distance is 3 cm", MERCSRBayesian::CalculateMinLeverArm(MVector(1.0, 2.0, 6.0), MVector(1.0, 2.0, 3.0), MVector(4.0, 6.0, 3.0)), 3.0, 1E-14) && Passed;
  Passed = EvaluateNear("CalculateMinLeverArm()", "5 cm and 3 cm", "The shorter distance is 3 cm", MERCSRBayesian::CalculateMinLeverArm(MVector(4.0, 6.0, 3.0), MVector(1.0, 2.0, 3.0), MVector(1.0, 2.0, 6.0)), 3.0, 1E-14) && Passed;
  Passed = EvaluateNear("CalculateMinLeverArm()", "identical positions", "The distance is zero", MERCSRBayesian::CalculateMinLeverArm(MVector(1.0, 2.0, 3.0), MVector(1.0, 2.0, 3.0), MVector(4.0, 6.0, 3.0)), 0.0, 0.0) && Passed;

  // The materials of the telescope: silicon, CsI, germanium, and plastic
  MREHit* Silicon = CreateHit(MVector(0.0, 0.0, 3.0), 100.0, MDDetector::c_Strip2D, &m_Geometry);
  MREHit* CsI = CreateHit(MVector(1.0, 1.0, -8.0), 100.0, MDDetector::c_Calorimeter, &m_Geometry);
  MREHit* Germanium = CreateHit(MVector(30.2, 0.3, 0.5), 100.0, MDDetector::c_Strip3D, &m_Geometry);
  MREHit* Plastic = CreateHit(MVector(0.0, 0.0, 10.0), 100.0, MDDetector::c_Scintillator, &m_Geometry);
  Passed = Evaluate("GetMaterial()", "silicon", "Silicon is material 1", MERCSRBayesian::GetMaterial(Silicon), 1) && Passed;
  Passed = Evaluate("GetMaterial()", "germanium", "Germanium is material 2", MERCSRBayesian::GetMaterial(Germanium), 2) && Passed;
  Passed = Evaluate("GetMaterial()", "CsI", "CsI is material 4", MERCSRBayesian::GetMaterial(CsI), 4) && Passed;
  Passed = Evaluate("GetMaterial()", "plastic", "Another material is material 0", MERCSRBayesian::GetMaterial(Plastic), 0) && Passed;

  delete Silicon;
  delete CsI;
  delete Germanium;
  delete Plastic;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the correction of empty bins
bool UTERCSRBayesian::TestVerifyEntries()
{
  bool Passed = true;

  UTERCSRBayesianAccess CSR;

  // No good entries, but more than one bad entry: one good entry is assumed
  float Good = 0;
  float Bad = 2;
  CSR.VerifyEntries(Good, Bad);
  Passed = EvaluateNear("VerifyEntries()", "0 good, 2 bad", "One good entry is assumed", Good, 1.0F, 0.0F) && Passed;
  Passed = EvaluateNear("VerifyEntries()", "0 good, 2 bad", "The bad entries are not changed", Bad, 2.0F, 0.0F) && Passed;

  Good = 0;
  Bad = 1;
  CSR.VerifyEntries(Good, Bad);
  Passed = EvaluateNear("VerifyEntries()", "0 good, 1 bad", "The good entries are not changed", Good, 0.0F, 0.0F) && Passed;

  Good = 3;
  Bad = 0;
  CSR.VerifyEntries(Good, Bad);
  Passed = EvaluateNear("VerifyEntries()", "3 good, 0 bad", "The good entries are not changed", Good, 3.0F, 0.0F) && Passed;
  Passed = EvaluateNear("VerifyEntries()", "3 good, 0 bad", "The bad entries are not changed", Bad, 0.0F, 0.0F) && Passed;

  Good = 0;
  Bad = 0;
  CSR.VerifyEntries(Good, Bad);
  Passed = EvaluateNear("VerifyEntries()", "0 good, 0 bad", "The good entries are not changed", Good, 0.0F, 0.0F) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test setting the parameters and the text of the options
bool UTERCSRBayesian::TestSetParametersAndToString()
{
  bool Passed = true;

  MERCSRBayesian CSR;

  // A file which does not exist
  MString Missing = GetTemporaryFileName("Missing.mc.goodbad.rsp");
  DisableDefaultStreams();
  bool Accepted = CSR.SetParameters(Missing, &m_Geometry, 0.25, 0.75, 4, false, false);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "missing file", "The parameters are rejected", Accepted) && Passed;

  MString Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "missing file", "The title is listed", Text.Contains("# CSR - Bayesian options:\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "missing file", "The file name is listed", Text.Contains(MString("# Filename:                        ") + Missing + "\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "missing file", "The minimum quality factor is listed", Text.Contains("# QualityFactorMin:                   0.25\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "missing file", "The maximum quality factor is listed", Text.Contains("# QualityFactorMax:                    0.75\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "missing file", "The maximum number of interactions is listed", Text.Contains("# MaxNInteractions:                    4\n")) && Passed;

  // A file with another suffix
  MString WrongSuffix = GetTemporaryFileName("Response.rsp");
  Passed = EvaluateTrue("WriteTextFile()", "response with wrong suffix", "The file can be created", WriteTextFile(WrongSuffix, "# No response\n")) && Passed;
  DisableDefaultStreams();
  Accepted = CSR.SetParameters(WrongSuffix, &m_Geometry);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "file with wrong suffix", "The parameters are rejected", Accepted) && Passed;

  // Only permutations: no files are needed, but the cross sections of the geometry, which the test telescope does not have
  DisableDefaultStreams();
  Accepted = CSR.SetParameters(Missing, &m_Geometry, 0.0, 0.5, 5, true, true);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "only permutations, geometry without cross sections", "The parameters are rejected", Accepted) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTERCSRBayesian Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSRBayesian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
