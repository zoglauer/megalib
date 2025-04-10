/*
 * MDShape.cxx
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
// MDShape
//
// This class is some kind of interface to the ROOT-class shape without
// creating a ROOT object at the very beginning (in order to save memory
// when the user does not want to view the geometry, but only wants to use it
// for event/image reconstruction)
//
// This is a virtual base class. The real classes are MDShapeBRIK, MDShapeTUBS
// etc.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShape.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TGeoBBox.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDShape)
#endif


////////////////////////////////////////////////////////////////////////////////


const double MDShape::c_NoIntersection = -947957.243756;


////////////////////////////////////////////////////////////////////////////////


MDShape::MDShape(const MString& Name)
{
  m_Name = Name;
  m_Geo = nullptr;
  m_IsValidated = false;

  m_Scaler = 1.0;
  m_ScalingAxis = "";
}


////////////////////////////////////////////////////////////////////////////////


MDShape::~MDShape()
{
  // default destructor

  // delete m_Geo; // Root will delete this!
}


////////////////////////////////////////////////////////////////////////////////


void MDShape::SetColor(int Color)
{
  // Set the color of this shape

  m_Color = Color;
}


////////////////////////////////////////////////////////////////////////////////


int MDShape::GetColor()
{
  // Get the color of this shape

  return m_Color;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShape::GetType()
{
  // Return the type of this shape

  return m_Type;
}


////////////////////////////////////////////////////////////////////////////////


vector<MVector> MDShape::CreateSurfacePattern(const unsigned int Detail) const
{
  // Create a vector of points which are on the surface of this volume
  // and can be used to check for intersections

  vector<MVector> Points;

  return Points;
}


////////////////////////////////////////////////////////////////////////////////


double MDShape::DistanceInsideOut(const MVector& Pos, const MVector& Dir, double Tolerance)
{
  // Attention: Dir needs to be a unit vector

  massert(m_Geo != 0);

  double point[3];
  point[0] = Pos.m_X;
  point[1] = Pos.m_Y;
  point[2] = Pos.m_Z;

  double dir[3];
  dir[0] = Dir.m_X;
  dir[1] = Dir.m_Y;
  dir[2] = Dir.m_Z;
 
  return m_Geo->DistFromInside(point, dir);
}


////////////////////////////////////////////////////////////////////////////////


double MDShape::DistanceOutsideIn(const MVector& Pos, const MVector& Dir, double Tolerance)
{
  // Attention: Dir needs to be a unit vector

  massert(m_Geo != 0);

  double point[3];
  point[0] = Pos.m_X;
  point[1] = Pos.m_Y;
  point[2] = Pos.m_Z;

  double dir[3];
  dir[0] = Dir.m_X;
  dir[1] = Dir.m_Y;
  dir[2] = Dir.m_Z;

  return m_Geo->DistFromOutside(point, dir);
}


////////////////////////////////////////////////////////////////////////////////


bool MDShape::IsInside(const MVector& Pos, const double Tolerance, const bool PreferOutside)
{
  massert(m_Geo != 0);
  
  Double_t point[3];
  point[0] = Pos.m_X;
  point[1] = Pos.m_Y;
  point[2] = Pos.m_Z;

  bool Inside = m_Geo->Contains(point);

  if (Tolerance > 0.0) {
    if (Inside == true && PreferOutside == true) {
      if (m_Geo->Safety(point, Inside) < Tolerance) {
        Inside = false;
      }
    }
    if (Inside == false && PreferOutside == false) {
      if (m_Geo->Safety(point, Inside) < Tolerance) {
        Inside = true;
      }
    }
  }

  return Inside;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShape::GetRandomPositionInside()
{
  // Return a random position inside this shape
  // The is the default function which works for all shapes
  // Several frequently used shapes have their own optimized version
  
  // Create a random number within the bounding box:
  TGeoBBox* BBox = (TGeoBBox*) m_Geo; // All geometries are derived from TGeoBBox!
  
  MVector R;

  unsigned long Counter = 0;
  do {
    R.SetX((2.0*gRandom->Rndm()-1.0)*BBox->GetDX() + BBox->GetOrigin()[0]);
    R.SetY((2.0*gRandom->Rndm()-1.0)*BBox->GetDY() + BBox->GetOrigin()[1]);
    R.SetZ((2.0*gRandom->Rndm()-1.0)*BBox->GetDZ() + BBox->GetOrigin()[2]);

    Counter++;

    if (Counter > 10000000) {
      mout<<"   ***  FATAL ERROR  ***  in shape "<<m_Type<<endl;
      mout<<"GetRandomPositionInside() required more than "<<Counter<<" trials."<<endl;
      mout<<"There is likely something wrong with your shape, especially when it is a boolean one. You have: "<<m_Type<<endl;
      abort();
      return g_VectorNotDefined;
    }
  } while (IsInside(R) == false);

  if (Counter > 10000) {
    mout<<"   ***  Warning  ***  in shape "<<m_Type<<endl;
    mout<<"GetRandomPositionInside() required more than "<<Counter<<" trials."<<endl;
    mout<<"You might want to ask the developers to write an optimized version of GetRandomPositionInside() for type "<<m_Type<<endl;
  }

  return R;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShape::Scale(const double Scale, const MString Axes)
{
  // Base class: Call this one first in derived classed
  // Performs sanity checks on scaling, and saves data

  if (IsScaled() == true) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<endl;
    mout<<"The shape is already scaled."<<endl;
    return false;
  }

  for (unsigned int c = 0; c < Axes.Length(); ++c) {
    char C = Axes[c];
    if (C != 'X' && C != 'Y' && C != 'Z') {
      mout<<"   ***  Error  ***  in shape "<<m_Name<<endl;
      mout<<"The scaling axis can only contain X, Y, or Z, and not \""<<C<<"\"."<<endl;
      return false;
    }
  }

  if (Scale == 1.0) return true;

  m_Scaler = Scale;
  m_ScalingAxis = Axes;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! True if we are scaled
bool MDShape::IsScaled() const
{
  if (m_ScalingAxis != "") {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShape::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  merr<<"Error: The shape base class for shape "<<m_Type<<" cannot do any parsing..."<<endl; 
  return false;
}
 

////////////////////////////////////////////////////////////////////////////////


MDShape* MDShape::GetSubShape(unsigned int i)
{
  // Return a specific subshape - if there are no subshapes or if i is out of bound return 0

  if (i < m_SubShapes.size()) {
    return m_SubShapes[i];
  }
  
  return 0;
}
  

// MDShape.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
