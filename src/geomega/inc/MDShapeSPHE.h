/*
 * MDShapeSPHE.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeSPHE__
#define __MDShapeSPHE__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a spherical shape
class MDShapeSPHE : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeSPHE(const MString& Name);
  //! Default destructor
  virtual ~MDShapeSPHE();

  //! Set the all parameters of the shape
  bool Set(double Rmin, double Rmax);
  //! Set the all parameters of the shape
  bool Set(double Rmin, double Rmax, double Thetamin, double Thetamax, double Phimin, double Phimax);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  double GetRmin() const;
  double GetRmax() const;
  double GetThetamin() const;
  double GetThetamax() const;
  double GetPhimin() const;
  double GetPhimax() const;

  MString ToString();
  MString GetGeomega() const;

  double GetVolume();

  //! Scale this shape by Factor
  virtual void Scale(const double Factor);

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  //! Return a random position withn this volume
  virtual MVector GetRandomPositionInside(); 

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_Rmin;
  double m_Rmax;
  double m_Thetamin;
  double m_Thetamax;
  double m_Phimin;
  double m_Phimax;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeSPHE, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
