/*
 * MIsotope.h
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MIsotope__
#define __MIsotope__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MIsotope
{
  // public interface:
 public:
  MIsotope();
  virtual ~MIsotope();

  //! Return the element name
  MString GetName() const;
 	
	//! Set the element name
  void SetElement(const MString& Element) { m_Element = Element; }
  //! Return the element name
  MString GetElement() const { return m_Element; }
  
  //! Set the number of nucleons
  void SetNucleons(const unsigned int Nucleons) { m_Nucleons = Nucleons; }
  //! Get the number of nucleons
  unsigned int GetNucleons() const { return m_Nucleons; }
  
  //! Add line parameters:
  void AddLine(double Energy, double BranchingRatio);
  //! Return the number of lines
  unsigned GetNLines() const { return m_LineEnergies.size(); }
  //! Return the line energy
  double GetLineEnergy(unsigned int l) const;
  //! Return the line branching ratio
  double GetLineBranchingRatio(unsigned int l) const;

  //! Set line found
  void SetLineFound(unsigned int l, bool Found);
  // Get line found
  bool GetLineFound(unsigned int l);

  //! Set the rating of the isotope
  void SetRating(double Rating) { m_Rating = Rating; }
  //! Add something to the rating of the isotope
  void AddRating(double Rating) { m_Rating += Rating; }  
  //! Get the rating of the isotope
  double GetRating() const { return m_Rating; }
  
  // protected methods:
 protected:
  //MIsotope() {};
  //MIsotope(const MIsotope& Isotope) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  //! The element name
  MString m_Element;
  //! The number of nucleons
  unsigned int m_Nucleons; 
 
  //! The line energy
  vector<double> m_LineEnergies;
  //! The line branching ratio
  vector<double> m_LineBranchingRatios;
  //! The line branching ratio
  vector<bool> m_LineFound;

  //! The rating of the isotope
  double m_Rating;
 

#ifdef ___CINT___
 public:
  ClassDef(MIsotope, 0) // no description
#endif

};

//! Stream a nucleon
ostream& operator<<(ostream& os, const MIsotope& Isotope);

#endif


////////////////////////////////////////////////////////////////////////////////
