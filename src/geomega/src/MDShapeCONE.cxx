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


#ifdef ___CLING___
ClassImp(MDShapeCONE)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeCONE::MDShapeCONE(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_RminBottom = 0;
  m_RmaxBottom = 0;
  m_RminTop = 0;
  m_RmaxTop = 0;
  m_HalfHeight = 0;

  m_Type = "CONE";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeCONE::~MDShapeCONE()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeCONE::Set(double HalfHeight, 
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

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeCONE::Validate()
{
  delete m_Geo;
  m_Geo = new TGeoCone(m_HalfHeight, m_RminBottom, m_RmaxBottom, m_RminTop, m_RmaxTop);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeCONE::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    if (Tokenizer.GetNTokens() == 2+Offset+5) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset),
              Tokenizer.GetTokenAtAsDouble(4+Offset),
              Tokenizer.GetTokenAtAsDouble(5+Offset),
              Tokenizer.GetTokenAtAsDouble(6+Offset)) == false) {
        Info.Error("The parameters for the shape Cone are not OK.");
        return false;
      }
    } else {
      Info.Error("You have not correctly defined your shape Cone. It is defined by 5 parameters: half height, minimum radius bottom, maximum radius bottom, minimum radius top, maximum radius top");
      return false;
    }
  } else {
    Info.Error(MString("Unhandled descriptor in shape Cone: ") + Tokenizer.GetTokenAt(1));
    return false;
  }
 
  return true; 
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

  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeCONE::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return MVector(0.25*(m_RmaxBottom+m_RminBottom+m_RmaxTop+m_RminTop), 0.0, 0.0);
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


MString MDShapeCONE::ToString()
{
  // 

  ostringstream out;

  out<<"CONE ("<<m_HalfHeight<<","<<m_RminBottom<<","<<m_RmaxBottom<<","<<m_RminTop<<","<<m_RmaxTop<<")"<<endl;

  return out.str().c_str();
}


// MDShapeCONE.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
