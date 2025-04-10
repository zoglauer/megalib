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


#ifdef ___CLING___
ClassImp(MDShapeTUBS)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeTUBS::MDShapeTUBS(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_Rmin = 0;
  m_Rmax = 0;
  m_HalfHeight = 0;
  m_Phi1 = 0;
  m_Phi2 = 0;

  m_Type = "TUBS";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeTUBS::~MDShapeTUBS()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTUBS::Set(double Rmin, double Rmax, double HalfHeight, 
                      double Phi1, double Phi2) 
{
  // Set all parameters

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
  
  m_IsValidated = false;
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MDShapeTUBS::Validate()
{
  if (m_IsValidated == false) {
    delete m_Geo;
    if (m_Phi1 == 0 && m_Phi2 == 360) {
      m_Geo = new TGeoTube(m_Rmin, m_Rmax, m_HalfHeight);
    } else {
      m_Geo = new TGeoTubeSeg(m_Rmin, m_Rmax, m_HalfHeight, m_Phi1, m_Phi2);
    }
  
    m_IsValidated = true;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTUBS::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    if (Tokenizer.GetNTokens() == 2+Offset + 3) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset),
              Tokenizer.GetTokenAtAsDouble(4+Offset),
              0,
              360) == false) {
        Info.Error("The shape TUBS has not been defined correctly");
        return false;
      }
    } else if (Tokenizer.GetNTokens() == 2+Offset + 5) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset),
              Tokenizer.GetTokenAtAsDouble(4+Offset),
              Tokenizer.GetTokenAtAsDouble(5+Offset),
              Tokenizer.GetTokenAtAsDouble(6+Offset)) == false) {
        Info.Error("The shape TUBS has not been defined correctly");
        return false;
      }
    } else {
      Info.Error("You have not correctly defined your shape TUBS. It is defined by 5 parameters (rmin, rmax, half height, phi1, phi2)");
      return false;
    }
  } else {
    Info.Error(MString("Unhandled descriptor in shape TUBS: ") + Tokenizer.GetTokenAt(1));
    return false;
  }
  
  m_IsValidated = false;
  
  return true; 
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


MString MDShapeTUBS::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;
  out<<"TUBS "<<m_Rmin<<" "<<m_Rmax<<" "<<m_HalfHeight<<" "<<m_Phi1<<" "<<m_Phi2;

  return out.str().c_str();
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


bool MDShapeTUBS::Scale(const double Factor, const MString Axes)
{
  //! Scale the axes given in Axes by a factor Scaler

  // Don't do anything if the scaling has already been applied
  if (Factor == m_Scaler && Axes == m_ScalingAxis) return true;

  // Base class handles sanity checks and storing data
  if (MDShape::Scale(Factor, Axes) == false) return false;
  // If there was no scaling return true;
  if (IsScaled() == false) return true;

  if ((m_ScalingAxis.Contains("X") == true && m_ScalingAxis.Contains("Y") == false) || (m_ScalingAxis.Contains("X") == false && m_ScalingAxis.Contains("Y") == true)) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<endl;
    mout<<"This volume can only be scaled the same way in X as in Y axis, i.e., the axis string must contain \"XY\"."<<endl;
    return false;
  }


  // Scale
  if (m_ScalingAxis.Contains("X") == true && m_ScalingAxis.Contains("Y") == true) {
    m_Rmin *= m_Scaler;
    m_Rmax *= m_Scaler;
  }
  if (m_ScalingAxis.Contains("Z") == true) {
    m_HalfHeight *= m_Scaler;
  }


  // Validate
  m_IsValidated = false;
  return Validate();
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
