/*
 * MSimCluster.cxx
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
// MSimCluster
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSimCluster.h"

// Standard libs:
#include <iostream>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MSimHT.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSimCluster)
#endif


////////////////////////////////////////////////////////////////////////////////


MSimCluster::MSimCluster()
{
  // Construct an instance of MSimCluster

  m_HTList.clear();
  m_Position = MVector(0.0, 0.0, 0.0);
  m_Energy = 0;
  m_Time = 0;
  m_VolumeSequence = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSimCluster::~MSimCluster()
{
  // Delete this instance of MSimCluster

  // Do *NOT* delete the hits, only the container object...
  delete m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimCluster::GetPosition()
{
  // Return the center of energy coordinates of this simulated cluster

  return m_Position;
}


////////////////////////////////////////////////////////////////////////////////


double MSimCluster::GetEnergy()
{
  // Return the total enrgy of this cluster

  return m_Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MSimCluster::GetTime()
{
  // Return the earliest this cluster

  return m_Time;
}


////////////////////////////////////////////////////////////////////////////////


int MSimCluster::GetDetector()
{
  // Return the detector this cluster is inside

  return m_Detector;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimCluster::GetNHTs()
{
  // Return the number of hits, this cluster consists of

  return m_HTList.size();
}


////////////////////////////////////////////////////////////////////////////////


MSimHT* MSimCluster::GetHTAt(unsigned int i)
{
  // Return the SimHT at position i in this clusters list:

  if (i < GetNHTs()) {
    return m_HTList[i];
  } else {
    merr<<"Index out of range!"<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MSimCluster::AddHT(MSimHT* HT)
{
  // Add a hit to this cluster and recompute the clusters parameter:

  if (m_HTList.size() != 0) {
    if (m_VolumeSequence->HasSameDetector(HT->GetVolumeSequence()) == false) {
      mout<<"Error: Not adding hit to cluster because they are in different detectors!"<<endl;
      return false;
    }
  } else {
    m_VolumeSequence = new MDVolumeSequence(*(HT->GetVolumeSequence()));
  }

  m_Position = (1.0/(m_Energy + HT->GetEnergy()))*(m_Energy*m_Position + HT->GetEnergy()*HT->GetPosition());
  m_Energy += HT->GetEnergy();
  
  m_Detector = HT->GetDetector();
  
  if (HT->GetTime() < m_Time) m_Time = HT->GetTime();
  
  vector<unsigned int> Origins = HT->GetOrigins();
  for (unsigned int o = 0; o < Origins.size(); ++o) {
    if (HasOrigin(Origins[o]) == false) {
      m_Origins.push_back(Origins[o]);
    }
  }
  m_HTList.push_back(HT);
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MSimCluster::HasHT(MSimHT* HT)
{
  //! Return if this hit is part of the cluster (pointer comparison only)
  
  for (unsigned int h = 0; h < m_HTList.size(); ++h) {
    if (HT == m_HTList[h]) {
      return true;
    }
  }
  
  return false;
}
  
  
////////////////////////////////////////////////////////////////////////////////


vector<unsigned int> MSimCluster::GetOrigins() const
{
  //! Return all origins

  return m_Origins;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimCluster::HasOrigin(unsigned int Origin)
{
  // 

  for (unsigned int o = 0; o < m_Origins.size(); ++o) {
    if (m_Origins[o] == Origin) return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence* MSimCluster::GetVolumeSequence()
{
  // Return the volume, in which this hit lies, or zero

  return m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


MSimHT* MSimCluster::CreateHT()
{  
  // Convert the content to a hit
  
  if (m_HTList.size() == 0) return nullptr;

  MSimHT* HT = new MSimHT(m_HTList[0]->GetGeometry());
  HT->Set(m_Detector, m_Position, m_Energy, m_Time, m_Origins, false);
  return HT;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimCluster::ToString()
{
  // Return the content of this class...

  ostringstream out;

  out<<"Cluster at ("<<m_Position[0]<<", "<<m_Position[1]<<", "<<m_Position[2]<<") ";
  out<<"with "<<m_Energy<<" keV ";
  out<<"and the hits:"<<endl;                
  for (unsigned int i = 0; i < GetNHTs(); i++) {
    out<<GetHTAt(i)->ToString()<<endl;
  }

  return out.str().c_str();
}


// MSimCluster.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
