/*
 * MCActivatorParticle.cxx
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
#include "MCActivatorParticle.hh"

// MEGAlib:
#include "MAssert.h"
#include "MStreams.h"

// Geant4:
#include "G4ParticleTable.hh"
#include "G4RadioactiveDecay.hh"
#include "G4DecayTable.hh"
#include "G4VDecayChannel.hh"

// Standard lib:


/******************************************************************************
 * Not yet implemented
 */
MCActivatorParticle::MCActivatorParticle()
{
  // Intentionally left blank

  m_ID = 0;
  m_Excitation = 0;
  m_Definition = 0; 
  m_HalfLife = 0; 
  m_BranchingRatio = 1.0; // Don't change!
  m_ProductionRate = 1.0;
  m_Counts = 0;
  m_Activation = 0;
  m_StorageMarker = 0;
}


/******************************************************************************
 * Not yet implemented
 */
MCActivatorParticle::MCActivatorParticle(const MCActivatorParticle& A)
{
  m_ID = A.m_ID;
  m_Excitation = A.m_Excitation;
  m_HalfLife = A.m_HalfLife;
  m_BranchingRatio = A.m_BranchingRatio;
  m_Definition = A.m_Definition;
  m_ProductionRate = A.m_ProductionRate;
  m_Counts = A.m_Counts;
  m_Activation = A.m_Activation;
  m_StorageMarker = A.m_StorageMarker;
}


/******************************************************************************
 * Not yet implemented
 */
MCActivatorParticle::~MCActivatorParticle()
{
  // Intentionally left blank
}


/******************************************************************************
 * Not yet implemented
 */
MCActivatorParticle& MCActivatorParticle::operator=(const MCActivatorParticle& A)
{
  m_ID = A.m_ID;
  m_Excitation = A.m_Excitation;
  m_HalfLife = A.m_HalfLife;
  m_BranchingRatio = A.m_BranchingRatio;
  m_Definition = A.m_Definition;
  m_ProductionRate = A.m_ProductionRate;
  m_Counts = A.m_Counts;
  m_Activation = A.m_Activation;
  m_StorageMarker = A.m_StorageMarker;

  return *this;
}


/******************************************************************************
 * The equal operator checks only the base data, not the production rate,
 * count, activation or storage markers!!
 */
bool MCActivatorParticle::operator==(const MCActivatorParticle& A)
{
  // All double originate from the same source, thus have to be identical!
  if (m_ID == A.m_ID && 
      m_Excitation == A.m_Excitation &&
      m_HalfLife == A.m_HalfLife && 
      m_BranchingRatio == A.m_BranchingRatio) {
    return true;
  }

  return false;
}


/******************************************************************************
 * Not yet implemented
 */
void MCActivatorParticle::SetIDAndExcitation(unsigned int ID, double Excitation)
{
  m_ID = ID;
  m_Excitation = Excitation;

  if (m_Excitation < 0.0) m_Excitation = 0.0;

  G4ParticleTable* Table = G4ParticleTable::GetParticleTable();
  int AtomicNumber = int(m_ID/1000);
  int AtomicMass = m_ID - int(m_ID/1000)*1000;
  m_Definition = Table->GetIon(AtomicNumber, AtomicMass, Excitation);

  massert(m_Definition != 0);
}


/******************************************************************************
 * Return the decay constant
 */
double MCActivatorParticle::GetDecayConstant() const
{
  if (m_HalfLife == 0) return numeric_limits<double>::max();
  if (m_HalfLife == numeric_limits<double>::max()) return 0;

  return log(2.0)/m_HalfLife; // ln = log
}


/******************************************************************************
 * Return if the particle is stable
 */
bool MCActivatorParticle::IsStable(G4ParticleDefinition* P)
{
  // I would like tu use
  // return P->GetPDGStable()
  // but as of 10.0 it is always stable and even if it has a life time is doesn't always have decay chains...
  
  /*
  if (P->GetParticleType() == "nucleus" && 
      P->GetParticleName() != "alpha") {

    G4Ions* Nucleus = dynamic_cast<G4Ions*>(P); 

    if (Nucleus->GetExcitationEnergy() > 0.0) {
      cout<<P->GetParticleName()<<": Not stable - we have an excitation energy"<<endl;      
      return false;
    } else {
      // It is stable if we don't have entries in the decay table
      G4RadioactiveDecay* Decay = new G4RadioactiveDecay();
      G4DecayTable* DecayTable = Decay->LoadDecayTable(*P);
      if (DecayTable == 0) {
        cout<<P->GetParticleName()<<": Stable - no decay table"<<endl;
        return true;
      }
      if (DecayTable->entries() == 0) {
        cout<<P->GetParticleName()<<": Stable - decay table is empty"<<endl;
        return true;        
      }
      // Does the decay table contain something else but this element?
      DecayTable->DumpInfo();
      bool HasSomethingElse = false;
      for (int c = 0; c < DecayTable->entries(); ++c) {
        G4VDecayChannel* Channel = DecayTable->GetDecayChannel(c);
                
        for (int d = 0; d < Channel->GetNumberOfDaughters(); ++d) {
          if (Channel->GetDaughter(d) != P) {
            HasSomethingElse = true;
            break;
          }
        }
        if (HasSomethingElse == true) break;
      }
      if (HasSomethingElse == true) {
        cout<<P->GetParticleName()<<": Not stable: The decay table has real decay chains..."<<endl;
        return false;
      }
          
      cout<<P->GetParticleName()<<": Stable - the decay table just contains this element"<<endl; 
      return true;
    }
  }
  */
    
  return P->GetPDGStable();
}

  
/*
 * MCActivatorParticle.cc: the end...
 ******************************************************************************/
