/*
 * MDShapeCONS.cxx
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
#include "MDShapeCONS.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include <TGeoCone.h>

// MEGALib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDShapeCONS)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeCONS::MDShapeCONS(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_RminBottom = 0;
  m_RmaxBottom = 0;
  m_RminTop = 0;
  m_RmaxTop = 0;
  m_HalfHeight = 0;
  m_PhiMin = 0;
  m_PhiMax = 0;

  m_Type = "CONS";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeCONS::~MDShapeCONS()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeCONS::Set(double HalfHeight, 
                      double RminBottom, double RmaxBottom, 
                      double RminTop, double RmaxTop, 
                      double PhiMin, double PhiMax) 
{
  // default constructor

  if (RminBottom < 0) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"RminBottom needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (RmaxBottom <= 0) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"RmaxBottom needs to be larger then 0"<<endl;
    return false;            
  }
  if (RminBottom >= RmaxBottom) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"RmaxBottom needs to be larger than RminBottom"<<endl;
    return false;            
  }
  if (RminTop < 0) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"RminTop needs to be larger or equal 0: "<<endl;
    return false;            
  }
  if (RmaxTop < 0) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"RmaxTop needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (RminTop >= RmaxTop) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"RmaxTop needs to be larger than RminTop"<<endl;
    return false;            
  }
  if (HalfHeight <= 0) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"HalfHeight needs to be larger than zero"<<endl;
    return false;            
  }
  if (PhiMin < 0 || PhiMin >= 360) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"PhiMin needs to be within [0;360["<<endl;
    return false;            
  }
  if (PhiMax-PhiMin <= 0 || PhiMax-PhiMin > 360) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"PhiMax-PhiMin needs to be within ]0;360]"<<endl;
    return false;            
  }
  if (PhiMin >= PhiMax) {
    mout<<"   ***  Error  ***  in shape CONS "<<endl;
    mout<<"PhiMax needs to be larger than PhiMin"<<endl;
    return false;            
  }


  m_RminBottom = RminBottom;
  m_RmaxBottom = RmaxBottom;
  m_RminTop = RminTop;
  m_RmaxTop = RmaxTop;
  m_HalfHeight = HalfHeight;
  m_PhiMin = PhiMin;
  m_PhiMax = PhiMax;
  
  m_IsValidated = false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeCONS::Validate()
{
  if (m_IsValidated == false) {
    delete m_Geo;
    m_Geo = new TGeoConeSeg(m_HalfHeight, m_RminBottom, m_RmaxBottom, m_RminTop, m_RmaxTop, m_PhiMin, m_PhiMax);
    
    m_IsValidated = true;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeCONS::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    if (Tokenizer.GetNTokens() == 2+Offset+7) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset),
              Tokenizer.GetTokenAtAsDouble(4+Offset),
              Tokenizer.GetTokenAtAsDouble(5+Offset),
              Tokenizer.GetTokenAtAsDouble(6+Offset),
              Tokenizer.GetTokenAtAsDouble(7+Offset),
              Tokenizer.GetTokenAtAsDouble(8+Offset)) == false) {
        Info.Error("The parameters for the shape CONS are not OK.");
        return false;
      }
    } else {
      Info.Error("You have not correctly defined your shape Cons. It is defined by 7 parameters: half height, minimum radius bottom, maximum radius bottom, minimum radius top, maximum radius top, minimum phi angle, maximum phi angle");
      return false;
    }
  } else {
    Info.Error(MString("Unhandled descriptor in shape Cons: ") + Tokenizer.GetTokenAt(1));
    return false;
  }
 
  m_IsValidated = false;
 
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONS::GetRminBottom()
{
  //

  return m_RminBottom;
}

////////////////////////////////////////////////////////////////////////////////


double MDShapeCONS::GetRmaxBottom()
{
  //

  return m_RmaxBottom;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONS::GetRminTop()
{
  //

  return m_RminTop;
}

////////////////////////////////////////////////////////////////////////////////


double MDShapeCONS::GetRmaxTop()
{
  //

  return m_RmaxTop;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONS::GetHalfHeight()
{
  //

  return m_HalfHeight;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONS::GetPhiMin()
{
  //

  return m_PhiMin;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONS::GetPhiMax()
{
  //

  return m_PhiMax;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeCONS::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;

  out<<"CONS "<<m_HalfHeight<<" "<<m_RminBottom<<" "<<m_RmaxBottom<<" "
     <<m_RminTop<<" "<<m_RmaxTop<<" "<<m_PhiMin<<" "<<m_PhiMax;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeCONS::GetSize()
{
  // Return the size of a surrounding box

  return MVector(m_RmaxTop, m_RmaxTop, m_HalfHeight);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeCONS::GetVolume()
{
  // Return the volume of this cone segment

  return 1.0/6.0*(2*c_Pi)*(2*m_HalfHeight) * ((m_RmaxBottom*m_RmaxBottom+m_RmaxBottom*m_RmaxTop+m_RmaxTop*m_RmaxTop) - (m_RminBottom*m_RminBottom+m_RminBottom*m_RminTop+m_RminTop*m_RminTop))*(m_PhiMax-m_PhiMin)/360;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeCONS::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_RminBottom *= Factor;
  m_RmaxBottom *= Factor;
  m_RminTop *= Factor;
  m_RmaxTop *= Factor;
  m_HalfHeight *= Factor;
  
  m_IsValidated = false;
  
  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeCONS::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  double R = 0.25*(m_RmaxBottom+m_RminBottom+m_RmaxTop+m_RminTop); 
  double Angle = 0.5*(m_PhiMin+m_PhiMax)*c_Rad; 

  return MVector(R*cos(Angle), R*sin(Angle), 0.0);
}

////////////////////////////////////////////////////////////////////////////////


MString MDShapeCONS::ToString()
{
  // 

  ostringstream out;

  out<<"CONS ("<<m_HalfHeight<<","<<m_RminBottom<<","<<m_RmaxBottom<<","<<m_RminTop<<","<<m_RmaxTop<<","<<m_PhiMin<<","<<m_PhiMax<<")"<<endl;

  return out.str().c_str();
}


// MDShapeCONS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
