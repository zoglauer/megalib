/*
 * MDShapeTUBS.cxx
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
// MDShapeTUBS
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeTUBS.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGeoTube.h"
#include "TRandom.h"

// MEGALib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDShapeTUBS)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeTUBS::MDShapeTUBS() : MDShape()
{
  // default constructor

  m_Rmin = 0;
  m_Rmax = 0;
  m_HalfHeight = 0;
  m_Phi1 = 0;
  m_Phi2 = 0;

  m_TUBS = 0;

  m_Type = "TUBS";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeTUBS::~MDShapeTUBS()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTUBS::Initialize(double Rmin, double Rmax, double HalfHeight, 
                             double Phi1, double Phi2) 
{
  // default constructor

  if (Rmin < 0) {
    mout<<"   ***  Error  ***  in shape TUBS "<<endl;
    mout<<"Rmin needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (Rmax <= 0) {
    mout<<"   ***  Error  ***  in shape TUBS "<<endl;
    mout<<"Rmax needs to be larger then 0"<<endl;
    return false;            
  }
  if (Rmin >= Rmax) {
    mout<<"   ***  Error  ***  in shape TUBS "<<endl;
    mout<<"Rmax needs to be larger than Rmin"<<endl;
    return false;            
  }
  if (HalfHeight <= 0) {
    mout<<"   ***  Error  ***  in shape TUBS "<<endl;
    mout<<"HalfHeight needs to be larger than zero"<<endl;
    return false;            
  }
  if (Phi1 < 0 || Phi1 >= 360) {
    mout<<"   ***  Error  ***  in shape TUBS "<<endl;
    mout<<"Phi1 needs to be within [0;360["<<endl;
    return false;            
  }
  if (Phi2-Phi1 <= 0 || Phi2-Phi1 > 360) {
    mout<<"   ***  Error  ***  in shape TUBS "<<endl;
    mout<<"Phi2-Phi1 needs to be within ]0;360]"<<endl;
    return false;            
  }
  if (Phi1 >= Phi2) {
    mout<<"   ***  Error  ***  in shape TUBS "<<endl;
    mout<<"Phi2 needs to be larger than Phi1"<<endl;
    return false;            
  }

  m_Rmin = Rmin;
  m_Rmax = Rmax;
  m_HalfHeight = HalfHeight;
  m_Phi1 = Phi1;
  m_Phi2 = Phi2;

  m_Geo = new TGeoTubeSeg(m_Rmin, m_Rmax, m_HalfHeight, m_Phi1, m_Phi2);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeTUBS::CreateShape()
{
  //

  if (m_TUBS == 0) {
    m_TUBS = new TTUBS("Who", "knows...", "void", m_Rmin, 
                       m_Rmax, m_HalfHeight, m_Phi1, m_Phi2);
  }
}

////////////////////////////////////////////////////////////////////////////////



double MDShapeTUBS::GetRmin()
{
  //

  return m_Rmin;
}

////////////////////////////////////////////////////////////////////////////////


double MDShapeTUBS::GetRmax()
{
  //

  return m_Rmax;
}

////////////////////////////////////////////////////////////////////////////////


double MDShapeTUBS::GetHeight()
{
  //

  return m_HalfHeight;
}

////////////////////////////////////////////////////////////////////////////////


double MDShapeTUBS::GetPhi1()
{
  //

  return m_Phi1;
}

////////////////////////////////////////////////////////////////////////////////


double MDShapeTUBS::GetPhi2()
{
  //

  return m_Phi2;
}

////////////////////////////////////////////////////////////////////////////////


TShape* MDShapeTUBS::GetShape()
{
  //

  return (TShape *) m_TUBS;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTUBS::GetGeant3DIM(MString ShortName)
{
  ostringstream out;

  out<<"      REAL V"<<ShortName<<"VOL"<<endl;
  out<<"      DIMENSION V"<<ShortName<<"VOL(5)"<<endl;  

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTUBS::GetGeant3DATA(MString ShortName)
{
  //

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);
  out<<"      DATA V"<<ShortName<<"VOL/"<<m_Rmin<<","<<m_Rmax<<","<<m_HalfHeight<<","<<m_Phi1<<","<<m_Phi2<<"/"<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTUBS::GetMGeantDATA(MString ShortName)
{
  // Write the shape parameters in MGEANT/mggpod format.
  
  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);

  out<<"           "<<m_Rmin<<" "<<m_Rmax<<" "<<m_HalfHeight<<endl;
  out<<"           "<<m_Phi1<<" "<<m_Phi2<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTUBS::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;
  out<<"TUBS "<<m_Rmin<<" "<<m_Rmax<<" "<<m_HalfHeight<<" "<<m_Phi1<<" "<<m_Phi2;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTUBS::GetGeant3ShapeName()
{
  //

  return "TUBS";
}


////////////////////////////////////////////////////////////////////////////////


int MDShapeTUBS::GetGeant3NumberOfParameters()
{
  //

  return 5;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTUBS::GetSize()
{
  // Return the size of a surrounding box

  return MVector(m_Rmax, m_Rmax, m_HalfHeight);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTUBS::GetVolume()
{
  // Return the volume of this cylinder

  return 2*m_HalfHeight*(m_Rmax*m_Rmax - m_Rmin*m_Rmin)*c_Pi*(m_Phi2-m_Phi1)/360;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeTUBS::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_Rmin *= Factor;
  m_Rmax *= Factor;
  m_HalfHeight *= Factor;

  delete m_Geo;
  m_Geo = new TGeoTubeSeg(m_Rmin, m_Rmax, m_HalfHeight, m_Phi1, m_Phi2);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTUBS::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  double R = 0.0;
  if (m_Rmin > 0.0) R = 0.5*(m_Rmax+m_Rmin);
  double Angle = 0.5*(m_Phi2+m_Phi1)*c_Rad; 

  return MVector(R*cos(Angle), R*sin(Angle), 0.0);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTUBS::GetRandomPositionInside()
{
  // Return a random position inside this shape

  double Phi = m_Phi1 + gRandom->Rndm()*(m_Phi2-m_Phi1);
  double R = sqrt(m_Rmin*m_Rmin + (m_Rmax*m_Rmax-m_Rmin*m_Rmin)*gRandom->Rndm());
  double H = (2*gRandom->Rndm()-1)*m_HalfHeight;

  return MVector(R*cos(Phi*c_Rad), R*sin(Phi*c_Rad), H);
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTUBS::ToString()
{
  // 

  ostringstream out;

  out<<"TUBS ("<<m_Rmin<<","<<m_Rmax<<","<<m_HalfHeight<<","<<m_Phi1<<","<<m_Phi2<<","<<")"<<endl;

  return out.str().c_str();
}


// MDShapeTUBS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
