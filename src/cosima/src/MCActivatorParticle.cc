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

  G4ParticleTable* ParticleTable = G4ParticleTable::GetParticleTable();
  int AtomicNumber = int(m_ID/1000);
  int AtomicMass = m_ID - int(m_ID/1000)*1000;
  m_Definition = ParticleTable->GetIon(AtomicNumber, AtomicMass, Excitation);

  massert(m_Definition != 0);
}


/******************************************************************************
 * Return the decay constant
 */
double MCActivatorParticle::GetDecayConstant()
{
  if (m_HalfLife == 0) return numeric_limits<double>::max();
  if (m_HalfLife == numeric_limits<double>::max()) return 0;

  return log(2.0)/m_HalfLife; // ln = log
}


/*
 * MCActivatorParticle.cc: the end...
 ******************************************************************************/
