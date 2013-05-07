/*
 * MCCalorBarHit.cxx
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
#include "MCCalorBarHit.hh"

// Standard lib:
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// MEGAlib:
#include "MStreams.h"
#include "MDDetector.h"


/******************************************************************************/


G4Allocator<MCCalorBarHit> MCCalorBarHitAllocator;


/******************************************************************************
 * Only initializes all members to default (non-set) values
 */
MCCalorBarHit::MCCalorBarHit() : MCVHit()
{
  m_ADCCountsTop = 0;
  m_ADCCountsBottom = 0;
  m_Position.set(0.0, 0.0, 0.0);
  m_XBar = -1;
  m_YBar = -1;
  m_DetectorType = MDDetector::c_Calorimeter;
}



/******************************************************************************
 * Nothing needs to be destructed:
 */
MCCalorBarHit::~MCCalorBarHit()
{
  // Intentionally left blank
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
MCCalorBarHit::MCCalorBarHit(const MCCalorBarHit& Hit) : MCVHit()
{
  m_Energy = Hit.m_Energy;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;

  m_Position = Hit.m_Position;
  m_XBar = Hit.m_XBar;
  m_YBar = Hit.m_YBar;
  m_ADCCountsTop = Hit.m_ADCCountsTop;
  m_ADCCountsBottom = Hit.m_ADCCountsBottom;

  m_Origins = Hit.m_Origins;

  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
const MCCalorBarHit& MCCalorBarHit::operator=(const MCCalorBarHit& Hit)
{
  m_Energy = Hit.m_Energy;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;

  m_Position = Hit.m_Position;
  m_ADCCountsTop = Hit.m_ADCCountsTop;
  m_ADCCountsBottom = Hit.m_ADCCountsBottom;
  m_XBar = Hit.m_XBar;
  m_YBar = Hit.m_YBar;

  m_Origins = Hit.m_Origins;

  m_VolumeHistory.resize(0);
  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;

  return *this;
}


/******************************************************************************
 * Compare two hits, whether they are within the same bar of the module, i.e.
 * only bars and detector names are compared
 */
int MCCalorBarHit::operator==(MCCalorBarHit& Hit)
{
  if (m_XBar != Hit.m_XBar || m_YBar != Hit.m_YBar) {
    return false;
  }

  for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
    if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
      return false;
    }
  }
  
  return true;
}


/******************************************************************************
 * Add the hit to the other one, when they are in the same module/bars
 * Otherwise do noting!
 */
const MCCalorBarHit& MCCalorBarHit::operator+=(const MCCalorBarHit& Hit)
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
        m_XBar == Hit.m_XBar && m_YBar == Hit.m_YBar) {
      m_Position = 
        (m_Energy*m_Position + Hit.m_Position*Hit.m_Energy)/
        (m_Energy+Hit.m_Energy);
      m_Energy += Hit.m_Energy;
      m_ADCCountsTop += Hit.m_ADCCountsTop;
      m_ADCCountsBottom += Hit.m_ADCCountsBottom;
      if (m_ADCCountsBottom < 0) m_ADCCountsBottom = -1;
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
MSimHT* MCCalorBarHit::GetCalibrated()
{
  MSimHT* HT = MCVHit::GetCalibrated();
  HT->SetPosition(MVector(m_Position.getX()/cm, m_Position.getY()/cm, m_Position.getZ()/cm));
  return HT;
}


/******************************************************************************
 * The printed information string look like:
 * ...
 */
void MCCalorBarHit::Print()
{
  mout<<"Hit in calorimeter \""<<m_Name
      <<"\" at position: ("<<m_Position.getX()/cm<<"/"<<m_Position.getY()/cm<<"/"<<m_Position.getZ()/cm
      <<")  ("<<m_XBar<<", "<<m_YBar<<")"
      <<" with "<<m_Energy/keV<<" keV (";
  if (m_ADCCountsBottom  != -1) { 
    mout<<(G4int) (m_ADCCountsTop+0.5)<<", "<<(G4int) (m_ADCCountsBottom+0.5)<<")"<<endl;
  } else {
    mout<<(G4int) (m_ADCCountsTop+0.5)<<")"<<endl;
  }
}


/*
 * MCCalorBarHit.cc: the end...
 ******************************************************************************/










