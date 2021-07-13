/*
 * MDummy.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDummy__
#define __MDummy__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MDummy
{
  // public interface:
 public:
  //! Default constructor
  MDummy();
  //! Default destuctor 
  virtual ~MDummy();


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MDummy, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
