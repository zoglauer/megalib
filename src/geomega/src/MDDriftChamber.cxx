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


#ifdef ___CINT___
ClassImp(MDDriftChamber)
#endif


////////////////////////////////////////////////////////////////////////////////


MDDriftChamber::MDDriftChamber(MString Name) : MDStrip3D(Name)
{
  // Construct an instance of MDDriftChamber

  m_Type = c_DriftChamber;
  m_Description = c_DriftChamberName;
  m_NoiseAxis = 3;

  m_LightSpeed = c_SpeedOfLight;
  m_LightDetectorPosition = 0; // No light detector
  m_DriftConstant = 0; // No opening angle for the drift
  m_EnergyPerElectron = 0.022; //kev: Not used if no opening angle is defined

  m_LightEnergyResolution = new MSpline(MSpline::Interpolation);
}


////////////////////////////////////////////////////////////////////////////////


MDDriftChamber::MDDriftChamber(const MDDriftChamber& D)
{
  m_LightSpeed = D.m_LightSpeed;
  m_LightDetectorPosition = D.m_LightDetectorPosition;
  m_DriftConstant = D.m_DriftConstant;
  m_EnergyPerElectron = D.m_EnergyPerElectron;

  m_LightEnergyResolution = new MSpline(*(D.m_LightEnergyResolution)); 
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDDriftChamber::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return new MDDriftChamber(*this);
}


////////////////////////////////////////////////////////////////////////////////


MDDriftChamber::~MDDriftChamber()
{
  // Delete this instance of MDDriftChamber

  delete m_LightEnergyResolution;
}


////////////////////////////////////////////////////////////////////////////////


void MDDriftChamber::SetLightEnergyResolution(const double Energy, 
                                              const double Resolution)
{
  // 

  massert(Energy >= 0);
  massert(Resolution >= 0);

  m_LightEnergyResolution->AddDataPoint(Energy, Resolution);
}


////////////////////////////////////////////////////////////////////////////////


double MDDriftChamber::GetLightEnergyResolution(const double Energy) const
{
  // 

  return m_LightEnergyResolution->Get(Energy);
}


////////////////////////////////////////////////////////////////////////////////


bool MDDriftChamber::NoiseLightEnergy(double& Energy) const
{
  Energy = gRandom->Gaus(Energy, GetLightEnergyResolution(Energy));
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDDriftChamber::Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const
{
  // Noise Position and energy of this hit:

  MDStrip3D::Noise(Pos, Energy, Time, Volume);
}


////////////////////////////////////////////////////////////////////////////////


vector<MDGridPoint> MDDriftChamber::Discretize(const MVector& PosInDetectorVolume, 
                                               const double& Energy, 
                                               const double& Time, 
                                               MDVolume* DetectorVolume) const
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
                                 yStrip+xWafer*m_NStripsX, 
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
  for (int d = 0; d < m_LightEnergyResolution->GetNDataPoints(); ++d) {
    out<<m_Name<<".DepthResolution "<<
      m_LightEnergyResolution->GetDataPointXValueAt(d)<<" "<<
      m_LightEnergyResolution->GetDataPointYValueAt(d)<<endl;
  }  

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDDriftChamber::GetGeant3() const
{
  ostringstream out;

  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);
  
  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<"      SENVOL("<<m_SVs[i]->GetSensitiveVolumeID()<<") = '"<<m_SVs[i]->GetShortName()<<"'"<<endl;
    out<<"      SENDET("<<m_SVs[i]->GetSensitiveVolumeID()<<") = "<<m_ID<<endl;
  }

  out<<"      DETNR("<<m_ID<<") = 4"<<endl;
  out<<"      DETTYP("<<m_ID<<") = 5"<<endl;
  out<<"      WIDTH("<<m_ID<<",1) = "<<m_WidthX<<endl;
  out<<"      WIDTH("<<m_ID<<",2) = "<<m_WidthY<<endl;
  out<<"      OFFSET("<<m_ID<<",1) = "<<m_OffsetX<<endl;
  out<<"      OFFSET("<<m_ID<<",2) = "<<m_OffsetY<<endl;
  out<<"      SPITCH("<<m_ID<<",1) = "<<m_PitchX<<endl;
  out<<"      SPITCH("<<m_ID<<",2) = "<<m_PitchX<<endl;
  out<<"      SLENGTH("<<m_ID<<",1) = "<<m_StripLengthX<<endl;
  out<<"      SLENGTH("<<m_ID<<",2) = "<<m_StripLengthY<<endl;
  out<<"      NSTRIP("<<m_ID<<",1) = "<<m_NStripsX<<endl;
  out<<"      NSTRIP("<<m_ID<<",2) = "<<m_NStripsX<<endl;
  out<<endl;


  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDDriftChamber::GetMGeant() const
{
  mimp<<"This class is not yet ready for MGGPOD's MEGAlib extension (standard MGGPOD ok)!!!"<<show;

  return MDStrip3D::GetMGeant();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDDriftChamber::ToString() const
{
  ostringstream out;

  out<<"Detector "<<m_Name<<" - drift chamber"<<endl;
  out<<"   with sensitive volumes: ";  
  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<m_SVs[i]->GetName()<<" ";
  }
  out<<endl<<"   width: "<<m_WidthX<<", "<<m_WidthY<<endl;
  out<<"   offset: "<<m_OffsetX<<", "<<m_OffsetY<<endl;
  out<<"   pitch: "<<m_PitchX<<", "<<m_PitchY<<endl;
  out<<"   striplength: "<<m_StripLengthX<<", "<<m_StripLengthY<<endl;
  out<<"   stripnumber: "<<m_NStripsX<<", "<<m_NStripsY<<endl;

  return out.str().c_str();  
}



// MDDriftChamber.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
