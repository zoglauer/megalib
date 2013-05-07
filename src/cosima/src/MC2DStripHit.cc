/*
 * MC2DStripHit.cxx
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
#include "MC2DStripHit.hh"

// Geant4:
#include "globals.hh"

// Standard lib:
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// MEGAlib:
#include "MStreams.h"
#include "MDDetector.h"


/******************************************************************************/


G4Allocator<MC2DStripHit> MC2DStripHitAllocator;


/******************************************************************************
 * Only initializes all members to default (non-set) values
 */
MC2DStripHit::MC2DStripHit(bool Is3D) : MCVHit(), m_Is3D(Is3D)
{
  m_ADCCounts = 0;
  m_Position.set(0.0, 0.0, 0.0);
  m_IsGuardringHit = false;
  m_XStrip = -1;
  m_YStrip = -1;
  if (m_Is3D == true) {
    m_DetectorType = MDDetector::c_Strip3D;
  } else {
    m_DetectorType = MDDetector::c_Strip2D;
  }
}


/******************************************************************************
 * Nothing needs to be destructed:
 */
MC2DStripHit::~MC2DStripHit()
{
  // Intentionally left blank
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
MC2DStripHit::MC2DStripHit(const MC2DStripHit& Hit) : MCVHit()
{
  m_Is3D = Hit.m_Is3D;

  m_Energy = Hit.m_Energy;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;

  m_ADCCounts = Hit.m_ADCCounts;
  m_Position = Hit.m_Position;
  m_IsGuardringHit = Hit.m_IsGuardringHit;
  m_XStrip = Hit.m_XStrip;
  m_YStrip = Hit.m_YStrip;

  m_Origins = Hit.m_Origins;

  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
const MC2DStripHit& MC2DStripHit::operator=(const MC2DStripHit& Hit)
{
  m_Is3D = Hit.m_Is3D;

  m_Energy = Hit.m_Energy;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;

  m_ADCCounts = Hit.m_ADCCounts;
  m_Position = Hit.m_Position;
  m_IsGuardringHit = Hit.m_IsGuardringHit;
  m_XStrip = Hit.m_XStrip;
  m_YStrip = Hit.m_YStrip;

  m_Origins = Hit.m_Origins;

  m_VolumeHistory.resize(0);
  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;

  return *this;
}


/******************************************************************************
 * Compare two hits, whether they are within the same strips of the layer, i.e.
 * only strips and detector name are compared
 */
int MC2DStripHit::operator==(MC2DStripHit& Hit)
{
  if (m_XStrip != Hit.m_XStrip || m_YStrip != Hit.m_YStrip) {
    return false;
  }

  for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
    if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
      return false;
    }
  }
  
  if (m_IsGuardringHit == true && Hit.m_IsGuardringHit == true) {
    return true;
  }
  
  return true;
}


/******************************************************************************
 * Add the hit to the other one, when they are in the same layer/strips.
 * Otherwise do nothing!
 */
const MC2DStripHit& MC2DStripHit::operator+=(const MC2DStripHit& Hit)
{
  if (m_VolumeHistory.size() == Hit.m_VolumeHistory.size()) {
    bool IsSameVolume = true;
    for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
      if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
        IsSameVolume = false;
        break;
      }
    }
      
    if (IsSameVolume == true){
      if (m_IsGuardringHit == true && Hit.m_IsGuardringHit == true) {
        // Position stays the same
        m_Energy += Hit.m_Energy;
        m_ADCCounts += Hit.m_ADCCounts;
        AddOrigins(Hit.m_Origins);
        if (Hit.m_Time < m_Time) {
          m_Time = Hit.m_Time;
        }     
      } else if (m_XStrip == Hit.m_XStrip && m_YStrip == Hit.m_YStrip) {
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
  }

  return *this;
}


/******************************************************************************
 * Return a guard ring hit
 */
MSimGR* MC2DStripHit::GetGuardringCalibrated()
{
  if (m_IsGuardringHit == false) return 0;

  // Most of the stuff can be put into the base class
  MSimGR* GR = new MSimGR();
  GR->SetDetectorType(m_DetectorType);
  GR->SetPosition(MVector(m_Position.getX()/cm, m_Position.getY()/cm, m_Position.getZ()/cm));
  GR->SetEnergy(m_Energy/keV);

  return GR;
}


/******************************************************************************
 * Return a hit 
 */
MSimHT* MC2DStripHit::GetCalibrated()
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
void MC2DStripHit::Print()
{
  mout<<"Hit ";
  if (m_IsGuardringHit == true) mout<<"(guardring) "; 
  mout<<"in strip detector \""<<m_Name
      <<"\" at: ("<<m_Position.getX()/cm<<"/"<<m_Position.getY()/cm<<"/"<<m_Position.getZ()/cm
      <<")  ("<<m_XStrip<<", "<<m_YStrip<<")"
      <<" with "<<m_Energy/keV<<" keV ("<<(int) (m_ADCCounts+0.5)<<")"<<endl;
}


/*
 * MC2DStripHit.cc: the end...
 ******************************************************************************/










