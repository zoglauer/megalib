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


#ifdef ___CINT___
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


MDACS::MDACS(const MDACS& A)
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


void MDACS::Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const
{

  if (m_NoiseActive == false) return;

	// Test for failure:
	if (gRandom->Rndm() < m_FailureRate) {
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


vector<MDGridPoint> MDACS::Discretize(const MVector& PosInDetectorVolume, 
                                      const double& Energy, 
                                      const double& Time, 
                                      MDVolume* DetectorVolume) const
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
                                           MDVolume* Volume)
{
  // Return the position in the detector volume
  // 

  return Volume->GetShape()->GetUniquePosition();
}


////////////////////////////////////////////////////////////////////////////////


MString MDACS::GetGeant3() const
{
  ostringstream out;

  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<"      SENVOL("<<m_SVs[i]->GetSensitiveVolumeID()<<") = '"<<m_SVs[i]->GetShortName()<<"'"<<endl;
    out<<"      SENDET("<<m_SVs[i]->GetSensitiveVolumeID()<<") = "<<m_ID<<endl;
  }

  out<<"      DETNR("<<m_ID<<") = "<<4<<endl;
  out<<"      DETTYP("<<m_ID<<") = 5"<<endl;
  out<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDACS::GetMGeant() const
{
  ostringstream out;

  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    MString Name = m_SVs[i]->GetShortName();
    Name.ToUpper();
    out<<"SENV "<<m_SVs[i]->GetSensitiveVolumeID()<<" "<<Name<<endl;
    out<<"SEND "<<m_SVs[i]->GetSensitiveVolumeID()<<" "<<m_ID<<endl;
    out<<"SEUP "<<m_SVs[i]->GetSensitiveVolumeID()<<" "
       <<m_SVs[i]->GetShape()->GetUniquePosition().GetX()<<" "
       <<m_SVs[i]->GetShape()->GetUniquePosition().GetY()<<" "
       <<m_SVs[i]->GetShape()->GetUniquePosition().GetZ()<<endl;
  }

  out<<"DTNR "<<m_ID<<" "<<4<<endl;
  out<<"DTTP "<<m_ID<<" 5"<<endl;
  out<<endl;

  return out.str().c_str();  
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

  unsigned int i;
  ostringstream out;

  out<<"Detector "<<m_Name<<" - Scintillator detector"<<endl;
  out<<"   with sensitive volumes: ";  
  for (i = 0; i < m_SVs.size(); i++) {
    out<<m_SVs[i]->GetName()<<" ";
  }

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


bool MDACS::Validate()
{
  // Make sure everything is reasonable:


  if (m_DetectorVolume == 0 && m_SVs.size() == 1) {
    SetDetectorVolume(m_SVs[0]);
  }
  if (m_SVs.size() == 1 && m_CommonVolume == 0) {
    m_CommonVolume = m_DetectorVolume;
  }

  if (m_DetectorVolume == 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Detector has no detector volume!"<<endl;
    return false;
  }
  if (m_DetectorVolume->IsVirtual() == true) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Detector volume is not allowed to be virtual!"<<endl;
    return false;
  }
  
  if (m_SVs.size() == 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Detector has no sensitive volume!"<<endl;
    return false;
  }
//   for (unsigned int i = 0; i < m_SVs.size(); ++i) {
//     if (m_SVs[i]->IsInside(m_HitPositions[m_SVs[i]]) == false) {
//       mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
//       mout<<"The hit position "<<m_HitPositions[m_SVs[i]]
// 	  <<" is not in sensitive material!"<<endl;
//       return false;
//     }
//   }

  for (unsigned int i = 0; i < m_SVs.size(); ++i) {
    if (m_SVs[i]->IsVirtual() == true) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"The sensitive volume is not allowed to be virtual!"<<endl;
      return false;
    }
  }

  if (m_EnergyResolutionType != c_EnergyResolutionTypeIdeal) {
    if (m_EnergyResolutionPeak1.GetSize() < 2 && m_EnergyResolutionType != c_EnergyResolutionTypeUnknown) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"Please give at least two data points for the energy resolution or state that you don't want an energy resolution."<<endl;
      return false;        
    }
    if (m_EnergyResolutionPeak1.GetSize() == 0) {
      mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
      mout<<"No energy resolution given. Assuming ideal."<<endl;
      mout<<"You might add a statement like \""<<m_Name<<".EnergyResolution Ideal\" to your file."<<endl;
      m_EnergyResolutionType = c_EnergyResolutionTypeIdeal;
    }
  }

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
