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

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSimHT
{
  // Public Interface:
 public:
  //! Simple constructor - set all values via the set commands 
  MSimHT(MDGeometryQuest* Geo = 0);
  //! Standard constructor, which contains all neccessary infos
  MSimHT(const int Detector, const MVector& Position, const double Energy, 
         const double Time, const vector<int>& Origins, MDGeometryQuest* Geo);
  //! Copy constructor
  MSimHT(const MSimHT& HT);
  //! Default destructor
  virtual ~MSimHT();

  //! Set everything via one line of input
  bool AddRawInput(MString LineBuffer, int Version = 100);

  //! Return the number of the detector, where the hit took place
  int GetDetectorType() const { return m_DetectorType; };
  //! Set the detctor type in which the hit took place
  void SetDetectorType(const int Detector) { m_DetectorType = Detector; }

  //! Return the location at which the hit happend
  MVector GetPosition() const { return m_Position; }
  //! Set the location at which the hit happend  
  void SetPosition(const MVector& Pos);

  //! Set the energy deposited during this hit
  double GetEnergy() const { return m_Energy; }
  //! Return the energy deposited during this hit
  void SetEnergy(const double Energy);

  //! Return the original location of the hit - before noising
  MVector GetOriginalPosition() const { return m_OriginalPosition; }
  //! Return the original energy deposit - before noising
  double GetOriginalEnergy() const { return m_OriginalEnergy; }
  //! Set the time when this hit happend
  double GetTime() const { return m_Time; }
  //! Return the time when this hit happend
  void SetTime(const double Time) { m_Time = Time; }

  //! Test if i is one of the origins of this hit
  bool IsOrigin(const int i) const;
  //! Return the i-th origin 
  int GetOriginAt(const unsigned int i) const;
  //! Return the number of origins
  unsigned int GetNOrigins() const;
  //! Add an origin - ignored if it already exists
  void AddOrigin(const int i);
  //! Set all origins
  void SetOrigins(const set<int>& Origins);
  //! Return the origin with the smallest id
  int GetSmallestOrigin(const int Except = -1) const;
  //! Return all origins
  vector<int> GetOrigins() const;

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


  // Depreciated

  //! Return the number of the detector, where the hit took place
  int GetDetector() const { return m_DetectorType; };
  //! Set the detctor type in which the hit took place
  void SetDetector(const int Detector) { m_DetectorType = Detector; }


  // protected methods:
 protected:
  //! Do the actual noising of this hit - called by SetEnergy & SetPosition
  bool Noise(bool RecalculateVolumeSequence = true);


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
  vector<int> m_Origins; 

  //! Position, where the hit took place, before noising
  MVector m_OriginalPosition;  
  //! Energy deposit, before noising
  double m_OriginalEnergy;       

  //! Pointer to the geometry description
  MDGeometryQuest* m_Geometry;     
  //! Volume history of this hit
  MDVolumeSequence* m_VolumeSequence;       

  //! Pointer to a cluster this hit eventually belongs to
  MSimCluster* m_Cluster;    
 
  //! A flag, indicating whether this hit has been added to something or not
  bool m_Added;           


#ifdef ___CLING___
 public:
  ClassDef(MSimHT, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
