/*
 * MDShapeCONE.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeCONE__
#define __MDShapeCONE__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a cone shape
class MDShapeCONE : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeCONE(const MString& Name);
  //! Default destructor
  virtual ~MDShapeCONE();

  //! Set the all parameters of the shape
  bool Set(double HalfHeight, double RminBottom, double RmaxBottom, double RminTop, double RmaxTop);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  MString ToString();
  MString GetGeomega() const;

  double GetHalfHeight();
  double GetRminBottom();
  double GetRmaxBottom();
  double GetRminTop();
  double GetRmaxTop();

  double GetVolume();

  //! Scale this shape by Factor
  virtual void Scale(const double Factor);

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
  double m_HalfHeight;
  double m_RminBottom;
  double m_RmaxBottom;
  double m_RminTop;
  double m_RmaxTop;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeCONE, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
