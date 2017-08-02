/*
 * MDShapeTRAP.cxx
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
// MDShapeTRAP
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeTRAP.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGeoArb8.h"

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDShapeTRAP)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeTRAP::MDShapeTRAP(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_Dz = 0;
  m_Theta = 0;
  m_Phi = 0;
  m_H1 = 0;
  m_H2 = 0;
  m_Bl1 = 0;
  m_Bl2 = 0;
  m_Tl1 = 0;
  m_Tl2 = 0;
  m_Alpha1 = 0;
  m_Alpha2 = 0;

  m_Type = "TRAP";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeTRAP::~MDShapeTRAP()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRAP::Set(double Dz, double Theta, double Phi, 
                      double H1, double Bl1, double Tl1, double Alpha1, 
                      double H2, double Bl2, double Tl2, double Alpha2) 
{
  // Correctly initialize this object

  if (Dz <= 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Dz needs to be within ]0;inf["<<endl;
    return false;            
  }
  
  while (Theta < 0) Theta += 180;
  while (Theta >= 180) Theta -= 180;
  
  while (Phi < 0) Phi += 360;
  while (Phi >= 360) Phi -= 360;
  
  if (H1 < 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"H1 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (Bl1 < 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Bl1 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (Tl1 < 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Tl1 needs to be within [0;inf["<<endl;
    return false;            
  }

  while (Alpha1 < -360) Alpha1 += 360;
  while (Alpha1 >= 360) Alpha1 -= 360;
  
  if (H2 < 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"H2 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (Bl2 < 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Bl2 needs to be within [0;inf["<<endl;
    return false;            
  }
  if (Tl2 < 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Tl2 needs to be within [0;inf["<<endl;
    return false;            
  }
  
  while (Alpha2 < -360) Alpha2 += 360;
  while (Alpha2 >= 360) Alpha2 -= 360;

  if (H1 == 0 && H2 == 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"At least H1 or H2 have to be larger than zero!"<<endl;
    return false;            
  }
  if (Bl1 == 0 && Bl2 == 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"At least Bl1 or Bl2 have to be larger than zero!"<<endl;
    return false;            
  }
  if (Tl1 == 0 && Tl2 == 0) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"At least Tl1 or Tl2 have to be larger than zero!"<<endl;
    return false;            
  }

  // Verify that all points of the 6 surfaces are coplanar (lie in the same plane):

  // Auxillary rotations:
  double xTilt = tan(Theta*c_Deg) * cos(Phi*c_Deg);
  double yTilt = tan(Theta*c_Deg) * sin(Phi*c_Deg);
  double xTilt1 = tan(Alpha1*c_Deg);
  double xTilt2 = tan(Alpha2*c_Deg);

  // Define vectors
  MVector v1(-Dz*xTilt-H1*xTilt1-Bl1, -Dz*yTilt-H1, -Dz); // -x, -y , -z
  MVector v2(-Dz*xTilt-H1*xTilt1+Bl1, -Dz*yTilt-H1, -Dz); // +x, -y , -z
  MVector v3(-Dz*xTilt+H1*xTilt1-Tl1, -Dz*yTilt+H1, -Dz); // -x, +y , -z
  MVector v4(-Dz*xTilt+H1*xTilt1+Tl1, -Dz*yTilt+H1, -Dz); // +x, +y , -z
  MVector v5(+Dz*xTilt-H2*xTilt2-Bl2, +Dz*yTilt-H2, +Dz); // -x, -y , +z
  MVector v6(+Dz*xTilt-H2*xTilt2+Bl2, +Dz*yTilt-H2, +Dz); // +x, -y , +z
  MVector v7(+Dz*xTilt+H2*xTilt2-Tl2, +Dz*yTilt+H2, +Dz); // -x, +y , +z
  MVector v8(+Dz*xTilt+H2*xTilt2+Tl2, +Dz*yTilt+H2, +Dz); // +x, +y , +z

  // Check coplanarity:
  if (v1.Coplanar(v2, v3, v4, 1E-5) == false) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Some of the surface vectors are not coplanar (v1, v2, v3, v4 --> the 4 edge points in -z direction do not form a plane)"<<endl;
    return false;            
  }
  if (v5.Coplanar(v6, v7, v8, 1E-5) == false) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Some of the surface vectors are not coplanar (v5, v6, v7, v8 --> the 4 edge points in +z direction do not form a plane)"<<endl;
    return false;            
  }
  if (v2.Coplanar(v4, v6, v8, 1E-5) == false) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Some of the surface vectors are not coplanar (v2, v4, v6, v8 --> the 4 edge points in +x direction do not form a plane)"<<endl;
    return false;            
  }
  if (v1.Coplanar(v3, v5, v7, 1E-5) == false) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Some of the surface vectors are not coplanar (v1, v3, v5, v7 --> the 4 edge points in -x direction do not form a plane)"<<endl;
    return false;            
  }
  if (v1.Coplanar(v2, v5, v6, 1E-5) == false) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Some of the surface vectors are not coplanar (v1, v2, v5, v6 --> the 4 edge points in -y direction do not form a plane)"<<endl;
    return false;            
  }
  if (v3.Coplanar(v4, v7, v8, 1E-5) == false) {
    mout<<"   ***  Error  ***  in shape TRAP "<<endl;
    mout<<"Some of the surface vectors are not coplanar (v3, v4, v7, v8 --> the 4 edge points in +y direction do not form a plane)"<<endl;
    return false;            
  }


  m_Dz = Dz;
  m_Theta = Theta;
  m_Phi = Phi;
  m_H1 = H1;
  m_H2 = H2;
  m_Bl1 = Bl1;
  m_Bl2 = Bl2;
  m_Tl1 = Tl1;
  m_Tl2 = Tl2;
  m_Alpha1 = Alpha1;
  m_Alpha2 = Alpha2;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRAP::Validate()
{
  delete m_Geo;
  m_Geo = new TGeoTrap(m_Dz, m_Theta, m_Phi, 
                       m_H1, m_Bl1, m_Tl1, m_Alpha1, 
                       m_H2, m_Bl2, m_Tl2, m_Alpha2);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeTRAP::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    if (Tokenizer.GetNTokens() == 2+Offset + 11) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset),
              Tokenizer.GetTokenAtAsDouble(4+Offset),
              Tokenizer.GetTokenAtAsDouble(5+Offset),
              Tokenizer.GetTokenAtAsDouble(6+Offset),
              Tokenizer.GetTokenAtAsDouble(7+Offset),
              Tokenizer.GetTokenAtAsDouble(8+Offset),
              Tokenizer.GetTokenAtAsDouble(9+Offset),
              Tokenizer.GetTokenAtAsDouble(10+Offset),
              Tokenizer.GetTokenAtAsDouble(11+Offset),
              Tokenizer.GetTokenAtAsDouble(12+Offset)) == false) {
        Info.Error("The shape TRAP has not been defined correctly");
        return false;
      }
    } else {
      Info.Error("You have not correctly defined your shape TRAP. It is defined by 11 parameters (Dz, Theta, Phi, H1, Bl1, Tl1, Alpha1, H2, Bl2, Tl2, Alpha2)");
      return false;
    }
  } else {
    Info.Error("Unhandled descriptor in shape TRAP!");
    return false;
  }
 
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetDz() const
{
  //

  return m_Dz;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetTheta() const
{
  //

  return m_Theta;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetPhi() const
{
  //

  return m_Phi;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetH1() const
{
  //

  return m_H1;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetBl1() const
{
  //

  return m_Bl1;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetTl1() const
{
  //

  return m_Tl1;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetAlpha1() const
{
  //

  return m_Alpha1;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetH2() const
{
  //

  return m_H2;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetBl2() const
{
  //

  return m_Bl2;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetTl2() const
{
  //

  return m_Tl2;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetAlpha2() const
{
  //

  return m_Alpha2;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTRAP::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;
  out<<"TRAP "<<m_Dz    <<" "<<m_Theta <<" "<<m_Phi<<" "
     <<m_H1<<" "<<m_Bl1<<" "<<m_Tl1<<" "<<m_Alpha1<<" "
     <<m_H2<<" "<<m_Bl2<<" "<<m_Tl2<<" "<<m_Alpha2;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTRAP::GetSize()
{
  // Return the size of a surrounding box

  Error("MVector MDShapeTRAP::GetSize()",
        "Size not supported");

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeTRAP::GetVolume()
{
  // Return the volume of this trapezoid
  // Angles should not influence the volume
  // The calculation is according to the Simpson formula 
  // The calculation could be optimized, but instead readability is preferred

  double AreaTrapTop = (2*m_Bl1+2*m_Tl1)/2 * 2*m_H1;
  double AreaTrapBottom = (2*m_Bl2+2*m_Tl2)/2 * 2*m_H2;
  double AreaTrapCenter = (2*(m_Bl1 + m_Bl2)/2 + 2*(m_Tl1 + m_Tl2)/2)/2 * 2*(m_H1+m_H2)/2;

  return (2*m_Dz)/6 * (AreaTrapTop + AreaTrapBottom + 4 * AreaTrapCenter);
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeTRAP::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_Dz *= Factor;
  m_H1 *= Factor;
  m_H2 *= Factor;
  m_Bl1 *= Factor;
  m_Bl2 *= Factor;
  m_Tl1 *= Factor;
  m_Tl2 *= Factor;

  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeTRAP::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  mimp<<"Is this really correct??"<<endl;

  return MVector(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeTRAP::ToString()
{
  // 

  ostringstream out;

  out<<"TRAP (";
  out<<m_Dz<<", "<<m_Theta<<", "<<m_Phi<<", "<<m_H1<<", "<<m_Bl1<<", "<<m_Tl1<<", "<<m_Alpha1<<", "<<m_H2<<", "<<m_Bl2<<", "<<m_Tl2<<", "<<m_Alpha2<<")"<<endl;

  return out.str().c_str();
}


// MDShapeTRAP.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
