/*
 * MBackpropagationMiddleNeuron.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackpropagationMiddleNeuron__
#define __MBackpropagationMiddleNeuron__


////////////////////////////////////////////////////////////////////////////////



// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackpropagationOutputNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackpropagationMiddleNeuron : public MBackpropagationOutputNeuron
{
  // public interface:
public:
  //! Default constructor
  MBackpropagationMiddleNeuron();
  //! Copy constructor
  MBackpropagationMiddleNeuron(const MBackpropagationMiddleNeuron& N);
  //! Standard constructor
  MBackpropagationMiddleNeuron(double LearningRate, double Momentum, int NValues, int NErrors);
  
  //! Sety all parameters
  virtual void Set(double LearningRate, double Momentum, int NValues, int NErrors);
  //! Compute the error
  virtual double ComputeError(int Mode = 0);
  
  
  // protected methods:
protected:
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  
  
  
  #ifdef ___CLING___
public:
  ClassDef(MBackpropagationMiddleNeuron, 0) // no description
  #endif
  
};


#endif


////////////////////////////////////////////////////////////////////////////////

