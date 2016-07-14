/*
 * MResponseImagingARM.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingARM__
#define __MResponseImagingARM__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingARM : public MResponseBase
{
  // public interface:
 public:
  MResponseImagingARM();
  virtual ~MResponseImagingARM();

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseImagingARM() {};
  //MResponseImagingARM(const MResponseImagingARM& ResponseImagingARM) {};

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
  ClassDef(MResponseImagingARM, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
