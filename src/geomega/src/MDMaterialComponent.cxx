/*
 * MDMaterialComponent.cxx
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
// MDMaterialComponent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDMaterialComponent.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDMaterialComponent)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDMaterialComponent::c_ByAtoms = 1;
const int MDMaterialComponent::c_ByMass = 2;


////////////////////////////////////////////////////////////////////////////////


MDMaterialComponent::MDMaterialComponent(double A, double Z, 
                                         double Weight, int Type)
{
  // default constructor

  SetA(A);
  SetZ(Z);
  SetWeight(Weight);
  SetType(Type);
}


////////////////////////////////////////////////////////////////////////////////


MDMaterialComponent::~MDMaterialComponent()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterialComponent::SetA(double A)
{
  // Set the amic mass of this component

  // In GCALOR Vacuum needs A = 0
  massert(A >= 0);

  m_A = A;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterialComponent::SetZ(double Z)
{
  // Set the number of protons/ of this component

  // In GCALOR Vacuum needs A = 0
  massert(Z >= 0);
  massert(Z <= 100);

  m_Z = Z;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterialComponent::SetWeight(double Weight)
{
  // Set the number of atoms or the fractional mass of this component
  // What it is is controled via the type

  massert(Weight > 0);

  m_Weight = Weight;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterialComponent::SetType(int Type)
{
  // Set the type, which is either component by mass of by atoms

  massert(Type == c_ByAtoms || Type == c_ByMass);  

  m_Type = Type;
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterialComponent::GetA()
{
  // Return the atomic mass of this component

  return m_A;
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterialComponent::GetZ()
{
  // Return the number of protons

  return m_Z;
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterialComponent::GetWeight()
{
  // Return the number of atoms or mass fraction

  return m_Weight;
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterialComponent::GetWeightByAtoms()
{
  // Return the number of atoms or mass fraction

  if (m_Type == c_ByAtoms) {
    return m_Weight;
  } else {
    return m_Weight/m_A;
  }
}


////////////////////////////////////////////////////////////////////////////////


int MDMaterialComponent::GetType()
{
  // Return the type, which is either component by mass of by atoms

  return m_Type;
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterialComponent::Validate()
{
  // check for initialisation errors:

  if (m_A < m_Z) {
    if ((m_Z != 1 && m_A != 1) ||
        (m_Z != 0 && m_A != 0)) {  // In GCALOR Vacuum needs A = Z = 0
      mout<<"   ***  Error  ***  in component "<<ToString()<<endl;
      mout<<"N < Z!"<<endl;
      return false;
    }
  }
  if (m_Z > 100) { // mgeant limitation!
    mout<<"   ***  Error  ***  in component "<<ToString()<<endl;
    mout<<"Z > 100!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterialComponent::ToString()
{
  // Return string with the content of this instance

  ostringstream out;
  if (m_Type == c_ByAtoms) {
    out<<"A="<<m_A<<", Z="<<m_Z<<", atoms="<<m_Weight;
  } else {
    out<<"A="<<m_A<<", Z="<<m_Z<<", mass fraction="<<m_Weight;
  }

  return out.str().c_str();  
}


// MDMaterialComponent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
