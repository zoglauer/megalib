/*
 * MAdalineNeuron.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MAdalineNeuron__
#define __MAdalineNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFeedForwardNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MAdalineNeuron : public MFeedForwardNeuron
{
  // public interface:
public:
  //! Default constructor
  MAdalineNeuron();
  //! Standard constructor
  MAdalineNeuron(double LearningRate);
  
  //! The Run routine:
  virtual void Learn(int Mode = 0);
  
  
  // protected methods:
protected:
  virtual double TransferFunction(double Value);
  
  // private methods:
private:
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  static const int c_ValueIndex = 0;
  static const int c_LearningRateIndex = 1;
  static const int c_ErrorIndex = 0;
  
  #ifdef ___CINT___
public:
  ClassDef(MAdalineNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
