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


MDMaterialComponent::MDMaterialComponent()
{
  // default constructor
  
  SetByAtomicWeighting("H", 1);
}


////////////////////////////////////////////////////////////////////////////////


MDMaterialComponent::MDMaterialComponent(const MDMaterialComponent& C)
{
  // default constructor

  m_AtomicWeight = C.m_AtomicWeight;
  m_AtomicNumber = C.m_AtomicNumber;
  m_WeightingType = C.m_WeightingType;
  m_Weighting = C.m_Weighting;
  m_HasNaturalComposition = C.m_HasNaturalComposition;
}


////////////////////////////////////////////////////////////////////////////////


MDMaterialComponent::~MDMaterialComponent()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterialComponent::SetElement(MString Name) 
{
  // Set the element by name
  
  vector<MString> ElementName = {
      "H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg",
      "Al","Si","P","S","Cl","Ar","K","Ca","Sc","Ti","V","Cr",
      "Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
      "Rb","Sr","Y","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
      "In","Sn","Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd",
      "Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
      "Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
      "At","Rn","Fr","Ra","Ac","Th","Pa","U","Np","Pu","Am","Cm",
      "Bk","Cf","Es","Fm","Md","No","Lr","Rf","Db","Sg","Bh","Hs","Mt","Rg"};
 
  vector<double> AverageAtomicWeight = { 
    1.0079, 4.0026, 6.941, 9.0122, 10.811, 12.0107, 14.0067, 15.9994, 18.9984, 20.1797, 22.9897, 24.305, 26.9815, 28.0855, 30.9738, 32.065, 35.453, 39.948, 39.0983, 40.078, 44.9559, 47.867, 50.9415, 51.9961, 54.938, 55.845, 58.9332, 58.6934, 63.546, 65.39, 69.723, 72.64, 74.9216, 78.96, 79.904, 83.8, 85.4678, 87.62, 88.9059, 91.224, 92.9064, 95.94, 98, 101.07, 102.9055, 106.42, 107.8682, 112.411, 114.818, 118.71, 121.76, 127.6, 126.9045, 131.293, 132.9055, 137.327, 138.9055, 140.116, 140.9077, 144.24, 145, 150.36, 151.964, 157.25, 158.9253, 162.5, 164.9303, 167.259, 168.9342, 173.04, 174.967, 178.49, 180.9479, 183.84, 186.207, 190.23, 192.217, 195.078, 196.9665, 200.59, 204.3833, 207.2, 208.9804, 209, 210, 222, 223, 226, 227, 232.0381, 231.0359, 238.0289, 237, 244, 243, 247, 247, 251, 252, 257, 258, 259, 262, 261, 262, 266, 264, 277, 268, 272
  };
      
      
  bool Found = false;
  for (unsigned int i = 0; i < 100; ++i) {
    MString Element(ElementName[i]);
    Element.ToLower();
    Name.ToLower();
    if (Element == Name) {
      m_AtomicNumber = i+1;
      m_AtomicWeight = AverageAtomicWeight[i];
      Found = true;
      break;
    }
  }
  if (Found == false) return false;
      
  m_HasNaturalComposition = true;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////


//! Set a component by atomic weighting
bool MDMaterialComponent::SetByAtomicWeighting(double AtomicWeight, int AtomicNumber, int Weighting)
{
  if (AtomicWeight <= 0) {
    mout<<"   ***  Error  ***  in material component with A="<<AtomicWeight<<" and Z="<<AtomicNumber<<endl;
    mout<<"The atomic weight must be positive."<<endl;
    return false;    
  }
  
  if (AtomicNumber <= 0 || AtomicNumber > 128) {
    mout<<"   ***  Error  ***  in material component with A="<<AtomicWeight<<" and Z="<<AtomicNumber<<endl;
    mout<<"The atomic number must be within [1...128]."<<endl;
    return false;    
  }
  
  if (Weighting <= 0) {
    mout<<"   ***  Error  ***  in material component with A="<<AtomicWeight<<" and Z="<<AtomicNumber<<endl;
    mout<<"The weighting by atoms must be a positive number."<<endl;
    return false;        
  }
  
  m_AtomicWeight = AtomicWeight;
  m_AtomicNumber = AtomicNumber;
  m_WeightingType = MDMaterialComponentWeightingType::c_ByAtoms;
  m_Weighting = Weighting;
  m_HasNaturalComposition = false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a component by atomic weighting with natural composition
bool MDMaterialComponent::SetByAtomicWeighting(MString Name, int Weighting)
{
  if (Weighting <= 0) {
    mout<<"   ***  Error  ***  in material component "<<Name<<endl;
    mout<<"The weighting by atoms must be a positive number."<<endl;
    return false;        
  }
  
  if (SetElement(Name) == false) {
    mout<<"   ***  Error  ***  in material component "<<Name<<endl;
    mout<<"Cannot set the element."<<endl;
    return false; 
  }
  
  m_WeightingType = MDMaterialComponentWeightingType::c_ByAtoms;
  m_Weighting = Weighting;
  m_HasNaturalComposition = true;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a component by mass weighting
bool MDMaterialComponent::SetByMassWeighting(double AtomicWeight, int AtomicNumber, double Weighting)
{
  if (AtomicWeight <= 0) {
    mout<<"   ***  Error  ***  in material component with A="<<AtomicWeight<<" and Z="<<AtomicNumber<<endl;
    mout<<"The atomic weight must be positive."<<endl;
    return false;    
  }
  
  if (AtomicNumber <= 0 || AtomicNumber > 128) {
    mout<<"   ***  Error  ***  in material component with A="<<AtomicWeight<<" and Z="<<AtomicNumber<<endl;
    mout<<"The atomic number must be within [1...128]."<<endl;
    return false;    
  }
  
  if (Weighting <= 0) {
    mout<<"   ***  Error  ***  in material component with A="<<AtomicWeight<<" and Z="<<AtomicNumber<<endl;
    mout<<"The weighting by mass must be a positive number."<<endl;
    return false;        
  }
  
  m_AtomicWeight = AtomicWeight;
  m_AtomicNumber = AtomicNumber;
  m_WeightingType = MDMaterialComponentWeightingType::c_ByMass;
  m_Weighting = Weighting;
  m_HasNaturalComposition = false;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////


//! Set a component by mass weighting with natural composition
bool MDMaterialComponent::SetByMassWeighting(MString Name, double Weighting)
{
  if (Weighting <= 0) {
    mout<<"   ***  Error  ***  in material component "<<Name<<endl;
    mout<<"The weighting by atoms must be a positive number."<<endl;
    return false;        
  }
  
  if (SetElement(Name) == false) {
    mout<<"   ***  Error  ***  in material component "<<Name<<endl;
    mout<<"Cannot set the element."<<endl;
    return false; 
  }
  
  m_WeightingType = MDMaterialComponentWeightingType::c_ByMass;
  m_Weighting = Weighting;
  m_HasNaturalComposition = true;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the weighting by mass
bool MDMaterialComponent::SetWeightingByMass(double Weighting)
{
  if (Weighting <= 0) {
    mout<<"   ***  Error  *** "<<endl;
    mout<<"The weighting by atoms must be a positive number."<<endl;
    return false;        
  }
  
  m_WeightingType = MDMaterialComponentWeightingType::c_ByMass;
  m_Weighting = Weighting;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the weighting by atoms
bool MDMaterialComponent::SetWeightingByAtoms(int Weighting)
{
  if (Weighting <= 0) {
    mout<<"   ***  Error  *** "<<endl;
    mout<<"The weighting by atoms must be a positive number."<<endl;
    return false;        
  }
  
  m_WeightingType = MDMaterialComponentWeightingType::c_ByAtoms;
  m_Weighting = Weighting;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterialComponent::Validate()
{
  // check for initialisation errors:

  if (m_AtomicWeight < m_AtomicNumber) {
    if ((m_AtomicNumber != 1 && m_AtomicWeight != 1) ||
        (m_AtomicNumber != 0 && m_AtomicWeight != 0)) {  // In GCALOR Vacuum needs A = Z = 0
      mout<<"   ***  Error  ***  in component "<<ToString()<<endl;
      mout<<"N < Z!"<<endl;
      return false;
    }
  }
  if (m_AtomicNumber > 100) { // mgeant limitation!
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
    out<<"Z="<<m_AtomicNumber<<" (A=natural composition), ";
  } else {
    out<<"Z="<<m_AtomicNumber<<", A="<<m_AtomicWeight<<", ";
  }
  
  if (m_WeightingType == MDMaterialComponentWeightingType::c_ByAtoms) {
    out<<"atoms="<<GetWeightingByAtoms();
  } else {
    out<<"mass fraction="<<GetWeightingByMass();
  }

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterialComponent::GetGeomega() const
{
  // Return the content in Geomega format:

  ostringstream out;

  if (m_WeightingType == MDMaterialComponentWeightingType::c_ByAtoms) {
    out<<".ComponentByAtoms ";
  } else {
    out<<".ComponentByMass ";      
  }
  if (HasNaturalIsotopeComposition() == true) {
    out<<MDMaterial::ConvertAtomicNumberToName(GetAtomicNumber())<<" ";
  } else {
    out<<m_AtomicWeight<<" "<<m_AtomicNumber<<" ";
  }
  if (m_WeightingType == MDMaterialComponentWeightingType::c_ByAtoms) {
    out<<GetWeightingByAtoms();
  } else {
    out<<GetWeightingByMass();
  }

  return out.str().c_str();
}


// MDMaterialComponent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
