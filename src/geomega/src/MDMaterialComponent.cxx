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
#include "MDMaterial.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDMaterialComponent)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDMaterialComponent::c_ByAtoms = 1;
const int MDMaterialComponent::c_ByMass = 2;
const double MDMaterialComponent::c_NaturalComposition = 999999999.99987654;


////////////////////////////////////////////////////////////////////////////////


MDMaterialComponent::MDMaterialComponent(): m_A(c_NaturalComposition), m_Z(1), m_Weight(1), m_Type(c_ByAtoms)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MDMaterialComponent::MDMaterialComponent(const MDMaterialComponent& C)
{
  // default constructor

  SetA(C.m_A);
  SetZ(C.m_Z);
  SetWeight(C.m_Weight);
  SetType(C.m_Type);
}


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


bool MDMaterialComponent::SetElement(MString Name) {
  // Set the element by name
  
  const char* elem[] = {"H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg",
      "Al","Si","P","S","Cl","Ar","K","Ca","Sc","Ti","V","Cr",
      "Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
      "Rb","Sr","Y","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
      "In","Sn","Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd",
      "Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
      "Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
      "At","Rn","Fr","Ra","Ac","Th","Pa","U","Np","Pu","Am","Cm",
      "Bk","Cf","Es","Fm","Md","No","Lr"};
 
  bool Found = false;
  for (unsigned int i = 0; i < 100; ++i) {
    MString Element(elem[i]);
    Element.ToLower();
    Name.ToLower();
    if (Element == Name) {
      m_Z = i+1;
      Found = true;
      break;
    }
  }
  if (Found == false) return false;
      
  m_A = c_NaturalComposition; // we don't need this anymore!
  
  return true;
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

  if (Weight <= 0) {
    mout<<"   ***  Warning  ***  in component "<<ToString()<<endl;
    mout<<"You have a material component with a non-positive weighting factor!"<<endl;
    m_Weight = 0;
  } else {
    m_Weight = Weight;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterialComponent::SetType(int Type)
{
  // Set the type, which is either component by mass of by atoms

  massert(Type == c_ByAtoms || Type == c_ByMass);  

  m_Type = Type;
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterialComponent::GetA() const
{
  // Return the atomic mass of this component

  return m_A;
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterialComponent::GetZ() const
{
  // Return the number of protons

  return m_Z;
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterialComponent::GetWeight() const
{
  // Return the number of atoms or mass fraction

  return m_Weight;
}


////////////////////////////////////////////////////////////////////////////////


int MDMaterialComponent::GetType() const
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


MString MDMaterialComponent::ToString() const
{
  // Return string with the content of this instance

  ostringstream out;
  if (HasNaturalIsotopeComposition()) {
    out<<"Z="<<m_Z<<" (A=natural composition), ";
  } else {
    out<<"Z="<<m_Z<<", A="<<m_A<<", ";
  }
  
  if (m_Type == c_ByAtoms) {
    out<<"atoms="<<m_Weight;
  } else {
    out<<"mass fraction="<<m_Weight;
  }

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterialComponent::GetGeomega() const
{
  // Return the content in Geomega format:

  ostringstream out;

  if (GetType() == MDMaterialComponent::c_ByAtoms) {
    out<<".ComponentByAtoms ";
  } else {
    out<<".ComponentByMass ";      
  }
  if (HasNaturalIsotopeComposition() == true) {
    out<<MDMaterial::ConvertZToString(GetZ())<<" ";
  } else {
    out<<GetA()<<" "<<GetZ()<<" ";
  }
  out<<GetWeight();
  

  return out.str().c_str();
}


// MDMaterialComponent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
