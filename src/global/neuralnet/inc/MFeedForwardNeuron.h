/*
 * MFeedForwardNeuron.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFeedForwardNeuron__
#define __MFeedForwardNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFeedForwardNeuron : public MNeuron
{
  // public interface:
public:
  //! Default constructor
  MFeedForwardNeuron();
  //! Copy constructor
  MFeedForwardNeuron(const MFeedForwardNeuron& N);
  //! Standard constructor
  MFeedForwardNeuron(int NValues, int NErrors);
  
  //! Set all parameters
  virtual void Set(int NValues = 1, int NErrors = 1);
  
  //! The Run routine:
  virtual void Run(int Mode = 0);
  
  
  
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
  ClassDef(MFeedForwardNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
