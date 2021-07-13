/*
 * MDShapeSubtraction.cxx
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
// MDShapeSubtraction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeSubtraction.h"

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


#ifdef ___CLING___
ClassImp(MDShapeSubtraction)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeSubtraction::MDShapeSubtraction(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_SubShapes.push_back(0);
  m_SubShapes.push_back(0);
  m_Orientation = 0;

  m_Type = "Subtraction";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeSubtraction::~MDShapeSubtraction()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeSubtraction::Set(MDShape* Minuend, MDShape* Subtrahend, MDOrientation* Orientation) 
{
  // Set the key poarameters

  // We clone the shapes here in order to be able to scale!
  m_SubShapes[0] = Minuend;
  m_SubShapes[1] = Subtrahend;
  m_Orientation = Orientation;
  
  m_IsValidated = false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeSubtraction::Validate()
{  
  if (m_IsValidated == false) {
    
    if (m_SubShapes[0] == 0) {
      mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
      mout<<"No minuend given"<<endl;
      return false;            
    }
    
    if (m_SubShapes[1] == 0) {
      mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
      mout<<"No subtrahend given"<<endl;
      return false;            
    }  
    
    if (m_SubShapes[0] == m_SubShapes[1]) {
      mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
      mout<<"Minuend and subtrahend are identical"<<endl;
      return false;            
    }
    
    if (this == m_SubShapes[0]) {
      mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
      mout<<"Minuend cannot be identical with this subtraction shape"<<endl;
      return false;            
    }
    
    if (this == m_SubShapes[1]) {
      mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
      mout<<"Subtrahend cannot be identical with this subtraction shape"<<endl;
      return false;            
    }
    
    if (m_SubShapes[0]->Validate() == false) return false;  
    if (m_SubShapes[1]->Validate() == false) return false;
    
    if (m_Orientation == 0) {
      mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
      mout<<"No orientation given"<<endl;
      return false;            
    }
    
    delete m_Geo;
    cout<<"Creating Node of: "<<m_SubShapes[0]->GetName()<<" & "<<m_SubShapes[1]->GetName()<<endl;
    TGeoSubtraction* Node = new TGeoSubtraction(m_SubShapes[0]->GetRootShape(), m_SubShapes[1]->GetRootShape(), 0, m_Orientation->GetRootMatrix());
    m_Geo = new TGeoCompositeShape(m_Name, Node);
    
    // Determine an almost unique position inside this shape:
    unsigned int Seed = gRandom->GetSeed();
    gRandom->SetSeed(12345678);
    m_AlmostUniquePosition = GetRandomPositionInside();
    gRandom->SetSeed(Seed);
    
    m_IsValidated = true;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeSubtraction::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  merr<<"Error: The Shape Subtraction cannot parse itself..."<<endl;
  return false; 
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeSubtraction::GetVolume()
{
  // Return the volume of this cylinder

  return m_Geo->Capacity();
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeSubtraction::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_Scaler = Factor;
  
  //if (m_Minuend->GetScaler() != Factor) m_Minuend->Scale(Factor);
  //if (m_Subtrahend->GetScaler() != Factor) m_Subtrahend->Scale(Factor);
  //if (m_Orientation->GetScaler() != Factor) m_Orientation->Scale(Factor);
  
  m_IsValidated = false;
  
  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeSubtraction::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return m_AlmostUniquePosition;
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeSubtraction::ToString()
{
  // Return a string describung this class

  ostringstream out;

  out<<"Subtraction!"<<endl;

  return out.str().c_str();
}


// MDShapeSubtraction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
