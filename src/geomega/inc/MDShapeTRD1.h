/*
 * MDShapeTRD1.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTRD1__
#define __MDShapeTRD1__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a trapezoid shape
class MDShapeTRD1 : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeTRD1(const MString& Name);
  //! Default destructor
  virtual ~MDShapeTRD1();

  //! Set the all parameters of the shape
  bool Set(double dx1, double dx2, double y, double z);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  double GetDx1() const;
  double GetDx2() const;
  double GetY() const;
  double GetZ() const;

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
  double m_Ddx1;
  double m_Ddx2;
  double m_Dy;
  double m_Dz;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeTRD1, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
