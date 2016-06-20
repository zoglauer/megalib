/*
 * MLMLClassicEM.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MLMLClassicEM__
#define __MLMLClassicEM__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MLMLAlgorithms.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MLMLClassicEM : public MLMLAlgorithms
{
  // public interface:
 public:
  //! Default constructor
  MLMLClassicEM();
  //! Default destructor
  virtual ~MLMLClassicEM();

  //! Perform one iteration
  virtual bool DoOneIteration();

  //! Set the response matrix
  virtual bool SetResponseSlices(vector<MBPData*>& Data, int NImageBins);

  //! return the initial image
  virtual double* GetInitialImage();

  // protected methods:
 protected:
  virtual void Convolve();
  virtual void Deconvolve();

  // private methods:
 private:



  // protected members:
 protected:
  //! view port factor: probability that the event came from within the image, not from outside
  double* m_Vi;
  //! Expectation or correction image
  double* m_Ej;               

  //! Initial likelihood of the image
  double m_InitialLikelihood; 
  //! Likelihood of the last iteration
  double m_LastLikelihood;
  //! Likelihood of the current (finished) iteration
  double m_CurrentLikelihood; 


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MLMLClassicEM, 0) // my "classic" EM (almost equivalent with Richardson-Lucy)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
