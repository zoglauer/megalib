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
#include <TMatrixD.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDVolume.h"
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
  
  m_Volumes.resize(V.m_Volumes.size());
  copy(V.m_Volumes.begin(), V.m_Volumes.end(), m_Volumes.begin());
  m_Positions.resize(V.m_Positions.size());
  copy(V.m_Positions.begin(), V.m_Positions.end(), m_Positions.begin());

  m_Detector = V.m_Detector;

  m_DetectorVolume = V.m_DetectorVolume;
  m_PositionInDetector = V.m_PositionInDetector;

  m_SensitiveVolume = V.m_SensitiveVolume;
  m_PositionInSensitiveVolume = V.m_PositionInSensitiveVolume;

  m_ValidRotation = V.m_ValidRotation;

  m_RotMatrix.ResizeTo(3,3);
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

  if (m_SensitiveVolume != 0 &&  VS.m_SensitiveVolume != 0) {
    if (m_SensitiveVolume->GetName() != VS.m_SensitiveVolume->GetName()) {
      m_Detector = 0;

      m_DetectorVolume = 0;
      m_PositionInDetector = g_VectorNotDefined;

      m_SensitiveVolume = 0;
      m_PositionInSensitiveVolume = g_VectorNotDefined;

      m_RotMatrix.ResizeTo(3,3);
      m_RotMatrix(0,0) = 1;
      m_RotMatrix(1,1) = 1;
      m_RotMatrix(2,2) = 1;
    } else if (m_Detector != 0 && VS.m_Detector!= 0) {
      if (m_Detector->GetName() != VS.m_Detector->GetName()) {
        m_Detector = 0;
        m_PositionInDetector = g_VectorNotDefined;

        m_SensitiveVolume = 0;
        m_PositionInSensitiveVolume = g_VectorNotDefined;
      }    
    } else {
      m_Detector = 0;
    }
  } else {
    m_DetectorVolume = 0;
    m_SensitiveVolume = 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::Reset()
{
  // Clear the content of this volume sequence

  m_Detector = 0;  

  m_DetectorVolume = 0;
  m_PositionInDetector = g_VectorNotDefined;

  m_SensitiveVolume = 0;
  m_PositionInSensitiveVolume = g_VectorNotDefined;

  m_ValidRotation = true;
  m_RotMatrix.ResizeTo(3,3);
  m_RotMatrix(0,0) = 1;
  m_RotMatrix(1,1) = 1;
  m_RotMatrix(2,2) = 1;

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


unsigned int MDVolumeSequence::GetNVolumes()
{
  //

  return m_Volumes.size();
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolumeSequence::GetVolumeAt(unsigned int i)
{
  // Return the clone at position i

  if (i < GetNVolumes()) {
    return m_Volumes[i];
  }
  
  merr<<"Index ("<<i<<") out of bounds (# volumes: "<<GetNVolumes()<<"). Returning zero pointer... Crash likely..."<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolumeSequence::GetDeepestVolume()
{
  if (GetNVolumes() == 0) return 0;

  return m_Volumes[GetNVolumes()-1];
}


////////////////////////////////////////////////////////////////////////////////


TMatrixD MDVolumeSequence::GetRotation()
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


MVector MDVolumeSequence::GetPositionAt(unsigned int i)
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

  massert(Detector != 0);
  m_Detector = Detector;
}


////////////////////////////////////////////////////////////////////////////////


MDDetector* MDVolumeSequence::GetDetector()
{
  // 

  return m_Detector;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::SetPositionInDetector(MVector Pos)
{
  //

  m_PositionInDetector = Pos;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolumeSequence::GetPositionInDetector()
{
  //
  
  return m_PositionInDetector;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDVolumeSequence::GetGridPoint() const
{
  //
  
  massert(m_Detector != 0);

  return m_Detector->GetGridPoint(m_PositionInDetector);
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::SetDetectorVolume(MDVolume* Volume)
{
  massert(Volume != 0);
  m_DetectorVolume = Volume;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolumeSequence::GetDetectorVolume()
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


MVector MDVolumeSequence::GetPositionInSensitiveVolume()
{
  //
  
  return m_PositionInSensitiveVolume;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolumeSequence::SetSensitiveVolume(MDVolume* Volume)
{
  massert(Volume != 0);
  m_SensitiveVolume = Volume;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolumeSequence::GetSensitiveVolume()
{
  return m_SensitiveVolume;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolumeSequence::HasVolume(MString Name) const
{
  unsigned int i_max = m_Volumes.size();
  for (unsigned int i = 0; i < i_max; i++) {
    if (m_Volumes[i]->GetName() == Name) return true; 
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolumeSequence::HasSameDetector(MDVolumeSequence& VS)
{
  return HasSameDetector(&VS);
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolumeSequence::HasSameDetector(MDVolumeSequence* VS)
{
  if (m_Detector == 0 || VS->m_Detector == 0) {
    mdebug<<"Not same detector: Zero pointer"<<endl;
    return false;
  }
  if (m_Detector->GetName() != VS->GetDetector()->GetName()) {
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
                                                   MDVolume* Volume)
{
  // Rotate "Position" from "Volume" to the first volume in the sequence
  // which is probably the world voluem...

  massert(Volume != 0);

  // Find the volume...
  unsigned int p = numeric_limits<unsigned int>::max();
  unsigned int i_max = m_Volumes.size();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_Volumes[i] == Volume) {
      p = i;
      break;
    } 
  }

  massert(p != numeric_limits<unsigned int>::max());

  MVector Pos = Position;

  for (unsigned int i = p; i <= p; --i) {
    Pos = m_Volumes[i]->GetInvRotationMatrix()*Pos;
    Pos += m_Volumes[i]->GetPosition();
  }

  return Pos;
}


////////////////////////////////////////////////////////////////////////////////


TMatrixD MDVolumeSequence::GetRotationInFirstVolume(MDVolume* Volume)
{
  // Return the rotation of a volume in the first (the world) volume

  mimp<<"Untested"<<endl;

  TMatrixD RotMatrix;
  RotMatrix.ResizeTo(3,3);
  RotMatrix(0,0) = 1;
  RotMatrix(1,1) = 1;
  RotMatrix(2,2) = 1;


  // Find the volume...
  unsigned int p = numeric_limits<unsigned int>::max();
  unsigned int i_max = m_Volumes.size();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_Volumes[i] == Volume) {
      p = i;
      break;
    } 
  }

  massert(p != numeric_limits<unsigned int>::max());

  for (unsigned int i = p; i <= p; --i) {
    RotMatrix *= m_Volumes[i]->GetInvRotationMatrix();
  }  

  return RotMatrix;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolumeSequence::GetPositionInVolume(const MVector& Position, 
                                              MDVolume* Volume)
{
  // Rotate "Position" from "Volume" to the first volume in the sequence
  // which is probably the world voluem...

  massert(Volume != 0);

  // Find the volume...
  unsigned int p = numeric_limits<unsigned int>::max();
  unsigned int i_max = m_Volumes.size();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_Volumes[i] == Volume) {
      p = i;
      break;
    } 
  }

  massert(p != numeric_limits<unsigned int>::max());

  // Then rotate&translate:
  MVector Pos = Position;
  for (unsigned int i = 0; i <= p; ++i) {
    Pos -= m_Volumes[i]->GetPosition();
    Pos = m_Volumes[i]->GetRotationMatrix()*Pos;
  }

  return Pos;
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolumeSequence::ToString()
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
  if (m_Detector != 0) {
    out<<m_Detector->GetName()<<" "<<m_PositionInDetector<<endl;
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


MString MDVolumeSequence::ToStringVolumes()
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
