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


//! Calculate the Compton response (cone width) using the known position & energy uncertainties
class MResponseGaussianByUncertainties : public MResponseGaussian
{
  // public interface:
 public:
  //! Default constructor
  MResponseGaussianByUncertainties();
  //! Default destructor
  virtual ~MResponseGaussianByUncertainties();

  //! Set a fixed increase (worsening) of the calculates cone width
  void SetIncrease(double Increase) { m_Increase = Increase; }
  
  //! Set this events data
  virtual bool AnalyzeEvent(MPhysicalEvent* Event);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The fixed worsening of the calculated cone width
  double m_Increase;

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MResponseGaussianByUncertainties, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
