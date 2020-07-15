/*
 * MDDetectorEffectsEngine.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDDetectorEffectsEngine__
#define __MDDetectorEffectsEngine__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MDGridPoint.h"
#include "MDGridPointCollection.h"
#include "MDVolumeSequence.h"

// Forward declarations:
class MDGeometry;

////////////////////////////////////////////////////////////////////////////////


//! The default MEGAlib detector effects engine v3
//! 
//! The commands have to be run in a very defined way
//! Create -> Set -> Validate -> Reset -> Add -> Run -> Get
class MDDetectorEffectsEngine
{
  // public interface:
 public:
  //! Default constructor
  MDDetectorEffectsEngine();
  //! Default destuctor 
  virtual ~MDDetectorEffectsEngine();
  
  //! Set the geometry
  void SetGeometry(MDGeometry* Geometry) { m_Geometry = Geometry; }
  
  //! Validate the trigger unit - call after all data is set
  bool Validate() const;
  
  //! Reset the stored event data - call before each new event
  void Reset();
  
  //! Add the event time
  void AddEventTime(const MTime& Time) { m_EventTime = Time; }
  
  //! Add a hit
  void AddHit(const MVector& Position, const double& Energy, const double& Time, const set<unsigned int>& OriginIDs, const MDVolumeSequence& S);

  
  //! Run the detector effects engine
  bool Run();

  
  //! Return all hits in form of the grid point collections
  const vector<MDGridPointCollection>& GetGridPointCollections() const { return m_GridPointCollections; }
  
  
  //const vector<MDGridPoint>& GetGridPoints() const { return m_GridPoints; }
  
  //! Return all volume sequences
  //const vector<MDVolumeSequence>& GetVolumeSequences() const { return m_VolumeSequences; }
  
  //! Return the event time
  const MTime& GetEventTime() const { return m_EventTime; } 
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The geometry
  MDGeometry* m_Geometry;

  //! The event time
  MTime m_EventTime;
  
  //! The grid points
  vector<MDGridPoint> m_GridPoints;
  
  //! The volume sequences
  vector<MDVolumeSequence> m_VolumeSequences;
  
  //! The grid point collection
  vector<MDGridPointCollection> m_GridPointCollections;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MDDetectorEffectsEngine, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
