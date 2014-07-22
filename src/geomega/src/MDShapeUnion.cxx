/*
 * MDShapeUnion.cxx
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
// MDShapeUnion
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapeUnion.h"

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
ClassImp(MDShapeUnion)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapeUnion::MDShapeUnion(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_SubShapes.push_back(0);
  m_SubShapes.push_back(0);
  m_Orientation = 0;

  m_Type = "Union";
}


////////////////////////////////////////////////////////////////////////////////


MDShapeUnion::~MDShapeUnion()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeUnion::Set(MDShape* Augend, MDShape* Addend, MDOrientation* Orientation) 
{
  // Set the key poarameters

  // We clone the shapes here in order to be able to scale!
  m_SubShapes[0] = Augend;
  m_SubShapes[1] = Addend;
  m_Orientation = Orientation;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeUnion::Validate()
{
  if (m_SubShapes[0] == 0) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
    mout<<"No augend given"<<endl;
    return false;            
  }
  if (m_SubShapes[0]->Validate() == false) return false;
  
  if (m_SubShapes[1] == 0) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
    mout<<"No addend given"<<endl;
    return false;            
  }  
  if (m_SubShapes[1]->Validate() == false) return false;
  
  if (m_Orientation == 0) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<" of type subtraction"<<endl;
    mout<<"No orientation given"<<endl;
    return false;            
  }
  
  delete m_Geo;
  TGeoUnion* Node = new TGeoUnion(m_SubShapes[0]->GetRootShape(), m_SubShapes[1]->GetRootShape(), 0, m_Orientation->GetRootMatrix());
  m_Geo = new TGeoCompositeShape(m_Name, Node);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapeUnion::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  merr<<"Error: The Shape Union cannot parse itself..."<<endl;
  return false; 
}


////////////////////////////////////////////////////////////////////////////////


double MDShapeUnion::GetVolume()
{
  // Return the volume of this cylinder

  return m_Geo->Capacity();
}


////////////////////////////////////////////////////////////////////////////////


void MDShapeUnion::Scale(const double Factor)
{
  // Scale this shape by Factor

  m_Scaler = Factor;
  
  //if (m_Augend->GetScaler() != Factor) m_Augend->Scale(Factor);
  //if (m_Addend->GetScaler() != Factor) m_Addend->Scale(Factor);
  //if (m_Orientation->GetScaler() != Factor) m_Orientation->Scale(Factor);

  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapeUnion::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  return m_SubShapes[0]->GetUniquePosition();
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapeUnion::ToString()
{
  // Return a string describung this class

  ostringstream out;

  out<<"Union!"<<endl;

  return out.str().c_str();
}


// MDShapeUnion.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
