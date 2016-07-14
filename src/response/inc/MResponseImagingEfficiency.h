/*
 * MResponseImagingEfficiency.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingEfficiency__
#define __MResponseImagingEfficiency__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingEfficiency : public MResponseBase
{
  // public interface:
 public:
  MResponseImagingEfficiency();
  virtual ~MResponseImagingEfficiency();

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseImagingEfficiency() {};
  //MResponseImagingEfficiency(const MResponseImagingEfficiency& ResponseImagingEfficiency) {};

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
  ClassDef(MResponseImagingEfficiency, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
