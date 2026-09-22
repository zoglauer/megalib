/*
 * MCScintillatorHit.cc
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
#include "MCScintillatorHit.hh"

// Standard libs:
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


G4Allocator<MCScintillatorHit> MCScintillatorHitAllocator;


/******************************************************************************
 * Only initializes all members to default (non-set) values
 */
MCScintillatorHit::MCScintillatorHit() : MCVHit()
{
  m_ADCCounts = 0;
  m_DetectorType = MDDetector::c_Scintillator;
}



/******************************************************************************
 * Nothing needs to be destructed:
 */
MCScintillatorHit::~MCScintillatorHit()
{
  // Intentionally left blank
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
MCScintillatorHit::MCScintillatorHit(const MCScintillatorHit& Hit) : MCVHit()
{
  m_Energy = Hit.m_Energy;
  m_Position = Hit.m_Position;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;

  m_ADCCounts = Hit.m_ADCCounts;

  m_Origins = Hit.m_Origins;

  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;
}


/******************************************************************************
 * Assign all values of the left hand side hit to this one 
 */
const MCScintillatorHit& 
MCScintillatorHit::operator=(const MCScintillatorHit& Hit)
{
  m_Energy = Hit.m_Energy;
  m_Position = Hit.m_Position;
  m_Name = Hit.m_Name;
  m_DetectorType = Hit.m_DetectorType;

  m_ADCCounts = Hit.m_ADCCounts;

  m_Origins = Hit.m_Origins;

  m_VolumeHistory.resize(0);
  for (unsigned int i = 0; i < Hit.m_VolumeHistory.size(); ++i) {
    m_VolumeHistory[i] = Hit.m_VolumeHistory[i];
  }

  m_Time = Hit.m_Time;

  return *this;
}


/******************************************************************************
 * Compare to hits, whether they are within the detector, i.e.
 * only the detector names are compared
 */
int MCScintillatorHit::operator==(MCScintillatorHit& Hit)
{
  for (unsigned int h = 0; h < m_VolumeHistory.size(); ++h) {
    if (m_VolumeHistory[h] != Hit.m_VolumeHistory[h]) {
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Add the hit to the other one, when they are in the same detector.
 * Otherwise do noting!
 */
const MCScintillatorHit& 
MCScintillatorHit::operator+=(const MCScintillatorHit& Hit)
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
      m_Energy += Hit.m_Energy;
      // Important no += !
      m_Position = Hit.m_Position;
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
 * The printed info string looks like:
 * Hit in Scintillator "zefal" with 6546.3 keV
 */
void MCScintillatorHit::Print()
{
  mout<<"Hit in scintillator \""<<m_Name<<"\" at ("
      <<m_Position.getX()/cm<<", "
      <<m_Position.getY()/cm<<", "
      <<m_Position.getZ()/cm<<") "
      <<" with "<<m_Energy/keV<<" keV"<<endl;
}


/*
 * MCScintillatorHit.cc: the end...
 ******************************************************************************/










