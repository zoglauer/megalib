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


class MDMaterialComponent
{
  // public interface:
 public:
  MDMaterialComponent(double AtomicMass, double NProtons, double Weight, int Type);
  virtual ~MDMaterialComponent();

  bool Validate();

  void SetA(double A);
  void SetZ(double Z);
  void SetWeight(double Weight);
  void SetType(int Type);

  //! Number of nucleons...
  double GetA(); 
  //! Number of protons...
  double GetZ();
  //! Get weighting as stored 
  double GetWeight();
  //! Get the original type of the weight
  int GetType();
  //! Get weight by atoms
  double GetWeightByAtoms();

  MString ToString();

  static const int c_ByAtoms;
  static const int c_ByMass;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_A;
  double m_Z;
  double m_Weight;

  int m_Type;


#ifdef ___CINT___
 public:
  ClassDef(MDMaterialComponent, 0) // one kind of atoms/isotopes for a material
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
