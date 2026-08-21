/*
 * MDDriftChamber.cxx
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
// MDDriftChamber
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDDriftChamber.h"

// Standard libs:
#include <limits>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDDriftChamber)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDDriftChamber::c_LightEnergyResolutionTypeUnknown    = 0;
const int MDDriftChamber::c_LightEnergyResolutionTypeIdeal      = 1;
const int MDDriftChamber::c_LightEnergyResolutionTypeGauss      = 2;


////////////////////////////////////////////////////////////////////////////////


MDDriftChamber::MDDriftChamber(MString Name) : MDStrip3D(Name)
{
  // Construct an instance of MDDriftChamber

  m_Type = c_DriftChamber;
  m_Description = c_DriftChamberName;

  m_LightSpeed = g_DoubleNotDefined;
  m_LightDetectorPosition = g_IntNotDefined; // No light detector

  m_LightEnergyResolutionType = c_LightEnergyResolutionTypeUnknown;
}


////////////////////////////////////////////////////////////////////////////////


MDDriftChamber::MDDriftChamber(const MDDriftChamber& D) : MDStrip3D(D)
{
  m_LightSpeed = D.m_LightSpeed;
  m_LightDetectorPosition = D.m_LightDetectorPosition;
  m_DriftConstant = D.m_DriftConstant;
  m_EnergyPerElectron = D.m_EnergyPerElectron;

  m_LightEnergyResolutionType = D.m_LightEnergyResolutionType;
  m_LightEnergyResolution = D.m_LightEnergyResolution; 
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDDriftChamber::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return new MDDriftChamber(*this);
}


////////////////////////////////////////////////////////////////////////////////


bool MDDriftChamber::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors
  
  MDStrip3D::CopyDataToNamedDetectors();
  
  if (m_IsNamedDetector == true) return true;
  
  for (unsigned int n = 0; n < m_NamedDetectors.size(); ++n) {
    if (dynamic_cast<MDDriftChamber*>(m_NamedDetectors[n]) == 0) {
      mout<<"   ***  Internal error  ***  in detector "<<m_Name<<endl;
      mout<<"We have a named detector ("<<m_NamedDetectors[n]->GetName()<<") which is not of the same type as the base detector!"<<endl;
      return false;
    }
    MDDriftChamber* D = dynamic_cast<MDDriftChamber*>(m_NamedDetectors[n]);

    if (D->m_LightSpeed == g_DoubleNotDefined && 
        m_LightSpeed != g_DoubleNotDefined) {
      D->m_LightSpeed = m_LightSpeed;
    }

    if (D->m_LightDetectorPosition == g_IntNotDefined && 
        m_LightDetectorPosition != g_IntNotDefined) {
      D->m_LightDetectorPosition = m_LightDetectorPosition;
    }
    
    if (D->m_LightEnergyResolutionType == c_LightEnergyResolutionTypeUnknown && 
        m_LightEnergyResolutionType != c_LightEnergyResolutionTypeUnknown) {
      D->m_LightEnergyResolutionType = m_LightEnergyResolutionType; 
      D->m_LightEnergyResolution = m_LightEnergyResolution; 
    }
  }
   
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


MDDriftChamber::~MDDriftChamber()
{
  // Delete this instance of MDDriftChamber
}


////////////////////////////////////////////////////////////////////////////////


void MDDriftChamber::SetLightEnergyResolution(const double Energy, 
                                              const double Resolution)
{
  // 

  massert(Energy >= 0);
  massert(Resolution >= 0);

  m_LightEnergyResolution.Add(Energy, Resolution);
}


////////////////////////////////////////////////////////////////////////////////


double MDDriftChamber::GetLightEnergyResolution(const double Energy) const
{
  // 

  return m_LightEnergyResolution.Evaluate(Energy);
}


////////////////////////////////////////////////////////////////////////////////


bool MDDriftChamber::NoiseLightEnergy(double& Energy) const
{
  if (m_LightEnergyResolutionType == c_LightEnergyResolutionTypeIdeal) {
    // do nothing
  } else if (m_LightEnergyResolutionType == c_LightEnergyResolutionTypeGauss) {
    Energy = gRandom->Gaus(Energy, GetLightEnergyResolution(Energy));
  } else {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Unknown light energy resolution type: "<<m_LightEnergyResolutionType<<endl;
    return false;    
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDDriftChamber::Noise(MVector& Pos, double& Energy, double& Time, MString& Flags, MDVolume* Volume) const
{
  // Noise Position and energy of this hit:

  MDStrip3D::Noise(Pos, Energy, Time, Flags, Volume);
}


////////////////////////////////////////////////////////////////////////////////


vector<MDGridPoint> MDDriftChamber::Grid(const MVector& PosInDetectorVolume, 
                                         const double& Energy, 
                                         const double& Time, 
                                         const MDVolume* DetectorVolume) const
{
  // Discretize Pos to a voxel of this volume and
  // calculate the new time

  vector<MDGridPoint> Points;
 
  MVector Pos = PosInDetectorVolume;

  // Translate into sensitive volume
  Pos += m_StructuralDimension;
  Pos -= m_StructuralOffset;
  int xWafer = int(Pos.X()/(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  int yWafer = int(Pos.Y()/(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));

  Pos.SetX(Pos.X() - xWafer*(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  Pos.SetY(Pos.Y() - yWafer*(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));
  Pos -= m_StructuralSize;

  // Ignore the hit if it is outside the sensitive part (guard ring?):
  if (fabs(Pos.X()) > m_WidthX/2.0 - m_OffsetX || 
      fabs(Pos.Y()) > m_WidthY/2.0 - m_OffsetY) {
    if (fabs(Pos.X()) > m_WidthX/2.0 || 
      fabs(Pos.Y()) > m_WidthY/2.0) {
      merr<<"Hit outside detector! ("
          <<Pos[0]<<", "<<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    } else {
      mdebug<<"Hit in guard ring: ("
            <<Pos[0]<<", "<<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    }
    return Points;
  }

  // Start with time:
  // Time is shortest distance to light sensitive detector area:
  double DriftTime = Time + GetLightTravelTime(Pos);


  // Then the drift:
  
  // Split the step into "electrons"
  int NElectrons = int(Energy/m_EnergyPerElectron);
  if (NElectrons == 0 || m_DriftConstant == 0) NElectrons = 1;
  double EnergyPerElectron = Energy/NElectrons;
  
  // Calculate the drift parameters
  double DriftLength = Pos.Z() + m_StructuralSize.Z();
  massert(DriftLength >= 0);
  double DriftRadiusSigma = m_DriftConstant * sqrt(DriftLength);

  // double DriftRadius = 0;
  // double DriftAngle = 0;
  double DriftX = 0;
  double DriftY = 0;

  //cout<<"Electron with: "<<Energy<<"!"<<NElectrons<<"!"<<m_EnergyPerElectron<<endl;
  Points.reserve(NElectrons);
  for (int e = 0; e < NElectrons; ++e) {

    // Randomize x, y position:

    // Original but much too narrow distribution:
    // DriftRadius = gRandom->Gaus(0, DriftRadiusSigma);
    // DriftAngle = gRandom->Rndm() * c_Pi;
    // DriftX = DriftRadius*cos(DriftAngle);
    // DriftY = DriftRadius*sin(DriftAngle);

    // Real 2D Gaussian ("Rannor" gives 1 sigma distributions):
    gRandom->Rannor(DriftX, DriftY);
    DriftX *= DriftRadiusSigma;
    DriftY *= DriftRadiusSigma;

    MVector DriftPosition = Pos + MVector(DriftX, DriftY, 0);
    if (fabs(DriftPosition.X()) > m_WidthX/2 - m_OffsetX || 
        fabs(DriftPosition.Y()) > m_WidthY/2 - m_OffsetY) {
      //mout<<"Energy lost at "<<DriftPosition<<": "<<m_WidthX/2<<", "<<m_WidthY/2<<endl;
      continue;
    }
    
    int xStrip;
    if (m_NStripsX == 1 || m_PitchX == 0) {
      xStrip = 0;
    } else {
      xStrip = (int) ((DriftPosition.X() + m_WidthX/2 - m_OffsetX)/m_PitchX);
    }
    
    int yStrip;
    if (m_NStripsY == 1 || m_PitchY == 0) {  
      yStrip = 0;
    } else {
      yStrip = (int) ((DriftPosition.Y() + m_WidthY/2 - m_OffsetY)/m_PitchY);
    }

    // Check if we have a reasonable strip:
    if (xStrip < 0 || xStrip >= m_NStripsX) {
      merr<<"Invalid x-strip number: "<<xStrip<<endl
          <<"   Position was "<<DriftPosition<<endl;
      continue;
    }
    if (yStrip < 0 || yStrip >= m_NStripsY) {
      merr<<"Invalid y-strip number: "<<yStrip<<endl
          <<"   Position was "<<DriftPosition<<endl;
      continue;
    }

    Points.push_back(MDGridPoint(xStrip+xWafer*m_NStripsX, 
                                 yStrip+yWafer*m_NStripsY, 
                                 0,
                                 MDGridPoint::c_VoxelDrift,
                                 MVector(0.0, 0.0, Pos.Z()), 
                                 EnergyPerElectron, 
                                 DriftTime));
  }

  return Points;
}



////////////////////////////////////////////////////////////////////////////////


//! Return the travel time between the interaction position and the PMTs
double MDDriftChamber::GetLightTravelTime(const MVector& Position) const
{
  if (m_LightDetectorPosition == 0) {
    return 0.0;
  } else if (m_LightDetectorPosition == 1) {
    return (m_StructuralSize.X()-Position.X())/m_LightSpeed;
  } else if (m_LightDetectorPosition == -1) {
    return (Position.X() - m_StructuralSize.X())/m_LightSpeed;
  } else if (m_LightDetectorPosition == 2) {
    return (m_StructuralSize.Y()-Position.Y())/m_LightSpeed;
  } else if (m_LightDetectorPosition == -2) {
    return (Position.Y() - m_StructuralSize.Y())/m_LightSpeed;
  } else if (m_LightDetectorPosition == 3) {
    return (m_StructuralSize.Z()-Position.Z())/m_LightSpeed;
  } else if (m_LightDetectorPosition == -3) {
    return (Position.Z() - m_StructuralSize.Z())/m_LightSpeed;
  } 

  merr<<"Wrong light detector position: "<<m_LightDetectorPosition<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDriftChamber::Validate()
{
  // Check if all input is reasonable

  if (MDStrip3D::Validate() == false) {
    return false;
  }

  //m_LightSpeed = c_SpeedOfLight;
  //m_LightDetectorPosition = 0; // No light detector
  //m_DriftConstant = 0; // No opening angle for the drift
  //m_EnergyPerElectron = 0.022; //kev: Not used if no opening angle is defined
  
  if (m_LightSpeed == g_DoubleNotDefined) {
     m_LightSpeed = c_SpeedOfLight;
  }
  
  if (m_LightDetectorPosition == g_IntNotDefined) {
     m_LightDetectorPosition = 0; // No light detector
  }
  
  if (m_LightEnergyResolutionType == c_LightEnergyResolutionTypeUnknown) {
    mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
    mout<<"No light energy resolution defined --- assuming ideal"<<endl; 
    m_LightEnergyResolutionType = c_LightEnergyResolutionTypeIdeal;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MDDriftChamber::GetGeomega() const
{
  // Return all detector characteristics in Geomega-Format

  ostringstream out;

  MString Strip3D = MDStrip3D::GetGeomega();
  Strip3D.ReplaceAll("Strip3D", "Strip3D");

  out<<Strip3D;
  out<<m_Name<<".LightSpeed "<<m_LightSpeed<<endl;
  out<<m_Name<<".LightDetectorPosition "<<m_LightDetectorPosition<<endl;
  out<<m_Name<<".DriftConstant "<<m_DriftConstant<<endl;
  out<<m_Name<<".EnergyPerElectron "<<m_EnergyPerElectron<<endl;
  for (unsigned int d = 0; d < m_LightEnergyResolution.GetNDataPoints(); ++d) {
    out<<m_Name<<".DepthResolution "<<
      m_LightEnergyResolution.GetDataPointX(d)<<" "<<
      m_LightEnergyResolution.GetDataPointY(d)<<endl;
  }  

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDDriftChamber::ToString() const
{
  ostringstream out;
  out<<MDStrip3D::ToString()<<endl;

  return out.str().c_str();  
}


// MDDriftChamber.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
