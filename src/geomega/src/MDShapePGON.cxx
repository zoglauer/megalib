/*
 * MDShapePGON.cxx
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
// MDShapePGON
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapePGON.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include <TGeoPgon.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDShapePGON)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapePGON::MDShapePGON() : MDShape()
{
  // default constructor

  m_Phi = 0;
  m_DPhi = 0;
  m_NSides = 0;
  m_NSections = 0;

  m_PGON = 0;

  m_Type = "PGON";
}


////////////////////////////////////////////////////////////////////////////////


MDShapePGON::~MDShapePGON()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapePGON::Initialize(double Phi, double DPhi, unsigned int NSides, 
                             unsigned int NSections)
{
  // Correctly initialize this shape

  if (Phi < 0 || Phi >= 360) {
    mout<<"   ***  Error  ***  in shape PGON "<<endl;
    mout<<"Phi ("<<Phi<<") needs to be within [0;360["<<endl;
    return false;            
  }
  if (DPhi <= 0 || DPhi > 360) {
    mout<<"   ***  Error  ***  in shape PGON "<<endl; 
    mout<<"Phimax ("<<DPhi<<") needs to be within ]0;360]"<<endl; 
    return false;            
  }
  if (NSides < 1) {
    mout<<"   ***  Error  ***  in shape PGON "<<endl;
    mout<<"PGON needs at least one side"<<endl;
    return false;            
  }
  if (NSections < 2) {
    mout<<"   ***  Error  ***  in shape PGON "<<endl;
    mout<<"PGON needs at least two segments"<<endl;
    return false;            
  }

  m_Phi = Phi;
  m_DPhi = DPhi;
  m_NSides = NSides;
  m_NSections = NSections;

  m_Z.resize(NSections);
  m_Rmin.resize(NSections);
  m_Rmax.resize(NSections);

  m_Geo = new TGeoPgon(Phi, DPhi, NSides, NSections);

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MDShapePGON::AddSection(unsigned int Section, double Z, double Rmin, double Rmax)
{
  // Add a section 

  if (Rmin < 0) {
    mout<<"   ***  Error  ***  in shape PGON "<<endl;
    mout<<"rmin ("<<Rmin<<") needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (Rmax < 0) {
    mout<<"   ***  Error  ***  in shape PGON "<<endl;
    mout<<"rmax ("<<Rmax<<") needs to be larger or equal than 0"<<endl;
    return false;            
  }
  if (Rmin >= Rmax) {
    mout<<"   ***  Error  ***  in shape PGON "<<endl;
    mout<<"rmax ("<<Rmax<<") needs to be larger than rmin ("<<Rmin<<")"<<endl;
    return false;            
  }

  m_Z[Section] = Z;
  m_Rmin[Section] = Rmin;
  m_Rmax[Section] = Rmax;
  
  dynamic_cast<TGeoPgon*>(m_Geo)->DefineSection(Section, Z, Rmin, Rmax);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapePGON::CreateShape()
{
  //

  if (m_PGON == 0) {
    m_PGON = new TPGON("Who", "knows...", "void", m_Phi, m_DPhi, m_NSides, m_NSections);
    for (unsigned int i = 0; i < m_NSections; ++i) {
      m_PGON->DefineSection(i, m_Z[i], m_Rmin[i], m_Rmax[i]);
    }
    m_PGON->SetLineColor(m_Color);
    m_PGON->SetFillColor(m_Color);
  }
}


////////////////////////////////////////////////////////////////////////////////


TShape* MDShapePGON::GetShape()
{
  //

  return (TShape *) m_PGON;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapePGON::GetGeant3DIM(MString ShortName)
{
  ostringstream out;

  out<<"      REAL V"<<ShortName<<"VOL"<<endl;
  out<<"      DIMENSION V"<<ShortName<<"VOL("<<4+3*m_NSections<<")"<<endl;  

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapePGON::GetGeant3DATA(MString ShortName)
{
  //

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);
  out<<"      DATA V"<<ShortName<<"VOL/"<<m_Phi<<","<<m_DPhi<<","<<m_NSides<<","<<m_NSections;
  for (unsigned int i = 0; i < m_NSections; ++i) {
    out<<","<<m_Z[i]<<","<<m_Rmin[i]<<","<<m_Rmax[i];
  }
  out<<"/"<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapePGON::GetMGeantDATA(MString ShortName)
{
  // Write the shape parameters in MGEANT/mggpod format.

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  out.precision(6);

  out<<"           "<<m_Phi<<" "<<m_DPhi<<" "<<m_NSides<<" "<<m_NSections<<endl;
  for (unsigned int i = 0; i < m_NSections; ++i) {
    out<<"           "<<m_Z[i]<<" "<<m_Rmin[i]<<" "<<m_Rmax[i]<<endl;
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapePGON::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;
  out<<"PGON "<<m_Phi<<" "<<m_DPhi<<" "<<m_NSides<<" "<<m_NSections;
  for (unsigned int i = 0; i < m_NSections; ++i) {
    out<<" "<<m_Z[i]<<" "<<m_Rmin[i]<<" "<<m_Rmax[i];
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapePGON::GetGeant3ShapeName()
{
  //

  return "PGON";
}


////////////////////////////////////////////////////////////////////////////////


int MDShapePGON::GetGeant3NumberOfParameters()
{
  //

  return 4 + 3*m_NSections;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapePGON::GetSize()
{
  // Return the size of a surrounding box

  double z = 0;
  double max = 0;
  for (unsigned int i = 0; i < m_NSections; ++i) {
    if (m_Rmax[i] > max) max = m_Rmax[i];
    z += m_Z[i];
  }
  return MVector(max, max, z);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePGON::GetPhi()
{
  return m_Phi;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePGON::GetDPhi()
{
  return m_DPhi;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDShapePGON::GetNSides()
{
  return m_NSides;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDShapePGON::GetNSections()
{
  return m_NSections;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePGON::GetZ(unsigned int Section)
{
  if (Section >= m_NSections) {
    massert(false);
  }

  return m_Z[Section];
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePGON::GetRmin(unsigned int Section)
{
  if (Section >= m_NSections) {
    massert(false);
  }

  return m_Rmin[Section];
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePGON::GetRmax(unsigned int Section)
{
  if (Section >= m_NSections) {
    massert(false);
  }

  return m_Rmax[Section];
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePGON::GetVolume()
{
  // Calculate the volume of this object...

  double Volume = 0;
  
  // The shape consists of several small frustums of pyramids
  // Thus four important parameters need to be calculated for the inner and the outer frustrum 
  double A;
  // A: length of the lower outer side
  double a;
  // a: length of the upper outer side
  double F;
  // F: bottom area
  double z;
  // z: height

  for (unsigned int i = 1; i < m_NSections; ++i) {
    z = m_Z[i]-m_Z[i-1];
    A = 2 * m_Rmax[i-1] * tan(m_DPhi/m_NSides*c_Rad/2);
    a = 2 * m_Rmax[i] * tan(m_DPhi/m_NSides*c_Rad/2);
    
    if (A != 0) {
      F = 1.0/2.0 * A * m_Rmax[i-1] * m_NSides;
      Volume += z * F * (1 + a/A + (a/A)*(a/A))/3;
    } else if (a != 0) {
      F = 1.0/2.0 * a * m_Rmax[i] * m_NSides;
      Volume += z * F * (1 + A/a + (A/a)*(A/a))/3;
    }


    A = 2 * m_Rmin[i-1] * tan(m_DPhi/m_NSides*c_Rad/2);
    a = 2 * m_Rmin[i] * tan(m_DPhi/m_NSides*c_Rad/2);
    if (A != 0) {
      F = 1.0/2.0 * A * m_Rmin[i-1] * m_NSides;
      Volume -= z * F * (1 + a/A + (a/A)*(a/A))/3;
    } else if (a != 0) {
      F = 1.0/2.0 * a * m_Rmin[i] * m_NSides;
      Volume -= z * F * (1 + A/a + (A/a)*(A/a))/3;
    }
  }

  return Volume;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapePGON::Scale(const double Factor)
{
  // Scale this shape by Factor

  delete m_Geo;
  m_Geo = new TGeoPgon(m_Phi, m_DPhi, m_NSides, m_NSections);

  for (unsigned int i = 0; i < m_NSections; ++i) {
    m_Rmin[i] *= Factor;
    m_Rmax[i] *= Factor;
    m_Z[i] *= Factor;  
    dynamic_cast<TGeoPcon*>(m_Geo)->DefineSection(i, m_Z[i], m_Rmin[i], m_Rmax[i]);
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapePGON::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  double R = 0.25*(m_Rmax[1]+m_Rmin[1]+m_Rmax[0]+m_Rmin[0]);
  double Angle = (m_Phi+0.5*m_DPhi)*c_Rad; 

  return MVector(R*cos(Angle), R*sin(Angle), (m_Z[1]+m_Z[0])/2.0);
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapePGON::ToString()
{
  // 

  ostringstream out;

  out<<"      PGON ("<<m_Phi<<","<<m_DPhi<<","<<m_NSides<<","<<m_NSections;
  for (unsigned int i = 0; i < m_NSections; ++i) {
    out<<","<<m_Z[i]<<","<<m_Rmin[i]<<","<<m_Rmax[i];
  }
  out<<")"<<endl;

  return out.str().c_str();
}



// MDShapePGON.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
