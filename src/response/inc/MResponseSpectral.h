/*
 * MResponseSpectral.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseSpectral__
#define __MResponseSpectral__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseSpectral : public MResponseBase
{
  // public interface:
 public:
  MResponseSpectral();
  virtual ~MResponseSpectral();

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseSpectral() {};
  //MResponseSpectral(const MResponseSpectral& ResponseSpectral) {};

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
  ClassDef(MResponseSpectral, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
