/*
 * UTSDStepFixture.h
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


#ifndef ___UTSDStepFixture___
#define ___UTSDStepFixture___


// Cosima:
#include "MCEventAction.hh"
#include "MCParameterFile.hh"
#include "MCTrackInformation.hh"

// Geant4:
#include "G4BosonConstructor.hh"
#include "G4Box.hh"
#include "G4DynamicParticle.hh"
#include "G4EventManager.hh"
#include "G4Gamma.hh"
#include "G4HCofThisEvent.hh"
#include "G4LogicalVolume.hh"
#include "G4NavigationHistory.hh"
#include "G4ParticleTable.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4TouchableHistory.hh"
#include "G4VSensitiveDetector.hh"

// Standard lib:
#include <vector>
using namespace std;


//! Fixture creating real Geant4 volumes, tracks and steps for the unit tests of the sensitive detectors
//! The volumes form a chain World -> first added volume -> second added volume ...
//! The physical volumes are named as given, the logical volumes get the suffix "Log", like in cosima
class UTSDStepFixture
{
public:
  //! Create the world volume
  UTSDStepFixture()
  {
    // Gamma is the particle of all tracks:
    G4BosonConstructor().ConstructParticle();

    G4Box* WorldBox = new G4Box("WorldBox", 1.0*m, 1.0*m, 1.0*m);
    m_Solids.push_back(WorldBox);
    G4LogicalVolume* WorldLogical = new G4LogicalVolume(WorldBox, nullptr, "WorldLog");
    m_Logical.push_back(WorldLogical);
    m_Physical.push_back(new G4PVPlacement(nullptr, G4ThreeVector(), WorldLogical, "World", nullptr, false, 0, false));
    m_LogicalOf.push_back(WorldLogical);
    m_Parents.push_back(-1);
  }

  //! Delete everything which has been created
  ~UTSDStepFixture()
  {
    for (G4Step* Step: m_Steps) delete Step;
    for (G4Track* Track: m_Tracks) delete Track;
    for (int i = m_Physical.size() - 1; i >= 0; --i) delete m_Physical[i];
    for (int i = m_Logical.size() - 1; i >= 0; --i) delete m_Logical[i];
    for (G4VSolid* Solid: m_Solids) delete Solid;
    for (G4RotationMatrix* Rotation: m_Rotations) delete Rotation;
  }

  //! Add a volume inside the volume with the given index (default: the last added volume) - translation and rotation are the ones used to place it in its mother
  //! The rotation is the one of the frame (as for G4PVPlacement) and is owned by the fixture
  int AddVolume(const G4String& Name, const G4ThreeVector& HalfSize, const G4ThreeVector& Translation, G4RotationMatrix* Rotation = nullptr, int Parent = -1)
  {
    if (Parent < 0) Parent = (int) m_Physical.size() - 1;
    G4Box* Box = new G4Box(Name + "Box", HalfSize.x(), HalfSize.y(), HalfSize.z());
    m_Solids.push_back(Box);
    G4LogicalVolume* Logical = new G4LogicalVolume(Box, nullptr, Name + "Log");
    m_Logical.push_back(Logical);
    if (Rotation != nullptr) m_Rotations.push_back(Rotation);
    m_Physical.push_back(new G4PVPlacement(Rotation, Translation, Logical, Name, m_LogicalOf[Parent], false, 0, false));
    m_LogicalOf.push_back(Logical);
    m_Parents.push_back(Parent);
    return (int) m_Physical.size() - 1;
  }

  //! Place the logical volume of an already added volume once more, with another physical name and position
  int AddPlacement(const G4String& Name, int VolumeOfLogical, const G4ThreeVector& Translation, G4RotationMatrix* Rotation = nullptr, int Parent = -1)
  {
    if (Parent < 0) Parent = (int) m_Physical.size() - 1;
    if (Rotation != nullptr) m_Rotations.push_back(Rotation);
    G4LogicalVolume* Logical = m_LogicalOf[VolumeOfLogical];
    m_Physical.push_back(new G4PVPlacement(Rotation, Translation, Logical, Name, m_LogicalOf[Parent], false, 1, false));
    m_LogicalOf.push_back(Logical);
    m_Parents.push_back(Parent);
    return (int) m_Physical.size() - 1;
  }

  //! Create a step of a gamma in the volume with the given index (default: the last added volume) with the given energy deposit and positions (world coordinates)
  //! The step is owned by the fixture
  G4Step* CreateStep(double Energy, const G4ThreeVector& PrePosition, const G4ThreeVector& PostPosition, G4StepStatus PostStepStatus = fPostStepDoItProc, int TrackID = 1, double GlobalTime = 0.0, int Volume = -1)
  {
    if (Volume < 0) Volume = (int) m_Physical.size() - 1;
    G4DynamicParticle* Particle = new G4DynamicParticle(G4Gamma::Definition(), G4ThreeVector(0.0, 0.0, 1.0), 1.0*MeV);
    G4Track* Track = new G4Track(Particle, GlobalTime, PrePosition);
    Track->SetUserInformation(new MCTrackInformation(TrackID, TrackID));
    Track->SetTrackID(TrackID);
    m_Tracks.push_back(Track);

    G4Step* Step = new G4Step();
    Step->SetTrack(Track);
    Step->SetTotalEnergyDeposit(Energy);

    // Both step points are in the volume - collect its ancestors from the world down:
    vector<int> Chain;
    for (int v = Volume; v > 0; v = m_Parents[v]) {
      Chain.insert(Chain.begin(), v);
    }
    G4NavigationHistory History;
    History.SetFirstEntry(m_Physical[0]);
    for (int v: Chain) {
      History.NewLevel(m_Physical[v], kNormal, 0);
    }
    G4TouchableHandle Pre(new G4TouchableHistory(History));
    G4TouchableHandle Post(new G4TouchableHistory(History));
    Step->GetPreStepPoint()->SetTouchableHandle(Pre);
    Step->GetPostStepPoint()->SetTouchableHandle(Post);
    Step->GetPreStepPoint()->SetPosition(PrePosition);
    Step->GetPostStepPoint()->SetPosition(PostPosition);
    Step->GetPostStepPoint()->SetStepStatus(PostStepStatus);
    m_Steps.push_back(Step);

    return Step;
  }

  //! Return the track information of the track of the step
  MCTrackInformation* GetTrackInformation(const G4Step* Step) const
  {
    return dynamic_cast<MCTrackInformation*>(Step->GetTrack()->GetUserInformation());
  }

  //! Register the hit collection of the detector, create the hit collections of an event and initialize the detector
  //! The returned object is owned by the caller
  G4HCofThisEvent* BeginEvent(G4VSensitiveDetector& SD)
  {
    Register(SD);
    G4HCofThisEvent* HCE = new G4HCofThisEvent(G4SDManager::GetSDMpointer()->GetCollectionCapacity());
    SD.Initialize(HCE);
    return HCE;
  }

  //! Finish the event and return the hit collection of the detector (owned by the hit collections of the event) or nullptr
  G4VHitsCollection* EndEvent(G4VSensitiveDetector& SD, G4HCofThisEvent* HCE)
  {
    SD.EndOfEvent(HCE);
    int HCID = G4SDManager::GetSDMpointer()->GetCollectionID(SD.GetCollectionName(0));
    if (HCID < 0) return nullptr;
    return HCE->GetHC(HCID);
  }

  //! Register only the collection of the detector - the detector itself stays owned by the caller
  void Register(G4VSensitiveDetector& SD)
  {
    for (int c = 0; c < SD.GetNumberOfCollections(); ++c) {
      G4SDManager::GetSDMpointer()->AddNewCollection(SD.GetName(), SD.GetCollectionName(c));
    }
  }

  //! Create an event manager with a cosima event action, as needed by detectors which never trigger
  //! Everything is owned by the fixture's process and never freed
  void SetupEventAction()
  {
    // Geant4 refuses user actions before the particle table is ready:
    G4ParticleTable::GetParticleTable()->SetReadiness();
    if (G4EventManager::GetEventManager() == nullptr) {
      new G4EventManager();
    }
    m_Parameters = new MCParameterFile();
    m_EventAction = new MCEventAction(*m_Parameters, false, 0, true);
    G4EventManager::GetEventManager()->SetUserAction(m_EventAction);
  }

  //! Return the event action created by SetupEventAction
  MCEventAction* GetEventAction() { return m_EventAction; }

private:
  //! All solids
  vector<G4VSolid*> m_Solids;
  //! All logical volumes
  vector<G4LogicalVolume*> m_Logical;
  //! All physical volumes
  vector<G4VPhysicalVolume*> m_Physical;
  //! All rotations
  vector<G4RotationMatrix*> m_Rotations;
  //! The index of the mother of every volume
  vector<int> m_Parents;
  //! The logical volume of every physical volume
  vector<G4LogicalVolume*> m_LogicalOf;
  //! All tracks
  vector<G4Track*> m_Tracks;
  //! All steps
  vector<G4Step*> m_Steps;
  //! The parameters of the event action
  MCParameterFile* m_Parameters = nullptr;
  //! The event action
  MCEventAction* m_EventAction = nullptr;
};


#endif


/*
 * UTSDStepFixture.h: the end...
 ******************************************************************************/
