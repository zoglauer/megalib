/*
 * MCDriftChamberHit.cxx
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


// Cosima:
#include "MCCommon.hh"
#include "MCDriftChamberHit.hh"

// Geant4:
#include "G4SystemOfUnits.hh"
#include "globals.hh"

// Standard lib:
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// MEGAlib:
#include "MStreams.h"


/******************************************************************************/


G4Allocator<MCDriftChamberHit> MCDriftChamberHitAllocator;


/******************************************************************************
 * Only initializes all members to default (non-set) values
 */
MCDriftChamberHit::MCDriftChamberHit() : MC2DStripHit(true)
{
  // Intentionally left blank
}


/******************************************************************************
 * Nothing needs to be destructed:
 */
MCDriftChamberHit::~MCDriftChamberHit()
{
  // Intentionally left blank
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
MCDriftChamberHit::MCDriftChamberHit(const MCDriftChamberHit& Hit) : MC2DStripHit(Hit)
{
  // Is the base class copy constructor called??
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
const MCDriftChamberHit& MCDriftChamberHit::operator=(const MCDriftChamberHit& Hit)
{
  MC2DStripHit::operator=(Hit); // ??

  return *this;
}


/******************************************************************************
 * Compare two hits, whether they are within the same strips of the layer, i.e.
 * only strips and detector name are compared
 */
int MCDriftChamberHit::operator==(MCDriftChamberHit& Hit)
{
  for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
    if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
      return false;
    }
  }
  
  if (m_XStrip != Hit.m_XStrip || m_YStrip != Hit.m_YStrip) {
    return false;
  }

  return true;
}


/******************************************************************************
 * Add the hit to the other one, when they are in the same layer/strips.
 * Otherwise do nothing!
 */
const MCDriftChamberHit& MCDriftChamberHit::operator+=(const MCDriftChamberHit& Hit)
{
  if (m_VolumeHistory.size() == Hit.m_VolumeHistory.size()) {
    bool IsSameVolume = true;
    for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
      if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
        IsSameVolume = false;
        break;
      }
    }
      
    if (IsSameVolume == true && 
        m_XStrip == Hit.m_XStrip && m_YStrip == Hit.m_YStrip) {
      m_Position = 
        (m_Energy*m_Position + Hit.m_Position*Hit.m_Energy)/
        (m_Energy+Hit.m_Energy);
      m_Energy += Hit.m_Energy;
      m_ADCCounts += Hit.m_ADCCounts;
      AddOrigins(Hit.m_Origins);
      if (Hit.m_Time < m_Time) {
        m_Time = Hit.m_Time;
      }
    }
  }

  return *this;
}


/******************************************************************************
 * Return a hit 
 */
MSimHT* MCDriftChamberHit::GetCalibrated()
{
  if (m_IsGuardringHit == true) return 0;

  MSimHT* HT = MCVHit::GetCalibrated();
  HT->SetPosition(MVector(m_Position.getX()/cm, m_Position.getY()/cm, m_Position.getZ()/cm));
  return HT;
}


/******************************************************************************
 * The printed information string look like:
 * Hit in Layer bachus near strips 12/45, real: (3.6/6.4/12.4) with 231.4 keV
 */
void MCDriftChamberHit::Print()
{
  mout<<"Hit in strip detector \""<<m_Name
      <<"\" at: ("<<m_Position.getX()/cm<<"/"<<m_Position.getY()/cm<<"/"<<m_Position.getZ()/cm
      <<")  ("<<m_XStrip<<", "<<m_YStrip<<")"
      <<" with "<<m_Energy/keV<<" keV ("<<(int) (m_ADCCounts+0.5)<<")"<<endl;
}


/*
 * MCDriftChamberHit.cc: the end...
 ******************************************************************************/










