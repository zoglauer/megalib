/*
 * MIsotopeStore.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MIsotopeStore__
#define __MIsotopeStore__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MIsotope.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A loader for a list of isotopes in standard MEGAlib format
class MIsotopeStore
{
  // public interface:
 public:
  //! Default constructor
  MIsotopeStore();
  //! Default destuctor 
  virtual ~MIsotopeStore();

  //! Load all isotopes form a standard MEGAlib isotope file
  bool Load(MString FileName, bool ExcludeExcludedLines = false);

  //! Return the number of isotopes
  unsigned int GetNumberOfIsotopes() const { return m_Isotopes.size(); }
  
  //! Return true if the store contains the isotope
  bool Contains(const MString& Name) const;
  
  //! Return the isotope with the given name
  //! If there is non throw an exception of type MExceptionIndexOutOfBounds
  const MIsotope& Get(const MString& Name) const;
  
  //! Return the isotope at the given position
  //! If there is non throw an exception of type MExceptionIndexOutOfBounds
  const MIsotope& Get(unsigned int i) const;
  
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
  //! The list of isotopes
  vector<MIsotope> m_Isotopes;


#ifdef ___CLING___
 public:
  ClassDef(MIsotopeStore, 0) // no description
#endif

};

//! Stream an isotope store
ostream& operator<<(ostream& os, const MIsotopeStore& IsotopeStore);

#endif


////////////////////////////////////////////////////////////////////////////////
