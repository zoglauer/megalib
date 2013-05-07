/*
 * MIsotope.cxx
 *
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Michelle Galloway & Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MIsotope
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MIsotope.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MIsotope)
#endif


////////////////////////////////////////////////////////////////////////////////


MIsotope::MIsotope()
{
  // Construct an instance of MIsotope

  m_Element = "";
  m_Nucleons = 0;
  m_Rating = -1;
}


////////////////////////////////////////////////////////////////////////////////


MIsotope::~MIsotope()
{
  // Delete this instance of MIsotope
}

////////////////////////////////////////////////////////////////////////////////


MString MIsotope::GetName() const
{
  //! Return the element name

  ostringstream out;
  out<<GetElement();
  if (GetNucleons() > 0) {
    out<<"-"<<GetNucleons();
  }

  return out.str();
}


////////////////////////////////////////////////////////////////////////////////


void MIsotope::AddLine(double Energy, double BranchingRatio) 
{ 
  //! Add line parameters:

  m_LineEnergies.push_back(Energy); 
  m_LineBranchingRatios.push_back(BranchingRatio); 
  m_LineFound.push_back(false); 
}

////////////////////////////////////////////////////////////////////////////////


double MIsotope::GetLineEnergy(unsigned int i) const
{
  //! Return the line energy
  
  if (i < m_LineEnergies.size()) return m_LineEnergies[i];

  merr<<"Index out of bounds: i="<<i<<" vs. size()="<<m_LineEnergies.size()<<endl;
  abort();
  return 0;
}

  
////////////////////////////////////////////////////////////////////////////////


double MIsotope::GetLineBranchingRatio(unsigned int i) const
{
  //! Return the line branching ratio

  if (i < m_LineBranchingRatios.size()) return m_LineBranchingRatios[i];

  merr<<"Index out of bounds: i="<<i<<" vs. size()="<<m_LineBranchingRatios.size()<<fatal;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MIsotope::SetLineFound(unsigned int l, bool Found)
{
  //! Set line found

  if (l < m_LineFound.size()) {
    m_LineFound[l] = Found;
    return;
  }

  merr<<"Index out of bounds: l="<<l<<" vs. size()="<<m_LineFound.size()<<fatal;
}


////////////////////////////////////////////////////////////////////////////////


bool MIsotope::GetLineFound(unsigned int l)
{
 //Get line found

  if (l < m_LineFound.size()) return m_LineFound[l];

  merr<<"Index out of bounds: l="<<l<<" vs. size()="<<m_LineFound.size()<<fatal;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MIsotope& Isotope)
{
  os<<"Isotope "<<Isotope.GetName();
  if (Isotope.GetRating() > 0) {
    os<<" (rating:"<<Isotope.GetRating()<<")";  
  }
  if (Isotope.GetNLines() > 0) {
    os<<" with the following lines: ";
    for (unsigned int i = 0; i < Isotope.GetNLines(); ++i) {
      os<<Isotope.GetLineEnergy(i)<<" keV ("<<Isotope.GetLineBranchingRatio(i)<<")";
      if (i != Isotope.GetNLines()-1) os<<", ";
    }  
  }
  os<<endl;

  return os;
}

// MIsotope.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
