/*
 * MCActivatorParticle.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Helper class for the activator, storing a particle in its activation stage
 *
 */

#ifndef ___MCActivatorParticle___
#define ___MCActivatorParticle___

// Geant4:
#include "G4ParticleDefinition.hh"

// Cosima:

// MEGAlib:

// Standard lib:

// Forward declarations:


/******************************************************************************/

class MCActivatorParticle
{
  // public interface:
public:
  /// Default constructor
  MCActivatorParticle();
  /// Copy constructor
  MCActivatorParticle(const MCActivatorParticle& A);
  /// Default destructor
  virtual ~MCActivatorParticle();

  /// Asignment operator
  MCActivatorParticle& operator=(const MCActivatorParticle& A);

  /// The equal operator checks only the base data, not the production rate,
  /// count, activation or storage markers!!
  bool operator==(const MCActivatorParticle& A);

  /// Set the numerical particle ID and excitation
  /// This creates internaly the particle definition
  void SetIDAndExcitation(unsigned int ID, double Excitation);

  /// Set the half life of the isotope
  /// numeric_limits<double>::max() means stable, 0 means immediate decay
  void SetHalfLife(double HalfLife) { m_HalfLife = HalfLife; }

  /// Set the branching ratio from mother to this nuclid
  void SetBranchingRatio(double BR) { m_BranchingRatio = BR; }

  /// Add the branching ratio from mother to this nuclid
  void AddBranchingRatio(double BR) { m_BranchingRatio += BR; }

  /// Set the number of calculated counts
  void SetProductionRate(double ProductionRate) { m_ProductionRate = ProductionRate; }

  /// Set the number of calculated counts
  void SetCounts(double Counts) { m_Counts = Counts; }

  /// Set the activation
  void SetActivation(double Activation) { m_Activation = Activation; }

  /// Set storage marker
  void SetStorageMarker(unsigned int i) { m_StorageMarker = i; }

  /// Return the ID
  unsigned int GetID() { return m_ID; }

  /// Return the excitation
  double GetExcitation() { return m_Excitation; }

  /// Return the particle name
  G4String GetName() { if (m_Definition != 0) return m_Definition->GetParticleName(); else return "Not yet defined!"; }

  /// Return the branching ratio from mother to this nuclid
  double GetBranchingRatio() { return m_BranchingRatio; }

  /// Return the decay constant
  double GetDecayConstant();

  /// Return the half life
  /// numeric_limits<double>::max() means stable, 0 means immediate decay
  double GetHalfLife() { return m_HalfLife; }

  /// Return the production rate
  double GetProductionRate() { return m_ProductionRate; }

  /// Return the counts
  double GetCounts() { return m_Counts; }

  /// Return the production rate
  double GetActivation() { return m_Activation; }

  /// Return storage marker (meaning of value defined outside this class)
  unsigned int GetStorageMarker() { return m_StorageMarker; }

  /// Get the particle definition
  G4ParticleDefinition* GetDefinition() { return m_Definition; }

  /// Get the particle definition
  static bool IsStable(G4ParticleDefinition* P);


  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// The numerical particle ID
  unsigned int m_ID;

  /// The excitation state
  double m_Excitation;

  /// The Geant4 particle definition
  G4ParticleDefinition* m_Definition;

  /// The half life
  double m_HalfLife;

  /// The branching ratio from mother to this nuclid
  double m_BranchingRatio;

  /// The production rate
  double m_ProductionRate;

  /// The number of calculated counts
  double m_Counts;

  /// The activation
  double m_Activation;

  /// The storage marker
  unsigned int m_StorageMarker;

};

#endif


/*
 * MCActivatorParticle.hh: the end...
 ******************************************************************************/
