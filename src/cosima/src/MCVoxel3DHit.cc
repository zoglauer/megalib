/*
 * MCVoxel3DHit.cxx
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
#include "MCVoxel3DHit.hh"

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
#include "MDDetector.h"


/******************************************************************************/


G4Allocator<MCVoxel3DHit> MCVoxel3DHitAllocator;


/******************************************************************************
 * Only initializes all members to default (non-set) values
 */
MCVoxel3DHit::MCVoxel3DHit() : MCVHit()
{
  m_ADCCounts = 0;
  m_IsGuardringHit = false;
  m_XVoxel = -1;
  m_YVoxel = -1;
  m_YVoxel = -1;
  m_DetectorType = MDDetector::c_Voxel3D;
}


/******************************************************************************
 * Nothing needs to be destructed:
 */
MCVoxel3DHit::~MCVoxel3DHit()
{
  // Intentionally left blank
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
MCVoxel3DHit::MCVoxel3DHit(const MCVoxel3DHit& Hit) : MCVHit()
{
  m_Energy = Hit.m_Energy;
  m_Position = Hit.m_Position;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;

  m_ADCCounts = Hit.m_ADCCounts;
  m_IsGuardringHit = Hit.m_IsGuardringHit;
  m_XVoxel = Hit.m_XVoxel;
  m_YVoxel = Hit.m_YVoxel;
  m_ZVoxel = Hit.m_ZVoxel;

  m_Origins = Hit.m_Origins;

  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
const MCVoxel3DHit& MCVoxel3DHit::operator=(const MCVoxel3DHit& Hit)
{
  m_Energy = Hit.m_Energy;
  m_Position = Hit.m_Position;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;

  m_ADCCounts = Hit.m_ADCCounts;
  m_IsGuardringHit = Hit.m_IsGuardringHit;
  m_XVoxel = Hit.m_XVoxel;
  m_YVoxel = Hit.m_YVoxel;
  m_ZVoxel = Hit.m_ZVoxel;

  m_Origins = Hit.m_Origins;

  m_VolumeHistory.resize(0);
  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;

  return *this;
}


/******************************************************************************
 * Compare two hits, whether they are within the same voxels of the layer, i.e.
 * only voxels and detector name are compared
 */
int MCVoxel3DHit::operator==(MCVoxel3DHit& Hit)
{
  for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
    if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
      return false;
    }
  }
  
  if (m_IsGuardringHit == true && Hit.m_IsGuardringHit == true) {
    return true;
  }

  if (m_XVoxel != Hit.m_XVoxel || m_YVoxel != Hit.m_YVoxel || m_ZVoxel != Hit.m_ZVoxel) {
    return false;
  }

  return true;
}


/******************************************************************************
 * Add the hit to the other one, when they are in the same layer/voxels.
 * Otherwise do nothing!
 */
const MCVoxel3DHit& MCVoxel3DHit::operator+=(const MCVoxel3DHit& Hit)
{
  if (m_VolumeHistory.size() == Hit.m_VolumeHistory.size()) {
    bool IsSameVolume = true;
    for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
      if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
        IsSameVolume = false;
        break;
      }
    }
      
    if (IsSameVolume == true) {
      if (m_IsGuardringHit == true && Hit.m_IsGuardringHit == true) {
        // Position stays the same
        m_Energy += Hit.m_Energy;
        m_ADCCounts += Hit.m_ADCCounts;
        AddOrigins(Hit.m_Origins);
        if (Hit.m_Time < m_Time) {
          m_Time = Hit.m_Time;
        }
      } else if (m_XVoxel == Hit.m_XVoxel && 
                 m_YVoxel == Hit.m_YVoxel && 
                 m_ZVoxel == Hit.m_ZVoxel) {
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
MSimGR* MCVoxel3DHit::GetGuardringCalibrated()
{
  if (m_IsGuardringHit == false) return nullptr;

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
MSimHT* MCVoxel3DHit::GetCalibrated()
{
  if (m_IsGuardringHit == true) return nullptr;

  return MCVHit::GetCalibrated();
}


/******************************************************************************
 * The printed information string look like:
 * Hit in Layer bachus near voxels 12/45, real: (3.6/6.4/12.4) with 231.4 keV
 */
void MCVoxel3DHit::Print()
{
  mout<<"Hit ";
  if (m_IsGuardringHit == true) mout<<"(guardring) "; 
  mout<<"in voxel detector \""<<m_Name
      <<"\" at: ("<<m_Position.getX()/cm<<"/"<<m_Position.getY()/cm<<"/"<<m_Position.getZ()/cm
      <<")  ("<<m_XVoxel<<", "<<m_YVoxel<<")"
      <<" with "<<m_Energy/keV<<" keV ("<<(int) (m_ADCCounts+0.5)<<")"<<endl;
}


/*
 * MCVoxel3DHit.cc: the end...
 ******************************************************************************/










