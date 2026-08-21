/*
 * MDGridPointCollection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDGridPointCollection__
#define __MDGridPointCollection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDGridPoint.h"
#include "MDVolumeSequence.h"

// Forward declarations:
class MDDetector;

////////////////////////////////////////////////////////////////////////////////


class MDGridPointCollection
{
  // public interface:
 public:
  //! Default constructor - SetVolumeSequence() must be called before using 
  MDGridPointCollection() {};
  MDGridPointCollection(const MDVolumeSequence& VolumeSequence);
  MDGridPointCollection(const MDGridPointCollection& Grid);
  virtual ~MDGridPointCollection();

  //! Set volume sequence
  void SetVolumeSequence(const MDVolumeSequence& VolumeSequence) { m_VolumeSequence = VolumeSequence; }
  
  //! Add a new grid point to the grid and discretize (noise, thresholds, etc)
  void Add(const MVector& Position, const double Energy, const double Time, const set<unsigned int>& Origins);
  //! Only add a new grid point to the grid
  void AddUndiscretized(const MVector& PositionInDetector);
  //! Add another GridPointCollection
  void Add(MDGridPointCollection& Collection);
  //! Add another grid point - must be of similar type to the ones already in there
  void Add(MDGridPoint& Point);
  
  
  //! Has to be called after all hits have been added!
  void FinalizeGridding();

  //! Check if the volume sequences point to the same detector
  bool HasSameDetector(MDVolumeSequence* VolumeSequence);
  //! Check if the grid has the same detector
  bool HasSameDetector(MDGridPointCollection& Grid);
  //! Return a pointer to the detector
  MDDetector* GetDetector() { return m_VolumeSequence.GetDetector(); }

  //! Return the number of stored grid points
  unsigned int GetNGridPoints() const { return m_Points.size(); }
  //! Return the grid point at position i (no error checks for performance reasons!)
  const MDGridPoint& GetGridPointAt(unsigned int i) const { return m_Points[i]; }
  //! Return the grid point at position i (no error checks for performance reasons!)
  MDGridPoint& GetGridPointAt(unsigned int i) { return m_Points[i]; }
  //! Position (in world coordinates) of grid point i (no error checks for performance reasons!)
  MVector GetWorldPositionGridPointAt(unsigned int i) const;

  //! Remove not read-out grid points
  void RemoveNonReadOuts();
  
  
  //! Return the world position of the detector of the grid
  MVector GetWorldPosition();
  //! Return the total energy position in the grid
  double GetEnergy();
  //! Return the total number of hits in the grid
  unsigned int GetHits();
  //! Set the weight (some arbitrary scaling factor) of the hits
  void SetWeight(double Weight);
  //! Return the total number of hits in the grid weigthed by their individual weighting factor
  double GetWeightedHits();

  //! Return the volume sequence
  const MDVolumeSequence& GetVolumeSequence() const { return m_VolumeSequence; }
  
  //! Return the volume sequence
  MString GetVolumeTree() { return m_VolumeSequence.ToStringVolumes(); }


  // protected methods:
 protected:
  //! Default constructor is protected (thus we don't have one!)

  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The volume sequence of this grid
  MDVolumeSequence m_VolumeSequence;
  //! All the points in this grid
  vector<MDGridPoint> m_Points;
  //! True if we are in strip mode, false if we are in pixel mode - currently only pixel mode is supported!
  bool m_StripMode;

#ifdef ___CLING___
 public:
  ClassDef(MDGridPointCollection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
