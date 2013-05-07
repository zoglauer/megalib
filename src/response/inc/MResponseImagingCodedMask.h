/*
 * MResponseImagingCodedMask.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingCodedMask__
#define __MResponseImagingCodedMask__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseImaging.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingCodedMask : public MResponseImaging
{
  // public interface:
 public:
  MResponseImagingCodedMask();
  virtual ~MResponseImagingCodedMask();

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseImagingCodedMask() {};
  //MResponseImagingCodedMask(const MResponseImagingCodedMask& ResponseImagingCodedMask) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseImagingCodedMask, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
