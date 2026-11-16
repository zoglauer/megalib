/*
 * UTRESEIterator.cxx
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
#include <new>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MRESE.h"
#include "MRESEIterator.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the iterator over linked raw event sub elements MRESEIterator
class UTRESEIterator : public MUnitTest
{
public:
  //! Default constructor
  UTRESEIterator() : MUnitTest("UTRESEIterator") {}
  //! Default destructor
  virtual ~UTRESEIterator() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Create a chain of RESEs in which neighbors are linked in both directions - the caller owns the RESEs
  vector<MRESE*> CreateChain(unsigned int Length) const;
  //! Delete the RESEs of a chain
  void DeleteChain(vector<MRESE*>& Chain) const;

  //! Test that the default constructor sets all members
  bool TestDefaultConstruction();
  //! Test starting an iteration
  bool TestStart();
  //! Test the iteration over chains
  bool TestIteration();
  //! Test counting the RESEs
  bool TestGetNRESEs();
  //! Test the access by index
  bool TestGetRESEAt();
  //! Test starting again
  bool TestRestart();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRESEIterator::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestStart() && Passed;
  Passed = TestIteration() && Passed;
  Passed = TestGetNRESEs() && Passed;
  Passed = TestGetRESEAt() && Passed;
  Passed = TestRestart() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Create a chain of RESEs in which neighbors are linked in both directions - the caller owns the RESEs
vector<MRESE*> UTRESEIterator::CreateChain(unsigned int Length) const
{
  vector<MRESE*> Chain;
  for (unsigned int i = 0; i < Length; ++i) {
    Chain.push_back(new MRESE());
  }
  for (unsigned int i = 1; i < Length; ++i) {
    Chain[i-1]->AddLink(Chain[i]);
    Chain[i]->AddLink(Chain[i-1]);
  }

  return Chain;
}


////////////////////////////////////////////////////////////////////////////////


//! Delete the RESEs of a chain
void UTRESEIterator::DeleteChain(vector<MRESE*>& Chain) const
{
  for (unsigned int i = 0; i < Chain.size(); ++i) {
    delete Chain[i];
  }
  Chain.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the default constructor sets all members
bool UTRESEIterator::TestDefaultConstruction()
{
  bool Passed = true;

  // Construct on memory with a pattern: a member which is not set keeps the pattern
  alignas(MRESEIterator) unsigned char Block[sizeof(MRESEIterator)];
  FillMemory(Block, sizeof(Block), 0xAB);
  MRESEIterator* Iterator = new (Block) MRESEIterator();

  Passed = EvaluateTrue("MRESEIterator()", "default", "There is no previous RESE", Iterator->GetPrevious() == nullptr) && Passed;
  Passed = EvaluateTrue("MRESEIterator()", "default", "There is no current RESE", Iterator->GetCurrent() == nullptr) && Passed;
  // Defect REV-A-9: The constructors do not initialize the next RESE
  Passed = ExpectedToFail(EvaluateTrue("MRESEIterator()", "default", "There is no next RESE", Iterator->GetNext() == nullptr)) && Passed;

  // Defect REV-A-10: GetNRESEs and GetRESEAt start an iteration at a nullptr if the iterator was not started
  Passed = ExpectedToCrash(Evaluate("GetNRESEs()", "not started", "There are no RESEs", Iterator->GetNRESEs(), 0)) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("GetRESEAt()", "not started", "A nullptr is returned", Iterator->GetRESEAt(0) == nullptr)) && Passed;

  Iterator->~MRESEIterator();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test starting an iteration
bool UTRESEIterator::TestStart()
{
  bool Passed = true;

  vector<MRESE*> Chain = CreateChain(4);

  MRESEIterator Iterator;
  Passed = EvaluateTrue("Start()", "first RESE of a chain of four", "Starting succeeds", Iterator.Start(Chain[0])) && Passed;
  Passed = EvaluateTrue("Start()", "first RESE of a chain of four", "There is no previous RESE", Iterator.GetPrevious() == nullptr) && Passed;
  Passed = EvaluateTrue("Start()", "first RESE of a chain of four", "The current RESE is the start", Iterator.GetCurrent() == Chain[0]) && Passed;
  Passed = EvaluateTrue("Start()", "first RESE of a chain of four", "The next RESE is the second one", Iterator.GetNext() == Chain[1]) && Passed;

  MRESEIterator WithStart(Chain[3]);
  Passed = EvaluateTrue("MRESEIterator(MRESE*)", "last RESE of a chain of four", "There is no previous RESE", WithStart.GetPrevious() == nullptr) && Passed;
  Passed = EvaluateTrue("MRESEIterator(MRESE*)", "last RESE of a chain of four", "The current RESE is the start", WithStart.GetCurrent() == Chain[3]) && Passed;
  Passed = EvaluateTrue("MRESEIterator(MRESE*)", "last RESE of a chain of four", "The next RESE is the third one", WithStart.GetNext() == Chain[2]) && Passed;

  // The start must have exactly one link, otherwise the iteration ends at the start
  MRESEIterator Middle;
  Passed = EvaluateTrue("Start()", "RESE with two links", "Starting succeeds", Middle.Start(Chain[1])) && Passed;
  Passed = EvaluateTrue("Start()", "RESE with two links", "The current RESE is the start", Middle.GetCurrent() == Chain[1]) && Passed;
  Passed = EvaluateTrue("Start()", "RESE with two links", "There is no next RESE", Middle.GetNext() == nullptr) && Passed;

  MRESE Alone;
  MRESEIterator Single;
  Passed = EvaluateTrue("Start()", "RESE without links", "Starting succeeds", Single.Start(&Alone)) && Passed;
  Passed = EvaluateTrue("Start()", "RESE without links", "The current RESE is the start", Single.GetCurrent() == &Alone) && Passed;
  Passed = EvaluateTrue("Start()", "RESE without links", "There is no next RESE", Single.GetNext() == nullptr) && Passed;

  DeleteChain(Chain);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the iteration over chains
bool UTRESEIterator::TestIteration()
{
  bool Passed = true;

  vector<MRESE*> Chain = CreateChain(4);

  // Forward: each step returns the current RESE
  MRESEIterator Iterator(Chain[0]);
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 1", "The start is returned", Iterator.GetNextRESE() == Chain[0]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 1", "There is no previous RESE", Iterator.GetPrevious() == nullptr) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 1", "The current RESE is the start", Iterator.GetCurrent() == Chain[0]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 1", "The next RESE is the second one", Iterator.GetNext() == Chain[1]) && Passed;

  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 2", "The second RESE is returned", Iterator.GetNextRESE() == Chain[1]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 2", "The previous RESE is the start", Iterator.GetPrevious() == Chain[0]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 2", "The current RESE is the second one", Iterator.GetCurrent() == Chain[1]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 2", "The next RESE is the third one", Iterator.GetNext() == Chain[2]) && Passed;

  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 3", "The third RESE is returned", Iterator.GetNextRESE() == Chain[2]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 3", "The next RESE is the last one", Iterator.GetNext() == Chain[3]) && Passed;

  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 4", "The last RESE is returned", Iterator.GetNextRESE() == Chain[3]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 4", "The previous RESE is the third one", Iterator.GetPrevious() == Chain[2]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 4", "There is no next RESE", Iterator.GetNext() == nullptr) && Passed;

  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 5", "A nullptr ends the iteration", Iterator.GetNextRESE() == nullptr) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 5", "The previous RESE is the last one", Iterator.GetPrevious() == Chain[3]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 5", "There is no current RESE", Iterator.GetCurrent() == nullptr) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of four, step 6", "A nullptr is returned again", Iterator.GetNextRESE() == nullptr) && Passed;

  // Backward, with the loop of the class description:
  MRESEIterator Backward(Chain[3]);
  vector<MRESE*> Visited;
  MRESE* RESE = nullptr;
  while ((RESE = Backward.GetNextRESE()) != nullptr) {
    Visited.push_back(RESE);
  }
  Passed = EvaluateSize("GetNextRESE()", "chain of four from its end", "Four RESEs are visited", Visited.size(), 4) && Passed;
  if (Visited.size() == 4) {
    bool Reversed = false;
    if (Visited[0] == Chain[3] && Visited[1] == Chain[2] && Visited[2] == Chain[1] && Visited[3] == Chain[0]) {
      Reversed = true;
    }
    Passed = EvaluateTrue("GetNextRESE()", "chain of four from its end", "The RESEs are visited in reversed order", Reversed) && Passed;
  }

  // The use in the response classes: start, one step, then the current and the next RESE
  MRESEIterator Usage;
  Usage.Start(Chain[0]);
  Usage.GetNextRESE();
  Passed = EvaluateTrue("GetCurrent()", "after the start and one step", "The current RESE is the start", Usage.GetCurrent() == Chain[0]) && Passed;
  Passed = EvaluateTrue("GetNext()", "after the start and one step", "The next RESE is the second one", Usage.GetNext() == Chain[1]) && Passed;

  // A chain of two:
  vector<MRESE*> Pair = CreateChain(2);
  MRESEIterator PairIterator(Pair[0]);
  Passed = EvaluateTrue("GetNextRESE()", "chain of two, step 1", "The first RESE is returned", PairIterator.GetNextRESE() == Pair[0]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of two, step 2", "The second RESE is returned", PairIterator.GetNextRESE() == Pair[1]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "chain of two, step 3", "A nullptr ends the iteration", PairIterator.GetNextRESE() == nullptr) && Passed;

  // A RESE without links:
  MRESE Alone;
  MRESEIterator Single(&Alone);
  Passed = EvaluateTrue("GetNextRESE()", "RESE without links, step 1", "The RESE is returned", Single.GetNextRESE() == &Alone) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "RESE without links, step 2", "A nullptr ends the iteration", Single.GetNextRESE() == nullptr) && Passed;

  // A start with two links ends at the start:
  MRESEIterator Middle(Chain[1]);
  Passed = EvaluateTrue("GetNextRESE()", "start with two links, step 1", "The start is returned", Middle.GetNextRESE() == Chain[1]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "start with two links, step 2", "A nullptr ends the iteration", Middle.GetNextRESE() == nullptr) && Passed;

  DeleteChain(Chain);
  DeleteChain(Pair);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test counting the RESEs
bool UTRESEIterator::TestGetNRESEs()
{
  bool Passed = true;

  vector<MRESE*> Chain = CreateChain(4);

  MRESEIterator Iterator(Chain[0]);
  Passed = Evaluate("GetNRESEs()", "chain of four", "There are four RESEs", Iterator.GetNRESEs(), 4) && Passed;

  // Counting does not move the iterator:
  Iterator.GetNextRESE();
  Iterator.GetNextRESE();
  Passed = Evaluate("GetNRESEs()", "chain of four, after two steps", "There are still four RESEs", Iterator.GetNRESEs(), 4) && Passed;
  Passed = EvaluateTrue("GetNRESEs()", "chain of four, after two steps", "The previous RESE is unchanged", Iterator.GetPrevious() == Chain[0]) && Passed;
  Passed = EvaluateTrue("GetNRESEs()", "chain of four, after two steps", "The current RESE is unchanged", Iterator.GetCurrent() == Chain[1]) && Passed;
  Passed = EvaluateTrue("GetNRESEs()", "chain of four, after two steps", "The next RESE is unchanged", Iterator.GetNext() == Chain[2]) && Passed;
  Passed = EvaluateTrue("GetNRESEs()", "chain of four, after two steps", "The iteration continues with the third RESE", Iterator.GetNextRESE() == Chain[2]) && Passed;

  MRESEIterator FromEnd(Chain[3]);
  Passed = Evaluate("GetNRESEs()", "chain of four from its end", "There are four RESEs", FromEnd.GetNRESEs(), 4) && Passed;

  MRESEIterator Middle(Chain[2]);
  Passed = Evaluate("GetNRESEs()", "start with two links", "Only the start is counted", Middle.GetNRESEs(), 1) && Passed;

  MRESE Alone;
  MRESEIterator Single(&Alone);
  Passed = Evaluate("GetNRESEs()", "RESE without links", "There is one RESE", Single.GetNRESEs(), 1) && Passed;

  vector<MRESE*> Pair = CreateChain(2);
  MRESEIterator PairIterator(Pair[1]);
  Passed = Evaluate("GetNRESEs()", "chain of two", "There are two RESEs", PairIterator.GetNRESEs(), 2) && Passed;

  DeleteChain(Chain);
  DeleteChain(Pair);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the access by index
bool UTRESEIterator::TestGetRESEAt()
{
  bool Passed = true;

  vector<MRESE*> Chain = CreateChain(4);

  MRESEIterator Iterator(Chain[0]);
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 0", "The start is returned", Iterator.GetRESEAt(0) == Chain[0]) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 0", "The current RESE is the returned one", Iterator.GetCurrent() == Chain[0]) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 3", "The last RESE is returned", Iterator.GetRESEAt(3) == Chain[3]) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 3", "The current RESE is the returned one", Iterator.GetCurrent() == Chain[3]) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 3", "The previous RESE is the third one", Iterator.GetPrevious() == Chain[2]) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 3", "There is no next RESE", Iterator.GetNext() == nullptr) && Passed;

  // An earlier index after a later one:
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 1", "The second RESE is returned", Iterator.GetRESEAt(1) == Chain[1]) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 1", "The previous RESE is the start", Iterator.GetPrevious() == Chain[0]) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 1", "The next RESE is the third one", Iterator.GetNext() == Chain[2]) && Passed;

  // The iteration continues behind the index:
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 2", "The third RESE is returned", Iterator.GetRESEAt(2) == Chain[2]) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "after index 2", "The iteration continues with the last RESE", Iterator.GetNextRESE() == Chain[3]) && Passed;

  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 4", "A nullptr is returned behind the end", Iterator.GetRESEAt(4) == nullptr) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 4", "There is no current RESE", Iterator.GetCurrent() == nullptr) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "chain of four, index 100", "A nullptr is returned far behind the end", Iterator.GetRESEAt(100) == nullptr) && Passed;

  // From the other end the index counts backward:
  MRESEIterator FromEnd(Chain[3]);
  Passed = EvaluateTrue("GetRESEAt()", "chain of four from its end, index 1", "The third RESE is returned", FromEnd.GetRESEAt(1) == Chain[2]) && Passed;

  MRESE Alone;
  MRESEIterator Single(&Alone);
  Passed = EvaluateTrue("GetRESEAt()", "RESE without links, index 0", "The RESE is returned", Single.GetRESEAt(0) == &Alone) && Passed;
  Passed = EvaluateTrue("GetRESEAt()", "RESE without links, index 1", "A nullptr is returned", Single.GetRESEAt(1) == nullptr) && Passed;

  DeleteChain(Chain);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test starting again
bool UTRESEIterator::TestRestart()
{
  bool Passed = true;

  vector<MRESE*> Chain = CreateChain(3);
  vector<MRESE*> Other = CreateChain(2);

  // Start again in the middle of an iteration:
  MRESEIterator Iterator(Chain[0]);
  Iterator.GetNextRESE();
  Iterator.GetNextRESE();
  Passed = EvaluateTrue("Start()", "same chain, after two steps", "Starting succeeds", Iterator.Start(Chain[0])) && Passed;
  Passed = EvaluateTrue("Start()", "same chain, after two steps", "There is no previous RESE", Iterator.GetPrevious() == nullptr) && Passed;
  Passed = EvaluateTrue("Start()", "same chain, after two steps", "The first step returns the start", Iterator.GetNextRESE() == Chain[0]) && Passed;
  Passed = EvaluateTrue("Start()", "same chain, after two steps", "The second step returns the second RESE", Iterator.GetNextRESE() == Chain[1]) && Passed;

  // Start again after the end:
  Iterator.GetNextRESE();
  Iterator.GetNextRESE();
  Passed = EvaluateTrue("GetNextRESE()", "end of the chain", "The iteration has ended", Iterator.GetCurrent() == nullptr) && Passed;
  Iterator.Start(Chain[2]);
  Passed = EvaluateTrue("Start()", "other end, after the end", "The first step returns the new start", Iterator.GetNextRESE() == Chain[2]) && Passed;
  Passed = EvaluateTrue("Start()", "other end, after the end", "The second step returns the middle RESE", Iterator.GetNextRESE() == Chain[1]) && Passed;

  // Start on another chain:
  Iterator.Start(Other[0]);
  Passed = Evaluate("Start()", "another chain of two", "There are two RESEs", Iterator.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("Start()", "another chain of two", "The first step returns its start", Iterator.GetNextRESE() == Other[0]) && Passed;
  Passed = EvaluateTrue("Start()", "another chain of two", "The second step returns its end", Iterator.GetNextRESE() == Other[1]) && Passed;
  Passed = EvaluateTrue("Start()", "another chain of two", "The third step ends the iteration", Iterator.GetNextRESE() == nullptr) && Passed;

  DeleteChain(Chain);
  DeleteChain(Other);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTRESEIterator Test;
  return Test.Run() == true ? 0 : 1;
}


// UTRESEIterator.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
