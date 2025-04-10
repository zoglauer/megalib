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

  //! Set the all parameters of the shape - The definition is the same as in ROOT
  //! dz: half-length along the z axis
  //! theta: polar angle of the line joining the centre of the face at -DZ to the centre of the one at +DZ
  //! phi: azimuthal angle of the line joining the centre of the face at -DZ to the centre of the one at +DZ
  //! h1: half-length along y of the face at -DZ
  //! bl1: half-length along x of the side at -H1 in y of the face at -DZ in z
  //! tl1: half-length along x of the side at +H1 in y of the face at -DZ in z
  //! alpha1: angle with respect to the y axis from the centre of the side at -H1 in y to the centre of the side at +H1 in y of the face at -DZ in z
  //! h2: half-length along y of the face at +DZ
  //! bl2: half-length along x of the side at -H2 in y of the face at +DZ in z
  //! tl2: half-length along x of the side at +H2 in y of the face at +DZ in z
  //! alpha2: angle with respect to the y axis from the centre of the side at -H2 in y to the centre of the side at +H2 in y of the face at +DZ in z
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
