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
  //! Do the convolution
  virtual void Convolve(unsigned int Start, unsigned int Stop);
  //! Handle convolution multi-threaded
  virtual void ConvolveMultiThreaded();
  //! Entry point for the convolution thread
  virtual void ConvolveThreadEntry(unsigned int ThreadID, unsigned int Start, unsigned int Stop);

  //! Do the deconvolution
  virtual void Deconvolve(unsigned int Start, unsigned int Stop);
  //! Handle deconvolution multi-threaded
  virtual void DeconvolveMultiThreaded();
  //! Entry point for the deconvolution thread
  virtual void DeconvolveThreadEntry(unsigned int ThreadID, unsigned int Start, unsigned int Stop);

  //! Reset the expection array to zero
  virtual void ResetExpectation();
  //! Do the final image correction
  virtual void CorrectImage();


  // private methods:
 private:



  // protected members:
 protected:
  //! view port factor: probability that the event came from within the image, not from outside
  double* m_Vi;
  //! Expectation or correction image
  double* m_Ej;
  //! Expectation or correction image - one per thread
  vector<vector<double>> m_tEj;
  
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
