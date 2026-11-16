/*
 * UTERQualityDataSet.cxx
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
#include "MERQualityDataSet.h"
#include "MGeometryRevan.h"
#include "MGlobal.h"
#include "MRESE.h"
#include "MString.h"
#include "MVector.h"
#include "UTERDataSetShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the data set of the TMVA event quality MERQualityDataSet
class UTERQualityDataSet : public UTERDataSetBase
{
public:
  //! Default constructor
  UTERQualityDataSet() : UTERDataSetBase("UTERQualityDataSet") {}
  //! Default destructor
  virtual ~UTERQualityDataSet() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructors on memory with a pattern
  bool TestConstruction();
  //! Test the initialization
  bool TestInitialize();
  //! Test filling a sequence of three hits
  bool TestFillEventData();
  //! Test filling a sequence of two hits without path samples
  bool TestFillEventDataTwoHits();
  //! Test filling sequences at and beyond the limits of the Compton kinematics
  bool TestFillEventDataKinematicLimits();
  //! Test filling several events into one data set
  bool TestFillEventDataReuse();
  //! Test filling the evaluation section
  bool TestFillEvaluation();
  //! Test the tree
  bool TestCreateTree();
  //! Test the reader
  bool TestCreateReader();

  //! Return the names of the values of a data set with a sequence length of three and four path samples, in the order of the tree
  vector<MString> GetNames() const;
  //! Fill a sequence of two hits into a default constructed data set, which has no arrays
  bool FillDefaultConstructed();
  //! Fill a sequence of three hits into a data set for sequences of two hits
  bool FillTooLongSequence();
  //! Return the number of variables of a reader for the given sequence length and number of path samples
  unsigned int GetNReaderVariables(unsigned int MaximumSequenceLength, unsigned int NumberOfPathSamples);
  //! Return true if the variables of a reader for a sequence length of three and four path samples have the names and the order of the branches of the tree
  bool HasReaderVariablesOfTree();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERQualityDataSet::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestFillEventData() && Passed;
  Passed = TestFillEventDataTwoHits() && Passed;
  Passed = TestFillEventDataKinematicLimits() && Passed;
  Passed = TestFillEventDataReuse() && Passed;
  Passed = TestFillEvaluation() && Passed;
  Passed = TestCreateTree() && Passed;
  Passed = TestCreateReader() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the names of the values of a data set with a sequence length of three and four path samples, in the order of the tree
vector<MString> UTERQualityDataSet::GetNames() const
{
  // The names count from one
  vector<MString> Names = { "SimulationID", "SequenceLength", "Energy1", "Energy2", "Energy3", "X1", "X2", "X3", "Y1", "Y2", "Y3", "Z1", "Z2", "Z3",
                            "InteractionDistances1", "InteractionDistances2", "CosComptonScatterAngle1", "CosComptonScatterAngle2",
                            "NormalizedKleinNishinaValue1", "NormalizedKleinNishinaValue2", "CosComptonScatterAngleDifference1",
                            "AbsorptionProbabilities1", "AbsorptionProbabilities2", "MinimumNadirAngle",
                            "PathSample1", "PathSample2", "PathSample3", "PathSample4",
                            "EvaluationIsCompletelyAbsorbed", "EvaluationIsReconstructable", "EvaluationIsDecay", "EvaluationZenithAngle" };

  return Names;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill a sequence of two hits into a default constructed data set, which has no arrays
bool UTERQualityDataSet::FillDefaultConstructed()
{
  MGeometryRevan Geometry;
  ScanTelescopeGeometry(Geometry);
  LoadTestCrossSections(Geometry);

  vector<MRESE*> Sequence = CreateSequence({ MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0) }, { 200.0, 461.657 });
  MERQualityDataSet DataSet;
  DataSet.FillEventData(1, Sequence, &Geometry);
  DeleteSequence(Sequence);

  if (DataSet.m_Energies.size() != 0) return false;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill a sequence of three hits into a data set for sequences of two hits
bool UTERQualityDataSet::FillTooLongSequence()
{
  MGeometryRevan Geometry;
  ScanTelescopeGeometry(Geometry);
  LoadTestCrossSections(Geometry);

  vector<MRESE*> Sequence = CreateSequence({ MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0), MVector(3.0, -3.0, -12.0) }, { 200.0, 150.0, 311.657 });
  MERQualityDataSet DataSet(2, 0);
  DataSet.FillEventData(1, Sequence, &Geometry);
  DeleteSequence(Sequence);

  if (DataSet.m_Energies.size() != 2) return false;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructors on memory with a pattern
bool UTERQualityDataSet::TestConstruction()
{
  bool Passed = true;

  // The standard constructor: 0xA5A5A5A5 is no default of any member, neither as integer nor as float
  {
    alignas(MERQualityDataSet) unsigned char Block[sizeof(MERQualityDataSet)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MERQualityDataSet* DataSet = new (Block) MERQualityDataSet(3, 4);
    Passed = Evaluate("MERQualityDataSet(3, 4)", "memory with 0xA5", "The simulation ID is -1", DataSet->m_SimulationID, -1) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are three energies", DataSet->m_Energies.size(), 3) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are three x positions", DataSet->m_PositionsX.size(), 3) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are three y positions", DataSet->m_PositionsY.size(), 3) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are three z positions", DataSet->m_PositionsZ.size(), 3) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are two interaction distances", DataSet->m_InteractionDistances.size(), 2) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are two Compton scatter angles", DataSet->m_CosComptonScatterAngles.size(), 2) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are two Klein-Nishina values", DataSet->m_KleinNishinaProbability.size(), 2) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There is one Compton scatter angle difference", DataSet->m_CosComptonScatterAngleDifference.size(), 1) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are two absorption probabilities", DataSet->m_AbsorptionProbabilities.size(), 2) && Passed;
    Passed = EvaluateSize("MERQualityDataSet(3, 4)", "memory with 0xA5", "There are four path samples", DataSet->m_PathSamples.size(), 4) && Passed;
    Passed = EvaluateNear("MERQualityDataSet(3, 4)", "memory with 0xA5", "The minimum nadir angle is -1", DataSet->m_MinimumNadirAngle, -1.0, 0.0) && Passed;
    Passed = Evaluate("MERQualityDataSet(3, 4)", "memory with 0xA5", "The completely absorbed flag is -1", DataSet->m_EvaluationIsCompletelyAbsorbed, -1) && Passed;
    Passed = Evaluate("MERQualityDataSet(3, 4)", "memory with 0xA5", "The reconstructable flag is -1", DataSet->m_EvaluationIsReconstructable, -1) && Passed;
    Passed = Evaluate("MERQualityDataSet(3, 4)", "memory with 0xA5", "The decay flag is -1", DataSet->m_EvaluationIsDecay, -1) && Passed;
    Passed = EvaluateNear("MERQualityDataSet(3, 4)", "memory with 0xA5", "The zenith angle is -1", DataSet->m_EvaluationZenithAngle, -1.0, 0.0) && Passed;
    // Defect REV-D-8: Initialize does not set the sequence length, which is a branch of the tree and a variable of the reader
    Passed = ExpectedToFail(EvaluateFalse("MERQualityDataSet(3, 4)", "memory with 0xA5", "The sequence length is initialized", IsFillPattern(&DataSet->m_SequenceLength, sizeof(Int_t), 0xA5))) && Passed;
    DataSet->~MERQualityDataSet();
  }

  // The default constructor
  {
    alignas(MERQualityDataSet) unsigned char Block[sizeof(MERQualityDataSet)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MERQualityDataSet* DataSet = new (Block) MERQualityDataSet();
    Passed = EvaluateSize("MERQualityDataSet()", "memory with 0xA5", "There are no energies", DataSet->m_Energies.size(), 0) && Passed;
    Passed = EvaluateSize("MERQualityDataSet()", "memory with 0xA5", "There are no interaction distances", DataSet->m_InteractionDistances.size(), 0) && Passed;
    Passed = EvaluateSize("MERQualityDataSet()", "memory with 0xA5", "There are no path samples", DataSet->m_PathSamples.size(), 0) && Passed;
    // Defect REV-D-8: The default constructor does not initialize the simulation ID, the sequence length, the minimum nadir angle, and the evaluation values
    Passed = ExpectedToFail(EvaluateFalse("MERQualityDataSet()", "memory with 0xA5", "The simulation ID is initialized", IsFillPattern(&DataSet->m_SimulationID, sizeof(Int_t), 0xA5))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("MERQualityDataSet()", "memory with 0xA5", "The sequence length is initialized", IsFillPattern(&DataSet->m_SequenceLength, sizeof(Int_t), 0xA5))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("MERQualityDataSet()", "memory with 0xA5", "The minimum nadir angle is initialized", IsFillPattern(&DataSet->m_MinimumNadirAngle, sizeof(Float_t), 0xA5))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("MERQualityDataSet()", "memory with 0xA5", "The completely absorbed flag is initialized", IsFillPattern(&DataSet->m_EvaluationIsCompletelyAbsorbed, sizeof(Int_t), 0xA5))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("MERQualityDataSet()", "memory with 0xA5", "The reconstructable flag is initialized", IsFillPattern(&DataSet->m_EvaluationIsReconstructable, sizeof(Int_t), 0xA5))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("MERQualityDataSet()", "memory with 0xA5", "The decay flag is initialized", IsFillPattern(&DataSet->m_EvaluationIsDecay, sizeof(Int_t), 0xA5))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("MERQualityDataSet()", "memory with 0xA5", "The zenith angle is initialized", IsFillPattern(&DataSet->m_EvaluationZenithAngle, sizeof(Float_t), 0xA5))) && Passed;
    DataSet->~MERQualityDataSet();
  }

  // Defect REV-D-9: FillEventData does not check the length of the sequence against its arrays: a default constructed data set has
  // a maximum sequence length of 2, but no arrays, and a sequence which is longer than the maximum is written behind the arrays
  Passed = ExpectedToCrash(EvaluateTrue("FillEventData()", "default constructed, two hits", "A data set without arrays stores nothing", FillDefaultConstructed())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("FillEventData()", "three hits, maximum sequence length two", "A sequence which is too long is not stored", FillTooLongSequence())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the initialization
bool UTERQualityDataSet::TestInitialize()
{
  bool Passed = true;

  // The smallest data set: two hits, no path samples
  MERQualityDataSet DataSet;
  DataSet.Initialize(2);
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There are two energies", DataSet.m_Energies.size(), 2) && Passed;
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There is one interaction distance", DataSet.m_InteractionDistances.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There is one Compton scatter angle", DataSet.m_CosComptonScatterAngles.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There is one Klein-Nishina value", DataSet.m_KleinNishinaProbability.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There is no Compton scatter angle difference", DataSet.m_CosComptonScatterAngleDifference.size(), 0) && Passed;
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There is one absorption probability", DataSet.m_AbsorptionProbabilities.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(2)", "smallest data set", "There are no path samples by default", DataSet.m_PathSamples.size(), 0) && Passed;
  Passed = Evaluate("Initialize(2)", "smallest data set", "The simulation ID is -1", DataSet.m_SimulationID, -1) && Passed;

  // Larger, with path samples, after the evaluation was filled
  DataSet.FillEvaluationIsCompletelyAbsorbed(true);
  DataSet.FillEvaluationIsReconstructable(true);
  DataSet.FillEvaluationIsDecay(true);
  DataSet.FillEvaluationZenithAngle(45.0);
  DataSet.Initialize(5, 18);
  Passed = EvaluateSize("Initialize(5, 18)", "after (2)", "There are five energies", DataSet.m_Energies.size(), 5) && Passed;
  Passed = EvaluateSize("Initialize(5, 18)", "after (2)", "There are five z positions", DataSet.m_PositionsZ.size(), 5) && Passed;
  Passed = EvaluateSize("Initialize(5, 18)", "after (2)", "There are four interaction distances", DataSet.m_InteractionDistances.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(5, 18)", "after (2)", "There are four Compton scatter angles", DataSet.m_CosComptonScatterAngles.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(5, 18)", "after (2)", "There are four Klein-Nishina values", DataSet.m_KleinNishinaProbability.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(5, 18)", "after (2)", "There are three Compton scatter angle differences", DataSet.m_CosComptonScatterAngleDifference.size(), 3) && Passed;
  Passed = EvaluateSize("Initialize(5, 18)", "after (2)", "There are four absorption probabilities", DataSet.m_AbsorptionProbabilities.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(5, 18)", "after (2)", "There are 18 path samples", DataSet.m_PathSamples.size(), 18) && Passed;
  Passed = Evaluate("Initialize(5, 18)", "after (2)", "The completely absorbed flag is -1 again", DataSet.m_EvaluationIsCompletelyAbsorbed, -1) && Passed;
  Passed = Evaluate("Initialize(5, 18)", "after (2)", "The reconstructable flag is -1 again", DataSet.m_EvaluationIsReconstructable, -1) && Passed;
  Passed = Evaluate("Initialize(5, 18)", "after (2)", "The decay flag is -1 again", DataSet.m_EvaluationIsDecay, -1) && Passed;
  Passed = EvaluateNear("Initialize(5, 18)", "after (2)", "The zenith angle is -1 again", DataSet.m_EvaluationZenithAngle, -1.0, 0.0) && Passed;

  // Smaller again, without path samples
  DataSet.Initialize(3, 0);
  Passed = EvaluateSize("Initialize(3, 0)", "after (5, 18)", "There are three energies", DataSet.m_Energies.size(), 3) && Passed;
  Passed = EvaluateSize("Initialize(3, 0)", "after (5, 18)", "There is one Compton scatter angle difference", DataSet.m_CosComptonScatterAngleDifference.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(3, 0)", "after (5, 18)", "There are no path samples", DataSet.m_PathSamples.size(), 0) && Passed;

  // A sequence needs two hits: shorter ones are refused with an error message
  DisableDefaultStreams();
  DataSet.Initialize(1, 0);
  EnableDefaultStreams();
  // Defect REV-D-10: A refused Initialize stores the new maximum sequence length, but keeps the arrays of the initialization before
  Passed = ExpectedToFail(EvaluateSize("Initialize(1, 0)", "after (3, 0)", "A data set whose initialization was refused has no arrays", DataSet.m_Energies.size(), 0)) && Passed;

  DisableDefaultStreams();
  MERQualityDataSet Zero(0, 0);
  EnableDefaultStreams();
  Passed = EvaluateSize("MERQualityDataSet(0, 0)", "no hits", "A refused data set has no arrays", Zero.m_Energies.size(), 0) && Passed;
  Passed = EvaluateSize("MERQualityDataSet(0, 0)", "no hits", "A refused data set has no interaction distances", Zero.m_InteractionDistances.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling a sequence of three hits
bool UTERQualityDataSet::TestFillEventData()
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

  MERQualityDataSet DataSet(3, 4);
  DataSet.FillEventData(77, Sequence, &Geometry);

  // (0) ID and length
  Passed = Evaluate("FillEventData()", "three hits, ID 77", "The simulation ID is stored", DataSet.m_SimulationID, 77) && Passed;
  Passed = Evaluate("FillEventData()", "three hits", "The sequence length is three", DataSet.m_SequenceLength, 3) && Passed;

  // (a) The hits
  for (unsigned int h = 0; h < 3; ++h) {
    Passed = EvaluateNear("FillEventData()", MString("hit ") + h, "The energy is stored", DataSet.m_Energies[h], Energies[h], GetFloatTolerance(Energies[h])) && Passed;
    Passed = EvaluateNear("FillEventData()", MString("hit ") + h, "The x position is stored", DataSet.m_PositionsX[h], Positions[h].X(), 0.0) && Passed;
    Passed = EvaluateNear("FillEventData()", MString("hit ") + h, "The y position is stored", DataSet.m_PositionsY[h], Positions[h].Y(), 0.0) && Passed;
    Passed = EvaluateNear("FillEventData()", MString("hit ") + h, "The z position is stored", DataSet.m_PositionsZ[h], Positions[h].Z(), 0.0) && Passed;
  }

  // (b) The distances: |(-3, -1.5, -11)| = sqrt(132.25) = 11.5 and |(5, -3.5, -4)| = sqrt(53.25)
  Passed = EvaluateNear("FillEventData()", "hit 0 to 1", "The interaction distance is 11.5 cm", DataSet.m_InteractionDistances[0], 11.5, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "hit 1 to 2", "The interaction distance is sqrt(53.25) cm", DataSet.m_InteractionDistances[1], sqrt(53.25), GetFloatTolerance(sqrt(53.25))) && Passed;

  // (c) The Compton scatter angles: the first electron has 200 keV and the gamma ray 461.657 keV afterwards, the second electron 150 keV and the gamma ray 311.657 keV
  double CosPhi0 = MComptonEvent::ComputeCosPhiViaEeEg(200.0, 461.657);
  double CosPhi1 = MComptonEvent::ComputeCosPhiViaEeEg(150.0, 311.657);
  Passed = EvaluateNear("FillEventData()", "first interaction", "The cosine of the Compton scatter angle is the one of the Compton equation", DataSet.m_CosComptonScatterAngles[0], CosPhi0, GetFloatTolerance(CosPhi0)) && Passed;
  Passed = EvaluateNear("FillEventData()", "second interaction", "The cosine of the Compton scatter angle is the one of the Compton equation", DataSet.m_CosComptonScatterAngles[1], CosPhi1, GetFloatTolerance(CosPhi1)) && Passed;

  // The Klein-Nishina value for the energy of the gamma ray before the interaction: 661.657 keV and 461.657 keV
  double KleinNishina0 = MComptonEvent::GetKleinNishinaNormalizedByArea(661.657, acos(CosPhi0));
  double KleinNishina1 = MComptonEvent::GetKleinNishinaNormalizedByArea(461.657, acos(CosPhi1));
  Passed = EvaluateNear("FillEventData()", "first interaction", "The Klein-Nishina value is the one for 661.657 keV", DataSet.m_KleinNishinaProbability[0], KleinNishina0, GetFloatTolerance(KleinNishina0)) && Passed;
  Passed = EvaluateNear("FillEventData()", "second interaction", "The Klein-Nishina value is the one for 461.657 keV", DataSet.m_KleinNishinaProbability[1], KleinNishina1, GetFloatTolerance(KleinNishina1)) && Passed;

  // (d) The geometric scatter angle at the second hit minus the kinematic one: (-3, -1.5, -11) * (5, -3.5, -4) = 34.25
  double CosPhiGeometry = 34.25/(11.5*sqrt(53.25));
  Passed = EvaluateNear("FillEventData()", "second interaction", "The difference of the geometric and the kinematic cosine of the Compton scatter angle is stored", DataSet.m_CosComptonScatterAngleDifference[0], CosPhiGeometry - CosPhi1, 2*GetFloatTolerance(1.0)) && Passed;

  // (e) The absorption probabilities between the hits for the energy of the gamma ray on this path
  double Absorption0 = Geometry.GetAbsorptionProbability(Positions[0], Positions[1], 461.657);
  double Absorption1 = Geometry.GetAbsorptionProbability(Positions[1], Positions[2], 311.657);
  Passed = EvaluateTrue("GetAbsorptionProbability()", "fixture", "The absorption probability on the first path depends on the energy", Absorption0 > 0.1 && fabs(Absorption0 - Geometry.GetAbsorptionProbability(Positions[0], Positions[1], 661.657)) > 0.01) && Passed;
  Passed = EvaluateNear("FillEventData()", "hit 0 to 1", "The absorption probability is the one for 461.657 keV", DataSet.m_AbsorptionProbabilities[0], Absorption0, GetFloatTolerance(Absorption0)) && Passed;
  Passed = EvaluateNear("FillEventData()", "hit 1 to 2", "The absorption probability is the one for 311.657 keV", DataSet.m_AbsorptionProbabilities[1], Absorption1, GetFloatTolerance(Absorption1)) && Passed;

  // (f) The closest approach of the Compton cone to the nadir in degrees: the cone axis is (3, 1.5, 11), its angle to (0, 0, -1) is acos(-11/11.5)
  double Nadir = fabs(acos(-11.0/11.5) - acos(CosPhi0))*c_Deg;
  Passed = EvaluateNear("FillEventData()", "three hits", "The minimum nadir angle is the angle of the cone axis to the nadir minus the Compton scatter angle", DataSet.m_MinimumNadirAngle, Nadir, GetFloatTolerance(Nadir)) && Passed;

  // (g) The Compton absorption probabilities on four paths along the Compton cone to the first hit, for the full energy
  vector<double> Cone = GetConeProbabilities(Geometry, Positions[0], Positions[1], acos(CosPhi0), 661.657, 4);
  bool ConeDiffers = false;
  for (unsigned int s = 0; s < 4; ++s) {
    Passed = EvaluateNear("FillEventData()", MString("path sample ") + s, "The sample is the Compton absorption probability from far outside to the first hit along the Compton cone", DataSet.m_PathSamples[s], Cone[s], GetFloatTolerance(Cone[s]) + 1e-9) && Passed;
    if (fabs(Cone[s] - Cone[0]) > 0.001) ConeDiffers = true;
  }
  Passed = EvaluateTrue("GetConeProbabilities()", "fixture", "The paths along the cone cross different amounts of material", ConeDiffers) && Passed;

  // The evaluation section is not touched
  Passed = Evaluate("FillEventData()", "three hits", "The completely absorbed flag is still -1", DataSet.m_EvaluationIsCompletelyAbsorbed, -1) && Passed;
  Passed = EvaluateNear("FillEventData()", "three hits", "The zenith angle is still -1", DataSet.m_EvaluationZenithAngle, -1.0, 0.0) && Passed;

  DeleteSequence(Sequence);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling a sequence of two hits without path samples
bool UTERQualityDataSet::TestFillEventDataTwoHits()
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

  MERQualityDataSet DataSet(2, 0);
  DataSet.FillEventData(-5, Sequence, &Geometry);

  Passed = Evaluate("FillEventData()", "two hits, ID -5", "The simulation ID is stored", DataSet.m_SimulationID, -5) && Passed;
  Passed = Evaluate("FillEventData()", "two hits", "The sequence length is two", DataSet.m_SequenceLength, 2) && Passed;
  Passed = EvaluateNear("FillEventData()", "two hits", "The first energy is stored", DataSet.m_Energies[0], 400.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "two hits", "The second energy is stored", DataSet.m_Energies[1], 773.25, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "two hits", "The second z position is stored", DataSet.m_PositionsZ[1], 2.0, 0.0) && Passed;

  // |(3, 4, 11)| = sqrt(146)
  Passed = EvaluateNear("FillEventData()", "two hits", "The interaction distance is sqrt(146) cm", DataSet.m_InteractionDistances[0], sqrt(146.0), GetFloatTolerance(sqrt(146.0))) && Passed;

  double CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(400.0, 773.25);
  Passed = EvaluateNear("FillEventData()", "two hits", "The cosine of the Compton scatter angle is the one of the Compton equation", DataSet.m_CosComptonScatterAngles[0], CosPhi, GetFloatTolerance(CosPhi)) && Passed;
  double KleinNishina = MComptonEvent::GetKleinNishinaNormalizedByArea(1173.25, acos(CosPhi));
  Passed = EvaluateNear("FillEventData()", "two hits", "The Klein-Nishina value is the one for 1173.25 keV", DataSet.m_KleinNishinaProbability[0], KleinNishina, GetFloatTolerance(KleinNishina)) && Passed;

  // The gamma ray has 773.25 keV between the hits
  double Absorption = Geometry.GetAbsorptionProbability(Positions[0], Positions[1], 773.25);
  Passed = EvaluateNear("FillEventData()", "two hits", "The absorption probability is the one for 773.25 keV", DataSet.m_AbsorptionProbabilities[0], Absorption, GetFloatTolerance(Absorption)) && Passed;

  // The cone axis is (-3, -4, -11): its angle to (0, 0, -1) is acos(11/sqrt(146))
  double Nadir = fabs(acos(11.0/sqrt(146.0)) - acos(CosPhi))*c_Deg;
  Passed = EvaluateNear("FillEventData()", "two hits", "The minimum nadir angle is the angle of the cone axis to the nadir minus the Compton scatter angle", DataSet.m_MinimumNadirAngle, Nadir, GetFloatTolerance(Nadir)) && Passed;

  Passed = EvaluateSize("FillEventData()", "two hits, no path samples", "There are still no path samples", DataSet.m_PathSamples.size(), 0) && Passed;
  Passed = EvaluateSize("FillEventData()", "two hits", "There is still no Compton scatter angle difference", DataSet.m_CosComptonScatterAngleDifference.size(), 0) && Passed;

  DeleteSequence(Sequence);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling sequences at and beyond the limits of the Compton kinematics
bool UTERQualityDataSet::TestFillEventDataKinematicLimits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;
  Passed = EvaluateTrue("LoadTestCrossSections()", "telescope", "The cross sections can be loaded", LoadTestCrossSections(Geometry)) && Passed;

  vector<MVector> Positions = { MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0) };

  // The electron cannot get 600 keV of 661.657 keV: the Compton edge is at 477.3 keV, the cosine is 1 - 511/61.657 + 511/661.657 = -6.5
  {
    vector<MRESE*> Sequence = CreateSequence(Positions, { 600.0, 61.657 });
    MERQualityDataSet DataSet(2, 4);
    DataSet.FillEventData(1, Sequence, &Geometry);
    double CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(600.0, 61.657);
    Passed = EvaluateTrue("MComptonEvent::ComputeCosPhiViaEeEg()", "600 keV of 661.657 keV", "The cosine is below -1", CosPhi < -1.0) && Passed;
    Passed = EvaluateNear("FillEventData()", "kinematically impossible", "The cosine of the Compton equation is stored as it is", DataSet.m_CosComptonScatterAngles[0], CosPhi, GetFloatTolerance(CosPhi)) && Passed;
    Passed = EvaluateNear("FillEventData()", "kinematically impossible", "The minimum nadir angle is -1", DataSet.m_MinimumNadirAngle, -1.0, 0.0) && Passed;
    for (unsigned int s = 0; s < 4; ++s) {
      Passed = EvaluateNear("FillEventData()", MString("kinematically impossible, path sample ") + s, "The path sample is zero", DataSet.m_PathSamples[s], 0.0, 0.0) && Passed;
    }
    DeleteSequence(Sequence);
  }

  // No energy in the first hit: the cosine is exactly 1, which still gives a nadir angle, but no path samples
  {
    vector<MRESE*> Sequence = CreateSequence(Positions, { 0.0, 300.0 });
    MERQualityDataSet DataSet(2, 4);
    DataSet.FillEventData(1, Sequence, &Geometry);
    Passed = EvaluateNear("FillEventData()", "no energy in the first hit", "The cosine is exactly 1", DataSet.m_CosComptonScatterAngles[0], 1.0, 0.0) && Passed;
    // The cone axis is (3, 1.5, 11) with |axis| = 11.5, and the Compton scatter angle is zero
    double Nadir = acos(-11.0/11.5)*c_Deg;
    Passed = EvaluateNear("FillEventData()", "no energy in the first hit", "The minimum nadir angle is the angle of the cone axis to the nadir", DataSet.m_MinimumNadirAngle, Nadir, GetFloatTolerance(Nadir)) && Passed;
    for (unsigned int s = 0; s < 4; ++s) {
      Passed = EvaluateNear("FillEventData()", MString("no energy in the first hit, path sample ") + s, "The path sample is zero", DataSet.m_PathSamples[s], 0.0, 0.0) && Passed;
    }
    DeleteSequence(Sequence);
  }

  // Just inside: a first hit of 1 keV of 301 keV has the cosine 1 - 511 (1/300 - 1/301) = 0.99434
  {
    vector<MRESE*> Sequence = CreateSequence(Positions, { 1.0, 300.0 });
    MERQualityDataSet DataSet(2, 4);
    DataSet.FillEventData(1, Sequence, &Geometry);
    double CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(1.0, 300.0);
    Passed = EvaluateTrue("MComptonEvent::ComputeCosPhiViaEeEg()", "1 keV of 301 keV", "The cosine is just below 1", CosPhi < 1.0 && CosPhi > 0.99) && Passed;
    vector<double> Cone = GetConeProbabilities(Geometry, Positions[0], Positions[1], acos(CosPhi), 301.0, 4);
    for (unsigned int s = 0; s < 4; ++s) {
      Passed = EvaluateNear("FillEventData()", MString("1 keV in the first hit, path sample ") + s, "The path sample is calculated", DataSet.m_PathSamples[s], Cone[s], GetFloatTolerance(Cone[s]) + 1e-9) && Passed;
    }
    DeleteSequence(Sequence);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling several events into one data set
bool UTERQualityDataSet::TestFillEventDataReuse()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;
  Passed = EvaluateTrue("LoadTestCrossSections()", "telescope", "The cross sections can be loaded", LoadTestCrossSections(Geometry)) && Passed;

  MERQualityDataSet DataSet(2, 4);

  vector<MVector> FirstPositions = { MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0) };
  vector<MRESE*> First = CreateSequence(FirstPositions, { 200.0, 461.657 });
  DataSet.FillEventData(1, First, &Geometry);
  DeleteSequence(First);

  // The second event replaces everything, also with impossible kinematics after possible ones
  vector<MVector> SecondPositions = { MVector(-1.0, -2.0, -9.0), MVector(2.0, 2.0, 2.0) };
  vector<MRESE*> Second = CreateSequence(SecondPositions, { 600.0, 61.657 });
  DataSet.FillEventData(2, Second, &Geometry);
  DeleteSequence(Second);

  Passed = Evaluate("FillEventData()", "second event", "The simulation ID is replaced", DataSet.m_SimulationID, 2) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event", "The first energy is replaced", DataSet.m_Energies[0], 600.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event", "The first z position is replaced", DataSet.m_PositionsZ[0], -9.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event", "The interaction distance is replaced", DataSet.m_InteractionDistances[0], sqrt(146.0), GetFloatTolerance(sqrt(146.0))) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event", "The minimum nadir angle of the event before does not stay", DataSet.m_MinimumNadirAngle, -1.0, 0.0) && Passed;
  for (unsigned int s = 0; s < 4; ++s) {
    Passed = EvaluateNear("FillEventData()", MString("second event, path sample ") + s, "The path sample of the event before does not stay", DataSet.m_PathSamples[s], 0.0, 0.0) && Passed;
  }

  // And back to the first event: the same values as in a new data set
  vector<MRESE*> Third = CreateSequence(FirstPositions, { 200.0, 461.657 });
  DataSet.FillEventData(3, Third, &Geometry);
  MERQualityDataSet Fresh(2, 4);
  Fresh.FillEventData(3, Third, &Geometry);
  DeleteSequence(Third);
  Passed = EvaluateNear("FillEventData()", "third event", "The Compton scatter angle is the one of a new data set", DataSet.m_CosComptonScatterAngles[0], Fresh.m_CosComptonScatterAngles[0], 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "third event", "The minimum nadir angle is the one of a new data set", DataSet.m_MinimumNadirAngle, Fresh.m_MinimumNadirAngle, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "third event", "The absorption probability is the one of a new data set", DataSet.m_AbsorptionProbabilities[0], Fresh.m_AbsorptionProbabilities[0], 0.0) && Passed;
  for (unsigned int s = 0; s < 4; ++s) {
    Passed = EvaluateNear("FillEventData()", MString("third event, path sample ") + s, "The path sample is the one of a new data set", DataSet.m_PathSamples[s], Fresh.m_PathSamples[s], 0.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling the evaluation section
bool UTERQualityDataSet::TestFillEvaluation()
{
  bool Passed = true;

  MERQualityDataSet DataSet(2, 0);

  // The flags are stored as 1 and 0, and they are independent
  DataSet.FillEvaluationIsCompletelyAbsorbed(true);
  Passed = Evaluate("FillEvaluationIsCompletelyAbsorbed()", "true", "The flag is 1", DataSet.m_EvaluationIsCompletelyAbsorbed, 1) && Passed;
  Passed = Evaluate("FillEvaluationIsCompletelyAbsorbed()", "true", "The reconstructable flag is not changed", DataSet.m_EvaluationIsReconstructable, -1) && Passed;
  Passed = Evaluate("FillEvaluationIsCompletelyAbsorbed()", "true", "The decay flag is not changed", DataSet.m_EvaluationIsDecay, -1) && Passed;
  DataSet.FillEvaluationIsCompletelyAbsorbed(false);
  Passed = Evaluate("FillEvaluationIsCompletelyAbsorbed()", "false", "The flag is 0", DataSet.m_EvaluationIsCompletelyAbsorbed, 0) && Passed;

  DataSet.FillEvaluationIsReconstructable(true);
  Passed = Evaluate("FillEvaluationIsReconstructable()", "true", "The flag is 1", DataSet.m_EvaluationIsReconstructable, 1) && Passed;
  Passed = Evaluate("FillEvaluationIsReconstructable()", "true", "The completely absorbed flag is not changed", DataSet.m_EvaluationIsCompletelyAbsorbed, 0) && Passed;
  DataSet.FillEvaluationIsReconstructable(false);
  Passed = Evaluate("FillEvaluationIsReconstructable()", "false", "The flag is 0", DataSet.m_EvaluationIsReconstructable, 0) && Passed;

  DataSet.FillEvaluationIsDecay(true);
  Passed = Evaluate("FillEvaluationIsDecay()", "true", "The flag is 1", DataSet.m_EvaluationIsDecay, 1) && Passed;
  Passed = Evaluate("FillEvaluationIsDecay()", "true", "The reconstructable flag is not changed", DataSet.m_EvaluationIsReconstructable, 0) && Passed;
  DataSet.FillEvaluationIsDecay(false);
  Passed = Evaluate("FillEvaluationIsDecay()", "false", "The flag is 0", DataSet.m_EvaluationIsDecay, 0) && Passed;

  // The values are exact in single precision
  for (Float_t Angle : { 0.0f, 45.5f, 180.0f, -12.25f }) {
    DataSet.FillEvaluationZenithAngle(Angle);
    Passed = EvaluateNear("FillEvaluationZenithAngle()", Angle, "The zenith angle is stored", DataSet.m_EvaluationZenithAngle, Angle, 0.0) && Passed;
  }
  Passed = Evaluate("FillEvaluationZenithAngle()", "-12.25", "The flags are not changed", DataSet.m_EvaluationIsDecay, 0) && Passed;
  Passed = Evaluate("FillEvaluationZenithAngle()", "-12.25", "The event data is not changed", DataSet.m_SimulationID, -1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the tree
bool UTERQualityDataSet::TestCreateTree()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;
  Passed = EvaluateTrue("LoadTestCrossSections()", "telescope", "The cross sections can be loaded", LoadTestCrossSections(Geometry)) && Passed;

  MERQualityDataSet DataSet(3, 4);
  TTree* Tree = DataSet.CreateTree("Quality");
  Passed = EvaluateTrue("CreateTree()", "sequence length 3, 4 path samples", "A tree is created", Tree != nullptr) && Passed;
  if (Tree == nullptr) return Passed;

  Passed = Evaluate("CreateTree()", "sequence length 3, 4 path samples", "The tree has the given name", MString(Tree->GetName()), MString("Quality")) && Passed;
  // ID, length + 3 energies + 3 x 3 positions + 2 distances + 2 angles + 2 Klein-Nishina values + 1 angle difference + 2 absorption probabilities
  // + nadir angle + 4 path samples + 4 evaluation values = 32
  vector<MString> Names = GetNames();
  Passed = Evaluate("CreateTree()", "sequence length 3, 4 path samples", "The tree has 32 branches", Tree->GetNbranches(), 32) && Passed;
  for (unsigned int n = 0; n < Names.size(); ++n) {
    Passed = EvaluateTrue("CreateTree()", Names[n], "The branch exists", Tree->GetBranch(Names[n]) != nullptr) && Passed;
  }
  Passed = EvaluateTrue("CreateTree()", "Energy4", "There is no branch for a fourth hit", Tree->GetBranch("Energy4") == nullptr) && Passed;
  Passed = EvaluateTrue("CreateTree()", "PathSample5", "There is no branch for a fifth path sample", Tree->GetBranch("PathSample5") == nullptr) && Passed;

  // Two events: the tree reads back into the data set
  vector<MRESE*> First = CreateSequence({ MVector(1.0, 2.0, 3.0), MVector(-2.0, 0.5, -8.0), MVector(3.0, -3.0, -12.0) }, { 200.0, 150.0, 311.657 });
  DataSet.FillEventData(11, First, &Geometry);
  DataSet.FillEvaluationIsCompletelyAbsorbed(true);
  DataSet.FillEvaluationIsReconstructable(false);
  DataSet.FillEvaluationIsDecay(true);
  DataSet.FillEvaluationZenithAngle(12.5);
  Float_t FirstNadir = DataSet.m_MinimumNadirAngle;
  Float_t FirstSample = DataSet.m_PathSamples[3];
  Float_t FirstDifference = DataSet.m_CosComptonScatterAngleDifference[0];
  Tree->Fill();
  DeleteSequence(First);

  vector<MRESE*> Second = CreateSequence({ MVector(-1.0, -2.0, -9.0), MVector(2.0, 2.0, 2.0), MVector(4.0, 4.0, 0.0) }, { 400.0, 300.0, 473.25 });
  DataSet.FillEventData(12, Second, &Geometry);
  DataSet.FillEvaluationIsCompletelyAbsorbed(false);
  DataSet.FillEvaluationIsReconstructable(true);
  DataSet.FillEvaluationIsDecay(false);
  DataSet.FillEvaluationZenithAngle(170.25);
  Float_t SecondNadir = DataSet.m_MinimumNadirAngle;
  Tree->Fill();
  DeleteSequence(Second);

  Passed = Evaluate("CreateTree()", "two events", "The tree has two entries", Tree->GetEntries(), static_cast<Long64_t>(2)) && Passed;

  Tree->GetEntry(0);
  Passed = Evaluate("CreateTree()", "first entry", "The simulation ID is read back", DataSet.m_SimulationID, 11) && Passed;
  Passed = Evaluate("CreateTree()", "first entry", "The sequence length is read back", DataSet.m_SequenceLength, 3) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The first energy is read back", DataSet.m_Energies[0], 200.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second x position is read back", DataSet.m_PositionsX[1], -2.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second y position is read back", DataSet.m_PositionsY[1], 0.5, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The third z position is read back", DataSet.m_PositionsZ[2], -12.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The first interaction distance is read back", DataSet.m_InteractionDistances[0], 11.5, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The Compton scatter angle difference is read back", DataSet.m_CosComptonScatterAngleDifference[0], FirstDifference, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The minimum nadir angle is read back", DataSet.m_MinimumNadirAngle, FirstNadir, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The last path sample is read back", DataSet.m_PathSamples[3], FirstSample, 0.0) && Passed;
  Passed = Evaluate("CreateTree()", "first entry", "The completely absorbed flag is read back", DataSet.m_EvaluationIsCompletelyAbsorbed, 1) && Passed;
  Passed = Evaluate("CreateTree()", "first entry", "The reconstructable flag is read back", DataSet.m_EvaluationIsReconstructable, 0) && Passed;
  Passed = Evaluate("CreateTree()", "first entry", "The decay flag is read back", DataSet.m_EvaluationIsDecay, 1) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The zenith angle is read back", DataSet.m_EvaluationZenithAngle, 12.5, 0.0) && Passed;

  Tree->GetEntry(1);
  Passed = Evaluate("CreateTree()", "second entry", "The simulation ID is read back", DataSet.m_SimulationID, 12) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The first energy is read back", DataSet.m_Energies[0], 400.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The third x position is read back", DataSet.m_PositionsX[2], 4.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The minimum nadir angle is read back", DataSet.m_MinimumNadirAngle, SecondNadir, 0.0) && Passed;
  Passed = Evaluate("CreateTree()", "second entry", "The reconstructable flag is read back", DataSet.m_EvaluationIsReconstructable, 1) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The zenith angle is read back", DataSet.m_EvaluationZenithAngle, 170.25, 0.0) && Passed;

  delete Tree;

  // Without path samples: ID, length + 2 energies + 2 x 3 positions + distance + angle + Klein-Nishina value + absorption probability + nadir angle + 4 evaluation values = 19
  MERQualityDataSet Smallest(2, 0);
  TTree* SmallestTree = Smallest.CreateTree("Smallest");
  Passed = EvaluateTrue("CreateTree()", "sequence length 2, no path samples", "A tree is created", SmallestTree != nullptr) && Passed;
  if (SmallestTree != nullptr) {
    Passed = Evaluate("CreateTree()", "sequence length 2, no path samples", "The tree has 19 branches", SmallestTree->GetNbranches(), 19) && Passed;
    Passed = EvaluateTrue("CreateTree()", "sequence length 2, no path samples", "There is no branch for a path sample", SmallestTree->GetBranch("PathSample1") == nullptr) && Passed;
    Passed = EvaluateTrue("CreateTree()", "sequence length 2, no path samples", "There is no branch for a Compton scatter angle difference", SmallestTree->GetBranch("CosComptonScatterAngleDifference1") == nullptr) && Passed;
  }
  delete SmallestTree;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of variables of a reader for the given sequence length and number of path samples
unsigned int UTERQualityDataSet::GetNReaderVariables(unsigned int MaximumSequenceLength, unsigned int NumberOfPathSamples)
{
  MERQualityDataSet DataSet(MaximumSequenceLength, NumberOfPathSamples);
  BeginCaptureOutput();
  TMVA::Reader* Reader = DataSet.CreateReader();
  EndCaptureOutput();
  if (Reader == nullptr) return 0;

  unsigned int NVariables = Reader->DataInfo().GetNVariables();
  delete Reader;

  return NVariables;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the variables of a reader for a sequence length of three and four path samples have the names and the order of the branches of the tree
bool UTERQualityDataSet::HasReaderVariablesOfTree()
{
  MERQualityDataSet DataSet(3, 4);
  BeginCaptureOutput();
  TMVA::Reader* Reader = DataSet.CreateReader();
  EndCaptureOutput();
  if (Reader == nullptr) return false;

  bool Identical = true;
  vector<MString> Names = GetNames();
  if (Reader->DataInfo().GetNVariables() != Names.size()) {
    Identical = false;
  } else {
    for (unsigned int n = 0; n < Names.size(); ++n) {
      if (MString(Reader->DataInfo().GetVariableInfo(n).GetExpression()) != Names[n]) Identical = false;
    }
  }
  delete Reader;

  return Identical;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the reader
bool UTERQualityDataSet::TestCreateReader()
{
  bool Passed = true;

  // Defect REV-D-11: CreateReader adds the simulation ID and the sequence length as integer variables, which the TMVA reader refuses with
  // a fatal error: the function throws a std::runtime_error out of TMVA, which nobody catches, for every data set
  // The reader has the same 32 values as the tree
  Passed = ExpectedToCrash(Evaluate("CreateReader()", "sequence length 3, 4 path samples", "The reader has 32 variables", GetNReaderVariables(3, 4), 32U)) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("CreateReader()", "sequence length 3, 4 path samples", "The variables have the order and the names of the branches of the tree", HasReaderVariablesOfTree())) && Passed;
  // Without path samples
  Passed = ExpectedToCrash(Evaluate("CreateReader()", "sequence length 2, no path samples", "The reader has 19 variables", GetNReaderVariables(2, 0), 19U)) && Passed;

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

  UTERQualityDataSet Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERQualityDataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
