/*
 * MDShapePCON.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapePCON__
#define __MDShapePCON__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a polycone shape
class MDShapePCON : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapePCON(const MString& Name);
  //! Default destructor
  virtual ~MDShapePCON();

  //! Set the all parameters of the shape
  bool Set(double Phi, double DPhi, unsigned int NSection);
  //! Add a section
  bool AddSection(unsigned int Section, double z, double Rmin, double Rmax);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  MString ToString();
  MString GetGeomega() const;

  double GetPhi();
  double GetDPhi();
  unsigned int GetNSections();
  double GetZ(unsigned int Section);
  double GetRmin(unsigned int Section);
  double GetRmax(unsigned int Section);

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
  double m_Phi;
  double m_DPhi;
  unsigned int m_NSections;
  vector<double> m_Z;
  vector<double> m_Rmin;
  vector<double> m_Rmax;

#ifdef ___CINT___
 public:
  ClassDef(MDShapePCON, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
