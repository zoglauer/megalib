/*
 * MCAngerCameraHit.cc
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


// Cosima:
#include "MCCommon.hh"
#include "MCAngerCameraHit.hh"

// Standard lib:
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// Geant4:
#include "G4SystemOfUnits.hh"

// MEGAlib:
#include "MStreams.h"
#include "MDDetector.h"


/******************************************************************************/


G4Allocator<MCAngerCameraHit> MCAngerCameraHitAllocator;


/******************************************************************************
 * Only initializes all members to default (non-set) values
 */
MCAngerCameraHit::MCAngerCameraHit() : MCVHit()
{
  m_DetectorType = MDDetector::c_AngerCamera;
}



/******************************************************************************
 * Nothing needs to be destructed:
 */
MCAngerCameraHit::~MCAngerCameraHit()
{
  // Intentionally left blank
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
MCAngerCameraHit::MCAngerCameraHit(const MCAngerCameraHit& Hit) : MCVHit()
{
  m_Energy = Hit.m_Energy;
  m_Position = Hit.m_Position;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;


  m_Origins = Hit.m_Origins;

  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
const MCAngerCameraHit& MCAngerCameraHit::operator=(const MCAngerCameraHit& Hit)
{
  m_Energy = Hit.m_Energy;
  m_Position = Hit.m_Position;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;


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
int MCAngerCameraHit::operator==(MCAngerCameraHit& Hit)
{
  for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
    if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Add the hit to the other one, when they are in the same module/bars
 * Otherwise do nothing!
 */
const MCAngerCameraHit& MCAngerCameraHit::operator+=(const MCAngerCameraHit& Hit)
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
      m_Position = 
        (m_Energy*m_Position + Hit.m_Position*Hit.m_Energy)/
        (m_Energy+Hit.m_Energy);
      m_Energy += Hit.m_Energy;
      AddOrigins(Hit.m_Origins);
      if (Hit.m_Time < m_Time) {
        m_Time = Hit.m_Time;
      }    
    }
  }

  return *this;
}


/******************************************************************************
 * The printed information string look like:
 * ...
 */
void MCAngerCameraHit::Print()
{
  mout<<"Hit in Anger camera \""<<m_Name
      <<"\" at position: ("<<m_Position.getX()/cm<<"/"<<m_Position.getY()/cm<<"/"<<m_Position.getZ()/cm
      <<") with "<<m_Energy/keV<<" keV"<<endl;
}


/*
 * MCAngerCameraHit.cc: the end...
 ******************************************************************************/










