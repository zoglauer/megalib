/*
 * UTTrackingAction.cc
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
#include "MCTrackingAction.hh"
#include "MCTrackInformation.hh"

// MEGAlib:
#include "MUnitTest.h"

// Geant4:
#include "G4BosonConstructor.hh"
#include "G4DynamicParticle.hh"
#include "G4Gamma.hh"
#include "G4LeptonConstructor.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4UserTrackingAction.hh"

// Standard lib:
#include <cstring>
#include <memory>
#include <new>
#include <set>
#include <vector>
using namespace std;


//! Unit test class for MCTrackingAction
class UTTrackingAction : public MUnitTest
{
public:
  //! Default constructor
  UTTrackingAction() : MUnitTest("UTTrackingAction") {}
  //! Default destructor
  virtual ~UTTrackingAction() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the base class and the lifecycle
  bool TestConstruction();
  //! Test that the members do not depend on the memory the action is created in
  bool TestNoUninitializedMembers();
  //! Test the assignment of ids to primary tracks
  bool TestPrimaryIds();
  //! Test that tracks which already have information are not touched
  bool TestExistingInformation();
  //! Test resetting the number of generated particles
  bool TestSetNGeneratedParticles();
  //! Test many primaries in one event
  bool TestManyPrimaries();

  //! Create a new track without user information
  G4Track* CreateTrack();
  //! Return the information of a track, or nullptr
  MCTrackInformation* GetInformation(const G4Track* Track);
  //! Construct the particles
  void InitializeGeant4();
};


////////////////////////////////////////////////////////////////////////////////


bool UTTrackingAction::Run()
{
  bool Passed = true;

  InitializeGeant4();

  Passed = TestConstruction() && Passed;
  Passed = TestNoUninitializedMembers() && Passed;
  Passed = TestPrimaryIds() && Passed;
  Passed = TestExistingInformation() && Passed;
  Passed = TestSetNGeneratedParticles() && Passed;
  Passed = TestManyPrimaries() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTTrackingAction::InitializeGeant4()
{
  G4BosonConstructor().ConstructParticle();
  G4LeptonConstructor().ConstructParticle();
  G4ParticleTable::GetParticleTable()->SetReadiness();
}


////////////////////////////////////////////////////////////////////////////////


G4Track* UTTrackingAction::CreateTrack()
{
  G4DynamicParticle* Particle = new G4DynamicParticle(G4Gamma::Definition(), G4ThreeVector(0, 0, 1), 511*keV);
  return new G4Track(Particle, 0.0, G4ThreeVector(0, 0, 0));
}


////////////////////////////////////////////////////////////////////////////////


MCTrackInformation* UTTrackingAction::GetInformation(const G4Track* Track)
{
  return dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackingAction::TestConstruction()
{
  bool Passed = true;

  MCTrackingAction Action;
  G4UserTrackingAction* Base = &Action;
  Passed = EvaluateTrue("MCTrackingAction()", "base class", "The action is a Geant4 user tracking action which can be cast back", dynamic_cast<MCTrackingAction*>(Base) == &Action) && Passed;

  // Repeated creation and destruction:
  for (unsigned int i = 0; i < 50; ++i) {
    MCTrackingAction* Temporary = new MCTrackingAction();
    delete Temporary;
  }
  Passed = EvaluateTrue("~MCTrackingAction()", "50 actions", "Many actions can be created and destroyed", true) && Passed;

  // The Geant4 post action is not overridden and must do nothing:
  G4Track* Track = CreateTrack();
  Action.PostUserTrackingAction(Track);
  Passed = EvaluateTrue("PostUserTrackingAction()", "default", "The post tracking action does not attach information", GetInformation(Track) == nullptr) && Passed;
  delete Track;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackingAction::TestNoUninitializedMembers()
{
  bool Passed = true;

  // Create the action in memory filled with two different patterns and attach the information to a fresh primary:
  vector<int> Ids;
  for (unsigned char Pattern: { (unsigned char) 0x00, (unsigned char) 0xA5 }) {
    alignas(MCTrackingAction) unsigned char Buffer[sizeof(MCTrackingAction)];
    memset(Buffer, Pattern, sizeof(Buffer));
    MCTrackingAction* Action = new (Buffer) MCTrackingAction();

    G4Track* Track = CreateTrack();
    Action->PreUserTrackingAction(Track);
    MCTrackInformation* Info = GetInformation(Track);
    Ids.push_back(Info != nullptr ? Info->GetId() : 12345678);
    delete Track;

    Action->~MCTrackingAction();
  }
  // Known failure: the constructor of MCTrackingAction does not initialize m_NPrimaries, the id of the first primary is garbage
  Passed = ExpectedToFail(Evaluate("MCTrackingAction()", "memory patterns", "The id of a primary does not depend on the memory the action is created in - the number of primaries is initialized", Ids[0], Ids[1])) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackingAction::TestPrimaryIds()
{
  bool Passed = true;

  MCTrackingAction Action;
  Action.SetNGeneratedParticles(3);

  vector<G4Track*> Tracks;
  for (unsigned int i = 0; i < 4; ++i) Tracks.push_back(CreateTrack());
  vector<int> ExpectedIds = { 3, 2, 1, 0 };
  for (unsigned int i = 0; i < Tracks.size(); ++i) {
    Action.PreUserTrackingAction(Tracks[i]);
    MCTrackInformation* Info = GetInformation(Tracks[i]);
    Passed = EvaluateTrue("PreUserTrackingAction()", MString("primary ") + i, "A primary track gets information attached", Info != nullptr) && Passed;
    if (Info != nullptr) {
      Passed = Evaluate("PreUserTrackingAction()", MString("primary ") + i, "The ids of the primaries count down from the number of generated particles", Info->GetId(), ExpectedIds[i]) && Passed;
      Passed = Evaluate("PreUserTrackingAction()", MString("primary ") + i, "A primary is its own origin", Info->GetOriginId(), ExpectedIds[i]) && Passed;
      Passed = EvaluateFalse("PreUserTrackingAction()", MString("primary ") + i, "A new track is not digitized", Info->IsDigitized()) && Passed;
    }
  }
  for (G4Track* T: Tracks) delete T;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackingAction::TestExistingInformation()
{
  bool Passed = true;

  MCTrackingAction Action;
  Action.SetNGeneratedParticles(5);

  // A secondary already has the information set by the stepping action:
  G4Track* Secondary = CreateTrack();
  MCTrackInformation* Original = new MCTrackInformation(99, 98);
  Original->SetDigitized(true);
  Secondary->SetUserInformation(Original);
  Action.PreUserTrackingAction(Secondary);
  MCTrackInformation* After = GetInformation(Secondary);
  Passed = EvaluateTrue("PreUserTrackingAction()", "secondary", "The information of a secondary is kept", After == Original) && Passed;
  Passed = Evaluate("PreUserTrackingAction()", "secondary", "The id of a secondary is kept", After->GetId(), 99) && Passed;
  Passed = Evaluate("PreUserTrackingAction()", "secondary", "The origin id of a secondary is kept", After->GetOriginId(), 98) && Passed;
  Passed = EvaluateTrue("PreUserTrackingAction()", "secondary", "The flags of a secondary are kept", After->IsDigitized()) && Passed;

  // The secondary must not consume a primary id:
  G4Track* Primary = CreateTrack();
  Action.PreUserTrackingAction(Primary);
  Passed = EvaluateTrue("PreUserTrackingAction()", "primary after secondary", "The primary after a secondary gets information", GetInformation(Primary) != nullptr) && Passed;
  if (GetInformation(Primary) != nullptr) {
    Passed = Evaluate("PreUserTrackingAction()", "primary after secondary", "A secondary does not consume a primary id", GetInformation(Primary)->GetId(), 5) && Passed;
  }

  // The same track can be handled twice - the second time it already has information:
  Action.PreUserTrackingAction(Primary);
  Passed = Evaluate("PreUserTrackingAction()", "twice", "Handling a track twice does not change its id", GetInformation(Primary)->GetId(), 5) && Passed;
  G4Track* Next = CreateTrack();
  Action.PreUserTrackingAction(Next);
  Passed = Evaluate("PreUserTrackingAction()", "twice", "Handling a track twice does not consume a second id", GetInformation(Next)->GetId(), 4) && Passed;

  delete Secondary;
  delete Primary;
  delete Next;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackingAction::TestSetNGeneratedParticles()
{
  bool Passed = true;

  MCTrackingAction Action;

  // A new event resets the ids:
  Action.SetNGeneratedParticles(2);
  G4Track* First = CreateTrack();
  Action.PreUserTrackingAction(First);
  Passed = Evaluate("SetNGeneratedParticles()", "2", "The first primary of the first event gets the id 2", GetInformation(First)->GetId(), 2) && Passed;

  Action.SetNGeneratedParticles(7);
  G4Track* Second = CreateTrack();
  Action.PreUserTrackingAction(Second);
  Passed = Evaluate("SetNGeneratedParticles()", "7", "Setting the number again restarts the ids", GetInformation(Second)->GetId(), 7) && Passed;

  Action.SetNGeneratedParticles(0);
  G4Track* Zero = CreateTrack();
  Action.PreUserTrackingAction(Zero);
  Passed = Evaluate("SetNGeneratedParticles()", "0", "Zero generated particles are stored as they are", GetInformation(Zero)->GetId(), 0) && Passed;

  Action.SetNGeneratedParticles(-3);
  G4Track* Negative = CreateTrack();
  Action.PreUserTrackingAction(Negative);
  Passed = Evaluate("SetNGeneratedParticles()", "-3", "A negative number is stored as it is", GetInformation(Negative)->GetId(), -3) && Passed;

  delete First;
  delete Second;
  delete Zero;
  delete Negative;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackingAction::TestManyPrimaries()
{
  bool Passed = true;

  MCTrackingAction Action;
  const int N = 1000;
  Action.SetNGeneratedParticles(N);

  set<int> Ids;
  bool AllHaveInformation = true;
  vector<G4Track*> Tracks;
  for (int i = 0; i < N; ++i) {
    G4Track* Track = CreateTrack();
    Action.PreUserTrackingAction(Track);
    if (GetInformation(Track) == nullptr) {
      AllHaveInformation = false;
    } else {
      Ids.insert(GetInformation(Track)->GetId());
    }
    Tracks.push_back(Track);
  }
  Passed = EvaluateTrue("PreUserTrackingAction()", "1000 primaries", "All primaries get information", AllHaveInformation) && Passed;
  Passed = EvaluateSize("PreUserTrackingAction()", "1000 primaries", "All ids are unique", Ids.size(), N) && Passed;
  Passed = Evaluate("PreUserTrackingAction()", "1000 primaries", "The smallest id is 1", *Ids.begin(), 1) && Passed;
  Passed = Evaluate("PreUserTrackingAction()", "1000 primaries", "The largest id is the number of primaries", *Ids.rbegin(), N) && Passed;
  for (G4Track* T: Tracks) delete T;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTTrackingAction Test;
  return Test.Run() == true ? 0 : 1;
}
