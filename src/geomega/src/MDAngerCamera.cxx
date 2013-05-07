/*
 * MDAngerCamera.cxx
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
// MDAngerCamera
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDAngerCamera.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDAngerCamera)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDAngerCamera::c_PositionResolutionUnknown  = MDGridPoint::c_Unknown;
const int MDAngerCamera::c_PositionResolutionXY       = MDGridPoint::c_XYAnger;
const int MDAngerCamera::c_PositionResolutionXYZ      = MDGridPoint::c_XYZAnger;


////////////////////////////////////////////////////////////////////////////////


MDAngerCamera::MDAngerCamera(MString String) : MDDetector(String)
{
  // default constructor

  m_Type = c_AngerCamera;
  m_Description = c_AngerCameraName;
  m_PositionResolutionType = c_PositionResolutionUnknown;
}


////////////////////////////////////////////////////////////////////////////////


MDAngerCamera::MDAngerCamera(const MDAngerCamera& A) : MDDetector(A)
{
  m_PositionResolutionType = A.m_PositionResolutionType;
  m_PositionResolution = A.m_PositionResolution;
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDAngerCamera::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return new MDAngerCamera(*this);
}


////////////////////////////////////////////////////////////////////////////////


bool MDAngerCamera::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors

  MDDetector::CopyDataToNamedDetectors();
  
  if (m_IsNamedDetector == true) return true;
  
  for (unsigned int n = 0; n < m_NamedDetectors.size(); ++n) {
    if (dynamic_cast<MDAngerCamera*>(m_NamedDetectors[n]) == 0) {
      mout<<"   ***  Internal error  ***  in detector "<<m_Name<<endl;
      mout<<"We have a named detector ("<<m_NamedDetectors[n]->GetName()<<") which is not of the same type as the base detector!"<<endl;
      return false;
    }
    MDAngerCamera* D = dynamic_cast<MDAngerCamera*>(m_NamedDetectors[n]);
    
    if (D->m_PositionResolutionType == c_PositionResolutionUnknown && 
        m_PositionResolutionType != c_PositionResolutionUnknown) {
      D->m_PositionResolutionType = m_PositionResolutionType; 
      D->m_PositionResolution = m_PositionResolution; 
    }
  }
   
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


MDAngerCamera::~MDAngerCamera()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MDAngerCamera::SetPositionResolution(const double Energy, 
                                          const double Resolution)
{
  // Set a variable position resolution

  if (Energy < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy for position resolution needs to be positive!"<<endl;
    return; 
  }
  if (Resolution <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Position resolution needs to be positive!"<<endl;
    return; 
  }

  m_PositionResolution.Add(Energy, Resolution);
}


////////////////////////////////////////////////////////////////////////////////


void MDAngerCamera::Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const
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

  // Noise the position:
  MDShape* Shape = Volume->GetShape();
  if (Shape->IsInside(Pos, 1E-5) == false) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Position is not in the volume"<<endl;
    Energy = 0;
    return;
  }

  int Trials = 100;
  MVector NewPosition;
  do {
    if (m_PositionResolutionType == c_PositionResolutionXY) {
      // Randomly determine an angle:
      double Angle = 2*c_Pi*gRandom->Rndm();
      NewPosition.SetXYZ(cos(Angle), sin(Angle), 0.0);
      NewPosition *= gRandom->Gaus(0.0, m_PositionResolution.Evaluate(Energy));
      NewPosition += Pos;
    } else {
      // Randomly determine an angle:
      NewPosition.SetMagThetaPhi(gRandom->Gaus(0.0, m_PositionResolution.Evaluate(Energy)),
                                 c_Pi*gRandom->Rndm(), 
                                 2*c_Pi*gRandom->Rndm());
      NewPosition += Pos;
    }
  } while (Shape->IsInside(NewPosition, 1E-5) == false && Trials-- >= 0);
  if (Trials <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Couldn't find a good new (= noised) position --- not noising the position"<<endl;    
  }
  Pos = NewPosition;

  // Noise the time:
  ApplyTimeResolution(Time, Energy);

  // Now calibrate
  ApplyEnergyCalibration(Energy);
}


////////////////////////////////////////////////////////////////////////////////


vector<MDGridPoint> MDAngerCamera::Discretize(const MVector& PosInDetectorVolume, 
                                              const double& Energy, 
                                              const double& Time, 
                                              MDVolume* DetectorVolume) const
{
  // Discretize Pos to a voxel of this detector

  vector<MDGridPoint> Points;
  
  if (m_PositionResolutionType == c_PositionResolutionXY) {
    MVector Position = PosInDetectorVolume;
    Position.SetZ(0.0);
    Points.push_back(MDGridPoint(0, 0, 0, MDGridPoint::c_XYAnger, Position, Energy, Time));
  } else { 
    Points.push_back(MDGridPoint(0, 0, 0, MDGridPoint::c_XYZAnger, PosInDetectorVolume, Energy, Time));
  }
  return Points;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDAngerCamera::GetGridPoint(const MVector& PosInDetectorVolume) const
{
  // Discretize Pos to a voxel of this detector

  if (m_PositionResolutionType == c_PositionResolutionXY) {
    return MDGridPoint(0, 0, 0, MDGridPoint::c_XYAnger);
  } else {
    return MDGridPoint(0, 0, 0, MDGridPoint::c_XYZAnger);
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector MDAngerCamera::GetPositionInDetectorVolume(const unsigned int xGrid, 
                                                   const unsigned int yGrid,
                                                   const unsigned int zGrid,
                                                   const MVector PositionInGrid,
                                                   const unsigned int Type,
                                                   MDVolume* Volume)
{
  // Return the position in the detector volume

  return PositionInGrid;
}


////////////////////////////////////////////////////////////////////////////////


MString MDAngerCamera::GetGeant3() const
{
  ostringstream out;

  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<"      SENVOL("<<m_SVs[i]->GetSensitiveVolumeID()<<") = '"<<m_SVs[i]->GetShortName()<<"'"<<endl;
    out<<"      SENDET("<<m_SVs[i]->GetSensitiveVolumeID()<<") = "<<m_ID<<endl;
  }

  out<<"      DETNR("<<m_ID<<") = "<<m_Type<<endl;
  if (m_PositionResolutionType == c_PositionResolutionXY) {
    out<<"      DETTYP("<<m_ID<<") = 6"<<endl;
  } else if (m_PositionResolutionType == c_PositionResolutionXYZ) {
    out<<"      DETTYP("<<m_ID<<") = 7"<<endl;
  }
  out<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDAngerCamera::GetMGeant() const
{
  ostringstream out;


  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    MString Name = m_SVs[i]->GetShortName();
    Name.ToUpper();
    out<<"SENV "<<m_SVs[i]->GetSensitiveVolumeID()<<" "<<Name<<endl;
    out<<"SEND "<<m_SVs[i]->GetSensitiveVolumeID()<<" "<<m_ID<<endl;
  }

  out<<"DTNR "<<m_ID<<" "<<m_Type<<endl;
  if (m_PositionResolutionType == c_PositionResolutionXY) {
    out<<"DTTP "<<m_ID<<" 6"<<endl;
  } else if (m_PositionResolutionType == c_PositionResolutionXYZ) {
    out<<"DTTP "<<m_ID<<" 7"<<endl;
  }
  out<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDAngerCamera::GetGeomega() const
{
  // Return all detector characteristics in Geomega-Format

  ostringstream out;

  out<<"AngerCamera "<<m_Name<<endl;
  out<<GetGeomegaCommon()<<endl;
  if (m_PositionResolutionType == c_PositionResolutionXY) {
    out<<m_Name<<".Positioning XY"<<endl;
  } else {
    out<<m_Name<<".Positioning XYZ"<<endl;
  }
  for (unsigned int d = 0; d < m_PositionResolution.GetNDataPoints(); ++d) {
    out<<m_Name<<".PositionResolution "<<
      m_PositionResolution.GetDataPointX(d)<<" "<<
      m_PositionResolution.GetDataPointY(d)<<endl;
  }
  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDAngerCamera::ToString() const
{
  //

  ostringstream out;
  out<<MDDetector::ToString()<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


bool MDAngerCamera::Validate()
{
  // Make sure everything is reasonable:

  if (MDDetector::Validate() == false) {
    return false;
  }

  if (m_PositionResolutionType == c_PositionResolutionUnknown) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"No position resolution given!"<<endl;
    return false;
    
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDAngerCamera::GetPositionResolution(const MVector& Pos, const double Energy) const
{
  // Return the position resolution:
  
  return MVector(1.0/sqrt(3.0)*m_PositionResolution.Evaluate(Energy), 
                 1.0/sqrt(3.0)*m_PositionResolution.Evaluate(Energy),
                 1.0/sqrt(3.0)*m_PositionResolution.Evaluate(Energy));
}



// MDAngerCamera.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
