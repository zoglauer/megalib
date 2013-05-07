/*
 * MDCalorimeter.cxx
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
// MDCalorimeter
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDCalorimeter.h"

// Standard libs:
#include <limits>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MDShapeBRIK.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDCalorimeter)
#endif


////////////////////////////////////////////////////////////////////////////////


MDCalorimeter::MDCalorimeter(MString String) : MDDetector(String)
{
  // default constructor

  m_Type = c_Calorimeter;
  m_Description = c_CalorimeterName;
  m_DepthResolutionType = c_DepthResolutionTypeUnknown;
}


////////////////////////////////////////////////////////////////////////////////


MDCalorimeter::MDCalorimeter(const MDCalorimeter& C) : MDDetector(C)
{
  m_DepthResolutionType = C.m_DepthResolutionType;
  m_DepthResolution = C.m_DepthResolution; 
  m_DepthResolutionSigma = C.m_DepthResolutionSigma; 
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDCalorimeter::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return new MDCalorimeter(*this);
}


////////////////////////////////////////////////////////////////////////////////


bool MDCalorimeter::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors
  
  MDDetector::CopyDataToNamedDetectors();
  
  if (m_IsNamedDetector == true) return true;
  
  for (unsigned int n = 0; n < m_NamedDetectors.size(); ++n) {
    if (dynamic_cast<MDCalorimeter*>(m_NamedDetectors[n]) == 0) {
      mout<<"   ***  Internal error  ***  in detector "<<m_Name<<endl;
      mout<<"We have a named detector ("<<m_NamedDetectors[n]->GetName()<<") which is not of the same type as the base detector!"<<endl;
      return false;
    }
    MDCalorimeter* D = dynamic_cast<MDCalorimeter*>(m_NamedDetectors[n]);

    if (D->m_DepthResolutionType == c_DepthResolutionTypeUnknown && 
        m_DepthResolutionType != c_DepthResolutionTypeUnknown) {
      D->m_DepthResolutionType = m_DepthResolutionType; 
      D->m_DepthResolution = m_DepthResolution; 
      D->m_DepthResolutionSigma = m_DepthResolutionSigma; 
    }
  }
   
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


MDCalorimeter::~MDCalorimeter()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDCalorimeter::SetDepthResolutionAt(const double Energy, 
                                         const double Resolution, 
                                         const double Sigma)
{
  // Set a variable depth resolution

  if (Energy < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy for depth resolution needs to be positive!"<<endl;
    return false; 
  }
  if (Resolution <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Depth resolution needs to be positive!"<<endl;
    return false; 
  }
  if (Sigma < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Sigma of depth resolution needs to be positive!"<<endl;
    return false; 
  }

  m_DepthResolutionType = c_DepthResolutionTypeGauss;
  m_DepthResolution.Add(Energy, Resolution);
  m_DepthResolutionSigma.Add(Energy, Sigma);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDCalorimeter::Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const
{
  // Noise Position and Energy:

  if (m_NoiseActive == false) return;

  bool IsOverflow = false;

  // Test for failure:
  if (gRandom->Rndm() < m_FailureRate) {
    Energy = 0;
    return;
  }
  
  // Noise:
  ApplyEnergyResolution(Energy);

  // Overflow:
  IsOverflow = ApplyOverflow(Energy);

  // Noise threshold:
  if (ApplyNoiseThreshold(Energy) == true) {
    Pos[2] = 0.0; // Really needed, because when below threshold, then E = 0 -> Rejection
    return;
  } 

  // Noise the z-coordinate:
  if (HasDepthResolution() == true) {
    mdebug<<"Noise Cal: Pos "<<Pos[2]
          <<" cm  -  Res: "<<m_DepthResolution.Evaluate(Energy)<<endl;
    MVector Size = Volume->GetSize();
    if (IsOverflow == true) { // No depth resolution!
      Pos[2] = 0.0;     
    } else {
      int Trials = 5;
      double Sigma;
      do {
        Sigma = gRandom->Gaus(m_DepthResolution.Evaluate(Energy), 
                              m_DepthResolutionSigma.Evaluate(Energy));
        Trials--;
      } while (Sigma < 0 && Trials >= 0);
      if (Trials <= 0) {
        Sigma = m_DepthResolution.Evaluate(Energy);
      }

      Trials = 10;
      double z = numeric_limits<double>::max();      
      z = gRandom->Gaus(Pos[2], Sigma);
      if (z < -Size[2]) {
        z = -0.9999*Size[2];
      }
      if (z > Size[2]) {
        z = 0.9999*Size[2];
      }

      Pos[2] = z;
    }
    mdebug<<"Noise Cal: Pos after "<<Pos[2]<<" cm"<<endl;
  } else {
    // In theory the simulation should have taken care of this, but if it doesn't, center the position
    Pos[2] = 0.0; 
  }

  // Noise the time:
  ApplyTimeResolution(Time, Energy);

  // Now calibrate
  ApplyEnergyCalibration(Energy);
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDCalorimeter::GetGridPoint(const MVector& Position) const
{
  // Return the grid point for this position in the detector

  MVector Pos = Position;

  // (0, 0, 0) is (-, -, -)-corner of the first sensitive volume
  Pos += m_StructuralDimension;
  Pos -= m_StructuralOffset;

  // The following is ok, since 
  // (a) The position is always in x,y-center and 
  // (b) z is always 0
  int xBar = int(Pos.X()/(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  int yBar = int(Pos.Y()/(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));
  int zBar = 0;

  MDGridPoint Point(xBar, yBar, zBar, MDGridPoint::c_Voxel);

  return Point;
}


////////////////////////////////////////////////////////////////////////////////


vector<MDGridPoint> MDCalorimeter::Discretize(const MVector& PosInDetectorVolume, 
                                              const double& Energy, 
                                              const double& Time, 
                                              MDVolume* DetectorVolume) const
{
  // Discretize Pos to a voxel of this volume

  vector<MDGridPoint> Points;

  MDGridPoint GridPoint = GetGridPoint(PosInDetectorVolume);

  MVector PositionInGrid(0.0, 0.0, 0.0);
  if (m_DepthResolution.GetNDataPoints() > 0) {
    PositionInGrid.SetZ(PosInDetectorVolume.Z() - m_StructuralSize.Z() + 
                        m_StructuralDimension.Z() - m_StructuralOffset.Z());
  }
  GridPoint.SetPosition(PositionInGrid);
  GridPoint.SetEnergy(Energy);
  GridPoint.SetTime(Time);

  Points.push_back(GridPoint);
  
  return Points;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDCalorimeter::GetPositionInDetectorVolume(const unsigned int xGrid, 
                                                   const unsigned int yGrid,
                                                   const unsigned int zGrid,
                                                   const MVector PositionInGrid,
                                                   const unsigned int Type,
                                                   MDVolume* Volume)
{
  // Return the position in the detector volume

  //cout<<"Bar <- "<<xGrid<<":"<<yGrid<<":"<<Depth<<endl;

  // Position in bar relative to its center:
  MVector Position = PositionInGrid;

  // Position in bar relative to its negative edge:
  Position += m_StructuralSize;

  // Position in Detector relative to lower edge of detector at (-x, -y, -z)
  Position.SetX(Position.X() + xGrid*(2*m_StructuralSize.X()+m_StructuralPitch.X()));  
  Position.SetY(Position.Y() + yGrid*(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));  

  // Position in Detector realitive to its negative edge: 
  Position += m_StructuralOffset;

  // Position in detector relaitive to its center:  
  Position -= m_StructuralDimension;

  //cout<<"Pos <- "<<Position<<endl;

  return Position;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDCalorimeter::GetPositionResolution(const MVector& Pos, const double Energy) const
{
  // Return the position resolution at position Pos

  MVector Res;
  Res.SetX(2*m_StructuralSize.GetX()/sqrt(12.0));
  Res.SetY(2*m_StructuralSize.GetY()/sqrt(12.0));

  if (m_DepthResolution.GetSize() > 0) {
    Res.SetZ(m_DepthResolution.Evaluate(Energy));
  } else {
    Res.SetZ(2*m_StructuralSize.GetZ()/sqrt(12.0));
  }

  return Res;
}


////////////////////////////////////////////////////////////////////////////////


bool MDCalorimeter::AreNear(const MVector& Pos1, const MVector& dPos1, 
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

  static const double Epsilon = 0.00001; 

  if (fabs(Pos1[0] - Pos2[0])/(m_StructuralPitch.X() + 2*m_StructuralSize.X())*
      fabs(Pos1[0] - Pos2[0])/(m_StructuralPitch.X() + 2*m_StructuralSize.X()) +
      fabs(Pos1[1] - Pos2[1])/(m_StructuralPitch.Y() + 2*m_StructuralSize.Y())*
      fabs(Pos1[1] - Pos2[1])/(m_StructuralPitch.Y() + 2*m_StructuralSize.Y()) < Level+Epsilon) {
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

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MDCalorimeter::GetGeant3() const
{
  ostringstream out;

  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<"      SENVOL("<<m_SVs[i]->GetSensitiveVolumeID()<<") = '"<<m_SVs[i]->GetShortName()<<"'"<<endl;
    out<<"      SENDET("<<m_SVs[i]->GetSensitiveVolumeID()<<") = "<<m_ID<<endl;
  }

  out<<"      DETNR("<<m_ID<<") = 2"<<endl;
  if (m_DepthResolution.GetNDataPoints() == 0) {
    out<<"      DETTYP("<<m_ID<<") = 2"<<endl;
  } else {
    out<<"      DETTYP("<<m_ID<<") = 3"<<endl;
  }
  out<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDCalorimeter::GetMGeant() const
{
  ostringstream out;

  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    MString Name = m_SVs[i]->GetShortName();
    Name.ToUpper();
    out<<"SENV "<<m_SVs[i]->GetSensitiveVolumeID()<<" "<<Name<<endl;
    out<<"SEND "<<m_SVs[i]->GetSensitiveVolumeID()<<" "<<m_ID<<endl;
  }

  out<<"DTNR "<<m_ID<<" 2"<<endl;
  if (m_DepthResolution.GetNDataPoints() == 0) {
    out<<"DTTP "<<m_ID<<" 2"<<endl;
  } else {
    out<<"DTTP "<<m_ID<<" 3"<<endl;
  }
  out<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDCalorimeter::GetGeomega() const
{
  // Return all detector characteristics in Geomega-Format

  ostringstream out;

  out<<"Calorimeter "<<m_Name<<endl;
  out<<GetGeomegaCommon()<<endl;
  for (unsigned int d = 0; d < m_DepthResolution.GetSize(); ++d) {
    out<<m_Name<<".DepthResolution "<<
      m_DepthResolution.GetDataPointX(d)<<" "<<
      m_DepthResolution.GetDataPointY(d)<<" "<<
      m_DepthResolutionSigma.GetDataPointY(d)<<endl;
  }
  
  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


bool MDCalorimeter::Validate()
{
  // Check if all input is reasonable

  if (MDDetector::Validate() == false) {
    return false;
  }

  if (m_DetectorVolume->GetShape()->GetType() != "BRIK") {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"The detector volume has to be a box!"<<endl;
    return false;
  }
  m_StructuralDimension = 
    dynamic_cast<MDShapeBRIK*>(m_DetectorVolume->GetShape())->GetSize();

  mdebug<<"Structural dimension: "<<m_StructuralDimension<<endl;

  if (m_SVs.size() != 1) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"You need exactly one sensitive volume!"<<endl;
    return false;
  }
  if (m_SVs[0]->GetShape()->GetType() != "BRIK") {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"The sensitive volume has to be a box!"<<endl;
    return false;
  }
  m_StructuralSize = 
    dynamic_cast<MDShapeBRIK*>(m_SVs[0]->GetShape())->GetSize();

  mdebug<<"Structural size: "<<m_StructuralSize<<endl;

  if (m_DepthResolutionType == c_DepthResolutionTypeUnknown) {
     m_DepthResolutionType = c_DepthResolutionTypeNone;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDCalorimeter::CreateBlockedTriggerChannelsGrid()
{
  // Create the grid only if it is really used

  // Initialize Grid:
  double xNGrid = (2*(m_StructuralDimension.X()-m_StructuralOffset.X())+m_StructuralPitch.X())/
    (m_StructuralPitch.X() + 2*m_StructuralSize.X());
  double yNGrid = (2*(m_StructuralDimension.Y()-m_StructuralOffset.Y())+m_StructuralPitch.Y())/
    (m_StructuralPitch.Y() + 2*m_StructuralSize.Y());

  m_BlockedTriggerChannels.Set(MDGrid::c_Voxel, int(xNGrid+0.5), int(yNGrid+0.5));

  mout<<"Grid: x="<<xNGrid<<" y="<<yNGrid<<endl;

  if (fabs(int(xNGrid+0.5) - xNGrid) > 0.01 ||
      fabs(int(yNGrid+0.5) - yNGrid) > 0.01) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Cannot determine an integer as number of bars --- something is wrong with your geometry!"<<endl;
    return;
  }

  m_AreBlockedTriggerChannelsUsed = true;
}



////////////////////////////////////////////////////////////////////////////////


MString MDCalorimeter::ToString() const
{
  //

  ostringstream out;
  out<<MDDetector::ToString()<<endl;

  return out.str().c_str();  
}


// MDCalorimeter.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
