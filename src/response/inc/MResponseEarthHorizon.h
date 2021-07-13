/*
 * MResponseEarthHorizon.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseEarthHorizon__
#define __MResponseEarthHorizon__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseEarthHorizon : public MResponseBase
{
  // public interface:
 public:
  MResponseEarthHorizon();
  virtual ~MResponseEarthHorizon();

  //! Do all the response creation
  virtual bool CreateResponse();


  // protected methods:
 protected:
  //MResponseEarthHorizon() {};
  //MResponseEarthHorizon(const MResponseEarthHorizon& ResponseEarthHorizon) {};

  //! Load the simulation file in revan and mimrec as well as the configuration files:
  virtual bool OpenFiles();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MResponseEarthHorizon, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
