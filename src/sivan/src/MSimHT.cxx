/*
 * MSimHT.cxx
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
// MSimHT
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSimHT.h"

// Standard libs:
#include <iomanip>
#include <iostream>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSimEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSimHT)
#endif


////////////////////////////////////////////////////////////////////////////////


MSimHT::MSimHT(MDGeometryQuest* Geo)
{
  // standard constructor

  m_Added = false;
  m_Geometry = Geo;

  m_DetectorType = -1;
  m_Energy = 0;
  m_Time = 0;
  m_OriginalEnergy = 0;

  m_Cluster = 0;
  m_VolumeSequence = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSimHT::MSimHT(const MSimHT& HT)
{
  // copy constructor

  m_DetectorType = HT.m_DetectorType;
  m_Position = HT.m_Position;
  m_Energy = HT.m_Energy;
  m_Time = HT.m_Time;

  m_OriginalPosition = HT.m_OriginalPosition;
  m_OriginalEnergy = HT.m_OriginalEnergy;

  m_Origins = HT.m_Origins;

  m_Added = HT.m_Added;

  m_Geometry = HT.m_Geometry;

  if (HT.m_VolumeSequence != 0) {
    m_VolumeSequence = new MDVolumeSequence(*(HT.m_VolumeSequence));
  } else {
    m_VolumeSequence = 0;
  }

  m_Cluster = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSimHT::~MSimHT()
{
  // standard destructor

  m_Cluster = 0;
  delete m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


void MSimHT::Set(const int Detector, const MVector& Position, const double Energy, const double Time, const vector<int>& Origins, bool NeedsNoising)
{
  //! Set all core data
  
  m_DetectorType = Detector;
  
  m_OriginalEnergy = Energy; 
  m_OriginalPosition = Position;
  m_OriginalTime = Time;
  
  m_Origins = Origins;
  
  if (NeedsNoising == true) {
    Noise();
  } else {
    m_Energy = Energy;
    m_Position = Position;
    m_Time = Time;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSimHT::Set(const int Detector, const MVector& Position, const double Energy, const double Time, const set<int>& Origins, bool NeedsNoising)
{
  //! Set all core data
  
  m_DetectorType = Detector;
  
  m_OriginalEnergy = Energy; 
  m_OriginalPosition = Position;
  m_OriginalTime = Time;
  
  m_Origins.clear();
  for (set<int>::const_iterator Iter = Origins.begin(); Iter != Origins.end(); ++Iter) {
    m_Origins.push_back(*Iter);
  }
  
  if (NeedsNoising == true) {
    Noise();
  } else {
    m_Energy = Energy;
    m_Position = Position;
    m_Time = Time;
  }
}

////////////////////////////////////////////////////////////////////////////////


bool MSimHT::AddRawInput(MString LineBuffer, int Version)
{
  // Analyze one line of text input...

  // Handle the most common MEGAlib 2.x sim / evta file versions
  if (Version == 25 && LineBuffer.BeginsWith("HTsim ")) {
    Version = 101;
  } else if (Version == 21 && LineBuffer.BeginsWith("HT ")) {
    Version = 200;
  }


  int Reads = 0;
  if (Version == 100) { // No time
    if (sscanf(LineBuffer.Data(), "HTsim %d;%lf;%lf;%lf;%lf",
               &m_DetectorType, &m_Position[0], &m_Position[1], &m_Position[2], &m_Energy) != 6) {
      mout<<"MSimHT: Unable to parse hit of version "<<Version<<": "<<LineBuffer<<endl;
      return false;
    }
    Reads = 5;
  } else if (Version == 101) { // with time 
    if (sscanf(LineBuffer.Data(), "HTsim %d;%lf;%lf;%lf;%lf;%lf",
               &m_DetectorType, &m_Position[0], &m_Position[1], &m_Position[2], &m_Energy, &m_Time) != 6) {
      mout<<"MSimHT: Unable to parse hit of version "<<Version<<": "<<LineBuffer<<endl;
      return false;
    }
    Reads = 6;
  } else if (Version == 200) { // no time, with uncertainties
    double dx,dy,dz,dE;
    if (sscanf(LineBuffer.Data(), "HT %d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
               &m_DetectorType, &m_Position[0], &m_Position[1], &m_Position[2], &m_Energy, &dx, &dy, &dz, &dE) != 9) {
      mout<<"MSimHT: Unable to parse hit of version "<<Version<<": "<<LineBuffer<<endl;
      return false;
    }
    m_Time = 0;
    Reads = 9;
  } else if (Version == 201) {// with time & uncertainties
    double dx, dy, dz, dE, dT;
    if (sscanf(LineBuffer.Data(), "HT %d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
               &m_DetectorType, &m_Position[0], &m_Position[1], &m_Position[2], &m_Energy, &m_Time, &dx, &dy, &dz, &dE, &dT) != 11) {
      mout<<"MSimHT: Unable to parse hit of version "<<Version<<": "<<LineBuffer<<endl;
      return false;
    }
    Reads = 10;
  } else {
    merr<<"Unknown version of sim/evta file (version: "<<Version<<"), please upgrade (or use old version of MEGAlib prior to 3.0)"<<endl;
    return false;
  }
  
  m_OriginalPosition = m_Position;
  m_OriginalEnergy = m_Energy;
  m_OriginalTime = m_Time;

  // Remove everything but the (variable) origin part...
  MString Origin(LineBuffer);
  for (int i = 0; i < Reads; ++i) {
    if (Origin.First(';') == MString::npos) {
      Origin = "";
    } else {
      Origin.Remove(0, Origin.First(';')+1);
    }
  }

  // Search for the number of origins:
  int OriginIA = 0;
  m_Origins.clear();
  while (Origin.First(';') != MString::npos) {
    sscanf((char *) Origin.Data(), "%d", &OriginIA);
    m_Origins.push_back(OriginIA);
    Origin.Remove(0, Origin.First(';')+1);
  }
  sscanf((char *) Origin.Data(), "%d", &OriginIA); 
  m_Origins.push_back(OriginIA);

  // Reset to calculate the volume sequence:
  if (Noise(true) == false) {
    // No error message here, because false also means below threshold
    // mout<<"MSimHT: Error during noising: "<<LineBuffer<<endl;
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimHT::ToString() const
{
  // Return information about this instance

  ostringstream out;
  out<<"Hit in D"<<m_DetectorType<<" ";
  out<<"at ("<<m_Position[0]<<", "<<m_Position[1]<<", "<<m_Position[2]<<") ";
  out<<"with "<<m_Energy<<" keV ";
  out<<"from:                 ";
  for (unsigned int i = 0; i < m_Origins.size(); ++i) {
    out<<m_Origins[i]<<", ";
  }

  return out;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimHT::ParseBinary(MBinaryStore& Store, const bool HasOrigins, const bool HasTime, const int OriginIDPrecision, const int BinaryPrecision, const int Version)
{
  //! Convert the content to binary
  
  //Reset();
  
  //m_DetectorType = Store.GetUInt8();
  if (BinaryPrecision == 32) {
    m_Position = Store.GetVectorFloat();
    m_Energy = Store.GetFloat();
    if (HasTime == true) {
      m_Time = Store.GetFloat();
    }
  } else {
    m_Position = Store.GetVectorDouble();
    m_Energy = Store.GetDouble();
    if (HasTime == true) {
      m_Time = Store.GetDouble();
    } 
  }
  
  if (HasOrigins == true) {
    if (OriginIDPrecision == 8) {
      uint8_t NOrigins = Store.GetUInt8();
      for (uint8_t i = 0; i < NOrigins; ++i) {
        m_Origins.push_back(Store.GetUInt8());
      }
    } else if (OriginIDPrecision == 16) {
      uint16_t NOrigins = Store.GetUInt16();
      for (uint16_t i = 0; i < NOrigins; ++i) {
        m_Origins.push_back(Store.GetUInt16());
      }
    } else {
      uint32_t NOrigins = Store.GetUInt32();
      for (uint32_t i = 0; i < NOrigins; ++i) {
        m_Origins.push_back(Store.GetUInt32());
      }
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimHT::ToBinary(MBinaryStore& Out, const bool StoreOrigins, const bool StoreTime, const int WhatToStore, const int OriginIDPrecision, const int BinaryPrecision, const int Version) {
  //! Convert the content to binary
  
  //Out.AddUInt8(m_DetectorType);
  if (BinaryPrecision == 32) {
    Out.AddVectorFloat(m_Position);
    Out.AddFloat(m_Energy);
    if (StoreTime == true) Out.AddFloat((float) m_Time);
  } else {
    Out.AddVectorDouble(m_Position);
    Out.AddDouble(m_Energy);
    if (StoreTime == true) Out.AddDouble(m_Time);
  }
  
  if (StoreOrigins == true) {
    if (OriginIDPrecision == 8) {
      Out.AddUInt8(m_Origins.size());
      for (unsigned int o = 0; o < m_Origins.size(); ++o) {
        Out.AddUInt8(m_Origins[o]);
      }
    } else if (OriginIDPrecision == 16) {
      Out.AddUInt16(m_Origins.size());
      for (unsigned int o = 0; o < m_Origins.size(); ++o) {
        Out.AddUInt16(m_Origins[o]);
      }
    } else {
      Out.AddUInt32(m_Origins.size());
      for (unsigned int o = 0; o < m_Origins.size(); ++o) {
        Out.AddUInt32(m_Origins[o]);
      }
    } 
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimHT::ToSimString(const int WhatToStore, const int ScientificPrecision, const int Version) const
{
  // Convert this SimEvent to the original *.sim file format...

  // If there is a significant change, make sure you make new sim version,
  // and update all reading functions

  ostringstream S;

  int WidthPos;
  int WidthEnergy;
  int WidthTime;
  int Precision;
  if (ScientificPrecision > 0) {
    WidthPos = ScientificPrecision+7;
    WidthEnergy = ScientificPrecision+6;
    WidthTime = ScientificPrecision+6;
    Precision = ScientificPrecision;
    S.setf(ios_base::scientific, ios_base::floatfield);
  } else {
    WidthPos = 10;
    WidthEnergy = 10;
    WidthTime = 11;
    Precision = 5;
    S.setf(ios_base::fixed, ios_base::floatfield);
  }

  S<<"HTsim "<<m_DetectorType<<";";
  S<<setprecision(Precision);
  S<<setw(WidthPos)<<m_Position[0]<<";";
  S<<setw(WidthPos)<<m_Position[1]<<";";
  S<<setw(WidthPos)<<m_Position[2]<<";";
  S<<setw(WidthEnergy)<<m_Energy<<";";
  S<<scientific;
  S<<setw(WidthTime)<<m_Time;
  if (WhatToStore == MSimEvent::c_StoreSimulationInfoAll) {
    for (unsigned int o = 0; o < m_Origins.size(); ++o) {
      S<<";"<<m_Origins[o];
    }
  }

  return S;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimHT::IsOrigin(const int Origin) const
{
  for (unsigned int i = 0; i < m_Origins.size(); ++i) {
    if (m_Origins[i] == Origin) {
      return true;
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


int MSimHT::GetOriginAt(const unsigned int i) const
{
  // Return the first entry of the origin list

  if (i < m_Origins.size()) {
    return m_Origins[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds! Max: "<<m_Origins.size()<<endl;
    massert(false);
    return numeric_limits<unsigned int>::max();
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimHT::GetNOrigins() const
{
  // Return the number of origins of his hit

  return m_Origins.size();
}


////////////////////////////////////////////////////////////////////////////////


vector<int> MSimHT::GetOrigins() const
{
  // Return all origins as a vector!

  return m_Origins;
}


////////////////////////////////////////////////////////////////////////////////


void MSimHT::AddOrigin(int i)
{
  // Add a new origin to this HT

  if (IsOrigin(i) == false) {
    m_Origins.push_back(i);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSimHT::SetOrigins(const set<int>& Origins)
{
  // Set all origins

  m_Origins.clear();
  for (set<int>::const_iterator Iter = Origins.begin(); Iter != Origins.end(); ++Iter) {
    m_Origins.push_back(*Iter);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSimHT::SetCluster(MSimCluster *Cluster)
{
  // Set the cluster, this hit belongs to
  
  m_Cluster = Cluster;
}


////////////////////////////////////////////////////////////////////////////////


MSimCluster* MSimHT::GetCluster()
{
  // Return the cluster, this hit belongs to or zero otherwise

  return m_Cluster;
}


////////////////////////////////////////////////////////////////////////////////


void MSimHT::SetVolumeSequence(MDVolumeSequence* VolumeSequence)
{
  // Set the volume, inside which this hit lies

  delete m_VolumeSequence;
  m_VolumeSequence = VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence* MSimHT::GetVolumeSequence()
{
  // Return the volume, in which this hit lies, or zero

  return m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


int MSimHT::GetSmallestOrigin(int Except) const
{
  // Return the smallest origin
  // if Except > 0 then ignore this origin
  // Return -1 if there is no smallest origin

  int Smallest = numeric_limits<int>::max();
  for (unsigned int i = 0; i < m_Origins.size(); ++i) {
    if (m_Origins[i] != Except) {
      if (m_Origins[i] < Smallest) {
        Smallest = m_Origins[i];
      }
    }
  }
  if (Smallest == numeric_limits<int>::max()) {
    Smallest = -1;
  }

  return Smallest;
}


////////////////////////////////////////////////////////////////////////////////


void MSimHT::OffsetOrigins(int Offset)
{
  for (unsigned int i = 0; i < m_Origins.size(); ++i) {
    m_Origins[i] += Offset;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MSimHT::Noise(bool RecalculateVolumeSequence) 
{ 
  // Recalculate the volume sequence:

  // Make sure we noise not twice:
  m_Position = m_OriginalPosition;
  m_Energy = m_OriginalEnergy;
  m_Time = m_OriginalTime;
  
  //cout<<"Before: "<<m_Energy;
  if (m_Geometry != 0) {
    if (RecalculateVolumeSequence == true) {
      delete m_VolumeSequence;
      m_VolumeSequence = m_Geometry->GetVolumeSequencePointer(m_Position, true, true);
      if (m_VolumeSequence->GetDetector() == 0) {
        mout<<"This hit at "<<m_Position<<" has no corresponding detector volume!"<<endl;
        mout<<"  --> It will not show up in the later analysis!"<<endl;
        return false;
      }
      if (m_VolumeSequence->GetSensitiveVolume() == 0) {
        mout<<"This hit at "<<m_Position<<" has no corresponding sensitive volume!"<<endl;
        mout<<"  --> It will not show up in the later analysis!"<<endl;
        return false;
      }
    }
    if (m_Geometry->GetActivateNoising() == true) {
      m_Geometry->Noise(m_Position, m_Energy, m_Time, *m_VolumeSequence);
      if (m_Energy <= 0.0) return false;
    }    
  }
  //cout<<"After "<<m_Energy<<endl;

  return true;
}


// MSimHT: the end...
////////////////////////////////////////////////////////////////////////////////
