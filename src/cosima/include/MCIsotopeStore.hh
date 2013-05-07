/*
 * MCIsotopeStore.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Stores some isotops and writes them to/ reads them from file
 *
 */

#ifndef ___MCIsotopeStore___
#define ___MCIsotopeStore___

// Geant4:
#include "G4TouchableHistory.hh"
#include "G4ParticleDefinition.hh"

// Cosima:
#include "MCSource.hh"

// MEGAlib:

// ROOT:
#include "MString.h"

// Standard lib:
#include <vector>
using namespace std;

// Forward declarations:


/******************************************************************************/

class MCIsotopeStore
{
  // public interface:
public:
  /// Default constructor
  MCIsotopeStore();
  /// Default destructor
  virtual ~MCIsotopeStore();

  /// Reset all data
  void Reset();

  /// Add a new particle to the store:
  void Add(MString VolumeName, int ID, double Excitation, double Value = 1.0);

  /// Remove a particle from the store:
  void Remove(unsigned int v, unsigned int i, unsigned int e);

  /// Save the store to disk
  bool Save(MString FileName);

  /// Save the store to disk
  bool Load(MString FileName);

  /// Scale (multiply) to content by a certain factor
  void Scale(double Scale);

  /// Add a IsotopeStore
  void Add(const MCIsotopeStore& RPS);

  /// Remove all stable elements:
  void RemoveStableElements();

  /// Sort the array acording to volume names, IDs, and excitations
  void Sort();

  /// Create a source list assuming the value is a activity (Bq)
  vector<MCSource*> CreateSourceListByActivity();

  /// Create a source list assuming the value is an isotope count
  vector<MCSource*> CreateSourceListByIsotopeCount();

  /// Set the simulation time
  void SetTime(double Time) { m_Time = Time; }
  /// Get the simulation time
  double GetTime() const { return m_Time; } 

  /// Return the particle definition generated for the given volume, ID, and excitation level
  G4ParticleDefinition* GetParticleDefinition(unsigned int v, unsigned int i, unsigned int e);

  /// Return the number of volumes:
  unsigned int GetNVolumes() const;

  /// Return the number of IDs for a given volume:
  unsigned int GetNIDs(unsigned int v) const;

  /// Return the number of excitation levels for a given volume and ID:
  unsigned int GetNExcitations(unsigned int v, unsigned int i) const;

  /// Return a volume name:
  MString GetVolume(unsigned int v) const;

  /// Return an ID for a given volume:
  int GetID(unsigned int v, unsigned int i) const;

  /// Return an excitation for a given volume and ID:
  double GetExcitation(unsigned int v, unsigned int i, unsigned int e) const;

  /// Return the value for a given volume:
  double GetValue(unsigned int v, unsigned int i, unsigned int e) const;


  /// Return the particle definition generated for the given volume and ID
  static G4ParticleDefinition* GetParticleDefinition(int ID, double Excitation);

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Name of the volumes
  vector<MString> m_VolumeNames;
  /// Name of the isotope IDs
  vector<vector<int> > m_IDs;
  /// Name of the isotope excitations
  vector<vector<vector<double> > > m_Excitations;
  /// Detected counts
  vector<vector<vector<double> > > m_Values;
  /// Total simulation time
  double m_Time;
};

/// Dumping operator
std::ostream& operator<<(std::ostream& os, const MCIsotopeStore& S);

#endif


/*
 * MCIsotopeStore.hh: the end...
 ******************************************************************************/
