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

// MEGAlib
#include "MStreams.h"
#include "MDDetector.h"


/******************************************************************************/


/******************************************************************************
 * Initialize all members to default values
 */
MCVHit::MCVHit()
{
  m_Energy = 0;
  m_Time = 0;
  m_Name = "Unknown";
  m_DetectorType = MDDetector::c_NoDetectorType;

  m_LastAddedOrigin = -1; 
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
void MCVHit::AddOrigin(int TrackId)
{
  // Since m_origin is a set, the origin is not added if it already exists!

  m_Origins.insert(TrackId);
}


/******************************************************************************
 * Add several origins to the list
 */
void MCVHit::AddOrigins(const set<int>& Origins)
{
  set<int>::const_iterator Iter;
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
  HT->SetDetectorType(m_DetectorType);
  HT->SetEnergy(m_Energy/keV);
  HT->SetTime(m_Time/s);
  HT->SetOrigins(m_Origins);
  return HT;
}


/*  
 * MCVHit.cc: the end...
 ******************************************************************************/
