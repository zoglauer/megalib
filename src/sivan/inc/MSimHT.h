/*
 * MSimHT.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSimHT__
#define __MSimHT__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// Standard libs
#include <vector>
#include <set>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MDGeometryQuest.h"
#include "MDVolumeSequence.h"
#include "MSimCluster.h"
#include "MBinaryStore.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSimHT
{
  // Public Interface:
 public:
  //! Simple constructor - set all values via the set commands 
  MSimHT(MDGeometryQuest* Geo = nullptr);
  //! Copy constructor
  MSimHT(const MSimHT& HT);
  //! Default destructor
  virtual ~MSimHT();

  //! Set everything via one line of input from sim file - noising will be applied automatically if set in the geometry
  bool AddRawInput(MString LineBuffer, int Version = 100);

  //! Convert the *key* content to binary
  bool ParseBinary(MBinaryStore& Out, const bool StoreOrigins, const bool StoreTime, const int OriginIDPrecision = 32, const int BinaryPrecision = 32, const int Version = 25);
  //! Convert the *key* content to binary
  bool ToBinary(MBinaryStore& Out, const bool StoreOrigins, const bool StoreTime, const int WhatToStore = 1, const int OriginIDPrecision = 32, const int BinaryPrecision = 32, const int Version = 25);
  
  
  //! Set all core data - choose if you want to do noising or not
  void Set(const int DetectorType, const MVector& Position, const double Energy, const double Time, const vector<unsigned int>& Origins, bool PerformNoising);
  
  //! Set all core data - choose if you want to do noising or not
  void Set(const int DetectorType, const MVector& Position, const double Energy, const double Time, const set<unsigned int>& Origins, bool PerformNoising);
  
  //! Return the number of the detector, where the hit took place
  int GetDetectorType() const { return m_DetectorType; };
  //! Set the detctor type in which the hit took place -- no noising will be applied and the volume sequence will not be updated
  void SetDetectorType(const int Detector) { m_DetectorType = Detector; }

  //! Return the location at which the hit happend
  MVector GetPosition() const { return m_Position; }
  //! Return the original location of the hit - before noising
  MVector GetOriginalPosition() const { return m_OriginalPosition; }
  //! Set the location at which the hit happend -- no noising will be applied and the volume sequence will not be updated
  void SetPosition(const MVector& Pos) { m_OriginalPosition = Pos; m_Position = Pos; }

  //! Return the energy deposited during this hit
  double GetEnergy() const { return m_Energy; }
  //! Get the original energy deposit - before noising
  double GetOriginalEnergy() const { return m_OriginalEnergy; }
  //! Set the energy deposited during this hit  -- no noising will be applied and the volume sequence will not be updated
  void SetEnergy(const double Energy) { m_OriginalEnergy = Energy; m_Energy = Energy; }

  
  //! Return the time when this hit happend
  double GetTime() const { return m_Time; }
  //! Return the original time when this hit happend - before noising
  double GetOriginalTime() const { return m_OriginalTime; }
  //! Set the time when this hit happend  -- no noising will be applied and the volume sequence will not be updated
  void SetTime(const double Time) { m_OriginalTime = Time; m_Time = Time; }

  //! Test if i is one of the origins of this hit
  bool IsOrigin(const unsigned int i) const;
  //! Return the i-th origin 
  unsigned int GetOriginAt(const unsigned int i) const;
  //! Return the number of origins
  unsigned int GetNOrigins() const;
  //! Add an origin - ignored if it already exists
  void AddOrigin(const unsigned int i);
  //! Set all origins
  void SetOrigins(const set<unsigned int>& Origins);
  //! Return the origin with the smallest ID, ignore any IDs given in Except unless Except is zero, return 0 if nor smallest origin is found 
  unsigned int GetSmallestOrigin(const unsigned int Except = 0) const;
  //! Return all origins
  vector<unsigned int> GetOrigins() const;

  //! Set the cluster this hit is in
  void SetCluster(MSimCluster* Cluster);
  //! Return the cluster this hit is in
  MSimCluster* GetCluster();

  //! Set the volume sequence of this hit
  void SetVolumeSequence(MDVolumeSequence* VS);
  //! Return the volume sequence of this hit
  MDVolumeSequence* GetVolumeSequence();

  //! Add a simple offset to all hit origins - needed when concatenating sim events
  void OffsetOrigins(const int Offset);

  //! Return a string of the content of this hit
  MString ToString() const;
  //! Return this hit as sim file capable string
  MString ToSimString(const int WhatToStore = 1, const int Precision = 0, const int Version = 25) const;
  
  //! Set a flag that this hit has been added to something
  void SetAddFlag(const bool Flag = true) { m_Added = Flag; }
  //! Return the flag whether this has been added to something 
  bool IsAdded() const { return m_Added; }

  //! Return the associated geometry
  MDGeometryQuest* GetGeometry() { return m_Geometry; }

  //! Do the actual noising of this hit
  bool Noise(bool RecalculateVolumeSequence = true);
  
  

  // Deprecated

  //! Return the number of the detector, where the hit took place
  int GetDetector() const { return m_DetectorType; };
  //! Set the detctor type in which the hit took place
  void SetDetector(const int Detector) { m_DetectorType = Detector; }


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Type of the detector where the hit took place
  int m_DetectorType;
  //! Position, where the hit took place
  MVector m_Position;  
  //! Energy deposit
  double m_Energy;
  //! Time of the deposit
  double m_Time;           
  //! Particles from these vertices (= origins) contributed to this hit
  vector<unsigned int> m_Origins; 

  //! Position, where the hit took place, before noising
  MVector m_OriginalPosition;  
  //! Energy deposit, before noising
  double m_OriginalEnergy;       
  //! Time, before noising
  double m_OriginalTime;       
  
  //! Pointer to the geometry description
  MDGeometryQuest* m_Geometry;     
  //! Volume history of this hit
  MDVolumeSequence* m_VolumeSequence;       

  //! Pointer to a cluster this hit eventually belongs to
  MSimCluster* m_Cluster;    
 
  //! A flag, indicating whether this hit has been added to something or not
  bool m_Added;           

  //! The flags from noising
  MString m_NoiseFlags;
  
#ifdef ___CLING___
 public:
  ClassDef(MSimHT, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
