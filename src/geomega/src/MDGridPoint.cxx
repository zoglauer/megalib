/*
 * MDGridPoint.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MDGridPoint
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDGridPoint.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDGridPoint)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MDGridPoint::c_Unknown = 0;
const unsigned int MDGridPoint::c_Voxel = 1;
const unsigned int MDGridPoint::c_VoxelDrift = 2;
const unsigned int MDGridPoint::c_XStrip = 3;
const unsigned int MDGridPoint::c_YStrip = 4;
const unsigned int MDGridPoint::c_XYAnger = 5;
const unsigned int MDGridPoint::c_XYZAnger = 6;
const unsigned int MDGridPoint::c_XYZIndependentAnger = 7;
const unsigned int MDGridPoint::c_GuardRing = 8;
const unsigned int MDGridPoint::c_MinType = MDGridPoint::c_Voxel;
const unsigned int MDGridPoint::c_MaxType = MDGridPoint::c_GuardRing;


////////////////////////////////////////////////////////////////////////////////


MDGridPoint::MDGridPoint() :
  m_xGrid(0), m_yGrid(0), m_zGrid(0), m_Type(c_Unknown), 
  m_Position(0.0, 0.0, 0.0), m_Energy(0), m_Time(0), m_Hits(1), m_Weight(1.0), m_Flags(""), m_IsAboveTriggerThreshold(false), m_IsReadOut(false)
{
  // Construct an instance of MDGridPoint
  // Type must be c_Unknown!
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint::MDGridPoint(const unsigned int xGrid, 
                         const unsigned int yGrid, 
                         const unsigned int zGrid, 
                         const unsigned int Type, 
                         const MVector Position, 
                         const double Energy, 
                         const double Time, 
                         const unsigned int Hits,
                         const double Weight) :
  m_xGrid(xGrid), m_yGrid(yGrid), m_zGrid(zGrid), m_Type(Type), 
  m_Position(Position), m_Energy(Energy), m_Time(Time), m_Hits(Hits), m_Weight(Weight), m_Flags(""), m_IsAboveTriggerThreshold(false), m_IsReadOut(false)
{
  // Construct an instance of MDGridPoint
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint::MDGridPoint(const MDGridPoint& GridPoint)
  
{
  // Construct an instance of MDGridPoint

  m_xGrid = GridPoint.m_xGrid;
  m_yGrid = GridPoint.m_yGrid;
  m_zGrid = GridPoint.m_zGrid;
  m_Position.Set(GridPoint.m_Position);
  m_Energy = GridPoint.m_Energy;
  m_Time = GridPoint.m_Time;
  m_Type = GridPoint.m_Type;
  m_Hits = GridPoint.m_Hits;
  m_Weight = GridPoint.m_Weight;
  
  //m_OriginIDs.clear();
  //m_OriginIDs.insert(GridPoint.m_OriginIDs.begin(), GridPoint.m_OriginIDs.end());
  m_OriginIDs = GridPoint.m_OriginIDs;

  if (GridPoint.m_Flags.IsEmpty() == false) {
    m_Flags = GridPoint.m_Flags;
  }
  
  m_IsAboveTriggerThreshold = GridPoint.m_IsAboveTriggerThreshold;
  m_IsReadOut = GridPoint.m_IsReadOut;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint::~MDGridPoint()
{
  // Delete this instance of MDGridPoint
}


////////////////////////////////////////////////////////////////////////////////


const MDGridPoint& MDGridPoint::operator=(const MDGridPoint& GridPoint)
{
  // Assign all values of the left hand side hit to this one 
 
  m_xGrid = GridPoint.m_xGrid;
  m_yGrid = GridPoint.m_yGrid;
  m_zGrid = GridPoint.m_zGrid;
  m_Position = GridPoint.m_Position;
  m_Energy = GridPoint.m_Energy;
  m_Time = GridPoint.m_Time;
  m_Type = GridPoint.m_Type;
  m_Hits = GridPoint.m_Hits;
  m_Weight = GridPoint.m_Weight;

  m_OriginIDs.clear();
  m_OriginIDs.insert(GridPoint.m_OriginIDs.begin(), GridPoint.m_OriginIDs.end());
  //m_OriginIDs = GridPoint.m_OriginIDs;
  
  m_Flags = GridPoint.m_Flags;
  
  m_IsAboveTriggerThreshold = GridPoint.m_IsAboveTriggerThreshold;
  m_IsReadOut = GridPoint.m_IsReadOut;
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGridPoint::operator==(const MDGridPoint& GridPoint)
{
  // Compare two hits, whether they are within the same strips of the layer, i.e.
  // only strips and detector name are compared
  
  if (m_Type == GridPoint.m_Type) {
    if (m_Type == c_Voxel || m_Type == c_VoxelDrift) {
      if (m_xGrid != GridPoint.m_xGrid || 
          m_yGrid != GridPoint.m_yGrid || 
          m_zGrid != GridPoint.m_zGrid) {
        return false;
      }
    } else if (m_Type == c_XStrip) {
      if (m_xGrid != GridPoint.m_xGrid) {
        return false; 
      }
    } else if (m_Type == c_YStrip) {
      if (m_yGrid != GridPoint.m_yGrid) {
        return false; 
      }
    } else if (m_Type == c_GuardRing || 
               m_Type == c_XYAnger ||
               m_Type == c_XYZAnger) {
      return true;
    } else {
      merr<<"MDGridPoint::operator==: Unknown grid point type: "<<m_Type<<endl;
      massert(false);
    }
  } else {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


const MDGridPoint& MDGridPoint::operator+=(const MDGridPoint& GridPoint)
{
  // Add GridPoint to this one - if they happend in the same grid!

  if (m_Type == GridPoint.m_Type) {
 
    if (m_Type == c_Voxel || m_Type == c_VoxelDrift ||m_Type == c_XYAnger || m_Type == c_XYZAnger) {
      if (m_xGrid == GridPoint.m_xGrid && m_yGrid == GridPoint.m_yGrid && m_zGrid == GridPoint.m_zGrid) {
        // Position, ...
        if (m_Type == c_Voxel || m_Type == c_VoxelDrift) {
          m_Position.SetX(0.0);
          m_Position.SetY(0.0);
          m_Position.SetZ((m_Energy*m_Position.Z() + GridPoint.m_Position.Z()*GridPoint.m_Energy)/
                          (m_Energy+GridPoint.m_Energy));
        } else if (m_Type == c_XYAnger || m_Type == c_XYZAnger) {
          m_Position = (m_Energy*m_Position + GridPoint.m_Position*GridPoint.m_Energy)*
            (1.0/(m_Energy+GridPoint.m_Energy));
          if (m_Type == c_XYAnger) {
            m_Position.SetZ(0.0);
          }
        }
        // ... Energy (not before depth!),  
        m_Energy += GridPoint.m_Energy;
        // ... Time &
        if (GridPoint.m_Time < m_Time) {
          m_Time = GridPoint.m_Time;
        }
        
        // OriginIDs
        m_OriginIDs.insert(GridPoint.m_OriginIDs.begin(), GridPoint.m_OriginIDs.end());
      
        // Make sure that if m_Weight is some cts/sec normalization factor it stays correct
        m_Weight = (m_Weight*m_Hits + GridPoint.m_Weight*GridPoint.m_Hits)/(m_Hits+GridPoint.m_Hits);
        // Finally add the hits:
        m_Hits += GridPoint.m_Hits;
      }
    
    } else if (m_Type == c_XStrip || m_Type == c_YStrip) {
      if ((m_Type == c_XStrip && m_xGrid == GridPoint.m_xGrid) || (m_Type == c_YStrip && m_yGrid == GridPoint.m_yGrid)) {
        
        // Position, ...
        m_Position.SetZ((m_Energy*m_Position.Z() + GridPoint.m_Position.Z()*GridPoint.m_Energy)/(m_Energy+GridPoint.m_Energy));
        // ... Energy (not before depth!),  
        m_Energy += GridPoint.m_Energy;
        // ... Time &
        if (GridPoint.m_Time < m_Time) {
          m_Time = GridPoint.m_Time;
        }
        
        // OriginIDs
        m_OriginIDs.insert(GridPoint.m_OriginIDs.begin(), GridPoint.m_OriginIDs.end());
        
        // Make sure that if m_Weight is some cts/sec normalization factor it stays correct
        m_Weight = (m_Weight*m_Hits + GridPoint.m_Weight*GridPoint.m_Hits)/(m_Hits+GridPoint.m_Hits);
        // Finally add the hits:
        m_Hits += GridPoint.m_Hits;
      }
    } else if (m_Type == c_GuardRing) {
      m_Energy += GridPoint.m_Energy;
      m_Position.SetXYZ(0.0, 0.0, 0.0);
      // Make sure that if m_Weight is some cts/sec normalization factor it stays correct
      m_Weight = (m_Weight*m_Hits + GridPoint.m_Weight*GridPoint.m_Hits)/(m_Hits+GridPoint.m_Hits);
      // Finally add the hits
      m_Hits += GridPoint.m_Hits;
      m_Time = 0;
      m_OriginIDs.clear();
    } else {
      merr<<"MDGridPoint::operator+=: Unknown grid point type: "<<m_Type<<endl;
    }
    
    // Some flags might be duplicate
    m_Flags += GridPoint.m_Flags;
    
    // If the new grid point is read out or above the trigger threshold then the old one is too
    if (GridPoint.m_IsAboveTriggerThreshold == true) {
      m_IsAboveTriggerThreshold = true;
    }
    if (GridPoint.m_IsReadOut == true) {
      m_IsReadOut = true;
    }
    
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Set most of the variables:
void MDGridPoint::Set(const unsigned int xGrid, 
                      const unsigned int yGrid, 
                      const unsigned int zGrid, 
                      const unsigned int Type, 
                      const MVector Position, 
                      const double Energy, 
                      const double Time,
                      const unsigned int Hits,
                      const double Weight)
{
  m_xGrid = xGrid;
  m_yGrid = yGrid;
  m_zGrid = zGrid;
  m_Type = Type;
  m_Position = Position;
  m_Energy = Energy;
  m_Time = Time;
  m_Hits = Hits;
  m_Weight = Weight;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MDGridPoint& GridPoint)
{
  os<<"Grid point (x="
    <<GridPoint.m_xGrid<<", y="
    <<GridPoint.m_yGrid<<", z="
    <<GridPoint.m_zGrid<<") at "
    <<GridPoint.m_Position<<" cm with "
    <<GridPoint.m_Energy<<" keV and "
    <<GridPoint.m_Hits<<" (original) hits after "
    <<GridPoint.m_Time<<" s with the weight "
    <<GridPoint.m_Weight<<" with Flags ("
    <<GridPoint.m_Flags<<") "
    <<(GridPoint.m_IsAboveTriggerThreshold ? "above" : "below")<<" trigger threshold and "
    <<(GridPoint.m_IsReadOut ? "IS" : "IS NOT")<<" read out (";
  if (GridPoint.m_Type == MDGridPoint::c_Voxel) {
    os<<"Voxel)"<<endl;
  } else if (GridPoint.m_Type == MDGridPoint::c_VoxelDrift) {
    os<<"VoxelDrift)"<<endl;
  } else if (GridPoint.m_Type == MDGridPoint::c_XStrip) {
    os<<"XStrip)"<<endl;
  } else if (GridPoint.m_Type == MDGridPoint::c_YStrip) {
    os<<"YStrip)"<<endl;
  } else if (GridPoint.m_Type == MDGridPoint::c_XYAnger) {
    os<<"XYAnger)"<<endl;
  } else if (GridPoint.m_Type == MDGridPoint::c_XYZAnger) {
    os<<"XYZAnger)"<<endl;
  } else if (GridPoint.m_Type == MDGridPoint::c_GuardRing) {
    os<<"GuardRing)"<<endl;
  } else {
    os<<"not (completely) implemented type)"<<endl;
  }

  return os;
}


// MDGridPoint.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

