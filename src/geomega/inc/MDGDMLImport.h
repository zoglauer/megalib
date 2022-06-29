/*
 * MDGDMLImport.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDGDMLImport__
#define __MDGDMLImport__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TGeoManager.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MDGDMLImport
{
  // public interface:
 public:
  //! Default constructor
  MDGDMLImport();
  //! Default destuctor 
  virtual ~MDGDMLImport();

  //! Parse the GDMl file
  bool Parse(MString FileName);
  
  //! Return the data as Geomega file
  MString GetAsGeomega();
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
   TGeoManager* m_GeoManager;


#ifdef ___CLING___
 public:
  ClassDef(MDGDMLImport, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
