/*
 * MQualifiedIsotope.h
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MQualifiedIsotope__
#define __MQualifiedIsotope__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MIsotope.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Isotope with additional qualifiying characteristics, such as a rating
class MQualifiedIsotope : public MIsotope
{
  // public interface:
 public:
  MQualifiedIsotope();
  virtual ~MQualifiedIsotope();

  //! Add line parameters:
  void AddLine(double Energy, double BranchingRatio, const MString& Flags);
  //! Set line found
  void SetLineFound(unsigned int l, bool Found);
  //! Get line found
  bool GetLineFound(unsigned int l);

  //! Set the rating of the isotope
  void SetRating(double Rating) { m_Rating = Rating; }
  //! Add something to the rating of the isotope
  void AddRating(double Rating) { m_Rating += Rating; }  
  //! Get the rating of the isotope
  double GetRating() const { return m_Rating; }
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The line branching ratio
  vector<bool> m_LineFound;

  //! The rating of the isotope
  double m_Rating;
 

#ifdef ___CINT___
 public:
  ClassDef(MQualifiedIsotope, 0) // no description
#endif

};

//! Stream a nucleon
ostream& operator<<(ostream& os, const MQualifiedIsotope& Isotope);

#endif


////////////////////////////////////////////////////////////////////////////////
