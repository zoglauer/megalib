/*
 * MDGuardRing.cxx
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
// MDGuardRing
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDGuardRing.h"
#include "MDShapeBRIK.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDGuardRing)
#endif


////////////////////////////////////////////////////////////////////////////////


MDGuardRing::MDGuardRing(MString String) : MDDetector(String)
{
  // default constructor

  m_Type = c_GuardRing;
  m_Description = c_GuardRingName;
  
  m_MotherDetector = nullptr;
  m_UniquePosition = g_VectorNotDefined;
  
  // By default the guard ring is OFF
  m_IsActive = false;
}


////////////////////////////////////////////////////////////////////////////////


MDGuardRing::MDGuardRing(const MDGuardRing& A) : MDDetector(A)
{
  // Copy constructor 
  
  m_MotherDetector = nullptr; // Mother detector cannot be the same
  m_IsActive = A.m_IsActive;
  m_UniquePosition = A.m_UniquePosition;
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDGuardRing::Clone()
{
  // Duplicate this detector

  return new MDGuardRing(*this);
}


////////////////////////////////////////////////////////////////////////////////


bool MDGuardRing::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors

  mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
  mout<<"A guard ring cannot have named detectors, since it is always part of a strip detector"<<endl;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


MDGuardRing::~MDGuardRing()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MDGuardRing::Noise(MVector& Pos, double& Energy, double& Time, MString& Flags, MDVolume* Volume) const
{
  // If we are not active, then we return no energy
  if (m_IsActive == false) {
    Energy = 0;
    return;
  }
  
  if (m_NoiseActive == false) return;
  
  // Test for failure:SetNoiseThresholdEqualsTriggerThreshold
  if (m_FailureRate > 0 && gRandom->Rndm() < m_FailureRate) {
    Energy = 0;
    return;
  }

  // Noise:
  ApplyEnergyResolution(Energy);

  // Overflow:
  ApplyOverflow(Energy);
  
  // Noise threshold:
  if (ApplyNoiseThreshold(Energy) == true) {
    return;
  }

  // Noise the time:
  ApplyTimeResolution(Time, Energy);

  // Now calibrate
  ApplyEnergyCalibration(Energy);
}


////////////////////////////////////////////////////////////////////////////////


vector<MDGridPoint> MDGuardRing::Grid(const MVector& PosInDetectorVolume, 
                                      const double& Energy, 
                                      const double& Time, 
                                      const MDVolume* DetectorVolume) const
{
  // Discretize Pos to a voxel of this detector

  vector<MDGridPoint> Points;
  Points.push_back(MDGridPoint(0, 0, 0, MDGridPoint::c_GuardRing, m_UniquePosition, Energy, Time));
 
  //cout<<"Guard ring gridding"<<endl;
  
  return Points;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDGuardRing::GetGridPoint(const MVector& PosInDetectorVolume) const
{
  // Discretize Pos to a voxel of this detector

  return MDGridPoint(0, 0, 0, MDGridPoint::c_GuardRing);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDGuardRing::GetPositionInDetectorVolume(const unsigned int xGrid, 
                                           const unsigned int yGrid,
                                           const unsigned int zGrid,
                                           const MVector PositionInGrid,
                                           const unsigned int Type,
                                           const MDVolume* Volume) const
{
  // Return the position in the detector volume
  // 

  return m_UniquePosition;
}


////////////////////////////////////////////////////////////////////////////////


MString MDGuardRing::GetGeomega() const
{
  // Part of MDStrip
  
  return "";
}


////////////////////////////////////////////////////////////////////////////////


MString MDGuardRing::ToString() const
{
  //

  ostringstream out;
  out<<MDDetector::ToString()<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


bool MDGuardRing::Validate()
{
  // Make sure everything is reasonable:
  
  // Verify that we have a mother
  if (m_MotherDetector == nullptr) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"The mother detector (i.e. the strip or voxel detector) of the guard ring has not been set!"<<endl;
    return false;      
  }
  
  // Copy the sensitive volumes from the mother
  if (m_SVs.size() == 0) {
    for (unsigned int s = 0; s < m_MotherDetector->GetNSensitiveVolumes(); ++s) {
      m_SVs.push_back(m_MotherDetector->GetSensitiveVolume(s));
    }
  }
  
  if (m_DetectorVolume == nullptr) {
    // We intentionally do not set the guard ring as the detector in the volume, since this has to remain the actual strip or voxel detector
    m_DetectorVolume = m_SVs[0];
  }
    
  if (m_IsActive == false) {
    SetEnergyResolutionType(c_EnergyResolutionTypeNone);
    SetTriggerThreshold(numeric_limits<double>::max()/10);
    SetNoiseThresholdEqualsTriggerThreshold(true);
  }
    
    
  if (MDDetector::Validate() == false) return false;

  // Verify that we have a unique position
  if (m_UniquePosition == g_VectorNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"No unique guard ring position given!"<<endl;
    return false;        
  }
  
  return true;
}


// MDGuardRing.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
