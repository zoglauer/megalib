/*
 * MDShapeSPHE.cxx
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
// MDShapeSPHE
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeSPHE.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGeoSphere.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDShapeSPHE)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeSPHE::MDShapeSPHE(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_Rmin = 0;
  m_Rmax = 0;
  m_Thetamin = 0;
  m_Thetamax = 0;
  m_Phimin = 0;
  m_Phimax = 0;

  m_Type = "SPHE";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeSPHE::~MDShapeSPHE()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeSPHE::Set(double Rmin, double Rmax, 
                      double Thetamin, double Thetamax, 
                      double Phimin, double Phimax) 
{
  // default constructor
  
  if (Rmin < 0) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Rmin needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (Rmax <= 0) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Rmax needs to be larger than 0"<<endl;
    return false;            
  }
  if (Rmin >= Rmax) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Rmax needs to be larger than Rmin"<<endl;
    return false;            
  }
  if (Thetamin < 0 || Thetamin >= 180) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Thetamin needs to be within [0;180["<<endl;
    return false;            
  }
  if (Thetamax <= 0 || Thetamax > 180) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Thetamax needs to be within ]0;180]"<<endl;
    return false;            
  }
  if (Thetamin >= Thetamax) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Thetamax needs to be larger than Thetamin"<<endl;
    return false;            
  }
  if (Phimin < 0 || Phimin >= 360) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Phimin needs to be within [0;360["<<endl;
    return false;            
  }
  if (Phimax <= 0 || Phimax > 360) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Phimax needs to be within ]0;360]"<<endl;
    return false;            
  }
  if (Phimin >= Phimax) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Phimax needs to be larger than Phimin"<<endl;
    return false;            
  }

  m_Rmin = Rmin;
  m_Rmax = Rmax;
  m_Thetamin = Thetamin;
  m_Thetamax = Thetamax;
  m_Phimin = Phimin;
  m_Phimax = Phimax;
  
  m_IsValidated = false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeSPHE::Set(double Rmin, double Rmax) 
{
  // default constructor
  
  if (Rmin < 0) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Rmin needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (Rmax <= 0) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Rmax needs to be larger than 0"<<endl;
    return false;            
  }
  if (Rmin >= Rmax) {
    mout<<"   ***  Error  ***  in shape SPHE "<<endl;
    mout<<"Rmax needs to be larger than Rmin"<<endl;
    return false;            
  }

  m_Rmin = Rmin;
  m_Rmax = Rmax;
  m_Thetamin = 0;
  m_Thetamax = 180;
  m_Phimin = 0;
  m_Phimax = 360;
  
  m_IsValidated = false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeSPHE::Validate()
{  
  if (m_IsValidated == false) {
    delete m_Geo;
    m_Geo = new TGeoSphere(m_Rmin, m_Rmax, m_Thetamin, m_Thetamax, m_Phimin, m_Phimax);
  
    m_IsValidated = true;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeSPHE::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    if (Tokenizer.GetNTokens() == 2+Offset+2) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset)) == false) { 
        Info.Error("The shape Sphere has not been defined correctly");
        return false;      
      }
    } else if (Tokenizer.GetNTokens() == 2+Offset+6) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset), 
              Tokenizer.GetTokenAtAsDouble(4+Offset), 
              Tokenizer.GetTokenAtAsDouble(5+Offset), 
              Tokenizer.GetTokenAtAsDouble(6+Offset), 
              Tokenizer.GetTokenAtAsDouble(7+Offset)) == false) {
        Info.Error("The shape Sphere has not been defined correctly");
        return false;
      }
    } else {
      Info.Error("You have not correctly defined your shape Sphere. It is either defined by 2 parameters (rmin & rmax) or by 6 parameters (rmin, rmax, thetamin, thetamax, phimin, phimax)");
      return false;
    }
  } else {
    Info.Error(MString("Unhandled descriptor in shape Sphere: ") + Tokenizer.GetTokenAt(1));
    return false;
  }
  
  m_IsValidated = false;
  
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeSPHE::GetRmin() const
{
  // 

  return m_Rmin;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeSPHE::GetRmax() const
{
  // 

  return m_Rmax;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeSPHE::GetThetamin() const
{
  // 

  return m_Thetamin;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeSPHE::GetThetamax() const
{
  // 

  return m_Thetamax;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeSPHE::GetPhimin() const
{
  // 

  return m_Phimin;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeSPHE::GetPhimax() const
{
  // 

  return m_Phimax;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeSPHE::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;
  out<<"SPHE "<<m_Rmin<<" "<<m_Rmax<<" "
     <<m_Thetamin<<" "<<m_Thetamax<<" "<<m_Phimin<<" "<<m_Phimax;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeSPHE::GetSize()
{
  // Return the size of a surrounding box

  return MVector(m_Rmax, m_Rmax, m_Rmax);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeSPHE::GetVolume()
{
  // Volume of the sphere:
  double Vol = 2.0/3.0*
    (m_Rmax*m_Rmax*m_Rmax*(cos(m_Thetamin*c_Rad)-cos(m_Thetamax*c_Rad)) - 
     m_Rmin*m_Rmin*m_Rmin*(cos(m_Thetamin*c_Rad)-cos(m_Thetamax*c_Rad)))*c_Pi;
  Vol *= (m_Phimax - m_Phimin)/360;

  return Vol;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeSPHE::Scale(const double Factor, const MString Axes)
{
  //! Scale the axes given in Axes by a factor Scaler

  // Don't do anything if the scaling has already been applied
  if (Factor == m_Scaler && Axes == m_ScalingAxis) return true;

  // Base class handles sanity checks and storing data
  if (MDShape::Scale(Factor, Axes) == false) return false;
  // If there was no scaling return true;
  if (IsScaled() == false) return true;

  if (!(m_ScalingAxis.Contains("X") == true && m_ScalingAxis.Contains("Y") == true && m_ScalingAxis.Contains("Z") == true)) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<endl;
    mout<<"This volume can only be scaled the same way in X, Y, and Z axis, i.e., the axis string must contain \"XYZ\"."<<endl;
    return false;
  }

  // Scale
  if (m_ScalingAxis.Contains("X") == true && m_ScalingAxis.Contains("Y") == true && m_ScalingAxis.Contains("Z") == true) {
    m_Rmin *= m_Scaler;
    m_Rmax *= m_Scaler;
  }
  
  // Validate
  m_IsValidated = false;
  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeSPHE::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  MVector Unique;
  Unique.SetMagThetaPhi(0.5*(m_Rmax+m_Rmin), 
                        0.5*(m_Thetamax+m_Thetamin)*c_Rad, 
                        0.5*(m_Phimax+m_Phimin)*c_Rad);

  return Unique; 
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeSPHE::GetRandomPositionInside()
{
  // Return a random position inside this shape

  double Phi = m_Phimin*c_Rad + gRandom->Rndm()*(m_Phimax*c_Rad-m_Phimin*c_Rad);
  double Theta = acos(cos(m_Thetamin*c_Rad) - gRandom->Rndm()*(cos(m_Thetamin*c_Rad)-cos(m_Thetamax*c_Rad)));
  double R = pow(m_Rmin*m_Rmin*m_Rmin + (m_Rmax*m_Rmax*m_Rmax-m_Rmin*m_Rmin*m_Rmin)*gRandom->Rndm(), 1.0/3.0);
  
  return MVector(R*cos(Phi)*sin(Theta), R*sin(Phi)*sin(Theta), R*cos(Theta));
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeSPHE::ToString()
{
  // 

  ostringstream out;

  out<<"SPHE ("<<m_Rmin<<","<<m_Rmax<<","<<m_Thetamin<<","<<m_Thetamax<<","<<m_Phimin<<","<<m_Phimax<<")"<<endl;

  return out.str().c_str();
}


// MDShapeSPHE.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
