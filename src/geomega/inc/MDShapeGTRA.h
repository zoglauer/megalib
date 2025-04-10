/*
 * MDShapeGTRA.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeGTRA__
#define __MDShapeGTRA__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a general trapezoid
class MDShapeGTRA : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeGTRA(const MString& Name);
  //! Default destructor
  virtual ~MDShapeGTRA();

  //! Set the all parameters of the shape
  bool Set(double Dz, double Theta, double Phi, double Twist, 
           double H1, double Bl1, double Tl1, double Alpha1, 
           double H2, double Bl2, double Tl2, double Alpha2);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  MString ToString();
  MString GetGeomega() const;

  double GetVolume();

  //! Scale the axes given in Axes by a factor Scaler
  virtual bool Scale(const double Scaler, const MString Axes = "XYZ");

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_Dz;
  double m_Theta;
  double m_Phi;
  double m_Twist;
  double m_H1;
  double m_Bl1;
  double m_Tl1;
  double m_Alpha1; 
  double m_H2;
  double m_Bl2;
  double m_Tl2;
  double m_Alpha2;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeGTRA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
