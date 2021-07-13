/*
 * MPhysicalEventHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPhysicalEventHit__
#define __MPhysicalEventHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MVector.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A hit in a physical event
//! Coming from revan it can be originally a cluster or the start point of a track
class MPhysicalEventHit
{
  // public interface:
 public:
  //! Default constructor
  MPhysicalEventHit();
  //! Copy constructor
  MPhysicalEventHit(const MPhysicalEventHit& H) { 
    m_Position = H.m_Position;
    m_PositionUncertainty = H.m_PositionUncertainty;
    m_Energy = H.m_Energy;
    m_EnergyUncertainty = H.m_EnergyUncertainty;
    m_Time = H.m_Time;
    m_TimeUncertainty = H.m_TimeUncertainty;
  }
  //! Default destuctor 
  virtual ~MPhysicalEventHit();

  //! Assignment operator 
  MPhysicalEventHit& operator= (const MPhysicalEventHit& H) { 
    m_Position = H.m_Position;
    m_PositionUncertainty = H.m_PositionUncertainty;
    m_Energy = H.m_Energy;
    m_EnergyUncertainty = H.m_EnergyUncertainty;
    m_Time = H.m_Time;
    m_TimeUncertainty = H.m_TimeUncertainty;
    return *this; 
  }
  
  //! Set all
  void Set(MVector Position, MVector PositionUncertainty, double Energy, double EnergyUncertainty, MTime Time, MTime TimeUncertainty) {
    m_Position = Position;
    m_PositionUncertainty = PositionUncertainty;
    m_Energy = Energy;
    m_EnergyUncertainty = EnergyUncertainty;
    m_Time = Time;
    m_TimeUncertainty = TimeUncertainty;
  }
  
  //! Get the position
  MVector GetPosition() const { return m_Position; }
  //! Get the position uncertainty
  MVector GetPositionUncertainty() const { return m_PositionUncertainty; }
  
  //! Get the energy
  double GetEnergy() const { return m_Energy; }
  //! Get the energy uncertainty
  double GetEnergyUncertainty() const { return m_EnergyUncertainty; }
  
  //! Get the time
  MTime GetTime() const { return m_Time; }
  //! Get the time uncertainty
  MTime GetTimeUncertainty() const { return m_TimeUncertainty; }
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The position
  MVector m_Position;
  //! The position uncertainty
  MVector m_PositionUncertainty;
  
  //! The energy
  double m_Energy;
  //! The energy uncertainty
  double m_EnergyUncertainty;
  
  //! The time
  MTime m_Time;
  //! The time uncertainty
  MTime m_TimeUncertainty;



#ifdef ___CLING___
 public:
  ClassDef(MPhysicalEventHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
