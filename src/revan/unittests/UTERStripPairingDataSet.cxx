/*
 * UTERStripPairingDataSet.cxx
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
#include "TTree.h"

// MEGAlib libs:
#include "MERStripPairingDataSet.h"
#include "MString.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the data set of the TMVA strip pairing MERStripPairingDataSet
class UTERStripPairingDataSet : public MUnitTest
{
public:
  //! Default constructor
  UTERStripPairingDataSet() : MUnitTest("UTERStripPairingDataSet") {}
  //! Default destructor
  virtual ~UTERStripPairingDataSet() {}

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
  //! Test the tree
  bool TestCreateTree();
  //! Test the reader
  bool TestCreateReader();

};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERStripPairingDataSet::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestFillEventData() && Passed;
  Passed = TestFillResultData() && Passed;
  Passed = TestCreateTree() && Passed;
  Passed = TestCreateReader() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructors on memory with a pattern
bool UTERStripPairingDataSet::TestConstruction()
{
  bool Passed = true;

  // The standard constructor: 0xA5A5A5A5 is the float -2.87e-16, which is no default of any member
  {
    alignas(MERStripPairingDataSet) unsigned char Block[sizeof(MERStripPairingDataSet)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MERStripPairingDataSet* DataSet = new (Block) MERStripPairingDataSet(3, 2);
    Passed = EvaluateNear("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "The simulation ID is -1", DataSet->m_SimulationID, -1.0, 0.0) && Passed;
    Passed = EvaluateNear("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "The number of interactions is zero", DataSet->m_ResultNumberOfInteractions, 0.0, 0.0) && Passed;
    Passed = EvaluateNear("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "The flag for undetected interactions is zero", DataSet->m_ResultUndetectedInteractions, 0.0, 0.0) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "There are three x strip IDs", DataSet->m_XStripIDs.size(), 3) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "There are two y strip IDs", DataSet->m_YStripIDs.size(), 2) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "There are three x strip energies", DataSet->m_XStripEnergies.size(), 3) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "There are two y strip energies", DataSet->m_YStripEnergies.size(), 2) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "There are 3 x 2 interactions", DataSet->m_ResultInteractions.size(), 6) && Passed;
    bool AllZero = true;
    for (unsigned int i = 0; i < DataSet->m_XStripIDs.size(); ++i) {
      if (DataSet->m_XStripIDs[i] != 0 || DataSet->m_XStripEnergies[i] != 0) AllZero = false;
    }
    for (unsigned int i = 0; i < DataSet->m_YStripIDs.size(); ++i) {
      if (DataSet->m_YStripIDs[i] != 0 || DataSet->m_YStripEnergies[i] != 0) AllZero = false;
    }
    for (unsigned int i = 0; i < DataSet->m_ResultInteractions.size(); ++i) {
      if (DataSet->m_ResultInteractions[i] != 0) AllZero = false;
    }
    Passed = EvaluateTrue("MERStripPairingDataSet(3, 2)", "memory with 0xA5", "All IDs, energies, and interactions are zero", AllZero) && Passed;
    DataSet->~MERStripPairingDataSet();
  }

  // The default constructor
  {
    alignas(MERStripPairingDataSet) unsigned char Block[sizeof(MERStripPairingDataSet)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MERStripPairingDataSet* DataSet = new (Block) MERStripPairingDataSet();
    Passed = EvaluateSize("MERStripPairingDataSet()", "memory with 0xA5", "There are no x strip IDs", DataSet->m_XStripIDs.size(), 0) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet()", "memory with 0xA5", "There are no y strip IDs", DataSet->m_YStripIDs.size(), 0) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet()", "memory with 0xA5", "There are no x strip energies", DataSet->m_XStripEnergies.size(), 0) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet()", "memory with 0xA5", "There are no y strip energies", DataSet->m_YStripEnergies.size(), 0) && Passed;
    Passed = EvaluateSize("MERStripPairingDataSet()", "memory with 0xA5", "There are no interactions", DataSet->m_ResultInteractions.size(), 0) && Passed;
    // Defect REV-D-4: The default constructor does not initialize the simulation ID, the number of interactions, and the flag for undetected interactions
    Passed = ExpectedToFail(EvaluateFalse("MERStripPairingDataSet()", "memory with 0xA5", "The simulation ID is initialized", IsFillPattern(&DataSet->m_SimulationID, sizeof(Float_t), 0xA5))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("MERStripPairingDataSet()", "memory with 0xA5", "The number of interactions is initialized", IsFillPattern(&DataSet->m_ResultNumberOfInteractions, sizeof(Float_t), 0xA5))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("MERStripPairingDataSet()", "memory with 0xA5", "The flag for undetected interactions is initialized", IsFillPattern(&DataSet->m_ResultUndetectedInteractions, sizeof(Float_t), 0xA5))) && Passed;

    // Without strips no data is accepted
    vector<unsigned int> IDs = { 1 };
    vector<double> Energies = { 100.0 };
    DisableDefaultStreams();
    bool Filled = DataSet->FillEventData(1, IDs, IDs, Energies, Energies);
    EnableDefaultStreams();
    Passed = EvaluateFalse("FillEventData()", "default constructed, one strip each", "A data set without strips accepts no strips", Filled) && Passed;
    DataSet->~MERStripPairingDataSet();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the initialization
bool UTERStripPairingDataSet::TestInitialize()
{
  bool Passed = true;

  MERStripPairingDataSet DataSet;
  DataSet.Initialize(1, 1);
  Passed = EvaluateSize("Initialize(1, 1)", "smallest data set", "There is one x strip ID", DataSet.m_XStripIDs.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(1, 1)", "smallest data set", "There is one y strip ID", DataSet.m_YStripIDs.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(1, 1)", "smallest data set", "There is one interaction", DataSet.m_ResultInteractions.size(), 1) && Passed;
  Passed = EvaluateNear("Initialize(1, 1)", "smallest data set", "The simulation ID is -1", DataSet.m_SimulationID, -1.0, 0.0) && Passed;

  // Fill it, and initialize it again with another size: the sizes change, and the IDs are reset
  vector<unsigned int> XIDs = { 7 };
  vector<unsigned int> YIDs = { 9 };
  vector<double> XEnergies = { 100.0 };
  vector<double> YEnergies = { 101.0 };
  vector<double> Interactions = { 1.0 };
  Passed = EvaluateTrue("FillEventData()", "one strip each", "The data is accepted", DataSet.FillEventData(42, XIDs, YIDs, XEnergies, YEnergies)) && Passed;
  Passed = EvaluateTrue("FillResultData()", "one strip each", "The result is accepted", DataSet.FillResultData(1, true, Interactions)) && Passed;

  DataSet.Initialize(4, 5);
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are four x strip IDs", DataSet.m_XStripIDs.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are five y strip IDs", DataSet.m_YStripIDs.size(), 5) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are four x strip energies", DataSet.m_XStripEnergies.size(), 4) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are five y strip energies", DataSet.m_YStripEnergies.size(), 5) && Passed;
  Passed = EvaluateSize("Initialize(4, 5)", "after (1, 1)", "There are 20 interactions", DataSet.m_ResultInteractions.size(), 20) && Passed;
  Passed = EvaluateNear("Initialize(4, 5)", "after (1, 1)", "The simulation ID is -1 again", DataSet.m_SimulationID, -1.0, 0.0) && Passed;
  Passed = EvaluateNear("Initialize(4, 5)", "after (1, 1)", "The number of interactions is zero again", DataSet.m_ResultNumberOfInteractions, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("Initialize(4, 5)", "after (1, 1)", "The flag for undetected interactions is zero again", DataSet.m_ResultUndetectedInteractions, 0.0, 0.0) && Passed;

  // Smaller again
  DataSet.Initialize(2, 1);
  Passed = EvaluateSize("Initialize(2, 1)", "after (4, 5)", "There are two x strip IDs", DataSet.m_XStripIDs.size(), 2) && Passed;
  Passed = EvaluateSize("Initialize(2, 1)", "after (4, 5)", "There is one y strip ID", DataSet.m_YStripIDs.size(), 1) && Passed;
  Passed = EvaluateSize("Initialize(2, 1)", "after (4, 5)", "There are two interactions", DataSet.m_ResultInteractions.size(), 2) && Passed;

  // Without strips in one direction the initialization is refused with an error message
  vector<unsigned int> TwoIDs = { 1, 2 };
  vector<unsigned int> OneID = { 3 };
  vector<double> TwoEnergies = { 10.0, 20.0 };
  vector<double> OneEnergy = { 30.0 };
  DisableDefaultStreams();
  DataSet.Initialize(0, 1);
  bool FilledAfterNoX = DataSet.FillEventData(1, TwoIDs, OneID, TwoEnergies, OneEnergy);
  DataSet.Initialize(2, 1);
  DataSet.Initialize(2, 0);
  bool FilledAfterNoY = DataSet.FillEventData(1, TwoIDs, OneID, TwoEnergies, OneEnergy);
  EnableDefaultStreams();
  // Defect REV-D-5: A refused Initialize stores the new numbers of strips, but keeps the arrays of the initialization before, so the data set still accepts data of the old size
  Passed = ExpectedToFail(EvaluateFalse("Initialize(0, 1)", "after (2, 1)", "A data set whose initialization was refused accepts no data", FilledAfterNoX)) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("Initialize(2, 0)", "after (2, 1)", "A data set whose initialization was refused accepts no data", FilledAfterNoY)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling the event data
bool UTERStripPairingDataSet::TestFillEventData()
{
  bool Passed = true;

  MERStripPairingDataSet DataSet(3, 2);

  vector<unsigned int> XIDs = { 5, 17, 0 };
  vector<unsigned int> YIDs = { 63, 2 };
  vector<double> XEnergies = { 100.5, 20.25, 661.0 };
  vector<double> YEnergies = { 400.125, 381.5 };
  Passed = EvaluateTrue("FillEventData()", "3 x 2 strips", "The data is accepted", DataSet.FillEventData(123456, XIDs, YIDs, XEnergies, YEnergies)) && Passed;

  // All values are exact in single precision
  Passed = EvaluateNear("FillEventData()", "ID 123456", "The simulation ID is stored", DataSet.m_SimulationID, 123456.0, 0.0) && Passed;
  for (unsigned int i = 0; i < 3; ++i) {
    Passed = EvaluateNear("FillEventData()", MString("x strip ") + i, "The ID is stored", DataSet.m_XStripIDs[i], XIDs[i], 0.0) && Passed;
    Passed = EvaluateNear("FillEventData()", MString("x strip ") + i, "The energy is stored", DataSet.m_XStripEnergies[i], XEnergies[i], 0.0) && Passed;
  }
  for (unsigned int i = 0; i < 2; ++i) {
    Passed = EvaluateNear("FillEventData()", MString("y strip ") + i, "The ID is stored", DataSet.m_YStripIDs[i], YIDs[i], 0.0) && Passed;
    Passed = EvaluateNear("FillEventData()", MString("y strip ") + i, "The energy is stored", DataSet.m_YStripEnergies[i], YEnergies[i], 0.0) && Passed;
  }
  Passed = EvaluateNear("FillEventData()", "3 x 2 strips", "The result is not changed", DataSet.m_ResultNumberOfInteractions, 0.0, 0.0) && Passed;

  // A second event replaces the first one
  vector<unsigned int> XIDs2 = { 1, 2, 3 };
  vector<unsigned int> YIDs2 = { 4, 5 };
  vector<double> XEnergies2 = { 1.5, 2.5, 3.5 };
  vector<double> YEnergies2 = { 4.5, 5.5 };
  Passed = EvaluateTrue("FillEventData()", "second event", "The data is accepted", DataSet.FillEventData(-7, XIDs2, YIDs2, XEnergies2, YEnergies2)) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event, ID -7", "The simulation ID is replaced", DataSet.m_SimulationID, -7.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event", "The last x strip ID is replaced", DataSet.m_XStripIDs[2], 3.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "second event", "The last y strip energy is replaced", DataSet.m_YStripEnergies[1], 5.5, 0.0) && Passed;

  // Each of the four arrays is checked for its size: one too few and one too many, and nothing is stored
  vector<unsigned int> TwoIDs = { 8, 9 };
  vector<unsigned int> ThreeIDs = { 8, 9, 10 };
  vector<unsigned int> FourIDs = { 8, 9, 10, 11 };
  vector<unsigned int> OneID = { 8 };
  vector<double> TwoEnergies = { 80.0, 90.0 };
  vector<double> ThreeEnergies = { 80.0, 90.0, 100.0 };
  vector<double> FourEnergies = { 80.0, 90.0, 100.0, 110.0 };
  vector<double> OneEnergy = { 80.0 };
  vector<unsigned int> NoIDs;
  vector<double> NoEnergies;

  DisableDefaultStreams();
  bool TooFewX = DataSet.FillEventData(99, TwoIDs, TwoIDs, ThreeEnergies, TwoEnergies);
  bool TooManyX = DataSet.FillEventData(99, FourIDs, TwoIDs, ThreeEnergies, TwoEnergies);
  bool TooFewY = DataSet.FillEventData(99, ThreeIDs, OneID, ThreeEnergies, TwoEnergies);
  bool TooManyY = DataSet.FillEventData(99, ThreeIDs, ThreeIDs, ThreeEnergies, TwoEnergies);
  bool TooFewXEnergies = DataSet.FillEventData(99, ThreeIDs, TwoIDs, TwoEnergies, TwoEnergies);
  bool TooManyXEnergies = DataSet.FillEventData(99, ThreeIDs, TwoIDs, FourEnergies, TwoEnergies);
  bool TooFewYEnergies = DataSet.FillEventData(99, ThreeIDs, TwoIDs, ThreeEnergies, OneEnergy);
  bool TooManyYEnergies = DataSet.FillEventData(99, ThreeIDs, TwoIDs, ThreeEnergies, ThreeEnergies);
  bool Empty = DataSet.FillEventData(99, NoIDs, NoIDs, NoEnergies, NoEnergies);
  EnableDefaultStreams();
  Passed = EvaluateFalse("FillEventData()", "two x strips for three", "Too few x strips are refused", TooFewX) && Passed;
  Passed = EvaluateFalse("FillEventData()", "four x strips for three", "Too many x strips are refused", TooManyX) && Passed;
  Passed = EvaluateFalse("FillEventData()", "one y strip for two", "Too few y strips are refused", TooFewY) && Passed;
  Passed = EvaluateFalse("FillEventData()", "three y strips for two", "Too many y strips are refused", TooManyY) && Passed;
  Passed = EvaluateFalse("FillEventData()", "two x energies for three", "Too few x energies are refused", TooFewXEnergies) && Passed;
  Passed = EvaluateFalse("FillEventData()", "four x energies for three", "Too many x energies are refused", TooManyXEnergies) && Passed;
  Passed = EvaluateFalse("FillEventData()", "one y energy for two", "Too few y energies are refused", TooFewYEnergies) && Passed;
  Passed = EvaluateFalse("FillEventData()", "three y energies for two", "Too many y energies are refused", TooManyYEnergies) && Passed;
  Passed = EvaluateFalse("FillEventData()", "no strips", "Empty arrays are refused", Empty) && Passed;
  Passed = EvaluateNear("FillEventData()", "refused data", "The simulation ID of the event before stays", DataSet.m_SimulationID, -7.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "refused data", "The x strip IDs of the event before stay", DataSet.m_XStripIDs[0], 1.0, 0.0) && Passed;
  Passed = EvaluateNear("FillEventData()", "refused data", "The y strip energies of the event before stay", DataSet.m_YStripEnergies[0], 4.5, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test filling the result data
bool UTERStripPairingDataSet::TestFillResultData()
{
  bool Passed = true;

  MERStripPairingDataSet DataSet(3, 2);

  // The interactions are filled in the order x + y * (number of x strips)
  vector<double> Interactions = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
  Passed = EvaluateTrue("FillResultData()", "3 x 2 strips, two interactions", "The result is accepted", DataSet.FillResultData(2, true, Interactions)) && Passed;
  Passed = EvaluateNear("FillResultData()", "two interactions", "The number of interactions is stored", DataSet.m_ResultNumberOfInteractions, 2.0, 0.0) && Passed;
  Passed = EvaluateNear("FillResultData()", "undetected interactions", "The flag is stored as 1", DataSet.m_ResultUndetectedInteractions, 1.0, 0.0) && Passed;
  for (unsigned int i = 0; i < 6; ++i) {
    Passed = EvaluateNear("FillResultData()", MString("interaction ") + i, "The interaction is stored", DataSet.m_ResultInteractions[i], Interactions[i], 0.0) && Passed;
  }
  Passed = EvaluateNear("FillResultData()", "3 x 2 strips", "The event data is not changed", DataSet.m_SimulationID, -1.0, 0.0) && Passed;

  // A second result replaces the first one
  vector<double> Interactions2 = { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 };
  Passed = EvaluateTrue("FillResultData()", "second result", "The result is accepted", DataSet.FillResultData(1, false, Interactions2)) && Passed;
  Passed = EvaluateNear("FillResultData()", "second result", "The number of interactions is replaced", DataSet.m_ResultNumberOfInteractions, 1.0, 0.0) && Passed;
  Passed = EvaluateNear("FillResultData()", "no undetected interactions", "The flag is stored as 0", DataSet.m_ResultUndetectedInteractions, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("FillResultData()", "second result", "The first interaction is replaced", DataSet.m_ResultInteractions[0], 0.0, 0.0) && Passed;
  Passed = EvaluateNear("FillResultData()", "second result", "The second interaction is replaced", DataSet.m_ResultInteractions[1], 1.0, 0.0) && Passed;
  Passed = EvaluateNear("FillResultData()", "second result", "The last interaction is replaced", DataSet.m_ResultInteractions[5], 0.0, 0.0) && Passed;

  // The number of interactions has to be the product of the numbers of strips: one too few, one too many, none
  vector<double> Five = { 1.0, 1.0, 1.0, 1.0, 1.0 };
  vector<double> Seven = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
  vector<double> None;
  DisableDefaultStreams();
  bool TooFew = DataSet.FillResultData(5, true, Five);
  bool TooMany = DataSet.FillResultData(7, true, Seven);
  bool Empty = DataSet.FillResultData(0, true, None);
  EnableDefaultStreams();
  Passed = EvaluateFalse("FillResultData()", "five interactions for six", "Too few interactions are refused", TooFew) && Passed;
  Passed = EvaluateFalse("FillResultData()", "seven interactions for six", "Too many interactions are refused", TooMany) && Passed;
  Passed = EvaluateFalse("FillResultData()", "no interactions", "An empty array is refused", Empty) && Passed;
  Passed = EvaluateNear("FillResultData()", "refused result", "The number of interactions of the result before stays", DataSet.m_ResultNumberOfInteractions, 1.0, 0.0) && Passed;
  Passed = EvaluateNear("FillResultData()", "refused result", "The flag of the result before stays", DataSet.m_ResultUndetectedInteractions, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("FillResultData()", "refused result", "The interactions of the result before stay", DataSet.m_ResultInteractions[1], 1.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the tree
bool UTERStripPairingDataSet::TestCreateTree()
{
  bool Passed = true;

  MERStripPairingDataSet DataSet(3, 2);
  TTree* Tree = DataSet.CreateTree("StripPairing");
  Passed = EvaluateTrue("CreateTree()", "3 x 2 strips", "A tree is created", Tree != nullptr) && Passed;
  if (Tree == nullptr) return Passed;

  Passed = Evaluate("CreateTree()", "3 x 2 strips", "The tree has the given name", MString(Tree->GetName()), MString("StripPairing")) && Passed;
  Passed = Evaluate("CreateTree()", "3 x 2 strips", "The tree has the given title", MString(Tree->GetTitle()), MString("StripPairing")) && Passed;
  // ID + 3 x IDs + 2 y IDs + 3 x energies + 2 y energies + number of interactions + undetected flag + 3 x 2 interactions = 19
  Passed = Evaluate("CreateTree()", "3 x 2 strips", "The tree has 19 branches", Tree->GetNbranches(), 19) && Passed;

  // The names count from one
  MString Names[19] = { "SimulationID", "XStripID1", "XStripID2", "XStripID3", "YStripID1", "YStripID2",
                        "XStripEnergy1", "XStripEnergy2", "XStripEnergy3", "YStripEnergy1", "YStripEnergy2",
                        "ResultNumberOfInteractions", "ResultUndetectedInteractions",
                        "ResultInteraction1", "ResultInteraction2", "ResultInteraction3", "ResultInteraction4", "ResultInteraction5", "ResultInteraction6" };
  for (unsigned int n = 0; n < 19; ++n) {
    Passed = EvaluateTrue("CreateTree()", Names[n], "The branch exists", Tree->GetBranch(Names[n]) != nullptr) && Passed;
  }
  Passed = EvaluateTrue("CreateTree()", "XStripID4", "There is no branch for a fourth x strip", Tree->GetBranch("XStripID4") == nullptr) && Passed;
  Passed = EvaluateTrue("CreateTree()", "XStripID0", "There is no branch with index zero", Tree->GetBranch("XStripID0") == nullptr) && Passed;

  // Two events: the tree reads back into the data set
  vector<unsigned int> XIDs = { 5, 17, 0 };
  vector<unsigned int> YIDs = { 63, 2 };
  vector<double> XEnergies = { 100.5, 20.25, 661.0 };
  vector<double> YEnergies = { 400.125, 381.5 };
  vector<double> Interactions = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
  DataSet.FillEventData(11, XIDs, YIDs, XEnergies, YEnergies);
  DataSet.FillResultData(2, true, Interactions);
  Tree->Fill();

  vector<unsigned int> XIDs2 = { 1, 2, 3 };
  vector<unsigned int> YIDs2 = { 4, 5 };
  vector<double> XEnergies2 = { 1.5, 2.5, 3.5 };
  vector<double> YEnergies2 = { 4.5, 5.5 };
  vector<double> Interactions2 = { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 };
  DataSet.FillEventData(12, XIDs2, YIDs2, XEnergies2, YEnergies2);
  DataSet.FillResultData(1, false, Interactions2);
  Tree->Fill();

  Passed = Evaluate("CreateTree()", "two events", "The tree has two entries", Tree->GetEntries(), static_cast<Long64_t>(2)) && Passed;

  Tree->GetEntry(0);
  Passed = EvaluateNear("CreateTree()", "first entry", "The simulation ID is read back", DataSet.m_SimulationID, 11.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second x strip ID is read back", DataSet.m_XStripIDs[1], 17.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The first y strip ID is read back", DataSet.m_YStripIDs[0], 63.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The third x strip energy is read back", DataSet.m_XStripEnergies[2], 661.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The second y strip energy is read back", DataSet.m_YStripEnergies[1], 381.5, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The number of interactions is read back", DataSet.m_ResultNumberOfInteractions, 2.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The flag for undetected interactions is read back", DataSet.m_ResultUndetectedInteractions, 1.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The first interaction is read back", DataSet.m_ResultInteractions[0], 1.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "first entry", "The last interaction is read back", DataSet.m_ResultInteractions[5], 1.0, 0.0) && Passed;

  Tree->GetEntry(1);
  Passed = EvaluateNear("CreateTree()", "second entry", "The simulation ID is read back", DataSet.m_SimulationID, 12.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The second x strip ID is read back", DataSet.m_XStripIDs[1], 2.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The second y strip energy is read back", DataSet.m_YStripEnergies[1], 5.5, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The flag for undetected interactions is read back", DataSet.m_ResultUndetectedInteractions, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("CreateTree()", "second entry", "The second interaction is read back", DataSet.m_ResultInteractions[1], 1.0, 0.0) && Passed;

  delete Tree;

  // The smallest data set: 1 + 1 + 1 + 1 + 1 + 2 + 1 = 8 branches
  MERStripPairingDataSet Smallest(1, 1);
  TTree* SmallestTree = Smallest.CreateTree("Smallest");
  Passed = EvaluateTrue("CreateTree()", "1 x 1 strips", "A tree is created", SmallestTree != nullptr) && Passed;
  if (SmallestTree != nullptr) {
    Passed = Evaluate("CreateTree()", "1 x 1 strips", "The tree has 8 branches", SmallestTree->GetNbranches(), 8) && Passed;
  }
  delete SmallestTree;

  // Without strips only the three single values are in the tree
  MERStripPairingDataSet Default;
  TTree* DefaultTree = Default.CreateTree("Default");
  Passed = EvaluateTrue("CreateTree()", "default constructed", "A tree is created", DefaultTree != nullptr) && Passed;
  if (DefaultTree != nullptr) {
    Passed = Evaluate("CreateTree()", "default constructed", "The tree has 3 branches", DefaultTree->GetNbranches(), 3) && Passed;
  }
  delete DefaultTree;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the reader
bool UTERStripPairingDataSet::TestCreateReader()
{
  bool Passed = true;

  MERStripPairingDataSet DataSet(3, 2);
  BeginCaptureOutput();
  TMVA::Reader* Reader = DataSet.CreateReader();
  EndCaptureOutput();
  Passed = EvaluateTrue("CreateReader()", "3 x 2 strips", "A reader is created", Reader != nullptr) && Passed;
  if (Reader == nullptr) return Passed;

  // The same 19 values as in the tree are the variables of the reader
  Passed = Evaluate("CreateReader()", "3 x 2 strips", "The reader has 19 variables", Reader->DataInfo().GetNVariables(), 19U) && Passed;
  MString Names[19] = { "SimulationID", "XStripID1", "XStripID2", "XStripID3", "YStripID1", "YStripID2",
                        "XStripEnergy1", "XStripEnergy2", "XStripEnergy3", "YStripEnergy1", "YStripEnergy2",
                        "ResultNumberOfInteractions", "ResultUndetectedInteractions",
                        "ResultInteraction1", "ResultInteraction2", "ResultInteraction3", "ResultInteraction4", "ResultInteraction5", "ResultInteraction6" };
  if (Reader->DataInfo().GetNVariables() == 19U) {
    for (unsigned int n = 0; n < 19; ++n) {
      Passed = Evaluate("CreateReader()", MString("variable ") + n, "The variables have the order and the names of the branches of the tree", MString(Reader->DataInfo().GetVariableInfo(n).GetExpression()), Names[n]) && Passed;
    }
  }
  delete Reader;

  // A second reader of the same data set
  BeginCaptureOutput();
  TMVA::Reader* Second = DataSet.CreateReader();
  EndCaptureOutput();
  Passed = EvaluateTrue("CreateReader()", "second reader", "A second reader is created", Second != nullptr) && Passed;
  if (Second != nullptr) {
    Passed = Evaluate("CreateReader()", "second reader", "The second reader has 19 variables", Second->DataInfo().GetNVariables(), 19U) && Passed;
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

  UTERStripPairingDataSet Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERStripPairingDataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
