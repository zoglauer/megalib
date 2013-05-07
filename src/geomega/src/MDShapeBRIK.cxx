/*
 * MDShapeBRIK.cxx
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
// MDShapeBRIK
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeBRIK.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGeoBBox.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDShapeBRIK)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeBRIK::MDShapeBRIK() : MDShape()
{
  // default constructor

  m_Dx = 0;
  m_Dy = 0;
  m_Dz = 0;

  m_BRIK = 0;

  m_Type = "BRIK";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeBRIK::~MDShapeBRIK()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeBRIK::Initialize(double x, double y, double z)
{
  // Correctly initialize this shape

  if (x <= 0) {
    mout<<"   ***  Error  ***  in shape BRIK "<<endl;
    mout<<"x-dimension needs to be larger than 0"<<endl;
    return false;            
  }
  if (y <= 0) {
    mout<<"   ***  Error  ***  in shape BRIK "<<endl;
    mout<<"y-dimension needs to be larger than 0"<<endl;
    return false;            
  }
  if (z <= 0) {
    mout<<"   ***  Error  ***  in shape BRIK "<<endl;
    mout<<"z-dimension needs to be larger than 0"<<endl;
    return false;            
  }

  m_Dx = x;
  m_Dy = y;
  m_Dz = z;

  m_Geo = new TGeoBBox(x, y, z);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeBRIK::CreateShape()
{
  //

  // We only need

  if (m_BRIK == 0) {
    m_BRIK = new TBRIK("Who", "knows...", "void", m_Dx, m_Dy, m_Dz);
    m_BRIK->SetLineColor(m_Color);
    m_BRIK->SetFillColor(m_Color);
  }
}


////////////////////////////////////////////////////////////////////////////////


TShape* MDShapeBRIK::GetShape()
{
  //

  return (TShape *) m_BRIK;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeBRIK::GetGeant3DIM(MString ShortName)
{
  ostringstream out;

  out<<"      REAL V"<<ShortName<<"VOL"<<endl;
  out<<"      DIMENSION V"<<ShortName<<"VOL(3)"<<endl;  

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeBRIK::GetGeant3DATA(MString ShortName)
{
  //

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);
  out<<"      DATA V"<<ShortName<<"VOL/"<<m_Dx<<","<<m_Dy<<","<<m_Dz<<"/"<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeBRIK::GetMGeantDATA(MString ShortName)
{
  // Write the shape parameters in MGEANT/mggpod format.

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);

  out<<"           "<<m_Dx<<" "<<m_Dy<<" "<<m_Dz<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeBRIK::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;

  out<<"BRIK "<<m_Dx<<" "<<m_Dy<<" "<<m_Dz;

  return out.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////


MString MDShapeBRIK::GetGeant3ShapeName()
{
  //

  return "BOX ";
}


////////////////////////////////////////////////////////////////////////////////


int MDShapeBRIK::GetGeant3NumberOfParameters()
{
  //

  return 3;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeBRIK::GetSize()
{
  // Return the size of a surrounding box

  return MVector(m_Dx, m_Dy, m_Dz);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::GetSizeX()
{
  return m_Dx;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::GetSizeY()
{
  return m_Dy;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::GetSizeZ()
{
  return m_Dz;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::GetVolume()
{
  return 8*m_Dx*m_Dy*m_Dz;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeBRIK::ToString()
{
  // 

  ostringstream out;

  out<<"BRIK ("<<m_Dx<<", "<<m_Dy<<", "<<m_Dz<<")"<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeBRIK::IsInside(const MVector& Pos, const double Tolerance, const bool PreferOutside)
{
  // Overwrite to allow some tolerance:

  //massert(Tolerance >= 0);

  if (PreferOutside == false) {
    if (fabs(Pos.m_Z) > m_Dz + Tolerance) {
      return false;
    } 
    if (fabs(Pos.m_X) > m_Dx + Tolerance) {
      return false; 
    }
    if (fabs(Pos.m_Y) > m_Dy + Tolerance) {
      return false; 
    }
  } else {
    if (fabs(Pos.m_Z) > m_Dz - Tolerance) {
      return false;
    } 
    if (fabs(Pos.m_X) > m_Dx - Tolerance) {
      return false; 
    }
    if (fabs(Pos.m_Y) > m_Dy - Tolerance) {
      return false; 
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeBRIK::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_Dx *= Factor;
  m_Dy *= Factor;
  m_Dz *= Factor;
  
  delete m_Geo;
  m_Geo = new TGeoBBox(m_Dx, m_Dy, m_Dz);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeBRIK::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return MVector(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeBRIK::GetRandomPositionInside()
{
  // Return a random position inside this shape

  return MVector((2.0*gRandom->Rndm()-1.0)*m_Dx, 
                 (2.0*gRandom->Rndm()-1.0)*m_Dy, 
                 (2.0*gRandom->Rndm()-1.0)*m_Dz);
}


////////////////////////////////////////////////////////////////////////////////


vector<MVector> MDShapeBRIK::CreateSurfacePattern(const unsigned int Detail) const
{
 	// Create a vector of points which are on the surface of this volume
  // and can be used to check for intersections

  vector<MVector> Points;

  unsigned int Bins = 2 + Detail;

  double xDelta = 2*m_Dx/(Bins-1); 
  double yDelta = 2*m_Dy/(Bins-1); 
  double zDelta = 2*m_Dz/(Bins-1);

  for (unsigned int bx = 0; bx < Bins; ++bx) {
    for (unsigned int by = 0; by < Bins; ++by) {
      for (unsigned int bz = 0; bz < Bins; ++bz) {
        // If at least one parameter is at the surface:
        if (bx == 0 || by == 0 || bz == 0 || bx == Bins-1 || by == Bins-1 || bz == Bins-1) {
          Points.push_back(MVector(-m_Dx+bx*xDelta, -m_Dy+by*yDelta, -m_Dz+bz*zDelta));
        }
      }
    }
  }

	return Points; 
}


// MDShapeBRIK.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
