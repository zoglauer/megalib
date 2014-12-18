/*
 * MDShapeTUBS.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTUBS__
#define __MDShapeTUBS__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a tube shape
class MDShapeTUBS : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeTUBS(const MString& Name);
  //! Default destructor
  virtual ~MDShapeTUBS();

  //! Set the all parameters of the shape
  bool Set(double Rmin, double Rmax, double HalfHeight, double Phi1 = 0.0, double Phi2 = 360.0);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  MString ToString();
  MString GetGeant3DIM(MString ShortName);
  MString GetGeant3DATA(MString ShortName);
  MString GetGeant3ShapeName();
  int GetGeant3NumberOfParameters();
  MString GetMGeantDATA(MString ShortName);
  MString GetGeomega() const;
  
  double GetRmin();
  double GetRmax();
  double GetHeight();
  double GetPhi1();
  double GetPhi2();

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
  double m_HalfHeight;
  double m_Phi1;
  double m_Phi2;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeTUBS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
