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


#ifdef ___CLING___
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


//! Less than operator for sorting!
bool MIsotope::operator<(const MIsotope& I) const
{
  if (m_Nucleons < I.m_Nucleons) {
    return true;
  } else if (m_Nucleons > I.m_Nucleons) {
    return false;
  } else {
    return (m_Element < I.m_Element); 
  }
    
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
void MIsotope::AddLine(double Energy, double BranchingRatio, const MString& Flags, bool ExcludeExcludedLines)
{ 
  if (ExcludeExcludedLines == false || (ExcludeExcludedLines == true && Flags.Contains("E") == false)) {
    m_LineEnergies.push_back(Energy);
    m_LineBranchingRatios.push_back(BranchingRatio);
    m_LineExcludeFlags.push_back(false);
    m_LineDefaultFlags.push_back(false);
    if (Flags.Contains("E") == true) m_LineExcludeFlags.back() = true;
    if (Flags.Contains("D") == true) m_LineDefaultFlags.back() = true;
  }
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


//! Return the line exclude flag
bool MIsotope::GetLineExcludeFlag(unsigned int i) const
{
  if (i < m_LineExcludeFlags.size()) return m_LineExcludeFlags[i];
  
  merr<<"Index out of bounds: i="<<i<<" vs. size()="<<m_LineExcludeFlags.size()<<fatal;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the default line, or -1 if non could be found
int MIsotope::GetDefaultLine() const
{
  if (m_LineDefaultFlags.size() == 0) return -1;
  
  // If we have defined one , and it is not excluded
  for (unsigned int l = 0; l < m_LineDefaultFlags.size(); ++l) {
    if (m_LineDefaultFlags[l] == true && m_LineExcludeFlags[l] == false) {
      return l; 
    }
  }
  
  // If not return the line with the strongest branching ratio which is not excluded
  int Line = -1;
  double Strength = -1;
  for (unsigned int l = 0; l < m_LineBranchingRatios.size(); ++l) {
    if (m_LineExcludeFlags[l] == false) {
      if (m_LineBranchingRatios[l] > Strength) {
        Line = l;
        Strength = m_LineBranchingRatios[l];
      }
    }
  }
  
  return Line;
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
