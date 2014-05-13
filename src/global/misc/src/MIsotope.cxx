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


//! Default constructor
MIsotope::MIsotope()
{
  m_Element = "";
  m_Nucleons = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MIsotope::~MIsotope()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Check for equality
bool MIsotope::operator==(const MIsotope& I) const
{
  if (m_Element != I.m_Element || m_Nucleons != I.m_Nucleons) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the element name
MString MIsotope::GetName() const
{
  ostringstream out;
  out<<GetElement();
  if (GetNucleons() > 0) {
    out<<"-"<<GetNucleons();
  }

  return out.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Add line parameters:
void MIsotope::AddLine(double Energy, double BranchingRatio) 
{ 
  m_LineEnergies.push_back(Energy); 
  m_LineBranchingRatios.push_back(BranchingRatio); 
}

////////////////////////////////////////////////////////////////////////////////


//! Return the line energy  
double MIsotope::GetLineEnergy(unsigned int i) const
{
  if (i < m_LineEnergies.size()) return m_LineEnergies[i];

  merr<<"Index out of bounds: i="<<i<<" vs. size()="<<m_LineEnergies.size()<<endl;
  return 0;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Return the line branching ratio
double MIsotope::GetLineBranchingRatio(unsigned int i) const
{
  if (i < m_LineBranchingRatios.size()) return m_LineBranchingRatios[i];

  merr<<"Index out of bounds: i="<<i<<" vs. size()="<<m_LineBranchingRatios.size()<<fatal;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MIsotope::ToString() const
{
  ostringstream os;
  os<<"Isotope "<<GetName();
  if (m_LineEnergies.size() > 0) {
    os<<" with the following lines: ";
    for (unsigned int i = 0; i < m_LineEnergies.size(); ++i) {
      os<<m_LineEnergies[i]<<" keV ("<<m_LineBranchingRatios[i]<<")";
      if (i != m_LineEnergies.size()-1) os<<", ";
    }  
  }

  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Stream the isotope
ostream& operator<<(ostream& os, const MIsotope& Isotope)
{
  os<<Isotope.ToString();

  return os;
}


// MIsotope.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
