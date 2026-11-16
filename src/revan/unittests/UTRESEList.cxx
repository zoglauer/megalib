/*
 * UTRESEList.cxx
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
#include <algorithm>
#include <vector>
using namespace std;

// ROOT libs:
#include "TRandom.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MRESEList.h"
#include "MUnitTest.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the list of raw event sub elements MRESEList
class UTRESEList : public MUnitTest
{
public:
  //! Default constructor
  UTRESEList() : MUnitTest("UTRESEList") {}
  //! Default destructor
  virtual ~UTRESEList() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return true if setting a RESE behind the end of the list keeps the list
  bool SetRESEBehindEnd();
  //! Return true if the text of a list with an empty slot can be created
  bool ToStringWithEmptySlot();

  //! Test an empty list
  bool TestDefaultConstruction();
  //! Test adding, finding, and replacing RESEs
  bool TestAddAndGet();
  //! Test the search through all levels
  bool TestGetRESEContainingRESE();
  //! Test removing RESEs and compressing the list
  bool TestRemoveAndCompress();
  //! Test deleting all RESEs and the ownership
  bool TestDeleteAll();
  //! Test the random shuffling
  bool TestShuffle();
  //! Test the duplication with its links
  bool TestDuplicate();
  //! Test the text output
  bool TestToString();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRESEList::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestAddAndGet() && Passed;
  Passed = TestGetRESEContainingRESE() && Passed;
  Passed = TestRemoveAndCompress() && Passed;
  Passed = TestDeleteAll() && Passed;
  Passed = TestShuffle() && Passed;
  Passed = TestDuplicate() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if setting a RESE behind the end of the list keeps the list
bool UTRESEList::SetRESEBehindEnd()
{
  MRESEList List;
  MRESE First;
  MRESE Other;
  List.AddRESE(&First);
  List.SetRESEAt(1, &Other);

  if (List.GetNRESEs() == 1 && List.GetRESEAt(0) == &First) {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the text of a list with an empty slot can be created
bool UTRESEList::ToStringWithEmptySlot()
{
  MRESEList List;
  MRESE First;
  MRESE Second;
  List.AddRESE(&First);
  List.AddRESE(&Second);
  List.RemoveRESE(&First);

  if (List.ToString() == MString("Contents of this RESEList:\n") + Second.ToString(true, 1)) {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an empty list
bool UTRESEList::TestDefaultConstruction()
{
  bool Passed = true;

  MRESEList List;
  Passed = Evaluate("MRESEList()", "default", "A new list is empty", List.GetNRESEs(), 0) && Passed;
  Passed = EvaluateFalse("ContainsRESE(int)", "empty list", "No ID is contained", List.ContainsRESE(1)) && Passed;
  Passed = EvaluateTrue("GetRESEContainingRESE()", "empty list", "A nullptr is returned", List.GetRESEContainingRESE(1) == nullptr) && Passed;

  DisableDefaultStreams();
  MRESE* OutOfBounds = List.GetRESEAt(0);
  MRESE* UnknownID = List.GetRESE(1);
  MRESE* Removed = List.RemoveRESEAt(0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetRESEAt()", "empty list", "A nullptr is returned", OutOfBounds == nullptr) && Passed;
  Passed = EvaluateTrue("GetRESE()", "empty list", "A nullptr is returned", UnknownID == nullptr) && Passed;
  Passed = EvaluateTrue("RemoveRESEAt()", "empty list", "A nullptr is returned", Removed == nullptr) && Passed;

  // These do nothing on an empty list:
  List.RemoveAllRESEs();
  List.Compress();
  List.CompressRESEs();
  List.DeleteAll();
  List.Shuffle();
  Passed = Evaluate("MRESEList()", "after removing, compressing, deleting, and shuffling", "The list is still empty", List.GetNRESEs(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding, finding, and replacing RESEs
bool UTRESEList::TestAddAndGet()
{
  bool Passed = true;

  MRESEList List;
  MRESE First;
  MRESE Second;
  MRESE Third;
  MRESE NotAdded;

  List.AddRESE(&First);
  Passed = Evaluate("AddRESE()", "first RESE", "There is one RESE", List.GetNRESEs(), 1) && Passed;
  List.AddRESE(&Second);
  Passed = Evaluate("AddRESE()", "second RESE", "There are two RESEs", List.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "index 0", "The first added RESE is first", List.GetRESEAt(0) == &First) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "index 1", "The second added RESE is second", List.GetRESEAt(1) == &Second) && Passed;

  List.AddRESEFirst(&Third);
  Passed = Evaluate("AddRESEFirst()", "third RESE", "There are three RESEs", List.GetNRESEs(), 3) && Passed;
  Passed = EvaluateTrue("AddRESEFirst()", "third RESE", "The RESE is the first one", List.GetRESEAt(0) == &Third) && Passed;
  Passed = EvaluateTrue("AddRESEFirst()", "third RESE", "The former first RESE is the second one", List.GetRESEAt(1) == &First) && Passed;
  Passed = EvaluateTrue("AddRESEFirst()", "third RESE", "The former second RESE is the last one", List.GetRESEAt(2) == &Second) && Passed;

  // The same RESE can be in the list several times:
  List.AddRESE(&First);
  Passed = Evaluate("AddRESE()", "a RESE which is in the list", "The RESE is added again", List.GetNRESEs(), 4) && Passed;
  List.RemoveRESEAt(3);
  List.Compress();

  Passed = EvaluateTrue("GetRESE()", "ID of the second RESE", "The RESE with the ID is returned", List.GetRESE(Second.GetID()) == &Second) && Passed;
  Passed = EvaluateTrue("GetRESE()", "ID of the RESE at the front", "The RESE with the ID is returned", List.GetRESE(Third.GetID()) == &Third) && Passed;
  Passed = EvaluateTrue("ContainsRESE(MRESE*)", "added RESE", "An added RESE is contained", List.ContainsRESE(&First)) && Passed;
  Passed = EvaluateFalse("ContainsRESE(MRESE*)", "other RESE", "Another RESE is not contained", List.ContainsRESE(&NotAdded)) && Passed;
  Passed = EvaluateTrue("ContainsRESE(int)", "ID of an added RESE", "The ID of an added RESE is contained", List.ContainsRESE(Second.GetID())) && Passed;
  Passed = EvaluateFalse("ContainsRESE(int)", "ID of another RESE", "The ID of another RESE is not contained", List.ContainsRESE(NotAdded.GetID())) && Passed;

  DisableDefaultStreams();
  MRESE* OutOfBounds = List.GetRESEAt(3);
  MRESE* UnknownID = List.GetRESE(NotAdded.GetID());
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetRESEAt()", "index behind the last RESE", "A nullptr is returned", OutOfBounds == nullptr) && Passed;
  Passed = EvaluateTrue("GetRESE()", "unknown ID", "A nullptr is returned", UnknownID == nullptr) && Passed;

  // Defect REV-A-3: GetRESEAt only checks the upper limit, a negative index reads in front of the list
  Passed = ExpectedToCrash(EvaluateTrue("GetRESEAt()", "index -1", "A nullptr is returned", List.GetRESEAt(-1) == nullptr)) && Passed;

  // Replace a RESE:
  List.SetRESEAt(1, &NotAdded);
  Passed = Evaluate("SetRESEAt()", "index 1", "The number of RESEs is unchanged", List.GetNRESEs(), 3) && Passed;
  Passed = EvaluateTrue("SetRESEAt()", "index 1", "The new RESE is at the index", List.GetRESEAt(1) == &NotAdded) && Passed;
  Passed = EvaluateFalse("SetRESEAt()", "index 1", "The replaced RESE is not contained any more", List.ContainsRESE(&First)) && Passed;
  Passed = EvaluateTrue("SetRESEAt()", "index 1", "The RESE in front is unchanged", List.GetRESEAt(0) == &Third) && Passed;
  Passed = EvaluateTrue("SetRESEAt()", "index 1", "The RESE behind is unchanged", List.GetRESEAt(2) == &Second) && Passed;

  // Defect REV-A-3: SetRESEAt checks the index only with a compiled-out assertion and writes behind the end of the list
  Passed = ExpectedToCrash(EvaluateTrue("SetRESEAt()", "index behind the last RESE", "The list is unchanged", SetRESEBehindEnd())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search through all levels
bool UTRESEList::TestGetRESEContainingRESE()
{
  bool Passed = true;

  // Leaf is in Middle, Middle is in Top, Top and Single are in the list
  MRESE Top;
  MRESE Middle;
  MRESE Leaf;
  MRESE Single;
  MRESE NotAdded;
  Middle.AddRESE(&Leaf);
  Top.AddRESE(&Middle);

  MRESEList List;
  List.AddRESE(&Single);
  List.AddRESE(&Top);

  Passed = EvaluateTrue("GetRESEContainingRESE()", "ID of a RESE of the list", "The RESE itself is returned", List.GetRESEContainingRESE(Single.GetID()) == &Single) && Passed;
  Passed = EvaluateTrue("GetRESEContainingRESE()", "ID of a RESE one level down", "The RESE of the list which contains it is returned", List.GetRESEContainingRESE(Middle.GetID()) == &Top) && Passed;
  Passed = EvaluateTrue("GetRESEContainingRESE()", "ID of a RESE two levels down", "The RESE of the list which contains it is returned", List.GetRESEContainingRESE(Leaf.GetID()) == &Top) && Passed;
  Passed = EvaluateTrue("GetRESEContainingRESE()", "unknown ID", "A nullptr is returned", List.GetRESEContainingRESE(NotAdded.GetID()) == nullptr) && Passed;

  // An empty slot is skipped:
  List.RemoveRESE(&Single);
  Passed = EvaluateTrue("GetRESEContainingRESE()", "empty slot in front", "The RESE behind the empty slot is found", List.GetRESEContainingRESE(Leaf.GetID()) == &Top) && Passed;
  Passed = EvaluateTrue("GetRESEContainingRESE()", "ID of the removed RESE", "A nullptr is returned", List.GetRESEContainingRESE(Single.GetID()) == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test removing RESEs and compressing the list
bool UTRESEList::TestRemoveAndCompress()
{
  bool Passed = true;

  MRESEList List;
  MRESE A;
  MRESE B;
  MRESE C;
  MRESE D;
  MRESE NotAdded;
  List.AddRESE(&A);
  List.AddRESE(&B);
  List.AddRESE(&C);
  List.AddRESE(&D);

  // Removing leaves an empty slot:
  Passed = EvaluateTrue("RemoveRESE()", "second of four", "The removed RESE is returned", List.RemoveRESE(&B) == &B) && Passed;
  Passed = Evaluate("RemoveRESE()", "second of four", "The number of slots is unchanged", List.GetNRESEs(), 4) && Passed;
  Passed = EvaluateTrue("RemoveRESE()", "second of four", "The slot is empty", List.GetRESEAt(1) == nullptr) && Passed;
  Passed = EvaluateFalse("RemoveRESE()", "second of four", "The RESE is not contained any more", List.ContainsRESE(&B)) && Passed;
  Passed = EvaluateFalse("RemoveRESE()", "second of four", "The ID is not contained any more", List.ContainsRESE(B.GetID())) && Passed;

  Passed = EvaluateTrue("RemoveRESEAt()", "index 3", "The removed RESE is returned", List.RemoveRESEAt(3) == &D) && Passed;
  Passed = Evaluate("RemoveRESEAt()", "index 3", "The number of slots is unchanged", List.GetNRESEs(), 4) && Passed;
  Passed = EvaluateTrue("RemoveRESEAt()", "index 3", "The slot is empty", List.GetRESEAt(3) == nullptr) && Passed;
  Passed = EvaluateTrue("RemoveRESEAt()", "index of an empty slot", "A nullptr is returned", List.RemoveRESEAt(1) == nullptr) && Passed;

  DisableDefaultStreams();
  MRESE* NotContained = List.RemoveRESE(&NotAdded);
  MRESE* OutOfBounds = List.RemoveRESEAt(4);
  EnableDefaultStreams();
  Passed = EvaluateTrue("RemoveRESE()", "RESE which is not in the list", "A nullptr is returned", NotContained == nullptr) && Passed;
  Passed = EvaluateTrue("RemoveRESEAt()", "index behind the last slot", "A nullptr is returned", OutOfBounds == nullptr) && Passed;
  Passed = Evaluate("RemoveRESE()", "two failed removals", "The number of slots is unchanged", List.GetNRESEs(), 4) && Passed;

  // Defect REV-A-3: RemoveRESEAt only checks the upper limit, a negative index writes in front of the list
  Passed = ExpectedToCrash(EvaluateTrue("RemoveRESEAt()", "index -1", "A nullptr is returned", List.RemoveRESEAt(-1) == nullptr)) && Passed;

  // Compressing keeps the order:
  List.Compress();
  Passed = Evaluate("Compress()", "two of four slots empty", "Two RESEs are left", List.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("Compress()", "two of four slots empty", "The first RESE stays first", List.GetRESEAt(0) == &A) && Passed;
  Passed = EvaluateTrue("Compress()", "two of four slots empty", "The third RESE is now second", List.GetRESEAt(1) == &C) && Passed;
  List.Compress();
  Passed = Evaluate("Compress()", "no empty slot", "Compressing again changes nothing", List.GetNRESEs(), 2) && Passed;

  List.RemoveRESEAt(0);
  List.CompressRESEs();
  Passed = Evaluate("CompressRESEs()", "first slot empty", "One RESE is left", List.GetNRESEs(), 1) && Passed;
  Passed = EvaluateTrue("CompressRESEs()", "first slot empty", "The remaining RESE is first", List.GetRESEAt(0) == &C) && Passed;

  // Remove all:
  List.AddRESE(&A);
  List.AddRESE(&B);
  List.RemoveAllRESEs();
  Passed = Evaluate("RemoveAllRESEs()", "three RESEs", "The number of slots is unchanged", List.GetNRESEs(), 3) && Passed;
  bool AllEmpty = true;
  for (int i = 0; i < List.GetNRESEs(); ++i) {
    if (List.GetRESEAt(i) != nullptr) {
      AllEmpty = false;
    }
  }
  Passed = EvaluateTrue("RemoveAllRESEs()", "three RESEs", "All slots are empty", AllEmpty) && Passed;
  List.Compress();
  Passed = Evaluate("Compress()", "only empty slots", "The list is empty", List.GetNRESEs(), 0) && Passed;

  // The list can be filled again:
  List.AddRESE(&D);
  Passed = Evaluate("AddRESE()", "after removing everything", "There is one RESE", List.GetNRESEs(), 1) && Passed;
  Passed = EvaluateTrue("AddRESE()", "after removing everything", "The RESE is first", List.GetRESEAt(0) == &D) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test deleting all RESEs and the ownership
bool UTRESEList::TestDeleteAll()
{
  bool Passed = true;

  const int Before = UTRevanCountedRESE::m_NumberOfLivingObjects;

  // Leaf is in Middle, Middle and Single are in the list, with an empty slot between them
  MRESEList List;
  UTRevanCountedRESE* Middle = new UTRevanCountedRESE();
  UTRevanCountedRESE* Leaf = new UTRevanCountedRESE();
  UTRevanCountedRESE* Single = new UTRevanCountedRESE();
  MRESE Removed;
  Middle->AddRESE(Leaf);
  List.AddRESE(Middle);
  List.AddRESE(&Removed);
  List.AddRESE(Single);
  List.RemoveRESE(&Removed);

  List.DeleteAll();
  Passed = Evaluate("DeleteAll()", "two RESEs, one with a RESE of its own", "All three RESEs are deleted", UTRevanCountedRESE::m_NumberOfLivingObjects - Before, 0) && Passed;
  Passed = Evaluate("DeleteAll()", "two RESEs, one with a RESE of its own", "The number of slots is unchanged", List.GetNRESEs(), 3) && Passed;
  Passed = EvaluateTrue("DeleteAll()", "two RESEs, one with a RESE of its own", "The first slot is empty", List.GetRESEAt(0) == nullptr) && Passed;
  Passed = EvaluateTrue("DeleteAll()", "two RESEs, one with a RESE of its own", "The last slot is empty", List.GetRESEAt(2) == nullptr) && Passed;
  List.DeleteAll();
  Passed = Evaluate("DeleteAll()", "only empty slots", "Deleting again changes nothing", List.GetNRESEs(), 3) && Passed;

  // The destructor does not delete the RESEs:
  UTRevanCountedRESE* Survivor = new UTRevanCountedRESE();
  MRESEList* Temporary = new MRESEList();
  Temporary->AddRESE(Survivor);
  delete Temporary;
  Passed = Evaluate("~MRESEList()", "list with one RESE", "The RESE is not deleted with the list", UTRevanCountedRESE::m_NumberOfLivingObjects - Before, 1) && Passed;
  delete Survivor;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the random shuffling
bool UTRESEList::TestShuffle()
{
  bool Passed = true;

  const unsigned int Size = 7;
  const unsigned int Seed = 20261115;

  // The documented procedure: twice as many swaps of two random slots as there are RESEs
  // The reference does them with indices, drawing the source before the target
  vector<unsigned int> Expected;
  for (unsigned int i = 0; i < Size; ++i) {
    Expected.push_back(i);
  }
  gRandom->SetSeed(Seed);
  for (unsigned int i = 0; i < 2*Size; ++i) {
    unsigned int From = gRandom->Integer(Size);
    unsigned int To = gRandom->Integer(Size);
    swap(Expected[To], Expected[From]);
  }

  MRESEList List;
  vector<MRESE*> RESEs;
  for (unsigned int i = 0; i < Size; ++i) {
    RESEs.push_back(new MRESE());
    List.AddRESE(RESEs[i]);
  }

  gRandom->SetSeed(Seed);
  List.Shuffle();

  Passed = Evaluate("Shuffle()", "seven RESEs", "The number of RESEs is unchanged", List.GetNRESEs(), static_cast<int>(Size)) && Passed;
  bool IsPermutation = true;
  bool IsExpectedOrder = true;
  bool IsOriginalOrder = true;
  for (unsigned int i = 0; i < Size; ++i) {
    if (List.ContainsRESE(RESEs[i]) == false) {
      IsPermutation = false;
    }
    if (List.GetRESEAt(i) != RESEs[Expected[i]]) {
      IsExpectedOrder = false;
    }
    if (List.GetRESEAt(i) != RESEs[i]) {
      IsOriginalOrder = false;
    }
  }
  Passed = EvaluateTrue("Shuffle()", "seven RESEs", "All RESEs are still in the list", IsPermutation) && Passed;
  Passed = EvaluateTrue("Shuffle()", "seven RESEs, seed 20261115", "The order is the one of fourteen random swaps", IsExpectedOrder) && Passed;
  Passed = EvaluateFalse("Shuffle()", "seven RESEs, seed 20261115", "The order has changed", IsOriginalOrder) && Passed;

  for (unsigned int i = 0; i < Size; ++i) {
    delete RESEs[i];
  }

  MRESEList One;
  MRESE Only;
  One.AddRESE(&Only);
  One.Shuffle();
  Passed = EvaluateTrue("Shuffle()", "one RESE", "The only RESE stays", One.GetRESEAt(0) == &Only) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the duplication with its links
bool UTRESEList::TestDuplicate()
{
  bool Passed = true;

  // A chain A - B - C linked in both directions, and an empty slot between B and C
  MRESE A;
  A.SetEnergy(10.0);
  MRESE B;
  B.SetEnergy(20.0);
  MRESE C;
  C.SetEnergy(30.0);
  MRESE Removed;
  A.AddLink(&B);
  B.AddLink(&A);
  B.AddLink(&C);
  C.AddLink(&B);

  MRESEList List;
  List.AddRESE(&A);
  List.AddRESE(&B);
  List.AddRESE(&Removed);
  List.AddRESE(&C);
  List.RemoveRESE(&Removed);

  MRESEList* Copy = List.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "chain of three with an empty slot", "A new list is returned", Copy != nullptr && Copy != &List) && Passed;
  if (Copy == nullptr) {
    return false;
  }

  Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The copy has four slots", Copy->GetNRESEs(), 4) && Passed;
  if (Copy->GetNRESEs() == 4) {
    MRESE* CopyA = Copy->GetRESEAt(0);
    MRESE* CopyB = Copy->GetRESEAt(1);
    MRESE* CopyC = Copy->GetRESEAt(3);
    Passed = EvaluateTrue("Duplicate()", "chain of three with an empty slot", "The empty slot is kept", Copy->GetRESEAt(2) == nullptr) && Passed;
    bool AllNew = false;
    if (CopyA != nullptr && CopyB != nullptr && CopyC != nullptr && CopyA != &A && CopyB != &B && CopyC != &C) {
      AllNew = true;
    }
    Passed = EvaluateTrue("Duplicate()", "chain of three with an empty slot", "The RESEs are new objects", AllNew) && Passed;
    if (AllNew == true) {
      Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The first RESE keeps its ID", CopyA->GetID(), A.GetID()) && Passed;
      Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The second RESE keeps its ID", CopyB->GetID(), B.GetID()) && Passed;
      Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The third RESE keeps its ID", CopyC->GetID(), C.GetID()) && Passed;
      Passed = EvaluateNear("Duplicate()", "chain of three with an empty slot", "The first RESE keeps its energy", CopyA->GetEnergy(), 10.0, 1e-15) && Passed;
      Passed = EvaluateNear("Duplicate()", "chain of three with an empty slot", "The third RESE keeps its energy", CopyC->GetEnergy(), 30.0, 1e-15) && Passed;

      // The links point to the copies, in the same order:
      Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The first RESE has one link", CopyA->GetNLinks(), 1) && Passed;
      Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The second RESE has two links", CopyB->GetNLinks(), 2) && Passed;
      Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The third RESE has one link", CopyC->GetNLinks(), 1) && Passed;
      Passed = EvaluateTrue("Duplicate()", "chain of three with an empty slot", "The first RESE is linked to the copy of the second", CopyA->GetLinkAt(0) == CopyB) && Passed;
      Passed = EvaluateTrue("Duplicate()", "chain of three with an empty slot", "The first link of the second RESE is the copy of the first", CopyB->GetLinkAt(0) == CopyA) && Passed;
      Passed = EvaluateTrue("Duplicate()", "chain of three with an empty slot", "The second link of the second RESE is the copy of the third", CopyB->GetLinkAt(1) == CopyC) && Passed;
      Passed = EvaluateTrue("Duplicate()", "chain of three with an empty slot", "The third RESE is linked to the copy of the second", CopyC->GetLinkAt(0) == CopyB) && Passed;

      // The copy is independent:
      CopyA->SetEnergy(99.0);
      Passed = EvaluateNear("Duplicate()", "copy changed", "The energy of the original is unchanged", A.GetEnergy(), 10.0, 1e-15) && Passed;
    }
  }
  Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The original keeps its four slots", List.GetNRESEs(), 4) && Passed;
  Passed = Evaluate("Duplicate()", "chain of three with an empty slot", "The original keeps its links", B.GetNLinks(), 2) && Passed;

  Copy->DeleteAll();
  delete Copy;

  // The copy constructor does the same:
  MRESEList Direct(&List);
  Passed = Evaluate("MRESEList(MRESEList*)", "chain of three with an empty slot", "The copy has four slots", Direct.GetNRESEs(), 4) && Passed;
  Passed = EvaluateTrue("MRESEList(MRESEList*)", "chain of three with an empty slot", "The first RESE is a new object", Direct.GetRESEAt(0) != &A && Direct.GetRESEAt(0) != nullptr) && Passed;
  Direct.DeleteAll();

  // An empty list:
  MRESEList Empty;
  MRESEList* EmptyCopy = Empty.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "empty list", "A new list is returned", EmptyCopy != nullptr && EmptyCopy != &Empty) && Passed;
  if (EmptyCopy != nullptr) {
    Passed = Evaluate("Duplicate()", "empty list", "The copy is empty", EmptyCopy->GetNRESEs(), 0) && Passed;
  }
  delete EmptyCopy;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTRESEList::TestToString()
{
  bool Passed = true;

  const MString Title = "Contents of this RESEList:\n";

  MRESEList List;
  Passed = Evaluate("ToString()", "empty list", "The text is the title", List.ToString(), Title) && Passed;
  // Three blanks per level:
  Passed = Evaluate("ToString()", "empty list, level 2", "The title is indented by six blanks", List.ToString(true, 2), MString("      ") + Title) && Passed;

  // The RESEs follow one level deeper:
  MREHit First;
  First.SetPosition(1.0, 2.0, 3.0);
  First.SetEnergy(100.0);
  First.SetDetector(MDDetector::c_Strip2D);
  MREHit Second;
  Second.SetPosition(-4.0, 5.0, -6.0);
  Second.SetEnergy(250.0);
  Second.SetDetector(MDDetector::c_Calorimeter);
  First.AddLink(&Second);
  List.AddRESE(&First);
  List.AddRESE(&Second);

  Passed = Evaluate("ToString()", "two hits", "The title is followed by the texts of the hits on level 1", List.ToString(), Title + First.ToString(true, 1) + Second.ToString(true, 1)) && Passed;
  Passed = Evaluate("ToString()", "two hits, level 1", "The hits are on level 2", List.ToString(true, 1), MString("   ") + Title + First.ToString(true, 2) + Second.ToString(true, 2)) && Passed;
  Passed = Evaluate("ToString()", "two hits, without links", "The links are left out", List.ToString(false, 0), Title + First.ToString(false, 1) + Second.ToString(false, 1)) && Passed;
  Passed = EvaluateFalse("ToString()", "two hits", "The text with links differs from the one without", List.ToString(true, 0) == List.ToString(false, 0)) && Passed;

  // Defect REV-A-1: ToString dereferences the empty slot which RemoveRESE leaves behind
  Passed = ExpectedToCrash(EvaluateTrue("ToString()", "empty slot", "The text is the title and the remaining RESE", ToStringWithEmptySlot())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTRESEList Test;
  return Test.Run() == true ? 0 : 1;
}


// UTRESEList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
