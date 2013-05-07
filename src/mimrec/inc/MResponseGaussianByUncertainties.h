/*
 * MResponseGaussianByUncertainties.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseGaussianByUncertainties__
#define __MResponseGaussianByUncertainties__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseGaussian.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseGaussianByUncertainties : public MResponseGaussian
{
  // public interface:
 public:
  MResponseGaussianByUncertainties();
  virtual ~MResponseGaussianByUncertainties();

  bool AnalyzeEvent(MPhysicalEvent* Event);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MResponseGaussianByUncertainties, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
