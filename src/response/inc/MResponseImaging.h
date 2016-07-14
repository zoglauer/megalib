/*
 * MResponseImaging.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImaging__
#define __MResponseImaging__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImaging : public MResponseBase
{
  // public interface:
 public:
  MResponseImaging();
  virtual ~MResponseImaging();

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseImaging() {};
  //MResponseImaging(const MResponseImaging& ResponseImaging) {};

  //! Load the simulation file in revan and mimrec as well as the configuration files:
  virtual bool OpenFiles();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseImaging, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
