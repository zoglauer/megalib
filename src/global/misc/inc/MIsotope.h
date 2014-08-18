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

//! An isotope with its DAUGHTER! decay lines
class MIsotope
{
  // public interface:
 public:
  //! Deafault constructor
  MIsotope();
  //! Default destructor
  virtual ~MIsotope();

  //! Compare two isotopes
  virtual bool operator==(const MIsotope& I) const;

  //! Is lower operator for sorting by name
  bool operator<(const MIsotope& I) const;

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
  virtual void AddLine(double Energy, double BranchingRatio, const MString& Flags);
  //! Return the number of lines
  unsigned GetNLines() const { return m_LineEnergies.size(); }
  //! Return the line energy
  double GetLineEnergy(unsigned int l) const;
  //! Return the line branching ratio
  double GetLineBranchingRatio(unsigned int l) const;
  //! Return the line exclude flag
  double GetLineExcludeFlag(unsigned int l) const;
  
  //! Dump a string
  virtual MString ToString() const;
  
  // protected methods:
 protected:

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
  //! The line exclude flags
  vector<bool> m_LineExcludeFlags;
 

#ifdef ___CINT___
 public:
  ClassDef(MIsotope, 0) // no description
#endif

};

//! Stream an isotope
ostream& operator<<(ostream& os, const MIsotope& Isotope);

#endif


////////////////////////////////////////////////////////////////////////////////
