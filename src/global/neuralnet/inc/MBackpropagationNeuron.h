/*
 * MBackpropagationNeuron.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackpropagationNeuron__
#define __MBackpropagationNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFeedForwardNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackpropagationNeuron : public MFeedForwardNeuron
{
  // public interface:
public:
  //! Standard constructor
  MBackpropagationNeuron();
  
  //! Copy constructor
  MBackpropagationNeuron(const MBackpropagationNeuron& N);
  
  //! Standard constructor
  MBackpropagationNeuron(int NValues, int NErrors);
  
  //! Set all parameters
  virtual void Set(int NValues = 1, int NErrors = 1);
  
  // protected methods:
protected:
  //! The transfer function
  virtual double TransferFunction(double Value);
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  
  
  
  #ifdef ___CLING___
public:
  ClassDef(MBackpropagationNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
