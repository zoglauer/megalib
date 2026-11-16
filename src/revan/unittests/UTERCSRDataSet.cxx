/*
 * UTERCSRDataSet.cxx
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
#include <cstring>
#include <new>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TMVA/Reader.h"
#include "TROOT.h"
#include "TTree.h"

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MERCSRDataSet.h"
#include "MGeometryRevan.h"
#include "MGlobal.h"
#include "MRESE.h"
#include "MString.h"
#include "MVector.h"
#include "UTERDataSetShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the data set of the TMVA Compton sequence reconstruction MERCSRDataSet
class UTERCSRDataSet : public UTERDataSetBase
{
public:
  //! Default constructor
  UTERCSRDataSet() : UTERDataSetBase("UTERCSRDataSet") {}
  //! Default destructor
  virtual ~UTERCSRDataSet() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructors on memory with a pattern
  bool TestConstruction();
  //! Test the initialization
  bool TestInitialize();
  //! Test filling a sequence of three hits
  bool TestFill();
  //! Test filling a sequence of two hits, with and without the path to the first interaction
  bool TestFillTwoHits();
  //! Test filling a sequence beyond the limits of the Compton kinematics
  bool TestFillKinematicLimits();
  //! Test the store of the path values of sequences of four hits
  bool TestFillStore();
  //! Test sequences which do not fit to the data set
  bool TestFillWrongLength();
  //! Test the trees
  bool TestCreateTrees();
  //! Test the readers
  bool TestCreateReaders();

  //! The expected average, maximum, and minimum of the Compton absorption probabilities along the Compton cone, with int(18 sin(Phi)) (at least 1) samples
  void GetExpectedPath(MGeometryRevan& Geometry, const MVector& First, const MVector& Second, double Phi, double Energy, double& Average, double& Maximum, double& Minimum, unsigned int& Steps) const;
  //! Fill a sequence of three hits into a data set for sequences of up to two hits
  bool FillTooLongSequence();
  //! Fill a sequence of one hit
  bool FillSingleHit();
  //! Fill a sequence of two hits into a default constructed data set
  bool FillDefaultConstructed();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSRDataSet::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestFill() && Passed;
  Passed = TestFillTwoHits() && Passed;
  Passed = TestFillKinematicLimits() && Passed;
  Passed = TestFillStore() && Passed;
  Passed = TestFillWrongLength() && Passed;
  Passed = TestCreateTrees() && Passed;
  Passed = TestCreateReaders() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! The expected average, maximum, and minimum of the Compton absorption probabilities along the Compton cone, with int(18 sin(Phi)) (at least 1) samples
void UTERCSRDataSet::GetExpectedPath(MGeometryRevan& Geometry, const MVector& First, const MVector& Second, double Phi, double Energy, double& Average, double& Maximum, double& Minimum, unsigned int& Steps) const
{
  // One sample every 20 degrees for a Compton scatter angle of 90 degrees, fewer for narrower cones
  Steps = static_cast<unsigned int>(18*sin(Phi));
  if (Steps < 1) Steps = 1;

  vector<double> Cone = GetConeProbabilities(Geometry, First, Second, Phi, Energy, Steps);
  Average = 0.0;
  Maximum = Cone[0];
  Minimum = Cone[0];
  for (unsigned int s = 0; s < Steps; ++s) {
    Average += Cone[s];
    if (Cone[s] > Maximum) Maximum = Cone[s];
    if (Cone[s] < Minimum) Minimum = Cone[s];
  }
  Average /= Steps;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill a sequence of three hits into a data set for sequences of up to two hits
bool UTERCSRDataSet::FillTooLongSequence()
{
  MGeometryRevan Geometry;
  ScanTelescopeGeometry(Geometry);
  LoadTestCrossSections(Geometry);

  vector<MRESE*> Sequence = CreateSequence({ MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0), MVector(3.0, -3.0, -12.0) }, { 200.0, 150.0, 311.657 });
  MERCSRDataSet DataSet(2, false);
  DataSet.Fill(1, Sequence, &Geometry);
  DeleteSequence(Sequence);

  if (DataSet.m_Energies.size() != 1) return false;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill a sequence of one hit
bool UTERCSRDataSet::FillSingleHit()
{
  MGeometryRevan Geometry;
  ScanTelescopeGeometry(Geometry);
  LoadTestCrossSections(Geometry);

  vector<MRESE*> Sequence = CreateSequence({ MVector(1.0, 2.0, 3.0) }, { 200.0 });
  MERCSRDataSet DataSet(3, false);
  DataSet.Fill(1, Sequence, &Geometry);
  DeleteSequence(Sequence);

  if (DataSet.m_Energies.size() != 2) return false;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill a sequence of two hits into a default constructed data set
bool UTERCSRDataSet::FillDefaultConstructed()
{
  MGeometryRevan Geometry;
  ScanTelescopeGeometry(Geometry);
  LoadTestCrossSections(Geometry);

  vector<MRESE*> Sequence = CreateSequence({ MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0) }, { 200.0, 461.657 });
  MERCSRDataSet DataSet;
  DataSet.Fill(1, Sequence, &Geometry);
  DeleteSequence(Sequence);

  if (DataSet.m_Energies.size() != 0) return false;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructors on memory with a pattern
bool UTERCSRDataSet::TestConstruction()
{
  bool Passed = true;

  // The default constructor
  {
    alignas(MERCSRDataSet) unsigned char Block[sizeof(MERCSRDataSet)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MERCSRDataSet* DataSet = new (Block) MERCSRDataSet();
    Passed = EvaluateSize("MERCSRDataSet()", "memory with 0xA5", "There are no simulation IDs", DataSet->m_SimulationIDs.size(), 0) && Passed;
    Passed = EvaluateSize("MERCSRDataSet()", "memory with 0xA5", "There are no energies", DataSet->m_Energies.size(), 0) && Passed;
    Passed = EvaluateSize("MERCSRDataSet()", "memory with 0xA5", "There are no zenith angles", DataSet->m_ZenithAngle.size(), 0) && Passed;
    DataSet->~MERCSRDataSet();
  }

  // The standard constructor: the sequence lengths 2 and 3
  {
    alignas(MERCSRDataSet) unsigned char Block[sizeof(MERCSRDataSet)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MERCSRDataSet* DataSet = new (Block) MERCSRDataSet(3, true);
    Passed = EvaluateSize("MERCSRDataSet(3, true)", "memory with 0xA5", "There are simulation IDs for two sequence lengths", DataSet->m_SimulationIDs.size(), 2) && Passed;
    Passed = EvaluateSize("MERCSRDataSet(3, true)", "memory with 0xA5", "There are energies for two sequence lengths", DataSet->m_Energies.size(), 2) && Passed;
    Passed = EvaluateSize("MERCSRDataSet(3, true)", "memory with 0xA5", "There are angle differences for one sequence length", DataSet->m_CosComptonScatterAngleDifference.size(), 1) && Passed;
    if (DataSet->m_Energies.size() == 2 && DataSet->m_CosComptonScatterAngleDifference.size() == 1) {
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 2", "There are two energies", DataSet->m_Energies[0].size(), 2) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 3", "There are three energies", DataSet->m_Energies[1].size(), 3) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 2", "There are two x positions", DataSet->m_PositionsX[0].size(), 2) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 3", "There are three y positions", DataSet->m_PositionsY[1].size(), 3) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 3", "There are three z positions", DataSet->m_PositionsZ[1].size(), 3) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 2", "There is one interaction distance", DataSet->m_InteractionDistances[0].size(), 1) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 3", "There are two interaction distances", DataSet->m_InteractionDistances[1].size(), 2) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 2", "There is one Compton scatter angle", DataSet->m_CosComptonScatterAngles[0].size(), 1) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 3", "There are two Compton scatter angles", DataSet->m_CosComptonScatterAngles[1].size(), 2) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 3", "There are two Klein-Nishina values", DataSet->m_KleinNishinaProbability[1].size(), 2) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 3", "There is one Compton scatter angle difference", DataSet->m_CosComptonScatterAngleDifference[0].size(), 1) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 2", "There is one absorption probability", DataSet->m_AbsorptionProbabilities[0].size(), 1) && Passed;
      Passed = EvaluateSize("MERCSRDataSet(3, true)", "sequence length 3", "There are two absorption probabilities", DataSet->m_AbsorptionProbabilities[1].size(), 2) && Passed;
      Passed = Evaluate("MERCSRDataSet(3, true)", "sequence length 2", "The simulation ID is zero", DataSet->m_SimulationIDs[0], static_cast<Long64_t>(0)) && Passed;
      Passed = EvaluateNear("MERCSRDataSet(3, true)", "sequence length 3", "The energies are zero", DataSet->m_Energies[1][2], 0.0, 0.0) && Passed;
    }
    Passed = EvaluateSize("MERCSRDataSet(3, true)", "memory with 0xA5", "There are two average absorption probabilities to the first interaction", DataSet->m_AbsorptionProbabilityToFirstIAAverage.size(), 2) && Passed;
    Passed = EvaluateSize("MERCSRDataSet(3, true)", "memory with 0xA5", "There are two maximum absorption probabilities to the first interaction", DataSet->m_AbsorptionProbabilityToFirstIAMaximum.size(), 2) && Passed;
    Passed = EvaluateSize("MERCSRDataSet(3, true)", "memory with 0xA5", "There are two minimum absorption probabilities to the first interaction", DataSet->m_AbsorptionProbabilityToFirstIAMinimum.size(), 2) && Passed;
    Passed = EvaluateSize("MERCSRDataSet(3, true)", "memory with 0xA5", "There are two zenith angles", DataSet->m_ZenithAngle.size(), 2) && Passed;
    Passed = EvaluateSize("MERCSRDataSet(3, true)", "memory with 0xA5", "There are two nadir angles", DataSet->m_NadirAngle.size(), 2) && Passed;
    DataSet->~MERCSRDataSet();
  }

  // Without the path to the first interaction
  MERCSRDataSet NoPath(3, false);
  Passed = EvaluateSize("MERCSRDataSet(3, false)", "no path", "There are energies for two sequence lengths", NoPath.m_Energies.size(), 2) && Passed;
  Passed = EvaluateSize("MERCSRDataSet(3, false)", "no path", "There are no average absorption probabilities to the first interaction", NoPath.m_AbsorptionProbabilityToFirstIAAverage.size(), 0) && Passed;
  Passed = EvaluateSize("MERCSRDataSet(3, false)", "no path", "There are no zenith angles", NoPath.m_ZenithAngle.size(), 0) && Passed;
  Passed = EvaluateSize("MERCSRDataSet(3, false)", "no path", "There are no nadir angles", NoPath.m_NadirAngle.size(), 0) && Passed;

  // The path is not used by default
  MERCSRDataSet Default(2);
  Passed = EvaluateSize("MERCSRDataSet(2)", "default", "There are no zenith angles: the path to the first interaction is not used by default", Default.m_ZenithAngle.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the initialization
bool UTERCSRDataSet::TestInitialize()
{
  bool Passed = true;

  // The smallest data set
  MERCSRDataSet DataSet;
  DataSet.Initialize(2);
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There is one sequence length", DataSet.m_SimulationIDs.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There are energies for one sequence length", DataSet.m_Energies.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There are no Compton scatter angle differences", DataSet.m_CosComptonScatterAngleDifference.size(), 0) && Passed;

  // Larger, with path
  DataSet.Initialize(5, true);
  Passed = EvaluateSize("Initialize(5, true)", "after (2)", "There are four sequence lengths", DataSet.m_SimulationIDs.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(5, true)", "after (2)", "There are energies for four sequence lengths", DataSet.m_Energies.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(5, true)", "after (2)", "There are Compton scatter angle differences for three sequence lengths", DataSet.m_CosComptonScatterAngleDifference.size(), 3) && Passed;
  Passed = EvaluateSize("Initialize(5, true)", "after (2)", "There are four zenith angles", DataSet.m_ZenithAngle.size(), 4) && Passed;
  if (DataSet.m_Energies.size() == 4 && DataSet.m_CosComptonScatterAngleDifference.size() == 3) {
    Passed = EvaluateSize("Initialize(5, true)", "sequence length 5", "There are five energies", DataSet.m_Energies[3].size(), 5) && Passed;
    Passed = EvaluateSize("Initialize(5, true)", "sequence length 5", "There are four interaction distances", DataSet.m_InteractionDistances[3].size(), 4) && Passed;
    Passed = EvaluateSize("Initialize(5, true)", "sequence length 5", "There are three Compton scatter angle differences", DataSet.m_CosComptonScatterAngleDifference[2].size(), 3) && Passed;
    Passed = EvaluateSize("Initialize(5, true)", "sequence length 4", "There are two Compton scatter angle differences", DataSet.m_CosComptonScatterAngleDifference[1].size(), 2) && Passed;
    Passed = EvaluateSize("Initialize(5, true)", "sequence length 4", "There are three absorption probabilities", DataSet.m_AbsorptionProbabilities[2].size(), 3) && Passed;
  }

  // Smaller again, without path: nothing of the larger one stays
  DataSet.Initialize(3, false);
  Passed = EvaluateSize("Initialize(3, false)", "after (5, true)", "There are two sequence lengths", DataSet.m_SimulationIDs.size(), 2) && Passed;
  Passed = EvaluateSize("Initialize(3, false)", "after (5, true)", "There are energies for two sequence lengths", DataSet.m_Energies.size(), 2) && Passed;
  Passed = EvaluateSize("Initialize(3, false)", "after (5, true)", "There are x positions for two sequence lengths", DataSet.m_PositionsX.size(), 2) && Passed;
  Passed = EvaluateSize("Initialize(3, false)", "after (5, true)", "There are Compton scatter angle differences for one sequence length", DataSet.m_CosComptonScatterAngleDifference.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(3, false)", "after (5, true)", "There are no zenith angles", DataSet.m_ZenithAngle.size(), 0) && Passed;
  Passed = EvaluateSize("Initialize(3, false)", "after (5, true)", "There are no minimum absorption probabilities to the first interaction", DataSet.m_AbsorptionProbabilityToFirstIAMinimum.size(), 0) && Passed;

  // A sequence needs two hits: shorter ones are refused with an error message, and the data set is empty
  DisableDefaultStreams();
  DataSet.Initialize(1, true);
  EnableDefaultStreams();
  Passed = EvaluateSize("Initialize(1, true)", "after (3, false)", "There are no sequence lengths", DataSet.m_SimulationIDs.size(), 0) && Passed;
  Passed = EvaluateSize("Initialize(1, true)", "after (3, false)", "There are no energies", DataSet.m_Energies.size(), 0) && Passed;
  Passed = EvaluateSize("Initialize(1, true)", "after (3, false)", "There are no absorption probabilities", DataSet.m_AbsorptionProbabilities.size(), 0) && Passed;
  Passed = EvaluateSize("Initialize(1, true)", "after (3, false)", "There are no zenith angles", DataSet.m_ZenithAngle.size(), 0) && Passed;

  DisableDefaultStreams();
  DataSet.Initialize(3, true);
  DataSet.Initialize(0, true);
  EnableDefaultStreams();
  Passed = EvaluateSize("Initialize(0, true)", "after (3, true)", "There are no sequence lengths", DataSet.m_SimulationIDs.size(), 0) && Passed;
  Passed = EvaluateSize("Initialize(0, true)", "after (3, true)", "There are no nadir angles", DataSet.m_NadirAngle.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling a sequence of three hits
bool UTERCSRDataSet::TestFill()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;
  Passed = EvaluateTrue("LoadTestCrossSections()", "telescope", "The cross sections can be loaded", LoadTestCrossSections(Geometry)) && Passed;

  // A Compton sequence of 661.657 keV: first interaction in the top tracker layer, the other two in the calorimeter
  vector<MVector> Positions = { MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0), MVector(3.0, -3.0, -12.0) };
  vector<double> Energies = { 200.0, 150.0, 311.657 };
  vector<MRESE*> Sequence = CreateSequence(Positions, Energies);

  MERCSRDataSet DataSet(3, true);
  DataSet.Fill(77, Sequence, &Geometry);

  // A sequence of three hits is stored at index 1, and the data of the sequences of two hits is not touched
  Passed = Evaluate("Fill()", "three hits, ID 77", "The simulation ID is stored for the sequence length 3", DataSet.m_SimulationIDs[1], static_cast<Long64_t>(77)) && Passed;
  Passed = Evaluate("Fill()", "three hits, ID 77", "The simulation ID of the sequence length 2 is not changed", DataSet.m_SimulationIDs[0], static_cast<Long64_t>(0)) && Passed;
  Passed = EvaluateNear("Fill()", "three hits", "The energies of the sequence length 2 are not changed", DataSet.m_Energies[0][0], 0.0, 0.0) && Passed;

  // (a) The hits
  for (unsigned int h = 0; h < 3; ++h) {
    Passed = EvaluateNear("Fill()", MString("hit ") + h, "The energy is stored", DataSet.m_Energies[1][h], Energies[h], GetFloatTolerance(Energies[h])) && Passed;
    Passed = EvaluateNear("Fill()", MString("hit ") + h, "The x position is stored", DataSet.m_PositionsX[1][h], Positions[h].X(), 0.0) && Passed;
    Passed = EvaluateNear("Fill()", MString("hit ") + h, "The y position is stored", DataSet.m_PositionsY[1][h], Positions[h].Y(), 0.0) && Passed;
    Passed = EvaluateNear("Fill()", MString("hit ") + h, "The z position is stored", DataSet.m_PositionsZ[1][h], Positions[h].Z(), 0.0) && Passed;
  }

  // (b) The distances: |(-3, -1.5, -11)| = sqrt(132.25) = 11.5 and |(5, -3.5, -4)| = sqrt(53.25)
  Passed = EvaluateNear("Fill()", "hit 0 to 1", "The interaction distance is 11.5 cm", DataSet.m_InteractionDistances[1][0], 11.5, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "hit 1 to 2", "The interaction distance is sqrt(53.25) cm", DataSet.m_InteractionDistances[1][1], sqrt(53.25), GetFloatTolerance(sqrt(53.25))) && Passed;

  // (c) The Compton scatter angles: the first electron has 200 keV and the gamma ray 461.657 keV afterwards, the second electron 150 keV and the gamma ray 311.657 keV
  double CosPhi0 = MComptonEvent::ComputeCosPhiViaEeEg(200.0, 461.657);
  double CosPhi1 = MComptonEvent::ComputeCosPhiViaEeEg(150.0, 311.657);
  double Phi0 = acos(CosPhi0);
  Passed = EvaluateNear("Fill()", "first interaction", "The cosine of the Compton scatter angle is the one of the Compton equation", DataSet.m_CosComptonScatterAngles[1][0], CosPhi0, GetFloatTolerance(CosPhi0)) && Passed;
  Passed = EvaluateNear("Fill()", "second interaction", "The cosine of the Compton scatter angle is the one of the Compton equation", DataSet.m_CosComptonScatterAngles[1][1], CosPhi1, GetFloatTolerance(CosPhi1)) && Passed;

  // The Klein-Nishina value for the energy of the gamma ray before the interaction: 661.657 keV and 461.657 keV
  double KleinNishina0 = MComptonEvent::GetKleinNishinaNormalizedByArea(661.657, Phi0);
  double KleinNishina1 = MComptonEvent::GetKleinNishinaNormalizedByArea(461.657, acos(CosPhi1));
  Passed = EvaluateNear("Fill()", "first interaction", "The Klein-Nishina value is the one for 661.657 keV", DataSet.m_KleinNishinaProbability[1][0], KleinNishina0, GetFloatTolerance(KleinNishina0)) && Passed;
  Passed = EvaluateNear("Fill()", "second interaction", "The Klein-Nishina value is the one for 461.657 keV", DataSet.m_KleinNishinaProbability[1][1], KleinNishina1, GetFloatTolerance(KleinNishina1)) && Passed;

  // (d) The geometric scatter angle at the second hit minus the kinematic one: (-3, -1.5, -11) * (5, -3.5, -4) = 34.25
  double CosPhiGeometry = 34.25/(11.5*sqrt(53.25));
  Passed = EvaluateNear("Fill()", "second interaction", "The difference of the geometric and the kinematic cosine of the Compton scatter angle is stored", DataSet.m_CosComptonScatterAngleDifference[0][0], CosPhiGeometry - CosPhi1, 2*GetFloatTolerance(1.0)) && Passed;

  // (e) The absorption probabilities between the hits for the energy of the gamma ray on this path
  double Absorption0 = Geometry.GetAbsorptionProbability(Positions[0], Positions[1], 461.657);
  double Absorption1 = Geometry.GetAbsorptionProbability(Positions[1], Positions[2], 311.657);
  Passed = EvaluateNear("Fill()", "hit 0 to 1", "The absorption probability is the one for 461.657 keV", DataSet.m_AbsorptionProbabilities[1][0], Absorption0, GetFloatTolerance(Absorption0)) && Passed;
  Passed = EvaluateNear("Fill()", "hit 1 to 2", "The absorption probability is the one for 311.657 keV", DataSet.m_AbsorptionProbabilities[1][1], Absorption1, GetFloatTolerance(Absorption1)) && Passed;

  // (f) The Compton absorption probabilities along the Compton cone to the first hit, for the full energy: cos(Phi) = 0.6654, sin(Phi) = 0.746, thus int(18 x 0.746) = 13 samples
  double Average = 0.0;
  double Maximum = 0.0;
  double Minimum = 0.0;
  unsigned int Steps = 0;
  GetExpectedPath(Geometry, Positions[0], Positions[1], Phi0, 661.657, Average, Maximum, Minimum, Steps);
  Passed = Evaluate("GetExpectedPath()", "three hits", "There are 13 samples on the cone", Steps, 13U) && Passed;
  Passed = EvaluateTrue("GetExpectedPath()", "fixture", "The paths along the cone cross different amounts of material", Maximum - Minimum > 0.001) && Passed;
  // The average is summed with single precision
  Passed = EvaluateNear("Fill()", "three hits", "The average absorption probability to the first interaction is the one of the samples on the cone", DataSet.m_AbsorptionProbabilityToFirstIAAverage[1], Average, Steps*GetFloatTolerance(1.0)) && Passed;
  Passed = EvaluateNear("Fill()", "three hits", "The maximum absorption probability to the first interaction is the one of the samples on the cone", DataSet.m_AbsorptionProbabilityToFirstIAMaximum[1], Maximum, GetFloatTolerance(Maximum) + 1e-9) && Passed;
  Passed = EvaluateNear("Fill()", "three hits", "The minimum absorption probability to the first interaction is the one of the samples on the cone", DataSet.m_AbsorptionProbabilityToFirstIAMinimum[1], Minimum, GetFloatTolerance(Minimum) + 1e-9) && Passed;

  // (g) The closest approach of the Compton cone to the zenith and the nadir in radians: the cone axis is (3, 1.5, 11) with length 11.5,
  // its angle to the zenith (0, 0, 1) is acos(11/11.5) and to the nadir (0, 0, -1) it is acos(-11/11.5)
  double Zenith = acos(11.0/11.5) - Phi0;
  double Nadir = acos(-11.0/11.5) - Phi0;
  // Defect REV-D-12: The zenith and nadir angles are calculated with the direction "zenith minus position of the first hit", which mixes a direction and a position
  Passed = ExpectedToFail(EvaluateNear("Fill()", "three hits", "The zenith angle is the angle of the cone axis to the zenith minus the Compton scatter angle", DataSet.m_ZenithAngle[1], Zenith, GetFloatTolerance(Zenith))) && Passed;
  Passed = ExpectedToFail(EvaluateNear("Fill()", "three hits", "The nadir angle is the angle of the cone axis to the nadir minus the Compton scatter angle", DataSet.m_NadirAngle[1], Nadir, GetFloatTolerance(Nadir))) && Passed;

  // The path values of the sequence length 2 are not touched
  Passed = EvaluateNear("Fill()", "three hits", "The zenith angle of the sequence length 2 is not changed", DataSet.m_ZenithAngle[0], 0.0, 0.0) && Passed;


  // A second sequence of the same event in another order replaces the first one
  vector<MRESE*> Reversed = { Sequence[2], Sequence[1], Sequence[0] };
  DataSet.Fill(77, Reversed, &Geometry);
  Passed = EvaluateNear("Fill()", "reversed sequence", "The first energy is replaced", DataSet.m_Energies[1][0], 311.657, GetFloatTolerance(311.657)) && Passed;
  Passed = EvaluateNear("Fill()", "reversed sequence", "The last z position is replaced", DataSet.m_PositionsZ[1][2], 3.0, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "reversed sequence", "The interaction distances are reversed", DataSet.m_InteractionDistances[1][1], 11.5, 0.0) && Passed;
  double ReversedCosPhi0 = MComptonEvent::ComputeCosPhiViaEeEg(311.657, 350.0);
  Passed = EvaluateNear("Fill()", "reversed sequence", "The cosine of the first Compton scatter angle is replaced", DataSet.m_CosComptonScatterAngles[1][0], ReversedCosPhi0, GetFloatTolerance(ReversedCosPhi0)) && Passed;

  DeleteSequence(Sequence);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling a sequence of two hits, with and without the path to the first interaction
bool UTERCSRDataSet::TestFillTwoHits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;
  Passed = EvaluateTrue("LoadTestCrossSections()", "telescope", "The cross sections can be loaded", LoadTestCrossSections(Geometry)) && Passed;

  // 1173.25 keV: 400 keV in the calorimeter, then back up into the tracker
  vector<MVector> Positions = { MVector(-1.0, -2.0, -9.0), MVector(2.0, 2.0, 2.0) };
  vector<double> Energies = { 400.0, 773.25 };
  vector<MRESE*> Sequence = CreateSequence(Positions, Energies);

  double CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(400.0, 773.25);
  double Phi = acos(CosPhi);
  double KleinNishina = MComptonEvent::GetKleinNishinaNormalizedByArea(1173.25, Phi);
  // The gamma ray has 773.25 keV between the hits
  double Absorption = Geometry.GetAbsorptionProbability(Positions[0], Positions[1], 773.25);

  // Without the path
  MERCSRDataSet NoPath(3, false);
  NoPath.Fill(-5, Sequence, &Geometry);
  Passed = Evaluate("Fill()", "two hits, ID -5, no path", "The simulation ID is stored for the sequence length 2", NoPath.m_SimulationIDs[0], static_cast<Long64_t>(-5)) && Passed;
  Passed = Evaluate("Fill()", "two hits, no path", "The simulation ID of the sequence length 3 is not changed", NoPath.m_SimulationIDs[1], static_cast<Long64_t>(0)) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The first energy is stored", NoPath.m_Energies[0][0], 400.0, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The second energy is stored", NoPath.m_Energies[0][1], 773.25, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The first x position is stored", NoPath.m_PositionsX[0][0], -1.0, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The second y position is stored", NoPath.m_PositionsY[0][1], 2.0, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The first z position is stored", NoPath.m_PositionsZ[0][0], -9.0, 0.0) && Passed;
  // |(3, 4, 11)| = sqrt(146)
  Passed = EvaluateNear("Fill()", "two hits, no path", "The interaction distance is sqrt(146) cm", NoPath.m_InteractionDistances[0][0], sqrt(146.0), GetFloatTolerance(sqrt(146.0))) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The cosine of the Compton scatter angle is the one of the Compton equation", NoPath.m_CosComptonScatterAngles[0][0], CosPhi, GetFloatTolerance(CosPhi)) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The Klein-Nishina value is the one for 1173.25 keV", NoPath.m_KleinNishinaProbability[0][0], KleinNishina, GetFloatTolerance(KleinNishina)) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The absorption probability is the one for 773.25 keV", NoPath.m_AbsorptionProbabilities[0][0], Absorption, GetFloatTolerance(Absorption)) && Passed;
  Passed = EvaluateSize("Fill()", "two hits, no path", "There are still no zenith angles", NoPath.m_ZenithAngle.size(), 0) && Passed;
  Passed = EvaluateSize("Fill()", "two hits, no path", "There are still no average absorption probabilities to the first interaction", NoPath.m_AbsorptionProbabilityToFirstIAAverage.size(), 0) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, no path", "The data of the sequence length 3 is not changed", NoPath.m_Energies[1][0], 0.0, 0.0) && Passed;

  // With the path: cos(Phi) = 1 - 511/773.25 + 511/1173.25 = 0.7747, sin(Phi) = 0.632, thus int(18 x 0.632) = 11 samples
  MERCSRDataSet Path(2, true);
  Path.Fill(-5, Sequence, &Geometry);
  double Average = 0.0;
  double Maximum = 0.0;
  double Minimum = 0.0;
  unsigned int Steps = 0;
  GetExpectedPath(Geometry, Positions[0], Positions[1], Phi, 1173.25, Average, Maximum, Minimum, Steps);
  Passed = Evaluate("GetExpectedPath()", "two hits", "There are 11 samples on the cone", Steps, 11U) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, path", "The absorption probability is the one for 773.25 keV", Path.m_AbsorptionProbabilities[0][0], Absorption, GetFloatTolerance(Absorption)) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, path", "The average absorption probability to the first interaction is the one of the samples on the cone", Path.m_AbsorptionProbabilityToFirstIAAverage[0], Average, Steps*GetFloatTolerance(1.0)) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, path", "The maximum absorption probability to the first interaction is the one of the samples on the cone", Path.m_AbsorptionProbabilityToFirstIAMaximum[0], Maximum, GetFloatTolerance(Maximum) + 1e-9) && Passed;
  Passed = EvaluateNear("Fill()", "two hits, path", "The minimum absorption probability to the first interaction is the one of the samples on the cone", Path.m_AbsorptionProbabilityToFirstIAMinimum[0], Minimum, GetFloatTolerance(Minimum) + 1e-9) && Passed;

  // The cone axis is (-3, -4, -11) with length sqrt(146)
  double Zenith = acos(-11.0/sqrt(146.0)) - Phi;
  double Nadir = acos(11.0/sqrt(146.0)) - Phi;
  // Defect REV-D-12: The zenith and nadir angles are calculated with the direction "zenith minus position of the first hit", which mixes a direction and a position
  Passed = ExpectedToFail(EvaluateNear("Fill()", "two hits, path", "The zenith angle is the angle of the cone axis to the zenith minus the Compton scatter angle", Path.m_ZenithAngle[0], Zenith, GetFloatTolerance(Zenith))) && Passed;
  Passed = ExpectedToFail(EvaluateNear("Fill()", "two hits, path", "The nadir angle is the angle of the cone axis to the nadir minus the Compton scatter angle", Path.m_NadirAngle[0], Nadir, GetFloatTolerance(Nadir))) && Passed;

  DeleteSequence(Sequence);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling a sequence beyond the limits of the Compton kinematics
bool UTERCSRDataSet::TestFillKinematicLimits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;
  Passed = EvaluateTrue("LoadTestCrossSections()", "telescope", "The cross sections can be loaded", LoadTestCrossSections(Geometry)) && Passed;

  vector<MVector> Positions = { MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0) };
  MERCSRDataSet DataSet(2, true);

  // A possible sequence first
  vector<MRESE*> Possible = CreateSequence(Positions, { 200.0, 461.657 });
  DataSet.Fill(1, Possible, &Geometry);
  DeleteSequence(Possible);
  Passed = EvaluateTrue("Fill()", "possible", "The average absorption probability to the first interaction is calculated", DataSet.m_AbsorptionProbabilityToFirstIAAverage[0] > 0.0) && Passed;

  // The electron cannot get 600 keV of 661.657 keV: the cosine is 1 - 511/61.657 + 511/661.657 = -6.5. The path values get their markers
  vector<MRESE*> Impossible = CreateSequence(Positions, { 600.0, 61.657 });
  DataSet.Fill(2, Impossible, &Geometry);
  DeleteSequence(Impossible);
  double CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(600.0, 61.657);
  Passed = EvaluateNear("Fill()", "kinematically impossible", "The cosine of the Compton equation is stored as it is", DataSet.m_CosComptonScatterAngles[0][0], CosPhi, GetFloatTolerance(CosPhi)) && Passed;
  Passed = EvaluateNear("Fill()", "kinematically impossible", "The average absorption probability to the first interaction is -0.1", DataSet.m_AbsorptionProbabilityToFirstIAAverage[0], -0.1, GetFloatTolerance(0.1)) && Passed;
  Passed = EvaluateNear("Fill()", "kinematically impossible", "The maximum absorption probability to the first interaction is -0.1", DataSet.m_AbsorptionProbabilityToFirstIAMaximum[0], -0.1, GetFloatTolerance(0.1)) && Passed;
  Passed = EvaluateNear("Fill()", "kinematically impossible", "The minimum absorption probability to the first interaction is -0.1", DataSet.m_AbsorptionProbabilityToFirstIAMinimum[0], -0.1, GetFloatTolerance(0.1)) && Passed;
  Passed = EvaluateNear("Fill()", "kinematically impossible", "The zenith angle is -4", DataSet.m_ZenithAngle[0], -4.0, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "kinematically impossible", "The nadir angle is -4", DataSet.m_NadirAngle[0], -4.0, 0.0) && Passed;

  // No energy in the first hit: the cosine is exactly 1, which is at the limit and gets the markers too
  vector<MRESE*> NoEnergy = CreateSequence(Positions, { 0.0, 300.0 });
  DataSet.Fill(3, NoEnergy, &Geometry);
  DeleteSequence(NoEnergy);
  Passed = EvaluateNear("Fill()", "no energy in the first hit", "The cosine is exactly 1", DataSet.m_CosComptonScatterAngles[0][0], 1.0, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "no energy in the first hit", "The average absorption probability to the first interaction is -0.1", DataSet.m_AbsorptionProbabilityToFirstIAAverage[0], -0.1, GetFloatTolerance(0.1)) && Passed;
  Passed = EvaluateNear("Fill()", "no energy in the first hit", "The zenith angle is -4", DataSet.m_ZenithAngle[0], -4.0, 0.0) && Passed;

  // Just inside: 1 keV of 301 keV has the cosine 0.99434, and sin(Phi) = 0.106, thus one sample
  vector<MRESE*> Small = CreateSequence(Positions, { 1.0, 300.0 });
  DataSet.Fill(4, Small, &Geometry);
  DeleteSequence(Small);
  double Average = 0.0;
  double Maximum = 0.0;
  double Minimum = 0.0;
  unsigned int Steps = 0;
  GetExpectedPath(Geometry, Positions[0], Positions[1], acos(MComptonEvent::ComputeCosPhiViaEeEg(1.0, 300.0)), 301.0, Average, Maximum, Minimum, Steps);
  Passed = Evaluate("GetExpectedPath()", "1 keV in the first hit", "There is one sample on the cone", Steps, 1U) && Passed;
  Passed = EvaluateNear("Fill()", "1 keV in the first hit", "The average absorption probability to the first interaction is the one sample", DataSet.m_AbsorptionProbabilityToFirstIAAverage[0], Average, GetFloatTolerance(Average) + 1e-9) && Passed;
  Passed = EvaluateNear("Fill()", "1 keV in the first hit", "The maximum is the one sample", DataSet.m_AbsorptionProbabilityToFirstIAMaximum[0], Average, GetFloatTolerance(Average) + 1e-9) && Passed;
  Passed = EvaluateNear("Fill()", "1 keV in the first hit", "The minimum is the one sample", DataSet.m_AbsorptionProbabilityToFirstIAMinimum[0], Average, GetFloatTolerance(Average) + 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the store of the path values of sequences of four hits
bool UTERCSRDataSet::TestFillStore()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;
  Passed = EvaluateTrue("LoadTestCrossSections()", "telescope", "The cross sections can be loaded", LoadTestCrossSections(Geometry)) && Passed;

  // 661.657 keV in four hits
  vector<MVector> Positions = { MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0), MVector(3.0, -3.0, -12.0), MVector(0.0, 0.0, -7.0) };
  vector<double> Energies = { 200.0, 150.0, 100.0, 211.657 };
  vector<MRESE*> Sequence = CreateSequence(Positions, Energies);

  double Phi0 = acos(MComptonEvent::ComputeCosPhiViaEeEg(200.0, 461.657));
  double Average = 0.0;
  double Maximum = 0.0;
  double Minimum = 0.0;
  unsigned int Steps = 0;
  GetExpectedPath(Geometry, Positions[0], Positions[1], Phi0, 661.657, Average, Maximum, Minimum, Steps);

  MERCSRDataSet DataSet(4, true);
  DataSet.Fill(5, Sequence, &Geometry);
  Passed = Evaluate("Fill()", "four hits, ID 5", "The simulation ID is stored for the sequence length 4", DataSet.m_SimulationIDs[2], static_cast<Long64_t>(5)) && Passed;
  Passed = EvaluateNear("Fill()", "four hits", "The last energy is stored", DataSet.m_Energies[2][3], 211.657, GetFloatTolerance(211.657)) && Passed;
  // The third interaction: the electron has 100 keV, the gamma ray 211.657 keV afterwards. Its geometric angle is between (5, -3.5, -4) and (-3, 3, 5): -15 - 10.5 - 20 = -45.5
  double CosPhi2 = MComptonEvent::ComputeCosPhiViaEeEg(100.0, 211.657);
  double CosPhiGeometry2 = -45.5/(sqrt(53.25)*sqrt(43.0));
  Passed = EvaluateNear("Fill()", "four hits", "The second Compton scatter angle difference is the one at the third hit", DataSet.m_CosComptonScatterAngleDifference[1][1], CosPhiGeometry2 - CosPhi2, 2*GetFloatTolerance(1.0)) && Passed;
  // The gamma ray has 211.657 keV on its last path
  double Absorption2 = Geometry.GetAbsorptionProbability(Positions[2], Positions[3], 211.657);
  Passed = EvaluateNear("Fill()", "four hits", "The last absorption probability is the one for 211.657 keV", DataSet.m_AbsorptionProbabilities[2][2], Absorption2, GetFloatTolerance(Absorption2)) && Passed;
  Passed = EvaluateNear("Fill()", "four hits", "The average absorption probability to the first interaction is the one of the samples on the cone", DataSet.m_AbsorptionProbabilityToFirstIAAverage[2], Average, Steps*GetFloatTolerance(1.0)) && Passed;

  // From four hits on the path values are stored for the pair of the first two hits, until a sequence with another event ID is filled
  Float_t StoredAverage = DataSet.m_AbsorptionProbabilityToFirstIAAverage[2];
  Float_t StoredZenith = DataSet.m_ZenithAngle[2];

  // Another order of the last two hits has the same first two hits: the stored values are used
  vector<MRESE*> Swapped = { Sequence[0], Sequence[1], Sequence[3], Sequence[2] };
  DataSet.Fill(5, Swapped, &Geometry);
  Passed = EvaluateNear("Fill()", "same first two hits", "The average absorption probability to the first interaction is the same", DataSet.m_AbsorptionProbabilityToFirstIAAverage[2], StoredAverage, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "same first two hits", "The zenith angle is the same", DataSet.m_ZenithAngle[2], StoredZenith, 0.0) && Passed;
  Passed = EvaluateNear("Fill()", "same first two hits", "The third energy is the one of the new order", DataSet.m_Energies[2][2], 211.657, GetFloatTolerance(211.657)) && Passed;

  // Another pair of first hits of the same event is calculated and stored in addition
  vector<MRESE*> OtherStart = { Sequence[1], Sequence[0], Sequence[2], Sequence[3] };
  DataSet.Fill(5, OtherStart, &Geometry);
  double OtherAverage = 0.0;
  GetExpectedPath(Geometry, Positions[1], Positions[0], acos(MComptonEvent::ComputeCosPhiViaEeEg(150.0, 511.657)), 661.657, OtherAverage, Maximum, Minimum, Steps);
  Passed = EvaluateNear("Fill()", "other first two hits", "The average absorption probability to the first interaction is the one for the new start", DataSet.m_AbsorptionProbabilityToFirstIAAverage[2], OtherAverage, Steps*GetFloatTolerance(1.0)) && Passed;

  // A new event empties the store: the first hit is moved, and the values are calculated for the new position
  MVector Moved(4.0, -4.0, 2.0);
  Sequence[0]->SetPosition(Moved);
  DataSet.Fill(6, Sequence, &Geometry);
  double MovedAverage = 0.0;
  GetExpectedPath(Geometry, Moved, Positions[1], Phi0, 661.657, MovedAverage, Maximum, Minimum, Steps);
  Passed = EvaluateTrue("GetExpectedPath()", "fixture", "The moved hit has another average absorption probability", fabs(MovedAverage - Average) > 0.001) && Passed;
  Passed = EvaluateNear("Fill()", "new event", "The average absorption probability to the first interaction is the one for the new event", DataSet.m_AbsorptionProbabilityToFirstIAAverage[2], MovedAverage, Steps*GetFloatTolerance(1.0)) && Passed;

  DeleteSequence(Sequence);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test sequences which do not fit to the data set
bool UTERCSRDataSet::TestFillWrongLength()
{
  bool Passed = true;

  // Defect REV-D-13: Fill does not check the length of the sequence: a sequence which is longer than the maximum of the data set, a single hit,
  // or any sequence for a default constructed data set is written outside the arrays
  Passed = ExpectedToCrash(EvaluateTrue("Fill()", "three hits, sequence length up to two", "A sequence which is too long is not stored", FillTooLongSequence())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("Fill()", "one hit", "A single hit is not stored", FillSingleHit())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("Fill()", "default constructed, two hits", "A data set without arrays stores nothing", FillDefaultConstructed())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the trees
bool UTERCSRDataSet::TestCreateTrees()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;
  Passed = EvaluateTrue("LoadTestCrossSections()", "telescope", "The cross sections can be loaded", LoadTestCrossSections(Geometry)) && Passed;

  MERCSRDataSet DataSet(3, true);
  // The lists are emptied first: these entries are no trees
  vector<TTree*> Good = { nullptr };
  vector<TTree*> Bad = { nullptr, nullptr };
  DataSet.CreateTrees(Good, Bad);
  Passed = EvaluateSize("CreateTrees()", "sequence length up to 3, path", "There is a good tree for each sequence length", Good.size(), 2) && Passed;
  Passed = EvaluateSize("CreateTrees()", "sequence length up to 3, path", "There is a bad tree for each sequence length", Bad.size(), 2) && Passed;
  if (Good.size() != 2 || Bad.size() != 2) return Passed;

  Passed = EvaluateTrue("CreateTrees()", "sequence length up to 3, path", "All trees are created", Good[0] != nullptr && Good[1] != nullptr && Bad[0] != nullptr && Bad[1] != nullptr) && Passed;
  if (Good[0] == nullptr || Good[1] == nullptr || Bad[0] == nullptr || Bad[1] == nullptr) return Passed;

  Passed = Evaluate("CreateTrees()", "good tree", "The name is Good", MString(Good[0]->GetName()), MString("Good")) && Passed;
  Passed = Evaluate("CreateTrees()", "bad tree", "The name is Bad", MString(Bad[1]->GetName()), MString("Bad")) && Passed;

  // Sequence length 2: ID + 2 energies + 2 x 3 positions + distance + angle + Klein-Nishina value + absorption probability + 5 path values = 18
  // Sequence length 3: ID + 3 energies + 3 x 3 positions + 2 distances + 2 angles + 2 Klein-Nishina values + 1 angle difference + 2 absorption probabilities + 5 path values = 27
  Passed = Evaluate("CreateTrees()", "good tree, sequence length 2", "The tree has 18 branches", Good[0]->GetNbranches(), 18) && Passed;
  Passed = Evaluate("CreateTrees()", "bad tree, sequence length 2", "The tree has 18 branches", Bad[0]->GetNbranches(), 18) && Passed;
  Passed = Evaluate("CreateTrees()", "good tree, sequence length 3", "The tree has 27 branches", Good[1]->GetNbranches(), 27) && Passed;
  Passed = Evaluate("CreateTrees()", "bad tree, sequence length 3", "The tree has 27 branches", Bad[1]->GetNbranches(), 27) && Passed;

  // The names count from one
  vector<MString> Names = { "SimulationIDs", "Energy1", "Energy2", "Energy3", "X1", "X2", "X3", "Y1", "Y2", "Y3", "Z1", "Z2", "Z3",
                            "InteractionDistances1", "InteractionDistances2", "CosComptonScatterAngle1", "CosComptonScatterAngle2",
                            "NormalizedKleinNishinaValue1", "NormalizedKleinNishinaValue2", "CosComptonScatterAngleDifference1",
                            "AbsorptionProbabilities1", "AbsorptionProbabilities2", "AbsorptionProbabilityToFirstIAAverage",
                            "AbsorptionProbabilityToFirstIAMaximum", "AbsorptionProbabilityToFirstIAMinimum", "ZenithAngle", "NadirAngle" };
  for (unsigned int n = 0; n < Names.size(); ++n) {
    Passed = EvaluateTrue("CreateTrees()", MString("good tree, sequence length 3, ") + Names[n], "The branch exists", Good[1]->GetBranch(Names[n]) != nullptr) && Passed;
    Passed = EvaluateTrue("CreateTrees()", MString("bad tree, sequence length 3, ") + Names[n], "The branch exists", Bad[1]->GetBranch(Names[n]) != nullptr) && Passed;
  }
  Passed = EvaluateTrue("CreateTrees()", "good tree, sequence length 2", "There is no branch for a third hit", Good[0]->GetBranch("Energy3") == nullptr) && Passed;
  Passed = EvaluateTrue("CreateTrees()", "good tree, sequence length 2", "There is no branch for a Compton scatter angle difference", Good[0]->GetBranch("CosComptonScatterAngleDifference1") == nullptr) && Passed;

  // A sequence of three hits goes into a tree of the sequence length 3 and is read back
  vector<MRESE*> Sequence = CreateSequence({ MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0), MVector(3.0, -3.0, -12.0) }, { 200.0, 150.0, 311.657 });
  DataSet.Fill(11, Sequence, &Geometry);
  Float_t Difference = DataSet.m_CosComptonScatterAngleDifference[0][0];
  Float_t Average = DataSet.m_AbsorptionProbabilityToFirstIAAverage[1];
  Float_t Nadir = DataSet.m_NadirAngle[1];
  Good[1]->Fill();

  vector<MRESE*> Reversed = { Sequence[2], Sequence[1], Sequence[0] };
  DataSet.Fill(11, Reversed, &Geometry);
  Bad[1]->Fill();
  DeleteSequence(Sequence);

  Passed = Evaluate("CreateTrees()", "good tree, sequence length 3", "The tree has one entry", Good[1]->GetEntries(), static_cast<Long64_t>(1)) && Passed;
  Passed = Evaluate("CreateTrees()", "bad tree, sequence length 3", "The tree has one entry", Bad[1]->GetEntries(), static_cast<Long64_t>(1)) && Passed;
  Passed = Evaluate("CreateTrees()", "good tree, sequence length 2", "The tree is empty", Good[0]->GetEntries(), static_cast<Long64_t>(0)) && Passed;

  Good[1]->GetEntry(0);
  Passed = Evaluate("CreateTrees()", "good tree, sequence length 3", "The simulation ID is read back", DataSet.m_SimulationIDs[1], static_cast<Long64_t>(11)) && Passed;
  Passed = EvaluateNear("CreateTrees()", "good tree, sequence length 3", "The first energy is read back", DataSet.m_Energies[1][0], 200.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTrees()", "good tree, sequence length 3", "The second x position is read back", DataSet.m_PositionsX[1][1], -2.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTrees()", "good tree, sequence length 3", "The third z position is read back", DataSet.m_PositionsZ[1][2], -12.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTrees()", "good tree, sequence length 3", "The first interaction distance is read back", DataSet.m_InteractionDistances[1][0], 11.5, 0.0) && Passed;
  Passed = EvaluateNear("CreateTrees()", "good tree, sequence length 3", "The Compton scatter angle difference is read back", DataSet.m_CosComptonScatterAngleDifference[0][0], Difference, 0.0) && Passed;
  Passed = EvaluateNear("CreateTrees()", "good tree, sequence length 3", "The average absorption probability to the first interaction is read back", DataSet.m_AbsorptionProbabilityToFirstIAAverage[1], Average, 0.0) && Passed;
  Passed = EvaluateNear("CreateTrees()", "good tree, sequence length 3", "The nadir angle is read back", DataSet.m_NadirAngle[1], Nadir, 0.0) && Passed;

  Bad[1]->GetEntry(0);
  Passed = EvaluateNear("CreateTrees()", "bad tree, sequence length 3", "The first energy is read back", DataSet.m_Energies[1][0], 311.657, GetFloatTolerance(311.657)) && Passed;
  Passed = EvaluateNear("CreateTrees()", "bad tree, sequence length 3", "The third z position is read back", DataSet.m_PositionsZ[1][2], 3.0, 0.0) && Passed;

  for (unsigned int t = 0; t < 2; ++t) {
    delete Good[t];
    delete Bad[t];
  }

  // Without the path: 5 branches less
  MERCSRDataSet NoPath(3, false);
  vector<TTree*> NoPathGood;
  vector<TTree*> NoPathBad;
  NoPath.CreateTrees(NoPathGood, NoPathBad);
  Passed = EvaluateSize("CreateTrees()", "sequence length up to 3, no path", "There is a good tree for each sequence length", NoPathGood.size(), 2) && Passed;
  Passed = EvaluateSize("CreateTrees()", "sequence length up to 3, no path", "There is a bad tree for each sequence length", NoPathBad.size(), 2) && Passed;
  if (NoPathGood.size() == 2 && NoPathBad.size() == 2) {
    Passed = Evaluate("CreateTrees()", "good tree, sequence length 2, no path", "The tree has 13 branches", NoPathGood[0]->GetNbranches(), 13) && Passed;
    Passed = Evaluate("CreateTrees()", "bad tree, sequence length 3, no path", "The tree has 22 branches", NoPathBad[1]->GetNbranches(), 22) && Passed;
    Passed = EvaluateTrue("CreateTrees()", "good tree, sequence length 3, no path", "There is no branch for the zenith angle", NoPathGood[1]->GetBranch("ZenithAngle") == nullptr) && Passed;
    Passed = EvaluateTrue("CreateTrees()", "good tree, sequence length 3, no path", "The absorption probabilities between the hits are in the tree", NoPathGood[1]->GetBranch("AbsorptionProbabilities2") != nullptr) && Passed;
  }
  for (unsigned int t = 0; t < NoPathGood.size(); ++t) {
    delete NoPathGood[t];
  }
  for (unsigned int t = 0; t < NoPathBad.size(); ++t) {
    delete NoPathBad[t];
  }

  // A default constructed data set has no trees
  MERCSRDataSet Default;
  vector<TTree*> DefaultGood;
  vector<TTree*> DefaultBad;
  Default.CreateTrees(DefaultGood, DefaultBad);
  Passed = EvaluateSize("CreateTrees()", "default constructed", "There are no good trees", DefaultGood.size(), 0) && Passed;
  Passed = EvaluateSize("CreateTrees()", "default constructed", "There are no bad trees", DefaultBad.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the readers
bool UTERCSRDataSet::TestCreateReaders()
{
  bool Passed = true;

  MERCSRDataSet DataSet(3, true);
  vector<TMVA::Reader*> Readers;
  BeginCaptureOutput();
  DataSet.CreateReaders(Readers);
  EndCaptureOutput();
  Passed = EvaluateSize("CreateReaders()", "sequence length up to 3, path", "There is a reader for each sequence length", Readers.size(), 2) && Passed;
  if (Readers.size() != 2) return Passed;
  Passed = EvaluateTrue("CreateReaders()", "sequence length up to 3, path", "Both readers are created", Readers[0] != nullptr && Readers[1] != nullptr) && Passed;
  if (Readers[0] == nullptr || Readers[1] == nullptr) return Passed;

  // The variables are the branches of the trees without the simulation ID: 17 and 26
  Passed = Evaluate("CreateReaders()", "sequence length 2, path", "The reader has 17 variables", Readers[0]->DataInfo().GetNVariables(), 17U) && Passed;
  Passed = Evaluate("CreateReaders()", "sequence length 3, path", "The reader has 26 variables", Readers[1]->DataInfo().GetNVariables(), 26U) && Passed;
  vector<MString> Names = { "Energy1", "Energy2", "Energy3", "X1", "X2", "X3", "Y1", "Y2", "Y3", "Z1", "Z2", "Z3",
                            "InteractionDistances1", "InteractionDistances2", "CosComptonScatterAngle1", "CosComptonScatterAngle2",
                            "NormalizedKleinNishinaValue1", "NormalizedKleinNishinaValue2", "CosComptonScatterAngleDifference1",
                            "AbsorptionProbabilities1", "AbsorptionProbabilities2", "AbsorptionProbabilityToFirstIAAverage",
                            "AbsorptionProbabilityToFirstIAMaximum", "AbsorptionProbabilityToFirstIAMinimum", "ZenithAngle", "NadirAngle" };
  if (Readers[1]->DataInfo().GetNVariables() == 26U) {
    for (unsigned int n = 0; n < Names.size(); ++n) {
      Passed = Evaluate("CreateReaders()", MString("sequence length 3, variable ") + n, "The variables have the order and the names of the branches of the tree", MString(Readers[1]->DataInfo().GetVariableInfo(n).GetExpression()), Names[n]) && Passed;
    }
  }

  // A second call adds its readers to the list
  BeginCaptureOutput();
  DataSet.CreateReaders(Readers);
  EndCaptureOutput();
  Passed = EvaluateSize("CreateReaders()", "second call", "The readers are added to the list", Readers.size(), 4) && Passed;
  for (unsigned int r = 0; r < Readers.size(); ++r) {
    delete Readers[r];
  }
  Readers.clear();

  // Without the path the absorption probabilities between the hits are no variables either: 4 l + 3 (l - 1) + (l - 2) = 11 and 19
  MERCSRDataSet NoPath(3, false);
  BeginCaptureOutput();
  NoPath.CreateReaders(Readers);
  EndCaptureOutput();
  Passed = EvaluateSize("CreateReaders()", "sequence length up to 3, no path", "There is a reader for each sequence length", Readers.size(), 2) && Passed;
  if (Readers.size() == 2 && Readers[0] != nullptr && Readers[1] != nullptr) {
    Passed = Evaluate("CreateReaders()", "sequence length 2, no path", "The reader has 11 variables", Readers[0]->DataInfo().GetNVariables(), 11U) && Passed;
    Passed = Evaluate("CreateReaders()", "sequence length 3, no path", "The reader has 19 variables", Readers[1]->DataInfo().GetNVariables(), 19U) && Passed;
  }
  for (unsigned int r = 0; r < Readers.size(); ++r) {
    delete Readers[r];
  }
  Readers.clear();

  // A default constructed data set has no readers
  MERCSRDataSet Default;
  Default.CreateReaders(Readers);
  Passed = EvaluateSize("CreateReaders()", "default constructed", "There are no readers", Readers.size(), 0) && Passed;

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

  UTERCSRDataSet Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSRDataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
