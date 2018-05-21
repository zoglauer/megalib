/*
 * MDShapeTRAP.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTRAP__
#define __MDShapeTRAP__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a trapzoid shape
class MDShapeTRAP : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeTRAP(const MString& Name);
  //! Default destructor
  virtual ~MDShapeTRAP();

  //! Set the all parameters of the shape
  bool Set(double Dz, double Theta, double Phi, 
           double H1, double Bl1, double Tl1, double Alpha1, 
           double H2, double Bl2, double Tl2, double Alpha2);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  double GetDz() const;
  double GetTheta() const;
  double GetPhi() const;
  double GetH1() const;
  double GetBl1() const;
  double GetTl1() const;
  double GetAlpha1() const;
  double GetH2() const;
  double GetBl2() const;
  double GetTl2() const;
  double GetAlpha2() const;

  MString ToString();
  MString GetGeomega() const;

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
  double m_Dz;
  double m_Theta;
  double m_Phi;
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
  ClassDef(MDShapeTRAP, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
