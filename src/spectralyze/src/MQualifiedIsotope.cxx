/*
 * MQualifiedIsotope.cxx
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
// MQualifiedIsotope
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MQualifiedIsotope.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MQualifiedIsotope)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MQualifiedIsotope::MQualifiedIsotope() : MIsotope()
{
  m_Rating = -1;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MQualifiedIsotope::~MQualifiedIsotope()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Add line parameters:
void MQualifiedIsotope::AddLine(double Energy, double BranchingRatio, const MString& Flags, bool ExcludeExcludedLines) 
{ 
  MIsotope::AddLine(Energy, BranchingRatio, Flags);
  m_LineFound.push_back(false);
}


////////////////////////////////////////////////////////////////////////////////


//! Set line found
void MQualifiedIsotope::SetLineFound(unsigned int l, bool Found)
{
  if (l < m_LineFound.size()) {
    m_LineFound[l] = Found;
    return;
  }

  merr<<"Index out of bounds: l="<<l<<" vs. size()="<<m_LineFound.size()<<fatal;
}


////////////////////////////////////////////////////////////////////////////////


//! Return if the line has been found
bool MQualifiedIsotope::GetLineFound(unsigned int l)
{
  if (l < m_LineFound.size()) return m_LineFound[l];

  merr<<"Index out of bounds: l="<<l<<" vs. size()="<<m_LineFound.size()<<fatal;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MQualifiedIsotope& Isotope)
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

// MQualifiedIsotope.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
