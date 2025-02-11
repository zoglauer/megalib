/*
 * MDShapeTRD2.cxx
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
// MDShapeTRD2
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeTRD2.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include <TGeoTrd2.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDShapeTRD2)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeTRD2::MDShapeTRD2(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_Dx1 = 0;
  m_Dx2 = 0;
  m_Dy1 = 0;
  m_Dy2 = 0;
  m_Z = 0;

  m_Type = "TRD2";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeTRD2::~MDShapeTRD2()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRD2::Set(double dx1, double dx2, double dy1, double dy2, double z)
{
  // Set all parameters

  if (dx1 < 0) {
    mout<<"   ***  Error  ***  in shape TRD2 "<<endl;
    mout<<"dx1 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (dx2 < 0) {
    mout<<"   ***  Error  ***  in shape TRD2 "<<endl;
    mout<<"dx2 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (dx1 == 0 && dx2 == 0) {
    mout<<"   ***  Error  ***  in shape TRD2 "<<endl;
    mout<<"Either dx1 or dx2 needs to be larger than zero"<<endl;
    return false;            
  }
  if (dy1 < 0) {
    mout<<"   ***  Error  ***  in shape TRD2 "<<endl;
    mout<<"dy1 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (dy2 < 0) {
    mout<<"   ***  Error  ***  in shape TRD2 "<<endl;
    mout<<"dy2 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (dy1 == 0 && dy2 == 0) {
    mout<<"   ***  Error  ***  in shape TRD2 "<<endl;
    mout<<"Either dy1 or dy2 needs to be larger than zero"<<endl;
    return false;            
  }
  if (z <= 0) {
    mout<<"   ***  Error  ***  in shape TRD2 "<<endl;
    mout<<"z needs to be within ]0;inf["<<endl;
    return false;            
  }

  m_Dx1 = dx1;
  m_Dx2 = dx2;
  m_Dy1 = dy1;
  m_Dy2 = dy2;
  m_Z = z;
  
  m_IsValidated = false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRD2::Validate()
{
  if (m_IsValidated == false) {
    delete m_Geo;
    m_Geo = new TGeoTrd2(m_Dx1, m_Dx2, m_Dy1, m_Dy2, m_Z);
  
    m_IsValidated = true;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRD2::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    if (Tokenizer.GetNTokens() == 2+Offset + 5) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset),
              Tokenizer.GetTokenAtAsDouble(4+Offset),
              Tokenizer.GetTokenAtAsDouble(5+Offset),
              Tokenizer.GetTokenAtAsDouble(6+Offset)) == false) {
        Info.Error("The shape TRD2 has not been defined correctly");
        return false;
      }
    } else {
      Info.Error("You have not correctly defined your shape TRD2. It is defined by 5 parameters (lower x distance, upper x distance, lower y distance, upper y distance, z distance)");
      return false;
    }
  } else {
    Info.Error(MString("Unhandled descriptor in shape TRD2: ") + Tokenizer.GetTokenAt(1));
    return false;
  }
  
  m_IsValidated = false;
  
  return true; 
}

 
////////////////////////////////////////////////////////////////////////////////


double MDShapeTRD2::GetDx1() const
{ 
  //

  return m_Dx1;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRD2::GetDx2() const
 { 
  //

  return m_Dx2;
}


////////////////////////////////////////////////////////////////////////////////

double MDShapeTRD2::GetDy1() const
 { 
  //

  return m_Dy1;
}


////////////////////////////////////////////////////////////////////////////////

double MDShapeTRD2::GetDy2() const
 { 
  //

  return m_Dy2;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRD2::GetZ() const
 { 
  //

  return m_Z;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTRD2::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;
  out<<"TRD2 "<<m_Dx1<<" "<<m_Dx2<<" "<<m_Dy1<<" "<<m_Dy2<<" "<<m_Z;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTRD2::GetSize()
{
  // Return the size of a surrounding box

  return MVector(((m_Dx1 > m_Dx2) ? m_Dx1 : m_Dx2), ((m_Dy1 > m_Dy2) ? m_Dy1 : m_Dy2), m_Z);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRD2::GetVolume()
{
  // Return the volume of this trapezoid
  // The calculation could be optimized, but instead readability is preferred

  return 2*m_Z * 2*(m_Dy1+m_Dy2)/2.0 * 2*(m_Dx1+m_Dx2)/2.0;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRD2::Scale(const double Factor, const MString Axes)
{
  //! Scale the axes given in Axes by a factor Scaler

  // Don't do anything if the scaling has already been applied
  if (Factor == m_Scaler && Axes == m_ScalingAxis) return true;

  // Base class handles sanity checks and storing data
  if (MDShape::Scale(Factor, Axes) == false) return false;
  // If there was no scaling return true;
  if (IsScaled() == false) return true;


  // Now scale
  if (m_ScalingAxis.Contains("X") == true) {
    m_Dx1 *= m_Scaler;
    m_Dx2 *= m_Scaler;
  }
  if (m_ScalingAxis.Contains("Y") == true) {
    m_Dy1 *= m_Scaler;
    m_Dy2 *= m_Scaler;
  }
  if (m_ScalingAxis.Contains("Z") == true) {
    m_Z *= m_Scaler;
  }


  // Validate
  m_IsValidated = false;
  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTRD2::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return MVector(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTRD2::ToString()
{
  // 

  ostringstream out;

  out<<"TRD2 ("<<m_Dx1<<", "<<m_Dx2<<", "<<m_Dy1<<", "<<m_Dy2<<", "<<m_Z<<")"<<endl;

  return out.str().c_str();
}


// MDShapeTRD2.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
