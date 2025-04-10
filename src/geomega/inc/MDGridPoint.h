/*
 * MDGridPoint.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDGridPoint__
#define __MDGridPoint__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <set>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDGridPoint
{
  // public interface:
 public:
  //! Empty default constructor - do not use it because you cannot set the 
  //! entries manually
  MDGridPoint();
  //! Prefered standard constructor
  MDGridPoint(const unsigned int xGrid, 
              const unsigned int yGrid, 
              const unsigned int zGrid, 
              const unsigned int Type = c_Voxel,
              const MVector Position = MVector(0.0, 0.0, 0.0), 
              const double Energy = 0.0, 
              const double Time = 0.0,
              const unsigned int Hits = 1,
              const double Weight = 1.0);
  //! Copy constructor
  MDGridPoint(const MDGridPoint& GridPoint);
  //! Default (empty) destructor
  virtual ~MDGridPoint();

  //! Default assignment constructor
  const MDGridPoint& operator=(const MDGridPoint& GP);

  //! Is-Equal operator
  bool operator==(const MDGridPoint& GP);
  //! Addition operator
  const MDGridPoint& operator+=(const MDGridPoint& GP);

  //! Set most of the variables:
  void Set(const unsigned int xGrid, 
           const unsigned int yGrid, 
           const unsigned int zGrid, 
           const unsigned int Type = c_Voxel, 
           const MVector Position = MVector(0.0, 0.0, 0.0), 
           const double Energy = 0.0, 
           const double Time = 0.0,
           const unsigned int Hits = 1,
           const double Weight = 1.0);

  //! Return the grid number in x dimension
  unsigned int GetXGrid() const { return m_xGrid; }
  //! Set the grid number in x dimension
  void SetXGrid(const unsigned int xGrid) { m_xGrid = xGrid; }

  //! Return the grid number in y dimension
  unsigned int GetYGrid() const { return m_yGrid; }
  //! Set the grid number in y dimension
  void SetYGrid(const unsigned int yGrid) { m_yGrid = yGrid; }

  //! Return the grid number in z dimension
  unsigned int GetZGrid() const { return m_zGrid; }
  //! Set the grid number in z dimension
  void SetZGrid(const unsigned int zGrid) { m_zGrid = zGrid; }

  //! Return the (combined) interaction position within the grid point
  MVector GetPosition() const { return m_Position; }
  //! Set the interaction position within the grid point
  void SetPosition(const MVector Position) { m_Position = Position; }

  //! Return the total energy deposit at this grid position 
  double GetEnergy() const { return m_Energy; }
  //! Set the total energy deposit at this grid position 
  void SetEnergy(const double Energy) { m_Energy = Energy; }

  //! Return the time of the initial interaction at this grid position
  double GetTime() const { return m_Time; }
  //! Set the time of the initial interaction at this grid position
  void SetTime(const double Time) { m_Time = Time; }

  //! Return the hits at this grid position
  unsigned int GetHits() const { return m_Hits; }
  //! Set the hits of this grid position
  void SetHits(const unsigned int Hits) { m_Hits = Hits; }
  
  //! Return the weight of this grid position
  double GetWeight() const { return m_Weight; }
  //! Set the weight of this grid position
  void SetWeight(const double Weight) { m_Weight = Weight; }
  
  //! Return the flags for this grid position
  MString GetFlags() const { return m_Flags; }
  //! Set the flags of this grid position
  void SetFlags(const MString Flags) { m_Flags = Flags; }
  
  //! Return all hit origins at this grid position
  set<unsigned int> GetOriginIDs() const { return m_OriginIDs; }
  //! Set all hit origins at this grid position
  void SetOriginIDs(const set<unsigned int>& OriginIDs) { m_OriginIDs = OriginIDs; }
  //! Add all hit origins at this grid position
  void AddOriginIDs(const set<int unsigned>& OriginIDs) { m_OriginIDs.insert(OriginIDs.begin(), OriginIDs.end()); }
  
  //! Return true if the grid point is above the trigger threshold
  bool IsAboveTriggerThreshold() const { return m_IsAboveTriggerThreshold; }
  //! Set whether the grid point is above thr trigger threshold
  void IsAboveTriggerThreshold(bool Above) { m_IsAboveTriggerThreshold = Above; }
  
  //! Return true if the grid point is read out
  bool IsReadOut() const { return m_IsReadOut; }
  //! Set whether the grid point is reade out
  void IsReadOut(bool ReadOut) { m_IsReadOut = ReadOut; }
  
  
  //! Set the type of the grid point
  void SetType(unsigned int Type) { m_Type = Type; }
  //! Return the type of the grid point
  unsigned int GetType() const { return m_Type; }
  

  // The different grid point types:
  static const unsigned int c_Unknown;
  static const unsigned int c_Voxel;
  static const unsigned int c_VoxelDrift;
  static const unsigned int c_XStrip;
  static const unsigned int c_YStrip;
  static const unsigned int c_XYAnger;
  static const unsigned int c_XYZAnger;
  static const unsigned int c_XYZIndependentAnger;
  static const unsigned int c_GuardRing;
  static const unsigned int c_MinType;
  static const unsigned int c_MaxType;


  // protected methods:
 protected:

  // private methods:
 private:
  friend ostream& operator<<(ostream& os, const MDGridPoint& GridPoint);



  // protected members:
 protected:


  // private members:
 private:
  //! Grid number in x dimension 
  unsigned int m_xGrid;
  //! Grid number in y dimension
  unsigned int m_yGrid;
  //! Grid number in z dimension
  unsigned int m_zGrid;
  //! Type of the grid point
  unsigned int m_Type;
  //! The individual or combined position of the hit in the grid point
  MVector m_Position;
  //! Total energy deposit at this grid position 
  double m_Energy;
  //! Time of the initial interaction at this grid position
  double m_Time;
  //! Number of hits at this grid position
  unsigned int m_Hits;
  //! A weight for this grid position - useful for normalizations to cts/sec
  double m_Weight;
    
  //! Hit origins at this grid position (Used only by sivan)
  set<unsigned int> m_OriginIDs;
  
  //! Special flags from the DEE
  MString m_Flags;
  
  //! True is the hit is above the trigger threshold -- NOT is it triggered
  bool m_IsAboveTriggerThreshold;
  
  //! True if the grid point need to be read-out
  bool m_IsReadOut;
  
  
  
  
#ifdef ___CLING___
 public:
  ClassDef(MDGridPoint, 0)
#endif

};

ostream& operator<<(ostream& os, const MDGridPoint& GridPoint);

#endif


////////////////////////////////////////////////////////////////////////////////
