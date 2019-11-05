/*
 * MDShapeBRIK.cxx
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
// MDShapeBRIK
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeBRIK.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGeoBBox.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDShapeBRIK)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeBRIK::MDShapeBRIK(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_Dx = 0;
  m_Dy = 0;
  m_Dz = 0;

  m_Type = "BRIK";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeBRIK::~MDShapeBRIK()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeBRIK::Set(double x, double y, double z)
{
  // Correctly initialize this shape

  if (x <= 0) {
    mout<<"   ***  Error  ***  in shape BRIK "<<endl;
    mout<<"x-dimension needs to be larger than 0"<<endl;
    return false;            
  }
  if (y <= 0) {
    mout<<"   ***  Error  ***  in shape BRIK "<<endl;
    mout<<"y-dimension needs to be larger than 0"<<endl;
    return false;            
  }
  if (z <= 0) {
    mout<<"   ***  Error  ***  in shape BRIK "<<endl;
    mout<<"z-dimension needs to be larger than 0"<<endl;
    return false;            
  }

  m_Dx = x;
  m_Dy = y;
  m_Dz = z;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeBRIK::Validate()
{
  // Correctly initialize this shape

  delete m_Geo;
  m_Geo = new TGeoBBox(m_Dx, m_Dy, m_Dz);


  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeBRIK::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    if (Tokenizer.GetNTokens() == 2+Offset + 3) {
      if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
              Tokenizer.GetTokenAtAsDouble(3+Offset), 
              Tokenizer.GetTokenAtAsDouble(4+Offset)) == false) {
        Info.Error("The shape BOX/BRIK has not been defined correctly");
        return false;
      }
    } else {
      Info.Error("You have not correctly defined your shape BOX/BRIK. It is defined by 3 parameters (x, y, z)");
      return false;
    }
  } else {
    Info.Error(MString("Unhandled descriptor in shape Box: ") + Tokenizer.GetTokenAt(1));
    return false;
  }
  
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeBRIK::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;

  out<<"BRIK "<<m_Dx<<" "<<m_Dy<<" "<<m_Dz;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeBRIK::GetSize()
{
  // Return the size of a surrounding box

  return MVector(m_Dx, m_Dy, m_Dz);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::GetSizeX()
{
  return m_Dx;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::GetSizeY()
{
  return m_Dy;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::GetSizeZ()
{
  return m_Dz;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::GetVolume()
{
  return 8*m_Dx*m_Dy*m_Dz;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeBRIK::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_Dx *= Factor;
  m_Dy *= Factor;
  m_Dz *= Factor;

  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeBRIK::ToString()
{
  // 

  ostringstream out;

  out<<"BRIK ("<<m_Dx<<", "<<m_Dy<<", "<<m_Dz<<")"<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeBRIK::IsInside(const MVector& Pos, const double Tolerance, const bool PreferOutside)
{
  // Overwrite to allow some tolerance:

  if (PreferOutside == false) {
    if (fabs(Pos.m_Z) > m_Dz + Tolerance) {
      return false;
    } 
    if (fabs(Pos.m_X) > m_Dx + Tolerance) {
      return false; 
    }
    if (fabs(Pos.m_Y) > m_Dy + Tolerance) {
      return false; 
    }
  } else {
    if (fabs(Pos.m_Z) > m_Dz - Tolerance) {
      return false;
    } 
    if (fabs(Pos.m_X) > m_Dx - Tolerance) {
      return false; 
    }
    if (fabs(Pos.m_Y) > m_Dy - Tolerance) {
      return false; 
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeBRIK::DistanceOutsideIn(const MVector& Pos, const MVector& Dir, double Tolerance)
{
  // Attention: Dir needs to be a unit vector
  
  if (Pos.m_X > +m_Dx && Dir.m_X > 0) return 0;
  if (Pos.m_X < -m_Dx && Dir.m_X < 0) return 0;
  
  if (Pos.m_Y > +m_Dy && Dir.m_Y > 0) return 0;
  if (Pos.m_Y < -m_Dy && Dir.m_Y < 0) return 0;
  
  if (Pos.m_Z > +m_Dz && Dir.m_Z > 0) return 0;
  if (Pos.m_Z < -m_Dz && Dir.m_Z < 0) return 0;
  
  return MDShape::DistanceOutsideIn(Pos, Dir, Tolerance);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeBRIK::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return MVector(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeBRIK::GetRandomPositionInside()
{
  // Return a random position inside this shape

  return MVector((2.0*gRandom->Rndm()-1.0)*m_Dx, 
                 (2.0*gRandom->Rndm()-1.0)*m_Dy, 
                 (2.0*gRandom->Rndm()-1.0)*m_Dz);
}


// MDShapeBRIK.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
