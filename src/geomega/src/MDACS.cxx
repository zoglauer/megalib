/*
 * MDACS.cxx
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
// MDACS
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDACS.h"
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
ClassImp(MDACS)
#endif


////////////////////////////////////////////////////////////////////////////////


MDACS::MDACS(MString String) : MDDetector(String)
{
  // default constructor

  m_Type = c_Scintillator;
  m_Description = c_ScintillatorName;
}


////////////////////////////////////////////////////////////////////////////////


MDACS::MDACS(const MDACS& A) : MDDetector(A)
{
  // Intentionally left empty
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDACS::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return new MDACS(*this);
}


////////////////////////////////////////////////////////////////////////////////


bool MDACS::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors

  return MDDetector::CopyDataToNamedDetectors();
}
  

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////


MDACS::~MDACS()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MDACS::SetDetectorVolume(MDVolume *Volume)
{
  // Set the volume which represents this detector
  // The volume does not need to be a box

  m_DetectorVolume = Volume;
  Volume->SetIsDetectorVolume(this);
}

////////////////////////////////////////////////////////////////////////////////


void MDACS::Noise(MVector& Pos, double& Energy, double& Time, MString& Flags, MDVolume* Volume) const
{

  if (m_NoiseActive == false) return;

  // Test for failure:
  if (gRandom->Rndm() < m_FailureRate) {
    Flags += " FAILURE";
    Energy = 0;
    return;
  }

  // Noise:
  ApplyEnergyResolution(Energy);

  // Overflow:
  if (ApplyOverflow(Energy) == true) {
    Flags += " OVERFLOW"; 
  }
  
  // Noise threshold:
  //if (ApplyNoiseThreshold(Energy) == true) {
  //  return;
  //}

  // Noise the time:
  ApplyTimeResolution(Time, Energy);

  // Now calibrate
  ApplyEnergyCalibration(Energy);
}


////////////////////////////////////////////////////////////////////////////////


vector<MDGridPoint> MDACS::Grid(const MVector& PosInDetectorVolume, 
                                const double& Energy, 
                                const double& Time, 
                                const MDVolume* DetectorVolume) const
{
  // Discretize Pos to a voxel of this detector

  vector<MDGridPoint> Points;
  Points.push_back(MDGridPoint(0, 0, 0, MDGridPoint::c_Voxel, DetectorVolume->GetShape()->GetUniquePosition(), Energy, Time));
 
  return Points;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDACS::GetGridPoint(const MVector& PosInDetectorVolume) const
{
  // Discretize Pos to a voxel of this detector

  return MDGridPoint(0, 0, 0, MDGridPoint::c_Voxel);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDACS::GetPositionInDetectorVolume(const unsigned int xGrid, 
                                           const unsigned int yGrid,
                                           const unsigned int zGrid,
                                           const MVector PositionInGrid,
                                           const unsigned int Type,
                                           const MDVolume* Volume) const
{
  // Return the position in the detector volume
  // 

  return Volume->GetShape()->GetUniquePosition();
}


////////////////////////////////////////////////////////////////////////////////


MString MDACS::GetGeomega() const
{
  // Return all detector characteristics in Geomega-Format

  ostringstream out;

  out<<"Scintillator "<<m_Name<<endl;
  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<m_Name<<".SensitiveVolume "<<m_SVs[i]->GetName()<<endl;
    out<<m_Name<<".DetectorVolume "<<m_SVs[i]->GetName()<<endl;
  }
  out<<GetGeomegaCommon(false, false)<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDACS::ToString() const
{
  //

  ostringstream out;
  out<<MDDetector::ToString()<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


bool MDACS::Validate()
{
  // Make sure everything is reasonable:

  if (MDDetector:: Validate() == false) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDACS::CreateBlockedTriggerChannelsGrid()
{
  // Create the grid only if it is really used

  m_BlockedTriggerChannels.Set(MDGrid::c_Voxel, 1, 1);
  m_AreBlockedTriggerChannelsUsed = true;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDACS::GetPositionResolution(const MVector& Pos, const double Energy) const
{
  // Get position resolution:

  if (m_SVs.size() == 1) {
    if (m_SVs[0]->GetShape()->GetType() == "BRIK") {
      return MVector(dynamic_cast<MDShapeBRIK*>(m_SVs[0]->GetShape())->GetSizeX(),
                     dynamic_cast<MDShapeBRIK*>(m_SVs[0]->GetShape())->GetSizeY(),
                     dynamic_cast<MDShapeBRIK*>(m_SVs[0]->GetShape())->GetSizeZ());
    }
  }

  return MVector(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


bool MDACS::IsVeto(const MVector& Position, const double Energy) const
{
  // Check if we have a veto

  // if (m_NoiseActive == false) return;

  // Test for failure:
  if (gRandom->Rndm() < m_FailureRate) {
    return false;
  }

  // Ignore if we are below threshold:
  if (Energy < gRandom->Gaus(m_TriggerThreshold, m_TriggerThresholdSigma)) {
    return false;
  }

  return true;
}


// MDACS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
