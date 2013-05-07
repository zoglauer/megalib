/*
 * MDShapeCONE.cxx
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
// MDShapeCONS
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeCONE.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include <TGeoCone.h>

// MEGALib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDShapeCONE)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeCONE::MDShapeCONE() : MDShape()
{
  // default constructor

  m_RminBottom = 0;
  m_RmaxBottom = 0;
  m_RminTop = 0;
  m_RmaxTop = 0;
  m_HalfHeight = 0;

  m_CONE = 0;

  m_Type = "CONE";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeCONE::~MDShapeCONE()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeCONE::Initialize(double HalfHeight, 
                             double RminBottom, double RmaxBottom, 
                             double RminTop, double RmaxTop) 
{
  // default constructor

  if (RminBottom < 0) {
    mout<<"   ***  Error  ***  in shape CONE "<<endl;
    mout<<"RminBottom needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (RmaxBottom <= 0) {
    mout<<"   ***  Error  ***  in shape CONE "<<endl;
    mout<<"RmaxBottom needs to be larger then 0"<<endl;
    return false;            
  }
  if (RminBottom >= RmaxBottom) {
    mout<<"   ***  Error  ***  in shape CONE "<<endl;
    mout<<"RmaxBottom needs to be larger than RminBottom"<<endl;
    return false;            
  }
  if (RminTop < 0) {
    mout<<"   ***  Error  ***  in shape CONE "<<endl;
    mout<<"RminTop needs to be larger or equal 0: "<<endl;
    return false;            
  }
  if (RmaxTop < 0) {
    mout<<"   ***  Error  ***  in shape CONE "<<endl;
    mout<<"RmaxTop needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (RminTop >= RmaxTop) {
    mout<<"   ***  Error  ***  in shape CONE "<<endl;
    mout<<"RmaxTop needs to be larger than RminTop"<<endl;
    return false;            
  }
  if (HalfHeight <= 0) {
    mout<<"   ***  Error  ***  in shape CONE "<<endl;
    mout<<"HalfHeight needs to be larger than zero"<<endl;
    return false;            
  }


  m_RminBottom = RminBottom;
  m_RmaxBottom = RmaxBottom;
  m_RminTop = RminTop;
  m_RmaxTop = RmaxTop;
  m_HalfHeight = HalfHeight;

  m_Geo = new TGeoCone(m_HalfHeight, m_RminBottom, m_RmaxBottom, m_RminTop, m_RmaxTop);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeCONE::CreateShape()
{
  //

  if (m_CONE == 0) {
    m_CONE = new TCONE("Who", "knows...", "void", m_HalfHeight, 
                       m_RminBottom, m_RmaxBottom, m_RminTop, m_RmaxTop);
  }
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONE::GetRminBottom()
{
  //

  return m_RminBottom;
}

////////////////////////////////////////////////////////////////////////////////


double MDShapeCONE::GetRmaxBottom()
{
  //

  return m_RmaxBottom;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONE::GetRminTop()
{
  //

  return m_RminTop;
}

////////////////////////////////////////////////////////////////////////////////


double MDShapeCONE::GetRmaxTop()
{
  //

  return m_RmaxTop;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONE::GetHalfHeight()
{
  //

  return m_HalfHeight;
}


////////////////////////////////////////////////////////////////////////////////


TShape* MDShapeCONE::GetShape()
{
  //

  return (TShape *) m_CONE;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeCONE::GetGeant3DIM(MString ShortName)
{
  ostringstream out;

  out<<"      REAL V"<<ShortName<<"VOL"<<endl;
  out<<"      DIMENSION V"<<ShortName<<"VOL(5)"<<endl;  

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeCONE::GetGeant3DATA(MString ShortName)
{
  //

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);
  out<<"      DATA V"<<ShortName<<"VOL/"<<m_HalfHeight<<","<<m_RminBottom<<","<<m_RmaxBottom<<","<<m_RminTop<<","<<m_RmaxTop<<"/"<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeCONE::GetMGeantDATA(MString ShortName)
{
  // Write the shape parameters in MGEANT/mggpod format.
  
  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  out.precision(4);

  out<<"           "<<m_HalfHeight<<" "<<m_RminBottom<<" "<<m_RmaxBottom<<endl;
  out<<"           "<<m_RminTop<<" "<<m_RmaxTop<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeCONE::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;

  out<<"CONE "<<m_HalfHeight<<" "<<m_RminBottom<<" "<<m_RmaxBottom<<" "
     <<m_RminTop<<" "<<m_RmaxTop;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeCONE::GetGeant3ShapeName()
{
  //

  return "CONE";
}


////////////////////////////////////////////////////////////////////////////////


int MDShapeCONE::GetGeant3NumberOfParameters()
{
  //

  return 5;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeCONE::GetSize()
{
  // Return the size of a surrounding box

  return MVector(m_RmaxTop, m_RmaxTop, m_HalfHeight);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONE::GetVolume()
{
  // Return the volume of this cylinder

  return 1.0/6.0*(2*c_Pi)*(2*m_HalfHeight) * ((m_RmaxBottom*m_RmaxBottom+m_RmaxBottom*m_RmaxTop+m_RmaxTop*m_RmaxTop) - (m_RminBottom*m_RminBottom+m_RminBottom*m_RminTop+m_RminTop*m_RminTop));
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeCONE::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_RminBottom *= Factor;
  m_RmaxBottom *= Factor;
  m_RminTop *= Factor;
  m_RmaxTop *= Factor;
  m_HalfHeight *= Factor;

  delete m_Geo;
  m_Geo = new TGeoCone(m_HalfHeight, m_RminBottom, m_RmaxBottom, m_RminTop, m_RmaxTop);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeCONE::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return MVector(0.25*(m_RmaxBottom+m_RminBottom+m_RmaxTop+m_RminTop), 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////////////////


MString MDShapeCONE::ToString()
{
  // 

  ostringstream out;

  out<<"CONE ("<<m_HalfHeight<<","<<m_RminBottom<<","<<m_RmaxBottom<<","<<m_RminTop<<","<<m_RmaxTop<<")"<<endl;

  return out.str().c_str();
}


// MDShapeCONE.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
