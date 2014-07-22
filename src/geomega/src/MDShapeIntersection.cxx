/*
 * MDShapeIntersection.cxx
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
// MDShapeIntersection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeIntersection.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGeoCompositeShape.h"
#include "TGeoBoolNode.h"
#include "TRandom.h"

// MEGALib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDShapeIntersection)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeIntersection::MDShapeIntersection(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_SubShapes.push_back(0);
  m_SubShapes.push_back(0);
  m_Orientation = 0;

  m_Type = "Intersection";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeIntersection::~MDShapeIntersection()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeIntersection::Set(MDShape* ShapeA, MDShape* ShapeB, MDOrientation* Orientation) 
{
  // Set the key poarameters

  // We clone the shapes here in order to be able to scale!
  m_SubShapes[0] = ShapeA;
  m_SubShapes[1] = ShapeB;
  m_Orientation = Orientation;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeIntersection::Validate()
{
  if (m_SubShapes[0] == 0) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type intersection"<<endl;
    mout<<"No first shape given"<<endl;
    return false;            
  }
  if (m_SubShapes[0]->Validate() == false) return false;
  
  if (m_SubShapes[1] == 0) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type intersection"<<endl;
    mout<<"No second shape given"<<endl;
    return false;            
  }  
  if (m_SubShapes[1]->Validate() == false) return false;
  
  if (m_Orientation == 0) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type intersection"<<endl;
    mout<<"No orientation given"<<endl;
    return false;            
  }
  
  delete m_Geo;
  TGeoIntersection* Node = new TGeoIntersection(m_SubShapes[0]->GetRootShape(), m_SubShapes[1]->GetRootShape(), 0, m_Orientation->GetRootMatrix());
  m_Geo = new TGeoCompositeShape(m_Name, Node);

  // Determine an almost unique position inside this shape:
  unsigned int Seed = gRandom->GetSeed();
  gRandom->SetSeed(12345678);
  m_AlmostUniquePosition = GetRandomPositionInside();
  gRandom->SetSeed(Seed);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeIntersection::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  merr<<"Error: The Shape Intersection cannot parse itself..."<<endl;
  return false; 
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeIntersection::GetVolume()
{
  // Return the volume of this cylinder

  return m_Geo->Capacity();
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeIntersection::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_Scaler = Factor;
  
  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeIntersection::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return m_AlmostUniquePosition;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeIntersection::ToString()
{
  // Return a string describung this class

  ostringstream out;

  out<<"Intersection!"<<endl;

  return out.str().c_str();
}


// MDShapeIntersection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
