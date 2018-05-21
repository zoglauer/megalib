/*
 * MPeak.cxx
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
// MPeak
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPeak.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MPeak)
#endif


////////////////////////////////////////////////////////////////////////////////


MPeak::MPeak()
{
  // Construct an instance of MPeak
  
  m_Energy = 0;
  m_EnergySigma = 0;
  m_PeakCounts = 0;
  m_BackgroundCounts = 0;
}


////////////////////////////////////////////////////////////////////////////////


MPeak::~MPeak()
{
  // Delete this instance of MPeak
}


////////////////////////////////////////////////////////////////////////////////


void MPeak::AddIsotope(MQualifiedIsotope* I, unsigned int LineID) 
{ 
  //! Add candidate isotope:

  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    if (m_Isotopes[i] == I) return; // We intentinally ignore the line ID...
  }

  m_Isotopes.push_back(I); 
  m_IsotopeLineIDs.push_back(LineID); 
}



////////////////////////////////////////////////////////////////////////////////


MQualifiedIsotope* MPeak::GetIsotope(unsigned int i) const
{
  //! Get the isotope

  if (i < m_Isotopes.size()) return m_Isotopes[i];
  merr<<"Index out of bounds: i="<<i<<" vs. size()="<<m_Isotopes.size()<<endl;
  abort();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MPeak::ContainsIsotope(MQualifiedIsotope* I) const 
{
  //! Contains the isotope

  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    if (m_Isotopes[i] == I) return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MPeak::RemoveIsotope(MQualifiedIsotope* Isotope)
{
  //! Remove the isotope
  
  vector<MQualifiedIsotope*>::iterator I;
  vector<unsigned int>::iterator L;
  
  for (I = m_Isotopes.begin(), L = m_IsotopeLineIDs.begin(); I != m_Isotopes.end(); ) {
    if ((*I) == Isotope) {
      I = m_Isotopes.erase(I);
      L = m_IsotopeLineIDs.erase(L);
    } else {
      ++I;
      ++L;
    } 
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MPeak::GetIsotopeLineID(unsigned int i) const
{
  //! Get the isotope line ID

  if (i < m_IsotopeLineIDs.size()) return m_IsotopeLineIDs[i];
  merr<<"Index out of bounds: i="<<i<<" vs. size()="<<m_IsotopeLineIDs.size()<<endl;
  abort();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MPeak& Peak)
{
  os<<"Peak at "<<Peak.GetEnergy()<<"+-"<<Peak.GetEnergySigma()<<" keV (cts: "<<Peak.GetPeakCounts()<<" Sign:"<<Peak.GetSignificance()<<")";
  if (Peak.GetNIsotopes() > 0) {
    os<<" - Candidates: ";
    for (unsigned int i = 0; i < Peak.GetNIsotopes(); ++i) {
      os<<Peak.GetIsotope(i)->GetElement();
      if (Peak.GetIsotope(i)->GetNucleons() > 0) {
        os<<"-"<<Peak.GetIsotope(i)->GetNucleons();
      }
      os<<" ("<<Peak.GetIsotope(i)->GetLineEnergy(Peak.GetIsotopeLineID(i))<<" keV)";
      if (i != Peak.GetNIsotopes()-1) os<<", ";
    }
  }
  os<<endl;    

  return os;
}


// MPeak.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
