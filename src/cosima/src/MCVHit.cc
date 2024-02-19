/*
 * MCVHit.cxx
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


// Standard lib:
#include <limits>
#include <algorithm>
using namespace std;

// Cosima:
#include "MCVHit.hh"
#include "MCCommon.hh"
#include "MStreams.h"
#include "MDDetector.h"

// Geant4:
#include "G4SystemOfUnits.hh"


/******************************************************************************/


/******************************************************************************
 * Initialize all members to default values
 */
MCVHit::MCVHit()
{
  m_Energy = 0;
  m_Position.set(0.0, 0.0, 0.0);
  m_Time = 0;
  m_Name = "Unknown";
  m_DetectorType = MDDetector::c_NoDetectorType;

  m_LastAddedOrigin = g_UnsignedIntNotDefined; 
}


/******************************************************************************
 * Nothing has to be destructed
 */
MCVHit::~MCVHit()
{
  // Intentionally left blank
}


/******************************************************************************
 * Add a track ID as origin to this hit
 */
void MCVHit::AddOrigin(unsigned int TrackId)
{
  // Since m_origin is a set, the origin is not added if it already exists!

  m_Origins.insert(TrackId);
}


/******************************************************************************
 * Add several origins to the list
 */
void MCVHit::AddOrigins(const set<unsigned int>& Origins)
{
  set<unsigned int>::const_iterator Iter;
  for (Iter = Origins.begin(); Iter != Origins.end(); ++Iter) {
    AddOrigin(*Iter);
  }
}


/******************************************************************************
 * Add an entry to the volume history
 */
void MCVHit::AddVolumeHistory(string Name)
{
  m_VolumeHistory.push_back(Name);
}

/******************************************************************************
 * Return a hit
 * Call this function in the derived class and fill the remaining data
 */
MSimHT* MCVHit::GetCalibrated()
{
  MSimHT* HT = new MSimHT();
  HT->Set(m_DetectorType, MVector(m_Position.getX()/cm, m_Position.getY()/cm, m_Position.getZ()/cm), m_Energy/keV, m_Time/s, m_Origins, false);

  return HT;
}


/*  
 * MCVHit.cc: the end...
 ******************************************************************************/
