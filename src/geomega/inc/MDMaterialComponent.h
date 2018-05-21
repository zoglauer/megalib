/*
 * MDMaterialComponent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDMaterialComponent__
#define __MDMaterialComponent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! enum defining the type of weighting, either by mass or by atoms
enum class MDMaterialComponentWeightingType : int { c_ByAtoms, c_ByMass };


////////////////////////////////////////////////////////////////////////////////
  

//! One component of a material
class MDMaterialComponent
{
  // public interface:
 public:
  //! Default constructor
  MDMaterialComponent();
  //! Copy constructor
  MDMaterialComponent(const MDMaterialComponent& MaterialComponent);
  //! Default constructor
  virtual ~MDMaterialComponent();

  //! Set a component by atomic weighting
  bool SetByAtomicWeighting(double AtomicWeight, int AtomicNumber, int Weighting);
  //! Set a component by atomic weighting with natural composition
  bool SetByAtomicWeighting(MString Name, int Weighting);
  //! Set a component by mass weighting
  bool SetByMassWeighting(double AtomicWeight, int AtomicNumber, double Weighting);
  //! Set a component by mass weighting with natural composition
  bool SetByMassWeighting(MString Name, double Weighting);
  
  //! Set the weighting by mass
  bool SetWeightingByMass(double Weighting);
  //! Set the weighting by atoms
  bool SetWeightingByAtoms(int Weighting);
  
  //! Get the atomic weight
  double GetAtomicWeight() const { return m_AtomicWeight; }
  //! Number of protons...
  int GetAtomicNumber() const { return m_AtomicNumber; }
  //! Get weighting by atoms
  //! Attention if you used a weighting by mass, call the correct function, or the number will be rounded to an integer
  int GetWeightingByAtoms() const { return int(m_Weighting + 0.5); }
  //! Get the weighting by mass
  double GetWeightingByMass() const { return m_Weighting; }
  //! Get the original type of the weighting
  MDMaterialComponentWeightingType GetWeightingType() const { return m_WeightingType; };
  //! Return true if we have natural composition
  bool HasNaturalIsotopeComposition() const { return m_HasNaturalComposition; };
  
  //! Validate the element
  bool Validate();
  
  //! Return in Geomega compatible format
  MString GetGeomega() const;

  //! Return as a simple string
  MString ToString() const;

  
  // protected methods:
 protected:
  //! Set the element given a name, return true on success
  bool SetElement(MString Name); 
   

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Atomic weight
  double m_AtomicWeight;
  //! Atomic number
  double m_AtomicNumber;
  //! The weight type, either by mass or by element
  MDMaterialComponentWeightingType m_WeightingType;
  //! Weighting of the element
  double m_Weighting;
  //! True if this element has natural composition
  bool m_HasNaturalComposition;



#ifdef ___CLING___
 public:
  ClassDef(MDMaterialComponent, 0) // one kind of atoms/isotopes for a material
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
