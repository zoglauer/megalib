/*
 * UTTrackInformation.cc
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
#include "MCTrackInformation.hh"

// MEGAlib:
#include "MUnitTest.h"

// Geant4:
#include "G4Track.hh"
#include "G4DynamicParticle.hh"
#include "G4Gamma.hh"
#include "G4ParticleTable.hh"
#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

// Standard lib:
#include <climits>
#include <limits>
#include <memory>
using namespace std;


//! Unit test class for MCTrackInformation
class UTTrackInformation : public MUnitTest
{
public:
  //! Default constructor
  UTTrackInformation() : MUnitTest("UTTrackInformation") {}
  //! Default destructor
  virtual ~UTTrackInformation() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default constructor
  bool TestDefaultConstruction();
  //! Test the extended constructor
  bool TestExtendedConstruction();
  //! Test the setters and getters
  bool TestGettersSetters();
  //! Test the attachment to a Geant4 track
  bool TestTrackAttachment();
};


////////////////////////////////////////////////////////////////////////////////


bool UTTrackInformation::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestExtendedConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestTrackAttachment() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackInformation::TestDefaultConstruction()
{
  bool Passed = true;

  MCTrackInformation Info;
  Passed = Evaluate("MCTrackInformation()", "default", "The default id is -1", Info.GetId(), -1) && Passed;
  Passed = Evaluate("MCTrackInformation()", "default", "The default origin id is -1", Info.GetOriginId(), -1) && Passed;
  Passed = EvaluateFalse("MCTrackInformation()", "default", "A new track is not digitized", Info.IsDigitized()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackInformation::TestExtendedConstruction()
{
  bool Passed = true;

  MCTrackInformation Info(17, 5);
  Passed = Evaluate("MCTrackInformation(int, int)", "17, 5", "The id is stored", Info.GetId(), 17) && Passed;
  Passed = Evaluate("MCTrackInformation(int, int)", "17, 5", "The origin id is stored", Info.GetOriginId(), 5) && Passed;
  Passed = EvaluateFalse("MCTrackInformation(int, int)", "17, 5", "A new track is not digitized", Info.IsDigitized()) && Passed;

  MCTrackInformation Same(3, 3);
  Passed = Evaluate("MCTrackInformation(int, int)", "primary", "A primary track is its own origin", Same.GetOriginId(), Same.GetId()) && Passed;

  MCTrackInformation Zero(0, 0);
  Passed = Evaluate("MCTrackInformation(int, int)", "zero", "Id zero is stored", Zero.GetId(), 0) && Passed;

  MCTrackInformation Negative(-5, -7);
  Passed = Evaluate("MCTrackInformation(int, int)", "negative", "A negative id is stored as it is", Negative.GetId(), -5) && Passed;
  Passed = Evaluate("MCTrackInformation(int, int)", "negative", "A negative origin id is stored as it is", Negative.GetOriginId(), -7) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackInformation::TestGettersSetters()
{
  bool Passed = true;

  MCTrackInformation Info;
  Info.SetId(42);
  Passed = Evaluate("SetId/GetId", "42", "GetId returns the id set before", Info.GetId(), 42) && Passed;
  Info.SetOriginId(11);
  Passed = Evaluate("SetOriginId/GetOriginId", "11", "GetOriginId returns the origin id set before", Info.GetOriginId(), 11) && Passed;
  Passed = Evaluate("SetId/GetId", "independence", "Setting the origin id does not change the id", Info.GetId(), 42) && Passed;

  Info.SetDigitized(true);
  Passed = EvaluateTrue("SetDigitized/IsDigitized", "true", "The digitized flag is set", Info.IsDigitized()) && Passed;
  Passed = Evaluate("SetDigitized/IsDigitized", "independence", "Setting the flag does not change the id", Info.GetId(), 42) && Passed;
  Info.SetDigitized(false);
  Passed = EvaluateFalse("SetDigitized/IsDigitized", "false", "The digitized flag is cleared", Info.IsDigitized()) && Passed;

  // Extreme values:
  Info.SetId(numeric_limits<int>::max());
  Passed = Evaluate("SetId/GetId", "INT_MAX", "The maximum id is stored", Info.GetId(), numeric_limits<int>::max()) && Passed;
  Info.SetOriginId(numeric_limits<int>::min());
  Passed = Evaluate("SetOriginId/GetOriginId", "INT_MIN", "The minimum origin id is stored", Info.GetOriginId(), numeric_limits<int>::min()) && Passed;

  // Print is not used and must not crash:
  Info.Print();
  Passed = EvaluateTrue("Print()", "call", "Print can be called", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTrackInformation::TestTrackAttachment()
{
  bool Passed = true;

  // Geant4 owns the user information after it is attached to a track and deletes it with the track:
  G4BosonConstructor().ConstructParticle();
  G4LeptonConstructor().ConstructParticle();
  G4ParticleTable::GetParticleTable()->SetReadiness();

  G4DynamicParticle* Particle = new G4DynamicParticle(G4Gamma::Definition(), G4ThreeVector(0, 0, 1), 511*keV);
  G4Track* Track = new G4Track(Particle, 0.0, G4ThreeVector(0, 0, 0));
  Passed = EvaluateTrue("G4Track::GetUserInformation()", "new track", "A new track has no user information", Track->GetUserInformation() == nullptr) && Passed;

  Track->SetUserInformation(new MCTrackInformation(7, 3));
  MCTrackInformation* Attached = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
  Passed = EvaluateTrue("SetUserInformation()", "attach", "The attached information can be retrieved as MCTrackInformation", Attached != nullptr) && Passed;
  if (Attached != nullptr) {
    Passed = Evaluate("SetUserInformation()", "attach", "The id survives the attachment", Attached->GetId(), 7) && Passed;
    Passed = Evaluate("SetUserInformation()", "attach", "The origin id survives the attachment", Attached->GetOriginId(), 3) && Passed;
    Attached->SetDigitized(true);
    MCTrackInformation* Again = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
    Passed = EvaluateTrue("SetUserInformation()", "attach", "A change through the attached pointer is visible on the track", Again != nullptr && Again->IsDigitized() == true) && Passed;
  }

  // The track deletes the information and the dynamic particle:
  delete Track;
  Passed = EvaluateTrue("~G4Track()", "delete", "The track and its user information can be deleted", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTTrackInformation Test;
  return Test.Run() == true ? 0 : 1;
}
