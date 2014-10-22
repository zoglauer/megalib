/*
 * MCTrackingAction.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Cosima:
#include "MCTrackingAction.hh"
#include "MCTrackInformation.hh"
#include "MCRunManager.hh"
#include "MCPhysicsList.hh"

// MEGAlib:
#include "MStreams.h"

// Geant4:

// Standard lib:


/******************************************************************************
 * Default constructor
 */
MCTrackingAction::MCTrackingAction()
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCTrackingAction::~MCTrackingAction()
{
  // Intentionally left blank
}


/******************************************************************************
 * Actions before tracking starts
 */
void MCTrackingAction::PreUserTrackingAction(const G4Track* Track)
{
  G4Track* MyTrack = const_cast<G4Track*>(Track); 

  if (MyTrack->GetUserInformation() == 0) {
    MyTrack->SetUserInformation(new MCTrackInformation(m_NPrimaries, m_NPrimaries));
    m_NPrimaries--;
  }
  
//   if (Track->GetDefinition()->GetParticleName() == "gamma") {
//     MCRunManager::GetMCRunManager()->GetPhysicsList()->SetGammaCuts();
//   }

}


/******************************************************************************
 * Set the number of generated particles
 */
void MCTrackingAction::SetNGeneratedParticles(int NGeneratedParticles)
{
  m_NPrimaries = NGeneratedParticles;
}


/*
 * MCTrackingAction.cc: the end...
 ******************************************************************************/
