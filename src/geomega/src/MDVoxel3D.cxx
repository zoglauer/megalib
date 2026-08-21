/*
 * MDVoxel3D.cxx
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
// MDVoxel3D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDVoxel3D.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDShapeBRIK.h"
#include "MDShapeSubtraction.h"
#include "MDShapeIntersection.h"
#include "MDGuardRing.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDVoxel3D)
#endif


////////////////////////////////////////////////////////////////////////////////


MDVoxel3D::MDVoxel3D(MString String) : MDDetector(String)
{
  // default constructor

  m_Type = c_Voxel3D;
  m_Description = c_Voxel3DName;

  m_WidthX = g_DoubleNotDefined;
  m_WidthY = g_DoubleNotDefined;
  m_WidthZ = g_DoubleNotDefined;

  m_OffsetX = g_DoubleNotDefined;
  m_OffsetY = g_DoubleNotDefined;
  m_OffsetZ = g_DoubleNotDefined;

  m_NVoxelsX = g_IntNotDefined;
  m_NVoxelsY = g_IntNotDefined;
  m_NVoxelsZ = g_IntNotDefined;

  m_NBlocksX = g_IntNotDefined;
  m_NBlocksY = g_IntNotDefined;
  m_NBlocksZ = g_IntNotDefined;

  m_VoxelSizeX = g_IntNotDefined;
  m_VoxelSizeY = g_IntNotDefined;
  m_VoxelSizeZ = g_IntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MDVoxel3D::MDVoxel3D(const MDVoxel3D& S) : MDDetector(S)
{
  m_WidthX = S.m_WidthX;
  m_WidthY = S.m_WidthY;
  m_WidthZ = S.m_WidthZ;

  m_OffsetX = S.m_OffsetX;
  m_OffsetY = S.m_OffsetY;
  m_OffsetZ = S.m_OffsetZ;

  m_NVoxelsX = S.m_NVoxelsX;
  m_NVoxelsY = S.m_NVoxelsY;
  m_NVoxelsZ = S.m_NVoxelsZ;

  m_NBlocksX = S.m_NBlocksX;
  m_NBlocksY = S.m_NBlocksY;
  m_NBlocksZ = S.m_NBlocksZ;

  m_VoxelSizeX = S.m_VoxelSizeX;
  m_VoxelSizeY = S.m_VoxelSizeY;
  m_VoxelSizeZ = S.m_VoxelSizeZ;
  
  if (m_HasGuardRing == true) {
    m_GuardRing->SetMotherDetector(this); 
  }
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDVoxel3D::Clone()
{
  // Duplicate this detector

  return dynamic_cast<MDDetector*>(new MDVoxel3D(*this));
}


////////////////////////////////////////////////////////////////////////////////


bool MDVoxel3D::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors

  MDDetector::CopyDataToNamedDetectors();
  
  if (m_IsNamedDetector == true) return true;
  
  for (unsigned int n = 0; n < m_NamedDetectors.size(); ++n) {
    if (dynamic_cast<MDVoxel3D*>(m_NamedDetectors[n]) == 0) {
      mout<<"   ***  Internal error  ***  in detector "<<m_Name<<endl;
      mout<<"We have a named detector ("<<m_NamedDetectors[n]->GetName()<<") which is not of the same type as the base detector!"<<endl;
      return false;
    }
    MDVoxel3D* D = dynamic_cast<MDVoxel3D*>(m_NamedDetectors[n]);
    
    D->m_WidthX = m_WidthX;
    D->m_WidthY = m_WidthY;
    D->m_WidthZ = m_WidthZ;

    D->m_OffsetX = m_OffsetX;
    D->m_OffsetY = m_OffsetY;
    D->m_OffsetZ = m_OffsetZ;

    D->m_NVoxelsX = m_NVoxelsX;
    D->m_NVoxelsY = m_NVoxelsY;
    D->m_NVoxelsZ = m_NVoxelsZ;

    D->m_NBlocksX = m_NBlocksX;
    D->m_NBlocksY = m_NBlocksY;
    D->m_NBlocksZ = m_NBlocksZ;

    D->m_VoxelSizeX = m_VoxelSizeX;
    D->m_VoxelSizeY = m_VoxelSizeY;
    D->m_VoxelSizeZ = m_VoxelSizeZ;
    
    if (D->HasGuardRing() == true) {
      D->m_GuardRing->SetMotherDetector(D);
    }
  }
   
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


MDVoxel3D::~MDVoxel3D()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


//! Set that this detector has a guard ring
void MDVoxel3D::HasGuardRing(bool HasGuardRing)
{
  if (HasGuardRing == true) {
    if (m_HasGuardRing == false) {
      m_HasGuardRing = true; 
      m_GuardRing = new MDGuardRing(m_Name + "_GuardRing");
      m_GuardRing->SetMotherDetector(this);
    }
  } else {
    if (m_HasGuardRing == true) {
      m_HasGuardRing = false; 
      delete m_GuardRing;
      m_GuardRing = nullptr;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDVoxel3D::Noise(MVector& Pos, double& Energy, double& Time, MString& Flags, MDVolume* Volume) const
{
  // Noise energy of this hit:

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


vector<MDGridPoint> MDVoxel3D::Grid(const MVector& PosInDetector, 
                                    const double& Energy, 
                                    const double& Time, 
                                    const MDVolume* DetectorVolume) const
{
  // Discretize Pos to a voxel of this volume

  vector<MDGridPoint> Points;

  MDGridPoint GridPoint = GetGridPoint(PosInDetector);
  if (GridPoint.GetType() != MDGridPoint::c_Unknown) {
    GridPoint.SetEnergy(Energy);
    GridPoint.SetTime(Time);
    Points.push_back(GridPoint);
  }

  return Points;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDVoxel3D::GetGridPoint(const MVector& PosInDetector) const
{
  // Discretize Pos to a voxel of this volume

  MDGridPoint GridPoint(0, 0, 0, MDGridPoint::c_Unknown);

  MVector Pos = PosInDetector;

  // Translate into center of sensitive volume
  Pos += m_StructuralDimension;
  Pos -= m_StructuralOffset;

  int xBlock = int(Pos.X()/(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  int yBlock = int(Pos.Y()/(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));
  int zBlock = int(Pos.Z()/(2*m_StructuralSize.Z()+m_StructuralPitch.Z()));

  // Check if we have a reasonable wafer:
  if (xBlock < 0 || xBlock >= m_NBlocksX) {
    mout<<"Invalid x-block number: "<<xBlock<<" Max: "<<m_NBlocksX-1
        <<" (Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }
  if (yBlock < 0 || yBlock >= m_NBlocksY) {
    mout<<"Invalid y-block number: "<<yBlock<<" Max: "<<m_NBlocksY-1
        <<" (Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }
  if (zBlock < 0 || zBlock >= m_NBlocksZ) {
    mout<<"Invalid z-block number: "<<zBlock<<" Max: "<<m_NBlocksZ-1
        <<" (Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }


  Pos.SetX(Pos.X() - xBlock*(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  Pos.SetY(Pos.Y() - yBlock*(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));
  Pos.SetZ(Pos.Z() - zBlock*(2*m_StructuralSize.Z()+m_StructuralPitch.Z()));
  Pos -= m_StructuralSize;

  // Ignore the hit if it is out side the sensitive part (guard ring?):
  if (fabs(Pos.X()) > m_WidthX/2.0 - m_OffsetX || 
      fabs(Pos.Y()) > m_WidthY/2.0 - m_OffsetY ||
      fabs(Pos.Z()) > m_WidthZ/2.0 - m_OffsetZ) {
    if (fabs(Pos.X()) > m_WidthX/2.0 || 
        fabs(Pos.Y()) > m_WidthY/2.0 || 
        fabs(Pos.Z()) > m_WidthZ/2.0) {
      merr<<"Hit outside detector: "<<m_Name<<": "
          <<Pos<<" <-> ("<<m_WidthX/2.0<<", "<<m_WidthY/2.0<<", "<<m_WidthZ/2.0<<")"<<endl;
    } else {
      mdebug<<"Hit in guard ring:  "<<m_Name<<":("
            <<Pos[0]<<", "<<Pos[1]<<", "<<Pos[2]<<")"<<endl;
      // We define the first voxel of the detector here, in order to be able to identify
      // the sensitive volume block of the detector 
      GridPoint.Set(xBlock*m_NVoxelsX, yBlock*m_NVoxelsY, 
                    zBlock*m_NVoxelsZ, MDGridPoint::c_GuardRing);
    }
    return GridPoint;
  }

  int xVoxel;
  if (m_NVoxelsX == 1) {
    xVoxel = 0;
  } else {
    xVoxel = (int) ((Pos.X() + m_WidthX/2 - m_OffsetX)/m_VoxelSizeX);
  }

  int yVoxel;
  if (m_NVoxelsY == 1) {  
    yVoxel = 0;
  } else {
    yVoxel = (int) ((Pos.Y() + m_WidthY/2 - m_OffsetY)/m_VoxelSizeY);
  }

  int zVoxel;
  if (m_NVoxelsZ == 1) {  
    zVoxel = 0;
  } else {
    zVoxel = (int) ((Pos.Z() + m_WidthZ/2 - m_OffsetZ)/m_VoxelSizeZ);
  }

  // Check if we have a reasonable voxel:
  if (xVoxel+xBlock*m_NVoxelsX < 0 || xVoxel+xBlock*m_NVoxelsX >= (xBlock+1)*m_NVoxelsX) {
    mout<<"Invalid x-strip number: "<<xVoxel+xBlock*m_NVoxelsX
        <<" (S="<<xVoxel<<", B="<<xBlock<<")"<<endl
        <<"   Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }
  if (yVoxel+yBlock*m_NVoxelsY < 0 || yVoxel+yBlock*m_NVoxelsY >= (yBlock+1)*m_NVoxelsY) {
    mout<<"Invalid y-strip number: "<<yVoxel+yBlock*m_NVoxelsY
        <<" (S="<<yVoxel<<", B="<<yBlock<<")"<<endl
        <<" - Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }
  if (zVoxel+zBlock*m_NVoxelsZ < 0 || zVoxel+zBlock*m_NVoxelsZ >= (zBlock+1)*m_NVoxelsZ) {
    mout<<"Invalid z-voxel number: "<<zVoxel+zBlock*m_NVoxelsZ
        <<" (S="<<zVoxel<<", B="<<zBlock<<")"<<endl
        <<" - Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }

  GridPoint.Set(xVoxel+xBlock*m_NVoxelsX, 
                yVoxel+yBlock*m_NVoxelsY,
                zVoxel+zBlock*m_NVoxelsZ,
                MDGridPoint::c_Voxel);

  return GridPoint;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVoxel3D::GetPositionInDetectorVolume(const unsigned int xGrid, 
                                               const unsigned int yGrid,
                                               const unsigned int zGrid,
                                               const MVector PositionInGrid,
                                               const unsigned int Type,
                                               const MDVolume* Volume) const
{
  // Return the position in the detector volume

  int xBlock = int(xGrid/m_NVoxelsX);
  int yBlock = int(yGrid/m_NVoxelsY);
  int zBlock = int(zGrid/m_NVoxelsZ);

  MVector Position;

  // Position in Wafer relative to its center:
  if (Type == MDGridPoint::c_GuardRing) {
    if (m_HasGuardRing == true) {
      Position = m_GuardRing->GetUniquePosition();
    } else {
      merr<<"This detector has no guard ring!"<<endl;
    }
  } else {
    Position.SetX(-m_WidthX/2 + m_OffsetX + (xGrid - xBlock*m_NVoxelsX + 0.5)*m_VoxelSizeX);
    Position.SetY(-m_WidthY/2 + m_OffsetY + (yGrid - yBlock*m_NVoxelsY + 0.5)*m_VoxelSizeY);
    Position.SetZ(-m_WidthZ/2 + m_OffsetZ + (zGrid - zBlock*m_NVoxelsZ + 0.5)*m_VoxelSizeZ);
  }

  // Position in Wafer relative to its negative edge:
  Position += m_StructuralSize;

  // Position in Detector relative to lower edge of detector at (-x, -y, -z)
  Position.SetX(Position.X() + xBlock*(2*m_StructuralSize.X()+m_StructuralPitch.X()));  
  Position.SetY(Position.Y() + yBlock*(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));  
  Position.SetZ(Position.Z() + zBlock*(2*m_StructuralSize.Z()+m_StructuralPitch.Z()));  

  // Position in Detector realitive to its negative edge: 
  Position += m_StructuralOffset;

  // Position in detector relaitive to its center:  
  Position -= m_StructuralDimension;

  return Position;
}


////////////////////////////////////////////////////////////////////////////////


void MDVoxel3D::SetOffset(const double x, const double y, const double z)
{
  m_OffsetX = x;
  m_OffsetY = y;
  m_OffsetZ = z;
}


////////////////////////////////////////////////////////////////////////////////


void MDVoxel3D::SetNVoxels(const int x, const int y, const int z)
{
  m_NVoxelsX = x;
  m_NVoxelsY = y;
  m_NVoxelsZ = z;
}


////////////////////////////////////////////////////////////////////////////////


MString MDVoxel3D::GetGeomega() const
{
  // Return all detector characteristics in Geomega-Format

  ostringstream out;

  out<<"Voxel3D "<<m_Name<<endl;
  out<<GetGeomegaCommon()<<endl;
  out<<m_Name<<".Offset "<<m_OffsetX<<" "<<m_OffsetY<<" "<<m_OffsetZ<<endl;
  out<<m_Name<<".Voxels "<<m_NVoxelsX<<" "<<m_NVoxelsY<<" "<<m_NVoxelsZ<<endl;
  
  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDVoxel3D::ToString() const
{
  //

  ostringstream out;

  out<<MDDetector::ToString()<<endl;
  out<<"   offset: "<<m_OffsetX<<", "<<m_OffsetY<<", "<<m_OffsetZ<<endl;
  out<<"   voxels: "<<m_NVoxelsX<<", "<<m_NVoxelsY<<", "<<m_NVoxelsZ<<endl;
  out<<"   v.size: "<<m_VoxelSizeX<<", "<<m_VoxelSizeY<<", "<<m_VoxelSizeZ<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


bool MDVoxel3D::AreNear(const MVector& Pos1, const MVector& dPos1, 
                        const MVector& Pos2, const MVector& dPos2, 
                        const double Sigma, const int Level) const
{
  // --------------------
  // | 5 | 4 | 5 | 8 | 13
  // --------------------
  // | 2 | 1 | 2 | 5 | 10
  // --------------------
  // | 1 | 0 | 1 | 4 |  9
  // --------------------
  // | 2 | 1 | 2 | 5 | 10
  // --------------------
  // | 5 | 4 | 5 | 8 | 13

  mimp<<"Are near is still wrong!"<<show;

  static const double Epsilon = 0.00001; 

  if (fabs(Pos1[0] - Pos2[0])/m_VoxelSizeX*fabs(Pos1[0] - Pos2[0])/m_VoxelSizeX +
      fabs(Pos1[1] - Pos2[1])/m_VoxelSizeY*fabs(Pos1[1] - Pos2[1])/m_VoxelSizeY +
      fabs(Pos1[2] - Pos2[2])/m_VoxelSizeZ*fabs(Pos1[2] - Pos2[2])/m_VoxelSizeZ < Level+Epsilon) {
    //cout<<"Adjacent!"<<endl;
    if (Sigma < 0) {
      return true;
    } else if (fabs(Pos1[2] - Pos2[2]) < Sigma*(dPos1[2]+Epsilon) ||
        fabs(Pos1[2] - Pos2[2]) < Sigma*(dPos2[2]+Epsilon) || Sigma < 0) {
      //cout<<"Within Sigma: "<<Sigma*(dPos2[2]+Epsilon)<<endl;
      return true;
    } else {
      //cout<<"Outside Sigma: "<<Sigma*(dPos2[2]+Epsilon)<<endl;
    }
  }
  //cout<<"Not adjacent: "<<fabs(Pos1[0] - Pos2[0])/m_VoxelSizeX*fabs(Pos1[0] - Pos2[0])/m_VoxelSizeX<<"!"
  //    <<fabs(Pos1[1] - Pos2[1])/m_VoxelSizeY*fabs(Pos1[1] - Pos2[1])/m_VoxelSizeY<<"!"<<Level+Epsilon<<endl;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVoxel3D::GetPositionResolution(const MVector& Pos, const double Energy) const
{
  return MVector(m_VoxelSizeX/sqrt(12.0), 
                 m_VoxelSizeY/sqrt(12.0), 
                 m_VoxelSizeZ/sqrt(12.0));
}


////////////////////////////////////////////////////////////////////////////////


bool MDVoxel3D::Validate()
{
  //! Check if all input is reasonable

  if (MDDetector::Validate() == false) {
    return false;
  }

  // The detector volume and the sensitive volume need to be boxes OR
  // The first volume of an intersection or subtraction must be a box
  
  MDShapeBRIK* DetectorShape = nullptr;
  if (m_DetectorVolume->GetShape()->GetType() == "BRIK") {
    DetectorShape = dynamic_cast<MDShapeBRIK*>(m_DetectorVolume->GetShape());
  } else if (m_DetectorVolume->GetShape()->GetType() == "Subtraction" && dynamic_cast<MDShapeSubtraction*>(m_DetectorVolume->GetShape())->GetMinuend()->GetType() == "BRIK") {
    DetectorShape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeSubtraction*>(m_DetectorVolume->GetShape())->GetMinuend());
  } else if (m_DetectorVolume->GetShape()->GetType() == "Intersection" && dynamic_cast<MDShapeIntersection*>(m_DetectorVolume->GetShape())->GetShapeA()->GetType() == "BRIK") {
    DetectorShape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeIntersection*>(m_DetectorVolume->GetShape())->GetShapeA());
  }
    
  if (DetectorShape == nullptr) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"The detector shape has to be a box (or an subtraction or intersection, where the first volume is a box)!"<<endl;
    return false;
  }
  m_StructuralDimension = DetectorShape->GetSize();
  mdebug<<"Structural dimension: "<<m_StructuralDimension<<endl;

  
  if (m_SVs.size() != 1) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"You need exactly one sensitive volume!"<<endl;
    return false;
  }
  MDShapeBRIK* SensitiveShape = nullptr;
  if (m_SVs[0]->GetShape()->GetType() == "BRIK") {
    SensitiveShape = dynamic_cast<MDShapeBRIK*>(m_SVs[0]->GetShape());
  } else if (m_SVs[0]->GetShape()->GetType() == "Subtraction" && dynamic_cast<MDShapeSubtraction*>(m_SVs[0]->GetShape())->GetMinuend()->GetType() == "BRIK") {
    SensitiveShape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeSubtraction*>(m_SVs[0]->GetShape())->GetMinuend());
  } else if (m_SVs[0]->GetShape()->GetType() == "Intersection" && dynamic_cast<MDShapeIntersection*>(m_SVs[0]->GetShape())->GetShapeA()->GetType() == "BRIK") {
    SensitiveShape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeIntersection*>(m_SVs[0]->GetShape())->GetShapeA());    
  }
  if (SensitiveShape == nullptr) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"The shape of the sensitive volume has to be a box (or an subtraction or intersection, where the first volume is a box)!"<<endl;
    return false;
  }
  m_StructuralSize = SensitiveShape->GetSize();

  mdebug<<"Structural size: "<<m_StructuralSize<<endl;

  if (m_OffsetX == g_DoubleNotDefined || m_OffsetY == g_DoubleNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"The strip/wire detector has no defined offset!"<<endl;
    return false;
  }

  if (m_OffsetX < 0 || m_OffsetY < 0 ) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"All offsets have to be non negative!"<<endl;
    return false;
  }

  if (m_NVoxelsX == g_IntNotDefined || m_NVoxelsY == g_IntNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"The strip/wire detector has no defined number of strips!"<<endl;
    return false;
  }

  if (m_NVoxelsX <= 0 || m_NVoxelsY <= 0 ) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"You need to have strips in x and y direction!"<<endl;
    return false;
  }

  // Per definition the detector is oriented in x-y-direction
  // So we can calculate the rest:
  m_WidthX = 2*m_StructuralSize.X();
  m_WidthY = 2*m_StructuralSize.Y();
  m_WidthZ = 2*m_StructuralSize.Z();

  m_VoxelSizeX = (m_WidthX-2*m_OffsetX)/m_NVoxelsX;
  m_VoxelSizeY = (m_WidthY-2*m_OffsetY)/m_NVoxelsY;
  m_VoxelSizeZ = (m_WidthZ-2*m_OffsetZ)/m_NVoxelsZ;

  m_NBlocksX = int((2*m_StructuralDimension.X() - m_StructuralOffset.X())/
                   (2*m_StructuralSize.X()+m_StructuralPitch.X()))+1;
  m_NBlocksY = int((2*m_StructuralDimension.Y() - m_StructuralOffset.Y())/
                   (2*m_StructuralSize.Y()+m_StructuralPitch.Y()))+1;
  m_NBlocksZ = int((2*m_StructuralDimension.Z() - m_StructuralOffset.Z())/
                   (2*m_StructuralSize.Z()+m_StructuralPitch.Z()))+1;

  if (m_NBlocksX <= 0 || m_NBlocksY <= 0 || m_NBlocksZ <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"You need to have at least one block in X, Y, Z direction!"<<endl;
    return false;
  }

  // In case we have a guard ring
  if (m_OffsetX < 10E-7 || m_OffsetY < 10E-7) { // ignore if smaller than 1 nanometer
    if (m_HasGuardRing == true) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"You said we have a guard ring, but we don't have one!"<<endl;
      return false;      
    }
  } else {
    if (m_HasGuardRing == false) {
      // Create the guard ring and all data correctly
      HasGuardRing(true);
    }
  }  
  
  if (m_HasGuardRing == true) {
    MVector UniqueGuardRingPosition;
    UniqueGuardRingPosition.SetXYZ(0.5*(-m_WidthX+m_OffsetX),
                                   0.5*(-m_WidthY+m_OffsetY),
                                   0.5*(-m_WidthZ+m_OffsetZ));
    m_GuardRing->SetUniquePosition(UniqueGuardRingPosition);
    
    if (m_GuardRing->Validate() == false) {
      return false; 
    }
  }
  
  if (m_EnergyLossType == c_EnergyLossTypeMap) {
    if (fabs(m_EnergyLossMap.GetXMin() - (-m_StructuralSize.X()+m_OffsetX)) > 10E-7 ||
        fabs(m_EnergyLossMap.GetXMax() - (+m_StructuralSize.X()-m_OffsetX)) > 10E-7 ||
        fabs(m_EnergyLossMap.GetYMin() - (-m_StructuralSize.Y()+m_OffsetY)) > 10E-7 ||
        fabs(m_EnergyLossMap.GetYMax() - (+m_StructuralSize.Y()-m_OffsetY)) > 10E-7 ||
        fabs(m_EnergyLossMap.GetZMin() - (-m_StructuralSize.Z()+m_OffsetZ)) > 10E-7 ||
        fabs(m_EnergyLossMap.GetZMax() - (+m_StructuralSize.Z()-m_OffsetZ)) > 10E-7) {

      cout<<fabs(m_EnergyLossMap.GetXMin() - (-m_StructuralSize.X()+m_OffsetX))<<":"<<
        fabs(m_EnergyLossMap.GetXMax() - (+m_StructuralSize.X()-m_OffsetX))<<":"<<
        fabs(m_EnergyLossMap.GetYMin() - (-m_StructuralSize.Y()+m_OffsetY))<<":"<<
        fabs(m_EnergyLossMap.GetYMax() - (+m_StructuralSize.Y()-m_OffsetY))<<":"<<
        fabs(m_EnergyLossMap.GetZMin() - (-m_StructuralSize.Z()+m_OffsetZ))<<":"<<
        fabs(m_EnergyLossMap.GetZMax() - (+m_StructuralSize.Z()-m_OffsetZ))<<endl;

      mout<<"   ***  Warning  ***  in detector "<<m_Name<<endl;
      mout<<"The energy loss map dimensions are not identical with the detector dimensions (sensitive area)!"<<endl;
      mout<<"The map has been rescaled to detector dimensions!"<<endl;

      // Rescale the map, to be sure it fits into the detector volume
      m_EnergyLossMap.RescaleX(-m_StructuralSize.X()+m_OffsetX, m_StructuralSize.X()-m_OffsetX);
      m_EnergyLossMap.RescaleY(-m_StructuralSize.Y()+m_OffsetY, m_StructuralSize.Y()-m_OffsetY);
      m_EnergyLossMap.RescaleZ(-m_StructuralSize.Z(), m_StructuralSize.Z());
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDVoxel3D::CreateBlockedTriggerChannelsGrid()
{
  // Create the grid only if it is really used

  m_BlockedTriggerChannels.Set(MDGrid::c_Voxel, m_NVoxelsX*m_NBlocksX, m_NVoxelsY*m_NBlocksY, m_NVoxelsZ*m_NBlocksZ);
  m_AreBlockedTriggerChannelsUsed = true;
}


// MDVoxel3D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
