/*
 * UTEREventClusterizerDataSet.cxx
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
#include <cstring>
#include <new>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TMVA/Reader.h"
#include "TROOT.h"
#include "TRandom.h"
#include "TTree.h"

// MEGAlib libs:
#include "MEREventClusterizerDataSet.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the data set of the TMVA event clusterizer MEREventClusterizerDataSet
class UTEREventClusterizerDataSet : public MUnitTest
{
public:
  //! Default constructor
  UTEREventClusterizerDataSet() : MUnitTest("UTEREventClusterizerDataSet") {}
  //! Default destructor
  virtual ~UTEREventClusterizerDataSet() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructors on memory with a pattern
  bool TestConstruction();
  //! Test the initialization
  bool TestInitialize();
  //! Test filling the event data
  bool TestFillEventData();
  //! Test filling the result data
  bool TestFillResultData();
  //! Test the string
  bool TestToString();
  //! Test the tree
  bool TestCreateTree();
  //! Test the reader
  bool TestCreateReader();

  //! Return the three hits of the tests: (1, 2, 3) cm with 100.5 keV, (-4, 5.5, -6.25) cm with 20.25 keV, (0, 0, 0) cm with 661 keV; the caller deletes them
  vector<MRESE*> CreateHits() const;
  //! Delete the hits
  void DeleteHits(vector<MRESE*>& Hits) const;
  //! Return true if the string of a default constructed data set on memory with a pattern only has the line of the ID
  bool IsDefaultStringOnlyID();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTEREventClusterizerDataSet::Run()
{
  bool Passed = true;

  // The result data has a random jitter
  gRandom->SetSeed(1);

  Passed = TestConstruction() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestFillEventData() && Passed;
  Passed = TestFillResultData() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestCreateTree() && Passed;
  Passed = TestCreateReader() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the three hits of the tests: (1, 2, 3) cm with 100.5 keV, (-4, 5.5, -6.25) cm with 20.25 keV, (0, 0, 0) cm with 661 keV; the caller deletes them
vector<MRESE*> UTEREventClusterizerDataSet::CreateHits() const
{
  // All values are exact in single precision
  vector<MRESE*> Hits;

  MREHit* First = new MREHit();
  First->SetPosition(MVector(1.0, 2.0, 3.0));
  First->SetEnergy(100.5);
  Hits.push_back(First);

  MREHit* Second = new MREHit();
  Second->SetPosition(MVector(-4.0, 5.5, -6.25));
  Second->SetEnergy(20.25);
  Hits.push_back(Second);

  MREHit* Third = new MREHit();
  Third->SetPosition(MVector(0.0, 0.0, 0.0));
  Third->SetEnergy(661.0);
  Hits.push_back(Third);

  return Hits;
}


////////////////////////////////////////////////////////////////////////////////


//! Delete the hits
void UTEREventClusterizerDataSet::DeleteHits(vector<MRESE*>& Hits) const
{
  for (unsigned int h = 0; h < Hits.size(); ++h) {
    delete Hits[h];
  }
  Hits.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the string of a default constructed data set on memory with a pattern only has the line of the ID
bool UTEREventClusterizerDataSet::IsDefaultStringOnlyID()
{
  alignas(MEREventClusterizerDataSet) unsigned char Block[sizeof(MEREventClusterizerDataSet)];
  FillMemory(Block, sizeof(Block), 0xA5);
  MEREventClusterizerDataSet* DataSet = new (Block) MEREventClusterizerDataSet();
  MString Text = DataSet->ToString();
  DataSet->~MEREventClusterizerDataSet();

  // One line: "ID <number>"
  if (Text.BeginsWith("ID ") == false) return false;
  if (Text.Tokenize("\n").size() != 1) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructors on memory with a pattern
bool UTEREventClusterizerDataSet::TestConstruction()
{
  bool Passed = true;

  // The standard constructor: 0xA5A5A5A5 is the float -2.87e-16, which is no default of any member
  {
    alignas(MEREventClusterizerDataSet) unsigned char Block[sizeof(MEREventClusterizerDataSet)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MEREventClusterizerDataSet* DataSet = new (Block) MEREventClusterizerDataSet(3, 2);
    Passed = EvaluateNear("MEREventClusterizerDataSet(3, 2)", "memory with 0xA5", "The simulation ID is -1", DataSet->m_SimulationID, -1.0, 0.0) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet(3, 2)", "memory with 0xA5", "There are three energies", DataSet->m_Energies.size(), 3) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet(3, 2)", "memory with 0xA5", "There are three x positions", DataSet->m_PositionsX.size(), 3) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet(3, 2)", "memory with 0xA5", "There are three y positions", DataSet->m_PositionsY.size(), 3) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet(3, 2)", "memory with 0xA5", "There are three z positions", DataSet->m_PositionsZ.size(), 3) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet(3, 2)", "memory with 0xA5", "There are groups for three hits", DataSet->m_ResultHitGroups.size(), 3) && Passed;
    bool AllZero = true;
    bool AllThreeGroups = true;
    for (unsigned int h = 0; h < DataSet->m_ResultHitGroups.size(); ++h) {
      if (DataSet->m_Energies[h] != 0 || DataSet->m_PositionsX[h] != 0 || DataSet->m_PositionsY[h] != 0 || DataSet->m_PositionsZ[h] != 0) AllZero = false;
      if (DataSet->m_ResultHitGroups[h].size() != 3) AllThreeGroups = false;
      for (unsigned int g = 0; g < DataSet->m_ResultHitGroups[h].size(); ++g) {
        if (DataSet->m_ResultHitGroups[h][g] != 0) AllZero = false;
      }
    }
    // Group 0 is the first group, and the last one is for all groups beyond the maximum
    Passed = EvaluateTrue("MEREventClusterizerDataSet(3, 2)", "memory with 0xA5", "Each hit has the two groups and one more for the overflow", AllThreeGroups) && Passed;
    Passed = EvaluateTrue("MEREventClusterizerDataSet(3, 2)", "memory with 0xA5", "All energies, positions, and groups are zero", AllZero) && Passed;
    DataSet->~MEREventClusterizerDataSet();
  }

  // The default constructor
  {
    alignas(MEREventClusterizerDataSet) unsigned char Block[sizeof(MEREventClusterizerDataSet)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MEREventClusterizerDataSet* DataSet = new (Block) MEREventClusterizerDataSet();
    Passed = EvaluateSize("MEREventClusterizerDataSet()", "memory with 0xA5", "There are no energies", DataSet->m_Energies.size(), 0) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet()", "memory with 0xA5", "There are no x positions", DataSet->m_PositionsX.size(), 0) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet()", "memory with 0xA5", "There are no y positions", DataSet->m_PositionsY.size(), 0) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet()", "memory with 0xA5", "There are no z positions", DataSet->m_PositionsZ.size(), 0) && Passed;
    Passed = EvaluateSize("MEREventClusterizerDataSet()", "memory with 0xA5", "There are no groups", DataSet->m_ResultHitGroups.size(), 0) && Passed;
    // Defect REV-D-6: The default constructor does not initialize the simulation ID, the number of hits, and the maximum number of groups
    Passed = ExpectedToFail(EvaluateFalse("MEREventClusterizerDataSet()", "memory with 0xA5", "The simulation ID is initialized", IsFillPattern(&DataSet->m_SimulationID, sizeof(Float_t), 0xA5))) && Passed;

    // Without hits no data is accepted: the number of hits is 0xA5A5A5A5 here, so this passes by chance
    vector<MRESE*> Hits = CreateHits();
    DisableDefaultStreams();
    bool Filled = DataSet->FillEventData(1, Hits);
    EnableDefaultStreams();
    DeleteHits(Hits);
    Passed = EvaluateFalse("FillEventData()", "default constructed, three hits", "A data set without hits accepts no hits", Filled) && Passed;
    DataSet->~MEREventClusterizerDataSet();
  }

  // Defect REV-D-6: ToString loops over the not initialized number of hits and reads the empty arrays
  Passed = ExpectedToCrash(EvaluateTrue("ToString()", "default constructed, memory with 0xA5", "The string of a data set without hits only has the ID", IsDefaultStringOnlyID())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the initialization
bool UTEREventClusterizerDataSet::TestInitialize()
{
  bool Passed = true;

  MEREventClusterizerDataSet DataSet;
  DataSet.Initialize(1, 1);
  Passed = EvaluateSize("Initialize(1, 1)", "smallest data set", "There is one energy", DataSet.m_Energies.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(1, 1)", "smallest data set", "There are groups for one hit", DataSet.m_ResultHitGroups.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(1, 1)", "smallest data set", "The hit has one group and the overflow", DataSet.m_ResultHitGroups[0].size(), 2) && Passed;
  Passed = EvaluateNear("Initialize(1, 1)", "smallest data set", "The simulation ID is -1", DataSet.m_SimulationID, -1.0, 0.0) && Passed;

  // Fill it, and initialize it again with another size: the sizes change, and the values are reset
  vector<MRESE*> Hits = CreateHits();
  vector<MRESE*> OneHit = { Hits[0] };
  vector<vector<int>> OneGroup = { { 0 } };
  Passed = EvaluateTrue("FillEventData()", "one hit", "The data is accepted", DataSet.FillEventData(42, OneHit)) && Passed;
  Passed = EvaluateTrue("FillResultData()", "one hit", "The result is accepted", DataSet.FillResultData(OneGroup)) && Passed;

  DataSet.Initialize(4, 5);
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are four energies", DataSet.m_Energies.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are four x positions", DataSet.m_PositionsX.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are four y positions", DataSet.m_PositionsY.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are four z positions", DataSet.m_PositionsZ.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are groups for four hits", DataSet.m_ResultHitGroups.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "The first hit has five groups and the overflow", DataSet.m_ResultHitGroups[0].size(), 6) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "The last hit has five groups and the overflow", DataSet.m_ResultHitGroups[3].size(), 6) && Passed;
  Passed = EvaluateNear("Initialize(4, 5)", "after (1, 1)", "The simulation ID is -1 again", DataSet.m_SimulationID, -1.0, 0.0) && Passed;
  Passed = EvaluateNear("Initialize(4, 5)", "after (1, 1)", "The group of the first hit is zero again", DataSet.m_ResultHitGroups[0][0], 0.0, 0.0) && Passed;

  // Smaller again
  DataSet.Initialize(2, 1);
  Passed = EvaluateSize("Initialize(2, 1)", "after (4, 5)", "There are two energies", DataSet.m_Energies.size(), 2) && Passed;
  Passed = EvaluateSize("Initialize(2, 1)", "after (4, 5)", "There are groups for two hits", DataSet.m_ResultHitGroups.size(), 2) && Passed;
  Passed = EvaluateSize("Initialize(2, 1)", "after (4, 5)", "A hit has one group and the overflow", DataSet.m_ResultHitGroups[1].size(), 2) && Passed;

  // Without hits or without groups the initialization is refused with an error message
  vector<MRESE*> TwoHits = { Hits[0], Hits[1] };
  vector<vector<int>> TwoGroups = { { 0 }, { 0 } };
  DisableDefaultStreams();
  DataSet.Initialize(0, 1);
  bool FilledAfterNoHits = DataSet.FillResultData(TwoGroups);
  DataSet.Initialize(2, 1);
  DataSet.Initialize(2, 0);
  bool FilledAfterNoGroups = DataSet.FillEventData(1, TwoHits);
  EnableDefaultStreams();
  // A refused initialization without hits accepts no data, since the stored number of hits is zero
  Passed = EvaluateFalse("Initialize(0, 1)", "after (2, 1)", "A data set whose initialization was refused accepts no data", FilledAfterNoHits) && Passed;
  // Defect REV-D-7: A refused Initialize without groups stores the new numbers, but keeps the arrays of the initialization before, so the data set still accepts hits
  Passed = ExpectedToFail(EvaluateFalse("Initialize(2, 0)", "after (2, 1)", "A data set whose initialization was refused accepts no data", FilledAfterNoGroups)) && Passed;

  DeleteHits(Hits);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling the event data
bool UTEREventClusterizerDataSet::TestFillEventData()
{
  bool Passed = true;

  MEREventClusterizerDataSet DataSet(3, 2);
  vector<MRESE*> Hits = CreateHits();

  Passed = EvaluateTrue("FillEventData()", "three hits", "The data is accepted", DataSet.FillEventData(123456, Hits)) && Passed;
  Passed = EvaluateNear("FillEventData()", "ID 123456", "The simulation ID is stored", DataSet.m_SimulationID, 123456.0, 0.0) && Passed;
  double Energies[3] = { 100.5, 20.25, 661.0 };
  double X[3] = { 1.0, -4.0, 0.0 };
  double Y[3] = { 2.0, 5.5, 0.0 };
  double Z[3] = { 3.0, -6.25, 0.0 };
  for (unsigned int h = 0; h < 3; ++h) {
    Passed = EvaluateNear("FillEventData()", MString("hit ") + h, "The energy is stored", DataSet.m_Energies[h], Energies[h], 0.0) && Passed;
    Passed = EvaluateNear("FillEventData()", MString("hit ") + h, "The x position is stored", DataSet.m_PositionsX[h], X[h], 0.0) && Passed;
    Passed = EvaluateNear("FillEventData()", MString("hit ") + h, "The y position is stored", DataSet.m_PositionsY[h], Y[h], 0.0) && Passed;
    Passed = EvaluateNear("FillEventData()", MString("hit ") + h, "The z position is stored", DataSet.m_PositionsZ[h], Z[h], 0.0) && Passed;
  }
  Passed = EvaluateNear("FillEventData()", "three hits", "The groups are not changed", DataSet.m_ResultHitGroups[0][0], 0.0, 0.0) && Passed;

  // A second event with the hits in reversed order replaces the first one
  vector<MRESE*> Reversed = { Hits[2], Hits[1], Hits[0] };
  Passed = EvaluateTrue("FillEventData()", "second event", "The data is accepted", DataSet.FillEventData(-7, Reversed)) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event, ID -7", "The simulation ID is replaced", DataSet.m_SimulationID, -7.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event", "The first energy is replaced", DataSet.m_Energies[0], 661.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event", "The last z position is replaced", DataSet.m_PositionsZ[2], 3.0, 0.0) && Passed;

  // The number of hits has to be the one of the data set: one too few, one too many, none - and nothing is stored
  vector<MRESE*> Two = { Hits[0], Hits[1] };
  vector<MRESE*> Four = { Hits[0], Hits[1], Hits[2], Hits[0] };
  vector<MRESE*> None;
  DisableDefaultStreams();
  bool TooFew = DataSet.FillEventData(99, Two);
  bool TooMany = DataSet.FillEventData(99, Four);
  bool Empty = DataSet.FillEventData(99, None);
  EnableDefaultStreams();
  Passed = EvaluateFalse("FillEventData()", "two hits for three", "Too few hits are refused", TooFew) && Passed;
  Passed = EvaluateFalse("FillEventData()", "four hits for three", "Too many hits are refused", TooMany) && Passed;
  Passed = EvaluateFalse("FillEventData()", "no hits", "An empty list is refused", Empty) && Passed;
  Passed = EvaluateNear("FillEventData()", "refused data", "The simulation ID of the event before stays", DataSet.m_SimulationID, -7.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "refused data", "The energies of the event before stay", DataSet.m_Energies[0], 661.0, 0.0) && Passed;

  DeleteHits(Hits);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling the result data
bool UTEREventClusterizerDataSet::TestFillResultData()
{
  bool Passed = true;

  // The entries of the groups have a random jitter: 0.001*(Rndm() - 0.5) is in ]-0.0005, 0.0005[
  const double Jitter = 0.0005;

  MEREventClusterizerDataSet DataSet(3, 2);

  // The first hit is in group 0, the second in the groups 0 and 1, the third in none
  vector<vector<int>> Groups = { { 0 }, { 0, 1 }, { } };
  Passed = EvaluateTrue("FillResultData()", "three hits, groups 0 | 0, 1 | none", "The result is accepted", DataSet.FillResultData(Groups)) && Passed;
  double Expected[3][3] = { { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 0.0 }, { 0.0, 0.0, 0.0 } };
  for (unsigned int h = 0; h < 3; ++h) {
    for (unsigned int g = 0; g < 3; ++g) {
      Passed = EvaluateNear("FillResultData()", MString("hit ") + h + ", group " + g, "The entry is 1 if the hit is in the group and 0 otherwise, within the jitter", DataSet.m_ResultHitGroups[h][g], Expected[h][g], Jitter) && Passed;
    }
  }
  Passed = EvaluateNear("FillResultData()", "three hits", "The event data is not changed", DataSet.m_SimulationID, -1.0, 0.0) && Passed;

  // The jitter is there: two zeros are not identical
  Passed = EvaluateTrue("FillResultData()", "jitter", "The entries are not exactly zero and differ from each other", DataSet.m_ResultHitGroups[2][0] != 0.0 && DataSet.m_ResultHitGroups[2][0] != DataSet.m_ResultHitGroups[2][1]) && Passed;

  // A second result replaces the first one completely
  vector<vector<int>> Groups2 = { { 1 }, { }, { 0 } };
  Passed = EvaluateTrue("FillResultData()", "second result", "The result is accepted", DataSet.FillResultData(Groups2)) && Passed;
  double Expected2[3][3] = { { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 } };
  for (unsigned int h = 0; h < 3; ++h) {
    for (unsigned int g = 0; g < 3; ++g) {
      Passed = EvaluateNear("FillResultData()", MString("second result, hit ") + h + ", group " + g, "The entry of the result before is replaced", DataSet.m_ResultHitGroups[h][g], Expected2[h][g], Jitter) && Passed;
    }
  }

  // Groups at and beyond the maximum number of groups (2) go into the last entry: group 1 is the last regular one, 2 and 7 are overflows
  vector<vector<int>> Overflow = { { 1 }, { 2 }, { 7 } };
  Passed = EvaluateTrue("FillResultData()", "groups 1 | 2 | 7", "The result is accepted", DataSet.FillResultData(Overflow)) && Passed;
  double ExpectedOverflow[3][3] = { { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
  for (unsigned int h = 0; h < 3; ++h) {
    for (unsigned int g = 0; g < 3; ++g) {
      Passed = EvaluateNear("FillResultData()", MString("overflow, hit ") + h + ", group " + g, "Groups beyond the maximum are in the last entry", DataSet.m_ResultHitGroups[h][g], ExpectedOverflow[h][g], Jitter) && Passed;
    }
  }

  // The same group twice
  vector<vector<int>> Twice = { { 0, 0 }, { }, { } };
  Passed = EvaluateTrue("FillResultData()", "group 0 twice", "The result is accepted", DataSet.FillResultData(Twice)) && Passed;
  Passed = EvaluateNear("FillResultData()", "group 0 twice", "The entry is 1, not 2", DataSet.m_ResultHitGroups[0][0], 1.0, Jitter) && Passed;

  // The number of hits has to be the one of the data set
  vector<vector<int>> Two = { { 0 }, { 1 } };
  vector<vector<int>> Four = { { 0 }, { 1 }, { 0 }, { 1 } };
  vector<vector<int>> None;
  DisableDefaultStreams();
  bool TooFew = DataSet.FillResultData(Two);
  bool TooMany = DataSet.FillResultData(Four);
  bool Empty = DataSet.FillResultData(None);
  EnableDefaultStreams();
  Passed = EvaluateFalse("FillResultData()", "two hits for three", "Too few hits are refused", TooFew) && Passed;
  Passed = EvaluateFalse("FillResultData()", "four hits for three", "Too many hits are refused", TooMany) && Passed;
  Passed = EvaluateFalse("FillResultData()", "no hits", "An empty list is refused", Empty) && Passed;
  Passed = EvaluateNear("FillResultData()", "refused result", "The result before stays", DataSet.m_ResultHitGroups[0][0], 1.0, Jitter) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string
bool UTEREventClusterizerDataSet::TestToString()
{
  bool Passed = true;

  MEREventClusterizerDataSet DataSet(3, 2);
  vector<MRESE*> Hits = CreateHits();
  DataSet.FillEventData(42, Hits);
  DeleteHits(Hits);

  // Without a result the groups are exactly zero
  MString Text = DataSet.ToString();
  vector<MString> Lines = Text.Tokenize("\n");
  Passed = EvaluateSize("ToString()", "three hits", "There is one line for the ID and one per hit", Lines.size(), 4) && Passed;
  if (Lines.size() == 4) {
    Passed = Evaluate("ToString()", "three hits", "The first line is the ID", Lines[0], MString("ID 42")) && Passed;
    Passed = EvaluateTrue("ToString()", "first hit", "The line has the index, the energy, and the position", Lines[1].BeginsWith("(0) E=100.5 P=(1/2/3) --> ")) && Passed;
    Passed = EvaluateTrue("ToString()", "second hit", "The line has the index, the energy, and the position", Lines[2].BeginsWith("(1) E=20.25 P=(-4/5.5/-6.25) --> ")) && Passed;
    Passed = EvaluateTrue("ToString()", "third hit", "The line has the index, the energy, and the position", Lines[3].BeginsWith("(2) E=661 P=(0/0/0) --> ")) && Passed;
  }

  // A second call gives the same string
  Passed = Evaluate("ToString()", "second call", "The string does not change", DataSet.ToString(), Text) && Passed;

  // A new data set
  MEREventClusterizerDataSet New(1, 1);
  vector<MString> NewLines = New.ToString().Tokenize("\n");
  Passed = EvaluateSize("ToString()", "new data set with one hit", "There is one line for the ID and one for the hit", NewLines.size(), 2) && Passed;
  if (NewLines.size() == 2) {
    Passed = Evaluate("ToString()", "new data set with one hit", "The ID is -1", NewLines[0], MString("ID -1")) && Passed;
    Passed = EvaluateTrue("ToString()", "new data set with one hit", "The hit is empty", NewLines[1].BeginsWith("(0) E=0 P=(0/0/0) --> 0 ")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the tree
bool UTEREventClusterizerDataSet::TestCreateTree()
{
  bool Passed = true;

  const double Jitter = 0.0005;

  MEREventClusterizerDataSet DataSet(3, 2);
  TTree* Tree = DataSet.CreateTree("EventClusterizer");
  Passed = EvaluateTrue("CreateTree()", "3 hits, 2 groups", "A tree is created", Tree != nullptr) && Passed;
  if (Tree == nullptr) return Passed;

  Passed = Evaluate("CreateTree()", "3 hits, 2 groups", "The tree has the given name", MString(Tree->GetName()), MString("EventClusterizer")) && Passed;
  // ID + 3 energies + 3 x 3 positions + 3 hits x (2 groups + overflow) = 22
  Passed = Evaluate("CreateTree()", "3 hits, 2 groups", "The tree has 22 branches", Tree->GetNbranches(), 22) && Passed;

  // The names count from one
  MString Names[22] = { "SimulationID", "Energy_1", "Energy_2", "Energy_3", "PositionX_1", "PositionX_2", "PositionX_3",
                        "PositionY_1", "PositionY_2", "PositionY_3", "PositionZ_1", "PositionZ_2", "PositionZ_3",
                        "ResultHitGroups_1_1", "ResultHitGroups_1_2", "ResultHitGroups_1_3",
                        "ResultHitGroups_2_1", "ResultHitGroups_2_2", "ResultHitGroups_2_3",
                        "ResultHitGroups_3_1", "ResultHitGroups_3_2", "ResultHitGroups_3_3" };
  for (unsigned int n = 0; n < 22; ++n) {
    Passed = EvaluateTrue("CreateTree()", Names[n], "The branch exists", Tree->GetBranch(Names[n]) != nullptr) && Passed;
  }
  Passed = EvaluateTrue("CreateTree()", "Energy_4", "There is no branch for a fourth hit", Tree->GetBranch("Energy_4") == nullptr) && Passed;
  Passed = EvaluateTrue("CreateTree()", "ResultHitGroups_1_4", "There is no branch for a fourth group", Tree->GetBranch("ResultHitGroups_1_4") == nullptr) && Passed;

  // Two events: the tree reads back into the data set
  vector<MRESE*> Hits = CreateHits();
  vector<vector<int>> Groups = { { 0 }, { 0, 1 }, { } };
  DataSet.FillEventData(11, Hits);
  DataSet.FillResultData(Groups);
  Tree->Fill();

  vector<MRESE*> Reversed = { Hits[2], Hits[1], Hits[0] };
  vector<vector<int>> Groups2 = { { 1 }, { }, { 5 } };
  DataSet.FillEventData(12, Reversed);
  DataSet.FillResultData(Groups2);
  Tree->Fill();
  DeleteHits(Hits);

  Passed = Evaluate("CreateTree()", "two events", "The tree has two entries", Tree->GetEntries(), static_cast<Long64_t>(2)) && Passed;

  Tree->GetEntry(0);
  Passed = EvaluateNear("CreateTree()", "first entry", "The simulation ID is read back", DataSet.m_SimulationID, 11.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The first energy is read back", DataSet.m_Energies[0], 100.5, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second x position is read back", DataSet.m_PositionsX[1], -4.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second y position is read back", DataSet.m_PositionsY[1], 5.5, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second z position is read back", DataSet.m_PositionsZ[1], -6.25, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second group of the second hit is read back", DataSet.m_ResultHitGroups[1][1], 1.0, Jitter) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second group of the first hit is read back", DataSet.m_ResultHitGroups[0][1], 0.0, Jitter) && Passed;

  Tree->GetEntry(1);
  Passed = EvaluateNear("CreateTree()", "second entry", "The simulation ID is read back", DataSet.m_SimulationID, 12.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The first energy is read back", DataSet.m_Energies[0], 661.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The last z position is read back", DataSet.m_PositionsZ[2], 3.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The overflow group of the third hit is read back", DataSet.m_ResultHitGroups[2][2], 1.0, Jitter) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The second group of the second hit is read back", DataSet.m_ResultHitGroups[1][1], 0.0, Jitter) && Passed;

  delete Tree;

  // The smallest data set: ID + 4 values of the hit + group + overflow = 7 branches
  MEREventClusterizerDataSet Smallest(1, 1);
  TTree* SmallestTree = Smallest.CreateTree("Smallest");
  Passed = EvaluateTrue("CreateTree()", "1 hit, 1 group", "A tree is created", SmallestTree != nullptr) && Passed;
  if (SmallestTree != nullptr) {
    Passed = Evaluate("CreateTree()", "1 hit, 1 group", "The tree has 7 branches", SmallestTree->GetNbranches(), 7) && Passed;
  }
  delete SmallestTree;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the reader
bool UTEREventClusterizerDataSet::TestCreateReader()
{
  bool Passed = true;

  MEREventClusterizerDataSet DataSet(3, 2);
  BeginCaptureOutput();
  TMVA::Reader* Reader = DataSet.CreateReader();
  EndCaptureOutput();
  Passed = EvaluateTrue("CreateReader()", "3 hits, 2 groups", "A reader is created", Reader != nullptr) && Passed;
  if (Reader == nullptr) return Passed;

  // The energies and positions are the variables, the ID is a spectator, and the groups are the targets of the training, thus not part of the reader
  Passed = Evaluate("CreateReader()", "3 hits, 2 groups", "The reader has 12 variables", Reader->DataInfo().GetNVariables(), 12U) && Passed;
  Passed = Evaluate("CreateReader()", "3 hits, 2 groups", "The reader has one spectator", Reader->DataInfo().GetNSpectators(), 1U) && Passed;
  MString Names[12] = { "Energy_1", "Energy_2", "Energy_3", "PositionX_1", "PositionX_2", "PositionX_3",
                        "PositionY_1", "PositionY_2", "PositionY_3", "PositionZ_1", "PositionZ_2", "PositionZ_3" };
  if (Reader->DataInfo().GetNVariables() == 12U) {
    for (unsigned int n = 0; n < 12; ++n) {
      Passed = Evaluate("CreateReader()", MString("variable ") + n, "The variables have the order and the names of the branches of the tree", MString(Reader->DataInfo().GetVariableInfo(n).GetExpression()), Names[n]) && Passed;
    }
  }
  if (Reader->DataInfo().GetNSpectators() == 1U) {
    Passed = Evaluate("CreateReader()", "spectator", "The spectator is the simulation ID", MString(Reader->DataInfo().GetSpectatorInfo(0).GetExpression()), MString("SimulationID")) && Passed;
  }
  delete Reader;

  // A second reader of the same data set
  BeginCaptureOutput();
  TMVA::Reader* Second = DataSet.CreateReader();
  EndCaptureOutput();
  Passed = EvaluateTrue("CreateReader()", "second reader", "A second reader is created", Second != nullptr) && Passed;
  if (Second != nullptr) {
    Passed = Evaluate("CreateReader()", "second reader", "The second reader has 12 variables", Second->DataInfo().GetNVariables(), 12U) && Passed;
  }
  delete Second;

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

  UTEREventClusterizerDataSet Test;
  return Test.Run() == true ? 0 : 1;
}


// UTEREventClusterizerDataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
