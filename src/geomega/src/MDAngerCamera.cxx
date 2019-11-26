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


#ifdef ___CLING___
ClassImp(MDAngerCamera)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDAngerCamera::c_PositionResolutionUnknown        = MDGridPoint::c_Unknown;
const int MDAngerCamera::c_PositionResolutionXY             = MDGridPoint::c_XYAnger;
const int MDAngerCamera::c_PositionResolutionXYZ            = MDGridPoint::c_XYZAnger;
const int MDAngerCamera::c_PositionResolutionXYZIndependent = MDGridPoint::c_XYZIndependentAnger;


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
  m_PositionResolutionX = A.m_PositionResolutionX;
  m_PositionResolutionY = A.m_PositionResolutionY;
  m_PositionResolutionZ = A.m_PositionResolutionZ;
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
      D->m_PositionResolutionX = m_PositionResolutionX; 
      D->m_PositionResolutionY = m_PositionResolutionY; 
      D->m_PositionResolutionZ = m_PositionResolutionZ; 
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


void MDAngerCamera::SetPositionResolutionXYZ(const double Energy, 
                                             const double ResolutionX,
                                             const double ResolutionY,
                                             const double ResolutionZ)
{
  // Set a variable position resolution
  
  if (Energy < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy for position resolution needs to be positive!"<<endl;
    return; 
  }
  if (ResolutionX <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Position resolution X needs to be positive!"<<endl;
    return; 
  }
  if (ResolutionY <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Position resolution Y needs to be positive!"<<endl;
    return; 
  }
  if (ResolutionZ <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Position resolution Z needs to be positive!"<<endl;
    return; 
  }
  
  m_PositionResolutionX.Add(Energy, ResolutionX);
  m_PositionResolutionY.Add(Energy, ResolutionY);
  m_PositionResolutionZ.Add(Energy, ResolutionZ);
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
    } else if (m_PositionResolutionType == c_PositionResolutionXYZ) {
      // Randomly determine an angle:
      NewPosition.SetMagThetaPhi(gRandom->Gaus(0.0, m_PositionResolution.Evaluate(Energy)),
                                 c_Pi*gRandom->Rndm(), 
                                 2*c_Pi*gRandom->Rndm());
      NewPosition += Pos;
    } else if (m_PositionResolutionType == c_PositionResolutionXYZIndependent) {
      // X, Y & Z axes are independently measured and independent Gaussians:
      NewPosition.SetXYZ(gRandom->Gaus(0.0, m_PositionResolutionX.Evaluate(Energy)),
                         gRandom->Gaus(0.0, m_PositionResolutionY.Evaluate(Energy)),
                         gRandom->Gaus(0.0, m_PositionResolutionZ.Evaluate(Energy)));
      
      // Add the original position
      NewPosition += Pos;
    }
  } while (Shape->IsInside(NewPosition, 1E-5) == false && Trials-- >= 0);
  if (Trials <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Couldn't find a good new (= noised) position --- not noising the position"<<endl;    
  }
  
  // Set as hit position
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
  } else if (m_PositionResolutionType == c_PositionResolutionXYZ) { 
    Points.push_back(MDGridPoint(0, 0, 0, MDGridPoint::c_XYZAnger, PosInDetectorVolume, Energy, Time));
  } else { 
    Points.push_back(MDGridPoint(0, 0, 0, MDGridPoint::c_XYZIndependentAnger, PosInDetectorVolume, Energy, Time));
  }
  return Points;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDAngerCamera::GetGridPoint(const MVector& PosInDetectorVolume) const
{
  // Discretize Pos to a voxel of this detector

  if (m_PositionResolutionType == c_PositionResolutionXY) {
    return MDGridPoint(0, 0, 0, MDGridPoint::c_XYAnger);
  } else if (m_PositionResolutionType == c_PositionResolutionXYZ) {
    return MDGridPoint(0, 0, 0, MDGridPoint::c_XYZAnger);
  } else {
    return MDGridPoint(0, 0, 0, MDGridPoint::c_XYZIndependentAnger);
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
  
  if (m_PositionResolutionType == c_PositionResolutionXY) {
    return MVector(1.0/sqrt(2.0)*m_PositionResolution.Evaluate(Energy), 
                   1.0/sqrt(2.0)*m_PositionResolution.Evaluate(Energy),
                   m_StructuralSize[2]/sqrt(12.0));
  } else if (m_PositionResolutionType == c_PositionResolutionXYZ) {
    return MVector(1.0/sqrt(3.0)*m_PositionResolution.Evaluate(Energy), 
                   1.0/sqrt(3.0)*m_PositionResolution.Evaluate(Energy),
                   1.0/sqrt(3.0)*m_PositionResolution.Evaluate(Energy));
  } else {
    return MVector(m_PositionResolutionX.Evaluate(Energy), 
                   m_PositionResolutionY.Evaluate(Energy),
                   m_PositionResolutionZ.Evaluate(Energy));
  }
}



// MDAngerCamera.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
