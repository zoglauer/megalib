/*
 * MDShapeTRD2.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTRD2__
#define __MDShapeTRD2__


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
class MDShapeTRD2 : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeTRD2(const MString& Name);
  //! Default destructor
  virtual ~MDShapeTRD2();

  //! Set the all parameters of the shape
  bool Set(double dx1, double dx2, double dy1, double dy2, double z);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  double GetDx1() const;
  double GetDx2() const;
  double GetDy1() const;
  double GetDy2() const;
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
  double m_Dx1;
  double m_Dx2;
  double m_Dy1;
  double m_Dy2;
  double m_Z;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeTRD2, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
