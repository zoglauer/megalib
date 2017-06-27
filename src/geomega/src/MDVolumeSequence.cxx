/*
 * MDVolumeSequence.cxx
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
// MDVolumeSequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDVolumeSequence.h"

// Standard libs:
#include <iostream>
#include <limits>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDVolume.h"
#include "MDDetector.h"
#include "MDGridPointCollection.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDVolumeSequence)
#endif


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence::MDVolumeSequence()
{
  // Construct an instance of MDVolumeSequence

  m_Volumes.reserve(10);
  m_Positions.reserve(10);

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence::~MDVolumeSequence()
{
  // Delete this instance of MDVolumeSequence
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence::MDVolumeSequence(const MDVolumeSequence& V)
{
  // Copy constructor - keep all volume pointers and positions!

  // This is a time critical function!
  
  // This is a tiny bit slower:
  // m_Volumes.resize(V.m_Volumes.size());
  // copy(V.m_Volumes.begin(), V.m_Volumes.end(), m_Volumes.begin());
  // m_Positions.resize(V.m_Positions.size());
  // copy(V.m_Positions.begin(), V.m_Positions.end(), m_Positions.begin());

  m_Volumes = V.m_Volumes;   
  m_Positions = V.m_Positions;
  
  m_Detector = V.m_Detector;
  m_NamedDetector = V.m_NamedDetector;

  m_DetectorVolume = V.m_DetectorVolume;
  m_PositionInDetector = V.m_PositionInDetector;

  m_SensitiveVolume = V.m_SensitiveVolume;
  m_PositionInSensitiveVolume = V.m_PositionInSensitiveVolume;

  m_ValidRotation = V.m_ValidRotation;
  m_RotMatrix = V.m_RotMatrix; 
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::Join(MDVolumeSequence VS)
{
  // Join two volume sequences:

  unsigned int MinSize;
  if (m_Volumes.size() > VS.GetNVolumes()) {
    MinSize = VS.GetNVolumes();
  } else {
    MinSize = m_Volumes.size();
  }

  m_Volumes.reserve(MinSize);
  m_Positions.reserve(MinSize);

  for (unsigned int i = 0; i < MinSize; ++i) {
    massert(VS.m_Volumes[i] != 0);
    massert(m_Volumes[i] != 0);
    if (m_Volumes[i]->GetName() != VS.m_Volumes[i]->GetName()) {
      // Shrink it:
      m_Volumes.resize(i);
      m_Positions.resize(i);
      break;
    } else {
      m_Positions[i] = 0.5*(m_Positions[i]+VS.m_Positions[i]);
    }
  }
  
  // Invalidate the rotation --- it will be regenerated next time:
  m_ValidRotation = false;
  m_RotMatrix.SetIdentity();    
  
  if (m_Detector == 0 || VS.m_Detector == 0 || m_Detector->GetName() != VS.m_Detector->GetName()) {
    m_Detector = 0;
    m_NamedDetector = 0;
      
    m_DetectorVolume = 0;
    m_PositionInDetector = g_VectorNotDefined;

    m_SensitiveVolume = 0;
    m_PositionInSensitiveVolume = g_VectorNotDefined;
  } else { // Detector names are equal
    massert(m_Detector->GetName() == VS.m_Detector->GetName());
    
    m_PositionInDetector = 0.5*(m_PositionInDetector + VS.m_PositionInDetector);
    
    if (m_SensitiveVolume == 0 || VS.m_SensitiveVolume == 0 || m_SensitiveVolume->GetName() != VS.m_SensitiveVolume->GetName()) {
      m_SensitiveVolume = 0;
      m_PositionInSensitiveVolume = g_VectorNotDefined;
    } else {
      m_PositionInSensitiveVolume = 0.5*(m_PositionInSensitiveVolume + VS.m_PositionInSensitiveVolume);      
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::Reset()
{
  // Clear the content of this volume sequence

  m_Detector = 0;  
  m_NamedDetector = 0;
  
  m_DetectorVolume = 0;
  m_PositionInDetector = g_VectorNotDefined;

  m_SensitiveVolume = 0;
  m_PositionInSensitiveVolume = g_VectorNotDefined;

  m_ValidRotation = false;
  m_RotMatrix.SetIdentity();

  m_Volumes.clear();
  m_Positions.clear();
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::AddVolume(MDVolume* Volume)
{
  //

  massert(Volume != 0);
  m_Volumes.push_back(Volume);
  m_ValidRotation = false;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::AddVolumeFront(MDVolume* Volume)
{
  //

  massert(Volume != 0);
  m_Volumes.insert(m_Volumes.begin(), Volume);
  m_ValidRotation = false;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDVolumeSequence::GetNVolumes() const
{
  //

  return m_Volumes.size();
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolumeSequence::GetVolumeAt(unsigned int i) const
{
  // Return the clone at position i

  if (i < GetNVolumes()) {
    return m_Volumes[i];
  }
  
  merr<<"Index ("<<i<<") out of bounds (# volumes: "<<GetNVolumes()<<"). Returning zero pointer... Crash likely..."<<endl;

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolumeSequence::GetDeepestVolume() const
{
  if (GetNVolumes() == 0) return nullptr;

  return m_Volumes[GetNVolumes()-1];
}


////////////////////////////////////////////////////////////////////////////////


MRotation MDVolumeSequence::GetRotation()
{
  // Return the rotation of the deepest volume relative to the world volume

  if (m_ValidRotation == false) {
    for (unsigned int i = 0; i < m_Volumes.size(); ++i) {
      m_RotMatrix = m_RotMatrix * m_Volumes[i]->GetRotationMatrix();
    }
    m_ValidRotation = true;
  }

  return m_RotMatrix;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::AddPosition(const MVector& V)
{
  m_Positions.push_back(V);
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::AddPositionFront(const MVector& V)
{
  m_Positions.insert(m_Positions.begin(), V);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolumeSequence::GetPositionAt(unsigned int i) const
{
  // Return the position at position i

  if (i < GetNVolumes()) {
    return m_Positions[i];
  }
  
  merr<<"Index ("<<i<<") out of bounds (# volumes: "<<GetNVolumes()<<"). Returning "<<g_VectorNotDefined<<". Wrong results guaranteed ;-)"<<endl;

  return g_VectorNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::SetDetector(MDDetector* Detector)
{
  //

  if (Detector == 0) {
    merr<<"Detector pointer is zero! But I will handle this gracefully..."<<endl;
    m_Detector = 0;
    m_NamedDetector = 0;
  } else if (Detector->IsNamedDetector() == true) {
    m_Detector = Detector->GetNamedAfterDetector();
    m_NamedDetector = Detector;
  } else {
    m_Detector = Detector;
    m_NamedDetector = m_Detector->FindNamedDetector(*this);
  }
  
  //cout<<"Det: "<<((m_Detector != 0) ? m_Detector->GetName() : "0")<<" Named Det: "<<((m_NamedDetector != 0) ? m_NamedDetector->GetName() : "0")<<endl;
}


////////////////////////////////////////////////////////////////////////////////


MDDetector* MDVolumeSequence::GetDetector() const
{
  // If a named detector exists returns the named detector otherwise the standard detector

  if (m_NamedDetector != nullptr) return m_NamedDetector;
  return m_Detector;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::SetPositionInDetector(MVector Pos)
{
  //

  m_PositionInDetector = Pos;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolumeSequence::GetPositionInDetector() const
{
  //
  
  return m_PositionInDetector;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDVolumeSequence::GetGridPoint() const
{
  //
  
  if (GetDetector() == nullptr) {
    return MDGridPoint(0, 0, 0, MDGridPoint::c_Unknown);
  }

  return GetDetector()->GetGridPoint(m_PositionInDetector);
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::SetDetectorVolume(MDVolume* Volume)
{
  m_DetectorVolume = Volume;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolumeSequence::GetDetectorVolume() const
{
  return m_DetectorVolume;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::SetPositionInSensitiveVolume(MVector Pos)
{
  //

  m_PositionInSensitiveVolume = Pos;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolumeSequence::GetPositionInSensitiveVolume() const
{
  //
  
  return m_PositionInSensitiveVolume;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::SetSensitiveVolume(MDVolume* Volume)
{
  m_SensitiveVolume = Volume;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolumeSequence::GetSensitiveVolume() const
{
  return m_SensitiveVolume;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolumeSequence::HasVolume(MString Name) const
{
  //! Return true if the volume is in the sequence

  for (MDVolume* V: m_Volumes) {
    if (V->GetName() == Name) return true; 
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolumeSequence::HasVolume(MDVolume* Volume) const
{
  //! Return true if the volume is in the sequence

  for (MDVolume* V: m_Volumes) {
    if (V == Volume) return true;
  }

  return false; 
}
  
  
////////////////////////////////////////////////////////////////////////////////


bool MDVolumeSequence::HasSameDetector(const MDVolumeSequence& VS) const
{
  if (GetDetector() == 0) {
    mdebug<<"HasSameDetector(): No - this VS has no detectors"<<endl;
    return false;
  }
  if (VS.GetDetector() == 0) {
    mdebug<<"HasSameDetector(): No - the tested VS has no detectors"<<endl;
    return false;
  }
  if (GetDetector()->GetName() != VS.GetDetector()->GetName()) {
    mdebug<<"HasSameDetector(): Different detector names"<<endl;
    return false;
  }

  unsigned int MinSize;
  if (m_Volumes.size() > VS.GetNVolumes()) {
    MinSize = VS.GetNVolumes();
  } else {
    MinSize = m_Volumes.size();
  }
  
  for (unsigned int i = 0; i < MinSize; i++) {
    if (m_Volumes[i]->GetName() == VS.GetVolumeAt(i)->GetName()) {
      if (m_Volumes[i] == m_DetectorVolume) {
        return true;
      }
    } else {
      break;
    } 
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolumeSequence::HasSameDetector(MDVolumeSequence* VS) const
{
  if (GetDetector() == 0 || VS->GetDetector() == 0) {
    mdebug<<"Not same detector: Zero pointer"<<endl;
    return false;
  }
  if (GetDetector()->GetName() != VS->GetDetector()->GetName()) {
    mdebug<<"Not same detector: Different names"<<endl;
    return false;
  }

  unsigned int MinSize;
  if (m_Volumes.size() > VS->GetNVolumes()) {
    MinSize = VS->GetNVolumes();
  } else {
    MinSize = m_Volumes.size();
  }
  
  for (unsigned int i = 0; i < MinSize; i++) {
    if (m_Volumes[i]->GetName() == VS->m_Volumes[i]->GetName()) {
      if (m_Volumes[i] == m_DetectorVolume) {
        return true;
      }
    } else {
      break;
    } 
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolumeSequence::GetPositionInFirstVolume(const MVector& Position, 
                                                   MDVolume* Volume) const
{
  // Rotate "Position" from "Volume" to the first volume in the sequence
  // which is probably the world voluem...

  if (Volume == nullptr) return g_VectorNotDefined;

  // Find the volume...
  unsigned int p = numeric_limits<unsigned int>::max();
  unsigned int i_max = m_Volumes.size();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_Volumes[i] == Volume) {
      p = i;
      break;
    } 
  }

  if (p == numeric_limits<unsigned int>::max()) return g_VectorNotDefined;

  MVector Pos = Position;

  for (unsigned int i = p; i <= p; --i) {
    Pos = m_Volumes[i]->GetInvRotationMatrix()*Pos;
    Pos += m_Volumes[i]->GetPosition();
  }

  return Pos;
}


////////////////////////////////////////////////////////////////////////////////


MRotation MDVolumeSequence::GetRotationInFirstVolume(MDVolume* Volume) const
{
  // Return the rotation of a volume in the first (the world) volume

  if (Volume == nullptr) return g_RotationNotDefined;


  // Find the volume...
  unsigned int p = numeric_limits<unsigned int>::max();
  unsigned int i_max = m_Volumes.size();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_Volumes[i] == Volume) {
      p = i;
      break;
    } 
  }

  if (p == numeric_limits<unsigned int>::max()) return g_RotationNotDefined;

  MRotation RotMatrix;
  for (unsigned int i = p; i <= p; --i) {
    RotMatrix *= m_Volumes[i]->GetInvRotationMatrix();
  }  

  return RotMatrix;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolumeSequence::GetPositionInVolume(const MVector& Position, MDVolume* Volume) const
{
  // Rotate "Position" from "Volume" to the first volume in the sequence
  // which is probably the world voluem...

  if (Volume == nullptr) return g_VectorNotDefined;

  // Find the volume...
  unsigned int p = numeric_limits<unsigned int>::max();
  unsigned int i_max = m_Volumes.size();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_Volumes[i] == Volume) {
      p = i;
      break;
    } 
  }

  if (p == numeric_limits<unsigned int>::max()) return g_VectorNotDefined;

  // Then rotate&translate:
  MVector Pos = Position;
  for (unsigned int i = 0; i <= p; ++i) {
    Pos -= m_Volumes[i]->GetPosition();
    Pos = m_Volumes[i]->GetRotationMatrix()*Pos;
  }

  return Pos;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolumeSequence::GetUniqueWorldPositionOfDetector() const
{
  // Return a unique world position of the detector volume
  // This position allows to distinguish different detectors even if they are clones/copies
  // Return g_VectorNotDefined in case of error

  if (m_Detector != nullptr && m_DetectorVolume != nullptr && HasVolume(m_DetectorVolume) == true) {
    MVector Position = m_DetectorVolume->GetShape()->GetUniquePosition();
    return GetPositionInFirstVolume(Position, m_DetectorVolume);
  }

  return g_VectorNotDefined;
}  

  
////////////////////////////////////////////////////////////////////////////////


MString MDVolumeSequence::ToString() const
{
  // 

  ostringstream out;

  out<<"Volume Sequence: "<<endl;

  out<<"  * Volumes: ";
  if (m_Volumes.size() > 0) {
    for (unsigned int i = 0; i < m_Volumes.size(); i++) {
      out<<m_Volumes[i]->GetName();
      if (i < m_Volumes.size()-1) {
        out<<" --> ";
      } else {
        out<<endl;
      }
    }
  } else {
    out<<"none defined!"<<endl;
  }

  if (m_Volumes.size() != 0) {
    out<<"  * Last volumes material: "<<m_Volumes.back()->GetMaterial()->GetName()<<endl;
    out<<"  * Last volumes type: "<<m_Volumes.back()->GetShape()->ToString();
  }
  out<<"  * Detector: ";
  if (GetDetector() != 0) {
    out<<GetDetector()->GetName()<<" "<<m_PositionInDetector<<endl;
  } else {
    out<<"none defined!"<<endl;
  }
  out<<"  * Sensitive volume: ";
  if (m_SensitiveVolume != 0) {
    out<<m_SensitiveVolume->GetName()<<" "<<m_PositionInSensitiveVolume<<endl;
  } else {
    out<<"none defined!"<<endl;
  }
  out<<"  * Detector volume: ";
  if (m_DetectorVolume != 0) {
    out<<m_DetectorVolume->GetName()<<" "<<m_PositionInDetector<<endl;
  } else {
    out<<"none defined!"<<endl;
  }

  return out.str().c_str(); 
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolumeSequence::ToStringVolumes() const
{
  // Return only the real sequence of volumes as one line without return

  ostringstream out;

  if (m_Volumes.size() > 0) {
    for (unsigned int i = 0; i < m_Volumes.size(); i++) {
      out<<m_Volumes[i]->GetName();
      if (i < m_Volumes.size()-1) {
        out<<" --> ";
      } 
    }
  } else {
    out<<"No volumes defined";
  }

  return out.str().c_str(); 
}


// MDVolumeSequence.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
