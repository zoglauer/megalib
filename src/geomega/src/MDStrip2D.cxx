/*
 * MDStrip2D.cxx
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
// MDStrip2D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDStrip2D.h"

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


#ifdef ___CINT___
ClassImp(MDStrip2D)
#endif


////////////////////////////////////////////////////////////////////////////////


MDStrip2D::MDStrip2D(MString String) : MDDetector(String)
{
  // default constructor

  m_Type = c_Strip2D;
  m_Description = c_Strip2DName;
  m_Orientation = 2; // z-axis

  m_WidthX = g_DoubleNotDefined;
  m_WidthY = g_DoubleNotDefined;
  m_OffsetX = g_DoubleNotDefined;
  m_OffsetY = g_DoubleNotDefined;
  m_PitchX = g_DoubleNotDefined;
  m_PitchY = g_DoubleNotDefined;
  m_NStripsX = g_IntNotDefined;
  m_NStripsY = g_IntNotDefined;
  m_NWafersX = g_IntNotDefined;
  m_NWafersY = g_IntNotDefined;
  m_StripLengthX = g_DoubleNotDefined;
  m_StripLengthY = g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MDStrip2D::MDStrip2D(const MDStrip2D& S) : MDDetector(S)
{
  m_WidthX = S.m_WidthX;
  m_WidthY = S.m_WidthY;
  m_OffsetX = S.m_OffsetX;
  m_OffsetY = S.m_OffsetY;
  m_PitchX = S.m_PitchX;
  m_PitchY = S.m_PitchY;
  m_NStripsX = S.m_NStripsX;
  m_NStripsY = S.m_NStripsY;
  m_NWafersX = S.m_NWafersX;
  m_NWafersY = S.m_NWafersY;
  m_StripLengthX = S.m_StripLengthX;
  m_StripLengthY = S.m_StripLengthY;
  
  m_Orientation = S.m_Orientation;
  
  if (m_HasGuardRing == true) {
    m_GuardRing->SetMotherDetector(this); 
  }
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDStrip2D::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return dynamic_cast<MDDetector*>(new MDStrip2D(*this));
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip2D::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors
    
  MDDetector::CopyDataToNamedDetectors();
  
  if (m_IsNamedDetector == true) return true;
  
  for (unsigned int n = 0; n < m_NamedDetectors.size(); ++n) {
    if (dynamic_cast<MDStrip2D*>(m_NamedDetectors[n]) == 0) {
      mout<<"   ***  Internal error  ***  in detector "<<m_Name<<endl;
      mout<<"We have a named detector ("<<m_NamedDetectors[n]->GetName()<<") which is not of the same type as the base detector!"<<endl;
      return false;
    }
    MDStrip2D* D = dynamic_cast<MDStrip2D*>(m_NamedDetectors[n]);
    
    D->m_WidthX = m_WidthX;
    D->m_WidthY = m_WidthY;
    D->m_OffsetX = m_OffsetX;
    D->m_OffsetY = m_OffsetY;
    D->m_PitchX = m_PitchX;
    D->m_PitchY = m_PitchY;
    D->m_NStripsX = m_NStripsX;
    D->m_NStripsY = m_NStripsY;
    D->m_NWafersX = m_NWafersX;
    D->m_NWafersY = m_NWafersY;
    D->m_StripLengthX = m_StripLengthX;
    D->m_StripLengthY = m_StripLengthY;
  
    D->m_Orientation = m_Orientation;
    
    if (D->HasGuardRing() == true) {
      D->m_GuardRing->SetMotherDetector(D);
    }
  }
   
  return true; 
}
  

////////////////////////////////////////////////////////////////////////////////


MDStrip2D::~MDStrip2D()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


//! Set that this detector has a guard ring
void MDStrip2D::HasGuardRing(bool HasGuardRing)
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


void MDStrip2D::Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const
{
  // Noise energy of this hit:
  
  if (m_NoiseActive == false) return;

  // Test for failure:
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


vector<MDGridPoint> MDStrip2D::Discretize(const MVector& PosInDetector, 
                                          const double& Energy, 
                                          const double& Time, 
                                          MDVolume* DetectorVolume) const
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


MDGridPoint MDStrip2D::GetGridPoint(const MVector& PosInDetector) const
{
  // Discretize Pos to a voxel of this volume

  MDGridPoint GridPoint(0, 0, 0, MDGridPoint::c_Unknown);

  MVector Pos = PosInDetector;

  // Translate into center of sensitive volume
  Pos += m_StructuralDimension;
  Pos -= m_StructuralOffset;

  //cout<<Pos.X()/(2*m_StructuralSize.X()+m_StructuralPitch.X())<<endl;
  int xWafer = int(Pos.X()/(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  int yWafer = int(Pos.Y()/(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));

  // Check if we have a reasonable wafer:
  if (xWafer < 0 || xWafer >= m_NWafersX) {
    mout<<"Invalid x-wafer number: "<<xWafer<<" Max: "<<m_NWafersX-1
        <<" (Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }
  if (yWafer < 0 || yWafer >= m_NWafersY) {
    mout<<"Invalid y-wafer number: "<<yWafer<<" Max: "<<m_NWafersY-1
        <<" (Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }

  Pos.SetX(Pos.X() - xWafer*(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  Pos.SetY(Pos.Y() - yWafer*(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));
  Pos -= m_StructuralSize;
  
  // Ignore the hit if it is out side the sensitive part (guard ring?):
  if (fabs(Pos.X()) > m_WidthX/2.0 - m_OffsetX || 
      fabs(Pos.Y()) > m_WidthY/2.0 - m_OffsetY) {
    if (fabs(Pos.X()) > m_WidthX/2.0 || 
      fabs(Pos.Y()) > m_WidthY/2.0) {
      merr<<"Hit outside detector: "<<m_Name<<": "
          <<Pos<<" <-> ("<<m_WidthX/2.0<<", "<<m_WidthY/2.0<<")"<<endl;
    } else {
      mdebug<<"Hit in guard ring:  "<<m_Name<<":("
            <<Pos[0]<<", "<<Pos[1]<<", "<<Pos[2]<<")"<<endl;

      GridPoint.Set(0, 0, 0, MDGridPoint::c_GuardRing);
    }
    return GridPoint;
  }

  int xStrip;
  if (m_NStripsX == 1 || m_PitchX == 0) {
    xStrip = 0;
  } else {
    xStrip = (int) ((Pos.X() + m_WidthX/2 - m_OffsetX)/m_PitchX);
  }

  int yStrip;
  if (m_NStripsY == 1 || m_PitchY == 0) {  
    yStrip = 0;
  } else {
    yStrip = (int) ((Pos.Y() + m_WidthY/2 - m_OffsetY)/m_PitchY);
  }

  // Check if we have a reasonable strip:
  if (xStrip+xWafer*m_NStripsX < 0 || xStrip+xWafer*m_NStripsX >= (xWafer+1)*m_NStripsX) {
    mout<<"Invalid x-strip number: "<<xStrip+xWafer*m_NStripsX
        <<" (S="<<xStrip<<", W="<<xWafer<<")"<<endl
        <<"   Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }
  if (yStrip+yWafer*m_NStripsY < 0 || yStrip+yWafer*m_NStripsY >= (yWafer+1)*m_NStripsY) {
    mout<<"Invalid y-strip number: "<<yStrip+yWafer*m_NStripsY
        <<" (S="<<yStrip<<", W="<<yWafer<<")"<<endl
        <<" - Position was "<<Pos[0]<<", "
        <<Pos[1]<<", "<<Pos[2]<<endl;
    mout<<"Check your detector information!"<<endl;
    return GridPoint;
  }

  GridPoint.Set(xStrip+xWafer*m_NStripsX, 
                yStrip+yWafer*m_NStripsY,
                0,
                MDGridPoint::c_Voxel);

  // cout<<xStrip+xWafer*m_NStripsX<<":"<<yStrip+yWafer*m_NStripsY<<":"<<(xWafer+1)*m_NStripsX<<endl;

  return GridPoint;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDStrip2D::GetPositionInDetectorVolume(const unsigned int xGrid, 
                                               const unsigned int yGrid,
                                               const unsigned int zGrid,
                                               const MVector PositionInGrid,
                                               const unsigned int Type,
                                               MDVolume* Volume)
{
  // Return the position in the detector volume

  int xWafer = int(xGrid/m_NStripsX);
  int yWafer = int(yGrid/m_NStripsY);

  MVector Position;

  // Position in Wafer relative to its center:
  if (Type == MDGridPoint::c_GuardRing) {
    if (m_HasGuardRing == true) {
      Position = m_GuardRing->GetUniquePosition();
    } else {
      merr<<"This detector has no guard ring!"<<endl;
    }
  } else {
    Position.SetX(-m_WidthX/2 + m_OffsetX + (xGrid - xWafer*m_NStripsX + 0.5)*m_PitchX);
    Position.SetY(-m_WidthY/2 + m_OffsetY + (yGrid - yWafer*m_NStripsY + 0.5)*m_PitchY);
    Position.SetZ(0.0);
  }

  // Position in Wafer relative to its negative edge:
  Position += m_StructuralSize;

  // Position in Detector relative to lower edge of detector at (-x, -y, -z)
  Position.SetX(Position.X() + xWafer*(2*m_StructuralSize.X()+m_StructuralPitch.X()));  
  Position.SetY(Position.Y() + yWafer*(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));  

  // Position in Detector realitive to its negative edge: 
  Position += m_StructuralOffset;

  // Position in detector relaitive to its center:  
  Position -= m_StructuralDimension;

  return Position;
}


////////////////////////////////////////////////////////////////////////////////


void MDStrip2D::SetWidth(const double x, const double y)
{
  m_WidthX = x;
  m_WidthY = y;
}


////////////////////////////////////////////////////////////////////////////////


void MDStrip2D::SetOffset(const double x, const double y)
{
  m_OffsetX = x;
  m_OffsetY = y;
}


////////////////////////////////////////////////////////////////////////////////


void MDStrip2D::SetPitch(const double x, const double y)
{
  m_PitchX = x;
  m_PitchY = y;
}


////////////////////////////////////////////////////////////////////////////////


void MDStrip2D::SetStripLength(const double x, const double y)
{
  m_StripLengthX = x;
  m_StripLengthY = y;
}


////////////////////////////////////////////////////////////////////////////////


void MDStrip2D::SetNStrips(const int x, const int y)
{
  m_NStripsX = x;
  m_NStripsY = y;
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip2D::GetGeant3() const
{
  ostringstream out;

  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);
  
  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<"      SENVOL("<<m_SVs[i]->GetSensitiveVolumeID()<<") = '"<<m_SVs[i]->GetShortName()<<"'"<<endl;
    out<<"      SENDET("<<m_SVs[i]->GetSensitiveVolumeID()<<") = "<<m_ID<<endl;
  }

  out<<"      DETNR("<<m_ID<<") = 1"<<endl;
  out<<"      DETTYP("<<m_ID<<") = 1"<<endl;
  out<<"      WIDTH("<<m_ID<<",1) = "<<m_WidthX<<endl;
  out<<"      WIDTH("<<m_ID<<",2) = "<<m_WidthY<<endl;
  out<<"      OFFSET("<<m_ID<<",1) = "<<m_OffsetX<<endl;
  out<<"      OFFSET("<<m_ID<<",2) = "<<m_OffsetY<<endl;
  out<<"      SPITCH("<<m_ID<<",1) = "<<m_PitchX<<endl;
  out<<"      SPITCH("<<m_ID<<",2) = "<<m_PitchY<<endl;
  out<<"      SLENGTH("<<m_ID<<",1) = "<<m_StripLengthX<<endl;
  out<<"      SLENGTH("<<m_ID<<",2) = "<<m_StripLengthY<<endl;
  out<<"      NSTRIP("<<m_ID<<",1) = "<<m_NStripsX<<endl;
  out<<"      NSTRIP("<<m_ID<<",2) = "<<m_NStripsY<<endl;
  out<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip2D::GetMGeant() const
{
  ostringstream out;

  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);
  
  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    MString Name = m_SVs[i]->GetShortName();
    Name.ToUpper();
    out<<"SENV "<<m_SVs[i]->GetSensitiveVolumeID()<<" "<<Name<<endl;
    out<<"SEND "<<m_SVs[i]->GetSensitiveVolumeID()<<" "<<m_ID<<endl;
  }

  out<<"DTNR "<<m_ID<<" 1 "<<endl;
  out<<"DTTP "<<m_ID<<" 1 "<<endl;
  out<<"WIDT "<<m_ID<<" 1 "<<m_WidthX<<endl;
  out<<"WIDT "<<m_ID<<" 2 "<<m_WidthY<<endl;
  out<<"OFFS "<<m_ID<<" 1 "<<m_OffsetX<<endl;
  out<<"OFFS "<<m_ID<<" 2 "<<m_OffsetY<<endl;
  out<<"SPIT "<<m_ID<<" 1 "<<m_PitchX<<endl;
  out<<"SPIT "<<m_ID<<" 2 "<<m_PitchY<<endl;
  out<<"SLEN "<<m_ID<<" 1 "<<m_StripLengthX<<endl;
  out<<"SLEN "<<m_ID<<" 2 "<<m_StripLengthY<<endl;
  out<<"NSTP "<<m_ID<<" 1 "<<m_NStripsX<<endl;
  out<<"NSTP "<<m_ID<<" 2 "<<m_NStripsY<<endl;
  if (m_HasGuardRing == true) {
    out<<"GRUP "<<m_ID<<" "<<m_GuardRing->GetUniquePosition().X()<<" "<<m_GuardRing->GetUniquePosition().Y()<<" "<<m_GuardRing->GetUniquePosition().Z()<<endl;
  }
  out<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip2D::GetGeant3Divisions() const
{
  ostringstream out;

  if (m_UseDivisions == true) {
    out.setf(ios::fixed, ios::floatfield);
    out.precision(4);

    out<<"      CALL GSDVX('"
       <<m_ShortNameDivisionX<<"', '"
       <<m_SVs[0]->GetShortName()<<"', "
       <<m_NStripsX<<", "
       <<1<<", "
       <<m_PitchX<<", "
       <<-m_StructuralSize.X() + m_OffsetX<<", "
       <<0<<", "
       <<m_NStripsX<<")"<<endl;
    out<<"      CALL GSDVX('"
       <<m_ShortNameDivisionY<<"', '"
       <<m_ShortNameDivisionX<<"', "
       <<m_NStripsY<<", "
       <<2<<", "
       <<m_PitchY<<", "
       <<-m_StructuralSize.Y() + m_OffsetY<<", "
       <<0<<", "
       <<m_NStripsY<<")"<<endl;
  }

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip2D::GetMGeantDivisions() const
{
  ostringstream out;

  if (m_UseDivisions == true) {
    out.setf(ios::fixed, ios::floatfield);
    out.precision(4);

    MString VolumeName = m_SVs[0]->GetShortName();
    VolumeName.ToUpper();
    MString ShortNameDivisionX = m_ShortNameDivisionX;
    ShortNameDivisionX.ToUpper();
    MString ShortNameDivisionY = m_ShortNameDivisionY;
    ShortNameDivisionY.ToUpper();
    MString MaterialName = m_SVs[0]->GetMaterial()->GetMGeantShortName();
    MaterialName.ToUpper();

    out<<"divx "
       <<ShortNameDivisionX<<" "
       <<VolumeName<<" "
       <<m_NStripsX<<" "
       <<1<<" "
       <<m_PitchX<<" "
       <<-m_StructuralSize.X() + m_OffsetX<<" "
       <<MaterialName<<" "
       <<m_NStripsX<<endl;
    out<<"divx "
       <<ShortNameDivisionY<<" "
       <<ShortNameDivisionX<<" "
       <<m_NStripsY<<" "
       <<2<<" "
       <<m_PitchY<<" "
       <<-m_StructuralSize.Y() + m_OffsetY<<" "
       <<MaterialName<<" "
       <<m_NStripsY<<endl;
  }

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip2D::GetGeomega() const
{
  // Return all detector characteristics in Geomega-Format

  ostringstream out;

  out<<"Strip2D "<<m_Name<<endl;
  out<<GetGeomegaCommon()<<endl;
  out<<m_Name<<".Offset "<<m_OffsetX<<" "<<m_OffsetY<<endl;
  out<<m_Name<<".StripNumber "<<m_NStripsX<<" "<<m_NStripsY<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip2D::ToString() const
{
  // Dump some detector info 

  ostringstream out;

  out<<MDDetector::ToString()<<endl;
  out<<"   width: "<<m_WidthX<<", "<<m_WidthY<<endl;
  out<<"   offset: "<<m_OffsetX<<", "<<m_OffsetY<<endl;
  out<<"   pitch: "<<m_PitchX<<", "<<m_PitchY<<endl;
  out<<"   striplength: "<<m_StripLengthX<<", "<<m_StripLengthY<<endl;
  out<<"   stripnumber: "<<m_NStripsX<<", "<<m_NStripsY<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip2D::AreNear(const MVector& Pos1, const MVector& dPos1, 
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

  if (fabs(Pos1[0] - Pos2[0])/m_PitchX*fabs(Pos1[0] - Pos2[0])/m_PitchX +
      fabs(Pos1[1] - Pos2[1])/m_PitchY*fabs(Pos1[1] - Pos2[1])/m_PitchY < Level+Epsilon) {
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
  //cout<<"Not adjacent: "<<fabs(Pos1[0] - Pos2[0])/m_PitchX*fabs(Pos1[0] - Pos2[0])/m_PitchX<<"!"
  //    <<fabs(Pos1[1] - Pos2[1])/m_PitchY*fabs(Pos1[1] - Pos2[1])/m_PitchY<<"!"<<Level+Epsilon<<endl;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDStrip2D::GetPositionResolution(const MVector& Pos, const double Energy) const
{
  // Assume we want the standard deviation of a function uniformly distributed between a and b
  // Variance and standard deviation are:
  // Var(X) = E((X-µ)^2) = \int^a_b (X-µ)^2 * f(x) dx.
  // where µ:    mean value: (a+b)/2
  //       f(x): probability density function: f(x)=1/(b-a) (Integral is one!)
  // Var(X) = (b-a)^2 / 12
  // S(X) = (b-a) / sqrt(12)

  return MVector(m_PitchX/sqrt(12.0), m_PitchY/sqrt(12.0), m_StructuralSize[2]/sqrt(12.0));
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip2D::DetermineStrips(const MVector& Pos, int& xStrip, int& yStrip) const
{
  // Determine the x and y strips...

  mdep<<"Please use GetGridPoint!"<<endl;
  
  MDGridPoint P = GetGridPoint(Pos);

  if (P.GetType() == MDGridPoint::c_Unknown) {
    return false;
  }

  xStrip = P.GetXGrid();
  yStrip = P.GetYGrid();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip2D::Validate()
{
  // Check if all input is reasonable

  if (MDDetector::Validate() == false) {
    return false;
  }

  // The detector volume and the sensitive volume need to be boxes OR
  // The first volume of an intersection or subtraction must be a box
  
  MDShapeBRIK* DetectorShape = 0;
  if (m_DetectorVolume->GetShape()->GetType() == "BRIK") {
    DetectorShape = dynamic_cast<MDShapeBRIK*>(m_DetectorVolume->GetShape());
  } else if (m_DetectorVolume->GetShape()->GetType() == "Subtraction" && dynamic_cast<MDShapeSubtraction*>(m_DetectorVolume->GetShape())->GetMinuend()->GetType() == "BRIK") {
    DetectorShape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeSubtraction*>(m_DetectorVolume->GetShape())->GetMinuend());
  } else if (m_DetectorVolume->GetShape()->GetType() == "Intersection" && dynamic_cast<MDShapeIntersection*>(m_DetectorVolume->GetShape())->GetShapeA()->GetType() == "BRIK") {
    DetectorShape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeIntersection*>(m_DetectorVolume->GetShape())->GetShapeA());
  }
    
  if (DetectorShape == 0) {
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
  MDShapeBRIK* SensitiveShape = 0;
  if (m_SVs[0]->GetShape()->GetType() == "BRIK") {
    SensitiveShape = dynamic_cast<MDShapeBRIK*>(m_SVs[0]->GetShape());
  } else if (m_SVs[0]->GetShape()->GetType() == "Subtraction" && dynamic_cast<MDShapeSubtraction*>(m_SVs[0]->GetShape())->GetMinuend()->GetType() == "BRIK") {
    SensitiveShape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeSubtraction*>(m_SVs[0]->GetShape())->GetMinuend());
  } else if (m_SVs[0]->GetShape()->GetType() == "Intersection" && dynamic_cast<MDShapeIntersection*>(m_SVs[0]->GetShape())->GetShapeA()->GetType() == "BRIK") {
    SensitiveShape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeIntersection*>(m_SVs[0]->GetShape())->GetShapeA());    
  }
  if (SensitiveShape == 0) {
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
    mout<<"All offsets have to be positive!"<<endl;
    return false;
  }

  if (m_NStripsX == g_IntNotDefined || m_NStripsY == g_IntNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"The strip/wire detector has no defined number of strips!"<<endl;
    return false;
  }

  if (m_NStripsX <= 0 || m_NStripsY <= 0 ) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"You need to have strips in x and y direction!"<<endl;
    return false;
  }

  // Per definition the detector is oriented in x-y-direction
  // So we can calculate the rest:
  m_WidthX = 2*m_StructuralSize.X();
  m_WidthY = 2*m_StructuralSize.Y();

  mdebug<<"Estimating width: "<<m_WidthX<<", "<<m_WidthY<<endl;

  m_StripLengthX = m_WidthX-2*m_OffsetX;
  m_StripLengthY = m_WidthY-2*m_OffsetY;

  mdebug<<"Estimating strip length: "<<m_StripLengthX<<", "<<m_StripLengthY<<endl;

  m_PitchX = m_StripLengthX/m_NStripsX;
  m_PitchY = m_StripLengthY/m_NStripsY;

  mdebug<<"Estimating pitch: "<<m_PitchX<<", "<<m_PitchY<<endl;

  m_NWafersX = int((2*m_StructuralDimension.X() - m_StructuralOffset.X())/
                 (2*m_StructuralSize.X()+m_StructuralPitch.X()))+1;
  m_NWafersY = int((2*m_StructuralDimension.Y() - m_StructuralOffset.Y())/
                 (2*m_StructuralSize.Y()+m_StructuralPitch.Y()))+1;

  if (m_NWafersX <= 0 || m_NWafersY <= 0 ) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"You need to have at least one wafer in X and Y direction!"<<endl;
    return false;
  }

  mdebug<<"Estimating wafers: "<<m_NWafersX<<", "<<m_NWafersY<<endl;

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
                                     0.0);
    if (m_DetectorVolume->GetVolume(UniqueGuardRingPosition, true) != 0 &&
        m_DetectorVolume->GetVolume(UniqueGuardRingPosition, true)->IsSensitive() == false) {
      UniqueGuardRingPosition.SetXYZ(0.5*(-m_WidthX+m_OffsetX),
                                       0.0,
                                       0.0);
      
      // Still no sensitive volume as guard ring found... Let's try random...
      int Trials = 100;
      while (m_DetectorVolume->GetVolume(UniqueGuardRingPosition, true) == 0 ||
             m_DetectorVolume->GetVolume(UniqueGuardRingPosition, true)->IsSensitive() == false) {
        if (--Trials < 0) {
          mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
          mout<<"Unable to find a unique guard ring hit position!"<<endl;
          return false;        
        }
        
        if (gRandom->Rndm() < 0.5) {
          if (gRandom->Rndm() < 0.5) {
            UniqueGuardRingPosition.SetXYZ(m_WidthX*(gRandom->Rndm()-0.5),
                                             +0.5*m_WidthY - gRandom->Rndm()*m_OffsetY,
                                             m_StructuralDimension.Z()*(gRandom->Rndm()-0.5));
          } else {
            UniqueGuardRingPosition.SetXYZ(m_WidthX*(gRandom->Rndm()-0.5),
                                             -0.5*m_WidthY + gRandom->Rndm()*m_OffsetY,
                                             m_StructuralDimension.Z()*(gRandom->Rndm()-0.5));
          }
        } else {
          if (gRandom->Rndm() < 0.5) {
            UniqueGuardRingPosition.SetXYZ(+0.5*m_WidthX - gRandom->Rndm()*m_OffsetX,
                                             m_WidthY*(gRandom->Rndm()-0.5),
                                             m_StructuralDimension.Z()*(gRandom->Rndm()-0.5));
          } else {
            UniqueGuardRingPosition.SetXYZ(-0.5*m_WidthX + gRandom->Rndm()*m_OffsetX,
                                             m_WidthY*(gRandom->Rndm()-0.5),
                                             m_StructuralDimension.Z()*(gRandom->Rndm()-0.5));
          }
        }
      }
    }
    m_GuardRing->SetUniquePosition(UniqueGuardRingPosition);
    
    if (m_GuardRing->Validate() == false) {
      return false; 
    }
  }

  if (m_EnergyLossType == c_EnergyLossTypeMap) {
    if (fabs(m_EnergyLossMap.GetXMin() - (-m_StructuralSize.X()+m_OffsetX)) > 10E-7 ||
        fabs(m_EnergyLossMap.GetXMax() - (+m_StructuralSize.X()-m_OffsetX)) > 10E-7 ||
        fabs(m_EnergyLossMap.GetYMin() - (-m_StructuralSize.Y()+m_OffsetY)) > 10E-7 ||
        fabs(m_EnergyLossMap.GetYMax() - (+m_StructuralSize.Y()-m_OffsetY)) > 10E-7) {

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


void MDStrip2D::CreateBlockedTriggerChannelsGrid()
{
  // Create the grid only if it is really used

  m_BlockedTriggerChannels.Set(MDGrid::c_Voxel, m_NStripsX*m_NWafersX, m_NStripsY*m_NWafersY);
  m_AreBlockedTriggerChannelsUsed = true;
}


// MDStrip2D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
