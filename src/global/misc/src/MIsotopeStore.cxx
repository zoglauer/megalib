/*
 * MIsotopeStore.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MIsotopeStore.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"
#include "MParser.h"
#include "MExceptions.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MIsotopeStore)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MIsotopeStore::MIsotopeStore()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MIsotopeStore::~MIsotopeStore()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Load all isotopes form a standard MEGAlib isotope file
bool MIsotopeStore::Load(MString FileName)
{
  MFile::ExpandFileName(FileName);
  if (MFile::Exists(FileName) == false) return false;
  
  // Load the isotopes:
  MParser iParser;
  iParser.Open(FileName);

  MString Flags;
  m_Isotopes.clear();
  for (unsigned int i = 0; i < iParser.GetNLines(); ++i) {
    if (iParser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (iParser.GetTokenizerAt(i)->GetNTokens() < 4 || iParser.GetTokenizerAt(i)->GetNTokens() > 5) {
      mout<<"Isotope file: An isotope line must have 4-5 arguments: "<<iParser.GetTokenizerAt(i)->GetText()<<endl;
      continue;
    }
    bool Found = false;
    MString Name = iParser.GetTokenizerAt(i)->GetTokenAtAsString(0);
    MString Flags;
    if (iParser.GetTokenizerAt(i)->GetNTokens() == 5) Flags = iParser.GetTokenizerAt(i)->GetTokenAtAsString(4);
    unsigned int Nucleons = iParser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
    for (unsigned int s = 0; s < m_Isotopes.size(); ++s) {
      if (m_Isotopes[s].GetElement() == Name && m_Isotopes[s].GetNucleons() == Nucleons) {
        m_Isotopes[s].AddLine(iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(2), 
                              iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(3),
                              Flags);
        Found = true;
        break;
      }
    }
    if (Found == false) {
      MIsotope Isotope;
      Isotope.SetElement(Name);
      Isotope.SetNucleons(Nucleons);
      Isotope.AddLine(iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(2), 
                      iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(3),
                      Flags);
      m_Isotopes.push_back(Isotope);
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the isotope at the given position
//! If there is non throw an exception of type MExceptionIndexOutOfBounds
const MIsotope& MIsotopeStore::Get(unsigned int i) const
{
  if (i < m_Isotopes.size()) {
    return m_Isotopes[i];  
  }
  
  throw MExceptionIndexOutOfBounds(0, m_Isotopes.size(), i);
  
  // Might crash - but we never reach this line unless we turn off exceptions...
  return m_Isotopes[0];
}


////////////////////////////////////////////////////////////////////////////////

  
//! Return true if the store contains the isotope
bool MIsotopeStore::Contains(const MString& Name) const
{
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    if (Name == m_Isotopes[i].GetName()) return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the isotope with the given name
//! If there is non throw an exception of type MExceptionObjectDoesNotExist
const MIsotope& MIsotopeStore::Get(const MString& Name) const
{
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    if (Name == m_Isotopes[i].GetName()) {
      return m_Isotopes[i];
    }
  }
  
  throw MExceptionObjectDoesNotExist(Name);
  
  // Might crash - but we never reach this line unless we turn off exceptions...
  return m_Isotopes[0];  
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MIsotopeStore::ToString() const
{
  ostringstream os;
  os<<"Isotope store with "<<m_Isotopes.size()<<" isotope:"<<endl;
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    os<<m_Isotopes[i]<<endl;
  }
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Stream the isotope store
ostream& operator<<(ostream& os, const MIsotopeStore& I)
{
  os<<I.ToString();
  return os;
}


// MIsotopeStore.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
