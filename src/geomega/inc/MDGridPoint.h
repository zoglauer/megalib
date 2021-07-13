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

  //! Return all hit origins at this grid position (Used only by sivan)
  vector<int> GetOrigins() const { return m_Origins; }
  //! Set all hit origins at this grid position (Used only by sivan)
  void SetOrigins(const vector<int>& Origins) { m_Origins = Origins; }

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
  vector<int> m_Origins;
  

#ifdef ___CLING___
 public:
  ClassDef(MDGridPoint, 0)
#endif

};

ostream& operator<<(ostream& os, const MDGridPoint& GridPoint);

#endif


////////////////////////////////////////////////////////////////////////////////
