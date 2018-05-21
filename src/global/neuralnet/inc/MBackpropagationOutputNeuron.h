/*
 * MBackpropagationOutputNeuron.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackpropagationOutputNeuron__
#define __MBackpropagationOutputNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MBackpropagationNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackpropagationOutputNeuron : public MBackpropagationNeuron
{
  // public interface:
public:
  //! Standard constructor
  MBackpropagationOutputNeuron();
  //! Copy constructor
  MBackpropagationOutputNeuron(const MBackpropagationOutputNeuron& N);
  //! Standard constructor
  MBackpropagationOutputNeuron(double LearningRate, double Momentum, int NValues = 1, int NErrors = 1);
  
  //! Set all parameters
  virtual void Set(double LearningRate, double Momentum, int NValues = 1, int NErrors = 1);
  
  //! Learn
  virtual void Learn(int Mode = 0);
  //! Compute the error
  virtual double ComputeError(int Mode = 0);
  
  //! Stream from the file
  virtual bool Stream(MFile& S, const int Version, const bool Read);
  //! Parse an individual line
  virtual int ParseLine(MString Line);
  //! Dump the content into a string 
  virtual MString ToString() const;
  
  // protected methods:
protected:
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  //! The learning rate
  double m_LearningRate;
  
  
  #ifdef ___CLING___
public:
  ClassDef(MBackpropagationOutputNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
