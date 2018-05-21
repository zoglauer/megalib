/*
 * MDShapeTRD1.cxx
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
// MDShapeTRD1
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeTRD1.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include <TGeoTrd1.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDShapeTRD1)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeTRD1::MDShapeTRD1(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_Ddx1 = 0;
  m_Ddx2 = 0;
  m_Dy = 0;
  m_Dz = 0;

  m_Type = "TRD1";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeTRD1::~MDShapeTRD1()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRD1::Set(double dx1, double dx2, double y, double z)
{
  // Set all parameters

  if (dx1 < 0) {
    mout<<"   ***  Error  ***  in shape TRD1 "<<endl;
    mout<<"dx1 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (dx2 < 0) {
    mout<<"   ***  Error  ***  in shape TRD1 "<<endl;
    mout<<"dx2 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (dx1 == 0 && dx2 == 0) {
    mout<<"   ***  Error  ***  in shape TRD1 "<<endl;
    mout<<"Either dx1 or dx2 needs to be larger than zero"<<endl;
    return false;            
  }
  if (y <= 0) {
    mout<<"   ***  Error  ***  in shape TRD1 "<<endl;
    mout<<"y needs to be within ]0;inf["<<endl;
    return false;            
  }
  if (z <= 0) {
    mout<<"   ***  Error  ***  in shape TRD1 "<<endl;
    mout<<"z needs to be within ]0;inf["<<endl;
    return false;            
  }

  m_Ddx1 = dx1;
  m_Ddx2 = dx2;
  m_Dy = y;
  m_Dz = z;

  m_Type = "TRD1";
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRD1::Validate()
{
  delete m_Geo;
  m_Geo = new TGeoTrd1(m_Ddx1, m_Ddx2, m_Dy, m_Dz);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRD1::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    if (Tokenizer.GetNTokens() == 2+Offset + 4) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset),
              Tokenizer.GetTokenAtAsDouble(4+Offset),
              Tokenizer.GetTokenAtAsDouble(5+Offset)) == false) {
        Info.Error("The shape TRD1 has not been defined correctly");
        return false;
      } 
    } else {
      Info.Error("You have not correctly defined your shape TRD1. It is defined by 4 parameters (lower x distance, upper x distance, y distance, z distance)");
      return false;
    }
  } else {
    Info.Error("Unhandled descriptor in shape TRD1!");
    return false;
  }
 
  return true; 
}

 
////////////////////////////////////////////////////////////////////////////////


double MDShapeTRD1::GetDx1() const
{ 
  //

  return m_Ddx1;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRD1::GetDx2() const
 { 
  //

  return m_Ddx2;
}


////////////////////////////////////////////////////////////////////////////////

double MDShapeTRD1::GetY() const
 { 
  //

  return m_Dy;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRD1::GetZ() const
 { 
  //

  return m_Dz;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTRD1::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;
  out<<"TRD1 "<<m_Ddx1<<" "<<m_Ddx2<<" "<<m_Dy<<" "<<m_Dz;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTRD1::GetSize()
{
  // Return the size of a surrounding box

  return MVector(((m_Ddx1 > m_Ddx2) ? m_Ddx1 : m_Ddx2), m_Dy, m_Dz);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRD1::GetVolume()
{
  // Return the volume of this trapezoid
  // The calculation could be optimized, but instead readability is preferred

  return 2*m_Dz * 2*m_Dy * (2*m_Ddx1+2*m_Ddx2)/2.0;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeTRD1::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_Ddx1 *= Factor;
  m_Ddx2 *= Factor;
  m_Dy *= Factor;
  m_Dz *= Factor;

  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTRD1::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return MVector(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTRD1::ToString()
{
  // 

  ostringstream out;

  out<<"TRD1 ("<<m_Ddx1<<", "<<m_Ddx2<<", "<<m_Dy<<", "<<m_Dz<<")"<<endl;

  return out.str().c_str();
}


// MDShapeTRD1.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
