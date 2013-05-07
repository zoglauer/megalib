/*
 * MResponseImagingBinnedMode.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingBinnedMode__
#define __MResponseImagingBinnedMode__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseImaging.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingBinnedMode : public MResponseImaging
{
  // public interface:
 public:
  MResponseImagingBinnedMode();
  virtual ~MResponseImagingBinnedMode();

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseImagingBinnedMode() {};
  //MResponseImagingBinnedMode(const MResponseImagingBinnedMode& ResponseImagingBinnedMode) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseImagingBinnedMode, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
